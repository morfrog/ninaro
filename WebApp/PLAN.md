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

1. **Unit tests from reading the C** (always). Test win detection on all 4 angles, `InsertTokenTarget`, `WinPotential` arithmetic (integer division, as in C), `RandomMoveFromList` tie selection, and each rule level's column scores on small hand-made boards.
2. **The golden-game harness** (`Tools/golden/`, built and run locally only). `build.bat` uses Visual Studio's MSVC (found with `vswhere`; `/J` makes `char` unsigned, as in Watcom). It compiles the unchanged `Legacy/GRID.C`, `CONNECTN.C`, `PLAYER.C` and `BINFILE.C`. `extract.py` copies `RuleMove` and `RandomMoveFromList` word for word out of `NINARO.C`. `stubs.c` supplies the seeded Watcom `rand()`, the two byte helpers from `SYS.C`, and empty drawing calls.
   It plays 1,600 seeded AI-vs-AI games: 8 boards × 5 × 5 AI levels × 8 seeds. It also loads `NINARO.TTL` with `GridLoadFull`, and writes everything to `WebApp/src/core/__golden__/games.json` (committed, about 300 KB, the same bytes on every run). This plays the role DOCO played for VTTSP.
3. **Classic mode** (brought forward to Phase 2). Run the original in js-dos and compare frames:
   - `window.saveReference(name)` on `classic.html` saves the emulator's exact 640×350 frame to `Temp/Reference/<name>.png` (git-ignored).
   - `compare.html?name=<name>` draws the port's version of the same state beside it. `window.comparePixels()` reports differing pixels and ink boxes for each window.
   - Both pages are dev-only: not build inputs, and the deploy guard checks for them.

Watcom's `rand()` is taken to be the ANSI C example generator (`next = next*1103515245 + 12345; return (next >> 16) & 0x7FFF`, starting at 1). The harness and the port share this, so the golden games don't depend on it. Confirm it against the original in classic mode in Phase 6.

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

## Known bugs (collected for Phase 7)

Found while porting. Behaviour is kept unless noted.

