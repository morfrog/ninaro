import { describe, expect, it } from 'vitest';
import { buzzNotes, tuneNotes } from './sound';

describe('Buzz', () => {
  it("alternates the pitches until done passes the length (DropToken's failure buzz)", () => {
    // Buzz(200,400,20,100): hold 10 ms each; done 20, 40, ... 120 > 100: 6 rounds.
    const notes = buzzNotes(200, 400, 20, 100);
    expect(notes).toHaveLength(12);
    expect(notes.slice(0, 2)).toEqual([
      { hz: 200, ms: 10 },
      { hz: 400, ms: 10 },
    ]);
    expect(notes.reduce((t, n) => t + n.ms, 0)).toBe(120);
  });
});

describe('PlayTune', () => {
  it("plays note and length pairs (the win tune)", () => {
    expect(tuneNotes(100, 50, 300, 50, 500, 100, 400, 5)).toEqual([
      { hz: 100, ms: 50 },
      { hz: 300, ms: 50 },
      { hz: 500, ms: 100 },
      { hz: 400, ms: 5 },
    ]);
  });
});
