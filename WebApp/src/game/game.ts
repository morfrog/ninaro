// The game (Legacy NINARO.C): the main loop, players' moves, dropping
// tokens, wins, draws, scores, sets, demo mode and the function keys.
//
// The C polled the keyboard and mouse in busy loops and called delay();
// here the same code awaits the input queue and sleep(). Menus, dialogs,
// help and the title screen come in phase 4, through GameUi.

import { RULES_FIXED, ruleMove, type AiRules } from '../core/ai';
import { InsertResult, gameLoad, gameSave, insertTokenInCol, insertTokenTarget, setWinSize } from '../core/connect';
import { Direction, Grid } from '../core/grid';
import { PlayerType, type Player } from '../core/player';
import { WatcomRandom } from '../core/random';
import { Speaker } from '../audio/sound';
import { Input, LMB, NMB, RMB, sleep, type TouchDrag } from '../input/input';
import {
  AF4, BKSP, CF9, CQ, CX, ENTER, ESC, F1, F10, F2, F3, F4, F5, F6, F7, F8, F9, KP2, KP4, KP6, KP8, KeyGroup, KeySys,
} from '../input/keys';
import { encodeConfig } from '../data/config';
import type { Graph } from '../render/graph';
import type { ViewMode } from '../render/viewMode';
import { Display, type ViewSys } from '../render/viewsys';
import { GameWindow } from './display';
import { FLASH_FRAME_MS, LogFlash, drawLogFlash } from './logFlash';
import { Color } from '../render/palette';
import { GoesFirst, PLAYER_SLOTS, type GameState } from './state';
import { OnlineLink, immanuelStore, type OnlineStoreFactory } from './online';
import { KIND_NAMES, boardText, cleanRoom } from '../online/protocol';
import type { ConnectResult, SessionEvent, SessionTimings } from '../online/session';

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

/**
 * Where the entry token starts: the middle column (the left of the two
 * middle ones on an even board). The original started one column right of
 * that on odd boards, except after loading a game (known bug 2, fixed).
 */
export function startColumn(cols: number): number {
  return Math.trunc((cols - 1) / 2);
}

/** A file the game keeps (NINARO.SAV, NINARO.CFG). */
export interface SavedFile {
  read(): Uint8Array | null;
  write(bytes: Uint8Array): boolean;
  remove(): void;
}

/** A file kept only while the page is open. */
export function memoryFile(): SavedFile {
  let data: Uint8Array | null = null;
  return {
    read: () => data,
    write: (bytes) => ((data = bytes.slice()), true),
    remove: () => void (data = null),
  };
}

/**
 * Whether bytes look like a NINARO.SAV this game can load (the original
 * trusted the file; a damaged one would have crashed it).
 */
export function validSave(bytes: Uint8Array): boolean {
  if (bytes.length < 24) return false;
  const v = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  const [p1, p2, current, size, width, height] = [0, 1, 2, 3, 4, 5].map((i) => v.getInt32(i * 4));
  const slot = (n: number) => n >= 0 && n <= 7;
  if (!slot(p1) || !slot(p2) || (current !== p1 && current !== p2)) return false;
  if (width < 3 || width > 50 || height < 3 || height > 50 || size < 3 || size > Math.max(width, height)) return false;
  if (bytes.length < 24 + width * height * 4) return false;
  for (let i = 0; i < width * height; i++) {
    if (!slot(v.getInt32(24 + i * 4) - 1) && v.getInt32(24 + i * 4) !== 0) return false;
  }
  return true;
}

/** Hatching meant for Suggest Move's token (SuggestMove's new_mask); never seen in the original. */
const SUGGEST_MASK = [0xa2, 0x15, 0xa2, 0x10, 0xa2, 0x10, 0xa2, 0x15];

/** Time for a human's token to fall the height of the board. */
const HUMAN_DROP_MS = 1000;
/** Pause between frames of a falling token. */
const FRAME_MS = 16;
/** How far a finger must go down, past any sideways move, to drop the token: this fraction of the board's height. */
const DROP_FRACTION = 1 / 3;

/**
 * The windows Fullscreen leaves out: everything at the right, where the
 * information is. The entry row and the board stay.
 */
const INFO_WINDOWS = [
  GameWindow.gameName,
  GameWindow.msgBox,
  GameWindow.statusBox,
  GameWindow.p1Type,
  GameWindow.p1Name,
  GameWindow.p1Score,
  GameWindow.p2Type,
  GameWindow.p2Name,
  GameWindow.p2Score,
] as const;

