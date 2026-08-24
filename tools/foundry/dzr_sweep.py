"""Foundry P6: full-game DZR/DZS fact-sheet sweep.
Walks every stage folder under the donor Stage root, parses each Room*.arc +
Stage.arc, and emits per-stage fact sheets:
  <out>/<stage>.md        — human-readable counts per file/chunk/layer
  <out>/json/<stage>.json — machine-readable full entries (differ fuel, P2)
  <out>/index.md          — one-line-per-stage overview + grand totals
Run with the Foundry 3.12 venv python (needs wwrando @ D:\\Decomps\\wwrando).
"""
import sys, io, os, json, traceback
from collections import Counter

sys.path.insert(0, r"<decomp-root>\wwrando")
sys.path.insert(0, r"<decomp-root>\wwrando\wwlib")

from gclib.rarc import RARC
from wwlib.dzx import DZx

STAGE_ROOT = r"<decomp-root>\Ex WW\files\res\Stage"
ACTOR_CHUNKS = {"ACTR", "ACT0", "ACT1", "ACT2", "ACT3", "SCOB", "TRES", "PLYR",
                "SHIP", "DOOR", "TGOB", "TGSC", "TGDR", "TRES", "SCLS",
                "RPAT", "RPPN", "PATH", "PPNT", "Pale", "Virt", "EnvR", "Colo"}
LIGHTING_CHUNKS = {"Pale", "Virt", "EnvR", "Colo"}

def load_rarc(path):
    with open(path, "rb") as f:
        data = f.read()
    if data[:4] == b"Yaz0":
        from gclib.yaz0_yay0 import Yaz0
        data = Yaz0.decompress(io.BytesIO(data)).read()
    return RARC(io.BytesIO(data))

def entry_dict(e, want_detail):
    d = {"name": getattr(e, "name", None)}
    if not want_detail:
        return d
    for ax in ("x_pos", "y_pos", "z_pos"):
        v = getattr(e, ax, None)
        if v is not None:
            d[ax[0]] = round(float(v), 1)
    p = getattr(e, "params", None)
    if isinstance(p, int):
        d["params"] = f"{p:08X}"
    for rot in ("x_rot", "y_rot", "z_rot"):
        v = getattr(e, rot, None)
        if isinstance(v, int) and v != 0:
            d[rot] = v
    # SCLS exit entries (P11 transition graph fuel)
    for f in ("dest_stage_name", "spawn_id", "room_index", "fade_type"):
        v = getattr(e, f, None)
        if v is not None:
            d[f] = v
    # RPAT path headers + RPPN waypoints (P12 path-parity fuel)
    for f in ("num_points", "next_path_index", "is_loop", "first_waypoint_offset",
              "action_type"):
        v = getattr(e, f, None)
        if v is not None:
            d[f] = v
    # EnvR/Colo lighting SELECTORS (P14): wwlib leaves these raw; decoded here
    # verbatim per decomp d_stage.h — stage_envr_info_class {u8 pselect_id[8]}
    # (L162-164) and stage_pselect_info_class {u8 palette_id[8]; f32 change_rate}
    # (L103-106), big-endian.
    raw = getattr(e, "raw_data_bytes", None)
    if raw is not None:
        import struct as _st
        tn = type(e).__name__
        if tn == "EnvR" and len(raw) >= 8:
            d["pselect_id"] = list(raw[0:8])
        elif tn == "Colo" and len(raw) >= 0xC:
            d["palette_id"] = list(raw[0:8])
            d["change_rate"] = _st.unpack_from(">f", raw, 8)[0]
        else:
            d["raw"] = raw.hex()
    # Pale/Virt lighting palettes (P14 fuel): dump every bunfoe field as text
    if type(e).__name__ in ("Pale", "Virt"):
        import dataclasses
        try:
            for fld in dataclasses.fields(e):
                if fld.name in ("data", "magic", "size", "_padding"):
                    continue
                v = getattr(e, fld.name, None)
                if v is not None:
                    d[fld.name] = str(v)
        except TypeError:
            d["raw_fields"] = "[INFERENCE-NEEDED: bunfoe fields not introspectable]"
    return d

