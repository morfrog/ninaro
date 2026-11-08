// The game (Legacy NINARO.C): the main loop, players' moves, dropping
// tokens, wins, draws, scores, sets, demo mode and the function keys.
//
// The C polled the keyboard and mouse in busy loops and called delay();
// here the same code awaits the input queue and sleep(). Menus, dialogs,
// help and the title screen come in phase 4, through GameUi.

import { ruleMove } from '../core/ai';
import { InsertResult, gameLoad, gameSave, insertTokenInCol, setWinSize } from '../core/connect';
import { Direction, Grid } from '../core/grid';
import { PlayerType, type Player } from '../core/player';
import { WatcomRandom, startRandomSeed } from '../core/random';
import { Speaker } from '../audio/sound';
import { Input, LMB, NMB, RMB, sleep } from '../input/input';
import {
  AF4, BKSP, CF9, CQ, CX, ENTER, ESC, F1, F10, F2, F3, F4, F5, F6, F7, F8, F9, KP2, KP4, KP6, KP8, KeyGroup, KeySys,
} from '../input/keys';
import type { Graph } from '../render/graph';
import { Display, type ViewSys } from '../render/viewsys';
import { GameWindow } from './display';
import { GoesFirst, PLAYER_SLOTS, type GameState } from './state';

/** What the game needs from the menus, dialogs, help and title (phase 4). */
export interface GameUi {
  /** MenuCall on the main menu. */
  menuCall(game: Game): Promise<void>;
  /** HelpContentsFunc */
  help(game: Game): Promise<void>;
  /** MenuItemEnable(mainmenu, sub, item, enable) */
  menuItemEnable(sub: number, item: number, enable: boolean): void;
  /** MenuSubCurrentSet(mainmenu, sub, item) */
  menuSubCurrentSet(sub: number, item: number): void;
  /** ExitGame: saves the configuration; the original then quit to DOS. */
  exit(game: Game): Promise<void>;
}

/** A GameUi with no menus yet: the menu and help keys do nothing. */
export const NO_UI: GameUi = {
  menuCall: async () => {},
  help: async () => {},
  menuItemEnable: () => {},
  menuSubCurrentSet: () => {},
  exit: async () => {},
};

/** Hatching meant for Suggest Move's token (SuggestMove's new_mask); never seen in the original. */
const SUGGEST_MASK = [0xa2, 0x15, 0xa2, 0x10, 0xa2, 0x10, 0xa2, 0x15];

export class Game {
  readonly rng = new WatcomRandom();
  readonly speaker = new Speaker();
  readonly pkeys = new KeyGroup(KP4, KP6, KP8, KP2, ENTER, BKSP);
  readonly p1keys = new KeyGroup(...'adwseq'.split('').map((c) => c.charCodeAt(0)));
  readonly p2keys = new KeyGroup(KP4, KP6, KP8, KP2, ENTER, BKSP);
  readonly ks = new KeySys(this.pkeys);
  ingame = false;
  /** 0 none, 1 partial (moves only), 2 full (menus and dialogs too). */
  mouseUsage = 2;
  /** The saved game (NINARO.SAV); kept in storage from phase 5. */
  saved: Uint8Array | null = null;
  ui: GameUi = NO_UI;
  /** Stops run() (for tests). */
  stopped = false;

  constructor(
    readonly state: GameState,
    readonly g: Graph,
    readonly vs: ViewSys,
    readonly input: Input,
  ) {
    this.ks.iFunc(0, () => this.keyLeft());
    this.ks.iFunc(1, () => this.keyRight());
    this.ks.iFunc(2, () => this.keyUp());
    this.ks.iFunc(3, () => this.keyDown());
    this.ks.iFunc(4, () => this.keyDown()); // KeySelect
    this.ks.iFunc(5, () => this.keyUp()); // KeyCancel
  }

  // ---- Players ----

  private player(slot: number): Player {
    return this.state.player(slot);
  }

  currentPlayer(): Player {
    return this.player(this.state.playerCurrent);
  }

  /** OtherPlayer */
  otherPlayer(): Player {
    const s = this.state;
    return this.player(s.playerCurrent === s.playerOne ? s.playerTwo : s.playerOne);
  }

  /** BothPlayerAI */
  bothPlayerAI(): boolean {
    return this.state.playerOne > 1 && this.state.playerTwo > 1;
  }

