#!/usr/bin/env python3
# ============================================================================
# ERA WARNING (Foundry kit-family sweep, 2026-08-21 - user order: kits may
# still carry mount/bake/legs-era recipes; History saw remnants and this
# sweep CONFIRMED them family-wide).
#
# THE LAWS HERE INCLUDE MOUNT-ERA LAWS (RE_ACQUIRE accepts both
# dExtWwMount/dExtNpcMount spellings). They lint FORK-side kits
# correctly and MUST NOT be read as plugin law - a plugin TU matching
# RE_ACQUIRE is a defect, not a compliance.
#
# Era doctrine of record: actor_kit.py's banner + its era-split checklist
# ([FORK] vs [PLUGIN] columns) and the trace method at
# docs/state/ww-plugin-outset.md #How-ports-land. PLUGIN delivery never
# routes through dExtNpcMount / f_pc edits / files.cmake / model_replacements
# - those are FORK-era mechanisms a stock-dusklight user never receives.
# This banner marks the era; a DEEP retool of this kit is a named plate item,
# not implied by the banner's presence.
# ============================================================================
# ============================================================================
# kit_laws.py — §423: the sky-campaign's five laws, as an EXECUTABLE LINT.
#
# Housing Security ferried five laws for the porting kits (Foundry-Intake §423,
# from commit 9c62ae05a0 / §407-§420, user-verified). Laws written into a doc
# get cited; laws written into a linter get ENFORCED. This is the second form.
#
# Every check below reports one of:
#   PASS       the law is satisfied, and the evidence is named
#   VIOLATION  the law is broken, with the offending line
#   N/A        the law does not apply to this TU, and WHY it does not
#   UNKNOWN    the check could not decide  (№31-C: never a silent pass)
#
# N/A IS LOad-BEARING. A lint that shouts at every file teaches lanes to ignore
# it — the §389b calibration lesson. So each law states its own applicability
# test: the lighting contract cannot apply to an actor that draws no model, and
# saying so explicitly is different from staying quiet.
#
# Usage:
#   kit_laws.py <file.cpp> [<file.cpp> ...]
#   kit_laws.py --sweep            all src/d/**/d_*ext*.cpp + WW-port actors
#   kit_laws.py --sweep --failures-only
# Read-only. Exit 1 if any VIOLATION, 2 if any UNKNOWN and no violations.
# ============================================================================
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]

# ============================================================================
# §426 KIT-LINEAGE TAG — ratified by the user 2026-08-04.
#
#   // KIT-LINEAGE: native-port | bridge-owed[:ledger-ref] | host-plumbing
#
# GREP-EXACT AND NEVER INFERRED. The lint previously scored every TU as if it
# were a native port, which made "donor wind sourcing" a question for the mount
# infrastructure — noise. The obvious repair was to guess lineage from banner
# prose, and that is exactly the co-occurrence inference that made law 2 and
# law 3 produce false findings twice in one sitting. So lineage is DECLARED by
# the TU, or it is UNKNOWN. There is no third path.
#
# Semantics (user's ruling, verbatim intent):
#   native-port   all five laws scored normally
#   bridge-owed   scored, but reported OWED against the ledger ref — the
#                 standing directive is that bridges are labeled and kept on the
#                 owed list, never silently passed
#   host-plumbing donor-draw laws return N/A **with the lineage as the stated
#                 reason**, which is auditable in a way silence is not
#   (missing)     UNKNOWN — the gap cannot hide
# ============================================================================
# ============================================================================
# LINEAGE MATCHER REWRITTEN 2026-08-16 (Integrator's find, landed by the tool
# owner per their spec). The old alternation named THREE of the SIX in-tree
# values - `mixed` and `donor-port` were never in it, so **the gate could not
# see a single file declaring itself a DONOR-FAITHFUL PORT, the thing this
# project exists to produce.** And `^//` cannot match a line whose file opens
# with a UTF-8 BOM - which hid d_stage.cpp, the most-cited TU on the board.
# 18 of 107 declared files were invisible (17%).
#
# The shape of the fix is the lesson: match ANY family token and VALIDATE
# against the domain, so a new lineage value can never again be silently
# unseen - it surfaces as UNKNOWN-VALUE:<token>, loudly, instead of parsing
# as "no header". Absent and unrecognised are different facts (No.31-C).
# NOTE: the ratchet (ww_ratchet.py) never used this matcher - its substring
# count was tolerant all along, so the pinned baseline (TU 102) DOES NOT MOVE.
# ============================================================================
LINEAGE_DOMAIN = {"native-port", "bridge-owed", "host-plumbing",
                  "mixed", "donor-port"}
RE_LINEAGE = re.compile("^\ufeff?//"  # explicit escape - never a literal BOM
                        r"\s*KIT-LINEAGE:\s*"
                        r"([A-Za-z][\w-]*)"          # any family token
                        r"(?::(\S+))?"               # optional citation (:§N)
                        r"(?:\s*\([^)\n]*\))?\s*$",  # optional (qualifier)
                        re.M)


def lineage(raw):
    m = RE_LINEAGE.search(raw)
    if not m:
        return (None, None)
    fam = m.group(1)
    if fam not in LINEAGE_DOMAIN:
        # visible, never silent - and distinct from "no header"
        return ("UNKNOWN-VALUE:%s" % fam, m.group(2))
    return (fam, m.group(2))

# --- law 1: LIGHTING CONTRACT -----------------------------------------------
# Every WW actor draw routes dKyWw_settingTevStruct (donor-authored TEV type) ->
# dKyWw_setLightTevColorType. Never hand-write C0/K0. The §406 canary logs any
# unfed tevstr.
RE_MODEL_DRAW = re.compile(r"mDoExt_modelUpdateDL\s*\(|mDoExt_modelEntryDL\s*\(")
RE_SETTING_TEV = re.compile(r"dKyWw_settingTevStruct\s*\(")
RE_SET_LIGHT_TEV = re.compile(r"dKyWw_setLightTevColorType\s*\(")
RE_TP_SETTING_TEV = re.compile(r"g_env_light\.settingTevStruct\s*\(")
RE_TP_SET_LIGHT = re.compile(r"g_env_light\.setLightTevColorType\s*\(")
# Hand-written channel colour = the thing the contract exists to forbid.
RE_HAND_C0K0 = re.compile(r"GXSetTevColor\s*\(|GXSetTevKColor\s*\(|"
                          r"\bsetTevColor\s*\(\s*GX_TEVREG0|\bmTevKColor\b\s*=")

