// The menus, dialogs, help and About box as NINARO.C's ProgInit set them up,
// and the functions behind them (P1Dialog, GameSizeCustom, PlayerOptions,
// GameOptions, HelpAbout, ...).

import type { Viewport } from '../render/screen';
import { borderedBox, setViewport } from '../render/screen';
import { Color, EGA } from '../render/palette';
import { Display } from '../render/viewsys';
import { Control, Dialog, Show } from '../ui/dialog';
import { Help } from '../ui/help';
import { Menu } from '../ui/menu';
import { displayGameName } from './display';
import type { Game, GameUi } from './game';
import { GoesFirst, PLAYER_SLOTS } from './state';

const DIALOG_VP: Viewport = { x1: 50, y1: 50, x2: 590, y2: 300 };
const OPTIONS_VP: Viewport = { x1: 50, y1: 30, x2: 590, y2: 320 };
const HELP_VP: Viewport = { x1: 10, y1: 10, x2: 629, y2: 339 };
const ABOUT_VP: Viewport = { x1: 190, y1: 181, x2: 449, y2: 259 };

/** The About box's lines (ProgInit), as in the redacted source. */
const ABOUT = [
  "~Scroll Up/Down, Any Key OK's",
  'Project',
  'By Drew',
  '',
  '      Semester One 1998',
  '',
  '       Copyright 1998',
  '        Version 2.07',
];

/** The background option: _remappalette so BLACK and WHITE swap for a white background. */
export function applyBackground(game: Game): void {
  const p = game.g.palette;
  if (game.state.back === Color.BLACK) {
    p.remap(Color.BLACK, EGA._BLACK);
    p.remap(Color.WHITE, EGA._WHITE);
  } else {
    p.remap(Color.BLACK, EGA._BRIGHTWHITE);
    p.remap(Color.WHITE, EGA._BLACK);
  }
}

export class NinaroUi implements GameUi {
  readonly menu: Menu;
  readonly playerDialog: Dialog;
  readonly boardDialog: Dialog;
  readonly gameOptionsDialog: Dialog;
  readonly optionsDialog: Dialog;
  readonly helpBox: Help;
  readonly about: Help;
  /** Restarts the program after Exit (the original quit to DOS). */
  onExit: () => Promise<void> = async () => {};

