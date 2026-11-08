// Classic mode page (development only): the original 1998 NINARO.EXE running
// in js-dos. The dev server's classic plugin (dev/classicPlugin.ts) serves
// the original files if they are present locally; nothing of the original is
// ever built or deployed.

const JS_DOS_VERSION = '8.5.3';
const JS_DOS = `https://cdn.jsdelivr.net/npm/js-dos@${JS_DOS_VERSION}/dist`;

interface OriginalStatus {
  available: boolean;
  location: string;
  missing: string[];
  font: boolean;
}

declare global {
  interface Window {
    Dos?: (element: HTMLElement, options: Record<string, unknown>) => unknown;
  }
}

const root = document.querySelector<HTMLElement>('#classic')!;

function el<K extends keyof HTMLElementTagNameMap>(tag: K, className?: string, text?: string): HTMLElementTagNameMap[K] {
  const e = document.createElement(tag);
  if (className) e.className = className;
  if (text !== undefined) e.textContent = text;
  return e;
}

function load(tag: 'script' | 'link', url: string): Promise<void> {
  return new Promise((resolve, reject) => {
    const e = document.createElement(tag);
    if (e instanceof HTMLScriptElement) e.src = url;
    else {
      e.rel = 'stylesheet';
      e.href = url;
    }
    e.onload = () => resolve();
    e.onerror = () => reject(new Error(`Could not load ${url}`));
    document.head.append(e);
  });
}

async function start(): Promise<void> {
  root.append(el('h1', undefined, 'Ninaro Classic'));
  const note = el('p', 'classic__note');
  root.append(note);

  if (!import.meta.env.DEV) {
    note.textContent = 'Classic mode is only available when developing (npm run dev).';
    return;
  }

  const base = import.meta.env.BASE_URL;
  let status: OriginalStatus;
  try {
    status = (await (await fetch(`${base}classic/status.json`)).json()) as OriginalStatus;
  } catch {
    note.textContent = 'The dev server did not answer. Is this running under npm run dev?';
    return;
  }
  if (!status.available) {
    note.textContent = `The original program isn't here. Put ${status.missing.join(' and ')} (and NINARO.HLP and NINARO.TTL) in ${status.location}, or set NINARO_ORIGINAL_DIR, then reload.`;
    return;
  }

  note.textContent = `The original 1998 program, running in DOSBox (js-dos ${JS_DOS_VERSION}), from ${status.location}${status.font ? ', with Windows MODERN.FON' : ' (no MODERN.FON found: set NINARO_FONT)'}. Click the screen to capture the mouse. Development only: never deployed.`;
  const screen = el('div', 'classic__screen');
  root.append(screen);
  try {
    await Promise.all([load('link', `${JS_DOS}/js-dos.css`), load('script', `${JS_DOS}/js-dos.js`)]);
    window.Dos!(screen, {
      url: `${base}classic/ninaro.jsdos`,
      pathPrefix: `${JS_DOS}/emulators/`,
      autoStart: true,
      kiosk: true,
      // Keep the emulator's command interface for comparing screens while
      // developing: window.classicCi.screenshot() gives the 640x350 frame,
      // and window.saveReference('name') saves it to Temp/Reference/name.png.
      onEvent: (event: string, ci?: unknown) => {
        if (event === 'ci-ready') (window as unknown as { classicCi?: unknown }).classicCi = ci;
      },
    });
  } catch (error) {
    note.textContent = error instanceof Error ? error.message : String(error);
  }
}

interface Screenshotter {
  screenshot(): Promise<ImageData>;
}

/** Saves the emulator's current 640x350 frame to Temp/Reference/<name>.png. */
async function saveReference(name: string): Promise<string> {
  const ci = (window as unknown as { classicCi?: Screenshotter }).classicCi;
  if (!ci) throw new Error('The emulator is not running yet.');
  const frame = await ci.screenshot();
  const canvas = document.createElement('canvas');
  canvas.width = frame.width;
  canvas.height = frame.height;
  canvas.getContext('2d')!.putImageData(new ImageData(new Uint8ClampedArray(frame.data), frame.width, frame.height), 0, 0);
  const png = await new Promise<Blob>((done) => canvas.toBlob((b) => done(b!), 'image/png'));
  const res = await fetch(`${import.meta.env.BASE_URL}classic/reference/${name}.png`, { method: 'POST', body: png });
  return res.text();
}
(window as unknown as { saveReference: typeof saveReference }).saveReference = saveReference;

void start();

export {};
