#!/usr/bin/env python3
# ============================================================================
# ppc_sha1_gate.py - NEGATIVE CONTROL for the (A) gate: donor-toolchain REL
# byte-identity. Registered the day the Decoder lane's first hash landed
# (DECODER-BRIEFING s6.1; Foundry's same-day commitment on the charter row).
#
# THE GATE IT CONTROLS: a TU claims BYTE-TRUE only via a REL built under the
# donor's own toolchain whose SHA1 matches config/GZLE01/build.sha1
# (src=ppc-rel-sha1:<rel>@<hash>). THIS CONTROL proves the gate can go RED:
#   1. the pristine built REL MUST match its build.sha1 entry (the claim
#      itself, re-derived - never trusted from a row);
#   2. a ONE-BYTE perturbation (scratch copy, offset 0x100 XOR 1) MUST break
#      the hash - two artifacts that hash identically after a byte flip
#      would mean the comparison is not reading the bytes.
# Absent REL or absent build.sha1 -> UNKNOWN, refuse (a gate that cannot run
# is not a clean gate).
# ============================================================================
import hashlib
import subprocess
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

REL = Path(r"D:/XXXXXXX/WW DP/build/GZLE01/d_a_alldie/d_a_alldie.rel")
SHA1S = Path(r"D:/XXXXXXX/WW DP/config/GZLE01/build.sha1")


def main():
    if not REL.is_file() or not SHA1S.is_file():
        print("UNKNOWN - REL or build.sha1 absent; the (A) gate cannot run "
              "on this machine right now (not a pass, not a fail)")
        return 2
    want = None
    for ln in SHA1S.read_text(encoding="utf-8", errors="replace").splitlines():
        if ln.strip().endswith("d_a_alldie.rel"):
            want = ln.split()[0].lower()
            break
    if not want:
        print("UNKNOWN - no d_a_alldie.rel entry in build.sha1")
        return 2
    # ================================================================
    # THE CIRCULARITY CHECK (added 2026-08-17 — History/Bridge ran it by
    # hand while reviewing the campaign's first BYTE-TRUE claim, and this
    # fixture did not do it). A HASH MATCHING A MANIFEST SOMEBODY EDITED
    # PROVES NOTHING AT ALL: the "target" would simply be a copy of the
    # artifact's own hash, and the comparison would agree with anything.
    # The manifest must come from UPSTREAM, not from the lane under
    # review. Dirty manifest => UNKNOWN, refused — never a pass.
    # ================================================================
    try:
        r = subprocess.run(["git", "status", "--porcelain", "--",
                            "config/GZLE01/build.sha1"],
                           capture_output=True, text=True, timeout=60,
                           cwd=str(SHA1S.parents[2]))
        dirty = bool((r.stdout or "").strip())
        manifest_known = (r.returncode == 0)
    except Exception as e:
        dirty, manifest_known = False, False
        print("manifest integrity: UNCHECKABLE (%s)" % e)
    if manifest_known and dirty:
        print("REFUSED — config/GZLE01/build.sha1 IS LOCALLY MODIFIED. The "
              "expected hash is not upstream's, so a match would be "
              "circular (the target would be a copy of our own output).")
        return 2
    if not manifest_known:
        print("manifest integrity: UNKNOWN (git could not answer) — the "
              "match below is NOT independent; treat as UNVERIFIED")
    else:
        print("manifest integrity: CLEAN (build.sha1 unmodified vs upstream "
              "— the comparison is independent)")

    data = REL.read_bytes()
    got = hashlib.sha1(data).hexdigest()
    ok_match = (got == want)
    perturbed = bytearray(data)
    perturbed[0x100] ^= 1
    got2 = hashlib.sha1(bytes(perturbed)).hexdigest()
    ok_red = (got2 != want)
    print("pristine : %s  (%s build.sha1)" % (got, "==" if ok_match else "!="))
    print("perturbed: %s  (%s - one byte XOR 1 @0x100)"
          % (got2, "DETECTED as different" if ok_red else "**BLIND**"))
    if ok_match and ok_red:
        print("CONTROL OK - the (A) gate matches the real artifact AND goes "
              "red on a single flipped byte")
        return 0
    print("** GATE UNSOUND **")
    return 1


if __name__ == "__main__":
    sys.exit(main())
