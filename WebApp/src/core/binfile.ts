// Byte buffers for Ninaro's files (Legacy BINFILE.C). Ints are 4 bytes,
// most significant first (SYS.C IntToByteString / ByteStringToInt).
// Used for NINARO.TTL (the title picture) and the original NINARO.CFG and
// NINARO.SAV layouts. The unused "encryption" functions are not ported.

export class BinFile {
  readonly store: Uint8Array;
  /** Position of the next put or get (the C's bf->bytes). */
  bytes = 0;

  /** BinFileNew (size bytes of zeros), or BinFileRead of existing bytes. */
  constructor(sizeOrData: number | Uint8Array) {
    this.store = typeof sizeOrData === 'number' ? new Uint8Array(sizeOrData) : sizeOrData;
  }

  get size(): number {
    return this.store.length;
  }

  /** BinFileRemain */
  remain(): number {
    return this.size - this.bytes;
  }

  /** BinFilePut: all the bytes, or nothing if they don't fit. */
  put(element: ArrayLike<number>): boolean {
    if (this.remain() < element.length) return false;
    for (let x = 0; x < element.length; x++) this.store[this.bytes++] = element[x] & 0xff;
    return true;
  }

  /** BinFileGet: up to count bytes, as many as remain. */
  get(count: number): Uint8Array {
    const get = Math.min(this.remain(), count);
    const out = this.store.slice(this.bytes, this.bytes + get);
    this.bytes += get;
    return out;
  }

  /** BinFilePutInt */
  putInt(element: number): boolean {
    return this.put(intToByteString(element));
  }

  /**
   * BinFileGetInt: -1 at the end of the file. A partial int (1-3 bytes left)
   * used whatever was in the C's stack buffer; here the missing bytes are 0.
   */
  getInt(): number {
    const got = this.get(4);
    if (got.length === 0) return -1;
    const buffer = new Uint8Array(4);
    buffer.set(got);
    return byteStringToInt(buffer);
  }
}

/** SYS.C IntToByteString */
export function intToByteString(value: number): number[] {
  const out: number[] = [];
  for (let x = 0; x < 4; x++) out.push((value >> ((3 - x) * 8)) & 0xff);
  return out;
}

/** SYS.C ByteStringToInt, with Watcom's unsigned char, wrapping at 32 bits. */
export function byteStringToInt(array: ArrayLike<number>): number {
  let ret = 0;
  for (let x = 0; x < 4; x++) ret = (ret + ((array[x] & 0xff) << ((3 - x) * 8))) | 0;
  return ret;
}
