// The title screen (Legacy NINARO.C TitleScreen): the tokens of the picture
// in NINARO.TTL drop into a 36 x 9 board in random columns, spelling NINARO
// in red over pulsing dark tokens, until a key is pressed.
//
// The original ran its loop as fast as the PC allowed, dropping one token
// and stepping the background colour each time round. Here each display
// frame is one time round (about 5 seconds to fill, a pulse about a
// second), and a click or tap also continues. The slide-off animation and
// mouse calibration that followed on a first run are not needed.

import { BinFile } from '../core/binfile';
import { insertTokenInCol, insertTokenTarget, removeToken } from '../core/connect';
import { Grid } from '../core/grid';
import { PlayerType, playerNew, type Player } from '../core/player';
import { startRandomSeed } from '../core/random';
import { LMB } from '../input/input';
import { HAlign, VAlign } from '../render/graph';
import { MONO_DARKEST, MONO_LIGHTEST, MONO_SECOND } from '../render/displayStyle';
import { Color } from '../render/palette';
import { drawDisc, drawGrid, resetPalette, setViewport, type Viewport } from '../render/screen';
import type { Game } from './game';

/** The YELLOW slot's pulse: one channel up a step each time round, then back down. */
const ADDS = [0x010000, 0x000100, 0x000001];
const BOTTOM = 0x020306;
const TOP = BOTTOM + 0x080808;

interface Dropped {
  col: number;
  row: number;
  color: number;
}

/** NINARO.TTL as a grid of the game's player group (cells hold pid + 1). */
export function loadTitle(game: Game, ttl: Uint8Array): Grid {
  return Grid.load(new BinFile(ttl), game.state.players);
}

export async function titleScreen(game: Game, ttl: Uint8Array, nextFrame = () => new Promise<void>((r) => requestAnimationFrame(() => r()))): Promise<void> {
  const { g, state, input } = game;
  const filegrid = loadTitle(game, ttl);
  const plname: Player = playerNew('', Color.RED, PlayerType.Human);
  const plback: Player = playerNew('', Color.YELLOW, PlayerType.Human);
  const vp: Viewport = { x1: 0, y1: 0, x2: 639, y2: 349 - 30 };
  const tokens = filegrid.maxTokens();
  g.palette.remap(Color.YELLOW, 0x020202);
  g.palette.remap(Color.GREEN, 0x040404);
  // Monochrome: the background tokens are the darkest non-black, the title's tokens the lightest green.
  g.setMonoLook(Color.YELLOW, MONO_DARKEST);
  g.setMonoLook(Color.RED, MONO_LIGHTEST);
  game.rng.seed(startRandomSeed()); // StartRandom
  game.resetGameSize(filegrid.cols(), filegrid.rows(), 4);

  const dropped: Dropped[] = [];
  const draw = () => {
    g.setViewport(0, 0, 639, 349);
    g.clearScreen(false);
    g.setColor(Color.WHITE);
    g.setCharSize(20, 10);
    g.setCharSpacing(0);
    g.setTextAlign(HAlign.CENTER, VAlign.TOP);
    g.grText(vp.x1 + Math.trunc((vp.x2 - vp.x1) / 2), vp.y2 + 5, 'Hit Any Key To Continue...');
    setViewport(g, vp);
    g.setColor(Color.GREEN);
    drawGrid(g, state.playgrid.cols(), state.playgrid.rows());
    for (const d of dropped) drawDisc(g, d.col, state.playgrid.cols(), d.row, state.playgrid.rows(), d.color, 0.95);
  };

  let sety = BOTTOM;
  let up = true;
  let add = 0;
  let x = 0;
  input.clear();
  for (;;) {
    if (x < tokens) {
      const scol = game.aiMove(0); // AIRandomMove
      const pl = removeToken(filegrid, scol);
      const row = insertTokenTarget(state.playgrid, scol);
      const into = pl && game.state.players.find(pl) === 0 ? plback : plname;
      insertTokenInCol(state.playgrid, scol, into, state.win);
      dropped.push({ col: scol, row, color: into.color });
      x++;
    }
    if (up) {
      sety += ADDS[add];
      if (sety > TOP) up = false;
    } else {
      sety -= ADDS[add];
      if (sety < BOTTOM) up = true;
    }
    add = add === 2 ? 0 : add + 1;
    g.palette.remap(Color.YELLOW, sety);
    // Monochrome: the pulse shows as a flash between the two darkest looks.
    g.setMonoLook(Color.YELLOW, sety > (BOTTOM + TOP) / 2 ? MONO_SECOND : MONO_DARKEST);
    draw();
    if (input.kbhit() || input.getMouseInput().button === LMB) break;
    await nextFrame();
  }
  if (input.kbhit()) await input.getKey();
  setViewport(g, vp);
  g.setColor(Color.GREEN);
  drawGrid(g, state.playgrid.cols(), state.playgrid.rows());
  g.setViewport(0, 0, 639, 349);
  g.clearScreen(false);
  g.setMonoLook(Color.YELLOW, null);
  g.setMonoLook(Color.RED, null);
  resetPalette(g);
}
