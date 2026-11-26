# FalseMyth2 – exploration notes

Game folder (READ-ONLY, do not write here): `F:\SteamLibrary\steamapps\common\FalseMyth2`
All work/output for this project goes in `F:\Personal Projjects\FM2`.

Explored with plain filesystem listing only (no REA, nothing extracted yet).

## Engine
- **WOLF RPG Editor (Wolf RPG / ウディタ)**, runtime `Game.exe` v3.735.2026.1024 (Wolf 3.x, UTF-8 build — `wolfVersionFixTo` contains `u`).
- `Config.exe` = stock Wolf config tool (SilverSecond, 3.31).
- `GuruguruSMF4.dll` = MIDI playback lib bundled with Wolf.
- Game data is **packed**: `Data\*.wolf` files start with bytes `44 58 08 00` = **DX Archive v8** (DxLib), header is encrypted/keyed. Wolf 3.x Pro-style protection — needs a known key to unpack.

## Layout
| Path | Notes |
|---|---|
| `Game.exe`, `Config.exe`, `Game.ini` | vanilla runtime + settings (windowed, SE+BGM on, no log output) |
| `Data\BasicData.wolf` (5.9 MB) | database / common events (Wolf's `BasicData` — DB .project/.dat, CommonEvent.dat, Game.dat) |
| `Data\MapData.wolf` (252 KB) | maps (.mps) |
| `Data\SystemFile.wolf`, `SystemGraphic.wolf` | system graphics/sounds |
| `Data\MapChip.wolf` (118 MB), `CharaChip.wolf`, `Back.wolf`, `Fog*.wolf` | map tiles, sprites, backgrounds, fog |
| `Data\BattleEffect.wolf` (285 MB), `BattlePic.wolf` | battle animations / pictures |
| `Data\CG.wolf`, `HCG.wolf`, `Picture.wolf`, `face.wolf` | event CG, H-CG, pictures, face portraits |
| `Data\BGM.wolf` (605 MB), `SE.wolf` (41 MB) | audio |
| `Data\cv<Name>.wolf` (+ `*Loop.wolf`) | **voice packs per character**: Chuba, Jass, Levi, Lin, Marica, Maxi, Memeru, Miria, Nagi, Rick, Rita, Roma, Rouge(+C), Shil, Sxu(+2), Wech |
| `Data\*.ttf/.otf/.sfd` | bundled fonts (FalseMyth2 Condensed/Serif, Aozora Mincho, Zen Antique, Sawarabi Gothic, k8x12S, まにまに) — `NotoSansTC-Symbols-OFL.txt` is a font license |
| `Save\SaveData101.sav` + `steam_autocloud.vdf` | real Steam saves (only one) |
| `Save_MTool\` | MTool's own save dir (SaveData01/02/101-103) — see below |

Total data ≈ 1.4 GB, mostly BGM/BattleEffect.

## MTool residue (third-party cheat/translation tool, previously used on this install)
- `MTool_Game.exe` (3.736, modified runtime), `MTool_Config.exe`, `MTool_Game.ini`
- `StartWithTool.bat` injects `F:\Games\MTool\Tool\loaders\wolfHook3.dll` into `MTool_Game.exe` and launches `F:\Games\MTool\Tool\MTool.exe`.
- `Remove Tool files from game.bat` — cleanup script, restores `GuruguruSMF4.dll` from `.dll_mtoolbak`.
- `wolfDataLock.json` — MTool value lock: variable `003: Vars > 000 > 000 > 001 "★所持AP"` (held AP) = 840, lock=false. Gives one confirmed system DB variable path: **CDB/Vars type 3? / var index [0,1] = "★所持AP"**.
- Note: `Game.exe` (Oct 24 2026 build) differs from `MTool_Game.exe` (Nov 10 build) — the latter is newer, so the Steam game was likely patched; worth checking which one is the pristine reference.
- `Data` folder last modified 20-Nov-26, i.e. after MTool use — possible a Steam update touched it.

## Open questions / next steps
1. Decrypt/unpack the `.wolf` archives (need Wolf Pro key or WolfDec-style tooling) into `F:\Personal Projjects\FM2\extracted\...` — never in the Steam folder.
2. Decide what the actual goal is (translation? mod? cheat? asset study?) — not yet stated.
3. Compare `Game.exe` vs `MTool_Game.exe` to confirm which is vanilla.
4. Hold off on REA until the user says go.

---
## Update 2026-11-26 — Phase 0/1 done

**Approvals:** user OK'd REA and downloading a Wolf unpacker. Goal: mod/cheats, independent of MTool. Plan in `PLAN.md`.

**Backup** (`FM2\backup\`, from the Steam install, read-only copy): `Game.exe` (sha256 150EE7EC59DC…), `Game.ini`, `GuruguruSMF4.dll`, `Data\{BasicData,MapData,SystemFile}.wolf`, `Save\`, `Save_MTool\`.

**Unpacker:** `tools\UberWolf\UberWolfCli.exe` — official release v0.6.4 from github.com/Sinflower/UberWolf (sha256 0c964573…fb3a; the exe's own banner says v0.5.4). It auto-detected the key from `Game.exe`; no Pro protection key was needed (no `-u`).

**Unpacked to** `FM2\extracted\` (a working copy: `Game.exe` + `Data\*.wolf` + the extracted folders; the Steam folder is untouched):
- `Data\BasicData\` — `DataBase.dat/.project` (user DB), `SysDatabase.dat/.project` (system DB), `CDataBase.dat/.project` (variable DB: action gauge, battle chars, party stats, love level, equipment, events…), `CommonEvent.dat` (3.9 MB — all game logic), `Game.dat`, `TileSetData.dat`, `MapTree.dat` + ~388 PNG UI images.
- `Data\MapData\` — 122 `.mps` maps (Map001…).
- `Data\SystemFile\` — 13 `.ogg` + 12 `.png`.
- `.project` files carry the field/type names in readable UTF-8 (Japanese). `.dat` files use the Wolf `WOLFM`/`WOLFC` container (header bytes `00 57 00 00 4F 4C …`); they need a Wolf `.dat` parser to read values.
- Interesting names seen: CDataBase has `★現在冒険ポイント` (adventure points), `PTキャラステ` (HP/ATK/DEF/crit/PT Lv/love level…), `セバスショップ` (shop), `レベルロック解除？`, `☆DEVフラグ管理` (a DEV flag table — worth a look), SysDatabase has a `デバッグルーム` (debug room) map.

**Still not extracted** (not needed yet): graphics/audio/voice archives.

## Next
1. Parse `.dat` (DataBase / CDataBase / SysDatabase / CommonEvent / Game.dat) into JSON/text — find a parser (e.g. WolfTL / a Python Wolf dat reader) or write one; REA can help verify.
2. Locate AP (`[0,1] ★所持AP` var), currency, items, unlock flags, and the DEV flag / debug room.
3. Crack the `.sav` format (compare `Save\SaveData101.sav` with the `Save_MTool` ones) → save editor (Track A).

---
## Update 2026-11-26 — Phase 2 started: .dat parsed (WolfTL)

**Tool:** `tools\WolfTL\WolfTL.exe` v0.6.2 (official release, github.com/Sinflower/WolfTL, sha256 a66ffcdb…c276). Run: `WolfTL.exe --create --skip-game_dat <extracted\Data> <outdir>`.
- `Game.dat` fails with `Invalid magic string: "0000-0000 + Translation Tool [Pro]"` -> skipped (`--skip-game_dat`). Unexplained: the packed Game.dat carries a "Translation Tool [Pro]" marker (possibly added by a tool run on this copy; not investigated).
- Output: `work\wolftl\dump\` — `db\{DataBase,CDataBase,SysDatabase}.json`, `common\*.json` (942 common events, full command lists with intArgs), `mps\*.json` (122 maps).
- **Limitation:** WolfTL is a translation tool. The DB dump has type/field/item *names* and string values, but NOT the numeric cell values (enemy stats, equipment stats, drop rates…). Gameplay-number tweaks need a fuller `.dat` parser (or the Wolf editor).

**Variable ID scheme (confirmed from CommonEvent usage):** normal var n = `2000000+n`; spare var k (1–9) item n = `2000000 + k*100000 + n`. Strings use `3000000+` (SetString target). `1600000+n` = common-event self variable n. Full list: `docs\variables.md` (312 named vars, with reference counts; generator `tools\build_varmap.py`).

**Key variables for cheats**
| Var ID | Name | Meaning |
|---|---|---|
| 2000001 | ★所持AP | held AP (matches MTool lock path [0,1]) |
| 2000002 | ★所持リネル | held Linel (main currency) |
| 2000045 | ★所持お食事券 | meal tickets |
| 2000061 | ★隠しリネル保存用 | hidden-Linel save slot |
| 2100046 | ※還元用AP消費 | AP spent (refund) |
| 2100060 / 2100064 | 戦闘リネル倍 / リネル持ち帰り可 | battle Linel multiplier / can-keep-Linel flag |
| 2300007 | ミニゲーム リネル | minigame Linel |
AP is written by ~37 SetVariable commands, Linel by ~50 (e.g. common events 196 戦闘金リネル習得, 537 ★メニューステータスアップ, 601 ★マナちゃん工房, 594 釣り).

**Leftover dev content:** common event 39 `★デバッグ用` (shows ally/enemy XY debug text) and 207 `★テストイベント` (test scene) exist but nothing obviously calls them; SysDatabase has a `デバッグルーム` map; CDB type 2 `☆DEVフラグ管理`.

## Next
1. Save editor: crack `.sav` layout (compare `Save\SaveData101.sav` vs `Save_MTool\*`), set vars 2000001/2000002 etc.
2. Cheat menu / data mod: needs the real `.dat` writer — WolfTL `--patch` only rewrites translatable text, so editing commands/numbers requires another route (own `.dat` read/write, or Wolf editor).
3. Decide how to fetch numeric DB values (enemy/equipment tables) for gameplay mods.

---
## Update 2026-11-26 — Testbed + writer proven

**Testbed** `FM2\testbed\` : `Game.exe/Game.ini/Config.exe/GuruguruSMF4.dll` copied; every `Data\*.wolf` except the three logic archives is a **hard link** to the Steam file (don't modify those in place!); `Data\BasicData\`, `MapData\`, `SystemFile\` are loose folders (copies); `Save\` has a copy of SaveData101.sav. Run: `Start-Process testbed\Game.exe -WorkingDirectory testbed`. Game version 1.21 (window title).

**Findings**
- **The game loads loose `Data\<Name>\` folders instead of the .wolf archives** (booted to the joypad prompt with the three archives absent). So a mod = drop files into those folders; no repacking, no exe patch.
- **WolfTL is MIT and contains a full read/write library** (`WolfRPG/*.hpp`: CommonEvents, Database, Map, Game.dat, FileCoder). Built from source with MinGW:
  `git clone https://github.com/Sinflower/WolfTL work\wolftl-src`
  `cmake -S wolftl-src -B wolftl-build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-DUNICODE -D_UNICODE -DNOMINMAX" -DCMAKE_EXE_LINKER_FLAGS="-static"` then `cmake --build wolftl-build -j8` → `work\wolftl-src\bin\WolfTL.exe` (static; without `-static` it exits 127 silently).
- **Round trip OK:** `--patch --skip-game_dat` with the unmodified dump rewrites BasicData/MapData (CommonEvent.dat 3.51 MB, unencrypted) and the game still boots to the same first screen from the testbed. NOT yet verified deeper in gameplay (title/new game/battle).
- WolfTL `patch` only overwrites existing commands' text/args → adding new events/commands needs a small C++ program on top of the WolfRPG headers (build Command objects via `Command::Init` from a FileCoder buffer).
- Screenshot trick: use `PrintWindow(hwnd, dc, 2)` on the Game window (a plain `CopyFromScreen` grabs whatever is in front).

## Next
1. C++ tool `fm2mod` on WolfRPG headers: load CommonEvent.dat, parse each event's trigger fields (the `m_unknown*` blocks) to find an always-running parallel/auto event, append a new cheat common event, save.
2. Deeper round-trip test in gameplay (needs a human or key-injection into the testbed).

## Correction — saves (2026-11-26)
My first testbed only had Steam's `Save\SaveData101.sav`. The real playthrough saves (SaveData01, 02, 102, 103 + a newer 101) live ONLY in `Save_MTool\` because MTool relocates saves (its note says vanilla `Game.exe` and MTool's exe use different save-format versions; "don't copy these into Save"). With no slot saves the testbed went straight to the intro instead of the title menu (likely cause, not confirmed).
Testbed `Save\` now = copy of all `Save_MTool\SaveData*.sav` (old testbed saves kept in `testbed\Save_before_mtool_copy\`). Steam folders untouched. OPEN QUESTION: can vanilla Game.exe 3.735 read the MTool-written saves? If not, fallback = a fresh playthrough for testing, or analyze the .sav format.
Save files are encrypted per-file (random-looking header bytes, 0x55 at offset 6, 0xFF at offset 8).

---
## Update 2026-11-26 — playtest OK; hook research

**Round-trip verified by user:** title menu, slot loading, walking, NPC talk all fine on rewritten data in testbed.

**fm2mod** (`tools\fm2mod\fm2mod.cpp`, `build.ps1`): C++20 tool on the WolfTL headers (`#define private public` trick to reach fields). Mode `dump-triggers <CommonEvent.dat>` prints each event's header fields → `work\triggers.tsv`. Can also build/modify/save events (next).

**Common-event header decoding (inferred from data, NOT yet verified in-game):**
- `u1` = `(conditionVarID << 8) | flags` (e.g. 0x1E848020 → var 2000000). Low byte of flags: low nibble = trigger type (0 call-only, 1 auto-run, 2 parallel — matches names like `ACT管理並列処理` = 0x…22 and battle-victory events = 0x…21), high nibble = comparison op; `u2[1]` = compared value.
- 546+ events are call-only (var 2000000, flags 0x20). Parallel (type 2) examples: 556 ☆マップ表示 (var 2000018==1), 119 フィールド画面拡大率 (var 2000009==1), 824 ゲームパッド (var 2800001), 924 ジョイコン？キー判定.
- Plan: add our own parallel event conditioned on a variable that is always true, OR prepend a call to our hotkey-check event inside an always-running parallel event.

**InputKey (command 123) decoding** (from wolfrpg-map-parser `input_key.hexpat`): intArgs = `[resultVar, packed, keycode?]`. packed byte0: Basic mode: dir(4 bits)|ok(0x10)|cancel(0x20)|sub(0x40)|wait(0x80); Keyboard/Pad mode: byte0 bit7 = wait. packed byte1: low nibble = type (0 basic,1 keyboard,2 pad,3 mouse), 0x10 = specific key (then arg 3 = key code). The game uses e.g. `[1600012, 4352, 147]` (keyboard, key 147), `[1600000, 513, 100]` (pad). **Key-code numbering unknown** → needs an in-game probe.

**Other sources:** the Rust crate `g1org1owo/wolfrpg-map-parser` (hexpat files in `extra/wolf/`) documents command layouts.

## Next
- Build the first cheat event (AP/Linel) with a hotkey; needs in-game iteration → ask user whether I may drive the testbed (focus + SendInput + PrintWindow screenshots) while they're away from the PC.

---
## Update 2026-11-26 — FIRST WORKING LIVE CHEAT (F9 → +10000 AP & Linel)  ✅ verified in testbed
Driven by me via `tools\testbed\drive.ps1` (launch, focus, keybd_event, PrintWindow screenshots; testbed only). Slot-1 save: AP 790→10790, Linel 99→10099 after one F9 press (latch works: once per press).

**Hard-won facts about editing CommonEvent.dat**
1. **Every event must end with a Blank (code 0, indent 0) command.** Events without it are silently not run (this cost 4 failed probes).
2. Branch structure (indent = nesting): `111 VariableCondition [1,var,value,op]` ; `401 ChoiceCase [1]` ; body at indent+1 ending in `0 Blank` (indent+1) ; optional `420 ElseCase [0]` + body + Blank ; `499 BranchEnd` ; `0 Blank` (same indent as 499). op 2 = "==" (1 appears to be a different compare; untested).
3. Calling an event: `210 CommonEvent [500000+id, packedArgCount, args…]` (e.g. `[500566, 0]`).
4. **Trigger header** (`m_unknown1`, `m_unknown2`) copied from existing event 119 = *parallel, runs while V2000009 (★★移動) == 1 = on the field*. Copying a header verbatim works. Other triggers seen: battle events key on V2100000 (★アタック開始！).
5. **Appended events (id ≥ 942) were never proven**; instead reuse **blank slots** (name '' + 1 Blank cmd): ids 6–11, 82, 206, 211, 500, 592, 600, 611, 647, 768, 820, 841, 901, 929, 933–938. Slot **934 = "FM2 cheat hotkeys"**, 933 used in a call test. (Likely appending works too now that the Blank terminator is known — untested.)
6. **InputKey** `[var, 4352, code]` (keyboard + specific key) writes the **key code itself** to var while held, 0 otherwise. Game key codes (not DirectInput/VK): **Z=144, F1=159, F9=167, F10=168** (A reads 119?/unverified). Compare `var == 167`.
7. Text pictures: `Picture [32, num, 0,1,1,1,255, x,y, zoom, 0]` + string; variables via `\cself[n]` (single backslash in the real string). Picture 99999 draws on top.
8. Unused global var group: spare 5 (ID 2500000+) has no names/uses → used **V2500000 = F9 latch**. More free IDs in 2500000–2599999 and 2700000+.
9. Game keys: Z confirm, X menu/cancel, C sub (call heroine), V map. Title: Enter,Enter → Continue → slot → confirm.

**fm2mod build-probe variants** (`tools\fm2mod\fm2mod.exe build-probe <orig CommonEvent.dat> <out> <A|B|C|C2|D|K|P0>`; `FM2_DEBUG=1` adds on-screen K/L readout; `show-event <dat> <id> [n]` prints commands with indents). Output `work\probe\CommonEvent_P0.dat` is currently deployed in testbed.

## Next
- Turn P0 into a real cheat menu (Choices UI / multiple hotkeys: AP, Linel, items, god mode, unlocks…), add a battle-side trigger, then packaging (install/uninstall scripts for the Steam folder — NOT done; Steam folder still untouched).

---
## Update 2026-11-26 — INSERT CHEAT MENU WORKING (field only)  ✅ verified in testbed
`fm2mod build-probe <orig CommonEvent.dat> <out> MENU1` → deploy as `testbed\Data\BasicData\CommonEvent.dat` (`Deploy-CommonEvent` in `tools\testbed\drive.ps1`).
- **Insert** (key code 310) opens a dimmed menu: `+10000 AP`, `+10000 Linel`, `+100000 AP`, `+100000 Linel`, `Close menu`; Up/Down move (wraps), Z uses, X or Insert closes. Live AP/Linel shown. Verified: AP 790→10790, Linel 99→10099 via menu.
- **Input freeze:** `V2000009=0` (stops all V9-gated field events: game menu key 529, C/V keys, etc.) **plus `BanInput [496]`** (stops the engine's built-in walking). Restored on close with `V9=1` and `BanInput [247]` (the game's own value after its menus). Character does not move while open (verified by screenshots).
- **Close is deferred:** closing hides the UI but keeps the freeze until X/Insert/Z are released (state var CL), so a long X hold can't leak into the game's own menu (verified with a 1.2 s hold).
- Layout: event A = slot **500** (id < 529 so it runs before the game's menu-key event 529; header cloned from 119: parallel while V9==1) detects Insert; event B = slot **934** (parallel while M==1) is the menu loop.
- **Key codes:** Wolf code = **100 + DirectInput scancode**: Z 144, X 145, C 146, V 147, A 130, Enter 128, F1 159, F9 167, F10 168, Up 300, Down 308, Left 303?, Insert 310. Insert is also a default *cancel* key in the game (it opened the game menu until A ran first + froze V9).
- **Variables:** only ids that exist in SysDatabase tables are valid ("unsecured variable" error otherwise); event-local `cself` only 0-4 valid for cloned events. Free (allocated, unreferenced, not trigger vars) used: **2800000 M, 2800005 S, 2800006 LI, 2800007 LU, 2800008 LD, 2800009 LZ, 2900000 LX, 2900009 CL**. Still free: 2800006.. see `docs/` / scan in session; others: group 9 idx 10,13,17,20,21,22,36,43,44; group 3 idx 26-29; group 4 idx 16,17.
- Wolf shows an on-screen error box naming `CommonEv <id> Line <n>` — handy for debugging.
- Text escapes: `\v[n]` normal var n, `\vK[n]` spare group K var n, `\cself[n]`.
- **Gotcha:** in the Bash tool, backslashes in heredocs get eaten (`\v` → `\v`); use the Edit tool for C++ strings containing backslashes.
- `tools\fm2mod\evbuilder.hpp` = small DSL (`set/add/sub/mul/ifEq/edge/pic`) that emits correctly nested commands.

**Not done:** works on the FIELD only (V9==1). Battle / dungeon states need their own trigger + freeze flags. More cheats (stats, items, unlocks, god mode), install/uninstall scripts for the real Steam folder, save editor.

---
## Update 2026-11-26 — time system studied → `docs/time-system.md`
V22 ☆TIME管理 = time-of-day slot 0..13 (night = >9), V31 ◆タスク管理 = move counter (every 6th map transition → V22+1; HUD number = 6−V31), V0 = time stop, sleep event 520 sets V22=0/V31=0. No separate day counter. Live overlay verified T=10 (night icon), C=4 (HUD "2"). `FM2_TIMEDBG=1` adds an on-screen readout (slot 82).

---
## Update 2026-11-26 — TIME CHEATS in the Insert menu ✅ (see docs/time-system.md)
Menu is now 13 items (AP/Linel ×4, Morning/Midday/Night, ±1 slot, Freeze, reset move counter, New day, Close). Freeze = parallel event in slot 6 (cond FZ==1). Debug aids: `FM2_TIMEDBG=1` (on-screen T/C/SC/SH/NX/TS/FZ) and `FM2_TESTTICK=1` (event slot 929 pushes the clock +1/frame, capped at 11) when running `fm2mod build-probe <orig> <out> MENU1`. Correction: HUD number = slots left in the current period (day 8−slot, night 12−slot), not 6−V31. (CORRECTED: an earlier claim that slot 13 dumps the game to the title screen was wrong – that test never left the title menu because my boot key sequence was out of sync; Re-tested with the map confirmed: clock held at 13 for 10+ s on the field, nothing happened. Whether 13 forces something on the next transition/event is unknown.)

---
## Update 2026-11-26 — Group-1 cheats added (paged menu, warp, unlocks, floor-skip flags, currencies, zoom) → `docs/cheat-menu.md`
Menu is now paged (main + Currency, Time, Unlocks, Floor-skip, Zoom, 4 Warp pages). Built with `tools\build-mod.ps1 [-Deploy]`, output `mod\Data\BasicData\CommonEvent.dat`, deployed to testbed. **Validated offline only** (balanced blocks, valid variable ids); not run in the game – user will test. Warp = Teleport command layout `[target=-1, x, y, mapId, options]` (options bit0 = precise coords, high nibble = transition); mapId = index in SysDatabase 'マップ設定'; entry coords harvested from the game's own Teleports (`warp_targets.inc`). Debug room = map 109 (25x19). Floor-skip vars V71/V77/V79 coded, untested by request.

---
## Update 2026-11-26 — user test feedback round 1 → fixes
User report: lottery tickets work; zoom works but resets on area change; warp just closed the menu; casino coins / meal tickets not unlocked yet (note: still need testing); wants controller support + a suggested open combo.
- Zoom: added pin event (slot 7: ZE var 2300027 / ZV 2300028 → V10 each frame while on field). Items for 100/150/250/300 set the pin; "200% game default" turns it off.
- Warp: Teleport does not run from a parallel event → now an AUTO-RUN event (slot 8, header `(WPF<<8)|0x21`, WPF=2300026) runs when the menu closes with a pending warp (WP). Untested.
- Controller navigation: menu now reads BASIC input (8 up / 2 down / 10 confirm / 11 cancel) which merges keyboard + pad; verified under BanInput 496 with a probe (Down→2, Z→10, Up→8). Opening via controller still needs real button codes → `-PadProbe` overlay build deployed; waiting for the user's readings.
- Casino coins / meal tickets: kept in the menu but labelled "(untested)".
- New: `tools\validate_events.py` (offline structure/variable check; it caught a missing terminator in the warp event), `build-mod.ps1 -PadProbe`.

## Update 2026-11-26 — controller open combo = L3 + R3
User's probe readings (button,padcode,mask): select 307/128, start 308/256, L3 309/512, R3 310/1024, L1 305/32, R1 306/64, L3+R3 309/1536, L1+R3 305/1056. => code = 300 + mask bit; specific pad reads (`[var,4608,code]`) work and multi-button reads work. Select+Start rejected: Steam overlay. Built: hold L3+R3 opens (event 500) and closes (event 934) the menu; latch var LC 2300029. Clean build (no probe) deployed to testbed. Untested.

---
## Update 2026-11-26 — round 2 feedback: L3+R3 ✅, controller navigation ✅, zoom ✅; warp STILL does nothing; move speed requested
**Warp v3 = diagnostics, not a fix yet.** Picture 99950 "FM2 warp queued -> map N" is drawn when the menu closes with a pending warp; the auto-run warp event (slot 8) draws 99951 "FM2 warp event running" and erases both at its end. Interpretation: (a) "queued" stays on screen → the auto-run event never starts (trigger/auto-run semantics wrong); (b) "queued" disappears (maybe "running" flashes) but no map change → Teleport ignored/failed in that context; (c) both fine → arrival state. Other facts: the game itself has a Teleport inside a parallel event (556 world-map travel) and its door sequence ends with call 122 and no V9=1, so V9 is restored by the destination map's own init (unverified).
**Move speed implemented (untested):** route command ids (from wolfrpg-map-parser MoveType, low byte of the 16-bit id): 29=0x011d set move speed, 30=set move frequency, 31=set animation speed, 49=full-tile movement. Hero = Move target -2 (used by dungeon system + event 120). Slot 9 event applies `Move[-2]` route `29(n)` for n=1..8; vars SE 2400016 (enable) / SS 2400017 (speed); hooked at the end of events 122 and 566. New tools: `fm2mod route-stats|route-speed <dat>`.

---
## Update 2026-11-26 — round 3: root causes found (warp + speed)
User: speed doesn't work in the overworld; warp closes the menu instantly, no debug text.
1. **Warp = my menu bug, not a game limitation:** confirm dispatch cascaded into the newly selected page's row 0 in the same frame (see docs/cheat-menu.md). Row 0 of the warp page = a warp target → set WP + close() (hence "instantly closes", and no debug text because the warp event/queued text only appear after finalize... actually `queued` is drawn in finalize; the menu closing path was entered, but the user never reached a real selection). Fixed with a snapshot dispatch. Also explains "zoom works": entering the zoom page applied 100% zoom automatically.
2. **Speed:** target was -2 ("this event"); hero is -1. Fixed. Still unknown whether `Move[-1] 29(n)` changes the overworld hero's speed (the game never sets hero speed in common events; dungeon steps use a variable target + 29(6)).
Build is in `mod\`; the testbed copy was NOT updated because Game.exe was running (user must close the game, then `.\tools\build-mod.ps1 -Deploy`). New tool mode: `fm2mod route-event <dat> <id>`.

---
## Update 2026-11-26 — round 4: warp id-0 bug, speed via event 120
- User confirmed: menu navigation better, warp list visible, pages no longer act when opened. Warp still "did nothing, no debug text" — cause: the user picked "Rouge's Room" = map id 0 = my "no pending warp" value. Fixed (WP = index+1). Other targets probably would have queued; real Teleport behaviour still unverified, the on-screen diagnostics are still in.
- Speed: root cause of "no effect" = event 120 re-sets speed every frame (straight 3 / diagonal 2). Build now patches those five Move commands to honour SE/SS. Default walking speed = 3. Event 9 and the hooks in events 122/566 were removed (not needed). Kill-switch: `FM2_NO_SPEED=1` at build time.
- Build hygiene: `build.ps1` now deletes the old exe first and `build-mod.ps1` aborts if the compile failed (a failed compile had silently reused a stale exe for one validation run; the deployed files were not affected).

---
## Update 2026-11-26 — round 5: speed ✅ (works), warp arrives in darkness
User: movement speed works (event-120 patch). Warp teleports but the destination is black and the field stays frozen. See `docs/cheat-menu.md` ("Warp — arrival problem"). Rebuilt warp from event 556's sequence + watchdog; debug readout deployed (`-WarpDebug`). Also learned: in a *common event*, Move target -2 is the hero (that's why the event-120 speed patch works and why event 556's `Move [-2]` faces the hero).

---
## Update 2026-11-26 — round 7: warp declared working by user; Left/Right warp paging + custom amount
Built and deployed (clean build, no debug readout). Custom amount: signed CA 2500000, STEP 2700000, clamp-at-0 via `EB::ifBelowZero` (op 4 = "<"). Left/Right via basic input 4/6. Page indices: speed page 6 (if enabled), custom page 7, warp pages 8+. Untested.

## Update 2026-11-26 — round 8: Back arrow fixed, hold-to-repeat
User: warp Left/Right paging works perfectly. Fixed the stray cursor after Back on the custom-amount page (PS cursor mismatch) and added hold-to-repeat with acceleration. Deployed, untested.

---
## Update 2026-11-26 — round 9: Party, items & shop (untested)
Added equipment giving (event 197 call), love level (CDB 9), learn-all-skills (CDB 5), shop perks purchasable (CDB 6). New tools: `tools/dbdecode.py`, `tools/gen_equipment.py` (writes equipment_items.inc / shop_perks.inc). Database packed arg decoded: bits 16+ = which of type/item/field are CONSTANT (7 = all const, 5 = item variable, 0 = all variable), bits 8-15 = db kind (low nibble 0 CDB, 1 SYS, 2 UDB) | op (high nibble 1 = read, 0 = write), low byte = assignment operator << 4. Casino anti-cheat variable: only an unused 1800 set at load (see docs). Page indices: ... custom, warp pages, equipment pages (18), party hub, love page.

## Update 2026-11-26 — round 10: equipment hub (browse / type filter / first-letter finder)
User confirmed on testbed: equipment giving, love level, learn-all-skills and shop perks all work (the "(untested)" labels were removed for those). Built the hub described in docs/cheat-menu.md (Round 10). Code structure: page indices are precomputed (EQ0/EQHUB/TYPELIST/catFirst/LS0/letFirst/PARTY_PG/LOVE_PG), `parentOf` and `groups` drive Back/Cancel and Left/Right. Untested.

- 2026-11-26: live equipment search built + deployed to testbed (see docs/cheat-menu.md Round 11). TO TEST: typing filters, Backspace/Delete, paging, give, Z/X/Space typed as letters, Esc back. Still untested: equipment hub (type/letter), casino coins, meal tickets, floor-skip flags.

## 2026-11-26 — Installer
`tools\make-package.ps1` (after `build-mod.ps1`) builds `FM2\package\` = loose `Data\BasicData|MapData|SystemFile` (testbed base + modded CommonEvent.dat) + `fm2-install.ps1` + `install.bat`/`uninstall.bat` + `package.json` (SHA256 of the three original archives, game v1.21). `fm2-install.ps1 [-Action Install|Uninstall|Status] [-GameDir] [-DryRun] [-Force]`: Install refuses if the game runs / archives don't match the built-for version / a foreign loose folder exists; MOVES the 3 archives to `Data\_fm2_backup` (+manifest with hashes), copies the loose folders. Re-running Install = update/repair (removes archives Steam restored if identical to the backup). Uninstall deletes the loose folders and moves archives back (hash-checked). Saves never touched. Tested on a fake game folder (install, repair, uninstall → hashes identical). NOT yet run on the real Steam folder. Open: save location — the real playthrough saves are in `Save_MTool\`, the Steam `Save\` only has SaveData101; user must decide how to bring saves over.

## 2026-11-26 — cleanup + allies/gallery research (partial)
- Removed the "warp queued" diagnostic text; README.txt written (also copied into package\). Rebuilt + reinstalled into the Steam folder.
- Allies: events 858/860/880 = "ally gacha obtain" (offer Rinnel, rarity from UDB10 field 45, join-add count field 46); party members = UDB10 (31 entries). Per-character runtime state = CDB9 (fields 16 "can be taken out", 18 "PT menu OK?", 25 "ever unlocked?", 23 level lock released). Where "owned" is recorded is NOT yet identified (858 only writes temp CDB17).
- Gallery: album screens 825/842 read UDB 78/43 for data; unlock state is accessed through an indirect variable pointer (values like 1131000000 computed per scene) — decoding needed. Fish encyclopedia = CDB19 (40 entries "fish caught?"), simplest candidate (event 596). Encyclopedia event 930 reads UDB 77.
- Dungeon/battle menu: waiting until the user reaches a dungeon.

- 2026-11-26: GALLERY DECODED. Album screens 825/842 read CDB31 item n field 0 (seen flag, n=1..45) from the COMMON save slot 101 via command 221 [var,101,ptr,1]; each scene event writes it with command 222 SaveGame [1,101,1131000000+n*100,0] (pointer = 1,1TT,0000,00 style: 1,1xx = CDB type xx; + item*100 + field). Added menu row 'Unlock all album scenes (1-45)' (Unlocks page) = 45 x 222. UNTESTED. Slot 101 backups: Save_MTool\SaveData101.sav and FM2\backup. Fish encyclopedia (CDB19, fields 0/1/4-9 written by 598) and allies not yet decoded.
