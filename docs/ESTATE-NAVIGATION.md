# Estate navigation — how this repo is organized, and what is currently wrong with it

era: era-independent
<!-- era rationale: describes the doc tree's shape, which outlives any port era | Librarian, 2026-08-16, user ruling "assign it by era" -->

**Librarian audit, 2026-08-16, user-ordered.** Measured, not impressionistic: every number below
came from a command, and the commands are named so you can re-run them instead of trusting this file.
Per [INVENTORY-SCHEMA §0](state/ww-staging/INVENTORY-SCHEMA.md), **cite the command, not the count** —
these figures were true on 2026-08-16 and will drift.

---

## 1. THE ONE FINDING THAT MATTERS: there are TWO navigation trees and NO bridge between them

| tree | entry point | covers | size |
|---|---|---|---|
| **Documents** | `CLAUDE.md` → [AGENT_INDEX.md](AGENT_INDEX.md) | docs, lanes, live state | 77 top-level docs |
| **Instruments** | `python tools/foundry/workflow.py` → `tool_index.py` | the 7-phase port workflow, every tool | 105 instruments |

**They do not reference each other. At all.**

```bash
grep -c "workflow.py\|tool_index" CLAUDE.md AGENTS.md docs/AGENT_INDEX.md   # 0, 0, 0
grep -c "tools/foundry" docs/AGENT_INDEX.md                                  # 0
```

**Consequence, and it is not hypothetical:** an instance onboarded the documented way — `CLAUDE.md`
→ `DO-NOT.md` → `AGENT_INDEX.md` — can work an entire session without ever learning that 105
instruments exist. `tool_index.py`'s own header records what that costs: *"A lane cannot use what it
cannot find… `l2c_equiv.py` carried its own four-way `--selftest` for its whole life while every lane,
me loudest, called it uncontrolled."* **That fix was correct and it was installed INSIDE the tool
tree — so it is only discoverable by someone already standing in the tool tree.** The index that
needed it never got it.

**The cheapest high-value change in this document is one line in `AGENT_INDEX.md` naming
`workflow.py`.**

---

## 2. Measured state

| measurement | value | command |
|---|---|---|
| top-level docs | 77 | `ls docs/*.md \| wc -l` |
| **docs unreachable from AGENT_INDEX** | **47 of 76 (62%)** | see §5 |
| dead links in AGENT_INDEX | **0** | link sweep |
| instruments | 105 | `ls tools/foundry/*.py \| wc -l` |
| **instruments with no purpose line** | **34 (32%)** | `tool_index.py \| grep -c UNDOCUMENTED` |
| docs untouched >30 days | 38 of 77 | mtime sweep |
| docs touched in last 7 days | 14 of 77 | mtime sweep |
| handoff files | 17, in **4 naming shapes** | §4 |

**The index is ACCURATE and INCOMPLETE.** Zero dead links is a genuinely good result — nothing in
`AGENT_INDEX.md` points at a file that does not exist. The problem is entirely the other direction.

---

## 3. Redundancy register — specific pairs, not a general complaint

**(a) TWO HISTORY HANDOFFS, inverted names, different directories, neither referencing the other.**

| file | lines | last touched | opens with |
|---|---|---|---|
| `docs/HANDOFF-HISTORY.md` | 179 | 2026-08-11 | *"Read this before touching WW-lane work"* |
| `docs/state/HISTORY-HANDOFF.md` | 141 | 2026-08-01 | *"Read this first, then…"* |

**Both claim to be the first thing a History successor reads.** They disagree by ten days of state.
A successor who finds the wrong one inherits 2026-08-01 and has no signal that a newer one exists.
**This is the highest-consequence redundancy in the estate** — handoffs are read exactly once, by
someone with no context to detect they got the stale copy.

**(b) THE COOKBOOK PAIR — the deprecated file is bigger and has the better name.**

| file | lines | last touched | status |
|---|---|---|---|
| `WW-Restoration-Cookbook-SUPERSEDED.md` | 319 | 2026-07-23 | header says **"UNCERTAIN ACCURACY — DO NOT WRITE"** |
| `WW-Restoration-Cookbook-CANONICAL.md` | 193 | 2026-07-30 | the live one |

**A `grep`/glob for "cookbook" surfaces the deprecated file first and it looks more substantial.**
The deprecation lives in a banner INSIDE the file, so it only works on a reader who opens it and
reads the top — not on a tool, a glob, or a hurried instance. **This estate's own repeated lesson is
that a rule in prose does not survive and a rule in a mechanism does. A filename is a mechanism.**

