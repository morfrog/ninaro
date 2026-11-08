import { describe, expect, it } from 'vitest';
import { StoredFile, fromBase64, toBase64, type KeyValueStore } from './storage';

function mapStore(): KeyValueStore & { map: Map<string, string> } {
  const map = new Map<string, string>();
  return {
    map,
    getItem: (k) => map.get(k) ?? null,
    setItem: (k, v) => void map.set(k, v),
    removeItem: (k) => void map.delete(k),
  };
}

const throwing: KeyValueStore = {
  getItem: () => {
    throw new Error('blocked');
  },
  setItem: () => {
    throw new Error('quota');
  },
  removeItem: () => {
    throw new Error('blocked');
  },
};

describe('base64', () => {
  it('round-trips every byte value', () => {
    const all = Uint8Array.from({ length: 256 }, (_, i) => i);
    expect(fromBase64(toBase64(all))).toEqual(all);
  });
});

describe('StoredFile', () => {
  it('writes, reads and removes under its key', () => {
    const store = mapStore();
    const f = new StoredFile('ninaro:v1:cfg', store);
    expect(f.read()).toBe(null);
    expect(f.write(new Uint8Array([0, 0, 0, 7]))).toBe(true);
    expect(store.map.has('ninaro:v1:cfg')).toBe(true);
    expect(f.read()).toEqual(new Uint8Array([0, 0, 0, 7]));
    f.remove();
    expect(f.read()).toBe(null);
  });

  it('works without storage, remembering nothing', () => {
    const f = new StoredFile('k', null);
    expect(f.write(new Uint8Array([1]))).toBe(false);
    expect(f.read()).toBe(null);
  });

  it('survives storage that throws', () => {
    const f = new StoredFile('k', throwing);
    expect(f.write(new Uint8Array([1]))).toBe(false);
    expect(f.read()).toBe(null);
    expect(() => f.remove()).not.toThrow();
  });

  it('treats unreadable contents as missing', () => {
    const store = mapStore();
    store.map.set('k', '%%% not base64');
    expect(new StoredFile('k', store).read()).toBe(null);
  });
});
