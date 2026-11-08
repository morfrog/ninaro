// AI tournaments: the five computer players playing each other, with the
// game loop of NINARO.C main() for two AIs at the fastest speed (the same
// loop as the golden-game harness, Tools/golden/harness.c).

import { AI_NAMES, ruleMove } from '../core/ai';
import { InsertResult, insertTokenInCol, newWin } from '../core/connect';
import { Grid } from '../core/grid';
import { PlayerType, playerNew } from '../core/player';
import { WatcomRandom } from '../core/random';

export interface Board {
  cols: number;
  rows: number;
  connect: number;
}

export type Result = 'one' | 'two' | 'draw';

/** One game between AI levels (0 Random .. 4 Genius); `first` is 0 or 1. */
export function playGame(board: Board, levels: readonly [number, number], seed: number, first: 0 | 1): { result: Result; moves: number[] } {
  const p = [playerNew('One', 1, PlayerType.Computer), playerNew('Two', 2, PlayerType.Computer)];
  const rng = new WatcomRandom(seed);
  const playgrid = new Grid(board.cols, board.rows);
  const win = newWin(board.connect);
  const game = { playgrid, win };
  const moves: number[] = [];
  let current: number = first;
  for (;;) {
    const col = ruleMove(game, playgrid, levels[current], p[current], p[1 - current], rng);
    if (col < 0) return { result: 'draw', moves };
    const r = insertTokenInCol(playgrid, col, p[current], win);
    moves.push(col);
    if (r === InsertResult.Win) return { result: current === 0 ? 'one' : 'two', moves };
    if (playgrid.tokens() === playgrid.maxTokens()) return { result: 'draw', moves };
    current = 1 - current;
  }
}

/** Wins, draws and losses for the row AI against the column AI. */
export interface Tally {
  wins: number;
  draws: number;
  losses: number;
}

export interface TournamentResult {
  board: Board;
  games: number;
  seed: number;
  /** cells[a][b]: AI a playing AI b (each going first in half the games). */
  cells: Tally[][];
}

export const LEVELS = AI_NAMES.length;

/**
 * Every AI against every other (and itself), `games` games a pairing, the
 * first move alternating. Game i of pairing (a, b) uses seed + i, so a
 * tournament can be repeated exactly. `progress` is told after each pairing.
 */
export function runTournament(board: Board, games: number, seed: number, progress?: (done: number, total: number) => void): TournamentResult {
  const cells: Tally[][] = Array.from({ length: LEVELS }, () => Array.from({ length: LEVELS }, () => ({ wins: 0, draws: 0, losses: 0 })));
  const total = LEVELS * LEVELS;
  let done = 0;
  for (let a = 0; a < LEVELS; a++) {
    for (let b = 0; b < LEVELS; b++) {
      const t = cells[a][b];
      for (let i = 0; i < games; i++) {
        const { result } = playGame(board, [a, b], (seed + i) >>> 0, (i % 2) as 0 | 1);
        if (result === 'one') t.wins++;
        else if (result === 'two') t.losses++;
        else t.draws++;
      }
      progress?.(++done, total);
    }
  }
  return { board, games, seed, cells };
}

/** An AI's games against all the others (not itself), as player one or two. */
export function totals(r: TournamentResult): Tally[] {
  return Array.from({ length: LEVELS }, (_, a) => {
    const t: Tally = { wins: 0, draws: 0, losses: 0 };
    for (let b = 0; b < LEVELS; b++) {
      if (a === b) continue;
      t.wins += r.cells[a][b].wins + r.cells[b][a].losses;
      t.losses += r.cells[a][b].losses + r.cells[b][a].wins;
      t.draws += r.cells[a][b].draws + r.cells[b][a].draws;
    }
    return t;
  });
}

/** The menu's boards, and Custom's limits. */
export const BOARDS: Record<string, Board> = {
  '7x8x4': { cols: 7, rows: 8, connect: 4 },
  '12x13x6': { cols: 12, rows: 13, connect: 6 },
};

/** Whether a custom board is one the game's dialog could set. */
export function validBoard(b: Board): boolean {
  const ok = (n: number, lo: number, hi: number) => Number.isInteger(n) && n >= lo && n <= hi;
  return ok(b.cols, 3, 50) && ok(b.rows, 3, 50) && ok(b.connect, 3, Math.max(b.cols, b.rows));
}
