# HANDOFF — Housing/Engine (instance retiring 2026-08-17)

era: mounted
<!-- era rationale: instance handoff of the vanilla-collision + quest-wiring campaign; state, not method law | Housing/Engine, 2026-08-17 -->

> *(Cross-reference block, carried forward from the 08-16 handoff. Six files now
> answer to "the Housing handoff". Nothing merged, nothing rewritten. Any older
> "the current one is…" pointer below this block predates it.)*
>
> | doc | kind | dated |
> |---|---|---|
> | [`HANDOFF-HOUSING-ENGINE-2026-08-17.md`](HANDOFF-HOUSING-ENGINE-2026-08-17.md) | **CURRENT INSTANCE HANDOFF** — the vanilla-collision campaign, the EVNT quest chain, the warp menu, four rulings. **Start here for present state.** | 2026-08-17 |
> | [`HANDOFF-HOUSING-ENGINE-2026-08-16.md`](HANDOFF-HOUSING-ENGINE-2026-08-16.md) | **PRIOR INSTANCE HANDOFF** — §1002 order + §7 monitor exemplar. Superseded for state; still owns its DO-NOT-TRUST list and the monitor pattern. | 2026-08-16 |
> | [`HANDOFF-HOUSING-ENGINE-2026-08-14.md`](HANDOFF-HOUSING-ENGINE-2026-08-14.md) | **PRIOR INSTANCE HANDOFF** — pre-08-16 history only. | 2026-08-14 |
> | [`HANDOFF-HOUSING-ENGINE.md`](HANDOFF-HOUSING-ENGINE.md) | **PRE-ORDER HANDOFF** — pre-08-11 history only. | 2026-08-10 |
> | [`HOUSING-HANDOFF.md`](HOUSING-HANDOFF.md) | **RETIRED-INSTANCE RECORD** — trust-failure account; not current state. | 2026-08-04 |
> | [`HOUSINGTEMP-HANDOFF.md`](HOUSINGTEMP-HANDOFF.md) | **A DIFFERENT LANE'S CHARTER** — listed because its name says handoff. | 2026-08-05 |
>
> **Picking this lane up: read this file, then [`LANES.md`](LANES.md) and
> [`DO-NOT.md`](DO-NOT.md).** Older files only for the history each owns.

Written to the §1002 six-section order. Cross-reader: Integrator.
Retired on the user's all-lanes retirement order, 2026-08-17 (Decoder exempt).

---

> ## LANE AWAKE — resumed 2026-08-18T15:14Z on the user's order. Watcher ARMED.
>
> *(Stood down 07:27Z on the all-lanes order; woken ~8h later. The instruction
> below was written for that gap and it WORKED — re-arming was the first action on
> waking, before anything else was read. Keeping it because it will be needed
> again, and because a stale "the watcher is off" banner at the top of a handoff is
> the exact failure this document keeps recording.)*
>
> **FIRST ACTION ON ANY RESUME, before reading anything else:**
> ```
> python -u tools/foundry/engine_watch.py --exit-on-event
> ```
> **19 rows landed during the stand-down and none touched this lane's surface**
> (all Decoder `ko1`/`so` work) — but that was luck, not design.
> **The last stand-down in this estate cost a missed call** — a `HISTORY, HOUSING`
> assignment landed unseen in the gap and the *user* had to point at it. That is not
> an argument against standing down; it is an argument for re-arming the moment work
> resumes, **and for the user not having to be the monitor.** This session armed it
> 54 times and re-armed inside the same turn on every single delivery — until this
> one, deliberately, on the order.
>
> **OPEN RIGHT NOW, in priority order:**
> 1. **The `wwHost` probe at `d_a_bg.cpp:425` — THE PIVOT, not a downstream item.**
>    `dExtWwSave_isWwHostStage` gates the collision skip at `:251` **and** the
>    per-shape clip at `:424`. One predicate, both symptoms.
> 2. **Grep the existing boot log for `№257 skip`** — free, no rebuild, and it
>    splits "daBg deliberately owns no BgW" from "it fell through to the dzb path".
> 3. **The two stall probes are FIXED and BUILDING** (per-id census + pump tally +
>    loud overflows). **Needs one boot** to read `m_is_creating_census` (`pinned`
>    count) and `ctrq_tally` (`max_pumps` splits the two faults).
> 4. The BMT `CLASS-ON-RAW` candidate — latent, armed, not blocking.
>
> **⚠ `room_set_bgw = 0` IS NOT THE SYMPTOM IT LOOKED LIKE — `setBgW` is inline,
> so that probe cannot report anything else. See §2.** The live symptom is that
> **the transition never completes** (`scn_change_req` 141 fired / 137 returning 0)
> **and Outset draws nothing.**

## 1. WHAT THIS LANE OWNS

Receiver-side WW work: `src/d/` and `src/d/ext_plugin/`, plus the WW half of
`mods-src/ww_donor_disc/`. Housing Security duty rides with it — zone rulings,
strip-set / push-gate questions, and anything asking "may this ship".

**LANES.md was stale on 08-16 and may still be**: it listed Engine as a Cursor
lane and said Housing "builds nothing". Both are wrong. You hold both.

---

## 2. HALF-DONE — the one open item, and it is buildable cold

**The convergence-point collision fix.** NOT started.

- **The defect**: the plugin converts collision data (`dzb`) in a hook keyed on
  the resource NAME. A `dzb` fetched BY INDEX never meets it and reaches
  `cBgW::Set` unconverted — un-rebased offsets resolve against their own field
  addresses. Deterministic garbage, **no crash**, bad AABBs, `-INF` ground.
- **The scale, measured** (Decoder sweep, `decode-drafts/index-fetch-inventory.md`):
  **307 donor TUs fetch by index; 100 pair an index fetch with a collision
  consumer in the same TU.** Per-site guards would be ~100 hand-placed calls that
  each fail silently when forgotten.
- **The shape, settled**: hook **`cBgW::Set`** — the point the donor's own code
  converges on. Every `dzb` passes through it however it was fetched, so it needs
  no proof about the other paths and no future port can miss it. The receiver's
  `ConvDzb` carries a `0x80000000` latch, so conversion is idempotent — that
  property is what makes a `Set` hook safe to adopt without auditing anything else.
- **Already landed as belt-and-braces**: an explicit `ConvDzb` at the one live
  index-fetch site (`registry.cpp`, `WwAkabe_solidHeapCB`).
- **The user's delivery ruling (plugin + patcher, 2026-08-17) makes this
  deliverable**: tree-side code is patcher-delivered, not undeliverable.
- **⚠ THE ENTRY BELOW IS RETRACTED BY A LATER BOOT (2026-08-18, second
  correction the same day). READ THIS FIRST.** I wrote that **nothing passes
  through `cBgW::Set`** on Outset, so the convergence fix was *"buildable but not
  verifiable"*. **MEASURED: `cbgw_set` n=3, ret=0 — it IS called and it SUCCEEDS.**
  Also measured: `dbgs_regist` n=3 ret=0 (admitted to the searchable set) and
  `bgw_ground` **2,700 calls / 274 TRUES / distinct 3**.
  - ***And that falsifies my own discriminator, in my favour's opposite direction:***
    I offered *"present with `trues=0` = the walk rejects everything"* as the
    unconverted-`dzb` signature. **274 trues means the walk HITS — the geometry is
    not garbage.** That is **evidence AGAINST the unconverted-`dzb` defect being
    the Outset cause.** The §2 fix may still be correct as a port; it is no longer
    supported as *this* blank's explanation.
  - **AND `room_set_bgw = 0` IS NOT MEASURABLE.** `dStage_roomControl_c::setBgW`
    is **inline in the header** (`d_stage.h:1222`, a three-line `static`), so the
    hook on its mangled symbol reads zero regardless. **`registry.cpp:921` already
    said so** — *"setBgW's zero receipts are an INLINING artifact (host warned)"* —
    twelve lines above the `DEFINE_HOOK_SYMBOL` it qualifies.
  - **AND ZERO CALLS IS THE INTENDED BEHAVIOUR HERE ANYWAY:** `d_a_bg.cpp:250-258`
    (`TARGET_PC`) returns **before** `setBgW` when
    `isWwHostStage(stage) && isRoomLaneRoom(roomNo)` — №257, *the room-lane mount
    owns collision*. **Free discriminator, no rebuild: that branch logs
    `[daBg] №257 skip room{} collision`. Grep the boot log for `№257 skip`.**
  - **BOTH SYMPTOMS TURN ON ONE PREDICATE:** `dExtWwSave_isWwHostStage` gates the
    collision skip at `:251` **and** the per-shape clip at `:424`. **The four-reading
    `wwHost` probe is therefore NOT downstream — it is the pivot.**

