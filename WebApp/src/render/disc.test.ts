import { describe, expect, it } from 'vitest';
import { nullGraph } from '../testFakes';
import { drawDisc } from './screen';

describe('drawDisc', () => {
  it('draws a circle in device pixels, centred in its cell, on a board stretched across', () => {
    const g = nullGraph();
    g.setViewport(0, 0, 383, 349); // the board: 384 by 350 EGA pixels
    g.setPixelScale(2.5, 1.2); // stretched wider than it is tall
    const drawn: number[][] = [];
    g.ellipseW = (_fill, x1, y1, x2, y2) => void drawn.push([x1, y1, x2, y2]);

    drawDisc(g, 3, 7, 4, 8, 1, 0.9);

    const [x1, y1, x2, y2] = drawn[0];
    const radiusX = ((x2 - x1) / 2) * 384 * 2.5;
    const radiusY = ((y2 - y1) / 2) * 350 * 1.2;
    expect(radiusX).toBeCloseTo(radiusY);
    expect((x1 + x2) / 2).toBeCloseTo(3.5 / 7);
    expect((y1 + y2) / 2).toBeCloseTo(4.5 / 8);
  });
});
