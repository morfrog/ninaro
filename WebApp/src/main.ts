// Ninaro web port: the title screen, then the game with its menus, dialogs
// and help, as the original's main() ran them.

import helpText from '../../Legacy/NINARO.HLP?raw';
import { decodeConfig } from './data/config';
import { loadDisplayMode } from './render/displayMode';
import { loadViewMode } from './render/viewMode';
import { CONFIG_KEY, ONLINE_KEY, SAVE_KEY, StoredFile } from './data/storage';
import ttlUrl from '../../Legacy/NINARO.TTL?url';
import { gameViewSys } from './game/display';
import { Game } from './game/game';
import { GameState, PLAYER_SLOTS } from './game/state';
import { titleScreen } from './game/title';
import { NinaroUi, applyBackground, applyDisplayMode } from './game/ui';
import { Input } from './input/input';
import { ESC } from './input/keys';
import { DEFAULT_ROOM, askRoom, initialRoom, roomFromUrl, saveRoom } from './ui/roomPrompt';
import { withOnlineHelp } from './game/onlineHelp';
import { Screen } from './render/screen';
import './devLink';

const canvas = document.querySelector<HTMLCanvasElement>('#screen')!;
const screen = new Screen(canvas);
const state = new GameState();
const vs = gameViewSys(screen.graph, canvas, state, { x1: 0, y1: 0, x2: 639, y2: 349 });
const input = new Input();
input.attach(window, canvas, (x, y) => screen.toScreen(x, y));

const game = new Game(state, screen.graph, vs, input);
const ui = new NinaroUi(game, withOnlineHelp(helpText));
ui.askRoom = () => askRoom(initialRoom(location.search));
game.ui = ui;

// NINARO.CFG and NINARO.SAV live in the browser's storage. LoadCfg: the
// settings from last time, if there are any (ProgInit's defaults if not).
game.configFile = new StoredFile(CONFIG_KEY);
game.saveFile = new StoredFile(SAVE_KEY);
game.onlineFile = new StoredFile(ONLINE_KEY);
// With no settings yet (a first visit), player two is AI Normal, so a new
// player can start playing at once; the original started with two humans.
const cfg = game.configFile.read();
if (!cfg || !decodeConfig(cfg, game)) game.setP2(PLAYER_SLOTS.aiNormal);

// The original saved its settings when it quit; a page can be closed at any
// time, so they are also saved whenever it is hidden or closed.
// Not during the title screen: it plays on its own 36x9 board, which would be
// saved as the player's board.
window.addEventListener('pagehide', () => {
  if (!inTitle) game.saveConfig();
  game.leaveOnlineNow(); // one try at telling an online opponent
});
document.addEventListener('visibilitychange', () => {
  if (document.visibilityState === 'hidden' && !inTitle) game.saveConfig();
});

// Browsers only allow sound after the player has pressed a key or clicked.
for (const type of ['keydown', 'pointerdown'] as const) {
  window.addEventListener(type, () => game.speaker.unlock(), { capture: true });
}

// The toolbar's Menu button, for players without a keyboard: the same as Esc.
// On the title screen it reads Play, and Esc (or a click) starts the game.
const menuButton = document.querySelector<HTMLButtonElement>('#menu-button');
menuButton?.addEventListener('click', () => input.pushKey(ESC));

let ttl = new Uint8Array();
let inTitle = false;
screen.onResize = () => {
  if (!inTitle) game.redrawAll();
};

// The Display option (in the menu bar): how the screen is drawn. Monochrome
// changes how colours are drawn; Modern is drawn as VGA until its own step.
applyDisplayMode(game, loadDisplayMode());

// The View option (in the menu bar): Normal, or Fullscreen (the board only).
// The page follows the game through onView: the screen is cropped to the
// board, and the title and board size appear in the toolbar.
const frame = document.querySelector<HTMLElement>('#screen-frame');
const boardTitle = document.querySelector<HTMLElement>('#board-title');
const boardReadout = document.querySelector<HTMLElement>('#board-readout');
const boardPlayers = document.querySelector<HTMLElement>('#board-players');
game.onView = (view) => {
  document.documentElement.dataset.view = view.fullscreen ? 'fullscreen' : 'normal';
  frame?.classList.toggle('screen-frame--cropped', view.cropped);
  if (boardTitle) boardTitle.hidden = !view.fullscreen;
  if (boardReadout && boardReadout.textContent !== view.readout) boardReadout.textContent = view.readout;
  if (boardPlayers) {
    const key = `${view.players.one}|${view.players.two}`;
    if (boardPlayers.dataset.players !== key) {
      boardPlayers.dataset.players = key;
      const vs = document.createElement('span');
      vs.className = 'toolbar__vs';
      vs.textContent = 'vs';
      boardPlayers.replaceChildren(view.players.one, vs, view.players.two);
    }
  }
};
game.setViewMode(loadViewMode());

/** main(): the title screen, then the first game on the configured board. */
async function program(): Promise<void> {
  const size = { cols: state.gridCols, rows: state.gridRows, connect: state.win.tokens };
  game.setScreenOn(false);
  inTitle = true;
  if (menuButton) menuButton.textContent = 'Play';
  await titleScreen(game, ttl);
  inTitle = false;
  if (menuButton) menuButton.textContent = 'Menu';
  applyBackground(game);
  game.start(undefined, size);
  // As the page opens (once): ?online=<room> makes player two Online in that
  // room; otherwise a player who was Online last time is again, in the last room.
  if (!opened) {
    opened = true;
    const urlRoom = roomFromUrl(location.search);
    const side = game.savedOnlineSide();
    if (urlRoom !== null) {
      const room = urlRoom || DEFAULT_ROOM;
      saveRoom(room);
      void game.goOnline(room, 2);
    } else if (side !== null) {
      void game.goOnline(initialRoom(''), side);
    }
  }
}

let opened = false;

// Exit: the original saved its configuration and quit to DOS; here the
// program starts again from the title screen.
ui.onExit = async () => {
  game.saveConfig();
  game.leaveOnline();
  game.ingame = false;
  state.resetMessages();
  await program();
};

// The menu's Exit goes to the home page instead.
const HOME_URL = 'https://morfrog.github.io/';
ui.onHome = async () => {
  game.saveConfig();
  game.leaveOnline();
  window.location.href = HOME_URL;
};

void (async () => {
  ttl = new Uint8Array(await (await fetch(ttlUrl)).arrayBuffer());
  await program();
  await game.run();
})();
