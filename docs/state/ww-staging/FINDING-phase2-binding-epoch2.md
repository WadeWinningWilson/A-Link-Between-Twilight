# FINDING — Phase 2 re-measure: the binding plan against epoch 2

era: mounted
<!-- era rationale: measurement against a named host commit | Foundry, 2026-08-18 -->

**Foundry, 2026-08-18.** Ordered by Integrator (CALLS row 386, "Phase 2 — the
re-measure — is yours"). Read-only except for one instrument change, recorded
below. Baseline **pinned**: `c880d46fb5` (dusklight-main, `main`, epoch 2,
`GAME_SERVICE_MAJOR 2u`).

## The headline answer

Integrator's question was: *how many of the 87 ABSENT-hookable are now EXISTS or
covered by a published service (Save/Stage/Item/Window)?*

**The four new services publish 40 more functions — and ZERO additional WW seams
bind to them.** The (a) SERVICE bucket is **5 symbols / 478 sites, UNCHANGED**.
The services are real and they are not on our call paths.

| bucket | epoch 1 | epoch 2 (`c880d46fb5`) | delta |
|---|---|---|---|
| export surface identifiers | 18,532 | 16,359 | **-2,173** |
| SDK services published | 102 | **142** | **+40** |
| **(a) SERVICE** | 5 sym / 478 sites | **5 / 478** | **0** |
| (b) LINK | 912 / 5,680 | **721** / 5,174 | **-191 syms** |
| (c) NEEDS `resolve()` | 21 / 263 | **22** / 264 | **+1** |
| (e) inline/template | 1,017 / 7,484 | 1,198 / 7,935 | +181 |
| (e) macro | 52 / 691 | 61 / 745 | +9 |

**The -191 LINK symbols are exactly accounted for: 181 + 9 + 1 = 191.** Nothing
vanished; the smaller export surface pushed them into other buckets.

## Carry set: unchanged, and PROVEN sensitive

| baseline | changed vs origin | CARRY | .inc | blind spots |
|---|---|---|---|---|
| `be82e606b2ef` (pre-merge) | 1,577 | 149 | 5 | 0 |
| `c880d46fb5` (epoch 2) | **1,368** | **149** | **5** | **0** |

The carry set did not move. **That is a result and not an artifact**: the same
tool reports 1,577 vs 1,368 on the two baselines, so it demonstrably responds to
the baseline. Negative control run first (`patch_feed.py --control`) — specimen
flagged, so the detector produces noise. Same 1 stale map row
(`include/d/actor/d_a_e_ww.h`, `not-ww`) and same 6 CARRY `.cpp` with no seam row
on both sides.

## Seam tracker

**123 rows, 0 errors** (not 122 — the population grew by one; `INDEX.md`
regenerated). The doorway classification is NOT re-derived here: Housing/Engine
already re-ran it against `c880d46fb5` on 2026-08-18 with **0 flips**, and the
1 ABSENT-unhookable **survives**. That third is closed; re-running it would have
been duplicated work.

## THE INSTRUMENT DEFECT THIS PASS EXISTS TO REPORT

`binding_plan.py` had **`--def` parameterised and `SDK` hardcoded** to
`C.REPO / "sdk/include/mods"`. Run from this tree it always counted THIS tree's
service headers — epoch 1, eleven headers, no Save/Stage/Item/Window.

**It reported `SDK services : 102 published functions` and named none of them.**
That is a confident, well-formed answer to the *previous epoch's* question, and
nothing about the output marked it as such. An SDK path that resolves and parses
cleanly is indistinguishable from the CORRECT SDK path.

Fixed: `--sdk PATH` added, mirroring `--def`. Negative control run (a
non-existent path is refused, exit 2). **Half a parameterised input pair is the
trap** — one input moving while its partner stays pinned lets the two describe
different worlds without erroring.

## LIMITS — read before citing any number above

1. **The (e) INLINE growth is judged against OUR headers, not the new host's.**
   Classification is a waterfall: a symbol absent from the (smaller) epoch-2
   export surface falls through to "inline" if it is inline in *this* tree's
   `include/`. So +181 is a CONSEQUENCE of the smaller export surface, **not
   evidence those symbols are inline in the epoch-2 host.** Do not read it as
   "fine, they compile in."
2. Identifier-level matching: **overloads collapse to one name.** This answers
   *can a plugin reach this NAME*, not *which overload*.
3. The (c) bucket is **not proven unreachable** — the embedded symbol manifest
   `resolve()` reads lives in the linked image and cannot be consulted from
   source. Splitting c from d needs a built binary.
4. `dusklight-main` has a **2026-08-17 build**; if it is rebuilt, re-run.

## Still open, and NOT done by this pass

The **123-seam census** itself — per-row re-derivation against epoch 2 — is not
in this document. `row_store validate` (123/0) proves the rows are well-formed;
it does not re-derive their CONTENT against the new host. History/Bridge argued
the 99 rows outside the four new-service domains rather than re-deriving them,
and that gap is still open.

Reproduce:

```
python tools/foundry/patch_feed.py --control
python tools/foundry/patch_feed.py --base c880d46fb5 --emit <path>
python tools/foundry/row_store.py validate && python tools/foundry/row_store.py index
python tools/foundry/binding_plan.py   --sdk  ../dusklight-main/sdk/include/mods   --def  ../dusklight-main/build/windows-msvc-relwithdebinfo/dusklight_exports.def
```
