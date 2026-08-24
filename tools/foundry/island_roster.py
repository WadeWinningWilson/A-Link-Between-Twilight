"""Foundry §239 (kit v4, piece 1): the ISLAND ROSTER — placement-grounded cast
list joining three donor truths, so no instance ever ports the wrong island's
NPCs again:
  fact-sheet placements (WHO is on this island, per layer)
  × donor l_objectName table (ACTR name → proc; d_stage.cpp:437)
  × donor g_profile definitions (proc → SOURCE FILE)

Usage: island_roster.py <stage> <arc(s) comma-sep or 'all'> <out.md>
       island_roster.py --island <stage> <exterior-arc> <out.md>
The --island mode (§240) follows the exterior room's SCLS exits to every
INTERIOR stage (donor-authored island→interiors mapping), recursively, and
emits one combined roster: exterior + each interior stage.
e.g.   island_roster.py --island sea Room44.arc rosters/outset-island.md
"""
import sys, os, re, json
from collections import Counter, defaultdict

FS_JSON = r"%USERPROFILE%\Documents\dusklight\docs\WW Linked\fact-sheets\json"
STB_CAST = os.path.join(FS_JSON, "stb-cast.json")
DONOR_SRC = r"<decomp-root>\WW DP\src\d"
DSTAGE = os.path.join(DONOR_SRC, "d_stage.cpp")

def objname_table():
    text = open(DSTAGE, encoding="utf-8", errors="replace").read()
    return {m.group(1): m.group(2) for m in
            re.finditer(r'OBJNAME\("([^"]+)",\s*(\w+)', text)}

def profile_files():
    """fpcNm_X_e -> source file, via g_profile_X definitions across donor src."""
    out = {}
    for dirpath, _d, files in os.walk(DONOR_SRC):
        for fn in files:
            if not fn.endswith((".cpp", ".inc")):
                continue
            p = os.path.join(dirpath, fn)
            try:
                t = open(p, encoding="utf-8", errors="replace").read()
            except OSError:
                continue
            for m in re.finditer(r"fopAc_prof\w*\s+g_profile_(\w+)\s*=|"
                                 r"actor_process_profile_definition\s+g_profile_(\w+)\s*=", t):
                name = m.group(1) or m.group(2)
                out.setdefault(name, os.path.relpath(p, DONOR_SRC))
    return out

_BASE_CACHE = {}
DONOR_INC_ACTOR = r"<decomp-root>\WW DP\include\d\actor"

def base_tier(src_rel):
    """§241+§242: TWO gates per actor.
    Gate 1 (decomp): un-decompiled stubs (>=20 Nonmatching bodies) have NO
      source to port AND their scaffold header defaults to fopAc_ac_c, so the
      base class read is UNRELIABLE (History's §242 anti-correlation finding —
      ko1 scaffolds fopAc but is really fopNpc). Stub verdict overrides gate 2.
    Gate 2 (base class): fopAc/fopEn exist in the port; fopNpc_npc_c requires
      the d_npc framework port (GREENLIT by user ruling, §242)."""
    if src_rel in _BASE_CACHE:
        return _BASE_CACHE[src_rel]
    cpp = os.path.join(DONOR_SRC, src_rel)
    nm = 0
    if os.path.exists(cpp):
        nm = open(cpp, encoding="utf-8",
                  errors="replace").read().count("Nonmatching")
    if nm >= 20:
        tier = (f"**STUB — un-decompiled donor ({nm}× Nonmatching): no source "
                f"to port; header base unreliable (§242)**")
        _BASE_CACHE[src_rel] = tier
        return tier
    hdr = os.path.join(DONOR_INC_ACTOR,
                       os.path.basename(src_rel).replace(".cpp", ".h"))
    tier = "?"
    if os.path.exists(hdr):
        t = open(hdr, encoding="utf-8", errors="replace").read()
        # Gate 3 (§242 ji1 lesson): dNpc_* usage in header OR cpp means the
        # actor needs the d_npc framework EVEN IF its base class is fopAc_ac_c
        # (ji1/Sturgeon embeds dNpc_EventCut_c/JntCtrl_c/HeadAnm_c members).
        cpp_t = (open(cpp, encoding="utf-8", errors="replace").read()
                 if os.path.exists(cpp) else "")
        uses_dnpc = bool(re.search(r"\bdNpc_\w+", t + cpp_t))
        bases = re.findall(r"(?:class|struct)\s+\w+\s*:\s*public\s+(\w+)", t)
        base = next((b for b in bases if b.startswith("fop")), None)
        if base is None and re.search(r"struct\s+\w+_class\b", t):
            # C-style actor: fopAc/fopEn embedded as first MEMBER, not a base
            m = re.search(r"struct\s+\w+_class\s*\{[^;{]*?(fop\w+_c)\s+\w+;",
                          t, re.S)
            base = (m.group(1) + " (embedded)") if m else None
        if base:
            root = base.split(" ")[0]
            if root == "fopNpc_npc_c":
                tier = (f"{base} — portable AFTER d_npc framework "
                        f"(GREENLIT §242)")
            elif root in ("fopAc_ac_c", "fopEn_enemy_c"):
                if uses_dnpc:
                    tier = (f"{base} but USES dNpc_* members — needs d_npc "
                            f"framework (§242 ji1 lesson)")
                else:
                    tier = f"{base} — portable"
            else:
                tier = f"{base} — [check base's own base]"
            tier += f" · decompiled ({nm} NM)"
        elif bases:
            tier = f"[no fop* base found; classes derive: {', '.join(bases[:3])}]"
    _BASE_CACHE[src_rel] = tier
    return tier

