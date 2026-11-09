import { describe, expect, it } from 'vitest';
import { EGA, NINARO_PALETTE } from './palette';
import { MONO_GREENS, MONO_LOOKS, isBackgroundLook, monoLookIndices, monoTilePixels, monoToneCss } from './displayStyle';

/** The palette as ApplyBackground leaves it for a white background. */
function whiteBackground(): number[] {
  const p = [...NINARO_PALETTE];
  p[0] = EGA._BRIGHTWHITE;
  p[7] = EGA._BLACK;
  return p;
}

const SOLID_MASK = Array(8).fill(0xff);

describe('monochrome looks', () => {
  it('has sixteen looks, the first being the background only', () => {
    expect(MONO_LOOKS).toHaveLength(16);
    expect(isBackgroundLook(MONO_LOOKS[0])).toBe(true);
    expect(MONO_LOOKS.slice(1).every((l) => !isBackgroundLook(l))).toBe(true);
  });

  it('builds every non-background look from the three greens only', () => {
    const bg = '#000000';
    for (const look of MONO_LOOKS.slice(1)) {
      const colours = new Set(monoTilePixels(look, bg));
      expect(colours.has(bg)).toBe(false);
      expect(look.a).toBeGreaterThan(0);
      expect(look.b).toBeGreaterThan(0);
    }
  });

  it('gives every non-background look a different combination of greens and hatch', () => {
    const keys = MONO_LOOKS.slice(1).map((l) => `${l.a}/${l.b}/${l.mask.join(',')}`);
    expect(new Set(keys).size).toBe(15);
  });

  it('gives every colour slot a different look', () => {
    for (const values of [NINARO_PALETTE, whiteBackground()]) {
      const indices = monoLookIndices(values);
      expect([...indices].sort((a, b) => a - b)).toEqual(Array.from({ length: 16 }, (_, i) => i));
    }
  });

  it('gives the background slot the background look, on black or white', () => {
    expect(monoLookIndices(NINARO_PALETTE)[0]).toBe(0);
    expect(monoLookIndices(whiteBackground())[0]).toBe(0);
  });

  it('gives the slot furthest from the background the brightest green', () => {
    // Black background: light yellow (slot 10, the lightest colour) is furthest from it.
    expect(MONO_LOOKS[monoLookIndices(NINARO_PALETTE)[10]]).toEqual({ a: 3, b: 3, mask: SOLID_MASK });
    // White background: black is then furthest from it.
    expect(MONO_LOOKS[monoLookIndices(whiteBackground())[7]].a).toBe(3);
  });

  it('draws the mostly-one-green look as three quarters a and a quarter b', () => {
    const bg = '#000000';
    const look = MONO_LOOKS.find((l) => l.a !== l.b && l.mask.some((r) => r !== 0xff && r !== 0))!;
    const pixels = monoTilePixels(look, bg);
    expect(pixels.filter((p) => p === monoToneCss(look.a, bg))).toHaveLength(48);
    expect(pixels.filter((p) => p === monoToneCss(look.b, bg))).toHaveLength(16);
  });

  it('draws a solid look as one green', () => {
    const solid = MONO_LOOKS[15];
    expect(new Set(monoTilePixels(solid, '#000000'))).toEqual(new Set([MONO_GREENS[2]]));
  });
});
