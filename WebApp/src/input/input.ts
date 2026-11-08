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

export class Input {
  private keys: number[] = [];
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
      dragY = toScreen(e.clientX, e.clientY).y;
      dragged = false;
    });
    canvas.addEventListener('pointermove', (e) => {
      const p = toScreen(e.clientX, e.clientY);
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
      if (e.button !== 0 || dragged) return;
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
