# Online Play: Plan

Two devices play one game through a tiny shared key-value store. Each device runs its own game (board, win checks, animation, sound); the store only carries the handshake and the moves, so both games stay in step.

## The store: keyvalue.immanuel.co

Checked by hand on 2026-11-10 (the home page and Swagger page were down; the API answered):

| Call | Result |
|---|---|
| `GET /api/KeyVal/GetAppKey` | A new app key, e.g. `"bk3qyo3s"` (a JSON string) |
| `GET /api/KeyVal/GetValue/{appKey}/{key}` | `"value"`, or `""` for a key never written |
| `POST /api/KeyVal/UpdateValue/{appKey}/{key}/{value}` (empty body) | `true` |

What matters for the design:

- **It fails a lot.** About half of all requests returned `503 Service Unavailable` in bursts. Every call must retry with backoff, and the protocol must never depend on any single request getting through.
- **Latency** about 0.85 s per request.
- **The value is in the URL path.** `.` gives a 404 and `:` an error page; letters, digits, `,` `-` `_` `~` `|` work. 64 characters stored fine (80 did too, but we keep to 64). Keys with `-` work.
- **CORS:** `GET` comes back with `Access-Control-Allow-Origin` echoing the page's origin. A `POST` with no body and no custom headers is a "simple" request, so the browser sends no preflight. Use exactly that: `fetch(url, { method: 'POST' })`.
- **No compare-and-swap**: the last write wins. There is an `ActOnValue` (increment) call, but we don't need it.
- **No security.** The app key ships in the page, and anyone who knows a room name can read or write it. That's fine for a game; the plan says so in the help.

The app key is fixed in the code: one generated with `GetAppKey` when this is built.

## Rooms and keys

- **Room name:** what the player types (e.g. `smithy`), or `?online=smithy` in the URL, or `ninaro` if neither is given. It is cleaned to `[a-z0-9-]`, at most 24 characters.
- **Two keys per room, one per side:** `ninaro1-<room>-h` (host) and `ninaro1-<room>-g` (guest). The `1` is the protocol version.

The request asked for one 64-character buffer. Two keys are better: each device **only writes its own key and only reads the other's**, so a write can never wipe out a message the other side hasn't read yet. That matters with a store that drops half its requests and has no locking. It costs nothing extra: each device still makes one GET per poll.

## Messages: each key holds its writer's whole state

Each write is a snapshot of the sender's state, not an event. So reading the latest value is always enough, a missed poll loses nothing, and writing the same thing again is harmless.

```
1,<role>,<nonce>,<cols>x<rows>x<connect>,<color>,<phase>,<game>,<move>,<col>,<beat>
```

| Field | Meaning |
|---|---|
| `1` | Protocol version. A different version aborts like a board mismatch. |
| `role` | `h` or `g` |
| `nonce` | 4 random base-36 characters, new each session. Tells two sessions in the same room apart, and settles a race to be host. |
| `cols x rows x connect` | This device's board, e.g. `7x8x4` |
| `color` | The sender's token colour, one hex digit (palette index 0–15) |
| `phase` | `W` waiting for an opponent (host) or joining (guest); `S` start, sent by the host; `P` playing; `X` aborted; `Q` quit |
| `game` | Game number in this session, from 1 |
| `move` | Number of this side's latest move in the game (0 = none yet). For `S`, who goes first: `h` or `g`. |
| `col` | Column of that move (0-based) |
| `beat` | Heartbeat counter, raised every 5 s while waiting so the other side knows this device is still there |

Example: `1,g,k3f9,7x8x4,c,P,2,11,5,0` is guest, colour 12, game 2, its move 11 in column 5. That's 27 characters; the longest possible (50x50x50, big counters) is about 42.

The request's `1,2,5` (player 1, second move, position 5) is this message's `role,move,col`. The other fields are what's needed to make it work with this store.

## Handshake