  constructor(
    private readonly game: Game,
    helpText: string,
  ) {
    const { g, state, vs, pkeys } = game;
    const hilite = () => state.hilite;
    const text = () => state.text;
    const name = (slot: number) => state.player(slot).name;

    this.playerDialog = new Dialog(g, pkeys, (d, last) => this.modifyPlayer(d, last), 'Player One', hilite, [
      {
        name: 'Player',
        enabled: true,
        selectable: true,
        store: Control.list(Show.Radio, 3, 2, 13, [
          { value: 1, name: 'Human' },
          ...[2, 3, 4, 5, 6].map((slot) => ({ value: slot, name: name(slot) })),
        ]),
      },
      { name: 'Color', enabled: true, selectable: true, store: Control.range(Show.Color, 1, 10, 5, 1, '', 7, '') },
    ]);
    // The Connect slider's top is MaxOf(grid_rows, grid_cols), read before
    // the configuration loaded them, so 0 (known bug 17).
    this.boardDialog = new Dialog(g, pkeys, () => this.gameSizeFit(), 'Custom Board Size', hilite, [
      { name: 'Rows', enabled: true, selectable: true, store: Control.range(Show.Slider, 1, 1, 10, 3, '', 50, '') },
      { name: 'Columns', enabled: true, selectable: true, store: Control.range(Show.Slider, 1, 1, 10, 3, '', 50, '') },
      { name: 'Connect', enabled: true, selectable: true, store: Control.range(Show.Slider, 1, 1, 10, 3, '', 0, '') },
    ]);
    this.gameOptionsDialog = new Dialog(g, pkeys, null, 'Game Options', hilite, [
      {
        name: 'Background',
        enabled: true,
        selectable: true,
        store: Control.list(Show.Color, 1, 2, 15, [
          { value: Color.BLACK, name: '' },
          { value: Color.WHITE, name: '' },
        ]),
      },
      {
        name: 'Hilite',
        enabled: true,
        selectable: true,
        store: Control.list(Show.Color, 1, 5, 11, [Color.RED, Color.GREEN, Color.BLUE, Color.YELLOW, Color.ORANGE].map((value) => ({ value, name: '' }))),
      },
      {
        name: 'Mouse Use',
        enabled: true,
        selectable: true,
        store: Control.list(Show.Radio, 1, 3, 8, [
          { value: 0, name: 'None' },
          { value: 1, name: 'Partial' },
          { value: 2, name: 'Full' },
        ]),
      },
      // Shown instead of the choice when no mouse driver was found; a browser always has a pointer.
      { name: 'Mouse Use', enabled: false, selectable: false, store: Control.list(Show.Name, 1, 1, 24, [{ value: 0, name: 'Mouse Not Initialised' }]) },
    ]);
    this.optionsDialog = new Dialog(g, pkeys, null, 'Player Options', hilite, [
      {
        name: 'Goes First',
        enabled: true,
        selectable: true,
        store: Control.list(
          Show.Radio,
          3,
          2,
          13,
          ['Player One', 'Player Two', 'Loser', 'Winner', 'Successive', 'Random'].map((n, value) => ({ value, name: n })),
        ),
      },
      {
        name: 'AI Speed',
        enabled: true,
        selectable: true,
        store: Control.list(Show.Slider, 1, 1, 7, [
          { value: 4, name: 'Slowest' },
          { value: 3, name: 'Slow' },
          { value: 2, name: 'Normal' },
          { value: 1, name: 'Fast' },
          { value: 0, name: 'Fastest' },
        ]),
      },
      {
        name: 'Best Of',
        enabled: true,
        selectable: true,
        store: Control.list(Show.Slider, 1, 2, 8, [
          ...[3, 5, 11, 25, 51, 101, 1000].map((value) => ({ value, name: String(value) })),
          { value: 0, name: 'Infinity' },
        ]),
      },
    ]);

    this.menu = new Menu(g, vs.main, vs.current, pkeys, hilite);
    const item = (n: string, run: () => void | Promise<void>, enabled = true) => ({ name: n, shortcut: -1, run, enabled });
    this.menu.addSub('Game', [
      item('New', () => game.newGame(), false),
      item('Load', () => game.loadGame()),
      item('Save', () => game.saveGame()),
      item('End', () => game.endGame()),
      item('Suggest Move', () => game.suggestMove()),
      item('Toggle Sound', () => game.toggleSound()),
      item('Options...', () => this.gameOptions()),
      item('Exit', () => this.exit()),
    ]);
    this.menu.addSub('Board', [
      item('7x8x4', () => game.gameSize7x8x4()),
      item('12x13x6', () => game.gameSize12x13x6()),
      item('Custom...', () => this.gameSizeCustom()),
    ]);
    this.menu.addSub('Player', [
      item('One...', () => this.pDialog(1), false),
      item('Two...', () => this.pDialog(2)),
      item('Options...', () => this.playerOptions()),
      item('Reset Scores', () => game.resetScores()),
    ]);
    this.menu.addSub('Help', [item('Contents', () => this.helpContentsFunc()), item('About', () => this.helpAbout())]);

    this.helpBox = new Help(g, helpText, pkeys, 20, Color.RED, text, hilite, 60);
    this.about = new Help(g, null, pkeys, 3, Color.LTBLUE, text, hilite, 29);
    for (const line of ABOUT) this.about.addLine(line);
  }

  private get useMouse(): boolean {
    return this.game.mouseUsage === 2;
  }

  // ---- GameUi ----

  /** KeyUp's MenuCall, with HelpMenu for help. */
  async menuCall(): Promise<void> {
    await this.game.withOverlay(
      () => this.menu.display(),
      () => this.menu.call(this.game.input, () => this.helpMenu(), this.useMouse),
    );
  }

  async help(): Promise<void> {
    await this.helpContentsFunc();
  }

  menuItemEnable(sub: number, item: number, enable: boolean): void {
    this.menu.itemEnable(sub, item, enable);
  }

  menuSubCurrentSet(sub: number, item: number): void {
    this.menu.subCurrentSet(sub, item);
  }

