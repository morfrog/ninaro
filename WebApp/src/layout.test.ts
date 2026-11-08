import { describe, expect, it } from 'vitest';
import { ENTRY_GRID, PLAY_GRID, SCREEN, viewportFromViewport } from './layout';

describe('viewportFromViewport', () => {
  it('places the play grid as the original did', () => {
    // 639 * 0.6 = 383.4 and 349 * 0.09 = 31.41, truncated.
    expect(viewportFromViewport(PLAY_GRID, SCREEN)).toEqual({ x1: 0, y1: 31, x2: 383, y2: 349 });
  });

  it('places the entry grid as the original did', () => {
    // 349 * 0.08 = 27.92, truncated.
    expect(viewportFromViewport(ENTRY_GRID, SCREEN)).toEqual({ x1: 0, y1: 0, x2: 383, y2: 27 });
  });

  it('offsets by the parent viewport', () => {
    const parent = { x1: 50, y1: 50, x2: 590, y2: 300 };
    expect(viewportFromViewport({ x1: 0.5, y1: 0.5, x2: 1, y2: 1 }, parent)).toEqual({
      x1: 320,
      y1: 175,
      x2: 590,
      y2: 300,
    });
  });
});
