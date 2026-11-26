"""Which normal string variables (3000000+n, SysDatabase type 4) are never referenced anywhere in the common events / map events?"""
import json, glob, re, collections, os
D = r"F:\Personal Projjects\FM2\work\wolftl\dump"
refs = collections.Counter(); uses = []
def walk(o, cb):
    if isinstance(o, dict):
        cb(o)
        for v in o.values(): walk(v, cb)
    elif isinstance(o, list):
        for v in o: walk(v, cb)
for f in glob.glob(D + r"\common\*.json") + glob.glob(D + r"\mps\*.json"):
    d = json.load(open(f, encoding="utf-8")); nm = os.path.basename(f)[:30]
    def cb(o):
        for x in o.get("intArgs", []) or []:
            if 3000000 <= (x & 0xFFFFFF) < 3000043: refs[x & 0xFFFFFF] += 1
        for s in o.get("stringArgs", []) or []:
            for m in re.finditer(r"\\s\[(\d+)\]", s): uses.append((nm, int(m.group(1)), s[:44]))
    walk(d, cb)
names = json.load(open(D + r"\db\SysDatabase.json", encoding="utf-8"))["types"][4]["data"]
print("string variable table size:", len(names))
print("text-code uses of \\s[n] (first 5):", uses[:5], "total", len(uses))
print("unreferenced AND unnamed:", [i for i in range(len(names)) if refs[3000000 + i] == 0 and not names[i]["name"].strip()])
print("unreferenced:", [(i, names[i]["name"]) for i in range(len(names)) if refs[3000000 + i] == 0])
