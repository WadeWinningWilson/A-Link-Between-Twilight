#!/usr/bin/env python3
# ============================================================================
# build_ledger.py - B6: ONE IDENTITY, THREE PURPOSES.
#
# Records, per build: the BUILD ID (the CodeView GUID+age the crash handler
# already emits - read via build_identity.py, never re-invented), the
# CLASSIFICATION STATE of the WW accounting at that moment (ratchet TU /
# floor / tier2 sites, plus content hashes of the map, the row store and the
# row baseline), and the git position. Append-only JSONL in the repo.
#
# WHY (roadmap B6, verbatim): "that build ID is the same durable identity the
# PDB archive needs and the same one the inventory's staleness check needs.
# One thing built, three debts retired - the 08-12 crash was permanently lost
# for want of exactly this." The 08-12 crash stays lost; the NEXT one joins:
#   crash line build_id -> ledger entry (what the tree claimed at that build)
#                       -> build_identity resolve (the matching PDB)
#
# Verdicts on resolve: FOUND or UNKNOWN. **Never nearest-match** - a
# confidently wrong build is how the 08-12 crash almost got mis-symbolicated
# before the Integrator refused to guess.
#
# Usage:
#   build_ledger.py record            append an entry for the current exe
#   build_ledger.py resolve <id>      entry + PDB for a crash's build_id
#   build_ledger.py list
#   build_ledger.py --selftest        control: roundtrip + fabricated-id
# ============================================================================
import hashlib
import json
import subprocess
import sys
import time
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
LEDGER = REPO / "docs" / "state" / "ww-staging" / "build-ledger.jsonl"
EXE = REPO / "build" / "windows-msvc-relwithdebinfo" / "dusklight.exe"

sys.path.insert(0, str(HERE))


def sha12(path):
    p = Path(path)
    if not p.exists():
        return None
    h = hashlib.sha256()
    if p.is_dir():
        for f in sorted(p.rglob("*")):
            if f.is_file():
                h.update(f.name.encode())
                h.update(f.read_bytes())
    else:
        h.update(p.read_bytes())
    return h.hexdigest()[:12]


def git(*args):
    r = subprocess.run(["git", *args], capture_output=True, text=True,
                       encoding="utf-8", errors="replace", cwd=str(REPO))
    return r.stdout.strip()


def build_id():
    """The exe's own CodeView identity via build_identity - never re-derived."""
    import build_identity as BI
    if not EXE.is_file():
        return None
    # Path, not str - read_codeview calls .read_bytes() on its argument. The
    # first version passed str(EXE); the AttributeError was swallowed by a
    # bare except and REPORTED AS "no exe" - a wrong-cause error message,
    # which is worse than a crash because it sends the reader to the wrong
    # fix. The except now names what actually failed.
    try:
        cv = BI.read_codeview(EXE)
        return "%s-%s" % (cv[0], cv[1]) if cv else None
    except Exception as e:
        print("  (build_id read failed: %s: %s - exe EXISTS, the read broke)"
              % (type(e).__name__, e))
        return None


def classification():
    """The WW accounting state, hashes-first so `record` stays in seconds."""
    import ww_ratchet as R
    tus, _names = R.lineage_tus()
    floor_n, _ = R.terminal_floor(_names)
    return {
        "ratchet_tu": tus,
        "terminal_floor": floor_n,
        "map_sha": sha12(REPO / "docs/state/ww-staging/ww-ownership-map.json"),
        "rows_sha": sha12(REPO / "docs/state/ww-staging/tracker/rows"),
        "baseline_sha": sha12(HERE / "ww-ratchet-baseline.json"),
    }


