// The 16-colour EGA palette as Ninaro set it (SCREEN.C ResetPalette) and
// changed it (_remappalette). Colours are Watcom graph.h values: 0x00BBGGRR
// with 6-bit channels (0..0x3F), shown 8-bit as DOSBox does (v * 255 / 63).

/** SCREEN.H colour slots. */
export const Color = {
  BLACK: 0, RED: 1, YELLOW: 2, BLUE: 3, GREEN: 4, PURPLE: 5, ORANGE: 6, WHITE: 7,
  CYAN: 8, LTRED: 9, LTYELLOW: 10, LTBLUE: 11, LTGREEN: 12, LTPURPLE: 13, GREY: 14, LTGREY: 15,
} as const;

/** Watcom graph.h colour values. */
export const EGA = {
  _BLACK: 0x000000,
  _BLUE: 0x2a0000,
  _GREEN: 0x002a00,
  _CYAN: 0x2a2a00,
  _RED: 0x00002a,
  _MAGENTA: 0x2a002a,
  _BROWN: 0x00152a,
  _WHITE: 0x2a2a2a,
  _GRAY: 0x151515,
  _LIGHTBLUE: 0x3f1515,
  _LIGHTGREEN: 0x153f15,
  _LIGHTCYAN: 0x3f3f15,
  _LIGHTRED: 0x15153f,
  _LIGHTMAGENTA: 0x3f153f,
  _YELLOW: 0x153f3f,
  _LIGHTYELLOW: 0x153f3f,
  _BRIGHTWHITE: 0x3f3f3f,
} as const;

/** SCREEN.C ResetPalette, slot by slot. */
export const NINARO_PALETTE: readonly number[] = [
  EGA._BLACK,
  EGA._RED,
  EGA._YELLOW - 0x010101,
  EGA._BLUE + 0x150000,
  EGA._GREEN,
  EGA._MAGENTA,
  EGA._BROWN,
  EGA._WHITE,
  EGA._CYAN,
  EGA._LIGHTRED,
  EGA._LIGHTYELLOW,
  EGA._LIGHTBLUE,
  EGA._LIGHTGREEN,
  EGA._LIGHTMAGENTA,
  EGA._GRAY,
  EGA._GRAY + 0x080808,
];

/** A 6-bit 0x00BBGGRR value as CSS #rrggbb. */
export function toCss(value: number): string {
  const channel = (shift: number) => Math.round((((value >> shift) & 0x3f) * 255) / 63);
  const hex = (n: number) => n.toString(16).padStart(2, '0');
  return `#${hex(channel(0))}${hex(channel(8))}${hex(channel(16))}`;
}

/** A '#rrggbb' colour as a 6-bit 0x00BBGGRR palette value (the nearest step of each channel). */
export function hexToValue(hex: string): number {
  const channel = (i: number) => Math.round((parseInt(hex.slice(1 + 2 * i, 3 + 2 * i), 16) * 63) / 255);
  return (channel(2) << 16) | (channel(1) << 8) | channel(0);
}

/**
 * The Modern look's sixteen slots: a dark background with light text, and the
 * game's colours in brighter, flatter tones. The same slots as the VGA
 * palette, so the game's drawing is unchanged.
 */
export const MODERN_PALETTE: readonly number[] = [
  '#1e1e1e', // BLACK: the background
  '#e5484d', // RED
  '#f5c542', // YELLOW
  '#3b82f6', // BLUE
  '#46a758', // GREEN
  '#8e4ec6', // PURPLE
  '#f76b15', // ORANGE
  '#e6e6e6', // WHITE: the text
  '#12a4b0', // CYAN
  '#ff8a8f', // LTRED
  '#ffe58a', // LTYELLOW
  '#8ab4ff', // LTBLUE
  '#8fd49a', // LTGREEN
  '#c49cff', // LTPURPLE
  '#6e6e73', // GREY
  '#a0a0a8', // LTGREY
].map(hexToValue);

export class Palette {
  private readonly slots: number[];
  /** The values reset() goes back to: the VGA palette, or the Modern one. */
  private base: number[];

  constructor(values: readonly number[] = NINARO_PALETTE) {
    this.slots = [...values];
    this.base = [...values];
  }

  /** Makes these the values reset() returns to, and applies them now. */
  setBase(values: readonly number[]): void {
    this.base = [...values];
    this.reset();
  }

  /** ResetPalette: back to the base values. */
  reset(): void {
    this.base.forEach((v, i) => (this.slots[i] = v));
  }

  /** _remappalette */
  remap(slot: number, value: number): void {
    this.slots[slot & 15] = value;
  }

  /** _remapallpalette, as ResetPalette uses it */
  remapAll(values: readonly number[]): void {
    values.forEach((v, i) => (this.slots[i] = v));
  }

  value(slot: number): number {
    return this.slots[slot & 15];
  }

  css(slot: number): string {
    return toCss(this.value(slot));
  }
}
