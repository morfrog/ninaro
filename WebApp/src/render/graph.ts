// The parts of Watcom's graph.h library Ninaro used, drawn on a canvas.
//
// Coordinates are EGA mode 16 pixels (640 x 350). The canvas context passed
// in is already scaled from those to device pixels, so a 1-pixel line covers
// one EGA pixel (taller than wide, as on a 4:3 screen) while text strokes and
// curves stay smooth at the device's resolution.
//
// Behaviour measured against the original running in DOSBox:
// - _setviewport clips to a rectangle and moves the view origin to its
//   corner; _setvieworg moves only the origin.
// - Window coordinates (the _w functions, after ScreenInit's
//   _setwindow(0, 0.0, 0.0, 1.0, 1.0)) span the current viewport:
//   x = x1 + trunc(wx * (x2 - x1)).
// - But _setcharsize_w and _setcharspacing_w sizes are scaled by the whole
//   screen (639 across, 349 down), not the viewport.
// - Text is fixed pitch: each character fills a cell of the _setcharsize
//   width, plus the _setcharspacing gap. Capitals start 0.1 of the cell
//   height down and are 0.7 of it tall.

import type { DisplayMode } from './displayMode';
import { HERSHEY_SIMPLEX } from './hersheyFont';
import { MONO_LOOKS, isBackgroundLook, monoInkCss, monoLookIndices, monoTilePixels, type MonoLook } from './displayStyle';
import type { Palette } from './palette';

export const SCREEN_WIDTH = 640;
export const SCREEN_HEIGHT = 350;

/** What the full-screen window's 0..1 spans, in pixels, for _w sizes. */
const WINDOW_SCALE_X = SCREEN_WIDTH - 1;
const WINDOW_SCALE_Y = SCREEN_HEIGHT - 1;

/** _settextalign horizontal values. */
export const HAlign = { NORMAL: 0, LEFT: 1, CENTER: 2, RIGHT: 3 } as const;
export type HAlign = (typeof HAlign)[keyof typeof HAlign];

/** _settextalign vertical values. */
export const VAlign = { NORMAL: 0, TOP: 1, CAP: 2, HALF: 3, BASE: 4, BOTTOM: 5 } as const;
export type VAlign = (typeof VAlign)[keyof typeof VAlign];

/** _GBORDER / _GFILLINTERIOR */
export const BORDER = false;
export const FILL = true;

/** Where the top of a text cell sits relative to the y given, for each vertical alignment. */
const CELL_TOP: Record<VAlign, number> = { 0: 0, 1: 0, 2: -0.1, 3: -0.5, 4: -0.8, 5: -1 };

/** Hershey units across a cell, measured to match Modern's glyph widths. */
const GLYPH_UNITS_PER_CELL = 32;
/** Hershey cap height (y 1 to 22) and where capitals sit in a cell. */
const HERSHEY_CAP = 21;
const CAP_TOP = 0.1;
const CAP_HEIGHT = 0.7;

/** A _setlinestyle mask as canvas dash lengths: runs of on and off bits from the high bit. */
export function lineDash(mask: number): number[] {
  const bits = mask & 0xffff;
  if (bits === 0xffff) return [];
  const runs: number[] = [];
  let on = true; // canvas dashes start with a drawn run
  let run = 0;
  for (let b = 15; b >= 0; b--) {
    const set = ((bits >> b) & 1) === 1;
    if (set === on) run++;
    else {
      runs.push(run);
      on = !on;
      run = 1;
    }
  }
  runs.push(run);
  if (runs.length % 2) runs.push(0);
  return runs;
}

interface Rect {
  x1: number;
  y1: number;
  x2: number;
  y2: number;
}

export class Graph {
  readonly ctx: CanvasRenderingContext2D;
  readonly palette: Palette;

  private clipRect: Rect = { x1: 0, y1: 0, x2: SCREEN_WIDTH - 1, y2: SCREEN_HEIGHT - 1 };
  private originX = 0;
  private originY = 0;
  private color = 15;
  private fillMask: readonly number[] | null = null;
  private lineDash: number[] = [];
  private posX = 0;
  private posY = 0;
  private charHeight = 8;
  private charWidth = 8;
  private charSpacing = 0;
  private alignH: HAlign = HAlign.NORMAL;
  private alignV: VAlign = VAlign.NORMAL;
  private display: DisplayMode = 'vga';
  /** Monochrome hatch patterns, by look and background colour. */
  private readonly patterns = new Map<string, string | CanvasPattern>();
  /** Colour slots fixed to a monochrome look, by look index. */
  private readonly monoFixed = new Map<number, { look: number; value: number }>();