def record(ledger=LEDGER, exe_id=None):
    entry = {
        "ts": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "build_id": exe_id if exe_id is not None else build_id(),
        "git_head": git("rev-parse", "--short=10", "HEAD"),
        "git_dirty": len(git("status", "--porcelain").splitlines()),
        "classification": classification(),
    }
    if entry["build_id"] is None:
        print("RECORD REFUSED - no exe at %s; a ledger entry without a build "
              "id joins to nothing (No.31-C: refused, not defaulted)" % EXE)
        return 1, None
    with open(ledger, "a", encoding="utf-8", newline="\n") as f:
        f.write(json.dumps(entry) + "\n")
    print("recorded build %s  (TU %s / floor %s / map %s / rows %s / git %s+%d)"
          % (entry["build_id"], entry["classification"]["ratchet_tu"],
             entry["classification"]["terminal_floor"],
             entry["classification"]["map_sha"],
             entry["classification"]["rows_sha"],
             entry["git_head"], entry["git_dirty"]))
    return 0, entry


def resolve(bid, ledger=LEDGER):
    hits = []
    if Path(ledger).is_file():
        for ln in Path(ledger).read_text(encoding="utf-8").splitlines():
            try:
                e = json.loads(ln)
            except ValueError:
                continue
            if e.get("build_id") == bid:
                hits.append(e)
    if not hits:
        print("UNKNOWN - build %s has no ledger entry. NOT matching a nearby "
              "build: a confidently wrong PDB symbolicates to a confidently "
              "wrong function (the 08-12 lesson)." % bid)
        return 2, None
    e = hits[-1]
    print("FOUND %s  recorded %s  git %s+%d dirty"
          % (bid, e["ts"], e["git_head"], e["git_dirty"]))
    for k, v in e["classification"].items():
        print("  %-14s %s" % (k, v))
    guid = bid.split("-")[0]
    r = subprocess.run([sys.executable, str(HERE / "build_identity.py"),
                        "resolve", guid], capture_output=True, text=True,
                       encoding="utf-8", errors="replace")
    print("  pdb join     -> %s" % (r.stdout.strip().splitlines()[-1]
                                    if r.stdout.strip() else "(no output)"))
    return 0, e


def main():
    if "--selftest" in sys.argv:
        # ============================================================
        # CONTROL, both directions on a TEMP ledger (never the real one):
        # a recorded id must round-trip FOUND with identical content;
        # a fabricated id must come back UNKNOWN - never nearest-match.
        # ============================================================
        import tempfile
        tmp = Path(tempfile.mkdtemp()) / "ledger.jsonl"
        rc, entry = record(ledger=tmp, exe_id="SELFTEST-ID-1")
        ok1 = rc == 0
        rc2, back = resolve("SELFTEST-ID-1", ledger=tmp)
        ok2 = rc2 == 0 and back and back["classification"] == entry["classification"]
        rc3, none = resolve("ZZZ-NOT-A-BUILD", ledger=tmp)
        ok3 = rc3 == 2 and none is None
        print()
        for label, ok in (("record", ok1), ("roundtrip identical", ok2),
                          ("fabricated id -> UNKNOWN", ok3)):
            print("  [%s] %s" % ("ok " if ok else "FAIL", label))
        fired = ok1 and ok2 and ok3
        print("SELFTEST %s" % ("FIRED" if fired else "**FAILED**"))
        return 0 if fired else 1

    if "record" in sys.argv:
        return record()[0]
    if "resolve" in sys.argv:
        i = sys.argv.index("resolve")
        return resolve(sys.argv[i + 1])[0] if i + 1 < len(sys.argv) else 2
    if "list" in sys.argv:
        if not LEDGER.is_file():
            print("(empty ledger)")
            return 0
        for ln in LEDGER.read_text(encoding="utf-8").splitlines():
            e = json.loads(ln)
            print("%s  %s  git %s+%d  TU %s"
                  % (e["build_id"], e["ts"], e["git_head"], e["git_dirty"],
                     e["classification"]["ratchet_tu"]))
        return 0
    print("usage: build_ledger.py record | resolve <build_id> | list | --selftest")
    return 2


if __name__ == "__main__":
    sys.exit(main())
