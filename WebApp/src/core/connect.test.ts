import { describe, expect, it } from 'vitest';
import {
  InsertResult,
  checkForWin,
  gameLoad,
  gameSave,
  insertTokenInCol,
  insertTokenTarget,
  newWin,
  removeToken,
  removeTokenTarget,
  winPotential,
} from './connect';
import { Grid } from './grid';
import { PlayerGroup } from './player';
import { board, o, picture, x } from './testBoard';

const cells = (w: { win: { col: number; row: number }[] }) => w.win.map((t) => [t.col, t.row]);

describe('dropping tokens', () => {
  it('lands on the lowest empty row', () => {
    const g = board('...', '...', 'X..');
    expect([insertTokenTarget(g, 0), insertTokenTarget(g, 1)]).toEqual([1, 2]);
    expect([removeTokenTarget(g, 0), removeTokenTarget(g, 1)]).toEqual([2, 3]);
  });

  it('fails on a full column', () => {
    const g = board('X.', 'O.');
    expect(insertTokenTarget(g, 0)).toBe(-1);
    expect(insertTokenInCol(g, 0, x, newWin(2))).toBe(InsertResult.Failure);
    expect(g.tokens()).toBe(2);
  });

  it('succeeds without a win, and removeToken takes the top one back', () => {
    const g = new Grid(3, 3);
    expect(insertTokenInCol(g, 1, x, newWin(3))).toBe(InsertResult.Success);
    expect(insertTokenInCol(g, 1, o, newWin(3))).toBe(InsertResult.Success);
    expect(picture(g)).toEqual(['...', '.O.', '.X.']);
    expect(removeToken(g, 1)).toBe(o);
    expect(removeToken(g, 1)).toBe(x);
    expect(removeToken(g, 1)).toBe(null);
  });
});

describe('wins', () => {
  it('finds a vertical win, listing cells from the new token down', () => {
    const g = board('....', 'X...', 'X...', 'X...');
    const win = newWin(4);
    expect(insertTokenInCol(g, 0, x, win)).toBe(InsertResult.Win);
    expect(cells(win)).toEqual([[0, 0], [0, 1], [0, 2], [0, 3]]);
  });

  it('finds a horizontal win when the gap is filled', () => {
    const g = board('....', 'XX.X');
    const win = newWin(4);
    expect(insertTokenInCol(g, 2, x, win)).toBe(InsertResult.Win);
    expect(cells(win)).toEqual([[0, 1], [1, 1], [2, 1], [3, 1]]);
  });

  it('finds a / diagonal win', () => {
    const g = board('...', '.X.', 'OOX');
    const win = newWin(3);
    // (0, 0) completes (0,0) (1,1) (2,2).
    expect(checkForWin(g, 3, 0, 0, win, x)).toBe(true);
    expect(cells(win)).toEqual([[2, 2], [1, 1], [0, 0]]);
  });

  it('finds a \\ diagonal win', () => {
    const g = board('X..', 'OX.', 'OO.');
    const win = newWin(3);
    expect(insertTokenInCol(g, 2, x, win)).toBe(InsertResult.Win);
    expect(cells(win)).toEqual([[2, 2], [1, 1], [0, 0]]);
  });

  it("doesn't count the other player's tokens", () => {
    const g = board('XXO.OO');
    expect(checkForWin(g, 3, 3, 0, newWin(3), x)).toBe(false);
    expect(checkForWin(g, 3, 3, 0, newWin(3), o)).toBe(true);
  });

  it('keeps a longer run to `size` cells that include the new token', () => {
    const g = board('XXX.XXX');
    const win = newWin(4);
    expect(checkForWin(g, 4, 3, 0, win, x)).toBe(true);
    expect(cells(win)).toEqual([[0, 0], [1, 0], [2, 0], [3, 0]]);
  });
});

describe('winPotential', () => {
  it('scores an empty 7x8 board as the C did', () => {
    // Worked by hand from AnglePotential: (40 * sum) / (3 * 10 * 8).
    const g = new Grid(7, 8);
    const scores = [0, 1, 2, 3, 4, 5, 6].map((c) => winPotential(g, 4, c, 7, x, o, 40));
    expect(scores).toEqual([1, 1, 1, 2, 1, 1, 1]);
  });

  it('counts own tokens 10 each and empty cells 1 each, stopping at the opponent', () => {
    // Bottom row X X _ _ O, looking at column 2 with connect 3. Horizontal:
    // 2 X to the left, 1 empty to the right before O = 2*10 + 1 = 21.
    // Vertical and both diagonals have only 1 empty cell of room (< 3), so 0.
    const g = board('.....', 'XX..O');
    expect(winPotential(g, 3, 2, 1, x, o, 40)).toBe(Math.trunc((40 * 21) / (2 * 10 * 8)));
  });
});

describe('saved games', () => {
  it('round-trips NINARO.SAV', () => {
    const pg = new PlayerGroup([x, o]);
    const g = board('...', 'XO.');
    const bytes = gameSave(0, 1, 1, 3, g, pg);
    expect(bytes.length).toBe(16 + 4 * (2 + 6));
    const loaded = gameLoad(bytes, pg);
    expect([loaded.p1, loaded.p2, loaded.current, loaded.size]).toEqual([0, 1, 1, 3]);
    expect(picture(loaded.grid)).toEqual(['...', 'XO.']);
  });
});
