// One online session between two devices (ONLINE-PLAN.md): who is host,
// the handshake, the heartbeat, and passing moves. Each side writes only its
// own key (ninaro1-<room>-h or -g) and reads only the other's, and every
// write is that side's whole state, so a missed read loses nothing.

import { KvError, abortableSleep, type KvStore } from './kvStore';
import {
  COUNTER_LIMIT, encodeMessage, newNonce, parseMessage, roomKey, sameBoard, type Board, type Message, type Role,
} from './protocol';

export interface SessionTimings {
  /** Poll while waiting on the other side. */
  pollMs: number;
  /** Poll while waiting a long time for an opponent, or just watching for one leaving. */
  idlePollMs: number;
  /** How long a host waits for an opponent at the fast rate. */
  idleAfterMs: number;
  /** How often our heartbeat goes up. */
  beatMs: number;
  /** A side whose message hasn't changed this long is gone (or its device asleep). */
  staleMs: number;
  /** After writing to become host, wait this long and read back, to settle a race. */
  settleMs: number;
}

export const DEFAULT_TIMINGS: SessionTimings = {
  pollMs: 1000,
  idlePollMs: 5000,
  idleAfterMs: 120_000,
  beatMs: 5000,
  staleMs: 20_000,
  settleMs: 2000,
};

export type SessionEvent =
  /** Host, waiting for an opponent to join. */
  | { type: 'hosting' }
  /** Found a host, saying hello. */
  | { type: 'joining' }
  /** The opponent's heartbeat stopped (true), or came back (false). */
  | { type: 'offline'; offline: boolean }
  /** The opponent quit. */
  | { type: 'left' };

export type ConnectResult =
  | {
      kind: 'started';
      role: Role;
      /** Who goes first in game 1. */
      first: Role;
      myColor: number;
      theirColor: number;
      /** Our colour was the host's, so we (the guest) took another. */
      colorChanged: boolean;
      /** Who plays on the opponent's device: 0 a human, 1..5 an AI (KIND_NAMES). */
      theirKind: number;
    }
  | { kind: 'mismatch'; mine: Board; theirs: Board }
  /** Two others are already playing in this room. */
  | { kind: 'busy' };

/** The opponent quit (or ended the game) while we waited for their move. */
export class OpponentLeft extends Error {
  constructor() {
    super('Opponent left');
  }
}

/** The opponent's moves don't follow ours: the two games are out of step. */
export class OutOfStep extends Error {}

export interface SessionOptions {
  store: KvStore;
  room: string;
  board: Board;
  /** Our token colour, palette index 0..15. */
  color: number;
  /** Who plays on this device: 0 a human, 1..5 an AI (KIND_NAMES). */
  kind?: number;
  /** Ends the session: every wait and loop stops. The caller should give the store the same signal. */
  signal?: AbortSignal;
  timings?: Partial<SessionTimings>;
  onEvent?: (e: SessionEvent) => void;
  random?: () => number;
  now?: () => number;
  sleep?: (ms: number, signal?: AbortSignal) => Promise<void>;
  /** For pagehide: one try at a write that can outlive the page. */
  putNow?: (key: string, value: string) => Promise<boolean>;
}

type HandshakeResult = ConnectResult | 'retry';

/** The colour a guest takes when it has the host's: as setP1_2, white, or 1..6 if it was white. */
export function giveWayColor(color: number, random: () => number): number {
  return color === 7 ? 1 + Math.floor(random() * 6) : 7;
}

export class OnlineSession {
  readonly room: string;
  readonly board: Board;
  readonly timings: SessionTimings;
  role: Role | null = null;
  myColor: number;
  theirColor = -1;
  /** Who goes first in game 1 (later games alternate). */
  firstOfSession: Role | null = null;
  private readonly store: KvStore;
  private readonly signal?: AbortSignal;
  private readonly onEvent: (e: SessionEvent) => void;
  private readonly random: () => number;
  private readonly now: () => number;
  private readonly sleepFn: (ms: number, signal?: AbortSignal) => Promise<void>;
  private readonly putNowFn?: (key: string, value: string) => Promise<boolean>;
  private readonly nonce: string;
  /** Our state, as written to our key. */
  private me: Message;
  /** Bumped each time `me` changes; `written` is the last one stored. */
  private version = 0;
  private written = 0;
  private writing: Promise<void> | null = null;
  /** Hosts found to be gone: a key still holding one of these is free to take. */
  private readonly deadHosts = new Set<string>();
  private beating = false;
  private watching = false;
  private ended = false;
  private offline = false;

