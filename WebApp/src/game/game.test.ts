import { describe, expect, it } from 'vitest';
import { RULES_1998 } from '../core/ai';
import { PlayerType } from '../core/player';
import golden from '../core/__golden__/games.json';
import { Input } from '../input/input';
import { ESC, KP4 } from '../input/keys';
import type { Graph } from '../render/graph';
import { viewportFromViewport } from '../render/screen';
import type { ViewSys } from '../render/viewsys';
import { Game, startColumn } from './game';
import { GameState, GoesFirst } from './state';

const SCREEN = { x1: 0, y1: 0, x2: 639, y2: 349 };

/** A game with no screen: the window layout without drawing. */
function makeGame() {
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
  game.animateDrops = false; // the tests play whole games; the drops would take seconds
  return { game, state, input };
}

const tick = () => new Promise((r) => setTimeout(r, 0));

interface GoldenGame {
  cols: number;
  rows: number;
  connect: number;
  levels: [number, number];
  seed: number;
  first: number;
  moves: string;
  result: string;
}

describe('the main loop', () => {
  it('plays a golden game between two AIs exactly as the C did', async () => {
    // Golden games where player two moved first, as "Successive" gives a new start.
    const picks = (golden.games as unknown as GoldenGame[]).filter((g) => g.cols === 7 && g.first === 1 && g.levels[0] !== g.levels[1]).slice(0, 6);
    for (const g of picks) {
      const { game, state } = makeGame();
      game.aiRules = RULES_1998; // the golden games are the original's rules
      state.aiSpeed = 0; // Fastest: no "Press ESC" between AI games
      state.playerOne = 2 + g.levels[0];
      state.playerTwo = 2 + g.levels[1];
      state.playerCurrent = state.playerOne;
      const moves: number[] = [];
      const drop = game.dropToken.bind(game);
      game.dropToken = async () => {
        moves.push(state.keypos);
        const r = await drop();
        if (state.winOccurred || state.isDraw) game.stopped = true;
        return r;
      };
      game.start(g.seed);
      await game.run();
      const result = state.isDraw ? 'draw' : state.playerOneScore ? 'one' : 'two';
      expect({ moves: moves.join(' '), result }, `levels ${g.levels}, seed ${g.seed}`).toEqual({ moves: g.moves, result: g.result });
    }
  });

  it('moves and drops for a human from the keyboard', async () => {
    const { game, state, input } = makeGame();
    game.start(1);
    // "Successive" on a new start: player two (keypad keys) first, token in the middle (column 3).
    expect(state.playerCurrent).toBe(state.playerTwo);
    const run = game.run();
    input.pushKey(KP4);
    await tick();
    input.pushKey(13);
    await tick();
    await tick();
    expect(state.playgrid.statusOfCR(2, 7)).toBe(state.player(1));
    expect(state.playerCurrent).toBe(state.playerOne);
    game.stopped = true;
    input.pushKey(0);
    await run;
  });

  it('drops where the mouse clicks, and a tap moves before it drops', async () => {
    const { game, state, input } = makeGame();
    game.start(1);
    const run = game.run();
    const x = (col: number) => Math.round(((col + 0.5) * 384) / 7);
    input.pushButton(1, x(1), 200, 'mouse');
    await tick();
    await tick();
    expect(state.playgrid.statusOfCR(1, 7)).toBe(state.player(1));
    input.pushButton(1, x(5), 200, 'touch');
    await tick();
    expect(state.keypos).toBe(5);
    expect(state.playgrid.inCol(5)).toBe(0);
    input.pushButton(1, x(5), 200, 'touch');
    await tick();
    await tick();
    expect(state.playgrid.statusOfCR(5, 7)).toBe(state.player(0));
    game.stopped = true;
    input.pushKey(0);
    await run;
  });

  it('waits for Esc after a win, then starts the next game', async () => {
    const { game, state, input } = makeGame();
    game.start(1);
    state.goesFirst = GoesFirst.One;
    // Player two drops four in column 3 while player one plays column 0.
    for (let i = 0; i < 3; i++) {
      state.playgrid.insertTokenAtCR(3, 7 - i, state.player(1));
      state.playgrid.insertTokenAtCR(0, 7 - i, state.player(0));
    }
    const run = game.run();
    input.pushKey(13); // player two drops in column 3 and wins
    for (let i = 0; i < 5; i++) await tick();
    await new Promise((r) => setTimeout(r, 250)); // the win tune's length, muted: none
    expect(state.playerTwoScore).toBe(1);
    expect(state.msgbox.line(state.msgbox.size)).toBe('Human 1 Goes First'); // NewGame ran
    input.pushKey(97); // not Esc
    await tick();
    input.pushKey(ESC);
    await tick();
    await tick();
    expect(state.playgrid.tokens()).toBe(0);
    game.stopped = true;
    input.pushKey(0);
    await run;
  });
});

