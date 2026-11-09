// Dialogs (Legacy DIALOG.C): a stack of controls, each a list or a range
// shown as radio buttons, colour swatches, a slider, names or buttons, with
// OK / Reset / Cancel buttons added at the bottom. Changes only count if OK
// is chosen and something changed; Reset puts back the values the dialog
// opened with.
//
// Keys are the current player's: left and right change the current
// control's value, up and down move between controls, select presses the
// highlighted button, cancel cancels; O, R, C and Esc press OK, Reset and
// Cancel; F1 or H shows help. The original's hidden mouse changed controls
// and values as it moved and pressed the highlighted button on a click.
// Here, with a visible pointer, clicking a control's item chooses it,
// clicking a button presses it, and a right click cancels.

import type { Input } from '../input/input';
import { LMB, RMB } from '../input/input';
import { ESC, F1, KeySys, type KeyGroup } from '../input/keys';
import { BORDER, FILL, HAlign, VAlign, type Graph } from '../render/graph';
import type { MemPage } from '../render/memPage';
import { Color } from '../render/palette';
import { type Viewport } from '../render/screen';
import { inkSelected, inkText, panelFill, radioMark, raised, selectionFrame, trackLine, MODERN, isModern } from '../render/chrome';

/** Display types (DIALOG.H). */
export const Show = { Button: 0, Radio: 1, Color: 2, Name: 3, Slider: 4, Dial: 5 } as const;
export type Show = (typeof Show)[keyof typeof Show];

export const CONTROLTEXT = Color.BLACK;
export const CONTROLBACK = Color.LTGREY;

/** DialogSelect's result for Reset (the C's UNKNOWN). */
const RESET = -1;

export interface ListElem {
  value: number;
  /** null ends the list early, as the C's NULL name did. */
  name: string | null;
}

/** A CONTROL: a list of named values, or a range of numbers. */
export class Control {
  /** List: index of the current element. Range: the current number. */
  current: number;

  private constructor(
    readonly kind: 'list' | 'range',
    public show: Show,
    public rows: number,
    public cols: number,
    public strwidth: number,
    readonly elems: ListElem[],
    public min: number,
    public minName: string,
    public max: number,
    public maxName: string,
  ) {
    this.current = kind === 'range' ? min : 0;
  }

  /** ControlNew(List, ...) */
  static list(show: Show, rows: number, cols: number, strwidth: number, elems: ListElem[]): Control {
    return new Control('list', show, rows, cols, strwidth, elems, 0, '', 0, '');
  }

  /** ControlNew(Range, ...) */
  static range(show: Show, rows: number, cols: number, strwidth: number, min: number, minName: string, max: number, maxName: string): Control {
    return new Control('range', show, rows, cols, strwidth, [], min, minName, max, maxName);
  }

  /** ControlReset for a range: new limits (the current value is left alone). */
  resetRange(min: number, minName: string, max: number, maxName: string): void {
    this.min = min;
    this.minName = minName;
    this.max = max;
    this.maxName = maxName;
  }

  /** ControlValueGet: a list element's value, or a range's min (0) or max (1). */
  valueGet(pos: number): number | null {
    if (this.kind === 'list') return pos >= 0 && pos < this.elems.length ? this.elems[pos].value : null;
    return pos === 0 ? this.min : pos === 1 ? this.max : null;
  }

  /** ControlMove: by howfar, if that stays in range (and, for lists, on a named element). */
  move(howfar: number): boolean {
    if (this.kind === 'range') {
      const to = this.current + howfar;
      if (to < this.min || to > this.max) return false;
      this.current = to;
      return true;
    }
    const to = this.current + howfar;
    if (to < 0 || to > this.elems.length - 1) return false;
    if (this.elems[to].name === null) return false;
    this.current = to;
    return true;
  }

  prev(): boolean {
    return this.move(-1);
  }

  next(): boolean {
    return this.move(1);
  }

  /** ControlCurrentSet: by index (list) or number (range), if in range. */
  currentSet(current: number): boolean {
    const [lo, hi] = this.kind === 'list' ? [0, this.elems.length - 1] : [this.min, this.max];
    if (current < lo || current > hi) return false;
    this.current = current;
    return true;
  }