  constructor(options: SessionOptions) {
    this.store = options.store;
    this.room = options.room;
    this.board = options.board;
    this.myColor = options.color;
    this.signal = options.signal;
    this.timings = { ...DEFAULT_TIMINGS, ...options.timings };
    this.onEvent = options.onEvent ?? (() => {});
    this.random = options.random ?? Math.random;
    this.now = options.now ?? (() => Date.now());
    this.sleepFn = options.sleep ?? abortableSleep;
    this.putNowFn = options.putNow;
    this.nonce = newNonce(this.random);
    this.me = {
      role: 'h',
      nonce: this.nonce,
      peer: null,
      board: options.board,
      color: options.color,
      kind: options.kind ?? 0,
      phase: 'W',
      game: 0,
      move: 0,
      col: 0,
      beat: 0,
      first: null,
    };
  }

  private key(role: Role): string {
    return roomKey(this.room, role);
  }

  private get myKey(): string {
    return this.key(this.role!);
  }

  private get theirKey(): string {
    return this.key(this.role === 'h' ? 'g' : 'h');
  }

  private sleep(ms: number): Promise<void> {
    return this.sleepFn(ms, this.signal);
  }

  private checkAborted(): void {
    if (this.signal?.aborted) throw this.signal.reason;
  }

  // ---- Writing our state ----

  /** Changes our state; flush() writes it. */
  private update(changes: Partial<Message>): void {
    this.me = { ...this.me, ...changes };
    this.version++;
  }

  /**
   * Resolves once our latest state is stored. One write at a time, always of
   * the latest state, so a heartbeat can never land after (and undo) a move.
   */
  private async flush(): Promise<void> {
    const target = this.version;
    while (this.written < target) {
      this.writing ??= this.writeLoop();
      await this.writing;
    }
  }

  private async writeLoop(): Promise<void> {
    try {
      while (this.written < this.version) {
        const v = this.version;
        await this.store.put(this.myKey, encodeMessage(this.me));
        this.written = v;
      }
    } finally {
      this.writing = null;
    }
  }

  /** Raises our heartbeat every beatMs until the session ends. */
  private startBeats(): void {
    if (this.beating) return;
    this.beating = true;
    void (async () => {
      try {
        while (this.beating && !this.signal?.aborted) {
          await this.sleep(this.timings.beatMs);
          if (!this.beating) break;
          this.update({ beat: (this.me.beat + 1) % COUNTER_LIMIT });
          await this.flush();
        }
      } catch {
        // Aborted: the session is over.
      }
    })();
  }

  private stopBeats(): void {
    this.beating = false;
  }

  /** The other side's message, or null if there is none (or it isn't valid). */
  private async read(key: string): Promise<Message | null> {
    return parseMessage(await this.store.get(key));
  }

  // ---- Connecting ----

  /**
   * Finds or becomes the host, then shakes hands. Resolves when game 1 can
   * start, or with why it can't.
   */
  async connect(): Promise<ConnectResult> {
    for (;;) {
      this.checkAborted();
      const found = await this.elect();
      if (found === 'busy') return { kind: 'busy' };
      const result = found === 'host' ? await this.hostHandshake() : await this.guestHandshake(found);
      if (result !== 'retry') {
        if (result.kind === 'started') this.startWatching();
        else this.stopBeats();
        return result;
      }
      this.stopBeats();
    }
  }

