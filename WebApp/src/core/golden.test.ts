// Replays the golden games made by the 1998 C code (Tools/golden) and checks
// the TypeScript port makes exactly the same moves, results and win cells.

import { readFileSync } from 'node:fs';
import { resolve } from 'node:path';
import { describe, expect, it } from 'vitest';
import { ruleMove } from './ai';
import { BinFile } from './binfile';
import { InsertResult, insertTokenInCol, newWin } from './connect';
import { Grid } from './grid';
import { PlayerGroup, PlayerType, playerNew } from './player';
import { WatcomRandom } from './random';
import golden from './__golden__/games.json';

interface GoldenGame {
  cols: number;
  rows: number;
  connect: number;
  levels: [number, number];
  seed: number;
  first: number;
  moves: string;
  result: string;
  win: [number, number][];
}

/** The harness's game loop (NINARO.C main and DropToken for two AIs). */
function play(g: GoldenGame) {
  const p = [playerNew('One', 1, PlayerType.Computer), playerNew('Two', 2, PlayerType.Computer)];
  const rng = new WatcomRandom(g.seed);
  const playgrid = new Grid(g.cols, g.rows);
  const win = newWin(g.connect);
  const game = { playgrid, win };
  const moves: number[] = [];
  let current = g.first;
  let result = 'draw';
  let last: InsertResult = InsertResult.Success;
  for (;;) {
    const col = ruleMove(game, playgrid, g.levels[current], p[current], p[1 - current], rng);
    if (col < 0) {
      result = 'nomove';
      break;
    }
    last = insertTokenInCol(playgrid, col, p[current], win);
    moves.push(col);
    if (last === InsertResult.Win) {
      result = current === 0 ? 'one' : 'two';
      break;
    }
    if (last === InsertResult.Failure) {
      result = 'failure';
      break;
    }
    if (playgrid.tokens() === playgrid.maxTokens()) break;
    current = 1 - current;
  }
  const cells = last === InsertResult.Win ? win.win.map((t) => [t.col, t.row]) : [];
  return { moves: moves.join(' '), result, win: cells };
}

const games = golden.games as GoldenGame[];

describe('golden games from the 1998 code', () => {
  it('has games for every board, level pair and seed', () => {
    expect(games).toHaveLength(8 * 5 * 5 * 8);
  });

  const boards = [...new Set(games.map((g) => `${g.cols}x${g.rows}x${g.connect}`))];
  for (const board of boards) {
    it(`replays every ${board} game`, () => {
      for (const g of games.filter((x) => `${x.cols}x${x.rows}x${x.connect}` === board)) {
        const label = `levels ${g.levels.join(' v ')}, seed ${g.seed}`;
        expect(play(g), label).toEqual({ moves: g.moves, result: g.result, win: g.win });
      }
    });
  }
});

describe('title picture', () => {
  it('loads NINARO.TTL as GridLoadFull did', () => {
    const bytes = new Uint8Array(readFileSync(resolve(__dirname, '../../../Legacy/NINARO.TTL')));
    const pg = new PlayerGroup(Array.from({ length: 8 }, (_, i) => playerNew(String(i), 0, PlayerType.Computer)));
    const grid = Grid.load(new BinFile(bytes), pg);
    const cells = grid.grid.map((column) => column.map((p) => (p === null ? 0 : pg.find(p) + 1)).join(''));
    expect({ cols: grid.cols(), rows: grid.rows(), tokens: grid.tokens(), cells }).toEqual(golden.title);
  });
});