def census_arc(arc_path):
    """Returns list of {file, chunk, layer, count, names:{}, entries:[]}."""
    rarc = load_rarc(arc_path)
    out = []
    for fe in rarc.file_entries:
        name = getattr(fe, "name", "")
        if not (name.endswith(".dzr") or name.endswith(".dzs")):
            continue
        dzx = DZx(fe)
        for chunk in dzx.chunks:
            ctype = chunk.chunk_type.__name__
            layer = getattr(chunk, "layer", None)
            ltxt = layer.name if layer is not None else "?"
            detail = ctype in ACTOR_CHUNKS
            entries = [entry_dict(e, detail) for e in chunk.entries]
            out.append({
                "file": name, "chunk": ctype, "layer": ltxt,
                "count": len(entries),
                "names": dict(Counter(d["name"] or ctype for d in entries)),
                "entries": entries if detail else [],
            })
    return out

def main():
    out_dir = sys.argv[1] if len(sys.argv) > 1 else \
        r"%USERPROFILE%\Documents\dusklight\docs\WW Linked\fact-sheets"
    os.makedirs(os.path.join(out_dir, "json"), exist_ok=True)

    index_rows, failures = [], []
    stages = sorted(os.listdir(STAGE_ROOT))
    for si, stage in enumerate(stages):
        sdir = os.path.join(STAGE_ROOT, stage)
        if not os.path.isdir(sdir):
            continue
        arcs = sorted(a for a in os.listdir(sdir) if a.endswith(".arc"))
        stage_data, md = {}, [f"# Fact sheet — stage `{stage}`", ""]
        actr_default = actr_layered = 0
        names_seen = Counter()
        for arc in arcs:
            apath = os.path.join(sdir, arc)
            try:
                rows = census_arc(apath)
            except Exception as ex:
                failures.append(f"{stage}/{arc}: {ex.__class__.__name__}: {ex}")
                continue
            stage_data[arc] = rows
            md.append(f"\n## `{arc}`\n")
            md.append("| file | chunk | layer | count | top names |")
            md.append("|---|---|---|---|---|")
            for r in rows:
                top = ", ".join(f"`{n}`×{c}" for n, c in
                                Counter(r["names"]).most_common(6))
                md.append(f"| {r['file']} | {r['chunk']} | {r['layer']} | "
                          f"{r['count']} | {top} |")
                if r["chunk"] == "ACTR":
                    if r["layer"] == "Default":
                        actr_default += r["count"]
                    else:
                        actr_layered += r["count"]
                if r["chunk"] in ACTOR_CHUNKS:
                    names_seen.update(r["names"])
        with open(os.path.join(out_dir, f"{stage}.md"), "w", encoding="utf-8") as f:
            f.write("\n".join(md) + "\n")
        with open(os.path.join(out_dir, "json", f"{stage}.json"), "w",
                  encoding="utf-8") as f:
            json.dump(stage_data, f, separators=(",", ":"))
        index_rows.append((stage, len(arcs), actr_default, actr_layered,
                           len(names_seen)))
        if si % 10 == 0:
            print(f"[{si}/{len(stages)}] {stage}", flush=True)

    idx = ["# WW donor fact-sheet index (P6 sweep)", "",
           "| stage | arcs | ACTR default | ACTR layered | distinct actor names |",
           "|---|---|---|---|---|"]
    for s, a, ad, al, dn in index_rows:
        idx.append(f"| [`{s}`]({s}.md) | {a} | {ad} | {al} | {dn} |")
    idx.append(f"\nTotals: {len(index_rows)} stages, "
               f"{sum(r[1] for r in index_rows)} arcs, "
               f"ACTR default {sum(r[2] for r in index_rows)}, "
               f"layered {sum(r[3] for r in index_rows)}.")
    if failures:
        idx.append(f"\n## Parse failures ({len(failures)})\n")
        idx += [f"- `{f}`" for f in failures]
    with open(os.path.join(out_dir, "index.md"), "w", encoding="utf-8") as f:
        f.write("\n".join(idx) + "\n")
    print(f"DONE: {len(index_rows)} stages, {len(failures)} failures")

if __name__ == "__main__":
    main()
