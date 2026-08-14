#!/usr/bin/env python3
# ============================================================================
# port_planner.py — the PORT QUEUE as a computation (user-ratified 2026-08-12).
#
# "We need a system based on decompiled status + priority" — this is it:
#   RANK = CLASS x READINESS x DEMAND, with the ratified amendment that the
#   INITIAL PHASE weights toward what can be VISIBLY AND CODE-RATIFIED ON
#   OUTSET (the ported area); post-Outset the general plan pulls through.
#
# Inputs (all pre-existing instruments; this file only merges):
#   READINESS  decomp_status.parse_configure (MATCHED / NONMATCHING) +
#              /* Nonmatching */ marker counts in the donor TU (campaign size)
#   DEMAND     placement counts across the Outset stage set, read off the
#              DISC via space_kit (donor DZR layer chunks, all layers + base)
#   COST       kit_laws --donor-precast (DN-3 sites) + --donor-deps (name
#              gaps) run per-actor at port time — referenced, not duplicated
#   IDENTITY   donor l_objectName (d_stage.cpp:437) maps placement name ->
#              fpcNm enum -> donor TU (mechanical lowercase rule, misses
#              reported UNKNOWN, never guessed — №31-C)
#
# PORTED detection is mechanical: src/d/actor/d_a_ww_<stem>.cpp exists, plus
# a declared alias set for systems that port under another name. Absent from
# both = UNPORTED. The queue is a RANKING for the user to ratify, not an
# order to execute unreviewed.
# ============================================================================
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(Path(__file__).parent))
sys.path.insert(0, str(REPO / "tools" / "ww_crew_restoration_skeleton"))

import decomp_status

DONOR_ROOT = Path("D:/XXXXXXX/WW DP")
DONOR_STAGE_CPP = DONOR_ROOT / "src" / "d" / "d_stage.cpp"

# The Outset set: sea room 44 + every adopted/declared interior + forest.
OUTSET_STAGES = [
    ("sea", 44), ("LinkRM", None), ("LinkUG", None), ("Ojhous", None),
    ("Ojhous2", None), ("Onobuta", None), ("Omasao", None), ("Pjavdou", None),
    ("A_mori", None),
]

# Ported-under-another-name declarations (DECLARED, never inferred):
PORTED_ALIASES = {
    "d_a_grass": "d_a_ext_vegetation (kusa/flower/swood system)",
    "d_a_kytag00": "receiver kankyo (dKyWw) hosts",
    "d_a_ep": "d_a_ext_ep",
}

# ============================================================================
# SALVAGE TRACK (user ruling 2026-08-12, verbatim intent): "Not striked.
# Everything from the original game will be saved/salvaged — repurposed later
# in some manner, maybe players will interact via a pop-up menu in-game."
# Nothing is ever struck from the queue. Donor features whose runtime target
# does not exist on PC port INERT and enter this register, awaiting their
# repurposing ruling. Inert ports keep donor placements and donor logic;
# only the missing hardware/mechanic seam is dormant, and the seam is named.
# ============================================================================
SALVAGE = {
    "d_a_agbsw0": "GBA/Tingle Tuner trigger regions — no GBA link on PC; "
                  "dormant seam = the agb session; repurpose candidate: "
                  "in-game pop-up/companion menu",
    "d_a_agb": "GBA/Tingle Tuner core — same seam as agbsw0",
    "d_a_salvage": "ocean Salvage pickup points — L-8 deferred mechanic; "
                   "salvage-arm seam dormant until the mechanic ports",
}

RE_OBJNAME = re.compile(
    r'OBJNAME\s*\(\s*"([^"]+)"\s*,\s*fpcNm_(\w+?)_e\b')
RE_NONMATCH = re.compile(r"/\*\s*Nonmatching\s*\*/")


def objname_table():
    """placement name -> donor TU stem (d_a_xxx), via the donor's own table."""
    txt = DONOR_STAGE_CPP.read_text(encoding="utf-8", errors="replace")
    out = {}
    for m in RE_OBJNAME.finditer(txt):
        out[m.group(1)] = "d_a_" + m.group(2).lower()
    return out


