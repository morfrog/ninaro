// The room box for playing Online: a real text box over the canvas (the
// canvas dialogs have none, and a phone only shows its keyboard for one),
// with Connect, Copy link and Cancel. The room comes from ?online=, then the
// last room used, then "ninaro".

import { cleanRoom } from '../online/protocol';

export const ROOM_KEY = 'ninaro:v1:room';
export const DEFAULT_ROOM = 'ninaro';

function store(): Storage | null {
  try {
    return typeof localStorage === 'undefined' ? null : localStorage;
  } catch {
    return null;
  }
}

/** The room in the page's ?online=, cleaned; '' if the parameter is there but empty; null if absent. */
export function roomFromUrl(search: string): string | null {
  const v = new URLSearchParams(search).get('online');
  if (v === null) return null;
  return v.trim() === '' ? '' : cleanRoom(v);
}

/** The room to offer: the URL's, then the last one used, then the default. */
export function initialRoom(search: string): string {
  const fromUrl = roomFromUrl(search);
  if (fromUrl) return fromUrl;
  try {
    const saved = store()?.getItem(ROOM_KEY);
    if (saved) return cleanRoom(saved);
  } catch {
    // Storage blocked: fall through.
  }
  return DEFAULT_ROOM;
}

export function saveRoom(room: string): void {
  try {
    store()?.setItem(ROOM_KEY, room);
  } catch {
    // Storage blocked or full: the room just isn't remembered.
  }
}

/** This page's address with ?online=<room>, to send to the other player. */
export function roomLink(room: string, href: string): string {
  const url = new URL(href);
  url.search = '';
  url.hash = '';
  url.searchParams.set('online', cleanRoom(room));
  return url.toString();
}

/** Shows the room box. The cleaned room on Connect, or null on Cancel or Esc. */
export function askRoom(initial: string): Promise<string | null> {
  const box = document.querySelector<HTMLElement>('#online-dialog');
  const form = document.querySelector<HTMLFormElement>('#online-form');
  const input = document.querySelector<HTMLInputElement>('#online-room');
  const copy = document.querySelector<HTMLButtonElement>('#online-copy');
  const cancel = document.querySelector<HTMLButtonElement>('#online-cancel');
  const hint = document.querySelector<HTMLElement>('#online-hint');
  if (!box || !form || !input || !copy || !cancel || !hint) return Promise.resolve(null);
  const hintText = hint.textContent ?? '';
  const returnFocus = document.activeElement instanceof HTMLElement ? document.activeElement : null;

  return new Promise((resolve) => {
    const finish = (room: string | null) => {
      form.removeEventListener('submit', submitted);
      form.removeEventListener('keydown', keyed);
      copy.removeEventListener('click', copied);
      cancel.removeEventListener('click', cancelled);
      box.hidden = true;
      hint.textContent = hintText;
      returnFocus?.focus();
      if (room !== null) saveRoom(room);
      resolve(room);
    };
    const submitted = (e: SubmitEvent) => {
      e.preventDefault();
      finish(cleanRoom(input.value));
    };
    const keyed = (e: KeyboardEvent) => {
      if (e.key !== 'Escape') return;
      e.preventDefault();
      finish(null);
    };
    const cancelled = () => finish(null);
    const copied = async () => {
      const link = roomLink(input.value, location.href);
      try {
        await navigator.clipboard.writeText(link);
        hint.textContent = 'Link copied. Send it to the other player.';
      } catch {
        hint.textContent = link; // no clipboard: show it to copy by hand
      }
    };
    form.addEventListener('submit', submitted);
    form.addEventListener('keydown', keyed);
    copy.addEventListener('click', copied);
    cancel.addEventListener('click', cancelled);
    input.value = initial;
    box.hidden = false;
    input.focus();
    input.select();
  });
}
