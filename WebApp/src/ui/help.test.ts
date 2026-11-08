import { readFileSync } from 'node:fs';
import { resolve } from 'node:path';
import { describe, expect, it } from 'vitest';
import { Input } from '../input/input';
import { BKSP, ENTER, ESC, KP1, KP2, KP4, KP6, KP7, KP8, KeyGroup } from '../input/keys';
import { nullGraph, nullPage } from '../testFakes';
import { Help, helpLines } from './help';

const HLP = readFileSync(resolve(__dirname, '../../../Legacy/NINARO.HLP'), 'latin1');

describe('helpLines', () => {
  it('splits at carriage returns, skipping the line feed', () => {
    expect(helpLines('~Title\r\n\r\nText\r\n')).toEqual(['~Title', '', 'Text']);
  });

  it('reads a file stored with LF line endings the same way', () => {
    expect(helpLines('~Title\n\nText\n')).toEqual(['~Title', '', 'Text']);
  });

  it('cuts long lines at 79 characters, losing the next one as the original did', () => {
    const long = 'a'.repeat(79) + 'Xbc\r\n';
    expect(helpLines(long)).toEqual(['a'.repeat(79), 'bc']);
  });

  it('reads the help file with its headings', () => {
    const lines = helpLines(HLP);
    expect(lines[0]).toMatch(/^~Ninaro User Manual/);
    for (const h of ['~The Game Menu', '~The Board Menu', '~The Player Menu', '~The Help Menu', '~The Player Dialog', '~The Custom Board Dialog', '~The Player Options Dialog', '~The Game Options Dialog']) {
      expect(lines.some((l) => l.includes(h)), h).toBe(true);
    }
  });
});

describe('Help', () => {
  const make = () => new Help(nullGraph(), HLP, new KeyGroup(KP4, KP6, KP8, KP2, ENTER, BKSP), 20, 1, () => 7, () => 4, 60);

  it('scrolls to a heading for context help', () => {
    const help = make();
    expect(help.scrollString('~The Player Dialog')).toBe(true);
    expect(help.tb.visibleLines()[0]).toContain('~The Player Dialog');
    help.home();
    expect(help.tb.atTop()).toBe(true);
  });

  it('scrolls by page and line with the player keys, and closes on Esc', async () => {
    const help = make();
    const input = new Input();
    const call = help.call({ x1: 10, y1: 10, x2: 629, y2: 339 }, nullPage(), input);
    input.pushKey(KP6); // a page down
    input.pushKey(KP2); // a line down
    await new Promise((r) => setTimeout(r, 0));
    expect(help.tb.visibleLines()[0]).toBe(help.tb.line(22));
    input.pushKey(KP1); // the end
    input.pushKey(KP7); // the top
    input.pushKey(ESC);
    await call;
    expect(help.tb.atTop()).toBe(true);
  });

  it('scrolls with the wheel and closes on a click', async () => {
    const help = make();
    const input = new Input();
    const call = help.call({ x1: 10, y1: 10, x2: 629, y2: 339 }, nullPage(), input);
    input.pushWheel(3);
    await new Promise((r) => setTimeout(r, 0));
    expect(help.tb.visibleLines()[0]).toBe(help.tb.line(4));
    input.pushButton(1, 100, 100);
    await call;
  });
});