- **BUILDABLE IS NOT VERIFIABLE — READ THIS BEFORE YOU BUILD IT (2026-08-18).**
  The design above stands. **But on Outset today NOTHING PASSES THROUGH
  `cBgW::Set`**: Integrator measured `room_set_bgw = 0` on epoch-2 vanilla —
  `on_roomSetBgW` (`registry.cpp:3649`) hooks the room registration itself and it
  **never runs**. **A convergence hook on a point nothing reaches produces a
  perfect green and proves nothing** — the vacuous pass, on this lane's own open
  item, waiting for whoever follows this handoff.
  - **DO NOT FUSE THE TWO FAILURES.** Mine predicts registration **RUNS** with
    garbage data (un-rebased offsets, bad AABBs, `-INF` ground, no crash).
    Integrator's is registration **NEVER RUNNING**. *A defect that corrupts
    registration cannot cause its absence.* Both are "Outset collision is
    broken", and a reader meeting both will merge them — *repetition feels like
    corroboration*.
  - **THE DISCRIMINATOR IS ALREADY DEPLOYED**, twenty lines above that probe:
    `bgw_ground` (`registry.cpp:3641`) states its own reading contract — **ROOM
    BgW absent from `distinct` = never reached by the element loop (Integrator's
    failure); present with `trues=0` = its internal walk rejects everything (MY
    unconverted-`dzb` signature).** One free reading on the next boot settles
    which defect Outset actually has.
  - **SUSPECT REMOVED 2026-08-18 — `getRes` RETURNING VTABLE-BEARING OBJECTS FOR
    `.bmd`/`.btk` IS CORRECT, NOT A STUB.** Foundry measured 27 of 41 non-null
    `getRes` returns carrying a per-extension-constant first qword in the
    module-image range, and read it as a silent-non-draw shape. **Rooted here:
    the arc-load pass REPLACES the raw buffer with a parsed object before it is
    ever stored** — `d_resorce.cpp:455` sends `BMDR`/`BMDV`/`BMDE`/`BMWR`/`BMWE`
    to `loaderBasicBmd` -> `J3DModelLoaderDataBase::load` (a `J3DModelData*`),
    and `:556` sends `BTK `/`BTP `/`BPK `/`BRK `/`BLK `/`BVA `/`BXA ` to
    `J3DAnmLoaderDataBase::load`. **A vtable at offset 0 IS the storage contract
    for those node types; the `J3D2` magic is consumed at parse time and never
    reaches the slot.** The `.bdl`/`dzr`/`dzs`/`dzb`/`kcl`/`plc` asymmetry is the
    pack-archive branch at `:419` keeping those raw — **two contracts in one
    table, by design.** Corroborated by the codebase having an explicit opt-out:
    `dExtNpcMount_shouldSkipBtp` (`:562`) leaves `BTP ` raw *on purpose*.
    **A Yaz0 refusal cannot explain it either — a refusal returns NULL, and these
    are non-null constructed objects.**
    - **FALSIFIED AGAINST AND IT SURVIVED.** Foundry built
      `tools/foundry/res_consumer_census.py` specifically to kill this rooting:
      2,466 call sites bucketed by the cast applied to the result — **PARSED
      1,075, OVERLAY 79, RAW 32, UNCAST 1,280** — and **all 32 raw sites fetch
      `.dat`, `.bti` or a texture-bank index; not one is BMD/BTK-family.** Their
      own bucket error was caught pre-publication: `ResTIMG`/`cBgD_t` are
      **overlays on a raw buffer, not loader outputs** (there is no `TIMG`/`BTI `
      node type; `:155` casts a raw fetch straight to `ResTIMG*`) — ***an overlay
      cast is a RAW reading with a typed name on it.***
    - **THE ONE UNRESOLVED RAW SITE IS RESOLVED — `d_demo.cpp:566` FETCHES A
      `.stb`.** `branchFile(resName)` is called from `JSGSetData` (`:602`, `:614`)
      with a string parsed out of the STB stream itself — a cutscene naming the
      next cutscene — and the bytes become `m_branchData` -> `m_data`, the demo
      script JStudio runs. **`d_resorce.cpp` has NO `STB` node type in its parse
      dispatch**, so STB stays raw and `(u8*)` is the correct cast. **Raw bucket
      now 32/32 accounted.**
    - **THE 52%-UNRESOLVED LIMIT WAS RETRACTED BY FOUNDRY THE SAME HOUR — IT WAS
      THEIR REGEX, NOT THE CODEBASE, AND I HAD QUOTED IT HERE VERBATIM.**
      v1 required the cast to sit immediately after an `=`; real sites put the
      cast **inside a call argument** (`mpBgW->Set((cBgD_t*)dComIfG_getObjectRes(…))`)
      or on a **continuation line**. *"The 811 I filed as UNCAST were never
      uncast — they were UNMATCHED."* **Resolution 48% -> 88.9%; the verdict is
      unchanged and stronger: ZERO raw casts on a BMD/BTK-family resource, now
      across 61 enumerated raw sites instead of 32.**
      Final buckets of 2,460: **PARSED 1,867 · OVERLAY 179 · OPAQUE 71 · RAW 61 ·
      NULL-CHECK 9 · UNRESOLVED 273 (11.1%)**, with resolution *kind* reported
      separately because the kinds are not equal evidence (cast 2,048 direct;
      declared-type 117 and enclosing-return 13 are inference).
    - **AND THE OPAQUE SPLIT IS THE SHARPEST DISTINCTION ON THIS SURFACE:**
      widening the net took RAW 32 -> 132, and the new entries were nearly all
      `void* objRes = …`. ***`u8*`/`char*` ASSERT the slot holds bytes; `void*`
      asserts nothing and cannot misread a vtable.*** Leaving them merged would
      have published ~100 phantom candidates. *"A bucket that quadruples when you
      widen the net is usually the net, not the finding."*
    ***REMAINING LIMIT, in Foundry's corrected wording: 273 sites (11.1%)
    unresolved, enumerated in full and never truncated.*** Not "the consumer path
    is clean."
    ***AND THE STANDING LIMIT: this removes a suspect, it does NOT solve the
    blank.*** Do not let it close the draw question.
  - **THE DRAW QUESTION, FILED TO FOUNDRY 2026-08-18 — AND THE LEAD IS A
    MECHANISM, NOT A GUESS.** `d_a_bg.cpp:424`:
    `const bool wwHost = wwStage != NULL && dExtWwSave_isWwHostStage(wwStage);`
    then `if (!wwHost && mDoLib_clipper::clip(...)) shape->hide();`. **The §682
    comment immediately above states that the receiver's per-shape clip on WW's
    island-sized shapes turns view-matrix skew into WHOLE-TERRAIN HOLES — which
    is why the WW branch skips it. So `wwHost == false` on a WW stage is a
    complete mechanism for "draws nothing" needing no other defect.**
    - **The gate is DATA-DEPENDENT, which makes it plausible rather than
      theoretical:** `dExtWwSave_isWwHostStage` (`d_ext_npc_mount.cpp:11971`) is
      true only for `R_DL*`/`F_DL*` prefixes **or** names in `s_wwStageNames`,
      populated at runtime by `dExtWwSave_registerWwStage` (`:11931`) from a
      mod-side declaration (`:4724`). **Outset is room 44 of `sea` — a
      vanilla-named donor stage matching no prefix, so it is a WW host stage
      ONLY IF the declaration ran.**
    - **AND THAT SYMBOL IS `s815faa6d`, the FORK row Phase 2 just split out**
      (plugin binds `?dExtWwSave_registerWwStage@@YAXPEBD@Z`,
      `registry.cpp:5232`). The same registrar calls `dBootStage_add` —
      `sfb76e669`, the one surviving `ABSENT-unhookable`. **Both Phase-2 residue
      rows sit on this single call.** The draw failure may be a consequence of
      the tracker's own open items rather than a separate bug.
    - **Suspect killed by reading, so nobody spends a build on it:**
      `dExtWwSave_isWwHostStage` has a second definition returning `false`
      (`ext_plugin/ww_misc_dispatch.cpp:81`) — **not** a link ambiguity, it sits
      inside `#if defined(DUSK_EXCLUDE_WW_ACTIVE)`. Checked because this estate
      has a same-name-wrong-function trap on record.
    - **Ten-reading probe set filed** (registrar fired? · `wwStage`+`wwHost` at
      `:424` · hidden-vs-shown counts · `daBg_c` COMPLETE? · `bg_model` null? ·
      shape count · draw entered/frame · view-matrix sanity · `dBootStage_add`
      called? · the existing `bgw_ground` reading). **Reading ④ can moot the
      lead outright — if `daBg_c` never completes, both symptoms have one
      upstream parent and the clip question is premature.**
    - **NARROWED 2026-08-18 TO ONE HALF OF THE REGISTRAR, AND THE OTHER HALF IS
      RULED OUT BY THE BOARD'S OWN DATA.** History/Bridge verified the accessor
      (`d_s_play.cpp:1333` re-sets the start stage on every load, so
      `getStartStageName()` is the CURRENT stage — the "START sounds boot-only"
      suspicion is dead). And `sfb76e669`'s negative control, **written 08-16,
      predicted this observable two days early** — but it names **`dBootStage_add`**,
      the BOOT half, and predicts *"zero resources requested, no donor-disc
      serve."* **That is FALSIFIED here: 135 creates / 35 COMPLETE and 112 `getRes`
      lookups with 41 non-null. The stage LOADS and resources ARE served.**
      **So the boot half was reached; the live suspect is the `s_wwStageNames`
      half alone** — and the registrar's own comment (`:11934`) documents exactly
      that gap: *"that gap is what left the warp window showing only R_DL02 while
      'sea' was declared and staged."* A stage can be **reachable** while the
      WW-host set was never populated.
    - **PROBE, four readings at `d_a_bg.cpp:425` — one build, four different
      fixes:** `wwHost==true` -> clip is not the problem, look downstream ·
      `false` + `wwStage==NULL` -> accessor/timing · `false` + non-NULL + **no**
      `§632 WW stage declared` line -> **the set was never populated; fix is
      registration** · `false` + non-NULL + `§632` line present for that name ->
      **name-normalisation mismatch** (declaration carries `NAME,room,layer`, the
      registrar keys on `NAME` alone).
    - **DO NOT FUSE: the clip explains "draws nothing". IT DOES NOT EXPLAIN
      `room_set_bgw = 0`** — shape visibility and BgW registration are different
      paths. This board fused three threads into one twice in one night.
    - ***I have READ this path, not RUN it.*** Hypotheses with named readings,
      not a diagnosis.
  - **ORDER: Phase 2 -> Phase 5 typed rebuild -> registration returns -> then the
    `bgw_ground` reading.**

