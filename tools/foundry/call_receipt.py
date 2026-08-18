#!/usr/bin/env python3
# ============================================================================
# call_receipt.py — DID THE ADDRESSEE ACTUALLY RECEIVE THE CALL?
#
# User's ask, verbatim: "there needs to be a check system that a lane RECEIVED
# their call on calls.md".
#
# THE GAP: filing and receiving are the SAME EVENT in our system and are not
# the same thing. A row can be correctly filed and reach nobody — the sender
# sees a filed row and reasonably assumes delivery. Proven twice: §837 routed
# a lane and no row was ever written (caught by routing_check.py), and a row
# that DOES exist reaches nobody if that lane is not watching.
#
# THE BLOCKER THE INTEGRATOR MEASURED, AND WHY THIS TOOL DOES NOT INHERIT IT:
# they found the trailing ATTRIBUTION field unparseable (`| Integrator, … |`
# vs `| tale §970 (Foundry) |` vs `| tale §925 |` with no lane at all), so a
# key-on-lane scan finds only 2 live lanes and Foundry reads as never having
# signed anything. TRUE — and avoidable, because receipt does not need that
# field. Measured instead:
#     · WHO a row is addressed to  -> `- [ ] LANE |` prefix, ALREADY uniform
#     · WHEN it was filed          -> the § it cites (118 of the rows do)
#     · WHETHER the lane is alive  -> bus section headers `## §N LANE`.
#       Measured on the tale bus: 451 `## §` sections, 386 with a parseable
#       `## §N LANE` header, 268 naming a roster lane. FIVE lanes are visible
#       that way — FOUNDRY, HISTORY, HOUSING, HOUSINGTEMP, INTEGRATOR — versus
#       the TWO an attribution-field scan finds. **LIBRARIAN is visible on
#       NEITHER**, which is a finding and not a bug: they are reported UNKNOWN
#       rather than silent, because absence of a signal is not evidence of
#       silence (№31-C).
# So this needs NO format migration and no new protocol for lanes to adopt.
# (Same principle as reading the symbol manifest the exe already carries
# instead of demanding a new one be produced.)
#
# WHY § IS THE CLOCK AND THE DATE IS NOT: every row dated today carries the
# same `2026-08-14`, so dates cannot order anything. § numbers are issued
# monotonically by staging.py's allocator, so they ARE a usable clock — and
# they are per-bus, so a tale § is NEVER compared against an interconnected §.
#
# WHAT IT CAN AND CANNOT PROVE — the honest limit, built in from the start:
#   · CAN prove a row is PROBABLY UNRECEIVED: addressed to a lane that has
#     authored NOTHING since the row was filed. Silence is a strong signal.
#   · CANNOT prove receipt. A lane active since may still not have read THIS
#     row, so an active lane is reported ACTIVE-SINCE (unconfirmed), never
#     "received". No verdict in this tool ever asserts delivery.
#   · CANNOT catch the §837 case (row never written) — that is
#     routing_check.py. The two are COMPLEMENTS, not substitutes, and
#     "0 unreceived" must never be read as "0 dropped calls".
# Absent/unknown is reported as UNKNOWN and never as a pass (№31-C).
#
# Usage:  call_receipt.py [--all] [--lane LANE]
#         --all   include answered rows (default: open rows only)
# Exit 0 nothing probably-unreceived · 1 something is · 2 could not run.
# ============================================================================
import re
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
CALLS = REPO / "docs" / "state" / "ww-staging" / "CALLS.md"
BUSES = {
    "tale": REPO / "docs" / "state" / "ww-tale-dmesg-live-state.md",
    "interconnected": REPO / "docs" / "WW Linked" / "ww-bridge-tool-interconnected.md",
}

# Lane canonicalisation. HOUSINGTEMP is a DISTINCT lane from HOUSING and must
# not be folded into it. Bridge is covered by History per the user's ruling
# ("File their call as History/Bridge"), so they share a clock.
# HOUSINGTEMP AND INTEGRATOR ARE THE SAME INSTANCE (user, 2026-08-14):
# "HousingTemp/Integrator". They were modelled as two lanes, which is why an
# `ALL` row expanded to both and why HOUSINGTEMP read as SILENT SINCE tale
# §697 — that identity simply stopped posting under its old name while the
# same instance kept working as INTEGRATOR. **That false reading is one of the
# two hits in the accusation I withdrew at tale §977**, and this is its actual
# root cause: not a stale lane, a merged identity the roster never recorded.
ALIASES = {
    "HOUSINGTEMP": "INTEGRATOR",
    "HOUSING": "HOUSING",
    "ENGINE": "HOUSING",
    "HISTORY": "HISTORY",
    "BRIDGE": "HISTORY",
    "INTEGRATOR": "INTEGRATOR",
    "FOUNDRY": "FOUNDRY",
    "LIBRARIAN": "LIBRARIAN",
}
ROSTER = ["FOUNDRY", "HISTORY", "HOUSING", "INTEGRATOR", "LIBRARIAN"]

