# HOUSING/ENGINE — work plate

era: era-independent
<!-- era rationale: lane work-continuation input, read by the timer | Housing/Engine, 2026-08-21 -->

Read by `integrator_timer.py --plate` (Decoder's timer shape, on the user's order:
**match Decoder's monitor + timer**). An empty plate stands the timer down; `- [~]`
marks an item blocked on someone else and does not count as open.

## Open

- [x] **`ww_sky.cpp` SPLIT — DONE 2026-08-22.** Six increments, commits `436c55503f`
  → `9655e48c56`. ww_sky.cpp 1155 → 424; `d_kankyo_rain.cpp` 1678, `d_kankyo.cpp` 928.
  Every increment built exit 0 with the artifact checked FRESH (not trusted from the
  exit code — build_install.bat has printed [OK] Installed over a fatal link error).
  Donor-owned bodies now sit in donor-named units; what remains in ww_sky.cpp is
  plugin-side orchestration with no donor counterpart (Always/phase poller,
  wwSkyMoveSun/MoveStar, wwSkyEntryBuf, the packet draw()s and public entry points).
  Sequencing that made it cheap: give shared state its donor OWNER first
  (`WwCelestial::setSunpos`, mirroring `dScnKy_env_light_c` 0xAB4/0xAC0), then the
  consumers move with nothing new to publish.
  Two defects found and fixed IN the split, both mine: inc.1/inc.2 left donor
  citation banners behind so the moved bodies lost their line refs and the banners
  floated above unrelated functions; and inc.6 claimed "every rain-owned citation is
  gone (checked)" when the check had not run yet and returned 1. Corrected in
  `9655e48c56`.
  **NOT verified: nothing in this split was diffed against the donor or RUN. The sky
  has not been observed rendering since inc.1.** A split moves code; it does not test
  it. That run is the open risk this item hands forward.
- [x] **`ww_wave.cpp` remainder — DONE 2026-08-22.** `b770e4115d` (usonami_set →
  d_kankyo, donor `dKy_usonami_set` d_kankyo.cpp:3427) and `72b0eaee62` (remainder
  merged into `d_kankyo_wether.cpp`; **ww_wave.cpp DELETED** and de-registered from
  both CMakeLists).
  **The plugin no longer fuses donor units** — every WW unit is named for the donor
  unit it carries: d_kankyo_wether 1486, d_kankyo_rain 1678, d_kankyo 952, ww_sky 424
  (plugin orchestration only), d_a_sea 134.
  Two mistakes recorded in `72b0eaee62`, both mine: a collision check that reported
  "NONE" while being structurally unable to see anonymous-namespace declarations
  (the form this codebase uses everywhere), and a failed transform whose `rm`/`sed`
  ran anyway because they sat outside it — file deleted with no merge done, restored
  from HEAD after confirming no other lane's work was in the diff.
  **NOT verified: never RUN.** Waves and sky unobserved since this work began.
- [x] **RULING FILED — in-tree target does NOT gain `FEATURES webgpu`** (`478cd79dca`,
  CALLS row filed). The call CALLS.md left this lane as `ww_gfx_spike.cpp`'s owner.
  The spike is a STOOD-DOWN diagnostic and the GX path it bypassed is fixed, so
  widening the canonical target's dependency surface (dawn) buys nothing. Gated with
  conditional inline no-ops in `registry.h`; **all four call sites byte-identical** —
  main.cpp and registry.cpp untouched. Closes 4 of the 23 unresolved symbols; the
  other 19 (dllimport static data, symgen filter) are not this lane's.
  **MEASURED:** standalone builds exit 0, spike still compiled, artifact fresh.
  **NOT LINK-TESTED:** no parent project here includes the in-tree target, so the
  in-tree link is reasoned from static evidence only. Integrator can settle it.

- [~] **WW GRASS AND FLOWERS BLACK — REFRAMED, NOT A LIGHTING DEFECT. Artifact:
  `FINDING-outset-vegetation-is-TP-grass.md`.** The WW colour path WORKS at every hop
  (pale_bind 57/37/52/10; bg_overlay writes real colour; room 44 models correctly
  typed 32/33/35 — 8/8/1 mats confirmed offline; actor_leg amb = Pale[4].actor_c0).
  **THE WW VEGETATION ACTORS ARE NOT PORTED.** Room 44 places 164 of them
  (`kusax1` 46, `kusax21` 36, `kusax7` 30, `pflower` 41, `flwr*` 11) and `kusax` /
  `pflower` / `flwr` appear **0 times** in the boot log — no spawn, no failure.
  What IS on Outset is the RECEIVER's `fpcNm_GRASS_e`: **100 created AFTER the sea
  bind.** TP grass draws via its own J3DPacket+entryImm, never reaches the WW overlay,
  and takes colour from a receiver kankyo that has nothing for a WW stage → unlit.
  **BLOCKED ON A LANE HANDOFF, not on this lane:** the fix is porting the donor's
  `kusa`/`pflower` actors (HISTORY, porting queue). Colour-patching TP grass is
  forbidden — DN-11 (no TP assets in WW spaces) and DN-10 (port the donor's system).
  **AWAITING USER CONFIRMATION** that the black objects are those TP grass actors;
  measured everything else, but that last step is theirs.
- [ ] **PORTING QUEUE — History has a backlog; this lane is now free.** lwood was the
  blocker holding the WW actor port; with the packet discipline understood, the same
  one-entry-per-fill rule applies to every J3D-path WW actor ported next.

- [x] *(STOOD DOWN — the GX path is fixed, so the bypass is a diagnostic, not a path.)* **GfxService SPIKE increment 1 — ran, succeeded.**
  `mods-src/ww_donor_disc/ww_gfx_spike.cpp` (new TU), declared in `registry.h`,
  called from `main.cpp` init/shutdown, added to `standalone/CMakeLists.txt`.
  Gated `WW_GFX_SPIKE=1`; unarmed builds pay nothing. Answers the three
  preconditions in ONE boot — is GfxService acquirable, does `register_draw_type`
  take, does `SCENE_AFTER_TERRAIN` fire — before any WGSL work is paid for.
  **The import is `IMPORT_OPTIONAL_SERVICE`, deliberately:** the required form
  makes the mod FAIL TO LOAD when the host cannot satisfy it, which would take
  the whole plugin — working Outset warp and disc reader included — offline.
  Pre-flight clean: braces/parens balanced, both entry points at global scope
  (anon-namespace link trap checked), 5/5 call sites arity-matched.
- [x] *(STOOD DOWN — see above; recipe preserved at docs/RECIPE-plugin-side-j3d-rendering.md.)* **GfxService SPIKE increment 2 — real mesh ran, 21,232 draws, zero overflow.**
  Deliberately NOT written yet: it is the expensive half and is worth nothing if
  increment 1 says the route is unreachable. Shape is already fixed by increment
  1 (same stage, same callback), so nothing relocates. Increment 1's receipt
  records `reversed_z` and `sample_count` precisely because the pipeline state
  must match the scene pass exactly.
- [x] **Alwd bisect run 1 — `WW_LWOOD_DRAW=10` — DONE, FAIL @ n=3, ATTRIBUTABLE.**
  `zeroed=2 mats=2 bytes_was=896 armed=1`; mode-1 control clean the same minute.
  SharedDL content is not the trigger.
- [x] **Alwd bisect run 2 — `WW_LWOOD_DRAW=11` — DONE, FAIL, arm proven** (`pkts_total:8`,
  `unarmed_calls:0`). Named its cause: `aurora::gx::fifo buffer size overflow`.
- [x] **Route lwood through `daBg`'s draw path — mode 12 RAN AND FAILED** (`submit_fn:"entry"` proved the receiver path really ran; the frame-interp multiplier is not the mechanism) — Room44's `model.bdl`
  is the same SharedDL class and survives every frame through the receiver's own
  submitter; lwood goes through the plugin's `s_fnModelUpdateDL`. Same class, same
  backend, opposite fate. This is the next test, and it is code, not a gate.
- [x] **Link WHITE — the k0 lead: FALSIFIED from receipts already on disk.** The
  `actor_leg` receipt records the post-overlay values at the live site, and across
  two runs it is unanimous: **`type:10` (player leg), `k0:[0,0,0]`, `c0:[0,0,0]`,
  `amb:[172,133,97]`, `lm:1`** — sampled every 300 calls to n=900+. **k0's pure
  white never reaches Link.** Two supports: the `settingTevStruct` hook is
  **NOT-INSTALLED** on the stock binary (`hook_manifest`), so the type-0 branch is
  reached only by plugin-side WW actor calls, never by Link; and the live path is
  `setLightTevColorType_MAJI` (INSTALLED), where `t` comes from `tevstr->Type` = 10.
- [~] **Is Link still white at all?** *(blocked: only the user can see the screen; asked)* The runtime values are sane, so the handoff's
  *"Remaining: Link WHITE after Pale overlay"* may be STALE — the fix its own code
  comment describes (`c0` into `TevColor` blew Link white) is in place. **A metric
  cannot answer this; the user can.**
- [~] **`Pale[3]` from the disc — RE-TESTED, genuinely blocked.** Cannot be derived
  from runtime: AmbCol is `c0 x actR` and the ratio is in no receipt (observed
  spread 172,133,97 / 171,132,96 / 132,102,74 proves it varies). Handed them a
  falsifiable bound instead: `Pale[3].actor_c0 >= (172,133,97)`.
  *(blocked on History/Bridge — this time after checking)* *(was: awaiting that lane)* They measured `Pale[2]` = 156,140,134,
  but the runtime selects **`pale:[3,3]`** — entry **3**, not 2. Their number is
  right for the wrong row; `amb` reads 172,133,97 here.
- [x] **FORK-STRIP LEDGER — partial-row cut confirmation DELIVERED** (whole-function cuts verified stock-absent; in-function arms are all one `isSkyHost` if/else shape; `drawWave` flagged NOT strippable). Remaining ledger work is Foundry's lint hook and the strip commits, which wait on the milestone commit.
- [~] **Grass — RE-SCOPED BY MEASUREMENT + DN-11.** Plugin-side, `fpcNm_GRASS_e`
  in the WW stage = **ZERO** (all 118 are pre-warp), so the product is compliant
  **by absence** — DN-11 prefers a missing prop to a foreign one. The port target
  is the fork's own `d_a_ext_vegetation.cpp` (donor `l_Oba_kusa_a_cutDL`, 112/187
  placements). **Do not strip that TU** — it is the only DN-11-compliant grass
  server until the plugin has a counterpart. What the user actually saw is still
  unidentified: the only receiver-native actors measured in-stage are 4×`WMARK_e`
  and 1×`WPILLAR_e`. *(blocked: needs the user's eye; H11 no longer required for this)*
- [~] *(superseded)* TP-native placement — grass *(was blocked on H11: the receiver logs creates but NOT deletes, so persistence is unmeasurable until the probe build lands)*, named by the user in the frame
  confirmation. Receiver content still standing on a WW island.

- [~] **Mode 15 — the TEN-HYPOTHESIS probe set (H1–H10) — BUILT AND SHIPPING (md5 29275b11).**
  Build is INTEGRATOR's lane (corrected 2026-08-21; this lane had been building
  its own probes all session). H9 is the control leg: the surviving room model
  measured with the same fields. Blocked on the build, and Integrator's watcher
  is down.

- [~] **LWOOD IS OPEN — acceptance is a USER-CONFIRMED DRAW on a non-crashed
  Outset** (user ruling 2026-08-21). No mechanism gets called a root cause before
  that. Blocked on Integrator's build of the H1–H11 probe set.

- [~] **Animation arm at the getObjectRes choke — `wwParseAnimOnce` WRITTEN AND
  WIRED, unbuilt.** Seven-type family via the receiver's own loader; BCK named
  as unsupported at runtime (its `mDoExt_transAnmBas` does not resolve).
  **BUILT — shipping in md5 29275b11. Not blocked.**

- [~] **Probe legs H1 + H2 repaired** — H1 never existed, H2 was an identity field
  rather than a double-entry counter. Both now keyed on the receiver's sim tick
  (`g_Counter.mCounter0`), arity-checked, placebo field removed.
  **BUILT — shipping in md5 29275b11. Not blocked.**

- [x] **H6 identity — DONE. 43/43 own-block deltas = +0x68 (descending), and 80/80
  stale links are MAT-CORRESPONDING, zero crossed. The `-0x68` is fully explained;
  the 'crossed mapping' branch is withdrawn.**
- [x] ~~**H6 — CONFIRM THE CYCLE BY IDENTITY, not by address containment.**~~ The
  topology (0→2→1→0, model[3] dangling in) is resolved from stale pointers in
  log 154613. What is inferred is which instance OWNS each target address; a run
  with `WW_LWOOD_CENSUS=1` names them outright. **Not blocked — this lane simply
  had not tried the data it already had.**
- [~] **Purge question — NARROWED TO ONE UNMEASURED CALL, then genuinely blocked.**
  Tried the data first: `creq_create cmd=2` shows the scene clear ARMED and
  completing on both sides of the warp, so "the clear never fired" is falsified.
  What no receipt covers is whether `fpcBs_Delete` reached the 118 `GRASS_e` /
  11 `ITEM_e` — the receiver has no delete log line at all. H11's counter answers
  it in one boot. *(blocked on Integrator's build — this time after checking)*

- [x] **Mode 0 — RAN (boot 170646), `tev:1`, fork-identical draw — STILL DIES at
  draw 44 with the same rings. THE DRAW SHAPE IS FULLY ELIMINATED.** The remaining
  variable is the host: nine fork-only aurora files never reached stock aurora,
  all display-list/FIFO/command-processor code (`fifo.cpp` +90 among them) and the
  crash is a FIFO overflow. **Integrator's lead, not this lane's.**
- [x] **`finish_toon` — BUILT, RAN (boot 171434), FALSIFIED.** Gate engaged,
  `setToonTex` receipts present, vegetation still black. The toon-texture
  explanation is dead.
- [x] **BG AmbCol lever — WITHDRAWN (defaulted OFF).** `applyBgOverlay` writes
  TevColor/TevKColor and never touches AmbCol; AmbCol is the ACTOR path. The lever
  would have written undefined data into a field BG draws do not read.
- [~] **`bg_overlay` receipt — authored, unbuilt** *(blocked: Integrator's build)*. Logs
  what the BG leg actually writes per bg index; History/Bridge read `Pale[3].bg3_c0`
  = (0,0,0) from disc and 36/101 draws are bg3 (verified from the run). `dKyWw_tryWwMaji`'s BG branch never
  called `writeAmbCol`; only the player leg did. daBg-drawn room models consume
  AmbCol the same way, and stage `sea` has no receiver kankyo data — unlit = black.
  Gated `WW_BG_AMBCOL=0`; `bg_ambcol` receipt carries the colour written.
- [x] **Port shape question — CLOSED.** The fork's `d_a_ext_vegetation.cpp` already
  ported the donor's packet system (grass + flower packets, `mGrassAnm[104]` /
  `mAnm[72]` anim pools, donor `calc` offsets, raw-GX geometry) and references the
  swood tree packet. noclip and the fork agree; the gap is a PLUGIN counterpart,
  not unknown shape. Note: the fork's `d_a_lwood` does NOT use a packet and works
  anyway — so this is a faithfulness question, not the crash.
- [~] **`ITEM_e` (rupees) never deleted — 11 created, 0 deleted** *(blocked: needs the
  `first_at`/`last_at` ordering from the pending build; the capped receipts cannot
  answer it and I already burned one vacuous test proving that)*. Separate
  defect from grass's partial 100/118; nothing has looked at it yet.

## Blocked on the user (not this lane's to unblock)

- [~] Integrator watcher — a stand-down order and a later re-arm order contradict.
