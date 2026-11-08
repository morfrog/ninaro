import { describe, expect, it } from 'vitest';
import { BinFile } from './binfile';
import { Direction, Grid, funcEqual, funcNotEqual } from './grid';
import { PlayerGroup } from './player';
import { board, o, picture, x } from './testBoard';

describe('Grid counts', () => {
  it('counts tokens by column, row and in total', () => {
    const g = board('...', 'X..', 'XO.');
    expect([g.inCol(0), g.inCol(1), g.inCol(2)]).toEqual([2, 1, 0]);
    expect([g.inRow(0), g.inRow(1), g.inRow(2)]).toEqual([0, 1, 2]);
    expect([g.notInCol(0), g.notInRow(2)]).toEqual([1, 1]);
    expect(g.tokens()).toBe(3);
    expect(g.maxTokens()).toBe(9);
  });

  it('returns -1 for columns and rows past the end', () => {
    const g = new Grid(3, 2);
    expect([g.inCol(3), g.notInCol(3), g.inRow(2), g.notInRow(2)]).toEqual([-1, -1, -1, -1]);
  });

  it('only inserts into empty cells', () => {
    const g = new Grid(2, 2);
    expect(g.insertTokenAtCR(0, 1, x)).toBe(true);
    expect(g.insertTokenAtCR(0, 1, o)).toBe(false);
    expect(g.insertTokenAtCR(1, 1, null)).toBe(false);
    expect(g.tokens()).toBe(1);
  });

  it('removes tokens and reports who was there', () => {
    const g = board('X.', 'XO');
    expect(g.removeTokenAtCR(1, 1)).toBe(o);
    expect(g.removeTokenAtCR(1, 1)).toBe(null);
    expect(g.tokens()).toBe(2);
    expect(g.inCol(1)).toBe(0);
  });

  it('resets a cell, inserting if it was empty', () => {
    const g = board('X.');
    expect(g.resetTokenAtCR(0, 0, o)).toBe(true);
    expect(g.resetTokenAtCR(1, 0, x)).toBe(false);
    expect(picture(g)).toEqual(['OX']);
    expect(g.tokens()).toBe(2);
  });

  it('empties', () => {
    const g = board('XO', 'OX');
    g.empty();
    expect(picture(g)).toEqual(['..', '..']);
    expect([g.tokens(), g.inCol(0), g.inRow(1)]).toEqual([0, 0, 0]);
  });
});

describe('moveTokenAtCR (the entry row)', () => {
  it('moves left and right within the grid', () => {
    const g = board('.X.');
    expect(g.moveTokenAtCR(1, 0, Direction.Left)).toBe(true);
    expect(picture(g)).toEqual(['X..']);
    expect(g.moveTokenAtCR(0, 0, Direction.Left)).toBe(false);
    expect([g.inCol(0), g.inCol(1)]).toEqual([1, 0]);
  });

  it('will not move onto another token', () => {
    const g = board('XO');
    expect(g.moveTokenAtCR(0, 0, Direction.Right)).toBe(false);
  });

  it('moves up and down, keeping row counts', () => {
    const g = board('X', '.');
    expect(g.moveTokenAtCR(0, 0, Direction.Down)).toBe(true);
    expect([g.inRow(0), g.inRow(1)]).toEqual([0, 1]);
  });
});

describe('funcNumInDirection', () => {
  const g = board('XXX.O', 'X....');

  it('counts matching cells up to the first that differs', () => {
    expect(g.funcNumInDirection(0, 0, 1, 0, x, funcNotEqual)).toBe(2);
    expect(g.funcNumInDirection(3, 0, -1, 0, x, funcNotEqual)).toBe(3);
  });

  it('stops at the edge', () => {
    expect(g.funcNumInDirection(0, 0, 0, 1, x, funcNotEqual)).toBe(1);
    expect(g.funcNumInDirection(0, 0, -1, 0, x, funcNotEqual)).toBe(0);
  });

  it('with funcEqual counts cells until the player is found', () => {
    expect(g.funcNumInDirection(0, 0, 1, 0, o, funcEqual)).toBe(3);
  });
});

describe('numsInDirection', () => {
  const g = board('.X.X.O.');

  it('counts each kind until the stop token', () => {
    expect(g.numsInDirection(0, 0, 1, 0, x, null, o, 10)).toEqual({ number: 4, p1i: 2, p2i: 2 });
  });

  it('stops after istop cells', () => {
    expect(g.numsInDirection(0, 0, 1, 0, x, null, o, 3)).toEqual({ number: 3, p1i: 2, p2i: 1 });
  });

  it('stops after one cell when istop is -1, as the C did', () => {
    expect(g.numsInDirection(0, 0, 1, 0, x, null, o, -1)).toEqual({ number: 1, p1i: 1, p2i: 0 });
  });

  it('stops at the edge', () => {
    expect(g.numsInDirection(6, 0, 1, 0, x, null, o, 10)).toEqual({ number: 0, p1i: 0, p2i: 0 });
  });
});

describe('save and load', () => {
  it('round-trips through bytes, column by column', () => {
    const pg = new PlayerGroup([o, x]);
    const g = board('..O', 'XOX');
    const bf = new BinFile(g.saveSize());
    g.save(bf, pg);
    expect(bf.remain()).toBe(0);
    // width 3, height 2, then col 0 rows 0..1, col 1, col 2: pid + 1.
    const ints = Array.from({ length: 8 }, (_, i) => new DataView(bf.store.buffer).getInt32(i * 4));
    expect(ints).toEqual([3, 2, 0, 2, 0, 1, 1, 2]);
    const loaded = Grid.load(new BinFile(bf.store), pg);
    expect(picture(loaded)).toEqual(['..O', 'XOX']);
    expect(loaded.tokens()).toBe(4);
  });
});
