// Compare page (development only): the port's drawing of a known state next
// to a frame of the original saved from classic mode with
// window.saveReference(name) (Temp/Reference/*.png, which git ignores).
// Open compare.html?name=main-human; window.comparePixels() in the console
// compares the two window by window.
//
// Each reference needs a matching state below, set up as the original was
// when the frame was saved.

import { gameViewSys } from '../game/display';
import { GameState } from '../game/state';
import { Screen } from '../render/screen';
import { Display } from '../render/viewsys';

/** States matching the saved reference frames. */
const STATES: Record<string, (s: GameState) => void> = {
  // The release's NINARO.CFG (AI Random v AI Genius demo), then F7 for Human v Human.
  'main-human': (s) => {
    s.resetGameSize(7, 8, 4);
    s.playerCurrent = 1;
    s.entrygrid.empty();
    s.keypos = 5;
    s.entrygrid.insertTokenAtCR(5, 0, s.player(1));
    s.gameStatus();
    s.msgbox.empty();
    for (const line of [
      'Successive Player Goes First',
      'AI Genius Wins',
      'Successive Player Goes First',
      'AI Genius Wins',
      'Successive Player Goes First',
      '~Human vs Human',
      'Successive Player Goes First',
    ]) {
      s.msgbox.fixedAddLine(line);
    }
  },
};

const root = document.querySelector<HTMLElement>('#compare')!;
const name = new URLSearchParams(location.search).get('name') ?? 'main-human';

const pair = document.createElement('div');
pair.className = 'compare__pair';
const original = document.createElement('figure');
const img = document.createElement('img');
img.src = `${import.meta.env.BASE_URL}classic/reference/${name}.png`;
img.alt = `The original: ${name}`;
original.append(img, Object.assign(document.createElement('figcaption'), { textContent: `Original: ${name}` }));
const port = document.createElement('figure');
const canvas = document.createElement('canvas');
canvas.className = 'screen';
port.append(canvas, Object.assign(document.createElement('figcaption'), { textContent: 'Port' }));
pair.append(original, port);
root.append(pair);

const setup = STATES[name];
if (setup) {
  const screen = new Screen(canvas);
  const state = new GameState();
  setup(state);
  const vs = gameViewSys(screen.graph, canvas, state, { x1: 0, y1: 0, x2: 639, y2: 349 });
  const draw = () => vs.display(Display.DisplayAll);
  screen.onResize = draw;
  draw();
} else {
  port.append(`No state for "${name}". Known: ${Object.keys(STATES).join(', ')}`);
}

interface Box {
  x1: number;
  y1: number;
  x2: number;
  y2: number;
}

/** The lit (non-black) pixels' bounding box inside vp, or null. */
function inkBox(data: Uint8ClampedArray, vp: Box): Box | null {
  let box: Box | null = null;
  for (let y = vp.y1; y <= vp.y2; y++) {
    for (let x = vp.x1; x <= vp.x2; x++) {
      const k = (y * 640 + x) * 4;
      if (data[k] | data[k + 1] | data[k + 2]) {
        if (!box) box = { x1: x, y1: y, x2: x, y2: y };
        else {
          box.x1 = Math.min(box.x1, x);
          box.y1 = Math.min(box.y1, y);
          box.x2 = Math.max(box.x2, x);
          box.y2 = Math.max(box.y2, y);
        }
      }
    }
  }
  return box;
}

/**
 * For the console: draws the state at exactly 640 x 350 and compares it
 * with the reference, window by window: differing pixels and ink boxes.
 */
async function comparePixels(): Promise<string[]> {
  const setupState = STATES[name];
  if (!setupState) return [];
  const { Graph } = await import('../render/graph');
  const { Palette } = await import('../render/palette');
  const c = document.createElement('canvas');
  c.width = 640;
  c.height = 350;
  const ctx = c.getContext('2d')!;
  const state = new GameState();
  setupState(state);
  const vs = gameViewSys(new Graph(ctx, new Palette()), c, state, { x1: 0, y1: 0, x2: 639, y2: 349 });
  vs.display(Display.DisplayAll);
  const portData = ctx.getImageData(0, 0, 640, 350).data;
  await img.decode();
  const r = document.createElement('canvas');
  r.width = 640;
  r.height = 350;
  const rc = r.getContext('2d')!;
  rc.drawImage(img, 0, 0);
  const refData = rc.getImageData(0, 0, 640, 350).data;
  const fmt = (b: Box | null) => (b ? `${b.x1},${b.y1}-${b.x2},${b.y2}` : 'none');
  return vs.windows.map((w, i) => {
    let diff = 0;
    for (let y = w.vp.y1; y <= w.vp.y2; y++) {
      for (let x = w.vp.x1; x <= w.vp.x2; x++) {
        const k = (y * 640 + x) * 4;
        if (refData[k] !== portData[k] || refData[k + 1] !== portData[k + 1] || refData[k + 2] !== portData[k + 2]) diff++;
      }
    }
    return `${i} ${w.help}: diff ${diff}; ink original ${fmt(inkBox(refData, w.vp))} port ${fmt(inkBox(portData, w.vp))}`;
  });
}
(window as unknown as { comparePixels: typeof comparePixels }).comparePixels = comparePixels;

export {};
