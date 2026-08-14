#!/usr/bin/env python3
# ============================================================================
# port_deps.py — the SYSTEM-DEPENDENCY GATE (user-ordered 2026-08-13 after the
# palm nearly passed the build gate twice).
#
# THE HOLE IT CLOSES: --donor-deps catches SYMBOL gaps, but the palm's wind
# dependency slipped straight through it — the receiver HAS dKyw_get_wind_vec
# and dKyw_get_wind_pow (TP's own wind), so no name-gap fires; what is missing
# is the WW wind SYSTEM being armed behind those symbols for WW hosts (the
# kit_laws law-5 / §416 contract). SYMBOL-PRESENT-BUT-SYSTEM-UNARMED is
# invisible to name matching, by construction. Systems must be DECLARED.
#
# THE MECHANISM (systemic, instance-independent):
#   1. A REGISTRY of donor systems: signature symbols -> system -> STATUS.
#      Statuses move ONLY by bus receipt (the §N that ruled it), never by
#      inference. Absent from registry = UNKNOWN = held (№31-C).
#   2. `check <donor actor>`: scan the donor TU for signature hits; any hit
#      whose system is not PORTED/DESCENDANT is a FINDING — the actor cannot
#      port verbatim-correct until the system does (or the system row rides
#      the SAME batch).
#   3. FAMILY NET: any dKyw_/dPa_/dKy_ symbol in the TU that no registry row
#      covers = "unregistered system-family symbol" — register before porting.
#   4. `--unlocks <SYSTEM>`: the REVERSE index — every donor actor TU gated on
#      that system (+ its demand), so when a system ports, its beneficiaries
#      ride the same wave instead of being rediscovered one crash at a time.
#
# Wired into port_preflight (section [DEPS]) and the BUILD-QUEUE DEP-GATE rule.
# Read-only. Exit 0 clean · 1 findings · 2 could-not-run.
# ============================================================================
import re
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
DONOR_ACTOR_SRC = Path("D:/XXXXXXX/WW DP/src/d/actor")

# ---------------------------------------------------------------------------
# THE REGISTRY. Every status carries its receipt (§N). Update BY RULING ONLY.
# ---------------------------------------------------------------------------
SYSTEMS = {
    "WW-WIND": {
        "signatures": ["dKyw_get_wind_vec", "dKyw_get_wind_pow",
                       "dKyw_get_wind_power", "dKyw_pntwind"],
        "status": "PRESENT-UNARMED",
        "receipt": "kit_laws law 5 / §416 — TP symbols exist; WW hosts must arm "
                   "tact-default (1,0,0) + authored FILI level; not yet armed "
                   "globally. THE PALM CASE (2026-08-13, caught at gate twice).",
    },
    "SHADOW-SIMPLE2": {
        "signatures": ["dComIfGd_setSimpleShadow2"],
        "status": "PORTED",
        "receipt": "tale §823 (History) + §828 ruling: renamed twin w/ default-arg "
                   "translation at the boundary.",
    },
    "MODEL-EMITTER": {
        "signatures": ["dPa_J3DmodelEmitter_c", "dComIfGp_particle_addModelEmitter"],
        "status": "DESCENDANT",
        "receipt": "tale §827: receiver dPa_modelEcallBack IS the donor system's "
                   "descendant (lifecycle receipts); consume via setModel idiom.",
    },
    "RES-LIFETIME-DEMO": {
        "signatures": ["dComIfG_resDeleteDemo"],
        "status": "EQUIVALENT-RETAIL",
        "receipt": "tale §872 (Integrator) VERIFIED by Foundry against the donor "
                   "header (d_com_inf_game.h:4025-4031): a BUILD-VARIANT MACRO — "
                   "retail branch expands to dComIfG_resDelete exactly; receiver "
                   "call sites reproduce the donor's own retail expansion "
                   "verbatim. Never needs porting; DEMO branch stays visible "
                   "here for any future demo-select content. Unblocks 134 TUs.",
    },
    "WW-DEMO-CAMERA": {
        "signatures": ["SkipSmoother", "dDemo_actor_c", "ENABLE_TRANS_e",
                       "ENABLE_ROTATE_e"],
        "status": "PORTED",
        "receipt": "ADJUDICATED (CALLS row, tale §880 triage; Foundry "
                   "tree-verified): the surfaces EXIST and are LOAD-BEARING — "
                   "dDemo_actor_c + ENABLE_TRANS/ROTATE_e in d/d_demo.h (the "
                   "established cutscene-shim layer, 8+ NPC binders incl. "
                   "ba1/bm1/jb1/ls1) and SkipSmoother as the §879 latch shim "
                   "dExtWwCam_SkipSmoother (adjudicated no-op, §880). npc_p1 "
                   "wired 13:26 and COMPILES. Original ABSENT was filed from a "
                   "bad call the caller retracted. Consume idiom: the shim "
                   "family, not donor spellings.",
    },
    "OBJ-EFF-SMOKE": {
        "signatures": ["daObjEff"],
        "status": "OWED",
        "receipt": "tale §816 owed list: donor break/land smoke companion actor "
                   "unported; JPA half of each break fires without it (LOUD).",
    },
    "WW-LIGHTING": {
        "signatures": ["dKy_plight_set", "dKy_plight_cut"],
        "status": "PORTED",
        "receipt": "§687 setLight engine + §694 TevColor/TevKColor (tale §809 "
                   "ruling: bake normalizers retired BY these ports).",
    },
}