  /** ControlCurrentSetTo: a list's first element with this value, or a range's number. */
  currentSetTo(value: number): boolean {
    if (this.kind === 'list') {
      const x = this.elems.findIndex((e) => e.value === value);
      if (x < 0) return false;
      this.current = x;
      return true;
    }
    return this.currentSet(value);
  }

  /** ControlTransCurrentGet: the position within the control. */
  position(): number {
    return this.kind === 'list' ? this.current : this.current - this.min;
  }

  /** ControlRange: how many positions there are. */
  range(): number {
    return this.kind === 'list' ? this.elems.length : this.max - this.min + 1;
  }

  /** ControlElemSet: sets a list element; for a range, renames (and moves) a limit. */
  elemSet(elem: number, value: number, name: string): void {
    if (this.kind === 'range') {
      if (elem === this.min) {
        this.minName = name;
        this.min = value;
      }
      if (elem === this.max) {
        this.maxName = name;
        this.max = value;
      }
    } else this.elems[elem] = { value, name };
  }

  /** Moves to a position (list index, or offset from a range's min). */
  positionSet(pos: number): boolean {
    return this.currentSet(this.kind === 'range' ? this.min + pos : pos);
  }
}

export interface DialogElem {
  name: string;
  enabled: boolean;
  selectable: boolean;
  store: Control;
}

/** Where a control was drawn, for the pointer. */
interface Placed {
  index: number;
  vp: Viewport;
}

export class Dialog {
  current: number;
  readonly elems: DialogElem[];
  readonly ks: KeySys;
  private state: number[] = [];
  private placed: Placed[] = [];
  private frame: Viewport = { x1: 0, y1: 0, x2: 0, y2: 0 };

  /** DialogNew: the controls, then OK / Reset / Cancel buttons. */
  constructor(
    private readonly g: Graph,
    kg: KeyGroup,
    readonly change: ((d: Dialog, last: number) => void) | null,
    public name: string,
    private readonly hilite: () => number,
    elems: DialogElem[],
  ) {
    const buttons = Control.list(Show.Button, 1, 3, 6, [
      { value: 0, name: 'OK' },
      { value: 1, name: 'Reset' },
      { value: 2, name: 'Cancel' },
    ]);
    this.elems = [...elems, { name: '', enabled: true, selectable: true, store: buttons }];
    this.current = this.elems.length - 1;
    this.ks = new KeySys(kg);
    this.ks.iFunc(0, () => (this.controlPrev() ? 1 : 0));
    this.ks.iFunc(1, () => (this.controlNext() ? 1 : 0));
    this.ks.iFunc(2, () => (this.prev() ? 1 : 0));
    this.ks.iFunc(3, () => (this.next() ? 1 : 0));
    this.ks.iFunc(4, () => this.select());
    this.ks.iFunc(5, () => this.cancel());
  }

  /** DialogControlGet */
  control(n: number): Control {
    return this.elems[n].store;
  }

  /** DialogMove: skips controls that are disabled or unselectable. */
  move(howfar: number): boolean {
    if (howfar === 0) return false;
    const to = this.current + howfar;
    if (to < 0 || to > this.elems.length - 1) return false;
    const e = this.elems[to];
    if (e.selectable && e.enabled) {
      this.current = to;
      return true;
    }
    return this.move(howfar > 0 ? howfar + 1 : howfar - 1);
  }

  prev(): boolean {
    return this.move(-1);
  }

  next(): boolean {
    return this.move(1);
  }

  controlNext(): boolean {
    return this.control(this.current).next();
  }

  controlPrev(): boolean {
    return this.control(this.current).prev();
  }

  /** DialogCurrentSet (the C returned TRUE whenever current was in range). */
  currentSet(current: number): boolean {
    if (current < 0 || current > this.elems.length - 1) return false;
    if (this.elems[current].selectable && this.elems[current].enabled) this.current = current;
    return true;
  }

  /** DialogDefaultGet: which of OK, Reset, Cancel is highlighted. */
  defaultGet(): number {
    return this.control(this.elems.length - 1).current;
  }

  /** DialogDefaultSet */
  defaultSet(select: number): void {
    this.control(this.elems.length - 1).currentSet(select);
  }

  /** DialogStateGet: every control's current. */
  stateGet(): number[] {
    return this.elems.map((e) => e.store.current);
  }

