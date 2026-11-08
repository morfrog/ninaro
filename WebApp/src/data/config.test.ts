import { describe, expect, it } from 'vitest';
import { intToByteString } from '../core/binfile';
import { Input } from '../input/input';
import type { ViewSys } from '../render/viewsys';
import { nullGraph } from '../testFakes';
import { Game } from '../game/game';
import { GameState, GoesFirst, PLAYER_SLOTS } from '../game/state';
import { decodeConfig, encodeConfig } from './config';

function makeGame() {
  const vs = { windows: [], display: () => true, windowChanged: () => {} } as unknown as ViewSys;
  return new Game(new GameState(), nullGraph(), vs, new Input());
}

const bytes = (...ints: number[]) => new Uint8Array(ints.flatMap(intToByteString));

/** The 1998 release's NINARO.CFG: AI Random v AI Genius at the fastest speed, mouse configured. */
const RELEASE_CFG = bytes(7, 8, 4, 4, 6, 0, 500, 2, 6, 0, 0, 4, 2, 0, 1, 37683200, 8847360, 2, 1, 6, 4, 3, 2, 5, 4);

describe('NINARO.CFG', () => {
  it("reads the release's file", () => {
    const game = makeGame();
    expect(decodeConfig(RELEASE_CFG, game)).toBe(true);
    const s = game.state;
    expect([s.gridCols, s.gridRows, s.win.tokens]).toEqual([7, 8, 4]);
    expect(s.goesFirst).toBe(GoesFirst.Successive);
    expect([s.aiSpeed, s.aiDelay, s.bestOf]).toEqual([0, 500, 0]);
    expect([s.playerOne, s.playerTwo]).toEqual([PLAYER_SLOTS.aiRandom, PLAYER_SLOTS.aiGenius]);
    expect(s.hilite).toBe(4);
    expect(game.mouseUsage).toBe(2);
    expect(s.players.players.map((p) => p.color)).toEqual([2, 1, 6, 4, 3, 2, 5, 4]);
  });

  it('writes the same 100-byte layout as SaveCfg', () => {
    const game = makeGame();
    decodeConfig(RELEASE_CFG, game);
    const out = encodeConfig(game);
    expect(out.length).toBe(100);
    // Mouse calibration isn't kept (no x, y), so the colours follow directly.
    expect(out).toEqual(bytes(7, 8, 4, 4, 6, 0, 500, 2, 6, 0, 0, 4, 2, 0, 0, 2, 1, 6, 4, 3, 2, 5, 4, 0, 0));
    const again = makeGame();
    expect(decodeConfig(out, again)).toBe(true);
    expect(encodeConfig(again)).toEqual(out);
  });

  it('keeps settings changed in the game', () => {
    const game = makeGame();
    const s = game.state;
    s.gridCols = 12;
    s.gridRows = 13;
    s.win.tokens = 6;
    s.mute = true;
    s.back = 7;
    s.bestOf = 25;
    s.player(0).color = 5;
    const again = makeGame();
    decodeConfig(encodeConfig(game), again);
    expect([again.state.gridCols, again.state.gridRows, again.state.win.tokens, again.state.mute, again.state.back, again.state.bestOf]).toEqual([
      12, 13, 6, true, 7, 25,
    ]);
    expect(again.state.player(0).color).toBe(5);
    expect(again.speaker.muted).toBe(true);
  });

  it('rebuilds the copy slot when both players were the same AI', () => {
    const game = makeGame();
    game.setP1(PLAYER_SLOTS.aiExpert);
    game.setP2(PLAYER_SLOTS.copy);
    const again = makeGame();
    decodeConfig(encodeConfig(game), again);
    expect(again.state.player(PLAYER_SLOTS.copy).name).toBe('AI Expert');
    expect(again.state.copyOf).toBe(PLAYER_SLOTS.aiExpert);
  });

  it('refuses a damaged file and changes nothing', () => {
    const game = makeGame();
    expect(decodeConfig(bytes(7, 8), game)).toBe(false);
    expect(decodeConfig(bytes(99, 8, 4, 4, 0, 2, 500, 0, 1, 0, 0, 4, 2, 0, 0, 2, 1, 6, 4, 3, 2, 5, 4), game)).toBe(false);
    expect(game.state.gridCols).toBe(7);
  });
});