1. **Who is host.** On Connect, read the `-h` key.
   - Empty, or its `beat` hasn't changed in 15 s (a room left over from earlier): **become host**. Write `W` with a new nonce. After 2 s, read it back. If the nonce there isn't ours, another device wrote at the same moment and won (last write wins, and both devices read the same final value), so become guest instead.
   - A live `W` from a host: **become guest**.
   - A host already in `S` or `P` with a different guest nonce: the room is busy. Say "Room smithy is in use" and stop.
2. **Guest says hello.** The guest writes `W` (its role, nonce, board and colour) to `-g`, then polls `-h`. If its colour is the host's, it changes its own first (see Colours).
3. **Host checks.** The host polls `-g`. On a guest `W`:
   - Board or version different: write `X` and show `Board mismatch: you 7x8@4, them 12x13@6`. The guest reads `X`, shows the same, and both end the online game.
   - Same: clear the board, pick who goes first, and write `S,1,<h|g>`. The guest reads `S`, clears its board and starts too. Both show `Online: <name> goes first`.
4. **Who goes first.** Game 1 is random, picked by the host. After that, it alternates (Successive). Both devices work it out from the game number, so later games need no handshake.

## Colours

Both boards look the same: each player keeps their own colour on both screens.

- Each message carries its sender's colour. A device draws its own player in its own colour, and the Online player in the colour from the other side's message.
- **A clash** (both chose red): the guest gives way, before it writes its hello. It uses the same rule as `setP1_2`: white (7), or a random colour 1–6 if its colour was white. It says `Your colour is now <colour>` in the message box. The host never changes, so the result is the same on both devices.
- Changing your colour (the Player dialog) during an online game counts as changing player: it ends the online game, as below.
- The Online slot's own colour setting is never used; it always shows the opponent's colour.

## Playing

- On each device the remote side is a player of the new type **Online**. The local side is a human or any of the AIs. An AI against an online opponent comes free, since it's just another `getMove`; it moves at the local AI Speed. Two AIs can even play each other across two devices.
- Only one side on a device can be Online.
- **Local move:** after `dropToken` succeeds, write `P,<game>,<n>,<col>` with this side's move count `n`.
- **Remote move:** the Online player's `getMove` polls the other key every second until that side's move number is the next one expected for this game, then returns its `col`. The token then moves and drops exactly as an AI's does. While waiting, keys and the menu still work, as in `humanMove`.
- The game alternates strictly, so a side can't make move n+1 before reading the other's move n. One latest-move slot per side is enough.
- **Win and draw** are worked out on each device by the same code from the same moves, so they agree. At game over each device waits for its own Esc. Moves for the next game carry the next game number, and a device keeps polling until it gets there, so it doesn't matter who presses Esc first.
- **Leaving:** End Game (F4), changing a player, a board size change, Load (F3), demo mode (F6/F7) or closing the page writes `Q`. The other side shows `Opponent left` and ends the game. `pagehide` sends the write with `fetch(..., { keepalive: true })`, best effort.
- **Save (F2), Load (F3) and Suggest (F5) on the opponent's turn** are turned off during an online game.

## Network handling

- `kvGet`/`kvPut` retry 503s, network errors and timeouts: 300 ms, then doubling up to 4 s, with a little random jitter. A write isn't done until it returns `true`.
- Poll every 1 s while waiting for a move or reply, and slow to every 5 s after 2 minutes with no opponent, to go easy on a free service.
- After 20 s of failed requests the status reads `Connection lost, retrying...`. Nothing ends on its own; Esc or F4 leaves.
- Polling stops while the page is hidden and starts again when it's shown (phones suspend background tabs anyway). Snapshots make catching up free.

## The UI

**Choosing Online.** The Player dialog's list gets **Online** after the five AIs. The canvas dialogs have no text field, and a phone only brings up its keyboard for a real `<input>`. So choosing Online opens a small **HTML overlay** over the canvas, in the game's colours: `Room: [smithy ]  [Connect] [Cancel]`. It's filled from `?online=`, then the last room used (in `localStorage`), then `ninaro`. Enter connects; Esc cancels and puts the player back as it was.

