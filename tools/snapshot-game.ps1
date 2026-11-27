# Read-only snapshot of a game folder: SHA256 + size + timestamp of every file (saves excluded: only names/sizes), plus mod state.
#   .\tools\snapshot-game.ps1 -Name A_current            -> work\snapshots\A_current.json
#   .\tools\snapshot-game.ps1 -Diff A_current B_after    -> prints added / removed / changed files between two snapshots
param([string]$Name, [string[]]$Diff, [string]$GameDir = 'F:\SteamLibrary\steamapps\common\FalseMyth2')
$ErrorActionPreference = 'Stop'
$snap = 'F:\Personal Projjects\FM2\work\snapshots'
New-Item -ItemType Directory -Force $snap | Out-Null
if ($Diff) {
	$a = Get-Content "$snap\$($Diff[0]).json" -Raw | ConvertFrom-Json; $b = Get-Content "$snap\$($Diff[1]).json" -Raw | ConvertFrom-Json
	$ma = @{}; foreach ($f in $a.files) { $ma[$f.p] = $f }; $mb = @{}; foreach ($f in $b.files) { $mb[$f.p] = $f }
	"== only in $($Diff[1]) (added):";   $mb.Keys | Where-Object { -not $ma.ContainsKey($_) } | Sort-Object | ForEach-Object { "  + $_  ($($mb[$_].s) bytes)" }
	"== only in $($Diff[0]) (removed):"; $ma.Keys | Where-Object { -not $mb.ContainsKey($_) } | Sort-Object | ForEach-Object { "  - $_  ($($ma[$_].s) bytes)" }
	"== changed:"; $ma.Keys | Where-Object { $mb.ContainsKey($_) -and ($ma[$_].h -ne $mb[$_].h -or $ma[$_].s -ne $mb[$_].s) } | Sort-Object | ForEach-Object { "  ~ $_  ($($ma[$_].s) -> $($mb[$_].s) bytes)" }
	return
}
if (-not $Name) { throw 'give -Name <snapshot name> or -Diff <a> <b>' }
$files = foreach ($f in Get-ChildItem -LiteralPath $GameDir -Recurse -File -Force) {
	$rel = $f.FullName.Substring($GameDir.Length)
	$isSave = $rel -match '^\\Save(_MTool)?\\'
	[pscustomobject]@{ p = $rel; s = $f.Length; t = $f.LastWriteTime.ToString('s'); h = $(if ($isSave) { '' } else { (Get-FileHash -Algorithm SHA256 -LiteralPath $f.FullName).Hash }) }
}
[pscustomobject]@{ name = $Name; at = (Get-Date).ToString('s'); gameDir = $GameDir; files = @($files) } | ConvertTo-Json -Depth 4 | Set-Content "$snap\$Name.json" -Encoding UTF8
"snapshot $Name : $(@($files).Count) files -> $snap\$Name.json"
