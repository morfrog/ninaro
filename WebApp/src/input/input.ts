// Input as the original polled it: kbhit/GetKey for keys, and GetMouseInput
// for mouse button releases (LMB, then RMB) or, with no release waiting, the
// mouse position (NMB). Instead of spinning in a polling loop, code awaits
// wait(), which resolves when anything new arrives.

import { legacyKey } from './keys';

/** MOUSEIN.H */
export const NMB = 0;
export const LMB = 1;
export const RMB = 2;

export interface MouseInput {
  button: typeof NMB | typeof LMB | typeof RMB;
  /** Screen position in EGA pixels; -1 if the pointer has never been seen. */
  x: number;
  y: number;
  /** "touch" for a finger, so a tap can both move and drop. */
  pointerType: string;
}

/** A finger moved this far (EGA pixels) from where it went down is a drag, not a tap. */
export const TAP_SLOP = 8;

/** A finger that is down: where it went down, where it is now, and whether it has been dragged. */
export interface TouchDrag {
  startX: number;
  startY: number;
  x: number;
  y: number;
  dragging: boolean;
  /** Whether this press is tracked as a drag at all (the game decides by where it went down). */
  tracked: boolean;
  /** Set by the game once the finger has dropped its token, so the same press does not drop another. */
  spent: boolean;
}

export class Input {
  private keys: number[] = [];
  private touch: TouchDrag | null = null;
  /** Whether a press that went down at (x, y) is tracked as a drag. The game sets this. */
  trackTouch: (x: number, y: number) => boolean = () => true;
  private lmb = false;
  private rmb = false;
  private x = -1;
  private y = -1;
  private moved = false;
  private pointerType = 'mouse';
  private waiters: (() => void)[] = [];
  /** Lines to scroll, from the mouse wheel or a vertical drag (positive is down). */
  private wheelLines = 0;

  /** Adds a key as if typed (for buttons on the page, and tests). */
  pushKey(key: number): void {
    this.keys.push(key);
    this.wake();
  }

  /** A mouse button released at (x, y), in EGA pixels. */
  pushButton(button: typeof LMB | typeof RMB, x: number, y: number, pointerType = 'mouse'): void {
    if (button === LMB) this.lmb = true;
    else this.rmb = true;
    this.x = x;
    this.y = y;
    this.pointerType = pointerType;
    this.wake();
  }

  /** The pointer moved to (x, y), in EGA pixels. */
  pushMove(x: number, y: number, pointerType = 'mouse'): void {
    this.x = x;
    this.y = y;
    this.moved = true;
    this.pointerType = pointerType;
    this.wake();
  }

  /** A finger went down on the page at (x, y), in EGA pixels. */
  touchDown(x: number, y: number): void {
    this.touch = { startX: x, startY: y, x, y, dragging: false, tracked: this.trackTouch(x, y), spent: false };
    this.wake();
  }

  /** The finger moved to (x, y). Past TAP_SLOP from where it went down, a tracked press is a drag. */
  touchMove(x: number, y: number): void {
    const t = this.touch;
    if (!t) return;
    t.x = x;
    t.y = y;
    if (Math.hypot(x - t.startX, y - t.startY) > TAP_SLOP) t.dragging = true;
    this.wake();
  }

  /** The finger lifted. Returns true if it had been dragged (so it is not a tap). */
  touchUp(): boolean {
    const t = this.touch;
    this.touch = null;
    return t?.dragging ?? false;
  }

  /** The finger that is down, or null. */
  touchDrag(): TouchDrag | null {
    return this.touch;
  }

  /** The finger has dropped its token: the same press drops no more. */
  spendTouch(): void {
    if (this.touch) this.touch.spent = true;
  }

  /** Forgets a click released so far and any press in progress, so neither acts on a turn that has only just begun. */
  forgetPresses(): void {
    this.lmb = false;
    this.spendTouch();
  }

  /** Scroll by this many lines (down if positive). */
  pushWheel(lines: number): void {
    this.wheelLines += lines;
    this.wake();
  }

  /** The lines scrolled since last asked. */
  takeWheel(): number {
    const lines = this.wheelLines;
    this.wheelLines = 0;
    return lines;
  }

  /** Wakes anything awaiting wait(), with no input: something else (the online game) changed. */
  poke(): void {
    this.wake();
  }