# CASE-AGNOSTIC LANE FIELD (Housing's 5th delivery-failure shape, and this
# tool had it too). A character class of [A-Z/ ] silently DROPS a row
# addressed to `INTEGRATOR/HousingTemp` — the lowercase fails the class, so
# the row is invisible BY CONSTRUCTION, not by inattention. Enumerate on
# "anything up to the first pipe" and let canon() fold the case.
RE_ROW = re.compile(r"^- \[( |x|X)\]\s+([^|]+?)\s*\|")
RE_HEAD = re.compile(r"^##\s*§(\d+)[a-z]*\s+([A-Za-z/]+)")
# "tale §970" / "interconnected §837" / bare "§925"
RE_QUAL = re.compile(r"\b(tale|interconnected)\s*§(\d+)", re.I)
RE_BARE = re.compile(r"§(\d+)")


def canon(tok):
    t = tok.strip().upper()
    if not t:
        return None
    if t in ("ALL", "ALL LANES"):
        return "ALL"
    return ALIASES.get(t.split("/")[0].strip())


def addressed(field):
    """The `- [ ] LANE |` prefix -> canonical lane list. ALL expands."""
    f = field.strip().upper()
    if f in ("ALL", "ALL LANES"):
        return list(ROSTER), True
    out = []
    for part in f.split("/"):
        c = ALIASES.get(part.strip())
        if c and c not in out:
            out.append(c)
    return out, False


def lane_clocks():
    """Per-bus max § authored by each lane, plus parse coverage."""
    clocks = {b: {} for b in BUSES}
    cover = {}
    heads = {}
    for bus, path in BUSES.items():
        seen = tot = 0
        head = 0
        if not path.is_file():
            cover[bus] = (0, 0, "MISSING")
            heads[bus] = 0
            continue
        for ln in path.read_text(encoding="utf-8", errors="replace").splitlines():
            # DENOMINATOR = SECTION HEADERS ONLY. Counting every `##` line swept
            # in `###` subheadings and made coverage look far worse than it is
            # (268/569 vs the true 268/451) — a misleading denominator is the
            # same derived-view error class as measuring the wrong array.
            if not ln.startswith("## "):
                continue
            m = RE_HEAD.match(ln)
            if not m:
                continue
            tot += 1
            n = int(m.group(1))
            if n > head:
                head = n
            lane = canon(m.group(2))
            if not lane or lane == "ALL":
                continue
            seen += 1
            if n > clocks[bus].get(lane, 0):
                clocks[bus][lane] = n
        cover[bus] = (seen, tot, "ok")
        heads[bus] = head
    return clocks, cover, heads


def calls_liveness():
    """Lane -> max § cited by a CALLS row whose ATTRIBUTION names that lane.

    THE INTEGRATOR'S LIMIT 2: this tool read the buses only, so LIBRARIAN came
    back "no presence on ANY bus" x4 — literally true, and misleading, because
    they have signed rows in CALLS.md itself. A lane can be demonstrably active
    in the call surface and still read as bus-invisible.

    The attribution field is NOT reliably parseable (21 shapes from one lane in
    one day), so this is a SUBSTRING HEURISTIC and is honest about it. It is
    safe anyway because of an asymmetry: the result is used ONLY to RESCUE a
    lane from a silence verdict, never to produce one. An over-match makes the
    tool MORE CONSERVATIVE (one fewer accusation); it can never invent one.
    That is why a heuristic is acceptable here and would not be as a clock.
    """
    out = {}
    if not CALLS.is_file():
        return out
    for ln in CALLS.read_text(encoding="utf-8", errors="replace").splitlines():
        if "|" not in ln:
            continue
        attrib = ln.rsplit("|", 2)[0] if ln.count("|") >= 2 else ""
        _b, n = row_clock(ln)
        if n is None:
            continue
        up = attrib.upper()
        for lane in ROSTER:
            if lane in up and n > out.get(lane, 0):
                out[lane] = n
    return out


