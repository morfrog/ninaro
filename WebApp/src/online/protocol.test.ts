import { describe, expect, it } from 'vitest';
import { safeValue } from './kvStore';
import {
  boardText, cleanRoom, encodeMessage, messageVersion, newNonce, parseMessage, roomKey, sameBoard, type Message,
} from './protocol';

const base: Message = {
  role: 'g',
  nonce: 'k3f9',
  board: { cols: 7, rows: 8, connect: 4 },
  color: 12,
  phase: 'P',
  game: 2,
  move: 11,
  col: 5,
  beat: 0,
  first: null,
};

describe('encodeMessage', () => {
  it('writes the plan\'s format', () => {
    expect(encodeMessage(base)).toBe('1,g,k3f9,7x8x4,c,P,2,11,5,0');
  });

  it('puts who goes first in the move field of a start message', () => {
    expect(encodeMessage({ ...base, role: 'h', phase: 'S', game: 1, move: 0, col: 0, first: 'g' })).toBe('1,h,k3f9,7x8x4,c,S,1,g,0,0');
  });

  it('keeps the longest message within 64 safe characters', () => {
    const big = { ...base, board: { cols: 50, rows: 50, connect: 50 }, game: 999_999, move: 999_999, col: 49, beat: 999_999 };
    const text = encodeMessage(big);
    expect(text.length).toBeLessThanOrEqual(64);
    expect(safeValue(text)).toBe(true);
  });

  it('refuses fields out of range', () => {
    expect(() => encodeMessage({ ...base, nonce: 'AB12' })).toThrow();
    expect(() => encodeMessage({ ...base, board: { cols: 2, rows: 8, connect: 4 } })).toThrow();
    expect(() => encodeMessage({ ...base, board: { cols: 7, rows: 8, connect: 9 } })).toThrow();
    expect(() => encodeMessage({ ...base, color: 16 })).toThrow();
    expect(() => encodeMessage({ ...base, col: 7 })).toThrow();
    expect(() => encodeMessage({ ...base, move: -1 })).toThrow();
    expect(() => encodeMessage({ ...base, beat: 1_000_000 })).toThrow();
    expect(() => encodeMessage({ ...base, first: 'h' })).toThrow();
    expect(() => encodeMessage({ ...base, phase: 'S' })).toThrow();
  });
});

describe('parseMessage', () => {
  it('reads back what encodeMessage wrote', () => {
    for (const m of [
      base,
      { ...base, role: 'h' as const, phase: 'W' as const, game: 0, move: 0, col: 0, beat: 37, color: 0 },
      { ...base, phase: 'S' as const, move: 0, first: 'h' as const },
      { ...base, phase: 'X' as const },
      { ...base, phase: 'Q' as const, board: { cols: 12, rows: 13, connect: 6 }, col: 11, color: 15 },
    ]) {
      expect(parseMessage(encodeMessage(m))).toEqual(m);
    }
  });

  it('rejects anything that is not a valid message', () => {
    for (const text of [
      '',
      '"x"',
      '1,2,5',
      '2,g,k3f9,7x8x4,c,P,2,11,5,0', // another version
      '1,x,k3f9,7x8x4,c,P,2,11,5,0',
      '1,g,K3F9,7x8x4,c,P,2,11,5,0',
      '1,g,k3f9,7x8,c,P,2,11,5,0',
      '1,g,k3f9,2x8x4,c,P,2,11,5,0',
      '1,g,k3f9,7x8x4,g,P,2,11,5,0',
      '1,g,k3f9,7x8x4,c,Z,2,11,5,0',
      '1,g,k3f9,7x8x4,c,P,02,11,5,0',
      '1,g,k3f9,7x8x4,c,P,2,11,7,0', // column off the board
      '1,g,k3f9,7x8x4,c,P,2,h,5,0', // first player outside a start message
      '1,g,k3f9,7x8x4,c,S,2,3,5,0', // a start message without first player
      '1,g,k3f9,7x8x4,c,P,2,11,5,0,extra',
    ]) {
      expect(parseMessage(text), text).toBeNull();
    }
  });
});

describe('messageVersion', () => {
  it('reads the version of any message, ours or not', () => {
    expect(messageVersion('1,g,k3f9,7x8x4,c,P,2,11,5,0')).toBe(1);
    expect(messageVersion('2,anything')).toBe(2);
    expect(messageVersion('')).toBeNull();
    expect(messageVersion('x,1')).toBeNull();
  });
});

describe('boards', () => {
  it('compares and shows them', () => {
    expect(sameBoard({ cols: 7, rows: 8, connect: 4 }, { cols: 7, rows: 8, connect: 4 })).toBe(true);
    expect(sameBoard({ cols: 7, rows: 8, connect: 4 }, { cols: 8, rows: 7, connect: 4 })).toBe(false);
    expect(boardText({ cols: 12, rows: 13, connect: 6 })).toBe('12x13@6');
  });
});

describe('newNonce', () => {
  it('makes 4 base-36 characters', () => {
    expect(newNonce(() => 0)).toBe('0000');
    expect(newNonce(() => 0.99999)).toBe('zzzz');
    expect(newNonce()).toMatch(/^[0-9a-z]{4}$/);
  });
});

describe('rooms', () => {
  it('cleans what the player typed', () => {
    expect(cleanRoom('smithy')).toBe('smithy');
    expect(cleanRoom('  Smithy & Co. ')).toBe('smithy-co');
    expect(cleanRoom('a'.repeat(30))).toBe('a'.repeat(24));
    expect(cleanRoom('abcdefghijklmnopqrstuvw-xyz')).toBe('abcdefghijklmnopqrstuvw');
    expect(cleanRoom('')).toBe('ninaro');
    expect(cleanRoom('!!!')).toBe('ninaro');
  });

  it('makes one key per side', () => {
    expect(roomKey('Smithy', 'h')).toBe('ninaro1-smithy-h');
    expect(roomKey('Smithy', 'g')).toBe('ninaro1-smithy-g');
  });
});