### SMALL STANDING ITEM — THE BMT CASTS ARE ARMED, NOT BROKEN (2026-08-18)

**Three sites cast a `getRes` result straight to `J3DMaterialTable*` with no parse
in between** — `d_a_kb.cpp:2481`, `d_a_npc_bm1.cpp:3891`, `d_a_npc_bm1.cpp:3917`.
**The cast is genuinely wrong:** `BMT ` is **absent** from `d_resorce.cpp`'s 22-entry
dispatch (`ARC BCK BCKS BLK BLS BMDA BMDE BMDG BMDL BMDP BMDR BMDV BMWE BMWR BPK BRK
BTK BTP BVA BXA DZB KCL`), so the slot holds **raw file bytes**; and
`J3DMaterialTable` is **polymorphic** (`virtual ~J3DMaterialTable()`,
`J3DMaterialAttach.h:32`), so it has a vtable at offset 0 **where the BMT magic sits**.

**It has never crashed because nothing dereferences the pointer.** Both consuming
shims discard it: `dExtKb_setMaterialTable(…) {}` is an empty body
(`d_ext_ww_actor_shims.cpp:33`), and `dExtNpcBm1_entryDL_mat` takes the table as a
**commented-out parameter** (`:471`). Fetched, stored, passed, thrown away.

**Both files are WW donor ports, NOT vanilla** (`KIT-LINEAGE: native-port`;
`d_a_npc_bm1`'s enum is Akoot·Skett·Basht·Bisht·Hoskit·Quill·Ilari·Pashli·Namali —
Rito names). That distinction is the whole point: *"vanilla presumably works"* would
mean leave it alone; **latent in our own deferred code means put the answer where it
will be implemented.**

> **THE MOMENT EITHER SHIM IS FILLED IN FOR THE `§246` RECOLOR, THIS BECOMES A LIVE
> CRASH.** The correct call is already in this tree —
> `J3DModelLoaderDataBase::loadMaterialTable`, used properly by our own WW code at
> `d_ext_npc_mount.cpp:2176/2180`.

**Precedent, so nobody treats it as theoretical: the sumo BMT crash (№50-C)**, whose
scar is quoted verbatim at `J3DMaterialAttach.h:44`. BMT plus a wrong lifetime
assumption is a proven crash shape here.
**Recommended (not done — live actor code, not touched unprompted): a one-line note
at each of the three cast sites naming `loadMaterialTable`.** Not blocking anything. Integrator's ruling that this is a Phase-5 regression
    in a `void*` path the typed rebuild deletes is accepted; registration worked
    before the fast-forward. **This item is not blocking and should not be built
    against Outset until registration is back.**

### CORRECTION, 2026-08-18 — "the one open item" WAS AN UNDERCOUNT. There are four.

**This lane had no CALLS watcher and no watch script — not a dead one, none.**
Three assignments landed on the board addressed to ENGINE and were never read.
The user caught them; the instrument did not exist to. **Everything above this
block was written blind to them**, so read the count in this section's title as
"one item I knew about", not "one item open".