  /** ExitGame: the original saved the configuration and quit to DOS. */
  async exit(): Promise<void> {
    await this.onExit();
  }

  // ---- Help ----

  /** HelpContents */
  private async helpContents(): Promise<void> {
    const { vs, input } = this.game;
    await this.game.withOverlay(
      () => {
        vs.current.put();
        this.helpBox.display(HELP_VP);
      },
      () => this.helpBox.call(HELP_VP, vs.current, input),
    );
  }

  /** HelpContentsFunc */
  async helpContentsFunc(): Promise<void> {
    this.helpBox.home();
    await this.helpContents();
  }

  /** HelpPlayerDialog, HelpBoardDialog, ...: help scrolled to a heading. */
  private async helpAt(heading: string): Promise<void> {
    this.helpBox.home();
    this.helpBox.scrollString(heading);
    await this.helpContents();
  }

  /** HelpMenu: help on the open menu. */
  private async helpMenu(): Promise<void> {
    const headings = ['~The Game Menu', '~The Board Menu', '~The Player Menu', '~The Help Menu'];
    this.helpBox.home();
    const heading = headings[this.menu.current];
    if (heading) this.helpBox.scrollString(heading);
    await this.helpContents();
  }

  /** HelpAbout */
  async helpAbout(): Promise<void> {
    const { g, vs } = this.game;
    vs.display(Display.DisplayCurrent);
    g.setViewport(0, 0, 639, 349);
    borderedBox(g, 180, 100, 459, 269, Color.GREY, Color.LTBLUE);
    borderedBox(g, 190, 110, 449, 179, Color.BLACK, Color.LTBLUE);
    setViewport(g, { x1: 190, y1: 110, x2: 449, y2: 179 });
    displayGameName(g);
    vs.current.get(); // the About box is now part of the saved screen, as in the original
    await this.game.withOverlay(
      () => this.about.display(ABOUT_VP),
      () => this.about.call(ABOUT_VP, vs.current, this.game.input),
    );
  }

  /** DialogCall, with help at `heading`, redrawn after a resize. */
  private runDialog(d: Dialog, vp: Viewport, namefract: number, chars: number, heading: string): Promise<boolean> {
    const { game } = this;
    return game.withOverlay(
      () => d.display(vp, game.vs.current, namefract, chars),
      () => d.call(vp, game.vs.current, namefract, chars, () => this.helpAt(heading), game.input, this.useMouse),
    );
  }

  // ---- Board ----

  /** GameSizeFit: keeps Connect within the larger of Rows and Columns. */
  private gameSizeFit(): void {
    const d = this.boardDialog;
    const rows = d.control(0).current;
    const cols = d.control(1).current;
    const size = Math.max(cols, rows);
    const gsize = d.control(2);
    if (gsize.current > size) gsize.currentSet(size);
    gsize.resetRange(3, '', size, '');
  }

  /** GameSizeCustom */
  async gameSizeCustom(): Promise<void> {
    const { game } = this;
    const s = game.state;
    const d = this.boardDialog;
    d.stateSet([s.gridRows, s.gridCols, s.win.tokens, 0]);
    this.gameSizeFit();
    if (await this.runDialog(d, DIALOG_VP, 5, 8, '~The Custom Board Dialog')) {
      const [rows, cols, inARow] = d.stateGet();
      game.resetGameSize(cols, rows, inARow);
      s.gameStatus();
    }
    game.resetScores();
  }

  // ---- Players ----

  /** SetupPlayer: the dialog set to a player's slot and colour. */
  private setupPlayer(slot: number): void {
    const { state: s } = this.game;
    const d = this.playerDialog;
    if (slot === s.playerOne) d.name = 'Player One';
    else if (slot === s.playerTwo) d.name = 'Player Two';
    else d.name = 'New Player';
    if (slot === 0 || slot === 1) d.control(0).currentSetTo(slot);
    else if (slot === PLAYER_SLOTS.copy) d.control(0).currentSetTo(s.players.who(s.player(PLAYER_SLOTS.copy).name));
    else d.control(0).currentSetTo(slot);
    d.control(1).currentSetTo(s.player(slot).color);
  }

