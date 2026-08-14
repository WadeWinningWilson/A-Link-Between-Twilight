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


# ---------------------------------------------------------------------------
# L4 — TP↔WW INTERACTION MAPPINGS (History authors · Bridge hosts)
#
# THE RULE: these live here as DATA. Never inline in ported actor code. The
# reason is the plan's own: TP-Link interaction is a TRANSLATION problem, not a
# fork — same class as §332's collision attributes and §212's JPA bits, both of
# which became tables and stopped being arguments. Inline now = rewrite when
# tunables (L5) arrive.
#
# THE BOUNDARY WITH L-10 (port-liberties BY-DESIGN) — I wrote both, so I owe the
# line between them explicitly, because they touch the same subject from
# opposite sides:
#
#   L4 (here)  = VALUES that translate.   damage numbers, weapon classes, item
#                ids, health units. WW authored a number, TP wants a number, and
#                a row states the correspondence. Reconcilable, testable, owed.
#   L-10 (BY-DESIGN) = WHO IS PERFORMING. TP Link's rig, animation set,
#                proportions, reach, acting vocabulary. Not translatable,
#                nothing owed — restoring the donor value means replacing the
#                protagonist.
#
# THE FAILURE THIS PREVENTS: a row like "TP Link's interaction range → WW's"
# encodes a BY-DESIGN property as a translation debt. It would sit here looking
# owed forever, and any attempt to "fix" it is the §338 y-band mistake with a
# bigger surface — tuning the receiver's hero toward a character the port does
# not contain. If a proposed row's donor side describes WW LINK rather than WW
# DATA, it belongs in port-liberties as BY-DESIGN, not in this table.
#
# STATE: seeded with what is EVIDENCED. The rest is listed as owed measurement
# rather than guessed — same discipline as the R1 contract gaps, and for the
# same reason: an invented mapping that "works" is indistinguishable from a
# correct one until it silently isn't.
# ---------------------------------------------------------------------------
INTERACTION_MAP = {
    # --- HEALTH (measured 2026-08-08) ---------------------------------------
    "health.storage": F(
        "u16 mMaxLife@0x00 + u16 mLife@0x02 — IDENTICAL layout both games", "LAW",
        "donor d_save.h:40-41 vs receiver d_save.h:183-184. Same fields, same offsets, "
        "same width. A naive port LOOKS correct here, which is exactly why it was measured."),
    "health.unit": F(
        "quarter-hearts in BOTH — damage values transfer WITHOUT scaling", "LAW",
        "donor initialises mMaxLife = 12 (= 3 hearts); receiver sets 20 new-game / 25 upgraded "
        "(= 5 / 6.25 hearts) at d_s_menu.cpp:1787/1671. Different TOTALS, SAME UNIT. This is a "
        "materially better answer than the feared 'a heart is not a quarter-heart': no damage "
        "row needs a scale factor. Cheap to check, expensive to assume."),

    # --- WEAPON / ATTACK-TYPE BITS (measured 2026-08-08) --------------------
    "weapon.at_type_bits": F(
        {"AGREE":      {"BOMB": 5},
         "COLLIDE":    {"donor BOOMERANG=6 / receiver 40=6",
                        "donor NORMAL_ARROW=14 / receiver HOOKSHOT=14",
                        "donor SKULL_HAMMER=16 / receiver BOOMERANG=16",
                        "donor FIRE=9 / receiver LANTERN_SWING=9",
                        "donor MACHETE=10 / receiver CSTATUE_SWING=10"},
         "RECEIVER_ONLY": {"HORSE": 2, "SHIELD_ATTACK": 4, "SLINGSHOT": 7,
                           "SPINNER": 19, "HEAVY_BOOTS": 21, "IRON_BALL": 22}},
        "LAW",
        "donor d_cc_d.h:14-27 vs receiver c_cc_d.h:33-56, read bit for bit. §212's JPA-bit "
        "precedent CONFIRMED AND WORSE THAN FEARED: only BOMB(5) agrees. Sword is bit 1 in "
        "both but named NORMAL_SWORD in the receiver (wooden/ordon) — a same-bit/different-"
        "scope case, not a clean agree. Five bits are OCCUPIED BY DIFFERENT WEAPONS in each "
        "game: passing a donor AT_TYPE word through unchanged turns a WW arrow into a TP "
        "hookshot and a WW skull hammer into a boomerang. NEVER pass these words across; "
        "translate per bit through this row. Receiver-only bits (HORSE, SPINNER, HEAVY_BOOTS, "
        "IRON_BALL...) have no donor counterpart — they are TP_INTO_WW additions, and HORSE@2 "
        "is Epona's, which is why the user's Epona note lands here as well as in TP_INTO_WW."),
    # --- DAMAGE (measured 2026-08-10) ---------------------------------------
    "damage.plumbing": F(
        "IDENTICAL path both games: int atp -> cCcD_ObjAt::mAtp (u8 @0x14) -> PlusDmg", "LAW",
        "donor c_cc_d.h:433 + d_cc_s.cpp:69 vs receiver c_cc_d.h:257 + d_cc_s.cpp:576. Same "
        "field, same offset, same width, same consumption; with health.unit above this closes "
        "the loop: attack power transfers with NO scale factor. Cross-checks that literally "
        "agree: bomb 4=4 (donor d_a_bomb2.cpp:306 / receiver d_a_nbomb.cpp:115), normal arrow "
        "2=2 (donor d_a_arrow.cpp:230 / receiver d_a_arrow.cpp:90)."),
    "damage.index": F(
        {"donor":    "SWORD TIER indexes damage (Hero 1/2 vs Master 2/4; flat across moves)",
         "receiver": "ATTACK MOVE indexes damage (1/2/3/4; flat across sword tiers)",
         "tier_carrier_receiver": "AT_TYPE_MASTER_SWORD bit 26, consumed enemy-side"},
        "LAW",
        "THE structural divergence, and the trap this row exists to name: donor "
        "d_a_player_sword.inc:298-384 branches every atp on checkNormalSwordEquip() (all three "
        "Master tiers share one branch, d_a_player.h:597-606); receiver d_a_alink_cut.inc "
        "hardcodes atp per move (:1142 slash=2, :1795 jump=3, :2031 spin=3/4, :1395 finish=3/4) "
        "and expresses the tier as a TYPE BIT (getSwordAtType, d_a_alink_cut.inc:80-86), not an "
        "atp delta. A ported WW actor inferring 'atp==1 means starter sword' reads TP's running "
        "slash as Hero's Sword; a ported TP actor reading the MASTER_SWORD bit on donor words "
        "collides with weapon.at_type_bits above. Neither game's damage table is a table — both "
        "are call-site literals (no HIO field on either side; measured, not assumed)."),
    "damage.values.sword": F(
        {"donor":    {"slash": "1/2", "finisher": "2/4", "jump": "2/4", "spin": "2/4 (soup x2)"},
         "receiver": {"run": 1, "slash": 2, "jump": 3, "finisher": "3 (4 Mortal Draw)",
                      "spin": "3 (4 Great Spin)"}},
        "LAW",
        "verbatim literals; donor pairs are Hero/Master (d_a_player_sword.inc:302/:306, "
        ":318/:322, :368/:372, :1342-1364), receiver per-move (d_a_alink_cut.inc:554/:1142/"
        ":1795/:1395/:1991-2005). Same unit, same 1-8 scale. Values transfer; INDEXES do not — "
        "see damage.index."),
    "damage.soup_multiplier": F(
        "donor-only: AT_TYPE_SWORD atp x2 while daPyFlg1_SOUP_POWER_UP", "LAW",
        "donor setAtParam d_a_player_main.cpp:9591-9596 (spin sites inline the doubled values, "
        "d_a_player_sword.inc:1342-1364). NO receiver counterpart (only the DEBUG one-hit-kill "
        "127 override, d_a_alink_cut.inc:227-229). A port that carries Elixir Soup must carry "
        "the gate; a port that reads donor atp literals must know 2/4/8 spin values are ONE "
        "attack under a state flag, not three attacks."),

    # --- ITEMS (measured 2026-08-10 — and the measurement CORRECTED this section) ---
    "item.get_message_formula": F(
        "get-message = item_no + 0x65 (101) — THE SAME FORMULA IN BOTH GAMES", "LAW",
        "donor MSG_NO_FOR_ITEM, d_item_data.h:265 (all 5 call sites read); receiver "
        "d_a_alink_demo.inc:2600-2625 and d_a_alink_bottle.inc:944 compute + 0x65 "
        "natively. There is NO donor item->get-message table. Verified against the "
        "donor BMG dump (ww_messages.tsv, 4412 rows): every get-message equals "
        "itemNo+101, including FUKU 0x32 -> 151 — §171's 'donor presents 151 via "
        "EXPLICIT override' claim is NOT FOUND in donor source and is hereby retired. "
        "Overrides that DO exist: donor heart-piece variants only (dproc.inc:620-624, "
        ":791-792); receiver per-case list (F_SP115/D_MN11/D_MN07 + heartPieceMessage"
        "[maxLife%5]). mItemMesgNum is a DEAD field (zero call sites) in the item-NAME "
        "band 0x191-0x1F0 — a nearby, plausible, WRONG source; named so nobody maps "
        "through it."),
    "item.id_space": F(
        "the REAL translation surface: item-ID SPACES differ, the formula does not",
        "LAW",
        "trap #3 (name-collision values) in item-id form: TP 0x2F = WEAR_KOKIRI, WW "
        "0x2F = HOOKSHOT; TP 0x47 = W_HOOKSHOT. Feeding a TP id into donor message "
        "numbering (host_item=0x2F in clothes_bundle.ini -> 0x2F+0x65=148 'You got "
        "the Hookshot!') was the ACTUAL mechanism behind the recorded 'divergence'. "
        "Any port passing an item id across games must map the ID, never the message "
        "— the message follows by formula on the correct side's id."),
    "item.47.correction": F(
        {"old_claim": "donor 47 -> 186 'Elixir Soup!' (WRONG)",
         "measured":  "donor 47 (0x2F HOOKSHOT) -> 148; 186 belongs to 0x55 SOUP_BOTTLE",
         "receiver_148": "was CORRECT donor arithmetic on the WRONG (TP) id"},
        "LAW",
        "supersedes the two rows that stood here (item.47.donor_message=186 / "
        "item.47.receiver_computed=148, sourced from bus §170/§171). Kept as a "
        "correction row rather than deleted, per this table's own rule: a table that "
        "stores only the right answer cannot prove it fixed anything. The clothes get "
        "id on the DONOR side is 0x32 FUKU -> msg 151 (INF1 3095); current "
        "clothes_bundle.ini get_text carries msg 601/INF1 334 (Telescope birthday "
        "text) — the live wrong-text root, owed to the W4 thread."),

    # --- §853 item 3 / §856: the port-inline mappings moved to rows (the
    # standing rule "History authors as rows, never inline in ported actors"
    # is now enforceable — these rows are the ones the landed actors carry
    # inline today; K3-era ports read HERE instead). ---------------------------
    "at_type.map": F(
        {"WW AT_TYPE_BOMB":         "AT_TYPE_BOMB (the ONE same-bit case beyond sword bit 1)",
         "WW AT_TYPE_UNK8 (thrown)": "AT_TYPE_THROW_OBJ (same bit 3, same semantic)",
         "WW sword family (SWORD|BOKO_STICK|MACHETE|UNK800|SPIKE|UNK2000|DARKNUT|MOBLIN)":
             "NORMAL_SWORD|MASTER_SWORD|WOLF_ATTACK|WOLF_CUT_TURN (player swings; enemy "
             "melee joins as WW enemies land)",
         "WW AT_TYPE_SKULL_HAMMER": "AT_TYPE_IRON_BALL (ball & chain — the blunt-crush class)",
         "WW AT_TYPE_WIND (leaf gust)": "AT_TYPE_BOOMERANG (gale boomerang — receiver's wind)",
         "WW AT_TYPE_HOOKSHOT":     "AT_TYPE_HOOKSHOT (bit 14 receiver-side)"},
        "HOST",
        "tsubo §805 [T2] damage matrix + damage_tg_acc, in-game since §816; trap #5 "
        "(only BOMB agrees) is the law behind the whole row"),
    "item.consumption": F(
        {"rupees g/b/y/r/p/o/s (0x01-06,0x0F)": "dComIfGp_setItemRupeeCount(1/5/10/20/50/100/200)",
         "heart 0x00 / triple 0x1E": "setItemLifeCount(4.0f/12.0f, 0) — quarter-heart unit, "
                                     "donor amounts transfer unscaled (health.unit row)",
         "magic small 0x09 / large 0x0A": "setItemMagicCount(8/32)",
         "arrows 0x10/0x11/0x12": "setItemArrowNumCount(10/20/30)",
         "bombs 0x0B-0x0E": "OWED — TP setItemBombNumCount needs the bag index "
                            "(d_item.cpp:2328); LOUD until wired",
         "unmapped ids": "LOUD warn, never silent (№31-C)"},
        "HOST",
        "ww_item §741 [I1] wwItm_execItemGet — receiver ops read from TP's own "
        "d_item.cpp:559/578/630/667, donor amounts verbatim"),
    "item.spawn_action_codes": F(
        {"1": "sword-hit launch (speedH/10, rnd yaw)",
         "2/4/9": "burst up (launch + rnd yaw, no H)",
         "3": "wall/roof pop (y 25, velScale H)",
         "7": "bomb burst (velScale*1.5 H)",
         "0xB": "placed (no motion)"},
        "LAW",
        "donor daItem action ids: tsubo damaged() passes them (1 sword/2 ground/3 wall-"
        "roof/7 bomb/9 other) and ww_item initAction §741 implements the same codes — "
        "the two ports agree because both read the donor"),
    "item.create_from_table": F(
        {"0x00-0x1F": "direct donor item id — spawn WW_ITEM with id + action code",
         "0x20-0x3E": "drop-table roll (life-scaled 0x21-0x24 band, rnd 16-slot tables) "
                      "— OWED: needs dComIfGp_getItemTable + getItemNoByLife port",
         "0x3F/0xFF": "NO DROP (donor early-return, f_op_actor_mng.cpp:893)"},
        "LAW",
        "donor fopAcM_createItemFromTable read whole (§805 [T3]); the 0x3F no-drop "
        "branch covers most furniture placements, in-game since §816"),
}

