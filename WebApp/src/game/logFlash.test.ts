import { describe, expect, it } from 'vitest';
import { FLASH_FADE_MS, FLASH_HOLD_MS, FLASH_MS, LogFlash, flashAlpha } from './logFlash';

describe('flashAlpha', () => {
  it('holds a message in full, then fades it to nothing', () => {
    expect(flashAlpha(0)).toBe(1);
    expect(flashAlpha(FLASH_HOLD_MS)).toBe(1);
    expect(flashAlpha(FLASH_HOLD_MS + FLASH_FADE_MS / 2)).toBeCloseTo(0.5);
    expect(flashAlpha(FLASH_MS)).toBe(0);
    expect(flashAlpha(FLASH_MS + 1000)).toBe(0);
  });

  it('fades out within a couple of seconds', () => {
    expect(FLASH_MS).toBeLessThanOrEqual(2500);
  });
});

describe('LogFlash', () => {
  it('shows the latest line, and marks a leading ~ as bold', () => {
    const f = new LogFlash();
    expect(f.current(0)).toBeNull();
    f.show('Game Saved', 0);
    f.show('~Press ESC To Continue', 100);
    expect(f.current(100)).toEqual({ text: 'Press ESC To Continue', bold: true, alpha: 1 });
  });

  it('goes away once faded, and does not come back', () => {
    const f = new LogFlash();
    f.show('It\'s A Draw...', 0);
    expect(f.current(FLASH_HOLD_MS + 1)?.alpha).toBeLessThan(1);
    expect(f.current(FLASH_MS)).toBeNull();
    expect(f.current(0)).toBeNull();
  });

  it('can be cleared', () => {
    const f = new LogFlash();
    f.show('Game Loaded', 0);
    f.clear();
    expect(f.current(0)).toBeNull();
  });
});
