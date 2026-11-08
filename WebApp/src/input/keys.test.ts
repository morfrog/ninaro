import { describe, expect, it } from 'vitest';
import { AF4, BKSP, CF9, CQ, CX, ENTER, ESC, F1, F10, KP2, KP4, KP6, KP8, KeyGroup, KeySys, legacyKey, SF1 } from './keys';

const key = (key: string, code: string, mods: Partial<{ ctrlKey: boolean; altKey: boolean; shiftKey: boolean }> = {}) =>
  legacyKey({ key, code, ctrlKey: false, altKey: false, shiftKey: false, ...mods });

describe('legacyKey', () => {
  it('gives letters and digits as typed', () => {
    expect(key('a', 'KeyA')).toBe(97);
    expect(key('A', 'KeyA', { shiftKey: true })).toBe(65);
    expect(key('h', 'KeyH')).toBe(104);
  });

  it('gives arrows and the keypad as the DOS keypad with NumLock off', () => {
    expect([key('ArrowLeft', 'ArrowLeft'), key('ArrowRight', 'ArrowRight'), key('ArrowUp', 'ArrowUp'), key('ArrowDown', 'ArrowDown')]).toEqual([
      KP4, KP6, KP8, KP2,
    ]);
    expect(key('4', 'Numpad4')).toBe(KP4);
    expect(key('ArrowLeft', 'Numpad4')).toBe(KP4);
  });

  it('gives Enter, Backspace and Esc', () => {
    expect([key('Enter', 'Enter'), key('Enter', 'NumpadEnter'), key('Backspace', 'Backspace'), key('Escape', 'Escape')]).toEqual([
      ENTER, ENTER, BKSP, ESC,
    ]);
  });

  it('gives function keys, with Shift, Ctrl and Alt', () => {
    expect(key('F1', 'F1')).toBe(F1);
    expect(key('F10', 'F10')).toBe(F10);
    expect(key('F1', 'F1', { shiftKey: true })).toBe(SF1);
    expect(key('F9', 'F9', { ctrlKey: true })).toBe(CF9);
    expect(key('F4', 'F4', { altKey: true })).toBe(AF4);
  });

  it('gives Ctrl+letter as control codes', () => {
    expect(key('x', 'KeyX', { ctrlKey: true })).toBe(CX);
    expect(key('q', 'KeyQ', { ctrlKey: true })).toBe(CQ);
  });

  it('ignores keys DOS never saw', () => {
    expect(key('Shift', 'ShiftLeft', { shiftKey: true })).toBe(null);
    expect(key('é', 'KeyE')).toBe(null);
  });
});

describe('KeySys', () => {
  it('runs the function for a key of its group, by index', async () => {
    const pkeys = new KeyGroup(KP4, KP6, KP8, KP2, ENTER, BKSP);
    const ks = new KeySys(pkeys);
    ks.iFunc(1, () => 7);
    expect(await ks.key(KP6, null)).toEqual({ used: true, funcret: 7, index: 1 });
    expect(await ks.key(KP4, null)).toEqual({ used: false, funcret: 0, index: 0 }); // no function
    expect(await ks.key(97, null)).toEqual({ used: false, funcret: 0, index: -1 });
  });

  it('follows the group when its keys are copied from a player', async () => {
    const pkeys = new KeyGroup(KP4, KP6, KP8, KP2, ENTER, BKSP);
    const ks = new KeySys(pkeys);
    ks.iFunc(0, () => 1);
    pkeys.copy(new KeyGroup(...'adwseq'.split('').map((c) => c.charCodeAt(0))));
    expect((await ks.key(97, null)).used).toBe(true);
    expect((await ks.key(KP4, null)).index).toBe(-1);
  });

  it('skips disabled keys', async () => {
    const ks = new KeySys(new KeyGroup(ENTER));
    ks.iFunc(0, () => 1);
    ks.iEnable(0, false);
    expect((await ks.key(ENTER, null)).used).toBe(false);
  });
});
