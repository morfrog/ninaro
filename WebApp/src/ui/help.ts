// The help viewer (Legacy HELP.C): a text box over the saved screen. The
// current player's keys scroll it (left/right a page, up/down a line);
// Home/End (keypad 7/1) and PgUp/PgDn (keypad 9/3) work too; select,
// cancel, Esc or F1 close it. A click or tap also closes it, and the mouse
// wheel or a finger dragged up and down scrolls, for players without a
// keyboard.

import type { Input } from '../input/input';
import { LMB, RMB } from '../input/input';
import { ESC, F1, KP1, KP3, KP7, KP9, KeySys, type KeyGroup } from '../input/keys';
import { FILL, type Graph } from '../render/graph';
import type { MemPage } from '../render/memPage';
import { Color } from '../render/palette';
import type { Viewport } from '../render/screen';
import { ATEND, TextBox, type ColorRef } from './textbox';

/**
 * The help file's lines as HelpNew read them: up to 79 characters at a time
 * up to each carriage return, then one character (the line feed) skipped.
 */
export function helpLines(source: string): string[] {
  // Git may store the file with LF line endings; read it as the DOS file it was.
  const text = source.replace(/\r?\n/g, '\r\n');
  const lines: string[] = [];
  let at = 0;
  while (at < text.length) {
    const cr = text.indexOf('\r', at);
    const end = Math.min(cr < 0 ? text.length : cr + 1, at + 79);
    lines.push(text.slice(at, end).replace(/\r$/, ''));
    at = end + 1; // the line feed
  }
  return lines;
}

export class Help {
  readonly tb: TextBox;
  readonly ks: KeySys;

  /** HelpNew: lines come from the help file's text, if any. */
  constructor(
    private readonly g: Graph,
    text: string | null,
    kg: KeyGroup,
    lines: number,
    borderCol: number,
    textCol: ColorRef,
    boldCol: ColorRef,
    maxWidth: number,
  ) {
    this.tb = new TextBox(lines, borderCol, textCol, boldCol, maxWidth);
    this.ks = new KeySys(kg);
    this.ks.iFunc(0, () => (this.pageUp() ? 1 : 0));
    this.ks.iFunc(1, () => (this.pageDown() ? 1 : 0));
    this.ks.iFunc(2, () => (this.tb.scroll(-1) ? 1 : 0));
    this.ks.iFunc(3, () => (this.tb.scroll(1) ? 1 : 0));
    this.ks.iFunc(4, () => 0);
    this.ks.iFunc(5, () => 0);
    if (text !== null) for (const line of helpLines(text)) this.tb.addLine(line, ATEND);
  }

  /** HelpAddLine */
  addLine(s: string): void {
    this.tb.addLine(s, ATEND);
  }

  pageUp(): boolean {
    return this.tb.scroll(-this.tb.lines);
  }

  pageDown(): boolean {
    return this.tb.scroll(this.tb.lines);
  }

  /** HelpHome */
  home(): boolean {
    return this.tb.scrollHome();
  }

  /** HelpScrollString */
  scrollString(s: string): boolean {
    return this.tb.scrollString(s);
  }

  /** HelpDisplay */
  display(vp: Viewport): void {
    this.g.setViewport(0, 0, 639, 349);
    this.g.setColor(Color.BLACK);
    this.g.rectangle(FILL, vp.x1, vp.y1, vp.x2, vp.y2);
    this.tb.display(this.g, vp);
  }

  /** HelpCall: shows the help over mp until it is closed. */
  async call(vp: Viewport, mp: MemPage, input: Input): Promise<void> {
    mp.put();
    this.display(vp);
    for (;;) {
      const scroll = input.takeWheel();
      if (scroll && this.tb.scroll(scroll)) this.display(vp);
      const m = input.getMouseInput();
      if (m.button === LMB || m.button === RMB) return;
      if (!input.kbhit()) {
        await input.wait();
        continue;
      }
      const key = await input.getKey();
      let { used, index, funcret } = await this.ks.key(key, null);
      if (!used) {
        if (key === ESC || key === F1) return;
        used = true;
        index = 6;
        if (key === KP7) funcret = this.home() ? 1 : 0;
        else if (key === KP1) funcret = this.tb.scrollEnd() ? 1 : 0;
        else if (key === KP9) funcret = this.pageUp() ? 1 : 0;
        else if (key === KP3) funcret = this.pageDown() ? 1 : 0;
        else used = false;
      }
      if (used) {
        if (index === 4 || index === 5) return;
        if (funcret) this.display(vp);
      }
    }
  }
}
