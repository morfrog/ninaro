import { resolve } from 'node:path';
import { defineConfig } from 'vite';

// GitHub Pages serves the site from https://morfrog.github.io/ninaro/
export default defineConfig({
  base: '/ninaro/',
  server: {
    // The help text and title picture are bundled from the repository's Legacy/ folder.
    fs: { allow: ['..'] },
  },
  build: {
    rollupOptions: {
      input: {
        main: resolve(__dirname, 'index.html'),
      },
    },
  },
});