# ---------------------------------------------------------------------------
# SERVICES — the §868 STANDING BOUNDARY RULING (Foundry, user-confirmed default).
# "Does a shared-signature engine service count as depending on a TP system?"
#
# THE RULING: the boundary is NOT a category list — it is a PER-SERVICE
# ADJUDICATION with a strict default:
#   DEFAULT (the user's provisional YES, confirmed): an unadjudicated
#     shared-signature service COUNTS as a dependency — VERIFY tier, never
#     silently bindable (№-31-C).
#   ADJUDICATION = compare donor impl vs receiver impl (both decomps in hand;
#     `--adjudicate <fn>` surfaces both bodies + fingerprint):
#     IDENTICAL-ALGORITHM  -> BINDABLE-VERBATIM. Binding TP's copy IS running
#                             WW's own code (shared SSystem ancestry). There is
#                             no "TP system" distinct from WW's here — the
#                             user's parallel-systems rule is satisfied by
#                             identity, not violated by binding.
#     DIVERGENT            -> SYSTEM row (WW's runs parallel — the WIND case).
#     DIVERGENT-TRANSLATABLE-> SEAM row (the setSimpleShadow2 shape).
# Rows move BY RECEIPT ONLY, one service at a time.
# ---------------------------------------------------------------------------
SERVICES = {
    "dKy_Sound_set": {
        "status": "BINDABLE-VERBATIM",
        "receipt": "tale §874 adjudication: donor d_kankyo.cpp:3180 vs receiver "
                   "d_kankyo.cpp — same algorithm line-for-line (camera-distance "
                   "gate, 1500.0f threshold, priority rule, SND_INFLUENCE writes); "
                   "only decomp field NAMES differ. The §868 test case.",
    },
}

# Family prefixes whose symbols imply SOME system — unregistered hit = UNKNOWN.
FAMILY_PREFIXES = ("dKyw_", "dPa_", "dKy_")

RE_TOKEN = re.compile(r"[A-Za-z_]\w{4,}")


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def donor_tokens(actor):
    fp = DONOR_ACTOR_SRC / (actor if actor.endswith(".cpp") else actor + ".cpp")
    if not fp.is_file():
        return None
    return set(RE_TOKEN.findall(strip_comments(
        fp.read_text(encoding="utf-8", errors="replace"))))