def outset_placements():
    """{placement name: count} across the Outset stage set, all layers, disc."""
    import space_kit
    counts = {}
    for stage, room in OUTSET_STAGES:
        try:
            rooms = space_kit.donor_rooms(stage)
        except Exception as e:
            print("  (stage %s unreadable: %s)" % (stage, e))
            continue
        for rname in rooms:
            room_no = int("".join(ch for ch in rname if ch.isdigit()) or 0)
            if room is not None and room_no != room:
                continue
            rbytes, _src = space_kit.donor_file(stage, rname)
            _, dzr = space_kit.get_member(rbytes, ".dzr")
            if dzr is None:
                continue
            for tag, entries in space_kit.dz_chunks(dzr).items():
                base3 = tag[:3]
                if tag in ("ACTR", "TGOB"):
                    stride = 0x20
                elif tag in ("SCOB", "TGSC", "DOOR", "TGDR", "Door"):
                    stride = 0x24
                elif base3 in ("ACT", "SCO") and tag[3] in space_kit._LAYER_IDX:
                    stride = 0x20 if base3 == "ACT" else 0x24
                else:
                    continue
                for cnt, off in entries:
                    for i in range(cnt):
                        nm = dzr[off + i * stride: off + i * stride + 8].split(b"\x00")[0]
                        if nm:
                            counts[nm.decode("ascii", "replace")] = \
                                counts.get(nm.decode("ascii", "replace"), 0) + 1
    return counts


def actor_class(stem):
    if stem.startswith("d_a_npc_"):
        return "NPC"
    if stem.startswith(("d_a_obj_", "d_a_tsubo", "d_a_kanban", "d_a_door")):
        return "OBJ"
    if stem.startswith(("d_a_e_", "d_a_boko")) or stem in ("d_a_mo2", "d_a_ph"):
        return "ENEMY"
    if stem.startswith(("d_a_tag", "d_a_kytag", "d_a_sw", "d_a_agb", "d_a_evsw")):
        return "TRIGGER"
    if stem.startswith(("d_a_item", "d_a_spc_item")):
        return "ITEM"
    return "MISC"


def readiness(stem, table):
    key = "d/actor/%s.cpp" % stem
    st = table.get(key)
    if st is None:
        return ("NO-TU", None)
    if st == "MATCHED":
        return ("MATCHED", 0)
    fp = DONOR_ROOT / "src" / "d" / "actor" / (stem + ".cpp")
    n = len(RE_NONMATCH.findall(
        fp.read_text(encoding="utf-8", errors="replace"))) if fp.is_file() else -1
    return ("NONMATCHING", n)