  /** SetP1 */
  setP1(player: number): void {
    const s = this.state;
    if (s.playerCurrent === s.playerOne) {
      s.playerOne = player;
      s.playerCurrent = s.playerOne;
    } else s.playerOne = player;
  }

  /** SetP2 */
  setP2(player: number): void {
    const s = this.state;
    if (s.playerCurrent === s.playerTwo) {
      s.playerTwo = player;
      s.playerCurrent = s.playerTwo;
    } else s.playerTwo = player;
  }

  /** SetP1_2: both players; player two changes colour if they'd match. */
  setP1_2(p1: number, p2: number): void {
    const player2 = this.player(p2);
    const player2col = player2.color;
    this.state.playerCurrent = p1;
    this.setP1(p1);
    this.setP2(p2);
    if (this.player(p1).color === player2col) {
      player2.color = player2col === 7 ? 1 + this.rng.random(6) : 7;
    }
  }

  /** SetP1Current */
  setP1Current(): void {
    this.state.playerCurrent = this.state.playerOne;
    this.ui.menuItemEnable(2, 0, true);
    this.ui.menuItemEnable(2, 1, false);
    this.ui.menuSubCurrentSet(2, 0);
  }

  /** SetP2Current */
  setP2Current(): void {
    this.state.playerCurrent = this.state.playerTwo;
    this.ui.menuItemEnable(2, 1, true);
    this.ui.menuItemEnable(2, 0, false);
    this.ui.menuSubCurrentSet(2, 1);
  }

  /** NextPlayer */
  nextPlayer(): void {
    if (this.state.playerCurrent === this.state.playerOne) this.setP2Current();
    else this.setP1Current();
  }

  /** ResetScores */
  resetScores(): void {
    const s = this.state;
    s.playerOneScore = 0;
    s.playerTwoScore = 0;
    s.gamesPlayed = 0;
    s.playerWinner = -1;
  }

  // ---- The board ----

  /** ResetGameSize */
  resetGameSize(cols: number, rows: number, inARow: number): void {
    const s = this.state;
    s.gridCols = cols;
    s.gridRows = rows;
    s.keypos = Math.trunc(cols / 2) + (cols % 2);
    s.winOccurred = false;
    setWinSize(s.win, inARow);
    s.playgrid = new Grid(cols, rows);
    s.entrygrid = new Grid(cols, 1);
    s.entrygrid.insertTokenAtCR(s.keypos, 0, this.currentPlayer());
  }

  /** GameSize7x8x4 (Board menu) */
  gameSize7x8x4(): void {
    this.resetGameSize(7, 8, 4);
    this.resetScores();
    this.state.gameStatus();
  }

  /** GameSize12x13x6 (Board menu) */
  gameSize12x13x6(): void {
    this.resetGameSize(12, 13, 6);
    this.resetScores();
    this.state.gameStatus();
  }

  // ---- Keys during a move ----

  /** KeyUp and KeyCancel: the menu. */
  async keyUp(): Promise<number> {
    await this.ui.menuCall(this);
    return Display.DisplayAll;
  }

  /** KeyLeft */
  keyLeft(): number {
    const s = this.state;
    if (s.entrygrid.moveTokenAtCR(s.keypos, 0, Direction.Left)) {
      --s.keypos;
      this.vs.windowChanged(GameWindow.entryGrid, true);
      return Display.DisplayChanged;
    }
    return Display.NoDisplay;
  }

  /** KeyRight */
  keyRight(): number {
    const s = this.state;
    if (s.entrygrid.moveTokenAtCR(s.keypos, 0, Direction.Right)) {
      ++s.keypos;
      this.vs.windowChanged(GameWindow.entryGrid, true);
      return Display.DisplayChanged;
    }
    return Display.NoDisplay;
  }

  /** KeyDown and KeySelect */
  keyDown(): number {
    return 1;
  }

  /** Moves the entry token to col (the mouse's MouseMove). */
  private moveEntryTo(col: number): void {
    const s = this.state;
    if (col === s.keypos) return;
    s.entrygrid.insertTokenAtCR(col, 0, s.entrygrid.removeTokenAtCR(s.keypos, 0));
    s.keypos = col;
    this.vs.windowChanged(GameWindow.entryGrid, true);
  }

