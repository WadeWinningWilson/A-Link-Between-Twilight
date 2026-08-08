#!/usr/bin/env python3
"""R5 CONVERSION DB — the single consulted table for WW→TP conversion facts.

BRIDGE HOSTS · HISTORY CLASSIFIES (unified plan, Band 2 #4).

WHY THIS EXISTS
---------------
The conversion facts a port needs are currently scattered across the tools that
happened to discover them: chunk record sizes live in the room baker, the
start-code alias lives in the space kit, DZB and island rosters live in two more
places again. Every new port re-locates them, and — the failure mode that
actually costs rounds — a fact fixed in one table stays wrong in the others.
§379a/§423's premise applies to tooling as much as to code: ONE source, many
consumers.

It also unblocks V1 (the three-source accessor differ), whose job is to compare
independent decoders of the same bytes. A differ needs a table that states what
this project BELIEVES; that is this file.

CLASSIFICATION (History's half of the ownership split)
------------------------------------------------------
Every row carries a TIER, because "we know this" is not one thing:

  LAW        proven against the binary/decomp/Winditor; disagreement is a bug
             in the disagreeing decoder, not a question.
  DERIVED    computed from LAW rows by a rule stated in the row.
  OBSERVED   holds everywhere we have looked, no proof it must; a differ
             disagreement here is a real question, not a bug.
  HOST       a receiver-side mapping decision (our choice, donor-silent) —
             never presented as donor fact.

The tier is the point. A consumer that treats OBSERVED as LAW will "fix" the
wrong side of a disagreement, which is exactly how §375 (the endian overlay) and
§396 (the name-collision enum) each cost a round.

USAGE
    from conversion_db import DB, get
    size = get("chunk_size", "ACTR")          # -> 0x20
    DB["chunk_size"]["ACTR"].tier             # -> "LAW"
    python tools/conversion_db.py             # print the table + provenance
"""
from __future__ import annotations

import sys
from dataclasses import dataclass

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")


@dataclass(frozen=True)
class Fact:
    value: object
    tier: str      # LAW | DERIVED | OBSERVED | HOST
    why: str       # provenance — the receipt, not a restatement of the value


def F(value, tier, why):
    return Fact(value, tier, why)


# ---------------------------------------------------------------------------
# stage/room chunk record sizes  (absorbed: bake_room_chunks.KNOWN_SIZE)
# ---------------------------------------------------------------------------
CHUNK_SIZE = {
    "ACTR": F(0x20, "LAW", "donor dzr walk + Winditor EntityTemplates; 100s of rows parsed"),
    "SCOB": F(0x24, "LAW", "same walk; SCOB carries the extra scale/pad triple"),
    "DOOR": F(0x24, "LAW", "SCOB-shaped (Winditor treats DOOR as a SCOB variant)"),
    "TGDR": F(0x24, "LAW", "SCOB-shaped"),
    "SCLS": F(0x0C, "LAW", "stage exit table; §379/§423 door work read these directly"),
    "PLYR": F(0x20, "LAW", "§400b patched all 7 host spawns through this stride"),
    "STAF": F(0x50, "LAW", "event staff record — §423 A2 diffed donor vs receiver field by field"),
    "EVNT": F(0xB0, "LAW", "merge_event.py ESZ, proven by the §379a event merges"),
    "CUT":  F(0x50, "LAW", "merge_event.py CSZ"),
    "EVDT": F(0x40, "LAW", "merge_event.py DSZ"),
}

# ---------------------------------------------------------------------------
# start-code aliasing  (absorbed: space_kit.STARTCODE_ALIAS)
# ---------------------------------------------------------------------------
STARTCODE_ALIAS = {
    ("LinkRM", 0): F(0xCB, "HOST",
                     "our host-mapping choice, donor-silent: LinkRM spawn 0 aliases to the "
                     "host's baked entry. NOT a donor fact — never cite it as one."),
}