def main():
    table = decomp_status.parse_configure()
    names = objname_table()
    print("l_objectName rows: %d" % len(names))
    counts = outset_placements()
    print("Outset placement names: %d (%d placements)"
          % (len(counts), sum(counts.values())))

    receiver_ww = {p.stem for p in (REPO / "src" / "d" / "actor").glob("d_a_ww_*.cpp")}

    rows = {}
    unknown_names = []
    for nm, cnt in counts.items():
        stem = names.get(nm) or names.get(nm.lower())
        if stem is None:
            unknown_names.append((nm, cnt))
            continue
        r = rows.setdefault(stem, {"names": [], "count": 0})
        r["names"].append(nm)
        r["count"] += cnt

    ranked = []
    for stem, r in rows.items():
        ww = "d_a_ww_" + stem[4:]
        if ww in receiver_ww:
            status = "PORTED (%s)" % ww
        elif stem in PORTED_ALIASES:
            status = "PORTED-AS (%s)" % PORTED_ALIASES[stem]
        elif (REPO / "src" / "d" / "actor" / (stem + ".cpp")).is_file():
            # A receiver TU under the DONOR stem is ambiguous: knob00/ls1/zl1
            # are WW ports under donor names, but a TP-native namesake would
            # look identical. Named, never guessed (№31-C).
            status = "RECEIVER-TU (verify: WW port or TP namesake)"
        else:
            status = "UNPORTED"
        if stem in SALVAGE and status == "UNPORTED":
            status = "UNPORTED->SALVAGE (port inert)"
        rd, markers = readiness(stem, table)
        ranked.append((stem, r["count"], sorted(set(r["names"])), status, rd, markers))

    # Phase-1 order: unported, MATCHED first, then fewest markers, then demand.
    def key(row):
        stem, cnt, _, status, rd, markers = row
        tier = 0 if rd == "MATCHED" else (1 if rd == "NONMATCHING" else 2)
        ported_rank = (0 if status == "UNPORTED"
                       else 1 if status.startswith("RECEIVER-TU") else 2)
        return (ported_rank, tier,
                markers if markers is not None else 999, -cnt)
    ranked.sort(key=key)

    out = REPO / "docs" / "state" / "ww-staging" / "PORT-QUEUE.md"
    with open(out, "w", encoding="utf-8", newline="\r\n") as f:
        f.write("# PORT QUEUE — computed, user-ratifies (port_planner.py)\n\n")
        f.write("Ratified frame (2026-08-12): CLASS x READINESS x DEMAND, initial phase\n")
        f.write("weighted to what is VISIBLY AND CODE-RATIFIABLE ON OUTSET; post-Outset\n")
        f.write("the general plan applies. Regenerate any time — one command.\n\n")
        f.write("## Phase 1 — Outset-verifiable actor ports (disc placements, all layers)\n\n")
        f.write("| donor TU | class | Outset placements | names | decomp | markers | status |\n")
        f.write("|---|---|---|---|---|---|---|\n")
        for stem, cnt, nms, status, rd, markers in ranked:
            f.write("| %s | %s | %d | %s | %s | %s | %s |\n"
                    % (stem, actor_class(stem), cnt, " ".join(nms[:6]),
                       rd, "" if markers is None else markers, status))
        if unknown_names:
            f.write("\n**UNKNOWN placement names (no l_objectName row — resolve, "
                    "never guess):** %s\n"
                    % ", ".join("%s(x%d)" % t for t in sorted(unknown_names)))
        f.write("\n## Stage layout classes (room_layout.py, tale §837/§838)\n\n")
        f.write("Room relationships per Outset stage — interacting layouts are invisible\n")
        f.write("in play and in any per-room inspection; a wrong room index on an\n")
        f.write("interacting stage fails silently (the Orca/Sturgeon lesson):\n\n")
        try:
            import room_layout as rl
            _iso = rl.default_iso()
            if _iso:
                _f, _boot = rl.wd.iso_open(_iso)
                _rooms = rl.stage_rooms(_f, _boot)
                for stage, room in OUTSET_STAGES:
                    rs = _rooms.get(stage, [])
                    f.write("- **%s** (%d room(s)): %s\n"
                            % (stage, len(rs), rl.stage_summary(_f, _boot, stage, rs)))
            else:
                f.write("- (no ISO configured — layout column UNKNOWN, not clean)\n")
        except Exception as e:
            f.write("- (layout read failed: %s — UNKNOWN, not clean)\n" % e)
        f.write("\n## Salvage register (user ruling 2026-08-12: nothing struck)\n\n")
        f.write("Every original-game feature is saved/salvaged for later repurposing\n")
        f.write("(e.g. an in-game pop-up interaction). These port INERT — donor\n")
        f.write("placements and logic kept, the missing seam named and dormant:\n\n")
        for stem, why in sorted(SALVAGE.items()):
            f.write("- **%s** — %s\n" % (stem, why))
        f.write("\n## Phase 1 systems track\n\n")
        f.write("Systems certify on Outset through the actors that consume them.\n")
        f.write("Source: absent-systems-census.md (name-gap tiers, tale §829) — top\n")
        f.write("rows are the systems worklist; twin/family rows need adjudication\n")
        f.write("(§828 rules) before any port is opened.\n\n")
        f.write("## Phase 2 — post-Outset\n\n")
        f.write("The ratified general plan: class order S systems > A lighting > B VFX >\n")
        f.write("C SFX > D actors > E decorative, each class MATCHED-first, demand from\n")
        f.write("the full census. Regenerate Phase 1 against the next adopted area's\n")
        f.write("stage set by editing OUTSET_STAGES.\n")

    unported = [r for r in ranked if r[3] == "UNPORTED"]
    print("\nPhase-1 head (UNPORTED, cheapest-first):")
    for stem, cnt, nms, status, rd, markers in unported[:15]:
        print("  %-22s %-7s x%-3d %-11s markers=%-4s %s"
              % (stem, actor_class(stem), cnt, rd,
                 "" if markers is None else markers, " ".join(nms[:4])))
    print("\n%d actor TU(s) placed on Outset · %d unported · artifact -> %s"
          % (len(ranked), len(unported), out))
    return 0


if __name__ == "__main__":
    sys.exit(main())
