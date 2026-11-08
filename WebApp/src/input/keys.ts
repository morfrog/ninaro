// Keys as Ninaro saw them (Legacy KEY.C/H): GetKey's codes are the ASCII
// value, or 256 + the scan code for extended keys (function keys, arrows,
// the keypad with NumLock off). Key groups hold a player's six keys, and a
// key system runs a function for each key of the current group.

export const BKSP = 8;
export const ENTER = 13;
export const ESC = 27;
export const SPACE = 32;
/** Ctrl+letter: CA = 1 .. CZ = 26 */
export const ctrl = (letter: string) => letter.toUpperCase().charCodeAt(0) - 64;
export const CQ = ctrl('Q');
export const CX = ctrl('X');
export const CN = ctrl('N');
export const CE = ctrl('E');

export const F1 = 256 + 59;
export const F2 = F1 + 1;
export const F3 = F1 + 2;
export const F4 = F1 + 3;
export const F5 = F1 + 4;
export const F6 = F1 + 5;
export const F7 = F1 + 6;
export const F8 = F1 + 7;
export const F9 = F1 + 8;
export const F10 = F1 + 9;
export const F11 = 256 + 133;
export const F12 = 256 + 134;
export const KP7 = 256 + 71;
export const KP8 = 256 + 72;
export const KP9 = 256 + 73;
export const KP4 = 256 + 75;
export const KP5 = 256 + 76;
export const KP6 = 256 + 77;
export const KP1 = 256 + 79;
export const KP2 = 256 + 80;
export const KP3 = 256 + 81;
export const KP0 = 256 + 82;
export const KPDEL = 256 + 83;
/** Shift, Ctrl and Alt + F1..F10 */
export const SF1 = 256 + 84;
export const CF1 = SF1 + 10;
export const AF1 = CF1 + 10;
export const CF9 = CF1 + 8;
export const AF4 = AF1 + 3;

/** The extended keys by KeyboardEvent.code, with NumLock on or off. */
const EXTENDED: Record<string, number> = {
  ArrowLeft: KP4,
  Numpad4: KP4,
  ArrowRight: KP6,
  Numpad6: KP6,
  ArrowUp: KP8,
  Numpad8: KP8,
  ArrowDown: KP2,
  Numpad2: KP2,
  Home: KP7,
  Numpad7: KP7,
  PageUp: KP9,
  Numpad9: KP9,
  End: KP1,
  Numpad1: KP1,
  PageDown: KP3,
  Numpad3: KP3,
  Insert: KP0,
  Numpad0: KP0,
  Delete: KPDEL,
  NumpadDecimal: KPDEL,
  Numpad5: KP5,
};

const NAMED: Record<string, number> = {
  Backspace: BKSP,
  Enter: ENTER,
  NumpadEnter: ENTER,
  Escape: ESC,
  Tab: 9,
};

/**
 * The GetKey code for a browser key press, or null for keys DOS programs
 * never saw (Shift alone and so on). The keypad's keys always act as the
 * DOS keypad did with NumLock off, which the manual asked players to set.
 */
export function legacyKey(e: Pick<KeyboardEvent, 'key' | 'code' | 'ctrlKey' | 'altKey' | 'shiftKey'>): number | null {
  const f = /^F(\d{1,2})$/.exec(e.key);
  if (f) {
    const n = Number(f[1]);
    if (n === 11) return F11;
    if (n === 12) return F12;
    if (n < 1 || n > 10) return null;
    if (e.altKey) return AF1 + n - 1;
    if (e.ctrlKey) return CF1 + n - 1;
    if (e.shiftKey) return SF1 + n - 1;
    return F1 + n - 1;
  }
  if (e.code in EXTENDED) return EXTENDED[e.code];
  if (e.code in NAMED) return NAMED[e.code];
  if (e.ctrlKey && /^[a-z]$/i.test(e.key)) return ctrl(e.key);
  if (e.key.length === 1 && !e.ctrlKey && !e.altKey) {
    const code = e.key.charCodeAt(0);
    return code < 128 ? code : null;
  }
  return null;
}

/** KEYGROUP: a set of keys in order (left, right, up, down, select, cancel). */
export class KeyGroup {
  readonly key: number[];

  /** KeyGroupNew */
  constructor(...keys: number[]) {
    this.key = keys;
  }

  get keys(): number {
    return this.key.length;
  }

  /** KeyGroupIKey: -1 if index is out of range. */
  iKey(index: number): number {
    return index >= this.key.length || index < 0 ? -1 : this.key[index];
  }

  /** KeyGroupKey: the key's index, or -1. */
  indexOf(key: number): number {
    return this.key.indexOf(key);
  }

  /** KeyGroupCopy: this group's keys become source's. */
  copy(source: KeyGroup): void {
    for (let x = 0; x < this.key.length; x++) this.key[x] = source.key[x];
  }
}

/** A key function: returns a display mode, maybe after a modal menu. */
export type KeyFunc = (object: unknown) => number | Promise<number>;

/** KEYSYS: a function for each key of a group, each of which can be disabled. */
export class KeySys {
  private readonly funcs: (KeyFunc | null)[];
  private readonly enabled = new Set<number>();

  /** KeySysNew */
  constructor(public kg: KeyGroup) {
    this.funcs = new Array<KeyFunc | null>(kg.keys).fill(null);
  }

  /** KeySysIFunc: sets (and enables) the function for the index'th key. */
  iFunc(index: number, func: KeyFunc): void {
    this.funcs[index] = func;
    this.enabled.add(index);
  }

  /** KeySysIEnable */
  iEnable(index: number, enable: boolean): void {
    if (enable) this.enabled.add(index);
    else this.enabled.delete(index);
  }

  /**
   * KeySysKey: runs the key's function if the key is in the group and
   * enabled. index is -1 if the key isn't in the group.
   */
  async key(key: number, object: unknown): Promise<{ used: boolean; funcret: number; index: number }> {
    const index = this.kg.indexOf(key);
    const func = index >= 0 ? this.funcs[index] : null;
    if (index >= 0 && this.enabled.has(index) && func) return { used: true, funcret: await func(object), index };
    return { used: false, funcret: 0, index };
  }
}
