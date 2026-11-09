// The menu bar and its drop-down menus (Legacy MENU.C).
//
// Keys are the current player's (left, right, up, down, select, cancel),
// plus Esc or M to close and F1 or H for help. With the pointer, the
// original (its pointer hidden) followed the mouse's position across the
// whole screen and selected on any left click. Here, with a visible
// pointer: moving over a menu name or item highlights it, as before;
// clicking an item selects it, clicking a menu name opens that menu, and
// clicking anywhere else (or a right click) closes the menu.

import type { Input } from '../input/input';
import { LMB, NMB, RMB } from '../input/input';
import { ESC, F1, KeySys, type KeyGroup } from '../input/keys';
import { BORDER, FILL, HAlign, VAlign, type Graph } from '../render/graph';
import type { MemPage } from '../render/memPage';
import { Color } from '../render/palette';
import { type Viewport } from '../render/screen';
import { isModern, MODERN, windowBox } from '../render/chrome';

const MENUBACK = Color.LTGREY;
const MENUTEXT = Color.BLACK;
/** Height of the bar and of each item, in pixels. */
const ROW = 30;

export interface MenuItem {
  name: string;
  /** The shortcut shown in ProgInit (never used by the original). */
  shortcut: number;
  run: () => void | Promise<void>;
  enabled: boolean;
}

export interface SubMenu {
  name: string;
  items: MenuItem[];
  current: number;
}

export class Menu {
  current = 0;
  readonly subs: SubMenu[] = [];
  readonly ks: KeySys;

  /** MenuNew */
  constructor(
    private readonly g: Graph,
    private readonly vp: Viewport,
    private readonly background: MemPage,
    kg: KeyGroup,
    private readonly hilite: () => number,
  ) {
    this.ks = new KeySys(kg);
    this.ks.iFunc(0, () => this.left());
    this.ks.iFunc(1, () => this.right());
    this.ks.iFunc(2, () => this.up());
    this.ks.iFunc(3, () => this.down());
    this.ks.iFunc(4, () => this.select());
    this.ks.iFunc(5, () => 1); // MenuCancel
  }

  /** MenuNewSub: the current item starts as the last enabled one. */
  addSub(name: string, items: MenuItem[]): void {
    let current = 0;
    items.forEach((item, n) => {
      if (item.enabled) current = n;
    });
    this.subs.push({ name, items, current });
  }

  private get sub(): SubMenu {
    return this.subs[this.current];
  }

  /** MenuLeft */
  left(): number {
    if (this.current > 0) {
      this.current -= 1;
      return 1;
    }
    return 0;
  }

  /** MenuRight */
  right(): number {
    if (this.current < this.subs.length - 1) {
      this.current += 1;
      return 1;
    }
    return 0;
  }

  /** MenuUp: to the enabled item above, if any. */
  up(): number {
    const sm = this.sub;
    for (let n = sm.current - 1; n >= 0; n--) {
      if (sm.items[n].enabled) {
        sm.current = n;
        return 1;
      }
    }
    return 0;
  }

  /** MenuDown: to the enabled item below, if any. */
  down(): number {
    const sm = this.sub;
    for (let n = sm.current + 1; n < sm.items.length; n++) {
      if (sm.items[n].enabled) {
        sm.current = n;
        return 1;
      }
    }
    return 0;
  }

  /** MenuSelect: runs the current item (even if it has since been disabled, as the C did). */
  async select(): Promise<number> {
    await this.sub.items[this.sub.current].run();
    return 1;
  }

  /** MenuItemEnable */
  itemEnable(sub: number, item: number, enable: boolean): boolean {
    const sm = this.subs[sub];
    if (!sm || item >= sm.items.length) return false;
    sm.items[item].enabled = enable;
    return true;
  }

  /** MenuSubCurrentSet: only to an enabled item. */
  subCurrentSet(sub: number, current: number): boolean {
    const sm = this.subs[sub];
    if (!sm || current >= sm.items.length || !sm.items[current].enabled) return false;
    sm.current = current;
    return true;
  }

  // ---- Drawing ----

  private barViewport(): Viewport {
    return { x1: this.vp.x1, y1: this.vp.y1, x2: this.vp.x2, y2: this.vp.y1 + ROW };
  }

  private subViewport(): Viewport {
    const width = this.vp.x2 - this.vp.x1 + 1;
    const n = this.subs.length;
    return {
      x1: this.vp.x1 + Math.trunc((width * this.current) / n),
      y1: this.vp.y1 + ROW,
      x2: this.vp.x1 + Math.trunc((width * (this.current + 1)) / n),
      y2: this.vp.y1 + ROW + this.sub.items.length * ROW,
    };
  }

