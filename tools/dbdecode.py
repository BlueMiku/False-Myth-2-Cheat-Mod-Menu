"""Readable listing of an event's Database (250) commands, plus names of CDB/UserDB types and fields.
usage: python tools/dbdecode.py <eventId> [maxlines]"""
import json, glob, sys
D = r"F:\Personal Projjects\FM2\work\wolftl\dump"
cdb = json.load(open(D + r"\db\CDataBase.json", encoding="utf-8"))["types"]
udb = json.load(open(D + r"\db\DataBase.json", encoding="utf-8"))["types"]
sdb = json.load(open(D + r"\db\SysDatabase.json", encoding="utf-8"))["types"]
KIND = {0: ("CDB", cdb), 1: ("SYS", sdb), 2: ("UDB", udb)}
OPS = {0: "=", 1: "+=", 2: "-=", 3: "*=", 4: "/=", 5: "%=", 6: "max=", 7: "min="}

def isvar(x): return 1000000 <= x < 4294967000
def s(x): return ("var%d" % x) if isvar(x) else ("-1" if x == 4294967295 else str(x))

def tname(kind, t):
    tbl = KIND[kind][1]
    return tbl[t]["name"] if 0 <= t < len(tbl) else "?"
def fname(kind, t, f):
    tbl = KIND[kind][1]
    try: return tbl[t]["fields"][f]["name"]
    except Exception: return "?"

def decode(c):
    t, dta, fld, pk, val = c["intArgs"][:5]
    asg = pk & 0xFF; opt = (pk >> 8) & 0xFF; unk = (pk >> 16) & 0xFFFF
    kind = opt & 15; op = opt >> 4
    tn = tname(kind, t) if not isvar(t) else "?"
    fn = fname(kind, t, fld) if not isvar(t) and not isvar(fld) else "?"
    mode = "READ " if op == 1 else "WRITE"
    return f"{mode} {KIND.get(kind, ('?',))[0]}[{s(t)}:{tn}] item={s(dta)} field={s(fld)}:{fn}  {'->' if op == 1 else OPS.get(asg >> 4, '?')+' '} {s(val)}   (unk={unk})"

if __name__ == "__main__":
    eid = int(sys.argv[1]); mx = int(sys.argv[2]) if len(sys.argv) > 2 else 60
    d = json.load(open(glob.glob(D + rf"\common\{eid}_*.json")[0], encoding="utf-8"))
    print("=====", eid, d["name"], len(d["commands"]), "cmds")
    n = 0
    for c in d["commands"]:
        if c["codeStr"] == "Database":
            print(" ", c["index"], decode(c)); n += 1
            if n >= mx: break
