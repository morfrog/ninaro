// Ninaro web port: the title screen, then the game with its menus, dialogs
// and help, as the original's main() ran them.

import helpText from '../../Legacy/NINARO.HLP?raw';
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

// Browsers only allow sound after the player has pressed a key or clicked.
for (const type of ['keydown', 'pointerdown'] as const) {
  window.addEventListener(type, () => game.speaker.unlock(), { capture: true });
}

// Buttons for players without a keyboard: the same as Esc and F1.
document.querySelector('#menu-button')?.addEventListener('click', () => input.pushKey(ESC));
document.querySelector('#help-button')?.addEventListener('click', () => input.pushKey(F1));

let ttl = new Uint8Array();
let inTitle = false;
screen.onResize = () => {
  if (!inTitle) game.redrawAll();
};

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
  game.ingame = false;
  state.resetMessages();
  await program();
};

void (async () => {
  ttl = new Uint8Array(await (await fetch(ttlUrl)).arrayBuffer());
  await program();
  await game.run();
})();
