"""Foundry §237: BMT material-color extractor — the donor's authored per-variant
colors, offline (gclib MAT3). Settles shade disputes at the data tier before
anyone blames lighting.

Usage: bmt_colors.py <arc> <out.md> [name-filter]
"""
import sys, os, io

sys.path.insert(0, r"D:\XXXXXXX\wwrando")
from gclib.rarc import RARC
from gclib.j3d import J3D

def mat_colors(raw, label, lines):
    j = J3D(io.BytesIO(raw))
    mat3 = getattr(j, "mat3", None)
    if mat3 is None:
        lines.append(f"\n### `{label}` — no MAT3 parsed")
        return
    mats = getattr(mat3, "materials", None) or []
    names = getattr(mat3, "mat_names", None)
    lines.append(f"\n### `{label}` — {len(mats)} materials")
    lines.append("| # | material | tev/material colors |")
    lines.append("|---|---|---|")
    for i, m in enumerate(mats):
        nm = ""
        try:
            nm = names[i] if names else getattr(m, "name", "")
        except Exception:
            pass
        cols = []
        for attr in ("tev_colors", "tev_konst_colors", "material_colors",
                     "color_channels"):
            v = getattr(m, attr, None)
            if v:
                try:
                    cols.append(f"{attr}={[str(c) for c in v][:4]}")
                except Exception:
                    pass
        lines.append(f"| {i} | `{nm}` | {' · '.join(cols) if cols else '(introspect: ' + ', '.join(a for a in dir(m) if 'col' in a.lower())[:120] + ')'} |")

def main():
    arc_path, out_md = sys.argv[1], sys.argv[2]
    filt = sys.argv[3].lower() if len(sys.argv) > 3 else ""
    data = open(arc_path, "rb").read()
    rarc = RARC(io.BytesIO(data))
    lines = [f"# Donor material colors — `{os.path.basename(arc_path)}`"]
    for fe in rarc.file_entries:
        nm = getattr(fe, "name", "")
        if not nm.lower().endswith((".bmt", ".bdl", ".bmd")):
            continue
        if filt and filt not in nm.lower():
            continue
        raw = fe.data.getvalue() if hasattr(fe.data, "getvalue") else bytes(fe.data)
        try:
            mat_colors(raw, nm, lines)
        except Exception as ex:
            lines.append(f"\n### `{nm}` — parse failed: {ex.__class__.__name__}: {ex}")
    open(out_md, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    print(f"wrote {out_md}")

if __name__ == "__main__":
    main()