describe('NewGame', () => {
  it('picks at random for Loser goes first with no winner (known bug 1, fixed)', () => {
    const { game, state } = makeGame();
    state.goesFirst = GoesFirst.Loser;
    // Seed 1's first rand() is 16838: even, so 0 -> player one. Find a seed giving 1.
    let seed = 1;
    for (; ; seed++) {
      game.rng.seed(seed);
      if (game.rng.random(2) === 1) break;
    }
    // Random(2) is 1: player two goes first, whoever was current (the
    // original's missing () left the current player).
    game.rng.seed(seed);
    state.playerCurrent = state.playerOne;
    game.newGame();
    expect(state.playerCurrent).toBe(state.playerTwo);
    game.rng.seed(1); // Random(2) is 0: player one
    state.playerCurrent = state.playerTwo;
    game.newGame();
    expect(state.playerCurrent).toBe(state.playerOne);
  });

  it('alternates for Successive', () => {
    const { game, state } = makeGame();
    state.wentFirst = state.playerOne;
    game.newGame();
    expect(state.playerCurrent).toBe(state.playerTwo);
    game.newGame();
    expect(state.playerCurrent).toBe(state.playerOne);
  });
});

describe('DemoMode', () => {
  it('picks two different AIs', () => {
    const { game, state } = makeGame();
    game.rng.seed(5);
    game.demoMode(0);
    expect(state.playerOne).toBeGreaterThanOrEqual(2);
    expect(state.playerTwo).toBeGreaterThanOrEqual(2);
    expect(state.playerOne).not.toBe(state.playerTwo);
    expect(state.msgbox.visibleLines()).toContain('~Demo MODE!');
  });
});

describe('the starting column (known bug 2, fixed)', () => {
  it('is the middle column, the left of the two middle ones on even boards', () => {
    expect([3, 7, 8, 12, 50].map(startColumn)).toEqual([1, 3, 3, 5, 24]);
  });

  it('is the same for a new board and a loaded game', () => {
    const { game, state } = makeGame();
    game.start(1, { cols: 9, rows: 6, connect: 4 });
    expect(state.keypos).toBe(4);
    game.saveGame();
    game.resetGameSize(9, 6, 4);
    game.loadGame();
    expect(state.keypos).toBe(4);
  });
});

describe('a key waiting when a move is made (known bug 16, fixed)', () => {
  it('drops the token, then handles the key', async () => {
    const { game, state, input } = makeGame();
    game.start(1);
    const run = game.run();
    input.pushKey(13); // player two drops...
    input.pushKey(256 + 66); // ...with F8 (sound off) already waiting
    for (let i = 0; i < 4; i++) await tick();
    expect(state.playgrid.tokens()).toBe(1);
    expect(state.mute).toBe(true);
    game.stopped = true;
    input.pushKey(0);
    await run;
  });
});

describe('Suggest Move (known bug 15, fixed)', () => {
  it('hatches the token while it shows the suggestion', async () => {
    const { game, state } = makeGame();
    game.start(1);
    state.aiSpeed = 0;
    const masks: (readonly number[] | null)[] = [];
    const set = game.g.setFillMask.bind(game.g);
    game.g.setFillMask = (m) => {
      masks.push(m);
      set(m);
    };
    await game.suggestMove();
    expect(masks[0]).not.toBeNull();
    expect(masks.at(-1)).toBeNull();
  });
});