# Mappings the plan names but that are NOT yet measured. Listed, not guessed.
# Each line states WHERE the evidence lives, so filling it is a lookup rather
# than a research project.
INTERACTION_MAP_OWED = [
    # ALL FOUR MEASURED — list emptied 2026-08-10, not deleted (the empty list IS
    # the receipt that L4's measurement phase closed):
    # damage — 2026-08-10 -> damage.* rows (plumbing/index/values/soup).
    # wolf.damage — 2026-08-10 -> TP_INTO_WW wolf_link.damage_* rows (additions,
    #   not mappings — the multiplier question is answered IN the row).
    # item — 2026-08-10 -> item.get_message_formula / item.id_space /
    #   item.47.correction. The 'full correspondence' turned out to be a FORMULA
    #   shared by both games over DIFFERENT id spaces — the owed table does not
    #   exist to be measured, which is itself the measurement.
    # health — 2026-08-08 -> health.storage / health.unit.
]

# ---------------------------------------------------------------------------
# L4b — DIRECTION. The table above assumed ONE direction and the user corrected
# it: WW data → TP systems. Epona and WOLF LINK run the other way — receiver
# entities entering WW space — and that is not a mapping problem at all.
#
#   WW→TP  (rows above)  donor authored a value; the receiver needs its
#                        equivalent. A row states the correspondence. Owed,
#                        measurable, wrong-or-right.
#   TP→WW  (below)       the receiver has an entity the DONOR NEVER HAD. There
#                        is no donor value to converge on, so there is nothing
#                        to "map". These are ADDITIONS, not deviations.
#
# WHY THAT DISTINCTION EARNS ITS OWN SECTION: an addition cannot be faithful —
# it can only be COHERENT. The question is never "what did WW do here" (WW did
# nothing; the thing did not exist) but "does WW's world respond sensibly when
# it appears". That is a design judgement with a playtest, not a table lookup
# with a receipt. Filing it as a mapping row would put an unanswerable question
# on the owed list forever — the same trap L-10 exists to prevent, arriving from
# the opposite side.
#
# ALSO NOT port-liberties: that ledger tracks DEVIATIONS FROM WW VANILLA. An
# addition is not a deviation — nothing was changed away from the donor, because
# the donor had no position. Neither ledger currently owns these; naming them
# here is the honest interim, and where they finally live is the user's ruling.
# ---------------------------------------------------------------------------
TP_INTO_WW = {
    "wolf_link": F(
        "form with no donor counterpart", "HOST",
        "TP's wolf form entering WW content. NOT a mapping: WW has no wolf, so no donor "
        "value exists to converge on. What it DOES need, and what a damage table alone "
        "cannot supply: (a) wolf attacks have no WW weapon class — the L4 damage rows are "
        "keyed on weapons the wolf does not carry, so wolf-vs-WW-enemy needs its OWN rows, "
        "not a translation of Link's; (b) WW's attention/talk system was authored for a "
        "boy with a sword — a wolf addressing a WW NPC has no donor behaviour to be "
        "faithful to; (c) project memory records a suspected wolf damage MULTIPLIER, so "
        "any wolf row must state whether the multiplier is inside or outside it."),
    "wolf_link.damage_rows": F(
        {"jump/lunge (charged, finisher)": 3, "jump (mid-combo)": 2,
         "stand bite/scratch/tail (finisher)": 3, "stand (mid-combo)": 2,
         "roll/spin": 3, "midna_lock": 6, "howl_aoe (ALBW mod)": "8 -> pinned 100 at resolution",
         "down/pounce": "NO AT power (enemy-side down-attack path)"},
        "HOST",
        "measured 2026-08-10. All atp values are INLINE LITERALS at setCylAtParam call sites "
        "(d_a_alink_wolf.inc:7876/:7907/:7913/:8278/:8284/:8410/:8663/:3532) — no HIO table, "
        "no donor counterpart to converge on (the L4b point made concrete). Same plumbing as "
        "every other attack (mAtCyl + wolf-only mTgCyls[0..2], d_a_alink_cut.inc:249-267), so "
        "the values are in quarter-heart-compatible atp units and meet WW enemies unscaled."),
    "wolf_link.damage_multiplier": F(
        "OUTSIDE every row — hit-resolution only, and ONLY on midna_lock", "HOST",
        "the owed question answered: NO multiplier exists inside any wolf atp row (setCylAtParam "
        "body read whole, d_a_alink_cut.inc:224-273). The suspected multiplier is real and "
        "lives at cc_at_check d_cc_uty.cpp:589-612, gated on AT_TYPE_MIDNA_LOCK + wolf form + "
        "enemy group: twilight = base*rawMult^2*7/10, non-twilight = base*rawMult^2*25/100, "
        "floor 1. Rows 1-6+8 are AT_TYPE_WOLF_ATTACK/WOLF_CUT_TURN and are NEVER scaled by it. "
        "Also outside the rows: vanilla Master-Sword x2 / wood /2 are explicitly wolf-EXCLUDED "
        "(d_cc_uty.cpp:454-462), but getSwordAtUpTime x2 (:464-467) is NOT wolf-gated; art "
        "attacks (howl AoE, Midna arm) are pinned to 100 at resolution (:503-517), replacements "
        "not multipliers. Any wolf-vs-WW-enemy row derived later must cite THIS row's split of "
        "base-vs-resolution or it is ambiguous by construction."),
    "epona": F(
        "mount with no donor counterpart", "HOST",
        "TP's horse entering WW content (user: planned). WW's traversal answer is the boat "
        "(King of Red Lions), not a mount — so there is no donor row, and the real questions "
        "are coherence ones: WW island collision authored for a walking boy; WW's sea and "
        "shorelines; whether WW interiors admit her at all. Playtest questions, not table "
        "rows — recorded here so they are not mistaken for owed measurements."),
}

