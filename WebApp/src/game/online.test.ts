import { afterEach, describe, expect, it } from 'vitest';
import { decodeConfig, encodeConfig } from '../data/config';
import { Input } from '../input/input';
import { F6 } from '../input/keys';
import { MemoryStore } from '../online/kvStore';
import type { Graph } from '../render/graph';
import { Color } from '../render/palette';
import { viewportFromViewport } from '../render/screen';
import type { ViewSys } from '../render/viewsys';
import { Game } from './game';
import type { OnlineStoreFactory } from './online';
import { GameState, PLAYER_SLOTS } from './state';

const SCREEN = { x1: 0, y1: 0, x2: 639, y2: 349 };
const FAST = { pollMs: 2, idlePollMs: 10, idleAfterMs: 1000, beatMs: 10, staleMs: 500, settleMs: 20 };

/** A game with no screen, playing online through `store`. */
function makeGame(store: MemoryStore, seed: number) {
  const state = new GameState();
  const windows = [
    { x1: 0, y1: 0, x2: 0.6, y2: 0.08 },
    { x1: 0, y1: 0.09, x2: 0.6, y2: 1 },
  ].map((vw) => ({ vp: viewportFromViewport(vw, SCREEN) }));
  const vs = { windows, display: () => true, windowChanged: () => {} } as unknown as ViewSys;
  let mask: readonly number[] | null = null;
  const g = { getFillMask: () => mask, setFillMask: (m: readonly number[] | null) => (mask = m) } as unknown as Graph;
  const input = new Input();
  const game = new Game(state, g, vs, input);
  game.speaker.muted = true;
  game.animateDrops = false;
  game.onlineStore = (() => ({ store })) satisfies OnlineStoreFactory;
  game.onlineTimings = FAST;
  state.aiSpeed = 0; // the opponent's token goes straight to its column
  const said: string[] = [];
  const add = state.msgbox.fixedAddLine.bind(state.msgbox);
  state.msgbox.fixedAddLine = (line: string) => (said.push(line), add(line));
  game.start(seed);
  games.push(game);
  return { game, state, input, said };
}

const games: Game[] = [];
afterEach(() => {
  for (const game of games.splice(0)) {
    game.stopped = true;
    game.leaveOnline();
    game.input.poke();
  }
});

const delay = (ms: number) => new Promise((r) => setTimeout(r, ms));

async function until(test: () => boolean, ms = 3000): Promise<void> {
  const end = Date.now() + ms;
  while (!test()) {
    if (Date.now() > end) throw new Error('Timed out');
    await delay(2);
  }
}

/** The board as colours, row by row: what each screen shows. */
function colours(state: GameState): string {
  const rows: string[] = [];
  for (let r = 0; r < state.gridRows; r++) {
    const row: string[] = [];
    for (let c = 0; c < state.gridCols; c++) row.push(String(state.playgrid.statusOfCR(c, r)?.color ?? '.'));
    rows.push(row.join(' '));
  }
  return rows.join('\n');
}

/** Stops the game when its first game ends, keeping the board as it was. */
function stopAtGameEnd(game: Game, state: GameState) {
  const result = { board: '', winner: '' };
  const drop = game.dropToken.bind(game);
  game.dropToken = async () => {
    const r = await drop();
    if (state.winOccurred || state.isDraw) {
      result.board = colours(state);
      result.winner = state.isDraw ? 'draw' : String(state.player(state.playerCurrent).color);
      game.stopped = true;
    }
    return r;
  };
  return result;
}

/** A human pressing player one's keys (a, d, s: left, right, drop) at random on their turn. */
function human(game: Game, state: GameState, input: Input) {
  let seed = 7;
  const timer = setInterval(() => {
    if (!game.ingame || state.playerCurrent !== PLAYER_SLOTS.one || input.kbhit()) return;
    seed = (Math.imul(seed, 1103515245) + 12345) & 0x7fffffff;
    input.pushKey('adss'.charCodeAt((seed >> 16) % 4));
  }, 3);
  return () => clearInterval(timer);
}

