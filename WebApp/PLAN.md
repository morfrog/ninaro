# Ninaro Web Port: Plan

This ports the 1998 DOS game *Ninaro* ("N-in-a-row": Connect 4 on any board size, against rule-based AIs) to a static web app hosted on GitHub Pages. The original is v2.07, written in Watcom C/386 with DOS4GW, in `Legacy/`.

The plan reuses what worked for the VTTSP port (`tsp-visualiser/WebApp/PLAN.md`, and `PORTING-PLAYBOOK.md` in that repo). Where Ninaro differs from VTTSP, this plan says so.

**Stack:** Vite + TypeScript, HTML Canvas 2D, no UI framework, Vitest.
**Live URL:** `https://morfrog.github.io/ninaro/` (`base: '/ninaro/'`).

## Legacy summary

About 9,600 lines in 45 files. The help text (`NINARO.HLP`) is plain text in which `~` marks a heading or a highlighted line. The title picture (`NINARO.TTL`) is a saved 36 × 9 grid, written as big-endian 32-bit ints through `BINFILE`.

| Legacy file | Role | Web port |
|---|---|---|
| `GRID.C/H` | Board of player pointers, counts per column and row, moves, run counting (`FuncNumInDirection`, `NumsInDirection`), save and load | `src/core/grid.ts` |
| `CONNECTN.C/H` | Drop target, `CheckForWin` (4 angles), `WinPotential`, `Continuance`, game save and load | `src/core/connect.ts` |
| `NINARO.C` (`RuleMove`, `RandomMoveFromList`, `AI*Move`) | The 5 AIs: Random, Novice, Normal, Expert, Genius = rules 0–4 | `src/core/ai.ts` |
| `NINARO.C` (everything else) | Globals, main loop, `HumanMove`, `DropToken`, `NewGame`/`EndGame`, goes-first rules, best-of sets, demo mode, title screen, config, menus and dialogs set up in `ProgInit` | `src/game/*` |
| `PLAYER.C/H` | Player (name, colour, type, move function) and player group (8 slots: 2 humans, 5 AIs, 1 copy slot) | `src/core/player.ts` |
| `SYS.C` | `Random(n) = rand() % n`, `StartRandom` (clock seed), list helpers | `src/core/random.ts` |
| `BINFILE.C/H` | Byte buffer file I/O, big-endian ints, optional bit "encryption" | `src/core/binfile.ts` (reading `.TTL`, and optionally the original `.CFG`/`.SAV`) |
| `SCREEN.C/H`, `COLOR.C` | EGA mode 16 (640×350, 2 pages), custom 16-colour palette, viewports, `DrawGrid`, `DrawDisc`, `SwapPage` | `src/render/screen.ts`, `palette.ts` |
| `VIEWSYS.C/H` | Screen split into windows by fractions, with "changed" flags, redraw callbacks, and hint text | `src/render/viewsys.ts` |
| `TXTBOX.C/H` | Scrolling text boxes (messages and status), with `~` highlighting | `src/ui/textbox.ts` |
| `MENU.C/H` | Menu bar: Game, Board, Player, Help, driven by keys or mouse | `src/ui/menu.ts` |
| `DIALOG.C/H`, `CONTROLS.C/H` | Modal dialogs: List/Range controls drawn as Radio, Slider, Color, or Name; OK/Reset/Cancel | `src/ui/dialog.ts`, `controls.ts` |
| `HELP.C/H` | Scrolling help viewer, jump to heading (`HelpScrollString`) | `src/ui/help.ts` |
| `KEY.C/H` | `GetKey` (extended codes 256+n), key groups per player, key-to-function table | `src/input/keys.ts` |
| `MOUSE.C/H`, `MOUSEIN.C/H` | INT 33h driver, a hand-made mouse "calibration" | Replaced by pointer events |
| `SOUND.C/H` | PC speaker `Buzz`, `PlayTune` (they block for their own length) | `src/audio/sound.ts` (WebAudio square wave) |
| `MEMPAGE.C/H` | Saves and restores screen regions under menus and dialogs | Not needed: redraw instead |
| `ENTITY.C`, `REJECTS.C`, `GENNAME.C`, `IMAGE.C`, `MEMMAIN.C`, `MOUSETST.C`, `WIDTH.C` | Notes, discarded code, test programs | Not ported |

