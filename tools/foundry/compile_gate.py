#!/usr/bin/env python3
# ============================================================================
# compile_gate.py — the RECEIVER-API gate (CALLS row, tale §877; user-routed).
#
# THE HOLE: six actors passed VERBATIM (matched, 0 markers) and DEPS (systems
# registry clean) and FIVE failed to compile — undeclared registration enums,
# receiver APIs with different shapes (fopAcM_GetParamBit takes void* here),
# donor surfaces with no receiver definition (daObj::HitEff_kikuzu,
# WWEV_UNK_0820, camera_process_class...). No existing gate could see any of
# it: fidelity gates look at the DONOR, deps gates look at SYSTEMS — nothing
# asked "does the RECEIVER have the API this code calls, in this shape?"
#
# TWO TIERS:
#   Tier 1 (default, fast, no compiler): every identifier the TU references
#     that exists NOWHERE in the receiver tree (token index, comment-stripped)
#     = an undefined-reference candidate. Catches the registration-gap and
#     unported-surface classes (all 6 of §877's named symbols are this class).
#   Tier 2 (--compile): a REAL `cl /Zs` syntax+semantic pass using the
#     project's own flags, extracted live from ninja (-t commands) with the
#     source swapped in. /Zs writes NO object — this is a check, not a build;
#     the §839 protocol is untouched. Catches the API-SHAPE class tier 1
#     cannot (right name, wrong arity/types).
#
# Usage:  compile_gate.py <receiver TU .cpp> [--compile]
# Exit 0 clean · 1 findings · 2 could-not-run (№31-C: reported, never silent).
# ============================================================================
import re
import subprocess
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
BUILD = REPO / "build" / "windows-msvc-relwithdebinfo"
VCVARS = r"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
# A registered TU whose compile line serves as the flag template:
TEMPLATE_OBJ = "CMakeFiles/dusklight.dir/src/d/actor/d_a_bg.cpp.obj"

RE_TOKEN = re.compile(r"[A-Za-z_]\w{5,}")


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    return re.sub(r'"(?:[^"\\]|\\.)*"', '""', text)


def tier1(tu):
    import kit_laws
    rt = kit_laws._receiver_tokens()
    raw = tu.read_text(encoding="utf-8", errors="replace")
    self_toks = set(RE_TOKEN.findall(strip_comments(raw)))
    hdr = tu.with_suffix(".h")
    hdr2 = Path(str(tu)[:-4] + "_port.h")
    for h in (hdr, hdr2):
        if h.is_file():
            self_toks |= set(RE_TOKEN.findall(strip_comments(
                h.read_text(encoding="utf-8", errors="replace"))))
    # rt includes THIS TU (it is in src/), so presence there proves nothing —
    # each project-API candidate is re-checked with grep EXCLUDING the TU-pair:
    # a symbol defined nowhere else is a registration gap or unported surface.
    del rt  # the index primed kit_laws' cache; per-symbol grep is the truth here
    cands = []
    for t in sorted(self_toks):
        if re.match(r"(fpcNm_|fopAc|fopEv|dEvtFlag_|WWEV_|dRes_INDEX_|dSv_|"
                    r"daPy_|dCc|dBgS_|dKy|dPa_|dComIf|dMsg|dEv|dDemo|dStage_|"
                    r"cPhs_|PRM_|dProcName_)", t):
            cands.append(t)
    findings = []
    for t in cands:
        r = subprocess.run(["grep", "-rl", "--include=*.h", "--include=*.cpp",
                            "-F", t, str(REPO / "src"), str(REPO / "include")],
                           capture_output=True, text=True)
        hits = [h for h in r.stdout.splitlines()
                if Path(h).resolve() not in (tu.resolve(), hdr.resolve(), hdr2.resolve())]
        if not hits:
            findings.append(t)
    return findings


def tier2(tu):
    r = subprocess.run(["ninja", "-C", str(BUILD), "-t", "commands", TEMPLATE_OBJ],
                       capture_output=True, text=True)
    line = next((l for l in r.stdout.splitlines() if "cl.exe" in l), None)
    if not line:
        return None, "no template compile line (build dir unconfigured?)"
    # swap source, strip PCH consumption (keep the /FI forced include), no obj
    line = re.sub(r"/Yu\S+", "", line)
    line = re.sub(r"/Fp\S+", "", line)
    line = re.sub(r"/Fo\S+", "", line)
    line = re.sub(r"/showIncludes", "", line)
    line = re.sub(r"-c .*$", "", line)
    cmd = 'call "%s" >nul 2>&1 && %s /Zs -c "%s"' % (VCVARS, line, tu)
    r2 = subprocess.run(["cmd", "/c", cmd], capture_output=True, text=True,
                        cwd=str(BUILD), timeout=300)
    errs = [l for l in (r2.stdout + r2.stderr).splitlines()
            if re.search(r"error C\d+|fatal error", l)]
    return errs, None


def main():
    if len(sys.argv) < 2:
        print("usage: compile_gate.py <receiver TU .cpp> [--compile]")
        return 2
    tu = Path(sys.argv[1])
    if not tu.is_file():
        print("COMPILE-GATE: TU not found: %s" % tu)
        return 2
    sys.path.insert(0, str(HERE))
    findings = 0
    print("COMPILE-GATE %s" % tu.name)
    t1 = tier1(tu)
    if t1:
        findings += len(t1)
        print("  [TIER1] %d symbol(s) defined NOWHERE receiver-side beyond this TU:" % len(t1))
        for t in t1[:12]:
            print("      %s  (registration gap or unported donor surface)" % t)
    else:
        print("  [TIER1] clean — every project-API symbol resolves somewhere receiver-side")
    if "--compile" in sys.argv:
        errs, why = tier2(tu)
        if errs is None:
            print("  [TIER2] UNRESOLVED: %s — not proof of compilability (№31-C)" % why)
            findings += 1
        elif errs:
            findings += len(errs)
            print("  [TIER2] cl /Zs: %d error(s):" % len(errs))
            for e in errs[:10]:
                print("      %s" % e.strip()[:150])
        else:
            print("  [TIER2] cl /Zs clean — syntax+shape verified by the real compiler")
    else:
        print("  [TIER2] skipped (run with --compile for the cl /Zs shape check —")
        print("          the API-SHAPE class is only visible to a real compiler)")
    print("COMPILE-GATE %s" % ("CLEAN" if not findings else "FINDINGS: %d" % findings))
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
