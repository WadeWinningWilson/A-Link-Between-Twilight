#!/usr/bin/env python3
# ============================================================================
# drain_derive.py — DERIVE the mechanical half of a tracker row, per TU.
#
# The drain is 93 LEGACY-UNROWED TUs and hand-writing 93 rows one at a time is
# not a campaign, it is an accretion with extra steps. Foundry's worksheet made
# the per-TU cost "adjudication rather than enumeration"; this closes the other
# half — it derives every field that IS derivable and REFUSES to guess the rest.
#
#   python drain_derive.py --limit 10          print candidates for review
#   python drain_derive.py --limit 10 --write  write them into tracker/rows/
#
# WHAT IT DERIVES, each from a source that can be re-read:
#   tu         the worksheet line
#   symbols    g_profile_* and da*_c actually present in the TU
#   portable   the DONOR TU's status from configure.py — the authority
#              (Object(Matching,…) / Object(NonMatching,…)), never a marker grep,
#              which returned 0 on a file published at 5 markers (2026-08-16)
#   linked     REGISTERED iff the profile symbol appears in the plugin registry
#   doorway    EXISTS iff registered; else ABSENT-hookable for a profile actor
#
# WHAT IT REFUSES TO DERIVE, and this is the point rather than a limitation:
#   destination   axis A is an OWNERSHIP JUDGEMENT (PLUGIN/FORK/UPSTREAM/PATCH/
#                 SPLIT). A default here would file 93 unexamined verdicts that
#                 all LOOK adjudicated. Left blank; validate refuses the row.
#   negative-control  mandatory for PATCH and it is the one field that cannot be
#                 mechanised at all — it states what breaks if the seam is wrong.
#
# So a derived row is DELIBERATELY INVALID until a lane fills the judgement in.
# The generator cannot file work it has not thought about, which is the whole
# reason the drain exists rather than a bulk INSERT.
# ============================================================================
import argparse
import hashlib
import re
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
SHEET = REPO / "docs" / "state" / "ww-staging" / "DRAIN-WORKSHEET.md"
ROWS = REPO / "docs" / "state" / "ww-staging" / "tracker" / "rows"
# The profile REGISTRATION table. Not `registry.cpp` — that path was wrong and
# the probe returned NOT for all 93 TUs because it could never return anything
# else. A uniform result from a probe is the signature of a blind one.
REG_SOURCES = [
    REPO / "src" / "f_pc" / "f_pc_profile_lst.cpp",           # 493 profiles
    REPO / "src" / "d" / "ext_plugin" / "ww_profile_register.cpp",
]
DECOMP = Path("D:/XXXXXXX/WW DP")
TODAY = "2026-08-16"

# The donor disc is NTSC-U: d_ext_dmesg loads `rock_24_20_4i_usa.bfn`.
# This matters more than it looks — see donor_status().
DONOR_VERSION = "GZLE01"

ROW_LINE = re.compile(r"^\|\s*\d+\s*\|\s*`([^`]+)`\s*\|\s*([^|]+)\|")


