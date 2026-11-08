// Test stand-ins for the canvas: a context that accepts every drawing call
// and does nothing, and a saved-screen page that does nothing.

import { Graph } from './render/graph';
import type { MemPage } from './render/memPage';
import { Palette } from './render/palette';

/** A CanvasRenderingContext2D whose methods all do nothing. */
export function nullContext(): CanvasRenderingContext2D {
  const target: Record<string | symbol, unknown> = {};
  return new Proxy(target, {
    get: (t, key) => (key in t ? t[key] : () => null),
    set: (t, key, value) => {
      t[key] = value;
      return true;
    },
  }) as unknown as CanvasRenderingContext2D;
}

export function nullGraph(): Graph {
  return new Graph(nullContext(), new Palette());
}

export function nullPage(): MemPage {
  return { get() {}, put() {}, valid: true } as unknown as MemPage;
}