describe('online play', () => {
  it('a human and an AI on two devices play a game, and both boards match', async () => {
    const store = new MemoryStore();
    const a = makeGame(store, 1);
    a.state.player(PLAYER_SLOTS.one).color = Color.YELLOW;
    const b = makeGame(store, 2);
    b.game.setP1(PLAYER_SLOTS.aiNormal);
    b.state.player(PLAYER_SLOTS.aiNormal).color = Color.BLUE;

    const connectA = a.game.goOnline('Test Room', 2);
    await delay(40);
    const connectB = b.game.goOnline('test room', 2);
    const [ra, rb] = await Promise.all([connectA, connectB]);
    expect(ra?.kind).toBe('started');
    expect(rb?.kind).toBe('started');

    // Each device: its own player one, and the other's as Online in the other's colour.
    expect(a.state.playerTwo).toBe(PLAYER_SLOTS.online);
    expect(a.state.player(PLAYER_SLOTS.online).color).toBe(Color.BLUE);
    expect(b.state.player(PLAYER_SLOTS.online).color).toBe(Color.YELLOW);
    expect(a.said).toContain('Connected as host');
    expect(b.said).toContain('Connected as guest');
    // Each is told who they are playing, and the toolbar says so.
    expect(a.said).toContain('~Opponent: AI Normal');
    expect(b.said).toContain('~Opponent: Human');
    let toolbarA = { one: '', two: '' };
    a.game.onView = (v) => (toolbarA = v.players);
    a.game.setViewMode('normal');
    expect(toolbarA).toEqual({ one: 'Human', two: 'Online (AI Normal)' });
    // The same player starts on both: one device's player one is the other's Online.
    expect(a.state.playerCurrent === PLAYER_SLOTS.one).toBe(b.state.playerCurrent === PLAYER_SLOTS.online);

    const endA = stopAtGameEnd(a.game, a.state);
    const endB = stopAtGameEnd(b.game, b.state);
    const stopHuman = human(a.game, a.state, a.input);
    await Promise.all([a.game.run(), b.game.run()]);
    stopHuman();
    expect(endA.board).not.toBe('');
    expect(endA.board).toBe(endB.board);
    expect(endA.winner).toBe(endB.winner);
    // Each device tells its own player whether they won or lost.
    if (endA.winner !== 'draw') {
      const aWon = endA.winner === String(Color.YELLOW);
      expect(a.said).toContain(aWon ? '~You Win!' : '~You Lose');
      expect(b.said).toContain(aWon ? '~You Lose' : '~You Win!');
      expect(a.said.some((l) => l.endsWith(' Wins'))).toBe(false);
    }
  });

  it('aborts on both devices when the boards differ, and puts the players back', async () => {
    const store = new MemoryStore();
    const a = makeGame(store, 3);
    const b = makeGame(store, 4);
    b.game.gameSize12x13x6();
    const connectA = a.game.goOnline('room', 2);
    await delay(40);
    const [ra, rb] = await Promise.all([connectA, b.game.goOnline('room', 2)]);
    expect(ra?.kind).toBe('mismatch');
    expect(rb?.kind).toBe('mismatch');
    expect(a.said).toContain('You 7x8@4, them 12x13@6');
    expect(b.said).toContain('You 12x13@6, them 7x8@4');
    for (const d of [a, b]) {
      expect(d.game.online).toBeNull();
      expect(d.state.playerTwo).toBe(PLAYER_SLOTS.two);
    }
  });

  it('a guest with the host\'s colour takes another, on both screens', async () => {
    const store = new MemoryStore();
    const a = makeGame(store, 5);
    const b = makeGame(store, 6);
    a.state.player(PLAYER_SLOTS.one).color = Color.RED;
    b.state.player(PLAYER_SLOTS.one).color = Color.RED;
    const connectA = a.game.goOnline('room', 2);
    await delay(40);
    await Promise.all([connectA, b.game.goOnline('room', 2)]);
    expect(b.state.player(PLAYER_SLOTS.one).color).toBe(7);
    expect(a.state.player(PLAYER_SLOTS.online).color).toBe(7);
    expect(b.said).toContain('Your colour changed');
  });

  it('ends the game on the other device when one player leaves', async () => {
    const store = new MemoryStore();
    const a = makeGame(store, 7);
    const b = makeGame(store, 8);
    const connectA = a.game.goOnline('room', 2);
    await delay(40);
    await Promise.all([connectA, b.game.goOnline('room', 2)]);
    const runB = b.game.run();
    expect(b.game.ingame).toBe(true);
    a.game.endGame(); // F4 on A
    await until(() => !b.game.ingame);
    expect(b.said).toContain('~Opponent left');
    expect(b.game.online).toBeNull();
    expect(b.state.playerTwo).toBe(PLAYER_SLOTS.two);
    b.game.stopped = true;
    b.input.poke();
    await runB;
  });

  it('a game ended part way is lost by the one who ended it, and won by the other', async () => {
    const store = new MemoryStore();
    const a = makeGame(store, 13);
    const b = makeGame(store, 14);
    a.game.setP1(PLAYER_SLOTS.aiRandom);
    b.game.setP1(PLAYER_SLOTS.aiNormal);
    a.state.aiDelay = 50;
    b.state.aiDelay = 50;
    a.state.aiSpeed = 1;
    b.state.aiSpeed = 1;
    const connectA = a.game.goOnline('room', 2);
    await delay(40);
    await Promise.all([connectA, b.game.goOnline('room', 2)]);
    const runs = [a.game.run(), b.game.run()];
    const linkA = a.game.online!;
    await until(() => linkA.myMoves + linkA.theirMoves >= 2);
    a.game.endGame(); // F4 on A, part way through
    await until(() => !b.game.ingame);
    expect(a.said).toContain('~You Lose');
    expect(a.said).toContain('You ended the game');
    expect(b.said).toContain('~You Win!');
    expect(b.said).toContain('Opponent ended the game');
    expect(b.said).not.toContain('~Opponent left');
    // It counts: a win for B's player one, a game played on both.
    expect([b.state.playerOneScore, b.state.playerTwoScore, b.state.gamesPlayed]).toEqual([1, 0, 1]);
    expect([a.state.playerOneScore, a.state.playerTwoScore, a.state.gamesPlayed]).toEqual([0, 1, 1]);
    for (const d of [a, b]) {
      d.game.stopped = true;
      d.input.poke();
    }
    await Promise.all(runs);
  });

  it('gives up connecting when a new game is started instead', async () => {
    const store = new MemoryStore();
    const a = makeGame(store, 9);
    const connecting = a.game.goOnline('room', 1);
    expect(a.state.playerOne).toBe(PLAYER_SLOTS.online);
    a.game.newGame();
    expect(await connecting).toBeNull();
    expect(a.game.online).toBeNull();
    expect(a.state.playerOne).toBe(PLAYER_SLOTS.one);
  });

  it('saves the settings with Online as the human on its side', async () => {
    const store = new MemoryStore();
    const a = makeGame(store, 10);
    void a.game.goOnline('room', 2);
    expect(a.state.playerTwo).toBe(PLAYER_SLOTS.online);
    const bytes = encodeConfig(a.game);
    const b = makeGame(store, 11);
    expect(decodeConfig(bytes, b.game)).toBe(true);
    expect(b.state.playerTwo).toBe(PLAYER_SLOTS.two);
  });

  it('remembers new players at once, not only when the page closes', async () => {
    const store = new MemoryStore();
    const a = makeGame(store, 15);
    await a.game.extraFunctionality(F6); // demo: two AIs
    const bytes = a.game.configFile.read();
    expect(bytes).not.toBeNull();
    const b = makeGame(store, 16);
    expect(decodeConfig(bytes!, b.game)).toBe(true);
    expect([b.state.playerOne, b.state.playerTwo]).toEqual([a.state.playerOne, a.state.playerTwo]);
  });

  it('remembers which side is Online, until the online game ends', async () => {
    const store = new MemoryStore();
    const a = makeGame(store, 17);
    const b = makeGame(store, 18);
    expect(a.game.savedOnlineSide()).toBeNull();
    const connectA = a.game.goOnline('room', 1);
    await delay(40);
    await Promise.all([connectA, b.game.goOnline('room', 2)]);
    expect(a.game.savedOnlineSide()).toBe(1);
    expect(b.game.savedOnlineSide()).toBe(2);
    a.game.endGame(); // A leaves; B is told
    a.game.saveConfig();
    expect(a.game.savedOnlineSide()).toBeNull();
    const runB = b.game.run();
    await until(() => b.game.online === null);
    expect(b.game.savedOnlineSide()).toBeNull();
    b.game.stopped = true;
    b.input.poke();
    await runB;
  });

  it('will not save or load a game while online', async () => {
    const store = new MemoryStore();
    const a = makeGame(store, 12);
    void a.game.goOnline('room', 2);
    a.game.saveGame();
    a.game.loadGame();
    expect(a.said.filter((l) => l === '~Not while online')).toHaveLength(2);
    expect(a.game.saveFile.read()).toBeNull();
  });
});