# ---------------------------------------------------------------------------
# CHUNK FORMAT PROVENANCE (Bridge, stood up 2026-08-11 per WAVE-1 row 13; §612)
# The property that killed an assumption one chunk after it was born: a baked
# arc's chunks are NOT uniformly donor-format. Format is a PER-CHUNK property
# of WHICH TOOL WROTE THE CHUNK, so every reader must DETECT before decoding.
# ---------------------------------------------------------------------------
CHUNK_FORMAT = {
    "provenance": F(
        "PER-CHUNK, by authoring tool — DETECT before decode, never assume", "LAW",
        "§612 receipt: F_DL01/R44_00.arc carries SCLS at donor stride (§610) AND RCAM at "
        "receiver stride (num=5, flag words zeroed, clean at every record) IN THE SAME ARC — "
        "RCAM was baked by host-stage tooling (not in bake_room_chunks.KNOWN_SIZE at all). "
        "A blanket donor-stride re-read would have CORRUPTED R44's cameras. Detection shape "
        "that works (wwRoom_translateRcam): candidate-stride plausibility (printable-ASCII-"
        "or-NUL cam_type); implausible at BOTH strides = ERROR and refuse, never guess."),
}

# ---------------------------------------------------------------------------
# TAG DIALECT (Bridge, stood up 2026-08-11 per WAVE-1 row 13; §689, §686 class)
# Donor flat tags vs the receiver's layered tag dialect — SAME CONCEPT, two
# spellings. SCOPE, stated so the table cannot overclaim: these rows assert the
# TAG correspondence ONLY ("above record layout entirely", §689). Record-layout
# equivalence per pair is UNMEASURED unless its own row exists elsewhere.
# ---------------------------------------------------------------------------
TAG_DIALECT = {
    "EnvR": F("Env0", "LAW", "§689 corrected read: donor dump vs staged stage.dzs (3,328 b, "
              "11 chunks, §615 builder-validated). Environment record set."),
    "Colo": F("Col0", "LAW", "same §689 dump pair. Color set."),
    "Pale": F("PAL0", "LAW", "same §689 dump pair. Palette set (staged x34)."),
    "Virt": F("VRB0", "LAW", "same §689 dump pair. Virt/skybox set (staged x33); the Winditor "
              "Virt palette-order row above is the RECORD-level companion."),
    "EVNT": F("REVT", "LAW", "known since the §273/§321 family, Phase 4 of the overlay "
              "checklist; restated here so the dialect table is complete in one place."),
}

