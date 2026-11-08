// The AI tournament page: runs a tournament in a worker (or on the page if
// workers aren't available) and shows who beat whom.

import { AI_NAMES } from '../core/ai';
import { BOARDS, LEVELS, runTournament, totals, validBoard, type Board, type Tally, type TournamentResult } from './runner';
import type { TournamentRequest } from './tournament.worker';

const $ = <T extends HTMLElement>(id: string) => document.getElementById(id) as T;
const form = $<HTMLFormElement>('controls');
const boardSelect = $<HTMLSelectElement>('board');
const custom = $<HTMLElement>('custom');
const status = $<HTMLElement>('status');
const results = $<HTMLElement>('results');
const runButton = $<HTMLButtonElement>('run');
const num = (id: string) => Number($<HTMLInputElement>(id).value);

boardSelect.addEventListener('change', () => {
  custom.hidden = boardSelect.value !== 'custom';
});

function chosenBoard(): Board {
  return boardSelect.value === 'custom' ? { cols: num('cols'), rows: num('rows'), connect: num('connect') } : BOARDS[boardSelect.value];
}

function el<K extends keyof HTMLElementTagNameMap>(tag: K, text?: string, className?: string): HTMLElementTagNameMap[K] {
  const e = document.createElement(tag);
  if (text !== undefined) e.textContent = text;
  if (className) e.className = className;
  return e;
}

const pct = (n: number, of: number) => (of ? `${Math.round((100 * n) / of)}%` : '-');

/** "wins–draws–losses" for a cell, with the win share. */
function cell(t: Tally): HTMLTableCellElement {
  const td = el('td');
  const games = t.wins + t.draws + t.losses;
  td.append(el('span', `${t.wins}–${t.draws}–${t.losses}`, 'tournament__score'), el('span', pct(t.wins, games), 'tournament__pct'));
  td.title = `${t.wins} wins, ${t.draws} draws, ${t.losses} losses`;
  return td;
}

function showResults(r: TournamentResult): void {
  results.replaceChildren();
  const { cols, rows, connect } = r.board;

  const ranking = totals(r)
    .map((t, level) => ({ level, ...t }))
    .sort((a, b) => b.wins - a.wins || a.losses - b.losses);
  const summary = el('div', undefined, 'tournament__table');
  summary.append(el('h2', `Standings: ${cols} × ${rows}, connect ${connect}`));
  summary.append(el('p', `Against the other four AIs, ${r.games} games a pairing as player one and again as player two.`, 'tournament__note'));
  const st = el('table');
  const sh = el('tr');
  for (const h of ['', 'AI', 'Won', 'Drawn', 'Lost', 'Won %']) sh.append(el('th', h));
  st.append(el('thead'));
  st.tHead!.append(sh);
  const sb = el('tbody');
  ranking.forEach((x, i) => {
    const tr = el('tr');
    const games = x.wins + x.draws + x.losses;
    tr.append(el('td', String(i + 1)), el('th', AI_NAMES[x.level]), el('td', String(x.wins)), el('td', String(x.draws)), el('td', String(x.losses)), el('td', pct(x.wins, games)));
    sb.append(tr);
  });
  st.append(sb);
  summary.append(wrap(st));
  results.append(summary);

  const grid = el('div', undefined, 'tournament__table');
  grid.append(el('h2', 'Every pairing'));
  grid.append(el('p', 'Each row is player one, each column player two: player one’s wins–draws–losses, and their share of wins.', 'tournament__note'));
  const t = el('table');
  const head = el('tr');
  head.append(el('th', 'One \\ Two'));
  for (let b = 0; b < LEVELS; b++) head.append(el('th', AI_NAMES[b].replace('AI ', '')));
  t.append(el('thead'));
  t.tHead!.append(head);
  const body = el('tbody');
  for (let a = 0; a < LEVELS; a++) {
    const tr = el('tr');
    tr.append(el('th', AI_NAMES[a]));
    for (let b = 0; b < LEVELS; b++) tr.append(cell(r.cells[a][b]));
    body.append(tr);
  }
  t.append(body);
  grid.append(wrap(t));
  results.append(grid);
}

/** A table in a box that scrolls sideways on narrow screens. */
function wrap(table: HTMLTableElement): HTMLElement {
  const div = el('div', undefined, 'tournament__scroll');
  div.tabIndex = 0;
  div.append(table);
  return div;
}

function run(req: TournamentRequest): Promise<TournamentResult> {
  const progress = (done: number, total: number) => {
    status.textContent = `Playing... ${done} of ${total} pairings`;
  };
  try {
    const worker = new Worker(new URL('./tournament.worker.ts', import.meta.url), { type: 'module' });
    return new Promise((resolve, reject) => {
      worker.onmessage = (e) => {
        if (e.data.type === 'progress') progress(e.data.done, e.data.total);
        else {
          worker.terminate();
          resolve(e.data.result);
        }
      };
      worker.onerror = (e) => {
        worker.terminate();
        reject(new Error(e.message));
      };
      worker.postMessage(req);
    });
  } catch {
    return Promise.resolve(runTournament(req.board, req.games, req.seed, progress));
  }
}

form.addEventListener('submit', async (e) => {
  e.preventDefault();
  const board = chosenBoard();
  const games = Math.max(2, Math.min(1000, Math.round(num('games') / 2) * 2));
  const seed = Math.max(0, Math.min(4294967295, Math.trunc(num('seed'))));
  if (!validBoard(board)) {
    status.textContent = 'A board needs 3 to 50 columns and rows, and a connect of 3 up to the larger of the two.';
    status.classList.add('is-bad');
    return;
  }
  status.classList.remove('is-bad');
  runButton.disabled = true;
  const started = performance.now();
  try {
    const r = await run({ board, games, seed });
    const seconds = ((performance.now() - started) / 1000).toFixed(1);
    status.textContent = `Played ${25 * games} games in ${seconds} s (seed ${seed}).`;
    showResults(r);
  } catch (error) {
    status.textContent = error instanceof Error ? error.message : String(error);
    status.classList.add('is-bad');
  } finally {
    runButton.disabled = false;
  }
});

form.requestSubmit();
