// PC speaker sounds (Legacy SOUND.C): sound(Hz) and delay(ms) in a loop,
// played here as a square wave. Like the original, each call lasts as long
// as its sound (the returned promise resolves then), so the game's timing
// is the same; when muted, it returns at once, as the original did.

export interface Note {
  hz: number;
  ms: number;
}

/** Buzz's notes: pitch1 and pitch2 alternate, half of pitchhold each, until done > length. */
export function buzzNotes(pitch1: number, pitch2: number, pitchHold: number, length: number): Note[] {
  const hold = Math.trunc(pitchHold / 2);
  const notes: Note[] = [];
  let done = 0;
  for (;;) {
    notes.push({ hz: pitch1, ms: hold }, { hz: pitch2, ms: hold });
    done += pitchHold;
    if (done > length) return notes;
  }
}

/** PlayTune's notes, from (note, length) pairs. */
export function tuneNotes(...pairs: number[]): Note[] {
  const notes: Note[] = [];
  for (let i = 0; i + 1 < pairs.length; i += 2) notes.push({ hz: pairs[i], ms: pairs[i + 1] });
  return notes;
}

export class Speaker {
  private ctx: AudioContext | null = null;
  muted = false;

  /** Browsers only allow sound after a key press or click; call from one. */
  unlock(): void {
    if (this.ctx || typeof AudioContext === 'undefined') {
      void this.ctx?.resume();
      return;
    }
    this.ctx = new AudioContext();
  }

  /** Plays the notes back to back and resolves when they are done. */
  play(notes: readonly Note[]): Promise<void> {
    const total = notes.reduce((sum, n) => sum + n.ms, 0);
    if (this.muted || total <= 0) return Promise.resolve();
    const ctx = this.ctx;
    if (ctx && ctx.state === 'running') {
      const osc = ctx.createOscillator();
      const gain = ctx.createGain();
      osc.type = 'square';
      gain.gain.value = 0.08;
      osc.connect(gain).connect(ctx.destination);
      let t = ctx.currentTime;
      for (const n of notes) {
        osc.frequency.setValueAtTime(Math.max(20, n.hz), t);
        t += n.ms / 1000;
      }
      osc.start();
      osc.stop(t);
    }
    return new Promise((resolve) => setTimeout(resolve, total));
  }

  /** Buzz */
  buzz(pitch1: number, pitch2: number, pitchHold: number, length: number): Promise<void> {
    return this.play(buzzNotes(pitch1, pitch2, pitchHold, length));
  }

  /** PlayTune */
  playTune(...pairs: number[]): Promise<void> {
    return this.play(tuneNotes(...pairs));
  }
}