def donor_status():
    """{basename: status} from configure.py — the authority, all FIVE classes.

    THE FIRST CUT OF THIS FUNCTION READ ONLY `Object(Matching|NonMatching, …)`
    AND WAS WRONG TWICE OVER:

    (1) WW ACTORS ARE NOT DECLARED WITH `Object(...)` AT ALL. They use
        `ActorRel(status, "d_a_name")` (configure.py:344), which expands to
        d/actor/<name>.cpp. Every actor therefore read UNKNOWN while its donor
        file sat plainly on disk — and UNKNOWN IS NEVER A PASS (§31-C), so this
        would have filed 93 rows asserting an absence that was an instrument gap.

    (2) THE STATUS VOCABULARY IS FIVE CLASSES, NOT TWO:
          Matching      226 actors  - matches, linked
          NonMatching   100 actors  - does not match, NOT linked
          MatchingFor    85 actors  - matches for SPECIFIC VERSIONS only
          Equivalent      3 actors  - semantically equal, links under --non-matching
          EquivalentFor   1 actor
        `MatchingFor` alone is 85 actors — MORE than a binary read would have
        called NonMatching after Matching. And it is version-scoped: our donor
        is GZLE01 (NTSC-U), so `MatchingFor("GZLJ01","GZLE01","GZLP01")` IS
        MATCHING FOR US. A binary Matching/NonMatching framing silently demotes
        85 actors that are citable verbatim against the disc we actually port.
    """
    cfg = DECOMP / "configure.py"
    if not cfg.is_file():
        return {}
    txt = cfg.read_text(encoding="utf-8", errors="replace")
    out = {}

    def classify(raw):
        raw = raw.strip()
        if raw.startswith("MatchingFor") or raw.startswith("EquivalentFor"):
            vers = re.findall(r"\"([A-Z0-9]+)\"", raw)
            kind = "Matching" if raw.startswith("MatchingFor") else "Equivalent"
            return ("%s(%s)" % (kind, DONOR_VERSION) if DONOR_VERSION in vers
                    else "NotForDonorVersion")
        return raw

    # GREEDY on purpose. A non-greedy `(.+?),\s*"` stops at the FIRST `, "` —
    # which for `ActorRel(MatchingFor("GZLJ01","GZLE01","GZLP01"), "d_a_lamp")`
    # is INSIDE MatchingFor, yielding status="MatchingFor(" and name="GZLJ01".
    # d_a_lamp then read UNKNOWN while sitting plainly in configure.py. Greedy
    # takes the LAST quoted string on the line, which is the rel/path.
    for m in re.finditer(r"ActorRel\(\s*(.+),\s*\"([^\"]+)\"\s*[,)]", txt):
        out["%s.cpp" % m.group(2)] = classify(m.group(1))
    for m in re.finditer(r"Object\(\s*(.+),\s*\"([^\"]+)\"\s*[,)]", txt):
        out.setdefault(Path(m.group(2)).name, classify(m.group(1)))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--limit", type=int, default=10)
    ap.add_argument("--offset", type=int, default=0)
    ap.add_argument("--write", action="store_true")
    a = ap.parse_args()

    if not SHEET.is_file():
        print("no worksheet at %s" % SHEET)
        return 2
    tus = [(m.group(1).strip(), m.group(2).strip())
           for m in (ROW_LINE.match(l) for l in SHEET.read_text(
               encoding="utf-8", errors="replace").splitlines()) if m]
    if not tus:
        print("worksheet lists no TUs — drain may be complete")
        return 0

    status = donor_status()
    reg = "".join(p.read_text(encoding="utf-8", errors="replace")
                  for p in REG_SOURCES if p.is_file())
    if not reg:
        print("REFUSING: no profile registration source readable — `linked` would "
              "read NOT for every TU, which is a blind probe, not a measurement.")
        return 2
    existing = {p.stem for p in ROWS.glob("*.md")} if ROWS.is_dir() else set()
    rowed_tus = set()
    for p in (ROWS.glob("*.md") if ROWS.is_dir() else []):
        m = re.search(r"^tu:\s*(.+)$", p.read_text(encoding="utf-8", errors="replace"), re.M)
        if m:
            rowed_tus.update(x.strip() for x in m.group(1).split(","))

    made = 0
    for tu, lineage in tus[a.offset:a.offset + a.limit]:
        if tu in rowed_tus:
            print("SKIP  %s — already carries a row" % tu)
            continue
        src = REPO / tu
        if not src.is_file():
            print("SKIP  %s — not on disk" % tu)
            continue
        t = src.read_text(encoding="utf-8", errors="replace")
        profs = sorted(set(re.findall(r"\b(g_profile_\w+)", t)))
        classes = sorted(set(c for pair in re.findall(r"\bclass\s+(da\w+_c)\b|\b(da\w+_c)::", t)
                             for c in pair if c))
        syms = ", ".join((profs + classes)[:6]) or "UNDERIVED"
        st = status.get(Path(tu).name)
        # Version-scoped Matching counts as DECOMPILED **for our donor** and the
        # citation records which version made it so — the claim is only true
        # against GZLE01 and must not travel as if it were unconditional.
        portable = "UNKNOWN"
        if st == "Matching" or (st or "").startswith("Matching("):
            portable = "DECOMPILED"
        elif st == "NonMatching":
            portable = "NONMATCHING"
        elif st == "Equivalent" or (st or "").startswith("Equivalent("):
            portable = "NONMATCHING"   # links only under --non-matching: pessimistic
        elif st == "NotForDonorVersion":
            portable = "NONMATCHING"   # matches some version, NOT ours
        registered = bool(profs and any(p in reg for p in profs))
        rid = "s" + hashlib.sha1((tu + "drain").encode()).hexdigest()[:8]
        if rid in existing:
            print("SKIP  %s — id %s exists" % (tu, rid))
            continue

        body = (
            "id:            %s\n"
            "symbols:       %s\n"
            "tu:            %s\n"
            "doorway:       %s\n"
            "destination:   \n"
            "portable:      %s\n"
            "linked:        %s\n"
            "provenance:    see-file\n"
            "citations:\n"
            "  - %s KIT-LINEAGE %s (read %s)\n"
            "  - D:/XXXXXXX/WW DP/configure.py donor status %s (read %s)\n"
            # CITE WHAT WAS ACTUALLY READ. The first cut hardcoded
            # `src/d/ext_plugin/registry.cpp` here — a path that DOES NOT EXIST.
            # I fixed REG_SOURCES when the probe read NOT for all 93 TUs, and
            # left the citation template pointing at the dead path, so 92 rows
            # shipped a correct FINDING with an unverifiable RECEIPT.
            # Fixing an instrument is not finishing until its output says what
            # it read.
            "  - %s profile %s (read %s)\n"
            "notes: |\n"
            "  DERIVED by drain_derive.py - MECHANICAL HALF ONLY.\n"
            "  `destination` is BLANK ON PURPOSE: axis A is an ownership judgement\n"
            "  and a generated default would file an unexamined verdict that looks\n"
            "  adjudicated. row_store.py validate REFUSES this row until a lane\n"
            "  rules it. Do not bulk-fill.\n"
            "created:       HISTORY/BRIDGE %s drain-derive\n"
        ) % (rid, syms, tu,
             "EXISTS" if registered else "ABSENT-hookable",
             portable,
             "REGISTERED" if registered else "NOT",
             tu, lineage, TODAY,
             st or "NOT LISTED", TODAY,
             ", ".join(str(p.relative_to(REPO)).replace("\\", "/")
                       for p in REG_SOURCES if p.is_file()),
             "present" if registered else "absent", TODAY,
             TODAY)

        print("%-46s %-12s portable=%-11s linked=%s" % (
            tu, rid, portable, "REGISTERED" if registered else "NOT"))
        if a.write:
            (ROWS / (rid + ".md")).write_text(body, encoding="utf-8")
            made += 1

    if a.write:
        print("\nwrote %d derived row(s) — each INVALID until `destination` is ruled" % made)
    return 0


if __name__ == "__main__":
    sys.exit(main())