# --- law 2: CUSTOM-ARC MODELS = RAW J3D BYTES -------------------------------
# Never feed dComIfG_getObjectRes results to mDoExt_J3DModel__create; go through
# dExtWwMount_acquireModelData (parse-once cache). Two symbolicated crashes.
RE_J3D_CREATE = re.compile(r"mDoExt_J3DModel__create\s*\(")
RE_GET_OBJ_RES = re.compile(r"dComIfG_getObjectRes\s*\(")
# §426b — the law as ferried names `dExtWwMount_acquireModelData`, but the
# symbol that actually exists in this tree is `dExtNpcMount_acquireModelData`.
# Matching only the ferried spelling reported two COMPLIANT files as UNKNOWN.
# Both spellings accepted; the receiver's own symbol is the authority.
RE_ACQUIRE = re.compile(r"dExt(?:Ww|Npc)Mount_acquireModelData\s*\(")

# --- law 3: NATIVE-ACTOR WW-LEG PATTERN (vrbox precedent) -------------------
RE_PC_GUARD = re.compile(r"#if\s+TARGET_PC")
RE_SIZEOF_ASSERT = re.compile(r"static_assert\s*\(\s*sizeof|"
                              r"JUT_ASSERT\s*\([^)]*sizeof|"
                              r"CHECK_SIZE|STATIC_ASSERT\s*\(\s*sizeof")

# --- law 4: STATUS BIT 1 = "stage has a sky" --------------------------------
RE_STATUS_BIT1 = re.compile(r"\bstatus\b[^;\n]*\b(?:&|\|)\s*1\b|"
                            r"fopAcM_(?:On|Off|Check)Status\s*\([^,]+,\s*1\s*\)")
RE_CONTRACT_ASSERT = re.compile(r"§423|status bit 1|bit 1 =|has a sky", re.I)

# --- law 5: WIND ------------------------------------------------------------
# Hosts arm donor wind = tact-default (1,0,0) + authored FILI level (§416).
RE_WIND_SET = re.compile(r"dKyw_(?:get_)?wind|setWindVec|mWindVec|WIND_INFLUENCE")
RE_FILI = re.compile(r"\bFILI\b|getFiliLevel|fili", re.I)
RE_TACT_DEFAULT = re.compile(r"1\.0f\s*,\s*0\.0f\s*,\s*0\.0f|tact[- ]?default", re.I)


# --- laws 6-7: THE SURVIVING CRASH RECIPES (V5, §589) -----------------------
# From the CANONICAL cookbook's DIRECT-PORT CRASH RECIPES. Only 2 of the 9
# survived the SCOPE TEST -- a recipe is lintable iff its invariant lives
# entirely inside the scope the lint can see. Recipes 1, 2 and 5 all had a
# syntactic signature and were all FALSE on their first run, because their
# invariants span dataflow, a function boundary, and lexical distance
# respectively. These two span nothing.
RE_PTR_TRUNC = re.compile(r"\(\s*[us]32\s*\)\s*this\b|"
                          r"setUserArea\s*\(\s*\(\s*[us]32\s*\)")
RE_HIO_RAW = re.compile(r"\bmDoHIO_(?:create|delete)Child\s*\(")

def strip_comments(text):
    """Laws are about CODE. A banner that names a forbidden call must not read
    as a violation — that is how a linter trains people to delete comments."""
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    return text


def find_line(raw, pattern):
    for i, line in enumerate(raw.splitlines(), 1):
        if pattern.search(line):
            return i, line.strip()
    return None, None


def law1_lighting(raw, code):
    if not RE_MODEL_DRAW.search(code):
        return ("N/A", "this TU issues no model draw (no mDoExt_modelUpdateDL / "
                       "modelEntryDL), so there is no tevstr to feed")
    ww = RE_SETTING_TEV.search(code)
    wwlight = RE_SET_LIGHT_TEV.search(code)
    tp = RE_TP_SETTING_TEV.search(code)
    hand = find_line(raw, RE_HAND_C0K0)
    if hand[0] and not (ww and wwlight):
        return ("VIOLATION", f"hand-written channel colour at line {hand[0]}: "
                             f"{hand[1]!r} — the contract forbids writing C0/K0 "
                             f"by hand; route the donor-authored TEV type")
    if ww and wwlight:
        return ("PASS", "dKyWw_settingTevStruct -> dKyWw_setLightTevColorType")
    if ww and not wwlight:
        return ("VIOLATION", "dKyWw_settingTevStruct is called but "
                             "dKyWw_setLightTevColorType is not — the tevstr is "
                             "fed and then never applied (§406 canary would log "
                             "this as UNFED)")
    if tp:
        return ("VIOLATION", "draws via the TP lighting path "
                             "(g_env_light.settingTevStruct) — a WW actor draw "
                             "must route dKyWw_*, or its lighting is the "
                             "receiver's, not the donor's")
    return ("VIOLATION", "model draw with NO lighting route at all — §406 canary "
                         "will report UNFED tevstr")


def law2_rawbytes(raw, code):
    """§423c CALIBRATION — the first version was a FALSE POSITIVE GENERATOR.

    It flagged any file containing both `dComIfG_getObjectRes` and
    `mDoExt_J3DModel__create` and asserted a link between them. Its first and
    only "catch" was d_a_obj_otble.cpp:117 — which fetches a **DZB collision**
    resource, nothing to do with model data. Publishing that would have been the
    same error this campaign keeps paying for: asserting a connection that was
    never traced.

    So the check now follows the DATA. It resolves which variable each
    getObjectRes result is bound to, then asks whether that variable is what
    reaches mDoExt_J3DModel__create. No traced flow, no violation — UNKNOWN, and
    it says what a human has to confirm.
    """
    if not RE_J3D_CREATE.search(code):
        return ("N/A", "this TU never calls mDoExt_J3DModel__create")
    if RE_ACQUIRE.search(code):
        return ("PASS", "model data comes from dExtWwMount_acquireModelData "
                        "(parse-once cache)")

    # variables bound to a getObjectRes result
    bound = {}
    for i, line in enumerate(raw.splitlines(), 1):
        m = re.search(r"(\w+)\s*=\s*(?:\([^)]*\)\s*)?dComIfG_getObjectRes\s*\(", line)
        if m:
            bound[m.group(1)] = i

    for i, line in enumerate(raw.splitlines(), 1):
        m = re.search(r"mDoExt_J3DModel__create\s*\(\s*\(?[^,)]*?(\w+)", line)
        if not m:
            continue
        arg = m.group(1)
        if arg in bound:
            return ("VIOLATION",
                    f"'{arg}' is bound from dComIfG_getObjectRes at line "
                    f"{bound[arg]} and reaches mDoExt_J3DModel__create at line "
                    f"{i} — custom-mounted arcs serve RAW J3D bytes; two "
                    f"symbolicated crashes prove this path. Route it through "
                    f"dExtWwMount_acquireModelData")

    if bound:
        return ("UNKNOWN", f"getObjectRes results exist (lines "
                           f"{sorted(bound.values())}) but none was traced into "
                           f"mDoExt_J3DModel__create — they may be DZB/other "
                           f"resources. Confirm the model source by hand")
    return ("UNKNOWN", "calls mDoExt_J3DModel__create but its data source could "
                       "not be traced in this file — confirm it is not a "
                       "getObjectRes result before shipping")


