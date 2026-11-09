// Monochrome display (the Display option's second mode): black stays the
// background, and every other colour is drawn in combinations of three
// greens (dark, mid and bright), with hatching where two greens meet. The
// game's fifteen colours cannot all be told apart by shade alone, so each
// gets a "look": one of fifteen distinct green combinations. The colours are
// ranked by how far each is from the background, so the ones nearest the
// background get the darkest look and the ones furthest the brightest. No two
// colours share a look, so the players stay apart.

import { toCss } from './palette';

/** Dark, mid and bright green. */
export const MONO_GREENS = ['#006400', '#00b000', '#66ff66'] as const;

/** Which green a look uses: 0 is the background, 1..3 index MONO_GREENS. */
export type MonoTone = 0 | 1 | 2 | 3;

/**
 * One of the sixteen looks. Each pixel of its 8 x 8 tile is colour `a` where
 * the hatch is set and colour `b` where it is clear. Tone 0 is the background.
 */
export interface MonoLook {
  a: MonoTone;
  b: MonoTone;
  /** Eight rows, top first, bit 7 leftmost; a set bit is `a`. */
  mask: readonly number[];
}

/** Builds an 8 x 8 hatch from a rule for each cell (x and y in 0..7). */
function hatch(on: (x: number, y: number) => boolean): number[] {
  return Array.from({ length: 8 }, (_, y) => {
    let row = 0;
    for (let x = 0; x < 8; x++) if (on(x, y)) row |= 0x80 >> x;
    return row;
  });
}

const SOLID = hatch(() => true);
/** Half the pixels, in alternate rows: an even mix of two greens. */
const ROWS = hatch((_, y) => y % 2 === 0);
/** Half the pixels, in a checkerboard: an even mix of two greens. */
const CHECKER = hatch((x, y) => (x + y) % 2 === 0);
/** Three quarters `a`, with `b` showing through as a dot on every second row and column. */
const MOSTLY = hatch((x, y) => !(x % 2 === 0 && y % 2 === 0));

const D = 1;
const M = 2;
const B = 3;

/**
 * The sixteen looks, from the darkest to the brightest: the background, then
 * fifteen combinations of the three greens. Solids come as the single green;
 * a mix of two greens is either even (checkerboard or rows) or mostly one
 * green with the other as dots.
 */
export const MONO_LOOKS: readonly MonoLook[] = [
  { a: 0, b: 0, mask: SOLID }, // the background itself
  { a: D, b: D, mask: SOLID },
  { a: D, b: M, mask: MOSTLY },
  { a: D, b: B, mask: MOSTLY },
  { a: D, b: M, mask: CHECKER },
  { a: D, b: M, mask: ROWS },
  { a: M, b: D, mask: MOSTLY },
  { a: D, b: B, mask: CHECKER },
  { a: D, b: B, mask: ROWS },
  { a: M, b: M, mask: SOLID },
  { a: M, b: B, mask: MOSTLY },
  { a: B, b: D, mask: MOSTLY },
  { a: M, b: B, mask: CHECKER },
  { a: M, b: B, mask: ROWS },
  { a: B, b: M, mask: MOSTLY },
  { a: B, b: B, mask: SOLID },
];

/** The look index of the darkest non-black colour: the solid dark green. */
export const MONO_DARKEST = 1;
/** The look index of the lightest green: the solid bright green. */
export const MONO_LIGHTEST = 15;
/** The look index of the second darkest non-black colour: the one after MONO_DARKEST. */
export const MONO_SECOND = 2;

/** Whether a look is the background (black, on the default screen). */
export function isBackgroundLook(look: MonoLook): boolean {
  return look.a === 0 && look.b === 0;
}

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

/** The colour of a tone: the background colour for tone 0, else its green. */
export function monoToneCss(tone: MonoTone, backgroundCss: string): string {
  return tone === 0 ? backgroundCss : MONO_GREENS[tone - 1];
}

/**
 * A single colour for a look, for fills that need one (such as the suggest
 * hatch): the look's `a` green.
 */
export function monoInkCss(look: MonoLook, backgroundCss: string): string {
  return monoToneCss(look.a, backgroundCss);
}

/** The 64 pixels of a look's tile, as CSS colours (row by row, left to right). */
export function monoTilePixels(look: MonoLook, backgroundCss: string): string[] {
  const a = monoToneCss(look.a, backgroundCss);
  const b = monoToneCss(look.b, backgroundCss);
  const pixels: string[] = [];
  for (let y = 0; y < 8; y++) {
    for (let x = 0; x < 8; x++) pixels.push(look.mask[y] & (0x80 >> x) ? a : b);
  }
  return pixels;
}
