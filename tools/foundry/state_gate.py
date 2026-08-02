"""Foundry §224: the P13 acceptance gate — ported-actor state stream vs donor law.

Reads the receiver's action-state events (actor_state, from state_tap.hpp via
dusktap_to_jsonl) and the donor statemap artifact (state_map.py output), and
gates: every action/mode VALUE the port enters must be a value the donor's own
transition writes assign (cited in the statemap). Unknown values = DRIFT rows.
Order/semantics beyond value-membership are reviewed against the statemap's
dispatchers by the content lane — flagged, not auto-passed (covenant).

Usage: state_gate.py <receiver.jsonl> <donor-statemap.md> [--tag pig] <out.md>
"""
import sys, json, re
from collections import Counter

# Any cited transition-write line in the statemap: named (mAction/mMode),
# helper-call target, or anonymous state field (§224 idioms) — the numeric
# target is the donor-legal value either way.
ASSIGN_VAL = re.compile(r"`[^`]*=\s*(-?(?:0x[0-9A-Fa-f]+|\d+))\s*;")
# §231 (History): re.MULTILINE — without it `$` only matches end-of-string, so NONE of the
# per-line `cases:` dispatcher labels entered the legal set ("0 from case labels"), which
# false-DRIFTed switch states the donor reaches by fallthrough/increment rather than an
# explicit `=` write (pig modes 1/3/5, act 1). Flagged to Foundry, bus §233.
CASE_VALS = re.compile(r"cases: (.+)$", re.MULTILINE)

def donor_value_sets(statemap_path):
    text = open(statemap_path, encoding="utf-8", errors="replace").read()
    assigned = set()
    for m in ASSIGN_VAL.finditer(text):
        assigned.add(int(m.group(1), 0))
    cases = set()
    for m in CASE_VALS.finditer(text):
        for c in re.findall(r"`(-?(?:0x[0-9A-Fa-f]+|\d+))`", m.group(1)):
            cases.add(int(c, 0))
    return assigned, cases

def npc_roster(statemap_path):
    """§238 NPC mode: the legal set = the statemap's action-method roster
    (set_action targets + the roster line)."""
    text = open(statemap_path, encoding="utf-8", errors="replace").read()
    names = set(re.findall(r"set_action\s*\(\s*&?\w*::(\w+)", text))
    m = re.search(r"## NPC idiom — action-method state roster\n(.+)", text)
    if m:
        names |= set(re.findall(r"`(\w+)`", m.group(1)))
    return names

def main():
    args = [a for a in sys.argv[1:]]
    tag = None
    npc = "--npc" in args
    if npc:
        args.remove("--npc")
    if "--tag" in args:
        i = args.index("--tag")
        tag = args[i + 1]
        del args[i:i + 2]
    recv_jsonl, statemap, out_md = args
    if npc:
        roster = npc_roster(statemap)
        seen, drift = [], []
        with open(recv_jsonl, encoding="utf-8-sig") as f:
            for line in f:
                ev = json.loads(line)
                if ev.get("site") != "actor_state_npc":
                    continue
                if tag and ev["key"][1] != tag:
                    continue
                fn = ev["key"][2]
                seen.append((ev["t_s"], fn))
                if fn not in roster and fn not in {d[1] for d in drift}:
                    drift.append((ev["t_s"], fn))
        lines = [f"# P13b NPC state gate — `{recv_jsonl}` vs `{statemap}`", ""]
        if not seen:
            lines.append(f"**VERDICT: UNKNOWN — no actor_state_npc events (tag={tag}).**")
            open(out_md, "w", encoding="utf-8").write("\n".join(lines) + "\n")
            print("VERDICT: UNKNOWN")
            sys.exit(3)
        lines.append(f"{len(seen)} transitions; donor roster = {len(roster)} methods.")
        c = Counter(fn for _, fn in seen)
        lines += ["", "| method | count | verdict |", "|---|---|---|"]
        for fn, n in c.most_common():
            ok = fn in roster
            lines.append(f"| `{fn}` | {n} | {'MATCH' if ok else 'DRIFT (not in donor roster)'} |")
        verdict = "MATCH (roster-membership)" if not drift else \
            f"DRIFT ({len(drift)} unknown methods)"
        lines.append(f"\n**VERDICT: {verdict}** — sequence semantics = content-lane review.")
        open(out_md, "w", encoding="utf-8").write("\n".join(lines) + "\n")
        print(f"VERDICT: {verdict}; wrote {out_md}")
        sys.exit(0 if not drift else 1)
    assigned, cases = donor_value_sets(statemap)
    legal = assigned | cases
    seen = []
    with open(recv_jsonl, encoding="utf-8-sig") as f:
        for line in f:
            ev = json.loads(line)
            if ev.get("site") != "actor_state":
                continue
            if tag and ev["key"][1] != tag:
                continue
            seen.append((ev["t_s"], int(ev["key"][2]), int(ev["key"][3])))
    lines = [f"# P13 state gate — `{recv_jsonl}` vs `{statemap}`", ""]
    if not seen:
        lines.append("**VERDICT: UNKNOWN — no actor_state events "
                     f"(tag={tag}).** A silent tap is not a passing tap.")
        open(out_md, "w", encoding="utf-8").write("\n".join(lines) + "\n")
        print("VERDICT: UNKNOWN")
        sys.exit(3)
    vals = Counter()
    drift = []
    for t, act, mode in seen:
        for v, kind in ((act, "act"), (mode, "mode")):
            vals[(kind, v)] += 1
            if v not in legal and (kind, v) not in {d[:2] for d in drift}:
                drift.append((kind, v, t))
    lines.append(f"{len(seen)} transitions; donor legal-value set: "
                 f"{len(legal)} values ({len(assigned)} from writes, "
                 f"{len(cases)} from case labels).")
    lines.append("\n| kind | value | count | verdict |")
    lines.append("|---|---|---|---|")
    for (kind, v), c in sorted(vals.items()):
        ok = v in legal
        lines.append(f"| {kind} | {v} | {c} | "
                     f"{'MATCH' if ok else 'DRIFT (not in donor law)'} |")
    lines.append("\n## Transition sequence (for content-lane review vs "
                 "dispatchers — order semantics are NOT auto-passed)")
    lines.append("| t(s) | act | mode |")
    lines.append("|---|---|---|")
    for t, act, mode in seen[:200]:
        lines.append(f"| {t:.2f} | {act} | {mode} |")
    verdict = "MATCH (value-membership)" if not drift else \
        f"DRIFT ({len(drift)} unknown values)"
    lines.append(f"\n**VERDICT: {verdict}** — order review = content lane, "
                 "vs the statemap dispatchers. [INFERENCE-NEEDED is retained "
                 "for order until reviewed.]")
    open(out_md, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    print(f"VERDICT: {verdict}; wrote {out_md}")
    sys.exit(0 if not drift else 1)

if __name__ == "__main__":
    main()
