#!/usr/bin/env python3
# ============================================================================
# dn_registry.py — the DO-NOT registry AS DATA (user-ratified 2026-08-13:
# "treat DNs as systems for now"; enforcement tiers deferred by the same word).
#
# WHAT THIS IS: docs/DO-NOT.md remains the CHARTER — the full mechanism text,
# hand-written, authoritative. This module is its machine-readable shadow on
# the port_deps.SYSTEMS pattern: one row per DN with SURFACES (globs + symbol
# families it guards), STATUS, and RECEIPTS — amended BY RULING ONLY, never
# silently. What it buys, today:
#   · `list` / `show <id>`  — the registry as a queryable object
#   · `touch <path|symbol>` — which DNs guard this surface? (a READ api, not
#     a gate — gate wiring is the deferred tier)
#   · `selftest`            — every row's surfaces verified to still EXIST in
#     the tree; a DN naming a dead surface FLAGS ITSELF for a ruling instead
#     of rotting (DNs change/adapt with the project — the user's premise)
#
# TRANSCRIPTION ONLY: rows compress the charter, they never extend it. The
# charter's numbering has no DN-5..DN-8 — recorded, not invented. Rule text
# lives in DO-NOT.md; go read the entry before touching a guarded surface.
# ============================================================================
import re
import subprocess
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
CHARTER = REPO / "docs" / "DO-NOT.md"

DNS = {
    "DN-1": {
        "title": "NEVER stamp room id 0 / guessed room onto collision-BG owners",
        "surfaces": {
            "globs": ["src/d/d_ext_npc_mount.cpp"],
            "symbols": ["SetRoomId", "resolveIdentityBgHostRoom", "dBgW", "daBg"],
        },
        "status": "ACTIVE",
        "receipts": ["№256→№264/№265 (2026-07-22..23) incident + sanctioned path"],
    },
    "DN-2": {
        "title": "NEVER 'repair' the №113 sea-K0 stash in PAL0 plight_col[2]",
        "surfaces": {
            "globs": ["tools/ww_crew_restoration_skeleton/convert_lighting.py"],
            "symbols": ["plight_col", "dKy_get_seacolor"],
        },
        "status": "ACTIVE",
        "receipts": ["§113/§148 mechanism; fix-revert loop №270/§112 stopped at cycle 3",
                     "NOTE: charter also guards F_DL01/STG_00.arc — a fork-stage "
                     "artifact now in arcs_retired_row21; selftest will surface "
                     "whether this half of the surface is stale"],
    },
    "DN-3": {
        "title": "NEVER parse BDL at arc-mount/globally (consume-time only)",
        "surfaces": {
            "globs": ["src/d/d_resorce.cpp", "src/d/d_ext_npc_mount.cpp"],
            "symbols": ["loadResource", "loadBinaryDisplayList",
                        "dExtNpcMount_acquireModelData", "s_modelDataCache"],
        },
        "status": "ACTIVE-AMENDED",
        "receipts": ["§180/§181 double-parse incident + Housing Approach A",
                     "AMENDED §398 (2026-08-04, user-directed): BMD family IS "
                     "mount-parsed (donor-faithful); the gap is BDL only; the "
                     "adapter premise disproven — rule STANDS, framing corrected",
                     "enforcement shadow exists: kit_laws laws 2 + 9 (fact, not "
                     "new machinery)"],
    },
    "DN-4": {
        "title": "NEVER the ALBW post-man dialogue box — ALWAYS Shade Watcher native",
        "surfaces": {
            "globs": ["src/d/d_ext_npc_mount.cpp",
                      "src/d/actor/d_a_albw_shade_watcher.cpp"],
            "symbols": ["dMsgFlow_c", "mountPaginate", "dExtWw_handleDemoMessage"],
        },
        "status": "ACTIVE",
        "receipts": ["user directive 2026-07-27, emphatic and standing; repeat "
                     "offender on Aryll/awake + Grandma tale before the entry"],
    },
    "DN-9": {
        "title": "NEVER mount a donor model onto a receiver proc (native systems only)",
        "surfaces": {
            "globs": ["src/d/d_ext_npc_mount.cpp"],
            "symbols": ["actor_map", "NPC_YAFLW"],
        },
        "status": "ACTIVE",
        "receipts": ["user ratification 2026-08-01: 'never doing mounting again'; "
                     "case receipts §192 (grass VFX 3 ferries) + §205 (inert flowers)"],
    },
    "DN-10": {
        "title": "NEVER instance-authored code before the native system is read+tried "
                 "(order of resort: donor port → boundary translation → authored-with-proof+go)",
        "surfaces": {
            "globs": ["**"],
            "symbols": [],
        },
        "status": "ACTIVE",
        "receipts": ["binds EVERY task, no surface limit (CLAUDE.md header)",
                     "DN-10-S clause added by user ruling (tale §819): substitution "
                     "is evidence, not a technique — enforcement shadow exists: "
                     "kit_laws law 10 + port_deps name-gap triager"],
    },
}