| # | Where | What happens | Port |
|---|---|---|---|
| 1 | `NINARO.C NewGame` | Loser goes first: `else SetP2Current;` is missing `()`, so with no previous winner and `Random(2)==1` the first player doesn't change. | Kept (Phase 3) |
| 2 | `NINARO.C` `ResetGameSize` / `LoadGame` | The token starts at column `cols/2 + cols%2` (right of centre on odd boards), but at `cols/2` after loading a game. | Kept (Phase 3) |
| 3 | `GRID.C NumsInDirection` | `istop` of -1 is documented as "infinity", but stops after 1 cell (`number >= -1`). Never called with -1. | Kept |
| 4 | `CONNECTN.C InsertToken` | On a full column, `InsertTokenTarget` is -1 and `InsertTokenAtCR` reads `grid[col][-1]` (out of bounds). In practice this returned Failure. | Guarded: returns Failure |
| 5 | `CONNECTN.C RemoveToken` | On an empty column it reads one past the end of the column. Only called on columns with tokens. | Guarded: returns null |
| 6 | `BINFILE.C BinFileGetInt` | With 1–3 bytes left, the missing bytes come from an uninitialised buffer. | Missing bytes read as 0 |
| 7 | `PLAYER.C PlayerGroupPlayer` | Only checks the upper bound, so a bad saved-game player number below 0 reads outside the array. | Returns null |
| 8 | `GRID.C MoveAll` | Recounts `tokensingrid` without resetting it first, so the count doubles. Only `Continuance` calls it, and nothing calls that. | Not ported |
| 9 | `CONNECTN.C UnBlockableWin` | Empty, with no return value. Never called. | Not ported |
| 10 | `NINARO.C RuleMove` | Rule 3 checks the cell above the landing cell while the landing cell is still empty, so vertical lines through it are never counted. | Kept |
| 11 | `NINARO.C BestOfResult` | "The Set is Drawn" is printed with an unused extra argument. | Harmless |
| 12 | `TXTBOX.C TextBoxFixedAddLine` | Copies with `strncpy(…, width+1)`, which leaves no terminator for lines of `width` characters or more, so the box shows whatever follows in memory. | Lines cut to `width` |
| 13 | `TXTBOX.C TextBoxSetLine` | `strcpy` with no length check: a line longer than `width` overruns the node's buffer. | No limit needed |
| 14 | `VIEWSYS.C ViewSysDisplay` | `DisplayAll` clears each window's changed flag but not the count of changed windows, so the next `DisplayChanged` loops over the windows for nothing. | Count reset (no visible change) |
| 15 | `NINARO.C SuggestMove` | Sets a fill mask to hatch the suggested token, but in the running original the token stays solid (checked frame by frame in classic mode). | Solid, as seen (`suggestMove(true)` would hatch) |
| 16 | `NINARO.C main` | A key already waiting when a move is chosen is handled by `ExtraFunctionality` and the move is thrown away, so a drop followed instantly by another key is lost. Only automated input is that fast. | Kept |
| 17 | `NINARO.C ProgInit` | The Custom Board dialog's Connect slider is made with a top of `MaxOf(grid_rows, grid_cols)` before `LoadCfg` has set them, so its range is 3..0. The first time the dialog opens, Connect can't be set to the board's 4 and shows 3 (seen in classic mode); after `GameSizeFit` it is right. | Kept |

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
1. **Core logic.** ✅ Done. `src/core/`:
   - `grid.ts` (`GRID.C`), `connect.ts` (`CONNECTN.C`, including `GameSave`/`GameLoad` as bytes), `player.ts`, `random.ts` (Watcom `rand`, seeded; `StartRandom`'s clock seed), `binfile.ts` (big-endian ints, Watcom's unsigned `char`).
   - `ai.ts`: `ruleScores` (the column scores, which `RuleMove`'s `TESTING` build printed), `ruleMove`, and `randomMoveFromList`. The AI reads the game's `playgrid` and writes its `win`, as the C did.
   - `Continuance`, `MoveAll`, `UnBlockableWin` and the `BINFILE` "encryption" were never used by the game, so they aren't ported.
   - Tests: 63 in 7 files. The golden replay (`golden.test.ts`) matches all 1,600 games (moves, result and winning cells) and the title picture. Breaking a rule on purpose (for example, removing the take-a-win bonus) fails it.
   - `Legacy/NINARO.HLP` is now the release's newer help (decision 2), with its About section redacted.
2. **Renderer.** ✅ Done. See "Phase 2 notes" below. Planned as: `palette.ts` (EGA indexes, `ResetPalette`, remapping), `screen.ts` (640×350 logical canvas with the device pixel ratio and 4:3 aspect, viewports, `_rectangle`/`_ellipse`/`_lineto` in pixel and window (`_w`) coordinates, fill masks for Suggest Move), `strokeFont.ts` (Hershey font with `_setcharsize`/`_setcharspacing`/`_settextalign`), `viewsys.ts` (windows, changed flags, hint text). Draw the board, discs, win highlight and player icons (`DisplayHuman`, `DisplayComputer`). *Check:* a static screen matching the layout table above.
3. **Game loop.** ✅ Done. See "Phase 3 notes" below. Planned as: the input queue and `sleep`, `HumanMove`, `AnimateMove`, `DropToken`, `NewGame`/`EndGame`, goes-first rules, scores, best-of sets, draws, "Press ESC To Continue", demo mode, `SuggestMove`, sound. *Check:* full games human vs AI and AI vs AI from the keyboard and the mouse, with the menu stubbed.
4. **Menus, dialogs, help, title.** ✅ Done. See "Phase 4 notes" below. Planned as: `textbox.ts`, `menu.ts` (4 menus as in `ProgInit`, enabling and disabling items as the C does), `dialog.ts`/`controls.ts` (Player, Custom Board with `GameSizeFit`, Player Options, Game Options), `help.ts` (scrolling, jump to heading, context help from menus and dialogs), the About box, and the title screen (TTL picture falling in, YELLOW cycling, "Hit Any Key To Continue...", the slide-off animation). Bundle `NINARO.HLP` and `NINARO.TTL` with `import.meta.glob(..., { query: '?raw' })`.
5. **Storage.** ✅ Done. See "Phase 5 notes" below. Planned as: `NINARO.CFG` becomes `localStorage['ninaro:v1:cfg']`, written on change and when leaving the page (`pagehide`). `NINARO.SAV` becomes `localStorage['ninaro:v1:save']`. Every access is guarded. Ctrl-F9 (delete config) is kept. *Optional:* import or export the original binary `.CFG`/`.SAV` files with the big-endian `BINFILE` layout.
6. **Extras.**
   - **Classic mode, development only.** ✅ Done early, in Phase 2 (see its notes). Copy VTTSP's `dev/classicPlugin.ts` and `classic.html`: js-dos 8.5.3 from jsDelivr, files from git-ignored `Temp/Source/connect4/pack` (`ninaro.exe`, `DOS4GW.EXE`, `ninaro.hlp`, `ninaro.ttl`), or from `NINARO_ORIGINAL_DIR`. Never built or deployed: the pack's `read.me` contains the author's name, and DOS4GW is third-party. The pack has no `MODERN.FON`, so `_setfont` may fail in DOSBox and the text may be missing or in a default font. Check this first.
   - **AI tournament page.** Seeded round-robin of the 5 AIs on 7×8×4 and 12×13×6 (and custom sizes), run in a worker, with a win/draw table and a seed you can change. It works as a check (Genius should beat Random almost always) and as a demo.

7. **Known bugs review (after Phase 6 is committed).** List every known bug (the running list is under "Known bugs" above): the original's bugs kept by decision 4 below, plus any found during the port. Give each one its file and function, what happens, and a suggested fix. The user picks some or all of them, and those are fixed in a separate commit on top of the faithful port. Each fix is listed under "Changes from the original", and the golden-game tests are updated where a fix changes moves.

### Phase 2 notes

- **Classic mode now, not in Phase 6.** `dev/classicPlugin.ts` packs the original from `Temp/Source/connect4/pack` with this computer's `C:\Windows\Fonts\MODERN.FON` (or `NINARO_FONT`), so the original shows its real text. Saved frames are the reference for every later screen.
- **`src/render/graph.ts`** is the Watcom `graph.h` subset, measured against the original's frames:
  - Viewports clip, and set the origin (`_setvieworg` moves only the origin).
  - Window coordinates span the current viewport and are **truncated** (the grid lines then match to the pixel).
  - `_setcharsize_w`/`_setcharspacing_w` sizes scale with the **whole screen** (×639, ×349), not the viewport.
  - Text is **fixed pitch**: each character fills a cell of the set width, plus spacing. Capitals sit 0.1 of the cell down and are 0.7 of it tall.
  - Drawing is in EGA pixels on a canvas scaled to the device, so lines are one EGA pixel thick (taller than wide) while text and discs stay smooth.
- **`palette.ts`**: `ResetPalette` slots with Watcom's 6-bit `0x00BBGGRR` values, shown as DOSBox shows them (`v × 255 / 63`). The colours measured in the original's frames match (for example YELLOW `#fbfb51`).
- **`hersheyFont.ts`** is generated once by `dev/make-font.mjs` from hersheytext 2.0.0 (MIT; Hershey Simplex Roman). The package isn't kept as a dependency, because its own dependencies are old.
- **Other new modules:**
  - `render/screen.ts`: `SCREEN.C` helpers, plus `Screen`, which sizes the canvas to the device and redraws on resize.
  - `render/viewsys.ts`: `VIEWSYS.C`.
  - `render/memPage.ts`: `MEMPAGE.C`, as a canvas copy.
  - `ui/textbox.ts`: `TXTBOX.C`, as an array plus a start index.
  - `game/state.ts`: `ProgInit`'s globals.
  - `game/display.ts`: `ViewSysInit` and the `Display*` functions.
- **Check against `main-human`:** the play grid matches pixel for pixel. Text, icons and discs are within 2 px of the original. Glyph shapes differ, since this is Hershey, not Modern.
- **Change:** the credit's redacted "By the author" is split over two lines ("By the" / "author"), as the release split the name, so it fits its window.
- **Tests:** 87 in 10 files.

### Phase 3 notes

- **Modules:**
  - `input/keys.ts`: `KEY.C` codes, key groups and the key system, plus browser key → DOS `GetKey` code.
  - `input/input.ts`: `kbhit`/`GetKey`/`GetMouseInput` over a queue, `wait()` in place of busy polling, and `sleep()` for `delay()`.
  - `audio/sound.ts`: `Buzz`/`PlayTune` as a WebAudio square wave that lasts as long as the original did.
  - `game/game.ts`: `main()`'s loop, `HumanMove`, `AnimateMove`, `DropToken`, `NewGame`/`EndGame`, the goes-first rules, scores, best-of sets, draws, "Press ESC To Continue", `DemoMode`, `SuggestMove`, `SaveGame`/`LoadGame` (kept in memory until Phase 5), and the function keys.
  - Menus, help and exit go through a `GameUi` interface that does nothing until Phase 4.
- **Check:** `game.test.ts` replays golden games between two AIs through the whole loop (key handling, animation and drops), and they match the C move for move. Other tests drive human moves by keys, mouse and touch, and the Esc wait after a win.
- **Played in the browser:** keyboard moves and drops for both players, a mouse-click win with the enlarged winning tokens, F6 demo (AI against AI with the token gliding at Normal speed), F5 Suggest Move, and F8 sound off.
- **Changes from the original:**
  - **Pointer column:** the pointer chooses the board column it is over. The original mapped the hidden mouse's x across the whole screen.
  - **Pointer for both humans:** the original let only player one use the mouse. Here both can, so two people can share a touch screen.
  - **Touch:** a tap on a column moves the token there, and a second tap drops it.
  - **Continuing:** a click or tap continues after "Press ESC To Continue", as Esc does.
  - **Keypad:** the keypad's keys always act as the DOS keypad with NumLock off (the manual asked players to turn NumLock off), and the arrow keys do too.
  - **Calibration:** F9 (reset the mouse calibration) does nothing, since there is nothing to calibrate.
- **Tests:** 108 in 13 files.

### Phase 4 notes

- **Modules:**
  - `ui/menu.ts`: `MENU.C`, the bar and its drop-down menus.
  - `ui/dialog.ts`: `DIALOG.C`'s controls (list or range, shown as radio buttons, colour swatches, sliders, names or buttons) and dialogs with OK / Reset / Cancel. `CONTROLS.C` was an older version that the game never linked, so it isn't ported.
  - `ui/help.ts`: `HELP.C`, reading `NINARO.HLP` as `HelpNew` did.
  - `game/ui.ts`: `ProgInit`'s menus and dialogs, and `P1Dialog`/`P2Dialog`, `ModifyPlayer`, `GameSizeCustom`/`GameSizeFit`, `PlayerOptions`, `GameOptions` (the white background swaps the BLACK and WHITE palette slots), `HelpAbout`, and context help.
  - `game/title.ts`: `TitleScreen`.
  - `render/graph.ts` gained `_setlinestyle` (the dotted control frames).
- **Classic mode:** `classic.html?cfg=fresh` swaps in a default `NINARO.CFG` (Human v Human, Normal speed, mouse configured), because the release's own `.CFG` starts a fastest-speed AI demo. Reference frames saved: the title, the four menus, About, help, and the Board, Player Options and Game Options dialogs. js-dos's `screenshot()` lags a change behind with this program's page flipping, so wait for the frame to settle before saving.
- **Checked against the frames:** the menus, help, About and the three dialogs match in layout and colour, including bug 17. The Player dialog was built from the code.
- **Title timing:** the original filled the picture as fast as the PC allowed (under a second in DOSBox), stepping the background colour each time round. Here one time round is one display frame: about 5 seconds to fill, with a pulse about once a second. A key, click or tap continues. The slide-off animation and mouse calibration only ran when the mouse wasn't yet calibrated, so they don't apply.
- **Exit (F10, Game > Exit):** the original saved its settings and quit to DOS. Here the program starts again: the title, then a new game with the ProgInit messages.
- **Changes from the original:**
  - **Credit:** at the author's request, the credit reads "By Drew", in the game name window, the About box and the help's About section. The release had the full name; the redacted source had "By the author".
  - **Menus and the pointer:** hovering highlights, as the hidden mouse did. Clicking an item runs it, clicking a menu name opens that menu, and clicking elsewhere (or a right click) closes the menu. The original ran the highlighted item on any click.
  - **Dialogs and the pointer:** clicking a control's item chooses it, and clicking OK, Reset or Cancel presses it. The original's hidden mouse changed values just by moving.
  - **Help without a keyboard:** the mouse wheel or a finger drag scrolls, and a click or tap closes it.
  - **Board clicks:** during a move, a click outside the board does nothing. The original dropped the token on a click anywhere.
  - **Page buttons:** Menu and Help buttons under the screen send Esc and F1.
  - **Resizing:** the window can be resized with a menu, dialog or help open; it is redrawn.
  - **Help line endings:** git stores `NINARO.HLP` with LF line endings, so the help reader turns them back into CR LF first.
- **Tests:** 144 in 17 files.

### Phase 5 notes

- **`data/config.ts`:** `SaveCfg`/`LoadCfg` in the original's 100-byte `NINARO.CFG` layout (big-endian ints). The release's own file reads correctly (a test uses its values). Mouse calibration isn't kept (`mouseconfig` 0), and the copy slot is rebuilt when both players were the same AI, as `LoadCfg` did.
- **`data/storage.ts`:** each file's bytes are kept base64-encoded in `localStorage`: `ninaro:v1:cfg` and `ninaro:v1:save`. Every access is guarded; with storage blocked or full the game plays the same and forgets.
- **When it is saved:**
  - **Settings:** read at start-up, as `ProgInit` did. They are written on Exit, as the original did, and also whenever the page is hidden or closed (`pagehide`, `visibilitychange`), since a tab can be closed at any time.
  - **Ctrl-F9:** removes them and stops them being written again for the rest of the visit (`savecfg = FALSE`).
  - **F2 / Game > Save:** writes the one saved game, or says "~Save Error!" if it can't be kept. F3 / Game > Load reads it back.
- **Checked in the browser:** a 12×13×6 board and "went first" survived a reload, F3 brought back a saved token (entry token at `cols / 2`, bug 2), and Ctrl-F9 left nothing stored.
- **Changes from the original:**
  - **Damaged files:** a damaged `.CFG` is ignored (the defaults stand), and a damaged `.SAV` gives "~Load Error!". The original read whatever was there.
  - **Original files:** importing or exporting the original's `.CFG`/`.SAV` files (the plan's optional extra) isn't done. The stored bytes use the same layout, so it could be added.