# ---------------------------------------------------------------------------
# PLACEMENT NAME COLLISIONS (History measured 2026-08-11, §719; Bridge hosts)
# Trap #3's third face: enums (§396), item ids (item.id_space), now PLACEMENT
# NAMES. Donor and receiver l_objectName tables share 100 names; a donor-placed
# name binding a TP-native profile spawns a TP actor on WW params (the field
# symptom: TP rupees at WW item positions).
# ---------------------------------------------------------------------------
PLACEMENT_NAME = {
    "collision_partition": F(
        {"shared_names": 100,
         "intentional_port_routed": 28,   # receiver enum >= 0x31C (profile-register rule)
         "capture_candidates": 72,        # sub-0x31C receiver profile = TP-native or mixed
         "confirmed_mechanism": "item -> fpcNm_ITEM_e 0x218 (TP d_a_obj_item) = the rupees",
         "witem": "RECEIVER-ONLY name — cannot collide; donor itemFLY/itemDek are "
                  "receiver-absent and no-op"},
        "LAW",
        "donor WW DP d_stage.cpp l_objectName (825 rows) x receiver d_stage.cpp:632 (936 "
        "rows) x f_pc_name.h enum values. Loud Outset candidates: P2a-c (pigs) -> NPC_P2 "
        "0x2a9, Gnd (Grandma) -> NPC_GND 0x233, Mk -> NPC_MK 0x2a8, bonbori -> EP, "
        "Mhsg* -> Obj_Ladder, ky_tag0-3 -> TP kytags (donor ky_tag3 remaps to KYTAG01 in "
        "the receiver's own row), TagEv -> TAG_EVENT, Grass -> TP GRASS. Full listing in "
        "bus §719. THE DE-MOUNT COUPLING: mounted NPCs shield their names today; row-21 "
        "retirement hands names back to this table, so every capture candidate needs a "
        "disposition (port profile / no-op row / native adoption) BEFORE its shielding "
        "retires. Regenerate: intersect OBJNAME rows of both d_stage.cpp, resolve receiver "
        "enums via f_pc_name.h X-macro values, split at 0x31C."),
}

