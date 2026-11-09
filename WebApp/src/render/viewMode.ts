// The View option: Normal shows the whole screen, with the information area
// at the right. Fullscreen shows only the game board (and the entry row above
// it), with the title and the board's size in the page's toolbar. The choice
// is kept in the browser, like the Display option.

import { type KeyValueStore } from '../data/storage';

export const VIEW_MODES = ['normal', 'fullscreen'] as const;
export type ViewMode = (typeof VIEW_MODES)[number];

export const DEFAULT_VIEW_MODE: ViewMode = 'normal';
export const VIEW_KEY = 'ninaro:v1:view';

/** The name shown in the View menu. */
export const VIEW_LABELS: Record<ViewMode, string> = {
  normal: 'Normal',
  fullscreen: 'Fullscreen',
};

/** Any stored text that is not a known mode gives the default. */
export function parseViewMode(text: string | null | undefined): ViewMode {
  return (VIEW_MODES as readonly string[]).includes(text ?? '') ? (text as ViewMode) : DEFAULT_VIEW_MODE;
}

/** Reads the saved mode; the default if none is saved or storage is blocked. */
export function loadViewMode(store: KeyValueStore | null = browserStore()): ViewMode {
  try {
    return parseViewMode(store?.getItem(VIEW_KEY));
  } catch {
    return DEFAULT_VIEW_MODE;
  }
}

/** Saves the mode. Returns false if storage refused it (the game still works). */
export function saveViewMode(mode: ViewMode, store: KeyValueStore | null = browserStore()): boolean {
  try {
    if (!store) return false;
    store.setItem(VIEW_KEY, mode);
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