  /** DialogStateSet */
  stateSet(state: readonly number[]): boolean {
    let ok = true;
    this.elems.forEach((e, x) => {
      if (!e.store.currentSet(state[x])) ok = false;
    });
    return ok;
  }

  /** DialogControlEnable */
  controlEnable(control: number, enable: boolean): void {
    this.elems[control].enabled = enable;
  }

  /** DialogCancel: back to the opening values. */
  cancel(): number {
    this.stateSet(this.state);
    return 0;
  }

  /** DialogSelect: OK 1, Reset -1 (values put back), Cancel 0. */
  select(): number {
    switch (this.defaultGet()) {
      case 0:
        return 1;
      case 1:
        this.stateSet(this.state);
        this.change?.(this, 1);
        return RESET;
      case 2:
        return this.cancel();
    }
    return 1;
  }

  // ---- Drawing ----

  /** DialogDisplay: the saved screen, then the dialog in vp. */
  display(vp: Viewport, mp: MemPage, namefract: number, chars: number): void {
    const g = this.g;
    const hilite = this.hilite();
    let totalrows = 1;
    for (const e of this.elems) totalrows += e.enabled ? e.store.rows : 0;
    const height = vp.y2 - vp.y1 + 1;
    const width = vp.x2 - vp.x1 + 1;
    const rowheight = Math.trunc(height / totalrows);
    const names = Math.trunc(width / namefract);
    mp.put();
    g.setViewport(0, 0, 639, 349);
    panelFill(g, vp.x1, vp.y1, vp.x2, vp.y2, CONTROLBACK);
    g.setCharSize(Math.trunc((4 * rowheight) / 5), Math.trunc(width / 20));
    g.setCharSpacing(0);
    g.setTextAlign(HAlign.CENTER, VAlign.HALF);
    inkText(g, CONTROLTEXT);
    g.grText(vp.x1 + Math.trunc(width / 2), vp.y1 + Math.trunc(rowheight / 2), this.name);
    g.grText(vp.x1 + Math.trunc(width / 2) + 1, vp.y1 + Math.trunc(rowheight / 2), this.name);
    g.setCharSize(Math.trunc(rowheight / 2), Math.trunc(names / chars));
    g.setTextAlign(HAlign.LEFT, VAlign.TOP);
    let aty = vp.y1 + rowheight;
    for (const e of this.elems) {
      if (!e.enabled) continue;
      g.grText(vp.x1 + 2, aty + 2, e.name);
      aty += e.store.rows * rowheight;
    }
    this.frame = vp;
    this.placed = [];
    aty = vp.y1 + rowheight;
    this.elems.forEach((e, x) => {
      if (!e.enabled) return;
      const control: Viewport = { x1: vp.x1 + 1, y1: aty + 1, x2: vp.x2 - 1, y2: aty + e.store.rows * rowheight - 1 };
      if (x === this.current) {
        selectionFrame(g, vp.x1, control.y1, vp.x2, control.y2, hilite);
      } else {
        g.setLineStyle(0x1111);
        g.setColor(CONTROLTEXT);
        g.rectangle(BORDER, vp.x1, control.y1, vp.x2, control.y2);
        g.setLineStyle(0xffff);
      }
      control.x1 = e.name === '' ? vp.x1 : vp.x1 + names;
      control.y1 += 1;
      control.y2 -= 1;
      controlDisplay(g, e.store, control, hilite);
      this.placed.push({ index: x, vp: { ...control } });
      aty += e.store.rows * rowheight;
    });
  }

  /** The control and position under (x, y), from the last display. */
  hit(x: number, y: number): { control: number; pos: number } | null {
    for (const p of this.placed) {
      if (x < p.vp.x1 || x > p.vp.x2 || y < p.vp.y1 || y > p.vp.y2) continue;
      const e = this.elems[p.index];
      if (!e.selectable) return null;
      const pos = positionAt(e.store, p.vp, x, y);
      return pos === null ? null : { control: p.index, pos };
    }
    return null;
  }

  inside(x: number, y: number): boolean {
    const f = this.frame;
    return x >= f.x1 && x <= f.x2 && y >= f.y1 && y <= f.y2;
  }

  // ---- Running ----

