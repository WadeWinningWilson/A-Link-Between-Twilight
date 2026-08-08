# Librarian Queue — routed flags

> **What this is.** The Librarian's flag registry. Each entry is a *structural* observation
> — a lint, a coverage gap, a stale pointer, or a content contradiction the Librarian is
> **not allowed to decide.** Every entry names the **lane it routes to via the user.** The
> Librarian never resolves a routed flag itself; it only records, and executes the
> *mechanical* fix once the owning lane/user rules.
>
> Charter: [Librarian.md](Librarian.md). This file is librarian-owned; lanes read their
> routed entries and reply through the user.

**Legend — type:** `LINT` (mechanical) · `COVERAGE` (something risks being lost) ·
`STALE` (pointer/number drifted from its authority) · `CONTENT` (truth question — Librarian
does **not** rule) · `ORPHAN` (missing back-link/discoverability).
**Status:** `OPEN` · `ROUTED` (awaiting owning-lane/user) · `CLEARED`.

---

## A. Cookbook fork (three files; canonical mechanism already ruled 2026-07-23, bus §106/§107)

Files: canonical [WW-Restoration-Cookbook-CANONICAL.md](WW-Restoration-Cookbook-CANONICAL.md) (writable) ·
frozen fork **A** [WW-Restoration-Cookbook.md](WW-Restoration-Cookbook.md) ·
frozen fork **B** [WW Linked/WW-Restoration-Cookbook.md](WW%20Linked/WW-Restoration-Cookbook.md).

| ID | Type | Finding | Evidence | Routes to | Status |
|----|------|---------|----------|-----------|--------|
| CB-1 | LINT | CANONICAL's pending-corroboration index lists the **DECOMP-FIRST** and **OffsetPos** banners as living in "linked copy" only. They are **byte-identical in BOTH forks** — i.e. **BILATERAL**, the cheapest admission route, not the History-confirm route the index assigns. | `diff` of lines 10–65 in A vs B = identical; banners present in both. | Index owner (Housing / canonical maintainer) | ROUTED |
| CB-2 | COVERAGE | Fork **A**-only technical chapters — lighting 4-table chain, RTBL trap, veg-asset extraction, arc-adaptation internals — are **not tracked as distinct pending rows**. The catch-all row calls process chapters "both copies, diverged," but lighting is A-only. If the index drains and the forks retire, A-only doctrine is **silently lost.** | `grep -c PAL0`: A=3, B=0. Companion/`PARENT-COMPOSE`: A=0, B=1 (mirror gap, B-side). | Index owner | ROUTED |
| CB-3 | STALE | Both fork banners assert **"542 diff lines"**; live `diff` now reports **534** (files changed after the banner was written; charter said ~536). Cosmetic. Candidate to replace with a generated count per the `inventory.md` "ground truth is the tool" model. | `diff A B \| grep -c '^[<>]'` = 534. | Index owner | ROUTED |
| CB-4 | CONTENT | Fork **A** §0 states №31 in its **older, narrower** form ("donor spaces receive only donor assets"); fork **B** §0 is the **broadened 2026-07-20** form (rupees, №31-B shaping, №31-C UNKNOWN). Canonical adopted **B**. **Is A's narrower §0 purely superseded, or does it phrase anything B/canonical dropped?** Librarian does **not** rule this. | Presentation-Parity: A=1, B=0. `COVERS SHAPING`/`CANNOT RUN`: A=0, B=1. | **History** (owning lane) | ROUTED |

**Librarian's own remaining mechanical tasks (do only on user go-ahead):** when the pending
index drains, collapse both frozen forks into one-line pointers to CANONICAL.

---

## B. Governance / process-model propagation (lane split + Librarian)

Scan question (user, this session): *do the docs abide by the lane/tool-split work process, or
are they products of it?* Answer: the **product docs** (state tips, recipes, handoffs, research)
are governed correctly via AGENT_INDEX → state; the **governance layer itself has not absorbed
the newest model** (four lanes + Librarian + ledger-as-data). Details:

