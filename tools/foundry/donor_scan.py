#!/usr/bin/env python3
# ============================================================================
# donor_scan.py - WOULD THIS COMMIT SHIP DONOR BYTES?
#
# Built 2026-08-22, the same night Foundry committed 524 KB of J3D2bdl4 model
# data and its own pre-commit check reported "none". Filing a row saying
# "everyone should pass -uall" does not make the mistake impossible; this
# does.
#
# THE TWO DEFECTS IN THE CHECK THAT FAILED, both fixed here:
#
#   1. DIRECTORY COLLAPSE. `git status --porcelain` prints ONE line for a new
#      untracked directory - `?? path/` - which matches no file-extension
#      pattern. Four `.bdl` files sat inside one and were invisible. **This
#      tool always passes `-uall`**, which expands directories to files.
#      (Proven: plain porcelain prints `?? sub/`; -uall prints
#      `?? sub/donor.bdl`.)
#
#   2. EXTENSION-ONLY MATCHING. A donor file renamed `notes.txt` passes any
#      extension check. **This tool reads MAGIC BYTES** - the file's own
#      first bytes - so `alwd.bdl` is caught as `J3D2` regardless of what it
#      is called. Extension is a secondary signal, never the only one.
#
# WHAT COUNTS AS A DONOR BYTE: the formats a Nintendo GameCube disc carries.
# Detected by magic where one exists, by extension where the format has no
# reliable header. **A hit is a STOP, not a warning** - zero-bake is the one
# hard line in this project, and the whole delivery model (ship code that
# reads the user's own disc, never the disc) depends on it holding.
#
# Usage:
#   donor_scan.py              scan staged + untracked (pre-commit gate)
#   donor_scan.py --staged     staged only
#   donor_scan.py --control    prove it catches both defect classes
# Exit 0 clean - 1 DONOR BYTES FOUND (do not commit) - 2 bad input.
# ============================================================================
import subprocess
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]

# Magic-byte signatures: (name, offset, bytes). Read from the file itself, so
# a rename cannot hide them -- the defect an extension-only check has.
MAGICS = [
    ("J3D model (bdl/bmd)", 0, b"J3D2"),
    ("RARC archive",        0, b"RARC"),
    ("Yaz0 compressed",     0, b"Yaz0"),
    ("Yay0 compressed",     0, b"Yay0"),
    ("BTI texture",         0, b"\x00\x00\x00\x02"),  # weak; extension corroborates
    ("U8 archive",          0, b"U\xaa8-"),
]
# Formats with no reliable magic -- extension is the only signal available.
DONOR_EXTS = {".dzb", ".dzr", ".dzs", ".bti", ".bms", ".aw", ".iso", ".arc",
              ".bdl", ".bmd", ".bck", ".btk", ".brk", ".btp", ".bas", ".bmt"}
# Paths where donor-shaped bytes are legitimate (none today -- kept explicit
# so an exemption must be WRITTEN, never assumed).
EXEMPT_PREFIXES = ()


def git(*args):
    return subprocess.run(["git", "-C", str(REPO)] + list(args),
                          capture_output=True, text=True)


def candidate_paths(staged_only=False):
    """Staged files + (unless --staged) untracked ones.

    -uall is the whole point: without it a new directory is one line and its
    contents are invisible."""
    paths = []
    r = git("diff", "--cached", "--name-only", "--diff-filter=ACMR")
    paths += [p for p in r.stdout.splitlines() if p.strip()]
    if not staged_only:
        r = git("status", "--porcelain", "-uall")
        for line in r.stdout.splitlines():
            if line.startswith("??"):
                paths.append(line[3:].strip())
    return sorted(set(paths))


def sniff(abs_path):
    """(reason, detail) if this file looks like donor bytes, else None."""
    p = Path(abs_path)
    try:
        if not p.is_file():
            return None
        with open(p, "rb") as fh:
            head = fh.read(16)
    except OSError:
        return None
    ext = p.suffix.lower()
    for name, off, sig in MAGICS:
        if head[off:off + len(sig)] == sig:
            # The weak BTI signature only counts when the extension agrees --
            # otherwise it false-positives on any file starting 00 00 00 02.
            if name.startswith("BTI") and ext != ".bti":
                continue
            return ("magic", "%s (%s at +%d)" % (name, sig.decode("latin1", "replace"), off))
    if ext in DONOR_EXTS:
        return ("extension", "donor format by extension (%s), no magic matched" % ext)
    return None


def scan(staged_only=False):
    hits = []
    for rel in candidate_paths(staged_only):
        if any(rel.startswith(x) for x in EXEMPT_PREFIXES):
            continue
        found = sniff(REPO / rel)
        if found:
            hits.append((rel, found[0], found[1]))
    return hits


def control():
    """Plant BOTH historical defect classes and require both be caught:
    a donor file inside a NEW UNTRACKED DIRECTORY (the -uall defect), and a
    donor file with a HARMLESS EXTENSION (the extension-only defect)."""
    import tempfile
    tmpdir = REPO / "_donor_scan_control"
    nested = tmpdir / "nested"
    nested.mkdir(parents=True, exist_ok=True)
    # (1) real donor magic, inside a brand-new directory
    (nested / "model.bdl").write_bytes(b"J3D2bdl4" + b"\x00" * 24)
    # (2) real donor magic wearing an innocent name
    (nested / "harmless_notes.txt").write_bytes(b"RARC" + b"\x00" * 28)
    try:
        hits = scan(staged_only=False)
        got = {h[0].split("/")[-1]: h for h in hits}
        c1 = "model.bdl" in got
        c2 = "harmless_notes.txt" in got and got["harmless_notes.txt"][1] == "magic"
        print("NEGATIVE CONTROL - both historical defect classes planted")
        print("  in-new-untracked-dir (.bdl)      %s" % ("CAUGHT" if c1 else "*** MISSED ***"))
        print("  magic-under-innocent-name (.txt) %s" % ("CAUGHT" if c2 else "*** MISSED ***"))
        for rel, why, detail in hits:
            if "_donor_scan_control" in rel:
                print("    flagged: %s [%s] %s" % (rel, why, detail))
        ok = c1 and c2
        print("  CONTROL %s" % ("PASSED - the scan sees inside new directories AND "
                                "past filenames." if ok else "*** FAILED ***"))
        return 0 if ok else 1
    finally:
        for f in nested.glob("*"):
            f.unlink()
        nested.rmdir()
        tmpdir.rmdir()


def main():
    if "--control" in sys.argv:
        return control()
    staged_only = "--staged" in sys.argv
    hits = scan(staged_only)
    scope = "staged" if staged_only else "staged + untracked"
    if not hits:
        print("DONOR SCAN (%s): clean - no donor bytes." % scope)
        print("  (magic-byte based and -uall expanded; this silence is a result,")
        print("   not the blind spot that shipped 524 KB on 2026-08-22.)")
        return 0
    print("DONOR SCAN (%s): *** %d DONOR-BYTE FILE(S) - DO NOT COMMIT ***" % (scope, len(hits)))
    for rel, why, detail in hits:
        print("  %-60s [%s] %s" % (rel, why, detail))
    print()
    print("Zero-bake is the hard line: ship the code that reads the user's own")
    print("disc, never the disc. If these are research extracts, gitignore them")
    print("and keep the script that regenerates them instead.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