  /**
   * DialogCall: true if OK was pressed and something changed; false if it
   * was cancelled (or OK'd with nothing changed).
   */
  async call(
    vp: Viewport,
    mp: MemPage,
    namefract: number,
    chars: number,
    help: () => Promise<void>,
    input: Input,
    useMouse: boolean,
  ): Promise<boolean> {
    this.currentSet(this.elems.length - 1);
    this.state = this.stateGet();
    this.display(vp, mp, namefract, chars);
    for (;;) {
      let used = false;
      let index = -1;
      let funcret = 0;
      if (useMouse) {
        const m = input.getMouseInput();
        if (m.button === RMB) {
          used = true;
          index = 5;
          funcret = this.cancel();
        } else if (m.button === LMB) {
          const hit = this.hit(m.x, m.y);
          if (hit && hit.control === this.elems.length - 1) {
            // A button: press it.
            this.current = hit.control;
            this.defaultSet(hit.pos);
            used = true;
            index = 4;
            funcret = this.select();
          } else if (hit) {
            const c = this.control(hit.control);
            const moved = hit.control !== this.current;
            this.current = hit.control;
            if (c.position() !== hit.pos) {
              c.positionSet(hit.pos);
              used = true;
              index = 0; // a change, as the C's mouse path reported it
              funcret = 1;
            } else if (moved) {
              used = true;
              index = 2;
              funcret = 1;
            }
          }
        }
      }
      if (!used && input.kbhit()) {
        const key = await input.getKey();
        const r = await this.ks.key(key, null);
        used = r.used;
        index = r.index;
        funcret = r.funcret;
        if (!used) {
          const lower = String.fromCharCode(key).toLowerCase();
          if (lower === 'o' || lower === 'r' || lower === 'c' || key === ESC) {
            this.defaultSet(lower === 'o' ? 0 : lower === 'r' ? 1 : 2);
            used = true;
            index = 4;
            funcret = this.select();
          } else if (lower === 'h' || key === F1) {
            await help();
            used = true;
            index = 6;
            funcret = 1;
          }
        }
      }
      if (used) {
        switch (index) {
          case 0:
          case 1:
            if (funcret) this.change?.(this, index);
            break;
          case 4:
            if (funcret === 1) {
              const now = this.stateGet();
              if (now.some((v, i) => v !== this.state[i])) return true;
              return false; // OK with nothing changed counts as cancel
            }
            if (funcret === RESET) break;
            return false;
          case 5:
            return false;
        }
        if (funcret) this.display(vp, mp, namefract, chars);
      } else if (!input.kbhit()) await input.wait();
    }
  }
}

// ---- Drawing controls (DisplayList, DisplayRange and their helpers) ----

/** DisplayColor: a swatch, with a dotted double frame when selected. */
function displayColor(g: Graph, x1: number, y1: number, x2: number, y2: number, hilite: number, color: number, selected: boolean): void {
  if (selected && isModern(g)) {
    selectionFrame(g, x1, y1, x2, y2, hilite);
  } else if (selected) {
    g.setColor(hilite);
    g.setLineStyle(0x5555);
    g.rectangle(BORDER, x1, y1, x2, y2);
    g.rectangle(BORDER, x1 + 2, y1 + 2, x2 - 2, y2 - 2);
    g.setLineStyle(0xffff);
    g.setColor(CONTROLTEXT);
    g.rectangle(BORDER, x1 + 1, y1 + 1, x2 - 1, y2 - 1);
    g.rectangle(BORDER, x1 + 3, y1 + 3, x2 - 3, y2 - 3);
  }
  g.setColor(color);
  g.rectangle(FILL, x1 + 4, y1 + 4, x2 - 4, y2 - 4);
}

/** The x of each slider tick and the left and section DisplayScale used. */
function scaleGeometry(vp: Viewport, range: number) {
  const width = vp.x2 - vp.x1 + 1;
  const edge = Math.trunc(width / 10);
  const left = vp.x1 + edge;
  const section = width - edge - edge;
  const ticks = range === 1 ? [left + Math.trunc(section / 2)] : Array.from({ length: range }, (_, x) => left + Math.trunc((x * section) / (range - 1)));
  return { width, edge, left, section, ticks };
}

