// Fullscreen's log: each new message appears over the board, in a box with
// the message box's border, and fades away on its own timer. Messages that
// come close together stack up, newest at the bottom, so none is cut short:
// "a", then "a b", then "b" once a has faded. An announcement (who won)
// stays until it is released, by a click, tap or key, and then fades at once.
// The message box itself is not shown in fullscreen, so this is how the
// player sees what happened.

import { BORDER, FILL, HAlign, VAlign, type Graph } from '../render/graph';
import { setViewport, type Viewport } from '../render/screen';

/** How long a message stays fully visible. */
export const FLASH_HOLD_MS = 2200;
/** How long it takes to fade out after that. */
export const FLASH_FADE_MS = 700;
export const FLASH_MS = FLASH_HOLD_MS + FLASH_FADE_MS;
/** Time between redraws while fading. */
export const FLASH_FRAME_MS = 33;
/** The most lines on show at once; a newer line pushes out the oldest (announcements last). */
export const FLASH_MAX_LINES = 4;

/** Characters per line the box is sized for: the message box's width. */
const BOX_CHARS = 30;

/** How opaque a message is `elapsed` ms after it appeared: full while held, then fading to nothing. */
export function flashAlpha(elapsed: number): number {
  if (elapsed <= FLASH_HOLD_MS) return 1;
  if (elapsed >= FLASH_MS) return 0;
  return 1 - (elapsed - FLASH_HOLD_MS) / FLASH_FADE_MS;
}

export interface FlashText {
  text: string;
  /** A line marked '~' in the message box, drawn in the hilite colour. */
  bold: boolean;
  alpha: number;
}

/** The messages on show, oldest first, each with its own timer. */
export class LogFlash {
  private lines: { text: string; bold: boolean; shownAt: number; sticky: boolean }[] = [];

  /**
   * Shows `line` from time `now` (milliseconds), below any still on show. A
   * sticky line (an announcement) stays until release().
   */
  show(line: string, now: number, sticky = false): void {
    const bold = line.startsWith('~');
    this.lines.push({ text: bold ? line.slice(1) : line, bold, shownAt: now, sticky });
    while (this.lines.length > FLASH_MAX_LINES) {
      const oldest = this.lines.findIndex((l) => !l.sticky);
      this.lines.splice(oldest >= 0 ? oldest : 0, 1);
    }
  }

  /** Sticky lines start fading now, with no hold. True if there were any. */
  release(now: number): boolean {
    let any = false;
    for (const l of this.lines) {
      if (!l.sticky) continue;
      l.sticky = false;
      l.shownAt = now - FLASH_HOLD_MS;
      any = true;
    }
    return any;
  }

  clear(): void {
    this.lines = [];
  }

  /** What to draw at time `now`, oldest first: empty once all have faded. */
  current(now: number): FlashText[] {
    this.lines = this.lines.filter((l) => l.sticky || flashAlpha(now - l.shownAt) > 0);
    return this.lines.map((l) => ({ text: l.text, bold: l.bold, alpha: l.sticky ? 1 : flashAlpha(now - l.shownAt) }));
  }

  /** Whether any line is on a timer at `now` (so the screen needs redrawing as it fades). Sticky lines don't change. */
  animating(now: number): boolean {
    return this.current(now).length > 0 && this.lines.some((l) => !l.sticky);
  }
}

/** The palette slots the box is drawn in: the message box's own. */
export interface FlashColors {
  /** The background (BLACK's slot). */
  back: number;
  /** The border: the message box's. */
  border: number;
  /** Ordinary text. */
  text: number;
  /** Text marked bold. */
  hilite: number;
}

/**
 * Draws the messages over the board, centred in it. The box is 92% of the
 * board's width, with a row for each line in the message box's cell size; it
 * fades with its last line, and each line fades on its own.
 */
export function drawLogFlash(g: Graph, board: Viewport, lines: FlashText[], colors: FlashColors): void {
  if (lines.length === 0) return;
  const width = board.x2 - board.x1 + 1;
  const height = board.y2 - board.y1 + 1;
  const boxW = Math.trunc(width * 0.92);
  const oneLineH = Math.trunc(height / 11);
  const cellH = oneLineH - 12;
  const step = cellH + 4;
  const boxH = oneLineH + (lines.length - 1) * step;
  const x1 = board.x1 + Math.trunc((width - boxW) / 2);
  const y1 = board.y1 + Math.trunc((height - boxH) / 2);
  const x2 = x1 + boxW - 1;
  const y2 = y1 + boxH - 1;
  const longest = Math.max(...lines.map((l) => l.text.length));
  const charW = Math.trunc((boxW - 16) / Math.max(BOX_CHARS, longest));
  // Each line is centred on the box's middle, as the Graph centres it on a point (the
  // modern font is narrower than its cells, so aligning by cells would sit it off-centre).
  // The capitals sit a tenth of a cell down and are seven tenths tall, so their middle is at 0.45 of the cell.
  const textX = x1 + boxW / 2;
  const firstY = y1 + oneLineH / 2 - 0.45 * cellH;

  const mask = g.getFillMask();
  const ctx = g.ctx;
  try {
    setViewport(g, board);
    g.setVieworg(0, 0);
    g.setFillMask(null);
    ctx.globalAlpha = Math.max(...lines.map((l) => l.alpha));
    g.setColor(colors.back);
    g.rectangle(FILL, x1, y1, x2, y2);
    g.setColor(colors.border);
    g.rectangle(BORDER, x1, y1, x2, y2);
    g.setCharSpacing(0);
    g.setCharSize(cellH, charW);
    g.setTextAlign(HAlign.CENTER, VAlign.TOP);
    lines.forEach((line, i) => {
      ctx.globalAlpha = line.alpha;
      g.setColor(line.bold ? colors.hilite : colors.text);
      g.grText(textX, firstY + i * step, line.text);
    });
  } finally {
    ctx.globalAlpha = 1;
    g.setFillMask(mask);
  }
}