def check(actor, quiet=False):
    toks = donor_tokens(actor)
    if toks is None:
        print("  DEPS: donor TU not found: %s" % actor)
        return 2
    findings = 0
    covered = set()
    for name, row in SYSTEMS.items():
        hits = [s for s in row["signatures"] if s in toks]
        if not hits:
            continue
        covered.update(hits)
        ok = row["status"] in ("PORTED", "DESCENDANT", "EQUIVALENT-RETAIL")
        tag = "ok  " if ok else "HOLD"
        if not ok:
            findings += 1
        if not quiet or not ok:
            print("  [%s] %-18s %-16s via %s" % (tag, name, row["status"], ", ".join(hits)))
            if not ok:
                print("         %s" % row["receipt"])
    for svc, row in SERVICES.items():
        if svc in toks:
            covered.add(svc)
            ok = row["status"] == "BINDABLE-VERBATIM"
            print("  [%s] service %-18s %-16s — %s"
                  % ("ok  " if ok else "HOLD", svc, row["status"], row["receipt"][:90]))
            if not ok:
                findings += 1
    loose = sorted(t for t in toks
                   if t.startswith(FAMILY_PREFIXES) and t not in covered
                   and not any(t in r["signatures"] for r in SYSTEMS.values()))
    if loose:
        # Calibration (first run): dPa_control_c/dKy_Sound_set are the SHARED
        # SPINE's own API — receiver-present family symbols are the WIND trap
        # candidates (present-but-maybe-unarmed) = UNKNOWN/verify; symbols
        # absent from the receiver entirely = unknown SYSTEM = HOLD. Both are
        # findings (№-31-C: neither is silently green).
        sys.path.insert(0, str(HERE))
        import kit_laws
        rt = kit_laws._receiver_tokens()
        absent = [x for x in loose if x not in rt]
        present = [x for x in loose if x in rt]
        if absent:
            findings += 1
            print("  [HOLD] UNREGISTERED + receiver-ABSENT symbol(s): %s" % ", ".join(absent[:8]))
            print("         an unknown donor SYSTEM — register it (with receipt) first")
        if present:
            findings += 1
            print("  [VERIFY] unregistered family symbol(s), receiver-PRESENT: %s"
                  % ", ".join(present[:8]))
            print("         the WIND-trap shape — confirm armed-for-WW or register the system")
    if findings == 0 and not quiet:
        print("  DEPS clean — every touched system PORTED/DESCENDANT")
    return 1 if findings else 0


def unlocks(system):
    row = SYSTEMS.get(system)
    if not row:
        print("unknown system %r — registry rows: %s" % (system, ", ".join(SYSTEMS)))
        return 2
    users = []
    for fp in sorted(DONOR_ACTOR_SRC.glob("d_a_*.cpp")):
        toks = set(RE_TOKEN.findall(strip_comments(
            fp.read_text(encoding="utf-8", errors="replace"))))
        if any(s in toks for s in row["signatures"]):
            users.append(fp.stem)
    print("%s [%s] unlocks %d donor actor TU(s):" % (system, row["status"], len(users)))
    for u in users:
        print("  " + u)
    print("\nWhen this system ports, these are its BENEFICIARIES — they ride the")
    print("same wave (re-preflight each) instead of failing one crash at a time.")
    return 0


def _find_fn(root, fn):
    for fp in Path(root).rglob("*.cpp"):
        txt = fp.read_text(encoding="utf-8", errors="replace")
        m = re.search(r"^[\w\*&:<>\s]+\b%s\s*\([^;{]*\)\s*\{" % re.escape(fn), txt, re.M)
        if m:
            depth, i = 1, txt.index("{", m.start()) + 1
            while i < len(txt) and depth:
                depth += {"{": 1, "}": -1}.get(txt[i], 0)
                i += 1
            return fp, txt[m.start():i]
    return None, None


def adjudicate(fn):
    """Surface both implementations + structural fingerprint. VERDICT IS THE
    LANE'S — the tool presents evidence (control-flow tokens + literals; the
    field-NAME differences between decomps make text-diff useless)."""
    dfp, dbody = _find_fn("D:/XXXXXXX/WW DP/src", fn)
    rfp, rbody = _find_fn(str(HERE.parents[1] / "src"), fn)
    print("ADJUDICATE %s" % fn)
    for tag, fp, body in (("DONOR", dfp, dbody), ("RECEIVER", rfp, rbody)):
        print("--- %s: %s" % (tag, fp if fp else "UNRESOLVED-LOOKUP (finder failed; NOT proof of absence - No31-C)"))
        if body:
            fp_toks = re.findall(r"\b(?:if|else|for|while|return|switch)\b|"
                                 r"[0-9]+\.[0-9]+f?|[<>=!]=|&&|\|\|", body)
            print("    fingerprint(%d): %s" % (len(fp_toks), " ".join(fp_toks[:40])))
            print(body[:900])
    print("")
    print("RULE: identical algorithm -> BINDABLE-VERBATIM row; divergent ->")
    print("SYSTEM row; translatable -> SEAM row. Add to SERVICES with receipt.")
    return 0