/** DisplayScale: a slider with a tick for each position and a box showing the value. */
function displayScale(g: Graph, vp: Viewport, range: number, pos: number, value: string, chars: number, hilite: number): void {
  const height = vp.y2 - vp.y1 + 1;
  const { edge, left, ticks } = scaleGeometry(vp, range);
  g.setCharSize(Math.trunc(height / 2), Math.trunc((edge + edge) / chars));
  g.setTextAlign(HAlign.CENTER, VAlign.HALF);
  trackLine(g, CONTROLTEXT);
  g.moveTo(left, vp.y2);
  g.lineTo(vp.x2 - edge, vp.y2);
  g.moveTo(left, vp.y1);
  g.lineTo(vp.x2 - edge, vp.y1);
  for (const at of ticks) {
    g.moveTo(at, vp.y1);
    g.lineTo(at, vp.y2);
  }
  const at = range === 1 ? ticks[0] : ticks[pos];
  raised(g, at - edge, vp.y1 + Math.trunc(height / 4), at + edge, vp.y2 - Math.trunc(height / 4), CONTROLBACK, hilite);
  inkText(g, CONTROLTEXT);
  g.grText(at, vp.y1 + Math.trunc(height / 2), value);
}

/** The part of a range's viewport its scale or swatches use (above its names, if it has any). */
function rangeArea(c: Control, vp: Viewport): Viewport {
  const height = Math.trunc((vp.y2 - vp.y1) / c.rows);
  const names = c.minName !== '' || c.maxName !== '';
  return names ? { x1: vp.x1, y1: vp.y1, x2: vp.x2, y2: vp.y2 - height } : vp;
}

/** DisplayRange */
function displayRange(g: Graph, c: Control, vp: Viewport, hilite: number): void {
  const height = Math.trunc((vp.y2 - vp.y1) / c.rows);
  if (isModern(g)) inkText(g, CONTROLTEXT);
  g.setCharSize(height, Math.trunc((vp.x2 - vp.x1) / (4 * c.strwidth)));
  g.setCharSpacing(0);
  if (c.minName !== '' && c.maxName !== '') {
    g.setTextAlign(HAlign.LEFT, VAlign.TOP);
    g.grText(vp.x1, vp.y2 - height, c.minName);
    g.setTextAlign(HAlign.RIGHT, VAlign.TOP);
    g.grText(vp.x2, vp.y2 - height, c.maxName);
  } else if (c.minName !== '') {
    g.setTextAlign(HAlign.CENTER, VAlign.TOP);
    g.grText(vp.x1 + Math.trunc((vp.x2 - vp.x1) / 2), vp.y2 - height, c.minName);
  }
  const area = rangeArea(c, vp);
  if (c.show === Show.Slider) {
    displayScale(g, area, c.max - c.min + 1, c.current - c.min, String(c.current), 3, hilite);
  } else if (c.show === Show.Color) {
    const width = Math.trunc((area.x2 - area.x1 + 1) / (c.max - c.min + 1));
    let at = area.x1;
    for (let x = c.min; x <= c.max; x++) {
      displayColor(g, at, area.y1, at + width, area.y2, hilite, x, x === c.current);
      at += width;
    }
  }
}

/** The cells of a list: where each element is drawn. */
function listCells(c: Control, vp: Viewport): Viewport[] {
  const width = Math.trunc((vp.x2 - vp.x1 + 1) / c.cols);
  const height = Math.trunc((vp.y2 - vp.y1 + 1) / c.rows);
  const count = Math.min(c.elems.length, c.rows * c.cols);
  const cells: Viewport[] = [];
  let atx = vp.x1;
  let aty = vp.y1;
  let col = 1;
  for (let x = 0; x < count; x++) {
    if (c.elems[x].name === null) break;
    cells.push({ x1: atx, y1: aty, x2: atx + width, y2: aty + height });
    if (col === c.cols) {
      col = 1;
      aty += height;
      atx = vp.x1;
    } else {
      atx += width;
      ++col;
    }
  }
  return cells;
}

