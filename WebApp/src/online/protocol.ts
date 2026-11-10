// The online game's messages (ONLINE-PLAN.md). Each side writes only its
// own key, and each write is that side's whole state, so the latest value is
// always enough and writing it again is harmless:
//
//   1,<role>,<nonce>,<cols>x<rows>x<connect>,<color>,<phase>,<game>,<move>,<col>,<beat>
//
// For phase S (start) the move field says who goes first: h or g.

export const PROTOCOL_VERSION = 1;

/** Host or guest. */
export type Role = 'h' | 'g';

/** W waiting / joining, S start (host), P playing, X aborted, Q quit. */
export type Phase = 'W' | 'S' | 'P' | 'X' | 'Q';
const PHASES: readonly string[] = ['W', 'S', 'P', 'X', 'Q'];

export interface Board {
  cols: number;
  rows: number;
  connect: number;
}

export interface Message {
  role: Role;
  /** 4 base-36 characters, new each session. */
  nonce: string;
  board: Board;
  /** The sender's token colour, palette index 0..15. */
  color: number;
  phase: Phase;
  /** Game number in the session, from 1 (0 before the first game starts). */
  game: number;
  /** Number of the sender's latest move in this game (0 = none yet). */
  move: number;
  /** Column of that move, 0-based. */
  col: number;
  /** Heartbeat, raised while waiting so the other side knows we're still here. */
  beat: number;
  /** Phase S only: who goes first. */
  first: Role | null;
}

/** Counters wrap here, which keeps every message well under 64 characters. */
export const COUNTER_LIMIT = 1_000_000;

const NONCE = /^[0-9a-z]{4}$/;
const COUNT = /^(0|[1-9][0-9]{0,5})$/;
const BOARD = /^([0-9]{1,2})x([0-9]{1,2})x([0-9]{1,2})$/;

/** A new nonce: 4 base-36 characters. */
export function newNonce(random: () => number = Math.random): string {
  let s = '';
  for (let i = 0; i < 4; i++) s += Math.floor(random() * 36).toString(36);
  return s;
}

/** Whether a board is one the game can play (as the Custom Board dialog allows). */
export function validBoard(b: Board): boolean {
  const inRange = (v: number, lo: number, hi: number) => Number.isInteger(v) && v >= lo && v <= hi;
  return inRange(b.cols, 3, 50) && inRange(b.rows, 3, 50) && inRange(b.connect, 3, Math.max(b.cols, b.rows));
}

export function sameBoard(a: Board, b: Board): boolean {
  return a.cols === b.cols && a.rows === b.rows && a.connect === b.connect;
}

/** "7x8@4", as the toolbar shows a board. */
export function boardText(b: Board): string {
  return `${b.cols}x${b.rows}@${b.connect}`;
}

/** The message as the store holds it. Throws if any field is out of range. */
export function encodeMessage(m: Message): string {
  const count = (v: number) => Number.isInteger(v) && v >= 0 && v < COUNTER_LIMIT;
  if (!NONCE.test(m.nonce)) throw new Error(`Bad nonce: ${m.nonce}`);
  if (!validBoard(m.board)) throw new Error(`Bad board: ${boardText(m.board)}`);
  if (!Number.isInteger(m.color) || m.color < 0 || m.color > 15) throw new Error(`Bad colour: ${m.color}`);
  if (!count(m.game) || !count(m.move) || !count(m.beat)) throw new Error('Bad counter');
  if (!Number.isInteger(m.col) || m.col < 0 || m.col >= m.board.cols) throw new Error(`Bad column: ${m.col}`);
  if ((m.phase === 'S') !== (m.first !== null)) throw new Error('Only a start message says who goes first');
  const move = m.phase === 'S' ? m.first : String(m.move);
  const { cols, rows, connect } = m.board;
  return [
    PROTOCOL_VERSION,
    m.role,
    m.nonce,
    `${cols}x${rows}x${connect}`,
    m.color.toString(16),
    m.phase,
    m.game,
    move,
    m.col,
    m.beat,
  ].join(',');
}

/** The protocol version a stored value claims, or null if it isn't one of ours. */
export function messageVersion(text: string): number | null {
  const m = /^([1-9][0-9]{0,2}),/.exec(text);
  return m ? Number(m[1]) : null;
}

/** A stored value as a message, or null if it isn't a valid one of this version. */
export function parseMessage(text: string): Message | null {
  const f = text.split(',');
  if (f.length !== 10) return null;
  const [version, role, nonce, board, color, phase, game, move, col, beat] = f;
  if (version !== String(PROTOCOL_VERSION)) return null;
  if (role !== 'h' && role !== 'g') return null;
  if (!NONCE.test(nonce)) return null;
  const b = BOARD.exec(board);
  if (!b) return null;
  const parsedBoard = { cols: Number(b[1]), rows: Number(b[2]), connect: Number(b[3]) };
  if (!validBoard(parsedBoard)) return null;
  if (!/^[0-9a-f]$/.test(color)) return null;
  if (!PHASES.includes(phase)) return null;
  if (!COUNT.test(game) || !COUNT.test(col) || !COUNT.test(beat)) return null;
  let first: Role | null = null;
  let moveNum = 0;
  if (phase === 'S') {
    if (move !== 'h' && move !== 'g') return null;
    first = move;
  } else {
    if (!COUNT.test(move)) return null;
    moveNum = Number(move);
  }
  const column = Number(col);
  if (column >= parsedBoard.cols) return null;
  return {
    role,
    nonce,
    board: parsedBoard,
    color: parseInt(color, 16),
    phase: phase as Phase,
    game: Number(game),
    move: moveNum,
    col: column,
    beat: Number(beat),
    first,
  };
}

/** The room as typed (or from ?online=), made safe for a key: a-z, 0-9 and -, at most 24 characters. */
export function cleanRoom(name: string, fallback = 'ninaro'): string {
  const room = name
    .toLowerCase()
    .replace(/[^a-z0-9]+/g, '-')
    .replace(/^-+|-+$/g, '')
    .slice(0, 24)
    .replace(/-+$/, '');
  return room || fallback;
}

/** The room's key for one side: ninaro1-<room>-h or -g. */
export function roomKey(room: string, role: Role): string {
  return `ninaro${PROTOCOL_VERSION}-${cleanRoom(room)}-${role}`;
}