def _pc_guard_inside_class(raw):
    """True only when a `#if TARGET_PC` sits INSIDE a class/struct body.

    §423b CALIBRATION — the first sweep flagged 8 files, and 8 of them were
    wrong. `#if TARGET_PC` is this codebase's ordinary platform guard; it appears
    in function bodies, includes and log lines everywhere. The vrbox pattern is
    much narrower: a TP same-lineage actor whose WW leg APPENDS FIELDS to the
    struct. So the applicability test is not "the file mentions TARGET_PC" — it
    is "a TARGET_PC guard occurs inside a type declaration". Crying wolf on a
    platform guard would have trained every lane to skip law 3, which is the
    §389b failure repeated.
    """
    depth = 0
    in_type = False
    type_depth = 0
    for line in raw.splitlines():
        s = line.strip()
        if not in_type and re.match(r"(class|struct)\s+\w+", s) and "{" in line:
            in_type, type_depth = True, depth
        if in_type and s.startswith("#if") and "TARGET_PC" in s:
            return True
        depth += line.count("{") - line.count("}")
        if in_type and depth <= type_depth:
            in_type = False
    return False


def law3_wwleg(raw, code):
    if not RE_PC_GUARD.search(code):
        return ("N/A", "no #if TARGET_PC in this TU")
    if not _pc_guard_inside_class(raw):
        return ("N/A", "TARGET_PC appears only as an ordinary platform guard, "
                       "not as PC-appended fields inside a type — this is not "
                       "the vrbox native-actor WW-leg pattern")
    if RE_SIZEOF_ASSERT.search(code):
        return ("PASS", "PC-appended fields inside a type, with a guarded "
                        "sizeof assert")
    return ("VIOLATION", "PC-appended fields inside a type but NO guarded "
                         "sizeof assert — the vrbox pattern requires the "
                         "appended layout to be size-asserted, or a drift "
                         "lands silently")


def law4_statusbit(raw, code):
    ln, txt = find_line(strip_comments(raw), RE_STATUS_BIT1)
    if not ln:
        return ("N/A", "this TU does not touch actor status bit 1")
    if RE_CONTRACT_ASSERT.search(raw):
        return ("PASS", f"status bit 1 used at line {ln} and the displaced "
                        f"contract is stated in-file")
    return ("VIOLATION", f"status bit 1 touched at line {ln}: {txt!r} — bit 1 "
                         f"means \"stage has a sky\" (daVrbox_Create only). Any "
                         f"host surface that suppresses a TP subsystem must "
                         f"assert the contract it displaced")


RE_WIND_READ = re.compile(r"dKyw_get_wind|getWindVec|GetWind")


def law5_wind(raw, code, lin=None):
    """§426.4 — the donor wind AUTHORITY is §416 on the WW tale bus: tact-default
    (1,0,0) + the authored FILI level (Winditor receipt sea/Room44 = 0 -> 0.3).

    The law says *hosts arm* donor wind. A native port that merely READS wind is
    downstream of that decision and has nothing to source — scoring it produced
    two UNKNOWNs that were never questions. Arming is the surface under law.
    """
    if not RE_WIND_SET.search(code):
        return ("N/A", "this TU does not arm or read wind")
    arms = not (RE_WIND_READ.search(code) and not RE_FILI.search(code))
    if lin == "native-port" and RE_WIND_READ.search(code) and not RE_FILI.search(code):
        return ("N/A", "native-port that READS wind only — the arming decision "
                       "belongs to the host surface (authority: §416)")
    if RE_FILI.search(raw) and RE_TACT_DEFAULT.search(raw):
        return ("PASS", "wind armed from tact-default (1,0,0) + authored FILI "
                        "level (authority: §416)")
    if RE_FILI.search(raw):
        return ("PASS", "references the authored FILI level; base vector is the "
                        "tact default per §416 (tact-default (1,0,0) + FILI, "
                        "Winditor receipt sea/Room44 = 0 -> 0.3)")
    if not arms:
        return ("N/A", "reads wind but does not arm it — §416 places the "
                       "sourcing duty on the arming host surface")
    return ("VIOLATION", "arms wind without an authored FILI level — hosts must "
                         "source donor wind from tact-default (1,0,0) + the "
                         "authored FILI level (§416); never invent ambient "
                         "constants")


def law6_ptrtrunc(raw, code, lin=None):
    """Crash recipe 3 — donor `setUserArea((u32)this)` chops the actor pointer
    on x86_64, and the joint callback then reads a wild address. Symbolicated on
    the pig port (§229-§234).

    Wholly local: a pointer truncated to 32 bits is wrong wherever it appears,
    so there is no distant guard that could make it right. That is why it
    survived the scope test when recipes 1, 2 and 5 did not."""
    m = RE_PTR_TRUNC.search(code)
    if not m:
        return ("PASS", "no 32-bit cast of a pointer")
    line = code[:m.start()].count(chr(10)) + 1
    return ("VIOLATION", f"line {line}: pointer truncated to 32 bits — use "
                         f"(uintptr_t). Donor casts are 32-bit by origin; audit "
                         f"every (u32)/(s32) cast of a pointer in ported code")


