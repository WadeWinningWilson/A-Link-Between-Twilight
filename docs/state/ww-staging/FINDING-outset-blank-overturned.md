# FINDING — the Outset blank: every overturned suspect, with what killed it

era: era-independent
<!-- era rationale: live investigation ledger, method + evidence | Housing/Engine, 2026-08-18 -->

**Purpose.** Fifteen suspects have been raised and overturned on this one symptom.
Without a ledger the dead ones keep returning, because each was *plausible* and
several were published as findings before they died. **This file is the list of
things that are no longer allowed to be the answer, and the evidence that ended
each.** Nothing here is a hypothesis; every row is a retirement.

**THE SYMPTOM, stated once:** on **vanilla** `dusklight-main` (`c880d46fb5`) with the
WW plugin loaded, warping to `sea` room 44 (Outset) draws **nothing**, overlay reads
**Room: -1**, no crash. On the **fork** build (`0abbfdbb91`) the same warp **draws
correctly**. Both load the donor disc; both register collision.

---

## A. OVERTURNED — resource / data layer

**1. Yaz0 refusal is starving the stage.**
KILLED: 20 `yaz0_decoded`, **zero refusals** of any kind on the live boot; every
decode `mode:"transient"` on a `room.dzr`. And structurally — *a refusal returns
NULL and cannot manufacture a `J3DModelData`*, so it could never explain non-null
constructed objects.

**2. `getRes` returns vtable-bearing objects for `.bmd`/`.btk` = silent-non-draw stubs.**
KILLED: **correct by construction.** `d_resorce.cpp:455` sends `BMDR/BMDV/BMDE/BMWR/BMWE`
to `loaderBasicBmd` → `J3DModelLoaderDataBase::load`; `:556` sends `BTK /BTP /…` to
`J3DAnmLoaderDataBase::load`. The raw buffer is **replaced by a parsed object before
storage**, so a vtable at offset 0 *is* the storage contract. `.bdl`/`dzr`/`dzs`/`dzb`/
`kcl`/`plc` stay raw via the pack-archive branch at `:419` — **two contracts in one
table, by design.** Corroborated by `dExtNpcMount_shouldSkipBtp` (`:562`) existing to
leave `BTP ` raw *on purpose*.

**3. A consumer casts a parsed resource to raw bytes.**
KILLED by census (Foundry, `res_consumer_census.py`): 2,460 sites, **96.5% classified,
ZERO raw casts on a parsed node type**. All 61 raw sites fetch `.dat`/`.bti`/texture-bank
indices. The one unresolvable site, `d_demo.cpp:566`, resolved here: it fetches a
**`.stb`**, which has **no node type in the 22-entry parse dispatch**, so `(u8*)` is correct.

**4. `CLASS-ON-RAW`: three sites cast a `getRes` result to `J3DMaterialTable*` with no parse.**
NOT the blank, and **latent not live**: `BMT ` is absent from the dispatch and
`J3DMaterialTable` *is* polymorphic (`virtual ~J3DMaterialTable()`), so the cast is
genuinely wrong — **but both consuming shims discard the pointer**
(`dExtKb_setMaterialTable(…) {}` empty; `dExtNpcBm1_entryDL_mat` takes the table as a
commented-out parameter). **Armed, not firing.** Becomes a live crash the moment the
`§246` recolor fills either shim; correct call is `loadMaterialTable`, already used
properly at `d_ext_npc_mount.cpp:2176/2180`.

---

## B. OVERTURNED — collision layer

**5. "Collision never registers" / `room_set_bgw = 0` is the defect.**
KILLED TWICE. (a) **Unmeasurable by construction**: `dStage_roomControl_c::setBgW` is
**inline in the header** (`d_stage.h:1222`, a three-line `static`), so the hook on its
mangled symbol reads zero regardless — and `registry.cpp:921` already said so
*twelve lines above the `DEFINE_HOOK_SYMBOL`*. (b) **Falsified by a positive control**:
`room_set_bgw = 0` **on the fork run that DRAWS PERFECTLY**.

**6. `№257 skip` is a free discriminator.**
KILLED TWICE, both mine. (a) The branch lives in the fork's `src/d/` and **is not in
the vanilla binary being tested** — the grep's result was fixed at zero before it ran.
(b) It reads **0 on the working fork run too**, so it is zero in the healthy state.

**7. `cBgW::Set` is never reached, so the convergence fix is "buildable but not verifiable".**
KILLED: `cbgw_set` **n=3, ret=0** — called and **succeeding**. Also `dbgs_regist` n=3
ret=0 (admitted to the searchable set).

