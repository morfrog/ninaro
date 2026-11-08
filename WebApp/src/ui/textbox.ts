// Scrolling text boxes (Legacy TXTBOX.C): the message and status boxes, and
// the help text. The C kept the lines in a circular doubly linked list with
// a `head` (line 1) and a `start` (the first line shown); here they are an
// array in head order and an index for start. A line starting with '~' is
// drawn in the bold (hilite) colour.

import { BORDER, HAlign, VAlign, type Graph } from '../render/graph';
import type { Viewport } from '../render/screen';

export const NOBORDER = -1;
export const ATSTART = 0;
export const ATEND = -1;

/** A colour that can change after the box is made (the C kept int pointers). */
export type ColorRef = () => number;

export class TextBox {
  /** Lines in order from the head (line 1). */
  private items: string[] = [];
  /** Index in items of the first line shown. */
  private start = 0;

  /** TextBoxNew */
  constructor(
    public lines: number,
    public borderCol: number,
    public textCol: ColorRef,
    public boldCol: ColorRef,
    /** Characters per line (the C's maxwidth). */
    readonly width: number,
  ) {}

  get size(): number {
    return this.items.length;
  }

  private clip(s: string): string {
    return s.slice(0, this.width);
  }

  /**
   * TextBoxFixedAddLine: adds a line at the end; once the box holds `lines`
   * lines, the first is dropped instead of growing (the shown lines move on
   * with it).
   */
  fixedAddLine(s: string): boolean {
    if (this.size < this.lines) return this.addLine(s, ATEND);
    // The C wrote into the head node, then moved head and start on one.
    this.items.push(this.clip(s));
    this.items.shift();
    return true;
  }

  /**
   * TextBoxAddLine: after line `afterLine` (1-based), at the start, or at the
   * end. False if afterLine was past the end (it is then added at the end).
   */
  addLine(s: string, afterLine: number): boolean {
    const line = this.clip(s);
    if (this.size === 0) {
      this.items = [line];
      this.start = 0;
      return true;
    }
    const outside = afterLine > this.size;
    if (afterLine === ATSTART) {
      // Inserted after the last line, then made the head: the first line.
      // The line shown first stays the same one.
      this.items.unshift(line);
      this.start += 1;
      return true;
    }
    if (afterLine === ATEND || outside) this.items.push(line);
    else {
      this.items.splice(afterLine, 0, line);
      if (afterLine <= this.start) this.start += 1;
    }
    return !outside;
  }

  /** TextBoxDelLine: removes line `line` (1-based); later lines move up. */
  delLine(line: number): boolean {
    if (line > this.size || line < 1) return false;
    const index = line - 1;
    if (index === this.start) {
      // start moved to the next line (around to the head after the last).
      this.items.splice(index, 1);
      this.start = this.items.length === 0 ? 0 : index % this.items.length;
    } else {
      this.items.splice(index, 1);
      if (index < this.start) this.start -= 1;
    }
    return true;
  }

  /** TextBoxLine: line `line` (1-based), or null past the end. */
  line(line: number): string | null {
    return line > this.size || line < 1 ? null : this.items[line - 1];
  }

  /** TextBoxSetLine */
  setLine(line: number, s: string): boolean {
    if (line > this.size || line < 1) return false;
    this.items[line - 1] = s;
    return true;
  }

  /** TextBoxEmpty */
  empty(): boolean {
    if (this.size === 0) return false;
    this.items = [];
    this.start = 0;
    return true;
  }

  /** The lines TextBoxDisplay shows: up to `lines` from start, stopping at the last line. */
  visibleLines(): string[] {
    if (this.size === 0) return [];
    return this.items.slice(this.start, this.start + this.lines);
  }

  /**
   * TextBoxDisplay: each line in a cell vp-height / lines high and
   * vp-width / width wide (integer division), from the top left of vp in
   * screen coordinates, then the border.
   */
  display(g: Graph, vp: Viewport): void {
    const width = vp.x2 - vp.x1 + 1;
    const height = vp.y2 - vp.y1 + 1;
    const cheight = Math.trunc(height / this.lines);
    let at = vp.y1;
    g.setCharSpacing(0);
    g.setTextAlign(HAlign.LEFT, VAlign.TOP);
    g.setCharSize(cheight, Math.trunc(width / this.width));
    for (const s of this.visibleLines()) {
      if (s.startsWith('~')) {
        g.setColor(this.boldCol());
        g.grText(vp.x1, at, s.slice(1));
      } else {
        g.setColor(this.textCol());
        g.grText(vp.x1, at, s);
      }
      at += cheight;
    }
    if (this.borderCol !== NOBORDER) {
      g.setColor(this.borderCol);
      g.rectangle(BORDER, vp.x1, vp.y1, vp.x2, vp.y2);
    }
  }

  /** TextBoxScrollHome */
  scrollHome(): boolean {
    if (this.start === 0) return false;
    this.start = 0;
    return true;
  }

  /** TextBoxScrollEnd: shows the last `lines` lines. */
  scrollEnd(): boolean {
    const before = this.start;
    this.start = Math.max(0, this.size - this.lines);
    return this.start !== before;
  }

  /**
   * TextBoxScroll: down (positive) or up (negative) by up to `amount`
   * lines, stopping at the first or last line; 0 goes to the top.
   */
  scroll(amount: number): boolean {
    const last = this.size - 1;
    if (amount < 0) {
      if (this.start === 0) return false;
      this.start = Math.max(0, this.start + amount);
    } else if (amount > 0) {
      if (this.start === last) return false;
      this.start = Math.min(last, this.start + amount);
    } else {
      this.start = 0;
      return false;
    }
    return true;
  }

  /** TextBoxScrollLine: shows from line `line` (1-based). */
  scrollLine(line: number): boolean {
    if (line > this.size || line < 1) return false;
    this.start = line - 1;
    return true;
  }

  /** TextBoxScrollString: shows from the next line containing s, wrapping around. */
  scrollString(s: string): boolean {
    for (let i = 1; i < this.size; i++) {
      const index = (this.start + i) % this.size;
      if (this.items[index].includes(s)) {
        this.start = index;
        return true;
      }
    }
    return false;
  }

  /** TextBoxAtTop */
  atTop(): boolean {
    return this.start === 0;
  }

  /** TextBoxAtBottom: the first line shown is the last line. */
  atBottom(): boolean {
    return this.start === this.size - 1;
  }
}
