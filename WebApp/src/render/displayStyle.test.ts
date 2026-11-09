import { describe, expect, it } from 'vitest';
import { EGA, NINARO_PALETTE, toCss } from './palette';
import { MONO_LOOKS, monoInkCss, monoLookIndices, monoTilePixels } from './displayStyle';

/** The palette as ApplyBackground leaves it for a white background. */
function whiteBackground(): number[] {
  const p = [...NINARO_PALETTE];
  p[0] = EGA._BRIGHTWHITE;
  p[7] = EGA._BLACK;
  return p;
}

describe('monochrome looks', () => {
  it('has sixteen looks, the first being the background', () => {
    expect(MONO_LOOKS).toHaveLength(16);
    expect(MONO_LOOKS[0].tone).toBe(0);
  });

  it('gives every colour slot a different look', () => {
    for (const values of [NINARO_PALETTE, whiteBackground()]) {
      const indices = monoLookIndices(values);
      expect(new Set(indices).size).toBe(16);
      expect([...indices].sort((a, b) => a - b)).toEqual(Array.from({ length: 16 }, (_, i) => i));
    }
  });

  it('gives the background slot the background look, on black or white', () => {
    expect(monoLookIndices(NINARO_PALETTE)[0]).toBe(0);
    expect(monoLookIndices(whiteBackground())[0]).toBe(0);
  });

  it('gives the slot furthest from the background the brightest green', () => {
    // Black background: light yellow (slot 10, the lightest colour) is furthest from it.
    expect(MONO_LOOKS[monoLookIndices(NINARO_PALETTE)[10]].tone).toBe(3);
    expect(monoLookIndices(NINARO_PALETTE)[10]).toBe(15);
    // White background: black is then furthest from it.
    expect(MONO_LOOKS[monoLookIndices(whiteBackground())[7]].tone).toBe(3);
  });

  it('uses only the background and the three greens for ink', () => {
    const bg = toCss(NINARO_PALETTE[0]);
    const inks = new Set(MONO_LOOKS.map((look) => monoInkCss(look, bg)));
    expect(inks.size).toBe(4);
  });

  it('draws a tile of 64 pixels in the look\'s own colours', () => {
    const bg = '#000000';
    const solid = MONO_LOOKS.find((l) => l.tone === 3 && l.mask.every((r) => r === 0xff))!;
    const pixels = monoTilePixels(solid, bg);
    expect(pixels).toHaveLength(64);
    expect(new Set(pixels)).toEqual(new Set([monoInkCss(solid, bg)]));
    const dotted = MONO_LOOKS[1];
    const dottedPixels = monoTilePixels(dotted, bg);
    expect(dottedPixels.filter((p) => p !== bg)).toHaveLength(4);
  });
});
