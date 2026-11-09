// Fullscreen's log: each new message appears once, as a single line in a box
// with the message box's border, centred over the board, and fades away.
// The message box itself is not shown in fullscreen, so this is how the
// player sees what happened.

import { BORDER, FILL, HAlign, VAlign, type Graph } from '../render/graph';
import { setViewport, type Viewport } from '../render/screen';

/** How long a message stays fully visible. */
export const FLASH_HOLD_MS = 1300;
/** How long it takes to fade out after that. */
export const FLASH_FADE_MS = 700;
export const FLASH_MS = FLASH_HOLD_MS + FLASH_FADE_MS;
/** Time between redraws while fading. */
export const FLASH_FRAME_MS = 33;

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

/** The message on show, if any. Only the latest one is kept. */
export class LogFlash {
  private line: { text: string; bold: boolean; shownAt: number } | null = null;

  /** Shows `line` from time `now` (milliseconds), replacing any message on show. */
  show(line: string, now: number): void {
    const bold = line.startsWith('~');
    this.line = { text: bold ? line.slice(1) : line, bold, shownAt: now };
  }

  clear(): void {
    this.line = null;
  }

  /** What to draw at time `now`, or null once it has faded out. */
  current(now: number): FlashText | null {
    if (!this.line) return null;
    const alpha = flashAlpha(now - this.line.shownAt);
    if (alpha <= 0) {
      this.line = null;
      return null;
    }
    return { text: this.line.text, bold: this.line.bold, alpha };
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
 * Draws the message over the board, centred in it, at the current alpha.
 * The box is 92% of the board's width and a line and a bit high, with the
 * text in the message box's cell size.
 */
export function drawLogFlash(g: Graph, board: Viewport, flash: FlashText, colors: FlashColors): void {
  const width = board.x2 - board.x1 + 1;
  const height = board.y2 - board.y1 + 1;
  const boxW = Math.trunc(width * 0.92);
  const boxH = Math.trunc(height / 11);
  const x1 = board.x1 + Math.trunc((width - boxW) / 2);
  const y1 = board.y1 + Math.trunc((height - boxH) / 2);
  const x2 = x1 + boxW - 1;
  const y2 = y1 + boxH - 1;
  const cellH = boxH - 12;
  const charW = Math.trunc((boxW - 16) / Math.max(BOX_CHARS, flash.text.length));
  const textW = flash.text.length * charW;
  const textX = x1 + Math.trunc((boxW - textW) / 2);
  const textY = y1 + Math.trunc((boxH - cellH) / 2);

  const mask = g.getFillMask();
  const ctx = g.ctx;
  ctx.globalAlpha = flash.alpha;
  try {
    setViewport(g, board);
    g.setVieworg(0, 0);
    g.setFillMask(null);
    g.setColor(colors.back);
    g.rectangle(FILL, x1, y1, x2, y2);
    g.setColor(colors.border);
    g.rectangle(BORDER, x1, y1, x2, y2);
    g.setCharSpacing(0);
    g.setCharSize(cellH, charW);
    g.setTextAlign(HAlign.LEFT, VAlign.TOP);
    g.setColor(flash.bold ? colors.hilite : colors.text);
    g.grText(textX, textY, flash.text);
  } finally {
    ctx.globalAlpha = 1;
    g.setFillMask(mask);
  }
}
