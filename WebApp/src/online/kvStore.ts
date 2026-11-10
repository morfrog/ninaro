// The shared key-value store that carries online games between devices
// (ONLINE-PLAN.md). The rest of the online code sees only KvStore, so the
// service behind it can be swapped.

/** A tiny shared store: one short string per key. */
export interface KvStore {
  /** The key's value, or '' if it has never been written. */
  get(key: string): Promise<string>;
  /** Sets the key's value. */
  put(key: string, value: string): Promise<void>;
}

/** Values go in the URL path: `.` and `:` break the service, these are safe. */
const SAFE_VALUE = /^[A-Za-z0-9,_|~-]{1,64}$/;
const SAFE_KEY = /^[a-z0-9-]{1,64}$/;

export function safeValue(value: string): boolean {
  return SAFE_VALUE.test(value);
}

export function safeKey(key: string): boolean {
  return SAFE_KEY.test(key);
}

/** A request the service turned down for good (bad key or value), not a passing failure. */
export class KvError extends Error {}

/** keyvalue.immanuel.co, with the app key made for Ninaro (anyone can read it: it ships in the page). */
export const IMMANUEL_BASE = 'https://keyvalue.immanuel.co/api/KeyVal';
export const IMMANUEL_APP_KEY = 'ilddtv8p';

/** Waits ms, or rejects with the signal's reason when it is aborted. */
export function abortableSleep(ms: number, signal?: AbortSignal): Promise<void> {
  return new Promise((resolve, reject) => {
    if (signal?.aborted) return reject(signal.reason);
    const timer = setTimeout(done, ms);
    function done() {
      signal?.removeEventListener('abort', aborted);
      resolve();
    }
    function aborted() {
      clearTimeout(timer);
      reject(signal!.reason);
    }
    signal?.addEventListener('abort', aborted, { once: true });
  });
}

export interface ImmanuelOptions {
  fetch?: typeof fetch;
  base?: string;
  appKey?: string;
  /** Stops all requests, and any retrying, at once. */
  signal?: AbortSignal;
  /** How long one request may take. */
  timeoutMs?: number;
  /** The first wait before a retry; it doubles up to maxDelayMs. */
  firstDelayMs?: number;
  maxDelayMs?: number;
  /** After this long failing, onTrouble(true); onTrouble(false) when a request gets through again. */
  troubleMs?: number;
  onTrouble?: (trouble: boolean) => void;
  sleep?: (ms: number, signal?: AbortSignal) => Promise<void>;
  random?: () => number;
  now?: () => number;
}

/**
 * keyvalue.immanuel.co. About half its requests fail with 503 in bursts, so
 * every get and put retries, with backoff and jitter, until it gets through
 * or the signal aborts. Nothing gives up on its own.
 */
export class ImmanuelStore implements KvStore {
  private readonly fetch: typeof fetch;
  private readonly base: string;
  private readonly appKey: string;
  private readonly signal?: AbortSignal;
  private readonly timeoutMs: number;
  private readonly firstDelayMs: number;
  private readonly maxDelayMs: number;
  private readonly troubleMs: number;
  private readonly onTrouble: (trouble: boolean) => void;
  private readonly sleep: (ms: number, signal?: AbortSignal) => Promise<void>;
  private readonly random: () => number;
  private readonly now: () => number;
  /** When requests started failing (null while they get through). */
  private failingSince: number | null = null;
  private inTrouble = false;

  constructor(options: ImmanuelOptions = {}) {
    this.fetch = options.fetch ?? ((input, init) => globalThis.fetch(input, init));
    this.base = options.base ?? IMMANUEL_BASE;
    this.appKey = options.appKey ?? IMMANUEL_APP_KEY;
    this.signal = options.signal;
    this.timeoutMs = options.timeoutMs ?? 8000;
    this.firstDelayMs = options.firstDelayMs ?? 300;
    this.maxDelayMs = options.maxDelayMs ?? 4000;
    this.troubleMs = options.troubleMs ?? 20000;
    this.onTrouble = options.onTrouble ?? (() => {});
    this.sleep = options.sleep ?? abortableSleep;
    this.random = options.random ?? Math.random;
    this.now = options.now ?? (() => Date.now());
  }