**8. The unconverted-`dzb` defect (index-fetch bypassing the name-keyed conversion) is the cause.**
KILLED as *this* symptom's cause: the predicted signature was *"present with
`trues=0` — the walk rejects everything"*. Measured: **`bgw_ground` 2,700 calls /
274 TRUES**. The walk **hits**; the geometry is not garbage. *(The port may still be
correct on its own terms — it is no longer supported as the blank's explanation.)*

---

## C. OVERTURNED — draw layer

**9. `mDoLib_clipper::changeFar` / far-plane divergence.**
KILLED BEFORE PROPOSING: `changeFar(1000000.0f)` is present in **both** trees and
**ungated**. Not a divergence.

**10. `daBg_c::draw` is inlined-dead and unhookable.**
KILLED: the hook resolves `INSTALLED/ATTACHED` and produced **1,800+ real calls**. It
is **not** among the five symbols the host flags as inlined (`changeFar`,
`setBgW`, `dComIfG_syncAllObjectRes`, `dComIfGp_setMsgDtArchive`, `startFadeOut`).

**11. "`ww==0` on every `bg_draw` receipt = the WW predicate is hiding the draw."**
KILLED BY LINE NUMBERS, and this was my own published contract. All twelve receipts
sat at log lines **1307–2448**; the first `start="sea"` was at **2489**. **Every
receipt predated the stage it was supposed to describe.** `calls:1200` reads as
damning only if you never ask *when* frame 1200 was.

**12. The ungated per-shape clip (§682) is what blanks vanilla.**
KILLED: vanilla clips unconditionally (`d_a_bg.cpp:328`) where the fork adds
`!wwHost &&` (`:430`) — real divergence, wrong consequence. **`daBg_c::draw` is never
called after the WW stage loads** (last call line 2404, predicate flips 2487, log runs
to 3753). **You cannot cull shapes in a draw that does not happen.** This also retires
the plugin-side "re-show the shapes" plan built on it.

**13. "Vanilla builds room 0, not room 44."**
KILLED — mine, same run. I sampled the **first eight** post-flip lookups, all
`R00_00`, and generalised. Full census: **`R44_00` has 20 `bg_model_lookup` calls**
(`R00_00` 35, `@bg0010` 33, `R01_00` 19). Room 44 is **not** skipped; several rooms
are resident at once, which is normal for `sea`.

---

## D. OVERTURNED — crash / harness layer

**14. The fork crash is an epoch mismatch (plugin updated, fork not).**
KILLED: the fork's own matched plugin **loaded cleanly** — `activating mod`,
`mod_initialize`, ISO attached, hooks `INSTALLED/ATTACHED`, **no ABI refusal anywhere**.
The two crashes were unrelated: a **`JKRExpHeap` allocation failure** during a
demo/Aryll BDL load, and a **spawn-point assert**.

**15. Spawn point 23 "worked before".**
REFRAMED: `sea`'s PLYR table holds **0-11, 99-103, 128, 151, 201-206 — no 23**.
`JUT_ASSERT(1636, i != num)` stops it; without the assert the next line
(`appen->base = player_data->base`) reads **one past the end** of the entry array.
It "worked" by landing on plausible memory. **Point 11 is valid and produced the
working fork run.**

**16. Stall shape: "three ids pinned", "pumps not climbing / many entries pumped once".**
KILLED as emission artifacts. `m_is_creating` compared against a **single shared
`lastRet`** (not per-id) and capped at 10 lines; `ctrq_do` emitted on `flip ||
pumps%300` with `lastRt` starting at sentinel `-99`, so **the first sighting of every
request logs at `pumps:1`** — one pump and 250 pumps are the identical line. Both
probes rebuilt; the stall's *shape* remains **unmeasured**.

---

## E. WHAT SURVIVES — the current evidence

- **The transition is requested and does not land**: `scn_change_req` fired 141,
  **137 returning 0** (unconditional probe — trustworthy census).
- **Resources load on vanilla**: `glb_res` 275 post-flip, `model_create` 12,
  `bg_model_lookup` on `R44_00` ×20.
- **Collision is healthy on vanilla**: `cbgw_set`, `dbgs_regist`, ground queries hit.
- **The predicate does flip**: 301 events carry `ww:1`, first at line 2487.
- **`daBg_c::draw` fires 1,800 times BEFORE the flip and ZERO times after.**
  ← *the live anomaly*
- **Fork vs vanilla, same warp**: fork logs `[daBg] §898-P1 room44 model[0]/[1]/[3]
  fetched` and `§757 room44 BgW REGISTERED … GndChk -> HIT`, and **draws**. Vanilla
  logs neither and does not.

## F. THE METHOD NOTE THIS FILE EXISTS TO CARRY

**Four readings of one hook were overturned in a row, and three fell to *when* or
*how much* rather than *what*:** inlined-dead → predicate-hiding → never-called →
room-mismatch. Every correction came from widening the window or the sample, never
from re-reading the code.

> **A throttle is a sampling decision, and a sampling decision made before the event
> under study is a decision to miss it.**

The recurring shape across all sixteen: **an instrument's limit reported as a property
of the subject.** A gated counter, a 300-call modulus, a shared `lastRet`, a first-eight
sample, a grep against a binary that lacks the code. In every case the silence or the
zero *was* the artifact. **Nothing here was caught by review; everything was caught by
measuring again, wider.**
