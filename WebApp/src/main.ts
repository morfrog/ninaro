// Ninaro web port. Phase 2: the game screen as the original drew it at the
// start of a game (Human vs Human on the default 7 x 8 board, connect 4).
// The game itself comes in phase 3.

import { gameViewSys } from './game/display';
import { GameState } from './game/state';
import { Display } from './render/viewsys';
import { Screen } from './render/screen';
import './devLink';

const canvas = document.querySelector<HTMLCanvasElement>('#screen')!;
const screen = new Screen(canvas);
const state = new GameState();
state.resetGameSize(7, 8, 4);
state.gameStatus();
state.msgbox.fixedAddLine('Successive Player Goes First');
state.msgbox.fixedAddLine('Currently Playing Game.');

const vs = gameViewSys(screen.graph, canvas, state, { x1: 0, y1: 0, x2: 639, y2: 349 });
const draw = () => vs.display(Display.DisplayAll);
screen.onResize = draw;
draw();
