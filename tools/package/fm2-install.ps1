# FM2 cheat mod installer / uninstaller / status.
#   .\fm2-install.ps1                    install (or update) the mod into the game folder
#   .\fm2-install.ps1 -Action Uninstall  restore the original game files
#   .\fm2-install.ps1 -Action Status     show what is installed
#   add -DryRun to see what would happen without changing anything; -GameDir <path> for a different game folder.
#
# How it works: the game loads loose folders Data\BasicData, Data\MapData and Data\SystemFile instead of the matching .wolf archives (proven in the testbed,
# where the archives are absent). Install MOVES those three archives into Data\_fm2_backup (same drive, instant, nothing deleted), then copies the modded
# loose folders in. Uninstall deletes the loose folders and moves the archives back. Save files and every other game file are never touched.
param(
	[ValidateSet('Install', 'Uninstall', 'Status')][string]$Action = 'Install',
	[string]$GameDir = 'F:\SteamLibrary\steamapps\common\FalseMyth2',
	[switch]$DryRun,
	[switch]$Force
)
$ErrorActionPreference = 'Stop'
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$pkgData = Join-Path $here 'Data'
$pkg = Get-Content (Join-Path $here 'package.json') -Raw | ConvertFrom-Json
$data = Join-Path $GameDir 'Data'
$bak = Join-Path $data '_fm2_backup'
$manifestPath = Join-Path $bak 'manifest.json'
$names = @('BasicData', 'MapData', 'SystemFile')

function Say($m) { Write-Host $m }
function Run($desc, [scriptblock]$sb) { if ($DryRun) { Say "[dry-run] $desc" } else { Say $desc; & $sb } }
function Sha($p) { (Get-FileHash -Algorithm SHA256 -LiteralPath $p).Hash }
function Fail($m) { Write-Host "ERROR: $m" -ForegroundColor Red; exit 1 }

if (-not (Test-Path (Join-Path $GameDir 'Game.exe'))) { Fail "Game.exe not found in $GameDir" }
$installed = Test-Path $manifestPath
$manifest = $null
if ($installed) { $manifest = Get-Content $manifestPath -Raw | ConvertFrom-Json }

if ($Action -eq 'Status') {
	Say "Game folder : $GameDir"
	Say "Installed   : $installed"
	foreach ($n in $names) {
		$arc = Join-Path $data "$n.wolf"; $loose = Join-Path $data $n; $b = Join-Path $bak "$n.wolf"
		Say ("  {0,-11} archive in Data: {1,-5}  loose folder: {2,-5}  archive in backup: {3}" -f $n, (Test-Path $arc), (Test-Path $loose), (Test-Path $b))
	}
	$ce = Join-Path $data 'BasicData\CommonEvent.dat'
	if (Test-Path $ce) { Say ("CommonEvent.dat matches this package: {0}" -f ((Sha $ce) -eq $pkg.commonEventSha256)) }
	if ($installed) { Say "Installed at: $($manifest.installedAt)" }
	exit 0
}

if (Get-Process Game -ErrorAction SilentlyContinue) { Fail 'Game.exe is running - close the game first.' }

