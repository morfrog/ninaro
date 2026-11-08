// Ninaro web port, phase 0: draws the empty default board (7 x 8) where the
// original put it, to check the layout and the 4:3 scaling.

import { ENTRY_GRID, PLAY_GRID, SCREEN, type Viewport, viewportFromViewport } from './layout';

const GREEN = '#00aa00'; // EGA green, Legacy GREEN

/** Legacy DrawGrid: lines across and down a viewport in window coordinates. */
function drawGrid(ctx: CanvasRenderingContext2D, vp: Viewport, cols: number, rows: number): void {
  const w = vp.x2 - vp.x1;
  const h = vp.y2 - vp.y1;
  ctx.beginPath();
  for (let row = 0; row <= rows; row++) {
    const y = vp.y1 + Math.round((h * row) / rows) + 0.5;
    ctx.moveTo(vp.x1, y);
    ctx.lineTo(vp.x2 + 1, y);
  }
  for (let col = 0; col <= cols; col++) {
    const x = vp.x1 + Math.round((w * col) / cols) + 0.5;
    ctx.moveTo(x, vp.y1);
    ctx.lineTo(x, vp.y2 + 1);
  }
  ctx.stroke();
}

const canvas = document.querySelector<HTMLCanvasElement>('#screen')!;
const ctx = canvas.getContext('2d')!;
ctx.strokeStyle = GREEN;
ctx.lineWidth = 1;
drawGrid(ctx, viewportFromViewport(ENTRY_GRID, SCREEN), 7, 1);
drawGrid(ctx, viewportFromViewport(PLAY_GRID, SCREEN), 7, 8);
