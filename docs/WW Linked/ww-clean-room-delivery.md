# Clean-room delivery — what "fresh dusklight + plugin + ISO" actually gets you

**Owner:** Housing/Engine · **Allocated:** tale §931, revised §934 · **Raised:** CALLS row 85 (user, via Librarian), 2026-08-14
**Status:** answers (1) and (2) stand. **Answer (3) is CORRECTED — see the banner.**

---

## ⛔ BANNER — THE USER STATED THE PRODUCT, AND WE DO NOT BUILD IT (tale §934, 2026-08-14)

**The intended product, in the user's words:** *any user with any dusklight version downloads
the plugin, puts their own ISO in, the plugin connects to dusklight, and that is WW-in-TP.*

**Measured against that goal, the current architecture delivers 0% of it:**

| Question | Measured |
|---|---|
| WW receiver-layer TUs in `files.cmake` (the `dusklight` target's source list) | **49 of 49** |
| WW plugins shipping through the Mod SDK | **ONE** — `mods-src/ww_donor_disc/` |
| Plugin carrying the receiver/consumption layer | **none** |

> **CORRECTION (tale §938), same day:** this table's first row originally read *"mods declared
> through the Mod SDK = ZERO, repo-wide"*. **That was false — I grepped `dusk_add_mod` when the
> SDK's function is `add_mod()`** (`cmake/ModSDK.cmake:1`). A false negative from my own pattern,
> asserted without checking the pattern matched anything. The corrected reading is below.

**`mods-src/ww_donor_disc/` is a real, SDK-declared plugin** — CMakeLists + `donor_disc.cpp/.h`
+ `main.cpp` + `mod.json`, built via `add_mod(ww_donor_disc …)`, packaged to
`build/**/mods/*.dusk`. Its own `mod.json`: *"L2: serves byte-identical donor files straight
from the user's own Wind Waker (USA) disc image… Configure `mod.wwDonorDisc.wwIsoPath`."*
**That is exactly the "user places their ISO in" half of the stated product, and it already
exists in plugin form.**

**So the split is:**
- **DATA path → already a plugin.** Donor bytes are served from the user's own ISO by
  `ww_donor_disc`, through the overlay service, configured by a plugin config var.
- **CONSUMPTION layer → a fork.** All 49 receiver-layer TUs (actors, arrival tables, room and
  camera seams, ja1/evt1) are in `files.cmake` and compile into `dusklight.exe`. No plugin
  carries them, and a user on stock dusklight gets none of them.

**The gap is real but narrower than "0% plugin", and the difference matters for the work:**
plugin-izing the receiver layer is **not greenfield** — there is a working in-repo precedent,
an SDK, a package format, and a proven config-service path to copy from.

### The §512/§519 reading in this doc's original (3) was WRONG — my error
The original text said the in-tree layer *"stays in-tree — already ruled twice"*. Re-read
against the stated product, that over-claims what those sections decided:

- **§519 said hook binding attaches by address, so *separability stopped gating the
  boundary*.** That removes a **blocker** to plugin-ization. It does **not** say the code may
  live in the fork exe and still be a plugin — a hook must be *in* the mod to hook *from* it.
- **§512's empty relocate-list** was about which subsystems qualified as WHOLESALE moves under
  the old framing, not a ruling that the delivery target is a fork.

**I read "there is nothing to migrate" as an endorsement of the status quo. It was the
removal of an obstacle to a migration that then never happened** — the code kept landing
in-tree, pass after pass, including work I landed myself this session.

**Correct status of (3): OPEN AND LOAD-BEARING, not closed.** If the stated product is the
goal, plugin-ization is the central unbuilt piece of work, not a retired question.

> Scope of this doc: the three asks in row 85. It is a *measurement and a reading of existing
> rulings*, not a new ruling. Where the record already decided something I cite it rather than
> re-deciding it.

---

## The short answer

**A fresh *vanilla upstream* dusklight + our mod folder + a legal ISO shows almost NONE of the
WW work.** The delivery target can only be **our fork build**. This is not a packaging gap to
be closed later — it is what the architecture was ruled to be, twice, in §512 and §519.

---

## (1) Which "dusklight" is the delivery target?

**OUR FORK BUILD.** Measured, not assumed:

| Group | `.cpp` | `.h` | Where it lives | In the mod folder? |
|---|---|---|---|---|
| `src/d/ext_plugin/` | 14 | 14 | **in-tree, compiled into `dusklight.exe`** | no |
| `src/d/ext_seq/` (ja1 audio) | 22 | 13 | **in-tree** | no |
| `src/d/ext_evt/` (evt1) | 3 | 1 | **in-tree** | no |
| `src/d/d_ext_*` | 10 | 16 | **in-tree** | no |
| **total (this scope)** | **49** | **44** | | |

Plus the Tier-2 class: edits to *native TP files* (`d_stage.cpp`, `d_event_manager.cpp`,
`d_camera.cpp`, `d_a_bg.cpp`, …). Those are modifications to upstream files and cannot exist
anywhere but in a build of our tree.

**The mod folder was inspected and is DATA ONLY** — `arcs/ assets/ audio/ dialogue/ files/
getitem/ npc/ particle/ pause/ population/ ww_stages.ini modinfo.ini`. **No binary, no `.dll`,
no plugin payload.** A `find -maxdepth 2 -iname "*plugin*"` at the repo root returns **nothing**.

### The naming trap that makes this question hard
**`src/d/ext_plugin/` is NOT "the plugin".** It is receiver-layer code that compiles into the
exe. The actual mod-host machinery is `src/dusk/mods/loader/` and the `.dusk` package format
(§558). Anyone reading the directory name as "this ships separately" will reach the wrong
answer — which is, I think, exactly how this question arose.

---

## (2) Is there a reproducible clean-room test?

**No. It has never been exercised end to end.** Two independent confirmations:

- **No such document exists.** A glob for a clean-room / bootstrap procedure returns nothing.
- **The record says so in its own words.** Step **19b** (`ww-code-lineage-posture.md`) specifies
  the symbol-resolution conformance check — *"run against our fork, pure upstream, and every
  future release"* — and states that **"the diff between runs is what turns 'works on any
  dusklight build' from assumption into evidence."** It is written as the thing that WOULD
  produce the evidence, and it is gated behind 19a, which is still live work.

**So "works on any dusklight build" is currently an assumption.** §519 already flags this
precisely: *"we bind to dusklight's symbol table, so 'works on any dusklight build' is a
**testable property, not a contract we own**."*

### What a real clean-room test would have to be
Not "fresh dusklight + plugin + ISO". That formulation cannot pass, because the receiver layer
is not in either the plugin or the mod folder. The honest test is:

1. fresh **checkout of our fork** → clean build → `dusklight.exe`
2. + the mod folder (data)
3. + a legal GZLE01 ISO at the configured `wwIsoPath`
4. → boots, Outset loads, WW content renders

**Nobody has run that from a clean checkout.** Every result we have comes from an incrementally
built working tree. I am not claiming it would fail — I am claiming we have never shown it passes.

---

## (3) Does the in-tree receiver layer stay in-tree, or migrate plugin-side?

**IT STAYS. This is already ruled — twice — and the row's framing of it as open is out of date.**

- **Step 18** — *"relocate WHOLESALE-verdict subsystems into the plugin"*: **work list is EMPTY
  (§512).** The census returned 8 PIECEWISE / 2 VETO-PENDING / **0 WHOLESALE**. Nothing qualified.
- **Step 19** — *"leg → hook migration"*: **SUPERSEDED (§519)** — *"hook binding attaches by
  address; **there is nothing to migrate**."*
- **Step 17** — promoting the ABI upstream: **MOOT (§519)** — *"`origin` IS dusklight — we cannot
  promote to a project we do not own."*

**The hard floor from `ww-code-lineage-posture.md` is the reason:** *"no duplication of singleton
runtime state. Two JAudio stacks coexist; two J3D or two kernels do not."* The receiver layer
extends the receiver's own singletons; it is not relocatable without duplicating them.

**"Coexist" (the branch name `integrate/dusk-api-coexist`) means ALBT + main coexisting in one
tree — not the WW layer migrating out of the tree.** `mod-api-host-promote.md` states the target
as *"Public: promote clean ALBT host capabilities into TwilitRealm main; Local: full ALBT+main
coexistence tree."* That is a different axis from where the WW receiver layer lives.

---

## A count discrepancy this surfaced — flagged, not resolved

Three different numbers describe "the receiver layer", and **none of them carries a scope tag**:

| Number | Source | Scope as stated |
|---|---|---|
| **34 Tier-1 receiver files** | `NEVER-PUSH-STRIP-SET.md`, produced 2026-07-25 | strip-set membership |
| **~79 `.cpp` + 51 `.h`** | row 85 (Librarian's read) | "Tier-1 receiver layer" |
| **49 `.cpp` + 44 `.h`** | this doc, measured today | the four named dirs only |

These may all be correct over different sets, and mine is deliberately the narrowest —
**I counted only `ext_plugin/ + ext_seq/ + ext_evt/ + d_ext_*` and say so.** I am not asserting
the others are wrong. **This is exactly the failure mode CALLS row 78 is being written to
prevent** (mandatory SCOPE field on every number), and it is worth having as a live example
while that schema is still being designed: three honest counts, one subject, no way to reconcile
them from the numbers alone.

---

## What would change these answers

- (1) flips only if the receiver layer is made relocatable — which §512/§519 say it is not.
- (2) flips the day someone runs the four-step test above from a clean checkout. **That is a
  concrete, cheap, unclaimed piece of work** and it belongs to whoever owns runs.
- (3) flips only by a new ruling that overturns §512 and §519.
