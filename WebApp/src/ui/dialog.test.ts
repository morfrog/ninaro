import { describe, expect, it } from 'vitest';
import { Input } from '../input/input';
import { BKSP, ENTER, ESC, KP2, KP4, KP6, KP8, KeyGroup } from '../input/keys';
import { nullGraph, nullPage } from '../testFakes';
import { Control, Dialog, Show, positionAt } from './dialog';

const keys = () => new KeyGroup(KP4, KP6, KP8, KP2, ENTER, BKSP);
const VP = { x1: 50, y1: 50, x2: 590, y2: 300 };

function makeDialog(change: Dialog['change'] = null) {
  return new Dialog(nullGraph(), keys(), change, 'Test', () => 4, [
    { name: 'Size', enabled: true, selectable: true, store: Control.range(Show.Slider, 1, 1, 10, 3, '', 9, '') },
    { name: 'Note', enabled: true, selectable: false, store: Control.list(Show.Name, 1, 1, 10, [{ value: 0, name: 'x' }]) },
    { name: 'Pick', enabled: true, selectable: true, store: Control.list(Show.Radio, 1, 3, 8, [
      { value: 10, name: 'A' },
      { value: 20, name: 'B' },
      { value: 30, name: null },
    ]) },
  ]);
}

async function run(d: Dialog, ...pushes: number[]) {
  const input = new Input();
  const call = d.call(VP, nullPage(), 5, 8, async () => {}, input, true);
  for (const k of pushes) input.pushKey(k);
  return call;
}

describe('Control', () => {
  it('moves a range within its limits', () => {
    const c = Control.range(Show.Slider, 1, 1, 10, 3, '', 5, '');
    expect(c.current).toBe(3);
    expect(c.prev()).toBe(false);
    expect(c.next() && c.next()).toBe(true);
    expect(c.next()).toBe(false);
    expect(c.position()).toBe(2);
  });

  it("stops a list at an element with no name", () => {
    const c = Control.list(Show.Radio, 1, 3, 8, [
      { value: 10, name: 'A' },
      { value: 20, name: 'B' },
      { value: 30, name: null },
    ]);
    expect(c.next()).toBe(true);
    expect(c.next()).toBe(false);
    expect(c.currentSetTo(20)).toBe(true);
    expect(c.currentSetTo(99)).toBe(false);
    expect(c.valueGet(c.current)).toBe(20);
  });

  it("can't be set outside its range (so a range of 3..0 keeps 3)", () => {
    const c = Control.range(Show.Slider, 1, 1, 10, 3, '', 0, '');
    expect(c.currentSet(4)).toBe(false);
    expect(c.current).toBe(3);
  });
});

describe('Dialog', () => {
  it('starts on the buttons, with OK highlighted', () => {
    const d = makeDialog();
    expect(d.current).toBe(3);
    expect(d.defaultGet()).toBe(0);
  });

  it('skips controls that are not selectable when moving', () => {
    const d = makeDialog();
    d.current = 0;
    expect(d.next()).toBe(true);
    expect(d.current).toBe(2);
    expect(d.prev()).toBe(true);
    expect(d.current).toBe(0);
    expect(d.prev()).toBe(false);
  });

  it('OK with nothing changed counts as cancel', async () => {
    expect(await run(makeDialog(), ENTER)).toBe(false);
  });

  it('OK after a change returns true and keeps it', async () => {
    const d = makeDialog();
    // Up twice to Size (skipping Note), right twice, then O for OK.
    expect(await run(d, KP8, KP8, KP6, KP6, 'o'.charCodeAt(0))).toBe(true);
    expect(d.control(0).current).toBe(5);
  });

  it('Cancel (Esc) puts the values back', async () => {
    const d = makeDialog();
    expect(await run(d, KP8, KP8, KP6, ESC)).toBe(false);
    expect(d.control(0).current).toBe(3);
  });

  it('Reset puts the values back and keeps the dialog open', async () => {
    const changes: number[] = [];
    const d = makeDialog((_, last) => changes.push(last));
    const input = new Input();
    const call = d.call(VP, nullPage(), 5, 8, async () => {}, input, true);
    for (const k of [KP8, KP8, KP6, 'r'.charCodeAt(0)]) input.pushKey(k);
    await new Promise((r) => setTimeout(r, 0));
    expect(d.control(0).current).toBe(3);
    expect(changes).toEqual([1, 1]); // the right move, then Reset's change
    input.pushKey(ESC);
    expect(await call).toBe(false);
  });

  it('chooses a clicked item and presses a clicked button', async () => {
    const d = makeDialog();
    const input = new Input();
    const call = d.call(VP, nullPage(), 5, 8, async () => {}, input, true);
    const pick = d.elems.indexOf(d.elems[2]);
    // Find where "B" was drawn, from the dialog's own layout.
    let target: { x: number; y: number } | null = null;
    for (let y = VP.y1; y <= VP.y2 && !target; y += 2) {
      for (let x = VP.x1; x <= VP.x2 && !target; x += 2) {
        const hit = d.hit(x, y);
        if (hit && hit.control === pick && hit.pos === 1) target = { x, y };
      }
    }
    expect(target).not.toBeNull();
    input.pushButton(1, target!.x, target!.y);
    await new Promise((r) => setTimeout(r, 0));
    expect(d.control(2).current).toBe(1);
    let ok: { x: number; y: number } | null = null;
    for (let y = VP.y2; y >= VP.y1 && !ok; y -= 2) {
      for (let x = VP.x1; x <= VP.x2 && !ok; x += 2) {
        const hit = d.hit(x, y);
        if (hit && hit.control === 3 && hit.pos === 0) ok = { x, y };
      }
    }
    input.pushButton(1, ok!.x, ok!.y);
    expect(await call).toBe(true);
  });
});

describe('positionAt', () => {
  it('finds the nearest slider tick', () => {
    const c = Control.range(Show.Slider, 1, 1, 10, 3, '', 7, '');
    const vp = { x1: 100, y1: 0, x2: 199, y2: 20 };
    // width 100: edge 10, left 110, section 80, 5 ticks 20 apart.
    expect(positionAt(c, vp, 110, 10)).toBe(0);
    expect(positionAt(c, vp, 151, 10)).toBe(2);
    expect(positionAt(c, vp, 199, 10)).toBe(4);
  });
});