**Screen layout** (`ViewSysInit`, fractions of 640×350):

| Area | Fraction (x1, y1, x2, y2) |
|---|---|
| Entry row (the token you are about to drop) | 0–0.6 × 0–0.08 |
| Board | 0–0.6 × 0.09–1.0 |
| Game name | 0.61–1.0 × 0–0.1 |
| Status box (columns, rows, connect, sound) | 0.61–1.0 × 0.1–0.37 |
| Each player's row: type icon, name, score/games | 0.61–1.0 × 0.40–0.448 (player one), 0.452–0.5 (player two) |
| Message box | 0.61–1.0 × 0.52–1.0 |

## How Ninaro differs from VTTSP

1. **Control flow is interactive, not computational.** VTTSP's algorithms ran for a long time and paused in `WaitTill`, so they became generators in a Web Worker. Ninaro's AI picks a move instantly (one pass over the columns). Its waiting is all UI: `HumanMove`'s `kbhit` loop, `AnimateMove`'s `delay`, "Press ESC To Continue", and the modal `MenuCall`/`DialogCall`/`HelpCall` loops.
   **Approach:** keep the C's shape with `async`/`await`. An input queue replaces `getch`/`kbhit`/`GetMouseInput` (`await input.key()`, `input.hasKey()`), and `await sleep(ms)` replaces `delay`. Modal menus and dialogs become `async` functions that return when closed, as their C versions did. No worker is needed.
2. **All text is graphics text.** Ninaro draws every string with `_grtext` in the Windows vector font *Modern*, stretched to fit each box (`_setcharsize`, `_setcharspacing`). VTTSP used fixed character grids. MODERN.FON is Microsoft's and can't be shipped.
   **Approach:** use a public-domain Hershey stroke font, which looks very like Modern, and draw text as strokes on the canvas, so stretching works as it did. Mirror the message box and menus into an ARIA live region and DOM labels, for screen readers.
3. **One canvas, not DOM panels.** The menus, dialogs and widgets are all drawn by the program itself, so the port draws a 640×350 logical canvas, scaled to the 4:3 pixel aspect (1.3714). Pointer positions map back to 640×350 coordinates for hit tests. On phones the same canvas is scaled to fit, with on-screen buttons underneath.
4. **The palette changes at run time.** `_remappalette` swaps black and white for the white background option, and cycles YELLOW on the title screen. The renderer draws with palette indexes (0–15), resolved through a live palette when drawing. Changing the palette redraws the screen. `ResetPalette`'s exact values (for example `_YELLOW-0x010101`, `_BLUE+0x150000`) are ported, and DOS 6-bit palette values become 8-bit.
5. **No benchmark tables to match.** VTTSP had 150 documented results. Ninaro has no recorded results, and its AI uses `rand() % n` seeded from the clock, so no 1998 game can be replayed. See "Oracle" below.
6. **Sound.** `Buzz` and `PlayTune` drive the PC speaker and block until done. Port them to WebAudio square waves and await their length, so the game's timing stays the same. Audio starts on the first key or click (a browser rule). The mute setting is honoured.

## Oracle

The order of preference:

1. **Unit tests from reading the C** (always). Test win detection on all 4 angles at board edges, `InsertTokenTarget`, `Continuance`, `MoveAll`, `WinPotential` arithmetic (integer division, as in C), `RandomMoveFromList` tie selection, and each rule level on small hand-made boards.
2. **A golden-game harness** (recommended, local only). Compile `GRID.C`, `CONNECTN.C`, `PLAYER.C` and `RuleMove`/`RandomMoveFromList` (copied out of `NINARO.C`) with a modern C compiler, using a stub screen and a seeded copy of Watcom's `rand()`. Play thousands of seeded AI-vs-AI games on several board sizes, and write each move list to JSON under `WebApp/src/core/__golden__/`. The TS port must replay every one exactly.
   No C compiler is installed now, so this needs one installed locally (clang, gcc or zig). The harness lives in `Tools/` and is not part of the web build. This plays the role DOCO played for VTTSP.
3. **Classic mode** (Phase 6). Play the original side by side to check layout, timing, colours and sounds.

Watcom's `rand()` needs checking. It is believed to be `seed = seed*1103515245 + 12345; return (seed >> 16) & 0x7FFF`. Confirm it against the original in classic mode, or a Watcom runtime listing, before the golden files are made.

