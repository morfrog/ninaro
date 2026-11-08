import { describe, expect, it } from 'vitest';
import golden from '../core/__golden__/games.json';
import { BOARDS, playGame, runTournament, totals, validBoard } from './runner';

interface GoldenGame {
  cols: number;
  rows: number;
  connect: number;
  levels: [number, number];
  seed: number;
  first: 0 | 1;
  moves: string;
  result: string;
}

describe('playGame', () => {
  it('plays every golden game exactly as the 1998 code did', () => {
    for (const g of golden.games as unknown as GoldenGame[]) {
      const r = playGame({ cols: g.cols, rows: g.rows, connect: g.connect }, g.levels, g.seed, g.first);
      expect({ moves: r.moves.join(' '), result: r.result }).toEqual({ moves: g.moves, result: g.result });
    }
  });
});

describe('runTournament', () => {
  const r = runTournament(BOARDS['7x8x4'], 40, 1998);

  it('counts every game once', () => {
    for (const row of r.cells) for (const t of row) expect(t.wins + t.draws + t.losses).toBe(40);
  });

  it('can be repeated exactly from its seed', () => {
    expect(runTournament(BOARDS['7x8x4'], 40, 1998)).toEqual(r);
  });

  it('has AI Genius beat AI Random almost every time', () => {
    const asOne = r.cells[4][0];
    const asTwo = r.cells[0][4];
    expect((asOne.wins + asTwo.losses) / 80).toBeGreaterThan(0.9);
  });

  it('ranks the AIs from Random up to Genius, by wins against the others', () => {
    const t = totals(r);
    expect(t[4].wins).toBeGreaterThan(t[0].wins);
    expect(t[3].wins).toBeGreaterThan(t[1].wins);
    // Each AI plays the other four as player one and as player two.
    for (const x of t) expect(x.wins + x.draws + x.losses).toBe(4 * 2 * 40);
  });

  it('reports progress after each pairing', () => {
    const seen: number[] = [];
    runTournament({ cols: 3, rows: 3, connect: 3 }, 2, 1, (done, total) => {
      seen.push(done);
      expect(total).toBe(25);
    });
    expect(seen).toEqual(Array.from({ length: 25 }, (_, i) => i + 1));
  });
});

describe('validBoard', () => {
  it('accepts what the Custom Board dialog allows', () => {
    expect(validBoard({ cols: 3, rows: 3, connect: 3 })).toBe(true);
    expect(validBoard({ cols: 50, rows: 4, connect: 50 })).toBe(true);
    expect(validBoard({ cols: 2, rows: 8, connect: 3 })).toBe(false);
    expect(validBoard({ cols: 7, rows: 8, connect: 9 })).toBe(false);
  });
});