| ID | Type | Finding | Evidence | Routes to | Status |
|----|------|---------|----------|-----------|--------|
| P-1 | ORPHAN | ~~The **Librarian** lane is referenced by exactly one file — its own charter.~~ **CLEARED 2026-07-25:** back-links added from [AGENT_INDEX.md](AGENT_INDEX.md), [AGENTS.md](../AGENTS.md), and [LANES.md](LANES.md). Bus back-link left to Bridge (not a Librarian edit). | `grep -rl Librarian docs/` → `docs/Librarian.md` only (before). | User (orientation-doc owner) | CLEARED |
| P-2 | ORPHAN | ~~The four-lane roster + tool-split is not defined in any governance doc.~~ **CLEARED 2026-07-25:** roster shelved to [LANES.md](LANES.md), provenance-stamped to the user's ruling + existing anchors (cookbook §0 division-of-labor; Librarian.md). Content is the user's to correct; Librarian maintains structure only. | `grep` for roster terms in bus → 0 hits (before). | User | CLEARED |
| P-3 | STALE | AGENT_INDEX line 32 marks the **Active run** as *Cut-Actors / Demo Restore*; `run-control.md` (the designated authority) says the active run is *WW Bridge Tool — Bridge ↔ Housing Security*. | run-control `run_name` vs AGENT_INDEX:32. | User / orientation owner | ROUTED |
| P-4 | STALE | AGENT_INDEX's Interconnected-Runs table **omits** the currently-active WW-Bridge-Tool run doc, its bus (`WW Linked/ww-bridge-tool-interconnected.md`), and its state tip (`state/ww-bridge-tool.md` — not linked anywhere in AGENT_INDEX). | `grep ww-bridge-tool docs/AGENT_INDEX.md` → no match. | User / orientation owner | ROUTED |

**Note (Librarian):** P-1…P-4 are all *flag-only*. Adding the lane roster or rewriting
AGENT_INDEX's orientation is **authoring / governance** — outside Librarian scope. Fixing the
P-3/P-4 stale pointers to match `run-control` is mechanical lint the Librarian *can* execute,
but only on explicit go-ahead (it involves a curation choice: point at one run, or list both).

---

---

## C. №113-STASH LAW — propagation tracking (ratified doctrine; Librarian carries the propagation)

**Law home (verified 2026-07-25):** [`WW-Restoration-Cookbook-CANONICAL.md` §113-STASH LAW](WW-Restoration-Cookbook-CANONICAL.md)
(lines 66–80) — admitted via **RECEIPT**, user ratification 2026-07-26, **bus §148/§149**. Carries
the (a)/(b) valid-exception clause (evidence → Engine·History·Housing sign-off → change → law
update; never a silent unilateral repair) and the **§106 ratchet target** (stash-presence assert
in gate/verify). Handoff to Librarian recorded at **bus §149**.

**Librarian's standing lint rule (in force):** any doc, restore instruction, or hash/diff surface
that touches `PAL0 plight_col[2]` for F_DL01 `STG_00.arc` must **carry the stash forward** or
**whitelist** it. A "repair" of that slot that does not go through the exception process is a
**flag**, routed to the sign-off trio — never actioned by one lane.

**Coverage map (structural — do the restore-path docs already reference the stash?):**

| Surface | Type | Stash-referenced? | Note / route |
|---|---|:---:|---|
| `WW-Restoration-Cookbook-CANONICAL.md` §113-STASH LAW | doctrine home | — | the law itself; complete |
| [state/lighting-purple-black-research.md](state/lighting-purple-black-research.md) | PAL0 restore research | ✓ ×6 | well-covered; primary restore rationale |
| [WW Linked/lighting-palette-reference.md](WW%20Linked/lighting-palette-reference.md) | PAL0 layout reference | ✓ ×1 | mentions it; thin — candidate for an explicit law pointer |
| [state/cut-actors-demo-restore.md](state/cut-actors-demo-restore.md) (№270 `cp …pre-ba1getitm-bak`) | live restore instruction | ✓ ×2 | HOT file; leave to owning lane, don't stamp mid-run |

