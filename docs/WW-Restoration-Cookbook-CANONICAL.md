# WW Restoration Cookbook — CANONICAL

**This is the only writable cookbook.** The two prior copies forked (542 diff lines, bus §106) and
neither can prove accuracy; both are frozen and marked `UNCERTAIN ACCURACY — DO NOT WRITE`:
[docs/WW-Restoration-Cookbook.md](WW-Restoration-Cookbook.md) ·
[docs/WW Linked/WW-Restoration-Cookbook.md](WW%20Linked/WW-Restoration-Cookbook.md).

**Admission protocol (user-ruled 2026-07-23):** an entry enters this book only via one of —
1. **RECEIPT** — a traceable ratification (user words + bus §/№ + date);
2. **BILATERAL** — present in both frozen copies with equivalent meaning (still cite where);
3. **RE-VERIFICATION** — re-proven against source (decomp/tool/measurement), cited.

Entries below carry their admission route. Anything not yet here is *pending*, not void — see the
corroboration index at the bottom.

---

## ADMITTED DOCTRINE

### The Receiver Covenant + Space-Purity core (№31)
**Route: RECEIPT** — user's founding decree (Housing charter, session ba0af71d; enforced all
session; encoded as a tool in commit `16ec60c842`).
- The vanilla exe + game tree contain ZERO WW content — no WW bytes, no WW file names, no WW
  dialogue strings. All WW content lives ONLY in the mod folder. `greplist.txt` gates builds
  (`ww_bridge gate`; M5a/M6).
- WW spaces get only WW assets; TP spaces only TP assets; no cross-pollination ever (rupees
  included — WW Vlupy visual, TP wallet credit). A missing prop is always preferable to a foreign
  one.
- NEVER COMMIT: WW arcs, anything under the mod folder, WW-named files.

### №31-B — Purity covers SHAPING, not just supply
**Route: RECEIPT** — user 2026-07-20: "№31 should be written for assets, music, lighting,
everything." Any receiver stage that modifies donor content on its way to the player is a purity
surface. Corollary: verify at the stage the player perceives, not the stage easiest to hash.
(Case receipts: grass colour via TP kankyo; TP velocity curve squaring donor audio.)

### №31-C — A check that cannot run must report UNKNOWN, never CLEAN
**Route: RECEIPT** — user 2026-07-21/22: "Put it in cookbook" (bus §61/§61c/§74 lineage). Green
must prove the check executed. Corollary: a pass must state what it inspected (path, count,
version) so it is falsifiable from its own output. (Origin: silent ported guards + the gate's own
empty-greplist false-CLEAN, fixed Bridge 0.19.0.)

### IVAN RULE — no invented identity labels
**Route: BILATERAL + RECEIPT** (referenced in both frozen copies; enforced/ratified continuously;
violation case receipts bus §47, §89). Names and resemblance are never evidence. Labels stay
`? (unverified)` until locked by decomp or user identity pass.
- *Etymology (user receipt 2026-07-22, bus §104):* "Ivan" was the color test — the color/lighting
  test subject's name during the demo-item work. `Ivan` is a greplist marker: the nickname must
  never appear in shipping code. The lighting recipe formerly nicknamed "Ivan/boots" is the
  **NEUTRAL-AMBIENT RECIPE**: `settingTevStruct(TEV_TYPE 0)` + neutral ambient, no MAJI, no warm
  tint (`d_a_demo_item.cpp:519` era).

### Donor-export naming law
**Route: RECEIPT** — user 2026-07-22 (bus §89/§89c; memory `donor-export-naming`). Donor-derived
exports carry the donor's own addressing as filenames (`IsleLink_0_wave025.wav`); hypothesis and
fan labels never reach filenames; interpretation goes in sidecars.

### Document-lifecycle rules (this book's own hygiene)
**Route: RECEIPT** — user rulings 2026-07-23 (bus §106/§107):
- Before assigning a bus §-number, grep the tip for the current maximum.
- Doctrine edits happen HERE only. Frozen copies are read-only sources.
- "PROVEN PATH" pointers must carry date + version; consumers diff against current standard
  before cloning (assessment rec 3, user-accepted by adoption of this book).

---

## PENDING CORROBORATION INDEX (not yet law — verify, then move up)

| candidate | lives in | corroboration route |
|---|---|---|
| **Presentation-Parity Principle (№255/№256)** | root copy only | History verifies against №-ledger №255/№256 receipts → RECEIPT |
| №31-D "completeness before attribution" | proposed bus §82 | awaiting explicit user ratification |
| DECOMP-FIRST banner (№177–№181 case) | linked copy | matches memory `reference_ww_extract`; History confirms ledger №176–№181 → RECEIPT |
| OffsetPos banner (№165/№175) | linked copy | History confirms ledger entries → RECEIPT |
| №22 split transform, №39 division of labor, FAIL-CLOSED, №50-B, remaining §0 doctrine | linked copy | per-entry ledger receipts (History) or re-verification (owning lane) |
| Process/pipeline chapters (steps 0–N), scaling notes | both copies, diverged | owning lanes re-verify against current practice; stale steps die here instead of migrating |

**Assignment:** each lane corroborates the entries it owns; the user ratifies on receipt; Housing
moves ratified entries up and records each move in the bus. Migration is incremental — no
deadline theater; the index shrinking is the metric.
