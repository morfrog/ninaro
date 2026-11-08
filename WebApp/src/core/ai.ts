// The computer players (Legacy NINARO.C RuleMove, RandomMoveFromList and the
// AI*Move functions). Each column starts at 100 (0 if full); each rule level
// adds to or takes from the columns, and a random best column is chosen.

import { InsertResult, checkForWin, insertTokenInCol, insertTokenTarget, newWin, removeToken, winPotential, type Win } from './connect';
import type { Grid } from './grid';
import type { Player } from './player';
import type { Rng } from './random';

/** The AIs in the player group, slots 2..6, by the number of rules they use. */
export const AiLevel = { Random: 0, Novice: 1, Normal: 2, Expert: 3, Genius: 4 } as const;
export type AiLevel = (typeof AiLevel)[keyof typeof AiLevel];

export const AI_NAMES = ['AI Random', 'AI Novice', 'AI Normal', 'AI Expert', 'AI Genius'] as const;

/**
 * NINARO.C globals the AI reads: RuleMove checks wins on `playgrid` (not its
 * grid argument) and writes into the game's `win`, as the original did.
 */
export interface AiGame {
  playgrid: Grid;
  win: Win;
}

/**
 * Which rules the AIs play by. The 1998 rules are kept for the golden games;
 * the game and the tournament play with `doubleThreat` on (a fix added in
 * 2026, not in the original).
 */
export interface AiRules {
  /** Genius: avoid moves that let the opponent set up two wins at once. */
  doubleThreat: boolean;
}
export const RULES_1998: AiRules = { doubleThreat: false };
export const RULES_FIXED: AiRules = { doubleThreat: true };

/** How much the double-threat rule takes off a column: more than win potential can add. */
export const DOUBLE_THREAT_PENALTY = 50;

/** RandomMoveFromList: index of a random one of the highest values. */
export function randomMoveFromList(list: readonly number[], rng: Rng): number {
  let max = list[0];
  let howMany = 1;
  let first = 0;
  for (let x = 1; x < list.length; x++) {
    if (list[x] > max) {
      max = list[x];
      howMany = 1;
      first = x;
    } else if (list[x] === max) ++howMany;
  }
  const move = rng.random(howMany);
  let found = 0;
  for (let x = first; x < list.length; x++) {
    if (list[x] === max) {
      ++found;
      if (found === move + 1) return x;
    }
  }
  return -1;
}

/**
 * RuleMove: the column to play, or -1 if the board is full. Levels:
 * 0 random; 1 take a win; 2 block the opponent's win (+50); 3 don't drop
 * where the opponent can then win above (-90) or be blocked (-20);
 * 4 add the win potential (0..40) and, with RULES_FIXED (not in the
 * original), avoid letting the opponent set up two wins at once.
 */
export function ruleMove(game: AiGame, grid: Grid, level: number, thisPlayer: Player, otherPlayer: Player, rng: Rng, rules: AiRules = RULES_FIXED): number {
  const col = ruleScores(game, grid, level, thisPlayer, otherPlayer, rules);
  return col === null ? -1 : randomMoveFromList(col, rng);
}

/**
 * The column scores RuleMove chooses from (the list its TESTING build
 * printed), or null if every column is full.
 */
export function ruleScores(
  game: AiGame,
  grid: Grid,
  level: number,
  thisPlayer: Player,
  otherPlayer: Player,
  rules: AiRules = RULES_FIXED,
): number[] | null {
  const { playgrid, win } = game;
  const cols = grid.cols();
  const rows = grid.rows();
  const col: number[] = [];
  let canUse = false;
  let triggered = false;

  for (let x = 0; x < cols; x++) {
    col[x] = grid.inCol(x) < rows ? 100 : 0;
    if (col[x]) canUse = true;
  }
  if (!canUse) return null;

  for (let atLevel = 0; atLevel <= level; atLevel++) {
    switch (atLevel) {
      case 0: // invoke zero rules is just random
        break;
      case 1: // take win rule
        for (let x = 0; x < cols; x++) {
          if (col[x] && checkForWin(playgrid, win.tokens, x, insertTokenTarget(playgrid, x), win, thisPlayer)) {
            col[x] += 1;
            triggered = true;
          }
        }
        if (triggered) atLevel = level + 1; // get out of loop
        break;
      case 2: // block opponent rule
        for (let x = 0; x < cols; x++) {
          if (col[x] && checkForWin(playgrid, win.tokens, x, insertTokenTarget(playgrid, x), win, otherPlayer)) {
            col[x] += 50;
          }
        }
        break;
      case 3: // other above rule, requires at least 2 empties in col
        for (let x = 0; x < cols; x++) {
          if (col[x] && rows - playgrid.inCol(x) > 1) {
            const above = insertTokenTarget(playgrid, x) - 1;
            if (checkForWin(playgrid, win.tokens, x, above, win, otherPlayer)) {
              col[x] -= 90; // drop here and opponent drops here and wins
            } else if (checkForWin(playgrid, win.tokens, x, above, win, thisPlayer)) {
              col[x] -= 20; // drop here and opponent drops here and blocks me
            }
          }
        }
        break;
      case 4:
        for (let x = 0; x < cols; x++) {
          if (col[x]) {
            col[x] += winPotential(playgrid, win.tokens, x, insertTokenTarget(playgrid, x), thisPlayer, otherPlayer, 40);
          }
        }
        if (rules.doubleThreat && winningCols(playgrid, win.tokens, otherPlayer) === 0) {
          for (let x = 0; x < cols; x++) {
            if (col[x] && allowsDoubleThreat(playgrid, win.tokens, x, thisPlayer, otherPlayer)) col[x] -= DOUBLE_THREAT_PENALTY;
          }
        }
        break;
    }
  }
  return col;
}

/** How many columns `player` could win in with their next drop. */
export function winningCols(grid: Grid, size: number, player: Player): number {
  const win = newWin(size);
  let n = 0;
  for (let x = 0; x < grid.cols(); x++) {
    if (grid.inCol(x) < grid.rows() && checkForWin(grid, size, x, insertTokenTarget(grid, x), win, player)) n++;
  }
  return n;
}

/**
 * The double-threat rule (2026, not in the original): if `me` drops in col,
 * can `other` answer with a drop that leaves them two columns to win in,
 * when `me` has no win of their own to take first? Only one can be blocked.
 */
export function allowsDoubleThreat(grid: Grid, size: number, col: number, me: Player, other: Player): boolean {
  const win = newWin(size);
  const r = insertTokenInCol(grid, col, me, win);
  if (r === InsertResult.Failure) return false;
  if (r === InsertResult.Win) {
    removeToken(grid, col);
    return false;
  }
  let found = false;
  for (let y = 0; y < grid.cols() && !found; y++) {
    if (grid.inCol(y) >= grid.rows()) continue;
    if (insertTokenInCol(grid, y, other, win) === InsertResult.Success) {
      found = winningCols(grid, size, other) >= 2 && winningCols(grid, size, me) === 0;
    }
    removeToken(grid, y);
  }
  removeToken(grid, col);
  return found;
}
