// The window system (Legacy VIEWSYS.C): the screen divided into windows,
// each with a drawing function, a mouse function and a one-line hint.
// Windows are drawn in the order they were added.

import type { Graph } from './graph';
import { MemPage } from './memPage';
import { pointInsideViewport, setViewport, viewportFromViewport, type Viewport, type ViewWin } from './screen';

/** Legacy display modes. */
export const Display = { NoDisplay: 0, DisplayAll: 1, DisplayChanged: 2, DisplayCurrent: 3 } as const;
export type Display = (typeof Display)[keyof typeof Display];

export interface ViewWindow {
  active: boolean;
  changed: boolean;
  vp: Viewport;
  display: (() => void) | null;
  /** x, y relative to the window, and its width and height */
  mouse: ((x: number, y: number, width: number, height: number) => void) | null;
  help: string;
}

export class ViewSys {
  readonly main: Viewport;
  readonly windows: ViewWindow[] = [];
  readonly current: MemPage;
  private changedCount = 0;

  /** ViewSysNew */
  constructor(
    private readonly g: Graph,
    canvas: HTMLCanvasElement,
    main: Viewport,
    private readonly maxWindows: number,
    private readonly background: (vp: Viewport) => void,
  ) {
    this.main = { ...main };
    this.current = new MemPage(canvas);
  }

  /** ViewSysWindowNew: window placed by fractions of the main viewport. */
  windowNew(
    x1: number,
    y1: number,
    x2: number,
    y2: number,
    active: boolean,
    display: (() => void) | null,
    mouse: ViewWindow['mouse'],
    help: string,
  ): number {
    if (this.windows.length >= this.maxWindows - 1) throw new Error('Exceeded Max Windows!!');
    const vw: ViewWin = { x1, y1, x2, y2 };
    this.windows.push({ active, changed: false, vp: viewportFromViewport(vw, this.main), display, mouse, help });
    return this.windows.length - 1;
  }

  /**
   * ViewSysDisplay. DisplayAll clears and draws every active window;
   * DisplayChanged restores the saved screen and redraws changed windows;
   * DisplayCurrent restores the saved screen. Returns false for NoDisplay.
   */
  display(mode: Display): boolean {
    switch (mode) {
      case Display.DisplayAll:
        this.background(this.main);
        for (const w of this.windows) {
          if (w.active && w.display) {
            setViewport(this.g, w.vp);
            w.display();
            w.changed = false;
          }
        }
        this.changedCount = 0;
        this.current.get();
        return true;
      case Display.DisplayChanged:
        if (!this.current.valid) return this.display(Display.DisplayAll);
        this.current.put();
        if (this.changedCount > 0) {
          for (const w of this.windows) {
            if (w.changed && w.active && w.display) {
              this.background(w.vp);
              setViewport(this.g, w.vp);
              w.display();
              w.changed = false;
            }
          }
          this.current.get();
          this.changedCount = 0;
        }
        return true;
      case Display.DisplayCurrent:
        if (!this.current.valid) return this.display(Display.DisplayAll);
        this.current.put();
        return true;
      default:
        return false;
    }
  }

  /** ViewSysWindowChanged */
  windowChanged(window: number, changed: boolean): void {
    const w = this.windows[window];
    if (w.changed && !changed) this.changedCount -= 1;
    else if (!w.changed && changed) this.changedCount += 1;
    w.changed = changed;
  }

  /** ViewSysWindowActivate */
  windowActivate(window: number, active: boolean): void {
    this.windows[window].active = active;
  }

  /** ViewSysWindowHelp */
  windowHelp(window: number): string {
    return this.windows[window].help;
  }

  /** The first active window at screen (x, y), or -1. */
  windowAt(x: number, y: number): number {
    return this.windows.findIndex((w) => w.active && pointInsideViewport(x, y, w.vp));
  }

  /**
   * ViewSysMouse: sends (x, y) to the first active window there. False if
   * none; null (the C's UNKNOWN) if that window has no mouse function.
   */
  mouse(x: number, y: number): boolean | null {
    const n = this.windowAt(x, y);
    if (n < 0) return false;
    const w = this.windows[n];
    if (!w.mouse) return null;
    w.mouse(x - w.vp.x1, y - w.vp.y1, w.vp.x2 - w.vp.x1 + 1, w.vp.y2 - w.vp.y1 + 1);
    return true;
  }
}
