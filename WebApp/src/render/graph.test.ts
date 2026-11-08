import { describe, expect, it } from 'vitest';
import { BORDER, FILL, Graph, HAlign, VAlign } from './graph';
import { Palette } from './palette';

/** A canvas context stand-in that records path points and fills. */
function recorder() {
  const calls: { op: string; args: number[]; style?: string }[] = [];
  const ctx = {
    fillStyle: '',
    strokeStyle: '',
    lineWidth: 1,
    lineCap: 'butt',
    lineJoin: 'miter',
    imageSmoothingEnabled: true,
    save() {},
    restore() {},
    beginPath() {},
    clip() {},
    stroke() {
      calls.push({ op: 'stroke', args: [], style: ctx.strokeStyle });
    },
    fill() {
      calls.push({ op: 'fill', args: [], style: ctx.fillStyle });
    },
    rect(...args: number[]) {
      calls.push({ op: 'clip', args });
    },
    moveTo(...args: number[]) {
      calls.push({ op: 'moveTo', args });
    },
    lineTo(...args: number[]) {
      calls.push({ op: 'lineTo', args });
    },
    fillRect(...args: number[]) {
      calls.push({ op: 'fillRect', args, style: ctx.fillStyle });
    },
    strokeRect(...args: number[]) {
      calls.push({ op: 'strokeRect', args, style: ctx.strokeStyle });
    },
    ellipse(...args: number[]) {
      calls.push({ op: 'ellipse', args });
    },
  };
  const g = new Graph(ctx as unknown as CanvasRenderingContext2D, new Palette());
  return { g, calls };
}

describe('window coordinates', () => {
  it('span the viewport and truncate, as the original grid lines show', () => {
    const { g, calls } = recorder();
    g.setViewport(0, 31, 383, 349);
    g.moveToW(1 / 7, 0);
    g.lineToW(1 / 7, 1);
    // 383 / 7 = 54.7 -> 54; lines are drawn through pixel centres.
    expect(calls.filter((c) => c.op === 'moveTo' || c.op === 'lineTo').map((c) => c.args)).toEqual([
      [54.5, 31.5],
      [54.5, 349.5],
    ]);
  });

  it('clip every drawing to the viewport', () => {
    const { g, calls } = recorder();
    g.setViewport(389, 181, 639, 349);
    g.rectangle(BORDER, 0, 0, 10, 10);
    expect(calls[0]).toEqual({ op: 'clip', args: [389, 181, 251, 169] });
  });
});

describe('rectangles and ellipses', () => {
  it('draw relative to the view origin, which _setvieworg can move', () => {
    const { g, calls } = recorder();
    g.setViewport(100, 50, 200, 150);
    g.setColor(1);
    g.rectangle(FILL, 0, 0, 9, 4);
    g.setVieworg(0, 0);
    g.rectangle(BORDER, 100, 50, 110, 60);
    expect(calls.filter((c) => c.op === 'fillRect' || c.op === 'strokeRect')).toEqual([
      { op: 'fillRect', args: [100, 50, 10, 5], style: '#aa0000' },
      { op: 'strokeRect', args: [100.5, 50.5, 10, 10], style: '#aa0000' },
    ]);
  });

  it('fill an ellipse over its whole bounding box', () => {
    const { g, calls } = recorder();
    g.ellipse(FILL, 284, 5, 317, 21);
    expect(calls.find((c) => c.op === 'ellipse')!.args.slice(0, 4)).toEqual([301, 13.5, 17, 8.5]);
  });
});

describe('text', () => {
  it('advances by the cell width plus spacing', () => {
    const { g } = recorder();
    g.setCharSize(20, 10);
    expect(g.textWidth('Hit Any Key To Continue...')).toBe(260);
    g.setCharSpacing(-2);
    expect(g.textWidth('ab')).toBe(18);
    expect(g.textWidth('')).toBe(0);
  });

  it('scales _w sizes by the whole screen, not the viewport', () => {
    const { g } = recorder();
    g.setViewport(389, 0, 639, 34);
    g.setCharSizeW(0.1, 0.04);
    g.setCharSpacingW(-0.01);
    // Measured: "Ninaro" advances 19.2 pixels a letter in the original.
    expect(g.textWidth('NN') - g.textWidth('N')).toBeCloseTo(19.17, 1);
  });

  it('centres a line, with capitals 0.1 of the cell down', () => {
    const { g, calls } = recorder();
    g.setCharSize(20, 10);
    g.setTextAlign(HAlign.CENTER, VAlign.TOP);
    g.grText(319, 324, 'H');
    // H's left stem: Hershey x 4 of an advance of 22 (half 11), y 1 to 22.
    const [x, y] = calls.find((c) => c.op === 'moveTo')!.args;
    expect(x).toBeCloseTo(319 + (4 - 11) * (10 / 32), 5);
    expect(y).toBeCloseTo(324 + 2, 5);
  });

  it('puts the baseline 0.8 of the cell down for _BASE', () => {
    const { g, calls } = recorder();
    g.setCharSize(10, 10);
    g.setTextAlign(HAlign.LEFT, VAlign.BASE);
    g.grText(0, 100, 'H');
    const ys = calls.filter((c) => c.op === 'lineTo').map((c) => c.args[1]);
    expect(Math.max(...ys)).toBeCloseTo(100, 5);
  });
});
