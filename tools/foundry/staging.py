#!/usr/bin/env python3
# ===========================================================================
# staging.py — the staging protocol's tool half (§694 spec, §695 #3 adjacent).
#
#   lint    manifest sanity: legal states, unique ids, depends-on resolvable,
#           draft file exists for every non-MERGED row
#   claims  the integrator's checklist: every READY row's claims, one per line
#   merge   the batch append: READY rows whose deps are MERGED, in dependency
#           order, each draft appended to the bus under a §N assigned AT MERGE
#           TIME from the live max (the §694/§702 lesson: lanes never number).
#           Rows flip to MERGED in the manifest. FLAGGED/DRAFT rows never move.
#
# The integrator still VERIFIES claims by hand/fan-out — this tool lists and
# appends; it does not judge. Content-neutrality is the role's wall, so it is
# the tool's wall too.
#
# Selftest (§695 #4 law): exercises lint failure, claims listing, and a merge
# into a TEMP bus — never the real one.
# ===========================================================================
import io
import os
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
STAGING = REPO / "docs" / "state" / "ww-staging"
MANIFEST = STAGING / "MANIFEST.md"
BUS = REPO / "docs" / "WW Linked" / "ww-bridge-tool-interconnected.md"

STATES = ("DRAFT", "READY", "FLAGGED", "MERGED")

# ===========================================================================
# THE SECTION ALLOCATOR (tale-bus call-out: four cross-bus collisions).
# One number line across EVERY bus: alloc scans all registered buses AND the
# ledger, returns max+1, and RESERVES it by appending a ledger row BEFORE the
# section is written. Citation discipline rides the output: cite as
# "<bus> §N", never bare §N — bare numbers are how four collisions happened.
# The ledger is the serialization point; a post-append readback detects a
# race (two lanes allocating in the same second) and retries once, so a
# collision is at worst DETECTED, never silent.
# ===========================================================================
BUSES = {
    "interconnected": REPO / "docs" / "WW Linked" / "ww-bridge-tool-interconnected.md",
    "tale": REPO / "docs" / "state" / "ww-tale-dmesg-live-state.md",
}
LEDGER = STAGING / "SECTION-LEDGER.md"


def _max_section():
    """Highest section number ANYWHERE — ledger rows plus every header form.

    ROOT CAUSE of the §840 and §901 double-issues (CALLS row, tale §904):
    NOT a race and NOT the ledger — both numbers appear EXACTLY ONCE in
    SECTION-LEDGER.md. The duplicate came from a lane WRITING A SECTION
    WITHOUT ALLOCATING, and the high-water mark could not see it: the old
    patterns required the § to sit immediately after the hashes, so a
    heading like `## §901b …`, a bare `## §901`, or any lane-prefixed form
    was INVISIBLE. Invisible high-water = the next honest allocator hands
    out a number already in use, and the lane that skipped the allocator
    never learns. Now: § anywhere in a heading line counts."""
    n = 0
    for name, path in BUSES.items():
        if path.is_file():
            txt = io.open(path, encoding="utf-8", errors="replace").read()
            for line in txt.splitlines():
                if line.startswith("#"):
                    for m in re.finditer(r"§(\d+)", line):
                        n = max(n, int(m.group(1)))
    if LEDGER.is_file():
        for m in re.finditer(r"\| §(\d+) \|",
                             io.open(LEDGER, encoding="utf-8", errors="replace").read()):
            n = max(n, int(m.group(1)))
    return n


def alloc(bus, lane):
    if bus not in BUSES:
        print("unknown bus %r — registered: %s" % (bus, ", ".join(BUSES)))
        return 2
    if not LEDGER.is_file():
        io.open(LEDGER, "w", encoding="utf-8", newline="\r\n").write(
            "# Section ledger — allocate BEFORE writing; cite as '<bus> §N'\n"
            "| §N | bus | lane |\n|---|---|---|\n")
    for attempt in range(2):
        n = _max_section() + 1
        with io.open(LEDGER, "a", encoding="utf-8", newline="\r\n") as f:
            f.write("| §%d | %s | %s |\n" % (n, bus, lane))
        rows = re.findall(r"\| §%d \|" % n,
                          io.open(LEDGER, encoding="utf-8", errors="replace").read())
        if len(rows) == 1:
            print("ALLOCATED: %s §%d  (cite as '%s §%d', never bare)" % (bus, n, bus, n))
            return 0
        print("race detected on §%d — retrying" % n)
    print("allocation raced twice — resolve the ledger by hand")
    return 1

