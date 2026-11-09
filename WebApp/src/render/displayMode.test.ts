import { describe, expect, it } from 'vitest';
import type { KeyValueStore } from '../data/storage';
import { DISPLAY_KEY, loadDisplayMode, parseDisplayMode, saveDisplayMode } from './displayMode';

function mapStore(): KeyValueStore & { map: Map<string, string> } {
  const map = new Map<string, string>();
  return {
    map,
    getItem: (k) => map.get(k) ?? null,
    setItem: (k, v) => void map.set(k, v),
    removeItem: (k) => void map.delete(k),
  };
}

describe('display mode', () => {
  it('accepts the three modes and falls back to VGA otherwise', () => {
    expect(parseDisplayMode('vga')).toBe('vga');
    expect(parseDisplayMode('mono')).toBe('mono');
    expect(parseDisplayMode('modern')).toBe('modern');
    expect(parseDisplayMode('Modern')).toBe('vga');
    expect(parseDisplayMode('')).toBe('vga');
    expect(parseDisplayMode(null)).toBe('vga');
  });

  it('saves and loads the choice', () => {
    const store = mapStore();
    expect(loadDisplayMode(store)).toBe('vga');
    expect(saveDisplayMode('mono', store)).toBe(true);
    expect(store.map.get(DISPLAY_KEY)).toBe('mono');
    expect(loadDisplayMode(store)).toBe('mono');
  });

  it('works without storage', () => {
    expect(loadDisplayMode(null)).toBe('vga');
    expect(saveDisplayMode('modern', null)).toBe(false);
  });

  it('survives storage that throws', () => {
    const broken: KeyValueStore = {
      getItem: () => {
        throw new Error('blocked');
      },
      setItem: () => {
        throw new Error('full');
      },
      removeItem: () => {},
    };
    expect(loadDisplayMode(broken)).toBe('vga');
    expect(saveDisplayMode('mono', broken)).toBe(false);
  });
});
