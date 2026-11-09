import { describe, expect, it } from 'vitest';
import type { KeyValueStore } from '../data/storage';
import { VIEW_KEY, loadViewMode, parseViewMode, saveViewMode } from './viewMode';

function mapStore(): KeyValueStore & { map: Map<string, string> } {
  const map = new Map<string, string>();
  return {
    map,
    getItem: (k) => map.get(k) ?? null,
    setItem: (k, v) => void map.set(k, v),
    removeItem: (k) => void map.delete(k),
  };
}

describe('view mode', () => {
  it('accepts normal and fullscreen and falls back to normal otherwise', () => {
    expect(parseViewMode('normal')).toBe('normal');
    expect(parseViewMode('fullscreen')).toBe('fullscreen');
    expect(parseViewMode('Fullscreen')).toBe('normal');
    expect(parseViewMode('')).toBe('normal');
    expect(parseViewMode(null)).toBe('normal');
  });

  it('saves and loads the choice', () => {
    const store = mapStore();
    expect(loadViewMode(store)).toBe('normal');
    expect(saveViewMode('fullscreen', store)).toBe(true);
    expect(store.map.get(VIEW_KEY)).toBe('fullscreen');
    expect(loadViewMode(store)).toBe('fullscreen');
  });

  it('keeps the default when storage is missing or throws', () => {
    expect(saveViewMode('fullscreen', null)).toBe(false);
    const broken: KeyValueStore = {
      getItem: () => {
        throw new Error('blocked');
      },
      setItem: () => {
        throw new Error('full');
      },
      removeItem: () => {},
    };
    expect(loadViewMode(broken)).toBe('normal');
    expect(saveViewMode('fullscreen', broken)).toBe(false);
  });
});
