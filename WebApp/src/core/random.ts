// Random numbers as Ninaro got them: SYS.C Random(n) = rand() % n, with
// Watcom C/386's rand() (the ANSI C example generator, RAND_MAX 32767).
// The original seeded it from the clock (StartRandom); here the seed can be
// chosen, so games can be repeated and tested.

export interface Rng {
  /** SYS.C Random: 0..num-1. */
  random(num: number): number;
}

export class WatcomRandom implements Rng {
  private next: number;

  /** Watcom's rand() starts as if srand(1) had been called. */
  constructor(seed = 1) {
    this.next = seed >>> 0;
  }

  /** srand */
  seed(seed: number): void {
    this.next = seed >>> 0;
  }

  /** rand(): next = next * 1103515245 + 12345 (mod 2^32), result bits 16..30. */
  rand(): number {
    this.next = (Math.imul(this.next, 1103515245) + 12345) >>> 0;
    return (this.next >>> 16) & 0x7fff;
  }

  random(num: number): number {
    return this.rand() % num;
  }
}

/**
 * SYS.C StartRandom: srand(hundredths * seconds % RAND_MAX) from the DOS
 * clock. Kept for the feel of the original (only 3600 different seeds).
 */
export function startRandomSeed(now = new Date()): number {
  const hsecond = Math.floor(now.getMilliseconds() / 10);
  return (hsecond * now.getSeconds()) % 32767;
}