/** DisplayList */
function displayList(g: Graph, c: Control, vp: Viewport, hilite: number): void {
  if (c.show === Show.Slider) {
    displayScale(g, vp, c.elems.length, c.current, c.elems[c.current].name ?? '', c.strwidth, hilite);
    return;
  }
  const width = Math.trunc((vp.x2 - vp.x1 + 1) / c.cols);
  const height = Math.trunc((vp.y2 - vp.y1 + 1) / c.rows);
  const threeQuarters = Math.trunc((3 * height) / 4);
  const texty = Math.trunc(height / 2);
  const textx = Math.trunc(width / 2);
  switch (c.show) {
    case Show.Button:
    case Show.Name:
      g.setTextAlign(HAlign.CENTER, VAlign.HALF);
      g.setCharSize(Math.trunc(height / 2), Math.trunc(width / (c.strwidth + 1)));
      break;
    case Show.Color:
      g.setTextAlign(HAlign.CENTER, VAlign.TOP);
      g.setCharSize(Math.trunc(height / 5), Math.trunc(width / (c.strwidth + 1)));
      break;
    case Show.Radio:
      g.setTextAlign(HAlign.LEFT, VAlign.HALF);
      g.setCharSize(Math.trunc(height / 2), Math.trunc((7 * width) / (8 * (c.strwidth + 1))));
      break;
  }
  listCells(c, vp).forEach((cell, x) => {
    const { x1: atx, y1: aty } = cell;
    const elem = c.elems[x];
    const name = elem.name ?? '';
    switch (c.show) {
      case Show.Radio:
        radioMark(g, atx, aty, atx + Math.trunc(width / 8), aty + height, x === c.current, CONTROLTEXT);
        inkText(g, CONTROLTEXT);
        g.grText(atx + Math.trunc(width / 8), aty + texty, name);
        break;
      case Show.Button:
        if (isModern(g)) {
          if (x === c.current) {
            g.setColorCss(MODERN.accent);
            g.rectangle(FILL, atx + 3, aty + 3, atx + width - 3, aty + height - 3);
            inkSelected(g, hilite);
            g.setColorCss(MODERN.accentText);
          } else {
            raised(g, atx + 3, aty + 3, atx + width - 3, aty + height - 3, CONTROLBACK, hilite);
            inkText(g, CONTROLTEXT);
          }
        } else {
          if (x === c.current) {
            g.setColor(hilite);
            g.rectangle(FILL, atx + 5, aty + 5, atx + width - 5, aty + height - 5);
          }
          g.setColor(CONTROLTEXT);
          g.rectangle(BORDER, atx + 3, aty + 3, atx + width - 3, aty + height - 3);
        }
        g.grText(atx + textx, aty + texty, name);
        break;
      case Show.Name:
        if (x === c.current) inkSelected(g, hilite);
        else inkText(g, CONTROLTEXT);
        g.grText(atx + textx, aty + texty, name);
        break;
      case Show.Color:
        displayColor(g, atx, aty, atx + width, aty + threeQuarters, hilite, elem.value, x === c.current);
        inkText(g, CONTROLTEXT);
        g.grText(atx + textx, aty + threeQuarters, name);
        break;
    }
  });
}

/** ControlDisplay */
export function controlDisplay(g: Graph, c: Control, vp: Viewport, hilite: number): void {
  if (c.kind === 'range') displayRange(g, c, vp, hilite);
  else displayList(g, c, vp, hilite);
}

/** The position (as ControlTransCurrentGet counts) under (x, y) in a control drawn in vp. */
export function positionAt(c: Control, vp: Viewport, x: number, y: number): number | null {
  if (c.show === Show.Slider) {
    const area = c.kind === 'range' ? rangeArea(c, vp) : vp;
    const { ticks } = scaleGeometry(area, c.range());
    let best = 0;
    ticks.forEach((t, i) => {
      if (Math.abs(t - x) < Math.abs(ticks[best] - x)) best = i;
    });
    return best;
  }
  if (c.kind === 'range') {
    const area = rangeArea(c, vp);
    const width = Math.trunc((area.x2 - area.x1 + 1) / c.range());
    const pos = Math.trunc((x - area.x1) / Math.max(1, width));
    return pos >= 0 && pos < c.range() ? pos : null;
  }
  const cells = listCells(c, vp);
  const i = cells.findIndex((cell) => x >= cell.x1 && x < cell.x2 && y >= cell.y1 && y < cell.y2);
  return i >= 0 ? i : null;
}
