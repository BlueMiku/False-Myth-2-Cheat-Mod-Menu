# Builds fm2mod.exe (static, MinGW). Requires work\wolftl-src (git clone https://github.com/Sinflower/WolfTL).
$fm = "F:\Personal Projjects\FM2"
$src = "$fm\work\wolftl-src\WolfTL"
$out = "$fm\tools\fm2mod\fm2mod.exe"
$obj = "$fm\work\lz4.o"
Remove-Item $out -ErrorAction SilentlyContinue   # never run a stale tool after a failed compile
if (-not (Test-Path $obj)) { gcc -O2 -c "$src\3rdParty\lz4\lz4.c" -o $obj }
g++ -std=c++20 -O2 -DUNICODE -D_UNICODE -DNOMINMAX -static -I"$src\3rdParty" -I"$src" "$fm\tools\fm2mod\fm2mod.cpp" $obj -o $out
if ($LASTEXITCODE -eq 0) { "built $out" }