  constructor(ctx: CanvasRenderingContext2D, palette: Palette) {
    this.ctx = ctx;
    this.palette = palette;
  }

  /** The Display option: 'mono' draws in the background and three greens; other modes use the colours. */
  setDisplay(mode: DisplayMode): void {
    if (mode === this.display) return;
    this.display = mode;
    this.patterns.clear();
  }

  /** The style for a colour slot: its colour, or in monochrome its hatch pattern. */
  private paintFor(slot: number): string | CanvasPattern {
    if (this.display !== 'mono') return this.palette.css(slot);
    const bg = this.palette.css(0);
    const look = this.monoLook(slot);
    if (isBackgroundLook(look)) return bg;
    const key = `${MONO_LOOKS.indexOf(look)}|${bg}`;
    const cached = this.patterns.get(key);
    if (cached) return cached;
    const tile = document.createElement('canvas');
    tile.width = 8;
    tile.height = 8;
    const t = tile.getContext('2d')!;
    monoTilePixels(look, bg).forEach((css, i) => {
      t.fillStyle = css;
      t.fillRect(i % 8, Math.floor(i / 8), 1, 1);
    });
    // Anchored at screen (0, 0) in EGA pixels, as the hardware did.
    const pattern = this.ctx.createPattern(tile, 'repeat') ?? monoInkCss(look, bg);
    this.patterns.set(key, pattern);
    return pattern;
  }

  /** A single colour for the slot: in monochrome, the ink of its look. */
  private inkCss(slot: number): string {
    if (this.display !== 'mono') return this.palette.css(slot);
    return monoInkCss(this.monoLook(slot), this.palette.css(0));
  }

  private monoLook(slot: number): MonoLook {
    const fixed = this.monoFixed.get(slot & 15);
    if (fixed !== undefined) return MONO_LOOKS[fixed.look];
    // Ranking uses each fixed slot's value as it was when fixed, so a pulsing
    // slot cannot move the other colours' looks.
    const values = Array.from({ length: 16 }, (_, i) => this.monoFixed.get(i)?.value ?? this.palette.value(i));
    return MONO_LOOKS[monoLookIndices(values)[slot & 15]];
  }

  /**
   * Fixes a colour slot to one of the monochrome looks (by index), or frees it
   * with null. Used where a screen needs a particular look for a colour, such
   * as the title screen's tokens.
   */
  setMonoLook(slot: number, look: number | null): void {
    if (look === null) {
      this.monoFixed.delete(slot & 15);
      return;
    }
    // The palette value is taken the first time the slot is fixed; later changes keep it.
    const value = this.monoFixed.get(slot & 15)?.value ?? this.palette.value(slot & 15);
    this.monoFixed.set(slot & 15, { look, value });
  }

  // ---- Viewports and coordinates ----

  /** _setviewport: clip to the rectangle and put the view origin at its corner. */
  setViewport(x1: number, y1: number, x2: number, y2: number): void {
    this.clipRect = { x1, y1, x2, y2 };
    this.originX = x1;
    this.originY = y1;
  }

  /** _setvieworg: put view (0, 0) at screen (x, y). */
  setVieworg(x: number, y: number): void {
    this.originX = x;
    this.originY = y;
  }

  get viewport(): Readonly<Rect> {
    return this.clipRect;
  }

  /** Window x (0..1 across the viewport) to screen pixels, truncated as Watcom did. */
  private windowX(wx: number): number {
    return this.clipRect.x1 + Math.trunc(wx * (this.clipRect.x2 - this.clipRect.x1));
  }

  private windowY(wy: number): number {
    return this.clipRect.y1 + Math.trunc(wy * (this.clipRect.y2 - this.clipRect.y1));
  }

  /** _clearscreen(_GCLEARSCREEN) or (_GVIEWPORT): fill with colour 0. */
  clearScreen(viewportOnly: boolean): void {
    const r = viewportOnly ? this.clipRect : { x1: 0, y1: 0, x2: SCREEN_WIDTH - 1, y2: SCREEN_HEIGHT - 1 };
    this.ctx.fillStyle = this.palette.css(0);
    this.ctx.fillRect(r.x1, r.y1, r.x2 - r.x1 + 1, r.y2 - r.y1 + 1);
  }

