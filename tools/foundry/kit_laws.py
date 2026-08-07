#!/usr/bin/env python3
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
RE_LINEAGE = re.compile(r"^//\s*KIT-LINEAGE:\s*"
                        r"(native-port|bridge-owed|host-plumbing)"
                        r"(?::(\S+))?\s*$", re.M)


def lineage(raw):
    m = RE_LINEAGE.search(raw)
    if not m:
        return (None, None)
    return (m.group(1), m.group(2))

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


LAWS = [
    ("1 LIGHTING CONTRACT", law1_lighting),
    ("2 RAW-BYTES / PARSE-ONCE", law2_rawbytes),
    ("3 NATIVE-ACTOR WW LEG", law3_wwleg),
    ("4 STATUS BIT 1", law4_statusbit),
    ("5 DONOR WIND", law5_wind),
]


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

        if lin is None:
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
    for pat in ("src/d/**/d_ext_*.cpp", "src/d/actor/d_a_ext_*.cpp",
                "src/d/actor/d_a_npc_ba1.cpp", "src/d/actor/d_a_obj_otble.cpp",
                "src/d/d_ext_tree.cpp"):
        out.extend(sorted(REPO.glob(pat)))
    return sorted(set(out))


def main():
    args = sys.argv[1:]
    failures_only = "--failures-only" in args
    args = [a for a in args if not a.startswith("--")]
    files = sweep_targets() if "--sweep" in sys.argv else [Path(a) for a in args]
    if not files:
        sys.exit("usage: kit_laws.py <file.cpp> ... | --sweep [--failures-only]")

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