def law7_hio_macro(raw, code, lin=None):
    """Crash recipe 8 — raw `mDoHIO_createChild`/`deleteChild` reference the
    debug-only `mDoHIO_root`, which is UNLINKED in the retail PC build: LNK2019.
    Use the `mDoHIO_CREATE_CHILD` / `mDoHIO_DELETE_CHILD` macros, which no-op
    outside DEBUG.

    Also wholly local, and its failure is a LINK error rather than a runtime
    one -- so unlike most of this file it costs a build, not a playtest. Reading
    the HIO tuning FIELDS is fine; only register/unregister calls are under
    law."""
    m = RE_HIO_RAW.search(code)
    if not m:
        return ("PASS", "no raw mDoHIO child register/unregister call")
    line = code[:m.start()].count(chr(10)) + 1
    return ("VIOLATION", f"line {line}: raw mDoHIO child call — unlinked in the "
                         f"retail PC build (LNK2019). Use mDoHIO_CREATE_CHILD / "
                         f"mDoHIO_DELETE_CHILD; the HIO FIELDS stay readable")



# --- law 8: THE CACHE-KEY LAW (tale-bus routing, user-refined) ---------------
# "A cache may not outlive the scope in which its key is unique. Use the
#  engine's own identity and lifecycle; add a differentiator ONLY where the
#  engine does not already resolve it upstream."
# Born from the LinkRM/Ojhous aliasing: R00_00 is a POSITION, not an IDENTITY,
# and a session-immortal cache keyed on it served one stage's model over
# another's collision while every file-layer measurement read clean.
#
# SCOPE-TEST HONESTY: key uniqueness is a semantic property no regex can
# judge. What IS lintable is the DECLARATION: each registered cache site must
# carry a CACHE-KEY annotation naming (a) unique-in scope and (b) the
# invalidated-by event; absent = UNKNOWN, never clean (the S31-C shape, same
# as KIT-PLUGIN). Sites enter the registry by DECLARATION, never inference.
CACHE_SITES = {
    "src/d/d_ext_npc_mount.cpp": [
        "acquireStageModelData",   # AFFECTED site of the S772 bug (positional key)
        "acquireModelData",        # object arcs, globally-unique names
        "acquireBgModel",
        "releaseArcModels",
    ],
}
RE_STRKEY_CONTAINER = re.compile(
    r"(?:std::)?(?:unordered_)?map\s*<\s*(?:std::)?(?:w)?string\b")
RE_CACHE_ANN = re.compile(
    r"CACHE-KEY:\s*unique-in=\S+\s+invalidated-by=\S+")


def law8_cachekey(raw, code, lin=None):
    """Registered cache sites must DECLARE key scope + invalidation — and per
    the integrator audit: (1) ANY TU containing a string-keyed cache container
    is UNKNOWN when undeclared, registered or not (registry-only scope was
    this law's own silence-as-green); (2) ALL sites report, not the first."""
    findings = []
    for rel, fns in CACHE_SITES.items():
        for fn in fns:
            idx = code.find(fn)
            if idx < 0:
                continue
            back = code[max(0, idx - 1200):idx]
            if not RE_CACHE_ANN.search(back):
                findings.append(fn)
    if findings:
        return ("UNKNOWN",
                "cache sites lacking CACHE-KEY annotations (%d): %s — key "
                "lifetime UNDECLARED, not clean (S772 class)"
                % (len(findings), ", ".join(findings)))
    # audit item (1): string-keyed cache container in an UNREGISTERED TU
    m = RE_STRKEY_CONTAINER.search(code)
    if m and not RE_CACHE_ANN.search(code):
        line = code[:m.start()].count(chr(10)) + 1
        return ("UNKNOWN",
                "line %d: string-keyed cache container with NO CACHE-KEY "
                "annotation anywhere in the TU — undeclared cache, not clean "
                "(S772 class; declare unique-in= + invalidated-by= or register "
                "the site)" % line)
    return ("PASS", "no undeclared cache site or string-keyed container in this TU")


# --- law 9: DN-3 RAW-BUFFER CAST (tale §812, integrator-routed) --------------
# The tsubo crash class: `(J3DModelData*)dComIfG_getObjectRes(...)` in a
# WW-layer TU. For WW/custom arcs get*Res returns the RAW FILE BUFFER (DN-3:
# never parse at arc-mount), so the cast is a type lie the NULL check cannot
# catch — the pointer is valid, the TYPE is wrong (fault was 0xa000000, not 0).
# Sanctioned paths, both already in-tree: the consume-time acquirer family
# (d_a_knob00.cpp:188 dExtNpcMount_acquireModelData) or explicit raw-detect
# routing (ww_room_loader.cpp:574 memcmp(res, "J3D2", 4)). The two-step
# compliant shape naturally escapes this regex — only a cast applied DIRECTLY
# to the get*Res call fires. Scoped by lineage like laws 1-5: vanilla TP TUs
# use this exact idiom CORRECTLY (native arcs are pipeline-parsed).
# ONE regex for every cast spelling the corpus actually uses (donor tsubo
# alone uses THREE: C-style, C-style-on-parenthesized-call, static_cast):
#   (T*)dComIfG_getObjectRes(...)  ·  (T*)(dComIfG_getObjectRes(...))
#   static_cast<T*>(dComIfG_getObjectRes(...))
RE_J3D_GETRES_CAST = re.compile(
    r"(?:\(\s*(J3D\w+)\s*\*\s*\)|static_cast\s*<\s*(J3D\w+)\s*\*\s*>\s*\()"
    r"\s*\(?\s*dComIfG_get(?:Object|Stage)Res")
# Anm-type casts WHITELISTED on tale §816's receipt, verified against the
# switch itself: d_resorce.cpp loadResource parses BCKS/BCK (:536) and
# BTP/BTK/BPK/BRK/BLK/BVA (:557) through J3DAnmLoaderDataBase AT MOUNT — DN-3's
# raw-serving covers only the BDL/model family, so a J3DAnm* cast of get*Res
# receives PARSED data and is sound. Non-anm, non-ModelData J3D casts stay
# UNKNOWN (honest residue).
DN3_ANM_WHITELIST_PREFIX = "J3DAnm"


