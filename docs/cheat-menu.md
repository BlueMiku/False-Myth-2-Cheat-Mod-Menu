# FM2 Insert cheat menu — current state (2026-11-26)

Build: `.\tools\build-mod.ps1 [-Deploy] [-Debug]` → `mod\Data\BasicData\CommonEvent.dat` (built from the pristine unpacked original; `-Deploy` copies it into
`testbed\Data\BasicData`). Source: `tools\fm2mod\fm2mod.cpp` (variant `MENU1`), `evbuilder.hpp`, `warp_targets.inc`.
To undo in the testbed: copy `extracted\Data\BasicData\CommonEvent.dat` over `testbed\Data\BasicData\CommonEvent.dat`.

## Controls
Insert opens (field only) · Up/Down move (wraps) · **Confirm** (Z / pad confirm) use · **Cancel** (X / pad cancel) back (closes from the main page) · Insert closes.
Navigation uses the engine's BASIC input (results 8 up / 2 down / 10 confirm / 11 cancel), which merges keyboard and the game's configured controller buttons, and keeps working under `BanInput 496` (verified). **Opening** with a controller is not done yet: needs real button codes — build with `-PadProbe` to read them (see below).
While open the field is frozen (V2000009=0 + `BanInput 496`); closing waits until X/Insert/Z are released.

## Pages
- **Currency & items:** +10000/+100000 AP, +10000/+100000 Linel, +1000/+10000 casino coins (V47), +5 meal tickets (V45), +10 lottery tickets (2300012).
- **Time:** Morning/Midday/Night, ±1 slot, freeze, reset move counter, New day (see `time-system.md`).
- **Unlocks** (set the flag to 1, the value the game itself writes): world-map access (V21), Wechina tour (V46), Linel circle (2200085), assassin display (2200086),
  subjugation challenge (2200090), Jasmine scene unlock (2900031), Wechina scene unlock (2900035).
