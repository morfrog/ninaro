// Test helper: builds a grid from a picture, top row first.
// '.' is empty, 'X' is player x, 'O' is player o.

import { Grid } from './grid';
import { PlayerType, playerNew, type Player } from './player';

export const x: Player = playerNew('X', 2, PlayerType.Computer);
export const o: Player = playerNew('O', 1, PlayerType.Computer);

export function board(...rows: string[]): Grid {
  const grid = new Grid(rows[0].length, rows.length);
  rows.forEach((line, row) => {
    [...line].forEach((ch, col) => {
      if (ch === 'X') grid.insertTokenAtCR(col, row, x);
      else if (ch === 'O') grid.insertTokenAtCR(col, row, o);
    });
  });
  return grid;
}

export function picture(grid: Grid): string[] {
  const out: string[] = [];
  for (let row = 0; row < grid.rows(); row++) {
    let line = '';
    for (let col = 0; col < grid.cols(); col++) {
      const p = grid.statusOfCR(col, row);
      line += p === null ? '.' : p === x ? 'X' : 'O';
    }
    out.push(line);
  }
  return out;
}