  /** MenuDisplayBar */
  private displayBar(vp: Viewport, dropped: boolean): void {
    const g = this.g;
    const width = vp.x2 - vp.x1 + 1;
    const height = vp.y2 - vp.y1 + 1;
    const menuwidth = Math.trunc(width / this.subs.length);
    windowBox(g, vp.x1, vp.y1, vp.x2, vp.y2, MENUBACK, this.hilite());
    const hx1 = vp.x1 + this.current * menuwidth;
    const hx2 = vp.x1 + (this.current + 1) * menuwidth;
    if (isModern(g)) {
      g.setColorCss(MODERN.accent);
      g.rectangle(FILL, hx1, vp.y1, hx2, vp.y2);
    } else {
      g.rectangle(dropped ? BORDER : FILL, hx1, vp.y1, hx2, vp.y2);
    }
    g.setTextAlign(HAlign.LEFT, VAlign.TOP);
    g.setCharSize(height - 4, Math.trunc(menuwidth / 12));
    g.setCharSpacing(-1);
    let pos = vp.x1;
    this.subs.forEach((sub, i) => {
      if (isModern(g)) g.setColorCss(i === this.current ? MODERN.accentText : MODERN.text);
      else g.setColor(MENUTEXT);
      g.grText(pos, vp.y1 + 1, sub.name);
      pos += menuwidth;
    });
  }

  /** MenuDisplaySub: uses the text size the bar set. */
  private displaySub(sub: SubMenu, vp: Viewport): void {
    const g = this.g;
    const hilite = this.hilite();
    const height = vp.y2 - vp.y1 + 1;
    const itemheight = Math.trunc(height / sub.items.length);
    windowBox(g, vp.x1, vp.y1, vp.x2, vp.y2, MENUBACK, hilite);
    g.setTextAlign(HAlign.LEFT, VAlign.TOP);
    if (isModern(g)) g.setColorCss(MODERN.accent);
    g.rectangle(FILL, vp.x1, vp.y1 + itemheight * sub.current, vp.x2, vp.y1 + itemheight * (sub.current + 1));
    let pos = vp.y1;
    sub.items.forEach((item, i) => {
      if (isModern(g)) {
        const current = i === sub.current;
        g.setColorCss(item.enabled ? (current ? MODERN.accentText : MODERN.text) : MODERN.dim);
      } else {
        g.setColor(item.enabled ? MENUTEXT : hilite);
      }
      g.grText(vp.x1 + 2, pos, item.name);
      pos += itemheight;
    });
  }

  /** MenuDisplay: the saved screen, then the bar and the open menu. */
  display(): void {
    this.background.put();
    this.g.setViewport(0, 0, 639, 349);
    this.displayBar(this.barViewport(), true);
    this.displaySub(this.sub, this.subViewport());
  }

  // ---- The pointer ----

  /** GetMouseMenuSub: the menu name under x. */
  private subAt(x: number): number {
    if (x < this.vp.x1 || x > this.vp.x2) return -1;
    const width = this.vp.x2 - this.vp.x1 + 1;
    return Math.trunc(((x - this.vp.x1) * this.subs.length) / width);
  }

  /** The open menu's item at (x, y), -2 on the bar, -1 elsewhere. */
  private itemAt(x: number, y: number): number {
    if (y <= this.vp.y1 + ROW) return y >= this.vp.y1 ? -2 : -1;
    const sv = this.subViewport();
    if (x < sv.x1 || x > sv.x2 || y > sv.y2) return -1;
    return Math.min(this.sub.items.length - 1, Math.trunc((y - (this.vp.y1 + ROW)) / ROW));
  }

  /**
   * MenuCall: shows the menu and handles keys and the pointer until an item
   * is selected or the menu is closed.
   */
  async call(input: Input, help: () => Promise<void>, useMouse: boolean): Promise<void> {
    if (this.current >= this.subs.length) this.current = 0;
    if (useMouse) input.getMouseInput(); // the original read (and dropped) any waiting button
    this.display();
    for (;;) {
      let changed = false;
      if (useMouse) {
        const m = input.getMouseInput();
        if (m.button === RMB) return;
        if (m.button === LMB || (m.button === NMB && m.moved && m.pointerType !== 'touch')) {
          const item = this.itemAt(m.x, m.y);
          if (item === -2) {
            const sub = this.subAt(m.x);
            if (sub >= 0 && sub !== this.current) {
              this.current = sub;
              changed = true;
            }
          } else if (item >= 0) {
            if (item !== this.sub.current && this.sub.items[item].enabled) {
              this.sub.current = item;
              changed = true;
            }
            if (m.button === LMB && this.sub.items[item].enabled) {
              await this.select();
              return;
            }
          } else if (m.button === LMB) return; // a click away from the menu closes it
        }
      }
      if (input.kbhit()) {
        const key = await input.getKey();
        const r = await this.ks.key(key, null);
        if (r.used) {
          if (r.index === 4 || r.index === 5) return; // selected, or cancelled
          changed ||= r.funcret !== 0;
        } else if (key === 'm'.charCodeAt(0) || key === ESC) return;
        else if (key === F1 || key === 'h'.charCodeAt(0)) {
          await help();
          changed = true;
        }
      }
      if (changed) this.display();
      else if (!input.kbhit()) await input.wait();
    }
  }
}