RE_SOLIDITY = {
    "bg-web (dBgW MoveBG)": r"dBgW_NewSet|dBgW::MOVE_BG|MoveBGCreate",
    "bg-registration (dBgS)": r"dBgS_MoveBgActor|RegistBox|dBgS_Bg\w*Regist",
    "cc-push (cCcD cylinder/sphere)": r"cCcD_Stts|dCcD_Cyl|dCcD_Sph|mStts\.Init",
    "arc-dzb token": r"dRes_INDEX_\w+_DZB_\w+",
}


def solidity(actor, recv=None):
    """CALLS-row ask (tale §884 era): 'is this actor SOLID in the donor, and
    is that solidity present here?' Donor sources scanned by REGISTRATION
    family (the lwood miss: collision was searched in the cc family while the
    donor used dBgW MoveBG from its OWN ARC's dzb member); disc arc checked
    for .dzb members; receiver TU compared when given."""
    import re as _re
    fp = DONOR_ACTOR_SRC / (actor if actor.endswith(".cpp") else actor + ".cpp")
    if not fp.is_file():
        print("  SOLIDITY: donor TU not found")
        return 2
    code = strip_comments(fp.read_text(encoding="utf-8", errors="replace"))
    print("SOLIDITY %s" % actor)
    sources = []
    for name, pat in RE_SOLIDITY.items():
        if _re.search(pat, code):
            sources.append(name)
            print("  donor source: %s" % name)
    m = _re.search(r'arcname\w*\[\d*\]\s*=\s*"(\w+)"', code)
    arcname = m.group(1) if m else None
    dzbs = []
    if arcname:
        try:
            import ww_disc as wd
            import json
            import os
            pth = os.environ.get("WW_ISO", "") or json.load(open(os.path.expandvars(
                r"%APPDATA%\TwilitRealm\Dusklight\config.json"),
                encoding="utf-8")).get("backend.extraIsoPath", "")
            f, boot = wd.iso_open(pth, wd.WW_IDS)
            raw = wd.iso_read_file(f, boot, "res/Object/%s.arc" % arcname)
            dzbs = [nm for nm, _d in wd.rarc_list(raw) if nm.lower().endswith(".dzb")]
            print("  disc arc %s.arc dzb member(s): %s" % (arcname, dzbs or "none"))
        except Exception as e:
            print("  disc arc check UNRESOLVED (%s) — not proof of absence" % e)
    if not sources and dzbs:
        print("  [VERIFY] arc carries a dzb but no donor registration matched — a")
        print("           shared system (d_tree/d_wood) may own it; read the donor.")
    if not sources and not dzbs:
        print("  donor appears NON-SOLID (no registration family, no arc dzb) —")
        print("  walk-through may be DONOR-FAITHFUL; verify in WW before 'fixing'.")
    if recv:
        rp = Path(recv)
        if rp.is_file():
            rcode = strip_comments(rp.read_text(encoding="utf-8", errors="replace"))
            dropped = [n for n, pat in RE_SOLIDITY.items()
                       if _re.search(pat, code) and not _re.search(pat, rcode)]
            if dropped:
                print("  [HOLD] SOLIDITY DROPPED in port: %s — the donor registers"
                      % ", ".join(dropped))
                print("         it; the receiver TU does not. Walk-through is the symptom.")
                return 1
            print("  receiver TU carries every donor solidity source — OK")
        else:
            print("  receiver TU not found: %s" % recv)
            return 2
    return 0


def main():
    if len(sys.argv) >= 3 and sys.argv[1] == "--solidity":
        return solidity(sys.argv[2], sys.argv[3] if len(sys.argv) > 3 else None)
    if len(sys.argv) >= 3 and sys.argv[1] == "--adjudicate":
        return adjudicate(sys.argv[2])
    if len(sys.argv) >= 3 and sys.argv[1] == "--unlocks":
        return unlocks(sys.argv[2])
    if len(sys.argv) >= 2 and not sys.argv[1].startswith("-"):
        print("PORT-DEPS gate — %s" % sys.argv[1])
        return check(sys.argv[1])
    print("usage: port_deps.py <donor actor> | --unlocks <SYSTEM>")
    return 2


if __name__ == "__main__":
    sys.exit(main())
