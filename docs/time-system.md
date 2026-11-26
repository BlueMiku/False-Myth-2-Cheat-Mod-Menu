# FalseMyth2 overworld time system (studied 2026-11-26)

Sources: CommonEvent dump (`work/wolftl/dump/common`), SysDatabase variable names, plus a live overlay in the testbed
(`FM2_TIMEDBG=1 fm2mod build-probe … MENU1`, event slot 82). Items marked (live) were seen in the running game,
(static) come from reading the events only and are NOT yet verified in play.

## The variables
| Var | ID | Name | Meaning |
|---|---|---|---|
| V22 | 2000022 | ☆TIME管理 | **Time-of-day slot, 0..13** (14 slots per day). Hard-capped at 13 (`ceil` op 0x700). (live: 10 at the test save) |
| V31 | 2000031 | ◆タスク管理 | **Move counter.** +1 per map transition; when it exceeds 5 it resets to 0 and **V22 += 1**. (live: 4) |
| V0  | 2000000 | ☆タイムストップ中 | Time-stop flag. While 1, event/battle time costs are skipped. Also checked all over the game (77 refs), so pinning it is risky. (live: 0) |
| V23 | 2000023 | ★★シナリオ管理 | Story-stage counter (live: 83). Several time costs only apply when V23 ≥ 10 (static), the world-map cost only when > 25. |
| V32 | 2000032 | 白羅の日 | "Shirara day" special-day flag; switches the clock icon. Reset to 0 by the sleep event (live: 0) |
| V14 | 2000014 | □翌日イベントあり | A next-day event is pending (live: 0) |
| V33/V34 | | ★食堂利用済 / ☆喫茶店利用済み | Cafeteria / café used today – reset on sleep |

There is **no separate day counter**. "Days" exist only as per-feature counters (e.g. 金のなる木日数 = money-tree days, +1 per sleep) and
the story stage V23 (+1 at some sleeps).

## Slots and day/night (V22)
- **Night = V22 > 9** (slots 10–13), day = 0–9 (static, from ★簡易時計 584 and ☆たびたびチェック 123; moon icon was shown at V22=10 (live)).
- Sleeping resets V22 = 0 (morning). End-of-day events set V22 = 13.
- `夜でも変わらない特別` (SysDatabase map setting) = maps that don't change at night.
- Time of day gates: lighting/tint (MAPICSET 557, ☆たびたびチェック 123), BGM, NPC/event placement
  (`☆イベント配置用管理` has fields 終了後時間強制 / イベント後時間止めるか？ = "force time after event / stop time after event"), night-only events, fishing windows
  (`魚種類` fields 釣れる時間早/遅 and 条件２時間帯), and time-of-day voice variants (…夜).

## HUD (★簡易時計, event 584; live)
Top-right: bar + sun/moon icon + number.
- **Bar = progress of the move counter V31** (empty at 0, fills toward 6).
- **Icon** = sun for V22 ≤ 9, moon for V22 > 9 (live: sun at slots 0 and 5, moon at 10 and 11).
- **Number = slots left in the current period** (live): slot 0 → 8, 5 → 3 (day: 8 − slot); 10 → 2, 11 → 1 (night: 12 − slot).
  (An earlier note here said "6 − V31"; that was wrong, it only matched by coincidence at slot 10.)
Drawn by 584 / ☆マップ表示 / 画面左上管理 each time the field redraws; calling 123 + 584 after changing V22 refreshes it (live).
**Slots 12/13 (UNVERIFIED):** static reading suggests 13 is the end-of-day slot (event 122 stops advancing at 13; some H-scene events force 13).
An earlier note claimed that pushing V22 to 13 dumped the game to the title screen – that was WRONG: that test run never got past the title
menu (my boot key sequence was out of sync), so it proved nothing.
**Re-test (live, with the map confirmed on screen first):** the clock was pushed to slot 13 and held there (overlay T=13) for 10+ seconds while
standing on the field: nothing happened – no title screen, no sleep prompt, no forced event. So parking on 13 is harmless while idle on the field.
Unknown: whether 13 forces something at the next map transition or after an event. (The HUD number did not refresh by itself; it still showed the old "2".)

## What advances time (static unless noted)
| Event | Cost |
|---|---|
| ☆☆移動用処理 (122) – every map transition | V31 += 1; at V31 > 5: V31=0, **V22 += 1** (skipped when V22 already 13) |
| ☆イベント実行判定 (518) – after a map event | V22 += 1 if V0==0 and V23 ≥ 10 |
| ★フィールド戦闘後処理 (519) – after a field battle | V22 += 1 (same conditions) and may snap to slot 11 / 7 |
| ☆マップ表示 (556) – fast travel via world map | V22 += 1, V31 −= 1 when V23 > 25 |
| 食堂システム 632, ペリーノ喫茶店 648, casino games 694/696/697/916, fishing 593/597/681 | V22 += 1 |
| 連れ込み中メニュ 627, コスプレ 808 | V22 += 3 |
| ★リネルの花観察 679 | V22 += 3 / 5 / 7 |
| 討伐チャレンジ 788/829 | V22 += 2 |
| ナギ/ウェチナ/ジャスミン H-scene managers | V22 += 1 or set to 13 (end of day) |
| **おやすみなさい (520) – sleep** | V31 = 0, **V22 = 0**, V32 = 0, snow flags 0, hint shown, money-tree days +1, café/cafeteria flags reset, V23 +1 in some branches, then re-draws HUD |

Fast travel (V key map): the list opens fine but pressing Z did not travel on the test save (live) – not investigated.

## Ideas for time cheats (all just variable writes, V22/V31, so they fit the Insert menu)
1. Set time of day: V22 = 0 (morning) / 5 (midday) / 10 (night) / 13.
2. +1 / −1 slot (clamp 0..13).
3. Freeze time: a tiny parallel event that pins V22 and V31 each frame (safer than V0).
4. Skip to morning: V22 = 0 and V31 = 0 (what sleep does for the clock; skips the other sleep side effects such as money-tree growth).
5. Fully sleep: call event 520 itself (shows its "Sleep / Don't sleep" prompt) – needs testing.
6. Reset move counter V31 = 0 / 5.

## Implemented in the Insert menu (verified live in the testbed, 2026-11-26)
| Menu item | Effect | Verified |
|---|---|---|
| Time: Morning / Midday / Night | V22 = 0 / 5 / 10, then refresh HUD+lighting (calls events 123, 584) | yes (status line + HUD icon/number changed) |
| Time: +1 slot / −1 slot | V22 ±1, clamped 0..13 | yes (10→11→10) |
| Time: Freeze on/off | flag FZ (V2900010). Event in slot 6 (parallel while FZ==1, any game state) pins V22/V31 to the values saved when freezing | yes – held slot 0 for seconds while a test event pushed the clock every frame; released correctly |
| Time: reset move counter | V31 = 0 | yes (Moves 4/6 → 0/6) |
| New day (morning + reset daily limits) | V22=0, V31=0, V32=0, V33=0 (cafeteria used), V34=0 (café used) + refresh. Does NOT do the other sleep side effects (money-tree growth, scenario +1) | yes for the variables; side effects untested |
| ~~Sleep now (call event 520)~~ | dropped: calling the game's sleep event from the menu's parallel event did nothing (it is designed to be started from a bed event) | n/a |

Free variables used: FZ 2900010, PT 2900013 (pinned slot), PC 2900017 (pinned counter) in addition to those listed in NOTES.md.
Not verified: the freeze holding across a real map transition / activity; the lighting tint actually changing on an outdoor map (the test map is indoors).