  async get(key: string): Promise<string> {
    if (!safeKey(key)) throw new KvError(`Bad key: ${key}`);
    const url = `${this.base}/GetValue/${this.appKey}/${key}`;
    return this.retry(async () => {
      const value = parseJsonString(await this.attempt(url, { method: 'GET' }));
      if (value === null) throw new Error('Not a value');
      return value;
    });
  }

  async put(key: string, value: string): Promise<void> {
    await this.retry(async () => this.putOnce(key, value));
  }

  /**
   * One try at a put, with `keepalive` so it can outlive the page: for the
   * quit message when the page is closed. False if it didn't get through.
   */
  async putNow(key: string, value: string): Promise<boolean> {
    try {
      await this.putOnce(key, value, true);
      return true;
    } catch {
      return false;
    }
  }

  /**
   * A POST with no body and no headers of our own is a "simple" request,
   * so the browser sends no CORS preflight first.
   */
  private async putOnce(key: string, value: string, keepalive = false): Promise<void> {
    if (!safeKey(key)) throw new KvError(`Bad key: ${key}`);
    if (!safeValue(value)) throw new KvError(`Bad value: ${value}`);
    const url = `${this.base}/UpdateValue/${this.appKey}/${key}/${value}`;
    const body = (await this.attempt(url, { method: 'POST', keepalive })).trim();
    if (body !== 'true') throw new Error('Not stored');
  }

  /** One request, with a timeout: the body, or an error. */
  private async attempt(url: string, init: RequestInit): Promise<string> {
    const outer = this.signal;
    if (outer?.aborted) throw outer.reason;
    const controller = new AbortController();
    const stop = () => controller.abort(outer?.reason);
    outer?.addEventListener('abort', stop, { once: true });
    const timer = setTimeout(() => controller.abort(new Error('Timed out')), this.timeoutMs);
    try {
      const res = await this.fetch(url, { ...init, cache: 'no-store', signal: controller.signal });
      const body = await res.text();
      // A 4xx other than timeout or too-many-requests won't get better by trying again.
      if (res.status >= 400 && res.status < 500 && res.status !== 408 && res.status !== 429) {
        throw new KvError(`HTTP ${res.status}`);
      }
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      return body;
    } finally {
      clearTimeout(timer);
      outer?.removeEventListener('abort', stop);
    }
  }

  /** Runs fn until it succeeds; a KvError or an abort ends it. */
  private async retry<T>(fn: () => Promise<T>): Promise<T> {
    let delay = this.firstDelayMs;
    for (;;) {
      if (this.signal?.aborted) throw this.signal.reason;
      try {
        const result = await fn();
        this.succeeded();
        return result;
      } catch (e) {
        // Aborted while a request was failing: report the abort, not the failure.
        if (this.signal?.aborted) throw this.signal.reason;
        if (e instanceof KvError) throw e;
        this.failed();
      }
      // Jitter: 75% to 125% of the delay, so two devices don't retry in step.
      await this.sleep(Math.round(delay * (0.75 + this.random() / 2)), this.signal);
      delay = Math.min(delay * 2, this.maxDelayMs);
    }
  }

  private succeeded(): void {
    this.failingSince = null;
    if (this.inTrouble) {
      this.inTrouble = false;
      this.onTrouble(false);
    }
  }

  private failed(): void {
    const now = this.now();
    this.failingSince ??= now;
    if (!this.inTrouble && now - this.failingSince >= this.troubleMs) {
      this.inTrouble = true;
      this.onTrouble(true);
    }
  }
}

/** The service answers a get with a JSON string; anything else (an error page) is null. */
function parseJsonString(body: string): string | null {
  try {
    const v: unknown = JSON.parse(body);
    return typeof v === 'string' ? v : null;
  } catch {
    return null;
  }
}

/** A store in memory, for tests: two sessions can share one. */
export class MemoryStore implements KvStore {
  readonly data = new Map<string, string>();

  async get(key: string): Promise<string> {
    return this.data.get(key) ?? '';
  }

  async put(key: string, value: string): Promise<void> {
    if (!safeValue(value)) throw new KvError(`Bad value: ${value}`);
    this.data.set(key, value);
  }
}