def verify():
    """Ledger vs buses: unallocated sections, duplicate headers, ledger dupes.

    The §904 lesson: a lane writing without allocating is INVISIBLE until it
    collides. This makes it a report — run it any time, cheap."""
    import collections
    ledger = collections.Counter()
    if LEDGER.is_file():
        for m in re.finditer(r"\| §(\d+) \|",
                             io.open(LEDGER, encoding="utf-8", errors="replace").read()):
            ledger[int(m.group(1))] += 1
    written = collections.Counter()
    where = {}
    for name, path in BUSES.items():
        if not path.is_file():
            continue
        for line in io.open(path, encoding="utf-8", errors="replace").read().splitlines():
            if not line.startswith("## "):
                continue
            m = re.search(r"§(\d+)([a-z]?)", line)
            if m:
                num = int(m.group(1))
                written[num] += 1
                where.setdefault(num, []).append("%s: %s" % (name, line[:70]))
    # SCOPE (calibration, first run): the ledger began mid-project, so every
    # pre-ledger section reads "unallocated" — 672 rows of history nobody can
    # act on, which is the §389b noise trap. Report only the ledger's own era.
    era = min(ledger) if ledger else 0
    written = {k: v for k, v in written.items() if k >= era}
    print("verify scope: sections >= §%d (the ledger's era)" % era)
    bad = 0
    for num, cnt in sorted(ledger.items()):
        if cnt > 1:
            print("[LEDGER-DUPE] §%d allocated %d times" % (num, cnt))
            bad += 1
    for num, cnt in sorted(written.items()):
        if num not in ledger:
            print("[UNALLOCATED] §%d written but never allocated:" % num)
            for w in where[num]:
                print("      %s" % w)
            bad += 1
        elif cnt > 1:
            print("[COLLISION] §%d has %d headers (allocated once):" % (num, cnt))
            for w in where[num]:
                print("      %s" % w)
            bad += 1
    print("verify: %s" % ("clean — ledger and buses agree" if not bad
                          else "%d issue(s) — renumber pre-citation, per the erratum pattern" % bad))
    return 1 if bad else 0


ROW = re.compile(r"^\|\s*([\w.-]+)\s*\|\s*([^|]+?)\s*\|\s*([^|]+?)\s*\|\s*"
                 r"(\w+)\s*\|\s*([^|]*?)\s*\|\s*([^|]*?)\s*\|\s*$")


def parse(manifest_path):
    rows = []
    for ln in io.open(manifest_path, encoding="utf-8", errors="replace"):
        m = ROW.match(ln.strip())
        if m and m.group(1) not in ("id",):
            rows.append(dict(zip(("id", "lane", "title", "state", "deps", "claims"),
                                 (g.strip() for g in m.groups()))))
    return rows


def lint(manifest_path, staging_dir):
    rows = parse(manifest_path)
    errs = []
    ids = [r["id"] for r in rows]
    for i in set(ids):
        if ids.count(i) > 1:
            errs.append("duplicate id: %s" % i)
    for r in rows:
        if r["state"] not in STATES:
            errs.append("%s: illegal state %r" % (r["id"], r["state"]))
        for d in filter(None, (x.strip() for x in (r["deps"] if r["deps"].strip() != "-" else "").split(","))):
            if d not in ids:
                errs.append("%s: depends-on %r not in manifest" % (r["id"], d))
        if r["state"] != "MERGED" and not (staging_dir / ("%s.md" % r["id"])).is_file():
            errs.append("%s: draft file missing (%s.md)" % (r["id"], r["id"]))
    print("staging lint: %d row(s), %d error(s)" % (len(rows), len(errs)))
    for e in errs:
        print("  ERROR %s" % e)
    return 1 if errs else 0


def claims(manifest_path):
    rows = [r for r in parse(manifest_path) if r["state"] == "READY"]
    if not rows:
        print("no READY rows — nothing for the integrator")
        return 0
    print("INTEGRATOR CHECKLIST — verify each claim against the LIVE tree:")
    for r in rows:
        print("\n  %s (%s) — %s" % (r["id"], r["lane"], r["title"]))
        for c in filter(None, (x.strip() for x in r["claims"].split(";"))):
            print("    [ ] %s" % c)
    return 0


