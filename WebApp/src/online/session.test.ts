import { afterEach, describe, expect, it } from 'vitest';
import { ImmanuelStore, MemoryStore } from './kvStore';
import { encodeMessage, parseMessage, roomKey, type Board } from './protocol';
import { OnlineSession, OpponentLeft, giveWayColor, type ConnectResult, type SessionEvent } from './session';

const BOARD: Board = { cols: 7, rows: 8, connect: 4 };
const FAST = { pollMs: 2, idlePollMs: 10, idleAfterMs: 1000, beatMs: 10, staleMs: 500, settleMs: 20 };

/** A seeded random (mulberry32), so a failing run can be repeated. */
function seeded(seed: number): () => number {
  return () => {
    seed = (seed + 0x6d2b79f5) | 0;
    let t = Math.imul(seed ^ (seed >>> 15), 1 | seed);
    t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
}

/**
 * The service as the tests see it: a shared map behind the real
 * ImmanuelStore, answering about half of all requests with 503, as the live
 * one does.
 */
class FlakyService {
  readonly data = new MemoryStore();
  requests = 0;
  failures = 0;
  constructor(
    private readonly random: () => number,
    private readonly failRate = 0.5,
  ) {}

  readonly fetch = (async (url: string) => {
    this.requests++;
    await Promise.resolve();
    if (this.random() < this.failRate) {
      this.failures++;
      return new Response('<h2>Service Unavailable</h2>', { status: 503 });
    }
    const get = /\/GetValue\/app\/([^/]+)$/.exec(url);
    if (get) return new Response(JSON.stringify(await this.data.get(get[1])));
    const put = /\/UpdateValue\/app\/([^/]+)\/([^/]+)$/.exec(url);
    if (put) {
      await this.data.put(put[1], put[2]);
      return new Response('true');
    }
    return new Response('not found', { status: 404 });
  }) as unknown as typeof fetch;
}

const controllers: AbortController[] = [];
afterEach(() => {
  for (const c of controllers.splice(0)) c.abort(new Error('test over'));
});

function device(service: FlakyService, color: number, seed: number, board = BOARD, room = 'smithy', kind = 0) {
  const controller = new AbortController();
  controllers.push(controller);
  const events: SessionEvent[] = [];
  const store = new ImmanuelStore({
    fetch: service.fetch,
    appKey: 'app',
    base: 'https://kv.test',
    signal: controller.signal,
    firstDelayMs: 1,
    maxDelayMs: 4,
  });
  const session = new OnlineSession({
    store,
    room,
    board,
    color,
    kind,
    signal: controller.signal,
    timings: FAST,
    random: seeded(seed),
    onEvent: (e) => events.push(e),
  });
  return { session, events, controller };
}

const delay = (ms: number) => new Promise((r) => setTimeout(r, ms));

function started(r: ConnectResult) {
  if (r.kind !== 'started') throw new Error(`Not started: ${r.kind}`);
  return r;
}

describe('OnlineSession', () => {
  it('the first device hosts, the second joins, and they agree who goes first', async () => {
    const service = new FlakyService(seeded(1));
    const a = device(service, 14, 11);
    const b = device(service, 4, 12);
    const pa = a.session.connect();
    await delay(40);
    const [ra, rb] = (await Promise.all([pa, b.session.connect()])).map(started);
    expect(ra.role).toBe('h');
    expect(rb.role).toBe('g');
    expect(ra.first).toBe(rb.first);
    expect(ra.theirColor).toBe(4);
    expect(rb.theirColor).toBe(14);
    expect(a.events[0]).toEqual({ type: 'hosting' });
    expect(b.events[0]).toEqual({ type: 'joining' });
    expect(service.failures).toBeGreaterThan(0);
  });

  it('tells each side who plays on the other device', async () => {
    const service = new FlakyService(seeded(16));
    const a = device(service, 14, 161); // a human
    const b = device(service, 4, 162, BOARD, 'smithy', 5); // AI Genius
    const pa = a.session.connect();
    await delay(40);
    const [ra, rb] = (await Promise.all([pa, b.session.connect()])).map(started);
    expect(ra.theirKind).toBe(5);
    expect(rb.theirKind).toBe(0);
  });

  it('settles two devices connecting at the same moment: one host, one guest', async () => {
    for (const seed of [2, 3, 4, 5]) {
      const service = new FlakyService(seeded(seed));
      const a = device(service, 14, seed * 10 + 1);
      const b = device(service, 4, seed * 10 + 2);
      const [ra, rb] = (await Promise.all([a.session.connect(), b.session.connect()])).map(started);
      expect([ra.role, rb.role].sort(), `seed ${seed}`).toEqual(['g', 'h']);
      expect(ra.first).toBe(rb.first);
    }
  });

  it('aborts both sides when the boards differ', async () => {
    const service = new FlakyService(seeded(6));
    const a = device(service, 14, 61);
    const b = device(service, 4, 62, { cols: 12, rows: 13, connect: 6 });
    const pa = a.session.connect();
    await delay(40);
    const [ra, rb] = await Promise.all([pa, b.session.connect()]);
    expect(ra).toEqual({ kind: 'mismatch', mine: BOARD, theirs: { cols: 12, rows: 13, connect: 6 } });
    expect(rb).toEqual({ kind: 'mismatch', mine: { cols: 12, rows: 13, connect: 6 }, theirs: BOARD });
  });

  it('a guest with the host\'s colour gives way', async () => {
    const service = new FlakyService(seeded(7));
    const a = device(service, 4, 71);
    const b = device(service, 4, 72);
    const pa = a.session.connect();
    await delay(40);
    const [ra, rb] = (await Promise.all([pa, b.session.connect()])).map(started);
    expect(rb.colorChanged).toBe(true);
    expect(rb.myColor).toBe(7);
    expect(ra.theirColor).toBe(7);
    expect(ra.colorChanged).toBe(false);
    expect(ra.myColor).toBe(4);
  });

  it('giveWayColor: white, or 1..6 if it was white', () => {
    expect(giveWayColor(4, () => 0)).toBe(7);
    expect(giveWayColor(7, () => 0)).toBe(1);
    expect(giveWayColor(7, () => 0.999)).toBe(6);
  });

  it('plays two games of moves in step, turn about', async () => {
    const service = new FlakyService(seeded(8));
    const a = device(service, 14, 81);
    const b = device(service, 4, 82);
    const pa = a.session.connect();
    await delay(40);
    await Promise.all([pa, b.session.connect()]);

    /** One side's play: its moves, and the opponent's as it sees them. */
    async function play(session: OnlineSession, game: number, mine: number[]) {
      const seen: number[] = [];
      let myTurn = session.meFirstIn(game);
      let n = 0;
      for (let turn = 0; turn < 20; turn++) {
        if (myTurn) {
          n++;
          await session.sendMove(game, n, mine[n - 1]);
        } else {
          seen.push(await session.waitMove(game, seen.length + 1));
        }
        myTurn = !myTurn;
      }
      return seen;
    }

    const movesA = Array.from({ length: 10 }, (_, i) => i % 7);
    const movesB = Array.from({ length: 10 }, (_, i) => (13 - i) % 7);
    for (const game of [1, 2]) {
      expect(a.session.meFirstIn(game)).toBe(!b.session.meFirstIn(game));
      const [seenByA, seenByB] = await Promise.all([play(a.session, game, movesA), play(b.session, game, movesB)]);
      expect(seenByA).toEqual(movesB);
      expect(seenByB).toEqual(movesA);
    }
    expect(a.session.meFirstIn(1)).toBe(!a.session.meFirstIn(2));
  });

  it('tells the other side when one quits', async () => {
    const service = new FlakyService(seeded(9));
    const a = device(service, 14, 91);
    const b = device(service, 4, 92);
    const pa = a.session.connect();
    await delay(40);
    await Promise.all([pa, b.session.connect()]);
    const waiting = b.session.waitMove(1, 1);
    await a.session.quit();
    await expect(waiting).rejects.toBeInstanceOf(OpponentLeft);
    expect(b.events).toContainEqual({ type: 'left' });
  });

  it('notices a quit on its own turn, while not waiting', async () => {
    const service = new FlakyService(seeded(10));
    const a = device(service, 14, 101);
    const b = device(service, 4, 102);
    const pa = a.session.connect();
    await delay(40);
    await Promise.all([pa, b.session.connect()]);
    await a.session.quit();
    for (let i = 0; i < 100 && !b.session.opponentGone; i++) await delay(10);
    expect(b.session.opponentGone).toBe(true);
  });

  it('takes over a room whose host has gone', async () => {
    const service = new FlakyService(seeded(11), 0.3);
    // A host that waited and then went away: its heartbeat never moves again.
    const ghost = { role: 'h' as const, nonce: 'dead', peer: null, board: BOARD, color: 2, kind: 0, phase: 'W' as const, game: 0, move: 0, col: 0, beat: 5, first: null };
    await service.data.put(roomKey('smithy', 'h'), encodeMessage(ghost));
    const a = device(service, 14, 111);
    const pa = a.session.connect();
    // A joins the ghost, sees it is stale, and becomes host.
    for (let i = 0; i < 600 && a.events.filter((e) => e.type === 'hosting').length === 0; i++) await delay(5);
    expect(a.events.map((e) => e.type)).toEqual(['joining', 'hosting']);
    const b = device(service, 4, 112);
    const [ra, rb] = (await Promise.all([pa, b.session.connect()])).map(started);
    expect(ra.role).toBe('h');
    expect(rb.role).toBe('g');
  });

  it('takes over a room whose game was abandoned', async () => {
    const service = new FlakyService(seeded(12), 0.3);
    const ghost = { role: 'h' as const, nonce: 'dead', peer: 'gone', board: BOARD, color: 2, kind: 0, phase: 'P' as const, game: 3, move: 7, col: 2, beat: 50, first: null };
    await service.data.put(roomKey('smithy', 'h'), encodeMessage(ghost));
    const a = device(service, 14, 121);
    const pa = a.session.connect();
    for (let i = 0; i < 600 && a.events.length === 0; i++) await delay(5);
    expect(a.events[0]).toEqual({ type: 'hosting' });
    const b = device(service, 4, 122);
    const [ra, rb] = (await Promise.all([pa, b.session.connect()])).map(started);
    expect([ra.role, rb.role]).toEqual(['h', 'g']);
  });

  it('says the room is busy when two others are playing in it', async () => {
    const service = new FlakyService(seeded(13), 0.3);
    const a = device(service, 14, 131);
    const b = device(service, 4, 132);
    const pa = a.session.connect();
    await delay(40);
    await Promise.all([pa, b.session.connect()]);
    const c = device(service, 1, 133);
    expect(await c.session.connect()).toEqual({ kind: 'busy' });
    // C didn't disturb the game.
    const host = parseMessage(await service.data.get(roomKey('smithy', 'h')))!;
    const guest = parseMessage(await service.data.get(roomKey('smithy', 'g')))!;
    expect(host.peer).toBe(guest.nonce);
    expect(guest.peer).toBe(host.nonce);
  });

  it('keeps rooms apart', async () => {
    const service = new FlakyService(seeded(14), 0.3);
    const a = device(service, 14, 141, BOARD, 'one');
    const b = device(service, 4, 142, BOARD, 'two');
    a.session.connect().catch(() => {});
    b.session.connect().catch(() => {});
    for (let i = 0; i < 600 && (a.events.length === 0 || b.events.length === 0); i++) await delay(5);
    expect(a.events[0]).toEqual({ type: 'hosting' });
    expect(b.events[0]).toEqual({ type: 'hosting' });
  });

  it('reports an opponent gone quiet, and back', async () => {
    const service = new FlakyService(seeded(15), 0.2);
    const a = device(service, 14, 151);
    const b = device(service, 4, 152);
    const pa = a.session.connect();
    await delay(40);
    await Promise.all([pa, b.session.connect()]);
    a.session.close(); // A's heartbeat stops, as when its phone sleeps
    const waiting = b.session.waitMove(1, 1).catch(() => -1);
    for (let i = 0; i < 600 && !b.events.some((e) => e.type === 'offline'); i++) await delay(5);
    expect(b.events).toContainEqual({ type: 'offline', offline: true });
    if (a.session.meFirstIn(1)) {
      await a.session.sendMove(1, 1, 3);
      expect(await waiting).toBe(3);
      expect(b.events).toContainEqual({ type: 'offline', offline: false });
    }
  });
});
