"""Offline sanity check of the generated mod events: balanced if/else blocks, indent rules, terminator, and that every variable id exists.
usage: python tools/validate_events.py <CommonEvent.dat> <id> [<id> ...]"""
import re, subprocess, sys
FM2 = r"F:\Personal Projjects\FM2"
SIZES = {0: 83, 1: 70, 2: 97, 3: 30, 4: 20, 5: 1, 6: 10, 7: 1, 8: 10, 9: 45}   # SysDatabase variable table sizes
dat = sys.argv[1]
for eid in map(int, sys.argv[2:]):
    out = subprocess.run([FM2 + r"\tools\fm2mod\fm2mod.exe", "show-event", dat, str(eid), "100000"], capture_output=True, text=True, encoding="utf-8").stdout.splitlines()
    cmds = []
    for l in out[1:]:
        m = re.match(r"(\d+) code=(\d+) indent=(\d+) args=\[(.*?)\]", l)
        if m: cmds.append((int(m.group(2)), int(m.group(3)), [int(x) for x in m.group(4).split(",") if x]))
    n111 = sum(c[0] == 111 for c in cmds); n499 = sum(c[0] == 499 for c in cmds)
    ok401 = all(cmds[i-1][0] == 111 and cmds[i-1][1] == cmds[i][1] for i, c in enumerate(cmds) if c[0] == 401)
    endok = cmds[-1][0] == 0 and cmds[-1][1] == 0
    bad = [i for i in range(1, len(cmds)) if cmds[i][1] > cmds[i-1][1] and cmds[i-1][0] not in (401, 420)]
    ids = set()
    for code, ind, a in cmds:
        if code == 121: ids.add(a[0]); ids.add(a[1]) if a[1] >= 2000000 else None
        elif code in (123, 111): ids.add(a[0] if code == 123 else a[1])
        elif code == 111 and len(a) > 2 and a[2] >= 2000000: ids.add(a[2])
    ids = {v for v in ids if 2000000 <= v < 3000000}
    badv = sorted(v for v in ids if (v - 2000000) % 100000 >= SIZES[(v - 2000000) // 100000])
    print(f"event {eid}: {out[0].split('name=')[1] if out else '?'} cmds={len(cmds)} if/end={n111}/{n499} 401ok={ok401} ends_blank0={endok} bad_indent_rises={len(bad)} vars={len(ids)} UNALLOCATED={badv}")
