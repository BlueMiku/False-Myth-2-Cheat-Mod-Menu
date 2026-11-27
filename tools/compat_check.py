"""Compare two WolfTL dumps (stock Steam vs patched) for everything the cheat mod depends on.
usage: python -I -X utf8 tools/compat_check.py <stock_dump_dir> <patched_dump_dir> <mod CommonEvent.dat built from the patched data>
A dump dir contains common/, mps/ and db/ (WolfTL --create output)."""
import json, glob, os, re, subprocess, sys, collections, hashlib

STOCK, FULL, MOD = sys.argv[1], sys.argv[2], sys.argv[3]
FM2 = r"F:\Personal Projjects\FM2"
INC = FM2 + r"\tools\fm2mod"

def load_events(root):
    ev = {}
    for f in glob.glob(os.path.join(root, "common", "*.json")):
        i = int(os.path.basename(f).split("_")[0])
        ev[i] = f
    return ev

def jhash(path):
    return hashlib.sha256(open(path, "rb").read()).hexdigest()

def walk_ints(o, cb):
    if isinstance(o, dict):
        a = o.get("intArgs")
        if a: cb(o, a)
        for v in o.values(): walk_ints(v, cb)
    elif isinstance(o, list):
        for v in o: walk_ints(v, cb)

es, ef = load_events(STOCK), load_events(FULL)
print("common events: stock %d, patched %d" % (len(es), len(ef)))

# 1. events the mod patches or calls
used = {119: "zoom", 120: "walking (speed patch)", 122: "movement", 123: "periodic check", 124: "arrival routine", 197: "equipment pickup",
        529: "menu key", 530: "menu expand", 556: "world-map warp template", 566: "load processing", 582: "HUD", 584: "HUD clock", 500: "slot", 934: "slot"}
print("\n[1] events the mod calls/patches: identical in both?")
for i, why in sorted(used.items()):
    if i in es and i in ef:
        same = jhash(es[i]) == jhash(ef[i])
        print("  %4d %-26s %s" % (i, why, "same" if same else "DIFFERENT"))
    else:
        print("  %4d %-26s missing in %s" % (i, why, "stock" if i not in es else "patched"))

# 2. blank slots the mod uses
print("\n[2] reusable slots still blank in the patched data:")
bad = []
for s in [6, 7, 8, 9, 10, 11, 82, 500, 929, 933, 934, 935, 936, 937, 938]:
    d = json.load(open(ef[s], encoding="utf-8"))
    if len(d["commands"]) != 0 or os.path.basename(ef[s]).split("_", 1)[1].replace(".json", "").strip():
        bad.append(s)
print("  all blank" if not bad else "  NOT blank: %s" % bad)

# 3. variables the mod writes/reads: referenced differently in the patched data?
out = subprocess.run([FM2 + r"\tools\fm2mod\fm2mod.exe", "show-event", MOD, "0", "0"], capture_output=True, text=True, encoding="utf-8")
mine = set()
for eid in [6, 7, 8, 9, 500, 934, 935, 936, 937]:
    t = subprocess.run([FM2 + r"\tools\fm2mod\fm2mod.exe", "show-event", MOD, str(eid), "200000"], capture_output=True, text=True, encoding="utf-8").stdout
    for m in re.finditer(r"args=\[([^\]]*)\]", t):
        for x in m.group(1).split(","):
            x = x.strip()
            if x.isdigit():
                v = int(x)
                if 2000000 <= v < 3000000 or 3000000 <= v < 3000100: mine.add(v)

def refs(root):
    r = collections.defaultdict(set)
    def cb(o, a):
        for x in a:
            if isinstance(x, int) and x in mine: r[x].add(o.get("code"))
    for f in glob.glob(os.path.join(root, "common", "*.json")) + glob.glob(os.path.join(root, "mps", "*.json")):
        d = json.load(open(f, encoding="utf-8"))
        name = os.path.basename(f)
        def cb2(o, a, name=name):
            for x in a:
                if isinstance(x, int) and x in mine: r[x].add(name)
        walk_ints(d, cb2)
    return r
rs, rf = refs(STOCK), refs(FULL)
print("\n[3] variables used by the mod: %d; references that differ between stock and patched:" % len(mine))
diff = [v for v in sorted(mine) if rs.get(v, set()) != rf.get(v, set())]
for v in diff[:30]:
    print("  %d  stock %d files, patched %d files; new in patched: %s" % (v, len(rs.get(v, [])), len(rf.get(v, [])), sorted(rf.get(v, set()) - rs.get(v, set()))[:3]))
if not diff: print("  none")

# 4. databases
print("\n[4] databases the mod reads:")
a = json.load(open(os.path.join(STOCK, "db", "DataBase.json"), encoding="utf-8"))["types"]
b = json.load(open(os.path.join(FULL, "db", "DataBase.json"), encoding="utf-8"))["types"]
for t, nm in ((15, "equipment"), (33, "shop perks"), (10, "party members")):
    na = [x["name"] for x in a[t]["data"]]; nb = [x["name"] for x in b[t]["data"]]
    print("  UDB %d %-14s items %d/%d  name diffs: %d" % (t, nm, len(na), len(nb), sum(1 for p, q in zip(na, nb) if p != q)))
sa = json.load(open(os.path.join(STOCK, "db", "SysDatabase.json"), encoding="utf-8"))["types"]
sb = json.load(open(os.path.join(FULL, "db", "SysDatabase.json"), encoding="utf-8"))["types"]
d = [i for i, (x, y) in enumerate(zip(sa, sb)) if len(x["data"]) != len(y["data"]) or [z.get("name") for z in x["data"]] != [z.get("name") for z in y["data"]]]
print("  SysDatabase tables with different size/names: %s" % (d or "none"))
ca = json.load(open(os.path.join(STOCK, "db", "CDataBase.json"), encoding="utf-8"))["types"]
cb_ = json.load(open(os.path.join(FULL, "db", "CDataBase.json"), encoding="utf-8"))["types"]
print("  CDataBase identical: %s" % (json.dumps(ca, sort_keys=True) == json.dumps(cb_, sort_keys=True)))

# 5. warp targets still present as teleports in the patched data
rows = re.findall(r"\{(\d+),\s*(\d+),\s*(\d+),\s*(\d+),\s*L\"([^\"]*)\"\}", open(INC + r"\warp_targets.inc", encoding="utf-8").read())
tele = set()
def cbt(o, a):
    if len(a) >= 5:
        tele.add((a[3], a[1], a[2]))   # map id, x, y as in Teleport [target, x, y, map, options]
for f in glob.glob(os.path.join(FULL, "common", "*.json")) + glob.glob(os.path.join(FULL, "mps", "*.json")):
    walk_ints(json.load(open(f, encoding="utf-8")), cbt)
missing = [(r[4], r[0]) for r in rows if (int(r[0]), int(r[1]), int(r[2])) not in tele]
print("\n[5] warp targets: %d, still found as teleports in patched data: %d, missing: %s" % (len(rows), len(rows) - len(missing), missing[:10] or "none"))
