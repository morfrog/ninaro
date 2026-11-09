// The game's state: NINARO.C's globals, as set up by ProgInit.

import { newWin, type Win } from '../core/connect';
import { Grid } from '../core/grid';
import { PlayerGroup, PlayerType, playerNew } from '../core/player';
import { Color } from '../render/palette';
import { NOBORDER, TextBox } from '../ui/textbox';

/** Legacy goes-first enum. */
export const GoesFirst = { One: 0, Two: 1, Loser: 2, Winner: 3, Successive: 4, Random: 5 } as const;
export type GoesFirst = (typeof GoesFirst)[keyof typeof GoesFirst];

/** Player group slots (ProgInit): 2 humans, 5 AIs and a copy slot. */
export const PLAYER_SLOTS = {
  one: 0,
  two: 1,
  aiRandom: 2,
  aiNovice: 3,
  aiNormal: 4,
  aiExpert: 5,
  aiGenius: 6,
  copy: 7,
} as const;

/** ColAdd: a player's second colour is their colour + 2. */
export const COL_ADD = 2;

export class GameState {
  readonly players = new PlayerGroup([
    playerNew('Player One', Color.YELLOW, PlayerType.Human),
    playerNew('Player Two', Color.RED, PlayerType.Human),
    playerNew('AI Random', Color.ORANGE, PlayerType.Computer),
    playerNew('AI Novice', Color.GREEN, PlayerType.Computer),
    playerNew('AI Normal', Color.BLUE, PlayerType.Computer),
    playerNew('AI Expert', Color.YELLOW, PlayerType.Computer),
    playerNew('AI Genius', Color.PURPLE, PlayerType.Computer),
    playerNew('Empty AI', Color.GREEN, PlayerType.Computer),
  ]);

  gridCols = 7;
  gridRows = 8;
  win: Win = newWin(4);
  winOccurred = false;
  isDraw = false;
  /**
   * The token on its way down (not drawn in the grid until it lands). `y` is
   * its centre in board rows, from -0.5 (the top edge) to row + 0.5.
   */
  falling: { col: number; row: number; y: number; color: number } | null = null;
  /** The column the entry token is over. */
  keypos = 0;
  playgrid = new Grid(7, 8);
  entrygrid = new Grid(7, 1);

  playerOne: number = PLAYER_SLOTS.one;
  playerTwo: number = PLAYER_SLOTS.two;
  playerCurrent: number = PLAYER_SLOTS.one;
  playerOneScore = 0;
  playerTwoScore = 0;
  gamesPlayed = 0;
  playerWinner = -1;
  /** Whose move function the copy slot (7) has: PlayerCopy copied getmove. */
  copyOf: number = PLAYER_SLOTS.aiGenius;

  goesFirst: GoesFirst = GoesFirst.Successive;
  wentFirst: number = PLAYER_SLOTS.one;
  bestOf = 0;
  aiSpeed = 2;
  aiDelay = 500;
  mute = false;

  hilite: number = Color.GREEN;
  text: number = Color.WHITE;
  back: number = Color.BLACK;

  readonly msgbox = new TextBox(7, Color.RED, () => this.text, () => this.hilite, 30);
  readonly statusbox = new TextBox(3, NOBORDER, () => this.text, () => this.hilite, 30);

  constructor() {
    this.resetMessages();
    for (let i = 0; i < 3; i++) this.statusbox.addLine(' ', -1);
  }

  /** The message box as ProgInit filled it. */
  resetMessages(): void {
    this.msgbox.empty();
    for (const line of ['Press F1 for HELP', 'Press Esc for MENU', 'Which uses arrow keys', 'Press F10 to QUIT', '~Or begin playing...']) {
      this.msgbox.fixedAddLine(line);
    }
  }

  player(slot: number) {
    return this.players.player(slot)!;
  }

  /** ResetGameSize, without the player and menu side effects. */
  resetGameSize(cols: number, rows: number, inARow: number): void {
    this.gridCols = cols;
    this.gridRows = rows;
    this.keypos = Math.trunc((cols - 1) / 2); // as Game's startColumn
    this.winOccurred = false;
    this.win = newWin(inARow);
    this.playgrid = new Grid(cols, rows);
    this.entrygrid = new Grid(cols, 1);
    this.entrygrid.insertTokenAtCR(this.keypos, 0, this.player(this.playerCurrent));
  }

  /** GameStatus: the three status lines. */
  gameStatus(): void {
    const pad = (n: number, width: number) => String(n).padStart(width);
    this.statusbox.setLine(1, 'Columns  Rows  Connect  Sound');
    this.statusbox.setLine(
      2,
      `~${pad(this.gridCols, 5)}  ${pad(this.gridRows, 4)}  ${pad(this.win.tokens, 6)}      ${this.mute ? 'Off' : 'On'}`,
    );
    this.statusbox.setLine(3, 'Type    Player      Score/Games');
  }
}
