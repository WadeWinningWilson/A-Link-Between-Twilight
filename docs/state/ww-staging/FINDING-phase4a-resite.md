# PHASE 4a — THE RE-SITING IS VIABLE. First measurement, 2026-08-17.

> Foundry's parting item, named twice and never run: *"`ABSENT-unhookable` IS A
> PROPERTY OF THE SYMBOL, NOT THE EFFECT. Nothing can hook
> `dExtWwSave_registerWwStage` because it does not exist — but the effect is
> 'the receiver knows this stage is WW-hosted,' and vanilla HAS a stage-load
> path. Find what CONSUMES that knowledge and hook the consumer. If it works
> the patch count goes to ZERO. Nobody has looked, including me."*
>
> Looked. **It works, on the evidence so far.**

## 1. The predicate was never the blocker

Every one of the twelve `dExtWwSave_isWwHostStage` call sites has the same
shape:

```cpp
const char* stage = dComIfGp_getStartStageName();
if (dExtWwSave_isWwHostStage(stage)) { /* WW */ } else { /* native */ }
```

**Its only input is `dComIfGp_getStartStageName()` — a vanilla function.** The
predicate is a pure function of the start-stage name, and **the plugin already
knows which names are WW-hosted**: it owns the donor disc, the roster verdict
and the registry. It never needed the host to *produce* this answer.

So `dExtWwSave_isWwHostStage` is not a facility to reproduce. **The blocker is
the CONSUMER SITES**, which are fork edits living inside vanilla TUs.

## 2. Census of the twelve sites — SEVEN ARE DIAGNOSTICS

| site | what it does | kind |
|---|---|---|
| `d_a_bg.cpp:158` | §898-P1 model-fetch log | **diagnostic** |
| `d_a_bg.cpp:350` | draw census log, per-room bitmask | **diagnostic** |
| `d_a_bg.cpp:719` | ground-check probe at a fixed point | **diagnostic** |
| `d_a_alink.cpp:9399` | ladder-phase tracking, `s_ldN++ % 30` | **diagnostic** |
| `d_a_alink.cpp:14845` | bg lifetime probe, "12-shot budget" | **diagnostic** |
| `d_a_alink.cpp:15104` | §798 exit-gate DENIED warn, 8-shot budget | **diagnostic** |
| `d_a_mirror.cpp:23` | an `#include` line | **not a site** |
| **`d_a_bg.cpp:252`** | **skip room collision — room-lane mount owns BgW** | **BEHAVIOUR** |
| **`d_a_bg.cpp:425`** | **disable frustum clipping for WW rooms** | **BEHAVIOUR** |
| **`d_bg_s_acch.cpp:170`** | **skip the thin-ceiling roof check** | **BEHAVIOUR** |
| **`d_a_swhit0.cpp:407`** | **WW lighting — `dKyWw_settingTevStruct(TEV_TYPE_BG0)`** | **BEHAVIOUR** |
| **`d_a_mirror.cpp:643`** | **WW lighting — `dKyWw_setLightTevColorType`** | **BEHAVIOUR** |

**More than half of the "blocker" is instrumentation that does not need porting
at all.** It needs deleting, or rebuilding plugin-side where it belongs.

## 3. The five behavioural sites resolve to five ordinary functions

| site | enclosing function |
|---|---|
| `d_a_bg.cpp:252` | `daBg_c::createHeap()` |
| `d_a_bg.cpp:425` | `daBg_c::draw()` |
| `d_bg_s_acch.cpp:170` | `dBgS_Acch::GroundCheck(dBgS&)` |
| `d_a_swhit0.cpp:407` | `daSwhit0_c::draw()` |
| `d_a_mirror.cpp:643` | `daMirror_c::draw()` |

## 4. ALL FIVE RESOLVE IN UNMODIFIED VANILLA

`symbol_manifest.py` against
`dusklight-main/build/windows-msvc-relwithdebinfo/dusklight.exe`
(`c880d46fb5`, epoch 2), **285,182 manifest entries**:

