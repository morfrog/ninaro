import { describe, expect, it } from 'vitest';
import { FLASH_FADE_MS, FLASH_HOLD_MS, FLASH_MAX_LINES, FLASH_MS, LogFlash, flashAlpha } from './logFlash';

describe('flashAlpha', () => {
  it('holds a message in full, then fades it to nothing', () => {
    expect(flashAlpha(0)).toBe(1);
    expect(flashAlpha(FLASH_HOLD_MS)).toBe(1);
    expect(flashAlpha(FLASH_HOLD_MS + FLASH_FADE_MS / 2)).toBeCloseTo(0.5);
    expect(flashAlpha(FLASH_MS)).toBe(0);
    expect(flashAlpha(FLASH_MS + 1000)).toBe(0);
  });

  it('stays long enough to read, and is gone within a few seconds', () => {
    expect(FLASH_HOLD_MS).toBeGreaterThanOrEqual(2000);
    expect(FLASH_MS).toBeLessThanOrEqual(3000);
  });
});

describe('LogFlash', () => {
  const texts = (f: LogFlash, now: number) => f.current(now).map((l) => l.text);

  it('marks a leading ~ as bold', () => {
    const f = new LogFlash();
    expect(f.current(0)).toEqual([]);
    f.show('~Press ESC To Continue', 0);
    expect(f.current(0)).toEqual([{ text: 'Press ESC To Continue', bold: true, alpha: 1 }]);
  });

  it('stacks lines that come close together, each going on its own timer: a, ab, b', () => {
    const f = new LogFlash();
    f.show('~You Win!', 0);
    expect(texts(f, 0)).toEqual(['You Win!']);
    f.show('Human Goes First', 500);
    expect(texts(f, 500)).toEqual(['You Win!', 'Human Goes First']);
    // The first fades on its own timer, while the second is still held.
    const fading = f.current(FLASH_HOLD_MS + FLASH_FADE_MS / 2);
    expect(fading[0].alpha).toBeCloseTo(0.5);
    expect(fading[1].alpha).toBe(1);
    expect(texts(f, FLASH_MS)).toEqual(['Human Goes First']);
    expect(texts(f, FLASH_MS + 500)).toEqual([]);
  });

  it('shows at most a few lines, pushing out the oldest', () => {
    const f = new LogFlash();
    for (let i = 1; i <= FLASH_MAX_LINES + 2; i++) f.show(`line ${i}`, i);
    const shown = texts(f, 10);
    expect(shown).toHaveLength(FLASH_MAX_LINES);
    expect(shown.at(-1)).toBe(`line ${FLASH_MAX_LINES + 2}`);
    expect(shown[0]).toBe('line 3');
  });

  it('goes away once faded, and does not come back', () => {
    const f = new LogFlash();
    f.show("It's A Draw...", 0);
    expect(f.current(FLASH_HOLD_MS + 1)[0].alpha).toBeLessThan(1);
    expect(f.current(FLASH_MS)).toEqual([]);
    expect(f.current(0)).toEqual([]);
  });

  it('keeps an announcement until it is released, then fades it at once', () => {
    const f = new LogFlash();
    f.show('~You Win!', 0, true);
    f.show('~Press ESC To Continue', 10);
    expect(texts(f, 60_000)).toEqual(['You Win!']); // the other line faded; the announcement stays
    expect(f.current(60_000)[0].alpha).toBe(1);
    expect(f.animating(60_000)).toBe(false); // nothing to redraw while it holds still
    expect(f.release(60_000)).toBe(true);
    expect(f.animating(60_000)).toBe(true);
    expect(f.current(60_000 + FLASH_FADE_MS / 2)[0].alpha).toBeCloseTo(0.5);
    expect(texts(f, 60_000 + FLASH_FADE_MS)).toEqual([]);
    expect(f.release(70_000)).toBe(false);
  });

  it('pushes out ordinary lines before an announcement', () => {
    const f = new LogFlash();
    f.show('~You Lose', 0, true);
    for (let i = 1; i <= FLASH_MAX_LINES; i++) f.show(`line ${i}`, i);
    const shown = texts(f, 10);
    expect(shown).toHaveLength(FLASH_MAX_LINES);
    expect(shown[0]).toBe('You Lose');
  });

  it('can be cleared', () => {
    const f = new LogFlash();
    f.show('Game Loaded', 0);
    f.show('Game Saved', 1);
    f.clear();
    expect(f.current(1)).toEqual([]);
  });
});