- **THE HOST MOVED UNDER THIS LANE (CALLS 435, Integrator).** `dusklight-main`
  fast-forwarded 39 commits to `c880d46fb5` — a PURE fast-forward, zero
  conflicts possible. `GAME_SERVICE_MAJOR` went `1u -> 2u`, so **every
  game-coupled mod on that host is invalidated by design** (dusklight's own
  `shadow_mod` fails identically — the guard working, not a WW defect).
  **Phase 3, adopting the published `svc/save.h` · `svc/stage.h` · `svc/item.h`
  · `svc/window.h`, is unblocked and it is THIS LANE'S.**
  - **⚠ PRECONDITION NOBODY HAD WRITTEN DOWN (added 2026-08-18, Foundry's tree
    correction, re-verified here): "unblocked" means unblocked IN THE EPOCH-2
    HOST, NOT IN OUR TREE. `dusklight-main` is a SEPARATE CLONE, not a worktree
    of this repo.** Measured here: our HEAD is **`74958baaff`** on
    `integrate/dusk-api-coexist`, `git merge-base --is-ancestor c880d46fb5 HEAD`
    is **NO**, and our own `sdk/include/mods/svc/game.h` still reads
    **`GAME_SERVICE_MAJOR 1u`**. *(The commit object DOES resolve here — a
    commit-ish that resolves is not a commit-ish you contain.)* **Our fork cannot
    adopt services it is not built against; the epoch move is the real first
    step.** Any instruction to "re-run against current vanilla" must **name which
    tree**, or it silently measures the old world — which is exactly how
    `binding_plan.py` came to print `SDK services : 102` for the previous epoch.
  - The `SaveService` / `s49f0caba` analysis in §2 is **unaffected** — it was read
    from `../dusklight-main/sdk/include/mods/svc/save.h` and was always about the
    epoch-2 host.
  - **THE TRAP**: `MOD_ABI_VERSION` is `1u` **in both epochs** — the MOD ABI did
    not bump, the GAME ABI did. Anything keying on it to detect epoch 2 **takes
    the wrong branch silently**. Integrator used `__has_include(<mods/svc/hook.hpp>)`.
  - `mods/hook.hpp` is a deprecated 3-line `#warning` shim; `IMPORT_SERVICE`
    dropped `static` (internal -> external linkage), so a service may be
    imported in **one file only** — two imports died `LNK2005`.
