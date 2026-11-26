# Rebuilds the FM2 cheat mod (modified CommonEvent.dat) from the ORIGINAL unpacked data and optionally deploys it to the testbed.
#   .\tools\build-mod.ps1            -> mod\Data\BasicData\CommonEvent.dat
#   .\tools\build-mod.ps1 -Deploy    -> also copies it into testbed\Data\BasicData (refuses if the game is running)
#   .\tools\build-mod.ps1 -Debug     -> adds the on-screen time/variable overlay (debug aid)
param([switch]$Deploy, [switch]$Debug, [switch]$PadProbe, [switch]$WarpDebug)
$fm   = "F:\Personal Projjects\FM2"
$orig = "$fm\extracted\Data\BasicData\CommonEvent.dat"      # pristine, unpacked from the Steam archive
$out  = "$fm\mod\Data\BasicData\CommonEvent.dat"
& "$fm\tools\fm2mod\build.ps1"
if ($Debug) { $env:FM2_TIMEDBG = "1" } else { Remove-Item Env:FM2_TIMEDBG -ErrorAction SilentlyContinue }
if ($WarpDebug) { $env:FM2_WARPDBG = "1" } else { Remove-Item Env:FM2_WARPDBG -ErrorAction SilentlyContinue }
if ($PadProbe) { $env:FM2_PADPROBE = "1" } else { Remove-Item Env:FM2_PADPROBE -ErrorAction SilentlyContinue }
& "$fm\tools\fm2mod\fm2mod.exe" build-probe $orig $out MENU1
if ($Deploy) {
  if (Get-Process Game -ErrorAction SilentlyContinue) { Write-Warning "Game.exe is running - close it first, then re-run with -Deploy."; return }
  Copy-Item $out "$fm\testbed\Data\BasicData\CommonEvent.dat" -Force
  "deployed to testbed"
}
