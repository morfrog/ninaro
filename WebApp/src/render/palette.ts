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

export class Palette {
  private readonly slots: number[];

  constructor(values: readonly number[] = NINARO_PALETTE) {
    this.slots = [...values];
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
