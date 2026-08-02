"""Foundry P9 first mission: spawn-parity verdict for one room.

Compares the donor fact-sheet roster (what the donor places, per layer) against the
receiver's [Spawn] census events (what the port actually spawned), from banked logs.

Covenant laws (docs/Foundry-P9-P13.md §Covenant): donor-verbatim; rows whose
verdict depends on unknown layer state are flagged [INFERENCE-NEEDED] rather than
judged; UNKNOWN/flag rows never MATCH.

Usage: python spawn_verdict.py <stage.json> <room_arc> <receiver.jsonl> <out.md>
e.g.   spawn_verdict.py fact-sheets/json/sea.json Room44.arc port-emitter-session.jsonl out.md
"""
import sys, json
from collections import Counter

def main():
    stage_json, room_arc, recv_jsonl, out_md = sys.argv[1:5]
    stage = json.load(open(stage_json, encoding="utf-8"))
    rows = stage.get(room_arc, [])

    donor = {}  # layer -> Counter(name)
    donor_params = {}  # name -> set(params) — multi-type detection (bus §214)
    for r in rows:
        if r["chunk"] not in ("ACTR", "SCOB", "TGDR", "DOOR", "TGOB", "TGSC"):
            continue
        donor.setdefault(r["layer"], Counter()).update(r["names"])
        for e in r["entries"]:
            if e.get("name") and e.get("params") is not None:
                donor_params.setdefault(e["name"], set()).add(e["params"])

    recv = Counter()
    with open(recv_jsonl, encoding="utf-8") as f:
        for line in f:
            ev = json.loads(line)
            if ev.get("site") == "stage_placement" and ev.get("addr") == "dusklog":
                recv[ev["key"][1]] += 1

    default = donor.get("Default", Counter())
    layered_names = Counter()
    for layer, c in donor.items():
        if layer != "Default":
            layered_names.update(c)

    lines = [f"# P9 spawn-parity verdict — `{room_arc}` vs receiver census", "",
             f"Donor: {sum(default.values())} Default-layer placements "
             f"({len(default)} names) + {sum(layered_names.values())} layered. "
             f"Receiver: {sum(recv.values())} census spawns ({len(recv)} names).", "",
             "| name | donor Default | donor layered | receiver | verdict |",
             "|---|---|---|---|---|"]
    match = miss = extra = flagged = 0
    for name in sorted(set(default) | set(layered_names) | set(recv)):
        nd, nl, nr = default.get(name, 0), layered_names.get(name, 0), recv.get(name, 0)
        if nd and nr:
            v = "MATCH" if nr >= nd else f"PARTIAL ({nr}/{nd})"
            match += 1 if nr >= nd else 0
            miss += 0 if nr >= nd else 1
        elif nd and not nr:
            v = "MISSING (donor Default, port never spawned)"
            miss += 1
        elif not nd and nl and nr:
            v = "[INFERENCE-NEEDED] layered-only in donor — verdict needs P10 layer state"
            flagged += 1
        elif not nd and nl and not nr:
            v = "[INFERENCE-NEEDED] layered-only, not seen — may be correct for layer"
            flagged += 1
        else:
            v = "PORT-ONLY (no donor placement — socket inhabitant or liberty?)"
            extra += 1
        # Multi-type guard (bus §214, the Ikada correction): a name with multiple
        # distinct donor params is a BLEND of subtypes — name-level MATCH is
        # provisional until params reach the receiver census + the actor's decode
        # (param_map.py) splits the row.
        nprm = len(donor_params.get(name, ()))
        if nprm > 1 and v.startswith(("MATCH", "PARTIAL")):
            v += f" **[MULTI-TYPE ×{nprm} params — name-level only, INFERENCE-NEEDED]**"
            flagged += 1
        lines.append(f"| `{name}` | {nd} | {nl} | {nr} | {v} |")
    lines.append("")
    lines.append(f"**Summary: {match} MATCH · {miss} MISSING/PARTIAL · {extra} "
                 f"PORT-ONLY · {flagged} [INFERENCE-NEEDED] (await P10 layer oracle).**")
    lines.append("Caveat: receiver census covers only rooms the session visited; "
                 "absence of a spawn is evidence only for visited space.")
    with open(out_md, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")
    print(f"wrote {out_md}: {match} match, {miss} missing, {extra} port-only, "
          f"{flagged} flagged")

if __name__ == "__main__":
    main()
