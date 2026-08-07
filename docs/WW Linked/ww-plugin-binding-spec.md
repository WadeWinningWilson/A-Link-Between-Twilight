# WW plugin binding — spec (roadmap step 15, rewritten)

**Owner:** Foundry specs · Engine builds. **Distribution:** step 20 ruled **(A)
prebuilt plugin**, standalone, loadable into any dusklight build (§518).

> ## The premise of steps 15–17 was wrong, and the correction is in our favour
>
> The roadmap has Foundry *designing* a Hook ABI (15), Engine *building* native
> plugin loading (16), and both being *promoted upstream* to dusklight (17).
>
> **All three are already done, by dusklight, and none of them are ours.**
> `sdk/include/mods/` ships a C ABI with a versioned service registry, and
> `src/dusk/mods/loader/native_module.cpp` is the native loader.
>
> The user's constraint makes this decisive rather than merely convenient: **we
> do not own or ship dusklight**, so we cannot ship anything conjoined to it and
> cannot rely on upstream accepting an interface we invent. A spec that assumed
> step 17 would land was a spec resting on someone else's decision. This one does
> not.

---

## 1. What dusklight already provides

| element | evidence |
|---|---|
| C ABI, versioned | `MOD_ABI_VERSION 1u` — `sdk/include/mods/api.h:26` |
| per-service semantic versioning | `HOOK_SERVICE_MAJOR/MINOR`, id `dev.twilitrealm.dusklight.hook` |
| forward-compatible structs | `struct_size` on every record; "any field introduced below N is populated" |
| native module loading | `src/dusk/mods/loader/native_module.cpp` |
| lifecycle | `initialize` · `modDetached` · `lifecycleApplied` · `frameBegin` (registry.hpp) |
| services | camera · config · game · gfx · hook · log · overlay · resource · texture · ui |
| **symbol resolution** | `resolve()` — name → address, **file-local statics included** |
| hook forms | pre / post / replace, with priority and `HOOK_REPLACE_CONFLICT` policy |

**Everything §518 said the ABI must be — C not C++, versioned, negotiated,
refusing rather than crashing — dusklight already is.** Foundry designs none of
it. That part of §518 stands as a description of dusklight, not a requirement on
us.

## 2. What actually replaces step 15: the BINDING PLAN

The WW layer imports **1,150 host symbols (floor) / 6,448 sites** (B4, §517).
Each must be reachable by exactly one of:

- **(a) a published SDK service** — the supported path; versioned, negotiated,
  survives dusklight updates by contract;
- **(b) `resolve()` by symbol name** — supported by the hook service explicitly,
  *including file-local statics* (`HOOK_SYMBOL_LOCAL`: "hookable, but never
  linkable");
- **(c) neither** — the genuine problem set, and the only part that needs design.

**Classifying all 1,150 into (a)/(b)/(c) is the step-15 deliverable.** It is
measurable with instruments that already exist and requires no new interface.

## 3. The property that changed, and it is the important one

§518 warned that under (A) the ABI becomes a frozen public contract of 1,150
symbols. **That warning is now void**, and its replacement is different in kind:

> We are not freezing an interface. We are binding to **dusklight's symbol
> table**. "Works on any dusklight build" therefore means **"every symbol we
> resolve still exists, and still means the same thing, in that build."**

That is not a contract anyone signs — it is a property that must be **tested per
build**, which is exactly what the user asked for. Consequences:

1. **No surface-reduction crisis.** §518's "1,150 is too large to freeze" does
   not apply; nothing is frozen. Narrowing remains good hygiene, not a blocker.
2. **Symbol availability is build-dependent.** A stripped or differently-inlined
   dusklight release can drop a local symbol that our fork exposes. This is the
   real fragility and it is invisible until tested.
3. **ICF folding is a live hazard.** `HOOK_SYMBOL_MULTI_NAME` — "other names
   share this address (ICF fold/alias): a hook intercepts them all." A hook
   placed on one function can silently intercept an unrelated folded one.
4. **Version negotiation is free** for path (a) and **absent** for path (b).
   Every (b) binding is an unversioned assumption about a foreign binary.

## 4. Deliverable: the symbol-resolution conformance check

**Foundry builds; it answers the user's test question mechanically.**

Take the WW layer's resolved import list, attempt resolution against a target
dusklight build, and report per symbol: RESOLVED (with flags) · MISSING ·
FOLDED (`MULTI_NAME`) · LOCAL-ONLY. Run against:

1. **this modded fork** — the development target;
2. **pure upstream dusklight** — the shipping target;
3. each future dusklight release, as a regression gate.

**The user's expectation — that a purely additive mod works on both — is
probably right and is not assumed.** A diff between runs (1) and (2) is the
evidence. Where the runs agree, the plugin is portable by measurement; where they
differ, the difference is named before a user ever meets it.

Per №31-C: a symbol that cannot be resolved reports **MISSING**, never CLEAN.

## 5. Consequences for the rest of the roadmap

* **Step 15** — replaced by §2's binding plan. No ABI is designed.
* **Step 16** ("native plugin loading, THE LONG POLE") — **already exists**.
  The long pole was mis-identified; it is not loading, it is the (c) set.
* **Step 17** ("promote the ABI upstream") — **moot**. The ABI is already
  upstream because it was never ours, and we could not promote to a project we
  do not own regardless.
* **Step 18** — its emptiness now matters even less (§6).

## 6. Why step 18 stops mattering under this model

Step 18 assumed relocation: lift a self-contained subsystem out and drop it into
a plugin behind a designed interface. That framing needed subsystems to be
separable, which is why the WHOLESALE/PIECEWISE verdict governed it — and why an
all-PIECEWISE result emptied it.

**A hook-based mod does not relocate anything.** It loads alongside dusklight and
attaches by address. Whether a subsystem is "self-contained" stops being the
question that decides feasibility; **what decides it is whether its symbols
resolve.** The census's wholesale/piecewise axis remains true and remains useful
for judging *coupling cost*, but it no longer gates the boundary.

## 7. What this spec does NOT do

It does not classify the 1,150 (that is the next work item), does not choose
hook forms per site, does not assume the (c) set is small, and does not claim
portability between builds — it specifies the test that would establish it.
