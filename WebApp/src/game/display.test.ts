import { describe, expect, it } from 'vitest';
import type { Graph } from '../render/graph';
import { FILL } from '../render/graph';
import { drawDropColumn } from './display';

describe('drawDropColumn', () => {
  it('fills the full height of the column the token is over, in the dark band colour', () => {
    const calls: unknown[][] = [];
    const g = {
      displayMode: 'vga',
      setColorCss: (c: string) => calls.push(['css', c]),
      rectangleW: (...args: unknown[]) => calls.push(['rect', ...args]),
    } as unknown as Graph;
    drawDropColumn(g, 2, 7);
    expect(calls).toEqual([
      ['css', '#262626'],
      ['rect', FILL, 2 / 7, 0, 3 / 7, 1],
    ]);
  });
});
