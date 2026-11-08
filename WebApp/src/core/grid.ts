// The board: a grid of player tokens (Legacy GRID.C). Columns run 0..width-1
// left to right, rows 0..height-1 top to bottom, so tokens fall towards the
// last row. Cells hold the player who put a token there, or null.

import { BinFile } from './binfile';
import type { Player, PlayerGroup } from './player';

/** Legacy direction enum {T_UP, T_DOWN, T_LEFT, T_RIGHT}. */
export const Direction = { Up: 0, Down: 1, Left: 2, Right: 3 } as const;
export type Direction = (typeof Direction)[keyof typeof Direction];

/** Compare functions for funcNumInDirection (FuncEqual, FuncNotEqual). */
export type Compare = (atCR: Player | null, player: Player | null) => boolean;
export const funcEqual: Compare = (p1, p2) => p1 === p2;
export const funcNotEqual: Compare = (p1, p2) => p1 !== p2;

export class Grid {
  readonly width: number;
  readonly height: number;
  /** grid[col][row] */
  readonly grid: (Player | null)[][];
  readonly numInCol: number[];
  readonly numInRow: number[];
  tokensInGrid = 0;

  /** GridNew */
  constructor(width: number, height: number) {
    this.width = width;
    this.height = height;
    this.grid = Array.from({ length: width }, () => new Array<Player | null>(height).fill(null));
    this.numInCol = new Array<number>(width).fill(0);
    this.numInRow = new Array<number>(height).fill(0);
  }

  /** GridEmpty */
  empty(): void {
    this.tokensInGrid = 0;
    for (const column of this.grid) column.fill(null);
    this.numInCol.fill(0);
    this.numInRow.fill(0);
  }

  /** GridCols */
  cols(): number {
    return this.width;
  }

  /** GridRows */
  rows(): number {
    return this.height;
  }

  /** GridInCol: -1 if col is past the right edge. */
  inCol(col: number): number {
    return col >= this.width ? -1 : this.numInCol[col];
  }

  /** GridNotInCol: empty cells in col, -1 if col is past the right edge. */
  notInCol(col: number): number {
    return col >= this.width ? -1 : this.height - this.numInCol[col];
  }

  /** GridInRow */
  inRow(row: number): number {
    return row >= this.height ? -1 : this.numInRow[row];
  }

  /** GridNotInRow */
  notInRow(row: number): number {
    return row >= this.height ? -1 : this.width - this.numInRow[row];
  }

  /** InsertTokenAtCR: true if the cell was empty and player not null. */
  insertTokenAtCR(col: number, row: number, player: Player | null): boolean {
    if (this.grid[col][row] === null && player !== null) {
      this.grid[col][row] = player;
      this.numInCol[col] += 1;
      this.numInRow[row] += 1;
      this.tokensInGrid += 1;
      return true;
    }
    return false;
  }

  /** ResetTokenAtCR: sets the cell; false if it was empty. */
  resetTokenAtCR(col: number, row: number, player: Player | null): boolean {
    if (this.grid[col][row] === null) {
      this.insertTokenAtCR(col, row, player);
      return false;
    }
    this.grid[col][row] = player;
    return true;
  }

  /** RemoveTokenAtCR: the player who was there, or null. */
  removeTokenAtCR(col: number, row: number): Player | null {
    const ret = this.grid[col][row];
    if (ret !== null) {
      this.grid[col][row] = null;
      this.numInCol[col] -= 1;
      this.numInRow[row] -= 1;
      this.tokensInGrid -= 1;
    }
    return ret;
  }

  /** MoveTokenAtCR: moves a token one cell if that cell is on the grid and empty. */
  moveTokenAtCR(col: number, row: number, direction: Direction): boolean {
    let targetCol = col;
    let targetRow = row;
    switch (direction) {
      case Direction.Up: --targetRow; break;
      case Direction.Down: ++targetRow; break;
      case Direction.Left: --targetCol; break;
      case Direction.Right: ++targetCol; break;
    }
    if (targetRow < 0 || targetRow > this.height - 1 || targetCol < 0 || targetCol > this.width - 1) return false;
    if (this.grid[targetCol][targetRow] !== null) return false;

    this.grid[targetCol][targetRow] = this.grid[col][row];
    this.grid[col][row] = null;
    switch (direction) {
      case Direction.Up:
      case Direction.Down:
        this.numInRow[row]--;
        this.numInRow[targetRow]++;
        break;
      case Direction.Left:
      case Direction.Right:
        this.numInCol[col]--;
        this.numInCol[targetCol]++;
        break;
    }
    return true;
  }

  /** StatusOfCR */
  statusOfCR(col: number, row: number): Player | null {
    return this.grid[col][row];
  }

  /** TokensInGrid */
  tokens(): number {
    return this.tokensInGrid;
  }

  /** MaxTokensInGrid */
  maxTokens(): number {
    return this.width * this.height;
  }

  /**
   * FuncNumInDirection: steps from (col, row) by (colMod, rowMod), counting
   * cells until one where compare(cell, player) is true or the edge.
   * The starting cell isn't counted.
   */
  funcNumInDirection(col: number, row: number, colMod: number, rowMod: number, player: Player | null, compare: Compare): number {
    let curCol = col;
    let curRow = row;
    let number = 0;
    for (;;) {
      curCol += colMod;
      curRow += rowMod;
      if (curCol > this.width - 1 || curCol < 0 || curRow > this.height - 1 || curRow < 0) return number;
      if (compare(this.grid[curCol][curRow], player)) return number;
      ++number;
    }
  }

  /**
   * NumsInDirection: steps from (col, row) by (colMod, rowMod) until the
   * stop player's token, the edge, or istop cells. Counts cells holding p1
   * and p2 (either may be null, meaning empty cells). As in the C, istop -1
   * ("infinity") actually stops after one cell.
   */
  numsInDirection(
    col: number,
    row: number,
    colMod: number,
    rowMod: number,
    p1: Player | null,
    p2: Player | null,
    stop: Player | null,
    istop: number,
  ): { number: number; p1i: number; p2i: number } {
    let curCol = col;
    let curRow = row;
    let number = 0;
    let p1i = 0;
    let p2i = 0;
    for (;;) {
      curCol += colMod;
      curRow += rowMod;
      if (curCol > this.width - 1 || curCol < 0 || curRow > this.height - 1 || curRow < 0) break;
      const atCR = this.grid[curCol][curRow];
      if (atCR === stop) break;
      if (atCR === p1) p1i += 1;
      if (atCR === p2) p2i += 1;
      ++number;
      if (number >= istop) break;
    }
    return { number, p1i, p2i };
  }

  /** GridSaveSize */
  saveSize(): number {
    return 4 * (2 + this.maxTokens());
  }

  /** GridSave: width, height, then each cell column by column as pid + 1 (0 = empty). */
  save(bf: BinFile, pg: PlayerGroup): void {
    bf.putInt(this.width);
    bf.putInt(this.height);
    for (let x = 0; x < this.width; x++)
      for (let y = 0; y < this.height; y++) {
        const player = this.grid[x][y];
        bf.putInt(player === null ? 0 : pg.find(player) + 1);
      }
  }

  /** GridLoad */
  static load(bf: BinFile, pg: PlayerGroup): Grid {
    const width = bf.getInt();
    const height = bf.getInt();
    const ret = new Grid(width, height);
    for (let x = 0; x < width; x++)
      for (let y = 0; y < height; y++) {
        const player = bf.getInt();
        if (player !== 0) ret.insertTokenAtCR(x, y, pg.player(player - 1));
      }
    return ret;
  }
}