def law9_dn3_cast(raw, code, lin=None):
    """Direct cast of dComIfG_getObjectRes/getStageRes to a parsed-J3D type in
    a WW-layer TU. J3DModelData = the CONFIRMED crash class (tale §812) =
    VIOLATION; other J3D* parsed-type casts = the same lie structurally but
    unconfirmed = UNKNOWN, never silently green (№-31-C). ALL sites report."""
    sites = []
    for m in RE_J3D_GETRES_CAST.finditer(code):
        ty = m.group(1) or m.group(2)
        line = code[:m.start()].count(chr(10)) + 1
        if ty == "J3DModelData":
            sites.append(("VIOLATION", line, ty))
        elif ty.startswith(DN3_ANM_WHITELIST_PREFIX):
            continue  # anm parsed at mount (§816 receipt)
        else:
            sites.append(("UNKNOWN", line, ty))
    if not sites:
        return ("PASS", "no direct parsed-J3D cast of get*Res in this TU")
    worst = "VIOLATION" if any(s[0] == "VIOLATION" for s in sites) else "UNKNOWN"
    detail = "; ".join("line %d: (%s*)get*Res [%s]" % (ln, ty, v)
                       for v, ln, ty in sites)
    return (worst,
            "%s — get*Res returns the RAW buffer for WW/custom arcs (DN-3); "
            "route through dExtNpcMount_acquire* (knob00:188) or raw-detect "
            "memcmp J3D2 first (ww_room_loader:574); a null/validity guard is "
            "NOT a fix (tale §812)" % detail)


# --- law 10: DN-10-S SUBSTITUTION MARKS (tale §819, user-ratified) ----------
# "Substitution is evidence, not a technique" — a substitution marks the exact
# line where a donor system was not ported, and A COMMENT RECORDING IT DOES
# NOT LICENSE IT (rule 4; the setSimpleShadow2 comment rode from §253 to §819
# unquestioned while the user was looking at the black square it caused).
# This law reads COMMENTS — deliberately opposite to every other law here —
# because the substitution RECORD is the detector. Two nets:
#   (a) near-name pair in one comment (setSimpleShadow2 vs setSimpleShadow)
#   (b) substitution vocabulary beside an identifier
# The UNDOCUMENTED case has no comment to read; that is --pair's job.
RE_COMMENT = re.compile(r"//[^\n]*|/\*.*?\*/", re.S)
RE_CMT_IDS = re.compile(r"[A-Za-z_]\w{5,}")
RE_SUBST_VOCAB = re.compile(
    r"signatures?\s+diverge|substitut|stand-?in|in place of|->\s*port\b", re.I)
# Calibration (first run): `X_e -> port mode` enum/field mappings are the
# SANCTIONED consumption-boundary translation, not substitutions — the vocab
# net only fires when the comment names a FUNCTION-ish identifier (API-family
# prefix or verb-cased). The near-name pair net is unaffected.
RE_FNISH_ID = re.compile(
    r"\b(?:dComIfGd?_\w+|mDoExt_\w+|dKy\w+_\w+|fopAcM_\w+|"
    r"(?:set|get|add|create|delete|entry|draw)[A-Z]\w+)")


def law10_dn10s(raw, code, lin=None):
    """Substitution comments = unported donor systems, reported per site."""
    hits = []
    for m in RE_COMMENT.finditer(raw):
        c = m.group(0)
        line = raw[:m.start()].count(chr(10)) + 1
        # pair net calibration (first sweep): only FUNCTION-ish pairs — prose
        # naming two sibling ACTORS/EVENTS (daVrbox/daVrbox2, TALE_DEMO chain,
        # shutter/shutter2 gate rows) is discussion, not a substitution record.
        ids = [i for i in RE_CMT_IDS.findall(c) if RE_FNISH_ID.fullmatch(i)]
        pair = None
        for a in ids:
            for b in ids:
                if a != b and a.startswith(b) and a[len(b):].isdigit() \
                        and len(a) - len(b) <= 2:
                    pair = (a, b)
                    break
            if pair:
                break
        if pair:
            hits.append("line %d: comment pairs %s / %s — donor system %s is "
                        "the port target" % (line, pair[0], pair[1], pair[0]))
        elif RE_SUBST_VOCAB.search(c) and RE_FNISH_ID.search(c):
            hits.append("TRIAGE line %d: substitution vocabulary beside %s"
                        % (line, RE_FNISH_ID.search(c).group(0)))
    if not hits:
        return ("PASS", "no substitution marks in comments (undocumented "
                        "substitutions need --pair against the donor TU)")
    # tiering (№31-C): an explicit A→B function pair = VIOLATION (rule 4);
    # vocabulary alone (may describe a PAST/removed stand-in) = UNKNOWN, a
    # human triages — never silently green, never falsely red.
    verdict = ("VIOLATION" if any(not h.startswith("TRIAGE") for h in hits)
               else "UNKNOWN")
    return (verdict, "DN-10-S: " + "; ".join(hits) +
            " — a comment recording a substitution does not license it "
            "(tale §819 rule 4)")


LAWS = [
    ("1 LIGHTING CONTRACT", law1_lighting),
    ("2 RAW-BYTES / PARSE-ONCE", law2_rawbytes),
    ("3 NATIVE-ACTOR WW LEG", law3_wwleg),
    ("4 STATUS BIT 1", law4_statusbit),
    ("5 DONOR WIND", law5_wind),
    ("6 POINTER TRUNCATION", law6_ptrtrunc),
    ("7 HIO MACRO", law7_hio_macro),
    ("8 CACHE-KEY", law8_cachekey),
    ("9 DN-3 RAW-BUFFER CAST", law9_dn3_cast),
    ("10 DN-10-S SUBSTITUTION", law10_dn10s),
]


LINEAGE_FREE_LAWS = {"6 POINTER TRUNCATION", "7 HIO MACRO", "8 CACHE-KEY",
                     "10 DN-10-S SUBSTITUTION"}

DONOR_DRAW_LAWS = {"1 LIGHTING CONTRACT", "2 RAW-BYTES / PARSE-ONCE",
                   "3 NATIVE-ACTOR WW LEG"}