# ---------------------------------------------------------------------------
# runtime-vs-file field classification  (§423 A2's finding, generalised)
# ---------------------------------------------------------------------------
FIELD_CLASS = {
    "staff.file_meaningful": F(
        ("tagID@0x20", "index@0x24", "flagID@0x28", "type@0x2C",
         "startCut@0x30", "currentCut@0x38", "curAction@0x3C"),
        "LAW",
        "§423 A2: donor and receiver agree at EVERY one of these offsets — the "
        "reason donor event data parses with receiver structs (§379a)"),
    "staff.runtime_scratch": F(
        ("0x40 wipeDirection(donor) vs bool+bool(TP)",
         "0x42 timer(donor) vs mData[](TP)",
         "0x46 advance(donor) vs mData[4](TP)"),
        "LAW",
        "§423 A2: same region, INCOMPATIBLE typing — per-game runtime state, not "
        "file data. Identified §319's 'ambiguous field_0x40' as the BE high half "
        "of the donor's wipe latch. Consumers must NOT alias across games here."),
}

# ---------------------------------------------------------------------------
# particle-id lineage  (§396 — the trap that cost a round)
# ---------------------------------------------------------------------------
PARTICLE_ID = {
    "AK_JN_TORCH":      F({"donor": 0x01EA, "receiver_enum": 0x41}, "LAW",
                          "§396: SAME NAME, DIFFERENT VALUE. Donor value verified present in "
                          "the staged common.jpc by JPAC1-00 walk; receiver's 0x41 is absent "
                          "from every WW jpc. Always take the DONOR number."),
    "AK_JP_O_KAGEROU00": F({"donor": 0x4004, "receiver_enum": 0x47}, "LAW",
                           "§396, same walk"),
}

# ---------------------------------------------------------------------------
# sky/celestial palette slots  (Winditor = law for authored colour data)
# ---------------------------------------------------------------------------
VIRT_PALETTE = {
    "order": F(("unk1..4 RGBA", "HorizonCloudColor RGBA", "CenterCloudColor RGBA",
                "SkyColor RGB", "FalseSeaColor RGB", "HorizonColor RGB", "unk5..7 u8"),
               "LAW",
               "Winditor EntityTemplates.cs:2346 byte order; §417b/§422c used this to "
               "prove CenterCloudColor is AUTHORED data that TP dropped"),
}

# ---------------------------------------------------------------------------
# retail presence  (what the shipped game actually contains)
# ---------------------------------------------------------------------------
RETAIL_PRESENCE = {
    "drawVrkumo":     F(True,  "LAW", "retail framework.map 0xE3C @8009AB88"),
    "drawVrkumokage": F(False, "LAW",
                        "§422b/c: ABSENT from retail framework.map (debug map only, "
                        "dead-stripped — nothing calls it) AND absent from Winditor's data "
                        "model. Porting it would ADD behavior vanilla never renders."),
}

DB = {
    "chunk_size": CHUNK_SIZE,
    "startcode_alias": STARTCODE_ALIAS,
    "field_class": FIELD_CLASS,
    "particle_id": PARTICLE_ID,
    "virt_palette": VIRT_PALETTE,
    "retail_presence": RETAIL_PRESENCE,
}

# Absorption ledger — what this file replaces, so consumers get migrated rather
# than duplicated. A row leaves this list only when its origin is DELETED.
ABSORBED = [
    ("bake_room_chunks.KNOWN_SIZE",  "chunk_size",      "origin still live; migrate on next touch"),
    ("space_kit.STARTCODE_ALIAS",    "startcode_alias", "origin still live; migrate on next touch"),
    ("ww_dzb_roster",                "PENDING",         "not yet absorbed — needs History classify pass"),
    ("island_roster / output roster","PENDING",         "absorb after R1's roster re-pin (they change)"),
]


def get(table: str, key):
    fact = DB[table][key]
    return fact.value


def main() -> int:
    print("R5 CONVERSION DB — tiers: LAW | DERIVED | OBSERVED | HOST\n")
    for tname, table in DB.items():
        print(f"== {tname}")
        for k, f in table.items():
            print(f"   {str(k):28s} {f.tier:8s} {f.value}")
            print(f"   {'':28s}          why: {f.why}")
        print()
    print("ABSORPTION LEDGER (origin -> table -> state)")
    for origin, tbl, state in ABSORBED:
        print(f"   {origin:32s} -> {tbl:16s} {state}")
    print("\nRULE: a consumer that treats OBSERVED as LAW will 'fix' the wrong side of a")
    print("disagreement. That is how §375 and §396 each cost a round. Check the tier.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
