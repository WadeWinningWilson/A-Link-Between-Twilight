# Donor pig↔bait dependency contract (Foundry §220)

> The §219 Pig verdict was "blocked on absent bait subsystem." This is that
> subsystem's donor spec, cited — the named prerequisite made implementable.
> Donor sources: `d_a_kb.cpp` (pig) + `d_a_esa.cpp` (bait, 餌) — both decompiled.

## The contract (verbatim evidence)

- Pig includes the bait header directly: `d_a_kb.cpp` → `#include "d/actor/d_a_esa.h"`.
- **Search:** `search_get_esa(kb_class*)` runs `fpcM_Search(esa_search_sub, ...)` —
  the pig scans live actors for `fpcNm_ESA_e`.
- **Acceptance criteria** (`esa_search_sub`, all cited in source):
  - bait `mState == 1` (landed/available — see esa statemap)
  - bait unclaimed: `field_0x298 == 0` **[INFERENCE-NEEDED: field name/semantics
    beyond "claim slot" unproven — read d_a_esa.h layout before porting]**
  - `|Δy| < 40` · XZ distance `< 400` · facing-angle window
    (`cLib_distanceAngleS` on angle.y)
- **Claim/release:** pig writes the bait's claim field (release site cited:
  `field_0x298 = 0`).

## The two state machines (spec skeletons, already extracted)

- `donor-statemap-bait-esa.md` — 6 transition writes (thrown → landing →
  available(mState 1) → eaten/expired lifecycle).
- `donor-statemap-pig-kb.md` — mode dispatchers + the esa-coupled action chain
  (approach → eat → follow behaviors ride the search result).
- `donor-parammap-pig-kb.md` — params decode for pig placements.

## Receiver mapping (covenant: donor logic verbatim, TP engine as substrate)

- `fpcM_Search` EXISTS in the receiver (same framework family) — the search/claim
  contract ports 1:1 onto engine primitives; no new engine subsystem needed.
- Missing pieces = content-lane work: (1) a bait actor socket implementing the esa
  state machine (item-drop entry point = the player bait-use path,
  **[INFERENCE-NEEDED: which port item/uses spawns it — History's call]**);
  (2) the pig's esa-coupled actions restored from the kb statemap.
- Verification: P13 tap pair on the pig's action stream per encounter scenario
  (bait-drop → approach → eat), seq-diffed vs the kb statemap.

## §221 — History RESOLVED both [INFERENCE-NEEDED] + locked the port spec

**[a] esa field layout — RESOLVED (cited `d_a_esa.cpp`):**
- `mState` (s16): **0** = thrown/falling · **1** = landed on GROUND, available · **2** = landed on
  WATER. Pig accepts **only `mState==1`** (`esa_search_sub:94`).
- `mGroundHeight` (f32): landing y — set from `dComIfG_Bgsp()->GroundCross` (+5.0) at spawn (`:36`);
  esa snaps `pos.y = mGroundHeight` and latches `mState` when `speed.y<=0 && pos.y<=mGroundHeight`.
- `field_0x298`: the **CLAIM slot** — `0` = unclaimed; a pig writes it to reserve the bait; release
  sets it back to `0` (`kb:167` `((esa_class*)pActor)->field_0x298 = 0`). Confirmed by the acceptance
  test `field_0x298 == 0` (`:94`).

**[b] which port item spawns the bait — RESOLVED (History's call):** the port has **no** All-Purpose-Bait
item, and adding the WW shop-item + drop mechanic is its own item-system effort. **Decision:** for the AI
restoration, the bait entry is a **stand-in drop** (a debug/context spawn of the bait actor near the
player), so the pig loop is testable now; the faithful All-Purpose-Bait item is deferred to the
item-system lane. Donor `esa` spawns via `fpcM_Create(fpcNm_ESA_e, 0, params)` (`d_a_esa.cpp:269`) — the
port entry calls the equivalent on the new bait proc.

**Port implementation spec (1:1, donor logic verbatim; content-lane, no new engine subsystem):**
1. **Bait ext actor** (new proc, e.g. `fpcNm_EXT_ESA`, plank-span template): fields `mState`,
   `mGroundHeight`, `mClaim` (`field_0x298`). Create → `GroundCross` sets `mGroundHeight`; execute →
   gravity-fall until `speed.y<=0 && pos.y<=mGroundHeight` → snap + `mState=1`; while available, wait on
   `mClaim`; when eaten (claimed pig reaches it) → delete. (Water-landing `mState==2` skipped —
   `dBgS_GetWaterHeight` absent, and the pig ignores `mState==2` anyway. Faithful in effect.)
2. **Pig AI** (on `NPC_KB`): each frame `fpcM_Search` for `EXT_ESA` with the acceptance test verbatim
   (`mClaim==0 && mState==1 && |Δy|<40 && XZdist<400 && cLib_distanceAngleS facing window`); on hit →
   `mClaim = pigID`, record target pos (`field_0x498`), walk toward it; on arrival → eat (delete bait),
   release. No bait in range → idle/wander (current mount idle).
3. **Verify:** P13 tap on the pig action stream (search→claim→approach→eat) seq-diffed vs `kb` statemap.

**Deps all present:** `fpcM_Search`, `fpcM_Create`, `GroundCross`, `fopAcM_searchActorDistanceXZ`,
`cLib_distanceAngleS` — verified in the receiver. **Status: implementable-from-spec; the 2-actor build
is the next focused content pass (playtest-iterated, not blind).**