def row_clock(text):
    """(bus, N) the row was filed at, or (None, None). Qualified wins."""
    q = RE_QUAL.findall(text)
    if q:
        bus, n = max(((b.lower(), int(v)) for b, v in q), key=lambda t: t[1])
        return bus, n
    b = RE_BARE.findall(text)
    if b:
        return "tale?", max(int(v) for v in b)
    return None, None


def main():
    want_all = "--all" in sys.argv
    only = None
    if "--lane" in sys.argv:
        i = sys.argv.index("--lane")
        if i + 1 < len(sys.argv):
            only = canon(sys.argv[i + 1])

    if not CALLS.is_file():
        print("UNRESOLVED: %s not found (not proof of anything)" % CALLS)
        return 2

    clocks, cover, heads = lane_clocks()
    calls_clock = calls_liveness()

    # --why LANE — SHOW THE EVIDENCE. The Integrator tried to independently
    # verify a silence verdict, built a grep too loose to discriminate, and
    # reported the cross-check as FAILED rather than dressing it up. That is
    # the right call, and the fix belongs in the tool: an instrument that makes
    # an accusation should be able to show its working so another lane can
    # check it without reverse-engineering the measurement.
    if "--why" in sys.argv:
        i = sys.argv.index("--why")
        want = canon(sys.argv[i + 1]) if i + 1 < len(sys.argv) else None
        if not want:
            print("usage: call_receipt.py --why <LANE>")
            return 2
        print("EVIDENCE FOR %s — every parsed section header attributed to this lane."
              % want)
        print("Verify by opening the bus at these § numbers; if a section of theirs is")
        print("missing here, the header did not parse and the clock is WRONG.")
        print()
        for bus, path in BUSES.items():
            if not path.is_file():
                print("  %s: MISSING" % bus)
                continue
            hits = []
            for ln in path.read_text(encoding="utf-8", errors="replace").splitlines():
                if not ln.startswith("## "):
                    continue
                m = RE_HEAD.match(ln)
                if m and canon(m.group(2)) == want:
                    hits.append(int(m.group(1)))
            hits.sort()
            print("  %-14s %d section(s); head of bus §%d; lane clock §%s"
                  % (bus, len(hits), heads.get(bus, 0),
                     hits[-1] if hits else "NONE"))
            print("     last 12: %s" % (", ".join("§%d" % h for h in hits[-12:])
                                        or "none"))
        cc = calls_clock.get(want)
        print("  CALLS.md      authorship heuristic: %s"
              % (("newest attributed row cites §%d" % cc) if cc else "no attributed row"))
        print()
        print("  This is the WHOLE input to any verdict about %s. Nothing else is used."
              % want)
        return 0
    print("CALL RECEIPT — was the addressee in a position to have seen it?")
    print()
    for bus, (seen, tot, st) in cover.items():
        if st == "MISSING":
            print("  bus %-14s **MISSING** — lanes on it read as UNKNOWN, not silent"
                  % bus)
        else:
            print("  bus %-14s %d/%d headers parsed · lanes seen: %s"
                  % (bus, seen, tot,
                     ", ".join("%s@§%d" % (k, v)
                               for k, v in sorted(clocks[bus].items())) or "none"))
    print()

    unrecv, unknown, active, toonew, appends = [], [], [], [], []
    self_answered = [0]
    parent = None          # (lane_field, answered, clock_n) of the row above
    for ln in CALLS.read_text(encoding="utf-8", errors="replace").splitlines():
        m = RE_ROW.match(ln)
        if not m:
            # THE APPEND CASE — Housing's caution, and the one that has actually
            # bitten: their five real calls arrived as APPENDS to rows already
            # marked [x], not as new unchecked rows. A checkbox-keyed receipt
            # check scores those DELIVERED while the ask sits unread. (I nearly
            # shipped that hole: my own first move on the §970 trace was an
            # append to a row Housing had already checked off.)
            # Detectable without a new protocol: an append citing a § LATER than
            # its parent's proves the answer PREDATES the append, so the [x]
            # cannot possibly cover it.
            if parent and ln.startswith("  ") and ln.strip():
                _pb, pn, pans, plabel = parent
                _ab, an = row_clock(ln)
                if pans and an is not None and pn is not None and an > pn:
                    # An appended ANSWER is fine; an appended ASK is the failure
                    # mode. The tool cannot read intent, so it only drops the
                    # ones that DECLARE themselves answers and reports the count
                    # rather than pretending to have classified the rest.
                    low = ln.lower()
                    if "-> answered" in low or "answers the" in low:
                        self_answered[0] += 1
                    else:
                        appends.append((plabel, pn, an, ln.strip()[:88]))
            continue
        answered = m.group(1).lower() == "x"
        _pbus, _pn = row_clock(ln)
        parent = (_pbus, _pn, answered, ln[:88].replace("**", ""))
        if answered and not want_all:
            continue
        lanes, is_all = addressed(m.group(2))
        bus, n = row_clock(ln)
        label = ln[:96].replace("**", "")
        attrib_up = (ln.rsplit("|", 2)[0] if ln.count("|") >= 2 else "").upper()
        for lane in lanes:
            if only and lane != only:
                continue
            # AUTHOR IS NOT ADDRESSEE — Housing's finding, and a real bug I had
            # not caught. They file their ANSWERS as `- [x] HOUSING/ENGINE | …`,
            # so a row Housing WROTE parses as a row addressed TO Housing, and
            # the tool then reports that Housing may not have received it. You
            # cannot fail to receive your own row. Uses the same substring
            # heuristic as calls_liveness() and is safe for the same reason: it
            # only ever SUPPRESSES a finding, never creates one.
            if lane in attrib_up:
                continue
            if n is None:
                unknown.append((lane, label, "row cites no § — no clock, cannot order it"))
                continue
            b = "tale" if bus in ("tale?", None) else bus
            note = " (bus UNQUALIFIED, assumed tale)" if bus == "tale?" else ""
            have = clocks.get(b, {}).get(lane)
            if have is None and not any(lane in clocks[x] for x in clocks):
                # WORDING FIXED (Integrator LIMIT 2): "no presence on ANY bus"
                # reads as "this lane is absent", which is false — Librarian has
                # signed rows in CALLS.md and 10 mentions in WAVE-1.md. Name the
                # SURFACE that was searched, not the lane's existence.
                extra = calls_clock.get(lane)
                unknown.append((lane, label,
                                "not visible on the BUSES (CALLS.md authorship %s) — "
                                "no bus clock, so nothing can be ordered against it%s"
                                % ((("suggests activity near §%d" % extra) if extra
                                    else "also shows nothing"), note)))
                continue
            # CROSS-BUS LIVENESS. The CLOCK is per-bus (a tale § is never
            # compared to an interconnected §), but LIVENESS must not be, or a
            # lane posting on the other bus reads as silent. Caught before
            # shipping: this flagged the Integrator — the single most active
            # lane today — as probably-unreceived purely because their recent
            # work sat on the interconnected bus.
            # Ages translate across buses without any magic constant: each
            # bus's own head is "now", so the row is (head[B] - N) old, and a
            # lane whose age on ANY bus is smaller than that posted after it.
            row_age = heads.get(b, 0) - n
            # NO OPPORTUNITY = NO VERDICT. A row filed AT the current head is
            # 0 § old, so by construction no lane can be "newer" than it and
            # every addressee would flag as unreceived — noise that teaches
            # lanes to ignore the tool. The window is DERIVED, not picked: one
            # § is one lane posting one section, so until at least one section
            # per lane has been posted, no lane has demonstrably had its turn.
            if row_age < len(ROSTER):
                toonew.append((lane, label,
                               "only %d § posted since filing (< %d lanes) — no lane has "
                               "had its turn yet, so silence proves nothing"
                               % (row_age, len(ROSTER))))
                continue
            newer_on = None
            for c in clocks:
                lm = clocks[c].get(lane)
                if lm is None:
                    continue
                if heads.get(c, 0) - lm < row_age:
                    newer_on = (c, lm)
                    break
            if have is not None and have >= n:
                active.append((lane, label,
                               "active since: lane reached %s §%d ≥ row@§%d%s"
                               % (b, have, n, note)))
            elif newer_on:
                active.append((lane, label,
                               "active since, ON ANOTHER BUS: %s §%d is newer than this "
                               "row (row is %d § old on %s)%s"
                               % (newer_on[0], newer_on[1], row_age, b, note)))
            elif calls_clock.get(lane) is not None and \
                    heads.get(b, 0) - calls_clock[lane] < row_age:
                # CALLS-ROW RESCUE (Housing's (5) / Integrator's LIMIT 2). A lane
                # that works through CALLS rows rather than bus sections is
                # invisible to a bus-header liveness test — Housing's last tale
                # section is §934 but they have filed 19 CALLS rows since, so the
                # tool called them unreachable ~9 times. Librarian is the total
                # case; Housing is the partial case, which is worse because it
                # produces confident wrong output instead of an honest UNKNOWN.
                # Housing proposed fixing the attribution format first. Not
                # needed: the substring match is imprecise, but it is used ONLY
                # to rescue, so an over-match costs one suppressed accusation and
                # can never manufacture one. No lane has to change how it writes.
                active.append((lane, label,
                               "active since, via CALLS AUTHORSHIP: a row attributed to "
                               "this lane cites §%d, newer than this row (heuristic — "
                               "rescue only, never used to accuse)%s"
                               % (calls_clock[lane], note)))
            else:
                unrecv.append((lane, label,
                               "silent on EVERY bus since filed: row@%s §%d, lane's last "
                               "%s section %s%s"
                               % (b, n, b, ("§%d" % have) if have else "none", note)))

    if unrecv:
        # ONE FINDING PER LANE, NOT PER ROW (Housing's (3)/(4)). Silence is a
        # fact about a LANE, not about each row addressed to it, and every
        # `ALL` row multiplied it. A receipt check that cries wolf gets ignored,
        # and then it fails at the one job it exists for.
        bylane = {}
        for lane, label, why in unrecv:
            bylane.setdefault(lane, [why, []])[1].append(label)
        print("  **PROBABLY UNRECEIVED — %d lane(s), addressee silent on every surface "
              "since the row was filed:**" % len(bylane))
        for lane, (why, labels) in sorted(bylane.items()):
            print("    %-12s %s" % (lane, why))
            print("      affects %d open row(s); oldest: %s" % (len(labels), labels[0]))
        # CONFIDENCE, next to the verdict rather than only in the header
        # (Integrator LIMIT 1). Every verdict rests on a partial view.
        worst = min((s / t if t else 0) for s, t, _ in cover.values())
        print("    CONFIDENCE: bus header parse coverage is as low as %.0f%% on one bus, "
              "so a lane" % (worst * 100))
        print("      active ONLY in unparsed headers is indistinguishable from a silent "
              "one. Treat as a")
        print("      PROMPT TO CHECK, not a conclusion.")
        print()
    if unknown:
        # Same collapse as UNRECEIVED: an `ALL` row repeats one fact six times.
        u = {}
        for lane, _label, why in unknown:
            u[(lane, why)] = u.get((lane, why), 0) + 1
        print("  UNKNOWN (%d distinct) — not a pass, and not a failure either (№-31-C):"
              % len(u))
        for (lane, why), cnt in sorted(u.items()):
            print("    %-12s %s%s" % (lane, why, ("  [x%d rows]" % cnt) if cnt > 1 else ""))
        print()
    if active:
        print("  ACTIVE-SINCE (%d) — the lane was alive after this row was filed. **This "
              "is NOT proof they read it**, only that silence does not explain it."
              % len(active))
        for lane, _label, why in active[:6]:
            print("    %-12s %s" % (lane, why))
        if len(active) > 6:
            print("    ... %d more" % (len(active) - 6))
        print()

    if toonew:
        print("  TOO NEW TO JUDGE (%d) — filed too recently for silence to mean anything."
              % len(toonew))
        for lane, _l, why in toonew[:3]:
            print("    %-12s %s" % (lane, why))
        if len(toonew) > 3:
            print("    ... %d more" % (len(toonew) - 3))
        print()

    if appends:
        print("  **APPEND AFTER ANSWER (%d) — the checkbox says delivered and CANNOT be, "
              "because the append cites a LATER § than the row it hangs on. %d further "
              "append(s) declared themselves ANSWERS and were dropped; these did not, so "
              "they may carry an unread ASK:**" % (len(appends), self_answered[0]))
        for plabel, pn, an, txt in appends:
            print("    parent [x] @§%s  <-  append @§%s" % (pn, an))
            print("      parent: %s" % plabel)
            print("      append: %s" % txt)
        print()

    print("  LIMITS, stated so the verdict is not over-read:")
    print("   · This CANNOT prove receipt. No verdict above asserts delivery — the")
    print("     strong signal is the negative one (silence since filing).")
    print("   · It CANNOT see a call that was never filed as a row. That is")
    print("     routing_check.py's job; the two are COMPLEMENTS.")
    print("     **'0 probably-unreceived' does NOT mean '0 dropped calls'.**")
    print("   · A lane whose bus headers do not parse reads UNKNOWN, never 'silent'.")
    print("   · APPEND-AFTER-ANSWER is detected only when the append cites a § later")
    print("     than its parent. An append citing NO § is invisible here — so a clean")
    print("     append report is not proof that no append went unread.")
    return 1 if unrecv else 0


if __name__ == "__main__":
    sys.exit(main())