# ---------------------------------------------------------------------------
# code dialect  (R5 absorption item 1, tale §853; authored §854 from the port
# receipts of seven landed WW actors: item §741 · shutter §793 · tsubo §805/
# §816 · Jb1 §822 · shelf §835 · paper/plant §837-queued · plus §823/§843).
# KEY = the donor spelling as it appears in WW DP source; VALUE = the receiver
# form a port writes instead. HOST tier = receiver mapping decision; LAW tier =
# a donor-value law the receiver must carry unchanged. This is the K3 codemod's
# belief table (tale §849) — extend HERE, never a parallel format (§851/§853).
# ---------------------------------------------------------------------------
CODE_DIALECT = {
    # --- fpc / lifecycle -----------------------------------------------------
    "cPhs_State": F("cPhs_Step", "HOST",
        "receiver renamed the phase enum; every port (ls1 §244 note :12, Jb1 §822) swaps it"),
    "fopAcM_ct_Retail": F("fopAcM_ct", "HOST",
        "receiver has the single ctor macro; ls1:2509, Jb1 §822 [J3]"),
    "fopAcM_ct_Demo": F("(dropped — fopAcM_ct covers it)", "HOST",
        "demo fields live on fopAc in the receiver; ls1/Jb1 create paths carry no second ctor"),
    "dComIfG_resDeleteDemo": F("dComIfG_resDelete", "HOST",
        "receiver name; ls1:2489, Jb1 §822 [J3], paper §837 [Q-delete]"),
    "fopAcM_RegisterCreateID/DeleteID": F("(dropped)", "HOST",
        "no receiver equivalent; ls1/Jb1 delete both calls"),
    "ARRAY_SSIZE": F("ARRAY_SIZE or literal", "HOST", "ls1 note :12"),
    "(u32)this  [setUserArea]": F("(uintptr_t)this", "HOST",
        "64-bit receiver; ls1 note :12, plant §837 [P-userarea]"),
    # --- message system (§239 port dialect) ---------------------------------
    "fopMsgStts_*": F("fopMsg_MODE_*", "HOST",
        "WW mStatus statuses -> port mode enum; d_npc.cpp:697-706 is the canonical mapping site"),
    "msg_class::mStatus": F("msg_class::mode", "HOST", "same §239 receipt"),
    "fopMsgM_messageSet(no, &eyePos)": F("fopMsgM_messageSet(no, this, 1000)", "HOST",
        "port arity takes the talk ACTOR + 1000; d_npc.cpp:692 and every port call site"),
    # --- event system --------------------------------------------------------
    "fopAcM_orderSpeakEvent(this)": F("fopAcM_orderSpeakEvent(this, 0, 0)", "HOST",
        "port +priority,+flag; ls1:1245"),
    "dComIfGp_evmng_getMyStaffId(name)": F("getMyStaffId(name, this, 0)", "HOST",
        "port arity; ls1:1660"),
    "eventInfo.mCommand == dEvtCmd_INTALK_e": F("eventInfo.checkCommandTalk()", "HOST",
        "receiver accessor form; Jb1 §822 checkOrder"),
    "eventInfo.mCommand == dEvtCmd_INDEMO_e": F("eventInfo.checkCommandDemoAccrpt()", "HOST",
        "receiver accessor form; Jb1 §822 checkOrder"),
    "dDemo_setDemoData(this, flags, morf, arc)": F("dDemo_setDemoData(..., arc, 0, NULL, 0, 0)",
        "HOST", "port 8-arg (+GndMtrlSndId, +reverb); ls1:2324"),
    "mEventCut.setActorInfo2(\"X\", this)": F("setActorInfo2((char*)\"X\", this)", "HOST",
        "port takes char*; ls1:336"),
    # --- kankyo / tevstr -----------------------------------------------------
    "g_env_light.settingTevStruct": F("dKyWw_settingTevStruct", "HOST",
        "§406 WW feeders serve WW-host actors (TEV_TYPE_ACTOR and BG0..BG3 legs both); "
        "ls1:2378, tsubo/shelf/paper draws"),
    "g_env_light.setLightTevColorType": F("dKyWw_setLightTevColorType", "HOST", "same §406"),
    "tevStr.mRoomNo": F("tevStr.room_no", "HOST", "receiver field name; tsubo §816"),
    "tevStr.mEnvrIdxOverride": F("tevStr.YukaCol", "HOST",
        "receiver floor-poly-color field carries the donor semantic; tsubo §816"),
    "LIGHT_INFLUENCE.mPos/.mPower": F("mPosition/mPow", "HOST",
        "receiver field names; Jb1 §822 (d_kankyo.h:17-23)"),
    # --- collision / bg ------------------------------------------------------
    "dComIfG_Bgsp()->X": F("dComIfG_Bgsp().X", "HOST", "receiver returns a reference; ls1 note :13"),
    "GetTriPla(polyInfo) -> cM3dGPla*": F("GetTriPla(polyInfo, &plane) -> bool", "HOST",
        "receiver out-param form; tsubo reflect/walk §816, obj_carry:749 is the native shape"),
    "mCyl.MoveCAtTg(pos)": F("mCyl.SetC(pos)", "HOST", "receiver center-move; tsubo §816"),
    "SetAtVec/SetTgVec(cXyz::Zero)": F("pass a local mutable cXyz", "HOST",
        "receiver takes cXyz& (non-const); tsubo §816"),
    "cSAngle::Val(u16-bit-pattern)": F("Val((s16)x)", "HOST",
        "receiver overloads are s16/f32 only; tsubo §816 init_rot_throw"),
    "(J3DModelData*)dComIfG_getObjectRes(...)": F(
        "dExtNpcMount_acquireModelData[ByIndex](...)", "LAW",
        "DN-3: WW BDL/BDLM/BMDM members are NOT mount-parsed in this port — the raw cast is "
        "the §810-2/§814 crash class; the consume-time acquirer is the ONLY route. Anm types "
        "(BCK/BTP/BRK) ARE mount-parsed (d_resorce.cpp:536/:557) and may raw-fetch (§816)"),
    # --- draw ---------------------------------------------------------------
    "dComIfGd_setSimpleShadow2(..., 1.0f, NULL)": F(
        "dComIfGd_setSimpleShadow(..., 1.0f, dDlst_shadowControl_c::getSimpleTex())", "LAW",
        "the donor's trailing NULL selects WW's DEFAULT soft tex internally; receiver NULL "
        "renders an opaque quad (the §817-1 black square). getSimpleTex() IS the donor's own "
        "default argument — verified in the donor header (§823, Integrator-confirmed)"),
    "static BOOL nodeCallBack(J3DNode*, int)": F("static int cb(J3DJoint*, int)", "HOST",
        "receiver J3DJointCallBack signature; npc_tk:3200 is the native shape; write via "
        "mDoMtx_stack + cMtx_copy to J3DSys::mCurrentMtx (plant §837 correction)"),
    # --- particles (§843 LAW) ------------------------------------------------
    "dPa_name::ID_* (donor effect ids)": F("the DONOR'S numeric values, never the receiver enum",
        "LAW",
        "receiver enum reuses WW NAMES with TP VALUES (§396 lamp, §843 TUBOHAHEN: donor 0x0017 "
        "vs receiver 0x003). Port = local WW_ID_* defines from WW DP d_particle_name.h + an "
        "sWwCommon row per id (d_particle.cpp:1528). 0x8000-bit ids are SCENE-bank — a "
        "separate unported surface (§843)"),
    "new dPa_J3DmodelEmitter_c + addModelEmitter": F(
        "dComIfGp_particle_set(id,...,&dPa_modelEcallBack::getEcallback(),...) + "
        "dPa_modelEcallBack::setModel(7-arg int form)", "HOST",
        "the receiver's modelEcallBack IS the WW model-emitter system one generation later "
        "(becomeImmortalEmitter both ctors; iteration inverted into backend callbacks) — "
        "§827 lineage receipts, shatter confirmed in play §845. Guard the emitter NULL "
        "(donor's own check, d_a_tsubo.cpp:3100)"),
    # --- attention / status --------------------------------------------------
    "fopAc_Attn_ACTION_CARRY_e (toggle)": F("fopAc_AttnFlag_CARRY_e + fopAcM_OnCarryType(LIGHT)",
        "HOST",
        "receiver carry protocol (obj_carry idiom) — the attention flag alone does not make "
        "TP Link lift; tsubo §805 [T8]"),
    "attention_info.distances[fopAc_Attn_TYPE_CARRY_e]": F("[fopAc_attn_CARRY_e]", "HOST",
        "receiver index name; f_op_actor.h:128"),
    "fopAc_Attn_TALKFLAG_READ_e": F("0x40000000 (donor value, local define)", "LAW",
        "WW f_op_actor.h:88; receiver lacks the name — value is donor law (paper §837 [Q4])"),
    "fpcDwPi_<donor>_e (absent slots)": F("fpcDwPi_E_RD_e", "HOST",
        "the family's standing ground/ambient draw-prio choice (pig→paper, every port)"),
    # --- shared-signature services (§874 mechanism): the ADJUDICATION AUTHORITY
    # is port_deps.py's SERVICES registry (per-service, one-time, receipted);
    # rows here MIRROR rulings for port authors, never originate them. ---------
    "dKy_Sound_set (shared-signature service)": F("BINDABLE-VERBATIM — bind the receiver's",
        "LAW",
        "§874 adjudication: donor d_kankyo.cpp:3180 vs receiver — same algorithm "
        "line-for-line; binding TP's copy IS running WW's own code (shared "
        "ancestry). Unadjudicated services default to VERIFY-tier deps (№31-C); "
        "run port_deps.py --adjudicate <fn> before binding any new one"),

    # --- §877 returned-error class: shapes the compile RETURN proved -------
    "fopAcM_GetParamBit(param, shift, width)": F("fopAcM_GetParamBit(this, shift, width)", "HOST",
        "receiver f_op_actor_mng.h:188 takes the ACTOR (void*) and reads its "
        "param itself; donor passes the fetched param value. §877 compile "
        "return caught it in tag_so/tag_kb_item/npc_p1"),
    "fopAcM_orderOtherEvent2(actor, name, flag)": F(
        "fopAcM_orderOtherEvent(actor, name, 0, flag, 0)", "HOST",
        "receiver keeps only the 5-arg name-form (f_op_actor_mng.h:606); "
        "donor's 2-suffix 3-arg form is absent. npc_p1 §877 fix"),
    "dEvtFlag_NOPARTNER_e": F("local #define 0x01 (donor d_event.h:32)", "HOST",
        "receiver event system lacks the donor flag enum; value carried "
        "verbatim at the call site. npc_p1 §877 fix"),
    "camera_process_class (demo camera deref)": F(
        '#include "f_op/f_op_camera_mng.h"', "HOST",
        "receiver forward-declares only; any dComIfGp_getCamera()->mCamera "
        "use needs the full type include (ls1 §244 precedent, re-proven §877)"),
    "updateDL() on a plain McaMorf (donor: mpMorf->updateDL())": F(
        "modelCalc() → anim entries → dComIfGd_setList() → entryDL()", "HOST",
        "KEY IS THE DONOR CALL SHAPE, not the declaring signature (§928 K3 "
        "review: the old key 'mDoExt_McaMorf::updateDL()' could never match "
        "real donor text and K3 silently missed it). NOTE McaMorfSO DOES have "
        "updateDL, so this row is REVIEW-tier: check which morf class the "
        "member is before rewriting. "
        "§244 recipe #2 split, anim/btp entries MUST follow the calc or the "
        "material calc resets them (Aryll/Grandma blank-face bug). §880 npc_p1"),
    "dComIfGd_setShadow (10-arg donor form)": F(
        "13-arg receiver form + trailing (0, 1.0f, dDlst_shadowControl_c::getSimpleTex())", "HOST",
        "receiver adds 3 shadow-tex params; standard actor values per ls1:2347/"
        "d_a_kb/alink; tex = the §823 donor-default receipt. §880 npc_p1"),
    "dEvent_exception_c::getEventName special table": F(
        "WW table via dExtWwEvt_getArrivalEventName (WW-host-scoped)", "LAW",
        "SHARED INDEX SPACE, DIFFERENT TABLES: donor 13 entries ids 201-213 "
        "(five *_COMEBACK first), receiver 14 ids 201-214 (one). 12 of 13 WW "
        "ids resolved to the wrong TP event — arrival events that no Outset "
        "staff can man, so the player stuck inside an unendable event. §901"),
    "dStageType_BOSS_e": F("ST_BOSS_ROOM", "HOST",
        "same value (3) and same STType bitfield extract in both lineages; the "
        "donor's 207 SHUTTER_START->BS_SHUTTER_START boss rule ports as-is. §901"),
    "dEvDtStaff_c::mAdvance (advance counter)": F(
        "donor field @0x46 inside the staff record — NEVER a side array", "LAW",
        "donor writer and reader share ONE storage by design (init=2, =1 per "
        "advanceCut, wind-down >1->1 else 0). A module-static s_advance[] made "
        "the WW wind-down write where the receiver's getIsAddvance never read: "
        "274 adv=0 vs 2 adv=1. Also port the donor's INIT: TP's staff init "
        "never touches 0x42-0x4F, so the byte is parsed-file garbage. §912"),
    "first-sight staff seeding (s_seen + mAdvance=1)": F(
        "DELETE IT — donor advanceCutLocal has no seeding; init's mAdvance=2 is "
        "the mechanism", "LAW",
        "a reconstruction can be wrong in VALUE and TIMING at once: seeding 1 "
        "inside advanceCutLocal is eaten by the wind-down in the same call "
        "(1>1 false -> 0), where the donor arrives holding 2 and winds to 1 — "
        "the one TRUE frame that breaks the deadlock. Storage fixes cannot "
        "reveal this; only the donor's own sequence does. §916"),
    "daObj::HitEff_kikuzu": F("dExtTpost_HitEff_kikuzu shim (owed FX)", "HOST",
        "receiver daObj has no kikuzu sawdust helper; §253 no-op shim carries "
        "the call until the WW dPa obj-FX pass. obj_paper §877 fix"),
}