  /**
   * Who is host. A live waiting host: we join it. A live host in a game with
   * someone else: busy. No host, or one gone stale: we become host, and if
   * another device wrote at the same moment, the last write wins.
   */
  private async elect(): Promise<'host' | 'busy' | Message> {
    for (;;) {
      this.checkAborted();
      const hKey = this.key('h');
      const host = await this.read(hKey);
      const live = host && host.role === 'h' && !this.deadHosts.has(host.nonce) && host.phase !== 'X' && host.phase !== 'Q';
      if (live && host.phase === 'W') return host;
      if (live && (await this.staysAlive(hKey, host))) return 'busy';

      // Become host.
      this.role = 'h';
      this.update({ role: 'h', peer: null, phase: 'W', game: 0, move: 0, col: 0, first: null, color: this.myColor });
      await this.flush();
      await this.sleep(this.timings.settleMs);
      const after = await this.read(hKey);
      if (after?.nonce === this.nonce) return 'host';
      // Another device became host at the same moment, and won: go round and join it.
    }
  }

  /** Whether a message changes within staleMs (its heartbeat goes up): the side is still there. */
  private async staysAlive(key: string, seen: Message): Promise<boolean> {
    const since = this.now();
    while (this.now() - since < this.timings.staleMs) {
      await this.sleep(this.timings.pollMs);
      const m = await this.read(key);
      if (!m || m.nonce !== seen.nonce) return false;
      if (m.beat !== seen.beat || m.phase !== seen.phase || m.move !== seen.move || m.game !== seen.game) return true;
    }
    this.deadHosts.add(seen.nonce);
    return false;
  }

  /** Host: wait for a guest's hello, check its board, then start or abort. */
  private async hostHandshake(): Promise<HandshakeResult> {
    this.onEvent({ type: 'hosting' });
    this.startBeats();
    const since = this.now();
    for (;;) {
      const guest = await this.read(this.key('g'));
      if (guest && guest.role === 'g' && guest.peer === this.nonce && guest.phase === 'W') {
        this.theirColor = guest.color;
        if (!sameBoard(guest.board, this.board)) {
          this.update({ phase: 'X', peer: guest.nonce });
          await this.flush();
          return { kind: 'mismatch', mine: this.board, theirs: guest.board };
        }
        const first: Role = this.random() < 0.5 ? 'h' : 'g';
        this.firstOfSession = first;
        this.update({ phase: 'S', peer: guest.nonce, game: 1, move: 0, col: 0, first });
        await this.flush();
        return { kind: 'started', role: 'h', first, myColor: this.myColor, theirColor: guest.color, colorChanged: false, theirKind: guest.kind };
      }
      // We may have gone stale (a sleeping phone) and been replaced as host.
      const self = await this.read(this.key('h'));
      if (self && self.nonce !== this.nonce) return 'retry';
      const waited = this.now() - since;
      await this.sleep(waited < this.timings.idleAfterMs ? this.timings.pollMs : this.timings.idlePollMs);
    }
  }

  /** Guest: say hello to the host, then wait for its start (or abort). */
  private async guestHandshake(host: Message): Promise<HandshakeResult> {
    this.onEvent({ type: 'joining' });
    this.role = 'g';
    let colorChanged = false;
    if (this.myColor === host.color) {
      this.myColor = giveWayColor(this.myColor, this.random);
      colorChanged = true;
    }
    this.theirColor = host.color;
    this.update({ role: 'g', peer: host.nonce, phase: 'W', game: 0, move: 0, col: 0, first: null, color: this.myColor });
    await this.flush();
    this.startBeats();
    const hKey = this.key('h');
    let last = host;
    let lastChange = this.now();
    for (;;) {
      await this.sleep(this.timings.pollMs);
      const m = await this.read(hKey);
      if (!m || m.role !== 'h' || m.nonce !== host.nonce) return 'retry'; // the host is gone, or another took over
      if (m.beat !== last.beat || m.phase !== last.phase) lastChange = this.now();
      last = m;
      this.theirColor = m.color;
      if (m.peer === this.nonce) {
        if (m.phase === 'X') return { kind: 'mismatch', mine: this.board, theirs: m.board };
        if (m.phase === 'Q') return 'retry';
        if (m.phase === 'S' || m.phase === 'P') {
          // A host already playing has made the first move: it went first.
          const first: Role = m.phase === 'S' ? m.first! : 'h';
          this.firstOfSession = first;
          return { kind: 'started', role: 'g', first, myColor: this.myColor, theirColor: m.color, colorChanged, theirKind: m.kind };
        }
      } else if (m.phase === 'S' || m.phase === 'P') {
        return { kind: 'busy' }; // another guest got there first
      }
      if (this.now() - lastChange >= this.timings.staleMs) {
        this.deadHosts.add(host.nonce);
        return 'retry';
      }
    }
  }

