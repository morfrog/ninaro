import { mkdirSync, mkdtempSync, rmSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { strFromU8, unzipSync } from 'fflate';
import { afterEach, describe, expect, it } from 'vitest';
import { buildBundle, classicPlugin, originalStatus } from './classicPlugin';

let dirs: string[] = [];
function tempRepo(): string {
  const dir = mkdtempSync(join(tmpdir(), 'ninaro-classic-'));
  dirs.push(dir);
  return dir;
}
afterEach(() => {
  for (const d of dirs) rmSync(d, { recursive: true, force: true });
  dirs = [];
});

describe('originalStatus', () => {
  it('reports what is missing, relative to the repository', () => {
    const repo = tempRepo();
    expect(originalStatus(join(repo, 'Temp/Source/connect4/pack'), repo, join(repo, 'none.fon'))).toEqual({
      available: false,
      location: 'Temp/Source/connect4/pack',
      missing: ['NINARO.EXE', 'DOS4GW.EXE'],
      font: false,
    });
  });

  it('is available once both programs are present, in any case', () => {
    const repo = tempRepo();
    const pack = join(repo, 'pack');
    mkdirSync(pack, { recursive: true });
    writeFileSync(join(pack, 'ninaro.exe'), 'MZ');
    writeFileSync(join(pack, 'DOS4GW.EXE'), 'MZ');
    writeFileSync(join(repo, 'MODERN.FON'), 'MZ');
    expect(originalStatus(pack, repo, join(repo, 'MODERN.FON'))).toMatchObject({ available: true, missing: [], font: true });
  });
});

describe('buildBundle', () => {
  it('packs every file upper-cased, adds the font, and starts NINARO', () => {
    const repo = tempRepo();
    const pack = join(repo, 'pack');
    mkdirSync(pack, { recursive: true });
    writeFileSync(join(pack, 'ninaro.exe'), 'MZ1');
    writeFileSync(join(pack, 'ninaro.hlp'), 'help');
    writeFileSync(join(repo, 'modern.fon'), 'font');
    const files = unzipSync(buildBundle(pack, join(repo, 'modern.fon')));
    expect(Object.keys(files).sort()).toEqual(['.jsdos/dosbox.conf', 'MODERN.FON', 'NINARO.EXE', 'NINARO.HLP']);
    expect(strFromU8(files['.jsdos/dosbox.conf'])).toMatch(/\[autoexec\][\s\S]*mount c \.[\s\S]*NINARO\.EXE/);
  });

  it("doesn't add a font when the folder has one", () => {
    const repo = tempRepo();
    const pack = join(repo, 'pack');
    mkdirSync(pack, { recursive: true });
    writeFileSync(join(pack, 'NINARO.EXE'), 'MZ');
    writeFileSync(join(pack, 'own.fon'), 'own');
    writeFileSync(join(repo, 'modern.fon'), 'font');
    expect(Object.keys(unzipSync(buildBundle(pack, join(repo, 'modern.fon')))).sort()).toEqual([
      '.jsdos/dosbox.conf',
      'NINARO.EXE',
      'OWN.FON',
    ]);
  });
});

describe('classicPlugin', () => {
  it('only applies to the dev server, never to builds', () => {
    expect(classicPlugin({ repoRoot: tempRepo(), base: '/ninaro/' }).apply).toBe('serve');
  });
});
