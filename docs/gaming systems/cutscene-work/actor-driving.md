# Demo-actor driving (the §52/§53 read-back)

How the WW cast gets its position/animation during a cutscene, and why it fought the step-in-step
freeze.

## ① What the donor does

JStudio owns a `dDemo_actor_c` per demo actor. Each frame the game copies that object's wanted
transform/anim onto the real actor via **`dDemo_setDemoData`** (`d/d_demo.cpp:336-412`): it reads
`demo_actor->getTrans()/getRotate()/getScale()/getAnmFrame()` (gated by per-field ENABLE bits) and
applies them to the actor + its `mDoExt_McaMorf`. When the storyboard is **suspended**, the
`dDemo_actor_c` bails inside `forward` (`stb.cpp:118`) — its wanted values **freeze** — so the read-back
naturally applies frozen values and the actor holds. That is the donor's freeze: nothing special in the
read-back, just frozen inputs.

The port instruments this path as **§52 read-back** (`[ExtWw] §52 demo read-back …` — actor bound,
enables mask, wanted-vs-actual position) and **§53 demo DRAW** (per-actor draw with `at=`, `morf`,
scale). These logs were how we proved actors were *moving* during a suspend (Aryll `NPC_LS` walked
through 5 positions), which pinned the bug.

## ② Traps

- **TRAP — the read-back advances animation independent of suspend.** In the port, a demo actor's morf
  clip kept **looping** during the hold (Aryll's wave repeating until input). The read-back was pushing a
  fresh frame every tick regardless of `getSuspend()`. In the donor this can't happen because the bailed
  `dDemo_actor_c` stops producing new frames; our WW actors are driven by this read-back and slipped past
  the bail.
- **TRAP — confusing `m_frame` with motion.** `m_frame` advancing during a suspend does **not** mean the
  actors are moving; check actual `§53` `at=` deltas. See [storyboard-stb.md](storyboard-stb.md).

## ③ Correct method (§197)

Freeze the read-back while the storyboard is suspended — hold the actor's whole demo state:
```cpp
// dDemo_setDemoData, top (after the demo_actor NULL check):
if (dDemo_c::getControl() != NULL && dDemo_c::getControl()->getSuspend() > 0)
    return 1;   // hold position + animation until the suspend lifts
```
This matches the donor's frozen picture (the bail freezes JStudio objects; this freezes our read-back
actors the same way). It applies to all demo actors, which is correct — native TP demos suspend their
actors too.

## ④ Imperfect

- The freeze is all-or-nothing per suspend; we have not audited whether any WW demo actor legitimately
  needs to keep a sub-animation running during a message hold (none observed so far).
- Model/anim resource lifetime during demos is handled elsewhere — see
  [../models-and-lighting/bdl-bmd-parse-timing.md](../models-and-lighting/bdl-bmd-parse-timing.md) for
  the parse-timing rules that keep demo models from crashing.