  /**
   * The board column under the pointer, or -1. (The original used the
   * pointer's x across the whole screen, with the pointer hidden; here it
   * is the column the visible pointer is over.)
   */
  mouseCol(x: number, y: number): number {
    const board = this.vs.windows[GameWindow.playGrid].vp;
    const entry = this.vs.windows[GameWindow.entryGrid].vp;
    if (x < board.x1 || x > board.x2 || y < entry.y1 || y > board.y2) return -1;
    const width = board.x2 - board.x1 + 1;
    return Math.min(this.state.gridCols - 1, Math.trunc(((x - board.x1) * this.state.gridCols) / width));
  }

  // ---- Moves ----

  /** DropToken: Win, Success or Failure. */
  async dropToken(): Promise<InsertResult> {
    const s = this.state;
    const ret = insertTokenInCol(s.playgrid, s.keypos, this.currentPlayer(), s.win);
    switch (ret) {
      case InsertResult.Success:
        s.entrygrid.removeTokenAtCR(s.keypos, 0);
        this.nextPlayer();
        s.entrygrid.insertTokenAtCR(s.keypos, 0, this.currentPlayer());
        this.vs.windowChanged(GameWindow.entryGrid, true);
        this.vs.windowChanged(GameWindow.playGrid, true);
        if (s.playgrid.tokens() === s.playgrid.maxTokens()) {
          s.msgbox.fixedAddLine("It's A Draw...");
          s.isDraw = true;
          ++s.gamesPlayed;
        }
        break;
      case InsertResult.Win:
        await this.speaker.playTune(100, 50, 300, 50, 500, 100, 400, 5);
        s.winOccurred = true;
        ++s.gamesPlayed;
        if (s.playerCurrent === s.playerOne) {
          ++s.playerOneScore;
          s.playerWinner = s.playerOne;
        } else {
          ++s.playerTwoScore;
          s.playerWinner = s.playerTwo;
        }
        s.msgbox.fixedAddLine(`${this.currentPlayer().name} Wins`);
        s.entrygrid.removeTokenAtCR(s.keypos, 0);
        break;
      case InsertResult.Failure:
        await this.speaker.buzz(200, 400, 20, 100);
        break;
    }
    return ret;
  }

  /** AnimateMove: steps the entry token to col over ai_speed half-seconds. */
  async animateMove(col: number): Promise<void> {
    const s = this.state;
    const goRight = col > s.keypos;
    const dist = goRight ? col - s.keypos : s.keypos - col;
    if (dist === 0) return;
    const del = Math.trunc((s.aiSpeed * s.aiDelay) / dist);
    do {
      if (goRight) this.keyRight();
      else this.keyLeft();
      this.vs.windowChanged(GameWindow.entryGrid, true);
      this.vs.display(Display.DisplayChanged);
      await sleep(del);
    } while (s.keypos !== col);
  }

  /**
   * HumanMove: the column chosen, or -1 if anything else happened (the menu,
   * a function key). Player keys move and drop; the pointer (for either
   * human here; only player one in the original) moves over the board,
   * and a click drops. On a touch screen the first tap on a column moves
   * the token there and a second tap drops it.
   */
  async humanMove(useMouse: boolean): Promise<number> {
    const s = this.state;
    for (;;) {
      if (this.input.kbhit()) {
        const key = await this.input.getKey();
        const r = await this.ks.key(key, null);
        if (r.used) {
          switch (r.index) {
            case 0:
            case 1:
              this.vs.display(r.funcret as Display);
              break;
            case 2:
            case 5:
              this.vs.display(Display.DisplayCurrent);
              return -1;
            case 3:
            case 4:
              return s.keypos;
          }
        } else if (await this.extraFunctionality(key)) return -1;
        continue;
      }
      if (this.mouseUsage && useMouse) {
        const m = this.input.getMouseInput();
        if (m.button === LMB) {
          const col = this.mouseCol(m.x, m.y);
          if (col >= 0 && col !== s.keypos) {
            this.moveEntryTo(col);
            this.vs.display(Display.DisplayChanged);
            if (m.pointerType === 'touch') continue; // tap again to drop
          }
          return s.keypos;
        }
        if (m.button === RMB) {
          const funcret = await this.keyUp();
          this.vs.display(funcret as Display);
          return -1;
        }
        if (m.moved && m.pointerType !== 'touch') {
          const col = this.mouseCol(m.x, m.y);
          if (col >= 0 && col !== s.keypos) {
            this.moveEntryTo(col);
            this.vs.display(Display.DisplayChanged);
          }
        }
      }
      if (this.stopped) return -1;
      await this.input.wait();
    }
  }

