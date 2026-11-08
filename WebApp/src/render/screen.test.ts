import { describe, expect, it } from 'vitest';
import { pointInsideViewport, viewportFromViewport } from './screen';
import { NINARO_PALETTE, Palette, toCss } from './palette';

const SCREEN = { x1: 0, y1: 0, x2: 639, y2: 349 };

describe('viewportFromViewport', () => {
  it('places the play grid as the original did', () => {
    // 639 * 0.6 = 383.4 and 349 * 0.09 = 31.41, truncated.
    expect(viewportFromViewport({ x1: 0, y1: 0.09, x2: 0.6, y2: 1 }, SCREEN)).toEqual({ x1: 0, y1: 31, x2: 383, y2: 349 });
  });

  it('places the message box as the original did', () => {
    expect(viewportFromViewport({ x1: 0.61, y1: 0.52, x2: 1, y2: 1 }, SCREEN)).toEqual({ x1: 389, y1: 181, x2: 639, y2: 349 });
  });

  it('offsets by the parent viewport', () => {
    const parent = { x1: 50, y1: 50, x2: 590, y2: 300 };
    expect(viewportFromViewport({ x1: 0.5, y1: 0.5, x2: 1, y2: 1 }, parent)).toEqual({ x1: 320, y1: 175, x2: 590, y2: 300 });
  });

  it('tests points against inclusive corners', () => {
    expect(pointInsideViewport(389, 181, { x1: 389, y1: 181, x2: 639, y2: 349 })).toBe(true);
    expect(pointInsideViewport(388, 181, { x1: 389, y1: 181, x2: 639, y2: 349 })).toBe(false);
  });
});

describe('palette', () => {
  it('shows 6-bit values as DOSBox does', () => {
    // Colours measured in the original's frames.
    const p = new Palette();
    expect([0, 1, 2, 3, 4, 5, 6, 7, 8].map((slot) => p.css(slot))).toEqual([
      '#000000', '#aa0000', '#fbfb51', '#0000ff', '#00aa00', '#aa00aa', '#aa5500', '#aaaaaa', '#00aaaa',
    ]);
  });

  it('remaps a slot', () => {
    const p = new Palette();
    p.remap(0, 0x3f3f3f);
    expect(p.css(0)).toBe('#ffffff');
    p.remapAll(NINARO_PALETTE);
    expect(p.css(0)).toBe('#000000');
    expect(toCss(0x020306)).toBe('#180c08');
  });
});