- **THE YAZ0 REFUSALS — NOW OWNED BY HISTORY/BRIDGE, AND BIGGER THAN ITS OWN
  TITLE (CALLS 436 Integrator; 438 Decoder's pointer; accepted by History/Bridge
  2026-08-18).** *Do not restart this — read the status before touching it.*
  **The scope was understated by everyone including me: `res/Msg` holds 12 Yaz0
  archives, but the disc-wide census is 640 Yaz0 vs 681 RARC — ~48% of ALL
  archives.** The refusal therefore rejects roughly one archive in two on **any**
  path reaching that reader, not just messages. `decodeSZS` is transcribed from
  the Matching TU per DN-10 step 1 and **proven 12/12 against real donor
  archives** (each yields RARC at byte 0, length matching the Yaz0 header).
  **What is left is a LIFETIME problem, and it is this lane's specialty:**
  `rarcFindMember` returns a **non-owning** pointer and `wwMessage_open` caches
  views into it, so decompressing into a local hands back a dangling pointer —
  it needs a module-owned cache outliving the reader. **Whoever designs that
  cache: this is the J3D pointer-fix trap in a new costume** — never free an arc
  while parsed data is cached, and evict at BOTH release seams (§4 above).
  - **UPDATE 2026-08-18 — WIRED AND BUILT by History/Bridge.** `yaz0Decode` from
    `JKRDecomp::decodeSZS` (DN-10 step 1, nothing authored); module-owned cache
    that **never evicts**, refusing loudly instead, because eviction would
    reintroduce the dangling view; bounds checks receiver-side and labelled; the
    false *"measured uncompressed"* comment corrected in place, with the old
    refusal recorded as **SUPERSEDED rather than mistaken** — it was right while
    no decoder existed. They also found they DID have a build after twice
    declaring none: Ninja+MSVC at `mods-src/ww_donor_disc/standalone/build`,
    which needs `vcvarsall x64` (from a bare shell it fails on `cmath`, and that
    is the SHELL, not the tree — nearly filed as *"the merge broke the mod
    build"*). **RUNTIME IS UNOBSERVED. A build is not a boot.**
  - **AND THIS LANE FOUND A DEFECT IN IT — `ww_message.cpp:198` KEYS THE CACHE ON
    A RAW ADDRESS IT DOES NOT OWN:** `s_yaz0Cache[i].src == arc && .srcSize ==
    size`, entries never removed. **The never-evict policy correctly protects the
    OUTPUT buffers; the hazard moved to the KEY.** ① A freed arc's address reused
    by a different archive of equal size returns the wrong decoded bytes —
    **silently, because the hit path logs nothing**. ② *The likely one:* the same
    archive re-loaded at a new address MISSES and eats a fresh slot, so across
    stage transitions the 24 (`kYaz0CacheMax`, not the 12 quoted on the board)
    slots fill with duplicates and `yaz0Acquire` refuses **permanently** —
    reading as *"messages worked, then stopped"*, which no short run surfaces.
    **Fix at the key, not the policy.** Filed 2026-08-18.
  - **FIXED SAME DAY, AND THEY WERE RIGHT TO REJECT MY PREFERRED OPTION.** I
    recommended keying on the member name as "the true identity". **It is not:
    two different archives each containing a `zel_00.bmg` collide under
    name-keying** — it would have traded an address bug for a name bug. They
    took the content option instead: **FNV-1a over the whole compressed buffer +
    `srcSize` + the declared decompressed size**, all three matched on lookup
    (`ww_message.cpp:207` / `:241`, read and verified, not taken on the row).
    Address-independence **dissolves** the churn failure rather than detecting
    it — a re-load at a new address is now a HIT. Measured 12 archives -> 12
    distinct keys, 0 collisions, and a byte-identical copy at a different
    address hashes equal. The short-buffer guard also moved AHEAD of the header
    read, which the first cut had after the lookup. **And the hit path now logs
    (`yaz0_cache_hit`)** — my point that a cache reporting only its misses cannot
    be audited. Bound corrected to 24 at the point of use.
  - **SCOPE CORRECTION 2026-08-18 — AND THIS IS THE BIGGEST THING ON THIS PAGE.
    THE Yaz0 REFUSAL WAS NEVER A MESSAGE-SYSTEM LIMITATION.** `yaz0Acquire` sits
    inside **`rarcFindMember`**, the generic archive-member lookup, so it gates
    **four** callers: `main.cpp:271` `"stage.dzs"`, `:309` `"room.dzr"`, `:706`
    the BMG self-proof, `:735` the colour table. **Two of them are the STAGE AND
    ROOM LOAD PATH.** History/Bridge's census: `res/Stage` is **328 Yaz0 / 377
    RARC — 46.5% compressed**, and it varies *within* a stage (`Abesso/Stage.arc`
    is Yaz0 while `Abesso/Room0.arc` is RARC). **So the refusal was failing
    `stage.dzs`/`room.dzr` reads for nearly half of all WW stages, and the
    comment's wrong scope hid that for two days.**
    - ***I asserted the opposite** — "the only caller is the boot self-proof" —
      and the mechanism is worth more than the correction: I grepped
      `wwMessage_open` and `yaz0Acquire`, saw the call, and **never asked what
      function it was inside**. Every symbol I searched began `wwMessage_`, so
      the generic helper was unreachable by my own search. **A search shaped by
      the names you already know cannot return the ones you don't.***
  - **CONSEQUENCE THIS LANE THEN FOUND, OPEN AS OF 2026-08-18: CAP EXHAUSTION
    RETURNS BY DIVERSITY, AND CONTENT-KEYING CANNOT FIX IT.** One shared 24-slot
    never-evict table (`s_yaz0Cache`, `:225`) now serves all four paths. The
    original churn failure was *duplicates* and was correctly killed. **This is
    genuine diversity: 328 distinct Yaz0 stage archives against 24 slots.**
    Cross ~24 compressed stages and the table fills with legitimate entries, then
    refuses permanently — **at which point `stage.dzs`/`room.dzr` fail and stages
    stop loading**, not merely messages. The memory justification is stale too:
    sized for 12 message archives / ~1.05 MB, now holding decompressed multi-MB
    stage archives forever.
  - **RESOLVED SAME DAY — I DID THE LIFETIME CHECK AND IT DISSOLVES THE SEVERITY.
    THE STAGE/ROOM PATH RETAINS NOTHING.** `sniffStageType` (`main.cpp:263-288`)
    extracts one int and hands it to `wwRegistry_setStageType`, whose
    `stageMetaFor` does `strncpy` into the meta table (`registry.cpp:4656`) —
    name copied, type stored as `signed char`, **no pointer into the decompressed
    buffer survives the call**. `sniffSeaExits` (`:291-345`) is the same shape:
    `SCLS` names `memcpy`'d into a local `char nm[9]`, copied again by
    `wwRegistry_setStageParent`. **Contrast the message path, which is why
    never-evict exists at all: `wwMessage_open` stores `s_base`/`s_inf1`/`s_dat1`,
    pointers INTO the buffer that outlive the call.**
    **So stage/room reads can decompress into a caller-owned buffer and never
    touch `s_yaz0Cache`, which closes three items together:** ① the
    cap-exhaustion finding dissolves — **severity retracted, mechanism stands**:
    it was correct *given* stage/room transit the cache, and the fix is to stop
    them transiting rather than to grow the table; ② the ~1.05 MB justification is
    restored, and was never wrong for the path it described; ③ the multi-MB hash
    leaves the stage-load path entirely, so the prefix-hash change may not be
    needed at all. **Scope of what I actually read: the two stage/room callers to
    the bottom. `:735` (colour) I did not read; `:706` (BMG) demonstrably
    retains.** Anyone making the change should re-read those two functions rather
    than trust this — they are short, which is the point: **the check that closed
    three findings was about forty lines of reading.**
  - **FIXED AND VERIFIED 2026-08-18 — `rarcFindMember` NOW ROUTES BY LIFETIME.**
    History/Bridge added a `persistent` flag: **persistent** (`:509`, `:516` —
    `wwMessage_openFromArc`, `wwMessageColor_openFromArc`) keeps the never-evict
    table, because those genuinely stash views that outlive the call;
    **transient** (`:503` — `wwMessage_rarcFind`, i.e. stage/room) decompresses
    into ONE reusable scratch buffer. I read the routing, the scratch, and all
    four transient refusal paths: **sound.** New contract, stated at the
    function: *a transient pointer is valid only until the next
    `wwMessage_rarcFind` call* — both present callers satisfy it trivially, and
    `delete[]`-before-realloc is safe **because** of that contract.
    **All three findings DISSOLVED rather than traded:** cap exhaustion gone (328
    stage archives never enter the table), memory bound restored, and the perf
    note evaporates — `yaz0KeyOf` runs only on the persistent path, so the
    multi-MB-hash-per-stage-load cannot occur and the prefix-hash task is
    correctly downgraded to nothing.
    - **One figure corrected: "back to ~1.05 MB" is the TABLE, not the
      footprint.** `s_yaz0Scratch` only ever grows and is never freed, so
      resident = table (~1.05 MB, bounded) **plus one scratch at the high-water
      mark of the largest archive ever decompressed** — the dominant term for a
      multi-MB `Stage.arc`. Not a defect; a reusable scratch at max size is what
      one IS. But it is the same shape as the 12-vs-24 slip: **the friendlier
      number attached to the wrong noun, and the number is what gets quoted.**
    - **THE DISTINCTION THAT CLOSED ALL OF IT WAS LIFETIME, and it was available
      from forty lines of reading before the cache was designed, before the key
      was chosen, before any of it.**
    - **AND TWO LANES RAN THAT CHECK IN PARALLEL WITHOUT COORDINATING, WHICH IS
      THE PART WORTH COPYING: we converged on the same answer and each closed a
      DIFFERENT escape route.** History/Bridge proved the **archive pointer**
      does not escape (`setStageType` takes an int, not the pointer); this lane
      followed one frame further into `stageMetaFor` and proved the **stage name**
      does not either (`strncpy` into the meta table, `registry.cpp:4656`).
      **Either read alone would have left a live gap and looked complete.**
  - **PERF NOTE (mine, adopted by History/Bridge as the next change, not this
    one):** `yaz0KeyOf` hashes the ENTIRE compressed buffer on every acquire,
    hits included. Trivial for a 30 KB message archive, **not trivial for a
    multi-MB `Stage.arc` hashed on every stage load**. A bounded prefix plus both
    sizes keeps identical address-independence far cheaper. **It fixes COST, not
    CAPACITY — it does not add a slot, so it is independent of the item above.**
  Original framing, kept because it is what the defect looks like at the source:
  `ww_message.cpp:104` refuses any archive whose first four bytes are `Yaz0`,
  on a premise its own comment states — *"the donor message archives are
  measured uncompressed"* — which is **false for 20 of them, measured per boot**.
  Every WW message archive behind those is unreachable. **DN-10 step 1 is
  already discharged for you**: the donor's own decoder is decoded and
  byte-matched — `D:\XXXXXXX\WW DP\src\JSystem\JKernel\JKRDecomp.cpp`,
  objdiff **100.0%** vs retail. Header: `include/JSystem/JKernel/JKRDecomp.h`.
  (**Both spellings of the path are the same directory** — `WWDP` is a JUNCTION
  onto `WW DP`; `dir /x` prints `<JUNCTION>` and the target and `samefile`
  returns True. I first published this as *"two trees exist, don't assume the
  same revision"* — **wrong**, and wrong in a self-confirming way: I compared
  one file across the two names, found it identical, and reported a guarantee as
  a coincidence. See §3.) Minimal port is `decodeSZS` alone (pure
  function; `orderSync` is donor async plumbing, not the algorithm).
  **Do not hand-roll a decompressor** — that is the exact instance-authored
  shortcut DN-10 exists to refuse.
- **THE POST-MERGE SEAM RE-MEASURE — ⚠ THE "CLOSED AT ZERO" RESULT BELOW IS
  RETRACTED BY ITS OWN AUTHOR. RE-RUN IS REQUIRED. (Updated 2026-08-18, later
  the same night.)** I recorded this as *"fully closed, do not re-run"*; **that
  instruction would have made a successor skip a measurement now known to be
  necessary, which is the worst thing a handoff can do.** History/Bridge
  retracted the load-bearing premise: they had argued the 99 rows outside the
  four epoch-2 service domains *"cannot have moved"* rather than measuring them,
  and Phase 2's opening pass returns **69 symbol-level `ABSENT`-but-PRESENT-on-
  vanilla hits**. *"How wrong is what Phase 2 will establish, but wrong is
  already certain."*
  - **WHAT STILL SURVIVES, freshly re-verified rather than inherited:** the
    single `ABSENT-unhookable` row **HOLDS** — `sfb76e669`/`dBootStage_add`
    against the epoch-2 `StageService`, which is actor-level with no
    stage-registration entry point. **That half of the briefing question is
    genuinely answered.** So is the `s49f0caba` residue (below).
  - **AND THE RAW 119-of-123-STALE HEADLINE IS NOT A FINDING YET** — correctly
    withheld. Two contaminants: **our fork's `dusklight.exe` is 2 hours older
    than the tree** (built 21:08, HEAD `f2a854728c` at 23:00), so the `n_own`
    arm cannot be quoted at all; and the planned-port premise below.
  **Superseded tally, kept because the funnel's SHAPE is still the right method:
  122 examined -> 23 in-domain -> 19 `ABSENT` -> 16 already `PLUGIN` -> 3
  candidates -> 1 residue -> 0 flips.**
  - **PHASE 2 RESULT — QUOTE BOTH HALVES OR NEITHER: THE INSTRUMENT SAYS 21 ROWS
    FLIPPED; THE FINDING IS THAT THE MERGE MOVED ZERO SEAMS.** Anyone re-running
    `row_doorway.py` will see **21**, so a bare "0" reads as falsified; but the 21
    are a frame mismatch, so a bare "21" reads as seams having moved. **Both, same
    sentence, always.** 21 rows declare `ABSENT-*` for symbols present on epoch-2
    vanilla — but the symbols
    are `J2DScreen` · `J2DPane` · `JUTFont` · `daBg_c` · **`daAlink_c`**. *A merge
    of the mod host did not add Link to Twilight Princess.* Corroborated by error
    in BOTH directions at once (69 `ABSENT`-but-present, 76 `EXISTS`-but-missing).
  - **AND THIS LANE ANSWERED THE PROVENANCE QUESTION WITH A NEGATIVE — THERE IS
    NO MISSING REFERENCE IMAGE. Counted over `tracker/rows/*.md`: 5 rows cite a
    vanilla image, 118 DO NOT.** `s00a5b17e` is one of the 21; every citation is a
    donor `configure.py Object(Matching, …)` line, and **its own notes record that
    the author counted 102 receiver files referencing `J2DScreen` and set
    `ABSENT-hookable` anyway.** So `doorway` was authored to mean *"the DONOR's
    version is absent from our port"* while `row_doorway.py` reads it as *"absent
    from VANILLA"* — **one field, two propositions**, which produces bidirectional
    error by construction and needs no bad build to explain.
    ***Limit, stated: I counted CITATIONS, not measurements — "118 do not RECORD a
    vanilla check" is weaker than "118 were never checked". The 5 that do cite one
    are the control group.***
  - **THAT IS THE THIRD INSTANCE OF ONE SHAPE IN THIS ONE TOOL, and together they
    are the finding:** ① `doorway` on a **FORK** row — unsatisfiable, permanent
    green; ② `n_own` on a **planned-port** row — fires on the row's own premise
    (my gate: 160 -> 58 noise, 4 -> 69 OK, 65 rows recovered); ③ `doorway` on a
    **parallel-port** row — ~118 rows, and unlike ② **this one is not fixable with
    a branch; the two propositions need separating in the schema.**
    **Every one is a check applied to a row-kind it was not written for.**
  History/Bridge ran the funnel; this lane answered the residue; they then
  verified that answer against `save.h` rather than adopting it on the row.
  **A measured null, which is a better outcome than a flip** — the 121 verdicts
  were previously an unaudited assumption and are now a checked one. Their
  funnel:
  **122 rows -> 23 touch an epoch-2 service domain -> 19 carry `ABSENT-*` -> 16
  already route `PLUGIN` (a new plugin-side service confirms that route, it
  cannot change it) -> 3 -> 1.** They also caught themselves: **all 14 `window`
  candidates were a homonym** — `WindowService` is HOST DESKTOP window
  management (`create_window`/`set_title`/`set_size`), nothing to do with
  `J2DScreen`/`J2DPane`. And `dBootStage_add` **stays unreachable** on a fresh
  measurement: `StageService` is actor-level only, with no stage-registration
  entry point. What they explicitly do NOT claim: the 99 rows outside the four
  new-service domains were argued, not re-derived.
  - **AND `sfb76e669` IS NOW SPLIT (2026-08-18), which puts Integrator's
    "different verdicts" point into a field a tool can read**: `sfb76e669` =
    `dBootStage_add` alone, `PATCH` (vanilla's symbol, genuinely unreachable);
    new row `s815faa6d` = `dExtWwSave_registerWwStage`, **`FORK`** (ours, absent
    from vanilla because we wrote it). `FORK` was in `_schema.json` all along
    with zero users. **CAVEAT THIS LANE MEASURED: the `doorway` field on a
    `FORK` row is an UNSATISFIABLE CHECK** — `row_doorway.py:105` only flags an
    `ABSENT-*` row whose symbol turns up on vanilla, and a symbol we authored
    never can. **Confirmed by RUNNING it, not by reading:
    `[ OK ] s815faa6d (ABSENT-unhookable, 1 symbol(s))` — a green that could not
    have been anything else. A gate that cannot go red is not a gate.**
    - **Two corrections to my own filing, both History/Bridge's and both right.**
      ① I headlined it as *"that OK means nothing"*; **the `n_own == 0` check IS
      live on a FORK row**, so the green means *"still present in our fork"* and
      carries **zero doorway evidence** — narrower and more useful, because
      otherwise a reader concludes the row is wholly unvalidated and it isn't.
      ② I said the fix *"belongs in the validator, not the schema"*. **Too
      strong: a validator printing `[ N/A ]` fixes the misleading GREEN but
      leaves a false VALUE in the data** — the row still asserts
      `ABSENT-unhookable` about a symbol we wrote. **The validator change is the
      mitigation; `doorway: N/A` is the remedy.** Both, in that order.
      Integrator's call on the schema term.
    - The caveat now lives in `s815faa6d`'s **own notes** — mechanism, quoted
      branch, run output, what remains live — so a reader meets it at the green
      tick instead of in a CALLS row they'd have to know to look for.
  - **THE RESIDUE, AND MY ANSWER TO IT (measured 2026-08-18).** Row
    `s49f0caba` — `src/d/d_ext_save_flags.cpp`, `SPLIT` / `ABSENT-hookable`.
    The hypothesis was that `SaveService` collapses it to a clean `PLUGIN`
    route. **It does not, and the reason is specific: `SaveService` can only
    write the CURRENT slot.** Payload is a non-issue — `mFlags[0x100]`, 264
    bytes framed, against a 65536-byte per-mod budget (0.4%). And three of the
    five operations map exactly onto `observe_saves`: `d_save.cpp:1920`
    `storeSlot` -> `on_save_written`+`set_blob`, `d_save.cpp:1978` `restoreSlot`
    -> `on_save_loaded`+`get_blob`, `reset()` -> `on_new_save`. **But
    `eraseSlot` and `copySlot` are called from `d_file_select.cpp:2804` and
    `:2817`** — arbitrary slots by index (`mSelectNum`, `mCpDataNum ->
    mCpDataToNum`), from the screen where no slot is active, which is precisely
    when the current-slot calls return `MOD_UNAVAILABLE`. `peek_blob` gives
    cross-slot READ, so copy has its source covered and **no way to write its
    destination**. **WHAT WOULD ACTUALLY COLLAPSE IT: one added SDK call — a
    slot-targeted write, the write-side mirror of the `peek_blob` that already
    exists on the read side.** Until then the SPLIT is correct as filed —
    **verified independently at `save.h` and adopted by History/Bridge, who also
    endorsed the SDK ask to Integrator. That ask is the only thing between this
    row and closure, and it is not this lane's to build.**
  - **AND A REASON TO DO IT SOONER RATHER THAN LATER**: the bit accessors
    (`isEventBit`/`onEventBit`/`setEventReg`/…) have **zero callers anywhere in
    the tree today** — only the four persistence sites use this module at all.
    Moving the storage strands no hot path *now*; once WW actors start reading
    event flags tree-side, it stops being cheap.
- **(original framing of the re-measure, kept for the reasoning it carries)**
  The 122-row census,
  `binding_plan`/19a and the doorway classification were all taken against a
  **May-2026 vanilla**. The merge has now actually happened. Re-run against
  `c880d46fb5` and answer what everything downstream branches on: how many of
  the 87 ABSENT-hookable are now EXISTS or covered by a published service.
  Carry in Integrator's correction — `dExtWwSave_isWwHostStage` has **twelve**
  consumers, **seven pure diagnostics**, and of the five behavioural ones
  **three are blocked on fork subsystems living tree-side, not on hookability**.
  *"ABSENT-unhookable" and "our code is in the wrong place" are different
  verdicts and only the second is ours to fix.*

---

### THE CLOSING DEFECT, CAUGHT ON MYSELF BY AUDIT — ANSWERING IS NOT CLOSING

**Eight rows addressed to this lane sat OPEN while I had acted on seven of them.**
I read them, verified them, corrected the handoff from them — and answered by
**filing new rows**, never by appending to and closing theirs. From the sender's
side every one of those read as silence.

**This is the exact failure History/Bridge confessed to Decoder an hour earlier**
(*"an unanswered row IS no response, whatever happened in the tree"* — the user
had to tell them Decoder was waiting). I read that confession, agreed with it,
and was committing it eight times over, including against History/Bridge itself,
in the same hours I was filing rows about closing loops. **Reading a lesson is
not applying it; I only found this by running a count over the board.**

All eight are now closed with notes naming what was done and where the answer
lives — **including one closed as *read, not acted on*, because claiming credit
for a row I only skimmed would be the same lie in the other direction.**

**AND THE INSTRUMENT HAS THE MATCHING BLIND SPOT — this is the real bequest:**
`engine_watch.py` fires on rows addressed to this lane. **Nothing tells you that a
row you ANSWERED is still open, or that a row you FILED is still unanswered.** The
watcher covers the inbox and neither outbox. A successor should either extend it
or run the audit by hand — it is one pass over `CALLS.md` comparing author field
and checkbox, and it found eight in one run.

---

## 3. DO NOT TRUST — my own wrong published claims

- **"The EVNT name is at 0x04"** — WRONG, it is at **0x01**. I transcribed the
  donor HEADER. Implementing it chops three chars off 57/57 names and matches
  NOTHING — no fault, no log. The bytes settle it; the header contradicts its own
  later fields and its own stated size.
- **"Effective name capacity is 12"** — that reading MAKES AN 11-RECORD DATA LOSS
  INVISIBLE, because nothing ever looks too long. The field is 15 wide.
- **"My offline AABB was wrong"** — WRONG RETRACTION. My numbers matched
  History/Bridge's independent derivation to within the engine's own ±1 padding.
  I withdrew a correct measurement on an assumption about what a tree node covers.
- **"The 31 banner-declared files are a push-gate blind spot"** — premise retracted
  by Foundry (the gate uses a UNION basis) and **my ruling on it is VOID**.
- **"grass-first"** — retracted by Foundry: the receiver ALREADY has a vegetation
  actor covering ~60% of that surface. Do NOT port `d_grass`/`d_flower`/`d_tree`.
- **"Two decomp trees exist — `WW DP` and `WWDP` — don't assume same revision"**
  (published 2026-08-18, retracted the same hour). **WRONG. One tree.** `WWDP` is
  a directory JUNCTION onto `WW DP`. **The failure worth carrying is the SHAPE, not
  the fact:** I compared one file across both names, found it byte-identical, and
  published that as a *coincidence requiring caution* — when it was **guaranteed by
  construction**. My evidence could not have come out any other way, which makes it
  no evidence at all, and the hedge I attached ("mtimes differ by a day") was the
  junction's own creation date. **`os.path.samefile` / `dir /x` is one command and
  settles it.** Ask what result would have falsified you BEFORE publishing the
  caution — this is the same vacuous-pass error the monitors exist to catch, made
  by the instance that had just spent the turn measuring watchers for exactly it.
  - **IT IS A CLASS, NOT AN INCIDENT — THREE IN ONE NIGHT** (named by History/Bridge
    when it confirmed the retraction at the filesystem rather than on my word):
    **① this**, comparing one file across two names of the same directory, which can
    only return byte-identical; **② the vacuous-match trap**, where excluding a TU
    makes its REL match, so narrowing a label manufactures the evidence for widening
    it back; **③ the broken completeness test**, a statement filter that counted the
    function signature line, so no function could ever flag. ***An instrument that
    cannot produce the negative result is not evidence, however green it reads.***
  - **A RETRACTION CAN VALIDATE WORK AS EASILY AS IT KILLS IT.** Decoder's Yaz0
    pointer cited the `WWDP` spelling; History/Bridge verified against `WW DP`.
    **Under my withdrawn two-tree claim those were different files and the 12/12
    decode proof would have been against an unpointed revision — a real hole.**
    Under the junction they are one file and the proof stands. Either way you only
    find out by looking.
  - **AND THE CORRECT FACT WAS ALREADY WRITTEN DOWN, since 08-16:**
    [`decode-drafts/PR-STAGING.md`](state/ww-staging/decode-drafts/PR-STAGING.md)
    line 11 — *"donor tree at `D:\XXXXXXX\WW DP` (junction `D:\XXXXXXX\WWDP`)"*.
    **The estate did not lack the fact; the reader lacked a path to it** — this
    session's whole theme, one layer out from the index row and the LANES pointer.

---

## 4. LOAD-BEARING ASSUMPTIONS

- **`OFFSET_PTR` fields never take pointers.** I assigned a native array to one
  while the whole board was hunting an offset-vs-pointer bug. Cost: a compile
  error, because that field is TYPED. The same bug reached through a raw buffer
  cost a full day of probing. **Typed fields catch in seconds what raw buffers
  cost a day.**
- **WW-scope at the SOURCE** (`dExtWwSave_isWwHostStage`), never per-consumer
  guards. A catch-all that also catches TP names converts honest failures into
  silent inert props — strictly worse than the bug it replaces.
- **DISCOVER, NEVER AUTHOR.** Resource identity from TYPE, never a hardcoded
  index. Three authored constants were wrong out of three attempts in one night.
- **Parse once** (keyed `(arc,index,raw)`, evicted at BOTH release seams), and
  by-index resources arrive RAW — convert at the fetch site.

---

## 5. THE METHOD LESSON — the whole session in one line

**Nearly every defect — mine and other lanes' — came from reasoning about an
artifact instead of reading it.** A header that lied about a field offset. An
archive index taken from a comment. A metric that measured TU size when we needed
collision content. A tree node whose contents I inferred instead of checking.

The estate's answer, now doctrine: **read the artifact, publish the receipt, and
retract with a search step** — a retraction that reaches only the announcement
leaves the claim alive wherever it was quoted. That bit three lanes.

And its mirror, which cost a measurement we already had: **retraction needs a
receipt too.** Withdrawing on an assumption is the same error as asserting on one.

**AND THE RULE THIS HANDOFF LEARNED BY BREAKING IT (History/Bridge, 2026-08-18):
THE RAW OUTPUT AND THE INTERPRETED FINDING MUST TRAVEL TOGETHER, IN THE SAME
SENTENCE, ALWAYS.** On the seam census the instrument says **21 rows flipped**;
the finding is **the merge moved ZERO seams** (the 21 are a donor-vs-receiver
frame mismatch). **Quote either alone and the next reader is misled in opposite
directions** — *"0 flips"* reads as a closed measurement, *"21 flips"* reads as
seams having moved.

***I carried the finding without the raw number*** — §2 said *"fully closed, zero
rows moved, do not re-run this"* — **and Foundry then skipped the classification
citing it, crediting it to this lane, hours after its own author had retracted
it.** I had already published the retraction. It did not travel.

> **A retraction only travels as far as the artifact carrying it.**

**AND THE ONE TWO LANES DERIVED INDEPENDENTLY FROM OPPOSITE DIRECTIONS, WHICH IS
WHY IT IS THE ONE TO KEEP: NAME THE DENOMINATOR *AND WHY THAT POPULATION*, IN THE
SAME BREATH AS THE VERDICT.**

The Librarian reached it after publishing *"C0 sweep run estate-wide — 295 files,
zero"* when the tree is 19,800: **the count was true, the word "estate-wide" was
not.** Foundry reached it from the other side — *"a bucket that quadruples when you
widen the net is usually the net, not the finding."* **Same rule: a coverage number
is meaningless without its population, and the population is a judgement that must
be ARGUED, not merely disclosed.**

***And it caught me one row after I filed it against them.*** I wrote *"I ran the
remainder"* having swept **2,738 of 27,801 files — 9.8%.** The proof that the wider
net is not automatically better is in my own out-of-scope list: it contains
`tools/foundry/controls/c0_injected.txt`, **a deliberate fixture for the very audit
in question**, and two vendor MSBuild files where a BOM is *correct*. **A sweep that
widened to "everything" would have reported the Librarian's test fixture as a
finding.**

**AND ITS FINAL FORM, reached after the rule caught a THIRD instance inside the
correction of the second: A FILTER MUST BE ENUMERATED, NOT NAMED.** When the
Librarian restated their denominator as *"296 of 20,285 source-extension files"*,
**I could not reproduce 20,285** — I get 21,893, because *"source-extension"* is a
category LABEL, not a definition. **Evicting the ambiguity from the total merely
relocates it into the filter's name.** The only forms that survive a second reader
are **the extension list itself, or the command.** (Both raw totals — their 29,250
and my 27,801 — reproduced to the digit. It was only the *named* filters that
failed.)

**AND THE TWO FORMS THAT MADE IT MECHANICAL, which is what finally stopped it:**

1. **STATE THE PREDICATE AS CODE.** The Librarian's
   `endswith(('.cpp','.h','.hpp','.c','.inc'))` under prune
   `{.git, build, extern, node_modules, tools}` re-derived **3 for 3** on my first
   attempt — 19,800 / 19,747 / 53 — **after four consecutive rounds of prose
   failing.** Every earlier form was advice about being clearer; each was defeated
   on the next row by someone following it.
2. **CHECK THE NUMERATOR IS A SUBSET OF THE DENOMINATOR — AND THEN CHECK IT IS THE
   RIGHT POPULATION.** The Librarian's *"296 of 19,800"* had **disjoint** halves:
   the numerator was `tools/foundry/*.py` + `docs/**.md`, the denominator excluded
   `tools/` entirely. **Not one of the 296 was inside the 19,800** — *"a ratio whose
   numerator is not a subset of its denominator is not a scope statement at all"*,
   and it passes every earlier form.
   ***Mine passed the subset test and was still wrong***: my `2,738 of 27,801` =
   9.8% measured C++ sources against **all files** including `.md`/`.json`/`.png`.
   Type-matched, against the C/C++ population my numerator is drawn from (19,845),
   **it is 13.8%**. **Subset is necessary, not sufficient — the denominator must be
   the population the numerator is DRAWN FROM.**

> *"Knowing the failure mode did not prevent it; it did not even slow it down."*
> — the Librarian, one row before I demonstrated it.

**AND THE CONCLUSION THAT ACTUALLY CHANGES PRACTICE, theirs, proved five times in
one hour across four lanes: *"the only thing that has actually caught any of them
is another lane re-deriving the number."* Vigilance caught zero. Re-derivation
caught five. A single-lane sweep is structurally unable to validate itself — so
cross-derivation belongs in the normal path, not in escalation.**

*(And the counter-discipline, because this thread reached its own limit: the
FINDING — 51 BOM sources + 1 UTF-16 — was stable across three derivations by two
lanes using two methods. **Only the denominators kept moving. Do not mistake
recursion for rigour; a fourth pass would have been theatre.**)*

**AND A THIRD, WHICH IS THE SAME ERROR ONE LEVEL UP — DO NOT REPORT YOUR
INSTRUMENT'S LIMIT AS A PROPERTY OF THE SUBJECT.** Foundry published *"52% of
consumer sites are unresolved"* when the true figure was 11.1%: the other 41%
were **unmatched by a regex requiring the cast to follow an `=`**, not uncast in
the code. Every lane was told the path was half-dark on the strength of it — and
**I quoted the number into this handoff verbatim.** *"Visible only by READING THE
SAMPLE instead of trusting the bucket."* **A coverage figure is a claim about
your tool until you have read the misses.**

**AND ITS SIBLING, from the `getRes` rooting: A CORRELATION TEST CANNOT ANSWER A
QUESTION THAT NEEDS A MECHANISM READ.** The offered discriminator was *"are the
failing lookups' archives Yaz0 or RARC?"* — it would have found a cluster and read
as confirmation of a hypothesis that was **already false**, because the vtables are
correct either way. **Reading `d_resorce.cpp:455` settled in minutes what no
correlation could have settled at all.** The lane that proposed it named the gap
itself: *"I offered a correlation test for a question that needed a mechanism
read."*

**And the sharpest instance of the whole night's class, self-reported by the same
lane: they hold the standing note that J3D data is pointer-fixed — and the root IS
that note one layer earlier (the parse happens at arc-load; the raw buffer is gone
before `getRes` is ever called). *"I had the principle written down and did not
apply it to the very finding it explains."*** Writing a principle down is not the
same as it being reachable where it is needed — which is the same defect as the
missing index row, the unread `FORK` term, the junction line in `PR-STAGING.md`,
and my own retraction that did not travel.

That is the single most expensive recurring defect on this board, it appeared four
times in one night, and this is the instance where the person who wrote the
retraction got to watch it fail. **When you retract, go find every artifact
quoting the claim — the announcement is not one of them.**

---

## 6. RULINGS I MADE (Housing Security)

1. **`d_ww_` filename prefix, binding on every ported subsystem file.** The strip
   set is GENERATED from filename convention; a donor-faithful name matches no
   pattern, collides with no receiver file, and **ships to public `main`
   invisibly**. Donor fidelity inside the TU (symbols keep donor names); receiver
   convention on the boundary (filenames). **Recorded durably at `DO-NOT.md`
   DN-11; enforcement verified BY EXECUTION** (the manifest catches
   receiver-named WW legs across 65 files).
2. **Registration always goes mod-side** — it is donor identity content AND it
   must resolve on an uncontrolled binary; both reasons point the same way.
3. **Negative receipts are unprunable** — a `not-ww` row is evidence a question
   was asked and answered; deleting one silently re-opens the ruling.
4. **VOID: the 31-file ruling.** See section 3.

---

## 7. WHAT SHIPPED THIS SESSION

All stamped `COMPILE STATUS` honestly; boot-verified where a boot reached them.

- **EVNT quest chain, end to end** — translator (`ww_stage_loader.cpp`), full-name
  hook, and the name-COMPARISON path routed through six audited consumers. WW
  stage events can reach the event manager for the first time.
- **Warp menu** — flat select restored per the user's ruling, islands in authored
  STORY order (Outset first), spawn defaulting to the measured stand-on-land point.
- **A_mori exit crash** — root-caused to a dangling msg-archive slot, fixed at the
  lifecycle owner. The trigger recurred 5× in one session and was survived each time.
- **Slot-1 owned mount**, **WW room reload table** (DOOR case-trap + TGDR
  inversion), **particle-id arming**, and the derived **name → arc map**
  (`tools/foundry/ww_name_to_arc.py`, 441 TUs, 192 arcs, 18 multi-arc TUs found).

---

## 8. OPEN ON OTHER SURFACES

- **The staged collision receipt is UNREAD.** One boot answers it. The fault is
  bounded: dzb parse, index/tree structure, group pass-check, AABB build, walk
  descent and leaf selection are ALL eliminated by measurement; the crash is
  logged **unrooted with full evidence** rather than given a plausible cause.
- **A tale-boot audit** (msg 3095 in the d09 item box) — runtime-only, never run.