def selftest():
    """Every surface verified to still exist — a stale surface is a ruling
    request, not silent rot. Absent charter file = hard fail."""
    bad = 0
    if not CHARTER.is_file():
        print("FAIL: charter docs/DO-NOT.md missing")
        return 1
    charter = CHARTER.read_text(encoding="utf-8", errors="replace")
    for dn, row in DNS.items():
        if ("## %s " % dn) not in charter and ("## %s—" % dn) not in charter \
                and ("## %s —" % dn) not in charter:
            print("[STALE] %s: no charter section found — registry/charter drift" % dn)
            bad += 1
        for g in row["surfaces"]["globs"]:
            if g == "**":
                continue
            if not list(REPO.glob(g)):
                print("[STALE] %s: surface glob matches nothing: %s" % (dn, g))
                bad += 1
        for s in row["surfaces"]["symbols"]:
            r = subprocess.run(["grep", "-rlq", "-F", s, str(REPO / "src"),
                                str(REPO / "include"), str(REPO / "tools")],
                               capture_output=True)
            if r.returncode != 0:
                print("[STALE] %s: symbol not found anywhere: %s" % (dn, s))
                bad += 1
    print("selftest: %s" % ("OK — every surface live" if not bad
                            else "%d stale surface(s) — file a ruling row, do not edit silently" % bad))
    return 1 if bad else 0


def touch(targets):
    """Which DNs guard these paths/symbols? A read API — the deferred tier
    would wire this into preflight/queue; today it answers when asked."""
    hits = []
    for dn, row in DNS.items():
        why = []
        for tgt in targets:
            for g in row["surfaces"]["globs"]:
                if g == "**" or Path(tgt).match(g) or tgt in g or g in tgt:
                    why.append("%s ~ %s" % (tgt, g))
            for s in row["surfaces"]["symbols"]:
                if s.lower() in tgt.lower():
                    why.append("%s ~ symbol %s" % (tgt, s))
        if why:
            hits.append((dn, row, sorted(set(why))))
    for dn, row, why in hits:
        print("%s [%s] — %s" % (dn, row["status"], row["title"]))
        for w in why:
            print("    %s" % w)
        print("    -> READ docs/DO-NOT.md '%s' BEFORE the edit" % dn)
    if not hits:
        print("no DN guards these targets (DN-10 still binds every task)")
    return 0


def main():
    args = sys.argv[1:]
    if args and args[0] == "selftest":
        return selftest()
    if args and args[0] == "show" and len(args) > 1:
        dn = args[1].upper()
        row = DNS.get(dn)
        if not row:
            print("unknown %s — ids: %s" % (dn, ", ".join(DNS)))
            return 2
        print("%s [%s] — %s" % (dn, row["status"], row["title"]))
        print("  globs:   %s" % ", ".join(row["surfaces"]["globs"]))
        print("  symbols: %s" % ", ".join(row["surfaces"]["symbols"]) or "(none)")
        for r in row["receipts"]:
            print("  receipt: %s" % r)
        return 0
    if args and args[0] == "touch" and len(args) > 1:
        return touch(args[1:])
    print("DO-NOT registry as data (charter: docs/DO-NOT.md; DN-5..8 do not "
          "exist in the charter — recorded, not invented)")
    for dn, row in DNS.items():
        print("  %-6s [%s] %s" % (dn, row["status"], row["title"]))
    print("usage: dn_registry.py [list] | show <id> | touch <path|symbol> ... | selftest")
    return 0


if __name__ == "__main__":
    sys.exit(main())
