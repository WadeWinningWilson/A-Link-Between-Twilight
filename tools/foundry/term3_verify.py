#!/usr/bin/env python3
"""term3_verify.py - decide the milestone's term 3 ("actually plugin-only") from a run log.

Housing/Engine, 2026-08-21. Companion to docs/state/ww-staging/FINDING-milestone-terms-audit.md.

Term 3 is an ABSENCE claim, so this tool REFUSES to return a verdict unless it can
first show the discriminator FIRES somewhere. Three properties, all required:

  PURITY     each marker must be absent from the plugin binary - a marker the plugin
             can emit proves nothing about which exe ran (bare `ExtWw` and
             `dExtWwSave_isWwHostStage` both FAIL this and are excluded).
  FIRING     at least one marker must be observed in a FORK log (--control).
             Without that, an absence on vanilla is unmeasured, not clean.
  IDENTITY   the log's own Build: line, and the .dusk actually on disk by md5/size.

Usage:
  python tools/foundry/term3_verify.py --log <vanilla-run.log> [--control <fork-run.log>]
"""
import argparse, hashlib, os, re, sys

FORK_EXE   = r"%USERPROFILE%/Documents/dusklight/build/windows-msvc-relwithdebinfo/dusklight.exe"
VAN_EXE    = r"%USERPROFILE%/Documents/dusklight-main/build/windows-msvc-relwithdebinfo/dusklight.exe"
DUSK       = r"%USERPROFILE%/Documents/dusklight-main/build/windows-msvc-relwithdebinfo/mods/ww_donor_disc.dusk"
DUSK_MD5   = "cc00092ef79ca5e6b5820adf67e2c75d"
DUSK_SIZE  = 237981
STOCK_REV  = "c880d46fb5"

# markers verified 2026-08-21: present in the fork exe, absent from the vanilla exe,
# absent from the plugin's mod.dll. `firing` = observed in a real fork run.
MARKERS = [
    ("[ExtWw]",            True),   # 4,068 lines on dusklight-20260821-114625.log
    ("[ExtNpcMount]",      True),   # 92 lines on the same run
    ("[daBg]",             False),  # 0 on every fork log on disk - unproven at runtime
    ("898-P1",             False),
    ("898-P2",             False),
    ("257 skip",           False),
    ("BgW REGISTERED",     False),
    ("checkDekuLeafGlide", False),
]
# excluded on purpose - the plugin itself contains these (7x / 1x in mod.dll)
CONTAMINATED = ["ExtWw (bare)", "dExtWwSave_isWwHostStage"]


def scan(path):
    with open(path, "rb") as fh:
        blob = fh.read()
    return {m: blob.count(m.encode()) for m, _ in MARKERS}, blob


def build_line(blob):
    m = re.search(rb"Build: ([^\r\n]+)", blob)
    return m.group(1).decode("utf-8", "replace") if m else None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--log", required=True, help="the VANILLA run to judge")
    ap.add_argument("--control", help="a FORK run, to prove the markers can fire")
    a = ap.parse_args()

    print("== IDENTITY ==")
    hits, blob = scan(a.log)
    bl = build_line(blob)
    print(f"  log            {a.log}")
    print(f"  Build:         {bl or 'ABSENT'}")
    rev_ok = bool(bl and STOCK_REV in bl)
    print(f"  stock rev      {'YES' if rev_ok else 'NO'}  (expect {STOCK_REV})")

    if os.path.exists(DUSK):
        md5 = hashlib.md5(open(DUSK, "rb").read()).hexdigest()
        size = os.path.getsize(DUSK)
        art_ok = (md5 == DUSK_MD5 and size == DUSK_SIZE)
        print(f"  .dusk          {size} B  md5 {md5}  {'MATCH' if art_ok else 'DRIFTED'}")
    else:
        art_ok = False
        print("  .dusk          MISSING")

    print("\n== FIRING (positive control) ==")
    fired = []
    if a.control:
        chits, _ = scan(a.control)
        for m, _ in MARKERS:
            if chits[m]:
                fired.append(m)
            print(f"  {m:22s} {chits[m]:>7d}  in control")
    else:
        print("  NO CONTROL LOG GIVEN")
    if not fired:
        print("\n  VERDICT: INDETERMINATE - no marker was shown to fire.")
        print("  An absence you cannot make appear is not evidence. Pass --control <fork run>.")
        return 2

    print("\n== ABSENCE (the claim) ==")
    live = [m for m in fired]
    dirty = [m for m in live if hits[m]]
    for m in live:
        print(f"  {m:22s} {hits[m]:>7d}  in judged log")
    print(f"\n  excluded as contaminated: {', '.join(CONTAMINATED)}")

    ok = rev_ok and art_ok and not dirty
    print("\n== VERDICT ==")
    if dirty:
        print(f"  TERM 3 FALSIFIED - fork-only markers present: {', '.join(dirty)}")
        return 1
    if not (rev_ok and art_ok):
        print("  TERM 3 UNPROVEN - markers clean, but identity did not check out "
              f"(rev_ok={rev_ok}, artifact_ok={art_ok}). A clean absence on the WRONG binary proves nothing.")
        return 2
    print(f"  TERM 3 VERIFIED - stock {STOCK_REV}, artifact md5 matches, and "
          f"{len(live)} marker(s) proven to fire read ZERO here.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
