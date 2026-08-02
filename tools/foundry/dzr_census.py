"""Foundry F7: DZR placement census — machine-read a donor WW room arc and emit
the actor placement roster (per chunk type, per layer) as a markdown fact sheet.
P6's first ingredient. Run with the Foundry 3.12 venv python; wwrando clone
required at D:\\Decomps\\wwrando.

Usage: python dzr_census.py <Room.arc path> <output.md path>
"""
import sys, io
from collections import Counter

sys.path.insert(0, r"D:\XXXXXXX\wwrando")
sys.path.insert(0, r"D:\XXXXXXX\wwrando\wwlib")

from gclib.rarc import RARC
from gclib import fs_helpers as fs
from wwlib.dzx import DZx

def load_rarc(path):
    with open(path, "rb") as f:
        data = f.read()
    if data[:4] == b"Yaz0":
        from gclib.yaz0_yay0 import Yaz0
        data = Yaz0.decompress(io.BytesIO(data)).read()
    return RARC(io.BytesIO(data))

def find_dzx_files(rarc):
    out = []
    for fe in rarc.file_entries:
        name = getattr(fe, "name", "")
        if name.endswith(".dzr") or name.endswith(".dzs"):
            out.append(fe)
    return out

def entry_summary(e):
    name = getattr(e, "name", "?")
    pos = ""
    for ax in ("x_pos", "y_pos", "z_pos"):
        v = getattr(e, ax, None)
        if v is not None:
            pos += f"{v:.0f} "
    params = getattr(e, "params", None)
    ptxt = f"{params:08X}" if isinstance(params, int) else ""
    return name, pos.strip(), ptxt

def main():
    arc_path, out_path = sys.argv[1], sys.argv[2]
    rarc = load_rarc(arc_path)
    dzx_entries = find_dzx_files(rarc)
    if not dzx_entries:
        print("no .dzr/.dzs in arc; files:",
              [getattr(fe, "name", "?") for fe in rarc.file_entries][:40])
        sys.exit(1)

    lines = [f"# DZR census — `{arc_path}`", ""]
    for fe in dzx_entries:
        dzx = DZx(fe)
        lines.append(f"## File `{fe.name}`")
        for chunk in dzx.chunks:
            ctype = chunk.chunk_type.__name__
            layer = getattr(chunk, "layer", None)
            ltxt = layer.name if layer is not None else "?"
            entries = chunk.entries
            lines.append(f"\n### {ctype} (layer {ltxt}) — {len(entries)} entries")
            if not entries:
                continue
            counts = Counter(getattr(e, "name", ctype) for e in entries)
            lines.append("| name | count |")
            lines.append("|---|---|")
            for n, c in counts.most_common():
                lines.append(f"| `{n}` | {c} |")
            if ctype in ("ACTR", "ACT0", "ACT1", "ACT2", "ACT3", "SCOB", "TRES",
                         "PLYR", "SHIP", "DOOR", "TGOB", "TGSC"):
                lines.append("\n<details><summary>full entries</summary>\n")
                lines.append("| name | pos | params |")
                lines.append("|---|---|---|")
                for e in entries:
                    n, pos, p = entry_summary(e)
                    lines.append(f"| `{n}` | {pos} | {p} |")
                lines.append("\n</details>")
    with open(out_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    print(f"wrote {out_path}: {sum(1 for l in lines if l.startswith('###'))} chunks")

if __name__ == "__main__":
    main()
