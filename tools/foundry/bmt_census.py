"""Foundry §236: BMT structure census — the pig-shade gap class, mechanized.

History's root cause: material-only BMTs (no TEX1 chunk) crash any code path
that swaps texture tables unconditionally (replaceTextures → empty table →
out-of-range → AV in makeSharedDL). This census inspects every .bmt inside the
given arcs OFFLINE (byte-level chunk presence — the same check History ran by
hand with gclib) and flags material-only ones, so every actor kit knows its
exposure BEFORE the crash.

Usage: bmt_census.py <arcs-dir-or-arc> <out.md>
"""
import sys, os, io

sys.path.insert(0, r"D:\XXXXXXX\wwrando")
from gclib.rarc import RARC
from gclib.yaz0_yay0 import Yaz0

def arc_bmts(path):
    data = open(path, "rb").read()
    if data[:4] == b"Yaz0":
        data = Yaz0.decompress(io.BytesIO(data)).read()
    try:
        rarc = RARC(io.BytesIO(data))
    except Exception as ex:
        return [("<arc unparseable: %s>" % ex.__class__.__name__, None, None, None)]
    out = []
    for fe in rarc.file_entries:
        name = getattr(fe, "name", "")
        if not name.lower().endswith(".bmt"):
            continue
        try:
            raw = fe.data.getvalue() if hasattr(fe.data, "getvalue") else bytes(fe.data)
        except Exception:
            raw = b""
        out.append((name, b"TEX1" in raw, b"MAT3" in raw, len(raw)))
    return out

def main():
    target, out_md = sys.argv[1], sys.argv[2]
    arcs = [target] if target.lower().endswith(".arc") else \
        [os.path.join(target, f) for f in sorted(os.listdir(target))
         if f.lower().endswith(".arc")]
    lines = ["# BMT structure census (§236 — the pig-shade gap class)", "",
             "| arc | bmt | TEX1? | MAT3? | bytes | exposure |",
             "|---|---|---|---|---|---|"]
    n_matonly = n_total = 0
    for arc in arcs:
        for name, tex1, mat3, size in arc_bmts(arc):
            if tex1 is None:
                lines.append(f"| `{os.path.basename(arc)}` | {name} | — | — | — | — |")
                continue
            n_total += 1
            expo = ""
            if not tex1:
                n_matonly += 1
                expo = ("**MATERIAL-ONLY — any unconditional texture-table swap "
                        "AVs (§236 law: guard replaceTextures on TEX1)**")
            lines.append(f"| `{os.path.basename(arc)}` | `{name}` | "
                         f"{'YES' if tex1 else 'NO'} | {'YES' if mat3 else 'NO'} | "
                         f"{size} | {expo} |")
    lines += ["", f"**{n_matonly} of {n_total} BMTs are MATERIAL-ONLY.** "
              "Donor semantics for these = J3DMatCopyFlag_Material (copy "
              "materials, keep the model's own textures)."]
    open(out_md, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    print(f"{n_matonly}/{n_total} material-only; wrote {out_md}")

if __name__ == "__main__":
    main()