def check(path):
    raw = path.read_text(encoding="utf-8", errors="replace")
    code = strip_comments(raw)
    lin, ref = lineage(raw)
    rows = []
    for name, fn in LAWS:
        try:
            verdict, why = fn(raw, code, lin) if fn is law5_wind else fn(raw, code)
        except TypeError:
            verdict, why = fn(raw, code)

        # LINEAGE-FREE LAWS (§590). Laws 1-5 are DONOR CONTRACTS: they only mean
        # something for WW-layer code, so without a lineage tag their result is
        # not evidence. Laws 6-7 are not contracts — a pointer truncated to 32
        # bits and a raw mDoHIO child call are wrong in ANY TU, receiver or WW.
        # Gating them on lineage would silence them exactly where lineage is
        # undeclared, which is where a stray donor-shaped cast is most likely to
        # be sitting unnoticed.
        if lin is None and name not in LINEAGE_FREE_LAWS:
            if verdict in ("PASS", "N/A"):
                verdict, why = ("UNKNOWN",
                                "no // KIT-LINEAGE tag — the law cannot be scoped "
                                "to this TU, so its result is not evidence (§426)")
        elif lin == "host-plumbing" and name in DONOR_DRAW_LAWS and verdict != "PASS":
            verdict, why = ("N/A", f"host-plumbing: this TU is infrastructure, not "
                                   f"a donor actor draw — {why}")
        elif lin == "bridge-owed" and verdict == "VIOLATION":
            verdict = "OWED"
            why = (f"bridge-owed{':' + ref if ref else ''} — recorded against the "
                   f"ledger, not passed: {why}")
        rows.append((name, verdict, why))
    return rows, lin, ref


def sweep_targets():
    out = []
    # d_a_ww_*.cpp added tale §812: the tsubo TU that crashed was OUTSIDE the
    # sweep — every WW-ported actor lives under that name shape.
    for pat in ("src/d/**/d_ext_*.cpp", "src/d/actor/d_a_ext_*.cpp",
                "src/d/actor/d_a_ww_*.cpp",
                "src/d/actor/d_a_npc_ba1.cpp", "src/d/actor/d_a_obj_otble.cpp",
                "src/d/d_ext_tree.cpp"):
        out.extend(sorted(REPO.glob(pat)))
    return sorted(set(out))


# ============================================================================
# PRE-PORT MODES (tale §817, integrator-routed): the cast is the DONOR'S
# UNIVERSAL IDIOM (correct in WW, DN-3-forbidden here), so every future port
# carries the defect by default. --donor-precast turns that from a per-crash
# discovery into a WORKLIST emitted BEFORE the port lands. --pair catches the
# §817 substitution class (receiver fn standing in for a donor fn of
# near-identical name: setSimpleShadow vs donor setSimpleShadow2).
# ============================================================================
DONOR_ACTOR_SRC = Path(r"D:/XXXXXXX/WW DP/src/d/actor")
DONOR_SRC = Path(r"D:/XXXXXXX/WW DP/src")

RE_CALL_IDENT = re.compile(r"\b([A-Za-z_]\w{3,})\s*\(")

# ============================================================================
# NON-ACTOR TU RESOLUTION — added by the INTEGRATOR 2026-08-16, same defect and
# same rule as port_deps.resolve_donor_tu (Foundry's `port_preflight d_msg`
# diagnosis). The donor TU was COMPOSED under `src/d/actor/`, never searched
# for, so a system TU like `d_msg` reported `MISSING donor TU` — the tool's
# path stated as the donor's contents. A tool may only say MISSING about a
# place it LOOKED; otherwise the verdict is UNSEARCHED/N-A.
# Actor fast path is preserved, so actor output is byte-unchanged.
# ============================================================================
_DONOR_TU_CACHE = {}


def _unresolved(name):
    """A non-existent path standing in for a SEARCHED-AND-ABSENT TU, so the
    caller's `is_file()` branch still fires and prints the UNSEARCHED verdict
    with a usable name. Never used to imply the file lives here."""
    stem = name[:-4] if name.endswith(".cpp") else name
    return DONOR_ACTOR_SRC / (stem + ".cpp")


def resolve_donor_tu(name):
    """Donor TU path for an actor OR system TU; None = SEARCHED AND ABSENT."""
    stem = name[:-4] if name.endswith(".cpp") else name
    if stem in _DONOR_TU_CACHE:
        return _DONOR_TU_CACHE[stem]
    fp = DONOR_ACTOR_SRC / (stem + ".cpp")
    if not fp.is_file():
        fp = next(iter(sorted(DONOR_SRC.rglob(stem + ".cpp"))), None) \
            if DONOR_SRC.is_dir() else None
    _DONOR_TU_CACHE[stem] = fp
    return fp


def donor_precast(names):
    """Per-donor-TU list of parsed-J3D casts of get*Res = the sites a port MUST
    translate at the consumption boundary (acquirer family / by-index form).
    Anm-type casts are listed but marked mount-parsed (§816: sound to keep)."""
    files = ([resolve_donor_tu(n) or _unresolved(n) for n in names]
             if names else sorted(DONOR_ACTOR_SRC.glob("d_a_*.cpp")))
    rows = []
    for fp in files:
        if not fp.is_file():
            print("UNSEARCHED/N-A: no TU named '%s' anywhere under %s — the"
                  % (fp.name, DONOR_SRC))
            print("  donor tree WAS walked and this name is not in it. No cast")
            print("  measurement was made for it; the counts below EXCLUDE it")
            print("  and imply no zero. (Was 'MISSING donor TU', which read as")
            print("  a fact about the donor and was a fact about the path.)")
            continue
        code = strip_comments(fp.read_text(encoding="utf-8", errors="replace"))
        sites = []
        for m in RE_J3D_GETRES_CAST.finditer(code):
            ty = m.group(1) or m.group(2)
            line = code[:m.start()].count(chr(10)) + 1
            tag = ("anm:mount-parsed-ok" if ty.startswith("J3DAnm")
                   else "TRANSLATE" if ty == "J3DModelData" else "review")
            sites.append((line, ty, tag))
        if sites:
            rows.append((fp.name, sites))
    rows.sort(key=lambda r: -sum(1 for s in r[1] if s[2] == "TRANSLATE"))
    total = sum(1 for _, ss in rows for s in ss if s[2] == "TRANSLATE")
    for name, sites in rows:
        must = [s for s in sites if s[2] == "TRANSLATE"]
        det = " ".join(":%d(%s)" % (ln, ty if tag != "TRANSLATE" else ty)
                       for ln, ty, tag in sites if tag == "TRANSLATE")
        anm = sum(1 for s in sites if s[2] == "anm:mount-parsed-ok")
        print("%-32s %2d TRANSLATE %s%s" % (name, len(must), det,
              ("  (+%d anm ok)" % anm) if anm else ""))
    print("\n%d TU(s) with casts · %d TRANSLATE site(s) across the corpus" % (len(rows), total))
    out = REPO / "docs" / "state" / "ww-staging" / "donor-precast-census.md"
    with open(out, "w", encoding="utf-8", newline="\r\n") as f:
        f.write("# Donor pre-cast census (kit_laws --donor-precast, tale §817)\n\n")
        f.write("Every `(J3D*)dComIfG_get*Res` site in donor actor source. TRANSLATE =\n")
        f.write("J3DModelData, must route through the acquirer family at port time (DN-3);\n")
        f.write("anm types are mount-parsed receiver-side (§816) and keep the donor idiom.\n\n")
        for name, sites in rows:
            f.write("- **%s**: %s\n" % (name, " · ".join(
                ":%d %s [%s]" % s for s in sites)))
    print("artifact -> %s" % out)
    return 0


