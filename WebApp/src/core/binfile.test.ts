import { describe, expect, it } from 'vitest';
import { BinFile, byteStringToInt, intToByteString } from './binfile';

describe('int bytes', () => {
  it('writes most significant byte first', () => {
    expect(intToByteString(500)).toEqual([0, 0, 1, 244]);
    expect(intToByteString(36)).toEqual([0, 0, 0, 36]);
    expect(intToByteString(-1)).toEqual([255, 255, 255, 255]);
  });

  it('reads bytes as unsigned, as Watcom char was', () => {
    expect(byteStringToInt([0, 0, 1, 244])).toBe(500);
    expect(byteStringToInt([0, 0, 0, 200])).toBe(200);
    expect(byteStringToInt([255, 255, 255, 255])).toBe(-1);
    expect(byteStringToInt([0x80, 0, 0, 0])).toBe(-2147483648);
  });

  it('round-trips any 32-bit int', () => {
    for (const v of [0, 1, -1, 255, 256, 65535, -65536, 2147483647, -2147483648]) {
      expect(byteStringToInt(intToByteString(v))).toBe(v);
    }
  });
});

describe('BinFile', () => {
  it('reads an original NINARO.CFG header', () => {
    // The first 12 ints of the 1998 release's NINARO.CFG.
    const bytes = new Uint8Array([7, 8, 4, 4, 6, 0, 500, 2, 6, 0, 0, 4].flatMap(intToByteString));
    const bf = new BinFile(bytes);
    const read = Array.from({ length: 12 }, () => bf.getInt());
    expect(read).toEqual([7, 8, 4, 4, 6, 0, 500, 2, 6, 0, 0, 4]);
    expect(bf.getInt()).toBe(-1); // end of file
  });

  it('refuses a put that does not fit', () => {
    const bf = new BinFile(6);
    expect(bf.putInt(1)).toBe(true);
    expect(bf.putInt(2)).toBe(false);
    expect(bf.remain()).toBe(2);
  });

  it('gets as many bytes as remain', () => {
    const bf = new BinFile(new Uint8Array([1, 2, 3]));
    expect([...bf.get(5)]).toEqual([1, 2, 3]);
    expect(bf.remain()).toBe(0);
  });

  it('reads a partial int with the missing bytes as zero', () => {
    const bf = new BinFile(new Uint8Array([0, 0, 1]));
    expect(bf.getInt()).toBe(256);
  });
});