def collect(stage, arcs, names, profs):
    """Roster rows + SCLS dest stages for the given stage/arcs."""
    data = json.load(open(os.path.join(FS_JSON, stage + ".json"),
                          encoding="utf-8"))
    if arcs == "all":
        arcs = list(data)
    roster = defaultdict(lambda: defaultdict(int))
    dests = set()
    for arc in arcs:
        for r in data.get(arc, []):
            if r["chunk"] == "SCLS":
                for e in r["entries"]:
                    d = str(e.get("dest_stage_name") or "").strip("\x00 ")
                    if d:
                        dests.add(d)
            if r["chunk"] not in ("ACTR", "SCOB", "TGOB", "TGSC", "TGDR", "DOOR"):
                continue
            for nm, c in r["names"].items():
                roster[nm][r["layer"]] += c
    stb = (json.load(open(STB_CAST, encoding="utf-8"))
           if os.path.exists(STB_CAST) else {})
    rows, unmapped = [], 0
    for nm in sorted(roster):
        placed = " ".join(f"{l}:{c}" for l, c in sorted(roster[nm].items()))
        proc = names.get(nm)
        tier = ""
        if proc:
            key = re.sub(r"^fpcNm_|_e$", "", proc)
            src = profs.get(key, "[INFERENCE-NEEDED: g_profile not located]")
            if src.endswith(".cpp"):
                tier = base_tier(src)
                # §251 gate 4: .stb cast membership = CUTSCENE tier (demo
                # shims load-bearing; recipes 10-15; kit cutscene.md)
                if nm in stb:
                    tier += (f" · **CUTSCENE ({len(stb[nm])} stb, §251 — "
                             f"demo shims load-bearing)**")
        else:
            src = "[INFERENCE-NEEDED: name not in l_objectName]"
            proc, unmapped = "?", unmapped + 1
        rows.append(f"| `{nm}` | {placed} | `{proc}` | `{src}` | {tier} |")
    return rows, unmapped, dests, arcs

HEADER = ["| ACTR name | placements (layer:count) | proc (fpcNm) | donor source file | base / portability |",
          "|---|---|---|---|---|"]

def main():
    argv = sys.argv[1:]
    island = "--island" in argv
    if island:
        argv.remove("--island")
    stage, arcs_arg, out_md = argv[:3]
    names = objname_table()
    profs = profile_files()
    lines = [f"# Island roster — `{stage}` / {arcs_arg}" +
             (" + INTERIORS (SCLS-followed)" if island else ""),
             "",
             "> The placement-grounded cast list (kit v4 §239/§240). If a name is",
             "> not in THIS document, it does not belong on THIS island — check",
             "> before porting (the Windfall≠Outset guard).", ""]
    total_names = total_unmapped = 0
    rows, unm, dests, arcs = collect(
        stage, arcs_arg if arcs_arg == "all" else arcs_arg.split(","),
        names, profs)
    lines += [f"## Exterior — `{stage}` / {', '.join(arcs)}", ""] + HEADER + rows
    total_names += len(rows)
    total_unmapped += unm
    if island:
        visited, frontier = {stage, "sea"}, sorted(dests - {"sea", stage})
        while frontier:
            st = frontier.pop(0)
            if st in visited:
                continue
            visited.add(st)
            fs = os.path.join(FS_JSON, st + ".json")
            if not os.path.exists(fs):
                lines += ["", f"## Interior `{st}` — "
                          "[INFERENCE-NEEDED: no fact sheet (dev/unswept stage?)]"]
                continue
            rows, unm, more, arcs2 = collect(st, "all", names, profs)
            lines += ["", f"## Interior — `{st}` (all arcs: {', '.join(arcs2)})",
                      ""] + HEADER + rows
            total_names += len(rows)
            total_unmapped += unm
            frontier += sorted(more - visited)
    lines.append(f"\n**Totals: {total_names} placed names across the island; "
                 f"{total_unmapped} unmapped.**")
    os.makedirs(os.path.dirname(out_md) or ".", exist_ok=True)
    open(out_md, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    print(f"wrote {out_md}: {total_names} names, {total_unmapped} unmapped")

if __name__ == "__main__":
    main()
