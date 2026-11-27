FM2 Cheat Mod Menu - patcher (for FalseMyth2, Steam version 1.21)
==================================================================

This tool contains NO game data. When you run it, it builds the mod on your own PC from your own copy of the game
and installs it. It works on the Steam version 1.21 only (it checks the game files and refuses anything else).

Install
  1. Close the game.
  2. Unzip this folder anywhere and double-click  Install.bat
     (it finds the game by itself; if it can't, paste the game folder, the one that contains Game.exe).
  3. Wait about a minute while it builds. Start the game from Steam as usual.

Works with the publisher's full-version (H) patch
  Supported game states: the stock Steam files (build 26781484) and the stock files plus the publisher's
  "False Myth 2 English Full Version Patch" dated 20261119. The installer detects which one you have and builds the mod
  from it, so the patch's content stays in the game.
  Order matters: the publisher's patch refuses to install while this mod is installed (the original archives are moved away).
  To apply or update the patch:  Uninstall.bat  ->  run the publisher's patch  ->  Install.bat.
  If you have a different patch version, the installer refuses (it only knows the exact files listed above).

Uninstall
  Double-click  Uninstall.bat   - the original game files are put back. Your saves are never touched.
  If Steam "Verify integrity" restores the original files, just run Install.bat again.

What it changes in the game folder
  - Data\BasicData.wolf, MapData.wolf and SystemFile.wolf are MOVED into Data\_fm2_backup (nothing is deleted).
  - Data\BasicData, Data\MapData and Data\SystemFile (modded loose folders) are added.
  Nothing else is touched.

Using the cheat menu (on the field, not in battle or dungeons yet)
  Open / close:  Insert (keyboard)   or   hold L3 + R3 (controller)
  Up/Down = select, Left/Right = change page or amount (hold to repeat), Z / Enter / pad A = use, X / Esc / pad B = back
  Pages: Currency & items, Time, Unlocks (incl. album scenes), Floor-skip flags, Map zoom, Move speed, Warp to map,
  Party / items / shop (give equipment with live search, love level, skills, shop perks).
  Menu entries marked (untested) could not be checked yet because the related game area is not unlocked.

Command line (optional)
  powershell -ExecutionPolicy Bypass -File fm2-install.ps1 [-Action Install|Uninstall|Status] [-GameDir "<path>"] [-DryRun]

Third-party tools included (both MIT licensed, see the licenses folder)
  UberWolfCli  - Sinflower, https://github.com/Sinflower/UberWolf  (unpacks the game archives)
  WolfTL       - Sinflower, https://github.com/Sinflower/WolfTL    (reads and writes Wolf RPG data)
Source code of this mod: see the project page.

Not affiliated with the game's developer or publisher. Use at your own risk; keep your saves backed up.