def pair_check(recv_path, donor_path=None):
    """§817 substitution class: receiver TU calls a function whose name is a
    donor-called function minus/plus a short trailing suffix (digits), while
    the donor TU never calls the receiver's variant. Suspects only — a human
    judges; the lint's job is that the pair is LOOKED AT."""
    rp = Path(recv_path)
    if donor_path is None:
        guess = rp.name.replace("d_a_ww_", "d_a_")
        dp = resolve_donor_tu(guess) or _unresolved(guess)
    else:
        dp = Path(donor_path)
    if not dp.is_file():
        print("UNSEARCHED/N-A: no TU named '%s' under %s (whole tree walked)"
              % (dp.name, DONOR_SRC))
        print("  — pass the donor path explicitly. Nothing was compared, so")
        print("  no name-gap result is implied either way.")
        return 2
    rc = strip_comments(rp.read_text(encoding="utf-8", errors="replace"))
    dc = strip_comments(dp.read_text(encoding="utf-8", errors="replace"))
    rcalls, dcalls = set(RE_CALL_IDENT.findall(rc)), set(RE_CALL_IDENT.findall(dc))
    sus = []
    # §819 net: same NAME called in both, receiver drops arguments (the donor's
    # dropped trailing params ARE the missing feature). Max-arity per site via
    # top-level comma count — a suspect list, not a proof.
    def max_arity(c, name):
        best = -1
        for m in re.finditer(r"\b%s\s*\(" % re.escape(name), c):
            depth, args, i = 1, 1, m.end()
            if c[i:i+1] == ")":
                args = 0
            while i < len(c) and depth:
                ch = c[i]
                if ch == "(":
                    depth += 1
                elif ch == ")":
                    depth -= 1
                elif ch == "," and depth == 1:
                    args += 1
                i += 1
            best = max(best, args)
        return best
    for n in sorted(rcalls & dcalls):
        ra, da = max_arity(rc, n), max_arity(dc, n)
        if 0 <= ra < da:
            sus.append((n + "() %d arg(s)" % ra, n + "() %d arg(s)" % da))
    for n in sorted(rcalls - dcalls):
        for d in dcalls:
            long, short = (n, d) if len(n) > len(d) else (d, n)
            if long != short and long.startswith(short) and long[len(short):].isdigit() \
                    and len(long) - len(short) <= 2:
                sus.append((n, d))
                break
    if not sus:
        print("PAIR %s vs %s: no near-name substitution suspects" % (rp.name, dp.name))
        return 0
    print("PAIR %s vs donor %s — %d SUSPECT(s):" % (rp.name, dp.name, len(sus)))
    for n, d in sus:
        print("  receiver calls %-28s donor calls %-28s — §817 class, verify"
              % (n + "()", d + "()"))
    return 1


# ============================================================================
# --donor-deps (tale §824, RESHAPED per tale §828): the NAME-GAP triager.
# §828's correction is this mode's constitution: "absent by name" is NOT
# "absent as a system" — a name miss has three causes and only one is an
# unported system: (a) genuinely unported, (b) renamed twin (WW's `2` suffix),
# (c) same capability under a different ARCHITECTURE (donor registry-object vs
# receiver callback: dPa_J3DmodelEmitter_c vs dPa_modelEcallBack — the false
# positive §824 itself produced). So this mode NEVER emits a verdict: every
# name miss is a NAME-GAP with the best mechanical evidence attached —
#   twin:<name>       receiver has a near-name (strip digits) — §828 rule 1,
#                     compare SIGNATURES by hand before concluding anything
#   family:<names>    receiver symbols in the same API family — rule 2,
#                     check for the capability under another architecture
#   res-header        dRes_INDEX_* — generated per port (Jb.h precedent)
#   enum-dialect      *_e — translation-table class, not a system
#   no-candidate      rules 1-2 found nothing — HIGHEST suspicion, still a
#                     question (№-31-C), never a finding
# ============================================================================
API_PREFIXES = ("dPa_", "dComIf", "dKy", "dDemo", "dEv", "dMsg", "fop", "mDo",
                "dBg", "cBg", "dCc", "dSv", "dStage_", "dRes", "dLib", "dAttention",
                "dMeter", "daPy", "dPn", "cM3d", "cLib", "cXyz", "csXyz", "cBgS",
                "JPA", "J3DPacket", "mDoExt_")
RE_TOKEN = re.compile(r"[A-Za-z_]\w{4,}")


def _receiver_tokens():
    # COMMENT-STRIPPED, and the first run proved why: setSimpleShadow2 and
    # dPa_J3DmodelEmitter_c live in receiver COMMENTS (§253 substitution
    # banners) while being ABSENT from receiver code — indexing raw text
    # declared §824's two headline systems "present".
    toks = set()
    for base in (REPO / "src", REPO / "include"):
        for fp in base.rglob("*"):
            if fp.suffix in (".cpp", ".h", ".hpp", ".inc") and fp.is_file():
                toks.update(RE_TOKEN.findall(strip_comments(
                    fp.read_text(encoding="utf-8", errors="replace"))))
    return toks


