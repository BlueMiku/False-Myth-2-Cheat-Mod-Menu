# Builds the installable package FM2\package\ : modded loose Data folders + installer + checksums of the game archives it was built for.
#   .\tools\make-package.ps1            (run .\tools\build-mod.ps1 first so mod\Data\BasicData\CommonEvent.dat is current)
# Base data = the testbed's loose folders (WolfTL round trip of the original data, verified in play); only CommonEvent.dat is replaced by the mod's.
param([string]$GameDir = 'F:\SteamLibrary\steamapps\common\FalseMyth2')
$ErrorActionPreference = 'Stop'
$fm = 'F:\Personal Projjects\FM2'
$out = "$fm\package"
$names = 'BasicData', 'MapData', 'SystemFile'
$modCE = "$fm\mod\Data\BasicData\CommonEvent.dat"
if (-not (Test-Path $modCE)) { throw "build the mod first: $modCE not found" }
if (Test-Path $out) { Remove-Item $out -Recurse -Force }
New-Item -ItemType Directory -Path "$out\Data" | Out-Null
foreach ($n in $names) { Copy-Item "$fm\testbed\Data\$n" "$out\Data" -Recurse }
Copy-Item $modCE "$out\Data\BasicData\CommonEvent.dat" -Force
$arch = [ordered]@{}
foreach ($n in $names) {
	$p = "$GameDir\Data\$n.wolf"
	if (-not (Test-Path $p)) { $p = "$GameDir\Data\_fm2_backup\$n.wolf" }   # already installed: the originals are in the backup folder
	$arch[$n] = [ordered]@{ size = (Get-Item $p).Length; sha256 = (Get-FileHash -Algorithm SHA256 $p).Hash }
}
[ordered]@{ builtAt = (Get-Date).ToString('s'); gameVersion = '1.21'; commonEventSha256 = (Get-FileHash -Algorithm SHA256 "$out\Data\BasicData\CommonEvent.dat").Hash; archives = $arch } |
	ConvertTo-Json -Depth 4 | Set-Content "$out\package.json" -Encoding UTF8
Copy-Item "$fm\tools\package\fm2-install.ps1" $out
Set-Content "$out\install.bat" "@echo off`r`npowershell -NoProfile -ExecutionPolicy Bypass -File `"%~dp0fm2-install.ps1`" -Action Install`r`npause" -Encoding ASCII
Set-Content "$out\uninstall.bat" "@echo off`r`npowershell -NoProfile -ExecutionPolicy Bypass -File `"%~dp0fm2-install.ps1`" -Action Uninstall`r`npause" -Encoding ASCII
Copy-Item "$fm\tools\package\README.txt" $out
"package built: $out  ($((Get-ChildItem $out -Recurse -File | Measure-Object Length -Sum).Sum / 1MB) MB)"
