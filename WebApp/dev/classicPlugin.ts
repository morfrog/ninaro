// Classic mode, for development only: runs the original 1998 NINARO.EXE in
// the browser through js-dos (DOSBox compiled to WebAssembly).
//
// The original program is not part of this repository and is never
// published: its files contain personal details, and DOS4GW.EXE and
// MODERN.FON are third-party. If the files are present at ORIGINAL_DIR (by
// default Temp/Source/connect4/pack in the repository, which git ignores),
// the dev server packs them into a js-dos bundle on request. `vite build`
// never runs this plugin, and classic.html is not a build input, so nothing
// of the original reaches dist/. The deploy workflow also checks that.
//
// NINARO.EXE loads "*.fon" from its folder for its text (_setfont
// "t'Modern'"). The release didn't include one, so this adds Windows' own
// MODERN.FON from this computer when it can find it.

import { existsSync, mkdirSync, readdirSync, readFileSync, statSync, writeFileSync } from 'node:fs';
import { join, relative, resolve } from 'node:path';
import { strToU8, zipSync, type Zippable } from 'fflate';
import type { Plugin } from 'vite';

/** Where the original files are expected, relative to the repository root. */
export const DEFAULT_ORIGINAL_DIR = 'Temp/Source/connect4/pack';

/** Files the original needs to start. */
export const REQUIRED_FILES = ['NINARO.EXE', 'DOS4GW.EXE'];

/** Where Windows keeps the Modern vector font. */
export const DEFAULT_FONT = join(process.env.WINDIR ?? 'C:\\Windows', 'Fonts', 'MODERN.FON');

/** DOSBox settings: start the program straight away, as typing NINARO at the C:\ prompt. */
const DOSBOX_CONF = `[sdl]
autolock=true

[cpu]
core=auto
cycles=auto

[autoexec]
mount c .
c:
NINARO.EXE
`;

export interface OriginalStatus {
  available: boolean;
  /** Where the files are looked for, relative to the repository root. */
  location: string;
  missing: string[];
  /** True if a .FON font will be in the bundle. */
  font: boolean;
}

function listFiles(dir: string): string[] {
  return readdirSync(dir).flatMap((name) => {
    const path = join(dir, name);
    return statSync(path).isDirectory() ? listFiles(path) : [path];
  });
}

function hasFont(dir: string): boolean {
  return existsSync(dir) && readdirSync(dir).some((n) => n.toUpperCase().endsWith('.FON'));
}

export function originalStatus(dir: string, repoRoot: string, fontPath: string): OriginalStatus {
  const location = relative(repoRoot, dir).replace(/\\/g, '/') || '.';
  const present = existsSync(dir) ? new Set(readdirSync(dir).map((n) => n.toUpperCase())) : new Set<string>();
  const missing = REQUIRED_FILES.filter((f) => !present.has(f));
  return { available: missing.length === 0, location, missing, font: hasFont(dir) || existsSync(fontPath) };
}

/**
 * A NINARO.CFG as SaveCfg wrote it (big-endian ints): ProgInit's defaults,
 * Human v Human, with the mouse marked as configured so the original skips
 * its mouse calibration screen. For starting the original in a known state.
 */
export function freshConfig(): Uint8Array {
  const ints = [
    7, 8, 4, // columns, rows, connect
    4, 0, // goes first (Successive), went first (player one)
    2, 500, // AI speed (Normal), AI delay
    0, 1, // player one, player two
    0, // mute
    0, 4, // background BLACK, hilite GREEN
    2, 0, // mouse use (Full), best of (infinity)
    1, 0, 0, // mouse configured, x and y adjust
    2, 1, 6, 4, 3, 2, 5, 4, // the 8 players' colours, as in ProgInit
  ];
  const out = new Uint8Array(ints.length * 4);
  ints.forEach((v, i) => new DataView(out.buffer).setInt32(i * 4, v));
  return out;
}

/**
 * Every file in `dir` (paths relative to it, upper case as DOS saw them),
 * the font if `dir` has none, and the DOSBox settings. With `fresh`, the
 * folder's NINARO.CFG is replaced by freshConfig().
 */
export function buildBundle(dir: string, fontPath: string, fresh = false): Uint8Array {
  const files: Zippable = { '.jsdos/dosbox.conf': strToU8(DOSBOX_CONF) };
  for (const path of listFiles(dir)) {
    const name = relative(dir, path).replace(/\\/g, '/').toUpperCase();
    files[name] = readFileSync(path);
  }
  if (!hasFont(dir) && existsSync(fontPath)) files['MODERN.FON'] = readFileSync(fontPath);
  if (fresh) files['NINARO.CFG'] = freshConfig();
  return zipSync(files);
}

export function classicPlugin(options: { repoRoot: string; base: string }): Plugin {
  const dir = resolve(options.repoRoot, process.env.NINARO_ORIGINAL_DIR ?? DEFAULT_ORIGINAL_DIR);
  const fontPath = process.env.NINARO_FONT ?? DEFAULT_FONT;
  const prefix = `${options.base.replace(/\/$/, '')}/classic`;
  return {
    name: 'ninaro-classic',
    apply: 'serve', // dev server only; never part of a build
    configureServer(server) {
      server.middlewares.use(`${prefix}/status.json`, (_req, res) => {
        res.setHeader('Content-Type', 'application/json');
        res.setHeader('Cache-Control', 'no-store');
        res.end(JSON.stringify(originalStatus(dir, options.repoRoot, fontPath)));
      });
      // Frames of the original saved for comparing the port's screens
      // (compare.html): POST a PNG to save it to Temp/Reference/<name>.png,
      // GET to read it back.
      server.middlewares.use(`${prefix}/reference/`, (req, res) => {
        const name = (req.url ?? '').replace(/^\//, '').replace(/\?.*$/, '').replace(/\.png$/i, '');
        const file = join(resolve(options.repoRoot, 'Temp/Reference'), `${name}.png`);
        if (!/^[a-z0-9-]{1,40}$/.test(name) || (req.method !== 'POST' && req.method !== 'GET')) {
          res.statusCode = 400;
          res.end('GET or POST classic/reference/<name>.png');
          return;
        }
        if (req.method === 'GET') {
          if (!existsSync(file)) {
            res.statusCode = 404;
            res.end(`No reference ${name}.png`);
            return;
          }
          res.setHeader('Content-Type', 'image/png');
          res.setHeader('Cache-Control', 'no-store');
          res.end(readFileSync(file));
          return;
        }
        const chunks: Buffer[] = [];
        req.on('data', (c: Buffer) => chunks.push(c));
        req.on('end', () => {
          mkdirSync(resolve(options.repoRoot, 'Temp/Reference'), { recursive: true });
          writeFileSync(file, Buffer.concat(chunks));
          res.end(`Temp/Reference/${name}.png`);
        });
      });
      server.middlewares.use(`${prefix}/ninaro.jsdos`, (req, res) => {
        const fresh = /[?&]cfg=fresh\b/.test(req.url ?? '');
        const status = originalStatus(dir, options.repoRoot, fontPath);
        if (!status.available) {
          res.statusCode = 404;
          res.end(`Original Ninaro files not found in ${status.location}`);
          return;
        }
        res.setHeader('Content-Type', 'application/zip');
        res.setHeader('Cache-Control', 'no-store');
        res.end(buildBundle(dir, fontPath, fresh));
      });
    },
  };
}
