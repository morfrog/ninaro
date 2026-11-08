// Ninaro web port. Phase 3: the game is playable from the keyboard, mouse
// or touch screen. Menus, help and the title screen come in phase 4.

import { gameViewSys } from './game/display';
import { Game } from './game/game';
import { GameState } from './game/state';
import { Input } from './input/input';
import { Display } from './render/viewsys';
import { Screen } from './render/screen';
import './devLink';

const canvas = document.querySelector<HTMLCanvasElement>('#screen')!;
const screen = new Screen(canvas);
const state = new GameState();
const vs = gameViewSys(screen.graph, canvas, state, { x1: 0, y1: 0, x2: 639, y2: 349 });
const input = new Input();
input.attach(window, canvas, (x, y) => screen.toScreen(x, y));

const game = new Game(state, screen.graph, vs, input);
screen.onResize = () => vs.display(Display.DisplayAll);

// Browsers only allow sound after the player has pressed a key or clicked.
for (const type of ['keydown', 'pointerdown'] as const) {
  window.addEventListener(type, () => game.speaker.unlock(), { capture: true });
}

game.start();
void game.run();
