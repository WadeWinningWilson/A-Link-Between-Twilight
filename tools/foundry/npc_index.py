"""Foundry §221: game-wide WW NPC state-machine index ("inklings for all NPCs").

Scans every d_a_npc_* donor source (.cpp + .h), counts both state idioms
(mode-switch writes and set_action transitions, header inlines included), and
emits one classification table: RICH (extractable now) vs THIN-SHELL (logic in
base/framework — needs a deeper decomp read before restoration). Outset cast
flagged. Mechanical counting; no semantics invented.

Usage: npc_index.py <decomp-root> <out.md>
"""
import sys, os, re

MODE_W = re.compile(r"^\s*[A-Za-z_][\w>.\-]*(?:mode|Mode|action|Action)\w*\s*=\s*[^;]+;")
SETACT = re.compile(r"set_action\s*\(")
ROSTER = re.compile(r"(\w*(?:action|Action)\w*)\s*\(")
OUTSET = {"aj1", "bm1", "bmcon1", "bms1", "bmsw", "ko1", "ko2", "ls1", "ob1",
          "ym1", "ym2", "p1", "sa1", "kam"}

def main():
    root, out_md = sys.argv[1], sys.argv[2]
    src = os.path.join(root, "src", "d", "actor")
    inc = os.path.join(root, "include", "d", "actor")
    rows = []
    for fn in sorted(os.listdir(src)):
        if not (fn.startswith("d_a_npc_") and fn.endswith(".cpp")):
            continue
        name = fn[len("d_a_npc_"):-4]
        texts = [open(os.path.join(src, fn), encoding="utf-8",
                      errors="replace").read()]
        h = os.path.join(inc, fn[:-4] + ".h")
        if os.path.exists(h):
            texts.append(open(h, encoding="utf-8", errors="replace").read())
        mode_w = setact = 0
        roster = set()
        kb = sum(len(t) for t in texts) // 1024
        for t in texts:
            for ln in t.splitlines():
                if MODE_W.match(ln):
                    mode_w += 1
                if SETACT.search(ln):
                    setact += 1
            roster |= {m.group(1) for m in ROSTER.finditer(t)
                       if "action" in m.group(1).lower()}
        total = mode_w + setact
        cls = "RICH" if total >= 5 else ("MODERATE" if total >= 1 else "THIN-SHELL")
        rows.append((name, kb, mode_w, setact, len(roster), cls,
                     "OUTSET" if name in OUTSET else ""))
    lines = ["# WW NPC state-machine index (all donor NPCs)", "",
             f"{len(rows)} NPC actors scanned (.cpp + .h). RICH = extractable via "
             "state_map now; THIN-SHELL = logic in base/framework patterns — "
             "[INFERENCE-NEEDED: deeper decomp read] before restoration.", "",
             "| npc | KB | mode writes | set_action | action-roster | class | note |",
             "|---|---|---|---|---|---|---|"]
    for name, kb, mw, sa, ro, cls, note in sorted(
            rows, key=lambda r: (-1 if r[6] else 0, -(r[2] + r[3]))):
        lines.append(f"| `{name}` | {kb} | {mw} | {sa} | {ro} | {cls} | {note} |")
    rich = sum(1 for r in rows if r[5] == "RICH")
    thin = sum(1 for r in rows if r[5] == "THIN-SHELL")
    lines.append(f"\nTotals: {rich} RICH · {len(rows)-rich-thin} MODERATE · "
                 f"{thin} THIN-SHELL of {len(rows)}.")
    open(out_md, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    print(f"wrote {out_md}: {len(rows)} NPCs ({rich} rich, {thin} thin)")

if __name__ == "__main__":
    main()
