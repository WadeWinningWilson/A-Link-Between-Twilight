# Wind Curs ↔ Wind Clau

**Interconnected chat doc** — WW `itemmdl` Track A (get-item bow pilot) ↔ Cursor implementation ↔ Claude review.

| | |
|---|---|
| **Cursor (Wind Curs)** | In-repo edits, build/launch loop, surgical wiring, handoff doc updates |
| **Claude (Wind Clau)** | Review roadmap, graphics strategy, Plan A vs Plan B, second opinion before big changes |
| **Canonical detail** | [`docs/wind-waker-item-work.md`](../wind-waker-item-work.md) — mesh indices, TWW rows, cut-enemy notes |
| **Status (2026-07-01)** | **4E committed** — whole bow + struct-0/no MAJI + efplight/beam skip + warm ambient polish. Next = replay vs WW ref, SC A/B if tips bloom, BTK after sign-off. |

---

## ▶ ASSET SIDE — Iron Boots WW re-rig DONE (2026-07-02)

**New track (Blender/asset, not draw-time skin):** Iron Boots are a **worn** model (`al_bootsh`, foot-bone-driven joints 1/2/3), so WW `vboot` was **re-rigged in Blender** to al_bootsh's skeleton. **Result: `vboot.bdl` built & validated** — 4 joints (0=root,1/2/3=A/B/C matching al_bootsh), `SC_boot`/`boot` materials, `V_boot.png`. Files: `D:\XXXXXXX\Ex TP\Blender workflow\DAE files\vboot.bdl` + `vboot_rerig_DONE.blend`. Full method: **[Blender-WW-Items.md → Task 1 DONE](../Blender-WW-Items.md)**.

