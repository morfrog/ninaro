// Monochrome display (the Display option's second mode): the screen is drawn
// in the background colour and three greens. The game's sixteen colours
// cannot all be told apart by shade alone, so each one gets a "look": a
// green tone and a hatch, out of sixteen distinct looks. The colours are
// ranked by how far each is from the background, so the ones nearest the
// background get the sparsest hatching and the ones furthest from it the
// solid greens. No two colours share a look, so the players stay apart.

import { toCss } from './palette';

/** Background, then dark, mid and bright green (the tones of a look). */
export const MONO_GREENS = ['#006400', '#00b000', '#66ff66'] as const;

/** One of the sixteen looks: a tone (0 = background) and an 8 x 8 hatch. */
export interface MonoLook {
  /** 0 is the background; 1..3 index MONO_GREENS. */
  tone: 0 | 1 | 2 | 3;
  /** Eight rows, top first, bit 7 leftmost; a set bit is ink. */
  mask: readonly number[];
}

/** Builds an 8 x 8 hatch from a rule for each cell (x and y in 0..7). */
function hatch(ink: (x: number, y: number) => boolean): number[] {
  return Array.from({ length: 8 }, (_, y) => {
    let row = 0;
    for (let x = 0; x < 8; x++) if (ink(x, y)) row |= 0x80 >> x;
    return row;
  });
}

const SOLID = hatch(() => true);
const DOTS_1_16 = hatch((x, y) => x % 4 === 0 && y % 4 === 0);
const DOTS_1_4 = hatch((x, y) => x % 2 === 0 && y % 2 === 0);
const ROWS_1_4 = hatch((_, y) => y % 4 === 0);
const ROWS_1_2 = hatch((_, y) => y % 2 === 0);
const COLUMNS_1_2 = hatch((x) => x % 2 === 0);
const CHECKER = hatch((x, y) => (x + y) % 2 === 0);
const DIAGONAL = hatch((x, y) => (x + y) % 8 === 0);
const DIAGONAL_BACK = hatch((x, y) => (x + 8 - y) % 8 === 0);

/**
 * The sixteen looks, from the sparsest to the densest. Dark greens come
 * first, then mid, then bright; the hatches within a tone run from sparse to
 * dense.
 */
export const MONO_LOOKS: readonly MonoLook[] = [
  { tone: 0, mask: SOLID }, // the background itself
  { tone: 1, mask: DOTS_1_16 },
  { tone: 1, mask: DOTS_1_4 },
  { tone: 1, mask: ROWS_1_4 },
  { tone: 1, mask: DIAGONAL },
  { tone: 1, mask: CHECKER },
  { tone: 1, mask: SOLID },
  { tone: 2, mask: DOTS_1_16 },
  { tone: 2, mask: COLUMNS_1_2 },
  { tone: 2, mask: DIAGONAL_BACK },
  { tone: 2, mask: ROWS_1_2 },
  { tone: 2, mask: CHECKER },
  { tone: 2, mask: SOLID },
  { tone: 3, mask: DOTS_1_4 },
  { tone: 3, mask: ROWS_1_2 },
  { tone: 3, mask: SOLID },
];

/** Rec. 601 luma of a 0x00BBGGRR palette value, 0..255. */
function luma(value: number): number {
  const css = toCss(value);
  const r = parseInt(css.slice(1, 3), 16);
  const g = parseInt(css.slice(3, 5), 16);
  const b = parseInt(css.slice(5, 7), 16);
  return 0.299 * r + 0.587 * g + 0.114 * b;
}

/**
 * The look index for each of the sixteen colour slots, given their palette
 * values. Slot 0 is the background, so it is always rank 0 (the background
 * look). Ties keep slot order, so the result is always a one-to-one mapping.
 */
export function monoLookIndices(values: readonly number[]): number[] {
  const bg = luma(values[0]);
  const order = values
    .map((v, slot) => ({ slot, ink: Math.abs(luma(v) - bg) }))
    .sort((a, b) => a.ink - b.ink || a.slot - b.slot);
  const index = new Array<number>(values.length);
  order.forEach((entry, rank) => (index[entry.slot] = rank));
  return index;
}

/** The ink colour of a look: its green, or the background for the background look. */
export function monoInkCss(look: MonoLook, backgroundCss: string): string {
  return look.tone === 0 ? backgroundCss : MONO_GREENS[look.tone - 1];
}

/** The 64 pixels of a look's tile, as CSS colours (row by row, left to right). */
export function monoTilePixels(look: MonoLook, backgroundCss: string): string[] {
  const ink = monoInkCss(look, backgroundCss);
  const pixels: string[] = [];
  for (let y = 0; y < 8; y++) {
    for (let x = 0; x < 8; x++) pixels.push(look.mask[y] & (0x80 >> x) ? ink : backgroundCss);
  }
  return pixels;
}