describe('save and load (F2, F3)', () => {
  it('restores the board and players, with the token in the middle', () => {
    const { game, state } = makeGame();
    game.start(1);
    state.playgrid.insertTokenAtCR(2, 7, state.player(0));
    game.saveGame();
    state.playgrid.empty();
    game.loadGame();
    expect(state.playgrid.statusOfCR(2, 7)).toBe(state.player(0));
    expect(state.keypos).toBe(3);
  });

  it('says Load Error with nothing saved', () => {
    const { game, state } = makeGame();
    game.loadGame();
    expect(state.msgbox.line(state.msgbox.size)).toBe('~Load Error!');
  });

  it("says Load Error for a damaged save, and Save Error when it can't be kept", () => {
    const { game, state } = makeGame();
    game.start(1);
    game.saveFile = { read: () => new Uint8Array([0, 0, 0, 9, 1, 2, 3]), write: () => false, remove: () => {} };
    game.loadGame();
    expect(state.msgbox.line(state.msgbox.size)).toBe('~Load Error!');
    game.saveGame();
    expect(state.msgbox.line(state.msgbox.size)).toBe('~Save Error!');
  });

  it('Ctrl-F9 removes the configuration and stops it being saved again', async () => {
    const { game, state } = makeGame();
    let removed = false;
    let writes = 0;
    game.configFile = { read: () => null, write: () => (writes++, true), remove: () => void (removed = true) };
    game.saveConfig();
    expect(writes).toBe(1);
    await game.extraFunctionality(256 + 102); // Ctrl-F9
    expect(removed).toBe(true);
    expect(state.msgbox.line(state.msgbox.size)).toBe('Config Removed...');
    game.saveConfig();
    expect(writes).toBe(1);
  });
});

describe('the pointer', () => {
  it('maps x over the board to columns, and -1 off the board', () => {
    const { game } = makeGame();
    expect(game.mouseCol(0, 100)).toBe(0);
    expect(game.mouseCol(383, 100)).toBe(6);
    expect(game.mouseCol(200, 10)).toBe(3); // over the entry row
    expect(game.mouseCol(500, 100)).toBe(-1);
  });
});

describe('log names and the player menu', () => {
  it('numbers same-type players 1 and 2, and names others by type', () => {
    const { game, state } = makeGame();
    expect(game.logName(state.playerOne)).toBe('Human 1');
    expect(game.logName(state.playerTwo)).toBe('Human 2');
    state.playerTwo = 6; // AI Genius
    expect(game.logName(state.playerOne)).toBe('Human');
    expect(game.logName(state.playerTwo)).toBe('AI Genius');
    state.playerOne = 6; // AI Genius vs AI Expert: both computers, so both numbered
    state.playerTwo = 5;
    expect(game.logName(state.playerOne)).toBe('AI Genius 1');
    expect(game.logName(state.playerTwo)).toBe('AI Expert 2');
  });

  it('locks the other player only when two humans are in a game', () => {
    const { game, state } = makeGame();
    const menu: Record<string, boolean> = {};
    game.ui = { ...game.ui, menuItemEnable: (sub, item, enable) => (menu[`${sub}.${item}`] = enable) } as typeof game.ui;
    game.ingame = true;
    state.playerCurrent = state.playerOne;
    game.updatePlayerMenu();
    expect(menu['2.0']).toBe(true);
    expect(menu['2.1']).toBe(false);
    state.playerTwo = 6; // an AI on the other side: both stay enabled
    game.updatePlayerMenu();
    expect(menu['2.1']).toBe(true);
  });
});