  /** P1Move */
  p1Move(): Promise<number> {
    this.pkeys.copy(this.p1keys);
    return this.humanMove(true);
  }

  /** P2Move */
  p2Move(): Promise<number> {
    this.pkeys.copy(this.p2keys);
    return this.humanMove(true);
  }

  /** The AI*Move functions: RuleMove with this many rules. */
  aiMove(level: number): number {
    return ruleMove(this.state, this.state.playgrid, level, this.currentPlayer(), this.otherPlayer(), this.rng);
  }

  /** The player's getmove function, by player group slot. */
  async getMove(slot: number): Promise<number> {
    if (slot === PLAYER_SLOTS.one) return this.p1Move();
    if (slot === PLAYER_SLOTS.two) return this.p2Move();
    if (slot === PLAYER_SLOTS.copy) return this.getMove(this.state.copyOf);
    return this.aiMove(slot - PLAYER_SLOTS.aiRandom);
  }

  // ---- Games ----

  /** NewGame */
  newGame(): void {
    const s = this.state;
    switch (s.goesFirst) {
      case GoesFirst.One:
        this.setP1Current();
        s.msgbox.fixedAddLine('Player One Goes First');
        break;
      case GoesFirst.Two:
        this.setP2Current();
        s.msgbox.fixedAddLine('Player Two Goes First');
        break;
      case GoesFirst.Loser:
        if (s.playerWinner === s.playerOne) this.setP2Current();
        else if (s.playerWinner === s.playerTwo) this.setP1Current();
        else if (this.rng.random(2) === 0) this.setP1Current();
        // else SetP2Current; -- the original forgot the (), so nothing changes (known bug 1)
        s.msgbox.fixedAddLine('Loser Goes First');
        break;
      case GoesFirst.Winner:
        if (s.playerWinner === s.playerOne) this.setP1Current();
        else if (s.playerWinner === s.playerTwo) this.setP2Current();
        else if (this.rng.random(2) === 0) this.setP1Current();
        else this.setP2Current();
        s.msgbox.fixedAddLine('Winner Goes First');
        break;
      case GoesFirst.Successive:
        if (s.wentFirst === s.playerTwo) this.setP1Current();
        else this.setP2Current();
        s.msgbox.fixedAddLine('Successive Player Goes First');
        break;
      case GoesFirst.Random:
        if (this.rng.random(2) === 0) this.setP1Current();
        else this.setP2Current();
        s.msgbox.fixedAddLine('Random Player Goes First');
        break;
    }
    s.wentFirst = s.playerCurrent;
    s.playgrid.empty();
    s.entrygrid.empty();
    this.vs.windowChanged(GameWindow.entryGrid, true);
    this.vs.windowChanged(GameWindow.playGrid, true);
    s.entrygrid.insertTokenAtCR(s.keypos, 0, this.currentPlayer());
    s.winOccurred = false;
    s.isDraw = false;
    this.ui.menuItemEnable(0, 3, true); // enable end game
    this.ui.menuItemEnable(0, 0, false); // disable new game
    this.ui.menuSubCurrentSet(0, 3);
    if (!this.ingame) {
      s.msgbox.fixedAddLine('Currently Playing Game.');
      this.vs.windowChanged(GameWindow.msgBox, true);
      this.resetScores();
    }
    this.ingame = true;
  }

  /** EndGame */
  endGame(): void {
    this.ui.menuItemEnable(2, 0, true); // enable players one and two
    this.ui.menuItemEnable(2, 1, true);
    this.ui.menuItemEnable(0, 3, false); // disable end game
    this.ui.menuItemEnable(0, 0, true); // enable new game
    this.ui.menuSubCurrentSet(0, 0);
    this.state.msgbox.fixedAddLine('Currently Not Playing.');
    this.ingame = false;
    this.state.playgrid.empty();
  }

  /** ToggleInGame (F4) */
  toggleInGame(): void {
    if (this.ingame) this.endGame();
    else this.newGame();
  }

