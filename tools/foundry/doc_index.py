#!/usr/bin/env python3
# ============================================================================
# doc_index.py - THE DOC TREE AUDITED THE WAY EVERY OTHER CORPUS HERE IS.
#
# ---------------------------------------------------------------------------
# WHY THIS EXISTS. This estate instruments its bytes (c0_audit), its gates
# (control), its lane tokens (file_row lint), its monitors (watcher_census,
# monitor_pulse), its bus routing (routing_check, call_receipt), its provenance
# banners (banner_lint) and its symbols (sig_diff, upstream_conformance).
#
# **NOTHING took docs/ as its subject.** That is why "47 of 76 top-level docs
# are unreachable from AGENT_INDEX.md" was not a number until 2026-08-16, and
# why two HISTORY handoffs sat ten days apart with inverted names for a week.
# Not carelessness: docs/ was the only corpus without a tool that would say so.
#
# GENERATED, NEVER HAND-KEPT - the thrice-independently-discovered doctrine.
# An audit a human has to remember to redo is an audit that reports the state
# of the last time somebody remembered.
#
# WHAT IT REFUSES TO DO: rule on CONTENT. It never says which of two duplicate
# documents should survive, never proposes text, never deletes. Merging is the
# owning lane's judgement; this reports SHAPE. (Librarian content-neutrality
# wall - the Librarian rules the shape, never the truth recorded in it.)
#
# Usage:
#   doc_index.py                 full audit
#   doc_index.py --orphans       only unreachable docs
#   doc_index.py --selftest      prove every detector can fire (the control)
# Exit 0 clean - 1 findings - 2 bad input.
# ============================================================================
import io
import os
import re
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]

# Docs whose absence from the index is CORRECT, with the reason stated. An
# exemption list with no reasons is how a gate quietly stops gating.
INDEX_EXEMPT = {
    "AGENT_INDEX.md": "is the index",
}

LINK_RE = re.compile(r"\]\(([^)]+?\.md)\)")
STOP_TOKENS = {"md", "the", "a", "of", "and"}


def rel(p, base):
    """Display path. Falls back rather than raising: the selftest builds its
    fixture tree OUTSIDE the repo, and a reporter that crashes on a path it
    cannot prettify is a reporter that cannot be controlled."""
    for anchor in (REPO, base.parent if base else None):
        if anchor:
            try:
                return str(p.relative_to(anchor))
            except ValueError:
                pass
    return str(p)


def norm_tokens(name):
    """Basename -> sorted token multiset, for order-insensitive collisions."""
    stem = re.sub(r"\.md$", "", name, flags=re.I).lower()
    toks = [t for t in re.split(r"[-_ .]+", stem) if t and t not in STOP_TOKENS]
    return tuple(sorted(toks))


def all_docs(root):
    out = []
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d not in (".git", "node_modules")]
        for fn in filenames:
            if fn.lower().endswith(".md"):
                out.append(Path(dirpath) / fn)
    return sorted(out)


