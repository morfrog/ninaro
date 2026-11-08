import { describe, expect, it } from 'vitest';
import { Input } from '../input/input';
import { ENTER, ESC, KP2, KP4, KP6, KP8 } from '../input/keys';
import { Color, EGA } from '../render/palette';
import type { ViewSys } from '../render/viewsys';
import { nullGraph, nullPage } from '../testFakes';
import { Game } from './game';
import { GameState, GoesFirst, PLAYER_SLOTS } from './state';
import { NinaroUi } from './ui';

function makeUi() {
  const state = new GameState();
  const g = nullGraph();
  const vs = {
    main: { x1: 0, y1: 0, x2: 639, y2: 349 },
    current: nullPage(),
    windows: [{ vp: { x1: 0, y1: 0, x2: 383, y2: 27 } }, { vp: { x1: 0, y1: 31, x2: 383, y2: 349 } }],
    display: () => true,
    windowChanged: () => {},
  } as unknown as ViewSys;
  const input = new Input();
  const game = new Game(state, g, vs, input);
  game.speaker.muted = true;
  const ui = new NinaroUi(game, '~Help\r\n');
  game.ui = ui;
  game.start(1);
  return { game, state, ui, input };
}

/** Starts a call, types the keys, and waits for it. */
async function typing<T>(input: Input, call: Promise<T>, ...keys: number[]): Promise<T> {
  for (const k of keys) {
    input.pushKey(k);
    await new Promise((r) => setTimeout(r, 0));
  }
  return call;
}

describe('the Custom Board dialog', () => {
  it("shows the board's Connect the first time too (known bug 17, fixed)", async () => {
    const { ui, state, input } = makeUi();
    expect(state.win.tokens).toBe(4);
    const call = ui.gameSizeCustom();
    await new Promise((r) => setTimeout(r, 0));
    expect(ui.boardDialog.control(2).current).toBe(4);
    await typing(input, call, ESC);
  });

  it('changes the board on OK, keeping Connect within the larger side', async () => {
    const { ui, state, input } = makeUi();
    // Up to Connect, right to 4..? then to Rows: down the board.
    await typing(input, ui.gameSizeCustom(), KP8, KP8, KP8, KP4, KP4, KP4, KP4, KP4, ENTER, 'o'.charCodeAt(0));
    // Rows 8 -> 3; Connect follows to at most max(3, 7) = 7.
    expect(state.gridRows).toBe(3);
    expect(state.gridCols).toBe(7);
    expect(state.playgrid.rows()).toBe(3);
  });
});

describe('the Player dialog', () => {
  it('keeps the two players from sharing a colour', async () => {
    const { ui, state, input } = makeUi();
    // Player two is RED; player one (YELLOW) moves left onto RED, so skips it.
    await typing(input, ui.pDialog(1), KP8, KP4, 'o'.charCodeAt(0));
    expect(state.player(0).color).not.toBe(Color.RED);
    expect(state.player(0).color).toBe(Color.YELLOW); // no colour left of red: stays
  });

  it('gives player two a copy when both pick the same AI', async () => {
    const { game, ui, state, input } = makeUi();
    game.setP1(PLAYER_SLOTS.aiGenius);
    // Player two's dialog: up to the names, right to the last (AI Genius), OK.
    await typing(input, ui.pDialog(2), KP8, KP8, KP6, KP6, KP6, KP6, KP6, 'o'.charCodeAt(0));
    expect(state.playerTwo).toBe(PLAYER_SLOTS.copy);
    expect(state.player(PLAYER_SLOTS.copy).name).toBe('AI Genius');
    expect(state.copyOf).toBe(PLAYER_SLOTS.aiGenius);
  });
});

describe('the options dialogs', () => {
  it('sets goes first, AI speed and best of', async () => {
    const { ui, state, input } = makeUi();
    await typing(input, ui.playerOptions(), KP8, KP6, KP8, KP6, KP8, KP6, 'o'.charCodeAt(0));
    expect(state.bestOf).toBe(0); // already Infinity, the last: no further
    expect(state.aiSpeed).toBe(1); // Normal -> Fast
    expect(state.goesFirst).toBe(GoesFirst.Random); // Successive -> Random
  });

  it('swaps black and white for a white background', async () => {
    const { game, ui, state, input } = makeUi();
    await typing(input, ui.gameOptions(), KP8, KP8, KP8, KP6, 'o'.charCodeAt(0));
    expect(state.back).toBe(Color.WHITE);
    expect(game.g.palette.value(Color.BLACK)).toBe(EGA._BRIGHTWHITE);
    expect(game.g.palette.value(Color.WHITE)).toBe(EGA._BLACK);
  });
});

describe('the menu', () => {
  it('ends the game from Game > End and starts one from Game > New', async () => {
    const { game, ui, input } = makeUi();
    expect(game.ingame).toBe(true);
    await typing(input, ui.menuCall(), ENTER); // End is current during a game
    expect(game.ingame).toBe(false);
    expect(ui.menu.subs[0].current).toBe(0); // New
    await typing(input, ui.menuCall(), ENTER);
    expect(game.ingame).toBe(true);
  });

  it('starts with New and Player One disabled, as ProgInit did', () => {
    const { ui } = makeUi();
    const fresh = new NinaroUi(makeUi().game, '');
    expect(fresh.menu.subs[0].items[0].enabled).toBe(false);
    expect(fresh.menu.subs[2].items[0].enabled).toBe(false);
    expect(ui.menu.subs[3].items.map((i) => i.name)).toEqual(['Contents', 'About']);
  });

  it('runs Exit through onExit', async () => {
    const { ui, input } = makeUi();
    let exited = false;
    ui.onExit = async () => void (exited = true);
    ui.menu.current = 0;
    ui.menu.subs[0].current = 7;
    await typing(input, ui.menuCall(), ENTER);
    expect(exited).toBe(true);
  });

  it('opens with the keys of whoever is moving (KP2 moves down for player two)', () => {
    const { game } = makeUi();
    expect(game.pkeys.key).toContain(KP2);
  });
});