**(c) `HANDOFF-HOUSING-ENGINE.md` + `HANDOFF-HOUSING-ENGINE-2026-08-14.md`** — undated original plus
dated successor, with no marker on the original saying it was superseded.

---

## 4. Naming conventions — currently four shapes for one thing

**Handoffs (17 files):** `HANDOFF-<LANE>.md` · `<LANE>-HANDOFF.md` · `<feature>-handoff.md` ·
`state/<LANE>-HANDOFF.md`. There is no rule, so every reader must guess, and every writer invents.

**Instruments — the same split, inside a family that a matcher reads:**

- `*_map.py`: `cascade_map` `cc_map` `param_map` `state_map` — but `*_maps.py`: `palette_maps` `path_maps`
- census, **13 tools, three shapes**: `census_axis_[c|d|p|w].py` · `<subject>_census.py`
  (`bmt` `dzr` `layer` `scls` `tex_color` `tier2` `watcher` `ww`) · `census_selftest.py`

**Why a Librarian raises this rather than shrugging: the estate spent 2026-08-15 discovering seven
separate cases of a matcher whose blind spot was reported as a fact about the corpus** (the inert
`sig_diff` regex, `file_row`'s roster tokenizer, a `head -12`, tier2's `\bww_`, `port_preflight`'s
hardcoded donor path, the lineage gate's 18 invisible files, a C-class list capped at 20).
**A singular/plural split inside one tool family is that same hazard, pre-installed.** Anyone who
globs `*_map.py` silently misses two tools and will never see an error.

---

## 5. The gap behind all of it: nothing instruments the DOCS

The estate audits its bytes (`c0_audit`), its gates (`control.py`), its lane tokens
(`file_row.py lint`), its monitors (`watcher_census`, `monitor_pulse`), its bus routing
(`routing_check`, `call_receipt`), its provenance banners (`banner_lint`) and its symbols
(`sig_diff`, `upstream_conformance`).

**No instrument takes `docs/` as its subject.** That is why a 62% orphan rate was never a number
until today — not because anyone was careless, but because **every other corpus here has a tool that
would have said so.**

Re-run the orphan sweep with:

```bash
for f in docs/*.md; do b=$(basename "$f"); grep -qF "$b" docs/AGENT_INDEX.md || echo "ORPHAN $b"; done
```

---

## 6. What to do — ranked cheapest-first, and NONE of it is a rewrite

1. **One line in `AGENT_INDEX.md` pointing at `workflow.py`.** Bridges the two trees. *(Done in this
   pass — see the index header.)*
2. ~~A `doc_index.py`~~ — **BUILT 2026-08-16, control registered and RED** (`control.py check
   doc_index`). Run it instead of re-reading this file:

   ```bash
   python tools/foundry/doc_index.py            # audit
   python tools/foundry/doc_index.py --selftest # the control
   ```

   **IT CORRECTED THIS AUDIT TWICE WHILE BEING BUILT, AND BOTH CORRECTIONS ARE THE ESTATE'S OWN
   FAILURE CLASS:**
   - **v1's collision detector reported 17, of which 15 were port-kit TEMPLATE files** (every kit
     legitimately owns a `ccmap.md`, `closure.md`, `surface.md`). A structured template tree is not
     a collision. **Fixed to 3 real ones — and the fix's own first version still missed nested
     templates** (`<kit>/ported_src/codemod-report.md`), so the template detector had a blind spot
     of exactly the kind it was written to find.
   - **The handoff detector grouped by NAME SYMMETRY and therefore found only two of THREE History
     handoffs.** It could not see `docs/state/ww-handoff-history-bridge.md` — neither spelling —
     **and that was the CURRENT one, the one a successor most needs.** Caught by History/Bridge, not
     by this lane. Now grouped by the **lane roster in `LANES.md`**: enumerate the side that cannot
     grow, because a name pattern cannot see a name it did not anticipate.

   **Corrected findings:** name collisions **3** (not 17) · **THREE** History handoffs, not two ·
   **THREE** cookbook copies, not two — the tool found `docs/WW Linked/WW-Restoration-Cookbook-SUPERSEDED.md`,
   which this hand audit missed entirely · lanes with >1 handoff: **HISTORY 3, HOUSING 4, ENGINE 2**.
3. **Deprecate by FILENAME, not by banner.** `WW-Restoration-Cookbook-SUPERSEDED.md` →
   `-SUPERSEDED.md`, or move superseded docs to `docs/superseded/`. **Proposal, not done —
   renames break inbound links and are the user's call.**
