# SPEC — the BOOT JOIN: feeding `LINKED` and `EXERCISED` from the game's own log

> Integrator scope, 2026-08-16, on the user's order to spec it for the lanes
> rather than absorb it. **Scoping only — nothing here is built.** The
> mod-side half is already shipping (see §3); what is missing is the join, and
> it rides the queue like any other change.

## 1. The gap, stated as a count

The tracker carries **five axes** (`_schema.json`), and exactly **one** of
them has a live feeder:

| axis | fed by | status |
|---|---|---|
| `PORTABLE` | `decomp_watch` ← the donor's own `configure.py` | **live, 2026-08-16** |
| `LINKED` | the plugin's hook manifest | **evidence exists, nothing joins it** |
| `EXERCISED` | the plugin's call counters | **evidence exists, nothing joins it** |
| `BYTE-TRUE` | the oracle stack | out of scope here |
| `VERIFIED` | MATCH / DRIFT | out of scope here |

`PORTABLE` got its feeder because a hand-typed axis drifts — the schema's own
`never_stored` rule ("a copied value is a second roster that drifts"). **That
argument applies unchanged to `LINKED` and `EXERCISED`, and they are further
from their evidence than `PORTABLE` ever was:** their evidence is regenerated
on every boot and thrown away.

The first `decomp_watch audit` found a real disagreement on its first run. A
boot join will do the same, and for the same reason.

## 2. Why this is worth a tool and not a habit

**The failure it catches has already happened twice, in one week, to the
person who built the instrument:**

- **`setStageRes`** was bound BARE, is `MULTI_NAME` on the user's image, and
  the hook **was never installed**. Its probe then logged
  `set_stage_res_calls: 0` — and that zero was read as a MEASUREMENT. One
  wasted user run.
- **`on_getRes`** was **defined and never registered**. Its probe produced
  zero lines across two boots. D1 reported the symbol `INSTALLED`, which was
  read as proof the probe was alive.

Both are `REGISTERED-NOT-EXERCISED` — **a value the schema already has and
nothing has ever emitted.** The vocabulary was ready before the feeder.

## 3. The evidence already exists — this is a join, not an instrument

A boot of the shipped plugin emits, today, without any change:

- `{"ev":"hook_manifest","name":"<mangled>","state":"INSTALLED|NOT-INSTALLED"}`
  — per declared hook, from the `modmeta$d` walk.
- `{"ev":"hook_manifest_summary","records":N,"hooks_installed":N,...}`
- `{"ev":"getres_attach","result":"MOD_OK"}` — **callback attachment, which is
  a different fact from symbol resolution.**
- per-seam counters at shutdown: `pf_hits`, `dyl_hits`, `name_hits`,
  `search_calls_total`, `set_stage_res_calls`, and the per-feature events
  (`bmdl_consume_parse`, `bg_model_lookup`, `bmdl_pass`, `msggroup_skipped`).

**Nothing consumes any of it.** The join reads a log and reports.

> ⚠ **CORRECTED 2026-08-16, and the original claim was false in the way that
> mattered.** This section first read *"the shipped plugin already emits all
> of it with no change"*, citing `getres_attach` as the attachment evidence.
> **`getres_attach` covered ONE hook** — a single-seam receipt I had added
> myself, generalised into a claim about the whole manifest.
>
> All 13 `hook_add_*` results *were* logged, which is why I believed it — but
> across **eight ad-hoc event shapes** among 31+ `ev` names. A join would have
> had to know every shape and which field carried which hook's result.
> **"The information exists somewhere in the log" is not "the join can consume
> it."**
>
> Housing added a uniform attachment axis on 2026-08-16 and **moved the
> manifest walk to the end of `wwRegistry_initialize`** — resolution is filled
> by the loader before the function runs, but attachment is recorded by the
> `hook_add_*` calls themselves, and **8 of the 13 happen below the old call
> site**. Emitting there would have reported 8 of 13 as NOT-ATTACHED: a false
> *alarm*, which is the same class of lie as a false clean and no better,
> because a gate that cries wolf gets discounted and then nobody re-checks it.
>
> Recorded rather than silently amended: a spec that quietly acquires the
> thing it assumed is how the next reader inherits the same false premise.