if ($Action -eq 'Install') {
	foreach ($n in $names) { if (-not (Test-Path (Join-Path $pkgData $n))) { Fail "package is missing Data\$n" } }
	if (-not $installed) {
		# fresh install: every archive must be present, match the version the mod was built for, and no loose folder may be in the way
		foreach ($n in $names) {
			$arc = Join-Path $data "$n.wolf"
			if (-not (Test-Path $arc)) { Fail "$arc not found (is the game already modded by something else?)" }
			if (Test-Path (Join-Path $data $n)) { Fail "Data\$n already exists and was not created by this tool - not touching it." }
			$h = Sha $arc
			if ($h -ne $pkg.archives.$n.sha256 -and -not $Force) { Fail "$n.wolf is not the version this mod was built for (game updated?). Rebuild the package, or use -Force at your own risk." }
		}
		Run "create backup folder $bak" { New-Item -ItemType Directory -Path $bak | Out-Null }
		$files = @()
		foreach ($n in $names) {
			$arc = Join-Path $data "$n.wolf"
			$files += [pscustomobject]@{ name = "$n.wolf"; size = (Get-Item $arc).Length; sha256 = (Sha $arc) }
		}
		Run 'write manifest' { [pscustomobject]@{ installedAt = (Get-Date).ToString('s'); files = $files } | ConvertTo-Json -Depth 4 | Set-Content -Path $manifestPath -Encoding UTF8 }
		foreach ($n in $names) { Run "move $n.wolf -> _fm2_backup" { Move-Item -LiteralPath (Join-Path $data "$n.wolf") -Destination (Join-Path $bak "$n.wolf") } }
	}
	else {
		# update / repair: archives that Steam restored (verify, update) are removed again once they are proven identical to the backed-up copy
		foreach ($n in $names) {
			$arc = Join-Path $data "$n.wolf"; $b = Join-Path $bak "$n.wolf"
			if (Test-Path $arc) {
				$want = ($manifest.files | Where-Object { $_.name -eq "$n.wolf" }).sha256
				if ((Sha $arc) -eq $want) { Run "Steam restored $n.wolf (identical to backup) - removing the duplicate" { Remove-Item -LiteralPath $arc } }
				elseif (-not $Force) { Fail "$n.wolf in Data differs from the backup (game updated?). Run Uninstall, rebuild the package for the new version, then Install. (-Force overrides.)" }
				else { Run "Force: removing the differing $n.wolf" { Remove-Item -LiteralPath $arc } }
			}
		}
	}
	foreach ($n in $names) {
		$dest = Join-Path $data $n
		if (Test-Path $dest) { Run "replace Data\$n" { Remove-Item -LiteralPath $dest -Recurse -Force } }
		Run "copy Data\$n" { Copy-Item -LiteralPath (Join-Path $pkgData $n) -Destination $data -Recurse }
	}
	Say ''
	Say 'Done. Start the game normally (Steam). In the field press Insert (keyboard) or L3+R3 (controller) to open the cheat menu.'
	Say 'To undo: .\fm2-install.ps1 -Action Uninstall'
	exit 0
}

if ($Action -eq 'Uninstall') {
	if (-not $installed) { Fail 'No manifest found - nothing installed by this tool.' }
	foreach ($n in $names) {
		$loose = Join-Path $data $n
		if (Test-Path $loose) { Run "remove loose Data\$n" { Remove-Item -LiteralPath $loose -Recurse -Force } }
	}
	foreach ($f in $manifest.files) {
		$arc = Join-Path $data $f.name; $b = Join-Path $bak $f.name
		if (-not (Test-Path $b)) { if (Test-Path $arc) { continue } else { Fail "backup of $($f.name) is missing and the game has no copy - run Steam 'Verify integrity of game files'." } }
		if (Test-Path $arc) {
			if ((Sha $arc) -eq $f.sha256) { Run "$($f.name) already back in Data (identical) - dropping the backup copy" { Remove-Item -LiteralPath $b } }
			else { Say "NOTE: $($f.name) in Data differs from the backup (game updated) - keeping the game's file; backup copy stays in _fm2_backup." }
		}
		else {
			if ((Sha $b) -ne $f.sha256 -and -not $Force) { Fail "backup of $($f.name) failed its checksum - not restoring (use -Force)." }
			Run "restore $($f.name)" { Move-Item -LiteralPath $b -Destination $arc }
		}
	}
	if (-not $DryRun) {
		Remove-Item -LiteralPath $manifestPath
		if (-not (Get-ChildItem -LiteralPath $bak -Force)) { Remove-Item -LiteralPath $bak }
	}
	Say ''
	Say 'Done. The game files are back to the original state.'
	exit 0
}