  // ---- Drawing state ----

  /** _setcolor */
  setColor(color: number): void {
    this.color = color;
  }

  /** _setfillmask: 8 bytes, top row first, bit 7 leftmost; null for solid. */
  setFillMask(mask: readonly number[] | null): void {
    this.fillMask = mask;
  }

  getFillMask(): readonly number[] | null {
    return this.fillMask;
  }

  /** _setlinestyle: a 16-pixel on/off pattern, high bit first (0xffff is solid). */
  setLineStyle(mask: number): void {
    this.lineDash = lineDash(mask);
  }

  private fillStyle(): string | CanvasPattern {
    if (!this.fillMask) return this.paintFor(this.color);
    const css = this.inkCss(this.color);
    const tile = document.createElement('canvas');
    tile.width = 8;
    tile.height = 8;
    const t = tile.getContext('2d')!;
    t.fillStyle = css;
    this.fillMask.forEach((row, y) => {
      for (let x = 0; x < 8; x++) if (row & (0x80 >> x)) t.fillRect(x, y, 1, 1);
    });
    // Anchored at screen (0, 0) in EGA pixels, as the hardware did.
    return this.ctx.createPattern(tile, 'repeat') ?? css;
  }

  /** Runs draw() clipped to the viewport. */
  private clipped(draw: (ctx: CanvasRenderingContext2D) => void): void {
    const { ctx } = this;
    const r = this.clipRect;
    ctx.save();
    ctx.beginPath();
    ctx.rect(r.x1, r.y1, r.x2 - r.x1 + 1, r.y2 - r.y1 + 1);
    ctx.clip();
    ctx.imageSmoothingEnabled = false;
    draw(ctx);
    ctx.restore();
  }

  // ---- Shapes (view coordinates) ----

  /** _rectangle */
  rectangle(fill: boolean, x1: number, y1: number, x2: number, y2: number): void {
    this.screenRectangle(fill, x1 + this.originX, y1 + this.originY, x2 + this.originX, y2 + this.originY);
  }

  /** _rectangle_w */
  rectangleW(fill: boolean, wx1: number, wy1: number, wx2: number, wy2: number): void {
    this.screenRectangle(fill, this.windowX(wx1), this.windowY(wy1), this.windowX(wx2), this.windowY(wy2));
  }

  private screenRectangle(fill: boolean, ax: number, ay: number, bx: number, by: number): void {
    const x1 = Math.min(ax, bx);
    const x2 = Math.max(ax, bx);
    const y1 = Math.min(ay, by);
    const y2 = Math.max(ay, by);
    this.clipped((ctx) => {
      if (fill) {
        ctx.fillStyle = this.fillStyle();
        ctx.fillRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
      } else {
        ctx.strokeStyle = this.paintFor(this.color);
        ctx.lineWidth = 1;
        ctx.setLineDash(this.lineDash);
        ctx.strokeRect(x1 + 0.5, y1 + 0.5, x2 - x1, y2 - y1);
      }
    });
  }

  /** _ellipse: the ellipse inside the bounding box. */
  ellipse(fill: boolean, x1: number, y1: number, x2: number, y2: number): void {
    this.screenEllipse(fill, x1 + this.originX, y1 + this.originY, x2 + this.originX, y2 + this.originY);
  }

  /** _ellipse_w */
  ellipseW(fill: boolean, wx1: number, wy1: number, wx2: number, wy2: number): void {
    this.screenEllipse(fill, this.windowX(wx1), this.windowY(wy1), this.windowX(wx2), this.windowY(wy2));
  }

  private screenEllipse(fill: boolean, ax: number, ay: number, bx: number, by: number): void {
    const x1 = Math.min(ax, bx);
    const x2 = Math.max(ax, bx);
    const y1 = Math.min(ay, by);
    const y2 = Math.max(ay, by);
    const cx = (x1 + x2 + 1) / 2;
    const cy = (y1 + y2 + 1) / 2;
    const inset = fill ? 0 : 0.5;
    const rx = Math.max(0, (x2 - x1 + 1) / 2 - inset);
    const ry = Math.max(0, (y2 - y1 + 1) / 2 - inset);
    this.clipped((ctx) => {
      ctx.beginPath();
      ctx.ellipse(cx, cy, rx, ry, 0, 0, Math.PI * 2);
      if (fill) {
        ctx.fillStyle = this.fillStyle();
        ctx.fill();
      } else {
        ctx.strokeStyle = this.paintFor(this.color);
        ctx.lineWidth = 1;
        ctx.stroke();
      }
    });
  }

