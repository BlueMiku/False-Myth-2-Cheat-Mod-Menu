"""Checks every warp target's entry coordinates against the map size (tile units vs half-tile 'precise' units)."""
import json, subprocess, re, os
FM2 = r"F:\Personal Projjects\FM2"
rows = json.load(open(FM2 + r"\work\wolftl\warp_candidates.json", encoding="utf-8"))
byid = {r[0]: r for r in rows}
targets = []
for l in open(FM2 + r"\tools\fm2mod\warp_targets.inc", encoding="utf-8").read().splitlines():
    m = re.match(r'\{(\d+), (\d+), (\d+), (\d+), L"(.*)"\},', l)
    if m: targets.append(tuple(int(m.group(i)) for i in range(1, 5)) + (m.group(5),))
print("id   name                              size      entry(x,y) opt   tiles  half-tiles")
for (i, x, y, o, name) in targets:
    f = byid[i][3] if i in byid else ""
    f = f if f.startswith("MapData") else "MapData/" + f
    path = os.path.join(FM2, "extracted", "Data", f.replace("/", os.sep))
    out = subprocess.run([FM2 + r"\tools\fm2mod\fm2mod.exe", "map-info", path], capture_output=True, text=True, encoding="utf-8").stdout.splitlines()
    m = re.match(r"size (\d+)x(\d+)", out[0]) if out else None
    if not m:
        print(i, name, "?? no size", path); continue
    w, h = int(m.group(1)), int(m.group(2))
    print(f"{i:3d}  {name:32s} {w:3d}x{h:<3d}   ({x:2d},{y:2d}) {o}     {str(x < w and y < h):5s}  {str(x / 2 < w and y / 2 < h)}")
