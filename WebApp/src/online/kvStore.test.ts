import { describe, expect, it } from 'vitest';
import { ImmanuelStore, KvError, MemoryStore, abortableSleep, safeValue } from './kvStore';

const UNAVAILABLE = () => new Response('<h2>Service Unavailable</h2>', { status: 503 });

/** A fetch that answers from a list in turn, recording each request. */
function scripted(answers: (() => Response | Promise<Response>)[]) {
  const calls: { url: string; init?: RequestInit }[] = [];
  const fetch = (async (url: string, init?: RequestInit) => {
    calls.push({ url, init });
    const answer = answers.shift();
    if (!answer) throw new Error('No more answers');
    return answer();
  }) as unknown as typeof globalThis.fetch;
  return { fetch, calls };
}

function store(fetch: typeof globalThis.fetch, extra: Partial<ConstructorParameters<typeof ImmanuelStore>[0]> = {}) {
  const sleeps: number[] = [];
  const s = new ImmanuelStore({
    fetch,
    appKey: 'app',
    base: 'https://kv.test/api',
    sleep: async (ms) => void sleeps.push(ms),
    random: () => 0.5, // no jitter
    ...extra,
  });
  return { s, sleeps };
}

describe('ImmanuelStore', () => {
  it('gets a value', async () => {
    const { fetch, calls } = scripted([() => new Response('"1,2,5"')]);
    const { s } = store(fetch);
    expect(await s.get('ninaro1-smithy-h')).toBe('1,2,5');
    expect(calls[0].url).toBe('https://kv.test/api/GetValue/app/ninaro1-smithy-h');
  });

  it('gets "" for a key never written', async () => {
    const { s } = store(scripted([() => new Response('""')]).fetch);
    expect(await s.get('k')).toBe('');
  });

  it('puts with a bodiless POST, so there is no CORS preflight', async () => {
    const { fetch, calls } = scripted([() => new Response('true')]);
    const { s } = store(fetch);
    await s.put('k', '1,h,ab12');
    expect(calls[0].url).toBe('https://kv.test/api/UpdateValue/app/k/1,h,ab12');
    expect(calls[0].init?.method).toBe('POST');
    expect(calls[0].init?.body).toBeUndefined();
    expect(calls[0].init?.headers).toBeUndefined();
  });

  it('retries 503s, network errors and error pages, doubling the wait up to the cap', async () => {
    const { fetch, calls } = scripted([
      UNAVAILABLE,
      () => Promise.reject(new TypeError('Failed to fetch')),
      () => new Response('<html>oops</html>'),
      UNAVAILABLE,
      UNAVAILABLE,
      UNAVAILABLE,
      () => new Response('"ok"'),
    ]);
    const { s, sleeps } = store(fetch, { firstDelayMs: 300, maxDelayMs: 4000 });
    expect(await s.get('k')).toBe('ok');
    expect(calls).toHaveLength(7);
    expect(sleeps).toEqual([300, 600, 1200, 2400, 4000, 4000]);
  });

  it('retries a put that was not stored', async () => {
    const { fetch, calls } = scripted([UNAVAILABLE, () => new Response('false'), () => new Response('true')]);
    const { s } = store(fetch);
    await s.put('k', 'v');
    expect(calls).toHaveLength(3);
  });

  it('jitters each wait between 75% and 125%', async () => {
    const low = store(scripted([UNAVAILABLE, () => new Response('"x"')]).fetch, { random: () => 0 });
    await low.s.get('k');
    const high = store(scripted([UNAVAILABLE, () => new Response('"x"')]).fetch, { random: () => 0.999999 });
    await high.s.get('k');
    expect(low.sleeps).toEqual([225]);
    expect(high.sleeps).toEqual([375]);
  });

  it('does not retry a 404 (a value the service cannot take)', async () => {
    const { fetch, calls } = scripted([() => new Response('not found', { status: 404 })]);
    const { s } = store(fetch);
    await expect(s.get('k')).rejects.toBeInstanceOf(KvError);
    expect(calls).toHaveLength(1);
  });

  it('refuses unsafe keys and values without asking the service', async () => {
    const { fetch, calls } = scripted([]);
    const { s } = store(fetch);
    await expect(s.put('k', 'a.b')).rejects.toBeInstanceOf(KvError);
    await expect(s.put('k', 'a:b')).rejects.toBeInstanceOf(KvError);
    await expect(s.put('k', 'x'.repeat(65))).rejects.toBeInstanceOf(KvError);
    await expect(s.get('Bad Key')).rejects.toBeInstanceOf(KvError);
    expect(calls).toHaveLength(0);
  });

  it('stops retrying when aborted', async () => {
    const controller = new AbortController();
    let n = 0;
    const fetch = (async () => {
      if (++n === 3) controller.abort(new Error('left'));
      return UNAVAILABLE();
    }) as unknown as typeof globalThis.fetch;
    const { s } = store(fetch, { signal: controller.signal });
    await expect(s.get('k')).rejects.toThrow('left');
    expect(n).toBe(3);
  });

  it('aborts a request in flight', async () => {
    const controller = new AbortController();
    const fetch = ((_url: string, init: RequestInit) =>
      new Promise((_resolve, reject) => {
        init.signal!.addEventListener('abort', () => reject(init.signal!.reason));
        controller.abort(new Error('left'));
      })) as unknown as typeof globalThis.fetch;
    const { s } = store(fetch, { signal: controller.signal });
    await expect(s.get('k')).rejects.toThrow('left');
  });

  it('times out a request that hangs, and tries again', async () => {
    let n = 0;
    const fetch = ((_url: string, init: RequestInit) => {
      if (++n === 2) return Promise.resolve(new Response('"late"'));
      return new Promise((_resolve, reject) => init.signal!.addEventListener('abort', () => reject(init.signal!.reason)));
    }) as unknown as typeof globalThis.fetch;
    const { s } = store(fetch, { timeoutMs: 5 });
    expect(await s.get('k')).toBe('late');
    expect(n).toBe(2);
  });

  it('reports trouble after failing for a while, and when it clears', async () => {
    let clock = 0;
    const events: boolean[] = [];
    const answers = [UNAVAILABLE, UNAVAILABLE, UNAVAILABLE, () => new Response('"x"')];
    const fetch = (async () => {
      clock += 10_000;
      return answers.shift()!();
    }) as unknown as typeof globalThis.fetch;
    const { s } = store(fetch, { now: () => clock, troubleMs: 20_000, onTrouble: (t) => events.push(t) });
    await s.get('k');
    expect(events).toEqual([true, false]);
  });

  it('putNow tries once, with keepalive, and says whether it got through', async () => {
    const ok = scripted([() => new Response('true')]);
    expect(await store(ok.fetch).s.putNow('k', 'v')).toBe(true);
    expect(ok.calls[0].init?.keepalive).toBe(true);
    const bad = scripted([UNAVAILABLE]);
    expect(await store(bad.fetch).s.putNow('k', 'v')).toBe(false);
    expect(bad.calls).toHaveLength(1);
  });
});

describe('safeValue', () => {
  it('allows what the service stores, up to 64 characters', () => {
    expect(safeValue('1,g,k3f9,7x8x4,c,P,2,11,5,0')).toBe(true);
    expect(safeValue('a-b_c~d|e')).toBe(true);
    expect(safeValue('x'.repeat(64))).toBe(true);
    expect(safeValue('')).toBe(false);
    expect(safeValue('a b')).toBe(false);
    expect(safeValue('a/b')).toBe(false);
  });
});

describe('MemoryStore', () => {
  it('keeps values, with "" for a key never written', async () => {
    const m = new MemoryStore();
    expect(await m.get('k')).toBe('');
    await m.put('k', 'v');
    expect(await m.get('k')).toBe('v');
  });
});

describe('abortableSleep', () => {
  it('rejects at once when aborted', async () => {
    const c = new AbortController();
    const p = abortableSleep(60_000, c.signal);
    c.abort(new Error('stop'));
    await expect(p).rejects.toThrow('stop');
  });
});
