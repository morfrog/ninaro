// The Display option: how the screen is drawn. VGA is the 16-colour screen
// the game was written for. Monochrome shows it in the background colour and
// three greens, with hatching for the other colours. Modern gives the menus
// and dialogs a current look. The choice is kept in the browser, like the
// game's settings, and can be changed at any time.

import { type KeyValueStore } from '../data/storage';

export const DISPLAY_MODES = ['vga', 'mono', 'modern'] as const;
export type DisplayMode = (typeof DISPLAY_MODES)[number];

export const DEFAULT_DISPLAY_MODE: DisplayMode = 'vga';
export const DISPLAY_KEY = 'ninaro:v1:display';

/** The name shown in the Display menu. */
export const DISPLAY_LABELS: Record<DisplayMode, string> = {
  vga: 'VGA',
  mono: 'Monochrome',
  modern: 'Modern',
};

/** Any stored text that is not a known mode gives the default. */
export function parseDisplayMode(text: string | null | undefined): DisplayMode {
  return (DISPLAY_MODES as readonly string[]).includes(text ?? '') ? (text as DisplayMode) : DEFAULT_DISPLAY_MODE;
}

/** Reads the saved mode; the default if none is saved or storage is blocked. */
export function loadDisplayMode(store: KeyValueStore | null = browserStore()): DisplayMode {
  try {
    return parseDisplayMode(store?.getItem(DISPLAY_KEY));
  } catch {
    return DEFAULT_DISPLAY_MODE;
  }
}

/** Saves the mode. Returns false if storage refused it (the game still works). */
export function saveDisplayMode(mode: DisplayMode, store: KeyValueStore | null = browserStore()): boolean {
  try {
    if (!store) return false;
    store.setItem(DISPLAY_KEY, mode);
    return true;
  } catch {
    return false;
  }
}

function browserStore(): KeyValueStore | null {
  try {
    return typeof localStorage === 'undefined' ? null : localStorage;
  } catch {
    return null;
  }
}