/** What the page shows of the view: whether the screen is cropped to the board, and the board's size. */
export interface ViewState {
  /** Fullscreen is on and no menu, dialog or help is open over it. */
  cropped: boolean;
  /** Fullscreen is chosen and the game screen is showing (not the title). */
  fullscreen: boolean;
  /** "cols x rows @ connect", as the status box shows them. */
  readout: string;
  /** Who is playing: "Human" and "AI Normal", for "Human vs AI Normal". */
  players: { one: string; two: string };
}

export class Game {
  readonly rng = new WatcomRandom();
  /** The AIs' rules: the fixed ones, or RULES_1998 to play exactly as the original. */
  aiRules: AiRules = RULES_FIXED;
  readonly speaker = new Speaker();
  readonly pkeys = new KeyGroup(KP4, KP6, KP8, KP2, ENTER, BKSP);
  readonly p1keys = new KeyGroup(...'adwseq'.split('').map((c) => c.charCodeAt(0)));
  readonly p2keys = new KeyGroup(KP4, KP6, KP8, KP2, ENTER, BKSP);
  readonly ks = new KeySys(this.pkeys);
  ingame = false;
  /** Shows tokens falling into the board. Tests turn it off to run fast. */
  animateDrops = true;
  /** 0 none, 1 partial (moves only), 2 full (menus and dialogs too). */
  mouseUsage = 2;
  /** NINARO.SAV: the one saved game. In memory unless main() gives it storage. */
  saveFile: SavedFile = memoryFile();
  /** NINARO.CFG: the settings, written by saveConfig(). */
  configFile: SavedFile = memoryFile();
  /**
   * Which side is Online (one byte, 1 or 2), so the page can reconnect when
   * the game next opens; removed when neither is. NINARO.CFG keeps the
   * original's layout, which has no Online player.
   */
  onlineFile: SavedFile = memoryFile();
  /** savecfg: false after Ctrl-F9 removes the configuration, so it isn't written again. */
  saveCfg = true;
  ui: GameUi = NO_UI;
  /** Stops run() (for tests). */
  stopped = false;
  /** Redraws whatever is open over the game screen (menu, dialog, help), after a resize. */
  readonly overlays: (() => void)[] = [];
  /** The View option. */
  private viewMode: ViewMode = 'normal';
  /** Whether the game screen is showing (not the title screen). */
  private screenOn = false;
  /** Told whenever the view changes, and after each redraw, so the page can follow. */
  onView: (view: ViewState) => void = () => {};
  private readonly flash = new LogFlash();
  private flashTimer: ReturnType<typeof setTimeout> | null = null;
  /** Whether the info windows are currently left out. */
  private infoHidden = false;
  /** The online game, while one is on or connecting. */
  online: OnlineLink | null = null;
  /** Makes the online game's store (tests use one in memory). */
  onlineStore: OnlineStoreFactory = immanuelStore;
  /** The online session's timings (tests make them fast). */
  onlineTimings?: Partial<SessionTimings>;

  constructor(
    readonly state: GameState,
    readonly g: Graph,
    readonly vs: ViewSys,
    readonly input: Input,
  ) {
    // A finger that goes down in the board's bottom third is a tap to drop, not a drag to track.
    this.input.trackTouch = (_x, y) => !this.inDropZone(y);
    this.ks.iFunc(0, () => this.keyLeft());
    this.ks.iFunc(1, () => this.keyRight());
    this.ks.iFunc(2, () => this.keyUp());
    this.ks.iFunc(3, () => this.keyDown());
    this.ks.iFunc(4, () => this.keyDown()); // KeySelect
    this.ks.iFunc(5, () => this.keyUp()); // KeyCancel
    vs.afterDisplay = () => this.afterDraw();
    state.msgbox.onAdd = (line) => this.logAdded(line);
  }

  /** Runs a menu, dialog or help call with `redraw` as its overlay. Menus need the whole screen, so Fullscreen is set aside while one is open. */
  async withOverlay<T>(redraw: () => void, call: () => Promise<T>): Promise<T> {
    this.overlays.push(redraw);
    this.applyView();
    try {
      return await call();
    } finally {
      this.overlays.pop();
      this.applyView();
    }
  }

  // ---- The View option ----

  /** Whether Fullscreen is showing: the board only, with the info windows left out. */
  get cropped(): boolean {
    return this.viewMode === 'fullscreen' && this.screenOn && this.overlays.length === 0;
  }

  /** Called when the title screen is shown (false) or the game screen is (true). Fullscreen applies to the game only. */
  setScreenOn(on: boolean): void {
    this.screenOn = on;
    this.applyView();
  }

  /** Sets the View option (Normal or Fullscreen) and redraws the screen to match. */
  setViewMode(mode: ViewMode): void {
    this.viewMode = mode;
    this.applyView();
  }

