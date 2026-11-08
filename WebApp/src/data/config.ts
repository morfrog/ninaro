// The configuration file (NINARO.C SaveCfg and LoadCfg): big-endian ints in
// the original NINARO.CFG layout, so the original's own files can be read.

import { newWin } from '../core/connect';
import { BinFile } from '../core/binfile';
import { PLAYER_SLOTS } from '../game/state';
import type { Game } from '../game/game';

/** SaveCfg's buffer: (17 + players) ints; unused ones stay 0 (100 bytes). */
const SIZE_INTS = 17 + 8;

/** SaveCfg */
export function encodeConfig(game: Game): Uint8Array {
  const s = game.state;
  const bf = new BinFile(SIZE_INTS * 4);
  bf.putInt(s.gridCols);
  bf.putInt(s.gridRows);
  bf.putInt(s.win.tokens);
  bf.putInt(s.goesFirst);
  bf.putInt(s.wentFirst);
  bf.putInt(s.aiSpeed);
  bf.putInt(s.aiDelay);
  bf.putInt(s.playerOne);
  bf.putInt(s.playerTwo);
  bf.putInt(s.mute ? 1 : 0);
  bf.putInt(s.back);
  bf.putInt(s.hilite);
  bf.putInt(game.mouseUsage);
  bf.putInt(s.bestOf);
  bf.putInt(0); // mouseconfig: nothing to calibrate in a browser
  for (const p of s.players.players) bf.putInt(p.color);
  return bf.store;
}

/**
 * LoadCfg: applies a NINARO.CFG to the game. False (and nothing changed)
 * if it is too short to be one or holds values the game can't use.
 */
export function decodeConfig(bytes: Uint8Array, game: Game): boolean {
  const bf = new BinFile(bytes);
  const n = () => bf.getInt();
  const cols = n();
  const rows = n();
  const tokens = n();
  const goesFirst = n();
  const wentFirst = n();
  const aiSpeed = n();
  const aiDelay = n();
  const p1 = n();
  const p2 = n();
  const mute = n();
  const back = n();
  const hilite = n();
  const mouseUsage = n();
  const bestOf = n();
  const mouseConfig = n();
  if (mouseConfig) {
    n(); // the mouse's x and y adjustment
    n();
  }
  const colors = game.state.players.players.map(() => n());

  const inRange = (v: number, lo: number, hi: number) => Number.isInteger(v) && v >= lo && v <= hi;
  const ok =
    inRange(cols, 3, 50) &&
    inRange(rows, 3, 50) &&
    inRange(tokens, 3, Math.max(cols, rows)) &&
    inRange(goesFirst, 0, 5) &&
    inRange(wentFirst, 0, 7) &&
    inRange(aiSpeed, 0, 4) &&
    inRange(aiDelay, 0, 5000) &&
    inRange(p1, 0, 7) &&
    inRange(p2, 0, 7) &&
    inRange(back, 0, 15) &&
    inRange(hilite, 0, 15) &&
    inRange(mouseUsage, 0, 2) &&
    inRange(bestOf, 0, 1000) &&
    colors.every((c) => inRange(c, 0, 15));
  if (!ok) return false;

  const s = game.state;
  s.gridCols = cols;
  s.gridRows = rows;
  s.win = newWin(tokens);
  s.goesFirst = goesFirst as typeof s.goesFirst;
  s.wentFirst = wentFirst;
  s.aiSpeed = aiSpeed;
  s.aiDelay = aiDelay;
  game.setP1(p1);
  game.setP2(p2);
  s.mute = mute !== 0;
  game.speaker.muted = s.mute;
  s.back = back;
  s.hilite = hilite;
  game.mouseUsage = mouseUsage;
  s.bestOf = bestOf;
  s.players.players.forEach((p, i) => (p.color = colors[i]));
  // Both players the same AI: the copy slot was a copy of the other player.
  const copySource = s.playerTwo === PLAYER_SLOTS.copy ? s.playerOne : s.playerOne === PLAYER_SLOTS.copy ? s.playerTwo : -1;
  if (copySource >= 0 && copySource !== PLAYER_SLOTS.copy) {
    const copy = s.player(PLAYER_SLOTS.copy);
    const source = s.player(copySource);
    copy.name = source.name;
    copy.type = source.type;
    s.copyOf = copySource;
  }
  return true;
}