```
RESOLVED  daBg_c::createHeap      rva=0x4e2370  [CODE|MULTI_NAME]
RESOLVED  daBg_c::draw            rva=0x4e2bf0  [CODE|MULTI_NAME]
RESOLVED  dBgS_Acch::GroundCheck  rva=0x236c90  [CODE|MULTI_NAME]
RESOLVED  daSwhit0_c::draw        rva=0x5158f0  [CODE|MULTI_NAME]
RESOLVED  daMirror_c::draw        rva=0x9db5a0  [CODE|MULTI_NAME]
```

**Five for five, with addresses.**

### ⚠️ I nearly filed the opposite, from the wrong table

The first check grepped **`dusklight_imports.lib`**, the export library, and
returned **0 of 5** — only `GroundCheck` appeared. Filed as-is that would have
read *"four of the five consumers are unreachable; Phase 4a is dead."*

**The export table is 28,961 names. The embedded manifest is 285,182.** They
are different instruments answering different questions, and `HookService`
resolves against the manifest — which is why `manifest.cpp` carries inline-site
flags at all. This is `HANDOFF-INTEGRATOR.md` §7.2's `bound=10 missing=0`
mistake in mirror image: **a narrower table read as the whole world.**

The export grep did produce a known-positive (`GroundCheck`), so the instrument
was working. **A working instrument pointed at the wrong table still yields a
false verdict** — the known-positive proves the tool runs, not that the
question is right.

## 5. NOT YET SAFE — the shape-A check is still owed

All five carry **`MULTI_NAME`**. `symbol_manifest.py`'s own comment records
that `--find` **false-SAFEs shape A** (many raw entries under one identical
name), where the host's `resolve()` returns `MOD_CONFLICT`:

> *"A static gate that disagrees with the runtime binder in the OPTIMISTIC
> direction is worse than no gate."*

`HANDOFF-INTEGRATOR.md` §6 says the same: use `upstream_conformance.py`, **not**
`symbol_manifest.py --find`. **RESOLVED here means "the name is present",
NOT "a hook on it will bind".** That is the next measurement and it is owed
before any code is written.

Two further things are unproven and must not be assumed:

- **Hookability ≠ sufficiency.** `daBg_c::draw()` can be hooked; whether the
  clipping decision inside it is *reachable* from a hook (rather than needing a
  mid-function edit) is a separate question per site.
- **`d_a_bg.cpp:252` returns `1` from `createHeap`** and nulls `mpBgW`/`mpKCol`.
  That is a lifecycle change, not a filter. DN-10 and the standing
  BG/collision-registration entries in `docs/DO-NOT.md` bind here — **read the
  donor's own system before writing anything.**

## 6. What this does to the blocker count

Foundry asked for its own number to be disbelieved: *"'1 blocker' rests on 98
of 122 seams that are `linked: NOT`. Treat 1 as a FLOOR."*

On this evidence the ABSENT-unhookable seam is **not one blocker and not five
symbols**. It is **five behavioural decisions in four vanilla TUs, all five of
whose host functions exist in the shipping binary**, plus seven diagnostics
that should never have been counted.

**Phase 4a is viable. It is not yet proven, and nothing has been built.**

---

## 7. SHAPE-A CHECK DISCHARGED — all five are SAFE, and the check was proven able to fail

`upstream_conformance.py --symbol` (the instrument `HANDOFF-INTEGRATOR.md` §6
names as the correct one), comparing our fork against `dusklight-main`'s
epoch-2 exe:

```
daBg_c::createHeap      fork SAFE · vanilla SAFE — resolves to exactly one
daBg_c::draw            fork SAFE · vanilla SAFE — resolves to exactly one
dBgS_Acch::GroundCheck  fork SAFE · vanilla SAFE — resolves to exactly one
daSwhit0_c::draw        fork SAFE · vanilla SAFE — resolves to exactly one
daMirror_c::draw        fork SAFE · vanilla SAFE — resolves to exactly one
```

