import { describe, expect, it } from 'vitest';
import type { Graph } from '../render/graph';
import { FILL } from '../render/graph';
import { Color } from '../render/palette';
import { drawDropColumn } from './display';

describe('drawDropColumn', () => {
  it('fills the full height of the column the token is over, in the grey band colour', () => {
    const calls: unknown[][] = [];
    const g = {
      displayMode: 'vga',
      setColor: (c: number) => calls.push(['color', c]),
      rectangleW: (...args: unknown[]) => calls.push(['rect', ...args]),
    } as unknown as Graph;
    drawDropColumn(g, 2, 7);
    expect(calls).toEqual([
      ['color', Color.GREY],
      ['rect', FILL, 2 / 7, 0, 3 / 7, 1],
    ]);
  });
});