  /** ModifyPlayer: keeps the two players' colours different. */
  private modifyPlayer(d: Dialog, last: number): void {
    const { state } = this.game;
    const names = d.control(0);
    const current = d.current;
    if (current !== 0 && current !== 1) return;
    if (current === 0) {
      const p = names.valueGet(names.current)!;
      d.control(1).currentSetTo(state.player(p).color);
    }
    // case 1 (and on from case 0): which human this dialog is for.
    const human = names.valueGet(0)!;
    const notColor = state.player(human === 0 ? state.playerTwo : state.playerOne).color;
    const color = d.control(1);
    if (color.current === notColor) {
      if (last === 0) {
        if (!color.prev()) color.next();
      } else if (last === 1) {
        if (!color.next()) color.prev();
      }
    }
  }

  /** PlayerDialog: the player chosen, or -1 if unchanged. */
  private async choosePlayer(pnum: number): Promise<number> {
    const { game } = this;
    const s = game.state;
    const d = this.playerDialog;
    this.setupPlayer(pnum);
    if (await this.runDialog(d, DIALOG_VP, 5, 8, '~The Player Dialog')) {
      let p = d.control(0).valueGet(d.control(0).current)!;
      if (p === 0 || p === 1) {
        // a human
      } else if ((pnum === s.playerTwo && p === s.playerOne) || (pnum === s.playerOne && p === s.playerTwo)) {
        // Both players the same AI: player two gets a copy (PlayerCopy).
        const copy = s.player(PLAYER_SLOTS.copy);
        const source = s.player(p);
        copy.name = source.name;
        copy.color = source.color;
        copy.type = source.type;
        s.copyOf = p;
        p = PLAYER_SLOTS.copy;
      }
      s.player(p).color = d.control(1).current;
      return p === pnum ? -1 : p;
    }
    return -1;
  }

  /** P1Dialog (n = 1) and P2Dialog (n = 2) */
  async pDialog(n: 1 | 2): Promise<void> {
    const { game } = this;
    const s = game.state;
    this.playerDialog.control(0).elemSet(0, n - 1, 'Human');
    const player = await this.choosePlayer(n === 1 ? s.playerOne : s.playerTwo);
    if (player === -1) return;
    if (n === 1) game.setP1(player);
    else game.setP2(player);
    if (game.ingame) {
      game.newGame();
      game.resetScores();
    } else if (n === 1) s.playerOneScore = 0;
    else s.playerTwoScore = 0;
  }

  /** PlayerOptions */
  async playerOptions(): Promise<void> {
    const { game } = this;
    const s = game.state;
    const d = this.optionsDialog;
    d.control(0).currentSetTo(s.goesFirst);
    d.control(1).currentSetTo(s.aiSpeed);
    d.control(2).currentSetTo(s.bestOf);
    if (await this.runDialog(d, OPTIONS_VP, 5, 11, '~The Player Options Dialog')) {
      s.goesFirst = d.control(0).current as GoesFirst;
      s.aiSpeed = d.control(1).valueGet(d.control(1).current)!;
      s.bestOf = d.control(2).valueGet(d.control(2).current)!;
    }
  }

  /** GameOptions: background, hilite and mouse use (used whether OK'd or not, as the original did). */
  async gameOptions(): Promise<void> {
    const { game } = this;
    const s = game.state;
    const d = this.gameOptionsDialog;
    const c1 = d.control(0);
    const c2 = d.control(1);
    const c3 = d.control(2);
    if (s.back === Color.BLACK) {
      c1.elemSet(0, Color.BLACK, '');
      c1.elemSet(1, Color.WHITE, '');
      c1.currentSet(0);
    } else {
      c1.elemSet(0, Color.WHITE, '');
      c1.elemSet(1, Color.BLACK, '');
      c1.currentSet(1);
    }
    c2.currentSetTo(s.hilite);
    c3.currentSetTo(game.mouseUsage);
    await this.runDialog(d, DIALOG_VP, 5, 11, '~The Game Options Dialog');
    s.back = c1.current === 0 ? Color.BLACK : Color.WHITE;
    applyBackground(game);
    s.hilite = c2.valueGet(c2.current)!;
    game.mouseUsage = c3.valueGet(c3.current)!;
  }
}