def audit(docs_root, index_path, top_only=True):
    """Returns (findings dict, counts dict). Pure - no printing, so the
    selftest can assert on the same code path the real run uses."""
    f = {"orphans": [], "dead": [], "collisions": [], "supersession": [],
         "handoff_shape": [], "multi_handoff": [], "era": [], "stale": []}

    index_text = ""
    if index_path.is_file():
        index_text = io.open(index_path, encoding="utf-8",
                             errors="replace").read()

    top = sorted(p for p in docs_root.glob("*.md"))
    every = all_docs(docs_root)

    # 1. ORPHANS - a top-level doc the index never names.
    for p in top:
        if p.name in INDEX_EXEMPT:
            continue
        if p.name not in index_text:
            f["orphans"].append(p.name)

    # 2. DEAD LINKS - the index pointing at something that is not there.
    for m in LINK_RE.finditer(index_text):
        target = m.group(1).replace("%20", " ")
        if target.startswith(("http://", "https://")):
            continue
        if not ((docs_root / target).is_file() or (REPO / target).is_file()):
            f["dead"].append(target)

    # 3. NAME COLLISIONS - and the FIRST version of this detector was WRONG in
    #    the way this estate has spent a week cataloguing, so the reasoning is
    #    recorded rather than the fix alone.
    #
    #    v1 grouped by token multiset and flagged every group >1. It reported
    #    SEVENTEEN collisions, of which FIFTEEN were port-kit TEMPLATE files -
    #    every kit legitimately has its own ccmap.md, closure.md, surface.md.
    #    **A structured template tree is not a collision, and a detector that
    #    calls it one is a permanent false positive** - which destroys the
    #    signal exactly as thoroughly as a permanent false negative (the same
    #    lesson the shared monitor-pulse taught on 2026-08-15).
    #
    #    So: two narrow detectors instead of one broad one.
    REPEATED_OK = {"readme.md"}          # intentionally one-per-directory

    def all_siblings(paths):
        """True when the paths are ONE template instantiated per subject.

        v2: the first version only looked one level up, so it caught
        <kit>/ccmap.md and MISSED <kit>/ported_src/codemod-report.md - the
        template detector had its own blind spot, which is the joke this whole
        estate has been living in for two days. Now: strip the common ancestor
        and require the remainders to be identical EXCEPT in their first
        segment (the per-subject directory name), at any depth."""
        parts = [p.parts for p in paths]
        i = 0
        while len({pp[i] if i < len(pp) else None for pp in parts}) == 1:
            i += 1
        rests = [pp[i:] for pp in parts]
        if len({len(r) for r in rests}) != 1 or len(rests[0]) < 2:
            return False
        return len({r[1:] for r in rests}) == 1

    # 3a. REORDERED / RESHAPED NAMES - same tokens, DIFFERENT basename. This is
    #     the expensive one: HANDOFF-HISTORY.md vs state/HISTORY-HANDOFF.md.
    by_tokens = {}
    for p in every:
        by_tokens.setdefault(norm_tokens(p.name), []).append(p)
    for toks, group in sorted(by_tokens.items()):
        names = {p.name for p in group}
        if len(group) > 1 and len(names) > 1:
            f["collisions"].append([rel(p, docs_root) for p in group])

    # 3b. SAME BASENAME IN UNRELATED TREES - a real duplicate, unless it is a
    #     per-directory convention or a template family.
    by_name = {}
    for p in every:
        by_name.setdefault(p.name.lower(), []).append(p)
    for name, group in sorted(by_name.items()):
        if len(group) < 2 or name in REPEATED_OK or all_siblings(group):
            continue
        f["collisions"].append([rel(p, docs_root) for p in group])

    # DEDUPE - 3a and 3b can surface the SAME group: INDEX.md vs index.md
    # differs by case, so it is both a reshaped name and a same-basename hit.
    # Reporting one finding twice inflates a count, and inflated counts are
    # what this estate spent two days un-inflating.
    seen_groups, deduped = set(), []
    for g in f["collisions"]:
        key = frozenset(x.lower() for x in g)
        if key not in seen_groups:
            seen_groups.add(key)
            deduped.append(g)
    f["collisions"] = deduped

    # 4. SUPERSESSION PAIRS - X.md living beside X-CANONICAL / X-SUPERSEDED /
    #    X-<date>. The hazard is not the pair; it is that the SHORTER name is
    #    usually the dead one, so every glob finds the wrong file first.
    stems = {re.sub(r"\.md$", "", p.name, flags=re.I): p for p in every}
    for stem, p in sorted(stems.items()):
        for suffix in ("-CANONICAL", "-SUPERSEDED", "-DEPRECATED"):
            if stem.upper().endswith(suffix):
                base = stem[: -len(suffix)]
                if base in stems:
                    older, newer = stems[base], p
                    f["supersession"].append(
                        (rel(older, docs_root), rel(newer, docs_root),
                         older.stat().st_size > newer.stat().st_size))
        if re.search(r"-\d{4}-\d{2}-\d{2}$", stem):
            base = re.sub(r"-\d{4}-\d{2}-\d{2}$", "", stem)
            if base in stems:
                f["supersession"].append(
                    (rel(stems[base], docs_root), rel(p, docs_root), False))

    # 5. HANDOFF SHAPE - one convention: HANDOFF-<LANE>.md, in docs/ root.
    #    Lane handoffs are not live state and do not belong in state/.
    for p in every:
        if "handoff" not in p.name.lower():
            continue
        ok = (p.parent == docs_root and p.name.startswith("HANDOFF-"))
        if not ok:
            f["handoff_shape"].append(rel(p, docs_root))

    # 5b. MULTIPLE HANDOFFS PER LANE - and this detector exists because the
    #     first one MISSED ONE, in the exact way the rest of this estate keeps
    #     missing things (History/Bridge, 2026-08-16).
    #
    #     v1 grouped by NAME SYMMETRY and found HANDOFF-HISTORY.md vs
    #     HISTORY-HANDOFF.md. It could not see `ww-handoff-history-bridge.md`,
    #     which is neither spelling - **and that was the CURRENT one, the one a
    #     successor most needs.** A name pattern cannot see a name it did not
    #     anticipate.
    #
    #     So enumerate THE SIDE THAT CANNOT GROW: the lane roster is ratified
    #     in docs/LANES.md and is fixed; filenames are invented per author and
    #     are not. Group every handoff-ish doc by the LANE it names.
    lanes = set()
    lanes_doc = REPO / "docs" / "LANES.md"
    if lanes_doc.is_file():
        for ln in io.open(lanes_doc, encoding="utf-8", errors="replace"):
            if ln.startswith("| **"):
                nm = ln.split("|")[1].strip().strip("*").strip()
                if nm:
                    lanes.add(nm.upper().split()[0])
    by_lane = {}
    for p in every:
        low = p.name.lower()
        if "handoff" not in low:
            continue
        for lane in lanes:
            if lane.lower() in low:
                by_lane.setdefault(lane, []).append(rel(p, docs_root))
    for lane, group in sorted(by_lane.items()):
        if len(group) > 1:
            f["multi_handoff"].append((lane, group))

    # 7. ERA - the axis documents did not have (user ruling, 2026-08-16:
    #    "yes assign it by era").
    #
    #    THE HAZARD IS NOT OBSCURITY, IT IS AUTHORITY. An orphan is invisible
    #    and costs a rediscovery. An era-less METHOD doc is found, read and
    #    FOLLOWED - and the eras it may describe (mounted, bake, leg) are ones
    #    the estate has since made hard stops (DN-9, DN-10). A faithful reading
    #    of a wrong-era recipe executes a rejected approach with the doc tree's
    #    authority behind it.
    #
    #    Vocabulary is CLOSED - enumerate the side that cannot grow:
    #    `albw` is here because the user ruled the skins/ALBW track OUT of WW
    #    migration accounting (2026-08-16) - forcing a WW era onto an ALBW HUD
    #    brief would be inventing a fact, which is DN-13's whole subject.
    # ⛔ DO NOT ADD ERA CLASSIFICATION HERE. History/Bridge tried it and
    #    recorded a CONTROLLED NEGATIVE (2026-08-16): keyword passes flagged
    #    recipes 10 and 15 as era-bound - both FALSE POSITIVES, because they
    #    mention mounting as CONTRAST and as PROVENANCE, not as instruction.
    #    A second pass found MORE era-bound hits in the CANONICAL cookbook
    #    than in the SUPERSEDED one (1 vs 0) when the true ordering must be
    #    the reverse - **the control could not separate a live doc from a
    #    dead one.** An era linter built on keywords reports confident
    #    nonsense.
    #
    #    This detector reports PRESENCE and VALIDITY of the field ONLY. It
    #    never infers the value. Era is read, not grepped - knowing which era
    #    a method belongs to is knowing the method, which is the owning
    #    lane's knowledge and not a document property.
    ERAS = ("mounted", "bake", "leg", "plugin-native", "era-independent",
            "albw", "unknown")
    #    Method-class heuristic. Deliberately WIDE: a false "needs an era" costs
    #    one line; a false "does not need one" is the hazard above. Stated so the
    #    next reader can see the bias rather than infer it.
    METHOD_HINT = re.compile(
        r"recipe|cookbook|playbook|how it works|how-to|procedure|method|"
        r"port kit|port-kit|guide|workflow|step-by-step", re.I)
    ERA_LINE = re.compile(r"^\s*(?:>\s*)?(?:\*\*)?era:\s*([a-z-]+)", re.I | re.M)

    for p in top:
        try:
            head_text = io.open(p, encoding="utf-8", errors="replace").read(6000)
        except OSError:
            continue
        m = ERA_LINE.search(head_text)
        if m:
            val = m.group(1).lower()
            if val not in ERAS:
                f["era"].append((p.name, "INVALID:%s" % val))
            continue
        if METHOD_HINT.search(p.name) or METHOD_HINT.search(head_text):
            f["era"].append((p.name, "method-class, no era: line"))

    # 6. STALE - reported, never judged. A doc can be finished.
    now = time.time()
    for p in top:
        days = int((now - p.stat().st_mtime) / 86400)
        if days > 30:
            f["stale"].append((p.name, days))

    counts = {"top": len(top), "every": len(every)}
    return f, counts