  // ---- Playing ----

  /** Who goes first in a game: game 1 as the host chose, then turn about. */
  firstIn(game: number): Role {
    const first = this.firstOfSession!;
    return game % 2 === 1 ? first : first === 'h' ? 'g' : 'h';
  }

  /** Whether we go first in a game. */
  meFirstIn(game: number): boolean {
    return this.firstIn(game) === this.role;
  }

  /** Our move: our `move`th in `game`, in column `col`. Resolves once it is stored. */
  async sendMove(game: number, move: number, col: number): Promise<void> {
    this.update({ phase: 'P', game, move, col, first: null });
    await this.flush();
  }

  /**
   * Waits for the opponent's `move`th move in `game`, and returns its
   * column. Rejects with OpponentLeft if they quit or abort, and OutOfStep
   * if their moves have got ahead of ours.
   */
  async waitMove(game: number, move: number): Promise<number> {
    let lastChange = this.now();
    let lastRaw = '';
    for (;;) {
      this.checkAborted();
      if (this.ended) throw new OpponentLeft();
      const raw = await this.store.get(this.theirKey);
      const m = parseMessage(raw);
      if (m && this.fromOpponent(m)) {
        if (raw !== lastRaw) {
          lastRaw = raw;
          lastChange = this.now();
          this.setOffline(false);
        }
        if (m.phase === 'Q' || m.phase === 'X') {
          this.opponentLeft();
          throw new OpponentLeft();
        }
        if (m.phase === 'P') {
          if (m.game === game && m.move === move) return m.col;
          if (m.game > game || (m.game === game && m.move > move)) {
            throw new OutOfStep(`Expected their move ${move} of game ${game}, got move ${m.move} of game ${m.game}`);
          }
        }
      }
      if (this.now() - lastChange >= this.timings.staleMs) this.setOffline(true);
      await this.sleep(this.timings.pollMs);
    }
  }

  /** A message from the side we are playing (not a stranger in the same room). */
  private fromOpponent(m: Message): boolean {
    return m.role !== this.role && m.peer === this.nonce && m.nonce === this.me.peer;
  }

  private setOffline(offline: boolean): void {
    if (offline === this.offline) return;
    this.offline = offline;
    this.onEvent({ type: 'offline', offline });
  }

  private opponentLeft(): void {
    if (this.ended) return;
    this.ended = true;
    this.stopBeats();
    this.watching = false;
    this.onEvent({ type: 'left' });
  }

  /** Whether the opponent has quit. */
  get opponentGone(): boolean {
    return this.ended;
  }

  /** While playing, checks now and then that the opponent hasn't quit (on our turn, nobody else is reading). */
  private startWatching(): void {
    if (this.watching) return;
    this.watching = true;
    void (async () => {
      try {
        while (this.watching && !this.signal?.aborted) {
          await this.sleep(this.timings.idlePollMs);
          if (!this.watching) break;
          const m = await this.read(this.theirKey);
          if (m && this.fromOpponent(m) && (m.phase === 'Q' || m.phase === 'X')) this.opponentLeft();
        }
      } catch (e) {
        if (e instanceof KvError) this.watching = false;
        // Otherwise aborted: the session is over.
      }
    })();
  }

  /** We leave: writes Q so the other side knows, and stops. */
  async quit(): Promise<void> {
    this.stopBeats();
    this.watching = false;
    if (this.role === null) return;
    this.update({ phase: 'Q', first: null });
    await this.flush();
  }

  /** The page is closing: one try at writing Q that can outlive the page. */
  quitNow(): void {
    this.stopBeats();
    this.watching = false;
    if (this.role === null || !this.putNowFn) return;
    this.update({ phase: 'Q', first: null });
    void this.putNowFn(this.myKey, encodeMessage(this.me));
  }

  /** Stops the heartbeat and watching (the caller aborts the signal to stop requests in flight). */
  close(): void {
    this.stopBeats();
    this.watching = false;
  }
}