describe('the falling token', () => {
  it('takes 1 second a board for a human, scaled by AI Speed for a computer', () => {
    const { game, state } = makeGame();
    const human = state.player(state.playerOne);
    const ai = state.player(state.playerTwo);
    ai.type = PlayerType.Computer;
    expect(game.dropMs(human)).toBe(1000);
    state.aiSpeed = 2; // Normal
    expect(game.dropMs(ai)).toBe(1000);
    state.aiSpeed = 0; // Fastest: still animates, but quickly
    expect(game.dropMs(ai)).toBe(250);
    state.aiSpeed = 4; // Slowest
    expect(game.dropMs(ai)).toBe(4000);
  });

  it('is hidden in the grid while it falls, and lands where the grid says', async () => {
    const { game, state } = makeGame();
    game.animateDrops = true;
    state.keypos = 3;
    await game.dropToken();
    expect(state.falling).toBeNull();
    expect(state.playgrid.statusOfCR(3, state.playgrid.height - 1)).not.toBeNull();
  });
});

describe('the View option', () => {
  /** A game whose windows record whether they are active, as ViewSys does. */
  function viewGame() {
    const state = new GameState();
    const active = Array.from({ length: 11 }, () => true);
    const views: { cropped: boolean; fullscreen: boolean; readout: string }[] = [];
    const vs = {
      windows: active.map(() => ({ vp: viewportFromViewport({ x1: 0, y1: 0, x2: 1, y2: 1 }, SCREEN) })),
      display: () => true,
      windowChanged: () => {},
      windowActivate: (i: number, on: boolean) => void (active[i] = on),
    } as unknown as ViewSys;
    const g = { getFillMask: () => null, setFillMask: () => {} } as unknown as Graph;
    const game = new Game(state, g, vs, new Input());
    game.onView = (v) => views.push(v);
    game.setScreenOn(true);
    return { game, active, views };
  }

  const INFO = [2, 3, 4, 5, 6, 7, 8, 9, 10];

  it('never crops the title screen', () => {
    const { game, active, views } = viewGame();
    game.setScreenOn(false);
    game.setViewMode('fullscreen');
    expect(active.every(Boolean)).toBe(true);
    expect(views.at(-1)).toEqual({ cropped: false, fullscreen: false, readout: '7x8@4', players: 'Human 1 vs Human 2' });
    game.setScreenOn(true);
    expect(active[INFO[0]]).toBe(false);
  });

  it('leaves out the info windows in Fullscreen, and shows them in Normal', () => {
    const { game, active, views } = viewGame();
    game.setViewMode('fullscreen');
    expect(active.slice(0, 2)).toEqual([true, true]);
    for (const i of INFO) expect(active[i]).toBe(false);
    expect(views.at(-1)).toEqual({ cropped: true, fullscreen: true, readout: '7x8@4', players: 'Human 1 vs Human 2' });

    game.setViewMode('normal');
    expect(active.every(Boolean)).toBe(true);
    expect(views.at(-1)).toEqual({ cropped: false, fullscreen: false, readout: '7x8@4', players: 'Human 1 vs Human 2' });
  });

  it('gives the whole screen back while a menu or dialog is open', async () => {
    const { game, active } = viewGame();
    game.setViewMode('fullscreen');
    let during: boolean[] = [];
    await game.withOverlay(
      () => {},
      async () => {
        during = active.slice();
      },
    );
    expect(during.every(Boolean)).toBe(true);
    expect(game.cropped).toBe(true);
    expect(active[INFO[0]]).toBe(false);
  });
});

describe('the entry token', () => {
  it('leaves the entry row when a token drops, and returns for the next player', async () => {
    const { game, state } = makeGame();
    game.resetGameSize(7, 8, 4);
    game.newGame();
    const first = game.currentPlayer();
    expect(state.entrygrid.statusOfCR(state.keypos, 0)).toBe(first);

    await game.dropToken();
    expect(state.entrygrid.statusOfCR(state.keypos, 0)).toBeNull();
    expect(game.currentPlayer()).not.toBe(first);

    // The next player's turn starts: their token appears over the same column.
    (game as unknown as { showEntryToken(): void }).showEntryToken();
    expect(state.entrygrid.statusOfCR(state.keypos, 0)).toBe(game.currentPlayer());
  });
});