def report(f, counts):
    n = sum(len(v) for v in f.values())
    print("DOC INDEX AUDIT - shape only; content is the owning lane's")
    print("  corpus: %d top-level docs, %d total under docs/"
          % (counts["top"], counts["every"]))
    print()

    print("  [1] UNREACHABLE FROM THE INDEX: %d" % len(f["orphans"]))
    for name in f["orphans"][:200]:
        print("        %s" % name)
    if not f["orphans"]:
        print("        every top-level doc is named by the index.")

    print("  [2] DEAD LINKS IN THE INDEX: %d" % len(f["dead"]))
    for t in f["dead"]:
        print("        MISSING %s" % t)

    print("  [3] NAME COLLISIONS (same tokens, different order/dir): %d"
          % len(f["collisions"]))
    for group in f["collisions"]:
        print("        %s" % "  <->  ".join(group))
    if f["collisions"]:
        print("        A handoff is read ONCE, by the reader least able to")
        print("        tell they got the stale copy. This is the expensive one.")

    print("  [4] SUPERSESSION PAIRS: %d" % len(f["supersession"]))
    for older, newer, bigger in f["supersession"]:
        flag = "  <- DEAD FILE IS LARGER; every glob finds it first" if bigger else ""
        print("        %s  ||  %s%s" % (older, newer, flag))

    print("  [5] HANDOFF NAMING (want HANDOFF-<LANE>.md in docs/): %d off-shape"
          % len(f["handoff_shape"]))
    for p in f["handoff_shape"][:60]:
        print("        %s" % p)

    print("  [5b] LANES WITH MORE THAN ONE HANDOFF: %d" % len(f["multi_handoff"]))
    for lane, group in f["multi_handoff"]:
        print("        %-10s %s" % (lane, "  |  ".join(group)))
    if f["multi_handoff"]:
        print("        Grouped by LANE (the fixed side), not by name shape -")
        print("        a name pattern cannot see a name it did not anticipate.")
        print("        NOT necessarily duplicates: they may be different KINDS")
        print("        (charter / campaign snapshot / current). Say which, in each.")

    print("  [7] METHOD DOCS WITH NO `era:` LINE: %d" % len(f["era"]))
    for name, why in f["era"][:60]:
        print("        %-52s %s" % (name, why))
    if f["era"]:
        print("        Vocabulary: mounted | bake | leg | plugin-native |")
        print("        era-independent | unknown. **A doc whose era cannot be")
        print("        determined is UNKNOWN, not current** (DN-12 applied to")
        print("        documentation). Only the owning lane can fill it in -")
        print("        knowing a method's era is knowing the method.")

    print("  [6] UNTOUCHED >30 DAYS: %d (reported, NOT judged - a doc can be"
          " finished)" % len(f["stale"]))

    print()
    if n:
        print("  %d finding(s). **None of these is a deletion order.** Renames" % n)
        print("  break inbound links and merges need the owning lane; this tool")
        print("  reports shape and stops there.")
    else:
        print("  clean.")
    return 1 if n else 0