4. **Resolve the two History handoffs into one, in one location.** Highest consequence, needs the
   History lane's judgement about which content survives; not a Librarian merge.
5. **One handoff convention: `HANDOFF-<LANE>.md` in `docs/`.** Lane handoffs are not live state and
   should not live in `state/`.
6. **Normalise the instrument families** — singular `_map`, one census shape. Mechanical, but it
   touches 15 filenames and every caller, so it queues rather than lands.
7. **The 34 undocumented instruments** need one purpose line each. `tool_index.py` already detects
   them, so this could be a gate rather than a chore.

---

## 6b. THE AXIS THE DOC TREE DOES NOT HAVE: ERA

**User, 2026-08-16:** *"a lot of the cookbooks are from mounted/bake/leg eras. Many may not be
applicable to the plugin/native loading era."*

**That is not a cookbook problem. It is a missing axis, and it is the most dangerous gap in this
document.** Every other measured corpus here carries state:
[INVENTORY-SCHEMA](state/ww-staging/INVENTORY-SCHEMA.md) gives a unit six axes; the ownership map
gives a file a category; a gate has a control. **A document carries no era and no status at all** —
nothing in `docs/` says whether it describes how we work *now* or how we worked *then*.

**Why that is worse than an orphan.** An orphan is invisible; the worst it does is waste a
rediscovery. **An era-less doc is FOUND, READ, AND FOLLOWED** — and the eras it may be describing are
the ones the estate has since made hard stops. Mounting is [DN-9](DO-NOT.md). Baking is
[DN-10](DO-NOT.md). **A recipe followed faithfully from the wrong era walks an instance into a
permanently rejected approach, with the doc tree's full authority behind it.**

The estate already knows this failure and named it for units: *a check that cannot run must report
UNKNOWN, never CLEAN* (№31-C). **A doc whose era cannot be determined is UNKNOWN, not current.**

**Marked so far, and only where the evidence was unambiguous:**

- `WW-Restoration-Cookbook-CANONICAL.md` — era boundary inserted between the doctrine and the
  recipes. **The `ADMITTED DOCTRINE` laws are era-INDEPENDENT and live** (№31-C was cited by four
  lanes on 2026-08-15/16); **the recipes below it are era-BOUND and were never marked.** Which
  specific recipes survive is a content judgement routed to History/Housing — this file does not
  answer it.
- The two diverged copies are now `-SUPERSEDED` **in the filename**, so a glob cannot find them
  first (see §6 item 3, done).

**Owed, and NOT a Librarian call:** an `era:` line on docs that describe method — minimally
`mounted | bake | leg | plugin-native | era-independent`. **The Librarian can require the field and
refuse a doc without one; only the owning lane can fill it in**, because knowing which era a method
belongs to is knowing the method.

**ALSO RE-SURFACED, PENDING SINCE 2026-07-27 (three weeks):** the canonical cookbook carries a
Librarian flag asking the user where the live laws should permanently live — *"migrate to a dedicated
doctrine doc / DO-NOT.md, or keep citing them from here?"* **It was never answered, and the era
marking makes it urgent: live laws are currently housed inside a document ruled ARCHIVAL, whose other
half is era-bound.** Recommendation, on the same reasoning that placed the project origin there:
**`DO-NOT.md`** — every instance reads it first, its entries never expire, and its native format is
*ratification + mechanism*. №31-C in particular is already a hard stop in everything but location.

---

## 7. What is genuinely working — stated because an audit that only finds faults is not calibrated

- **Zero dead links** in the index. Every pointer resolves.
- **`docs/gaming systems/`** is the best-organized tree here: subject-indexed, its own README, a
  declared spine (*donor cited → traps → method → what is still imperfect*), companion docs named
  rather than duplicated, and an honest "not yet 100% vanilla" section. **It is the model the rest of
  `docs/` should be measured against.**
- **`DO-NOT.md`** is the strongest doc in the repo: every entry carries the user's verbatim
  ratification, the mechanism, case receipts, and a non-expiry rule. It is also the only document
  `CLAUDE.md` names first — which is why the project origin now lives there (§ORIGIN).
- **The bus tooling** (`file_row`, `calls_rotate`, `routing_check`, `call_receipt`, `staging`) is a
  complete, self-verifying loop. Nothing in the doc tree has an equivalent.

---

*Librarian owns this file's structure and the conventions it proposes. It rules no content and
retires no document — renames, merges and deletions are the owning lane's or the user's.*