**The one true enforcement gap = tooling, and it's already routed:** the doc layer references the
stash; what's missing is the **automated assert**. Per the law's §106 ratchet and the user's
handoff, the **stash-presence assert / hash-whitelist belongs to Bridge** (deferred "when Bridge
recovers"). Librarian does not build it — it *operates* it once shipped and will add a coverage
row here when it lands. Restore paths in **code** are Engine/Bridge to gate, not docs.

| ID | Type | Finding | Routes to | Status |
|----|------|---------|-----------|--------|
| SL-1 | COVERAGE | Stash-presence assert (hash-whitelist + gate/verify check) not yet built — the law self-enforces only once it exists (§106 ratchet). | **Bridge** (deferred) | ROUTED |
| SL-2 | LINT | `lighting-palette-reference.md` references the stash only thinly (×1); candidate for an explicit pointer to the canonical §113-STASH LAW. Librarian can add the pointer on go-ahead (cross-link, not doctrine). | User (go/no-go) | OPEN |

---

## G. Aryll native-port fragmentation (2026-07-29 targeted sweep)

The Aryll port is spread across **6 docs in 4 subtrees, no cross-links between the overlapping pairs**, and — unlike the port-kit actors — she has **no consolidated kit**. Natural home = the [Aryll/](WW%20Linked/islands/Outset/characters/Aryll/) character folder. Librarian flags the overlaps; **does not merge or rule which is live** (content → History).

Doc map: [Aryll/README.md](WW%20Linked/islands/Outset/characters/Aryll/README.md) (hub) · [Aryll/voice-recipe.md](WW%20Linked/islands/Outset/characters/Aryll/voice-recipe.md) · [cutscene-audio-scoping.md](WW%20Linked/cutscene-audio-scoping.md) · [gaming systems/layers/outset-intro-layers.md](gaming%20systems/layers/outset-intro-layers.md) · [islands/Outset/outset-followup-plan.md](WW%20Linked/islands/Outset/outset-followup-plan.md) · [state/tale-step-in-step-hypotheses.md](state/tale-step-in-step-hypotheses.md) (tangential — Grandma tale, cites Aryll as a discriminator).

| ID | Type | Finding | Routes to | Status |
|----|------|---------|-----------|--------|
| A-1 | DEDUP | **RULED + EXECUTED (History 2026-07-29):** `voice-recipe.md` is LIVE (verified chain §85–93b; survives the native port, message-tied). `cutscene-audio-scoping.md` **§1 (voice) supersession-stamped → voice-recipe.md**; **§2 (opening music) left LIVE** (separate open item, not retired). | — | DONE |
| A-2 | DEDUP | **RULED + EXECUTED (History 2026-07-29):** NOT a dup — cross-link, don't merge. `outset-intro-layers.md` = verified census (live ref); `outset-followup-plan.md` = open TODO plan. **Reciprocal companion cross-links added.** `outset-followup-plan` also carries a **STATUS-REFRESH flag** (telescope demo now ported-but-dormant) → **History to update the content.** | History (content refresh) | OPEN (refresh) |
| A-3 | LINT (dup folder) | **port-kits/ has BOTH `npc-bm1/` and `npc_bm1/`** (hyphen vs underscore) — duplicate kit for the same actor, full 9-file spine each; also `npc-framework` breaks the `npc_*` convention. **History ruled: this is Foundry's kit-generation output, not History content → route to Foundry to collapse.** | **Foundry** (kit owner) | OPEN |
| A-hub | ORPHAN/DEDUP | **DONE 2026-07-29:** Aryll [character README](WW%20Linked/islands/Outset/characters/Aryll/README.md) now indexes the **8-doc** native-port set (History-specified additions folded in: cookbook recipes 10–15 + port-kits/HISTORY-KIT-FEEDBACK.md). Pointers only, no content moved. **Bonus: fixed 4 pre-existing broken links** in that README (all depth-off-by-one). | — | DONE |

---

## F. Foundry landing pass (2026-07-28/29) — 61 new files under docs/

Lint clean (0 conflict / 0 NUL). 43 of 61 linked; the `fact-sheets/` subfolder + Foundry hub are well-formed. Concerns are **placement + orphan hubs**, not integrity.

| ID | Type | Finding | Routes to | Status |
|----|------|---------|-----------|--------|
| P-9 | PLACEMENT | **`docs/WW Linked/` is overcrowding — 49 new loose files at the folder root** (donor-* maps, api-surface-*, enemy-port-surface-*, btk-*, dolphin-captures-*.txt) alongside recipes + the bus. The new `fact-sheets/` subfolder shows the right pattern; the donor/capture/enemy sets want the same (`donor/`, `captures/`, `enemy-port/`). **Caveat: a reorg moves 40+ files that 43 links point at → must be a single link-updating batch, and it's the owning lane's fresh work — needs their buy-in before I touch it.** Librarian can execute the move+relink once ruled. | Foundry / History / User | OPEN |
| P-10 | ORPHAN | **18 orphans cleared 2026-07-29 (bare pointer-links, user go-ahead):** 8 `donor-statemap-npc-*` now linked from their rows in [donor-npc-index.md](WW%20Linked/donor-npc-index.md) (index completed); remaining 10 (enemy/bokoblin-port set + misc) gathered in [UNFILED-foundry-orphans.md](WW%20Linked/UNFILED-foundry-orphans.md) as bare pointers. **Still owed by owning lane:** a real enemy-port hub, then delete the UNFILED scaffold. Descriptions were NOT authored (left to Foundry/History). | Foundry / History (build real hub) | CLEARED (discoverability) / OPEN (real hub) |
| P-11 | PLACEMENT | **6 `dolphin-captures-*.txt` capture artifacts loose in `WW Linked/`** — Foundry capture-reel *data*, not prose docs; belong in a `captures/` folder. Also confirm intent: committed reference vs regenerable scratch (dated 20260728). | Foundry | OPEN |
| P-12 | ORPHAN | `Foundry.md` reachable only via LANES.md (2-hop); its **state tip `foundry-methods.md` isn't in AGENT_INDEX's live-state map** (same class as P-4, ww-bridge-tool.md). One row fixes it. | User (go/no-go) | OPEN |

---

## H. "recipes 1–9" citation re-point (user chose A, 2026-08-06)

Citations of **"cookbook recipes 1–9"** as the crash-lint scope: `ttw-methods-review.md` **V5 (line 191)** + **Band-1 (line 226)**, and `ww-tale-dmesg-live-state.md` **§327 (line 241)**. User: "1–9 is likely a mis-reference → re-point (option A), don't renumber."

**Blocker (verified counts):** the "§9 with 18 rows" the user means is the **frozen `WW-Restoration-Cookbook.md` fork §9 "Failure classes" = 17 rows** (DO-NOT-WRITE). The **live** home is CANONICAL's "DIRECT-PORT CRASH RECIPES" = **9 recipes** (CANONICAL consolidated the old 17-class list into 9). So "recipes 1–9" currently matches the *live* count; re-pointing to §9 would cite frozen/superseded content. Also `§9` is overloaded (census "Spec §9" in `census_axis_*.py` is unrelated).

| ID | Type | Finding | Routes to | Status |
|----|------|---------|-----------|--------|
| H-3 | STALE CITATION | **DONE 2026-08-06 (Foundry ruled: the 9 live consolidated recipes; endorsed the method).** All 3 citations re-pointed to **"the cookbook's DIRECT-PORT CRASH RECIPES"** (named section + link to CANONICAL, **no hard count** → drift-proof): `ttw-methods-review.md` V5 + Band-1, `ww-tale-dmesg` §327. Verified: 0 stale "recipes 1–9" left, links resolve, no mixed endings. **Revisit trigger (Foundry, recorded):** a playtest failure matching a frozen-fork failure-class absent from the 9 → migrate that row into CANONICAL first, then extend. | — | CLOSED |

---

## E. Routed to owning lane (came to the Librarian, but out of scope)

> **✅ R-2 APPLIED 2026-07-27 (user go-ahead).** F-2 note stamped, ledger forward-stamped,
> chain-scope resolved (one stamp, history intact). **Only remaining piece = the `merge_event.py`
> note, routed to Bridge/Engine** (Librarian will not edit source). Ready-to-paste comment for
> Bridge to add near the Ba1_Get_Itm block (merge_event.py ~line 193):
>
> ```python
> # CLOTHES vs SOUP (user 2026-07-26): Ba1_Get_Itm is the SOUP give
> # (prm0=0x55=SOUP_BOTTLE), reserved for the post-kidnap recovery beat — NOT the
> # clothes give. The clothes/tale handover is a SEPARATE event; Grandma's cradle
> # pose (Demo01.arc/ba_wait_l.bck, noclip §F-2) belongs to it. Ref: Grandma README §36-44.
> ```


| ID | Type | Finding | Routes to | Status |
|----|------|---------|-----------|--------|
| R-4 | CONTENT CONTRADICTION | **✅ USER-LOCKED 2026-07-30 (identity authority): `Ji1`=ORCA · `Aj1`=STURGEON · `Ob1`=ROSE — confirmed via native data.** This ratifies bus §300 (was Foundry-asserted, now user-locked) and retires all three from the `[INFERENCE]`/unverified tier per the Ivan rule. **Librarian propagation state:** current-truth docs (`characters/README`, `layers-noclip-census`) already carry Aj1=Sturgeon + Ob1=Rose ✓ — nothing to rewrite. All remaining wrong forms (Ji1=Sturgeon ×6, Orca=Ob1 ×1) are in the **append-only bus** (lines 12108/12113/12157 inside §300's own correction narrative; 13678/13711/13736 = old historical entries) — **§300 supersedes them by the append-only mechanism; the Librarian does NOT rewrite append-only history.** **Routed to History:** sweep your own working docs (grandma-native-tale.md + related notes per Foundry) and add a `Ji1`=Orca row to the Outset roster (currently absent — content gap, History's to author with its evidence tier). Original finding below. | **History** (own-doc sweep + roster row) | RULED |
| ~~R-4 (orig)~~ | CONTENT CONTRADICTION | **Orca/Sturgeon/Ji1/Aj1 identity cluster is contradictory across live docs — Foundry's §300 correction is HALF-APPLIED.** Foundry ruled `Ji1`=ORCA / `Aj1`=STURGEON (bus §300, decomp receipts) and edited its own docs + the bus. But the live doc set still carries: **Ji1=Sturgeon ×6** (incl. bus lines 13646/13679/13704) vs **Ji1=Orca ×3** (§300) — a self-contradiction; **Aj1=Sturgeon ×6** (consistent — the stable thread); plus a **stray Orca=Ob1** (bus 13704) while `characters/README` says Ob1=Rose. **Librarian assessment (structural only, NOT archaeological): the correction is internally coherent and resolves a pre-existing incoherence** (old records mapped BOTH Ji1 and Aj1 to Sturgeon — impossible); **but I do NOT certify Ji1=Orca true** (decomp receipts are History's to evaluate) and I did NOT flip any names (identity = History's lane; "name on a face" = user's). **Routes to History:** rule the cluster authoritatively, then sweep — the 6 live Ji1=Sturgeon instances + the Orca=Ob1 stray. Archive/backup instances are frozen history (History decides whether to stamp). Librarian executes mechanical link/stamp fixes once History rules. Native-true work orders A (MULT co-render) + B (Ojhous variants) on §300 are Engine/Foundry (behavior), not Librarian. | **History** (rule + sweep) · Engine/Foundry (orders A/B) | OPEN |
| R-2 | SUPERSESSION | **Ba1_Get_Itm framing correction (user, 2026-07-26).** `Ba1_Get_Itm` = Grandma's **ELIXIR SOUP** give (prm0=0x55=SOUP_BOTTLE), **not** the clothes give; the earlier build repurposed it. The **pose finding stands** but applies to the **clothes/tale handover**, a separate event — not the soup get. Canonical source (already correct): [Grandma README](WW%20Linked/islands/Outset/characters/Grandma/README.md) lines 36–44. **Propagation status (2026-07-27):** (1) `merge_event.py` Ba1_Get_Itm notes → **ROUTED to Bridge/Engine** (source-file wall; Librarian does not edit) — ready-to-paste note below; (2) **ledger** → **STAMPED** — one forward supersession banner at the ledger top (№275/277/279/281 clothes-give framing marked SUPERSEDED; historical rows left intact = audit trail); (3) **F-2 note** → **STAMPED** — correction banner under [`noclip-fast-track.md`](WW%20Linked/noclip-fast-track.md) §F-2 (pose/`Demo01.arc` finding stands; "give scene = Ba1_Get_Itm" reframed to soup + clothes/tale handover). **Chain-scope call RESOLVED structurally:** one forward stamp, no retroactive rewrite of №275–281 — any deeper content reframe stays **History's** (flagged in the ledger banner). | Bridge/Engine (tool note only) | STAMPED — merge_event.py note → Bridge |
| R-1 | CONTENT/CODE | **Item 47 get-message routing.** User target (2026-07-26): our code computes **msg 148**; donor uses **WW msg 186** — point item 47's get-message at the WW clothes description string. This is a resolver change (code) or a BMG/data VALUE, plus a finding to verify against the donor. **Not a Librarian action** (no source edits, no ruling on 186's correctness). Bus **§163** already assigns "the message/BMG side of item 47" to **History** — reconcile with the user's `Bridge/Librarian (text)` tag. Librarian will keep docs consistent once it lands; no stale `item 47→msg 148` mapping exists to fix (the doc `148` hits are bus §148 STASH-LAW, unrelated). | **History** (verify + implement); user reconciles the ownership tag | ROUTED |

