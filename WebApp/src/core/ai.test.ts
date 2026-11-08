import { describe, expect, it } from 'vitest';
import { AiLevel, DOUBLE_THREAT_PENALTY, RULES_1998, allowsDoubleThreat, randomMoveFromList, ruleMove, ruleScores, winningCols } from './ai';
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

  it('plays the 1998 rules when asked (no double-threat rule)', () => {
    const g = board('.......', '.......', '.......', '.......', '.......', '.......', '...X...', '...O.O.');
    const old = ruleScores({ playgrid: g, win: newWin(4) }, g, AiLevel.Genius, x, o, RULES_1998)!;
    // The original stacks on its own token in column 3, the highest score.
    expect(old.indexOf(Math.max(...old))).toBe(3);
  });
});

describe('the double-threat rule (Genius, added in 2026)', () => {
  // O has 3 and 5 on the bottom row. If X doesn't play 4 (or 2 or 6), O
  // plays 4 and has 2 and 6 to win in: X can only block one.
  const setup = () => board('.......', '.......', '.......', '.......', '.......', '.......', '...X...', '...O.O.');

  it('sees which drops let the opponent set up two wins', () => {
    const g = setup();
    expect(allowsDoubleThreat(g, 4, 3, x, o)).toBe(true);
    expect(allowsDoubleThreat(g, 4, 4, x, o)).toBe(false);
    expect(allowsDoubleThreat(g, 4, 2, x, o)).toBe(false);
    expect(allowsDoubleThreat(g, 4, 6, x, o)).toBe(false);
    // The grid is left as it was.
    expect(g).toEqual(setup());
  });

  it('takes the penalty off those columns, so Genius blocks', () => {
    const g = setup();
    const fixedScores = scores(g, 4, AiLevel.Genius)!;
    const old = ruleScores({ playgrid: g, win: newWin(4) }, g, AiLevel.Genius, x, o, RULES_1998)!;
    expect(fixedScores[3]).toBe(old[3] - DOUBLE_THREAT_PENALTY);
    expect(fixedScores[4]).toBe(old[4]);
    expect([2, 4, 6]).toContain(fixedScores.indexOf(Math.max(...fixedScores)));
  });

  it('is only Genius: Expert scores are unchanged', () => {
    const g = setup();
    expect(scores(g, 4, AiLevel.Expert)).toEqual(ruleScores({ playgrid: g, win: newWin(4) }, g, AiLevel.Expert, x, o, RULES_1998));
  });

  it("doesn't apply when the opponent can already win (blocking comes first)", () => {
    const g = board('.......', '.......', '.......', '.......', '.......', '.......', '...X...', '.OOO.O.');
    expect(winningCols(g, 4, o)).toBe(2);
    expect(scores(g, 4, AiLevel.Genius)).toEqual(ruleScores({ playgrid: g, win: newWin(4) }, g, AiLevel.Genius, x, o, RULES_1998));
  });
});