  private wake(): void {
    const waiters = this.waiters;
    this.waiters = [];
    for (const w of waiters) w();
  }

  /** kbhit */
  kbhit(): boolean {
    return this.keys.length > 0;
  }

  /** GetKey: the next key, waiting for one. Mouse input is left waiting. */
  async getKey(): Promise<number> {
    while (this.keys.length === 0) await this.wait();
    return this.keys.shift()!;
  }

  /**
   * GetMouseInput: a waiting left release, else a right release, else NMB
   * with the position. `moved` says whether the pointer moved since the
   * last NMB report (the original reported the position every time).
   */
  getMouseInput(): MouseInput & { moved: boolean } {
    if (this.lmb) {
      this.lmb = false;
      return { button: LMB, x: this.x, y: this.y, pointerType: this.pointerType, moved: false };
    }
    if (this.rmb) {
      this.rmb = false;
      return { button: RMB, x: this.x, y: this.y, pointerType: this.pointerType, moved: false };
    }
    const moved = this.moved;
    this.moved = false;
    return { button: NMB, x: this.x, y: this.y, pointerType: this.pointerType, moved };
  }

  /** Forgets waiting keys and buttons (as the original's ClearBuffer did for keys). */
  clear(): void {
    this.keys = [];
    this.lmb = false;
    this.rmb = false;
    this.moved = false;
    this.wheelLines = 0;
  }

  /** Resolves when the next key, button or movement arrives. */
  wait(): Promise<void> {
    return new Promise((resolve) => this.waiters.push(resolve));
  }

  /**
   * Listens to the page: keys anywhere, pointer on the canvas. toScreen
   * converts a pointer position to EGA pixels. Keys the game uses don't
   * also scroll the page or open the browser's own help and find.
   */
  attach(target: Window, canvas: HTMLCanvasElement, toScreen: (clientX: number, clientY: number) => { x: number; y: number }): void {
    target.addEventListener('keydown', (e) => {
      if (e.target instanceof HTMLInputElement || e.target instanceof HTMLTextAreaElement) return;
      const key = legacyKey(e);
      if (key === null) return;
      // Leave the browser's own shortcuts (copy, reload with Ctrl, ...) alone.
      if ((e.ctrlKey || e.metaKey) && !['x', 'q', 'n', 'e', 'X', 'Q', 'N', 'E'].includes(e.key) && !/^F\d+$/.test(e.key)) return;
      e.preventDefault();
      this.pushKey(key);
    });
    // A finger dragged up or down scrolls (help) instead of tapping.
    let dragY: number | null = null;
    let dragged = false;
    canvas.addEventListener('pointerdown', (e) => {
      const p = toScreen(e.clientX, e.clientY);
      dragY = p.y;
      dragged = false;
      if (e.pointerType === 'touch') this.touchDown(p.x, p.y);
    });
    canvas.addEventListener('pointermove', (e) => {
      const p = toScreen(e.clientX, e.clientY);
      if (e.pointerType === 'touch') this.touchMove(p.x, p.y);
      if (dragY !== null && e.pointerType === 'touch') {
        const lines = Math.trunc((dragY - p.y) / 12);
        if (lines !== 0) {
          dragged = true;
          dragY -= lines * 12;
          this.pushWheel(lines);
        }
      }
      this.pushMove(p.x, p.y, e.pointerType);
    });
    canvas.addEventListener('pointerup', (e) => {
      dragY = null;
      const touchDragged = e.pointerType === 'touch' ? this.touchUp() : false;
      if (e.button !== 0 || dragged || touchDragged) return;
      const p = toScreen(e.clientX, e.clientY);
      this.pushButton(LMB, p.x, p.y, e.pointerType);
    });
    canvas.addEventListener(
      'wheel',
      (e) => {
        e.preventDefault();
        this.pushWheel(e.deltaY > 0 ? 3 : -3);
      },
      { passive: false },
    );
    canvas.addEventListener('contextmenu', (e) => {
      e.preventDefault();
      const p = toScreen(e.clientX, e.clientY);
      this.pushButton(RMB, p.x, p.y, 'mouse');
    });
  }
}

/** delay(ms) */
export function sleep(ms: number): Promise<void> {
  return new Promise((resolve) => setTimeout(resolve, Math.max(0, ms)));
}
