# False Myth 2 Cheat Mod Menu

An in-game cheat menu for **FalseMyth2** (a Wolf RPG Editor game, Steam version 1.21). It needs no injector, no memory editing and no third-party tool at runtime. The menu is built out of the game's own event system and installed by swapping three data archives for loose data folders.

Press **Insert** (keyboard) or hold **L3 + R3** (controller) on the field and a menu opens over the frozen game.

> Not affiliated with the game's developer or publisher. This repository contains **no game data**: you need your own copy of the game, and the tools here build the mod from it on your machine.

## Features

| Page | What it does |
|---|---|
| **Currency & items** | +AP, +Linel, lottery tickets, casino coins and meal tickets , and a **custom amount** editor |
| **Time** | Set the time-of-day slot, step it forward or back, freeze time, start a new day (resets daily limits) |
| **Unlocks** | World map, Wechina tour, Linel circle, assassin display, subjugation challenge, scene unlock flags |
| **Floor-skip flags** | The 50F / 70F / 100F dungeon skip flags |
| **Map zoom** | 100 to 300 %, and it stays when you change map |
| **Move speed** | Walking speed 2 to 8 |
| **Warp to map** | 41 destinations with proper arrival points; Left/Right flips pages |
| **Party, items & shop** | Give any of 188 pieces of equipment (browse everything, filter by type, find by first letter, or **live search by typing**), set the love level of each heroine, learn all skills, make every shop perk purchasable |


## How it works

Wolf RPG games keep their logic in *common events*: small scripts stored in `Data/BasicData/CommonEvent.dat`. This mod adds a few of them.

1. **Loose data folders override archives.** The game loads `Data/BasicData`, `Data/MapData` and `Data/SystemFile` as plain folders when they exist, so no executable patching or repacking is needed. The installer moves the three original `.wolf` archives into a backup folder and copies the modified folders in; uninstall reverses it.
2. **Event generator.** `tools/fm2mod/fm2mod.cpp` (C++20) loads the original `CommonEvent.dat` with the [WolfTL](https://github.com/Sinflower/WolfTL) `WolfRPG` headers, fills a handful of unused event slots with generated commands (via the small DSL in `evbuilder.hpp`), patches the game's walking event for the speed feature, and writes the file back. Nothing is hand-edited in a binary.
3. **The menu is a parallel event.** One event runs every frame while a "menu open" variable is set. It draws the menu as text pictures, reads input, and dispatches the action for the selected row. Other events:
   - *opener*: watches Insert or L3+R3 (reads keys with the `InputKey` command)
   - *freeze / zoom pin / warp*: keep the clock, zoom and teleports under control
   - *search events*: scan all 188 item names for the live search
4. **Input.** The engine's unified "basic input" gives up/down/left/right/confirm/cancel for keyboard and pad; specific-key reads handle Insert and the search typing; `BanInput` freezes the field while the menu is open.
5. **Live search.** There is no text-input command that suits this, so the menu reads about 40 individual keys each frame, keeps up to 8 typed characters in spare variables, rebuilds a string, and filters names by removing the typed text from each lowercase name and checking whether the name changed.
6. **Data discovery.** Cheats write the game's own variables and runtime databases, and equipment is given by calling the game's pickup event, so inventory and effects behave normally.

## Written in

- **C++20** (MinGW-w64 g++) for the generator, with the MIT-licensed WolfTL headers
- **PowerShell** for build, package, install and testbed helpers
- **Python 3** for data extraction and validation (generating the warp and equipment tables, offline checking of generated events)

## Install (end users)

Build the package (see below) or get a prebuilt `package/` folder, close the game, then run `install.bat` from it. `uninstall.bat` restores the original files. Saves are never touched. The installer refuses if the game's archives don't match version 1.21 (the mod replaces whole data files, so a mismatched version could break things).

## Build from source

You need your own installed copy of the game (Windows), MinGW-w64 `g++` and `gcc`, Python 3 and PowerShell.

1. Unpack `BasicData.wolf`, `MapData.wolf` and `SystemFile.wolf` with [UberWolf](https://github.com/Sinflower/UberWolf) into `extracted/Data/`.
2. Clone [WolfTL](https://github.com/Sinflower/WolfTL) into `work/wolftl-src` and dump the data (`WolfTL.exe --create ...`) into `work/wolftl/dump` for the `tools/gen_*.py` scripts.
3. `tools/fm2mod/build.ps1` builds `fm2mod.exe`.
4. `tools/build-mod.ps1` generates `mod/Data/BasicData/CommonEvent.dat` (add `-Deploy` to copy it into a test copy of the game).
5. `tools/make-package.ps1` assembles `package/` with the installer.

The scripts contain absolute paths at the top (`F:\Personal Projjects\FM2`, the Steam folder); change them to match your machine. `tools/validate_events.py` checks generated events offline (balanced blocks, valid variable ids).

## Repository layout

```
tools/fm2mod/         generator (fm2mod.cpp, evbuilder.hpp, generated *.inc tables, build.ps1)
tools/package/        installer script and player README
tools/*.py            table generators, validator, database decoder
tools/testbed/        helper for driving a test copy of the game
tools/build-mod.ps1   generate + deploy the mod
tools/make-package.ps1
docs/                 findings: variable map, time system, cheat menu design and test notes
NOTES.md, PLAN.md     working notes and the original plan
```

## Status and limitations

- Works on the field only. Dungeons and battles don't open the menu yet.
- Built for game version 1.21.

Work in progress: unlocking allies, the scene album and the encyclopedias, a dungeon and battle menu, and a save editor.

## Credits

- [WolfTL](https://github.com/Sinflower/WolfTL) and [UberWolf](https://github.com/Sinflower/UberWolf) by Sinflower (MIT) for reading and writing Wolf RPG data.
- Format notes from the Wolf RPG Editor community.
