// Ninaro web port: the title screen, then the game with its menus, dialogs
// and help, as the original's main() ran them.

import helpText from '../../Legacy/NINARO.HLP?raw';
import { decodeConfig } from './data/config';
import { loadDisplayMode, parseDisplayMode, saveDisplayMode, type DisplayMode } from './render/displayMode';
import { MODERN_PALETTE, NINARO_PALETTE } from './render/palette';
import { CONFIG_KEY, SAVE_KEY, StoredFile } from './data/storage';
import ttlUrl from '../../Legacy/NINARO.TTL?url';
import { gameViewSys } from './game/display';
import { Game } from './game/game';
import { GameState } from './game/state';
import { titleScreen } from './game/title';
import { NinaroUi, applyBackground } from './game/ui';
import { Input } from './input/input';
import { ESC, F1 } from './input/keys';
import { Screen } from './render/screen';
import './devLink';

const canvas = document.querySelector<HTMLCanvasElement>('#screen')!;
const screen = new Screen(canvas);
const state = new GameState();
const vs = gameViewSys(screen.graph, canvas, state, { x1: 0, y1: 0, x2: 639, y2: 349 });
const input = new Input();
input.attach(window, canvas, (x, y) => screen.toScreen(x, y));

const game = new Game(state, screen.graph, vs, input);
const ui = new NinaroUi(game, helpText);
game.ui = ui;

// NINARO.CFG and NINARO.SAV live in the browser's storage. LoadCfg: the
// settings from last time, if there are any (ProgInit's defaults if not).
game.configFile = new StoredFile(CONFIG_KEY);
game.saveFile = new StoredFile(SAVE_KEY);
const cfg = game.configFile.read();
if (cfg) decodeConfig(cfg, game);

// The original saved its settings when it quit; a page can be closed at any
// time, so they are also saved whenever it is hidden or closed.
window.addEventListener('pagehide', () => game.saveConfig());
document.addEventListener('visibilitychange', () => {
  if (document.visibilityState === 'hidden') game.saveConfig();
});

// Browsers only allow sound after the player has pressed a key or clicked.
for (const type of ['keydown', 'pointerdown'] as const) {
  window.addEventListener(type, () => game.speaker.unlock(), { capture: true });
}

// Buttons for players without a keyboard: the same as Esc and F1.
document.querySelector('#menu-button')?.addEventListener('click', () => input.pushKey(ESC));
document.querySelector('#help-button')?.addEventListener('click', () => input.pushKey(F1));

// The Display option: remembered in the browser and changeable at any time.
// Monochrome changes how colours are drawn; Modern is drawn as VGA until its
// own step.
let ttl = new Uint8Array();
let inTitle = false;
screen.onResize = () => {
  if (!inTitle) game.redrawAll();
};

const displaySelect = document.querySelector<HTMLSelectElement>('#display-mode');
if (displaySelect) {
  /** Modern has its own palette; VGA and Monochrome use the original's. */
  const applyMode = (mode: DisplayMode) => {
    document.documentElement.dataset.display = mode;
    screen.graph.palette.setBase(mode === 'modern' ? MODERN_PALETTE : NINARO_PALETTE);
    screen.graph.setDisplay(mode);
    applyBackground(game);
  };
  const initial: DisplayMode = loadDisplayMode();
  applyMode(initial);
  displaySelect.value = initial;
  displaySelect.addEventListener('change', () => {
    const mode = parseDisplayMode(displaySelect.value);
    saveDisplayMode(mode);
    applyMode(mode);
    if (!inTitle) game.redrawAll();
  });
}

/** main(): the title screen, then the first game on the configured board. */
async function program(): Promise<void> {
  const size = { cols: state.gridCols, rows: state.gridRows, connect: state.win.tokens };
  inTitle = true;
  await titleScreen(game, ttl);
  inTitle = false;
  applyBackground(game);
  game.start(undefined, size);
}

// Exit: the original saved its configuration and quit to DOS; here the
// program starts again from the title screen.
ui.onExit = async () => {
  game.saveConfig();
  game.ingame = false;
  state.resetMessages();
  await program();
};

// The menu's Exit goes to the home page instead.
const HOME_URL = 'https://morfrog.github.io/';
ui.onHome = async () => {
  game.saveConfig();
  window.location.href = HOME_URL;
};

void (async () => {
  ttl = new Uint8Array(await (await fetch(ttlUrl)).arrayBuffer());
  await program();
  await game.run();
})();