  /**
   * Leaves out the info windows when Fullscreen is on, and puts them back
   * otherwise, redrawing the game screen if it changed.
   */
  private applyView(): void {
    const hide = this.cropped;
    if (hide !== this.infoHidden) {
      this.infoHidden = hide;
      for (const w of INFO_WINDOWS) this.vs.windowActivate(w, !hide);
      if (this.screenOn) this.vs.display(Display.DisplayAll);
    }
    this.syncView();
  }

  private syncView(): void {
    const s = this.state;
    this.onView({
      cropped: this.cropped,
      fullscreen: this.viewMode === 'fullscreen' && this.screenOn,
      readout: `${s.gridCols}x${s.gridRows}@${s.win.tokens}`,
      players: { one: this.toolbarName(s.playerOne), two: this.toolbarName(s.playerTwo) },
    });
  }

  /** A player's name in the toolbar: as in the log, and an online opponent with who they are, as "Online (Human)". */
  private toolbarName(slot: number): string {
    const link = this.online;
    if (slot === PLAYER_SLOTS.online && link?.started) return `Online (${KIND_NAMES[link.theirKind]})`;
    return this.state.logName(slot);
  }

  /** Runs after each redraw: the page follows the view, and any message on show is drawn over the board. */
  private afterDraw(): void {
    this.syncView();
    if (!this.screenOn || !this.cropped) return;
    const flash = this.flash.current(performance.now());
    if (flash.length === 0) return;
    const s = this.state;
    drawLogFlash(this.g, this.vs.windows[GameWindow.playGrid].vp, flash, {
      back: Color.BLACK,
      border: Color.RED,
      text: s.text,
      hilite: s.hilite,
    });
  }

  /** A line added to the message box: in Fullscreen it is shown over the board for a moment. */
  private logAdded(line: string): void {
    if (!this.screenOn || !this.cropped || line.replace(/^~/, '').trim() === '') return;
    this.flash.show(line, performance.now());
    this.startFlash();
  }

  /** Redraws the screen until the message has faded. */
  private startFlash(): void {
    if (this.flashTimer !== null) return;
    const frame = () => {
      this.flashTimer = null;
      if (!this.screenOn || !this.cropped) {
        this.flash.clear();
        return;
      }
      const live = this.flash.current(performance.now()).length > 0;
      // Restores the saved screen, then draws the message if it is still showing.
      this.vs.display(Display.DisplayCurrent);
      if (live) this.flashTimer = setTimeout(frame, FLASH_FRAME_MS);
    };
    frame();
  }