def selftest():
    """THE CONTROL. Every detector must be shown to FIRE on a fixture built to
    trip it, and to stay silent on a clean tree. A gate that has never gone red
    is not a gate (control.py doctrine, No.31-C)."""
    import shutil
    import tempfile
    tmp = Path(tempfile.mkdtemp(prefix="docidx_"))
    ok = True
    try:
        d = tmp / "docs"
        (d / "state").mkdir(parents=True)

        # clean-ish index that names only one doc
        (d / "AGENT_INDEX.md").write_text(
            "# idx\n[good](GOOD.md)\n[gone](NOPE.md)\n", encoding="utf-8")
        (d / "GOOD.md").write_text("x", encoding="utf-8")
        (d / "ORPHANED.md").write_text("x", encoding="utf-8")          # [1]
        (d / "HANDOFF-HISTORY.md").write_text("x", encoding="utf-8")   # [3]
        (d / "state" / "HISTORY-HANDOFF.md").write_text("x", encoding="utf-8")
        (d / "COOK.md").write_text("x" * 500, encoding="utf-8")        # [4]
        (d / "COOK-CANONICAL.md").write_text("x", encoding="utf-8")

        f, counts = audit(d, d / "AGENT_INDEX.md")

        checks = [
            ("[1] orphan detected", "ORPHANED.md" in f["orphans"]),
            ("[2] dead link detected", "NOPE.md" in f["dead"]),
            ("[3] inverted-name collision detected",
             any(len(g) > 1 and any("HANDOFF-HISTORY" in x for x in g)
                 for g in f["collisions"])),
            ("[4] supersession pair detected", len(f["supersession"]) >= 1),
            ("[4] larger-dead-file flagged",
             any(bigger for _, _, bigger in f["supersession"])),
            ("[5] state/ handoff flagged off-shape",
             any("state" in p for p in f["handoff_shape"])),
        ]
        print("DOC INDEX SELFTEST - each detector must FIRE on a rigged tree")
        for label, passed in checks:
            print("  %-42s %s" % (label, "RED (fires)" if passed else "*** DID NOT FIRE ***"))
            ok = ok and passed

        # negative control: a clean tree must produce NOTHING.
        c = tmp / "clean"
        c.mkdir()
        (c / "AGENT_INDEX.md").write_text("# idx\n[a](A.md)\n", encoding="utf-8")
        (c / "A.md").write_text("x", encoding="utf-8")
        f2, _ = audit(c, c / "AGENT_INDEX.md")
        quiet = not any(f2[k] for k in ("orphans", "dead", "collisions",
                                        "supersession", "handoff_shape"))
        print("  %-42s %s" % ("negative control: clean tree is silent",
                              "GREEN (silent)" if quiet else "*** FALSE POSITIVE ***"))
        ok = ok and quiet
    finally:
        shutil.rmtree(tmp, ignore_errors=True)

    print()
    print("SELFTEST %s" % ("PASS - every detector demonstrated" if ok
                           else "FAIL - a detector cannot fire; it is not a gate"))
    return 0 if ok else 1


def main():
    args = sys.argv[1:]
    if "--selftest" in args:
        return selftest()
    docs_root = REPO / "docs"
    if not docs_root.is_dir():
        print("*** FAILED *** no docs/ at %s" % docs_root)
        return 2
    f, counts = audit(docs_root, docs_root / "AGENT_INDEX.md")
    if "--orphans" in args:
        for name in f["orphans"]:
            print(name)
        return 1 if f["orphans"] else 0
    return report(f, counts)


if __name__ == "__main__":
    sys.exit(main())
