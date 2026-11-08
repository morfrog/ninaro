// The computer players (Legacy NINARO.C RuleMove, RandomMoveFromList and the
// AI*Move functions). Each column starts at 100 (0 if full); each rule level
// adds to or takes from the columns, and a random best column is chosen.

import { checkForWin, insertTokenTarget, winPotential, type Win } from './connect';
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
 * 4 add the win potential (0..40).
 */
export function ruleMove(game: AiGame, grid: Grid, level: number, thisPlayer: Player, otherPlayer: Player, rng: Rng): number {
  const col = ruleScores(game, grid, level, thisPlayer, otherPlayer);
  return col === null ? -1 : randomMoveFromList(col, rng);
}

/**
 * The column scores RuleMove chooses from (the list its TESTING build
 * printed), or null if every column is full.
 */
export function ruleScores(game: AiGame, grid: Grid, level: number, thisPlayer: Player, otherPlayer: Player): number[] | null {
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
        break;
    }
  }
  return col;
}
