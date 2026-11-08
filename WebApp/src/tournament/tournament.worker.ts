// Runs a tournament off the page's thread, reporting progress.

import { runTournament, type Board } from './runner';

export interface TournamentRequest {
  board: Board;
  games: number;
  seed: number;
}

self.onmessage = (e: MessageEvent<TournamentRequest>) => {
  const { board, games, seed } = e.data;
  const result = runTournament(board, games, seed, (done, total) => self.postMessage({ type: 'progress', done, total }));
  self.postMessage({ type: 'done', result });
};