def _donor_api_deps(fp):
    """Project-API identifiers a donor TU depends on (its own actor-local
    names excluded by prefix heuristic)."""
    code = strip_comments(fp.read_text(encoding="utf-8", errors="replace"))
    stem = fp.stem  # d_a_tsubo -> daTsubo local-prefix guess
    local = "da" + "".join(w.capitalize() for w in stem.split("_")[2:])
    deps = set()
    for tok in RE_TOKEN.findall(code):
        if tok.startswith(local):
            continue
        if tok.startswith(API_PREFIXES) or tok.endswith("_c"):
            deps.add(tok)
    return deps


def _classify_gap(sym, rt, fam_cache):
    """§828 rules 1-2 as mechanical evidence. Returns a tier label."""
    if sym.startswith("dRes_INDEX_"):
        return "res-header"
    base = sym.rstrip("0123456789")
    if base != sym and base in rt:
        return "twin:%s" % base
    for suf in ("0", "1", "2"):
        if sym + suf in rt:
            return "twin:%s" % (sym + suf)
    if sym.endswith("_e"):
        return "enum-dialect"
    fam = sym.split("_")[0] + "_"
    if len(fam) >= 4:
        if fam not in fam_cache:
            fam_cache[fam] = sorted(x for x in rt if x.startswith(fam))
        pool = fam_cache[fam]
        if pool:
            import difflib
            # class symbols (_c) get a looser cutoff: §828's case (c) is an
            # ARCHITECTURE difference, so the names diverge more than a rename
            # does (dPa_J3DmodelEmitter_c vs dPa_modelEcallBack = 0.56).
            cutoff = 0.45 if sym.endswith("_c") else 0.6
            close = difflib.get_close_matches(sym, pool, n=3, cutoff=cutoff)
            if close:
                return "family:%s" % ",".join(close)
    return "no-candidate"


def donor_deps(args):
    census = args and args[0] == "--census"
    names = args[1:] if census else args
    rt = _receiver_tokens()
    fam_cache = {}
    files = ([resolve_donor_tu(n) or _unresolved(n) for n in names]
             if names else sorted(DONOR_ACTOR_SRC.glob("d_a_*.cpp")))
    demand = {}
    for fp in files:
        if not fp.is_file():
            print("UNSEARCHED/N-A: no TU named '%s' anywhere under %s — the"
                  % (fp.name, DONOR_SRC))
            print("  donor tree WAS walked and this name is not in it. No cast")
            print("  measurement was made for it; the counts below EXCLUDE it")
            print("  and imply no zero. (Was 'MISSING donor TU', which read as")
            print("  a fact about the donor and was a fact about the path.)")
            continue
        absent = sorted(d for d in _donor_api_deps(fp) if d not in rt)
        tiers = {a: _classify_gap(a, rt, fam_cache) for a in absent}
        for a in absent:
            demand.setdefault(a, []).append(fp.stem)
        if not census:
            if absent:
                print("%-28s %d NAME-GAP(s) (evidence attached — §828: "
                      "never verdicts):" % (fp.name, len(absent)))
                for a in absent:
                    print("    %-44s [%s]" % (a, tiers[a]))
            else:
                print("%-28s all project-API deps present receiver-side" % fp.name)
    if census:
        rows = sorted(demand.items(), key=lambda kv: -len(kv[1]))
        out = REPO / "docs" / "state" / "ww-staging" / "absent-systems-census.md"
        with open(out, "w", encoding="utf-8", newline="\r\n") as f:
            f.write("# Donor NAME-GAP census, ranked by demand "
                    "(kit_laws --donor-deps --census; §824 reshaped by §828)\n\n")
            f.write("Donor project-API symbols with no same-name receiver definition. A NAME\n")
            f.write("MISS IS NOT A FINDING (tale §828): each row carries mechanical evidence\n")
            f.write("for §828's rules — twin: = renamed candidate (compare signatures);\n")
            f.write("family: = same-API-family symbols (check for the capability under a\n")
            f.write("different architecture); no-candidate = highest suspicion, still a\n")
            f.write("question. Demand rank = triage priority, and the planner demand axis.\n\n")
            for sym, users in rows:
                f.write("- **%s** [%s] — %d TU(s): %s\n"
                        % (sym, _classify_gap(sym, rt, fam_cache), len(users),
                           ", ".join(sorted(users)[:12])
                           + (" …" if len(users) > 12 else "")))
        for sym, users in rows[:20]:
            print("%-44s %3d donor TU(s)  [%s]"
                  % (sym, len(users), _classify_gap(sym, rt, fam_cache)))
        print("\n%d name-gap(s) · artifact -> %s" % (len(rows), out))
    return 0


def main():
    if len(sys.argv) >= 2 and sys.argv[1] == "--donor-deps":
        sys.exit(donor_deps(sys.argv[2:]))
    if len(sys.argv) >= 2 and sys.argv[1] == "--donor-precast":
        sys.exit(donor_precast(sys.argv[2:]))
    if len(sys.argv) >= 3 and sys.argv[1] == "--pair":
        sys.exit(pair_check(sys.argv[2], sys.argv[3] if len(sys.argv) > 3 else None))
    args = sys.argv[1:]
    failures_only = "--failures-only" in args
    args = [a for a in args if not a.startswith("--")]
    files = sweep_targets() if "--sweep" in sys.argv else [Path(a) for a in args]
    if not files:
        sys.exit("usage: kit_laws.py <file.cpp> ... | --sweep [--failures-only] | --donor-precast [actor ...] | --donor-deps [--census] [actor ...] | --pair <recv.cpp> [donor.cpp]")

    n_viol = n_unk = n_owed = 0
    for f in files:
        if not f.exists():
            print(f"{f}: MISSING")
            continue
        rows, lin, ref = check(f)
        bad = [r for r in rows if r[1] in ("VIOLATION", "UNKNOWN", "OWED")]
        n_viol += sum(1 for r in rows if r[1] == "VIOLATION")
        n_owed += sum(1 for r in rows if r[1] == "OWED")
        n_unk += sum(1 for r in rows if r[1] == "UNKNOWN")
        if failures_only and not bad:
            continue
        print(f"\n=== {f.relative_to(REPO) if REPO in f.parents else f}")
        for name, verdict, why in rows:
            if failures_only and verdict not in ("VIOLATION", "UNKNOWN", "OWED"):
                continue
            print(f"  [{verdict:9s}] law {name}")
            print(f"              {why}")
    print(f"\n§423 lint: {len(files)} file(s), {n_viol} VIOLATION, {n_unk} UNKNOWN")
    return 1 if n_viol else (2 if n_unk else 0)


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