def merge(manifest_path, staging_dir, bus_path, dry=False):
    rows = parse(manifest_path)
    by_id = {r["id"]: r for r in rows}

    def deps_ok(r):
        return all(by_id.get(d, {}).get("state") == "MERGED"
                   for d in filter(None, (x.strip() for x in (r["deps"] if r["deps"].strip() != "-" else "").split(","))))

    ready = [r for r in rows if r["state"] == "READY" and deps_ok(r)]
    held = [r for r in rows if r["state"] == "READY" and not deps_ok(r)]
    for r in held:
        print("HELD %s — depends on non-MERGED work" % r["id"])
    if not ready:
        print("nothing mergeable")
        return 0

    bus = io.open(bus_path, encoding="utf-8", newline=None).read()
    n = max(int(m) for m in re.findall(r"^## §(\d+)", bus, re.M)) + 1
    out = []
    for r in ready:
        draft = io.open(staging_dir / ("%s.md" % r["id"]),
                        encoding="utf-8", errors="replace").read()
        out.append("\n## §%d — %s (staged as %s): %s\n\n%s\n"
                   % (n, r["lane"], r["id"], r["title"], draft.strip()))
        print("MERGE %s -> §%d" % (r["id"], n))
        n += 1
    if dry:
        print("(dry run — nothing written)")
        return 0
    with io.open(bus_path, "a", encoding="utf-8", newline="\r\n") as f:
        f.write("".join(out))
    mtxt = io.open(manifest_path, encoding="utf-8", newline=None).read()
    for r in ready:
        mtxt = re.sub(r"(\|\s*%s\s*\|[^|]*\|[^|]*\|\s*)READY(\s*\|)" % re.escape(r["id"]),
                      r"\g<1>MERGED\g<2>", mtxt)
    io.open(manifest_path, "w", encoding="utf-8", newline="\r\n").write(mtxt)
    print("batch appended: %d section(s); manifest updated" % len(ready))
    return 0


def selftest():
    import tempfile
    d = Path(tempfile.mkdtemp())
    man = d / "MANIFEST.md"
    io.open(man, "w", encoding="utf-8").write(
        "| id | lane | title | state | depends-on | claims |\n"
        "|---|---|---|---|---|---|\n"
        "| a-1 | L1 | first | READY | - | file X exists; count is 3 |\n"
        "| b-1 | L2 | second | READY | a-1 | tool T exits 0 |\n"
        "| c-1 | L3 | bad | BOGUS | zz-9 | - |\n")
    io.open(d / "a-1.md", "w", encoding="utf-8").write("body A")
    io.open(d / "b-1.md", "w", encoding="utf-8").write("body B")
    bus = d / "bus.md"
    io.open(bus, "w", encoding="utf-8").write("## §5 — seed\n")
    r1 = lint(man, d)          # expects errors: BOGUS state, zz-9 dep, c-1 draft missing
    claims(man)
    merge(man, d, bus)         # a-1 merges as §6; b-1 HELD (dep not yet MERGED)
    merge(man, d, bus)         # b-1 merges as §7 now
    final = io.open(bus, encoding="utf-8").read()
    ok = (r1 == 1 and "§6" in final and "§7" in final and "body B" in final)
    print("\nselftest:", "OK" if ok else "BROKEN")
    return 0 if ok else 1


def main():
    if "--selftest" in sys.argv:
        return selftest()
    cmd = sys.argv[1] if len(sys.argv) > 1 else "lint"
    dry = "--dry" in sys.argv
    if cmd == "lint":
        return lint(MANIFEST, STAGING)
    if cmd == "claims":
        return claims(MANIFEST)
    if cmd == "merge":
        return merge(MANIFEST, STAGING, BUS, dry)
    if cmd == "alloc":
        if len(sys.argv) < 4:
            print("usage: staging.py alloc <bus> <lane>")
            return 2
        return alloc(sys.argv[2], sys.argv[3])
    if cmd == "verify":
        return verify()
    print("usage: staging.py [lint|claims|merge [--dry]|alloc <bus> <lane>|"
          "verify|--selftest]")
    return 2


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
