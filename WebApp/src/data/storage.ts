// The browser's storage standing in for NINARO.CFG and NINARO.SAV in the
// game's folder. Each file's bytes are kept base64-encoded under a
// versioned key. Every access is guarded: with storage blocked or full the
// game works the same, it just doesn't remember.

export const CONFIG_KEY = 'ninaro:v1:cfg';
export const SAVE_KEY = 'ninaro:v1:save';

/** The part of Storage this needs (so tests can pass a Map-backed one). */
export type KeyValueStore = Pick<Storage, 'getItem' | 'setItem' | 'removeItem'>;

function browserStore(): KeyValueStore | null {
  try {
    return typeof localStorage === 'undefined' ? null : localStorage;
  } catch {
    return null; // e.g. storage disabled for the site
  }
}

export function toBase64(bytes: Uint8Array): string {
  let s = '';
  for (const b of bytes) s += String.fromCharCode(b);
  return btoa(s);
}

export function fromBase64(text: string): Uint8Array {
  const s = atob(text);
  return Uint8Array.from(s, (c) => c.charCodeAt(0));
}

/** A file kept in storage: read (null if missing or unreadable), write (false if it couldn't be), remove. */
export class StoredFile {
  constructor(
    private readonly key: string,
    private readonly store: KeyValueStore | null = browserStore(),
  ) {}

  read(): Uint8Array | null {
    try {
      const text = this.store?.getItem(this.key);
      return text ? fromBase64(text) : null;
    } catch {
      return null;
    }
  }

  write(bytes: Uint8Array): boolean {
    try {
      if (!this.store) return false;
      this.store.setItem(this.key, toBase64(bytes));
      return true;
    } catch {
      return false;
    }
  }

  remove(): void {
    try {
      this.store?.removeItem(this.key);
    } catch {
      // nothing to do
    }
  }
}
