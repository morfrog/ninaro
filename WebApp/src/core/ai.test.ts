import { describe, expect, it } from 'vitest';
import { AiLevel, randomMoveFromList, ruleMove, ruleScores } from './ai';
import { newWin } from './connect';
import { Grid } from './grid';
import type { Rng } from './random';
import { board, o, x } from './testBoard';

const fixed = (value: number): Rng => ({ random: () => value });
const scores = (g: Grid, size: number, level: number) => ruleScores({ playgrid: g, win: newWin(size) }, g, level, x, o);

describe('randomMoveFromList', () => {
  it('picks the nth of the highest values', () => {
    const list = [5, 9, 1, 9, 9];
    expect(randomMoveFromList(list, fixed(0))).toBe(1);
    expect(randomMoveFromList(list, fixed(1))).toBe(3);
    expect(randomMoveFromList(list, fixed(2))).toBe(4);
  });

  it('asks for a number below the count of best values', () => {
    let asked = 0;
    randomMoveFromList([3, 7, 7, 2], { random: (n) => ((asked = n), 0) });
    expect(asked).toBe(2);
  });
});

describe('ruleScores', () => {
  it('scores free columns 100 and full ones 0 (level 0, AI Random)', () => {
    const g = board('X..', 'O..');
    expect(scores(g, 3, AiLevel.Random)).toEqual([0, 100, 100]);
  });

  it('returns null, and ruleMove -1, when the board is full', () => {
    const g = board('XO', 'OX');
    expect(scores(g, 2, AiLevel.Genius)).toBe(null);
    expect(ruleMove({ playgrid: g, win: newWin(2) }, g, 4, x, o, fixed(0))).toBe(-1);
  });

  it('adds 1 for a winning column and stops applying rules (level 1+)', () => {
    // X wins in column 3; O could also win in column 4 but rule 2 is skipped.
    const g = board('.....', 'XXX.O', 'OOOXO');
    expect(scores(g, 4, AiLevel.Genius)).toEqual([100, 100, 100, 101, 100]);
  });

  it('adds 50 to block the opponent (level 2)', () => {
    const g = board('.....', 'OOO..', 'XXOX.');
    expect(scores(g, 4, AiLevel.Normal)).toEqual([100, 100, 100, 150, 100]);
  });

  it('takes 90 when the cell above a drop wins for the opponent (level 3)', () => {
    // Row 1 has O O O . with column 3 empty below it at rows 1 and 2.
    const g = board('.....', 'OOO..', 'XXX..', 'OXOO.');
    // X in column 3 lands on row 2 and wins (X X X X) so rule 1 fires instead.
    expect(scores(g, 4, AiLevel.Expert)).toEqual([100, 100, 100, 101, 100]);
    // Without that win, column 3 is penalised for the O win above it.
    const h = board('.....', 'OOO..', 'XOX..', 'OXOO.');
    expect(scores(h, 4, AiLevel.Expert)).toEqual([100, 100, 100, 10, 100]);
  });

  it('takes 20 when the cell above would win for itself (the opponent blocks)', () => {
    const g = board('.....', 'XXX..', 'OXO..', 'XOXO.');
    expect(scores(g, 4, AiLevel.Expert)).toEqual([100, 100, 100, 80, 100]);
  });

  it('adds win potential (level 4)', () => {
    expect(scores(new Grid(7, 8), 4, AiLevel.Genius)).toEqual([101, 101, 101, 102, 101, 101, 101]);
  });
});
