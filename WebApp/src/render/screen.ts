// Screen helpers from SCREEN.C, and the canvas that stands in for the EGA
// screen.

import { BORDER, FILL, Graph, SCREEN_HEIGHT, SCREEN_WIDTH } from './graph';
import { Palette } from './palette';

/** VIEWPORT: a pixel rectangle, corners inclusive. */
export interface Viewport {
  x1: number;
  y1: number;
  x2: number;
  y2: number;
}

/** VIEWWIN: a rectangle as fractions of a viewport. */
export type ViewWin = Viewport;

/** ViewportFromViewport: the vw part of vp, truncated as C's (int) casts did. */
export function viewportFromViewport(vw: ViewWin, vp: Viewport): Viewport {
  const width = vp.x2 - vp.x1;
  const height = vp.y2 - vp.y1;
  return {
    x1: vp.x1 + Math.trunc(vw.x1 * width),
    y1: vp.y1 + Math.trunc(vw.y1 * height),
    x2: vp.x1 + Math.trunc(vw.x2 * width),
    y2: vp.y1 + Math.trunc(vw.y2 * height),
  };
}

/** PointInsideViewport */
export function pointInsideViewport(x: number, y: number, vp: Viewport): boolean {
  return x <= vp.x2 && x >= vp.x1 && y <= vp.y2 && y >= vp.y1;
}

/** SetViewport */
export function setViewport(g: Graph, vp: Viewport): void {
  g.setViewport(vp.x1, vp.y1, vp.x2, vp.y2);
}

/** ClearViewport */
export function clearViewport(g: Graph, vp: Viewport): void {
  setViewport(g, vp);
  g.setVieworg(0, 0);
  g.clearScreen(true);
}

/** BorderedBox: filled in p, edged in s (screen coordinates). */
export function borderedBox(g: Graph, x1: number, y1: number, x2: number, y2: number, p: number, s: number): void {
  g.setColor(p);
  g.rectangle(FILL, x1, y1, x2, y2);
  g.setColor(s);
  g.rectangle(BORDER, x1, y1, x2, y2);
}

/** ResetPalette */
export function resetPalette(g: Graph): void {
  g.palette.reset();
}

/** DrawGrid: x columns and y rows of lines across the viewport. */
export function drawGrid(g: Graph, x: number, y: number): void {
  const height = 1.0 / y;
  const width = 1.0 / x;
  for (let row = 0; row <= y; row++) {
    g.moveToW(0.0, height * row);
    g.lineToW(1.0, height * row);
  }
  for (let col = 0; col <= x; col++) {
    g.moveToW(width * col, 0.0);
    g.lineToW(width * col, 1.0);
  }
}

/**
 * DrawDisc: a filled circle, discsize of the cell across, centred in it. The
 * cells may be wider than they are tall (Fullscreen stretches the board
 * across), so the circle is sized in device pixels and then converted back
 * to window fractions for the ellipse.
 */
export function drawDisc(g: Graph, col: number, cols: number, row: number, rows: number, color: number, discsize: number): void {
  const vp = g.viewport;
  const boardW = (vp.x2 - vp.x1 + 1) * g.pixelScaleX;
  const boardH = (vp.y2 - vp.y1 + 1) * g.pixelScaleY;
  const side = discsize / 2;
  const radius = side * Math.min(boardW / cols, boardH / rows);
  const wherex = (col + 0.5) / cols;
  const wherey = (row + 0.5) / rows;
  const hw = radius / boardW;
  const hh = radius / boardH;
  g.setColor(color);
  g.ellipseW(FILL, wherex - hw, wherey - hh, wherex + hw, wherey + hh);
}

/**
 * The canvas standing in for the EGA screen: 640 x 350 EGA pixels, shown at
 * the 4:3 shape of the monitors of the time and drawn at the device's
 * resolution. When its size changes the drawing is lost, so onResize must
 * draw everything again.
 */
export class Screen {
  readonly canvas: HTMLCanvasElement;
  readonly graph: Graph;
  onResize: () => void = () => {};

  constructor(canvas: HTMLCanvasElement, palette = new Palette()) {
    this.canvas = canvas;
    this.graph = new Graph(canvas.getContext('2d')!, palette);
    this.fit();
    new ResizeObserver(() => {
      if (this.fit()) this.onResize();
    }).observe(canvas);
  }

  /** Sizes the backing store to the displayed size; true if it changed. */
  private fit(): boolean {
    const rect = this.canvas.getBoundingClientRect();
    const dpr = window.devicePixelRatio || 1;
    const width = Math.max(SCREEN_WIDTH, Math.round(rect.width * dpr));
    const height = Math.max(SCREEN_HEIGHT, Math.round(rect.height * dpr));
    if (width === this.canvas.width && height === this.canvas.height) return false;
    this.canvas.width = width;
    this.canvas.height = height;
    this.graph.ctx.setTransform(width / SCREEN_WIDTH, 0, 0, height / SCREEN_HEIGHT, 0, 0);
    this.graph.setPixelScale(width / SCREEN_WIDTH, height / SCREEN_HEIGHT);
    return true;
  }

  /** Converts a pointer position on the canvas to EGA pixels. */
  toScreen(clientX: number, clientY: number): { x: number; y: number } {
    const rect = this.canvas.getBoundingClientRect();
    return {
      x: Math.floor(((clientX - rect.left) / rect.width) * SCREEN_WIDTH),
      y: Math.floor(((clientY - rect.top) / rect.height) * SCREEN_HEIGHT),
    };
  }
}
