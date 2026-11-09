import { describe, expect, it } from 'vitest';
import { MODERN_PALETTE, Palette, NINARO_PALETTE, hexToValue, toCss } from './palette';

describe('palette', () => {
  it('converts a hex colour to the nearest 6-bit step of each channel', () => {
    expect(toCss(hexToValue('#000000'))).toBe('#000000');
    expect(toCss(hexToValue('#ffffff'))).toBe('#ffffff');
    // Within one 6-bit step (4 of 255) of the original on each channel.
    const back = toCss(hexToValue('#0078d4'));
    expect(Math.abs(parseInt(back.slice(3, 5), 16) - 0x78)).toBeLessThanOrEqual(4);
    expect(Math.abs(parseInt(back.slice(5, 7), 16) - 0xd4)).toBeLessThanOrEqual(4);
  });

  it('has sixteen Modern slots with a dark background and light text', () => {
    expect(MODERN_PALETTE).toHaveLength(16);
    expect(toCss(MODERN_PALETTE[0])).toBe(toCss(hexToValue('#1e1e1e')));
    expect(toCss(MODERN_PALETTE[7])).toBe(toCss(hexToValue('#e6e6e6')));
  });

  it('reset goes back to the base palette, and setBase applies a new one', () => {
    const p = new Palette(NINARO_PALETTE);
    p.remap(4, 0x3f3f3f);
    p.reset();
    expect(p.value(4)).toBe(NINARO_PALETTE[4]);
    p.setBase(MODERN_PALETTE);
    expect(p.value(0)).toBe(MODERN_PALETTE[0]);
    p.remap(0, 0);
    p.reset();
    expect(p.value(0)).toBe(MODERN_PALETTE[0]);
  });
});
