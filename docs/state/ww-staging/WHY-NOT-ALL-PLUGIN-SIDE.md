# Why the WW layer cannot live entirely plugin-side

> ## ⏱️ MEASUREMENT-DATE STAMP — added 2026-08-17 on the user's supersession order
> **The measurements in this document were taken against a MAY-2026 VANILLA
> DUSKLIGHT.** The tree was **2,861 commits behind origin** (diverged
> 2026-05-08) when they were made. They therefore **PREDATE** origin's
> `SaveService`, `StageService`, `ItemService`, `WindowService`,
> `mods::hook::uninstall`, the embedded symbol database, the **Game ABI/headers
> refactoring** (GameService major 2, 2026-08-12), and open PR **#2270 "Mod SDK:
> Actor service"**.
>
> **NOT DELETED AND NOT WRONG.** These findings were correct for the binary they
> were measured against, and they are the BEFORE side of the comparison the
> post-merge Phase-0 rule depends on (*anything that moves after the merge is a
> merge artifact until proven otherwise*). **Re-measure before citing any number
> here as current.**
>
> ## ⏭️ THE EVENT THIS STAMP ANTICIPATED HAS NOW HAPPENED — amended 2026-08-17
> **THE MERGE LANDED.** `dusklight-main` fast-forwarded **39 commits to
> `c880d46fb5`** (2026-08-15), **ABI epoch 2**, and the services listed above
> are **ON DISK with real implementations** — 18 files in `src/dusk/mods/svc/`,
> verified at source, not relayed.
>
> **SO THE WORDS ABOVE HAVE CHANGED FORCE WITHOUT CHANGING MEANING.**
> *"Predates the services"* no longer reads as *"those services are
> hypothetical"*; it now reads as **"measured against a host that no longer
> exists."** Same sentence, opposite practical weight — a reader before the
> merge would treat these numbers as current-minus-a-roadmap, and a reader
> after it must treat them as historical.
>
> **A supersession stamp is itself dated evidence.** It was correct when
> written this afternoon and needed this amendment within hours, which is the
> argument for putting the COMMIT and the DATE in a stamp rather than only the
> list of what it predates: a list ages silently, a commit hash does not.
> (Integrator caught it; Librarian amended.)



*Compiled by Foundry 2026-08-17 from the banked record. Every claim below cites
where it was determined; nothing here is inference.*

**Primary sources:** `SPEC-patcher-requirements.md` (Integrator, status: banked,
proven on an unmodified receiver) · the seam tracker (`tracker/rows/`, 122 rows)
· `mods-src/ww_donor_disc/registry.cpp` header (§968 user ruling).

---

## The short version

A plugin can do far more than expected — a donor actor spawns, registers
collision, and draws on a completely unmodified dusklight with **zero receiver
edits and zero name-table rows**. That is proven, not projected.

What a plugin cannot do splits into three different kinds of problem, and they
are worth keeping apart because only one of them is permanent:

1. **One structural blocker** — a seam with nothing to hook.
2. **A class of practical failures** — plugin-side actor ports are written
   without a compiler checking them, and they fail in a specific, repeating way.
3. **A delivery constraint** — we can never modify the user's binary.

---

## 1. The structural blocker: one seam, and it has nothing to intercept

Of **122 seams** in the tracker:

| doorway | rows | meaning |
|---|---|---|
| `EXISTS` | 34 | symbol already in the user's binary — the plugin calls it |
| `ABSENT-hookable` | 87 | plugin installs a by-name hook |
| **`ABSENT-unhookable`** | **1** | **nothing exists to intercept** |

That one row is `sfb76e669` — `dExtWwSave_registerWwStage` / `dBootStage_add`.
Its own negative control states the consequence:

> Absent from vanilla entirely, so there is nothing to hook: without a patch the
> receiver is never told a stage is WW-hosted. Removal is observable as the
> Outset black screen — scene tears down, zero resources requested, no
> donor-disc serve after the warp.

**You cannot hook a function that does not exist.** This is the entire
irreducible patching requirement as currently measured, and it is also the known
cause of the black-screen failure mode.

*Not yet explored:* `ABSENT-unhookable` is a property of the **symbol**, not of
the **effect**. The effect is "the receiver knows this stage is WW-hosted." If
something that *consumes* that knowledge exists in vanilla, hooking the consumer
instead could remove even this blocker. **No one has measured this.**

---

## 2. The practical failures: plugin-side ports have no compiler behind them

The banked recommendation (`SPEC-patcher-requirements.md`):

> **Stop the actor-by-actor plugin ports.** They rebuild donor actors out of
> `void*`, hand-measured PDB offsets, and hand-authored constants, with no
> compiler checking any of it — and the R4 table is what that produces. The fork
> tree already carries several of these natively and in typed form. **Actor work
> belongs there.**