- **Tests:** 156 in 19 files.

## Decisions (agreed 2026-11-08)

1. **Golden-game harness: yes.** Install a local C compiler (clang, gcc or zig) in Phase 1. The harness goes in `Tools/`, and its JSON output is committed as test data.
2. **Help text: use the pack's newer `ninaro.hlp`** (June 1998, 15.7 KB, 48 headings) instead of `Legacy/NINARO.HLP` (11.9 KB, 38 headings). Its "About Ninaro" section names the author, the university, the course, email addresses, and the supervisor. In Phase 1, copy it into `Legacy/NINARO.HLP` with that section replaced by the redacted wording already used in the user manual ("written by the author in 1998, as a university project"). Grep the result before committing.
3. **Font: Hershey stroke font**, drawn as strokes so `_setcharsize` stretching works as in the original.
4. **Original bugs: keep them during the port** (Loser-goes-first included). Phase 7 lists them all once the port is committed, and the chosen fixes go in their own commit.

## Personal details

Keep the author's surname, institution, course, student number, email addresses and the supervisor's name out of the app and out of commits. At the author's request, the app credits him by first name: "By Drew". The `Legacy` C source keeps its redacted "By the author". The pack's `read.me` and executable aren't redacted, so they stay in git-ignored `Temp/`. Before each commit, grep for the name, the institution, and `n[0-9]{7}`.

## Development

```bash
cd WebApp
npm install
npm run dev       # local dev server (port 5174)
npm test          # Vitest
npm run build     # type-check + production build to dist/
npm run preview   # serve dist/ (port 4174)
```

To regenerate the golden games after changing the harness (Windows, needs Visual Studio's C++ tools and Python):

```bat
Tools\golden\build.bat
```
