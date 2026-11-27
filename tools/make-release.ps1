# Builds the PATCHER release zip (no game data inside): dist\FM2-Cheat-Mod-Patcher-<version>.zip
#   .\tools\make-release.ps1 [-Version v1.0]
# Prerequisites: .\tools\fm2mod\build.ps1 (fm2mod.exe) and .\tools\build-mod.ps1 (mod\Data\BasicData\CommonEvent.dat, used for its checksum).
# The original archive checksums come from the pristine copies in FM2\backup\Data.
param([string]$Version = 'v1.0')
$ErrorActionPreference = 'Stop'
$fm = 'F:\Personal Projjects\FM2'
$dist = "$fm\dist"; $rel = "$dist\FM2-Cheat-Mod-Patcher"
$names = 'BasicData', 'MapData', 'SystemFile'
$ce = "$fm\mod\Data\BasicData\CommonEvent.dat"
foreach ($f in "$fm\tools\fm2mod\fm2mod.exe", "$fm\tools\UberWolf\UberWolfCli.exe", "$fm\work\wolftl-src\bin\WolfTL.exe", $ce) { if (-not (Test-Path $f)) { throw "missing $f" } }
if (Test-Path $dist) { Remove-Item $dist -Recurse -Force }
New-Item -ItemType Directory -Path "$rel\tools", "$rel\licenses" | Out-Null
Copy-Item "$fm\tools\fm2mod\fm2mod.exe" "$rel\tools\"
Copy-Item "$fm\tools\UberWolf\UberWolfCli.exe" "$rel\tools\"
Copy-Item "$fm\work\wolftl-src\bin\WolfTL.exe" "$rel\tools\"
Copy-Item "$fm\tools\package\licenses\*" "$rel\licenses\"
Copy-Item "$fm\tools\package\fm2-install.ps1" $rel
Copy-Item "$fm\tools\package\RELEASE-README.txt" "$rel\README.txt"
# supported game states ("profiles"): archive checksums + the CommonEvent.dat the generator produces for them
function New-Profile([string]$name, [string]$archDir, [string]$ceFile) {
	$arch = [ordered]@{}
	foreach ($n in $names) { $p = "$archDir\$n.wolf"; if (-not (Test-Path $p)) { throw "missing $p" }; $arch[$n] = [ordered]@{ size = (Get-Item $p).Length; sha256 = (Get-FileHash -Algorithm SHA256 $p).Hash } }
	[ordered]@{ name = $name; archives = $arch; commonEventSha256 = (Get-FileHash -Algorithm SHA256 $ceFile).Hash }
}
$fullCe = "$fm\work\full_unpack\CommonEvent_mod.dat"      # generator output for the patched archives (see tools\compat_check.py)
$profiles = @(New-Profile 'stock Steam 1.21 (build 26781484)' "$fm\backup\Data" $ce)
if (Test-Path $fullCe) { $profiles += New-Profile 'Steam 1.21 + publisher full-version patch (en 20261119)' "$fm\work\full_unpack\Data" $fullCe }
[ordered]@{ version = $Version; builtAt = (Get-Date).ToString('s'); gameVersion = '1.21'; profiles = $profiles } | ConvertTo-Json -Depth 6 | Set-Content "$rel\package.json" -Encoding UTF8
Set-Content "$rel\Install.bat" "@echo off`r`npowershell -NoProfile -ExecutionPolicy Bypass -File `"%~dp0fm2-install.ps1`" -Action Install`r`npause" -Encoding ASCII
Set-Content "$rel\Uninstall.bat" "@echo off`r`npowershell -NoProfile -ExecutionPolicy Bypass -File `"%~dp0fm2-install.ps1`" -Action Uninstall`r`npause" -Encoding ASCII
$zip = "$dist\FM2-Cheat-Mod-Patcher-$Version.zip"
Compress-Archive -Path $rel -DestinationPath $zip
"release built: $zip  ($([math]::Round((Get-Item $zip).Length / 1MB, 1)) MB)"
Get-ChildItem $rel -Recurse -File | ForEach-Object { "  " + $_.FullName.Substring($rel.Length + 1) }
