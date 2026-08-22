# WW plugin Outset — live state (Engine / Bridge)

era: plugin-delivery
<!-- era rationale: stock vanilla + ww_donor_disc.dusk + user ISO; no vanilla src edits -->

**Tip.** Product: **dusklight-main** + `ww_donor_disc.dusk` + user GZLE01 ISO. Warp sea **room 44 / point 5**. Not the fork exe.

| Field | Value |
|-------|--------|
| **status** | Outset playable on plugin. Stretch fixed via Pale BDL **finish** consume (DN-10 step-2). **Open:** Alwd/lwood Dawn device-lost when trees draw. |
| **next** | Alwd bisect modes **10** / **11** (`WW_LWOOD_DRAW`). Keep finish as default sea BDL consume. Optional later: split finish vs flags vs skip-setToonTex. |
| **do-not** | Invent anti-stretch “stabilizers.” Adopt fork MDL3-skip as class authority (Room44 falsifies). Blame lwood for stretch while mode=1. Mount / private ModelData / sunburst. Skip **How ports land** below. |
| **updated** | 2026-08-21 |
| **TP items on Outset — ANSWERED** | **Two different defects, measured by `del_by_class` (run 165339, overflow 0 so counts are complete):** **GRASS_e 118 created / 100 deleted / 18 SURVIVE** — the purge reaches grass and does not finish. **ITEM_e (rupees) 11 created / 0 deleted / 11 survive** — the purge never touches the class. Board-wide: 249 actors, 53 classes — 19 fully purged, 19 partial, **15 never touched (41 actors)**, 90 survivors total. **Lead worth more than grass's 100/118:** the partials repeat a shape — `OVERLAP0_e` 4/1, `ROOM_SCENE_e` 4/1, `BG_e` 4/1, `KANKYO_e` 3/1, `KYEFF_e` 3/1 — *one* deleted, the rest surviving, across unrelated classes. That says something about which SET the purge walks. |
| **TP items on Outset** | ⛔ **CONFIRMED BY THE USER — grass, flowers, rupees.** My "compliant by absence" reading is **RETRACTED**: I measured zero `fpcNm_GRASS_e` *created* after the era line and read that as none *present*. **Creation count is not population count when nothing logs deletion.** They map exactly onto two pre-warp classes — **`GRASS_e` ×118** (grass *and* flowers: `kusax1/7/21` + `flower/flwr7/flwr17/pflower` all resolve to it) and **`ITEM_e` ×11** (rupees). Zero created in-stage + visibly present ⇒ **they persist across the stage change.** DN-11 violation on the live product: receiver art standing on a donor island. **Next: why the stage change does not purge them** — that is this lane's stage/room surface. |
| **frame** | ✅ **USER-CONFIRMED 2026-08-21 — the picture is real, not just the log.** *"Outset drew, looked right… Matches Engine's documentation of the progress visually."* Closes the aurora caution below (`3eed7c2` can convert a fatal into a silent non-draw, so a clean log alone could never certify a frame). **Carried out of it:** unported items / TP-native placement — **grass** named — still need replacing. |
| **term 3** | ✅ **VERIFIED** on `dusklight-20260821-143653.log` — stock `c880d46fb5`, install named in-log as `dusklight-main`, both proven-firing fork markers read 0, no fork-only module on any of 4,870 records, and the run DID the work (room 44 worldize, 11,700 `bg_draw`). **Which binary ran is settled; what the frame looked like is not.** |

---

## How ports land (trace method — read before coding)

Plugin ports are not “make it look right on Pale.” They are **donor-traced, then hooked at the Pale consume boundary**. DN-10 binds every step: **(1)** read and port the donor’s own system; **(2)** translate only at the consumption boundary when (1) is *proven* insufficient on Pale; **(3)** instance-authored code only with written proof + user go. No guards, patches, bakes, mounts, or legs as the first answer.

### Trace method (the loop that shipped Outset)

