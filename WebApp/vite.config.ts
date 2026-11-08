import { resolve } from 'node:path';
import { defineConfig } from 'vite';
import { classicPlugin } from './dev/classicPlugin';

const base = '/ninaro/';

// GitHub Pages serves the site from https://morfrog.github.io/ninaro/
export default defineConfig({
  base,
  server: {
    // The help text and title picture are bundled from the repository's Legacy/ folder.
    fs: { allow: ['..'] },
  },
  // Classic mode (the original NINARO.EXE in js-dos) exists only in `npm run dev`.
  plugins: [classicPlugin({ repoRoot: resolve(__dirname, '..'), base })],
  build: {
    rollupOptions: {
      // classic.html and compare.html are deliberately not listed: never built.
      input: {
        main: resolve(__dirname, 'index.html'),
        tournament: resolve(__dirname, 'tournament.html'),
      },
    },
  },
});
