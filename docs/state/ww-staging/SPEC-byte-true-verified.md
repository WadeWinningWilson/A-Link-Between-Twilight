# SPEC — feeding `BYTE-TRUE` and `VERIFIED` from the oracle stack

> Foundry scope, 2026-08-16, closing the roadmap's last unfed axes.
> **Scoping only — nothing here is built.** Follows `SPEC-boot-join.md`'s
> shape because that spec survived three mechanism corrections and this one
> should inherit the survivors, not rediscover them.

## 1. The gap, stated as a count

Five axes; three now have feeders (`PORTABLE` ← decomp_watch, `LINKED` and
`EXERCISED` ← boot_join + the two-axis manifest). **`BYTE-TRUE` (§4.4) and
`VERIFIED` (§4.5) have vocabulary, a laundering gate, and no feeder.** Every
value they hold today was typed by a lane, which is the drift the other three
axes just stopped tolerating.

The two axes answer different questions and MUST NOT share a feeder path:

- **BYTE-TRUE**: is the ported artifact byte-identical to the donor's?
  (BYTE-TRUE · EQUIVALENT · DIVERGENT · UNKNOWN) — a claim about BYTES.
- **VERIFIED**: does the seam BEHAVE as the donor does on a real run?
  (MATCH · DRIFT · UNKNOWN) — a claim about BEHAVIOUR.
  §801's whole point is that these are different claims; the feeder design
  must keep the distance the row store now enforces.

## 2. The evidence that already exists

- **BYTE-TRUE side** — differential oracles, all shipped or ruled:
  - `sig_diff` verdicts (declaration-level SAME/DIVERGENT);
  - `upstream_conformance` (symbol present/safe on both images);
  - the two-parser differential oracle pattern (color.bmc precedent:
    two independent implementations read one donor file, byte-compare all
    entries, a poisoned-byte fixture proves the gate can go red);
  - `decomp_status` (a NONMATCHING source CAPS this axis at DIVERGENT —
    the store and build gate already refuse the laundered combination).
- **VERIFIED side** — the Foundry instrument roster the lane was founded on:
  golden traces, capture reels, probe differ; plus the boot receipts
  (selfproof_color, selfproof, warp_names) which are single-run
  behavioural comparisons already emitting PASS/FAIL-shaped lines.

## 3. Required behaviour (inherited from boot-join, with the survivors)

1. **Report disagreement; never write a row.** A copied verdict is a second
   roster.
2. **Missing oracle output is UNKNOWN, not clean** — refuse, exit non-zero.
3. **Every emitted value carries `src`** (INVENTORY-SCHEMA §1):
   `byte-oracle:<tool>@<input-hash>` for BYTE-TRUE;
   `verify:<trace-or-log>@<build-id|UNPINNED>` for VERIFIED. A value with no
   src is UNKNOWN.
4. **BYTE-TRUE derivation must consult `decomp_status` first**: a
   NONMATCHING/EQUIVALENT source caps the axis at DIVERGENT regardless of
   what any downstream comparison says (the laundering invariant, enforced
   at derivation as well as at filing and build — three layers).
5. **VERIFIED=MATCH requires a named donor reference** (golden trace, donor
   log, or measured donor behaviour with a read timestamp). A MATCH against
   "what we expected" is a claim about expectations, not about the donor.
6. **Per-TU granularity** — one row, one TU, one verdict (the comma ruling).

## 3b. THE TWO TESTS BEHIND "VANILLA BYTE TRUE" (History/Bridge, Decoder-lane
## review row, 2026-08-17 — absorbed here because the ambiguity is exactly
## this spec's two axes wearing one phrase)

The phrase "vanilla byte true" RETIRES from review vocabulary; the lane is
graded on declared axes:

- **(A) THE DECOMP TEST → the BYTE-TRUE axis, donor-side.** The C++ compiles
  under the DONOR's PowerPC toolchain and the REL is SHA1-identical to
  retail (`config/GZLE01/build.sha1`, per-REL hashes — operationally
  defined). **Status: NOT OPERABLE on this machine** — the toolchain is
  fetchable (`download_tool.py`, compilers_tag 20251118), not present. The
  day it lands, a SHA1-clean REL derives `BYTE-TRUE` for that donor TU with
  `src=ppc-rel-sha1:<rel>@<hash>`, and the gate gets a control (one
  deliberately-perturbed byte must break the hash). **Consequence, stated
  as the lane's goal rather than a surprise: (A)-achieved work IS the
  decomp and belongs upstream in zeldaret/tww** — a new retirement class
  (drain-by-upstreaming) for Phase C's ledger.
- **(B) THE PORT TEST → the VERIFIED axis**, exactly as §1–3 spec it: our
  x86-64 build behaves identically, judged by the oracle stack. Byte
  identity to a GameCube REL is not a property our binary can have; (B) is
  what the port needs, and it is operable today.

A review that says "byte true" without naming (A) or (B) is refusable on
its face.

## 4. Controls it must carry (same-change rule)

- a poisoned byte in a differential-oracle fixture ⇒ BYTE-TRUE must derive
  DIVERGENT, never BYTE-TRUE (the color.bmc control, generalized);
- a NONMATCHING source with a byte-equal comparison ⇒ capped DIVERGENT
  (proves layer-3 of the laundering defense);
- an absent trace/log ⇒ REFUSED, exit non-zero;
- a golden trace that differs in one frame ⇒ VERIFIED must derive DRIFT;
- every emitted value must contain its `src` string.

## 5. Deliberately out of scope

- Building any new oracle. This joins EXISTING oracle output to rows.
- The fork's C3/V10-a receipts (frozen until a fork-run log is pinned).
- Rewriting rows, ever.
- Announcing anywhere but CALLS.md via `file_row.py`, one line.

## 6. Honest limits

- BYTE-TRUE for whole-subsystem ports is per-ARTIFACT (file/blob), not
  per-TU; where a row's TU maps to multiple artifacts the feeder reports
  per-artifact and the row takes the PESSIMISTIC aggregate (§P4).
- VERIFIED is route-scoped like EXERCISED: a MATCH on the Outset loop says
  nothing about Windfall. Report the route with the verdict.
- The first real feed waits on the same event as everything else: a boot
  log pinned to a build id.
