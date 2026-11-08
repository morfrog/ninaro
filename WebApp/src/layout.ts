// Screen layout from the original's VIEWSYS and SCREEN modules.

/** A pixel rectangle, corners inclusive (Legacy VIEWPORT). */
export interface Viewport {
  x1: number;
  y1: number;
  x2: number;
  y2: number;
}

/** A rectangle as fractions of a viewport (Legacy VIEWWIN). */
export interface ViewWin {
  x1: number;
  y1: number;
  x2: number;
  y2: number;
}

/** The whole EGA mode 16 screen (Legacy ViewSysInit(0,0,639,349,...)). */
export const SCREEN: Viewport = { x1: 0, y1: 0, x2: 639, y2: 349 };

/** Legacy ViewportFromViewport: the vw part of vp, truncated as C's (int) casts did. */
export function viewportFromViewport(vw: ViewWin, vp: Viewport): Viewport {
  const width = vp.x2 - vp.x1;
  const height = vp.y2 - vp.y1;
  return {
    x1: vp.x1 + Math.trunc(vw.x1 * width),
    y1: vp.y1 + Math.trunc(vw.y1 * height),
    x2: vp.x1 + Math.trunc(vw.x2 * width),
    y2: vp.y1 + Math.trunc(vw.y2 * height),
  };
}

/** Window 1 of Legacy ViewSysInit: the play grid. */
export const PLAY_GRID: ViewWin = { x1: 0.0, y1: 0.09, x2: 0.6, y2: 1.0 };

/** Window 0 of Legacy ViewSysInit: the entry grid above the board. */
export const ENTRY_GRID: ViewWin = { x1: 0.0, y1: 0.0, x2: 0.6, y2: 0.08 };