# ---------------------------------------------------------------------------
# object-name join key  (R5 absorption item 2, tale §853/§855; GENERATED table
# in conversion_db_objectname.py — regenerate via gen_objectname_data.py, never
# hand-edit). LAW tier: the donor's own l_objectName, verbatim, 825 rows,
# many-to-one preserved. Serves item 6 (placement-weighted join) from the same
# data. Runtime seam (dStage_searchName hook) stays L8/Housing.
# ---------------------------------------------------------------------------
try:
    from conversion_db_objectname import OBJECT_NAME_ROWS as _ONR
except ImportError:  # imported as tools.conversion_db
    from tools.conversion_db_objectname import OBJECT_NAME_ROWS as _ONR
OBJECT_NAME = {
    name: F(row, "LAW", "donor d_stage.cpp l_objectName, generated verbatim (§855)")
    for name, row in _ONR.items()
}

DB = {
    "chunk_size": CHUNK_SIZE,
    "code_dialect": CODE_DIALECT,
    "object_name": OBJECT_NAME,
    "placement_name": PLACEMENT_NAME,
    "startcode_alias": STARTCODE_ALIAS,
    "field_class": FIELD_CLASS,
    "particle_id": PARTICLE_ID,
    "virt_palette": VIRT_PALETTE,
    "retail_presence": RETAIL_PRESENCE,
    "interaction_map": INTERACTION_MAP,
    "tp_into_ww": TP_INTO_WW,
    "chunk_format": CHUNK_FORMAT,
    "tag_dialect": TAG_DIALECT,
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
    print("L4 INTERACTION MAPPINGS OWED (evidence location stated, not guessed)")
    for name, where in INTERACTION_MAP_OWED:
        print(f"   {name:10s} {where}")
    print()
    print("ABSORPTION LEDGER (origin -> table -> state)")
    for origin, tbl, state in ABSORBED:
        print(f"   {origin:32s} -> {tbl:16s} {state}")
    print("\nRULE: a consumer that treats OBSERVED as LAW will 'fix' the wrong side of a")
    print("disagreement. That is how §375 and §396 each cost a round. Check the tier.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
