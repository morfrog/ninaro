// The game's side of an online session (ONLINE-PLAN.md): which player is
// Online, the game and move counts, and the opponent's move being waited for.

import { ImmanuelStore, type KvStore } from '../online/kvStore';
import type { Board } from '../online/protocol';
import { OnlineSession, OpponentLeft, type SessionEvent, type SessionTimings } from '../online/session';

/** The store an online game talks through, and (for pagehide) one-try writes. */
export interface OnlineStore {
  store: KvStore;
  putNow?: (key: string, value: string) => Promise<boolean>;
}

/** Makes the store for a session; the signal ends it, onTrouble reports a long run of failures. */
export type OnlineStoreFactory = (signal: AbortSignal, onTrouble: (trouble: boolean) => void) => OnlineStore;

/** keyvalue.immanuel.co */
export const immanuelStore: OnlineStoreFactory = (signal, onTrouble) => {
  const store = new ImmanuelStore({ signal, onTrouble });
  return { store, putNow: (key, value) => store.putNow(key, value) };
};

/** What waiting for the opponent's move came to. */
export type MoveResult = { col: number } | { left: true } | { error: unknown };

export class OnlineLink {
  readonly session: OnlineSession;
  readonly controller = new AbortController();
  /** Handshake done: games can be played. */
  started = false;
  /** The opponent quit; the game loop ends the game. */
  left = false;
  /** Game number in the session (1 for the first). */
  game = 0;
  /** Our moves, and theirs, in this game. */
  myMoves = 0;
  theirMoves = 0;
  private pending: Promise<MoveResult> | null = null;

  constructor(
    readonly room: string,
    /** Which player is Online: 1 or 2. */
    readonly side: 1 | 2,
    /** The player that side had before, to go back to. */
    readonly prevSlot: number,
    board: Board,
    color: number,
    makeStore: OnlineStoreFactory,
    onEvent: (e: SessionEvent) => void,
    onTrouble: (trouble: boolean) => void,
    timings?: Partial<SessionTimings>,
  ) {
    const { store, putNow } = makeStore(this.controller.signal, onTrouble);
    this.session = new OnlineSession({ store, putNow, room, board, color, signal: this.controller.signal, timings, onEvent });
  }

  /** A new game: counts start again. */
  nextGame(): void {
    this.game++;
    this.myMoves = 0;
    this.theirMoves = 0;
    this.pending = null;
  }

  /**
   * The opponent's next move. The same promise until it settles and is
   * taken, so a menu opened while waiting doesn't start a second wait.
   */
  nextMove(): Promise<MoveResult> {
    this.pending ??= this.session.waitMove(this.game, this.theirMoves + 1).then(
      (col): MoveResult => ({ col }),
      (e): MoveResult => (e instanceof OpponentLeft ? { left: true } : { error: e }),
    );
    return this.pending;
  }

  /** The move was used. */
  tookMove(): void {
    this.pending = null;
    this.theirMoves++;
  }

  /** Our move: sent in the background (writes go out in order). */
  sendMove(col: number): void {
    this.myMoves++;
    this.session.sendMove(this.game, this.myMoves, col).catch(() => {});
  }

  /** Leaves: tells the opponent (unless they left first), then stops everything. */
  close(sendQuit: boolean): void {
    this.session.close();
    const stop = () => this.controller.abort(new Error('Left the online game'));
    if (!sendQuit) return stop();
    // The quit gets a few seconds to go through, then everything stops.
    const timer = setTimeout(stop, 5000);
    this.session.quit().then(
      () => {
        clearTimeout(timer);
        stop();
      },
      () => {},
    );
  }

  /** The page is closing: one try at telling the opponent. */
  closeNow(): void {
    this.session.quitNow();
  }
}