  /** DemoMode (F6 computers, F7 humans) */
  demoMode(type: PlayerType): void {
    if (type === PlayerType.Computer) {
      const p1 = this.rng.random(5);
      let p2: number;
      do p2 = this.rng.random(5);
      while (p1 === p2);
      this.setP1_2(2 + p1, 2 + p2);
      this.state.msgbox.fixedAddLine('~Demo MODE!');
    } else {
      this.setP1_2(0, 1);
      this.state.msgbox.fixedAddLine('~Human vs Human');
    }
    this.newGame();
    this.resetScores();
    this.vs.display(Display.DisplayAll);
  }

  /** SaveGame (F2) */
  saveGame(): void {
    const s = this.state;
    this.saved = gameSave(s.playerOne, s.playerTwo, s.playerCurrent, s.win.tokens, s.playgrid, s.players);
    s.msgbox.fixedAddLine('Game Saved');
  }

  /** LoadGame (F3) */
  loadGame(): void {
    const s = this.state;
    if (!this.saved) {
      s.msgbox.fixedAddLine('~Load Error!');
      return;
    }
    const loaded = gameLoad(this.saved, s.players);
    s.gridCols = loaded.grid.cols();
    s.keypos = Math.trunc(s.gridCols / 2); // not ResetGameSize's position (known bug 2)
    s.gridRows = loaded.grid.rows();
    setWinSize(s.win, loaded.size);
    s.playgrid = loaded.grid;
    s.entrygrid = new Grid(s.gridCols, 1);
    this.setP1_2(loaded.p1, loaded.p2);
    s.playerCurrent = loaded.current;
    s.msgbox.fixedAddLine('Game Loaded');
    s.winOccurred = false;
    s.entrygrid.resetTokenAtCR(s.keypos, 0, this.currentPlayer());
    s.gameStatus();
    if (this.ingame) {
      const one = s.playerCurrent === s.playerOne;
      this.ui.menuItemEnable(2, 0, one);
      this.ui.menuItemEnable(2, 1, !one);
      this.ui.menuSubCurrentSet(2, one ? 0 : 1);
    } else {
      this.ui.menuItemEnable(2, 0, true);
      this.ui.menuItemEnable(2, 1, true);
    }
  }

  /** ToggleSound (F8) */
  toggleSound(): void {
    this.state.mute = !this.state.mute;
    this.speaker.muted = this.state.mute;
    this.state.gameStatus();
  }

  /**
   * SuggestMove (F5): AI Genius moves the token where it would play. The C
   * set SUGGEST_MASK to hatch the token, but in the original, running, the
   * token stays solid (measured in classic mode), so no mask is set here
   * (known bug 15).
   */
  async suggestMove(hatch = false): Promise<void> {
    const s = this.state;
    const oldMask = this.g.getFillMask();
    if (hatch) this.g.setFillMask(SUGGEST_MASK);
    const col = this.aiMove(4);
    this.vs.display(Display.DisplayCurrent);
    if (s.keypos === col) {
      this.vs.windowChanged(GameWindow.entryGrid, true);
      this.vs.display(Display.DisplayChanged);
      await sleep(s.aiSpeed * s.aiDelay);
    } else {
      await this.animateMove(col);
    }
    this.g.setFillMask(oldMask);
  }

  /** BestOfResult */
  bestOfResult(): void {
    const s = this.state;
    let out: string;
    if (s.playerOneScore > s.playerTwoScore) out = `${this.player(s.playerOne).name} Wins The Set`;
    else if (s.playerTwoScore > s.playerOneScore) out = `${this.player(s.playerTwo).name} Wins The Set`;
    else out = 'The Set is Drawn';
    s.msgbox.fixedAddLine(out);
  }