**Hard-won facts (don't re-derive):**
- The Cowork/epitaxy **Claude Desktop build ignores `claude_desktop_config.json` mcpServers** — no hammer, and it rewrites that file on launch (read-only lock didn't help). **Drive Blender via the addon's `localhost:9876` socket directly** (Python `socket` → `execute_code`/`get_scene_info`/`get_viewport_screenshot`). This works; don't chase the hammer.
- **al_bootsh bones are runtime-driven** (all identity at origin) ⇒ Auto-Weights useless; **region-weight** each vboot vert to the nearest al_bootsh segment's bone (rigid). The **mesh** is still a valid alignment ref (segments spatially assembled).
- Align: rotate **90° Z** (vboot long axis Y → al_bootsh X), **uniform** scale ≈1.64, center-match. WW boot is chunkier/overhangs — fine, must still read WW.
- **UV-join beige trap:** joining vboot's 2 meshes → 2 UV layers, faces valid in only one → beige where they sample the empty one. Merge into one `tex0` layer (copy mat-1 faces' UVs over), re-export. Bug was in the BDL too.
- SuperBMD: armature must be named **`skeleton_root`**; **`--bdl`** flag (default is .bmd); `delete_loose`+`dissolve_degenerate` first; **rename materials `SC_boot`/`boot`** (strip `m0/m1` import prefixes — cel-shader keys on `SC_` at start).

**Design = TP↔WW Iron Boots toggle** (drop-in, same rig). **Game-side remaining (Cursor):** repack `vboot.bdl`→`itemmdl.arc`→`res/Object/`, wipe caches, Item-Viewer check (0xE), foot test, then **remove `s_albwWwBootsSkinned` rig-gating** in `d_a_alink*` and gate model choice (vboot vs al_bootsh) on the toggle flag so vanilla `setAnmMtx(1/2/3)` drives it.

---

## ▶ RESUME HERE (fresh-chat handoff — 2026-07-01)

**One-line state:** **4E committed** — body `(105,78,48)` cap **80**, SC ink `(58,48,42)`, skip **efplight** + demo **GETITEM beams**, SC A/B via Editor **`game.wwItemmdlBowScSuppress`** (default **off**). struct-0 + 2B‴ + 2N′. **No MAJI. No struct 14.**

**Active draw path:** 2N′ + persistent 2B‴ + struct **0** + **4E ambient-only** + skip `dKy_efplight_set` + skip demo GETITEM beams (`d_demo.cpp`) + `dComIfGd_setList()`.

**Git (main, bow get-item):** `e0374888ed` 2N′ · `ace05ce875` 2B‴ · `72a2f01194` 4B · `6023333a8b` 4D · `d4f7066637` 4E.

**Cursor must NOT retry:** per-shape double `modelUpdateDL`, per-shape dump at load, re-bake every spawn, per-flush callback clear, MAJI for brightness, struct 14, % room ambient alone (4C).

**Fresh chat opener (paste):**
> Continue WW `itemmdl` Hero's Bow get-item (Track A). Read **`docs/wind-waker-item-work.md`** + this doc **▶ RESUME HERE**. Pipeline is locked: 2N′ + 2B‴ + struct-0 + ambient-only (4E). Do not re-enable MAJI or struct 14. Next: compare close-up + cave replay to WW reference; if tips still #FFF bloom, flip `kWwBowSuppressScInkPassForDraw` for SC A/B; after color sign-off enable BTK spin then Track B held bow.

**Acceptance vs WW reference (still open):**

| Check | State |
|-------|--------|
| Whole mesh + nocked arrow | ✅ locked |
| Orange direction (not lemon) | ⚠️ hue right but **body too bright + flat** vs ref amber gradient — cap `80→70`, opt. Vbow_v TEV replay |
| SC detail present | ✅ restored by 4a SC TEV replay |
| Silver caps (not white bloom) | ❌ caps read **white**, ref = matte **silver/pewter** — temper st[2] `+HALF` (4b, warranted) |
| Cave full pose (no wash) | Beams skipped in 4E — geometry reads clean; brightness still hot |

**Next knobs — color at NEAR-SIGN-OFF after 4a (see `▶ SC_Vbow_v is a geometry+TEV pass` below). Summary:**
1. **SC A/B + dump + 4a all resolved:** `SC_Vbow_v` = real geometry (never ship suppress ON); blend = BM_NONE (not additive); **4a authentic TEV replay = ROOT FIX**, restored detail, generalizes to the other 20.
2. **Commit 4a** (+ SC A/B toggle wiring as a separate commit). Update canonical progress table "WW cel look" → Done.
3. **Optional 4b micro-matte** — only if tips still too hot: temper st[2] `+HALF` (single tunable, logged). Not required; teal cast on caps is correct.
4. **Body (`Vbow_v`) = SIGNED OFF.**
5. **Color signed off →** BTK `0x24` (verify texmtx-vs-texgen + 2B‴ re-applies after BTK each frame); then Track B `d_a_alink_bow.inc`; then generalize to 20.

**Log hygiene:** `%USERPROFILE%\Documents\dusklight\albw_ww_itemmdl_debug.txt` — once per session: `4E ambient-only: body=… scSuppress=… beams=1`; after first SC draw also `2J sc-dump blend: …`, `2J sc-dump kColor[…]`, `2J sc-dump st[…] colorIn …`. After gfx rebuild wipe **both** `%AppData%\TwilitRealm\Dusklight\dawn_cache.db*` and `pipeline_cache.db*`.

**Hard-won facts a new chat must not re-derive (all confirmed):**
- The long "flaky crash" was a **CRT fast-fail `0xC0000409`** = **unhandled `std::out_of_range`** from `std::bitset<8>::set()` in Aurora `shader_info.cpp` `color_arg_reg_info` — a TEV stage read a texture with **`texMapId = GX_TEXMAP_NULL (0xFF)`**; `CHECK` is a **no-op under `NDEBUG`** so it wasn't caught. **Fixed by A+A′** (guard the `.set()` in `shader_info.cpp` + the WGSL codegen in `shader.cpp`; skip unbound/out-of-range texmaps). Do **not** revert A/A′.
- Colorless bow = same unbound-texmap → Aurora emits `vec4f(0.0)`. **Fix B** (per-draw `GXSetTevOrder` from the *valid* material struct) restored `Vbow_v` color. The material **struct TevOrder is VALID** (Vbow_v texMap 1/0; SC_Vbow_v texMap 2/0/1) — **not** a struct-repair job; the locked baked-DL just wasn't realizing it into Aurora's pipeline.
- Aurora GX is a **deferred fifo** — `g_gxState` snapshot at draw ≠ what the shader builds from at drain; explains angle/room-dependent color and the `numTev=1/255` snapshots. Trust visuals + `2B apply` log over pre-drain snapshots.
- **`setLightTevColorType` (non-MAJI) is an empty stub on PC** — only `_MAJI` works. `settingTevStruct(14)` = get-item dark-ambient + **view-matrix lighting** = the outdoor angle swing. struct-0 attempts (3.1/3.1′) **regressed** — reverted.
- Earlier `[FATAL] unhandled tcg src 21` = `GX_MAX_TEXGENSRC` sentinel (numTexGens/texgen mismatch); addressed by **2N′** locked-DL bake + `GXSetNumTexGens` bind.
- **Image 2 in chat = the COMPLETE WW Hero's Bow (with nocked arrow), NOT the TP bow.** The arrow is real geometry that must draw.
- **`shapeNum=2`, `matNum=2`, `jointNum=2`** — no third-shape inventory gap; per-shape dump at load **AVs** (removed; summary only).

**Deep sections:** search this doc for `ROOT CAUSE FOUND`, `Fix A′ (approved)`, `Fix B step 1 — Wind Clau read`, `Fix B bind SUCCEEDED`, `REVERT the struct-0 delegate`, `Interconnected pass`, `First clean playtest`.

---

## ▶ NEXT ACTIONS & ROLE SPLIT (2026-07-01 — color final passes)

**Roles:** 🟩 **Cursor** = implements/builds source · 🟦 **User** = playtest / screenshot / commit-call · 🟪 **Wind Clau** = review + doc, no source edits. Details: [`▶ SC_Vbow_v is a geometry+TEV pass`](#-interconnected-pass--sc_vbow_v-is-a-geometrytev-pass-not-ink-ambient-2026-07-01).

**COMMIT GATE (do first)** — 🟩 Cursor implements, 🟦 user approves:
- Commit **4a** (SC full-TEV replay) as its own commit.
- Commit the **SC A/B toggle wiring** (`wwItemmdlBowScSuppress`: `settings.h`/`settings.cpp`, `editor.cpp`, `d_ww_itemmdl_pc.cpp` runtime toggle) as a **separate** commit.
- *Why:* three sessions of pipeline work are uncommitted — land before stacking color tweaks (commit-discipline rule).

**STEP 1 — SC caps: white → matte silver** — 🟩 Cursor (one build):

**❌ 1st attempt (st[2] +HALF temper) FAILED (2026-07-01):** caps still white/hot, detail present. ⇒ tone ceiling is **not** st[2]; it's set by an **earlier stage's konst**. Wind Clau miss — st[2] only adds on top of an already-white value.

**✅ Correct lever — scale K0 konst (do this):** `st[0] = lerp(reg0=0.5, KONST=K0, TEXC)` with **K0 = white (255)** → caps clip toward white. 4a proved the texture samples (cap detail visible) + st[2] temper failed ⇒ the ceiling is K0.
1. **Scale `kColor[0]` (K0) from `(255,255,255)` toward `~(160,160,160)`** — clamped 0–255. K0 feeds st[0] (B=KONST) and st[1] via `kSel=12 (KCSEL_K0)`. Pulls cap output ~0.8→~0.58 (silver), robust to exact texel. Single tunable + log.
2. **Leave `kColor[2]` alone** (`a=50 kSel=30` = OpaTexEdge alpha threshold, not color). Revert/keep the st[2] temper small — K0 is dominant.
3. Keep teal cap/nock bands. Also kills the string bloom. **STOP + screenshot** before body.

**✅ K0=160→150 WORKED (2026-07-01):** caps resolve to silver, body gradient good ≈ ref. **Remaining gap = engine BLOOM on near-white string/nock**, not tone: string/nock texels ≈ 1.0 so K0's lerp can't pull them below the bloom threshold; their glow **bleeds onto adjacent caps** (why some caps look un-silver = spatial bloom bleed, not material). Body ≈ **match — Step 2 likely skippable**.

**➡️ NEXT = 4b output ceiling (do NOT lower K0 further — would muddy good caps before taming string):** add **one SC-pass output clamp** — each channel ≤ ~**185–190** (or scale final output ~0.75), keep K0=150, single tunable + log. Leaves silver caps intact; pulls string/nock **below bloom threshold** → glow dies, string reads thin+matte like ref (half its apparent thickness is the halo). This is the reserved 4b attenuation, now correctly aimed at glow not tone; it's the per-item scalar for Phase 6.
- 🟩 Cursor: also confirm the Step-1 op-replay did **not** cause the crash (see crash note below); clamp K0 so Aurora never gets an out-of-range konst.
- 🟦 User: send crash log tail + when it hit; then replay K0 build, wipe **both** GPU caches, screenshot → Wind Clau.
- 🟪 Wind Clau: classify crash, judge caps vs ref, adjust K0 target.

**⚠️ CRASH (2026-07-01):** a crash accompanied the Step-1 build. **Classify before more tuning** — TEV-op/konst replay feeds the same Aurora material-config path as the original `tcg src 21` shader abort. Need: *when* (replay/load/spin/random) + last ~15 lines of `albw_ww_itemmdl_debug.txt` + Windows exit code. If Step-1 op replay destabilized it, that outranks tone.

**STEP 2 — body exposure** — 🟩 Cursor (only after Step 1 signs off):
1. Body cap `80→70` (or ambient `(105,78,48)→(95,70,44)`). Cheapest first.
2. Only if still flat: **generalize 4a full-TEV replay to `Vbow_v`** for amber gradient — **A/B for regression** (body already reads colored).
- 🟦 User: replay + screenshot. 🟪 Wind Clau: call **match** or next knob.

**AFTER COLOR MATCH:** 🟪 Wind Clau hands 🟩 Cursor the **BTK `0x24`** checklist (texmtx-vs-texgen decode + 2B‴ re-apply-after-BTK each frame); then Track B; then generalize to the other 20.

**DO-NOT (all roles):** ship suppress ON · darken SC toward brown · re-enable MAJI / struct-14 / % room ambient · change more than one knob per build.

**✅ COMMITTED (2026-07-01):** `155e41f392` 4b source (K0 cap 150 + output ceiling 185 + full op decode + editor tunables); `96d6f8ea90` docs. Get-item color = **good-for-now** (user viewing at ceiling **255 = glow ON**; committed default **185 = matte**; get-item-default glow-vs-matte is a pending user call). **Not yet pushed** to upstream.

---

## ▶ TRACK B KICKOFF — held bow "skin" (Phase 5, 2026-07-01)

**Goal:** WW `vbow` as a toggle skin for Link's **held** bow (aim/shoot), not just get-item. User plan (correct): setting toggle first, then scale/position to TP `AL_BOW` size. Expect **stiff-but-functional** (display ≠ behavior).

**⚠️ Reality check — held is a DIFFERENT draw + lighting path than get-item.** All our color work (2B‴ per-mat bind + 4a SC TEV replay + K0/ceiling) lives in the **get-item draw scope owned by the demo-item actor**. Held bow draws every frame as part of Link's model. So:

**STEP 1 ✅ ANSWERED (2026-07-01, Wind Clau solo) — BEST CASE:** the SC realization (`applyScAuthenticTevAndPeFromMaterial` + K0 + ceiling) runs **inside the global `wwBowMatDrawPostDl` callback** (`d_ww_itemmdl_pc.cpp:1362`), gated only by `isVbowDrawMaterial`/`isScVbowDrawMaterial` vs `s_wwBowDrawModelData` — **not** tied to the demo actor's `drawWwBowModel`. So the color pipeline is **model-data-driven + global** → **held reuses it with ZERO re-plumbing**, just by setting the scope on `mHeldItemModel`. Held bow = `mHeldItemModel`, loaded in `setBowModel()` (`d_a_alink_bow.inc:592`) via `initModel(loadAramBmd(AL_BOW))` → standard J3D draw path → callback fires.
**Remaining risks:** (a) BCK coupling — `setBowModel` binds `BVJMPCL` and derives `field_0x33dc` from `getBckAnm()->getFrameMax()`; BVJMPCL targets AL_BOW bones (can't drive 2-bone vbow) so skip binding but still satisfy `field_0x33dc`; (b) single-static scope conflict (bow get-item *while* holding WW bow — rare; `begin()` logs "begin replaced"); (c) scope lifetime — begin/clear around Link's held-item draw each frame so the global per-material filter doesn't run game-wide (pin the held-draw site).

**STEP 2 — swap + toggle (🟩 Cursor):** `game.wwItemmdlHeldBow` branch in `setBowModel()` (`d_a_alink_bow.inc`) → load `itemmdl` vbow instead of `AL_BOW`.

**STEP 3 — skip TP bow BCK, default on (🟩 Cursor):** `game.wwItemmdlHeldBowNoBck`. TP `BVJMPCL` targets `AL_BOW` bones; our 2-bone vbow would deform to garbage. Skip = stiff (no draw-flex) but aim/shoot works.

**STEP 4 — scale + hand matrix (🟩 Cursor + 🟦 user screenshot-tune):** vbow is WW-scale; start `transM(-1.3,0,-3)`, `XYZrotM(-74°,43.6°,1.9°)` in `d_a_alink.cpp ~6078`. Include Epona / left-hand path.

**STEP 5 — held lighting re-tune (🟪 Wind Clau judges):** held uses normal gameplay env light, **not** demo ambient/struct-0. The **material** realization (4a/K0) carries; the get-item-specific bits (fixed warm ambient, beam/efplight skip, struct-0) do NOT. For held, **drop the SC output ceiling to ~185 (or lower)** — a held bow should NOT glow. Re-judge exposure vs a held-bow reference.

**✅ STEP 2–4 BUILT (2026-07-01, Wind Clau solo; UNCOMMITTED, awaiting playtest):** `wwItemmdlHeldBow` + `wwItemmdlHeldBowScalePct` settings + editor controls; `setBowModel()` loads vbow (fallback AL_BOW); per-tick private-arc mount in `d_s_play` (`dWwItemmdl_tickHeldBowArcMount`) so held works without a get-item replay; scope-wrapped both `modelDraw(mHeldItemModel)` sites in `d_a_alink.cpp`; BVJMPCL entry skipped on vbow (stiff); live scale via `setBaseScale`. Build OK (30MB exe), caches wiped. Glow reuses the K0/ceiling sliders. **Pending:** playtest (appears? crash? scale?), then tune scale/hand-offset, then commit. **Left-hand/Epona path (`:20846`) wrapped too but untested.**

**▶ HELD BODY BLOCKER + PATH DECISION (2026-07-01):** Held bow loads / scales / draws; SC arrowhead colors; **body renders untextured + angle-swings black↔white** — the get-item "monochrome + view-matrix swing" refight, but now through Link's `daAlink_c::modelDraw` (which runs MAJI + chan-ctrl + material re-processing the clean demo actor never did). Tried: no-MAJI struct-0/ambient branch inside modelDraw; `mDoExt_modelUpdateDL` instead of `modelEntryDL` (held draw was param1=0 → modelEntryDL, no re-bake); get-item create flags (moot — `initModel` already ORs diff `0x11000084` at `d_a_alink.cpp:4331`; the black→white change came from mdlFlags 0→0x80000). Callback **does** fire for the body (`2B apply post-dl: mat=Vbow_v nTexGen=2 nTev=2`), so texgens bind — yet body stays untextured.

**DECISION (user, 2026-07-01): Path 1 — fix inside `modelDraw`** (not re-architecture, not pause). **Rationale: once the held bow renders correctly it is the implementation TEMPLATE for held versions of all 21 WW `itemmdl` items** — so it's worth the grind. Continue.

**Next diagnostic:** throttled callback now logs `mat=Vbow_v … tg0src=N`; need a run of the **current** build (10:11 exe; earlier test ran a stale exe — log had no `tg0src`) to read the body texgen source → decides texgen-source fix vs deeper Link-draw realization issue.

**✅✅ RESOLVED — HELD BOW FULLY COLORED (2026-07-01, build 23:02):** root cause was **scope lifetime, not color**. The `MatDrawPostDl` callback fires at draw-buffer **DRAIN** (after `modelDraw` returns) — but the held branch did `begin → modelUpdateDL → clear` all inside `modelDraw`, clearing the scope before the drain, so the callback **never ran** (no `2B apply` lines in the log despite `TrackB modelDraw` firing). SC showed as unrealized white-bloom, body untextured. **Fix: keep the scope active past `modelDraw`** — `beginBowDrawScope` re-points each frame, `clearBowDrawScope` only when the held item is no longer the WW bow (avoids dangling on item-switch). Gold body + silver caps + colored arrow now render on the held bow, matching get-item.

**★ THE HELD TEMPLATE — reusable for held versions of all 21 WW `itemmdl` items. Working chain:**
1. `setBowModel()` (`d_a_alink_bow.inc`): load via `dWwItemmdl_getVbowModelData("itemmdl")` + `patchModelForPc`; create with **`initModel(data, 0x80000, 0x11000084)`** (mdlFlags `0x80000` matters; `initModel` already ORs diff `0x11000084`).
2. `dWwItemmdl_tickHeldBowArcMount()` per play-tick (`d_s_play`) so the private itemmdl arc stays resident (no get-item replay needed).
3. `daAlink_c::modelDraw` branch for the WW model: **skip MAJI**; `g_env_light.settingTevStruct(0)` + `dWwItemmdl_setWwBowActorAmbient` + `dWwItemmdl_applyBowMaterialAmbientOnly` + **`mDoExt_modelUpdateDL`** (NOT `modelEntryDL`).
4. **Draw scope must stay active to DRAIN**: `beginBowDrawScope` (re-point each frame), clear only on item-switch — this is what makes the SC/K0/texgen realization callback fire.
5. Skip the item's BCK when it targets a different rig (stiff but functional); expose a live scale-% slider; glow reuses the shared K0 / SC-output-ceiling sliders.
Generalizing to another item = swap the item-id / material-name guards; the render chain (steps 2–4) is item-agnostic.

**Open (held polish, not blockers):**
- **Double arrow / jut:** vbow has a permanently-nocked arrow baked into its 2 shapes (`Vbow_v` shaft + `SC_Vbow_v` head/fletch); TP still spawns its own `daArrow_c` over it, and the vbow arrow sits at a fixed angle (BCK skipped = no aim flex). The WW arrow is **not separable** from the bow model, so it can't be moved onto the TP arrow. Options: hide TP's nocked/held arrow during draw (show only the vbow's), or skin `daArrow_c` with a standalone WW arrow mesh (own mini-application of this template, if such an asset exists).
- Scale / hand-position tuning.
- **Strip diagnostics** (`dWwItemmdl_logHeldBowDraw`, throttled `2B` `tg0src` log) and **commit** the held-bow template.

**Recommendation (deferred):** BTK `0x24` (get-item spin polish) and the 20-item generalization wait until held is cracked. Wind Clau BTK caveat still stands = confirm BTK animates texmtx (safe) not texgen src, and 2B‴ re-applies after BTK each frame.

---

## ▶ VIEWER RESULTS + NEXT SKINS + KING BULBLIN AXE (2026-07-01/02)

**✅ Viewer works for ALL 21 itemmdl meshes.** `game.wwItemmdlViewerBdlIndex` + the Replay button spins any BDL index through the proven get-item path — the generalized render core (per-index loader + `SC_`-prefix / accept-all guards) holds across the whole set. Findings (user):
- **Color-perfect, ≤ bow brightness:** **Iron Boots** (`vboot` 0xE), **Hookshot** (`vhook` 0x14) — read correct, not over-bright (some items land better than the bow).
- **Colored but angle-bright:** several swing nice-color ↔ washed-white monochrome at certain angles (view-lit on bright texels; the familiar per-item ceiling/ambient axis).
- **~1 off-color** item (identify which on the next pass).
⇒ Render core proven for all 21; remaining is **per-item** color tuning, not architecture.

**Next skin pass (both TP-hosted → held works via the template):**
- **Iron Boots** `vboot` 0xE → `dItemNo_HVY_BOOTS_e`; **Hookshot** `vhook` 0x14 → `dItemNo_HOOKSHOT_e`. Both already color-perfect in the viewer. Apply the held template to their model setters (analog of `setBowModel`) + the `modelDraw` no-MAJI branch keyed to those models; per-item scale/offset.

**King Bulblin axe — weapon-reskin thread (research done, NO code):**
- **Enemy:** King Bulblin = **`E_rdb`** (id `0x1D5`/469). (`E_rd` `0x1D4` regular rider, `E_rdy` `0x1D6` shadow — shared 32-bone `RD` skeleton.)
- **Weapon:** **`RB_ONO`** = `dRes_INDEX_E_RDB_BMD_RB_ONO_e = 0x55` in the `E_rdb` arc. "ONO"=斧 (axe); **single-joint prop** (`RB_ONO_JNT_RB_ONO_e=0x0`); swung via `RB_ATTACK01/02/03` + `ATTACK03_SPIN` (confirmed by `Z2SE_EN_RDB_DEMO_ONO_LAND`).
- **Feasibility:** **Phase 1 — Link wields it as a reskin over the sword** (`mSwordModel`, same `modelDraw` path) = **feasible**, same held template; Link swings the axe with his own sword moveset (stiff-but-functional, big-weapon scale tune). **Phase 2 — authentic Bulblin swings = NOT portable** (his attacks are on the 32-bone `RD` rig; Link's skeleton differs). **Phase 3 — unique moveset = new procs.**
- **Loading:** `RB_ONO` is inside the big `E_rdb` boss arc → private-mount `E_rdb` (heavy, whole boss resident) or extract `RB_ONO` to a light arc (Plan B, cleaner).
- **Integration (user's thoughts):** either graft into the **quick-switch sword system**, or a **standalone item/sword slot** if vanilla has unused slots (worth auditing slot availability). **Open question:** repurpose other item anims (**Dominion Rod**) or **cut Link animations** for a custom swing — decomp scan when this thread is picked up.

**STATUS: WAIT — no code changes pending user direction. Queued: (1) per-item color tuning, (2) Iron Boots + Hookshot held skins, (3) King Bulblin axe Phase-1 reskin.**

---

## ▶ IRON BOOTS SKIN — path traced + plan (2026-07-02, research)

**Visual path (not `setLinkBootsType` — that's audio):**
- **Model:** `al_bootsH.bmd` (heavy boots), from Link's clothes arc (`mArcName`). Built in the clothes builder `d_a_alink_wolf.inc:647–650` as **two** instances → `mpLinkBootModels[0]` (L) + `[1]` (R).
- **Rigging (every frame):** `d_a_alink.cpp:6205–6229` — `setBaseTRMtx(link base)` + `setAnmMtx(1/2/3, mpLinkModel->getAnmMtx(0x13–0x1A))` (foot/leg bones). Multi-joint, follows the foot.
- **Draw:** via `modelDraw(mpLinkBootModels[i])` (+ shadow `d_a_alink.cpp:20303`, swindow `basicModelDraw`).
- **Vanilla feet:** `field_0x06e0/6e4` shapes hidden when heavy boots equip (`setHeavyBoots`).

**Skin plan (feasible = stiff static boot swap):**
1. **Swap** in the clothes builder: when `wwItemmdlHeldSkin == IronBoots` (+ itemmdl arc ready), load `dWwItemmdl_getItemmdlModelData(0xE)` (`vboot`) into `mpLinkBootModels[i]` instead of `al_bootsH`; **fallback** `al_bootsH` if arc not ready.
2. **MANDATORY guard** at `d_a_alink.cpp:6205–6229`: `vboot` has only **2 joints** (`ROOT`+`MODEL`) vs `al_bootsH` 3+ → the `setAnmMtx(2)`/`(3)` calls write **OOB → crash**. For the skinned case, position via `setBaseTRMtx`/`setAnmMtx(1)` at the ankle only; skip joints 2/3.
3. **Draw recipe:** extend the `modelDraw` WW branch to also fire for `i_model == mpLinkBootModels[0]/[1]` when skin==IronBoots (struct-0 + ambient + scope + `modelUpdateDL`) → colors like the get-item `vboot`.
4. **Reversibility:** gated on skin==IronBoots; Off ⇒ `al_bootsH` as normal.

**Caveats / risks:**
- **OOB `setAnmMtx` on 2-joint vboot = crash — guard is not optional.**
- **Stiff:** no foot articulation (vboot won't flex with the ankle); static boot at the foot.
- **Two feet, one mesh:** both models = `vboot`; L/R mirror/orientation may be off → tune.
- **Timing wrinkle:** boots are built in `changeLink` (clothes build), so the swap applies on the **next clothes rebuild** (outfit change / area reload), not instantly on selecting the skin — may want to force a rebuild when the setting flips.
- **Clothes epoch/lifecycle:** `mpLinkBootModels` are treated as clothes models (freed/rebuilt on arc epoch); vboot *data* lives in the retained heap (persists), the *instance* in the clothes heap (rebuilt) — should be fine but watch the epoch guard in `modelDraw`.

**Status: research done; implement when the held pass (bow/hookshot) is verified.**

---

## ▶ PLAN B — asset/rig workflow (add joints, re-rig, extract sub-meshes) (2026-07-02)

Code can't add joints or split baked geometry — that's external asset work. Workflow:
**unpack arc → edit/convert model → repack → `res/Object/` → test in the Item Viewer → wire the skin.**

**Tools:** BMDView2 (inspect joints/mats/weights), SuperBMD / J3DUltra (BMD↔DAE), Blender + J3D addon (joint add + weighting), GCFT/arctool (unpack/repack RARC `.arc`).

**Add joints to `vboot` (articulated iron boots, remove the stiff guard):**
1. Unpack `itemmdl.arc` → `vboot.bdl` (2 joints: `ROOT`+`MODEL`).
2. The boot rig wants joints **1/2/3** ← Link foot bones `0x13–0x15` (L) / `0x18–0x1A` (R). Reference `al_bootsH.bmd` (clothes arc `Bmdl`/`Kmdl`) for the ankle→foot→toe layout.
3. `vboot.bdl → DAE` (SuperBMD), add the joint chain at indices 1/2/3 in Blender, weight the boot mesh to them, rebuild → repack.
4. **Code:** drop the `s_albwWwBootsSkinned` OOB guard in `d_a_alink.cpp` (the full `setAnmMtx(1/2/3)` rig is now safe) → boots flex with the foot.

**Analogous asset work:**
- **Extract WW arrow from `vbow`** → standalone `warrow.bmd` (the arrow isn't a separable joint/shape in code) → skin `daArrow_c` via the held template.
- **Re-rig any held item to Link's hand bone** (weight a joint to the hand) → removes the "skip BCK" stiffness; vanilla item BCK then flexes it.
- **King Bulblin axe:** unpack `E_rdb.arc`, extract `RB_ONO` (BMD `0x55`) to a light arc (avoid mounting the whole boss) → feeds the sword-reskin path.
- Keep material names on the WW convention (`<name>` + `SC_<name>`) so the render recipe (`SC_`-prefix guard) applies to edited/new meshes automatically.

**Iron boots skin (stiff) IMPLEMENTED (2026-07-02, uncommitted pending playtest):** `mpLinkBootModels` ← `vboot` when skin==IronBoots (clothes builder); `setAnmMtx(2/3)` OOB guarded (pin to ankle via base matrix, right boot X-flipped); `modelDraw` WW branch extended to the boot models (both share cached vboot data → one scope); scale slider applies. Applies on next clothes rebuild (changeLink). Golden tint = the shared-tevStr warm-ambient effect (liked; recreate deliberately by setting warm ambient on a target model's tevStr before its MAJI draw).

---

## ▶ IRON BOOTS INVISIBILITY — ⛔ SUPERSEDED (2026-07-06) by the Custom Model API

**Resolution: the invisibility was an artifact of the wrong DELIVERY mechanism, not a fixable bug in this path.** The code-injection swap (load WW `vboot` into the boot slot + drive it via the vanilla rig) is retired. Deliver the re-rigged `vboot` via the **Custom Model API** (`docs/Custom-Model-API-Work.md`): a **loose-BMD (Layer B) / arc overlay (Layer A)** replacing `al_bootsH`, so the **vanilla clothes pipeline + foot rig** draw it with zero special-casing — no custom load, no `setAnmMtx` OOB, no invisibility. That also makes it the clean TP↔WW toggle the Blender doc intended. The re-rig asset is still correct — **export it as BMD** (not BDL) for the al_bootsH drop-in. The code swap is now `#if 0`'d in `d_a_alink_wolf.inc` (`c57f98b366`) — always vanilla al_bootsH, safe on any ISO. Analysis below kept as record.

## ▶ IRON BOOTS INVISIBILITY — investigation (2026-07-02, superseded)

**Symptom:** re-rigged `vboot.bdl` (4 joints matching al_bootsH: 0=root, 1/2/3 = A/B/C; 2 mats `SC_boot`/`boot`) loads fine (`vboot.bdl size 21696`) and renders in the Item Viewer — but with **skin=IronBoots + a clothes rebuild, Link's ENTIRE body goes invisible** (body/hat/hands/boots). World fine, **no crash**, runs 1200+ frames, **no `tcg src 21` / FATAL**. Skin Off + rebuild restores Link. ⇒ draw/state issue on the boot-slot path, not a load failure or crash.

**Known-good baseline:** commit `44ab0a2f1a` (stiff single-pair swap) — draws ONE boot via `setBaseTRMtx` (no `setAnmMtx`), WW recipe, Link **visible**. The new (invisible) attempt drives the 4-joint vboot through the vanilla foot rig: `setAnmMtx(1/2/3)` on **both** boots + calc/draw both + (now) the vanilla clothes draw (`modelEntryDL`+MAJI).

**✅ RULED OUT (proven in-code):**
- **Create-flag mismatch — NO.** `0x80000` = `J3DMdlFlag_DifferedDLBuffer` (`J3DModel.h:17`). Vanilla al_bootsH `initModel(modelData, 0)` → 2-arg overload (`d_a_alink.h:3767`) → `initModel(data, 0x80000, 0)`; `initModel` ORs `0x11000084` into diff (`d_a_alink.cpp:4331`) ⇒ **`create(data, 0x80000, 0x11000084)`**. WW vboot `initModel(wwBootData, 0x80000, 0x11000084)` ⇒ **identical create call.** al_bootsH uses the DifferedDLBuffer flag too.
- **`patchModelForPc` — benign** (log-only, `d_ww_itemmdl_pc.cpp:855`).
- **Shared model-data across both instances — normal** (vanilla al_bootsH shares its `modelData` across both `mpLinkBootModels` too).

**Reframe:** the code path is now ~identical to al_bootsH's. Remaining delta = **(1) `vboot` is a BDL (locked DL); al_bootsH is a BMD**, and **(2) the two things the visible stiff baseline never did** — `setAnmMtx(1/2/3)` on vboot, and calc/draw of a **2nd** instance sharing one BDL's data. The viewer renders vboot via `modelUpdateDL` and never exercises either.

**Two leading hypotheses (both ⇒ modelDraw "SKIP corrupt model" drops Link's body ⇒ invisible, no crash):**
- **H1 — two shared instances of a BDL locked-DL model conflict** (locked/baked DL lives in the shared data; two instances calc/draw over it → corruption). Stiff drew one → visible.
- **H2 — `setAnmMtx(2/3)` OOB** if the runtime vboot has < 4 usable anm-matrix slots (SuperBMD BDL joint/matrix layout ≠ al_bootsH BMD) → writes past the buffer → corrupts adjacent clothes `J3DModelData` → body `matNum` garbage → corrupt-model guard skips it.

**Decisive diagnostics (do first):**
1. **Which guard drops Link:** in `modelDraw`, when `i_model == mpLinkModel`, log SKIP stale-epoch (`s_albwClothesModelEpoch != s_albwArcEpoch`) vs SKIP corrupt (`matNum>256` / `getModelData()==NULL`) + `matNum` + epoch values.
2. **Runtime vboot joints:** after `initModel(wwBootData,…)`, log `getJointNum()` + `mMtxBuffer` anm capacity. <4 ⇒ H2.
3. **Bisects (1 line each):** (a) remove `setAnmMtx(1/2/3)` → visible? ⇒ H2. (b) re-add boot[1] draw/calc skip (draw one) → visible? ⇒ H1.

**Fixes by outcome:**
- **H1 / general BDL trouble → re-export `vboot` as `.bmd` not `.bdl` (RECOMMENDED, cleanest):** al_bootsH is BMD; a BMD vboot is a true drop-in (shared data + `modelEntryDL` + 2 instances all work like vanilla; dissolves the locked-DL class of bugs). Asset-side (SuperBMD BMD output) — add to `docs/Blender-WW-Items.md` Task 1.
- **H1 but must stay BDL:** per-instance model-data (each boot its own copy, not the cached shared `getItemmdlModelData`).
- **H2:** verify vboot runtime joints==4 (BMDView2 vs al_bootsH; check draw/weight-matrix count, not just joints); re-export or guard `setAnmMtx` to the real count.
- **stale-epoch:** vboot load during `changeLink` bumps `s_albwArcEpoch` after the stamp → mount the itemmdl arc before the clothes build (per-tick mount), or re-stamp `s_albwClothesModelEpoch` after the boot load.

**Lead:** expect **SKIP corrupt** + the boot[1]-skip bisect restoring Link (H1) → fix = **re-export vboot as BMD**. **STATUS: analysis done; Wind Clau to implement on user's go (holding).**

---

## ▶ IRON BOOTS via Custom Model API — Layer B IMPLEMENTED (2026-07-06)

**Confirmed (user):** Layer B (loose per-arc BMD via `custom_assets::try_load`), toggled by a **"Wind Waker Skins" folder** in the Custom Models picker, **all four outfit arcs**. This is how all WW skins will ship going forward.

**Implemented:** `d_a_alink_wolf.inc` boots load now = `modelData = dusk::custom_assets::try_load(mArcName, bootsHIdx)` → fallback `getObjectRes(mArcName, "al_bootsH.bmd")` → `initModel(modelData, 0)`. `bootsHIdx` = **0xD (Kmdl/Mmdl) / 0xC (Bmdl/Zmdl)**. The re-rigged 4-joint BMD rides the **vanilla foot rig** — no special-casing, `s_albwWwBootsSkinned` stays false (normal clothes draw). Old code-injection swap removed; `#include dusk/custom_assets.hpp` added to `d_a_alink.cpp`.

**Layer-B target files — drop into `%AppData%/TwilitRealm/Dusklight/model_replacements/Wind Waker Skins/`:**
| Outfit | Arc | al_bootsH idx | File |
|---|---|---|---|
| Default (Hero/green) | Kmdl | 0xD=13 | `Kmdl_13.bmd` |
| Casual | Bmdl | 0xC=12 | `Bmdl_12.bmd` |
| Zora armor | Zmdl | 0xC=12 | `Zmdl_12.bmd` |
| Magic armor | Mmdl | 0xD=13 | `Mmdl_13.bmd` |
Same re-rigged BMD, one copy per name (al_bootsH is the same mesh+skeleton across arcs). `Kmdl.arc` is YAZ0-compressed, but Layer B loads the **loose BMD** directly — no arc, no compression concern.

**Interaction research (mostly clean):**
- **Sumo:** boots load from base `mArcName` (NOT the sumo private composite — `wolf.inc:562` face is private, `:602` boots are not; `mArcName` stays the base clothes arc during sumo) → try_load fires the same → **sumo wears WW boots for free**.
- **Toggle tracking:** Layer B reads the loose BMD fresh (bypasses the §9 `getObjectRes` name-cache lag) → tracks when `changeLink` rebuilds; sumo coordinator's `forceReapply`→`changeLink` rebuilds → tracks. Native same-outfit toggle may lag one outfit-switch (§9), acceptable.
- **Build-integrity scan** (`albwFirstCorruptMat`, `wolf.inc:660`): a malformed WW boot BMD → Link skipped, **no crash** → the re-export MUST pass the material scan (verify in BMDView2).
- **Caps:** separate private-mount path, no direct conflict. Explicitly test **toggle-while-sumo+cap+WW-boots** (§5's cap calc-window is the fragile spot, not the boots).
- **Multiple mods:** Layer B (boot slot) wins over a Layer-A arc overlay; independent otherwise.

**Test matrix:** default + all 4 arcs; + sumo; + each Cap Wear mode; toggle on/off mid-session (± sumo); + another custom model. **Blocked on the re-rig exported as BMD + placed in the folder.**

---

## ▶ WIND CLAU HANDOFF (fresh review-chat — 2026-07-01)

**Role:** Wind Clau = review / graphics-strategy / second-opinion. Reads Cursor's diffs, logs, and screenshots; picks branches; enforces the do-not-retry list. **Does not edit source** — Cursor implements. (Wind Clau may edit *this doc* and `wind-waker-item-work.md`.)

**Read, in this order, to continue:**
1. **`docs/wind-waker-item-work.md`** — canonical: 21-mesh table, TWW/SS decomp refs, the color-fix order, Track A/B plan, git milestones. **This is the source of truth for current technical state — trust it + the git commits over any chat's memory.**
2. **This doc → `▶ RESUME HERE`** (above) — live pipeline state (4E), hard-won facts, do-not-retry list, next-knobs.
3. Deep sections (above list) only if re-deriving the crash history.

**Honesty note for the next Wind Clau:** the prior Wind Clau chat went behind on the 4C/4D/4E iterations (color-tuning commits landed faster than review turns). **Do not reconstruct state from a stale summary** — re-read `▶ RESUME HERE` + `wind-waker-item-work.md` Status + the git log each session.

**Decided / locked (do not reopen):** Plan A (retail `bdl3` + code-side TEV); pipeline = 2N′ bake-once + 2B‴ persistent per-material bind + struct-0 + ambient-only (**no MAJI, no struct 14**); crash fixes A+A′ (Aurora unbound-texmap guards) and Fix B (per-draw `GXSetTevOrder`); geometry complete (2 shapes/2 mats). See do-not-retry list in `▶ RESUME HERE`.

**Open (Cursor iterating, Wind Clau reviews):** final cel color vs WW reference — warmth, **cream tips (not `#FFF` bloom)**, subtle **grey-brown ink** (not heavy black), SC A/B toggle. Then BTK spin (`0x24`), then Track B held bow.

**Wind Clau's job on the next inputs:** judge replay screenshots against the WW reference (goldenrod/orange body with soft cel gradient, cream ornamental tips, nocked arrow, *subtle* ink — the get-item light beams are reward flair, now skipped for the WW bow); confirm no MAJI/struct-14 regression; keep enforcing do-not-retry.

**Forward strategy (Wind Clau assessments — captured for when the bow is signed off):**
- **Scaling to the other 20 `itemmdl` items = feasible for DISPLAY** (get-item/field/viewer). Reusable as-is: retained `itemmdl` arc + heap (all 21 share the arc), A+A′ guards, bake-once, the dump tooling (2I/2S/2B), struct-0/ambient lighting, and the **2B‴ per-material bind** (it already reads each drawn material's *own* GX — just generalize the `isVbowDrawMaterial` guard to any `itemmdl` material). Per-item work is incremental: dump structure → generic bind → tune warmth → wire BTK/BRK. Far faster than the bow (infra + crash-playbook done).
- **Gameplay ≠ rendering. Bones give animation; behavior comes from code.** The `v*` models are **2-bone display props**, not gameplay rigs. Held/Track-B = **stiff but functional** for items TP already has (bow, boomerang, hookshot, boots, bombs, bottles, magic armor, telescope→hawkeye) because TP's code drives them; **no-TP-analogue** items (Deku Leaf, Sail, WW baton, Grappling Hook, Tingle Tuner, Picto Box, bags, Skull Hammer) need **new procs** (big).
- **Bone transplant (WW mesh → `AL_BOW` skeleton so TP's `BVJMPCL` BCK flexes it)** = possible but **asset-tool / Plan-B work**; shape mismatch (recurve vs TP) means retargeting the rig/anim is the hard part — visual-flex polish, not a functionality gain. **Enemy-weapon bones rarely help** (their motions ≠ item behavior). Full detail: see the "transplant bones" Q&A in chat history / summarize into Plan B if pursued.
- **Plan B (BMD re-rig in external tools)** stays the escape hatch for held/animated fidelity.

**Fresh Wind Clau opener (paste):**
> Continue as Wind Clau (review/graphics-strategy) on the WW `itemmdl` bow. Read `docs/wind-waker-item-work.md` (canonical, trust it + git over memory) then this doc's `▶ RESUME HERE` and `▶ WIND CLAU HANDOFF`. Pipeline is locked (2N′ + 2B‴ + struct-0 + ambient-only, no MAJI/struct-14). I review Cursor's replay screenshots vs the WW reference (goldenrod body, cream tips, subtle ink) and pick next-knob branches; I don't edit source. Do not reopen locked decisions or the do-not-retry list. Next after color sign-off: BTK spin then Track B held bow; then generalize the display formula to the other 20 items.

---

## ▶ Interconnected pass — MAJI kills detail (2026-06-30)

**User report (post struct-0 + MAJI, commit `64726c4`):**
- ✅ Color **way better** indoors and outdoors generally (MAJI fixed overbright skip-MAJI wash)
- ❌ **Visual detail missing everywhere color shows:** white tips, arrow fletching/gold head, orange cel gradient — flat orange fill only
- ❌ Outdoors: **color/black angle swing returns** at some camera angles (MAJI view-matrix lights on cel surfaces)

**Mechanism (pinned from code + symptoms):**

| Layer | Role | struct-0 + skip-MAJI | struct-0 + MAJI (#4) |
|-------|------|----------------------|----------------------|
| **2B‴** | Per-mat texgen+TEV after `callDL` | ✅ Texture samples → true orange + texel detail | ✅ Color samples |
| **MAJI** | `setAmbColor` + **6 view-matrix lights** on materials | Skipped → amb ~255 → overbright flat | **Crushes cel:** view lights on locked `bdl3`; **SC_Vbow_v** 3-stage pass washed |
| **Outdoor swing** | View-dependent diffuse | Reduced when textured + persistent hook | **Returns** via MAJI `setLight` + view matrix |

**4B fix (Cursor, in tree):** struct-0 + **`dWwItemmdl_applyBowMaterialAmbientOnly`** (room `AmbCol` on both mats, **no** MAJI lights) + 75% amb scale. Keeps 2B‴ sampling; drops view lighting that erased detail.

**Expected replay:** orange **with** gradient / tips / arrow detail; no outdoor black-out. Tune 75% scalar only if still bright — **do not re-enable MAJI**.

**User replay (2026-07-01):** ✅ **Color stable** indoors/outdoors. ⚠️ **Still too bright** — commit `72a2f01194`; **4C** lowered amb + skipped efplight + per-mat caps; screenshots still neon yellow / #FFF tips → **4D** (fixed warm + lower caps + absolute SC ink).

---

## ▶ Interconnected pass — 4D brightness + orange (2026-07-01)

**Problem (4C replay):** stable but wrong exposure/hue — neon yellow body (neutral 50% room amb), cream tips → #FFF bloom, cave halo (beams + remaining self-glow on bright texels). WW target = goldenrod/amber body + cream tips + grey-brown ink, not lemon + emissive white.

| Layer | 4C | 4D |
|-------|----|----|
| Room ambient | 50% of `settingTevStruct(0)` | **Fixed warm** `(110, 85, 55)` via `dWwItemmdl_setWwBowActorAmbient` |
| Vbow_v body | Cap 120 | Cap **90** |
| SC_Vbow_v ink | 65% of body amb | **Absolute** `(72, 58, 48)` — dark warm ink, not white fill |
| efplight | Skipped | Unchanged (skipped) |
| MAJI / struct 14 | Off | Off |

**Constants:** `d_ww_itemmdl_pc.cpp` — `kWwBowFixedAmbR/G/B`, `kWwBowBodyAmbCap`, `kWwBowInkAmbR/G/B`. Log once: `4D ambient-only: body=… cap=90 ink=72,58,48`

**Tune ladder (one knob at a time, screenshot after each):**
- Muddy → raise all three body channels ~10 → `(120, 95, 65)`
- Still bright → `(95, 75, 48)`; body cap 90 → **80**
- Tips/arrow still bloom after close-up improves → A/B `dWwItemmdl_suppressOutlineForDraw()` during get-item draw (one test build); if fixed → SC pass is bloom source
- Close-up OK, full cave pose washed → dim/skip **get-item beam particles** (not efplight), same pattern as efplight skip

**Do not:** re-enable MAJI, struct 14, or only drop 50%→40% without warming (grey-yellow, not WW orange).

**Acceptance gap (post-commit polish):** orange direction ✅ closest yet; still chase cream tips (#FFF bloom), cave full-pose halo (beams), less lemon / more amber.

**Post-commit order (do not revisit MAJI / struct 14 / % room amb):**
1. ~~**SC A/B wiring**~~ — `kWwBowSuppressScInkPassForDraw` in `drawWwBowModel` (default false).
2. ~~**Constant tweak**~~ — `(105,78,48)` + cap **80** (4E).
3. ~~**Beam pass**~~ — skip GETITEM flash/halo/star via `dWwItemmdl_shouldSuppressGetItemBeamParticle`.
4. ~~**Ink polish**~~ — `(58,48,42)` (4E).
5. **Replay + tune** — SC A/B flip, warmth/cap/ink one knob at a time.
6. After color sign-off → BTK spin, Track B held bow (`d_a_alink_bow.inc`).

---

## ▶ Interconnected pass — 4E polish (2026-07-01)

**Built on 4D baseline** (`6023333a8b`). User direction: pipeline locked; remaining work is exposure/hue vs WW reference, not recovery.

| Layer | 4D | 4E |
|-------|----|----|
| Body ambient | `(110, 85, 55)` | `(105, 78, 48)` — warmer, less yellow |
| Vbow_v cap | 90 | **80** |
| SC_Vbow_v ink | `(72, 58, 48)` | `(58, 48, 42)` — darker grey-brown target |
| efplight | Skipped | Unchanged |
| Demo GETITEM beams | On | **Skipped** (`ID_IT_JN_GETITEM_FLASH/HALO/STAR` in `dDemo_particle_c::emitter_create`) |
| SC draw A/B | — | `kWwBowSuppressScInkPassForDraw` (default **false**) |

**Files:** `d_ww_itemmdl_pc.cpp` (constants + draw suppress), `d_a_demo_item.cpp` (beam suppress flag lifecycle), `d_demo.cpp` (particle hook), `d_ww_itemmdl_pc.h`.

**Log:** `4E ambient-only: body=105,78,48 cap=80 ink=58,48,42 scSuppress=0 beams=1`

---

## ▶ Interconnected pass — `SC_Vbow_v` is a geometry+TEV pass, not "ink ambient" (2026-07-01)

**Trigger:** SC A/B replay (user, close-up cave). **Suppress ON** = exposure under control but **amputates** ornamental limb-tip caps + silver arrowhead + back-half feather + string. **Suppress OFF** (4E default) = full silhouette, closest to WW ref, but the SC parts **bloom white with a halo**.

**Verdict (Wind Clau — supersedes the "SC = grey-brown ink line-art" model in both docs):**
`SC_Vbow_v` is a **second cel draw pass carrying real geometry** — white/silver limb-tip caps + teal wrap bands, silver arrowhead, string/nock art, back-half of the fletching — **plus** the ink lines. It is **not** an optional outline. Proven by the A/B: hiding it deletes those parts, and in the WW reference those parts are genuinely white/silver/teal *geometry*, not outlines. ⇒ **Never ship `wwItemmdlBowScSuppress` ON.** It is a bisect diagnostic only ("is SC the bloom source?" → yes). Two sibling chats converged on this independently.

**Correction to the sibling-chat plan (which ranked ambient tweaks first, B→A→C): DUMP before ambient.** The evidence already falsifies "ambient controls SC brightness":
- SC ink ambient is **already dark** `(58,48,42)` yet the tips **still bloom pure white**. If ambient were the lever, dark ambient would have darkened them. It didn't.
- **Confirmed in source** (`d_ww_itemmdl_pc.cpp` `wwBowMatDrawPostDl` → `applyTexGenFromMaterial` + `applyTevOrderFromMaterial`): the 2B‴ post-DL hook replays **only** `GXSetNumTexGens`/`GXSetTexCoordGen` + `GXSetTevOrder`/`GXSetNumTevStages`, and forces **`GX_COLOR_NULL`** raster color on every stage. It does **NOT** replay TEV stage color/alpha ops, TEV/konst register colors, or blend mode.

⇒ SC's brightness/glow is governed by exactly the state we are **not** replaying (TEV konst / stage ops / blend mode) — the same "Aurora didn't realize the baked GX" class as the original colorless-body bug (Fix B). Ambient A/B cannot reach it.

**Mechanism (tips are the RIGHT color, just over-hot):** SC tips/string/bands are white/silver/teal in the WW ref too — our problem is they render **emissive-white with a halo**, i.e. an **attenuation** problem, not a recolor. Leading suspects, in order: (1) **additive / edge blend** on the SC PE block (`GX_BL_ONE,GX_BL_ONE` or `OpaTexEdge`) → blooms under any lighting + explains the halo; (2) a **hot TEV konst/register** near white; (3) engine **bloom post-threshold** catching bright texels. All three are downstream of TEV/blend; none are ambient. This also means the 4D→4E "darken ink toward grey-brown" moves were the **wrong axis** for the tips (ref tips are white/silver, not brown).

**Precondition (from sibling chat — verified real):** both passes must reach the hook. Once per session in `albw_ww_itemmdl_debug.txt`:
`2B apply post-dl: mat=Vbow_v nTexGen=2 nTev=2` **and** `2B apply post-dl: mat=SC_Vbow_v nTexGen=3 nTev=3`, plus `scSuppress=0` in the 4E line. If the SC line is missing → `isVbowDrawMaterial` / draw-scope bug; fix that first (no tuning helps until SC draws).

**Decision tree (canonical — copy to Cursor):**
```
SC suppress OFF (4E default — permanent)
  1. Log shows BOTH Vbow_v + SC_Vbow_v post-DL lines?
       NO  → fix 2B‴ scope / isVbowDrawMaterial; STOP (nothing else matters yet).
       YES → 2.
  2. DUMP (log-only, NO render change; reuse 2I tooling): for SC_Vbow_v log
       - blend mode (GXSetBlendMode type + src/dst factors)   <- additive test
       - each TEV stage color/alpha in+op, and konst/register colors
       - baked TevOrder rasColor (vs our forced GX_COLOR_NULL)
  3. Pick fix from the dump (this is sibling option C, ELEVATED — the identified gap,
     not a last resort; extend wwBowMatDrawPostDl to replay SC's full TEV/blend):
       additive/edge blend  -> set normal alpha blend (or clamp output) for SC pass
       hot konst/register   -> replay + scale SC konst toward matte cream/silver (~205,195,175)
       raster mismatch      -> replay baked rasColor instead of GX_COLOR_NULL for SC stages
  4. Only AFTER tips/head/feather read MATTE (no halo): micro-tune body warmth
     (100,72,44 or cap 75). Vbow_v body is otherwise SIGNED OFF — do not retune it.
```

**Dump result (2026-07-01, suppress OFF, frame 725):** blend `type=0` = **GX_BM_NONE** ⇒ **additive blend RULED OUT**. `opaTexEdge=1`. Konst/regs **hot white** — `kColor[0/1/3]=#FFF (kSel=12=KCSEL_K0)`, `tevReg[1]=tevReg[2]=#FFF`; `kColor[2] a=50 kSel=30` is the OpaTexEdge alpha threshold, not color. Stages: st[0] `lerp(reg0=128, KONST=white, TEXC)`; st[1] `CPREV*TEXC`; **st[2] `CPREV + HALF` (konst selects the 0.5 term) = a +0.5 highlight add — prime halo amplifier**. 2B‴ replays none of these.

**Caveat (Wind Clau — do not skip):** "hot white konst present" ≠ "konst is the cause." White konst is **normal** for a cel material that gets its color from the *texture*. The dump shows config, not the sampled texel — it does **not** distinguish (a) SC texture sampling correctly + engine **bloom-threshold** on legit-white tips, from (b) SC texture sampling **as white** (3-texgen binding issue) → stages collapse to white regardless of konst. Scaling konst blind papers over (b) with a wrong tint and won't generalize.

**Step 4 — two sub-steps (don't collapse realize + attenuate):**
- **4a — replay authentic FIRST, no scaling.** Extend `wwBowMatDrawPostDl` (SC only) to replay baked TEV konst/regs + stage color/alpha ops + OpaTexEdge/PE at their **real** values. Screenshot.
  - Bloom changes / detail returns ⇒ Aurora wasn't realizing baked SC TEV; **realization is the authentic fix** (generalizes cleanly to the other 20 — best case).
  - Bloom identical ⇒ tips genuinely white; halo is engine bloom threshold ⇒ 4b.
- **4b — attenuate ONLY if 4a still blooms.** One tunable output scalar on the SC pass (toward ~`205,195,175`, or temper the st[2] `+HALF`) below the bloom threshold. Log it. Cosmetic workaround, **not** authentic — every white-tipped item inherits this scalar in Phase 6; flag it.

**✅ RESULT (2026-07-01, 4a landed):** authentic SC TEV/regs/konst replay **CHANGED the bloom and RESTORED detail** — tip caps, arrowhead, string all read (user replay). ⇒ **4a is the ROOT FIX (Aurora was not realizing baked SC TEV); it generalizes to the other 20 items.** Commit 4a.

**⚠️ REVISED after clearer side-by-side (2026-07-01):** two brightness gaps remain — earlier "body signed off / 4b not needed" was **too early**:
- **SC caps must be matte SILVER/pewter (~RGB 160), not white.** Ref caps + arrowhead are brushed steel with teal bands; ours are near-white + glowing. Cause = **`st[2] CPREV + HALF`**: white konst makes it a constant **+0.5** that clips a mid-grey texel to white. **4b is now WARRANTED** — but first verify 4a faithfully replayed `st[2]` `colorOp=0x0b` **bias/scale** (≠ the 0x08 of other stages); if the op decode is right, **temper the `+HALF`** to reveal the silver (also kills the string bloom). Teal cast on caps/nock is **correct** — keep it.
- **Body (`Vbow_v`) NOT signed off — too bright + flat vs ref's amber cel gradient.** Cheap fix: cap `80→70` or ambient `(105,78,48)→(95,70,44)`. Authentic fix (optional, A/B for regression): **generalize the 4a full-TEV replay to `Vbow_v`** so its own cel TEV restores the two-tone amber (body is currently ambient-only flat fill — same flatness SC had pre-4a).

**Order (one knob/build):** 1) SC caps → silver (verify colorOp 0x0b, temper `+HALF`); 2) body cap `80→70`; 3) only if still flat, Vbow_v TEV-replay for gradient. Color at **~85%** — 1–2 cycles from a strong match.

**Demoted (fallbacks only — current evidence says they won't fix the bloom):** sibling's "cream SC ambient `(95,88,72)`" (A) and "stop overriding SC ambient" (B). Keep for the case where the dump shows SC *is* ambient-controllable; do not spend the first cycles on them.

**Do-not (carry forward + new):** don't ship suppress ON; don't darken SC toward brown (wrong axis — ref tips are white/silver); don't add body brightness to "fix" the tips (wrong material); don't re-enable MAJI / struct-14 / % room ambient.

---

## ▶ Interconnected pass — 4C brightness tune (2026-07-01) — superseded by 4D

**Problem:** stable color but over-exposed — white tips bloom, orange → yellow-white, arrow detail lost. Three stacked causes (external handoff + our tree):

| Layer | Fix (4C) |
|-------|----------|
| Room ambient 75% | **50%** in `setTevStr` |
| efplight `mLightStrength=8` | **Skip** `dKy_efplight_set` for WW bow (`actionStart`); beams/particles unchanged |
| Same amb on body + SC | **Vbow_v** cap 120; **SC_Vbow_v** 65% of base |

**Constants:** `d_ww_itemmdl_pc.cpp` — `kWwBowAmbScalePct`, `kWwBowBodyAmbCap`, `kWwBowOutlineAmbScalePct`. Log: `4C ambient-only: …`

**Do not:** re-enable MAJI or struct 14 for brightness. If still bright → 40% amb or fixed neutral `(140,120,90)` per room compare.

**Acceptance:** warm orange-tan body, visible gradient, cream tips (not #FFF bloom), readable arrow, no cave halo.

---

## ▶ First clean playtest (2026-06-30) — user report + log

**Build:** 2N′ bake-once + Fix B single-pass; 2S summary-only (`count=2 mats=2 joints=2`); per-shape dump removed; struct **14**, MAJI-skipped; no 2K.

**User report (functionality):**
- ✅ **Very stable** — no crashes across session
- ✅ Color **stable indoors**
- ⚠️ Color **unstable outdoors** (camera/room dependent — matches prior struct-14 outdoor swing)
- ⚠️ Overall color **very bright, almost white** (new vs earlier Fix B baseline that read as normally colored fill — Wind Clau to compare screenshots)

**User report (geometry):** Screenshots being sent to Wind Clau separately — assess whole bow vs fragments / missing arrow+ink from images.

**Log evidence (latest session, post dump-fix):** consecutive `Replay finished (6 s)` at frames **934, 1299, 1828, 2617, 2963, 3824, 4392** (room 1). Draw completes: `bracket: draw: after setShadow` each replay. `2S shape [draw-pre-dl]: count=2 mats=2 joints=2`.

**Wind Clau questions for screenshot review:**
1. Is the bow **whole** (arrow + recurve + string) or still **fragments/glow**?
2. Is brightness **washed-out white** on body, SC ink pass, or both?
3. Does outdoor instability look like **angle swing** (struct-14) or **flicker/crash-adjacent**?

**Recommended next code move (pending Wind Clau OK):** Safe **per-material GX** so `SC_Vbow_v` gets its own 3-stage TEV/texmap when its shape draws — root hypothesis for invisible SC / missing detail *and* possible overbright additive pass.

### Wind Clau screenshot verdict (2026-06-30)

**Q1 — whole or fragments? → WHOLE. This is the geometry win.** Close-up screenshot shows the complete bow: recurve limbs, grip, string, **and the nocked arrow**, all present. The missing-arrow/fragments problem is **SOLVED** by 2N′ bake-once + single-pass. Do not regress this.

**Q2 — brightness location? → the whole thing reads MONOCHROME (black↔white), i.e. `Vbow_v` body is UNTEXTURED, not "washed white on the ink pass."** Screenshot 1 (dark angle) = near-black bow; screenshot 2 (bright angle) = blown-white. Same surface, lit-only, no wood/gold texture color.

**Q3 — outdoor instability? → angle swing (struct-14), not crash-adjacent.** 7× `Replay finished`, fully stable. The black↔white is the struct-14 view-matrix swing acting on an *untextured* surface (no texel color to anchor it).

**Root of the color regression (from the 2I dump — evidence, not guess):** 2N applies texgen from the **max-count material `SC_Vbow_v` (3 texgens)** globally, but the two materials' texgen layouts **differ at slot 1**:
- `Vbow_v`: tg0=`TEX0`, tg1=**`COLOR0`**  (2 texgens)
- `SC_Vbow_v`: tg0=`TEX0`, tg1=**`NRM`**, tg2=`COLOR0`  (3 texgens)

Forcing `SC_Vbow_v`'s config onto the single pass feeds `Vbow_v` the **wrong texcoord at slot 1** (`NRM` vs `COLOR0`) → `Vbow_v` mis-samples its texture → untextured/monochrome. That's the regression from the earlier (colored) Fix B baseline, which used `Vbow_v`'s own texgen.

**PRIMARY FIX (safe, single-pass, no crash path): bind `Vbow_v`'s OWN texgen+TEV in `prepareWwBowGxForDraw` — not `SC_Vbow_v`'s max.** Restores main-body texture color while KEEPING the complete geometry. `SC_Vbow_v` (ink/edge) renders imperfect under the 2-texgen config, but it's the secondary pass — far better than a monochrome whole bow. This is a small revert of 2N's "max material" choice.

**Deferred (harder / polish):**
- Perfect `SC_Vbow_v` too ⇒ true **per-material GX** (each shape its own texgen+TEV) via a **safe per-shape draw** — NOT double-`modelUpdateDL` (overran) and NOT per-shape DL-parse (AV). Only if imperfect ink is unacceptable.
- struct-14 outdoor swing ⇒ optional ambient-only tevStr, *after* color is restored.

**Do:** switch `prepareWwBowGxForDraw` texgen source `SC_Vbow_v` → `Vbow_v`, keep everything else (2N′ bake-once, single-pass, struct-14, MAJI-skip), rebuild, wipe caches, replay indoor+outdoor. Expect: whole bow + wood/gold body color back; only the outdoor brightness swing remaining.

---

## 🧭 MASTER IMPLEMENTATION ROADMAP — READ THIS FIRST

> **Goal:** Retail TP `itemmdl.arc` → `vbow.bdl` in the bow get-item spin, **WW-faithful cel look** (not TP-material shortcut).  
> **Strategy:** **Plan A** — retail `bdl3` + code-side TEV/draw trial-and-error. **Plan B** (modder BMD swap) is a documented escape hatch only.  
> **No SS decomp** — trial-and-error + optional TWW Rosetta for struct-0 vs struct-14 only.  
> **Tool split:** Cursor executes phases below; Claude reviews diffs/screenshots/logs and revises this doc’s “Open questions” / experiment order.  
> **Phase numbering:** **This doc’s Phases 0–6 are canonical.** `wind-waker-item-work.md` “Implementation plan” uses a different 0–3 scale — ignore it for execution. Session openers must say **“roadmap Phase N”**.

### Visual target (what “done” looks like)

| Stage | On screen |
|-------|-----------|
| **Baseline (already hit once)** | WW bow **shape** spins in get-item; **dark/muddy** fill; possible **white edge shards** on outline pass |
| **Plan A success** | Same WW mesh/textures; **readable cel colors**; tuned **`SC_Vbow_v`** ink outline |
| **Plan B (alt)** | Same WW **silhouette**; shading reads **TP get-item glossy** — faster to stabilize, less WW-authentic |

Our path is **still feasible**: geometry + load path worked; remaining work is **lighting/TEV**, not “can we show the model.”

---

## Phase 0 — Boot hygiene (gate; no WW code)

**Owner:** User + either agent  
**Exit:** Title → opening demo green; no Aurora `unhandled tcg src 21`.

| Step | Action |
|------|--------|
| 0.1 | Build `RelWithDebInfo` via `build_run.bat` — no `DUSK_DRIVE*` for manual playtests |
| 0.2 | If logo/demo fatal: delete **both** `%AppData%\TwilitRealm\Dusklight\dawn_cache.db*` **and** `pipeline_cache.db*` ([build-fps-guidelines](../build-fps-guidelines.md)) |
| 0.3 | Confirm WW track still **out** of `files.cmake` — bisect proved boot crash was cache, not WW |

**Do not:** `git checkout`/`restore` whole files; `git stash`; PowerShell `git show HEAD:file > file`.

---

## Phase 1 — Re-link shell only (build, no item heap yet)

**Owner:** Cursor  
**Exit:** Project compiles and **launches** with WW object files linked; **`d_s_play` hook present but inert when toggle off** — no item-pipeline / heap behavior change.

| Step | Files / work |
|------|----------------|
| 1.1 | Add to `files.cmake`: `src/d/d_ww_itemmdl_pc.cpp`, `src/d/d_ww_itemmdl_test.cpp` (+ headers already present) |
| 1.2 | Restore settings: `game.wwItemmdlGetItem` in `include/dusk/settings.h`, `src/dusk/settings.cpp` |
| 1.3 | Restore Editor UI: toggle + replay button in `src/dusk/ui/editor.cpp` (Advanced → ALBW) |
| 1.4 | Restore play tick hook in `src/d/d_s_play.cpp` for replay helper |
| 1.5 | **Do not** touch `d_a_itembase_static.cpp`, `d_a_demo_item.cpp`, or `d_a_alink_demo.inc` |

**Playtest:** Launch only. Toggle off → confirm replay hook early-outs (no demo spawn side effects). Toggle may do nothing visible yet — that is OK.

**Claude review prompt:** “Phase 1 diff is shell-only — confirm no item pipeline or demo.inc changes.”

---

## Phase 2 — Wire get-item heap (back to broken mesh baseline)

**Owner:** Cursor  
**Exit:** Toggle **on** → Editor replay or real bow get-item shows **WW mesh spinning** (colors wrong OK).

| Step | Files / work |
|------|----------------|
| 2.1 | Surgical branch in `daItemBase::CreateItemHeap`: bow + toggle → `dWwItemmdl_getVbowModelData("itemmdl")` + `dWwItemmdl_patchModelForPc` |
| 2.2 | Demo item arc swap: bow + toggle → `dComIfG_resLoad` / `DeleteBase` on `"itemmdl"` (`d_a_demo_item.cpp`) |
| 2.3 | PC load: `J3DMLF_DoBdlMaterialCalc`, locked baked-DL draw (**no** unlock/`diff()`) |
| 2.4 | **Stability (2026-06-30 fix):** WW bow skips `setLightTevColorType_MAJI`; loader never attaches arc-resident `J3DModelData*`; **no** draw-time `SC_*` shape hide until Phase 4 |
| 2.5 | Joint null guards in demo item (`demoItemRootHalfHeight`) — WW `vbow` has 2 joints |
| 2.6 | Replay helper in `d_ww_itemmdl_test.cpp` only — **no** `d_a_alink_demo.inc` edits |

**Debug:** `Documents/dusklight/albw_ww_itemmdl_debug.txt`

**Playtest checklist:**

- [ ] Toggle off → vanilla `O_gD_bow` (~6 s replay)
- [x] Toggle on → WW `vbow` visible (broken colors OK; white edge shards OK for now) — **2M 1st replay**
- [x] Log shows `procCoGetItemInit returned` → `Replay finished (6 s).` — **frame 1096**
- [ ] **Repeatable** 2nd + 3rd replays (same room, after room change) — **2M 2nd failed at load**
- [ ] Toggle off again → vanilla

**Claude review prompt:** Paste screenshot (broken mesh) + last 20 lines of debug log; ask for smallest Phase 3 experiment order.

---

## Phase 2 stability gate — contingency (if retest still crashes)

**User rule (2026-06-30):** If the Phase 2 retest **still crashes**, Cursor **stops implementation** — no further code experiments in that session. Document findings here, wait for user direction + Wind Clau review, then execute the chosen branch only.

### How to read the log (classify before coding)

| Last log line(s) | Crash window | Meaning |
|------------------|--------------|---------|
| `patchModel: done` but no `demo item ready` | Heap / spawn | `CreateItemHeap` or actor init failed after load |
| `demo item ready` but no `starting procCoGetItemInit` | Pre-proc | Replay timing, joint null, or item deleted before demo start |
| `procCoGetItemInit returned` then silence | **First get-item draw** | Most common — locked `bdl3` + TP draw stack (`DrawBase`, `mDoExt_modelUpdateDL`, tev, shadow) |
| `Replay finished (6 s).` | — | **Phase 2 pass** → proceed to Phase 3 gate |

Paste **last ~20 lines** of `albw_ww_itemmdl_debug.txt` + note whether crash was instant on replay or on toggle-only.

### Strategy (ranked — one branch per approved session)

Do **not** stack these in one diff. Pick one after log classification + Claude sign-off.

| Priority | Branch | Hypothesis | Minimal change | Pass signal |
|----------|--------|------------|----------------|-------------|
| **2A** | Draw-path bisect | Fault inside `DrawBase` sub-step | Add **temporary** log lines only: before/after `setTevStr`, `animEntry`, `mDoExt_modelUpdateDL` in `daDitem_c::draw` / `DrawBase` | Last log line names the crashing call |
| **2B** | Private heap copy | Arc/cache alias or `finishLoadedModelData` still shares mutable state | Load `vbow` via `JKRReadIdxResource` → temp buffer → `loadBinaryDisplayList` (same flags); **never** return `dComIfG_getObjectRes` pointer | Load + draw without crash |
| **2C** | Minimal WW draw stub | TP get-item draw stack incompatible with locked WW cel DL | WW bow only: skip `setListDark`, skip shadow, struct **14** tev only (already no MAJI), plain `mDoExt_modelUpdateDL` | Mesh visible, ugly but stable |
| **2D** | Vanilla mesh / WW arc | Isolate loader vs draw — is it the mesh or the pipeline? | Toggle on loads `itemmdl` arc but heap still uses `O_gD_bow` BDL (one session only) | Crash gone ⇒ WW mesh/loader; crash remains ⇒ demo.inc / Link proc |
| **2E** | Defer edge without `hide()` | `SC_Vbow_v` white shards acceptable short-term | No shape mutation; accept dual-pass draw until Phase 4 | Stability over visuals |
| **2F** | Plan B preview | Plan A draw path exhausted | Modder BMD + TP get-item materials (see Plan B section) | Stable TP-styled WW silhouette |

### Explicit do-nots while gate is open

| Do not | Why |
|--------|-----|
| Phase 3 TEV experiments (struct 0, delegate, magenta) | Phase 2 must pass first |
| Re-enable draw-time `SC_*` hide | Correlated with first-draw crash |
| Re-enable MAJI on WW bow | Correlated with first-draw crash on locked `bdl3` |
| Unlock materials / `diff()` on BDL | Documented crash |
| Patch `d_a_alink_demo.inc` | Use replay helper only |
| Whole-file git restore | Encoding / WIP loss |

### Agent workflow on crash

1. User reports crash + log tail.
2. Cursor **updates session log** (below) with classification row — **no source edits** unless user explicitly approves a numbered branch (2A–2F).
3. User pastes log + this section to **Wind Clau** for branch pick.
4. Cursor implements **one** approved branch; user playtests; repeat until Phase 2 checklist passes.

**Deliverable when Phase 2 passes:** screenshot (toggle on, broken colors OK) + log tail → Wind Clau Phase 3 gate (3.0 magenta isolate first).

---

## Phase 2 crash analysis — 2026-06-30 retest (doc only; no code)

**Status:** Phase 2 stability fix **did not pass**. **Run A (widened bracket)** confirms fault is **post–CPU draw, GPU submit** — not heap-at-load, not pre-draw init. Cursor **paused** until user approves one branch below.

### Log evidence — Run A (widened-2A; frame ~1801)

Toggle **on**, 2D isolate **off**, Editor replay. Last lines of `albw_ww_itemmdl_debug.txt`:

```
frame=1801 heap: bracket: CreateItemHeap: enter pass 1
frame=1801 heap: bracket heap [CreateItemHeap enter]: ptr=... name=Actor mng (DemoItm)
frame=1801 heap: bracket: getVbow cache MISS
frame=1801 heap: getVbowModelData: loaded via name (vbow.bdl) mats=2 tex=3
frame=1801 heap: patchModel: done
frame=1801 heap: bracket: CreateItemHeap: after J3DModel__create OK
frame=1801 heap: bracket: CreateItemHeap: __CreateHeap OK
frame=1801 phase=5: procCoGetItemInit returned (dev replay; auto-end 6s)
frame=1801 heap: bracket: execute: first frame
frame=1801 heap: bracket: draw: entry (first visible draw)
... all DrawBase pins through ...
frame=1801 heap: bracket: draw: after modelUpdateDL
frame=1801 heap: bracket: draw: after setShadow
```

**No** `Replay finished (6 s).` Aurora: `[FATAL] unhandled tcg src 21` (GPU pipeline abort; Windows may show `0xC0000409`).

**Earlier narrow log (frame 1488, pre-bracket):** stopped at `procCoGetItemInit returned` with no draw pins — **misleading**; widened bracket proves draw **does** run on CPU.

**Control (still good):** Toggle on + **vanilla** `O_gD_bow` heap (or toggle off) → `Replay finished (6 s).` Crash is **WW mesh path**, not replay helper / Link proc alone.

### Classification (updated after Run A)

| Field | Value |
|-------|--------|
| **Crash window** | **After first draw CPU path completes** — all DrawBase steps log `after`; fault on **GPU replay** of submitted DL |
| **Load path** | Raw `vbow.bdl` → `loadBinaryDisplayList` + `finishLoadedModelData` (shared DL + `MaterialAnm`) |
| **Heap** | Load + model create on **`Actor mng (DemoItm)`**, callback **pass 1**, cache **MISS** — heap-at-load hypothesis **closed** |
| **Not boot / cache** | Game reached field + replay spawn; not `tcg src 21` at title (boot cache wipe still mandatory after reconfigure) |

### Ruled out this session (fixes were in build; crash persists)

| Fix applied | Expected if root cause | Result |
|-------------|------------------------|--------|
| Skip `setLightTevColorType_MAJI` on WW bow | No material light/amb patch crash | **Still crashes** |
| Remove draw-time `SC_*` `shape->hide()` | No shared-arc mutation on draw | **Still crashes** |
| Reject arc-resident `J3DModelData*` in loader | Heap-owned copy only | **Still crashes** — load via raw `bdl3` + `finishLoadedModelData` |
| Joint null guards | No pre-proc fault | **Passed** — reaches `procCoGetItemInit returned` |

**Conclusion:** Root cause is **not** MAJI, outline hide, arc-resident attach, heap-at-load, or any single DrawBase **CPU** step alone. Top suspect is **WW loader post-pass** (`finishLoadedModelData`: `newSharedDisplayList` + `makeSharedDL` + per-material `J3DMaterialAnm`) producing a DL/texgen config Aurora rejects on **first GPU draw**.

### Draw path already in play (narrows bisect)

`daDitem_c::setListStart()` is **empty** — demo items do **not** call `dComIfGd_setListDark()` (unlike field items). Remaining `DrawBase()` steps for WW bow:

1. `settingTevStruct(14)` only (MAJI skipped)
2. `animEntry()` — no-op for bow (no BTK/BCK in row)
3. `settingBeforeDraw()` — only if actor flag 4 (effect mtx)
4. **`mDoExt_modelUpdateDL(mpModel)`** ← top suspect
5. `setListEnd()` → `dComIfGd_setList()`
6. **`setShadow()`** ← second suspect

### Technical hypothesis (for Wind Clau)

**H1 — `mDoExt_modelUpdateDL` vs locked `bdl3` post-process (most likely)**  
`finishLoadedModelData()` runs after `loadBinaryDisplayList` with `DoBdlMaterialCalc`:

- Adds `J3DMaterialAnm` per material
- `newSharedDisplayList` + `makeSharedDL()`

`mDoExt_modelUpdateDL` (`m_Do_ext.cpp`) branches on shared-DL + lock state:

- Unlocked + shared DL → **`diff()`** path (documented crash on WW BDL)
- Locked BDL (`MDL3` sets material table lock) → **`unlock()` → `update()` → `lock()`** path

Either branch may fault on WW cel materials (`SC_Vbow_v` edge + `Vbow_v` fill) under TP get-item struct-14 tevstr, without MAJI. Vanilla `O_gD_bow` is a different asset class (TP BMD, not WW `bdl3`).

**H2 — Dual-material GPU draw (`SC_Vbow_v` + `Vbow_v`)**  
Both materials draw on first frame (outline not hidden). Edge pass (`OpaTexEdge`) + baked WW cel DL + struct-14 env may **GPU-fault** Aurora even when CPU path survives init.

**H3 — `setShadow()` on WW model**  
`DrawBase` always calls `setShadow()`; WW bounding / shadow path untested.

**H4 — `s_cached` static model**  
`dWwItemmdl_getVbowModelData` caches first successful load for process lifetime. Unlikely primary cause (first run crashes too), but **always use a fresh process** per playtest.

### Revised execution plan (superseded — see Wind Clau review below)

> **Do not run narrow 2A.** Use **widened-2A + 2D** per **2026-06-30 — Wind Clau Phase 2 crash review** and **Wind Curs response** in the same section.

| Order | Branch | When | Action | Pass signal |
|-------|--------|------|--------|-------------|
| **1** | **2A′ — Widened bracket** | **First session (with 2D)** | Log-only pins: `JKRGetCurrentHeap` + cache hit/miss at load; after `J3DModel__create`; after `__CreateHeap`; `execute()` entry; `draw()` entry; then each `DrawBase` step | Last pin names faulting function |
| **2** | **2D — Loader isolate** | **Same session as 2A′** | Toggle on + `itemmdl` arc, heap still `O_gD_bow` (no render change) | Crash gone ⇒ WW mesh/loader; remains ⇒ Link/demo proc |
| **3** | **2G′ — Drop shared DL** | Bracket pins `modelUpdateDL` | Skip `newSharedDisplayList` + `makeSharedDL` in `finishLoadedModelData` (~2 lines) **before** 2G/2H draw overrides | Draw survives |
| **4** | **2H — WW draw override** | Drop shared DL insufficient | Locked entry / body-mat-only draw | Mesh visible |
| **5** | **2C — Skip shadow** | Bracket pins `setShadow` | WW bow: no-op shadow in demo draw | Stable spin |
| **6** | **2F — Plan B** | 2A′–2H exhausted | Modder BMD + TP materials | Stable silhouette |

**Deprioritized:** narrow **2A** (DrawBase-only — false negatives); **2E** (already no hide); **2B** (already on raw bdl3 path).

### Playtest hygiene (mandatory)

1. **Quit dusklight fully** between attempts (clears `s_cached` + GPU state).
2. Note Windows exit code if shown (`0xC0000409` / `tcg src N` → GPU path).
3. Paste **last ~20 log lines** + which branch user approved before any code resumes.

### Wind Clau prompt (paste on branch pick)

> Phase 2 retest still crashes after MAJI/hide/arc-resident fixes. Log stops at `procCoGetItemInit returned`. Read **Phase 2 crash analysis** + **Wind Clau Phase 2 crash review** in `Wind Curs-Wind Clau.md`. Next step: **widened-2A + 2D** (log only). No Phase 3 until mesh spins 6 s.

---

### 2026-06-30 — Wind Clau Run A review + `tcg src 21` decode (READ BEFORE 2G′)

Run A was the right move and it paid off. Confirmed against source:

- **Widened bracket worked as intended.** CPU path logs through `after modelUpdateDL` and `after setShadow` — so the fault is **not** any DrawBase CPU step, not `J3DModel__create`, not `__CreateHeap`. My earlier "instrumentation gap" concern is now closed. Good empiricism.
- **Heap-at-load closed.** Load + create on `Actor mng (DemoItm)`, cache MISS — legit actor heap, not a transient. My point #3 (wrong-heap first-run) is answered for the first run. (The `s_cached` repeat-run dangle is still latent but not this bug.)

**But the decisive fact is under-analyzed. I traced the error in-repo:**

- The fatal is [`shader.cpp:1198`](../../extern/aurora/lib/gx/shader.cpp:1198): `FATAL("unhandled tcg src {}", underlying(tcg.src))` — fires while Aurora compiles the **shader/pipeline** from a material's texgen config.
- Decoding `GXTexGenSrc` in [`GXEnum.h:321`](../../extern/aurora/include/dolphin/gx/GXEnum.h:321): POS=0 … COLOR0=**19**, COLOR1=**20**, **`GX_MAX_TEXGENSRC` = 21**.
- **`tcg src 21` is the enum's terminator sentinel, not a real texgen source.** One of the WW bow materials (`Vbow_v` or `SC_Vbow_v`) has a texgen slot whose `src` is **uninitialized / phantom** (count says the slot exists; its source was never populated). Aurora reads the J3DMaterial struct's texgen block to build the shader, hits src=21, aborts pipeline compile.

**Why this reframes the fix:**

1. **This is a material-config bug, not a draw-list-structure bug.** It's almost certainly because a **locked baked-DL (MDL3) model carries its real GX state in the display list, while the J3DMaterial struct texgen block is a stub** — so `nTexGen` counts a slot Aurora then finds has src=21. `DoBdlMaterialCalc` (load flags `0x59020010|0x2000`) is supposed to populate that; evidently it doesn't fully, on this asset.
2. **2G′ (drop shared DL) is a plausible *guess*, not a targeted fix.** It only helps if `newSharedDisplayList`/`makeSharedDL` is what leaves the texgen stubbed. **Do the diagnostic first** — it's cheaper than a code branch and tells you exactly what to change:

   **2I — Texgen dump (log-only, ~10 lines, NO render change): approve this before 2G′.** In `finishLoadedModelData` / `patchModelForPc`, for each material log: material name, `getTexGenNum()`, and each texgen's `src` (and `type`). Do it **both** right after `loadBinaryDisplayList` **and** after `makeSharedDL`. This pins:
   - *Which* material + texgen index holds src=21;
   - *Whether* the sentinel exists on load (⇒ asset/`DoBdlMaterialCalc` issue, fix in loader/flags or a real texgen patch) **or only after `makeSharedDL`** (⇒ *then* 2G′ is the right fix).

3. **`patchModelForPc` is the natural home for the fix and is currently a no-op logger** (as I flagged last review). Once 2I names the bad slot, the fix is a targeted material patch there: clamp `nTexGen` to valid slots, or set the offending texgen `src` to a sane value (e.g. `GX_TG_TEX0`) / disable that stage. That is a real, scoped change — not Plan B.

**Probable link to the boot crash (flag, not proven):** the boot `unhandled tcg src 21` is the **same sentinel**. Most likely the WW bad pipeline gets baked into the GPU pipeline cache and resurfaces at title/logo on the next launch — which is exactly why a cache wipe "fixes" boot but it keeps returning. Two implications: (a) a full cache wipe between WW-on runs isn't just hygiene, it's **required** to keep boot alive; (b) fixing the texgen sentinel at the material level should kill **both** crashes. **Confirm with a cold-cache WW-on run** — if `tcg src 21` reproduces from a freshly-wiped cache, it's genuinely the WW material (not stale cache).

**Answer to "2G′ only, or Run B then 2G′?": neither yet — run 2I (texgen dump) first.** It's log-only, one session, and decides whether the fix is a loader/flags change, a `patchModelForPc` texgen patch, or actually 2G′. Approving 2G′ blind risks a wasted session on a fix that may not touch the sentinel. Phase 3 (magenta/struct-0/TEV color) stays gated — this is a hard pipeline-compile abort upstream of any lighting question.

---

## Phase 3 — Material / TEV pass (Plan A — trial and error)

**Owner:** Cursor implements; user playtests; Claude prioritizes experiments  
**Exit:** Fill colors **readable**; edge pass **deferred** until fill stable.

### Known root cause (decomp-backed via TWW, not SS)

TP `daDitem_c` uses **`settingTevStruct(14)` + `setLightTevColorType_MAJI`** — built for TP get-item props (`O_gD_bow`), not WW cel BDLs. TWW demo items use **`TEV_TYPE_ACTOR` (0)** + normal `setLightTevColorType`.

### Experiment order (try in sequence; one variable per session)

> **Claude revision (2026-06-30):** run **3.0 isolate first** — confirms TEV vs texture before any struct-0 work.

| # | Experiment | Where | Success signal |
|---|------------|-------|----------------|
| **3.0** | **Isolate:** flat magenta DDS via texture replacement API on vbow embedded tex | AppData `texture_replacements/` | Still dark → TEV/lighting (go 3.1). Magenta wrong/missing → albedo/decode path — **don’t waste time on 3.1–3.3 until fixed** |
| 3.1 | WW bow: **delegate to `daItemBase_c::setTevStr`** (no demo override). Fallback: struct **0** + `setLightTevColorType` | `d_a_demo_item.cpp` | Fill no longer mud-dark |
| 3.2 | Draw list: mask-off vs TP dark list | demo item draw setup | Slight improvement in contrast |
| 3.3 | Keep `d_ww_itemmdl_pc` locked BDL; tune tev block flags only | `d_ww_itemmdl_pc.cpp` | Stable + brighter |
| 3.4 | **Do not** unlock materials for `diff()` until 3.0–3.3 exhausted | — | Avoid post-demo crashes |

**Not in scope:** SS decomp port; `loadFromResourcePointer()` fallback; whole-file git restores.

**Claude review prompt:** “Given screenshot after 3.1, is darkness TEV or texture? What’s the next single change?”

---

## Phase 4 — Edge pass + motion polish (Track A complete)

**Owner:** Cursor  
**Exit:** Track A shippable for bow get-item dev toggle.

| Step | Work |
|------|------|
| 4.1 | Re-enable **`SC_Vbow_v`** draw-time hide/show tuning — cel ink without white shards |
| 4.2 | Enable BTK spin: `dRes_INDEX_ITEMMDL_BTK_VBOW_e` (0x24) in create path after colors stable |
| 4.3 | Stress: load/unload `itemmdl`, repeated Editor replay, real cutscene get-item |
| 4.4 | Update progress table in `wind-waker-item-work.md` |

---

## Phase 5 — Track B held bow (later)

**Owner:** Cursor + optional asset tooling  
**Exit:** `vbow` parented to Link hand; aim/shoot works (stiff mesh OK).

| Step | Work |
|------|------|
| 5.1 | Toggle `game.wwItemmdlHeldBow` + branch `setBowModel()` in `d_a_alink_bow.inc` |
| 5.2 | Default **skip** TP bow BCK on swapped mesh |
| 5.3 | Tune hand matrix in `d_a_alink.cpp` **or** Plan B joint edit in external tool (GCFT) |
| 5.4 | Epona / left-hand path playtest |

---

## Phase 6 — Scale beyond bow (optional)

| Step | Work |
|------|------|
| 6.1 | ImGui itemmdl viewer (21 BDL indices) |
| 6.2 | Extend Phase 3 tev path to other `v*` meshes using TWW arc table in handoff doc |
| 6.3 | Field pickup row for bow |

---

## Plan B — Asset shortcut (escape hatch)

Use only after **3.0 + 3.1–3.3 exhausted** **and** user accepts non-retail asset path.

| Step | Modder workflow | Dusk integration |
|------|-----------------|------------------|
| B.1 | Unpack WW mesh from `vbow`; repack as **BMD** (not BDL) | Add optional dev arc or replace heap target |
| B.2 | Clone materials from `O_gD_bow` / TP get-item template | May “just work” with struct 14 |
| B.3 | Dummy toon texture for glossy cel | Material slot / TP ramp |
| B.4 | GCFT joint edits for held alignment | Track B primarily |

**Visual:** TP-styled WW shape — not pixel-faithful WW get-item.

---

## Agent pitfalls (mandatory)

| Do not | Why |
|--------|-----|
| `git show HEAD:file.cpp > file.cpp` in PowerShell | UTF-16 BOM corrupts sources |
| `git checkout`/`restore` **whole files** without user naming the file | Wiped WIP (demo.inc incident). **Named-file** `git restore` is OK when user specifies the path |
| Patch `d_a_alink_demo.inc` for message skip | Use `d_ww_itemmdl_test.cpp` |
| Hide **both** `Vbow_v` and `SC_Vbow_v` at create | Post–get-item crash correlation |
| Draw-time / create-time `SC_*` `shape->hide()` on shared arc data | First-draw crash on Aurora (Phase 2 fix removed; Phase 4 only) |
| `setLightTevColorType_MAJI` on WW locked `bdl3` | First-draw crash; WW bow uses struct 14 tev only until Phase 3 |
| Unlock BDL for `diff()` early | Crashes; stay locked + `DoBdlMaterialCalc` |
| Bisect boot without cache wipe first | False positives (`tcg src 21`) |

---

## Key files (quick index)

| Area | Path |
|------|------|
| Handoff / mesh table | `docs/wind-waker-item-work.md` |
| PC loader / materials | `src/d/d_ww_itemmdl_pc.cpp`, `include/d/d_ww_itemmdl_pc.h` |
| Editor replay | `src/d/d_ww_itemmdl_test.cpp` |
| Get-item actor | `src/d/actor/d_a_demo_item.cpp`, `src/d/actor/d_a_itembase.cpp` |
| Item resource table | `src/d/d_a_itembase_static.cpp` |
| Indices | `assets/GZ2E01/res/Object/itemmdl.h` |
| Build / cache | `docs/build-fps-guidelines.md` |
| Git rules | `docs/commit-and-push.md` |

---

## Cursor ↔ Claude workflow

1. **Cursor** runs the phase in this doc; updates “Session log” below when a phase completes or fails.
2. **User** playtests and attaches screenshot + debug log snippet.
3. **Claude** reviews: confirms experiment order, flags scope creep, suggests Plan B only if Plan A stuck.
4. **Neither agent commits** unless user asks ([commit-and-push](../commit-and-push.md)).
5. **Phase 2 crash:** Cursor **does not** continue implementation — classify log, update session log, wait for user + branch approval from **Phase 2 stability gate** (or Wind Clau).

**Cursor session opener (paste):**

> Read `docs/Interconnected Chats/Wind Curs-Wind Clau.md` and `docs/wind-waker-item-work.md`. Execute **roadmap Phase [N]** only. No whole-file git restore. Minimal diff. Do not commit.

**Claude session opener (paste):**

> Read `docs/Interconnected Chats/Wind Curs-Wind Clau.md`. Review Phase [N] plan or attached screenshot/log. Revise experiment order; do not suggest SS decomp.

---

## Session log (append-only)

| Date | Phase | Result | Notes |
|------|-------|--------|-------|
| 2026-06-27 | — | Roadmap created | WW track disconnected; baseline mesh achieved pre-bisect. |
| 2026-06-30 | — | Claude first pass | Review merged: canonical numbering, Phase 1 exit tweak, Phase 3.0 isolate first, delegate-first 3.1. Cursor ack below. |
| 2026-06-30 | — | Loop closed | Wind Clau confirm + execution queue agreed. Cursor idle until Phase 0/1 requested. |
| 2026-06-30 | **Phase 0** | **Pass (user)** | Boot/build verified on other work — title runs clean; no WW wiring yet. Cleared for **roadmap Phase 1**. |
| 2026-06-30 | **Phase 1** | **Done (Cursor)** | Re-linked cmake/settings/editor/d_s_play; replay auto-ends at 6 s. User playtest: toggle off inert, launch green. |
| 2026-06-30 | **Phase 2** | **Fix (Cursor)** | Crash on first WW draw: skip MAJI on locked bdl3, remove draw-time SC hide, no arc-resident model attach. User playtest pending. |
| 2026-06-30 | **Phase 2** | **Awaiting user** | Retest queued. If still crashes → Cursor stops coding; use **Phase 2 stability gate** section; Wind Clau picks branch 2A–2F. |
| 2026-06-30 | **Phase 2** | **Fail (user)** | Retest crashed post-`procCoGetItemInit returned`. MAJI/hide/arc-resident fixes insufficient. **Analysis + revised strategy** in doc; **no code** until branch approved. |
| 2026-06-30 | **Phase 2** | **Review (Wind Clau)** | Pushback accepted: narrow 2A insufficient; widened-2A + 2D next; drop-shared-DL before 2G/2H; heap/cache audit. Cursor response in doc. **Idle.** |
| 2026-06-30 | **Phase 2** | **Run A bracket** | Widened pins complete. Last: `draw: after setShadow`; `tcg src 21`. Load heap OK (`Actor mng DemoItm)` pass 1). **Recommend 2G′ drop shared DL.** Cursor **paused.** Run B pending. |
| 2026-06-26 | **Phase 2** | **Run A confirm (user)** | Re-ran Run A post-bracket build — **same crash signature**. Doc updated with ruled-out table + execution queue; **no code**. |
| 2026-06-26 | **Boot** | **Testing blocked (user)** | Continuous crash **without new edits**. Latest logs: `tcg src 21` at **logo** (line ~68), not field — poisoned GPU caches. First Cursor wipe **did not delete** `.db` / `-wal` files. |
| 2026-06-26 | **Config** | **WW off (Cursor)** | Set `"game.wwItemmdlGetItem": false` in `%AppData%\TwilitRealm\Dusklight\config.json` — user still crashed at logo (cache poison, not toggle). |
| 2026-06-26 | **Boot** | **Pass (user)** | **Full** cache wipe (all 6 files: `dawn_cache.db*`, `pipeline_cache.db*`) with game quit → title runs again. See **Boot cache poison handoff** below. |
| 2026-06-30 | **Boot** | **Cache (not WW)** | Startup `tcg src 21` at logo after full CMake rebuild — fixed by deleting `dawn_cache.db*` + `pipeline_cache.db*`. WW code never runs at boot. |
| 2026-06-30 | **Phase 2** | **Wind Clau review** | Verified ruled-outs are real. Crash NOT proven to be in `DrawBase` (instrumentation gap after `patchModel: done`). Recommend widened-2A + 2D together; suspect self-built shared DL + load-time heap. See review section. |
| 2026-06-30 | **Phase 2** | **Run A (Cursor)** | Widened bracket: CPU completes all DrawBase steps; fault on GPU submit = `unhandled tcg src 21`. Heap-at-load closed. |
| 2026-06-26 | **Phase 2** | **2M (Cursor)** | Removed `s_cached` + heap pass static; reload each spawn; 2K kept. Built + cache wiped. |
| 2026-06-26 | **Phase 2** | **2K playtest (user)** | First replay PASS; second FAIL (stale cache). |
| 2026-06-26 | **Phase 2** | **2I playtest (user)** | Texgen dump at frame 1371 — **no src=21** in material struct. Still GPU crash. |
| 2026-06-30 | **Phase 2** | **2M playtest (user)** | 1st replay **PASS** (6 s, screenshot). 2nd replay **FAIL** — load null deref (not stale cache). |
| 2026-06-30 | **Phase 2** | **2O playtest (user)** | Confirmed `same_as_prev=1`; spawn 2 parse fatal. Wind Clau: **2Q not 2B**. |
| 2026-06-30 | **Phase 2** | **2Q + 2P (Cursor)** | Retained heap cache; skip `DeleteBase("itemmdl")`; outline clear on delete. Built. |
| 2026-06-30 | **Phase 2** | **2Q playtest (user)** | **4/4 PASS** (2 per room × 2 rooms). **Room 3 FAIL** — draw crash, cache HIT. |
| 2026-06-30 | **Phase 2** | **2Q′ + 2R (Cursor)** | Draw-entry audit; cap-style private `itemmdl` mount; demo item skips shared `resLoad`. Built. |
| 2026-06-30 | **Phase 2** | **2R playtest (user)** | **2/3 rooms PASS** (once per room). Room 3 FAIL — post-`setShadow`, 2Q′ ptrs stable. |
| 2026-06-30 | **Phase 2** | **2R control retest (user)** | Room 3 **bow visible ~1 s** then crash — **not** instant first-frame fail. |
| 2026-06-30 | **Phase 2** | **Wind Clau review** | Aurora silent AV; Cap Wear ruled out; **2V approved** (pre-demo create burst). |
| 2026-06-30 | **Phase 2** | **2V (Cursor)** | Defer dev replay until play/room fpc idle + 30-frame stable settle. Built. |
| 2026-06-30 | **Phase 2** | **A→B→C→D playtest (user)** | **3/4 PASS** — rooms 1/0/3 full 6 s; **4th post-warp crash**. Pre-2V binary. |
| 2026-06-30 | **Phase 2** | **2V playtest (user)** | **3/4 PASS** with `2V: room load idle`; run 4 instant crash — 2V gate too narrow. |
| 2026-06-30 | **Phase 2** | **2V′ revert (Cursor)** | Over-broad `any room loading` blocked all replays; restored original 2V. Built. |
| 2026-06-30 | **Phase 2** | **2V retest (user, `194443`)** | **2/3 PASS** (rooms 1, 0); **room 3 FAIL** after wait — `2V: room load idle` then frame-1 post-`setShadow` silence. |
| 2026-06-30 | **Phase 2** | **Wind Clau correction (final)** | **Retract** event-timing / `room=-1` / room-id theories — all ambient noise. **Next:** debugger stack or **2U-lite** past `setShadow`; stop log correlation branches (2V″/2S deprioritized). |
| 2026-06-30 | **Phase 2** | **Stack trace (user)** | **Root cause:** `std::out_of_range` @ `shader_info.cpp:61` — `sampledTextures.set(0xFF)` when TEV stage uses TEXC/TEXA with unbound texmap; `CHECK` compiled out (NDEBUG). **Fix A (Cursor):** guard `.set()` in `shader_info.cpp`. **Fix B:** WW material → 2N. |
| 2026-06-30 | **Phase 2** | **Fix A gate playtest (user)** | **3/4** — rooms 1/0/3 `Replay finished`; run 4 post-warp **FATAL `tex255_size_bias`**. Prior session room 3 same FATAL. **2K always on** (auto; 2D isolate off). Fix A incomplete. |
| 2026-06-30 | **Phase 2** | **Fix A′ (Cursor)** | `shader.cpp` `is_texmap_sampled()` guards mirror Fix A; neutral sample when unbound. Rebuilt + caches wiped. |
| 2026-06-30 | **Phase 2** | **Fix A+A′ extended gate (user)** | **16+ replays, no crashes.** Color OK ~1–3; flicker **room 4**; colorless **5+**. Screenshots → Wind Clau. |

---

### 2026-06-26 — 2I texgen dump results + plan (Cursor; paused)

**User report:** Crash on WW bow get-item replay after 2I build (cold cache, toggle on). Aurora: `unhandled tcg src 21` (`dusklight-20260630-175645.log`).

#### Log evidence (frame 1371 — authoritative)

```
2I texgen [after loadBinaryDisplayList]: mat[0]=SC_Vbow_v nTexGen=3
2I texgen [after loadBinaryDisplayList]: mat[0] tg[0] type=1 src=4    (MTX2x4, TEX0)
2I texgen [after loadBinaryDisplayList]: mat[0] tg[1] type=0 src=1    (MTX3x4, NRM)
2I texgen [after loadBinaryDisplayList]: mat[0] tg[2] type=10 src=19  (SRTG, COLOR0)
2I texgen [after loadBinaryDisplayList]: mat[1]=Vbow_v nTexGen=2
2I texgen [after loadBinaryDisplayList]: mat[1] tg[0] type=1 src=4    (MTX2x4, TEX0)
2I texgen [after loadBinaryDisplayList]: mat[1] tg[1] type=10 src=19  (SRTG, COLOR0)
… [after makeSharedDL]: identical values — no PHANTOM tags, no src=21 …
… bracket pins through draw: after setShadow …
(no Replay finished)
```

**Decode:** Aurora `shader.cpp` handles src 1 (NRM), 4 (TEX0), 19 (COLOR0) and type 10 (SRTG). **No logged slot has src=21 (`GX_MAX_TEXGENSRC`).**

#### What 2I ruled out

| Hypothesis | 2I result |
|------------|-----------|
| Phantom texgen **in J3DMaterial struct** at load (`DoBdlMaterialCalc` stub) | **Ruled out** — all slots 0..nTexGen−1 have valid src |
| **`makeSharedDL` introduces** phantom texgen in struct | **Ruled out** — dump identical before/after |
| **2G′ drop shared DL** as primary fix | **Deprioritized** — no struct change to target |
| **`patchModelForPc` clamp/set texgen src** from load-time dump | **Deprioritized** — nothing to clamp from dump |
| Load / heap / DrawBase CPU | Still **ruled out** (Run A + 2I bracket) |

#### Revised understanding (for Wind Clau)

`tcg src 21` is still **`GX_MAX_TEXGENSRC`** at Aurora shader compile ([`shader.cpp:1198`](../../extern/aurora/lib/gx/shader.cpp:1198)), but **not visible in `J3DMaterial::getTexCoord()`** at load time. Aurora builds shaders from **runtime GX pipeline state** replayed from the display list + per-frame material path — not from the J3D struct alone.

Most likely: **`mDoExt_modelUpdateDL`** on locked `bdl3` runs **unlock → update → lock**, rebuilding or extending GX texgen state so a **sampled texcoord slot** (possibly index ≥ logged `nTexGen`, or from dual-material + struct-14 TEV) hits default `tcg.src = GX_MAX_TEXGENSRC` before GPU submit. Wind Clau’s load-time stub theory was **falsified by 2I**; the bug is **runtime DL / draw-path desync**.

Boot cache poison remains a **separate symptom** (bad pipeline cached after field crash) — Cursor wipes caches before each WW playtest build.

#### Recommended plan (one branch per approved session)

| Priority | Branch | Hypothesis | Minimal change | Pass signal |
|----------|--------|------------|----------------|-------------|
| **1 (recommended)** | **2J — Skip `modelUpdateDL`** | Locked WW `bdl3` should draw baked DL; per-frame `update()` desyncs GX tcg | WW bow + toggle in `daDitem_c::draw` / `DrawBase`: skip `mDoExt_modelUpdateDL(mpModel)`; keep `loadSharedDL` path | `Replay finished (6 s).` |
| **2** | **2K — Body mat only** | Dual draw (`SC_Vbow_v` + `Vbow_v`) + struct-14 TEV references extra texcoord | WW bow demo draw: render **`Vbow_v` only** (one material / shape) | Stable spin (outline wrong OK) |
| **3** | **2I′ — TEV texcoord dump** | TEV stages reference texcoord ID ≥ `nTexGen` | Log-only: per-material TEV order `mTexCoord` vs `nTexGen` at draw entry | Names orphan coord ref |
| **4** | **2G′ — Drop shared DL** | Weaker after 2I | Skip `newSharedDisplayList`/`makeSharedDL` | Draw survives |
| **5** | **2F — Plan B** | Plan A draw exhausted | Modder BMD + TP materials | Stable silhouette |

**Do not:** Phase 3 TEV/magenta yet; load-time texgen patch from 2I data (nothing to patch).

#### Execution queue (post–2I)

| Step | Owner | Action | Status |
|------|-------|--------|--------|
| 1–5 | — | Widened bracket + 2I | **Done** |
| 6 | User | 2I playtest | **Done — fail, data captured** |
| 7 | **Wind Clau / user** | Approve **2K** + Aurora index log | **Done** |
| 8 | **Cursor** | Implement 2K + index log + build + cache wipe | **Done** |
| 9 | User | First 2K playtest | **Done — PASS (once)** |
| 10 | User | Second replay | **Done — FAIL (stale cache)** |
| 11 | **Wind Clau / user** | Approve **2M** (reject 2L′) | **Done** |
| 12 | **Cursor** | 2M + build + cache wipe | **Done** |
| 13 | User | Repeat replay ×3 | **Partial — 1/3 pass** |
| 14 | **Wind Clau** | Review 2M playtest + screenshot; approve **one** branch below | **Next** |
| 15 | **Cursor** | Execute approved branch only | Blocked |

**Cursor gate:** No code until Wind Clau approves **2M′**, **2B**, or **2O**.

---

### 2026-06-30 — 2M playtest results (user; Cursor analysis; paused)

**User report:** Played once — **worked** (~6 s spin, WW bow visible, **screenshot captured**). Second replay in same session → **crash**.

**Build / logs:** `v1.4.1-89-dirty` · `dusklight-20260630-182442.log` · `Documents/dusklight/albw_ww_itemmdl_debug.txt`

#### Log evidence (2M session only — ignore older `cache HIT` / `mats=1376` lines above frame 899 in the append-only file)

| Run | Frames | Key lines | Result |
|-----|--------|-----------|--------|
| **1st replay (PASS)** | 916–1096 | `getVbow: load (no cache)` · `mats=2 joints=2 tex=3` · 2I texgen clean · many `2K: suppress SC_*` · `Replay finished (6 s).` | **Phase 2 partial pass** — mesh visible, stable 6 s |
| **2nd replay (FAIL)** | 1239–1255 | `getVbow: load (no cache)` · `before load` · **silence** (no texgen dump, no `patchModel`) | **Load crash** — not stale `s_cached` |

**Aurora crash (2nd spawn, pid=204):**

```
EXCEPTION_ACCESS_VIOLATION (0xc0000005)
Fault addr: 0x0
```

No `unhandled tcg src 21` FATAL — fault is **CPU null deref during `CreateItemHeap` load**, before first draw of the second item.

#### What 2M fixed vs what remains

| Issue | 2M result |
|-------|-----------|
| Stale `s_cached` / `mats=1376` on 2nd spawn | **Fixed** — 2nd run logs `load (no cache)`, never `cache HIT` |
| Repeatable 6 s spin (3× gate) | **Not met** — new second-spawn load crash |
| Phase 3 (TEV / magenta) | **Still gated** — need repeatable replay first |

#### Interpretation (Cursor)

1. **2K + 2M first spawn path is validated** — user screenshot is the Phase 2 baseline deliverable for Wind Clau.
2. **Second crash is a different bug class** from pre-2M stale cache and from Run A GPU `tcg src 21`. Debug log stops inside `getVbowModelData` → `loadFromResourcePointer` / `loadBinaryDisplayList` (after `before load`, before `2I texgen [after loadBinaryDisplayList]`).
3. **Likely causes (ranked — pick ONE after Wind Clau review):**

| Priority | Branch | Hypothesis | Minimal change | Pass signal |
|----------|--------|------------|----------------|-------------|
| **1 (recommended)** | **2M′ — Heap pass audit** | DEBUG `entrySolidHeap_` may measure→`freeAll()`→re-call; second spawn may hit two-pass path while logs only show one `CreateItemHeap: enter` if crash is immediate inside load on pass 2 | Log **callback pass 1 vs 2** at `CreateItemHeap` enter; log immediately before/after `loadBinaryDisplayList` | Second replay reaches `patchModel: mats=2` + `Replay finished (6 s).` |
| **2** | **2B — Private heap copy** | Arc `getObjectRes` bytes or Aurora parse alias across delete/reload cycles | `JKRReadIdxResource` → temp buffer → `loadBinaryDisplayList` (never attach arc-resident `J3DModelData*`) | Same |
| **3** | **2O — Arc / res pointer log** | `DeleteBase("itemmdl")` + next `resLoad` returns bad/null raw pointer on 2nd spawn | Log `getObjectRes` ptr + magic at load; fail closed if not `J3D2`/`bdl3` | Pinpoints bad resource vs loader |
| **4 (hygiene)** | **2P — Clear `s_suppressedOutlineShape` on delete** | Static shape pointer may outlive freed demo item (draw hygiene; unlikely root of *load* crash) | `daDitem_c::Delete` → `dWwItemmdl_clearOutlineSuppress()` | Prevents stale draw static; combine with above if cheap |

4. **Do not** drop **2K** or jump to **2N** / Phase 3 until the **3× replay gate** passes.

#### User deliverables for Wind Clau

- [x] Screenshot — WW bow get-item spin (first replay)
- [x] Debug log tail — frames 916–1096 (pass) + 1239–1255 (fail)
- [x] Aurora log — `dusklight-20260630-182442.log` (null deref stack, not tcg FATAL)

#### Wind Clau prompt

> 2M **partial pass**: 1st replay 6 s + screenshot; 2nd replay crashes at **load** (`getVbow: load (no cache)` then silence; `EXCEPTION_ACCESS_VIOLATION` fault addr 0x0). Stale `s_cached` is gone. Approve **one** branch: **2M′** (heap pass + load pins), **2B** (private buffer load), or **2O** (res pointer audit)? Phase 3 still gated.

#### Execution queue (post–2M playtest)

| Step | Owner | Action | Status |
|------|-------|--------|--------|
| 13 | User | Repeat replay ×3 | **Partial — 1/3** |
| 14 | **Wind Clau / user** | Review screenshot + this section; approve **one** branch | **Next** |
| 15 | **Cursor** | Execute approved branch; build + cache wipe | Blocked |
| 16 | User | Re-run 3× replay gate | After step 15 |
| 17 | **Wind Clau** | Then **2N** or defer ink Phase 4; open Phase 3 | After step 16 |

**Cursor gate:** No implementation until **Execute [branch]** from user or Wind Clau.

---

### 2026-06-26 — Wind Clau 2I response + 2K approval (Cursor implements)

**Wind Clau verdict on 2I:** Load-time stub theory **falsified** (correct). Root cause refined: **TEV samples texcoord slot `i` but runtime `numTexGens ≤ i`**, so unprogrammed slot reads sentinel `src=21` (`gx.hpp:157`, `gx.cpp:759`, `shader.cpp:1159`).

**Approved (same session):**
| # | Branch | Owner | Status |
|---|--------|-------|--------|
| 1 | **2K** — draw `Vbow_v` body only; skip `SC_Vbow_v` edge (`suppressOutlineForDraw` in demo draw) | Cursor | **Done** |
| 2 | **Aurora index log** — fatal at `shader.cpp:1198` adds `tcg index` + `numTexGens` | Cursor | **Done** |
| 3 | Read 2K + index log → pick fix (likely `GXSetNumTexGens` or defer ink Phase 4) | Wind Clau | After playtest |
| — | **2J deprioritized** (#3) until 2K localizes material | — | — |

**Built:** `build_run.bat` OK. GPU caches wiped. `game.wwItemmdlGetItem: true`.

**Playtest:** Replay bow get-item → paste `2K:` line from debug log + Aurora fatal line if crash (now shows index/numTexGens) or `Replay finished (6 s).`

---

### 2026-06-26 — 2K playtest results + 2L′ plan (Cursor; paused)

**User report:** First replay **worked** — saw WW bow spinning (~6 s). Changed rooms, replayed → **crash**.

#### Log evidence

| Run | Frame | Key lines | Result |
|-----|-------|-----------|--------|
| **1st replay (PASS)** | 1564–1744 | `cache MISS` · `mats=2 joints=2 tex=3` · many `2K: suppress SC_*` · `Replay finished (6 s).` | **Phase 2 partial pass** — mesh visible, stable 6 s |
| **2nd replay (FAIL)** | 2495 | `CreateItemHeap: enter pass 2` · `getVbow cache HIT` · `patchModel: mats=1376 joints=24800 tex=0` | Use-after-free / stale `s_cached` |

**Interpretation:**

1. **2K confirms Wind Clau:** Skipping `SC_Vbow_v` (3 texgens) avoids the runtime `numTexGens`/TEV mismatch; body-only draw is stable when model data is valid.
2. **Second crash is NOT texgen regression** — it is **`s_cached` + heap callback pass 2**, exactly as Wind Clau flagged in the widened bracket review (`entrySolidHeap_` can `freeAll()` and re-call; static `s_cached` survives pointing at freed memory).
3. Static `s_heapCallbackPass` in `CreateItemHeap` **never resets** — second spawn logs pass 2 immediately and returns cache HIT without reload.
4. Aurora index log not reached on 2nd run — crash is corrupt `J3DModelData*` before sane GPU draw (stack trace in latest log, not `tcg src 21`).

#### Recommended plan (one branch next)

| Priority | Branch | Action | Pass signal |
|----------|--------|--------|-------------|
| **1 (recommended)** | **2L′ — Fix `s_cached` lifetime** | (a) On cache HIT, sanity-check `mats==2 && joints==2`; fail → invalidate + reload. (b) Invalidate cache when `CreateItemHeap` callback pass > 1. (c) Reset heap pass counter per demo-item spawn (or at replay start). **Keep 2K edge suppress.** | Repeated replays (same room + after room change) all reach `Replay finished (6 s).` |
| **2** | **2M — No static cache** | Remove `s_cached`; reload `vbow` each spawn (simplest, small cost) | Same |
| **3** | **2N — Proper texgen (Wind Clau step 4)** | After 2L′ stable: re-issue `GXSetNumTexGens` / per-slot texgens so **both** mats can draw; defer ink to Phase 4 | Both mats visible, no crash |
| — | Phase 3 TEV/magenta | Still gated on **repeatable** 6 s spin | — |

**Do not:** Drop 2K yet; revert to dual-material draw until 2L′ passes repeat playtest.

#### Execution queue (post–2K)

| Step | Owner | Action | Status |
|------|-------|--------|--------|
| 9 | User | First 2K playtest | **Done — PASS (once)** |
| 10 | User | Second replay / room change | **Done — FAIL (stale cache)** |
| 11 | **Wind Clau / user** | Approve **2M** (reject 2L′) | **Done** |
| 12 | **Cursor** | Remove `s_cached` + pass static; keep 2K; build + cache wipe | **Done** |
| 13 | User | Repeat replay ×3 (same room, replay again, after room change) | **Partial — 1/3** |
| 14 | **Wind Clau** | Review 2M playtest + screenshot; approve **2M′ / 2B / 2O** | **Next** |
| 15 | **Cursor** | Execute approved branch | Blocked |

**Cursor gate:** No code until Wind Clau approves next branch (see **2026-06-30 — 2M playtest results**).

---

### 2026-06-30 — Wind Clau 2O approval + in-place fixup theory (Cursor implements 2O)

**Milestone:** Screenshot confirms WW Hero's Bow mesh in get-item pose — Phase 2 baseline is visually real. **2M fixed stale cache** (`load (no cache)`, no `mats=1376`).

**New crash (spawn 2 only):** `ACCESS_VIOLATION @ 0x0` during load after arc delete/reload — **not** GPU `tcg src 21`.

**Root cause (Wind Clau):** `J3DModelLoader::loadBinaryDisplayList` binds `mpRawData = i_data` ([`J3DModelLoader.cpp:179`](../../libs/JSystem/src/J3DGraphLoader/J3DModelLoader.cpp:179)), walks blocks by reading sizes from the arc buffer ([`:229`](../../libs/JSystem/src/J3DGraphLoader/J3DModelLoader.cpp:229)), then **`indexToPtr()` in-place fixup** ([`:236`](../../libs/JSystem/src/J3DGraphLoader/J3DModelLoader.cpp:236)). Spawn 1 rewrites arc offsets → pointers into spawn-1's freed heap. Spawn 2's `resLoad` returns the **same mutated buffer** → second parse treats resolved pointers as offsets → wild pointer → null deref before texgen dump. Magic bytes untouched → guard passes → crash inside parse. **Exactly the observed signature.**

**Branch override:**

| Branch | Verdict |
|--------|---------|
| **2M′** (heap two-pass) | **Wrong primary** — spawn 1 passed both passes; failure is strictly cross-spawn (arc buffer lifecycle) |
| **2O** (resource-buffer audit) | **Execute now** — log-only confirm |
| **2B** (private pristine copy) | **Fix after 2O confirms** — copy must live on item solid heap as long as model (`mpRawData` draw lifetime) |
| **2P** (clear `s_suppressedOutlineShape` on delete) | **Bundle with 2B** — same dangling-static class |

#### Execution queue (Wind Clau sequencing)

| Step | Owner | Action | Status |
|------|-------|--------|--------|
| 1 | **Cursor** | **2O** log-only: `getObjectRes` ptr, hdr16, blk0, before/after `loadBinaryDisplayList`, `same_as_prev` | **Done** |
| 2 | User | Playtest: 1st + 2nd replay; paste `2O` lines | **Next** |
| 3 | **Wind Clau** | Read 2O → confirm/fix | After step 2 |
| 4 | **Cursor** | **2B + 2P** (private buffer on item heap + outline clear on delete); keep **2K** | Blocked |
| 5 | User | **3× replay gate** | After step 4 |
| 6 | **Wind Clau** | **2N** → Phase 3 | Gated on step 5 |

**2O log grep:** lines starting with `2O` — compare spawn 1 `#1` vs spawn 2 `#2`: `same_as_prev=1` + `size_looks_resolved=1` on spawn 2 pre-parse ⇒ double-fixup confirmed.

---

### 2026-06-30 — Wind Clau 2O results: re-parse confirmed fatal, but NOT header fixup → do 2Q (parse-once-retain), not 2B

Read the 2O lines (spawn #1 frame 1260, spawn #2 frame 1559). **Half my theory confirmed, half corrected — being explicit:**

| Signal | Prediction | Actual | Verdict |
|--------|-----------|--------|---------|
| Same buffer reused across spawns | `same_as_prev=1` | **`same_as_prev=1`** (ptr `…D8777940` both) | ✅ confirmed |
| First parse resolves header offsets in place | `size_looks_resolved=1` on spawn 2 | **`size_looks_resolved=0`**, `hdr16` byte-identical, magic `J3D2/bdl4` intact | ❌ **not** confirmed |
| 2nd parse of same buffer | crash | **crash inside `loadBinaryDisplayList serial=2`** | ✅ confirmed |

**What this means:** re-parsing the same resource buffer is **definitely fatal** (same pointer, 2nd parse dies), but the corruption is **not** in the header/INF1 — those stay pristine. So it's **either an in-place mutation of a *deeper* block** (SHP1/MDL3 baked-DL — which 2O didn't inspect) **or J3D/shared-DL loader-global state** keyed on the resource. Either way: **do not re-parse.**

**This changes the fix — 2B is now the wrong call:**
- **2B (copy-then-parse each spawn) is unsafe here.** If spawn 1 mutated a deeper block in place, by spawn 2 the buffer is *already dirty*, so copying it *now* copies dirt → still crashes. 2B only works if you snapshot **pristine** bytes at *first* load (before any parse) and keep that master — and it still pays a re-parse every spawn and re-runs the exact op that crashes.
- **2Q (parse once, retain, reuse) is the robust fix — and 2O proves it's viable.** Because `same_as_prev=1` (the itemmdl arc + resource pointer are **stable/resident** across spawns), we can: **parse the `vbow` model exactly once onto a *persistent* heap (not the per-item `Actor mng (DemoItm)` heap), cache the `J3DModelData`, keep the arc resident, and have each spawn only `mDoExt_J3DModel__create`-wrap the cached model.** No second parse → no crash, in *both* the deeper-mutation and loader-global scenarios. This is the vanilla-parity architecture (vanilla items get an arc-resident modelData loaded once). It's why the original `s_cached` was *the right instinct, wrong heap* — the model was on the transient item heap; put it on a retained heap + keep the arc, and caching becomes correct.
  - **Requirements:** (a) load/keep the `itemmdl` arc in a slot that isn't freed by the demo item's `DeleteBase("itemmdl")`; (b) allocate the parsed model on a retained heap; (c) each spawn wraps, never reparses; (d) `mpRawData` stays valid because the arc stays resident.

**Recommendation: Execute 2Q, not 2B.** Bundle **2P** (clear `s_suppressedOutlineShape` on delete — still needed). Keep **2K**.

**Separate issue — "bow tip not drawing" is expected, not a new bug.** 2K suppresses the **`SC_Vbow_v`** shape, and the bow tip geometry is bound to that material — so hiding `SC_Vbow_v` hides the tip, not just an ink outline. Its texgens are individually valid (`tg0 TEX0`, `tg1 NRM`, `tg2 SRTG/COLOR0`); the only reason it crashed is the runtime `numTexGens` shortfall. So **2N** (re-issue `GXSetNumTexGens(3)` + per-slot texgens for `SC_Vbow_v` so no TEV stage samples an unprogrammed slot) is what brings the **tip back** *and* the ink pass — after 2Q makes the spin repeatable. Don't chase the tip before 2Q + 3× gate.

**Sequencing:** 2Q + 2P (keep 2K) → 3× replay gate → 2N (restores tip/edge) → Phase 3 colors. Phase 3 still gated.

**2Q implemented (Cursor):** `d_ww_itemmdl_pc.cpp` — `WW itemmdl vbow` retained heap (1 MB), parse once → `s_cachedVbow`, spawn wraps via cache HIT (no re-parse). `d_a_demo_item.cpp` — skip `DeleteBase("itemmdl")` when WW toggle on; **2P** `dWwItemmdl_clearOutlineSuppress()` on delete.

| Step | Owner | Action | Status |
|------|-------|--------|--------|
| 1 | **Cursor** | **2Q + 2P** | **Done** |
| 2 | User | **3× replay gate** | **Partial — 4/5 (room C fail)** |
| 3 | **Wind Clau** | Approve **2Q′** / **2R** | **Next** |
| 4 | **Cursor** | Execute approved branch | Blocked |

**Playtest grep:** one `2Q: cache MISS — parse once` per session; replays `2Q: cache HIT`; all runs `Replay finished (6 s).`

---

### 2026-06-30 — 2Q playtest results (user; Cursor analysis; paused)

**User report:** Four successful replays total — **two in each of two rooms**. Third room, first replay → **crash**. (“Progress!” — cross-spawn re-parse bug is gone.)

**Log:** `dusklight-20260630-184521.log` (no crash footer flushed) · `albw_ww_itemmdl_debug.txt` (2Q session from frame 939)

#### Results table (2Q session)

| # | Room (inferred) | Frames | Cache | Result |
|---|-----------------|--------|-------|--------|
| 1 | Room A | 956→1136 | MISS → parse OK | **`Replay finished (6 s).`** |
| 2 | Room A | 1223→1403 | HIT | **`Replay finished (6 s).`** |
| 3 | Room B | 1707→1887 | HIT | **`Replay finished (6 s).`** |
| 4 | Room B | 2115→2295 | HIT | **`Replay finished (6 s).`** |
| 5 | **Room C** | 2788→? | HIT | **FAIL** — log stops after first draw |

Room boundaries inferred from frame gaps (1403→1691, 2295→2772) and new demo-item heap pointers (`89E148A0` → `89DA4630` → `89D7E690`).

#### What passed (2Q win)

- **One parse per session:** `2Q: cache MISS` once (frame 956); `2Q: parse OK heap=… model=… mats=2 joints=2`
- **Replays 2–4:** `2Q: cache HIT` — **no second `loadBinaryDisplayList`** (re-parse bug fixed)
- **Same-room repeat:** ✅ (runs 1–2)
- **Cross-room (first hop):** ✅ (runs 3–4 after room B load)

#### What failed (run 5 — room C)

| Stage | Evidence |
|-------|----------|
| Load / heap | `2Q: cache HIT` · `mats=2 joints=2 tex=3` · `J3DModel__create OK` · `procCoGetItemInit returned` |
| First draw (CPU) | All widened bracket pins through **`draw: after setShadow`** (frame 2788) |
| Tail | **Silence** — no frame 2789 `2K:` lines, no `Replay finished` |

**Not** the old spawn-2 load null-deref (`loadBinaryDisplayList serial=2`). **Not** stale `s_cached` (`mats=1376`). Crash window is **post–first-draw** (frame 2789+ draw loop, GPU submit, or dangling `mpRawData`/texture after room C load).

Aurora log for this session ends at demo-item create (pid 453) with **no `CRASHED` block** — likely hard fault before crash reporter flushed.

#### Interpretation (Cursor)

1. **2Q core hypothesis confirmed:** parse-once + cache HIT eliminates the fatal re-parse. This is real progress vs 2M.
2. **3× gate not met:** need same-room ×2 + room-change ×1 **all green**. Room C broke the room-change leg.
3. **Likely cause class:** cached `J3DModelData` still references **`itemmdl` bytes/textures via `mpRawData` + global `dComIfG` object-res slot**, while **room/stage transition remaps or invalidates that slot** even though we skip demo-item `DeleteBase("itemmdl")`. Runs 3–4 worked in room B — room C may be the first load that **re-mounts or frees** the shared object-res `itemmdl` entry (refcount skew from 4× `resLoad` / 0× `resDelete`, or stage `deleteObjectResMain` sweep).
4. **2K / tip:** unchanged — missing tip on `SC_Vbow_v` suppress is expected until **2N** after gate passes.

#### Recommended plan (one branch — Wind Clau to confirm)

| Priority | Branch | Hypothesis | Change | Pass signal |
|----------|--------|------------|--------|-------------|
| **1 (audit)** | **2Q′ — Room + arc pointer audit (log-only)** | Confirm `getObjectRes("vbow.bdl")` ptr / archive alive differs on room C vs parse time | Log room id + raw res ptr on every cache HIT; log on room change; compare to `s_vbowRawPtr` from parse | Room C fail shows ptr NULL or ptr ≠ parse-time before crash |
| **2 (fix, recommended)** | **2R — Independent `itemmdl` mount (ALBW cap pattern)** | Global object-res `itemmdl` is not stable across all room loads; cap-style private `dRes_info` + retained heap isolates arc + parse from stage churn | Mirror [`d_albw_sumo_test.cpp`](../src/d/d_albw_sumo_test.cpp) `ensureIndependentCap`: private `setRes`/`syncRes`/`getRes` for `itemmdl`, parse once on `WW itemmdl vbow` heap; demo item **stops** `resLoad`/`DeleteBase` on shared slot when WW toggle on | **3× gate green** including room C |
| **3 (fallback)** | **2Q″ — Cache invalidate on room change** | Stale cache after arc remap | Clear `s_cachedVbow` when room id changes | **Insufficient alone** if remap leaves unparseable buffer — only pairs with **2R** or pristine snapshot |
| — | **2N / Phase 3** | Tip + colors | After 3× gate | — |

**Do not:** Re-open **2B** (copy-then-parse). Do not drop **2K** until **2N**. Do not start Phase 3 colors.

#### 3× replay gate checklist

- [x] Same room ×2 (runs 1–2)
- [x] Room change ×1 — partial (runs 3–4 in room B after leaving A)
- [ ] **Room change ×2+ stable** (room C failed on run 5)
- [ ] All → `Replay finished (6 s).`

#### Execution queue (post–2Q playtest) — superseded by post–2R queue above

| Step | Owner | Action | Status |
|------|-------|--------|--------|
| 1 | User | 2Q playtest (4 OK, room 3 fail) | **Done** |
| 2 | **Wind Clau** | Approve **2Q′** + **2R** | **Done** |
| 3 | **Cursor** | **2Q′ + 2R** | **Done** |
| 4 | User | Multi-room gate A→B→C→D | **Partial (2/3)** |
| 5 | **Wind Clau** | Review logs + 2R diff → **2N** | After gate green |

---

### 2026-06-30 — 2Q′ + 2R implemented (Cursor)

**2Q′:** `dWwItemmdl_log2QPrimeAudit()` — tags `cache-hit`, `parse-ok`, `heap`, `draw-pre-dl`; fields per Wind Clau (room, cached `mpRawData`, shared vs private live ptr, arc resident flags, `raw_ne_shared`, `raw_eq_private`).

**2R:** Private `s_privateItemmdlInfo` + `dRes_control_c::setRes`/`syncRes`; parse from private `getRes` only; `dWwItemmdl_stepPrivateItemmdlArcLoad()` replaces shared `resLoad("itemmdl")` in demo item + replay helper; **2P** + skip shared `DeleteBase` unchanged; **2K** kept.

**Multi-room gate:** A→B→C→D sequential, all `Replay finished (6 s).` Log grep: one `2Q: cache MISS`; `2Q'` `raw_eq_private=1` on draws post-2R.

---

### 2026-06-30 — 2R playtest results (user; Cursor analysis; paused)

**User report:** Once per room, two rooms OK, **crash on third** (Link may have been in combat in room 3 — unconfirmed).

**Logs:** `dusklight-20260630-185730.log` (no `CRASHED` / `FATAL` footer — hard fault or log not flushed) · `albw_ww_itemmdl_debug.txt` (2R session from frame 723)

#### Results table (2R session)

| Run | Frame | Room | Cache | Outcome |
|-----|-------|------|-------|---------|
| 1 | 723→920 | 1 | MISS → HIT | **`Replay finished (6 s).`** |
| 2 | 1366→1562 | 0 | HIT | **`Replay finished (6 s).`** |
| 3 | 1940→1956 | 3 | HIT | **FAIL** — log stops after `draw: after setShadow` |

#### What passed (2R win vs pre-2R)

- **Private arc stable across rooms:** one `2R: private itemmdl arc ready` (frame 739); parse once `raw=…BF60`; all later spawns `2Q: cache HIT`.
- **2Q′ refutes shared-arc dangle on room 3:** at crash, `raw_eq_private=1`, `raw_ne_shared=0`, `parse_match=1`, `private_arc=1`, `shared_live=0` — cached `mpRawData` **matches** live private `vbow.bdl` ptr.
- **CPU draw path complete:** every bracket pin through `after modelUpdateDL` and `after setShadow` on frame 1956 (same signature as pre-2R room C, but **without** stale shared buffer).

#### Room 3 crash window (frame 1956)

```
2Q: cache HIT
2Q' [heap] room=3 … raw_eq_private=1 parse_match=1
procCoGetItemInit returned
draw: entry → … → before modelUpdateDL
2Q' [draw-pre-dl] room=3 … raw_eq_private=1 parse_match=1
draw: after modelUpdateDL → after setShadow
(silence — no frame 1957, no Replay finished)
```

#### Aurora context (room 3 only)

- `frameInterp: onTypeChange 31 -> 40` (enter get-item event) → shop/room scene churn → `40 -> 31` (leave event) → demo item `pid=451` created.
- Rooms 1–2 used different event-type sequences (room 2: `41→40→41→188` around demo spawn) and **did not crash**.
- **Combat hypothesis unconfirmed:** type **31** on room 3 may correlate with Link not fully idle; needs control replay **out of combat** before treating as root cause.

#### Mechanism update (Wind Clau)

| Theory | Pre-2R room C | Post-2R room 3 |
|--------|---------------|----------------|
| Shared `itemmdl` remount → stale `mpRawData` | **Likely** (2Q alone) | **Refuted** (2Q′ ptr audit) |
| Re-parse on cache HIT | Ruled out (2Q) | Still ruled out |
| Post-CPU GPU fault (present / texture) | Suspected | **Still primary** — ptrs valid, crash after `setShadow` |
| Link proc / combat / event overlap | Not logged | **Plausible** — room 3 event churn + user combat note |

**2R did its job:** isolate arc + parse from stage `resLoad` churn. Multi-room gate **still not met** — failure class shifted from **CPU stale buffer** to **GPU / runtime state**.

#### Ranked next branches (one per approved session)

| Priority | Branch | Hypothesis | Minimal change | Pass signal |
|----------|--------|------------|----------------|-------------|
| **0 (control)** | User retest | Crash is combat/timing, not room index | Room 3 replay **out of combat**, no enemies | Room 3 `Replay finished` without code change |
| **1** | **2S — Replay gate** | `procCoGetItemInit` + get-item draw conflicts with combat / event proc | Block dev replay when `event_runCheck`, Link not idle, or proc ≠ field idle; log reason | Room 3 in-combat → clean skip; out-of-combat → pass |
| **2** | **2T — GPU texture residency** | Private arc bytes stable but **GX tex objs** from parse time invalidated after room GPU churn | Log / re-bind TEX1 from private arc on room change or before first draw; optional `initTexObj` audit | Room 3 `Replay finished`; no post-`setShadow` silence |
| **3** | **2U — Crash-context log trim** | Per-frame 2Q′ spam hides signal | First draw frame only + Link proc id, event type, `event_runCheck` at spawn | Next fail names state without drowning log |

**Do not:** stack 2S+2T in one diff. **Do not** start **2N** or Phase 3 until multi-room gate green.

#### Execution queue (post–2R playtest)

| Step | Owner | Action | Status |
|------|-------|--------|--------|
| 1 | User | 2R multi-room playtest (2/3 pass) | **Done** |
| 2 | **Wind Clau** | Pick **2S** vs **2T** vs control-first | **Next** |
| 3 | User | Optional control: room 3 out of combat | **Suggested** |
| 4 | **Cursor** | Execute approved branch only | Blocked |
| 5 | **Wind Clau** | Review → **2N** | After gate green |

**Cursor gate:** No **2N** / Phase 3 until ≥3–4 sequential distinct rooms all `Replay finished (6 s).`

---

### 2026-06-30 — 2R control retest (user; Cursor analysis; paused)

**User report:** Room 3 **played and appeared** on the third replay, but **still crashed** (no clean 6 s finish).

**Logs:** `dusklight-20260630-190213.log` · `albw_ww_itemmdl_debug.txt` (session from frame 705)

#### Results table (control session)

| Run | Frame span | Room | Outcome |
|-----|------------|------|---------|
| 1 | 705→902 | 1 | **`Replay finished (6 s).`** (~197 frames) |
| 2 | 1203→1399 | 0 | **`Replay finished (6 s).`** (~196 frames) |
| 3 | 1745→1821+ | 3 | **PARTIAL** — ~**60 draw frames** (1761–1821), bow visible; log stops mid-spin, no `Replay finished` |

#### What changed vs prior 2R session (185730)

| | Session 185730 (room 3) | Session 190213 (room 3) |
|--|-------------------------|-------------------------|
| Draw frames before silence | **1** (frame 1956 only) | **~60** (1761–1821) |
| User-visible bow | Unknown / likely instant crash | **Confirmed visible + animating** |
| 2Q′ ptr audit | Stable (`raw_eq_private=1`) | Stable throughout all 60 frames |
| Aurora tail | Demo create → silence | `fapGm_Execute frame=1800` after demo create — game ran into replay |

**Interpretation:** Room 3 failure is **non-deterministic** and **timing-dependent**, not a fixed “first GPU submit always faults” bug. The mesh/load path **works** in room 3; something else kills the process mid-replay (~⅓ of the 6 s window at ~33 fps).

#### Room 3 concurrent activity (Aurora)

Same pattern as prior fail: `31 → 40 → 31` event transitions; **shop items** (`pid=445–448`) and **room scenes** (`449–450`) loading immediately before demo item `pid=451`. Runs 1–2 lacked this density of concurrent creates. Suggests **race with room-3 stage churn**, not pure “stale TEX1 on frame 1.”

#### Mechanism update

| Theory | Prior read | After control retest |
|--------|------------|----------------------|
| Instant GPU fault on first present | Primary | **One mode** — still seen in 185730 |
| Stale `mpRawData` / shared arc | Refuted (2R+2Q′) | Still refuted — 60 stable draws |
| **Mid-replay teardown / concurrent load race** | Secondary | **Elevated** — delayed crash + shop/room spawns |
| Link combat-only | Plausible | **Weakened** — bow ran visibly; crash is mid-demo not pre-demo |
| Pure first-frame texture invalidation | 2T candidate | **Weaker** — would not explain 60 good frames |

#### Ranked next branches (revised)

| Priority | Branch | Hypothesis | Minimal change | Pass signal |
|----------|--------|------------|----------------|-------------|
| **1** | **2V — Room idle gate** | Room 3 crash when replay fires during shop/room actor load | Defer replay until no pending `fpc` creates / room load complete; log block reason | Room 3 `Replay finished` |
| **2** | **2S — Link/event gate** | Get-item proc still conflicts with residual event state (31↔40 churn) | Block when `event_runCheck` or Link proc ≠ field idle | Clean skip or pass |
| **3** | **2U — Teardown bracket** | Crash on demo delete / replay end / concurrent delete | Log bracket around demo item `Delete`, replay timeout, `procCoGetItem` exit | Last line names teardown step |
| **4** | **2T — GPU texture re-bind** | Delayed fault when room GPU re-inits textures mid-spin | Re-upload TEX1 before draw on room change | All 3 rooms finish 6 s |

**Gate status:** Multi-room gate **still not met** (no room 3 `Replay finished`), but **functional milestone** — WW bow **renders in room 3**. Phase 3 / **2N** still blocked.

#### Execution queue (updated)

| Step | Owner | Action | Status |
|------|-------|--------|--------|
| 1 | User | Control retest (room 3 visible, crash mid-spin) | **Done** |
| 2 | **Wind Clau** | Pick **2V** vs **2S** vs **2U** | **Done → 2V** |
| 3 | **Cursor** | **Execute 2V** | **Done** |
| 4 | User | A→B→C→D gate with 2V build | **Partial (3/4)** — pre-2V binary |
| 5 | User | Rebuild + retest run 4 (post-warp) | **Next** (exe now fresh) |
| 6 | **Wind Clau** | Review → **2N** | After gate green |

---

### 2026-06-30 — 2V implemented (Cursor)

**File:** `src/d/d_ww_itemmdl_test.cpp` — in `REPLAY_PENDING`, after editor-close debounce (15 frames):

- Block while play scene or Link's room scene has pending creates (`fpcNd_IsCreatingFromUnder`) or queued create requests (`fpcCtIt_Judge` on layer id).
- Block while `dStage_roomControl_c::getRoomReadId() >= 0` or room status flags `0x02|0x04` (loading).
- Require **30 consecutive idle frames** before preload/spawn.
- Log once: `2V: defer — <reason>`; on proceed: `2V: room load idle — preloading demo item archive`.
- Timeout after 900 frames: `"Timed out waiting for room load to finish"`.

**Playtest grep:** room 3 should show `2V: defer — play scene pending creates` (or room variant) if replay pressed during load burst; then `2V: room load idle` before demo spawn; all three rooms → `Replay finished (6 s).`

**Optional free confirmation (user, no code):** replay room 3 **after** load burst settles, or real chest/event bow get-item — if 6 s clean, Phase 2 is functionally met for legit get-items.

---

### 2026-06-30 — A→B→C→D playtest (user; Cursor analysis; paused)

**User report:** Three rooms OK, warped to fourth — bow **played and appeared**, then crash (likely room load).

**Logs:** `dusklight-20260630-192743.log` · `albw_ww_itemmdl_debug.txt` (session from frame 616)

**Build note:** Aurora showed build `16:34:04` — **stale pre-2V binary** (no `2V:` in log). Cursor should have run `build_run.bat` immediately after implementing 2V; **now built** (see changelog). Retest required.

#### Results table

| Run | Frames | Room | Outcome |
|-----|--------|------|---------|
| 1 | 616→813 | 1 | **`Replay finished (6 s).`** (~197 frames) |
| 2 | 1109→1305 | 0 | **`Replay finished (6 s).`** (~196 frames) |
| 3 | 1892→2088 | 3 | **`Replay finished (6 s).`** (~196 frames) — **room 3 gate met** |
| 4 | 2751→2788+ | 1 (post-warp) | **PARTIAL** — ~21 draw frames; bow visible; silent crash |

#### What changed

- **Room 3 is now a full pass** (1908→2088, exact ~180-frame replay window). Prior sessions crashed mid-spin or frame-1 in room 3 — this confirms WW bow works when timing is favorable (likely waited for shop burst to settle).
- **Run 4 = post-warp load race** — Aurora shows `fopScnM_CreateReq` + room scene `creating_id=467` + massive resource load (`model.bmd` 778 KB, `@bg*.arc`, etc.) immediately before demo item `pid=530`. Same signature as pre-2V room-3 burst, but triggered by **warp into new area**, not shop.
- **WW model still clean at crash:** `raw_eq_private=1 parse_match=1` through frame 2788.

#### Gate status

| Criterion | Status |
|-----------|--------|
| A→B→C distinct rooms, all `Replay finished` | **Met** (rooms 1, 0, 3) |
| D (fourth room post-warp) | **Fail** |
| 2V build exercised | **No** in that session — stale binary; **built since** |

**Phase 2 functional read:** Core WW bow path works across three rooms including former problem room 3. Remaining failure is **dev replay during stage/warp load** — exactly what 2V targets.

#### Next steps

| Step | Owner | Action |
|------|-------|--------|
| 1 | User | **`build_run.bat`** — must pick up 2V code; grep log for `2V:` | **Done (Cursor)** |
| 2 | User | Repeat A→B→C→D; on run 4 wait ~2 s after warp before replay | **Next** |
| 3 | **Wind Clau** | If run 4 still fails **with** `2V: room load idle` before spawn → **2V′** (warp-specific: longer settle, block while any room loading / PLAY_SCENE recreate) |
| 4 | Gate green → **2N** → Phase 3 | All |

---

### 2026-06-30 — Wind Clau 2R-control review: 2R is SOLVED; get 2 free data points before 2V/2S/2U

**2R is a confirmed success — say it plainly.** The log shows every frame 1761→1821, *including the crash frame 1821*, with `cached_mpRawData=…11643F60 parse_raw=…11643F60 private_live=…11643F60 raw_eq_private=1 parse_match=1`. The private arc pointer is rock-stable and matches the parse-time pointer right up to death. **The arc/model/pointer-lifetime bug class is fully closed.** WW bow rendering + animating in a third room is a real milestone.

**Therefore the frame-1821 crash is provably NOT the WW model.** Pointers perfect at the crash frame ⇒ this rules out 2T (texture re-bind) and every remaining model-lifetime theory. The debug log only proves *our* stuff is fine and then stops — it does **not** say what faulted.

**Two free data points must come before building 2V/2S/2U — they decide whether this is even a WW-bow bug:**

1. **The Aurora fault line (address + module/function/stack).** The `…190213.log` tail Cursor quoted (`fapGm_Execute frame=1800`) is the game loop, not the fault. The actual Aurora `[FATAL]`/exception line names *what* died — event manager vs actor-mng vs GPU vs room-3 code. This single line likely picks the branch for us. **Paste it.**
2. **A settled-room-3 get-item.** The Editor replay is a **dev harness firing get-items at arbitrary times** — here, *during* room 3's load burst (shop `pid=445–448`, room scenes `449–450`, then demo `451`). A **real** get-item (chest/event) never fires mid-room-load. So test the distinction: trigger a real bow get-item in room 3 **after it has fully settled** (or just wait for the load burst to finish, then replay).
   - **Stable ⇒ this is a replay-during-load race, i.e. a harness artifact — and Phase 2 is functionally MET** (WW bow works in legit get-items across rooms). 2V (defer replay until idle) becomes a small dev-tool-robustness fix, not a core-feature fix.
   - **Still crashes ⇒ real event/room interaction** (the `31→40→31` event churn colliding with the get-item event sequence ~60 frames in) — higher stakes, affects real gameplay, and 2S (event gate) / proper event-slot handling is the real fix.

**Branch pick: don't build blind — let the two data points choose.** My prior: it's the replay-during-load race (delayed crash + the load-burst correlation both fit), so **2V is the likely fix and Phase 2 is likely already met for real get-items.** But:
- If the Aurora fault is in the **event manager** → **2S** (event/`event_runCheck` gate) is more targeted than 2V.
- If it's a **GPU/present** fault → different track (and 2V still helps by not spawning mid-load).
- **2U (teardown/context bracket)** is only worth it if the Aurora line is uninformative — it's a fallback diagnostic, not a first move.

**Recommendation:** Post the Aurora fault line + run the settled-room-3 get-item test **first** (both free, no code). Then I'll pick 2V vs 2S in one shot. If you want to move without waiting, **2V** is the safe bet — it can't hurt (dev replay *should* wait for room idle regardless) and likely turns the gate green.

**Gate reminder:** A→B→C→D multi-room, all `Replay finished (6 s)`. Keep 2K; 2N (restores tip) and Phase 3 still gated.

#### Addendum — Wind Clau read both logs directly (`…190213.log` + debug)

Read the Aurora log end-to-end. Findings, some correcting my own prior asks:

1. **There is no Aurora fault line.** No `[FATAL]`, no `tcg src`, no assert/exception/access-violation message anywhere. The only `ERROR` is a benign `rodan.bdf` font-load warning (line 54); the `error=4294967295` lines are `-1`/"no error" sentinels in normal create-request prints. The log **truncates mid-frame** right after the demo-item create — a **silent access violation, unflushed**. So "get the Aurora fault line" was a dead end — there's nothing to read. Concretely: **not** a GX/shader/texgen crash this time (2K's `SC_Vbow_v` suppression is doing its job).
2. **Cap Wear (`ALBW-CAP hat …`) is NOT the culprit — it's active in all three runs, including the two that finished 6 s.** It appears next to *every* demo-item create (pid 203 run 1, pid 264 run 2, pid 451 run 3). I briefly suspected a Cap-Wear × get-item-demo interaction (there's a known paused demo-BCK cap crash), but the log refutes it: present in the successes too, so it's not the differentiator. Dropping that lead.
3. **The real differentiator is exactly what Cursor said — room-3's load burst.** Run 3's demo item (`pid=451`, line 3294) is created *immediately after* a wave of scene/actor creates — shop items `445–448`, room scenes `fpcNdRq creating_id=449` (line 3108) and `450` (line 3147). Runs 1–2 (`pid 203/264`) have no comparable create burst right before the demo. This is the **mid-load race** signature, cleanly.
4. **The debug log proves the WW model is valid to the last frame** (`raw_eq_private=1 parse_match=1` at frame 1821). So the crash is external to the WW bow — consistent with a race against room-3 stage churn, not our model.

**Verdict: 2V (defer replay until room load idle) is the right branch.** The log localizes the differentiator to the pre-demo create burst, and there's no fault pointing elsewhere. The one remaining free confirmation: **replay in room 3 *after* the load burst settles (or a real chest/event get-item)** — if that finishes 6 s, the crash is purely the dev-replay-firing-mid-load race, **2V fixes it, and Phase 2 is functionally met for real get-items.** No need to wait on any Aurora fault (there isn't one).

Verified the room-C crash in the log (frame 2788): `2Q: cache HIT` → `mats=2 joints=2 tex=3` → `J3DModel__create OK` → `__CreateHeap OK` → **every CPU draw pin through `after setShadow`** → silence (no frame 2789, no `Replay finished`). Cursor's read is correct: this is **post-CPU-draw** (GPU-async or next frame), and the cached `J3DModelData` *object* is intact (`mats=2` reads fine — but note that number comes from the **retained object**, not from `mpRawData`, so it tells us nothing about the backing bytes).

**Sharper mechanism than "remaps the slot":** `daDitem_c::create()` calls `dComIfG_resLoad("itemmdl")` **every spawn** (`d_a_demo_item.cpp:532`). 2Q parses the model **once** (room A) and caches it with `mpRawData` bound to **room-A's arc buffer address**. On room C, `resLoad` re-mounts `itemmdl` at a **different address** (or the room-A mount was swept), but **cache HIT skips re-parse, so `mpRawData` is never re-pointed** — it still aims at freed room-A memory. CPU draw builds/submits a DL over that stale VTX1/TEX1 pointer; the **GPU reads freed memory on present → fault after `setShadow`**. Rooms A/B surviving is address-reuse luck; room C is the first remount to a new address. This is precisely the "keep the arc resident" caveat from my 2Q review — 2Q retained the *model* but bound it to a *shared-mount* buffer whose address the stage system owns.

**Why the fix must be 2R, not a cache tweak:** the cached parse is bound to a buffer whose address/lifetime the object-res/stage system controls. **2R (private `dRes_info` + retained heap, ALBW cap pattern) gives the arc a stable address for the model's whole life, and the demo item stops touching the shared slot** — immune to room churn. This matches the project's established independent-resource convention (`ensureIndependentCap` in `d_albw_sumo_test.cpp`). 2Q″ (invalidate cache on room change → re-parse) is a fragile band-aid: it only works if each room's `resLoad` yields pristine bytes, and it re-introduces the per-spawn re-parse we removed. **Endorse 2R; skip 2Q″.**

**Answer to "2Q′ then 2R?" — yes, both, in that order:**

1. **2Q′ (log-only, decisive) — approve.** Cursor's plan is right; one refinement so it's airtight: log **at draw entry** (right before `modelUpdateDL`), not only at cache HIT — record room id, the cached `mpRawData`, a **live** `getObjectRes("itemmdl","vbow.bdl")` ptr, and whether `dComIfG_getObjectResInfo("itemmdl")` is still resident. **Confirmation:** on room C, cached `mpRawData` ≠ live ptr (or arc info NULL) before the crash. This also *refutes* the theory cheaply if the ptrs match — in which case it's a texture/GPU-residency issue, not arc dangle, and 2R would be the wrong fix. Worth the one run before the 2R refactor.
2. **2R (fix) — approve, gated on 2Q′ confirming.** Private mount + parse-once on a retained heap; demo item skips `resLoad`/`DeleteBase` on the shared `itemmdl` slot when the WW toggle is on.

**Gate correction:** the old "3× gate" was really *same-room ×2 + one transition* — room C proves that's too weak. **New gate: replay in ≥3–4 *distinct sequential* rooms (A→B→C→D), all `Replay finished (6 s)`.** The bug only surfaces on the 2nd+ transition.

**Unchanged:** keep 2K (tip stays missing until 2N — expected); 2N and Phase 3 remain gated on the corrected multi-room gate.

---

### 2026-06-26 — Wind Clau 2M approval (rejects 2L′)

**Verdict:** 2K win stands. Second crash = `s_cached` dangle (confirmed). **2L′(a) rejected** — validating by dereferencing freed memory is UB. **Execute 2M:** remove `s_cached` + `s_heapCallbackPass`; reload `vbow` each spawn; keep 2K.

**Playtest gate:** Three replays must pass — first, second same room, after room change — all `Replay finished (6 s).`

**Next after 2M:** Wind Clau reviews diff (`resLoad == cPhs_COMPLEATE_e` before load); then **2N** (GXSetNumTexGens for dual mat) or defer edge to Phase 4. Phase 3 gated on repeatable 6 s.

#### Wind Clau prompt

> 2K **passed once** (6 s, mesh seen). Second replay: `cache HIT`, `mats=1376` — stale `s_cached` after heap pass 2. Approve **2L′** cache invalidation + keep 2K? Then step 4 texgen programming for dual mat?

---

### 2026-06-26 — Wind Clau 2K/2L review: do 2M (remove cache), NOT 2L′

2K is a real win and confirms the texgen mechanism. The second crash is the `s_cached` dangle I flagged in the first bracket review — Cursor's diagnosis is correct. **But the priority is inverted: 2L′ is unsound; 2M is the right fix.** Verified against source.

**Why 2L′(a) — "sanity-check `mats==2 && joints==2` on cache HIT" — is unsound:** it **dereferences a freed pointer to validate it**. Reading `s_cached->getMaterialNum()` after the backing heap is freed is undefined behavior — the memory may be (i) reused by something whose bytes coincidentally pass `mats==2/joints==2`, so you then use a genuinely-invalid model, or (ii) unmapped, so the *validation read itself* faults. You cannot validate a pointer by inspecting the storage it points at once that storage's lifetime has ended. This lowers crash frequency at best; it is not a fix.

**Why the cached model is *doubly* dead after the first item deletes** (confirmed in code):
- The `J3DModelData` is allocated in `CreateItemHeap` on the item's solid heap (`Actor mng (DemoItm)`), `freeAll`'d on delete.
- The **source `itemmdl` arc itself** is deleted on delete: `daDitem_c::Delete()` → `DeleteBase("itemmdl")` ([`d_a_demo_item.cpp:488`](../../src/d/actor/d_a_demo_item.cpp:488)). So the raw `bdl3` bytes `loadBinaryDisplayList` parsed are *also* freed.

So there is **no safe way to reuse `s_cached` across spawns** — both the model and its source bytes are gone.

**Why 2M (remove `s_cached`, reload each spawn) is correct *and* simpler:**
- `create()` already `resLoad`s the arc every spawn ([`d_a_demo_item.cpp:532`](../../src/d/actor/d_a_demo_item.cpp:532)), so `getObjectRes("itemmdl", …)` returns valid bytes on each spawn — reload always works.
- Reload-per-spawn matches the arc's per-spawn load/delete lifecycle exactly; each `CreateItemHeap` call parses fresh onto whatever heap is current at that call.
- It is **robust to the two-pass `entrySolidHeap` sizing** (measure→`freeAll`→real): each pass loads fresh onto its own heap, so there is no stale pointer and **no need for 2L′'s `s_heapCallbackPass` bookkeeping** (drop that fragile static too).
- **Caching buys ~nothing:** since the arc is deleted on every delete, the next spawn must `resLoad` it anyway (the expensive part). Re-parsing a 2-material bow off already-resident bytes is trivial. The cache added a use-after-free for zero real benefit.

**Recommendation:** **Execute 2M** (remove `s_cached` + the pass-counter static), **keep 2K** edge suppress. Do **not** ship 2L′(a)'s dereference-to-validate. One caveat to verify in the diff: ensure `resLoad("itemmdl")` has completed (`cPhs_COMPLEATE_e`) before `getVbowModelData` runs on **every** spawn, not just the first (it already gates via `mPhase` in `create()` — just confirm the WW path can't reach `CreateItemHeap` pre-load).

**Optional later (2P, not now):** if per-spawn reload ever measures as costly, load the model once onto a *retained* heap and keep the arc resident — then caching is safe. Adds arc-lifetime management; skip until there's a measured reason.

**Sequencing after 2M repeats reliably (same room + after room change):** → **2N** (re-issue `GXSetNumTexGens` + per-slot texgens so both materials draw / bring the edge pass back) → **Phase 3** colors. Phase 3 stays gated on a *repeatable* 6 s spin.

---

### 2026-06-30 — Wind Clau 2M-playtest review: it's the in-place DL fixup — do 2O→2B, NOT 2M′

**Milestone:** screenshot confirms the WW Hero's Bow mesh renders in the get-item pose. 2M fixed the stale-cache path (spawn 2 now logs `load (no cache)`, no `mats=1376`). Good.

**The new crash is a different bug and I traced it in the loader.** `EXCEPTION_ACCESS_VIOLATION @ 0x0` during `CreateItemHeap` load on **spawn 2 only**, after the arc was deleted (`DeleteBase("itemmdl")`) and re-`resLoad`ed. Root mechanism, confirmed in [`J3DModelLoader.cpp`](../../libs/JSystem/src/J3DGraphLoader/J3DModelLoader.cpp):

- `loadBinaryDisplayList` sets `mpModelData->mpRawData = i_data` ([`:179`](../../libs/JSystem/src/J3DGraphLoader/J3DModelLoader.cpp:179)) — the model **binds to the arc resource buffer**.
- It walks blocks by reading `block->mBlockSize` straight out of that buffer ([`:229`](../../libs/JSystem/src/J3DGraphLoader/J3DModelLoader.cpp:229)) and finishes with `mpModelData->indexToPtr()` ([`:236`](../../libs/JSystem/src/J3DGraphLoader/J3DModelLoader.cpp:236)) — **in-place index→pointer fixup on the resource data.**

⇒ **This is the classic "second parse of already-fixed-up display-list data" crash.** Spawn 1 converts the arc buffer's file-relative offsets into absolute pointers (into spawn-1's now-freed heap). If spawn 2's `resLoad` hands back the **same buffer already mutated** (cached, not re-decompressed) rather than pristine bytes, the second `loadBinaryDisplayList` walks already-resolved pointers as if they were offsets → wild `mBlockSize` / dangling pointer → null deref, **before** the texgen dump. Magic (`J3D2`/`bdl3`) is untouched by fixup, so `loadFromResourcePointer`'s magic guard passes and we crash inside the parse — exactly the observed signature.

**Why 2M′ is the wrong primary:** its hypothesis is the intra-spawn two-pass `entrySolidHeap` (measure→`freeAll`→re-call). But **spawn 1 passed both its passes** — if two-pass sizing were the fault, spawn 1 would crash too. The failure is strictly **cross-spawn** (after arc delete/reload) ⇒ resource-buffer lifecycle, not heap sizing. (2M′'s `loadBinaryDisplayList` pins are still useful instrumentation — fold them into 2O below.)

**Recommended: 2O (confirm) → 2B (fix).**

- **2O — resource-buffer audit (log-only, one run).** On every spawn/pass log: the `getObjectRes` pointer value, the first ~16 bytes (magic + the INF1 block size/first offset), and pins immediately before/after `loadBinaryDisplayList`. **Tell-tale:** if spawn-2's buffer address matches spawn-1's *and* the bytes after the magic differ from a fresh load (offsets already look like absolute pointers), the in-place-double-fixup theory is confirmed.
- **2B — parse from a private, pristine copy (the fix).** Copy the raw `vbow` file into a buffer **we own**, and run `loadBinaryDisplayList` on the copy so fixup always sees clean file offsets. **Critical lifetime caveat:** because `mpRawData` points into that buffer and *draw* reads vertex/DL data from it, the copy must live **as long as the model** — allocate it on the **item's solid heap** (freed with the model on delete), NOT a temp buffer freed after parse. Done naively (temp buffer) it fixes the load crash but crashes at draw instead.

**Deprioritized:** 2M′ (contradicted by spawn-1 pass); 2P — clearing `s_suppressedOutlineShape` on delete is **not** the load root cause (it's a draw static), but it *is* the same dangling-static class as `s_cached` and will bite at draw on spawn 2 once load is fixed — **bundle it as hygiene with 2B**, don't run it alone.

**Proper long-term (2Q, optional):** mirror vanilla — parse the `vbow` model **once**, resident with the arc, and have each spawn only `mDoExt_J3DModel__create`-wrap it (no reparse, like `dComIfG_getObjectRes` items). Eliminates the reparse hazard entirely, but needs the arc + model to persist across spawns (bigger change). 2B is the minimal correct fix now.

**Recommendation: Execute 2O + the loadBinaryDisplayList pins in one session; expect it to confirm in-place double-fixup; then 2B (private buffer, model-lifetime) bundled with 2P hygiene. Keep 2K. 3× replay gate unchanged. Phase 3 gated.**

---

### 2026-06-26 — Wind Clau 2I review: mechanism nailed (approve 2K + Aurora index log, NOT 2J-first)

2I was decisive and **my load-time stub theory is falsified** — owning that. Static material texgens are clean (`SC_Vbow_v`: TEX0/NRM/COLOR0; `Vbow_v`: TEX0/COLOR0; valid srcs, identical before/after `makeSharedDL`). Nothing to patch in `patchModelForPc` from the static struct, and **2G′ is correctly dead** (`makeSharedDL` never touched the texgen struct).

**I traced the exact fault in Aurora — mechanism is now unambiguous:**

- `TcgConfig::src` **defaults to `GX_MAX_TEXGENSRC` = 21** ([`gx.hpp:157`](../../extern/aurora/lib/gx/gx.hpp:157)).
- Aurora copies runtime GX texgen state into the shader config **only for `numTexGens` slots** ([`gx.cpp:759`](../../extern/aurora/lib/gx/gx.cpp:759)); any slot `i >= numTexGens` keeps the sentinel **21**.
- The fatal loop touches **only texcoord slots a TEV stage samples** (`info.sampledTexCoords.test(i)`, [`shader.cpp:1159`](../../extern/aurora/lib/gx/shader.cpp:1159)) → `FATAL` at `:1198`.
- **⇒ Root cause: a TEV stage samples texcoord slot `i`, but the runtime `numTexGens` programmed at draw is `≤ i`, so that slot was never filled and reads sentinel 21.** A **runtime numTexGens / TEV-texcoord mismatch** — exactly the runtime-GX-state family Cursor now suspects, and NOT any dumped src (TEX0/NRM/COLOR0/SRTG are all handled; the fault is a *phantom slot beyond the count*).

**Re-ranked branches:**

1. **2K (draw `Vbow_v` body only, skip edge) = #1.** `SC_Vbow_v` has 3 texgens vs `Vbow_v`'s 2 — the more texgens, the likelier one is unprogrammed when the runtime count is stale/low, so the edge material is the prime suspect. 2K is a pure draw-scope bisect, no DL-path change; if body-only survives, culprit is localized to `SC_Vbow_v` and skipping the edge pass is already the Phase-4 plan.
2. **Add the faulting index to Aurora's fatal — 1-line, log-only.** At [`shader.cpp:1198`](../../extern/aurora/lib/gx/shader.cpp:1198), include `i` **and** `g_gxState.numTexGens` (ideally the `sampledTexCoords` bits). One run confirms the faulting slot exceeds runtime `numTexGens`. Run alongside 2K.
3. **2J (skip `mDoExt_modelUpdateDL`) = #3, with a trap:** if bypassing `modelUpdateDL` means the texgens / `GXSetNumTexGens` are *never* programmed, it'll still fault (or worse). Only meaningful if the direct baked-DL entry issues the full texgen setup. Do it after 2K localizes.
4. **2I′** should log **runtime `numTexGens` + TEV texcoord refs** (not the static struct again) — largely subsumed by the index log in #2.

**Likely fix (pending 2K + index log):** reconcile `numTexGens` with the TEV texcoord refs for the WW bow at draw — explicitly re-issue `GXSetNumTexGens(nTexGen)` + per-slot texgens before the WW draw, or (if `SC_Vbow_v` is the sole offender) skip the edge pass now and defer ink to Phase 4. Scoped Plan-A work, not Plan B — and it probably also kills the boot `tcg src 21` (same sentinel poisoning the pipeline cache).

**Recommendation: approve 2K + the Aurora index-log together, in one session. Do NOT lead with 2J.** Phase 3 stays gated.

---

## Claude review / revisions

<!-- Wind Clau: append review comments, revised experiment order, or Plan A vs B recommendation here. -->

### 2026-06-30 — Phase 2 retest fail (Cursor analysis; awaiting Wind Clau)

**User report:** Crash persists through stability fix **and** Run A widened bracket. **No code changes** unless a branch is approved.

**Findings:** Widened bracket localizes fault to **post–CPU draw / GPU submit**, not heap-at-load or DrawBase CPU steps. Top suspect: self-built shared DL in `finishLoadedModelData`. See **Run A bracket analysis** below.

**Recommended next step:** Optional **Run B** (2D isolate); then approve **2G′ drop shared DL** (~2 lines in `d_ww_itemmdl_pc.cpp`).

**Cursor:** **paused** — idle until user or Wind Clau approves **2G′** (or Run B then 2G′).

---

### 2026-06-30 — Run A bracket analysis (Cursor; paused)

**User report (2026-06-30):** Run A **still crashes** — same strategy: plan + doc only, no implementation.

Wind Clau’s widened bracket **worked** — it overturns the old “fault is inside DrawBase CPU” assumption. Run A shows **every** DrawBase step including `modelUpdateDL` and `setShadow` logging **`after`** on CPU; crash is **`tcg src 21`** immediately after (GPU replay of locked WW `bdl3` + self-built shared DL from `finishLoadedModelData`). Heap-at-load hypothesis **closed** (actor solid heap, single callback pass, cache MISS).

#### Ruled out by Run A bracket

| Hypothesis | Evidence |
|------------|----------|
| Fault before `J3DModel__create` | Pin: `after J3DModel__create OK` |
| Fault in `__CreateHeap` | Pin: `__CreateHeap OK` |
| Fault before first `execute` / `draw` | Pins: `execute: first frame`, `draw: entry` |
| Fault inside any DrawBase CPU call | All `before`/`after` pairs complete, including `modelUpdateDL`, `setShadow` |
| Wrong heap at load | `Actor mng (DemoItm)` throughout load + create; pass 1 only |

#### Narrowed to

Post–first-draw **GPU submit** of WW model built with `finishLoadedModelData` (`d_ww_itemmdl_pc.cpp` ~114–119): `newSharedDisplayList(UseSingleDL)` + `makeSharedDL()` after per-material `J3DMaterialAnm` attach.

#### Recommended plan (one branch per session)

| Priority | Branch | Action | Pass signal |
|----------|--------|--------|-------------|
| **0 (optional)** | **Run B — 2D isolate** | WW toggle ON + **2D isolate ON** → replay | 6 s OK ⇒ fault in WW mesh/loader config; still crashes ⇒ broader demo draw path |
| **1 (recommended fix)** | **2G′ — Drop shared DL** | Skip `newSharedDisplayList` + `makeSharedDL` in `finishLoadedModelData` (~2 lines); keep `MaterialAnm` + `DoBdlMaterialCalc` for now | `Replay finished (6 s).` |
| **2** | **2G″ — Slim loader** | Also skip `MaterialAnm` attach if 2G′ insufficient | Draw survives |
| **3** | **2H — WW draw override** | Locked entry / body-mat-only draw | Mesh visible |
| **4** | **2C — Skip shadow** | Only if bracket ever pins `setShadow` (Run A did **not**) | Stable spin |
| **5** | **2F — Plan B** | Modder BMD + TP materials | Stable silhouette |

**Cursor gate:** No source edits until user or Wind Clau approves **2G′** (or Run B then 2G′). No Phase 3.

---

### 2026-06-26 — Boot cache poison handoff (for Wind Clau)

**Summary:** User could not playtest after Run A because **every launch** crashed at logo — even with WW toggle off and no new source edits. Root cause was **poisoned Aurora GPU shader caches**, not a WW boot regression. After a **complete** cache wipe, boot is green again. WW get-item work remains blocked until **2G′** is approved and implemented.

#### Two different `tcg src 21` crashes (do not conflate)

| Crash site | When | Last log context | WW code runs? | Fix |
|------------|------|------------------|---------------|-----|
| **A — Logo / title** | ~5–10 s after launch | `cDyl_InitCallback: fpcNm_LOGO_SCENE_e created, DONE` → immediate FATAL | **No** — toggle off; no itemmdl heap | Delete **all six** cache files (see below); quit game first |
| **B — Field / get-item** | Bow get-item replay, WW toggle **on** | Widened bracket ends at `draw: after setShadow` → FATAL | **Yes** | **2G′** drop shared DL (Phase 2); may **also** poison caches for crash A on next launch |

Crash **B** can write a bad pipeline entry that makes crash **A** happen on **every subsequent boot** until caches are fully wiped. Turning off `game.wwItemmdlGetItem` prevents crash B but **does not** clear already-poisoned caches.

#### Timeline (2026-06-30 sessions, build v1.4.1-89-dirty)

| Log file | Boot past logo? | Notes |
|----------|-----------------|-------|
| `dusklight-20260630-153602.log` (v87) | **Yes** | Control — normal logo flow |
| `dusklight-20260630-170818.log` (v89) | **Yes** | Proves **v89 is not a permanent boot regression** |
| `dusklight-20260630-170918.log` (v89) | Yes, then field crash | WW get-item replay → `tcg src 21` at Demo_Item create (~line 1600) |
| `dusklight-20260630-171740.log` (v89) | **No** | WW toggle **false** in config; still dies at logo line ~68 — poisoned cache |
| After full wipe (user confirm) | **Yes** | User: **"It runs"** |

Aurora log signature (logo crash):

```
[DEBUG | dusk] cDyl_InitCallback: fpcNm_LOGO_SCENE_e created, DONE
[FATAL | aurora::gfx::gx] unhandled tcg src 21
```

Healthy boot continues with `fapGm_Execute frame=0`, `dScnLogo_c::create`, etc. — see `170818` vs `171740`.

#### Why the first cache wipe failed

Cursor’s first wipe reported success but **`dawn_cache.db` and `-wal` sidecars remained on disk** (timestamps unchanged; Aurora log still showed `Using dawn cache at ...\dawn_cache.db`). SQLite WAL mode keeps poison alive if **`-wal` / `-shm` survive** or the game still holds the DB open.

**Mandatory procedure** ([`build-fps-guidelines.md`](../build-fps-guidelines.md) § Addendum):

```powershell
Get-Process dusklight -EA SilentlyContinue | Stop-Process -Force -EA SilentlyContinue
$dir = "$env:APPDATA\TwilitRealm\Dusklight"
Remove-Item "$dir\dawn_cache.db*" -Force -ErrorAction SilentlyContinue
Remove-Item "$dir\pipeline_cache.db*" -Force -ErrorAction SilentlyContinue
```

Verify **zero** `*cache*.db*` files remain before relaunch. First launch after wipe is slower (cache rebuild).

#### Config change (still in effect)

Cursor set in `%AppData%\TwilitRealm\Dusklight\config.json`:

```json
"game.wwItemmdlGetItem": false
```

User should leave this **off** for normal play until **2G′** is built and playtested. Re-enable via Editor (Advanced → ALBW) or config when resuming Phase 2.

#### Phase 2 state (unchanged by boot incident)

| Item | Status |
|------|--------|
| Run A (widened bracket) | **Fail** — all CPU draw pins pass; GPU fault after `setShadow` |
| Run B (2D isolate) | **Not run** — blocked while boot was broken; still optional fork |
| Heap-at-load | **Ruled out** (Run A: `Actor mng (DemoItm)`, pass 1, cache MISS) |
| Recommended next | **2I** texgen dump (log-only) → then targeted fix; **2G′** only if sentinel appears after `makeSharedDL` |
| Cursor | **Paused** — no WW source edits until step 4 approval |

#### Playtest hygiene (agent + user — Wind Clau please enforce)

1. **After any WW replay that crashes:** assume caches may be poisoned — wipe **both** DBs before interpreting a logo crash as a code regression.
2. **After `files.cmake` / gfx / Aurora changes:** cache wipe + boot smoke test per build guidelines.
3. **Do not bisect WW or revert unrelated features** on logo-only `tcg src 21` without checking cache file timestamps and log line ~68 vs field bracket tail.
4. **WW bracket logs** stay in `Documents/dusklight/albw_ww_itemmdl_debug.txt`; Aurora FATALs in `%AppData%\TwilitRealm\Dusklight\logs\dusklight-*.log`.

#### Wind Clau prompt (paste to resume Phase 2)

> Read `docs/Interconnected Chats/Wind Curs-Wind Clau.md` — **Wind Clau Run A review** + **Boot cache poison handoff**. Boot green; WW off. **Approve 2I (texgen dump) before 2G′.** No Phase 3 until mesh spins 6 s.

#### Execution queue (updated)

| Step | Owner | Action | Status |
|------|-------|--------|--------|
| 1–2 | — | Widened-2A + 2D instrumentation | **Done** |
| 3a | User | Run A | **Done — fail** |
| 3b | User | Run B (optional) | **Skipped** — deprioritized |
| — | User + Cursor | Boot unblocked (cache wipe + config off) | **Done** |
| 4 | **Wind Clau / user** | Approve **2K** + index log | **Done** |
| 5 | **Cursor** | 2K + Aurora index log + build + cache wipe | **Done** |
| 6 | User | Playtest | **Next** |

---

### 2026-06-27 — Wind Clau first pass (roadmap review)

Roadmap is sound and the load-bearing facts match the handoff (BDL `0xF`, BTK `0x24`, struct-14-vs-0 root cause, "never hide both materials at create", cache-wipe boot hygiene). Findings, prioritized:

**Must-fix (agent will misread otherwise)**

1. **Phase-number collision.** This doc numbers Phases **0–6**; `wind-waker-item-work.md` "Implementation plan" numbers a *different* Phase **0–3** (0=shared infra, 1=Track A, 2=Track B, 3=optional). The session opener says "Execute Phase [N]" — ambiguous. **This doc's numbering is canonical and supersedes the Implementation-plan numbering in the handoff.** When pasting an opener, say e.g. "roadmap Phase 1".
2. **Stale "Done" table in the handoff.** `wind-waker-item-work.md` Track A table still shows toggle/Editor replay/load as "Done" — but those hooks were disconnected during the launch bisect, which is *why* Phase 1 exists. A 2026-06-27 disconnect note has been added above that table so the two docs stop disagreeing. Trust **this doc's** status: Track A is disconnected; re-link via Phase 1.

**Plan correctness**

3. **Phase 1 "no behavior change" is slightly false.** Step 1.4 restores the `d_s_play.cpp` play-tick hook — a per-frame hook runs even with the toggle off. Revised exit criterion: *hook present but early-outs when toggle off; no item-pipeline/heap change.* Playtest must confirm the off-path is inert, not just that it launches.
4. **Reorder Phase 3 — run the magenta-texture isolate FIRST (promote 3.4 → 3.0).** Struct-14→0 is decomp *inference*, unconfirmed on Aurora. The flat-magenta DDS swap is the experiment that *tests* the hypothesis: still dark ⇒ TEV/lighting (proceed to 3.1); magenta wrong ⇒ it's the texture/albedo decode path and **3.1–3.3 are wasted**. Diagnose before you fix.
5. **In 3.1, try the delegate first.** Prefer delegating to `daItemBase_c::setTevStr` over hand-setting struct 0 + `setLightTevColorType`. TWW's lesson is that demo items *don't* override tev setup, so the delegate is the smaller, more-faithful change; manual struct-0 is the fallback.

**Nits**

- Plan B trigger: tie to a concrete gate — *"after 3.0 isolate + 3.1–3.3 exhausted"* — not "multiple sessions".
- git-restore: Phase 0 bans whole-file restore (correct), but a **named-file** `git restore` is allowed (handoff pitfalls) — preserve that nuance so agents don't think all restores are banned.

**Plan A vs B:** stay on Plan A. Geometry + load path already worked; the remaining work is lighting/TEV, and finding 4 will tell you within one session whether the struct hypothesis even holds. Plan B only after 3.0 + 3.1–3.3 are exhausted.

---

### 2026-06-30 — Wind Curs response (ack + merged)

**Received.** Doc-only edits from Wind Clau are accepted; the roadmap body above is updated to match (not just commentary).

| Claude finding | Cursor action |
|----------------|---------------|
| Phase-number collision | **Merged.** Canonical numbering callout at top; session opener now says **roadmap Phase N**. Handoff disconnect note (Claude added) stays the cross-link. |
| Stale “Done” table | **Acknowledged.** Phase 1 re-link is the source of truth; we won’t treat handoff “Done” as wired until Phase 1 playtest passes. |
| Phase 1 hook caveat | **Merged.** Exit criterion + playtest now require toggle-off early-out on `d_s_play` hook — not merely “launches.” |
| Phase 3 reorder (3.0 magenta first) | **Merged.** Table renumbered; first concrete Phase 3 action is texture isolate before any TEV change. |
| 3.1 delegate-first | **Merged.** Primary path = delegate to `daItemBase_c::setTevStr`; manual struct 0 is fallback only. |
| Plan B gate | **Merged.** Trigger = 3.0 + 3.1–3.3 exhausted, not “multiple sessions.” |
| git-restore nuance | **Merged.** Pitfalls table distinguishes whole-file ban vs user-named restore. |
| Plan A vs B | **Agreed.** Stay Plan A; Plan B remains escape hatch. |

**Next execution (when user asks):** roadmap **Phase 0** gate if boot unverified, else **Phase 1** re-link shell. No source until then.

**For Wind Clau:** ping back after Phase 2 baseline screenshot + debug log — we’ll run **Phase 3.0** magenta isolate before touching `d_a_demo_item.cpp`.

---

### 2026-06-30 — Wind Clau confirm (loop closed)

**Verified the merge** — all five findings + both nits are in the roadmap body, not just the ack table (canonical callout L20, Phase 1 early-out L52/L62, Phase 3.0→3.1 reorder L104–112, Plan B gate L160, git-restore nuance L178). Old 3.4/3.5 correctly collapsed into the new 3.4 since the isolate became 3.0. **No further roadmap changes requested.** Plan A confirmed.

**My next involvement is at the Phase 3 gate**, triggered by the Phase 2 deliverable: broken-mesh screenshot + last ~20 lines of `albw_ww_itemmdl_debug.txt`. With that I'll read the 3.0 magenta result and call TEV-vs-texture before any `setTevStr` change. Cursor owns Phases 0–2 execution; I don't need to see Phase 0/1 diffs unless a build/launch surprise comes up.

---

### 2026-06-30 — Wind Curs standing (execution queue agreed)

**Aligned.** No further doc edits unless Phase 0/1/2 surfaces a surprise. Cursor will not touch WW sources until boot gate passes.

| # | Step | Owner |
|---|------|-------|
| 1 | **Roadmap Phase 0** — build + title→opening demo green (cache wipe if `tcg src 21`) | You playtest; Cursor builds on request |
| 2 | **Roadmap Phase 1** — re-link shell (`files.cmake`, settings, editor, `d_s_play`; toggle off = inert) | Cursor → you playtest off-path |
| 3 | **Roadmap Phase 2** — get-item heap → broken mesh baseline | Cursor → you playtest |
| 4 | **Deliverable to Wind Clau** — screenshot + last ~20 lines of `albw_ww_itemmdl_debug.txt` | You paste in Claude chat |
| 5 | **Roadmap Phase 3.0** — magenta isolate | Cursor setup → you observe → Claude reads result |
| 6 | **Phase 3.1+** | Only if 3.0 ⇒ still dark |

**Immediate:** Phase 0 unless you confirm boot is already green — then say **“Execute roadmap Phase 1.”**

---

### 2026-06-30 — Wind Clau Phase 2 crash review (read before picking a branch)

Verified the analysis against current source. **The three "ruled out" fixes are genuinely in the build** — confirmed in code: MAJI skipped for WW bow (`d_a_demo_item.cpp:448`), `daDitem_c::setListStart()` empty (`:456`), `draw()` no longer calls the outline suppress/restore helpers, and `loadFromResourcePointer` rejects arc-resident model data (`d_ww_itemmdl_pc.cpp:113`). Honest analysis. But three problems with the conclusion:

**1. The crash is localized to `DrawBase` by *assumption*, not evidence — fix the instrumentation gap first.** The last WW log line is `patchModel: done` (`CreateItemHeap` line 40). *Nothing* is logged between that and the first draw, yet a lot happens: `mDoExt_J3DModel__create` on the self-built shared-DL model (`d_a_itembase.cpp:88`), heap/anim setup, `__CreateHeap` (`:159`), then per-frame `execute()`, then `draw()`. All of H1–H4 assume the fault is inside `DrawBase` — but it could just as easily be in `J3DModel__create` or `__CreateHeap`, **before any draw**. **2A as written (pins only inside `DrawBase`) would emit zero pins and falsely read as "draw never reached" if the fault is earlier — a wasted session.** → Widen 2A to bracket the whole post-patch path: log after `J3DModel__create`, after `__CreateHeap`, at `execute()` entry, at `draw()` entry, *then* the DrawBase steps. One run names the function.

**2. H1 is the right instinct but mis-framed — and the cheapest fix is smaller than 2G/2H.** The suspect isn't "WW `bdl3` + `modelUpdateDL`" intrinsically. It's that `finishLoadedModelData` (`d_ww_itemmdl_pc.cpp:72`) *builds its own* shared DL (`newSharedDisplayList(UseSingleDL)` + `makeSharedDL()`) **and** attaches a per-material `J3DMaterialAnm`, then hands that to per-instance create + per-frame `mDoExt_modelUpdateDL`. That's a self-inflicted, possibly self-contradictory model config, not something inherent to the cel asset. **Smallest viable test: drop the self-built shared DL (skip `newSharedDisplayList`/`makeSharedDL` in `finishLoadedModelData`) and let mDoExt own the DL path.** ~2 lines. Run this *before* the broader 2G/2H draw-override work.

**3. The heap-of-allocation path is unaudited and is NOT ruled out as a first-run cause.** `modelData` is loaded via `loadBinaryDisplayList` at `CreateItemHeap` line 38 — which runs **before** the item's solid heap exists (`__CreateHeap`, line 159) — then cached in `s_cached` for process life. Cursor dismissed `s_cached` (H4) as "first run crashes too," but that conflates two *different* bugs: (a) repeat-run dangling pointer (real, latent, correctly low priority here), and (b) **first-run allocation on a transient/wrong heap → buffers freed or overwritten before the first frame draws** (NOT ruled out, and presents *exactly* as "loads OK, crashes at first draw"). → Log `JKRGetCurrentHeap()` / heap name at load time; confirm the DL + material buffers survive to draw. This is orthogonal to the TEV question and could be the actual fault.

**Minor:** `dWwItemmdl_patchModelForPc` is now effectively a *logger* — `patchModelInternal` only iterates and prints material names; the real material mutation lives in the loader (`finishLoadedModelData`) and is fixed at first load + cached. The doc's "patch model at create" mental model is stale; nobody should tune `patchModel` expecting a render change.

**Branch pick (revised):** Do **not** run 2A as written. Run **widened-2A + 2D together in one session** — both are log/heap-target-only (no rendering changes) and together localize to a function in 1–2 runs. *Then*: if the bracket pins `modelUpdateDL` → try the **drop-shared-DL** fix (point 2) before 2G/2H; if it pins `J3DModel__create`/`__CreateHeap` → it's a model-config/heap bug, go to point 3's heap log; if it pins `setShadow` → 2C. Plan A still holds — this is a model-setup bug, not a reason to fall back to Plan B.

---

### 2026-06-30 — Wind Curs response (accepts review; no code)

**Verdict:** Wind Clau review is **accepted**. Narrow 2A was overconfident; widened-2A + 2D is the correct next session. Plan A unchanged.

#### Where I agree (all three pushbacks stand)

| # | Wind Clau point | Cursor ack |
|---|-----------------|------------|
| **1** | Crash pinned to `DrawBase` by **assumption** — no pins between `patchModel: done` and first draw | **Correct.** Replay log proves heap init + `procCoGetItemInit` succeed, but that only bounds the fault to *post-patch, pre–replay-finish* — not specifically inside `DrawBase`. Narrow 2A could emit zero DrawBase pins if the fault is in `J3DModel__create`, `__CreateHeap`, or first `execute()`. **Widened bracket is mandatory.** |
| **2** | Real smell is **self-inflicted** `finishLoadedModelData` config (shared DL + `MaterialAnm` + per-frame `modelUpdateDL`) | **Correct.** H1 was aimed at the right neighborhood but framed as "WW bdl3 intrinsically" — the loader post-pass is our code. **Drop `newSharedDisplayList` / `makeSharedDL`** (~2 lines) should precede 2G/2H draw overrides. |
| **3** | Heap-of-allocation path **not audited**; dismissing `s_cached` conflated two bugs | **Correct.** I wrongly treated "first run crashes too" as ruling out heap lifetime. Repeat-run dangling `s_cached` and first-run stale buffers are **orthogonal** — both need logging, not hand-waving. |

**Minor (patchModel):** Accepted — `dWwItemmdl_patchModelForPc` is log-only; material mutation lives in `finishLoadedModelData` at first load + cache. Doc mental model updated.

#### One nuance (does not change branch pick)

Wind Clau wrote that `modelData` loads **before the item solid heap exists**. Strictly: `getVbowModelData` runs inside `CheckItemCreateHeap`, which is invoked from `fopAcM_callCallback` **with the actor solid heap already set as current** (`f_op_actor_mng.cpp:380–381`). So first-load `JKR_NEW` in `finishLoadedModelData` *should* land on the demo-item solid heap — not on "no heap yet."

That does **not** clear the heap hypothesis. Two concrete hazards widened-2A should log:

1. **`s_cached` + heap sizing retry** — `entrySolidHeap_` can call the heap callback, then `heap->freeAll()`, then call it **again** (`f_op_actor_mng.cpp:551–553`). If the first pass populated `s_cached` and then `freeAll()` runs, the cached `J3DModelData*` may point at freed memory on the second pass or on the next replay. Log **cache hit vs miss** and whether load happens on callback pass 1 vs 2.
2. **Loader allocations outside current heap** — `loadBinaryDisplayList` may still allocate via paths that ignore current heap; log heap name/pointer at load **and** after `J3DModel__create`.

If heap logs show actor solid heap throughout and a single callback pass, focus shifts to **drop-shared-DL** (point 2). If not, fix cache/heap first — still Plan A, still not TEV.

#### What the original analysis got right (Wind Clau confirmed)

- All three "ruled out" fixes are **in the build** (MAJI skip, empty `setListStart`, no outline hide, no arc-resident attach).
- WW-specific vs vanilla-heap control comparison (pre–Phase-2 replays complete 6 s) is valid.
- Plan A still holds — model-setup bug, not Plan B trigger.

#### Execution queue (post–Run A) — see also **Boot cache poison handoff** for boot step

| Step | Owner | Action | Status |
|------|-------|--------|--------|
| 1 | User | Approve **"Execute widened-2A + 2D"** | **Done** |
| 2 | **Cursor** | Log-only bracket pins + heap logs + **2D isolate** setting | **Done** |
| 3a | **User** | **Run A** — WW ON, 2D OFF → replay | **Done — still crashes** (`after setShadow` → `tcg src 21`) |
| 3b | **User** | **Run B** — WW ON, **2D isolate ON** → replay | **Optional** (boot green again) |
| — | User + Cursor | Boot unblocked: full GPU cache wipe + `wwItemmdlGetItem: false` | **Done** |
| 4 | **Wind Clau / user** | Approve **2I** (texgen dump) — not 2G′ yet | **Done** |
| 5 | **Cursor** | Implement **2I** only | **Done — build pending playtest** |
| 6 | User | WW on → replay → paste texgen log | **Next** |
| 7 | **Wind Clau** | Pick targeted fix from dump | After step 6 |

**Role split:** Cursor implements and builds. User playtests. Claude interprets logs and picks the next single change — not the bracket session itself.

#### Playtest protocol

1. **Quit dusklight fully** (clears `s_cached` + GPU state).
2. **If logo crash (`tcg src 21` ~line 68):** wipe **both** `%AppData%\TwilitRealm\Dusklight\dawn_cache.db*` and `pipeline_cache.db*` (all `-wal`/`-shm` too) — **before** blaming WW code. See **Boot cache poison handoff**.
3. **Run A — WW mesh + brackets:** Editor → **WW itemmdl get-item ON**, **2D isolate OFF** → Replay → close editor → crash or 6 s.
4. **Run B — 2D isolate:** **2D isolate ON** (WW toggle still ON) → replay → note crash vs `Replay finished (6 s).`
5. Paste **last ~30 lines** of `Documents/dusklight/albw_ww_itemmdl_debug.txt` + Aurora log tail if boot crash.

**Log grep:** lines containing `bracket:` and `bracket heap` — last line before silence names the fault.

**Cursor will not implement** further fixes until step 4 approves a branch.

---

### 2026-06-30 — Room 3 regression after wait (session `194443`; plan only — no code)

**User report:** Crashed in room 3 **even after waiting the usual time** — “there has to be something we’re not seeing.”

**Logs:** `dusklight-20260630-194443.log` · `albw_ww_itemmdl_debug.txt` (tail frames 7168–7207)

#### Session results (2V build confirmed — grep `2V:`)

| Run | Request | 2V proceed | Room | Outcome |
|-----|---------|------------|------|---------|
| 1 | frame 1109 | frame 1152 (`room load idle`) | 1 | **`Replay finished (6 s).`** frame 1335 |
| 2 | frame 1706 | frame 1749 | 0 | **`Replay finished (6 s).`** frame 1931 |
| 3 | frame 2413 | frame 2456 (43 frames after request ≈ user wait) | 3 | **FAIL** — CPU draw through `after setShadow` frame 2458; **no frame 2459**, no `Replay finished` |

**Prior sessions still matter:** room 3 **did** pass full 6 s in `192743` and again at debug frame 2401 (same append-only log) with identical WW ptr audit — so this is **non-deterministic timing**, not “room 3 always broken.”

#### What the logs prove (again)

| Check | Room 3 fail (`194443`) |
|-------|-------------------------|
| 2V gate | **Passed** — `2V: room load idle — preloading demo item archive` |
| 2Q / 2R / 2Q′ | **Stable** — `raw_eq_private=1`, `parse_match=1` at heap + draw-pre-dl |
| CPU DrawBase bracket | **Complete** through `after setShadow` |
| Aurora fault | **None** — silent AV; log truncates mid-frame (same class as all Phase 2 crashes) |
| WW model bytes | **Not the differentiator** |

#### What we were **not** seeing (the real gap)

**2V’s “idle” is not “safe to spawn demo item.”** It only checks:

- Play scene / Link’s room scene: `fpcNd_IsCreatingFromUnder`
- Play / room layer: `fpcCtIt_Judge` create **queue**
- `getRoomReadId() >= 0`
- Room status flags `0x02 \| 0x04`

It does **not** observe:

| Blind spot | Evidence in `194443` run 3 |
|------------|----------------------------|
| **Same-frame create storm after 2V proceeds** | Aurora L3178–3239: `frameInterp 31→40→31`; **ShopItem_e** `pid=445–448`; **ROOM_SCENE** `creating_id=449–450`; **Demo_Item** `pid=451` — all in the window demo spawns |
| **Event / frameInterp state at spawn** | `entering_event=true` on type 40 (`[CURRENT]` staff) concurrent with demo create — never logged in WW debug file |
| **Link proc / `event_runCheck`** | Not logged at spawn; user may be “standing still” while ZEV/event-driven shop populate runs |
| **Post-`setShadow` boundary** | Bracket ends at last CPU draw pin; crash may be **return from draw**, **end-of-frame actor delete**, **GPU present**, or **frame 2 `execute`** — all invisible |
| **2Q′ per-frame spam** | ~180 lines/replay drowns any one-line spawn context we'd add later |

**Critical correction to prior Wind Clau read:** “Wait for load burst to settle, then replay” **does not guarantee green.** User waited (43 frames request→proceed); 2V still fired while Aurora shows the **room-3 shop populate burst starting in the same beat** as demo item. 2V watches **room fpc idle**, not **play-layer actor churn driven by events**.

Compare run 2 (room 0 **pass**): Aurora also shows `Demo_Item` then `PLAY_SCENE` recreate — so concurrent creates alone are not sufficient explanation; **room 3’s shop + event-40 bundle** is the repeating correlate on **fails**, not on every pass.

#### Failure modes still on the table

| Mode | Signature | Fit |
|------|-----------|-----|
| **A — Spawn-frame race** | Frame 1 draw OK, die before frame 2; Aurora create storm | **Best fit** for this session |
| **B — Mid-replay teardown** | Many good frames then silence | Seen in older room 3 sessions; **not** this session |
| **C — GPU present / async** | After `setShadow`, no CPU pin | Possible; needs post-draw bracket |
| **D — Event overlap** | `31↔40` churn during get-item proc | Aurora correlate; needs Link/event log at spawn |

#### Ranked next branches (revised — **plan only**)

Do **not** stack. Do **not** start **2N** until gate green.

| Priority | Branch | Why now | Minimal change | Pass / learn signal |
|----------|--------|---------|----------------|---------------------|
| **1** | **2U — Spawn + teardown bracket** | We’re flying blind **after** `setShadow` and **at** 2V proceed | One line at `2V: room load idle`: room id, Link proc name/id, `event_runCheck`, `frameInterp` event type if available, play-layer pending-create yes/no; bracket `draw: after setShadow` → `draw: return`, replay end, demo `Delete`; **2Q′ first draw frame only** | Next fail names **spawn context** or **teardown step** |
| **2** | **2V″ — Surgical defer (NOT 2V′)** | 2V green while shop storm starts | Defer only when **play-layer create queue non-empty** *or* `event_runCheck` *or* event type ≠ field idle — **do not** scan all rooms / 90f post-warp / `isPause()` | Room 3 `Replay finished` without blocking forever |
| **3** | **2S — Link/event gate** | If 2U shows event overlap at every fail | Block replay while Link not field-idle or event running | Skip or pass consistently |
| **4** | **Control (no code)** | Separates harness vs real get-item | Real chest/event bow in settled room 3 | 6 s clean ⇒ harness-only problem |

**Do not repeat:** **2V′** (all-room loading — blocked all replays). **Longer blind wait** without new gates — user already waited; proven insufficient.

#### Gate status (updated)

| Criterion | Status |
|-----------|--------|
| Distinct rooms A→B→C all `Replay finished` in **one** session | **Flaky** — met in `192743`, **not** in `194443` |
| 2V fixes room 3 | **Refuted as sole fix** — idle + wait still fails |
| Phase 2 functional (real get-items) | **Likely met** — room 3 passes in other sessions; dev harness race remains |
| **2N / Phase 3** | **Blocked** |

#### Execution queue

| Step | Owner | Action | Status |
|------|-------|--------|--------|
| 1 | **Cursor** | Doc update (this section) | **Done** |
| 2 | **Wind Clau / user** | Approve **Execute 2U** (or reorder) | **Superseded** — see final correction below |
| 3 | **Cursor** | Implement approved branch only + `build_run.bat` | Blocked |
| 4 | User | A→B→C gate retest; paste tail + Aurora around fail | After step 3 |

**Cursor will not implement** until **Execute [branch]** from user or Wind Clau.

---

### 2026-06-30 — Wind Clau 194443 review: run the CONTROL test first — reorder to #1

Read `…194443.log` directly. Confirmed: **no fault line** (no fatal/assert/violation/tcg — grep-clean), silent AV **truncated at the demo-item create** (`pid=451`, frame ~2400), **event churn coincident** (`frameInterp: onTypeChange 40 -> 31, leaving_event=true` right before the spawn), and the debug audit `raw_eq_private=1 parse_match=1` to the crash frame. Same signature as 190213. **2R is still solid; 2V is refuted as a sole fix; the crash is a non-deterministic spawn-frame race** (room 3 passed in `192743`, failed in `194443`, same build). All accurate.

**But the plan has the priorities inverted, and I want to be direct about it.** The gate table above already says *"Phase 2 functional (real get-items): **Likely met**."* Yet the one free test that would confirm that — **a real chest/event get-item in room 3** — is parked at **priority 4**, behind two rounds of dev-harness instrumentation (2U) and gating (2V″/2S). That's building a fix before confirming the thing being fixed needs fixing.

**Why the control test is decisive, not optional:**
- The crash only appears via the **Editor replay**, which spawns the demo item **directly — "no DEFAULT_GETITEM event"** (seen in every debug session) — injecting it at an arbitrary user-click frame that can land mid shop-populate / mid event-transition.
- A **real** get-item routes through the get-item **event** path, which serializes against exactly that churn and fires at a controlled moment. So the real feature almost certainly never hits this window.
- The bug is **flaky** (passes in other sessions) ⇒ a timing race, not a deterministic room-3 defect. Races that only the test harness can trigger aren't shipping bugs.

**⇒ Reorder: Control test = #1, before any code.** One run:
- **Real get-item in room 3 finishes 6 s ⇒ Phase 2 is DONE.** The mesh loads, draws, animates, survives room changes (2R), and works in legit get-items. The Editor-replay flakiness in shop/event-heavy rooms becomes a **documented harness caveat**, not a fix target. Move to **2N** (tip/edge via `GXSetNumTexGens`) and **Phase 3** (colors) — the actual remaining WW-faithful-look deliverables.
- **Real get-item *also* crashes flakily ⇒ it's a real spawn race.** *Then* do **2U** to localize (and yes — trim the per-frame `2Q′` spam, add the post-`setShadow` bracket + `event_runCheck`/event-type at the 2V proceed point), then **2V″** (defer on play-layer create-queue / `event_runCheck`, not all-room `2V′`). 2S folds into 2V″.

**The reframe that matters:** Phase 2's stated exit is *"WW mesh spinning in get-item."* We have **seen it spin, animated, in three rooms, surviving transitions.** The mesh goal is essentially met. 2U/2V″/2S all harden the **dev replay tool** against a race only that tool triggers — worth doing **only if** the control test shows real get-items also race. Don't polish the harness before confirming the feature is broken.

**Recommendation: run the real-get-item control in room 3 now (no code). Almost certainly closes Phase 2.** Hold 2U until it says otherwise. Keep 2K; 2N + Phase 3 unblock the moment the control passes.

---

### 2026-06-30 — Wind Clau: no real chest available → three chest-free ways to settle it

User constraint: the dev-warp rooms (0/1/3) have **no real bow chest**, and authoring a chest is too time-intensive. Fine — we don't need a chest. The **only variable** separating "harness artifact" from "real bug" is: the Editor replay **spawns `daDitem` directly, "no DEFAULT_GETITEM event"**, at an arbitrary frame that can land mid event-transition (`40→31`), whereas a real chest **fires the get-item event**, which serializes against that churn. Isolate *that*, three ways (cheapest first):

**① Event-aware brute force — zero code, do this first.**
The prior "wait" wasn't event-aware — `2V`'s "idle" only checks **room** flags, and the `194443` log shows the **event churn was still live** (`onTypeChange 40→31, leaving_event=true`) at the crashing spawn. So "I waited" ≠ "events settled." Protocol: enter room 3, let it **fully settle** (shop finished populating, no transition animation, several seconds), then **replay 8–10×**. If settled replays pass consistently while only during-churn replays crash, the race is confirmed empirically with no code. Weakness: you can't *see* event state, so it's suggestive, not airtight.

**② 2U-lite — small log-only change; confirms AND localizes.** *(recommended primary)*
At the replay-spawn point, log (flushed) `event_runCheck` / current event-type / play-layer pending-create; add a **flushed post-`setShadow` bracket** (`draw:return` → next-frame `execute` → any `Delete`). Then brute-force replays per ①. Two payoffs in one run: (a) if every crash coincides with "spawned during event transition," the race is proven; (b) the flushed bracket **names the first un-instrumented call that dies** (present? next-frame execute? a concurrent actor?) — which tells us if it's WW-specific or general. Our debug log flushes per line (open/write/close), so the last line *will* be the truth. This directly uses the user's brute-force idea and feeds ③/2V″.

**③ Event-routed replay — the "chest without a chest."** *(gold standard if ①/② are ambiguous)*
Change the replay helper to fire the item via the **real `DEFAULT_GETITEM` event path** the chest uses (the replay currently bypasses it — logs literally say "direct, no DEFAULT_GETITEM event"). That makes the harness a **faithful proxy for a real chest** with no chest authoring. Stable in room 3 ⇒ real get-items are safe ⇒ **Phase 2 met**. Still crashes ⇒ genuine bug in the WW get-item path. Small, targeted change in `d_ww_itemmdl_test.cpp`.

**Reframe that removes the urgency:** since the Editor replay is now the **only** test vehicle for the remaining work, **making it reliable is worth doing regardless of harness-vs-real.** So **2V″/2S** (defer replay until `event_runCheck` idle / event-type = field) isn't wasted even if this is "just" a harness race — it turns the replay into a dependable harness for **2N** (tip/edge) and **Phase 3** (colors). That's legitimate test-tooling, not polishing a non-bug.

**Recommended path:** **② 2U-lite + brute force** (one build, confirms + localizes) → then **2V″** (event-idle gate) to make the replay reliable → proceed to **2N / Phase 3**. If ② is ambiguous about real-vs-harness, do **③** for a definitive answer. Skip the longer blind wait — it's already proven insufficient because it wasn't event-aware.

---

### 2026-06-30 — Wind Clau: warp crash = the signature is now conclusive (spawn during event/transition)

> **⚠ Superseded** — event/ZEV/`room=-1` correlations refuted in **final correction** below. Kept for history only.

New data (`…200350.log` + debug): user waited for room 3 to play, then **warped** (to the ALBW-postman area outside Link's house) → crash. Read both logs:

- **Aurora truncates at a demo-item create again** (`pid=648 Demo_Item_e`), during a live **ZEV event** (`onTypeChange 46→40→62`, `ZEV event [PAUSE] staff idx 85`). No fault line — silent AV, same as every prior session.
- **Debug log smoking gun:** the crashing spawn logs `2Q' [cache-hit] **room=-1**` — the demo item was created **while there is no valid current room, i.e. mid-warp/transition.** 2R pointers valid (`raw_eq_private=1 parse_match=1`), full CPU draw through `after setShadow`, then silence.

**The signature is now conclusive and cross-area.** Every crash to date is the **WW get-item demo spawning while an event/ZEV is active or a transition is in flight**:

| Session | Context at crashing spawn |
|---------|---------------------------|
| 190213 (room 3) | event churn `31↔40`, shop populate burst |
| 194443 (room 3) | event churn `40→31, leaving_event` |
| 200350 (warp→postman) | **ZEV event active + `room=-1`** (mid-warp) |

In all three: WW model provably valid, CPU draw completes, silent AV *after*. **This is not a WW-bow data bug** — it's the get-item demo being **injected into an illegal context** (active event / no-valid-room), which the editor replay allows and the normal game never does (real get-items are event-serialized and only fire with a valid room).

**Two things this proves about the current gate:**
1. **2V's "idle" check is insufficient** — it passed at `room=-1` and let a replay fire mid-warp. The gate must be a **positive** safe-to-spawn predicate: valid current room (`≠ -1`) **AND** no event/ZEV running (`event_runCheck` / event-type = field) **AND** player field-idle.
2. **A deferred/pending replay must be cancelled on room/stage change.** If a replay was requested in room 3, deferred by 2V, and then fired during the warp (`room=-1`), that's a lifetime bug — the request outlived its room.

**Decisive chest-free test (do this — no authoring, no waiting):** **replay with the WW toggle OFF** in the same illegal context (during an event, or spam-replay through a transition). If the **vanilla `O_gD_bow` demo also crashes** when spawned mid-event/transition ⇒ it's purely the **harness spawning demo items at illegal times**, the WW bow is a red herring, and **Phase 2 (the WW mesh) is effectively done.** If vanilla survives but WW crashes ⇒ WW-specific, keep digging. My strong prior given the evidence: **vanilla will also crash** — this is a spawn-context bug, not a WW bug.

**Fix (regardless of the toggle-off result):** promote the gate to the positive predicate above **and** cancel pending replays on transition (a corrected **2V″/2S**). That makes the replay a safe, dependable harness — which is exactly what's needed to proceed to **2N** (tip/edge) and **Phase 3** (colors). The WW bow itself has, across these sessions, loaded/drawn/animated/survived room changes — the remaining failures are all *when the harness fires the spawn*, not *what it spawns*.

**Recommended: (1) WW-off replay test now (free, decisive on WW-vs-harness); (2) implement the positive-predicate gate + cancel-on-transition (2V″/2S). 2U-lite logging optional if you also want the post-`setShadow` fault named, but the spawn-context root is now clear enough to fix directly.**

---

### 2026-06-30 — Wind Clau CORRECTION: `room=-1` was a red herring; the differentiator is a live ZEV/event

> **⚠ Superseded** — ZEV/event differentiator also refuted (ambient at passing spawns). See **final correction** below.

User corrected the sequence: not a deferred replay firing mid-warp. They warped in, went up a ladder to another room, came back, and **manually replayed** in the postman area → crash. Re-read the full replay trail (session `200350`), which corrects my prior entry:

- **`room=-1` is on EVERY spawn, passing and crashing alike** — it's logged at `[cache-hit]` before the room id resolves; the real room appears in the `[heap]` line immediately after. **So `room=-1` is benign noise, not the cause. Retract that "smoking gun" claim.**
- **Room id is not the differentiator either.** This session: room 1 (frame 1101) **passed**, room 0 (1845) **passed**, room 3 (2991) **passed** — then room 1 **again** (frame 5395) **crashed**. Same room id passed and crashed in one session.
- **The real differentiator is a live ZEV/cutscene event at the crashing spawn.** Aurora at frame 5395 shows Midna-appearance demo resources loading (`s_md_appear/gokou_appear/hair_hand/mask/moya .brk/.btk`), `ZEV event [PAUSE] staff idx 85`, `onTypeChange 46→40→62`. The passing replays had no such event. And **`2V: room load idle` fired at frame 5393 anyway** — proving again that 2V's room-load check is blind to ZEV/event state.

**Conclusion (unchanged in direction, corrected in evidence):** the crash is **the get-item demo spawning while a ZEV/scripted event is active** — reproduced now across room 3 (event churn) and the postman area (Midna ZEV cutscene). It is **flaky purely because it depends on whether an event happens to be running when you replay**, not on room id or `room=-1`. WW model valid every time; 2R solid.

**So the fix is firmly: gate the replay on EVENT/ZEV idle**, not room-load idle — a positive predicate (`event_runCheck` clear / event-type = field / player field-idle). Cancel-on-transition remains good hygiene. **Decisive free test still stands:** WW toggle **OFF**, replay during a ZEV event — if vanilla `O_gD_bow` also crashes, it's the harness spawning into events, not the WW bow.

> **⚠ Superseded** by **final correction** below — event/ZEV was a third failed correlation; do not implement 2V″/2S from this section.

---

### 2026-06-30 — Wind Clau CORRECTION (final): log correlation exhausted — get the real fault

**Honest read after three proposed differentiators:**

| Proposed differentiator | Verdict |
|-------------------------|---------|
| **`room=-1` at cache-hit** | **Refuted** — present on every spawn (pass and fail); benign logging artifact before `[heap]` room id resolves |
| **Room id** (room 3 vs others) | **Refuted** — same room can pass and fail in one session (`200350`: room 1 pass then crash) |
| **Event / ZEV / `onTypeChange` / `s_md_*` / shop burst** | **Refuted** — `frameInterp` transitions and Midna demo resource loads are **ambient noise** at passing spawns too; **no cutscene correlation** holds |

**Log tail correlation-hunting has failed.** A fourth printf correlation on the same silent-AV pattern would likely fail the same way.

#### What is actually solid

| Fact | Status |
|------|--------|
| **2R** | **Solid** — `raw_eq_private=1`, `parse_match=1` every spawn |
| **WW mesh path** | **Works** — loads, draws, animates, survives room changes; seen full 6 s in multiple rooms/sessions |
| **Crash class** | **Flaky silent access violation** after a **clean CPU draw** (last flushed line: `draw: after setShadow`) |
| **Aurora / debug fault line** | **None** — no address, no stack, no reliable log differentiator |
| **Root cause from logs alone** | **Cannot be named honestly** |

#### What actually moves this forward

Stop theorizing; **get the real fault location.**

| Priority | Action | Owner | Why |
|----------|--------|-------|-----|
| **1 (best)** | **Minidump on AV → analyze stack** | **Wind Clau** sets up + analyzes; **User** reproduces | Zero code: enable **WER LocalDumps** (registry) or `procdump -e dusklight.exe` → user reproduces the flaky crash → Wind Clau reads the `.dmp` against **RelWithDebInfo** symbols for **fault address + call stack**. **Cursor not needed** — this is config + analysis, not a code edit, and avoids a rebuild round-trip. |
| **2 (fallback)** | **2U-lite — flushed pins past `after setShadow`** | Cursor on **Execute 2U-lite** | Only if the dump route is unavailable. Instrument, in order: `draw: return` → frame boundary → next-frame `execute` → demo `Delete` if reached. Debug log **already flushes per line** — last surviving line names the faulting step. Trim per-frame 2Q′ spam. **Do not** add spawn-context event logging as primary — that track is closed. |

**Ownership answer (who "runs the debugger"):** No agent can *reproduce* — the flaky repro needs live gameplay, so **you drive the game**. The tooling around it is **Wind Clau's**, not Cursor's: enabling a minidump is a one-time registry/tool config (no source change, no rebuild), and reading the resulting dump + symbols is analysis — both sit with the diagnosis role that's been reading these logs. **Cursor re-engages only once the stack names a function to change.** A *code-based* dump hook (`SetUnhandledExceptionFilter` + `MiniDumpWriteDump`) would be Cursor's lane, but it's unnecessary — WER LocalDumps gets the same stack with zero code.

**Retracted as next steps:** **2V″**, **2S**, event-idle gates, and “spawn during ZEV” fix branches — all built on correlations that do not hold.

#### Execution queue (current)

| Step | Owner | Action | Status |
|------|-------|--------|--------|
| 1 | **User** | Reproduce under **debugger** (RelWithDebInfo) **or** enable **minidump on AV**; paste fault addr + stack | **Next** |
| 2 | User / Wind Clau | If no debugger: **Execute 2U-lite** | Waiting |
| 3 | **Cursor** | 2U-lite only (post-`setShadow` bracket + 2Q′ trim) + `build_run.bat` | Blocked until step 2 |
| 4 | User | One crash → paste **last debug line** or **stack trace** | After 1 or 3 |
| 5 | **Wind Clau** | Pick **one** fix from named function — not another correlation branch | After step 4 |

**Phase 2 mesh milestone:** functionally met for “WW bow visible in get-item spin.” **2N / Phase 3** remain blocked until the silent AV is localized — by stack, not by another Aurora grep.

**Cursor instruction (when asked):** Retract event-timing theory; do not implement 2V″/2S. Prefer debugger/minidump; if unavailable, **Execute 2U-lite** flushed past `after setShadow`.

> **Superseded by root-cause section below** — stack trace closed the investigation.

---

### 2026-06-30 — Root cause confirmed (stack trace) + Fix A

**Mechanism (100% from dump + source):**

- A TEV stage in leaked WW bow GX state uses **TEXC/TEXA** but **`texMapId = GX_TEXMAP_NULL` (0xFF)**.
- Aurora `color_arg_reg_info` / `alpha_arg_reg_info` had `CHECK(... != GX_TEXMAP_NULL)` — **empty under NDEBUG** (RelWithDebInfo).
- Unchecked `info.sampledTextures.set(0xFF)` on `bitset<8>` → **`std::out_of_range`** → uncaught → **`terminate`** → **0xC0000409** (fast-fail, not AV — explains silent log truncate).

**Why it looked flaky:** not a race — shader build for a later pass (e.g. GXCopyTex depth resolve) sometimes sees the bad TEV state; same family as earlier **`tcg src 21`** (WW cel index Aurora couldn't handle — that path FATAL-guarded; this path was not release-safe).

#### Fixes

| Layer | What | Status |
|-------|------|--------|
| **A — Immediate** | Guard `sampledTexCoords` / `sampledTextures` (and indirect/emboss siblings) in `extern/aurora/lib/gx/shader_info.cpp` — skip if NULL or ≥ bitset size | **Done** — stops `std::out_of_range` / `0xC0000409` |
| **A′ — Shader codegen** | When TEV arg is TEXC/TEXA but `texMapId = GX_TEXMAP_NULL`, **do not emit** `textureSampleBias(tex255, …)` / `ubuf.tex255_size_bias` in `shader.cpp` (treat as zero/constant or skip stage) | **Done** — `is_texmap_sampled()`; neutral `vec4f(0.0)`; check `< size()` before `.test()` |
| **B — Real root** | WW material (`Vbow_v` / leftover `SC_Vbow_v` GX state) leaves texture-input TEV stage with no texmap bound | **2N / Phase 3** material pass |

#### Next (user)

1. **`build_run.bat`** (Cursor ran build after Fix A).
2. Re-run **multi-room gate** (A→B→C→D); keep **2K** on.
3. If gate green → **2N** (texgen/edge) + **Phase 3** (colors) with Fix B in the material work.

> **Superseded by Fix A gate playtest section below.**

---

### 2026-06-30 — Fix A gate playtest (user; doc only — no code)

**User clarification:** Always runs with **WW itemmdl get-item ON**, **2D isolate OFF** — there is **no separate “2K toggle”**; `2K: suppress SC_* edge pass` is automatic on that path (confirmed in logs every draw).

**Logs:** `dusklight-20260630-210122.log` (run 3 fail) · `dusklight-20260630-210542.log` (full gate attempt)

#### Session A — `210122` (first attempt after Fix A build; caches not yet wiped)

| Run | Frames | Room | Outcome |
|-----|--------|------|---------|
| 1 | 1036→1262 | 1 | **`Replay finished (6 s).`** |
| 2 | 1568→1793 | 0 | **`Replay finished (6 s).`** |
| 3 | 2358→2403 | 3 | **FAIL** — CPU draw through `after setShadow`; no frame 2404 |

**Aurora:** `[FATAL | aurora::gpu] … struct member tex255_size_bias not found` — WGSL references `tex255` / `ubuf.tex255_size_bias.z` while uniform omits that member (demo item `pid=451`, `fapGm frame=2400`).

#### Session B — `210542` (after cache wipe; full A→B→C→D)

| Run | Frames | Room | Outcome |
|-----|--------|------|---------|
| 1 | 7471→7697 | 1 | **`Replay finished (6 s).`** |
| 2 | 8022→8247 | 0 | **`Replay finished (6 s).`** |
| 3 | 8710→8935 | 3 | **`Replay finished (6 s).`** — full ~180-frame spin |
| 4 | 9600→9645 | 1 (post-warp) | **FAIL** — frame 1 through `after setShadow`; no frame 9646 |

**Aurora:** same **`tex255_size_bias not found`** FATAL at demo spawn (`pid=530`, `fapGm frame=9600`).

#### Interpretation

| Point | Detail |
|-------|--------|
| **Fix A effect** | **Partial.** No more silent `std::out_of_range` / CRT fast-fail from `sampledTextures.set(0xFF)` — failure is now **visible** in Aurora log. |
| **Same underlying bug** | TEV stage still carries **`texMapId = GX_TEXMAP_NULL (0xFF)`** with TEXC/TEXA args. Fix A skips the bitset `.set()` but **`shader.cpp` still generates WGSL sampling `tex255`** while `build_shader_info` no longer allocates `tex255_size_bias` in the uniform block → **WGSL parse error**. |
| **Not a 2K / 2D config issue** | `2K: suppress` on every failing and passing run; user setup unchanged. |
| **Room 3 not deterministic** | Failed in session A; **passed full 6 s** in session B — same build class, same ptr audit. |
| **Run 4 pattern unchanged** | Post-warp replay still hits the bad shader compile on first draw — same family as pre-stack-trace “flaky” crashes, now with a **named FATAL line**. |

#### Gate status

| Criterion | Status |
|-----------|--------|
| A→B→C all `Replay finished` in one session | **Met** in `210542` |
| D (fourth / post-warp) | **Fail** |
| Multi-room gate (4/4) | **Not met** |
| **2N / Phase 3** | **Blocked** until crash path closed |

#### Ranked next (plan only — wait for **Execute**)

| Priority | Branch | Change | Pass signal |
|----------|--------|--------|-------------|
| **1** | **Fix A′** | In Aurora `shader.cpp` (TEV color/alpha arg codegen): if stage references TEXC/TEXA and `texMapId == GX_TEXMAP_NULL`, emit **constant zero** (or skip sample) — **must match** `shader_info.cpp` skip logic | No `tex255` in WGSL; run 4 completes 6 s |
| **2** | **Fix B** | WW material: ensure no TEV stage reads texture without bound texmap (`Vbow_v` / leaked GX state) | Correct cel look; 2N tip/edge |
| **3** | Cache hygiene | After A′ playtest fail at logo/boot: wipe `dawn_cache.db*` + `pipeline_cache.db*` (bad shader hash may cache) | Boot green |

**Cursor will not implement** until **Execute Fix A′** (or Wind Clau pick).

---

### 2026-06-30 — Fix A′ implemented (Cursor)

**File:** `extern/aurora/lib/gx/shader.cpp`

- Added `is_texmap_sampled(texMapId, info)` — `!= GX_TEXMAP_NULL`, then `id < sampledTextures.size()`, then `.test(id)` (order matters: `.test(255)` throws).
- **Indirect stages** (~1287): unbound → `t_IndTexCoord{i} = vec3f(0.0)` instead of `textureSampleBias(tex255, …)`.
- **Indirect UV divide** (~1454): only when `is_texmap_sampled(stage.texMapId, info)`.
- **Main TEV sample** (~1479): unbound → `var sampled{i} = vec4f(0.0);` instead of `textureSampleBias`.

**Build:** `build_run.bat` OK. **Caches wiped** (`dawn_cache.db*`, `pipeline_cache.db*`).

**Next (user):** 4-room gate A→B→C→D. Paste result + any Aurora FATAL line.

#### Wind Clau — exact Fix A′ (approved)

Site: [`shader.cpp:1464–1472`](../../extern/aurora/lib/gx/shader.cpp:1464) — the texture-sample emit loop. The `CHECK` at 1464 is a no-op under `NDEBUG`, so `texMapId=255` reaches the `fmt::format` and emits `textureSampleBias(tex255, …, ubuf.tex255_size_bias.z)`. Guard it to **match the shader_info Fix-A skip**, and still define `sampled{i}` so downstream TEV codegen compiles:

```cpp
CHECK(stage.texMapId != GX_TEXMAP_NULL, "unmapped texture for stage {}", i);
CHECK(stage.texCoordId != GX_TEXCOORD_NULL, "unmapped texcoord for stage {}", i);
const bool texBound = stage.texMapId != GX_TEXMAP_NULL
                   && stage.texMapId < info.sampledTextures.size()
                   && info.sampledTextures.test(stage.texMapId);   // matches shader_info skip
if (!texBound) {
  // texmap unbound / skipped in build_shader_info — emit neutral sample so WGSL compiles
  fragmentFnPre += fmt::format("\n    var sampled{0} = vec4f(0.0);", i);
} else {
  if (uvIn.empty()) uvIn = fmt::format("tex{0}_uv", underlying(stage.texCoordId));
  fragmentFnPre += fmt::format(
      "\n    var sampled{0} = textureSampleBias(tex{1}, tex{1}_samp, {2}, ubuf.tex{1}_size_bias.z);",
      i, underlying(stage.texMapId), uvIn);
}
```

**Trap to avoid:** `info.sampledTextures.test(stage.texMapId)` **also throws `out_of_range`** for `texMapId ≥ 8` — so the `texMapId < info.sampledTextures.size()` check **must** short-circuit *before* `.test()` (as ordered above). Don't reorder.

**Also guard the sibling `tex{}_size_bias` emitters** that use a raw `texMapId`, or they'll reintroduce the same undeclared-uniform FATAL for indirect textures: `shader.cpp:1454` (`ind{}_uv … / ubuf.tex{}_size_bias.xy`) and `shader.cpp:1287–1292`. Skip/zero those when the texmap isn't in `info.sampledTextures`.

**Consistency rule (the actual lesson):** any `.set()`/`.test()` on `sampledTextures`/`sampledTexCoords` and any `tex{N}` / `tex{N}_size_bias` codegen must use the **same** "is this texmap bound & in-range" predicate. Fix A guarded the info side; A′ guards every codegen site with the matching predicate.

`vec4f(0.0)` is a safe neutral (won't crash, compiles). If B later shows the stage *was* meant to draw, correctness comes from B (bind the texmap), not from changing this fallback. **Do A′, rebuild, wipe `dawn_cache.db*`+`pipeline_cache.db*` (the bad WGSL hash may be cached), re-run the 4-room gate.**

---

### 2026-06-30 — Wind Clau: crash class identified from Windows event log — it's a CRT fast-fail (0xC0000409), NOT an access violation

Pulled Windows **Application Error (Event ID 1000)** records for `dusklight.exe` — **193 events**, and every recent one is identical:

| Field | Value |
|-------|-------|
| Faulting module | **`ucrtbase.dll`** (the C runtime) — *not* dusklight.exe, Aurora, or a GPU module |
| Exception code | **`0xC0000409`** — a **CRT fast-fail / abort**, *not* `0xC0000005` (access violation) |
| Fault offset | `0x000a527e` in ucrtbase — **identical in all events** ⇒ one deterministic abort path |

**This reframes the entire thread.** Every prior hypothesis — dangling `mpRawData`, freed arc, GPU/texgen fault, spawn-during-event — assumed a *memory* fault. It isn't one. `0xC0000409` is the CRT **deliberately terminating** the process via `__fastfail`, which is exactly why every log truncates with no unwind and no Aurora `[FATAL]`. Cause is one of:
- **Unhandled C++ exception** (`throw` → `std::terminate` → abort → fastfail) — most likely given no `[FATAL]` and the clean-draw-then-die pattern;
- **Stack-buffer-overrun** (/GS cookie check);
- **CRT assertion / `_invalid_parameter`**.

The consistent offset means it's **deterministic**, not the "flaky memory race" it appeared to be — it only *looked* flaky because the trigger condition isn't hit every spawn.

**Free stack sources exhausted (no admin):** WER `LocalDumps` (HKLM) denied; WER `ReportArchive\Report.wer` folders ACL-locked to SYSTEM; no `cdb`/WinDbg installed. Event log gives module+offset+code only (the abort *dispatcher*, not the culprit frame).

**Capture path set up (no admin):** downloaded **`procdump64.exe`** → `tools\procdump\`. User runs it before launching; it captures a **full dump** on the fast-fail. Analysis: open the `.dmp` in **Visual Studio** (MSVC symbols/`dusklight.pdb` present) for a symbolized stack, or Wind Clau attempts Python `minidump` parse. **Even better if VS is handy:** *Attach to `dusklight.exe` → enable break on all C++ exceptions → reproduce* — the debugger breaks at the actual `throw` with the exception type/message, naming the cause directly.

**Ownership:** capture tooling + analysis = **Wind Clau** (done: procdump staged); **reproduction = user** (one crash is enough); **Cursor** re-engages once the throw/abort site is named.

**Command (paste before launching the game):**
```
C:\Users\xxxxx\Documents\dusklight\tools\procdump\procdump64.exe -accepteula -ma -e -t -w dusklight.exe C:\Users\xxxxx\Documents\dusklight\crashdumps
```
Then reproduce the crash **once** (go straight to the reliably-crashing sequence — no need to replay the passing rooms; we need a single dump). Dump lands in `crashdumps\`.

---

### 2026-06-30 — Wind Clau: EXCEPTION TYPE captured — unhandled `std::out_of_range`

procdump caught it and decoded the C++ exception before the fast-fail:
```
[20:42:27] Exception: E06D7363.?AVout_of_range@std@@   ← thrown std::out_of_range
[20:42:27] Unhandled:  C0000409                        ← no catch → terminate → fast-fail
```
Full dump written: `crashdumps\dusklight.exe_260630_204227.dmp` (2.7 GB). (`406D1388` lines earlier = benign thread-naming exceptions.)

**Confirmed cause class:** an **unhandled `std::out_of_range`** — a bounds-checked access with a bad index/key (`vector::at()`, `map::at()`, `string::at/substr`, `stoi`, …) somewhere in the get-item demo path, thrown intermittently (hence "flaky"), with nothing catching it → `std::terminate` → `0xC0000409`. This retroactively explains every silent truncation.

**Getting the throw site:** because it's *unhandled*, the CRT calls `terminate` **without unwinding**, so the throwing frame is still on the crashing thread's stack in the dump. Open `…204227.dmp` in **Visual Studio** (auto-loads `dusklight.pdb` beside the exe) → the Call Stack shows the exact `.at()`/`at`/`substr` call and its caller in dusklight code. cdb/WinDbg not installed and WER folders are admin-locked, so VS is the fast symbolizer here.

**Next:** user pastes the VS call stack (top dusklight frames) → Wind Clau names the out-of-range access → **Cursor** fixes it (bounds guard / correct the index). This is a concrete, deterministic bug now — not a race, not a correlation.

---

### 2026-06-30 — ROOT CAUSE FOUND (stack from dump): unbound texmap in `color_arg_reg_info`, unguarded in release

VS call stack from `…204227.dmp` (throw site, since unhandled ⇒ no unwind before terminate):
```
std::bitset<8>::_Xran()                                      ← out_of_range thrown
std::bitset<8>::set(_Pos=?, bool)                            shader_info.cpp:? (Line 358 in <bitset>)
aurora::gx::color_arg_reg_info(GXTevColorArg, TevStage&, ShaderInfo&)   shader_info.cpp:61
aurora::gx::build_shader_info(const ShaderConfig&)           shader_info.cpp:210
aurora::gx::fifo::push_gx_draw(...)                          → build_shader_info
aurora::gx::fifo::process / drain
GXCopyTex(dest, clear)                                       ← EFB copy / depth resolve
drawDepth2(...)  →  mDoGph_Painter()  →  main01()
```

**Mechanism (fully confirmed in source):**
- [`shader_info.cpp:56–61`](../../extern/aurora/lib/gx/shader_info.cpp:56) — for a texture color arg (`GX_CC_TEXC/TEXA`):
  ```cpp
  CHECK(stage.texCoordId != GX_TEXCOORD_NULL, "tex coord not bound");
  CHECK(stage.texMapId  != GX_TEXMAP_NULL,  "tex map not bound");
  info.sampledTexCoords.set(stage.texCoordId);
  info.sampledTextures.set(stage.texMapId);   // ← line 61: throws
  ```
- [`internal.hpp:105–109`](../../extern/aurora/lib/internal.hpp:105): `#ifdef NDEBUG → #define CHECK(...)` **(empty)**. **RelWithDebInfo defines `NDEBUG`, so both `CHECK` guards are compiled OUT.**
- `sampledTextures` is `std::bitset<MaxTextures>` = **`bitset<8>`**; `GX_TEXMAP_NULL = 0xFF`. So `set(0xFF)` → `_Xran()` → `std::out_of_range` → nothing catches it → `std::terminate` → `0xC0000409`.

**⇒ A TEV stage reads a texture (`TEXC/TEXA`) but has NO texmap bound (`texMapId = 0xFF`).** In a Debug build the `CHECK` would `FATAL("tex map not bound")`; in this release build the guard vanishes and the unbounded index throws. Triggered while building the shader for a **`GXCopyTex` depth-resolve** using GX TEV state left resident by the WW bow draw — same **WW-cel-material-vs-Aurora** family as the earlier `tcg src 21` texgen sentinel (an out-of-range GX index Aurora doesn't handle), just an *unguarded-in-release* path this time.

#### Fix — two layers

**A) Aurora release-safety (immediate unblock; Cursor, ~4 lines).** The no-op `CHECK` followed by an unchecked `.set()` is a latent Aurora bug for *any* unbound-texmap TEV stage. Make it release-safe — skip instead of throw:
```cpp
case GX_CC_TEXC:
case GX_CC_TEXA:
  if (stage.texCoordId != GX_TEXCOORD_NULL && stage.texCoordId < info.sampledTexCoords.size())
    info.sampledTexCoords.set(stage.texCoordId);
  if (stage.texMapId != GX_TEXMAP_NULL && stage.texMapId < info.sampledTextures.size())
    info.sampledTextures.set(stage.texMapId);
  break;
```
Stops the fast-fail deterministically for all content. (Same guard belongs on the other `sampledTexCoords/sampledTextures.set()` sites, e.g. lines 134–135, 236–248, and `usesPTTexMtx`/`usedIndTexMtxs` for robustness.)

**B) Root — WW material TEV correctness (proper fix; Phase-2/3 material work).** Layer A stops the crash but a TEV stage reading a *nonexistent* texture will render wrong. Investigate why the WW bow's drawn material (`Vbow_v`, or leftover state from `SC_Vbow_v`) leaves a texture-input TEV stage with `texMapId = NULL` on Aurora — same material-programming gap as the texgen mismatch (**2N**). Likely the WW cel material's TEV/texmap bindings aren't fully re-issued for Aurora, or stale WW TEV state leaks into the `GXCopyTex` pass.

**Recommendation:** **do A now** (deterministic unblock — finally lets the replay run without the fast-fail), then fold **B** into the material pass (2N + Phase 3). Ownership: **Cursor** implements A in `extern/aurora`; the crash is now a named, deterministic bug — the multi-session flakiness was just how often a TEV stage happened to carry an unbound texmap. **2K stays**; re-run the multi-room gate after A.

---

### 2026-06-30 — Fix A+A′ extended gate playtest (user; doc only)

**User report:** **No crashes.** Extended run beyond 4 rooms. **Screenshots** going to Wind Clau (not in repo).

**Log:** `albw_ww_itemmdl_debug.txt` (session pointer `000001C1B5670F60`, frames 903→14660)

#### Stability (gate met)

| Metric | Result |
|--------|--------|
| Crashes / FATAL | **None** |
| `Replay finished (6 s).` | **16 consecutive** in this session |
| 2K | Active every draw (`2K: suppress SC_*`) |
| 2Q′ ptr audit | Stable (`raw_eq_private=1`, `parse_match=1`) throughout |

**Phase 2 stability gate: GREEN.** Fix A+A′ closed the crash class.

#### Visual (user + log room ids)

| Replay # | Log `room=` at spawn | User-visible color |
|----------|----------------------|-------------------|
| 1 | 1 | **OK** |
| 2 | 0 | **OK** |
| 3 | 3 | **OK** |
| 4 | 1 (post-warp) | **OK** (still in “first 3-ish” window) |
| 5 | **4** (first `room=4` in log) | **Flickering** — color angle-dependent |
| 6–16 | mostly 0 / 1 | **Colorless** (muddy / no cel fill) |

Progression is **monotonic with replay count**, not random per spawn — matches **Fix B** territory (WW material TEV/texmap not fully bound for Aurora; A+A′ neutral `vec4f(0.0)` on unbound TEXC/TEXA reads as missing texture contribution, worsening as GX/room state diverges).

#### What this means

| Layer | Status |
|-------|--------|
| **A+A′ (Aurora robustness)** | **Done** — no more terminate / tex255 WGSL FATAL |
| **Phase 2 mesh spin** | **Done** — bow visible, animates, survives many rooms |
| **Phase 2 cel color / Phase 3** | **Not done** — expected until **Fix B** (bind texmap on TEV stages; `setTevStr` / material programming) |
| **2N (tip / `SC_Vbow_v`)** | **Executed (2026-06-30)** — texgen bind + 2K suppress removed; user playtest gate pending |

#### Deliverable to Wind Clau

- **Screenshots** (user paste in Claude chat): early rooms with color → room 4 flicker → late rooms colorless.
- **Last ~20 lines** of `albw_ww_itemmdl_debug.txt` if needed — ends `frame=14660 phase=5: Replay finished (6 s).`

#### Next (wait for Wind Clau)

1. Wind Clau reads screenshots → confirms **Phase 3.0** (magenta isolate) vs straight **Fix B** / delegate `setTevStr`.
2. **Do not** add more Aurora guards — correctness is WW material side.
3. **Cursor** implements only after **Execute [Phase 3.x / Fix B]** from Wind Clau.

#### Wind Clau decision (2026-06-30): SKIP 3.0 magenta → go to Fix B, first step = TEV-order dump

**Gate green is the headline — Phase 2 mesh goal is met** (spins, animates, survives rooms, 16 replays, no crash). The colorless bow is the *expected* consequence of A+A′ zeroing an unbound-texmap TEV stage, not a new problem.

**Skip Phase 3.0 (magenta isolate) — the crash dump already answered what 3.0 was designed to test.** 3.0 existed to decide "is the darkness TEV/lighting or texture?" We now know from the fault: a texture-sampling TEV stage has `texMapId = NULL`, so **the texture isn't sampled at all** (A+A′ substitutes `vec4f(0.0)`). A flat-magenta DDS would therefore show **no magenta** — it would only re-confirm "texture not sampled," which we already know. Redundant; don't spend a session on it. (A flat-color texture is still useful *after* Fix B, as a sampling sanity check.)

**Go straight to Fix B — but its first step is a TEV-ORDER DUMP, not a blind bind.** Reason: the color timeline is **not** "always colorless." It was **OK for replays 1–4, then lost at room 4 / replay 5, monotonic after.** If the material's `texMapId` were a static NULL stub, color would be missing from replay 1. It wasn't — so `texMapId` is **valid early and drifts to NULL**, i.e. Aurora reads it from **runtime GX state** that starts correct (inherited) and diverges. We must know *which* before patching:

- **2I-style dump (log-only):** for `Vbow_v` and `SC_Vbow_v`, log each TEV stage's `texMapId` + `texCoordId` (`GXSetTevOrder` state) **both at load and at draw, across several replays**. Exactly parallel to the 2I texgen dump that cracked the `tcg src 21` case.
- **If the material struct's TevOrder texMapId is valid but runtime drifts to NULL** ⇒ Fix B = have the WW bow **re-issue `GXSetTevOrder(stage, texcoord, texmap, chan)` with valid bindings every draw** (don't rely on inherited GX state).
- **If it's a static NULL stub** ⇒ Fix B = repair the TevOrder texmap/texcoord in `dWwItemmdl_patchModelForPc` (which is *still* a no-op logger — this is where it finally does real work), same "baked-DL struct is a stub Aurora can't read" family as the texgen fix.

**After the texmap binds and the real texture samples:** if color returns → Phase 2 truly complete, move to **2N** (edge/tip). If it samples but is still dark/muddy → *then* the original struct-14 TEV-lighting question (old **3.1**: delegate `setTevStr` / struct 0) applies, on top. So the order is: **Fix B (bind texmap) → observe → 3.1 only if sampled-but-dark.**

**Recommendation: Execute Fix B, starting with the TEV-order dump.** Skip 3.0. No more Aurora guards.

#### Fix B step 1 — TEV-order dump (Cursor, 2026-06-30)

**Status:** Implemented — log-only, awaiting playtest dump paste.

**What shipped:**
- `dWwItemmdl_logTevOrderDump()` in `d_ww_itemmdl_pc.cpp` / `d_ww_itemmdl_pc.h`
- Tags: `2B tevorder [phase room=N]: …`
- Materials: **`Vbow_v`** and **`SC_Vbow_v`** only
- Per TEV stage: **`struct texMap` / `texCoord` / `colorChan`** from J3D `getTevOrder`
- Runtime GX (draw only): **`gx texMapId` / `texCoordId` / `channelId`** from `g_gxState.tevStages[]` + `numTevStages`
- **`NULL`** printed when id = `0xFF`

**When it logs:**
| Phase | Where |
|-------|--------|
| `after loadBinaryDisplayList` | `finishLoadedModelData` |
| `after makeSharedDL` | `finishLoadedModelData` |
| `patchModel` | `dWwItemmdl_patchModelForPc` |
| `draw-pre-setTevStr` | first visible draw per spawn (`d_a_demo_item.cpp`) + GX |
| `draw-pre-dl` | same spawn, before `modelUpdateDL` + GX |
| `draw-post-dl` | same spawn, after `modelUpdateDL` + GX |

**Playtest:** Editor WW itemmdl get-item **ON**, 2D isolate **OFF**. Run **≥5 replays** (need room 4+ in log). Paste all `2B tevorder` lines from `%USERPROFILE%\Documents\dusklight\albw_ww_itemmdl_debug.txt`. Wind Clau picks: runtime drift → per-draw `GXSetTevOrder`; static stub → repair in `patchModelForPc`.

**Do not:** Phase 3.0 magenta; more Aurora guards; 2N until Fix B bind lands.

#### Fix B step 2 — per-draw GX TEV bind (Cursor, 2026-06-30)

**Dump verdict (Wind Clau):** Material struct TevOrder **VALID** (`Vbow_v` texMap 1/0, `SC_Vbow_v` 2/0/1). Runtime `g_gxState` stuck at `numTev=1` / `texMapId=255` every draw — locked baked-DL draw does not realize material TEV into Aurora's pipeline. **Not** a `patchModelForPc` struct repair.

**Status:** Implemented — awaiting playtest (color + optional post-bind dump).

**What shipped:**
- `dWwItemmdl_applyTevOrderForDraw()` — reads **`Vbow_v`** `getTevOrder` per stage → `GXSetTevOrder(st, texCoord, texMap, GX_COLOR_NULL)` → `GXSetNumTevStages(nTev)`
- Called **immediately before** `mDoExt_modelUpdateDL` on every WW bow draw (`daDitem_c::DrawBase` override + first-frame bracket path)
- Log: `2B apply: Vbow_v nTev=N st[i] map=… coord=…`
- **No** material unlock / `diff()`

**Playtest gate:** ≥5 replays, stable, bow **colored** (not colorless). If samples but dark → **3.1** (`setTevStr` delegate). If color OK → **2N** edge/tip. Note: `2B tevorder` gx snapshot lines may still show pre-drain stale state; trust visual + `2B apply` log.

#### Fix B confirmed + Phase 3.1 — actor lighting (2026-06-30)

**Fix B:** Playtest confirmed — `2B apply` binds `Vbow_v` texmaps; bow **colored**, **stable**, zero crashes. Root crash + colorlessness closed.

**3.1 (Cursor):** WW bow `setTevStr`: `settingTevStruct(14)` → `settingTevStruct(0)` + `setLightTevColorType` (non-MAJI; MAJI fallback if fill looks wrong). Targets view-dependent outdoor darkening from struct-14 ambient/view-matrix lighting.

**Playtest gate:** ≥5 replays indoor + outdoor — even flat fill, no camera-angle swing. Pass → **2N** edge/tip.

#### 3.1 playtest fail (user, 2026-06-30) → **3.1′**

**Symptom:** 2 indoor replays stable; **3rd replay (room 3, outdoor)** color unstable then **crash** ~frame 1753 (~1 s into spin). Log shows CPU draw path complete; no `Replay finished`.

**Diagnosis:**
- First 3.1 used `setLightTevColorType` — **empty stub on PC** (`d_kankyo.cpp`), so materials never received actor lighting; struct 0 tevStr alone couldn't fix outdoor swing and may have left GPU state mismatched outdoors.
- Per-frame `2B apply` / `2Q'` logging (~180 lines/replay) added I/O noise; not root cause but throttled.

**3.1′ (Cursor):**
- WW bow: **`daItemBase_c::setTevStr()`** delegate = struct **0** + **`setLightTevColorType_MAJI`** (Wind Clau fallback; MAJI safe post–Fix B).
- Fix B: **`GXSetTevOrder` before AND after** `mDoExt_modelUpdateDL` (fifo sees WW TEV after locked DL replay).
- Throttle: patchModel tevorder dump once/process; remove per-frame apply log.

**Retest:** ≥5 replays indoor + outdoor; need stable + even fill + no crash.

#### 3.1′ playtest (user, 2026-06-30) — **stable, wrong direction** → Wind Clau review

**What shipped (3.1′):** WW bow delegates to `daItemBase_c::setTevStr()` (struct **0** + **MAJI**); Fix B TEV bind **before and after** `mDoExt_modelUpdateDL`; diagnostic log throttled.

**User report (3 replays, quit early):**
- **Stability:** ✅ No crash (contrast with first 3.1: crash on replay 3 outdoors).
- **Visual:** ❌ **Wrong direction** — not the expected flat/even outdoor fill. User did not finish ≥5-replay gate; stopped after 3 tries because stability was OK but lighting/color regressed vs the **Fix B baseline** (colored, struct-14 outdoor swing was the prior complaint; 3.1′ made things worse, not better).

**Open questions for Wind Clau:**
1. Is struct **0** + MAJI on **shared cached** `J3DModelData` the wrong pairing for WW locked `bdl3` (MAJI mutates materials every frame on the parse-once cache)?
2. Should we **revert to struct 14 + MAJI** (or struct 14 + no MAJI) now that Fix B binds texmaps — i.e. keep Fix B bind, undo only the struct-0 delegate?
3. Hybrid: struct **0** tevStr setup **without** MAJI material patch (manual amb only)? Or **3.1b**: struct 0 + MAJI but only on the **per-spawn** `J3DModel` instance, not shared `J3DModelData`?
4. Double Fix B apply (pre+post `modelUpdateDL`) — keep, drop one side, or move post-only?

**Cursor gate (superseded):** Wind Clau picked **revert + 2N** — see **Cursor executed: REVERT + 2N** below. No struct-0 or non-MAJI retries.

**For Claude:** Read this file from **Fix B confirmed** through this section; baseline to beat is **Fix B only** (colored, stable, outdoor angle swing) — not the pre–Fix B colorless path.

#### Wind Clau call (2026-06-30): REVERT the struct-0 delegate → keep Fix B baseline → go to 2N. Swing is optional polish.

**My own correction first:** I recommended "try non-MAJI `setLightTevColorType` first." That was wrong for PC — it's an **empty stub** (`d_kankyo.cpp`), so it applies no lighting and left GX state mismatched outdoors → the 3.1 crash. **On PC, `setLightTevColorType_MAJI` is the only functional lighting apply; plain is a no-op. Don't retry non-MAJI.**

**Where the two struct-0 attempts landed:**
| Config | Stability | Color |
|--------|-----------|-------|
| **Fix B baseline** (struct 14, MAJI-skipped, + texmap bind) | ✅ | ✅ colored; mild outdoor angle swing (only remaining nit) |
| 3.1 (struct 0 + non-MAJI stub) | ❌ crash outdoor | — (no lighting applied) |
| 3.1′ (struct 0 + MAJI delegate) | ✅ | ❌ **worse** — wrong direction, not flatter |

**Verdict: Fix B baseline is the best result; both struct-0 moves regressed. Revert to it.** My struct-14→0 hypothesis for the swing didn't pan out — struct 0 either does nothing (non-MAJI stub) or, with MAJI, feeds the WW cel material lighting values that push color the *wrong* way. The swing under struct 14 is a **minor, view-dependent nit** exaggerated by deliberately orbiting the camera; in a normal brief get-item spin it's unlikely to read as broken. It is **not worth trading the good colored baseline for.**

**Answers to the open questions:**
1. struct 0 + MAJI on the shared cached `J3DModelData` — plausibly part of why color went wrong (MAJI patches the cel material's TEV colors against struct-0 values it wasn't baked for), but not worth chasing — revert instead.
2. **Yes — revert the struct-0 delegate; keep the Fix B texmap bind.** This is the pick.
3. Hybrid amb-only / per-spawn-instance MAJI: possible *later* polish for the swing, but the non-MAJI path is a stub so "amb-only" needs custom tevStr code — **defer**, don't block on it.
4. Double Fix B apply (pre+post `modelUpdateDL`): revert to whatever the **known-good colored Fix B baseline** used (minimize variables). If baseline was single-apply, drop the extra; if it was already fine, keep. Not the color problem either way.

**Next: revert to Fix B baseline (struct 14 + texmap bind, MAJI-skipped), confirm colored+stable over ≥5 replays, then Execute 2N** (re-enable `SC_Vbow_v` edge/tip via the `GXSetNumTexGens` texgen fix) — that's the last piece of the WW look and is independent of fill lighting. **Outdoor-swing flattening = optional final polish**, only if it bothers in normal play; if pursued, it's a custom ambient-only tevStr, not another stock struct swap.

#### Cursor executed: REVERT + 2N (2026-06-30)

| Change | Detail |
|--------|--------|
| **3.1 / 3.1′ reverted** | `setTevStr()` back to `settingTevStruct(14)` + MAJI-skipped for WW bow (`d_a_demo_item.cpp`). |
| **Double apply dropped** | Single `dWwItemmdl_prepareWwBowGxForDraw()` before `mDoExt_modelUpdateDL` only (`wwBowDrawModel`). |
| **2K removed** | No `suppressOutlineForDraw` / `restoreOutlineAfterDraw` on draw path — both materials draw. |
| **2N added** | `dWwItemmdl_applyTexGenForDraw()` — `GXSetNumTexGens` + per-slot `GXSetTexCoordGen` from material with max texgen count (usually `SC_Vbow_v` = 3); then Fix B TEV bind for `Vbow_v`. |
| **Build** | `build_run.bat` ✅ |

**Playtest gate:** WW itemmdl get-item ON, 2D isolate OFF. Run ≥5 replays (mix indoor/outdoor). Pass = colored body + visible ink edge/tip, no crash. Log: `%USERPROFILE%\Documents\dusklight\albw_ww_itemmdl_debug.txt`.

#### Fix B step 1 — Wind Clau read of the dump (2026-06-30): struct VALID, runtime GX never has the WW TEV → per-draw `GXSetTevOrder`

Read all 322 `2B tevorder` lines across the session (rooms 1→0→3→3→1→4→1→0→0). Result is clean and consistent:

| Source | Finding |
|--------|---------|
| **Material struct** (`getTevOrder`) | **VALID & identical everywhere** — `SC_Vbow_v`: 3 stages `texMap=2/0/1`, `texCoord=2/0/1`; `Vbow_v`: 2 stages `texMap=1/0`, `texCoord=1/0`. Same at load, after `makeSharedDL`, `patchModel`, and **every** draw phase. `colorChan=255` (no raster channel — normal for a texture-modulate cel material). |
| **Runtime GX** (`g_gxState.tevStages`) | **Constantly `numTev=1`, `st[0] texMapId=255 (NULL) texCoordId=0 channelId=4`** — pre-dl *and* post-dl, in every room, every replay. A single raster-only stage with **no texture**. The WW material's 2–3 textured stages are **never** in the live pipeline. |

**Verdict: this is the "struct valid, runtime doesn't have it" branch → NOT a `patchModelForPc` struct repair.** The asset is correct; the locked baked-DL draw (`mDoExt_modelUpdateDL`) is **not realizing the material's TEV order into the GX pipeline** — Aurora builds shaders from a leftover/default 1-stage/`texMapId=255` state, so the texture never samples → colorless (and A+A′'s `vec4f(0.0)` fallback kicks in).

**Nuance (why color was intermittent, not always-off):** Aurora's GX is a **deferred fifo** — `g_gxState.tevStages` at draw time is the *last-drained* state, and the shader that actually renders is built later at fifo drain / `GXCopyTex` time from whatever TEV is current *then*. So the snapshot being uniformly `numTev=1/255` is the leftover state; the real shader sometimes drains with the WW TEV present (color) and sometimes with the leftover 1-stage (colorless/dark), depending on draw order — which is exactly the **angle/room-dependent** color the user saw. The snapshot confirms the WW TEV isn't *reliably* established.

**Fix B (the bind): explicitly establish the WW material's TEV order in the live pipeline every draw**, sourced from the (valid) struct values — `GXSetNumTevStages(nTev)` + per-stage `GXSetTevOrder(stage, texCoord, texMap, GX_COLOR_NULL)` for `Vbow_v` (and `SC_Vbow_v` when its edge pass is later re-enabled), issued so Aurora's fifo drains them as the WW bow's own TEV rather than inheriting the leftover 1-stage/`255`. Read the exact `texMap`/`texCoord` per stage straight from `getTevOrder` (they're already correct in the struct). **Do not unlock the material for `diff()`** (handoff-documented crash) — establish the TEV order alongside the locked-DL draw. Implementation mechanism is Cursor's call (the locked baked-DL not emitting TEV order to Aurora is the same "baked-DL struct-vs-Aurora-pipeline" family as the texgen/`tcg src 21` issue).

**After the texmaps bind and textures sample:** color should return. If it samples but is still dark/muddy → *then* the original struct-14 TEV-lighting question (old **3.1**: delegate `setTevStr` / struct 0) applies on top. Order stays: **bind (Fix B) → observe → 3.1 only if sampled-but-dark → 2N edge/tip.**

**Recommendation: Execute Fix B bind — per-draw `GXSetNumTevStages` + `GXSetTevOrder` from the valid struct values. Not a struct repair.**

#### Fix B bind SUCCEEDED → remaining fluctuation is struct-14 lighting → Execute 3.1 (2026-06-30)

Fix B bind landed (`2B apply: Vbow_v nTev=2 st[0] map=1 coord=0 st[1] map=0 coord=0`). **Texture sampling restored — the bow is colored.** Stability holds (no crashes). The `gx` snapshot still shows `numTev=1/255` = the known pre-fifo-drain artifact; trust visual + the `2B apply` line.

**Remaining symptom → diagnosis:** color is **stable indoors** (dungeon, house) but **fluctuates light↔dark by camera angle outdoors**. That is the signature of **`settingTevStruct(14)`** — TP's get-item TEV struct with dark ambient (~24,24,24) + **view-matrix lighting**. View-space lighting ⇒ brightness tracks camera angle; indoor uniform ambient reads stable, outdoor directional light swings as you orbit. The WW bow is still on struct 14 (MAJI skipped). This is **sampling-correct but view-dependently lit** — a lighting-struct problem, i.e. **Phase 3.1**, *not* 2N (edge is orthogonal and won't affect the swing).

**Execute 3.1:** in `daDitem_c::setTevStr()` (`d_a_demo_item.cpp:444`), for the WW bow switch `settingTevStruct(14)` → **`settingTevStruct(0)` (`TEV_TYPE_ACTOR`)** + `setLightTevColorType`. Try **non-MAJI `setLightTevColorType` first** (most TWW-faithful; TWW demo items don't use the get-item struct). MAJI is now likely safe (the old crash was the texmap bind, not MAJI) — fall back to it only if non-MAJI looks wrong. Expected: flat, even cel fill; no outdoor/angle darkening.

**Then:** even texture-correct fill → **2N** (re-enable `SC_Vbow_v` edge/tip via the `GXSetNumTexGens` texgen fix). Sequence: Fix B (bind) ✅ → 3.1 (lighting) → 2N (edge).

---

## Interconnected pass — 2026-06-30 (Cursor + Wind Clau sync)

### Session crashes (this build cycle)

| Attempt | Symptom | Root cause | Fix |
|---------|---------|------------|-----|
| REVERT+2N (dual mat, no 2K) | `tcg src 21` index 2 `numTexGens 0` | `J3DLockedMaterial::makeSharedDisplayList()` is **no-op** → empty shared DL | **2N′** force `J3DMaterial::makeSharedDisplayList()` once at parse |
| 2N′ + single pass | Stable colored fill; fragments / missing arrow | Dual-material **single GX state** — Fix B binds Vbow_v TEV only; SC needs 3 stages | Attempted two-pass / per-shape (below) |
| Per-shape dump loop | **AV at patchModel** (countVertex, then dlBytes/name) | **Removed** — summary line only: `count=2 mats=2 joints=2` |

GPU cache wipe required after each bad shader compile (`dawn_cache.db*` + `pipeline_cache.db*`).

### 2S shape inventory (partial — crash truncated first run)

```
2S shape [patchModel]: count=2 mats=2 joints=2
```

(Log stopped before per-shape lines — `countVertex` AV. **Fixed** in latest build; next run should print both `sh[0]` and `sh[1]` with mat names.)

**Wind Clau re-analysis (accepted):** Image 2 = full WW bow with nocked arrow (not TP). Color tuning was premature while geometry incomplete. With **count=2**, the “>2 shapes” branch is **closed**. Remaining geometry hypotheses:

1. **Dual-material GX mismatch** — single pre-bind cannot serve SC (3 TEV / 3 texgen) and Vbow (2/2) in one `modelUpdateDL`; fragments / missing ink+arrow fit this better than missing shapes.
2. **Submesh inside one shape** — arrow may live inside the `Vbow_v` shape DL; missing arrow = texmap/TEV stage not sampling, not a third shape.
3. **`shape->hide()` on locked DL** — still untested on stable path; per-shape attempt was inconclusive due to crash.

### Active code state (post interconnected pass)

| Layer | State |
|-------|--------|
| **A+A′** | Keep — Aurora unbound texmap guards |
| **2N′** | Bake locked shared DL **once** per cached `J3DModelData` |
| **Fix B** | Single pre-`modelUpdateDL` texgen (max slot) + Vbow_v TEV bind |
| **Lighting** | Struct **14**, MAJI-skipped (Fix B baseline) |
| **2K** | Off — both materials draw |
| **2S dump** | Safe metadata at `patchModel` + first `draw-pre-dl` |
| **Draw** | Single `mDoExt_modelUpdateDL` — **not** per-shape multi-pass |

### Wind Clau — pick next geometry branch (after user paste full 2S lines)

| If dump shows… | Likely fix |
|----------------|------------|
| `sh[0]=SC_Vbow_v`, `sh[1]=Vbow_v`, both `dlBytes>0` | **Per-material sequential draw without hide** — e.g. two `modelUpdateDL` passes binding SC GX then Vbow GX, *without* rebaking DL / without `countVertex`; or Aurora hook after each mat `callDL` |
| Arrow in `Vbow_v` shape (large `dlBytes`) but invisible | Fix B must bind **per-material TEV at mat draw time**, not one global Vbow-only bind before whole model |
| `hidden=1` unexpectedly | Loader / visibility flag bug |

**Do not:** struct-0, non-MAJI, `countVertex` in dump, per-shape hide loop, rebake shared DL every spawn.

**Deferred:** struct-14 outdoor swing polish until SC per-material GX + screenshot geometry read complete.

---

## Interconnected pass — playtest sync (2026-06-30, post dump-fix)

**Milestone:** First playtest where the bow **actually renders** (2S per-shape dump no longer AVs at load).

| Gate | Result |
|------|--------|
| Crashes | ✅ None (user: "very stable") |
| `Replay finished (6 s)` | ✅ 7× consecutive in log (frames 934–4392) |
| Indoor color | ✅ Stable |
| Outdoor color | ⚠️ Unstable (struct-14 swing pattern) |
| Brightness | ⚠️ **Very bright / almost white** — new symptom; Wind Clau + screenshots |
| Geometry | 📷 User screenshots → Wind Clau (whole bow TBD) |

**For Claude chat:** Attach screenshots + paste **Fresh Wind Clau opener** from **▶ RESUME HERE** above.

---

## ▶ Interconnected pass — 2B″ playtest (2026-06-30)

**User report (post 2N″ + 2B″):** Still **monochrome**, **unstable**; maybe **slightly more color indoors** (user unsure). Geometry unchanged (whole bow + arrow). Still stable.

**Log verdict (grep `2B apply post-dl`):** **Zero matches** across full session. The 2B″ implementation never executed at mat-draw time.

**Why 2B″ could not work (architecture):**

```
daDitem draw()  →  begin/end scope (2B″: only inside drawWwBowModel)
    modelUpdateDL  →  entry() queues J3DMatPacket on DB_OPA_LIST
    [callback cleared here in 2B″]
… later same frame …
mDoGph_Painter  →  dComIfGd_drawOpaList()
    J3DMatPacket::draw  →  callDL(); [hook]; shape drawFast   ← hook must be live HERE
```

**Material struct still valid** (unchanged): `Vbow_v` texMap 1/0 · `SC_Vbow_v` 2/0/1 · texgen slot-1 COLOR0 vs NRM mismatch only matters for **global** pre-bind (2N″), not per-mat bind.

**Attempts timeline (color):**

| Step | Change | Color result | Log signal |
|------|--------|--------------|------------|
| Fix B baseline | Pre-bind Vbow TEV + 2K suppress SC | ✅ Colored body | `2B apply` (old path) |
| REVERT+2N | SC max texgen, both mats | ❌ Monochrome | — |
| 2N″ | Vbow global texgen pre-bind | ❌ Still monochrome | — |
| 2B″ | Per-mat hook after `callDL` | ❌ Still monochrome | **No `2B apply post-dl`** |
| **2B‴** | Hook scope → OPA list flush | **Pending replay** | Expect 2× `2B apply post-dl` |

**2B‴ code (Cursor):** `beginBowDrawScope` in `DrawBase` / bracket path · `endBowDrawScopeIfActive` after `DB_OPA_LIST` in `drawOpaDrawList` · `J3DSys::MatDrawPostDlCallback` in `J3DMatPacket::draw` after `callDL`.

**Wind Clau decision tree after 2B‴ replay:**
1. **`2B apply post-dl` ×2 + colored** → struct-14 outdoor swing polish only (optional).
2. **`2B apply post-dl` ×2 + still monochrome** → inspect Aurora `sampledTextures` / fifo drain; consider 2K suppress SC to restore Fix B baseline body color without losing Vbow geometry.
3. **Still no hook lines** → demo item entries not on `DB_OPA_LIST`; bisect draw-buffer target.

**Do not regress:** 2N′ bake-once, single-pass, full bow geometry.

### Wind Clau — 2B‴ WORKS (true WW color achieved); flicker = per-flush callback scope (2026-06-30)

**Milestone:** with `struct 0` + skip-MAJI + the `2B‴` per-material post-DL bind, the bow renders **true WW orange** (user screenshot). The TWW-decomp recipe is correct. Remaining issue is a **rapid, consistent every-frame flicker** between true-color and monochrome (esp. outdoors), reproducible every replay — NOT rare.

**Log caveat:** `2B apply post-dl` has a **log-once guard** (`s_logged_vbow`/`s_logged_sc`, `d_ww_itemmdl_pc.cpp:912–933`) → prints twice regardless of fire rate. Count is NOT the fire rate.

**Flicker mechanism:** `dWwItemmdl_beginBowDrawScope` sets the material callback; `dWwItemmdl_endBowDrawScopeIfActive` **clears it after one `DB_OPA_LIST` flush**. But Dusk **frame-interpolates** (258 FPS seen; `frameInterp` in logs) — logic-frame flush has the callback set → per-material GX → **color**; interpolated re-flushes run with the callback already cleared → vbow mats draw with stale/default GX → **monochrome**. ⇒ rapid logic-vs-interp flicker.

**Fix (low-risk): make the post-DL callback PERSISTENT for the bow's lifetime** — set at spawn/first-draw, clear at `Delete` — not per-OPA-flush. The callback already guards on material (`isVbowDrawMaterial`, `:905`) so it **no-ops for all other materials/draws** — safe to leave set. Then every flush (logic + interpolated + any 2nd pass) re-binds the correct per-material GX → steady color.

**2B‴+ persistent (Cursor):** `beginBowDrawScope(model, ownerId)` in `CreateInit` · `clearBowDrawScope` in `Delete` · **4A:** owner-alive check in hook · `notifyRoomChange` + replay cancel on warp.

**Confirm (optional):** drop the log-once guard for one replay → hook fires on logic frames, not on interp re-flushes = confirmed.

**Fallback if persistent callback doesn't fix it:** interp is re-*presenting* a cached command buffer (not re-*flushing*) → bake GX into the packet, or exclude the get-item demo from interpolation.

**Do NOT regress:** struct-0 + skip-MAJI + 2B‴ per-material bind (this is what produced true color), geometry, bake-once.

### ✅ COLOR HOLDS — persistent callback fixed the flicker (2026-06-30). Next-steps roadmap.

**Milestone:** persistent post-DL callback → bow holds true WW color **consistently indoors AND outdoors** (warp not yet tested). Get-item bow is functionally DONE (textured, whole, stable). Remaining = polish + scope. Recipe locked: `struct-0` + skip-MAJI + `2B‴` per-material bind + 2N′ bake-once + full geometry.

**Immediate (before more polish):**
1. **Checkpoint/commit** this working state (hard-won; guard against a revert).
2. **Test WARP + repeated room changes** — untested, and transitions are the historically crash-prone path.
3. **AUDIT the persistent callback lifetime (real risk).** `wwBowMatDrawPostDl` is now set persistently and holds `s_wwBowDrawModelData` — a **static pointer with a lifetime**, same bug class as the earlier `s_cached` / `s_suppressedOutlineShape` UAFs. Must be nulled on `Delete` and reset cleanly across warp, or it's the next use-after-free. Verify before trusting warp.

**Polish (too bright / not-yet-cel):**
4. **Brightness/washed detail:** current skip-MAJI leaves light regs at blown defaults → fill multiplied toward white. Try **struct-0 + MAJI** (real moderate light; only tried unfairly at 3.1′ pre-texture) — if MAJI overwrites baked cel colors, fall back to **skip-MAJI + a manual neutral ambient register**.
5. **Ink outline (`SC_Vbow_v`):** the pass that gives WW its black cel outline — binding now; check if crisp vs washed (fixing #4 may reveal it), tune TEV/blend if needed. Biggest "actually-WW" lever.
6. **Enable BTK spin (`dRes_INDEX_ITEMMDL_BTK_VBOW_e = 0x24`):** authentic WW get-item rotation, deferred until color stable — now viable. Cheap authenticity win.

**Scope — "out of get-item state":**
7. **Track B: held bow** — Link holds `vbow` when equipped (aim/shoot). Documented in `wind-waker-item-work.md` (Track B: `setBowModel()` in `d_a_alink_bow.inc`, hand-matrix tuning). New sub-project; the per-material bind + texgen/TEV recipe + deferred-draw timing all carry over.

**Recommended order:** 1–3 (checkpoint + warp + callback-lifetime audit) → 4 (brightness) + 6 (BTK) cheap wins → 5 (ink outline) biggest look gain → 7 (held bow) scope jump.

**Authenticity note:** rendering a WW asset through TP/Aurora — can get close (texture + ink + toned brightness + BTK spin) but not guaranteed pixel-perfect WW cel. Current state is already the hard 90%.

**TARGET clarified (user, clean WW reference render):** in-game WW Hero's Bow = **bright orange recurve with SOFT cel gradient** (darker orange edges → lighter mid-limb), **white/silver ornamental tips**, **nocked arrow** (wood shaft, feather fletching, gold head). The get-item **light beams are reward-only flair, NOT the bow** — keep them in the get-item state but they wash the bow during evaluation. Implications:
- **#4 brightness is THE priority** — current overbright blows out the orange gradient, white tips, and arrow detail this reference depends on. Tune exposure down until those read. This reference is the concrete bar to tune against.
- **#5 ink outline = SUBTLE**, not heavy black — reference shows soft cel shading, not thick outlines. Don't over-do `SC_Vbow_v`.
- **Judge the bow OUTSIDE the get-item flash** — held-bow (Track B / #7) shows the true lit look with no flash overlay, so #4 and #7 reinforce each other.

### Wind Clau — TWW-decomp-grounded plan (2026-06-30, user pointed back to the decomp research)

2B‴ is the right fix for **texture sampling** (deferred J3D mat-packet draw). But even after it fires, we're still drawing with **TP's `struct 14`**, which the TWW decomp names as the cause of dark/muddy WW cel fill. **The flat WW look (reference: WW "You got the Hero's Bow" — even orange/tan, pale tips, no swing) needs the TWW draw recipe, not just per-material texgen.** Per [`wind-waker-item-work.md:116`](../wind-waker-item-work.md) TWW draws get-item demos with:
- `settingTevStruct(**TEV_TYPE_ACTOR = 0**)` — NOT `struct 14`
- `setLightTevColorType` (non-MAJI) — **PC stub**, so PC needs MAJI *or* minimal/no override (cel color is baked in the TEV)
- **`setListMaskOff()`** — NOT dark list (**never tried** = the "wrong draw buffer" branch)

**Two TWW pieces still un-landed** (both were blocked/unfairly tested before):
- **struct-14 → struct-0:** "tried" at 3.1 but **before texture sampled** (2B″ hook was dead) → unfair; struct-0 applied no-op lighting to an untextured surface. **Re-test after 2B‴.**
- **`setListMaskOff`:** never attempted.

**Sequenced path to the flat look:**
1. **2B‴** → confirm `2B apply post-dl` ×2 → texture samples (color returns, likely still dark/swinging).
2. **Land the TWW draw recipe:** `struct 14 → 0` **+** `setListMaskOff`. PC lighting caveat: non-MAJI is a stub, so use **struct-0 + MAJI** *or* **skip the lighting override entirely** (let the baked cel TEV show) — try minimal-override first for a baked material.
3. Residual outdoor swing → optional ambient polish.

**Refined branch #2 (colored-hook fires but still monochrome/dark):** most likely **struct-14's dark ambient crushing the TEV**, not a fresh fifo bug → **land TWW draw (struct-0 + mask-off) before chasing `sampledTextures`.** Decomp says struct-14 darkens WW cel fill even with a sampling texture.

**Do NOT retry:** struct-0 *without* fixing texture first (that was 3.1's unfair test), non-MAJI as the *sole* light apply (PC stub), per-shape dump, double-`modelUpdateDL`.