Each requirement below was earned from a real failure during the pilot:

**R2 — a resource fetched BY INDEX arrives RAW.** The plugin's conversion arms
are keyed on resource *name*; `dComIfG_getObjectRes(arc, index)` bypasses **all**
of them. A raw `J3D2 bdl4` image reached `mDoExt_J3DModel__create`, was cast to
`J3DModelData*`, and faulted at address `0x0`. This is a property of the routing
design, so it holds for **every** resource type — including ones nobody has
written a conversion arm for yet.

**R2 amended — conversion is a SEQUENCE, not a step.** The receiver converts a
model in four steps and every arm does all four. Doing only the first still
dies.

**R3 — "I parsed it" does not mean "it is parsed."** `getObjRes` hands back the
same raw image on every call, so the second consumer re-parses the first
consumer's buffer — whose vertex arrays the first pass already endian-fixed *in
place*. Result: `OSPanic J3DModelLoader.cpp:312 "Unable to find vertex attribute
format!"`. Requires a memo keyed on `(arc, index, raw)` **and** eviction at both
release seams. Note the donor already had this natively — `ConvDzb`'s
`0x80000000` latch is the same guard — which is why the collision path was safe
all along and only the model path broke.

**R4 — named donor constants are DATA; inferring them fails.** Three wrong in a
single session: the `alwd.dzb` resource index (wrote 5, truth **7**), "the
parsed head is a vtable so no re-parse" (**false**), `TEV_TYPE_BG0` (wrote 0,
truth **1** — 0 is `TEV_TYPE_ACTOR`). A typed in-tree port cannot make these
mistakes silently; a `void*` plugin port can and did.

**R5 — port the translation unit, not the method table.**
`daLwood_c::CreateInit()` is not in the method table; it is called from
`_create`. A method-table reading produced a port that was "verbatim" for
`_create`/`_execute`/`_delete`/`_draw` while silently dropping `setBaseScale`,
`setBaseTRMtx`, `mModel->calc()`, `fopAcM_SetMtx`, the cull box and the cull-far.

**R6 — bind counts measure AVAILABILITY, not COVERAGE.** `bound=10 missing=0`
printed on every boot while `setBaseTRMtx`, `fopAcM_setCullSizeFar`, `cM_rndF`
and `dBgW::Move` were bound and **never called**. Read as completeness for four
boots.

---

## 3. The delivery constraint: the user's binary is immutable

From `registry.cpp` (§968, user-ruled): the shipped profile list has **829
entries with 13 free**. Widening the array, reclaiming the 13, or shadowing
unused receiver rows **all require recompiling the user's executable** — which is
fork delivery, the exact thing "download the plugin, point it at your ISO"
replaces.

Above-enum indices plus exactly two hooks are therefore **not the cheapest
option; they are the only one that ships.** This constrains *how* plugin-side
work is done, and it is why the registry exists at all.

---

## 4. What is genuinely unknown

Stated as unknowns rather than folded into the case above:

- **98 of 122 seams are `linked: NOT`.** Only 24 are `REGISTERED`, and only a
  handful (lwood, akabe, TagSo) were ever exercised at runtime. **The doorway
  classification is a plan, not a demonstration.** Rows classified
  `ABSENT-hookable` may prove unhookable once wired, and each one that does adds
  to the irreducible patch residue. **The current "1 blocker" figure is a floor,
  not a guarantee.**
- **The lwood draw crash is UNROOTED.** One full frame completes, then the
  process dies before frame 2 with no CPU exception, no fault address, no dump.
  Backend is Dawn/WebGPU, where a device-lost kills the process at submission
  after every draw call has returned — "no fault marker" is *evidence*, not
  missing evidence. The staged four-mode bisect was removed with the lwood
  revert; whoever resumes rebuilds it from the spec.
- **R1's own caveat:** zero-`OBJNAME`-rows is proven *for the actors exercised*.
  A consumer that walks the name table directly would defeat it. None has been
  observed; none has been ruled out.
- **Whether the single blocker can be re-sited** (see §1) — unmeasured.

---

## What this does NOT say

R1 stands and is not weakened by any of the above: a donor actor spawns,
registers collision (`cbgw_set ret=0`, `dbgs_regist ret=0`) and draws a full
frame on an **unmodified receiver** with zero receiver edits and zero name rows.
The "split option" — adding donor names to the receiver's own name table — is
**dead**, and that finding does not depend on any actor remaining plugin-side.

The conclusion is not "plugins don't work." It is: **plugins are the delivery
mechanism; the tree is where the code belongs; and exactly one seam currently
requires a patch to bridge them.**