## 4. The mapping, and the one distinction that must not be collapsed

    LINKED     REGISTERED   symbol resolved AND a callback is attached
               PENDING      symbol resolved, NO callback attached
               NOT          symbol did not resolve (NOT-INSTALLED)
               RELINQUISHED deliberately handed to the host (profile-row shape)

    EXERCISED  EXERCISED                  the seam's counter is non-zero
               REGISTERED-NOT-EXERCISED   attached, counter zero
               NOT                        not attached, so nothing to exercise

> ⚠ **`INSTALLED` IS NOT `REGISTERED`.** D1's `INSTALLED` means *the
> declaration's symbol resolved*. A hook declared with `DEFINE_HOOK_SYMBOL`
> and never passed to `hook_add_*` reports `INSTALLED` **forever** and can
> never fire. That is exactly what `on_getRes` did, and the person who wrote
> D1 believed it. **The join MUST read attachment separately from resolution,
> or it reproduces the bug it exists to catch.**

## 5. Required behaviour

1. **Input is a boot log path** — no game control, no live hooking.
2. **Report disagreement; never rewrite a row.** Same refusal as
   `decomp_watch`: a copied verdict is a second roster.
3. **A missing log is UNKNOWN, not clean.** No log ⇒ exit non-zero and say
   so. This is the single most important line in the spec, because every
   failure in this class has been a false clean.
4. **A counter of zero is only meaningful once attachment is known.** Report
   `REGISTERED-NOT-EXERCISED` (attached, idle) and `PENDING` (never attached)
   as *different* states. Collapsing them is the defect.
5. **EVERY EMITTED VALUE CARRIES `src`. THIS IS THE SCHEMA'S RULE, NOT A
   PREFERENCE.** `INVENTORY-SCHEMA.md` §1: every `axes.*` object is
   `{state, src}`, `src` names the tool/measurement that produced the state
   "so the value is falsifiable from its own output", and **a value with no
   `src` is `UNKNOWN`**. For this join `src` must identify the *run*, not just
   the tool — at minimum `boot-join:<log-name>@<build-id>`, or
   `boot-join:<log-name>@UNPINNED` when the build cannot be identified.

   > **Added 2026-08-16 after Librarian caught its absence.** The first draft
   > of this spec did not contain the string `src` once, and instead invented
   > a weaker parallel requirement ("name the build"). The cause is exactly
   > the failure this estate keeps naming: **I built the spec against
   > `_schema.json` — the transcription — instead of §4/§1, the authority.
   > `_schema.json`'s `required_fields` omits `src` too**, so the loss ran
   > authority → transcription → spec without anyone noticing.
   >
   > **A log-fed value is where this bites hardest: it LOOKS authoritative
   > because a machine produced it, and without `src` a reader cannot tell
   > which boot, which build, or whether the feeder ran at all.** That is the
   > silent-zero class with better packaging.

## 6. Controls it must carry

A gate that cannot go red certifies nothing.

- a log with `NOT-INSTALLED` present ⇒ must report `LINKED: NOT`;
- a log with a hook attached and its counter at 0 ⇒ must report
  `REGISTERED-NOT-EXERCISED`, **not** `NOT` and **not** `EXERCISED`;
- **absent log ⇒ must refuse**, not pass;
- a row whose axis disagrees with the log ⇒ must be listed.

Register it in `control.py` **in the same change that builds it** — A4 was
sound, selftested, and outside the meta-gate, and nothing would have noticed
if it stopped being able to fail.

## 7. Deliberately out of scope

- Changing any plugin code. The emission already exists.
- Rewriting rows. Ever.
- `BYTE-TRUE` and `VERIFIED` — different evidence (oracle stack), different
  spec.
- Inventing a notification channel. If it needs to announce, it files to
  `CALLS.md` through `file_row.py`, **one line**, like `decomp_sync` does —
  and see that tool's comment for why a machine filer must be the most
  conservative writer on the board, not the one introducing a shape no human
  produced.

## 8. Honest limits

- A boot log proves what happened **on that run, on that image, on that
  route**. A seam not exercised because the player never walked into the room
  is not a defect, and the join cannot tell the difference. **Report
  `REGISTERED-NOT-EXERCISED` as a fact, never as a verdict.**
- The counters were authored per-feature for debugging, not as an axis feed.
  Some seams have no counter at all; those are `UNKNOWN`, and **saying so is
  the point** — a join that silently covers only the instrumented seams would
  be the same partial-coverage lie this spec exists to end.
