import { describe, expect, it } from 'vitest';
import { WatcomRandom, startRandomSeed } from './random';

describe('WatcomRandom', () => {
  it('gives the ANSI C sequence from the default seed of 1', () => {
    const r = new WatcomRandom();
    expect([r.rand(), r.rand(), r.rand(), r.rand()]).toEqual([16838, 5758, 10113, 17515]);
  });

  it('restarts the sequence when seeded again', () => {
    const r = new WatcomRandom(42);
    const first = [r.rand(), r.rand()];
    r.seed(42);
    expect([r.rand(), r.rand()]).toEqual(first);
  });

  it('Random(n) is rand() % n', () => {
    const a = new WatcomRandom(7);
    const b = new WatcomRandom(7);
    for (let i = 0; i < 20; i++) expect(a.random(6)).toBe(b.rand() % 6);
  });

  it('stays within 0..32767 for large seeds', () => {
    const r = new WatcomRandom(0xffffffff);
    for (let i = 0; i < 1000; i++) {
      const v = r.rand();
      expect(v).toBeGreaterThanOrEqual(0);
      expect(v).toBeLessThanOrEqual(32767);
    }
  });
});

describe('startRandomSeed', () => {
  it('is hundredths times seconds, as StartRandom did', () => {
    expect(startRandomSeed(new Date(2026, 0, 1, 12, 0, 30, 450))).toBe(45 * 30);
    expect(startRandomSeed(new Date(2026, 0, 1, 12, 0, 0, 990))).toBe(0);
  });
});
