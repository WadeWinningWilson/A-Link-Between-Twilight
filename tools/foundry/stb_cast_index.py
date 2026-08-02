"""Foundry §251 (kit v4 cutscene tier): the STB CAST INDEX — which actors
appear in which storyboard (.stb) casts, swept from the donor's own arcs.

Why: History's §247 Aryll saga — a cutscene-participating NPC is a different
porting tier (demo shims are LOAD-BEARING, not inert). The gate question is
"is this actor in any .stb cast?", answered here donor-authoritatively:
  every res/**.arc (RARC, Yaz0-aware) -> every *.stb entry -> byte-search for
  each l_objectName actor name (null-terminated, non-alnum-preceded).

Outputs: docs/WW Linked/stb-cast-index.md + fact-sheets/json/stb-cast.json
(actor name -> ["<arc-rel>/<stb-name>", ...]) for island_roster/enemy_port_kit.

Run with D:\\Decomps\\foundry-py312\\Scripts\\python.exe (gclib venv).
"""
import io, os, re, json, sys

FILES = r"D:\XXXXXXX\Ex WW\files"
OUT_MD = r"C:\Users\xxxxx\Documents\dusklight\docs\WW Linked\stb-cast-index.md"
OUT_JSON = (r"C:\Users\xxxxx\Documents\dusklight\docs\WW Linked\fact-sheets"
            r"\json\stb-cast.json")
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from island_roster import objname_table

def iter_stbs():
    from gclib.rarc import RARC
    from gclib.yaz0_yay0 import Yaz0
    for dp, _d, fs in os.walk(os.path.join(FILES, "res")):
        for f in fs:
            if not f.lower().endswith(".arc"):
                continue
            p = os.path.join(dp, f)
            data = open(p, "rb").read()
            if data[:4] == b"Yaz0":
                data = Yaz0.decompress(io.BytesIO(data)).read()
            if data[:4] != b"RARC":
                continue
            try:
                rarc = RARC(io.BytesIO(data))
            except Exception as e:
                print(f"[skip] {p}: {e}")
                continue
            for fe in rarc.file_entries:
                name = getattr(fe, "name", "")
                if name.lower().endswith(".stb"):
                    raw = fe.data
                    raw = raw.read() if hasattr(raw, "read") else bytes(raw)
                    if hasattr(fe.data, "seek"):
                        fe.data.seek(0)
                    yield os.path.relpath(p, FILES), name, raw

def main():
    names = sorted(objname_table(), key=len, reverse=True)
    pats = {nm: re.compile(re.escape(nm.encode()) + b"\x00") for nm in names}
    cast = {}      # actor -> set of "arc :: stb"
    stbs = 0
    for arc_rel, stb_name, raw in iter_stbs():
        stbs += 1
        where = f"{arc_rel} :: {stb_name}"
        for nm, pat in pats.items():
            for m in pat.finditer(raw):
                prev = raw[m.start() - 1: m.start()]
                # cast names are standalone C strings: reject mid-identifier hits
                if prev and (prev.isalnum() or prev == b"_"):
                    continue
                cast.setdefault(nm, set()).add(where)
                break
    os.makedirs(os.path.dirname(OUT_JSON), exist_ok=True)
    json.dump({k: sorted(v) for k, v in sorted(cast.items())},
              open(OUT_JSON, "w", encoding="utf-8"), indent=1)
    lines = ["# STB cast index — which actors appear in storyboard casts "
             "(Foundry §251, kit v4 cutscene tier)", "",
             "> Donor-authoritative sweep of every `.stb` in every donor arc.",
             "> If an actor is listed here, it is a CUTSCENE actor: demo-path",
             "> shims are LOAD-BEARING (§247) — apply cookbook recipes 10-15,",
             "> never green it on a NULL demo shim. If absent, demo shims are",
             f"> inert-safe. Swept {stbs} .stb files.", "",
             "| actor (l_objectName) | .stb casts |", "|---|---|"]
    for nm in sorted(cast):
        lines.append(f"| `{nm}` | " + "<br>".join(
            f"`{w}`" for w in sorted(cast[nm])) + " |")
    open(OUT_MD, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    print(f"wrote {OUT_MD}: {len(cast)} cast actors across {stbs} stb files")

if __name__ == "__main__":
    main()