---

## D. Docs-tree hygiene (found during the post-merge overview)

| ID | Type | Finding | Routes to | Status |
|----|------|---------|-----------|--------|
| H-1 | LINT | ~~493 KB hidden pre-compaction backup stray in `state/`.~~ **CLEARED 2026-07-26 (user: "make backup"):** git-moved to [state/backups/](state/backups/) (un-hidden) + folder [README](state/backups/README.md) index. | User | CLEARED |
| H-2 | LINT | `docs/state/fps-bisect-restore/editor.cpp.wip` — a **153 KB C++ source WIP** living under `docs/`. Source-shaped file in the docs tree; likely belongs in `src/`-adjacent scratch, not `docs/`. | Engine/Bridge | **HELD** (user: hold off on FPS bisect) |
| P-7 | DEDUP | ~~gaming-systems overlaps cookbook/recipes with zero cross-links.~~ **RULED 2026-07-27 (user): the cookbook is now ARCHIVAL** (record of how the work began — too catch-all for the full scope); **per-system dedicated docs (`gaming systems/`) are the going-forward model.** **EXECUTED:** archival banner stamped on [CANONICAL cookbook](WW-Restoration-Cookbook-CANONICAL.md) + back-link added to [gaming systems/README](gaming%20systems/README.md) Companion docs. **NEW FLAG spun off → R-3** (below): archiving the cookbook orphans the live LAWS it holds. | User (via R-3) | RULED |
| P-8 | LINT | `§`-glyph spans ≥2 namespaces (bus § up to §377; gaming-systems code-checkpoint §180→§202 — ranges overlap, `§190` ambiguous). **User will contact History** — Librarian stands down; no action pending. | User → History | ROUTED |
| R-3 | CONTENT | **Consequence of the P-7 ruling: the archived cookbook still holds LIVE laws.** `## ADMITTED DOCTRINE` (Receiver Covenant, №31/-B/-C, Ivan rule, §113-STASH LAW, donor-export naming) is cited live by DN-2, gate/verify tooling, and this queue. The archival banner explicitly keeps them in force so nothing treats them as dead — but their **permanent home** and the **pending-corroboration index's migration target** (now that per-system docs are the model) need a ruling: dedicated doctrine doc / DO-NOT.md / stay-cited-from-archival? Librarian executes once ruled; does not decide doctrine placement. | **User / History** | OPEN |
| P-6 | ORPHAN | New Outset docs from this landing aren't reachable from a hub: **`outset-followup-plan.md` linked from 0** (full orphan); the `presence/` cluster (`tale-scene-scripts.md`, `layers-noclip-census.md`) internally linked but not from the Outset `characters/README.md` hub or AGENT_INDEX. Same class as P-5; one hub anchor fixes both. | User (go/no-go on hub row) | OPEN |
| P-5 | ORPHAN | The **WW-restoration recipe web** (wind/water/waves/shore/sky/lighting under `docs/WW Linked/`) is cross-linked internally but **not reachable from AGENT_INDEX** — no WW-recipe hub row. A fresh instance on the boot path wouldn't find the newly-landed effect docs. Librarian can add one pointer row on go-ahead (discoverability, not authoring). | User | OPEN |

---

*Maintained by the Librarian lane. Entries are structural, never truth rulings. Last swept: cookbook fork + governance scan + post-merge integrity + №113-STASH LAW propagation.*