  /** _moveto */
  moveTo(x: number, y: number): void {
    this.posX = x + this.originX;
    this.posY = y + this.originY;
  }

  /** _moveto_w */
  moveToW(wx: number, wy: number): void {
    this.posX = this.windowX(wx);
    this.posY = this.windowY(wy);
  }

  /** _lineto */
  lineTo(x: number, y: number): void {
    this.screenLine(x + this.originX, y + this.originY);
  }

  /** _lineto_w */
  lineToW(wx: number, wy: number): void {
    this.screenLine(this.windowX(wx), this.windowY(wy));
  }

  private screenLine(x: number, y: number): void {
    const fromX = this.posX;
    const fromY = this.posY;
    this.clipped((ctx) => {
      ctx.strokeStyle = this.paintFor(this.color);
      ctx.lineWidth = 1;
      ctx.lineCap = this.lineDash.length ? 'butt' : 'square';
      ctx.setLineDash(this.lineDash);
      ctx.beginPath();
      ctx.moveTo(fromX + 0.5, fromY + 0.5);
      ctx.lineTo(x + 0.5, y + 0.5);
      ctx.stroke();
    });
    this.posX = x;
    this.posY = y;
  }

  // ---- Text ----

  /** _setcharsize, in pixels */
  setCharSize(height: number, width: number): void {
    this.charHeight = height;
    this.charWidth = width;
  }

  /** _setcharsize_w: sizes in full-screen window units */
  setCharSizeW(height: number, width: number): void {
    this.charHeight = height * WINDOW_SCALE_Y;
    this.charWidth = width * WINDOW_SCALE_X;
  }

  /** _setcharspacing, in pixels */
  setCharSpacing(spacing: number): void {
    this.charSpacing = spacing;
  }

  /** _setcharspacing_w */
  setCharSpacingW(spacing: number): void {
    this.charSpacing = spacing * WINDOW_SCALE_X;
  }

  /** _settextalign */
  setTextAlign(horizontal: HAlign, vertical: VAlign): void {
    this.alignH = horizontal;
    this.alignV = vertical;
  }

  /** The width _grtext gives a string with the current settings. */
  textWidth(text: string): number {
    if (text.length === 0) return 0;
    return text.length * (this.charWidth + this.charSpacing) - this.charSpacing;
  }

  /** _grtext, at view coordinates */
  grText(x: number, y: number, text: string): void {
    this.screenText(x + this.originX, y + this.originY, text);
  }

  /** _grtext_w */
  grTextW(wx: number, wy: number, text: string): void {
    this.screenText(this.windowX(wx), this.windowY(wy), text);
  }

  private screenText(x: number, y: number, text: string): void {
    const w = this.charWidth;
    const h = this.charHeight;
    const advance = w + this.charSpacing;
    const width = this.textWidth(text);
    let left = x;
    if (this.alignH === HAlign.CENTER) left = x - width / 2;
    else if (this.alignH === HAlign.RIGHT) left = x - width;
    const top = y + CELL_TOP[this.alignV] * h;
    const sx = w / GLYPH_UNITS_PER_CELL;
    const sy = (CAP_HEIGHT * h) / HERSHEY_CAP;
    this.clipped((ctx) => {
      ctx.strokeStyle = this.paintFor(this.color);
      ctx.lineWidth = 1;
      ctx.lineCap = 'round';
      ctx.lineJoin = 'round';
      ctx.beginPath();
      for (let i = 0; i < text.length; i++) {
        const glyph = HERSHEY_SIMPLEX[text.charCodeAt(i) - 32] ?? HERSHEY_SIMPLEX['?'.charCodeAt(0) - 32];
        const [half, strokes] = glyph;
        const cx = left + i * advance + w / 2;
        for (const stroke of strokes) {
          for (let p = 0; p < stroke.length; p += 2) {
            const px = cx + (stroke[p] - half) * sx;
            const py = top + CAP_TOP * h + (stroke[p + 1] - 1) * sy;
            if (p === 0) ctx.moveTo(px, py);
            else ctx.lineTo(px, py);
          }
        }
      }
      ctx.stroke();
    });
  }
}