  /** ExtraFunctionality: the function keys; true if the key was used. */
  async extraFunctionality(key: number): Promise<boolean> {
    switch (key) {
      case 'h'.charCodeAt(0):
      case F1:
        this.pkeys.copy(this.p2keys); // F1 to help defaults to numeric keys
        await this.ui.help(this);
        this.vs.display(Display.DisplayCurrent);
        break;
      case 'm'.charCodeAt(0):
      case ESC:
        this.pkeys.copy(this.p2keys); // ESC to menu defaults to numeric keys
        await this.ui.menuCall(this);
        this.vs.display(Display.DisplayAll);
        break;
      case F2:
        this.saveGame();
        this.vs.display(Display.DisplayAll);
        break;
      case F3:
        this.loadGame();
        this.vs.display(Display.DisplayAll);
        break;
      case F4:
        this.toggleInGame();
        this.vs.display(Display.DisplayAll);
        break;
      case F5:
        await this.suggestMove();
        this.vs.display(Display.DisplayAll);
        break;
      case F6:
        this.demoMode(PlayerType.Computer);
        break;
      case F7:
        this.demoMode(PlayerType.Human);
        break;
      case F8:
        this.toggleSound();
        this.vs.windowChanged(GameWindow.statusBox, true);
        this.vs.display(Display.DisplayChanged);
        break;
      case F9: // reset the mouse calibration: nothing to calibrate on the web
        break;
      case CF9:
        this.state.msgbox.fixedAddLine('Config Removed...');
        this.vs.windowChanged(GameWindow.msgBox, true);
        this.vs.display(Display.DisplayChanged);
        break;
      case AF4:
      case CX:
      case CQ:
      case F10:
        await this.ui.exit(this);
        break;
      default:
        return false;
    }
    return true;
  }

  // ---- The main loop ----

  /** StartRandom (the title screen did this) and the first game, as main() did. */
  start(seed = startRandomSeed()): void {
    const s = this.state;
    this.rng.seed(seed);
    this.speaker.muted = s.mute;
    this.resetGameSize(s.gridCols, s.gridRows, s.win.tokens);
    this.newGame();
    s.gameStatus();
    this.vs.display(Display.DisplayAll);
  }

  /** main()'s loop. */
  async run(): Promise<void> {
    const s = this.state;
    while (!this.stopped) {
      while (this.ingame && !this.stopped) {
        const col = await this.getMove(s.playerCurrent);
        if (this.mouseUsage && this.input.getMouseInput().button === RMB) await this.keyUp();
        if (this.input.kbhit()) {
          await this.extraFunctionality(await this.input.getKey());
        } else if (col >= 0) {
          if (col !== s.keypos) await this.animateMove(col);
          s.keypos = col;
          if ((await this.dropToken()) !== InsertResult.Failure) {
            if (s.winOccurred || s.isDraw) await this.gameOver();
            else this.vs.display(Display.DisplayChanged);
          }
        }
        // Let the page breathe between moves, which matters when two AIs
        // play at the fastest speed.
        await sleep(0);
      }
      if (this.stopped) break;
      // Not in a game: only the function keys and the menu do anything.
      if (this.input.kbhit()) await this.extraFunctionality(await this.input.getKey());
      const m = this.input.getMouseInput();
      if (this.mouseUsage === 2 && m.button === RMB) {
        await this.keyUp();
        this.vs.display(Display.DisplayAll);
      }
      if (!this.ingame && !this.input.kbhit()) await this.input.wait();
    }
  }

  /** The end of a game in main(): the set result, "Press ESC To Continue", the next game. */
  private async gameOver(): Promise<void> {
    const s = this.state;
    let waitEsc = s.aiSpeed !== 0 || !this.bothPlayerAI();
    if (s.bestOf && s.gamesPlayed === s.bestOf) {
      this.bestOfResult();
      s.msgbox.fixedAddLine('~Press ESC To Continue');
      this.vs.display(Display.DisplayAll);
      s.msgbox.delLine(7);
      await this.waitForEsc();
      waitEsc = false; // no longer need to display msg
      this.resetScores();
    }
    if (waitEsc) s.msgbox.fixedAddLine('~Press ESC To Continue');
    this.vs.display(Display.DisplayAll);
    if (waitEsc) s.msgbox.delLine(7);
    this.newGame();
    if (waitEsc) await this.waitForEsc();
    this.vs.display(Display.DisplayAll);
  }

  /**
   * while (GetKey()!=ESC) {}. A click or tap on the screen also continues,
   * for players without a keyboard.
   */
  async waitForEsc(): Promise<void> {
    for (;;) {
      if (this.stopped) return;
      if (this.input.kbhit()) {
        if ((await this.input.getKey()) === ESC) return;
        continue;
      }
      const m = this.input.getMouseInput();
      if (m.button === LMB) return;
      if (m.button === NMB) await this.input.wait();
    }
  }
}
