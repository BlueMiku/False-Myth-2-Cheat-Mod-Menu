# FalseMyth2 mod/cheat plan (draft — nothing here has been executed)

**Goal:** cheats and mods that work on a plain Steam install, with **no MTool dependency**. They should cover max AP/currency, stats/items/unlocks, and gameplay changes.
**Rules:** all work stays in `F:\Personal Projjects\FM2`; the Steam folder is never written to by me. Steps that need REA or a download wait for your OK.

## Phase 0 — Safety baseline (small, no tools)
1. Copy the pieces that matter into `FM2\backup\`: `Game.exe`, `Game.ini`, `Data\BasicData.wolf`, `MapData.wolf`, `SystemFile.wolf`, and `Save\`. This is about 7 MB and skips the 600 MB audio.
2. Decide which exe is vanilla. `MTool_Game.exe` (3.736) is MTool's patched copy, so the mod must target `Game.exe` (3.735). Use Steam "Verify integrity" to confirm it is unmodified.
3. Keep Steam Cloud in mind: `Save\steam_autocloud.vdf` means saves sync, so I would only ever edit copies.

## Phase 1 — Unpack the logic archives (needs your OK: REA or a third-party unpacker)
- Unpack only `BasicData.wolf` (DB and common events), `MapData.wolf` (maps) and `SystemFile.wolf`. The graphics and audio archives are not needed for cheats.
- The archives are encrypted DX-Archive v8. The key comes from `Game.exe`, so this is where REA (or a Wolf unpacker like UberWolf) is needed.
- Output goes to `FM2\extracted\`.

## Phase 2 — Map the game (read-only analysis)
- Decode the databases and common events. Find the variable and DB indexes for AP (`[0,1] ★所持AP` is already known from MTool's lock file), gold, items, character stats, unlock flags and CG/scene flags.
- Cross-check against `Save\SaveData101.sav` and the MTool saves to confirm the layout.
- Write the findings to `FM2\docs\` as a variable/DB map.

## Phase 3 — Build the mods (independent of MTool)
| Track | What | Covers | Notes |
|---|---|---|---|
| **A. Save editor** (do first) | Small script in `FM2\tools\` that edits a copy of a `.sav` | AP, currency, items, stats, flags | Touches no game files. Needs the save format cracked in Phase 2. |
| **B. Data overlay mod** | Modified `BasicData` and `MapData` as loose files or repacked archives, installed by a script | Gameplay changes: drop rates, difficulty, combat, events | I still have to verify that this Wolf build and its protection mode load loose files. Install and uninstall scripts restore the backup. |
| **C. In-game cheat menu** | Common event added to the DB, opened by a hotkey or menu item | Toggles (max AP, god mode), item spawner, unlock-all | Same delivery as B. This is the nicest end result. |
| ~~D. Exe/DLL hook~~ | Own injected hook, like MTool's | — | Skipped. It is the heaviest option and breaks on each game update. |

## Phase 4 — Package and maintain
- `FM2\mod\` holds the mod files, plus `install.ps1` and `uninstall.ps1` that back up first and restore on removal.
- The mod records which game version it was built for. A Steam update overwrites `Data\`, so the installer checks the version before applying.
- Test in a throwaway copy of the game folder in `FM2\testbed\`, not in the Steam folder.

## Proposed folder layout
`FM2\backup\  extracted\  docs\  tools\  mod\  testbed\  NOTES.md  PLAN.md`

## Decisions needed from you
1. **OK to use REA (or download a Wolf unpacker) for Phase 1?** Everything after that depends on it.
2. **Order:** I suggest A (save editor), then C (cheat menu), then B (gameplay tweaks). Say if you want a different order.
3. **Testbed:** OK to copy the full game (about 1.4 GB) into `FM2\testbed\`? Otherwise I'd copy only the logic archives and symlink the rest.