1. **Orient on prior work (assist, not law)** — read this tip, `docs/AGENT_INDEX.md` → matching `docs/state/*`, Interconnected / WW Linked handoffs, and **field logs** (`%AppData%\TwilitRealm\Dusklight\logs\`, older plugin boots). Also mine the **fork’s prior Outset work** where it already named the donor chain, choke points, or dead ends (fork `src/d/d_ext_*`, room-loader notes, scoreboards in this tip) — as **hypothesis and map**, never as authority. If fork and MatchingFor disagree, **donor wins**; if fork “worked” via mount/MDL3-skip/private ModelData, treat that as a **falsifier candidate**, not a port recipe.
2. **Name the donor system** — MatchingFor (or Matching) source for **GZLE01**, not a fork invention and not TP-by-analogy. Cite `file` + symbol (and MatchingFor address when known).
3. **Read the full chain** — profile (list_id, priority, status, cull, `process_size`) → method table → create/heap/execute/draw → every resource index the donor uses (BDL/DZB/BTK from the arc’s own table / `Lwood.h`-class headers, never guessed).
4. **Find the Pale choke point** — the by-name or by-index call every consumer already hits (`getRes` / `getObjectRes`, `loadResource`, `modelEntryDL`, `ConvDzb`, `fpcPf_Get`, …). Port logic **there**, not by editing `dusklight-main` `src/`.
5. **Hook + resolve** — `DEFINE_HOOK_SYMBOL` / mangled MSVC names; bind Pale callees from `imports.lib` (or resolved symbols). Log `hook_manifest` / attach state — INSTALLED ≠ ATTACHED.
6. **Ship a receipt** — structured `[WwRegistry]` JSON (`"ev":"…"`). First boots prove create/parse/finish; later boots prove the defect class (stretch, Dawn, miss). Cross-check against **prior logs** (same `ev` names, mode scoreboards) so you do not re-diagnose a closed class.
7. **A/B with env gates** — one variable, one hypothesis (`WW_BDL_CONSUME`, `WW_LWOOD_DRAW`, …). Confirm the gate **armed in the log** before trusting the playtest.
8. **Only then step 2** — if authentic WW still fails on Pale (stretch under frame-interp is the receipt), translate at consume and **write why** (donor path named; Pale contract named). Do not delete the WW arm; gate it.

**Falsifiers we used and still use:** Room44 vs Alwd (same MDL3 class ≠ same fate); mode 1 vs multi-submit; `bdl_consume_gate` mode=`ww` vs `finish`; private ModelData / MDL3-skip invented then **reverted** when DN-10 failed the write-up; fork “works” paths that depend on mount or TARGET_PC MDL3-skip (assistive contrast only).

### Where instances should look (exact map)

| Question | Open first |
|----------|------------|
| Live tip / roadblocks / this method | **This file** — `docs/state/ww-plugin-outset.md` |
| Topic map → other live tips | `docs/AGENT_INDEX.md` → `docs/state/<topic>.md` |
| Hard stops (BG room, bake, mount, …) | `docs/DO-NOT.md` (**DN-1**, **DN-9**, **DN-10**) |
| Product shape (stock + dusk + ISO) | `docs/WW Linked/ww-clean-room-delivery.md` |
| How WW systems work (teachable) | `docs/gaming systems/` — especially `models-and-lighting/` (BDL/DN-3), `receiver-architecture/`, `debugging-methods/` |
| Prior sessions / fork-era handoffs (assist) | `docs/Interconnected Chats/`, `docs/WW Linked/`, `docs/state/ww-staging/` — use for named defects and paths; re-verify on donor + plugin logs |
| Debt when Pale cannot be 1:1 | `docs/WW Linked/port-liberties.md` |
| Fork prior implementation (assist / contrast) | Fork tree `src/d/d_ext_*`, room-loader / ExtNpc comments — **compare**, do not copy mount or MDL3-skip as law |
| Plugin entry / disc serve | `mods-src/ww_donor_disc/main.cpp`, `donor_disc.cpp` |
| Actor registry, BDL/DZB consume, lwood, akabe, kytag, worldize | **`mods-src/ww_donor_disc/registry.cpp`** — search `PORT —`, `KIT-DONOR:`, `DN-10`, `TRACE / PRIOR` |
| Vrbox / sky / clouds | `ww_vrbox.cpp`, `ww_sky.cpp`, `ww_vrkumo.cpp` |
| Waves / shore | `ww_wave.cpp`, `ww_shore.cpp` |
| Kankyo / wind | `ww_kankyo.cpp`, `ww_kankyo_wind.cpp` |
| Cam | `ww_cam_data.cpp`, `ww_cam_select.cpp`, `ww_cam_crawl.cpp` |
| Offline Alwd / Room44 MDL3 decode | `mods-src/ww_donor_disc/_research/` |
| Donor law (BDL arm, setToonTex) | zeldaret TWW `src/d/d_resorce.cpp` (`'BDL '` → `0x2020` + `setToonTex`); kit MatchingFor notes in registry banners |
| Pale draw contract (why stretch) | `src/m_Do/m_Do_ext.cpp` — `mDoExt_modelEntryDL` `#if TARGET_PC` frame-interp branch |
| Build / install | `mods-src/ww_donor_disc/standalone/build_install.bat` → both mods folders → wipe AppData `dawn_cache.db*` / `pipeline_cache.db*` |
| Field logs (current + prior boots) | `%AppData%\TwilitRealm\Dusklight\logs\dusklight-*.log` — grep `WwRegistry`, `bdl_consume_gate`, `lwood_draw`; compare scoreboards across timestamps |

**In `registry.cpp`, start at the banner comments**, not the middle of a hook body. Actor ports carry `PORT —` / `KIT-DONOR:` / prior-failure → resolution lists (lwood R2–R6 is the template). Consume-boundary translations carry an explicit **DN-10 step 2** block naming the donor/receiver format gap (DZB attribute repack; BDL finish gate).

**Do not start from:** treating fork ExtNpc / mount / MDL3-skip as authority; guessing resource indices; editing vanilla `files.cmake` for plugin cargo; “stabilizer” code on top of an unread donor system. **Do** start by reading prior tips, logs, and fork notes as a map — then prove every claim against MatchingFor + a fresh plugin receipt.

### Instruments per step (Foundry addition, 2026-08-21 — this method predates them)

*(Engine derived the loop without these; the next instance has them. All in
`tools/foundry/`, all with `--control` where noted — run the control first.)*

| Trace step | Instrument |
|---|---|
| 3 — read the full chain | `pool_align.py <tu>` (control-proven) — string/rodata pool placement vs target; Decoder's type-signal bank (`METHOD-type-signal-bank.md`) for reading types off diffs |
| 4 — find the Pale choke point | `res_consumer_census.py` (control-proven) — 2,459 `getRes`-family sites classified PARSED / OVERLAY / OPAQUE / RAW per node type: know what the receiver expects **before** wiring a consume |
| 6 — ship a receipt | `row_store.py` + the 123-row seam tracker — the reachability answer per symbol; `patch_feed.py --control` for baseline pinning |
| 7 — A/B with env gates | **A silent probe is not a result until it is proven able to speak.** The config key is the FLAT DOTTED `mod.wwDonorDisc.wwDiagProbes` — a nested object creates a phantom the loader never reads (cost three wrong reads on 08-17). Prove the gate can fire before reading its silence. |
| any absence claim | **Positive control first**: a grep that reads 0 on a compressed/packed artifact proves nothing (the `.dusk` is a DEFLATE ZIP — zero-bake is tested by payload-absence, not magic-absence) |

---

## Aurora backend patch — USER RULING, 2026-08-21

*(Recorded by Foundry from a direct user ruling. Housing/Engine owns this file —
reword or relocate freely; the ruling itself is the user's, not this lane's.)*

**Measured first, so the ruling has something under it.** `dusklight-main` is
stock where it counts: `c880d46fb5`, 0 ahead / 0 behind `origin/main`, **zero
modifications under `src/`, `include/`, `sdk/`**. Zero-bake holds — the shipped
`.dusk` is 238 KB with **zero** `RARC` / `Yaz0` / `J3D2` / `bdl4` / `TIMG` /
`dzb` occurrences. **No donor bytes ship.**

> **⚠ CORRECTION 2026-08-21 (Housing/Engine) — the first sentence holds; the
> second sentence's TEST does not.** `c880d46fb5` / 0-0 / clean `src include sdk`
> re-derived today: **unchanged**. But **`.dusk` is a DEFLATE ZIP** (`50 4b 03 04`;
> `mod.dll` 784,384 B inside 237,981 B), so that grep **cannot see any member's
> bytes** — positive control: `WwRegistry`, `ww_donor_disc` and `dusk` all read
> **0** on the shipped file while being certainly present. Re-run on the extracted
> payload, the magics **are** there (`RARC` 4 · `Yaz0` 3 · `J3D2` 5 · `bdl4` 2 ·
> `bmd3` 2 · `INF1` 3 · `dzb` 8) and **all 27 are `.text` immediates or
> `[WwRegistry]` log literals — what an honest zero-bake plugin must carry to
> recognise those formats in the user's own ISO. No donor payload ships: the claim
> stands on a corrected test.** Use **payload-absence, not magic-absence**.
>
> **AND THE ARTIFACT IS NOT REPRODUCIBLE.** Shipped `.dusk` md5
> `cc00092ef79ca5e6b5820adf67e2c75d`, 237,981 B, 2026-08-19 17:12:36, byte-identical
> in both installs — **but built from UNCOMMITTED source.** `WW_BDL_CONSUME` (4),
> `bdl_consume_gate`, `lwood_zero_shareddl`, `lwood_hide_shapepackets`,
> `WW_LWOOD_SKIP_PATCH` are in the shipped `mod.dll` and **absent from `HEAD`** —
> tree-wide, `WW_BDL_CONSUME` at `HEAD` exists only as a mention in `CALLS.md`.
> **The finish-consume this file credits with closing stretch lives only in the
> worktree; `standalone/build_install.bat` is untracked as well.** Do not "tidy"
> `registry.cpp` — reverting it deletes the fix. Committing is the user's call.
> Full audit: [`ww-staging/FINDING-milestone-terms-audit.md`](ww-staging/FINDING-milestone-terms-audit.md).

**The one qualification:** `extern/aurora` is a *pointer* change, not a dirty
worktree. Vanilla pins `cf3ffc9886`; this tree runs `e9ec4a701f` = that plus
**two local commits, on no remote branch of `encounter/aurora`** (2026-08-17):

| commit | subject |
|---|---|
| `3eed7c2` | Skip-draw on shader/pipeline validation failure; fix TEV alpha compare ops |
| `e9ec4a7` | Restore the `aurora::gx` logger in `pipeline.cpp` |

So the product today is **stock game source + a patched graphics backend + one
`.dusk`**.

> **⚙ MECHANISM DIRECTION, USER, 2026-08-22: AURORA GOES *PATCHED*, NOT HOOKED.**
> *"We either go hooked/or patched for aurora. I favor patched so we don't get into
> weird mid-areas."* So any backend change is a tracked patch against the pin, never
> a hook covering the same behaviour — **what is ruled out is splitting ONE behaviour
> across both**, not the plugin's existing hooks into the game. **This is the mechanism
> if we go; it is not approval to apply the 9-file / +3,144 candidate, which remains
> the user's call and sits tracked and unapplied.**

**THE RULING:** the patch is **ACCEPTED AS-IS FOR NOW** — *"a patch will work
just as cleanly until we decide to move it."* **The eventual target is to
ENVELOP THE AURORA PATCH INTO THE PLUGIN**, so the deliverable becomes stock
backend + one `.dusk`. **This is not a defect and not a blocker; it is a known
shape with a named end state.** Do not re-litigate it as a violation, and do not
quietly let it expand — anything NEW added to the backend needs its own ruling.

**COMMIT-HOLD RULING (user, 2026-08-21):** the uncommitted milestone source
(see the non-reproducibility correction above) is **held deliberately — the
commit lands WHEN LWOOD LANDS, not before.** Until then: no branch operations,
no `git checkout` on `mods-src/`, no "tidying" `registry.cpp`. History's
held-cargo ports (CALLS row 494) grow this pile knowingly.

**One caution that survives the ruling**, because it is about evidence and not
about terms: `3eed7c2` **converts a fatal into a silent non-draw — it cannot
make anything appear.** It is therefore not what renders Outset, but it *can*
mask failures. Any "it renders" claim on this build must confirm **the frame is
real**, not merely that the crash stopped. Falsifier if ever wanted: rebuild at
`cf3ffc9886` and boot Outset. Patches are captured at
`docs/state/ww-staging/aurora-patches/`.

---

## Progress (plugin delivery)

Deliverable shape holds: stock Pale + one `.dusk` + user ISO. Outset sea loads, island geometry, grass, vrbox/cloud ring, waves/shore, kankyo/wind, and WW cam ports are in. Trees (lwood / Alwd) stay **off by default** (draw mode 1) so the island is playable while Dawn is open.

| Area | State |
|------|--------|
| Stage / Room44 data from disc | Working |
| Room BDL parse + MULT worldize + EntryDL show (grass clip) | Working |
| Vrbox / vrbox2 / vrkumo | Working (parallax class separate from stretch bug) |
| Waves / shore / wind / cam | Working (user-ruled not stretch cause) |
| BDL consume (sea / grass / cloud) | **Translated for Pale** — see verdict below |
| Lwood actor create | Works; draw multi-submit kills Dawn |
| Leaf sway / NPCs / Wave B / sunburst | Held |

### Stretch — closed

**Symptom:** grass cards and part of vrbox stretched / lagged with no FPS drop; fine at Wave A commit `efa49c5c06`, broken after WW BDL consume landed.

**Cause (confirmed A/B):** authentic WW `'BDL '` arm — `loadBinaryDisplayList(0x2020)` + `setToonTex` + **keep** baked MDL3 SharedDL (no `newSharedDisplayList`). Correct on GameCube. On Pale, frame-interp `mDoExt_modelEntryDL` on non-sim frames only `diff()`s and skips `viewCalc`; that presentation path × kept WW SharedDL (and GD texture patch into it) stretches those models.

**Verdict:** not a missing WW stabilizer. Pale needs **receiver translation at consume**: `0x59020010` + always `wwFinishModelData`, no setToonTex. Gate: `WW_BDL_CONSUME` (`finish` = default on this install; `ww` restores donor arm for A/B). Log: `bdl_consume_gate`.

---

## Roadblocks (open)

### 1. Alwd / lwood Dawn device-lost — **primary open**

Plugin creates/collides Alwd OK; Dawn dies after a few draws when ≥3 trees submit (no CPU Fault). Default mode **1** (no draw) = PASS.

| Mode | Result |
|------|--------|
| 1 none | PASS (playable default) |
| 5 one-submit | PASS |
| 4 / 7 / 9 multi | FAIL @ n≈3 |
| 6 hide shapes | PASS Dawn — but skips MatPacket **entry**, not a SharedDL vs drawFast split |
| **10 zeroSharedDL** | **FAIL @ n=3 — RESULT IS ATTRIBUTABLE: the arm was PROVEN REAL** (`zeroed=2 mats=2 bytes_was=896 armed=1`; **896 = 2 x 448 B, exactly the offline decode's SharedDL size**). 3 x `lwood_draw submit:1`, then death ~295 log lines (~8 vrbox frames) later; **log truncates mid-line, no fault marker** — the known device-lost signature, not a CPU fault at the call. **Same-binary control the same minute: mode 1 ran clean to `all mods unloaded`** — one variable. **READING: the kill SURVIVES having no SharedDL bytes at all, so SharedDL CONTENT is not the trigger.** |
| **11 hideShapePackets** | **FAIL @ n=3, arm proven** (`hidden:2` per instance x4 calls, `pkts_total:8`, `unarmed_calls:0`). **AND IT NAMED ITS OWN CAUSE — the first run in this hunt that did:** `[FATAL | aurora::gx::fifo] fifo::write_data: buffer size overflow`. |
| **10 + 11 together** | **BOTH FAIL ⇒ the documented THIRD ANSWER: the kill is in NEITHER HALF ALONE.** Not SharedDL content (10 zeroed it), not `drawFast` (11 hid every packet) — and the FATAL says what it actually is. |

### The Dawn kill has a NAME now — and it is a RUNAWAY, not a poison packet

**`fifo::write_data: buffer size overflow`, from `extern/aurora/lib/gx/fifo.cpp:151`.**
Read the assert before reading the symptom: it fires when `sBufferSize + length`
would exceed **`UINT32_MAX` — 4 GB** — and the buffer **doubles on demand**. So this
is **not a small fixed FIFO being filled by heavy geometry.** Something feeds the
FIFO while nothing drains it, for ~8 frames, from **2 materials**. Per-frame volume
of that order is not a drawing cost; it is a loop.

**The drain path is conditional, and one condition is a latch:** `publish()` returns
early while **`sInDisplayList`** is true (`fifo.cpp:173`). An unbalanced display-list
capture — `begin_display_list` without its `end_display_list` — produces exactly this
signature, and it explains every prior reading at once: **mode 5 (one submit) PASS**,
**mode 6 (skips MatPacket entry) PASS**, **n>=3 FAIL** — the more times the path is
entered, the sooner the ceiling.

**FALSIFIED THE SAME HOUR, so it is not carried as a live theory: repeated
draw-time re-baking.** `model_finish` = **8 in all three runs — including the
PASSING mode-1 control.** The SharedDL is made at load, once per model, in every
mode. That is what a control is for.

**PROBE ROUTE CLOSED, MEASURED NOT ASSUMED:** aurora's FIFO state is **not reachable
by name** — `in_display_list` / `begin_display_list` / `end_display_list` /
`write_data_grow` all return **0 hits** in the stock exe, and a mangled-name sweep for
`@fifo@gx@aurora@@` finds **0 symbols**. The plugin cannot read capture state
directly; the next discriminator has to be indirect.

## 🔬 MODE-15 RESULT — FOUR HYPOTHESES DEAD, H6 (STALE `next`) STANDS

**Integrator built and ran the probe set (boot `154613`); this lane re-derived the
analysis from the log rather than relaying it, and every number reproduces.**

| field | dying (83) | surviving (6) | verdict |
|---|---|---|---|
| `h3_locked` | all `1` | `1` | ELIMINATED — shared |
| `h3_changed` | all `0` | `0`,`1` | ELIMINATED — shared |
| `h4_texno` | all `0` | `0`,`7`,`13`,`65535` | ELIMINATED — shared |
| `h7_opa_texedge` | all `1` | `1` | ELIMINATED — shared |
| **`h6_stale_next`** | **non-null 75/83** | ~~non-null 0/6~~ **VOID** | **SURVIVES on the victim side only** |

> **⚠ THE `0/6` IS A STRUCTURAL ARTEFACT OF THE CONTROL'S SAMPLING (History/Bridge
> caught it; verified here).** The control logged the first *N calls*, and there are
> more than six room models — so it observed **six models exactly once each**. A first
> observation is where the victims also read NULL. **That control could not have shown
> a stale next whatever the truth was**, so H6's survivor side proves nothing. The
> discriminator now rests on the victim-side transition (NULL at p:1–4, non-null from
> p:5) alone. **A control sampled differently from the victim is not a control** — fixed
> to sample per model, with `obs_of_this_model` in the receipt. The four eliminations
> stand: they compare values both legs emit at any depth.

**The 75/83 is really 75/75.** The eight NULL rows sit at sample index 0 and 1 of
**all four** models — the two materials of each model's first observed draw, before
anything has linked into it. The exception class is *"not yet linked"*, not *"dies
without a stale next"*.

**And a limit that cuts the other way: effective N is FOUR, not 83.** The rows are
4 models sampled 22/21/20/20, and every eliminated field carries exactly one value
across all 83 — a homogeneous population. The eliminations still hold (a value
shared with a survivor eliminates regardless of count), but the dying side is four
models and the six-model control is the larger set.

**Not a root cause** — acceptance remains a user-confirmed draw. Next: attack H6 at
the seam that sets it (`entryMatSort` clears the ENTERING packet, but after a merge
the head is another instance's packet), measured rather than argued.

## ⚖️ ACCEPTANCE STANDARD FOR LWOOD — USER RULING, 2026-08-21

> **A DEFECT IS ROOTED WHEN THE USER CONFIRMS A DRAW ON A NON-CRASHED OUTSET.**
> Not when a chain explains modes 5, 6, 10 and 11. Not when every observation is
> accounted for. *"Housing believed it to be so but is calling early wins instead
> of user CONFIRMING a draw."*

**This lane accepts it, and the record already agreed before the ruling arrived:**
the root-cause chain filed at 22:2x was retracted by this lane at 22:3x on its own
reading (`mDoExt_modelDiff` DOES call `entry()`). The ruling names the habit, not
just the one claim — **explanatory completeness is what a plausible-but-wrong
mechanism has too; it is the property that makes it plausible**, so it should raise
scrutiny of the weakest link rather than lower it.

**Binding on this lane's reporting from here:** lwood is **OPEN**. No mechanism is
called a root cause in this file or on the bus until a user-confirmed draw exists.
The commit hold (*"commit when lwood lands"*) is unchanged and correctly holding.

**WHAT SURVIVES THE RETRACTION — every MEASUREMENT stands:** `cycle:1` ·
chain growth to 4 · `bytes_was:896` matching the offline 2 × 448 · mode 9
eliminating create flags · mode 5/6 PASS · `locked:0` · the fifo FATAL.
**One causal link died; the census did not.**

## ⛔ RETRACTED 2026-08-21 — THE MECHANISM BELOW IS WRONG AT STEP 3

**`mDoExt_modelDiff` calls `i_model->entry()`** (`m_Do_ext.cpp:309` —
`calcMaterial(); diff(); entry();`). The claim that lwood's branch skips the joint
walk, and therefore never runs `J3DJoint::entryIn`'s `setShapePacket` reset, is
**false**. The reset was never missing, so mode 14 — which re-applies it — is a
**no-op**, and its own receipt says so: `restored:0, already:2` on every logged
call, the packet already pointing at its own shape.

**What survives is the observation, not the explanation:** the chains merge
(`chain:4`), `cycle:1` fires at draws 42–44, mode 11 dies on the 4 GB FIFO
overflow, and mode 6 — the only mode skipping the MatPacket entry — is the only
one that passes.

**And one column I over-read: `chain:4` may be NORMAL.** The census samples at the
start of a draw, so it reports the *previous* entry's merged state — and merging
same-material packets is exactly what `entryMatSort` is for. **The anomaly is the
CYCLE, not the chain length.**

**Next question is a PHASE question, not a packet question:** `frameInit()` clears
the draw buffer's slot table once per frame. If lwood enters *after* the buffer has
been drawn and cleared, its entries persist into the next frame and the prepending
never starts from a clean slot. Receiver actors enter from the actor draw phase;
lwood enters wherever the plugin's actor sits. **When lwood enters, relative to
`frameInit`/`draw`, is the measurement.**

## (retracted) THE CENSUS FOUND IT — THE PACKET CHAIN MERGES, THEN CLOSES INTO A CYCLE

**Mode 13, `dusklight-20260821-152053.log`. The flag I built for this fired.**

| draws | what the census shows |
|---|---|
| 1–4 | four distinct models, each `chain:1`, `cycle:0` — four independent instances |
| 5+ | **model `…A048`'s MatPacket now points at model `…C780`'s ShapePacket** (`shapepkt:…CAA0`), `chain:4` — one MatPacket has adopted **all four** instances' shape packets |
| 42–44 | **`cycle:1`** — the chain closes on itself |

**MECHANISM, and it is the one the eliminations pointed at:** every lwood
instance shares one ModelData, therefore shares its **materials**, therefore
`J3DMatPacket::isSame` is TRUE — so `J3DDrawBuffer` **merges** them into a single
MatPacket chain. The chain accumulates (1→4) and eventually **becomes circular**.
A draw walker on a circular packet list never terminates: it writes into the GX
FIFO forever, which is precisely `fifo::write_data: buffer size overflow` at 4 GB.

**It explains the whole scoreboard at once:** mode 6 PASSES because it is the only
mode that skips the **MatPacket entry** — the merge never happens. Modes 10, 11 and
12 all failed because SharedDL content, `drawFast` and the submitter are all
*downstream* of a list that no longer terminates.

### ⚠ AND A CORRECTION TO THIS DOCUMENT'S OWN READING — "dies at n=3" WAS A LOGGING ARTEFACT

`lwood_draw` only prints `n<=3` (then every 300th). **"FAIL @ n=3" was the last
receipt PRINTED, never the draw that died.** The census logs every draw and shows
the run reaching **draw 44**. Every earlier row in this table that reads "@ n≈3"
means "died sometime after the third *printed* draw." The count-threshold framing
built on it — one submit PASS, two PASS, three FAIL — is **not evidence of a
per-submit ceiling**; it is evidence about `s_submitBudget`, which is what modes 5
and 8 actually vary.

### Next: create flags, with the census riding along

**`WW_LWOOD_CENSUS=1` now rides ANY mode** — the instrument that found the merge
has to travel to the mode that tests the fix, not stay welded to mode 13. Armed:
**mode 9** (create uses `UseSharedDL`, not Differed — donor flags are
`0x80000 + 0x11000022`), census on. **The question it answers: does the merge still
happen when the instances are created differently?** `chain` staying at 1 means the
create flags own it; `chain` growing to 4 again means they do not and the merge is
J3DDrawBuffer's own policy for shared materials.

**⛔ MODE 12 FAILED — MY PREDICTION WAS WRONG AND THE SUBMITTER IS NOT THE MECHANISM.**
`submit_fn:"entry"` on all three draws, `lwood_entrydl_unresolved` **0** — the
receiver's own function genuinely ran, and it died at **n=3** exactly like the
others (no fifo FATAL this time; silent truncation, as in mode 10). **The
frame-interp multiplier is dead as an explanation.**

**THREE SUPPRESSIONS, THREE FAILURES, ONE INVARIANT: 10 (SharedDL zeroed) · 11
(every ShapePacket hidden) · 12 (receiver submitter) ALL die at n=3.** Whatever
the kill is, it does not care what the draw draws or who submits it. **The one
mode that PASSES with models alive is 6 — and 6 is the only one that skips the
MatPacket ENTRY.** So the entry is the suspect, by elimination rather than by
theory.

**ARMED: MODE 13, A CENSUS RATHER THAN A SUPPRESSION.** `mMatPacket` is
per-J3DModel (`J3DModel.h:129`, 0xC0) — but `J3DDrawBuffer` **merges** packets it
judges `isSame`, and every lwood instance shares one ModelData, therefore shares
its **materials**. Mode 13 submits normally and logs, **before each submit**, the
MatPacket pointer, its ShapePacket, the chain length and a **cycle flag** (repeat
pointer within a bounded 24-step walk). Logged before, deliberately: the run dies
inside the submit, and a receipt written after the call never arrives. Gate armed
(`WW_LWOOD_DRAW=13`); build md5 `03907bea…`, receipts verified in the shipped DLL.

**(prior) BUILT AND ARMED — MODE 12, THE RECEIVER'S OWN SUBMITTER.** The asymmetry is one
function call, and it is decisive-shaped: `daBg_c::draw()` submits with
**`mDoExt_modelEntryDL`** (`d_a_bg.cpp:452`); lwood submits with
**`mDoExt_modelUpdateDL`**. Read both in `m_Do_ext.cpp` and they differ in one
place that matters here: **`modelEntryDL` opens with a `TARGET_PC` frame-interp
early-out — `if (!is_sim_frame()) { diff(); return; }` — and `modelUpdateDL` has
NONE.** So every receiver model submits on **sim frames only**, while lwood has
been submitting on **every frame, interpolated ones included**. That is a
per-frame multiplier on the exact path that overflows a self-doubling buffer.
Mode 12 = mode 4 with `EntryDL` as the submitter; the `lwood_draw` receipt now
carries **`submit_fn`** (`entry`/`update`/`none`), and an unresolved symbol logs
**`lwood_entrydl_unresolved` at WARN** rather than silently falling back to the
arm it was meant to replace. Gate armed (`WW_LWOOD_DRAW=12`); build md5
`f4c338a6…`, 238,950 B, receipts verified in the shipped DLL.

**THE ORIGINAL FRAMING, KEPT BECAUSE IT IS WHY MODE 12 EXISTS.** Room44's
`model.bdl` is the **same SharedDL class** and survives every frame; it reaches the
GPU through **`daBg`'s draw**, while lwood reaches it through the plugin's own
`s_fnModelUpdateDL(m->model)`. Same class, same backend, different submitter, opposite
fate. **DN-10 step 1 says route lwood through the path the survivor already uses**
rather than inventing a fourth mode.

**Falsified:** “all WW MDL3 SharedDL is poison.” Room44 `model.bdl` is the **same class** (448B BP+XF SharedDL) and survives every frame. Kill is Alwd-specific (SHP1/drawFast, TEX bind, or plugin parse/create vs daBg path) under multi-submit.

**Not the path:** fork ExtNpc MDL3-skip as authority. DN-10: keep proving Alwd vs room delta; modes 10/11 next.

### 2. Faithfulness tension (documented, not blocking play)

WW donor `'BDL '` stays the step-1 truth. Pale sea BDLs ship on step-2 **finish** so stretch stays gone. That is intentional consume-boundary translation, not deleting WW. Do not re-litigate by re-enabling `ww` consume without an A/B purpose.

### 3. Held cargo

Leaf sway · sunburst · Wave B · NPCs — not blocking Outset stand-up; do not pull them in to “fix” Dawn or stretch.

---

## Alwd research notes (for next bisect)

Offline decode: `_research/alwd_mdl3_decode.txt`, `_research/alwd_vs_room44_mdl3.txt`.

| Fact | Detail |
|------|--------|
| Alwd file | `J3D2bdl4`; SharedDL **448** B each mat; **BP+XF only** (no draw ops) |
| Geometry | SHP1 shapes via `drawFast`, not SharedDL |
| Room44 | Same SharedDL shape/class — lives on Pale |
| Mode 10 | drawFast; SharedDL `mSize=0` — FAIL ⇒ vtx; PASS ⇒ SharedDL/load |
| Mode 11 | SharedDL callDL; shapes Hidden — FAIL ⇒ SharedDL; PASS ⇒ drawFast |

Env: `WW_LWOOD_DRAW=10` or `11`. Create gate: `WW_LWOOD_CREATE` (separate).

### Bisect procedure — two boots, one variable each

**Why this needed a code change first.** Both arms were silent: mode 10 logged
`lwood_zero_shareddl` unconditionally *after* the arm ran, so **"zeroed 12 SharedDLs"
and "zeroed nothing" printed the identical line** — and mode 11 armed every instance
but logged **once**, so one armed tree beside two live ones read as "armed" on a kill
that needs n>=3. **Neither PASS nor FAIL would have been attributable.** The arms now
return counts and the receipts carry them; `zeroed=0` and `unarmed_calls>0` log at
**WARN**. Verified present in the shipped binary before handing this over.

| step | do | read |
|---|---|---|
| 1 | Boot Outset with `WW_LWOOD_DRAW=10` | `lwood_draw_mode mode=10`, then **`lwood_zero_shareddl zeroed=N mats=M bytes_was=B`** |
| 2 | Boot Outset with `WW_LWOOD_DRAW=11` | `lwood_draw_mode mode=11`, then **`lwood_hide_shapepackets`** per call — `hidden`, `pkts_total`, `unarmed_calls` |

**Gate check before trusting either playtest** (trace method step 7): the mode line must
say the mode you set, **and the arm receipt must show a non-zero count.** A WARN on
either line voids that run — re-arm, do not interpret.

| outcome | reading |
|---|---|
| **10 PASS** (`zeroed>0`) | the baked MDL3 SharedDL is the kill path |
| **10 FAIL** (`zeroed>0`) | not SharedDL ⇒ the vtx / `drawFast` path |
| **11 PASS** (`unarmed_calls=0`) | `drawFast` is the kill path |
| **11 FAIL** (`unarmed_calls=0`) | not `drawFast` ⇒ SharedDL `callDL` |
| **either, with `zeroed=0` / `unarmed_calls>0`** | **NOT A RESULT** — the arm did nothing; the run says nothing about either path |

**10 and 11 are complements**, so PASS/PASS or FAIL/FAIL means the kill is in neither
half alone (TEX bind or the MatPacket entry itself) — a real third answer, not a
contradiction. **Artifact note:** the milestone `.dusk` (md5 `cc00092e…`) is preserved
at `ww-staging/artifacts/` because this build overwrote both installs; the bisect build
is md5 `4cd47e5a…`, 238,609 B.

---

## Build / install

`mods-src/ww_donor_disc/standalone/build_install.bat` → copy `.dusk` to both mods folders → wipe `%AppData%\TwilitRealm\Dusklight\dawn_cache.db*` and `pipeline_cache.db*`. No CMake reconfigure for plugin-only changes.
