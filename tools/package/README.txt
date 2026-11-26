FM2 cheat mod for FalseMyth2 (game version 1.21)
==================================================

Install / uninstall
  install.bat      installs the mod into F:\SteamLibrary\steamapps\common\FalseMyth2
  uninstall.bat    puts the original game files back
  (other game folder:  powershell -File fm2-install.ps1 -GameDir "<path>" [-Action Install|Uninstall|Status] [-DryRun])
  Close the game first. Saves are never touched. The three original archives (BasicData/MapData/SystemFile .wolf)
  are kept in Data\_fm2_backup while the mod is installed. If Steam restores them ("Verify integrity"), run install.bat again.
  If the game is updated the installer refuses (the mod was built for 1.21); uninstall, then ask for a rebuilt package.

Using the cheat menu (on the field, not in battle or dungeons yet)
  Open / close:  Insert (keyboard)   or   hold L3 + R3 (controller)
  Up/Down = select, Left/Right = change page or amount (hold to repeat), Z / Enter / pad A = use, X / Esc / pad B = back
  While the menu is open the game is frozen; every action shows a "Done: ..." message at the bottom.

  Currency & items   +AP, +Linel, lottery tickets, casino coins (untested), meal tickets (untested), custom amount (+/-)
  Time               set the time-of-day slot, step it, freeze time, new day
  Unlocks            world map, Wechina tour, Linel circle, assassin display, subjugation, scene unlock flags
  Floor-skip flags   50F / 70F / 100F dungeon skip flags (untested)
  Map zoom           100-300 % (kept when changing map)
  Move speed         walking speed 2-8
  Warp to map        41 destinations (Left/Right flips pages)
  Party, items & shop  give equipment (all / by type / by first letter / live search by typing), love level, learn all skills, shop perks

Notes
  - Typing search is keyboard only; letters, digits, space, - and ' ; Backspace erases, Delete clears.
  - Menu options marked (untested) could not be checked yet because the related game area is not unlocked.