- **Dungeon floor-skip flags:** 50F (V77), 70F (V79), 100F (V71) = 1; "all off" = 0. The game tests each `== 1` in ★ダンジョン用移動処理.
- **Map zoom:** 100/150/250/300 set V10 *and* turn on a small parallel event (slot 7, runs before the game's field-zoom event) that re-applies the chosen value every frame, because the game resets V10 on area changes (reported by user). "Game default" turns the pinning off and sets 200.
- **Move speed (attempt 3, untested):** the game's walking driver is **event 120** (☆☆キャラチップ移動用): every frame, in its normal branch (V37 is 0 or 1), it reads the 8-way input and issues `Move [-2]` with route `29(n)` = *set move speed* — **3 for straight moves, 2 for diagonals** — so 3 is the game's normal speed, and any speed set once from outside is overwritten the next frame (attempts 1-2 failed for that reason, and attempt 1 also used the wrong target). Now each of those five Move commands is replaced by `if SE == 1 then Move with the chosen speed (straight = SS, diagonal = SS-1) else the original Move`. The page offers 2-8 (3 = normal) and "Back to the game's own speed". Whether the engine caps values above 6 is unknown. **To drop the feature from a build:** `$env:FM2_NO_SPEED=1; .	oolsuild-mod.ps1 -Deploy` (the page, the main-menu row and the event-120 patch all disappear; code stays in `fm2mod.cpp`). Not covered: the second group in event 120 (H-scene states, V37 >= 2, speeds 0/1) and the dungeon system (113/115, speed 6 via a variable target).
- **Warp to map:** 41 targets over 4 pages. Entry coordinates + options are the most common ones used by the game's own Teleport commands to that map.
  The menu closes, then sets WPF=1; an **auto-run** (blocking) event in slot 8 does the work: V11=1, V9=0, `Teleport [-1,x,y,map,opt]`, call event 122; the clock/move counter are saved before and restored after so a warp costs no time.
  (v1 did the Teleport from the menu's *parallel* event and did nothing – user report – so it was moved to an auto-run event, like the game's own map events.)

## Status of each piece
| Piece | Status |
|---|---|
| Insert open/close, freeze, navigation, AP/Linel items, time items | **tested in the testbed** (earlier builds) |
| Paging / sub-menus, new item layout | structure validated offline (balanced if/else blocks, terminators, all variables exist); **not yet run** |
| Lottery tickets | **works** (user) |
| Casino coins, meal tickets | **still need testing** (user hasn't unlocked them yet); labelled "(untested)" in the menu. Casino coin count is a plain counter; meal-ticket semantics (count vs 0/1) unclear |
| Unlock flags | not run; values from the game's own writes. 2900031 (Jasmine) is only ever written 0 in the data → effect unconfirmed |
| Floor-skip flags | not run (area not unlocked on the test save) – by request, to be tested later |
| Map zoom | **works** (user). Reset on area change → fixed with a pinning event, not yet re-tested |
| Warp | v1 (Teleport from a parallel event) did nothing; **v2 (auto-run event) also just closes the menu (user)**. v3 adds on-screen diagnostics: after closing, `FM2 warp queued -> map N` appears at the bottom-left; the warp event replaces it with `FM2 warp event running` and erases both when it finishes. What you see tells where it stops (see NOTES). Remaining risks: arrival state (V9 stays 0 until the destination map's own init sets it, like after a door); debug-room coordinates (12,10) are a guess; dungeon maps deliberately not included |

## Bugs found from user reports
- **Warp target 0 = "none":** the pending-warp variable WP held the map id, and the first target ("Rouge's Room") is map **0**, which the menu treats as "no warp pending" — so picking it closed the menu with no debug text (user screenshots). WP now holds the target index + 1. (fixed in the current build, awaiting re-test)
- **Page-entry cascade:** the confirm dispatch was a series of `if PG == p` blocks; an action that changed the page ("Warp to map >") was followed, in the same frame, by the later pages' blocks, which then saw the new page and ran *its first row*. Entering the warp page therefore warped to row 0 and closed the menu (user: "instantly closes"); entering Currency/Time/Unlocks/Zoom/Speed pages silently ran their first rows too. Fix: dispatch on a snapshot of page and row (C1/C2).
- **Wrong Move target for speed:** -2 is "this event" (the calling map event), **-1 is the hero** (sleep event 520: -2 faces right, -1 faces left = two people facing each other; Teleport also uses -1 for the hero). Speed event now targets -1.

## Left out on purpose
- Lower-route flags V55–V57: they hold route states 0–6, not on/off flags.
- Tutorial skip: the tutorial table lives in the runtime database (CDB), which the menu can't write yet.
- Dungeon/hidden-dungeon maps as warp targets: need dungeon state initialised by the dungeon system.

## Free variables used (global, allocated but unreferenced by the game)
M 2800000 · S 2800005 · LI 2800006 · LU 2800007 · LD 2800008 · LZ 2800009 · LX 2900000 · CL 2900009 · FZ 2900010 · PT 2900013 · PC 2900017 ·
PG 2900021 (page) · WP 2900022 (pending warp) · PS 2900036 (saved main cursor) · T31 2900043 / T22 2900044 (clock saved around a warp).
Event slots used: 6 (time freeze), 500 (opener), 934 (menu), optional debug 82 / 929.

## Controller support (2026-11-26)
- Navigation: engine BASIC input (keyboard + the game's configured controller buttons): up/down/confirm/cancel.
- **Open/close with a controller: hold L3 + R3 (click both sticks).** Pressing it again closes the menu. Insert still works on the keyboard.
- Game pad codes (measured by the user with the probe; code = 300 + bit position in the mask; specific reads `InputKey [var, 4608, code]` work and several buttons can be read at once):

| button | code | mask bit |
|---|---|---|
| L1 | 305 | 32 |
| R1 | 306 | 64 |
| Select/Back | 307 | 128 |
| Start | 308 | 256 |
| L3 | 309 | 512 |
| R3 | 310 | 1024 |
| L3+R3 | 309 (mask 1536) | |

- Select+Start is **not usable**: Steam grabs it (Steam overlay / on-screen keyboard). L3+R3 is not used by the game (the game uses confirm, cancel, sub and a map button that becomes a virtual V key press) and was readable.
- Probe build for finding more codes: `.	oolsuild-mod.ps1 -PadProbe -Deploy`.
- Status: **not yet run in the game** (the code was added after the user's probe readings).

## Warp — arrival problem (round 5)
User: warp now moves the hero but into "eternal darkness"; the hero can walk, but the in-game menu, the cheat menu and Call Heroine are dead (= V9 stays 0).
Findings: event 124 (画面表示コモン) is the arrival routine, started by each map's own "イベント設置処理" event: it draws the black picture 999999, lays out the map's events from DB 18, sets **V9 = 1**, erases the black picture and clears V11. The hero standing in black with a normal HUD = 124 started but never finished. Entry coordinates are fine (`tools/check_warp_bounds.py`: with the 'precise' bit the numbers are half-tile units and all 41 fit their maps).
Changes (untested): the warp event now copies the game's own menu-driven warp (event 556): black fade picture -> staging Teleport to map 28 -> Teleport to the destination (option | 0x20 = fade) -> Move -> V30=0 -> events 122, 584, **582** -> V8=0 -> Wait 1. A **watchdog** (slot 9, parallel while WF=2600001 == 1) forces V9=1, V11=0, V13=1 and erases picture 999999 if V9 is still 0 after 180 frames. `build-mod.ps1 -WarpDebug` adds a top-left readout `V9 V11 V13 V17 V18 WF WC` (appears after the first menu open).

## Warp — round 6 (user: warp works on other maps; Rouge's Room black; stuck inside props)
- Warp to ordinary maps **works** (HUD name updates, field control returns, watchdog idle: WF=0 WC=0).
- "Rouge's Room" was map **0 = the title-screen map** (black). The real Rouge's Room is map **2** (the 50x50 map with the 33 events) — swapped in.
- Stuck in props: the arrival points were the *most common* ones from all Teleports in the data, which for many maps are **story-scene spots** (Nagi's chair, the infirmary bed, ...). `tools/gen_warp_targets.py` now prefers, in order: (1) Teleports inside map events preceded by a door/drawer/step/open sound, (2) map-event Teleports using the fade option 33, (3) any map-event Teleport, (4) common-event Teleports (last resort). Result: 25 door-based, 11 map-fade, 1 map, 4 common (32 Entertainment Area, 48 Amda's Hideout, 51 Forest Depths, 74 Piera's Arcade (2), 97 Inn Interior, 98 Elevator are the least certain).
- If a destination still traps the hero, warp again from the cheat menu (it works while stuck because V9 is 1).

## Round 7 additions (2026-11-26) — untested
- **Warp pages scroll with Left/Right** (basic input 4/6, wraps around). The "Next/Previous page" rows still exist. Left/Right do nothing on other pages except the custom-amount page.
- **Custom amount** (Currency & items -> "Custom amount (add or subtract)"): a signed amount `CA` (V2500000) is edited with Left/Right on the first row by a step `STEP` (V2700000: 1/10/100/1000/10000/100000, cycled with Confirm on the second row; starts at 100). Rows: flip sign, set to 0, then "Add amount to" AP / Linel / lottery tickets / casino coins (untested) / meal tickets (untested). Negative amounts subtract. **Results are clamped at 0** (a variable that goes below 0 is set back to 0) because it is unknown how the game treats a negative AP/Linel.
- Left/Right latches LL 2600004 / LR 2600009. The warp debug readout is OFF in the normal build (`-WarpDebug` brings it back).

## Round 8 fixes (2026-11-26) — untested
- **Stray cursor after Back:** the custom-amount page is opened from the Currency page, but Back/Cancel returned to the *main* page and restored the Currency page's cursor row (8), which doesn't exist on the main page (8 rows, 0-7), so the arrow pointed at nothing. Now Back and Cancel on the custom page return to the **Currency page with the cursor on the "Custom amount" row**; entering the custom page no longer overwrites the saved main-menu cursor (PS).
- **Hold Left/Right on the amount row:** first press acts at once; after ~15 frames it repeats every 3rd frame; after ~60 frames every frame (frame counter HC = V2900020, reset on close). Uses `ifGt` (op 1 = ">") and `%=` (op 0x500). Step size still sets how much each repeat changes.

## Round 9 — Party, items & shop (2026-11-26) — ALL UNTESTED
Main menu -> **Party, items & shop**:
- **Give equipment** (18 pages, 188 items, Left/Right flip pages): each row runs the game's own "obtain equipment" event **197** with the same call the battle-victory/dungeon events use, `CommonEvent [500197, 4, 10100, 512, 384, <equipment id>]` (the 4th argument is the id into UserDB 15 ★装備品管理). The game itself updates the inventory (runtime table CDB 13) and plays the pickup effect. Unknowns: whether the pickup popup waits for a keypress (it uses basic-input confirm), and whether the inventory has a size cap.
- **Love level:** target level 0-10 (Left/Right on the first row), then per character or "all": writes runtime table **CDB 9 "PTキャラステ"** item = character (1 Rouge, 2 Romario, 3 Sylphina, 4 Lillie, 5 Memel, 6 Nagi), field 12 love level = target, field 13 love exp = 0, field 23 "level lock released" = 1. (Event 645 treats level > 9 specially unless the lock is released.) Which of these characters actually use the love system is not verified.
- **Learn all skills:** CDB 5 "party member skills learned / power", field 0 "learned?" = 1 for all 80 entries.
- **Shop: make every perk purchasable:** CDB 6 "Sebas shop owned count", field 1 "purchasable?" = 1 for all 46 named perks (UserDB 33: Max SP +1, Overkiller, Healing Mastery, Equip Slots +1, tomes, fishing perks, lucky cats ...).
- Back/Cancel now go to the page's parent (equipment/love -> Party hub, custom amount -> Currency, others -> main).
- How Database writes are built: `Database [type, item, field, packed, value]`, packed = (7<<16) [type/item/field constants] | (0<<8) [runtime DB, write] | (op<<4); verified against the game's own constant writes (packed 458752). `tools/dbdecode.py <eventId>` prints any event's Database commands readably.
- Not done: **allies** (events 858/860/880 are gacha/bonus-ally flows with their own state), **scene album & encyclopedia** (CDB 31 turned out to be casino-only; the real unlock storage is not found yet).

## Casino anti-cheat variable (V2900019 "★カジノ不正防止") — findings
The only reference in the whole data (942 common events + 122 maps) is `SetVariable [2900019, 1800]` in event 566 (load processing). Nothing reads it: no condition, no calculation, no text code, no indirect/range access. 1800 frames = 30 seconds at 60 fps, so it looks like a leftover "after loading a save, casinos are locked for 30 s" timer whose check was removed or never written. Caveat: absence in the event data isn't proof about the engine, and the casino games themselves haven't been run.

## Round 10 — equipment search/filter (2026-11-26) — untested
"Give equipment" is now a hub (Party, items & shop -> Give equipment):
- **Browse everything (by page):** the previous 18-page list (Left/Right flips pages).
- **Filter by type:** Swords 29, Spears 19, Axes & hammers 23, Armor & robes 25, Bows 19, Staves 24, Shields 19, Gauntlets & claws 13, Charms & consumables 13, Gems 4 (from UserDB 15 field 2: types 1,2,3,4,5,9,10,11,8,7+12). Each category is its own run of pages.
- **Find by first letter:** letters that start at least one name, 13 per selection page (Left/Right flips the selection pages), then one run of pages per letter. Names that start with a non-letter go under "#". There is no free-text search (the game has no text-input command usable from here).
- Back/Cancel return to the screen you came from, with the cursor on the row you opened (parentOf table); Left/Right wrap inside the current run of pages (groups table).
- Event 934 grew to ~12,000 commands (event 123 in the game has 5,000+); only the current page's block runs each frame.
- Equipment name cleanup now strips `\f[16]\-[02]`-style text codes (they leaked into a few names before).

## Round 11 — live equipment search (2026-11-26) — UNTESTED
Door in the equipment hub: **Search by typing (live, keyboard)**. Shows every item at first; each key press filters the list on the fly.
- Keys read each frame by event 937 (`InputKey [var,4352,100+DIK]`): A-Z, 0-9, space, `-`, `'`; Backspace erases one character, Delete clears; max 8 characters (key codes kept in 8 spare variables, count in V2000082, text rebuilt into string var 3000021).
- Event 935 (refresh): scans all 188 names (unrolled). Match = text empty, or "remove every occurrence of the typed text from the lowercase name" changes the name (SetString op 8 on a copy in string var 3000022, then StringCondition !=). Draws the matches in the window starting at OFFS (11 rows per page) as pictures; MC = match count, RC = rows shown.
- Event 936 (give): same scan, runs the pickup event 197 for the match whose rank == GT (= OFFS + cursor + 1).
- Up/Down move in the results, Left/Right flip result pages, Enter / pad A gives, Esc / pad B returns to the hub. Z, X and Space are letters on this page, so the basic confirm/cancel are ignored while a typing key is held (LASTK != 0). Opening the page sets LASTK = 999 so the key that selected the row isn't typed.
- Keyboard only for typing; names are matched case-insensitively (lowercased literals vs. typed lowercase text).
- UNVERIFIED engine assumptions (decoded from samples, never run): SetString op 8 with a string-variable source, StringCondition "!=" encoding, text code `\s[21]`, variable picture numbers on text pictures (used by the game on image pictures).
- Variables: typed chars V2000007/16/19/25/26/49/80/81, count V2000082, LASTK V2100017, MC V2100050, OFFS V2100065, GT V2100069, RC V2200018, strings 3000021/3000022.

### Round 11 fixes (after first test)
- Both reported bugs had one cause: in this engine, VariableCondition op 0 behaves as a STRICT ">" (not "≥"). `K0 >= 1` hid rank 1, and `TN >= n` made the typed text lag one key behind. Op 3 (<=) was confirmed inclusive. New code avoids op 0/1 boundaries: the typed text is rebuilt from the 8 slots (unused = 0), cursor wrapping uses ==, rank test is `K0 > 0`. NOTE: `ifOp(.., 0, ..)` means ">" ; `ifGt` (op 1) is probably ">=" — only used where the difference doesn't matter.

## Round 12 — action feedback toast (2026-11-26) — untested
Every cheat row now shows a line under the help text for ~2.5 s (150 frames): "Done: <row text>" plus the new value where useful (AP / Linel / casino coins / meal tickets / lottery tickets / time slot / freeze state / custom amount / step). Equipment rows and the search "give" show "Done: <id> <name>". Navigation rows (">" / "<" / Next page / Close menu / warp targets) give none. Picture 99917 (PIC_TOAST), timer variable V2200013 (FB); close() erases it. Implemented generically in `toastFor()` in fm2mod.cpp (Row.nofb opts a row out).
