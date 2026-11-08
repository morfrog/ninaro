import { describe, expect, it } from 'vitest';
import { Input } from '../input/input';
import { ENTER, ESC, KP2, KP4, KP6, KP8, BKSP, KeyGroup } from '../input/keys';
import { nullGraph, nullPage } from '../testFakes';
import { Menu } from './menu';

function makeMenu() {
  const ran: string[] = [];
  const menu = new Menu(nullGraph(), { x1: 0, y1: 0, x2: 639, y2: 349 }, nullPage(), new KeyGroup(KP4, KP6, KP8, KP2, ENTER, BKSP), () => 4);
  const item = (name: string, enabled = true) => ({ name, shortcut: -1, run: () => void ran.push(name), enabled });
  menu.addSub('Game', [item('New', false), item('Load'), item('Save'), item('End')]);
  menu.addSub('Board', [item('7x8x4'), item('12x13x6'), item('Custom...')]);
  menu.addSub('Player', [item('One...', false), item('Two...'), item('Options...'), item('Reset Scores')]);
  return { menu, ran };
}

const tick = () => new Promise((r) => setTimeout(r, 0));

describe('Menu', () => {
  it('starts each menu on its last enabled item', () => {
    const { menu } = makeMenu();
    expect(menu.subs.map((s) => s.current)).toEqual([3, 2, 3]);
  });

  it('moves up and down past disabled items, and stops at the ends', () => {
    const { menu } = makeMenu();
    menu.current = 2;
    menu.subs[2].current = 1;
    expect(menu.up()).toBe(0); // "One..." is disabled
    expect(menu.down()).toBe(1);
    expect(menu.subs[2].current).toBe(2);
    menu.itemEnable(2, 3, false);
    expect(menu.down()).toBe(0);
  });

  it('only makes an enabled item current', () => {
    const { menu } = makeMenu();
    expect(menu.subCurrentSet(0, 0)).toBe(false);
    menu.itemEnable(0, 0, true);
    expect(menu.subCurrentSet(0, 0)).toBe(true);
    expect(menu.subs[0].current).toBe(0);
  });

  it('runs the chosen item from the keys', async () => {
    const { menu, ran } = makeMenu();
    const input = new Input();
    const call = menu.call(input, async () => {}, true);
    input.pushKey(KP6); // Board
    input.pushKey(KP8); // 12x13x6
    input.pushKey(ENTER);
    await call;
    expect(ran).toEqual(['12x13x6']);
  });

  it('closes on Esc without running anything', async () => {
    const { menu, ran } = makeMenu();
    const input = new Input();
    const call = menu.call(input, async () => {}, true);
    input.pushKey(ESC);
    await call;
    expect(ran).toEqual([]);
  });

  it('runs an item clicked, and closes on a click away from it', async () => {
    const { menu, ran } = makeMenu();
    const input = new Input();
    let call = menu.call(input, async () => {}, true);
    await tick();
    input.pushButton(1, 300, 5); // on the bar over "Board" (the middle third): opens it
    await tick();
    expect(menu.current).toBe(1);
    input.pushButton(1, 300, 30 + 15); // its first item
    await call;
    expect(ran).toEqual(['7x8x4']);
    call = menu.call(input, async () => {}, true);
    await tick();
    input.pushButton(1, 600, 300);
    await call;
    expect(ran).toEqual(['7x8x4']);
  });

  it('shows help for F1 and stays open', async () => {
    const { menu } = makeMenu();
    const input = new Input();
    let helped = 0;
    const call = menu.call(input, async () => void helped++, true);
    input.pushKey(256 + 59);
    await tick();
    input.pushKey(ESC);
    await call;
    expect(helped).toBe(1);
  });
});