If even the overlay proves awkward, the fallback is what the request suggested: no text box, the room from `?online=` or `ninaro`.

**URL.** `?online=smithy` makes player two Online in room `smithy`, and connects as soon as the title screen is passed. A **Copy link** button in the overlay copies the page's address with `?online=<room>` for sending to the other player.

**Messages** go in the message box (and so the Fullscreen flash): `Online: smithy`, `Waiting for opponent...`, `Connected as host`, `Board mismatch: ...`, `Opponent's turn...`, `Opponent left`, `Connection lost, retrying...`. The Type column reads `Online` for the remote player, and the toolbar shows `Human vs Online`.

## Code changes

| File | Change |
|---|---|
| `src/online/kvStore.ts` (new) | A small `KvStore` interface (`get(key)`, `put(key, value)`), and the keyvalue.immanuel.co version of it with retries, backoff and timeout; the room-name cleaner. Takes a `fetch` so tests can fake it. Everything else uses only the interface, so the service can be swapped (see Decisions). |
| `src/online/protocol.ts` (new) | Encode and parse the message; reject anything that doesn't parse. Pure functions. |
| `src/online/session.ts` (new) | The session as a state machine: `connecting → hosting / joining → started → playing → ended`. Host election, handshake, heartbeat, polling, `sendMove`, `waitMove(game, n)`, `quit`. Uses an `AbortSignal` so leaving stops everything. |
| `src/core/player.ts` | `PlayerType.Online = 2`. |
| `src/game/state.ts` | New slot `online: 8`, `Online` player; `logName` says `Online`. |
| `src/game/game.ts` | `getMove` for the online slot waits on the session (still handling keys). Sends the move after a local drop. `newGame` takes who goes first from the session. Leaving actions send `Q`; F2/F3 off while online. |
| `src/game/ui.ts` | Online in the Player dialog; opens the room overlay; only one side may be Online. |
| `src/data/config.ts` | Nine player colours: the buffer has 25 ints and uses 23, so the ninth colour fits (an old file gives 0, so fall back to the default if 0). An Online player is saved as Human: a reload is not in a room. `validSave` stays 0–7, since online games aren't saved. |
| `src/main.ts`, `index.html`, `src/style.css` | Read `?online=`; the room overlay; `pagehide` quit. |
| `Legacy/NINARO.HLP` (the web copy of the help) | An Online Play section. |

## Steps

1. **Store and protocol.** `kvStore.ts` and `protocol.ts`, with unit tests: retry on 503, give up on abort, room-name cleaning, encode/parse round trips, bad messages rejected.
2. **Session.** `session.ts` tested with two sessions on one in-memory fake store that fails 50% of calls at random: host election (including two at once), board mismatch aborts both, start agrees on who goes first, a colour clash moves the guest, 20 moves in step, quit seen by the other side, a stale room taken over.
3. **Game.** The Online player type and slot, `getMove`, sending moves, game numbers, colours, leaving. A `game.test.ts` test runs two `Game`s (one human, one AI) through the fake store to a win, and checks both boards match, colours included.
4. **UI.** The Player dialog entry, the room overlay, `?online=`, messages, Copy link. Check in the browser pane with two tabs on the dev server: both orders of joining, a mismatch, a page closed mid-game, a phone-sized view.
5. **Real store.** Generate the app key and play two real devices against the live service, including a phone on mobile data. Then the help text.

## Decisions

1. **Two keys per room**, not one shared buffer: agreed.
2. **A local AI may play an online opponent**: agreed.
3. **Both boards match**: each player has their own colour, sent in every message, and the guest gives way on a clash (see Colours).
4. **Another store is fine if this one is too flaky.** If step 5 shows keyvalue.immanuel.co can't hold a game together, add a second `KvStore` (first choice: a free Firebase Realtime Database; or a Cloudflare Worker with KV). The protocol, session and game code don't change.
