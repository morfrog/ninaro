import { describe, expect, it } from 'vitest';
import { ATEND, ATSTART, NOBORDER, TextBox } from './textbox';

const box = (lines: number, width = 30) => new TextBox(lines, NOBORDER, () => 7, () => 4, width);

describe('TextBox', () => {
  it('fixedAddLine fills, then drops the oldest line', () => {
    const tb = box(3);
    for (const s of ['a', 'b', 'c', 'd']) tb.fixedAddLine(s);
    expect(tb.visibleLines()).toEqual(['b', 'c', 'd']);
    expect(tb.size).toBe(3);
  });

  it('cuts lines to the box width', () => {
    const tb = box(2, 5);
    tb.fixedAddLine('Successive');
    expect(tb.line(1)).toBe('Succe');
  });

  it('adds after a line, at the start and at the end', () => {
    const tb = box(10);
    tb.addLine('b', ATEND);
    tb.addLine('d', ATEND);
    expect(tb.addLine('c', 1)).toBe(true);
    tb.addLine('a', ATSTART);
    expect(tb.addLine('e', 99)).toBe(false);
    expect([1, 2, 3, 4, 5].map((n) => tb.line(n))).toEqual(['a', 'b', 'c', 'd', 'e']);
  });

  it('keeps showing the same first line when a line is added at the start', () => {
    const tb = box(2);
    tb.addLine('b', ATEND);
    tb.addLine('a', ATSTART);
    expect(tb.visibleLines()).toEqual(['b']);
  });

  it('deletes a line, moving the rest up (as "Press ESC To Continue" is removed)', () => {
    const tb = box(7);
    for (const s of ['1', '2', '3', '4', '5', '6', '7']) tb.fixedAddLine(s);
    expect(tb.delLine(7)).toBe(true);
    expect(tb.delLine(7)).toBe(false);
    expect(tb.visibleLines()).toEqual(['1', '2', '3', '4', '5', '6']);
  });

  it('scrolls within the lines', () => {
    const tb = box(2);
    for (const s of ['a', 'b', 'c', 'd']) tb.addLine(s, ATEND);
    expect(tb.scroll(-1)).toBe(false);
    expect(tb.scroll(2)).toBe(true);
    expect(tb.visibleLines()).toEqual(['c', 'd']);
    expect(tb.scroll(5)).toBe(true);
    expect(tb.atBottom()).toBe(true);
    expect(tb.visibleLines()).toEqual(['d']);
    expect(tb.scrollEnd()).toBe(true);
    expect(tb.visibleLines()).toEqual(['c', 'd']);
    expect(tb.scroll(0)).toBe(false);
    expect(tb.atTop()).toBe(true);
  });

  it('finds the next line containing a string, wrapping around', () => {
    const tb = box(2);
    for (const s of ['~The Game Menu', 'x', '~The Board Menu', 'y']) tb.addLine(s, ATEND);
    expect(tb.scrollString('~The Board Menu')).toBe(true);
    expect(tb.visibleLines()[0]).toBe('~The Board Menu');
    expect(tb.scrollString('~The Game Menu')).toBe(true);
    expect(tb.atTop()).toBe(true);
    expect(tb.scrollString('missing')).toBe(false);
  });

  it('scrolls to a line number', () => {
    const tb = box(2);
    for (const s of ['a', 'b', 'c']) tb.addLine(s, ATEND);
    expect(tb.scrollLine(3)).toBe(true);
    expect(tb.visibleLines()).toEqual(['c']);
    expect(tb.scrollLine(4)).toBe(false);
  });
});
