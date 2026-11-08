// Drawing the game screen: the window layout of NINARO.C ViewSysInit and
// its Display* functions.

import type { Grid } from '../core/grid';
import { PlayerType, type Player } from '../core/player';
import { BORDER, FILL, HAlign, VAlign, type Graph } from '../render/graph';
import { Color } from '../render/palette';
import { clearViewport, drawDisc, drawGrid, type Viewport } from '../render/screen';
import { ViewSys } from '../render/viewsys';
import { COL_ADD, type GameState } from './state';

/** Window numbers, in ViewSysInit's order (they are drawn in this order). */
export const GameWindow = {
  entryGrid: 0,
  playGrid: 1,
  gameName: 2,
  msgBox: 3,
  statusBox: 4,
  p1Type: 5,
  p1Name: 6,
  p1Score: 7,
  p2Type: 8,
  p2Name: 9,
  p2Score: 10,
} as const;

/** DisplayGrid (GRID.C): the grid lines in the current colour, then the tokens. */
export function displayGrid(g: Graph, grid: Grid, size: number): void {
  drawGrid(g, grid.width, grid.height);
  for (let x = 0; x < grid.width; x++) {
    for (let y = 0; y < grid.height; y++) {
      const p = grid.grid[x][y];
      if (p !== null) drawDisc(g, x, grid.width, y, grid.height, p.color, size);
    }
  }
}

/** DisplayHuman: a stick figure in the current viewport. */
export function displayHuman(g: Graph, primary: number, secondary: number): void {
  g.setColor(primary);
  g.ellipseW(FILL, 0.2, 0.1, 0.8, 0.25); // head
  g.moveToW(0.49, 0.25);
  g.lineToW(0.49, 0.7); // body
  g.moveToW(0.51, 0.25);
  g.lineToW(0.51, 0.7); // body
  g.setColor(secondary);
  g.moveToW(0.49, 0.7);
  g.lineToW(0.8, 0.9); // left leg
  g.moveToW(0.51, 0.7);
  g.lineToW(0.2, 0.9); // right leg
  g.moveToW(0.1, 0.4);
  g.lineToW(0.9, 0.4); // arms
}

/** DisplayComputer: a monitor in the current viewport. */
export function displayComputer(g: Graph, primary: number, secondary: number): void {
  g.setColor(secondary);
  g.rectangleW(BORDER, 0.1, 0.1, 0.9, 0.65);
  g.rectangleW(BORDER, 0.45, 0.65, 0.55, 0.75);
  g.rectangleW(BORDER, 0.1, 0.75, 0.9, 0.9);
  g.setColor(primary);
  g.rectangleW(FILL, 0.2, 0.2, 0.8, 0.55);
}

/** DisplayPlayerType */
export function displayPlayerType(g: Graph, player: Player): void {
  const pc = player.color;
  if (player.type === PlayerType.Human) displayHuman(g, pc, pc + COL_ADD);
  else displayComputer(g, pc, pc + COL_ADD);
}

/** DisplayPlayerName */
export function displayPlayerName(g: Graph, player: Player): void {
  g.setColor(Color.GREEN);
  g.setTextAlign(HAlign.LEFT, VAlign.BOTTOM);
  g.setCharSpacingW(-0.013);
  g.setCharSizeW(0.04, 0.03);
  g.grTextW(0.0, 1.0, player.name);
}

/** DisplayPlayerScore: uses the text settings DisplayPlayerName left. */
export function displayPlayerScore(g: Graph, score: number, gamesPlayed: number): void {
  g.setColor(Color.GREEN);
  g.grTextW(0.0, 1.0, `${String(score).padStart(3)}/${String(gamesPlayed).padStart(3)}`);
}

/**
 * The credit's three lines. The release had the author's full name over the
 * first two; the web port credits the author by first name only.
 */
const CREDIT = ['By Drew', '', '   1998  '];

/** DisplayGameName: "Ninaro" in red with a green shadow, and the credit. */
export function displayGameName(g: Graph): void {
  g.setTextAlign(HAlign.LEFT, VAlign.BASE);
  g.setCharSpacingW(-0.01);
  g.setCharSizeW(0.1, 0.04);
  g.setColor(Color.RED);
  g.grTextW(0.0, 0.7, 'Ninaro');
  g.setColor(Color.GREEN);
  g.grTextW(0.01, 0.7, 'Ninaro');
  g.setCharSpacingW(-0.001);
  g.setCharSizeW(0.03, 0.013);
  g.setColor(Color.RED);
  g.grTextW(0.65, 0.25, CREDIT[0]);
  g.grTextW(0.65, 0.55, CREDIT[1]);
  g.grTextW(0.65, 0.85, CREDIT[2]);
  g.setColor(Color.GREEN);
  g.grTextW(0.655, 0.25, CREDIT[0]);
  g.grTextW(0.655, 0.55, CREDIT[1]);
  g.grTextW(0.655, 0.85, CREDIT[2]);
}

/** ViewSysInit: the game screen's windows, drawing from `state`. */
export function gameViewSys(g: Graph, canvas: HTMLCanvasElement, state: GameState, main: Viewport): ViewSys {
  const vs = new ViewSys(g, canvas, main, 20, (vp) => clearViewport(g, vp));
  const p1 = () => state.player(state.playerOne);
  const p2 = () => state.player(state.playerTwo);
  vs.windowNew(0.0, 0.0, 0.6, 0.08, true, () => {
    g.setColor(Color.YELLOW);
    displayGrid(g, state.entrygrid, 0.6);
  }, null, 'Enter Your Token Here');
  vs.windowNew(0.0, 0.09, 0.6, 1.0, true, () => {
    g.setColor(Color.GREEN);
    displayGrid(g, state.playgrid, 0.6);
    if (state.winOccurred) {
      for (const t of state.win.win) {
        const p = state.playgrid.statusOfCR(t.col, t.row);
        if (p) drawDisc(g, t.col, state.playgrid.width, t.row, state.playgrid.height, p.color, 0.9);
      }
    }
  }, null, 'Enter Your Token Here');
  vs.windowNew(0.61, 0.0, 1.0, 0.1, true, () => displayGameName(g), null, 'Ninaro');
  vs.windowNew(0.61, 0.52, 1.0, 1.0, true, () => {
    g.setVieworg(0, 0);
    state.msgbox.display(g, g.viewport);
  }, null, 'Messages appear here');
  vs.windowNew(0.61, 0.1, 1.0, 0.37, true, () => {
    g.setVieworg(0, 0);
    state.statusbox.display(g, g.viewport);
  }, null, 'The Game Status');
  vs.windowNew(0.61, 0.4, 0.65, 0.448, true, () => displayPlayerType(g, p1()), null, "Player One's Type");
  vs.windowNew(0.66, 0.4, 0.85, 0.448, true, () => displayPlayerName(g, p1()), null, "Player One's Name");
  vs.windowNew(0.86, 0.4, 1.0, 0.448, true, () => displayPlayerScore(g, state.playerOneScore, state.gamesPlayed), null, "Player One's Score");
  vs.windowNew(0.61, 0.452, 0.65, 0.5, true, () => displayPlayerType(g, p2()), null, "Player Two's Type");
  vs.windowNew(0.66, 0.452, 0.85, 0.5, true, () => displayPlayerName(g, p2()), null, "Player Two's Name");
  vs.windowNew(0.86, 0.452, 1.0, 0.5, true, () => displayPlayerScore(g, state.playerTwoScore, state.gamesPlayed), null, "Player Two's Score");
  return vs;
}