  /** The whole screen again (after a resize): the game, then what is open over it. */
  redrawAll(): void {
    this.vs.display(Display.DisplayAll);
    this.overlays.at(-1)?.();
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

  /** SetP1. Changing either player leaves an online game. */
  setP1(player: number): void {
    if (this.online && player !== PLAYER_SLOTS.online) this.leaveOnline();
    const s = this.state;
    if (s.playerCurrent === s.playerOne) {
      s.playerOne = player;
      s.playerCurrent = s.playerOne;
    } else s.playerOne = player;
  }

  /** SetP2. Changing either player leaves an online game. */
  setP2(player: number): void {
    if (this.online && player !== PLAYER_SLOTS.online) this.leaveOnline();
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
    this.updatePlayerMenu();
    this.ui.menuSubCurrentSet(2, 0);
  }

  /** SetP2Current */
  setP2Current(): void {
    this.state.playerCurrent = this.state.playerTwo;
    this.updatePlayerMenu();
    this.ui.menuSubCurrentSet(2, 1);
  }

  /**
   * The Player menu's One and Two items. During a game with two humans, only
   * the player to move can be changed (the other human changes on their own
   * turn). Otherwise both can be changed at any time.
   */
  updatePlayerMenu(): void {
    const s = this.state;
    const bothHuman = this.player(s.playerOne).type === PlayerType.Human && this.player(s.playerTwo).type === PlayerType.Human;
    const locked = this.ingame && bothHuman;
    this.ui.menuItemEnable(2, 0, !locked || s.playerCurrent === s.playerOne);
    this.ui.menuItemEnable(2, 1, !locked || s.playerCurrent === s.playerTwo);
  }

  /**
   * The name a player goes by in the log: "Human", or the AI's name. When
   * both players are the same type, they are numbered 1 (player one) and 2.
   */
  logName(slot: number): string {
    return this.state.logName(slot);
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

  /** ResetGameSize. A new board leaves an online game. */
  resetGameSize(cols: number, rows: number, inARow: number): void {
    this.leaveOnline();
    const s = this.state;
    s.gridCols = cols;
    s.gridRows = rows;
    s.keypos = startColumn(cols);
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
    this.saveConfig(); // the players or settings may have changed: remember them now
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
    this.vs.windowChanged(GameWindow.playGrid, true); // the board shows the column the token is over
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
    const dropper = this.currentPlayer();
    const row = insertTokenTarget(s.playgrid, s.keypos);
    const ret = insertTokenInCol(s.playgrid, s.keypos, dropper, s.win);
    if (ret !== InsertResult.Failure) {
      // The token leaves the entry row as it drops. The next player's token comes back when their turn starts.
      s.entrygrid.removeTokenAtCR(s.keypos, 0);
      this.vs.windowChanged(GameWindow.entryGrid, true);
      await this.animateDrop(s.keypos, row, dropper);
    }
    switch (ret) {
      case InsertResult.Success:
        this.nextPlayer();
        this.vs.windowChanged(GameWindow.playGrid, true);
        if (s.playgrid.tokens() === s.playgrid.maxTokens()) {
          s.msgbox.fixedAddLine("It's A Draw...");
          s.isDraw = true;
          ++s.gamesPlayed;
        }
        break;
      case InsertResult.Win:
        // Online, the losing device gets a falling tune instead of the winner's.
        if (this.online?.started && s.playerCurrent === PLAYER_SLOTS.online) await this.speaker.playTune(400, 120, 300, 120, 200, 250);
        else await this.speaker.playTune(100, 50, 300, 50, 500, 100, 400, 5);
        s.winOccurred = true;
        ++s.gamesPlayed;
        if (s.playerCurrent === s.playerOne) {
          ++s.playerOneScore;
          s.playerWinner = s.playerOne;
        } else {
          ++s.playerTwoScore;
          s.playerWinner = s.playerTwo;
        }
        s.msgbox.fixedAddLine(this.winLine(s.playerCurrent));
        break;
      case InsertResult.Failure:
        await this.speaker.buzz(200, 400, 20, 100);
        break;
    }
    return ret;
  }

  /**
   * How long a token takes to fall the height of the board. A human's drop
   * is 1 second. A computer's drop scales with AI Speed:
   * Normal is also 1 second, each step faster or slower doubles or halves it
   * (Fastest 0.25 s, Slowest 4 s).
   */
  dropMs(player: Player): number {
    if (player.type !== PlayerType.Computer) return HUMAN_DROP_MS;
    return HUMAN_DROP_MS * 2 ** (this.state.aiSpeed - 2);
  }

  /**
   * Shows a token falling under gravity from the top edge of the board to its
   * row, so it speeds up the further it falls. Its distance grows with the
   * square of the time, and the height of the board takes dropMs. The token
   * is already in the grid, hidden, until it lands.
   */
  async animateDrop(col: number, row: number, player: Player): Promise<void> {
    if (!this.animateDrops) return;
    const s = this.state;
    // From the top edge (-0.5) to the centre of `row` (row + 0.5): row + 1 rows.
    // Free fall covers distance d in time T * sqrt(d / H), for a full board of height H.
    const distance = row + 1;
    const total = this.dropMs(player) * Math.sqrt(distance / s.playgrid.height);
    const start = performance.now();
    for (;;) {
      const t = Math.min(1, (performance.now() - start) / total);
      s.falling = { col, row, y: -0.5 + t * t * distance, color: player.color };
      this.vs.windowChanged(GameWindow.playGrid, true);
      this.vs.display(Display.DisplayChanged);
      if (t >= 1) break;
      await sleep(FRAME_MS);
    }
    s.falling = null;
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
      // Each column must stay on screen for at least one frame, or the page
      // never shows it and the token seems to skip squares. (Tests turn
      // animation off and run as fast as they can.)
      await sleep(this.animateDrops ? Math.max(del, FRAME_MS) : del);
    } while (s.keypos !== col);
  }

  /**
   * HumanMove: the column chosen, or -1 if anything else happened (the menu,
   * a function key). Player keys move and drop; the pointer (for either
   * human here; only player one in the original) moves over the board,
   * and a click on the board drops. On a touch screen the first tap on a column moves
   * the token there and a second tap drops it. A finger dragged from the board moves
   * the token sideways and drops it on a downward drag (touchDragMove).
   */
  async humanMove(useMouse: boolean): Promise<number> {
    const s = this.state;
    // A click or press made during the other player's (or the AI's) turn does not drop this player's token.
    this.input.forgetPresses();
    // The point the finger's drop is measured from: where it went down, or where it last moved the token.
    let anchor: { x: number; y: number } | null = null;
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
        const drag = this.input.touchDrag();
        if (drag) {
          anchor ??= { x: drag.startX, y: drag.startY };
          if (this.touchDragMove(drag, anchor)) return s.keypos;
        }
        const m = this.input.getMouseInput();
        if (m.button === LMB) {
          const col = this.mouseCol(m.x, m.y);
          // The original dropped on a click anywhere; with the pointer visible,
          // only a click on the board does.
          if (col < 0) continue;
          // A tap in the bottom third drops the token where it is, without moving it.
          if (m.pointerType === 'touch' && this.inDropZone(m.y)) return s.keypos;
          if (col !== s.keypos) {
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
      if (this.stopped || !this.ingame || this.online?.left) return -1;
      await this.input.wait();
    }
  }

  /**
   * Touch: the board's bottom third. A press there is not tracked, and a tap
   * in it drops the token in the tapped column at once.
   */
  inDropZone(y: number): boolean {
    const board = this.vs.windows[GameWindow.playGrid].vp;
    return y >= board.y1 + ((board.y2 - board.y1) * 2) / 3;
  }

  /**
   * A finger that went down on the board and is dragged: sideways moves the
   * entry token to the column under the finger, and down DROP_FRACTION of the board from the
   * last sideways move drops it. Returns true when it drops.
   */
  private touchDragMove(drag: TouchDrag, anchor: { x: number; y: number }): boolean {
    if (!drag.dragging || drag.spent) return false;
    // A press that began off the board is not a drag on the board; nor is one that has moved into the bottom third.
    if (this.mouseCol(drag.startX, drag.startY) < 0 || this.inDropZone(drag.y)) return false;
    const s = this.state;
    const dx = drag.x - anchor.x;
    const dy = drag.y - anchor.y;
    const board = this.vs.windows[GameWindow.playGrid].vp;
    const dropPx = ((board.y2 - board.y1 + 1) * DROP_FRACTION);
    if (dy > dropPx && dy > Math.abs(dx)) {
      this.input.spendTouch();
      this.input.takeWheel(); // the finger's vertical scroll is not for the help
      return true;
    }
    const col = this.mouseCol(drag.x, drag.startY);
    if (col >= 0 && col !== s.keypos) {
      this.moveEntryTo(col);
      this.vs.display(Display.DisplayChanged);
      anchor.x = drag.x;
      anchor.y = drag.y;
    }
    this.input.takeWheel();
    return false;
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
    return ruleMove(this.state, this.state.playgrid, level, this.currentPlayer(), this.otherPlayer(), this.rng, this.aiRules);
  }

  /** The player's getmove function, by player group slot. */
  async getMove(slot: number): Promise<number> {
    if (slot === PLAYER_SLOTS.one) return this.p1Move();
    if (slot === PLAYER_SLOTS.two) return this.p2Move();
    if (slot === PLAYER_SLOTS.copy) return this.getMove(this.state.copyOf);
    if (slot === PLAYER_SLOTS.online) return this.onlineMove();
    return this.aiMove(slot - PLAYER_SLOTS.aiRandom);
  }

  // ---- Games ----

  /** NewGame. Online, who goes first comes from the session; New while still connecting gives up connecting. */
  newGame(): void {
    const s = this.state;
    if (this.online && !this.online.started) this.leaveOnline();
    const link = this.online;
    if (link) {
      link.nextGame();
      const onlineFirst = !link.session.meFirstIn(link.game);
      const sideFirst = onlineFirst ? link.side : link.side === 1 ? 2 : 1;
      if (sideFirst === 1) this.setP1Current();
      else this.setP2Current();
    } else switch (s.goesFirst) {
      case GoesFirst.One:
        this.setP1Current();
        break;
      case GoesFirst.Two:
        this.setP2Current();
        break;
      case GoesFirst.Loser:
        if (s.playerWinner === s.playerOne) this.setP2Current();
        else if (s.playerWinner === s.playerTwo) this.setP1Current();
        else if (this.rng.random(2) === 0) this.setP1Current();
        else this.setP2Current(); // the original forgot the () here (known bug 1, fixed)
        break;
      case GoesFirst.Winner:
        if (s.playerWinner === s.playerOne) this.setP1Current();
        else if (s.playerWinner === s.playerTwo) this.setP2Current();
        else if (this.rng.random(2) === 0) this.setP1Current();
        else this.setP2Current();
        break;
      case GoesFirst.Successive:
        if (s.wentFirst === s.playerTwo) this.setP1Current();
        else this.setP2Current();
        break;
      case GoesFirst.Random:
        if (this.rng.random(2) === 0) this.setP1Current();
        else this.setP2Current();
        break;
    }
    // Who goes first, by name, whatever the setting that chose them.
    s.msgbox.fixedAddLine(`${this.logName(s.playerCurrent)} Goes First`);
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
    this.updatePlayerMenu();
  }

  /** EndGame. Leaves an online game. */
  endGame(): void {
    this.leaveOnline();
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
    if (this.online) {
      s.msgbox.fixedAddLine('~Not while online');
      return;
    }
    const bytes = gameSave(s.playerOne, s.playerTwo, s.playerCurrent, s.win.tokens, s.playgrid, s.players);
    s.msgbox.fixedAddLine(this.saveFile.write(bytes) ? 'Game Saved' : '~Save Error!');
  }

  /** SaveCfg, unless Ctrl-F9 removed the configuration (ExitGame's `if (savecfg)`). */
  saveConfig(): void {
    if (!this.saveCfg) return;
    this.configFile.write(encodeConfig(this));
    if (this.online) this.onlineFile.write(Uint8Array.of(this.online.side));
    else this.onlineFile.remove();
  }

  /** The side to make Online when the game opens (as it was last time), or null. */
  savedOnlineSide(): 1 | 2 | null {
    const side = this.onlineFile.read()?.[0];
    return side === 1 || side === 2 ? side : null;
  }

  /** LoadGame (F3) */
  loadGame(): void {
    const s = this.state;
    if (this.online) {
      s.msgbox.fixedAddLine('~Not while online');
      return;
    }
    const bytes = this.saveFile.read();
    const loaded = bytes && validSave(bytes) ? gameLoad(bytes, s.players) : null;
    if (!loaded) {
      s.msgbox.fixedAddLine('~Load Error!');
      return;
    }
    s.gridCols = loaded.grid.cols();
    s.keypos = startColumn(s.gridCols);
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
    this.updatePlayerMenu();
    if (this.ingame) this.ui.menuSubCurrentSet(2, s.playerCurrent === s.playerOne ? 0 : 1);
  }

  /** ToggleSound (F8) */
  toggleSound(): void {
    this.state.mute = !this.state.mute;
    this.speaker.muted = this.state.mute;
    this.state.gameStatus();
  }

  /**
   * SuggestMove (F5): AI Genius moves the token where it would play,
   * hatched. (The C set the hatching, but in the original, running, the
   * token stayed solid: known bug 15, fixed.)
   */
  async suggestMove(hatch = true): Promise<void> {
    const s = this.state;
    if (s.playerCurrent === PLAYER_SLOTS.online) return; // the opponent's move is theirs to make
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
    if (s.playerOneScore > s.playerTwoScore) out = this.winLine(s.playerOne, true);
    else if (s.playerTwoScore > s.playerOneScore) out = this.winLine(s.playerTwo, true);
    else out = 'The Set is Drawn';
    s.msgbox.fixedAddLine(out);
  }

  /**
   * Who won. Online, each device speaks to its own player: "You Win" on the
   * winner's, "You Lose" on the loser's. Otherwise the winner by name.
   */
  private winLine(winner: number, set = false): string {
    const what = set ? ' The Set' : '';
    if (!this.online?.started) return `${this.logName(winner)} Wins${what}`;
    return winner === PLAYER_SLOTS.online ? `~You Lose${what}` : `~You Win${what}!`;
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
        this.configFile.remove();
        this.onlineFile.remove();
        this.saveCfg = false;
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
    this.saveConfig(); // a function key or the menu may have changed the players: remember them now
    return true;
  }

  // ---- Online ----

  /**
   * Plays the opponent in a room online: `side` (player one or two) becomes
   * Online. Connects (which may wait a long time for an opponent), then
   * starts game 1. Null if it was left before connecting.
   */
  async goOnline(room: string, side: 1 | 2 = 2): Promise<ConnectResult | null> {
    const s = this.state;
    this.leaveOnline();
    if (this.ingame) this.endGame();
    const prevSlot = side === 1 ? s.playerOne : s.playerTwo;
    if (side === 1) s.playerOne = PLAYER_SLOTS.online;
    else s.playerTwo = PLAYER_SLOTS.online;
    if (s.playerCurrent === prevSlot) s.playerCurrent = PLAYER_SLOTS.online;
    const local = this.player(side === 1 ? s.playerTwo : s.playerOne);
    const board = { cols: s.gridCols, rows: s.gridRows, connect: s.win.tokens };
    const link: OnlineLink = new OnlineLink(
      cleanRoom(room),
      side,
      prevSlot,
      board,
      local.color,
      this.playerKind(side === 1 ? s.playerTwo : s.playerOne),
      this.onlineStore,
      (e) => this.onlineEvent(link, e),
      (trouble) => this.onlineTrouble(link, trouble),
      this.onlineTimings,
    );
    this.online = link;
    this.say(`Room: ${link.room}`);
    this.updatePlayerMenu();
    this.redraw();

    let result: ConnectResult;
    try {
      result = await link.session.connect();
    } catch {
      return null; // left while connecting
    }
    if (this.online !== link) return null;
    switch (result.kind) {
      case 'started':
        link.started = true;
        if (result.colorChanged) {
          local.color = result.myColor;
          this.say('Your colour changed');
        }
        this.player(PLAYER_SLOTS.online).color = result.theirColor;
        link.theirKind = result.theirKind;
        this.say(`Connected as ${result.role === 'h' ? 'host' : 'guest'}`);
        this.say(`~Opponent: ${KIND_NAMES[result.theirKind]}`);
        this.newGame();
        s.gameStatus();
        break;
      case 'mismatch':
        this.say('~Board mismatch:');
        this.say(`You ${boardText(result.mine)}, them ${boardText(result.theirs)}`);
        this.leaveOnline(false);
        break;
      case 'busy':
        this.say(`~Room in use: ${link.room}`);
        this.leaveOnline(false);
        break;
    }
    this.saveConfig(); // Online (or, if it didn't start, the player put back) is remembered for next time
    this.redraw();
    this.input.poke(); // the main loop may be waiting for input with no game on
    return result;
  }

  /** Who a player is, as the opponent's device is told: 0 a human, 1..5 AI Random to AI Genius. */
  private playerKind(slot: number): number {
    if (slot === PLAYER_SLOTS.copy) slot = this.state.copyOf;
    return slot >= PLAYER_SLOTS.aiRandom && slot <= PLAYER_SLOTS.aiGenius ? slot - PLAYER_SLOTS.aiRandom + 1 : 0;
  }

  /**
   * Leaves the online game (or stops connecting): tells the opponent unless
   * sendQuit is false, and puts back the player the Online side had before.
   */
  leaveOnline(sendQuit = true, forfeit = sendQuit): void {
    const link = this.online;
    if (!link) return;
    // Ending a game under way online gives it to the opponent.
    if (forfeit && !link.left && this.gameUnderWay(link)) this.forfeit(link, link.side);
    this.online = null;
    link.close(sendQuit && !link.left);
    const s = this.state;
    const wasCurrent = s.playerCurrent === PLAYER_SLOTS.online;
    if (link.side === 1) s.playerOne = link.prevSlot;
    else s.playerTwo = link.prevSlot;
    if (wasCurrent) s.playerCurrent = link.prevSlot;
    // The entry token was the Online player's: it becomes the player in its place.
    if (this.ingame && s.entrygrid.statusOfCR(s.keypos, 0) !== null) s.entrygrid.resetTokenAtCR(s.keypos, 0, this.currentPlayer());
    this.updatePlayerMenu();
    this.input.poke();
  }

  /** The page is closing: one try at telling the opponent. */
  leaveOnlineNow(): void {
    this.online?.closeNow();
  }

  /** Whether a game is under way online: started, with a move made, and not yet won or drawn. */
  private gameUnderWay(link: OnlineLink): boolean {
    const s = this.state;
    return link.started && this.ingame && !s.winOccurred && !s.isDraw && link.myMoves + link.theirMoves > 0;
  }

  /**
   * A game ended by a player before it was won: it counts as a win for the
   * other side. `winnerSide` is player one or two on this device.
   */
  private forfeit(link: OnlineLink, winnerSide: 1 | 2): void {
    const s = this.state;
    ++s.gamesPlayed;
    if (winnerSide === 1) ++s.playerOneScore;
    else ++s.playerTwoScore;
    const iWon = winnerSide !== link.side;
    this.say(iWon ? '~You Win!' : '~You Lose');
    this.say(iWon ? 'Opponent ended the game' : 'You ended the game');
    if (iWon) void this.speaker.playTune(100, 50, 300, 50, 500, 100, 400, 5);
    else void this.speaker.playTune(400, 120, 300, 120, 200, 250);
  }

  /** The opponent quit: the game ends, and if it was under way, we won it. */
  private opponentLeft(): void {
    const link = this.online;
    if (!link) return;
    if (this.gameUnderWay(link)) this.forfeit(link, link.side === 1 ? 2 : 1);
    else this.say('~Opponent left');
    this.leaveOnline(false);
    if (this.ingame) this.endGame();
    this.saveConfig();
    this.redraw();
  }

  private onlineEvent(link: OnlineLink, e: SessionEvent): void {
    if (this.online !== link) return;
    switch (e.type) {
      case 'hosting':
        this.say('Waiting for opponent...');
        break;
      case 'joining':
        this.say('Joining game...');
        break;
      case 'offline':
        this.say(e.offline ? '~Opponent offline' : 'Opponent is back');
        break;
      case 'left':
        link.left = true;
        this.input.poke();
        break;
    }
  }

  private onlineTrouble(link: OnlineLink, trouble: boolean): void {
    if (this.online !== link) return;
    this.say(trouble ? '~Connection lost, retrying...' : 'Connected again');
  }

  /** A line in the message box, shown now unless a menu or dialog is open over the game. */
  private say(line: string): void {
    this.state.msgbox.fixedAddLine(line);
    this.vs.windowChanged(GameWindow.msgBox, true);
    if (this.screenOn && this.overlays.length === 0) this.vs.display(Display.DisplayChanged);
  }

  /** The whole game screen again, unless a menu or dialog is open over it. */
  private redraw(): void {
    if (this.screenOn && this.overlays.length === 0) this.vs.display(Display.DisplayAll);
  }

  /**
   * The Online player's getmove: waits for the opponent's move, while keys
   * (the menu, help, the function keys) still work. -1 if anything else
   * happened first, as humanMove.
   */
  private async onlineMove(): Promise<number> {
    const link = this.online;
    if (!link?.started) return -1;
    this.input.forgetPresses();
    const move = link.nextMove();
    for (;;) {
      if (this.online !== link || link.left || !this.ingame || this.stopped) return -1;
      if (this.input.kbhit()) {
        if (await this.extraFunctionality(await this.input.getKey())) return -1;
        continue;
      }
      if (this.mouseUsage && this.input.getMouseInput().button === RMB) {
        this.vs.display((await this.keyUp()) as Display);
        return -1;
      }
      const r = await Promise.race([move, this.input.wait().then(() => null)]);
      if (!r) continue;
      if ('col' in r) {
        link.tookMove();
        return r.col;
      }
      if ('left' in r) {
        link.left = true;
        return -1;
      }
      // Out of step, or the session ended under us.
      if (this.online === link) {
        this.say('~Out of step: game ended');
        this.leaveOnline(true, false); // nobody's fault: no win or loss
        if (this.ingame) this.endGame();
        this.redraw();
      }
      return -1;
    }
  }

  // ---- The main loop ----

  /**
   * The first game, as main() started it after the title screen. The title
   * screen seeds the random numbers (StartRandom); without it, pass a seed.
   */
  start(seed?: number, size = { cols: this.state.gridCols, rows: this.state.gridRows, connect: this.state.win.tokens }): void {
    const s = this.state;
    if (seed !== undefined) this.rng.seed(seed);
    this.setScreenOn(true);
    this.speaker.muted = s.mute;
    this.resetGameSize(size.cols, size.rows, size.connect);
    this.newGame();
    s.gameStatus();
    this.vs.display(Display.DisplayAll);
  }

  /**
   * At the start of a turn, the player to move's token appears in the entry
   * row, over the column the last one dropped in.
   */
  private showEntryToken(): void {
    const s = this.state;
    if (s.entrygrid.statusOfCR(s.keypos, 0) !== null) return;
    s.entrygrid.insertTokenAtCR(s.keypos, 0, this.currentPlayer());
    this.vs.windowChanged(GameWindow.entryGrid, true);
    this.vs.display(Display.DisplayChanged);
  }

  /** main()'s loop. */
  async run(): Promise<void> {
    const s = this.state;
    while (!this.stopped) {
      while (this.ingame && !this.stopped) {
        if (this.online?.left) {
          this.opponentLeft();
          break;
        }
        this.showEntryToken();
        const mover = s.playerCurrent;
        const col = await this.getMove(s.playerCurrent);
        if (this.online?.left) {
          this.opponentLeft();
          break;
        }
        if (this.mouseUsage && this.input.getMouseInput().button === RMB) await this.keyUp();
        // The original handled a key waiting here instead of the move,
        // which was lost (known bug 16, fixed): the move goes in first.
        if (col >= 0) {
          if (col !== s.keypos) await this.animateMove(col);
          s.keypos = col;
          if ((await this.dropToken()) !== InsertResult.Failure) {
            // Our move goes to the opponent before anything else (game over waits for Esc).
            if (this.online?.started && mover !== PLAYER_SLOTS.online) this.online.sendMove(col);
            if (s.winOccurred || s.isDraw) await this.gameOver();
            else this.vs.display(Display.DisplayChanged);
          }
        }
        if (this.ingame && this.input.kbhit()) await this.extraFunctionality(await this.input.getKey());
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