## Fidelity notes found while reading

Keep the behaviour unless it's a crash, and list every change under "Changes from the original".

- `NewGame`, Loser goes first: `else SetP2Current;` is missing its `()`. With no previous winner and `Random(2)==1`, the current player stays as it was. Recommendation: **keep it** (it's harmless), and mention it.
- `RuleMove` ignores its `grid` argument and uses the global `playgrid`. `CheckForWin` writes into the global `win` array as a side effect. Port both as they are (the AI only runs on `playgrid`).
- `keypos` starts at `cols/2 + cols%2` in `ResetGameSize` (column 4 of 0–6 on 7 columns), but at `cols/2` in `LoadGame`. Keep both.
- `UnBlockableWin` is empty and returns garbage, but is never called. `PlayerSwap` is never called. Leave them out.
- The title screen fills the TTL picture using `AIRandomMove` (rule 0), so the order the tokens fall in is random.
- `AnimateMove` steps one column every `ai_speed*ai_delay/dist` ms. `ai_delay` is 500 ms, and `ai_speed` runs 0 (Fastest) to 4 (Slowest).
- With two AIs at Fastest, `waitesc` is false, so games repeat with no prompt (demo mode). Browsers need a yield between games (`await sleep(0)`) to stay responsive.
- F9 resets the mouse calibration. That has no meaning on the web, so drop it, along with the "Mouse Use: None/Partial/Full" sensitivity warning. Keep the option itself (Partial = mouse only during a move).

## Input mapping

| Original | Web |
|---|---|
| Player one: A D W S, E (select), Q (cancel) | The same |
| Player two: keypad 4 6 8 2, Enter, Backspace | The same, plus the arrow keys (`event.code` Numpad\* and Arrow\*) |
| Esc or M: menu. F1 or H: help | The same |
| F2 save, F3 load, F4 new/end, F5 suggest, F6 demo, F7 human vs human, F8 sound, F10 / Ctrl-X / Ctrl-Q / Alt-F4 exit, Ctrl-F9 delete config | Call `preventDefault` where the browser allows it. F5, Ctrl-Q, Ctrl-W and Alt-F4 can't be relied on, so every action is also in the menu. "Exit" returns to the title screen. |
| Mouse: move over a column, left button drops, right button opens the menu (player one only) | Pointer events. `contextmenu` opens the menu. |
| *(new)* Touch | Tap a column to move the token there, tap the same column again to drop. Add a Menu button and a Help button below the canvas. |

Use `event.code` for player keys, so they work on any keyboard layout.

## Phases

Each phase ends as before: tests green, `npm run build` clean, commit on a `webapp-phaseN` branch, fast-forward merge into `main`, push, and watch the deploy.

0. **Scaffold and deploy.** ✅ Done. The placeholder page draws the empty 7×8 board at the original's position (`src/layout.ts`, a port of `ViewportFromViewport`, with tests). Copy the VTTSP setup: `vite.config.ts` (`base: '/ninaro/'`, `server.fs.allow: ['..']`), `tsconfig.json`, Vitest, and the `deploy-pages.yml` workflow (build → test → guard → deploy) with its guard step from the start. Add `.claude/launch.json` with dev on **5174** and preview on **4174**, so it can run beside VTTSP's servers.
   *Manual step:* repo Settings → Pages → Source = **GitHub Actions**.
1. **Core logic.** `grid.ts`, `connect.ts`, `player.ts`, `random.ts` (Watcom `rand`, seeded and injectable), `binfile.ts` (big-endian reader for `NINARO.TTL`), `ai.ts` (`RuleMove`, `RandomMoveFromList`, the 5 levels). Unit tests as in Oracle 1. Comments name the C function each piece comes from.
   *Optional:* the golden-game harness and its replay tests (Oracle 2).
2. **Renderer.** `palette.ts` (EGA indexes, `ResetPalette`, remapping), `screen.ts` (640×350 logical canvas with the device pixel ratio and 4:3 aspect, viewports, `_rectangle`/`_ellipse`/`_lineto` in pixel and window (`_w`) coordinates, fill masks for Suggest Move), `strokeFont.ts` (Hershey font with `_setcharsize`/`_setcharspacing`/`_settextalign`), `viewsys.ts` (windows, changed flags, hint text). Draw the board, discs, win highlight and player icons (`DisplayHuman`, `DisplayComputer`). *Check:* a static screen matching the layout table above.
3. **Game loop.** The input queue and `sleep`, `HumanMove`, `AnimateMove`, `DropToken`, `NewGame`/`EndGame`, goes-first rules, scores, best-of sets, draws, "Press ESC To Continue", demo mode, `SuggestMove`, sound. *Check:* full games human vs AI and AI vs AI from the keyboard and the mouse, with the menu stubbed.
4. **Menus, dialogs, help, title.** `textbox.ts`, `menu.ts` (4 menus as in `ProgInit`, enabling and disabling items as the C does), `dialog.ts`/`controls.ts` (Player, Custom Board with `GameSizeFit`, Player Options, Game Options), `help.ts` (scrolling, jump to heading, context help from menus and dialogs), the About box, and the title screen (TTL picture falling in, YELLOW cycling, "Hit Any Key To Continue...", the slide-off animation). Bundle `NINARO.HLP` and `NINARO.TTL` with `import.meta.glob(..., { query: '?raw' })`.
5. **Storage.** `NINARO.CFG` becomes `localStorage['ninaro:v1:cfg']`, written on change and when leaving the page (`pagehide`). `NINARO.SAV` becomes `localStorage['ninaro:v1:save']`. Every access is guarded. Ctrl-F9 (delete config) is kept. *Optional:* import or export the original binary `.CFG`/`.SAV` files with the big-endian `BINFILE` layout.
6. **Extras.**
   - **Classic mode, development only.** Copy VTTSP's `dev/classicPlugin.ts` and `classic.html`: js-dos 8.5.3 from jsDelivr, files from git-ignored `Temp/Source/connect4/pack` (`ninaro.exe`, `DOS4GW.EXE`, `ninaro.hlp`, `ninaro.ttl`), or from `NINARO_ORIGINAL_DIR`. Never built or deployed: the pack's `read.me` contains the author's name, and DOS4GW is third-party. The pack has no `MODERN.FON`, so `_setfont` may fail in DOSBox and the text may be missing or in a default font. Check this first.
   - **AI tournament page.** Seeded round-robin of the 5 AIs on 7×8×4 and 12×13×6 (and custom sizes), run in a worker, with a win/draw table and a seed you can change. It works as a check (Genius should beat Random almost always) and as a demo.

7. **Known bugs review (after Phase 6 is committed).** List every known bug: the original's bugs kept by decision 4 below, plus any found during the port. Give each one its file and function, what happens, and a suggested fix. The user picks some or all of them, and those are fixed in a separate commit on top of the faithful port. Each fix is listed under "Changes from the original", and the golden-game tests are updated where a fix changes moves.

## Decisions (agreed 2026-11-08)

1. **Golden-game harness: yes.** Install a local C compiler (clang, gcc or zig) in Phase 1. The harness goes in `Tools/`, and its JSON output is committed as test data.
2. **Help text: use the pack's newer `ninaro.hlp`** (June 1998, 15.7 KB, 48 headings) instead of `Legacy/NINARO.HLP` (11.9 KB, 38 headings). Its "About Ninaro" section names the author, the university, the course, email addresses, and the supervisor. In Phase 1, copy it into `Legacy/NINARO.HLP` with that section replaced by the redacted wording already used in the user manual ("written by the author in 1998, as a university project"). Grep the result before committing.
3. **Font: Hershey stroke font**, drawn as strokes so `_setcharsize` stretching works as in the original.
4. **Original bugs: keep them during the port** (Loser-goes-first included). Phase 7 lists them all once the port is committed, and the chosen fixes go in their own commit.

## Personal details

Keep the author's name, institution and student number out of the app and out of commits (see the VTTSP memory note). The repo already uses "By the author" in source and the About box. The pack's `read.me` and executable aren't redacted, so they stay in git-ignored `Temp/`. Before each commit, grep for the name, the institution, and `n[0-9]{7}`.

## Development

```bash
cd WebApp
npm install
npm run dev       # local dev server (port 5174)
npm test          # Vitest
npm run build     # type-check + production build to dist/
npm run preview   # serve dist/ (port 4174)
```
