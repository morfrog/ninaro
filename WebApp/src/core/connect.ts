// N-in-a-row rules (Legacy CONNECTN.C): where a token falls, wins, the
// "win potential" the Genius AI uses, and saved games.
// Continuance, MoveAll and UnBlockableWin were never called by the game and
// aren't ported.

import { BinFile } from './binfile';
import { Grid, funcNotEqual, type Compare } from './grid';
import type { Player, PlayerGroup } from './player';

/** Legacy enum {Win, Success, Failure}. */
export const InsertResult = { Win: 0, Success: 1, Failure: 2 } as const;
export type InsertResult = (typeof InsertResult)[keyof typeof InsertResult];

export interface Token {
  col: number;
  row: number;
}

/** WIN: the number to connect, and the cells of the last win found. */
export interface Win {
  tokens: number;
  win: Token[];
}

const TOKEN_VALUE = 10;
const NULL_VALUE = 1;

/** SetWinSize */
export function newWin(size: number): Win {
  return { tokens: size, win: Array.from({ length: size }, () => ({ col: 0, row: 0 })) };
}

/** SetWinSize */
export function setWinSize(win: Win, size: number): void {
  win.tokens = size;
  win.win = Array.from({ length: size }, () => ({ col: 0, row: 0 }));
}

/** InsertTokenTarget: the row a token dropped in col lands on (-1 if full). */
export function insertTokenTarget(grid: Grid, col: number): number {
  return grid.notInCol(col) - 1;
}

/** RemoveTokenTarget: the row of the top token in col. */
export function removeTokenTarget(grid: Grid, col: number): number {
  return grid.rows() - grid.inCol(col);
}

/** InsertToken */
export function insertToken(grid: Grid, col: number, player: Player): boolean {
  const row = insertTokenTarget(grid, col);
  if (row < 0) return false; // the C indexed row -1 of a full column; it held a token, so this failed too
  return grid.insertTokenAtCR(col, row, player);
}

/** RemoveToken: takes the top token out of col. */
export function removeToken(grid: Grid, col: number): Player | null {
  const row = removeTokenTarget(grid, col);
  if (row >= grid.rows()) return null; // empty column: the C read one past the end
  return grid.removeTokenAtCR(col, row);
}

/**
 * CheckAngle: is (col, row) part of `size` in a row along this direction?
 * If so, writes the winning cells into win.win (as the C did, even when
 * called by the AI just to look).
 */
export function checkAngle(
  grid: Grid,
  size: number,
  col: number,
  row: number,
  colDir: number,
  rowDir: number,
  win: Win,
  player: Player | null,
  compare: Compare,
): boolean {
  let left = grid.funcNumInDirection(col, row, colDir, rowDir, player, compare);
  if (left >= size) left = size - 1; // ensures token is a part of the win
  let right = grid.funcNumInDirection(col, row, -colDir, -rowDir, player, compare);
  if (right >= size) right = size - 1;

  if (left + right + 1 >= size) {
    const startCol = col + colDir * left;
    const startRow = row + rowDir * left;
    for (let x = 0; x < size; x++) {
      win.win[x] = { col: startCol + x * -colDir, row: startRow + x * -rowDir };
    }
    return true;
  }
  return false;
}

/** CheckForWin: vertical, horizontal, /, then \ through (col, row). */
export function checkForWin(grid: Grid, size: number, col: number, row: number, win: Win, player: Player | null): boolean {
  return (
    checkAngle(grid, size, col, row, 0, -1, win, player, funcNotEqual) ||
    checkAngle(grid, size, col, row, -1, 0, win, player, funcNotEqual) ||
    checkAngle(grid, size, col, row, 1, -1, win, player, funcNotEqual) ||
    checkAngle(grid, size, col, row, 1, 1, win, player, funcNotEqual)
  );
}

/** AnglePotential: own tokens (10 each) and empty cells (1 each) along a line with room to win. */
export function anglePotential(
  grid: Grid,
  size: number,
  col: number,
  row: number,
  colDir: number,
  rowDir: number,
  player: Player,
  other: Player,
): number {
  const l = grid.numsInDirection(col, row, colDir, rowDir, player, null, other, size - 1);
  const r = grid.numsInDirection(col, row, -colDir, -rowDir, player, null, other, size - 1);
  if (l.number + r.number + 1 >= size) {
    return TOKEN_VALUE * (l.p1i + r.p1i) + NULL_VALUE * (l.p2i + r.p2i);
  }
  return 0;
}

/**
 * WinPotential: a 0..max measure of how much room to win there is around
 * (col, row), over the lines (0,-1), (-1,0), (1,-1) and (-1,-1) as the C
 * wrote them. Integer division, as in C.
 */
export function winPotential(grid: Grid, size: number, col: number, row: number, player: Player, other: Player, max: number): number {
  let ret = 0;
  ret += anglePotential(grid, size, col, row, 0, -1, player, other);
  ret += anglePotential(grid, size, col, row, -1, 0, player, other);
  ret += anglePotential(grid, size, col, row, +1, -1, player, other);
  ret += anglePotential(grid, size, col, row, -1, -1, player, other);
  return Math.trunc((max * ret) / ((size - 1) * TOKEN_VALUE * 8));
}

/**
 * InsertTokenInCol: drops player's token in col. Win (and win.win set) if
 * it connects win.tokens, Success if not, Failure if the column is full.
 */
export function insertTokenInCol(grid: Grid, col: number, player: Player, win: Win): InsertResult {
  const row = insertTokenTarget(grid, col);
  if (insertToken(grid, col, player)) {
    return checkForWin(grid, win.tokens, col, row, win, player) ? InsertResult.Win : InsertResult.Success;
  }
  return InsertResult.Failure;
}

export interface SavedGame {
  p1: number;
  p2: number;
  current: number;
  size: number;
  grid: Grid;
}

/** GameSave: the NINARO.SAV bytes. */
export function gameSave(p1: number, p2: number, current: number, size: number, grid: Grid, pg: PlayerGroup): Uint8Array {
  const bf = new BinFile(4 * 4 + grid.saveSize());
  bf.putInt(p1);
  bf.putInt(p2);
  bf.putInt(current);
  bf.putInt(size);
  grid.save(bf, pg);
  return bf.store;
}

/** GameLoad: reads NINARO.SAV bytes. */
export function gameLoad(data: Uint8Array, pg: PlayerGroup): SavedGame {
  const bf = new BinFile(data);
  const p1 = bf.getInt();
  const p2 = bf.getInt();
  const current = bf.getInt();
  const size = bf.getInt();
  return { p1, p2, current, size, grid: Grid.load(bf, pg) };
}