**`MULTI_NAME` in the manifest flags did NOT mean ambiguous binding.** Five for
five resolve to exactly one entry on both sides.

### The controls, because five SAFEs from a checker that has never said anything else prove nothing

> *"A check that can only produce silence must be proven able to produce NOISE
> before its silence is read as a result."*

| control | expected | got |
|---|---|---|
| `dExtWwSave_isWwHostStage` | absent from vanilla | fork **SAFE** · vanilla **MISSING — no manifest entry; cannot hook by this name** |
| `RES_U32` | shape A (the tool's own comment cites 685 entries) | **AMBIGUOUS — 685 raw entries** (fork) / **636** (vanilla); *"by-name binding cannot pick between them. Hook by address, or qualify."* |

Both fired. The checker distinguishes SAFE from MISSING from AMBIGUOUS, so the
five SAFE verdicts are results and not silence.

**Control 1 is also a free corroboration:** a *second, independent* instrument
confirms `dExtWwSave_isWwHostStage` is absent from vanilla — matching the
plugin's own runtime `hit:0`. Two instruments, one verdict, and neither is the
other's echo.

## 8. Where Phase 4a now stands

**CLEARED:** the five consumer functions exist in shipping vanilla, resolve
unambiguously, and are hookable by name.

**STILL OWED, and none of it is a formality:**

1. **Reachability per site.** A hook on `daBg_c::draw()` binds; whether the
   *clipping decision inside it* can be influenced from a hook — rather than
   needing a mid-function edit — is unanswered for all five.
2. **`d_a_bg.cpp:252` is a lifecycle change**, not a filter: it nulls
   `mpBgW`/`mpKCol` and returns `1` from `createHeap`. `docs/DO-NOT.md`'s
   BG/collision-registration entries and **DN-10** bind. **Read the donor's own
   system first.**
3. **The seven diagnostics need deleting or rebuilding plugin-side**, not
   porting. They were never blockers and should stop being counted as such.

---

## 9. PER-SITE REACHABILITY — and the residue is ONE root cause, not five

Hooking the *enclosing* function is not always the right move. The narrower
callee often carries the decision. Measured with `upstream_conformance.py`:

| site | verdict |
|---|---|
| `daBg_c::createHeap` @252 | ✅ **HOOKABLE, cleanly** — the WW branch nulls `mpBgW`/`mpKCol` and `return 1`, i.e. it short-circuits the *whole function*. A pre-hook returning 1 reproduces it exactly. No mid-function edit. |
| `dBgS_Acch::GroundCheck` @170 | 🟢 **NO HOOK NEEDED AT ALL** — see below |
| `daBg_c::draw` @425 | 🟡 **mid-function.** The decision is `if (!wwHost && clip(...)) shape->hide()` inside a shape loop. The narrow target is `mDoLib_clipper::clip` — but it is **AMBIGUOUS (2 raw entries under one name)**, so by-name binding cannot pick. Address-bind, or find another lever. |
| `daSwhit0_c::draw` @407 | 🔴 **BLOCKED — not on hooking** |
| `daMirror_c::draw` @643 | 🔴 **BLOCKED — same root cause** |

### 🟢 The roof check needs no hook: the native system already had the switch

`d_bg_s_acch.cpp:170` reads `if (!wwHost818 && !ChkGndThinCellingOff())`.
And in the same file:

```cpp
void dBgS_Acch::SetGndThinCellingOff() { m_flags |=  FLAG_GND_THIN_CELLING_OFF; }
bool dBgS_Acch::ChkGndThinCellingOff() { return m_flags & FLAG_GND_THIN_CELLING_OFF; }
```

**`wwHost818 == true` and `ChkGndThinCellingOff() == true` have the IDENTICAL
effect: skip the roof clamp.** Both accessors are **SAFE in vanilla**.

**This is DN-10 in its purest form.** *"NEVER bake — if there is an issue, AI
instances don't solve it, the native systems DO."* The fork added a WW-specific
condition directly beside a flag that already did the job. The native switch
was there the whole time.

It is arguably **better** than the fork's version: `m_flags` is **per-`dBgS_Acch`
instance**, so it applies to the actors that need it, while the fork's
start-stage test fires for every acch user in the stage — which is exactly what
its own comment worried about (*"every acch user; TP stages keep their clamp
untouched"*).

**Honest caveat, because per-instance is a real difference:** the plugin must
set the flag on the right instances at the right time, and *that* is unproven.
The call is available; the plumbing is not yet demonstrated.

### 🔴 The two lighting sites are one problem, and it is not a hook problem

```
dKyWw_settingTevStruct      fork SAFE · vanilla MISSING
dKyWw_setLightTevColorType  fork SAFE · vanilla MISSING
```

`daSwhit0_c::draw` and `daMirror_c::draw` do not need a *gate* — they
substitute **WW lighting calls** for native ones. Those functions are the
fork's tree-side WW kankyo layer (`d_kankyo_ww.h`) and **do not exist in
vanilla**. Hooking the draw functions is trivial; there is nothing to call.

**So the residue is not two sites. It is ONE root cause — the WW kankyo layer
lives tree-side — showing up at two call sites.** Any further lighting site
will join the same bucket.

**The ground under it is good:** `dKy_setLight` and `dKy_GxFog_set` are **SAFE
in vanilla**, so a plugin-side WW lighting layer has native entry points to
build against. This is Phase 5 work (typed, header-ful), not a patch.

## 10. Revised blocker count — third revision, each downward

| stated | count | basis |
|---|---|---|
| Foundry's briefing | **1** ABSENT-unhookable | symbol-level, flagged by its own author as *"a FLOOR"* |
| §2 of this document | **5** behavioural sites | after discarding 7 diagnostics |
| **now** | **1 root cause + 1 open question** | 1 site clean-hookable · 1 needs **no hook at all** (native flag) · 1 needs address-binding · **2 share one subsystem dependency** |

**Nothing is built and nothing is proven to run.** But the "last blocker" has
not survived contact as a blocker: it is a subsystem-location problem with a
known native landing site, and it is Phase 5's shape, not a patch's.

---

## 11. FIRST CONCRETE RE-SITING DESIGN — the roof-clamp site, zero receiver edits

### The fork already tried the native flag, and its own comment says why it stopped

`d_bg_s_acch.cpp` §818, in the fork's words:

> *"§796 fixed exactly ONE consumer (the player) via the per-actor flag; §817
> caught the pots dying the same way in Sturgeon's room. Per the №283 lifecycle
> law the skip moves HERE, the source: WW hosts get donor ground semantics for
> **every acch user**."*

**So the per-actor flag route is not a discovery — it is the fork's OWN earlier
approach, retired.** And the reason it was retired was **coverage**, not
mechanism: editing one actor fixes one actor. §818 generalized by moving to a
start-stage test at the source.

**That reframes my §9 finding and I am correcting it here rather than leaving
it flattering.** `SetGndThinCellingOff` is not an overlooked native switch. It
is the switch the fork used first and abandoned for a real reason.

### What changes the answer: a hook reaches every instance, an edit reached one

§796 could only set the flag where it edited code — one actor. **A hook on the
per-instance choke point sets it for every consumer**, which is precisely the
generality §818 went to a stage test to obtain.

| candidate choke point | fork | vanilla |
|---|---|---|
| **`dBgS_Acch::Init`** | **SAFE — exactly one** | **SAFE — exactly one** |
| `dBgS_Acch::GroundCheckInit` | SAFE — exactly one | SAFE — exactly one |
| `dBgS_Acch::CrrPos` | SAFE — exactly one | SAFE — exactly one |
| `dBgS_Acch::Set` | **AMBIGUOUS — 2 entries** (two overloads) | — |

And the flag survives, measured rather than hoped:

```cpp
void dBgS_Acch::Init() {
    ClrWallHit();
    for (int i = 0; i < m_tbl_size; i++) { pm_acch_cir[i].ClrWallHit(); ... }
}
```

**`Init()` clears wall-hit state only. It does NOT clear `m_flags`.** So a
post-hook doing `this->SetGndThinCellingOff()` is idempotent and is not undone,
whether `Init` runs once or every frame.

### The design

```
post-hook dBgS_Acch::Init(this):
    if (plugin knows the current start stage is WW-hosted)
        this->SetGndThinCellingOff();
```

- **Zero receiver edits.** The §818 hunk in `d_bg_s_acch.cpp` is not needed.
- **One hook, on a symbol SAFE in shipping vanilla.**
- **The effect is produced by the game's own flag**, not by authored logic —
  DN-10's order of resort, step 1.
- **The predicate needs no host symbol.** The plugin already knows which stages
  are WW-hosted; `dExtWwSave_isWwHostStage` is not required at all.
- It is **§818's generality via §796's mechanism** — the fork's two attempts
  each held half the answer.

### Still unproven, and it is the same shape as the gap that retired §796

**Do ALL acch users call `Init()`?** If any actor binds via `Set()` without
`Init()`, this hook has a coverage hole — *the exact failure mode that killed
the §796 approach*. `GroundCheckInit` and `CrrPos` are the fallback choke
points, both SAFE. **Measure the call sites before building.**

Nothing is built. This is the first Phase-4a site with a complete,
DN-10-conformant design and no patch in it.

---

## 12. THE COVERAGE CENSUS KILLED MY OWN CHOKE POINT — `Init` was the wrong one

§11 proposed hooking `dBgS_Acch::Init`. **Measured, and it does not work.**

Call sites in actor code, by member access (`something_acch.METHOD(`):

| method | call sites | files |
|---|---|---|
| `Init` | **0** | 0 |
| `GroundCheckInit` | **0** | 0 |
| **`CrrPos`** | **444** | **293** |
| `SetGndThinCellingOff` | 2 | 2 (the §796 remnants) |

**`Init` is SAFE, unique, resolvable — and no actor calls it.** A hook there
binds perfectly and never fires. That is `HANDOFF-INTEGRATOR.md` §7.4's
*"COMPILES BUT HAS NEVER RUN"* in a new costume, and the census caught it
**before** any code existed, which is the entire reason the check was owed.

**Stated precisely, because the sloppy version is another error:** those zeros
mean *no member-access call sites in actor code*. They do **not** mean the
functions never run — `GroundCheckInit` is demonstrably reached, because
`CrrPos` calls it internally. **"Actors do not call it" and "it never executes"
are different claims and only the first was measured.**

## 13. REVISED DESIGN — hook `CrrPos`, the universal path

`dBgS_Acch::CrrPos(dBgS&)` is the per-frame correction entry every acch user
goes through — `mLinkAcch.CrrPos(dComIfG_Bgsp())`, `mAcch.CrrPos(...)`,
`m_acch.CrrPos(...)` — **444 call sites across 293 files**, including the ALBW
actors. And inside it:

```
CrrPos(bgs)  ->  GroundCheckInit(bgs)  ->  GroundCheck(bgs)
```

So the flag set at `CrrPos` entry is in effect for **that same frame's**
ground check — the exact site of the §818 roof clamp.

```
pre-hook dBgS_Acch::CrrPos(this, bgs):
    if (this stage is WW-hosted)      // plugin-side knowledge; no host symbol
        this->SetGndThinCellingOff();
```

`CrrPos` is **SAFE — resolves to exactly one** in both fork and vanilla.

- **Zero receiver edits.** The §818 hunk is not needed.
- **One hook**, on the busiest and most universal acch entry point.
- The effect is the **game's own flag** — DN-10 step 1, not authored logic.
- **§818's coverage (every acch user) with §796's mechanism (the native
  per-actor flag)**, which is what neither attempt achieved alone.

### What is still unproven

- **Per-frame cost.** 444 call sites means this hook runs constantly. A
  trampoline on a hot per-frame path is exactly the measurement Phase 5 says to
  take before committing (*"hot per-frame subsystems stay tree-side"*).
- **Idempotence across frames is fine** (`|=`), but nothing clears the flag on
  leaving a WW stage. The hook must clear it too, or a TP stage entered
  afterwards inherits donor ground semantics. **`ClrGndThinCellingOff` exists**
  — that is the matching native call, and it is the obvious else-branch.
- Still nothing built.

---

## 14. CORRECTION TO §10 — it is TWO subsystems, not one root cause

§10 said the residue was *"1 root cause + 1 open question"*, on the basis that
the two lighting sites shared the `dKyWw_*` dependency. **`daBg_c::createHeap`
@252 was counted as cleanly hookable. It is not.**

Its guard has **two** predicates, and I only checked the first:

```cpp
if (dExtWwSave_isWwHostStage(stage) && dExtNpcMount_isRoomLaneRoom(roomNo)) {
    mpBgW = NULL; mpKCol = NULL;
    return 1;                       // "room-lane mount owns BgW"
}
```

```
dExtNpcMount_isRoomLaneRoom   fork SAFE · vanilla MISSING
```

**A second fork subsystem, and it is the larger of the two.** Measured:

| subsystem | files | distinct symbols | call sites |
|---|---|---|---|
| **`dExtNpcMount_*`** (NPC room-lane mount) | **57** | **113** | **804** |
| **`dKyWw_*`** (WW kankyo lighting) | **45** | **25** | — |

**The plugin does not implement either.** It references `dExtNpcMount` in
exactly two places, both *comments* in `registry.cpp` describing the fork's
parser — it never calls it.

## 15. Phase 4a, honestly stated — fourth revision

| site | status |
|---|---|
| `dBgS_Acch::GroundCheck` @170 | ✅ **DESIGNED** — pre-hook `CrrPos`, native flag, zero receiver edits (§13) |
| `daBg_c::draw` @425 | 🟡 narrow target `mDoLib_clipper::clip` is **AMBIGUOUS**; needs address-binding |
| `daBg_c::createHeap` @252 | 🔴 blocked on **`dExtNpcMount`** — 57 files / 113 symbols / 804 sites |
| `daSwhit0_c::draw` @407 | 🔴 blocked on **`dKyWw_*`** — 45 files / 25 symbols |
| `daMirror_c::draw` @643 | 🔴 same |

**So: 1 designed · 1 needs address-binding · 3 blocked on TWO tree-side
subsystems.**

### Why this is still not a patch argument

None of the three blocked sites is blocked *on a host symbol we cannot hook*.
They are blocked on **fork code living in the wrong place**. `dExtNpcMount` and
`dKyWw_*` are the WW port's own subsystems; they are tree-side only because the
fork built them there. Moving them is **Phase 5** (header-ful, plugin-side)
— the briefing's own direction — not a patch to the host.

**And DN-1 binds the `createHeap` site regardless of route:** *"NEVER stamp room
id 0 (or any guessed room) onto collision/BG owners."* That guard keys on
`roomNo`. Any re-siting must carry the real room identity, not a default.

### The pattern in my own three corrections today

§9 → §11 (the native flag was the fork's own retired approach) · §11 → §12
(`Init` is never called by actors) · §10 → §14 (a second subsystem I did not
check). **All three were "I checked the first thing and reported on the
whole."** That is §5's single most-repeated failure shape, and it took three
passes of my own work to stop producing it.

---

## 16. BUILT — the roof-clamp seam is re-sited. User-approved 2026-08-17.

`ww_donor_disc.dusk` **164,055 B**, sha `b00a3867…`, staged next to the
epoch-2 vanilla exe. Compiles clean against the epoch-2 SDK.

```cpp
HookAction on_acchCrrPosGate(ModContext*, void* args, void*, void*) {
    resolveAcchFlagFns();
    void* self = mods::arg<void*>(args, 0);
    if (self == nullptr) { return HOOK_CONTINUE; }
    if (startStageIsWw()) {
        if (s_fnAcchSetThinCeilOff != nullptr) { s_fnAcchSetThinCeilOff(self); ... }
    } else if (s_fnAcchClrThinCeilOff != nullptr) {
        s_fnAcchClrThinCeilOff(self); ...
    }
    return HOOK_CONTINUE;
}
```

Zero receiver edits. The two setters are resolved by name and **called on the
receiver's own object** — call-don't-hook, the same pattern as
`getStartStageName`.

### ⚠️ I nearly duplicated infrastructure that already existed

My first draft added a fresh `DEFINE_HOOK_SYMBOL` for
`?CrrPos@dBgS_Acch@@QEAAXAEAVdBgS@@@Z`, a new tag, and a new callback.
**All of it already existed** — declared at `registry.cpp:976`, with
`on_acchCrrPos` at 3449 and an install at 5381. I wrote 100 lines of duplicate
before grepping for the symbol I had just designed around.

**Same failure shape as §9/§12/§14, fourth instance today: I checked the first
thing and acted on the whole.** Reverted; the shipped change rides the existing,
proven declaration.

### Why a SECOND callback rather than editing the existing one

`on_acchCrrPos` is a **measurement instrument**, gated on `s_diagProbes` and
switchable off. The gate is **behaviour** and must run whenever the plugin is
active. Folding one into the other would make the probe un-disableable and hide
behaviour inside a diagnostic. **Add and label; do not substitute.**

The two also differ in kind: PRE hooks return `HookAction` (they may suppress
the original), POST hooks return `void`. This gate never suppresses — the
receiver's `CrrPos` must always run; we only set a flag it is about to read.
The first build caught the signature mistake.

### NOT PROVEN — it is built, not demonstrated

One boot decides it. The install and resolve both emit receipts precisely so a
silent failure cannot read as success:

```
{"ev":"acch_thinceil_resolve","set":1,"clr":1}   <- both setters bound
{"ev":"acch_thinceil_gate_install","r":0}        <- MOD_OK
```

**A `set:0` or `clr:0` means the gate is INERT and `d_bg_s_acch.cpp:170` is
still load-bearing.** Until that log line is read, this is
`HANDOFF-INTEGRATOR.md` §7.4's *"compiles but has never run"* — the exact
category that shipped a fork crash.

---

## 17. BOOT 211106 — INSTALLED AND RESOLVED ON UNMODIFIED VANILLA

```
{"ev":"acch_thinceil_gate_install","r":0}
{"ev":"acch_thinceil_resolve","set":1,"clr":1}
```

Host `c880d46fb5`, epoch 2, no receiver edits. **Both native setters bound by
name in the shipping binary**, and the pre-hook installed MOD_OK. The boot was
clean — the six `crash`-matching lines are the plugin's own guard messages
quoting a crash ID, not a fault.

**So the re-siting route works at the binding layer, on a stock build.**

### AND THE RECEIPT STOPPED ONE STEP SHORT — my instrumentation gap, not the design's

I wrote receipts for **install** and **resolve** and none for **invocation**.
`s_acchSetCalls` / `s_acchClrCalls` were incremented and never emitted, so
nothing in the log says the gate ever *ran*.

That is §7.7's chain — **announce ≠ record ≠ applied ≠ effective** — and I had
evidence for the first two while describing the result as though it reached the
third. The `acch_ground` probe cannot fill the gap either: it is gated on
`s_diagProbes`, which is off, so its absence measures the gate's switch and not
the hook.

**Fixed:** an `acch_thinceil_fired` receipt now emits at call 1, 100 and 10,000
(sparse, because `CrrPos` is a hot path). Rebuilt and restaged.

**Status, stated exactly:**

| step | proven |
|---|---|
| install | ✅ `r:0` |
| resolve | ✅ `set:1 clr:1` |
| **invocation** | ⏳ next boot |
| **effect** (actors stop falling through donor floors) | ⏳ needs a WW stage, in play |

---

## 18. BOOT 211829 — INVOCATION PROVEN. And the receipt still measured the wrong thing.

```
{"ev":"acch_thinceil_fired","total":1,  "set":0,"clr":1}
{"ev":"acch_thinceil_fired","total":100,"set":0,"clr":100}
```

**The gate runs.** 100 invocations, install `r:0`, both setters bound. Steps
1-3 of the chain are now evidenced on unmodified vanilla:

| step | proven |
|---|---|
| install | ✅ `r:0` |
| resolve | ✅ `set:1 clr:1` |
| **invocation** | ✅ **100 calls** |
| WW arm (`set`) | ❌ **never taken** |
| effect | ❌ not reached |

### `set:0, clr:100` — and the log ALSO says `"startstage_now":"sea"`

Read carelessly that is a contradiction: the start stage is a WW stage, yet
`startStageIsWw()` was false on all 100 calls. The likely resolution is timing —
those 100 `CrrPos` calls are menu/title frames that run **before** the stage
becomes `sea`, and the counter simply hit its next threshold at 100 and then
stopped talking.

**That is my third instrumentation misfire in this seam, and it is the same
mistake each time: I measured what was easy to count rather than what the claim
needed.** Thresholds of 1/100/10000 report *when the hot path is hot*. The
event that decides whether this seam works is the **transition into the WW
arm** — the first `set`. It could have been sitting one frame past the 100 mark
all along and the log would look identical.

**Corrected:** the receipt now fires on the **first `set`**, not on volume.
Rebuilt and restaged.

### What the failure DOES prove, and it is not nothing

**`clr` fired 100 times, meaning the CLEAR arm works.** That arm is the one
that stops WW ground semantics leaking into TP stages on a shared acch — the
"§818 hunk affects every acch user" hazard. It is exercised and correct.

**The `set` arm remains unproven, and until it is observed this seam is not
demonstrated** — only its safety half is.

---

## 19. ✅ BOOT 212953 — THE SEAM IS DEMONSTRATED END TO END

```
{"ev":"acch_thinceil_gate_install","r":0}
{"ev":"acch_thinceil_resolve","set":1,"clr":1}
{"ev":"acch_thinceil_fired","total":1,   "set":0,"clr":1}
{"ev":"acch_thinceil_fired","total":1265,"set":1,"clr":1264}
```

**All four links of the chain are now evidenced on UNMODIFIED VANILLA
(`c880d46fb5`, epoch 2), with ZERO receiver edits:**

| step | proven |
|---|---|
| install | ✅ `r:0` |
| resolve | ✅ both native setters bound by name |
| invocation | ✅ 1,265 calls |
| **WW arm (`set`)** | ✅ **fired** |

The shape is exactly right: **1,264 CLEAR calls through the menus, then the
first SET** the moment the start stage became WW-hosted. That is the transition
the corrected receipt was built to catch, and the earlier `set:0/clr:100`
reading was indeed a threshold landing in menu frames — not an inert gate.

**PHASE 4a HAS ITS FIRST RE-SITED SEAM.** `d_bg_s_acch.cpp:170`'s WW-host test
is replaced by a plugin-side pre-hook that throws the receiver's OWN
`FLAG_GND_THIN_CELLING_OFF`. For this seam the patch count is **zero** and the
receiver-edit count is **zero**.

**Read `"set":1` correctly:** the receipt fires *when* `s_acchSetCalls == 1`, so
that 1 is a property of the trigger, not a measurement of volume. **The true
set count after entry is unknown** until the `total == 10000` receipt lands.
Do not quote 1 as "it only fired once".

### What is proven, and what is still not

**PROVEN:** the mechanism. A native per-instance flag, thrown from plugin-side
at a universal choke point, on a stock binary.

**NOT PROVEN:** the *effect* — that actors stop falling through donor floors.
That needs the WW stage to actually draw and be walked, and **Outset still does
not draw on vanilla** (open, unrooted, see the plate). The seam being correct
does not make the stage visible; these are separate defects and this one's
success must not be read onto the other.
