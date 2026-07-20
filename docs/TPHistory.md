# TPHistory — Twilight Princess Development Archaeology

**Status: research COMPLETE (all three lanes — local decomp/disc evidence §1–4, cross-disc archaeology §5, community/TCRF corroboration §6).** Findings since productized into in-game tooling: the Demo Leftover Viewer (`d_demo_leftover_viewer`, 365 cataloged cutscene-arc models) and the Cut Actor Spawner (`d_cut_actor_spawn`) — see `tools/demo_cut_content/README.md`.
What TP reused from The Wind Waker, what was planned and cut, and what the data itself
proves about how TP was built. Evidence hierarchy: **byte-identical disc data > shipped
stubs/symbol maps > decomp source > community record (TCRF etc.)**. Local sources:
`D:\XXXXXXX\Ex TP\files` (TP GC USA extraction), `D:\XXXXXXX\Ex WW\files` (WW extraction),
`assets/<discid>/res` (per-disc resource indexes: GC demo DZDE01, GC US/JP/PAL, Wii
revisions, Shield port), and this repo's decomp source. Cross-refs:
[wind-waker-item-work.md](wind-waker-item-work.md) (itemmdl technical work + earlier
cut-enemy research this consolidates).

---

## 1. The headline: TP is "dolzel2"

Wind Waker's internal build tree is `d:\zelda\dolzel\bin\Final\...` (its
`framework.str` module path table). TP's is `D:\zeldaGC_USA\dolzel2\bin\Final\...`
(`str\Final\Release\frameworkF.str`). **TP's internal project name is literally
`dolzel2`** — at the toolchain level, Twilight Princess is Wind Waker 2. Build dates:
WW `COPYDATE` = 2003-02-19 (US build); TP `COPYDATE` = 2006-11-06.

Everything below is downstream of that fact: same engine family (JSystem: JKernel /
J3D / JStudio / JAudio lineage), same actor framework (`fopAc`, `d_a_*` modules as
RELs), same stage format family (RARC arcs, DZR/DZS chunks, BMD/BTI/BCK resources),
same disc layout (`Audiores`, `RELS.arc`, `res/{ActorDat, CardIcon, ItemTable, Object,
Particle, Stage}` on BOTH discs).

---

## 2. Byte-identical Wind Waker data on every TP disc (~450 KB)

Verified by `cmp` between the two extractions — not similar, **bit-for-bit WW files**:

### 2.1 `itemmdl.arc` — WW's item presentation layer, consolidated (21 models)

TP's `res/Object/itemmdl.arc` holds 21 `v*.bdl` WW item view models + 12 anims.
**16 of 21 models are byte-identical to WW's shipped files**, plus all 8 tested
companion `.btk/.brk` anims:

| TP itemmdl member | WW original arc | Item |
|---|---|---|
| vtact.bdl | Vtact.arc | **The Wind Waker baton itself** |
| vleaf.bdl | Vleaf.arc | Deku Leaf |
| vhamm.bdl | Vhamm.arc | Skull Hammer |
| vbow.bdl | Vbow.arc | Hero's Bow |
| vboot.bdl | Vboot.arc | Iron Boots |
| vhook.bdl | Vhook.arc | Hookshot |
| vrope.bdl | Vrope.arc | Grappling Hook |
| vboom.bdl | **Boom.arc** | Boomerang (non-V name — invisible to name-diffing) |
| vtele.bdl | **tele_00.arc** | Telescope (ditto) |
| vbage/vbagf/vbagh.bdl | VbagE/F/H.arc | Bait / Spoils / Delivery bags |
| vboxn.bdl, vchin.bdl, vho.bdl, vmags.bdl | VboxN/Vchin/Vho/VmagS.arc | misc |

**UPGRADE (research pass 2, full-disc SHA sweep): it is 21 of 21.** The five
"non-identical" models were compared against the wrong WW arcs — the bottles live in
WW's medicine arcs and the bomb in its own: `vbinb.bdl == Med_b.arc`, `vbing ==
Med_g.arc`, `vbinr == Med_r.arc`, `vbinx == Bin.arc`, `vbomb == Bomb_00.arc` — all
byte-identical, as are all four `vbina_*.btk` bottle color anims (each identical to
WW `Bin.arc/vbina_x.btk`) and `vboxn.btk == VboxA.arc/vboxa.btk`. **Every model and
every animation in itemmdl.arc is a bit-perfect Wind Waker file (33/33 members).**

**Retail TP code never loads it** — zero `resLoad("itemmdl")` in the decomp. WW wired
these same meshes as separate per-item arcs through its get-item flow; TP moved held
items to `ALANM`/item actors and orphaned the consolidated bundle. (This repo's WW
item work resurrects them — see wind-waker-item-work.md; the vbow/vboot are shipped
features now.) WW carries **102** `V*` view arcs total; TP kept the 21 relevant ones.

### 2.2 Inside `Always.arc` — WW's item table and the moon

TP and WW share 11 same-named Object arcs (`Always, Dalways, Ep, Gover, Kbota_00,
Kkiba_00, Mhsg, Osiblk, Table, Timer, Title`). Inside TP's `Always.arc` (loaded at
boot, resident always):

- **`item_table.bin` — byte-identical to WW's 512-byte item table.** TP has its own
  4 KB `res/ItemTable/item_table.bin`; the WW-era one rides along anyway.
- **`txa_tuki_as/bs/cs/ds.bti` — WW's four moon-phase textures, byte-identical.**
  (TP has no moon-phase mechanic; these are WW's Great Sea night sky moons.)
- `txa_ring_a_32hafe.bti` byte-identical.
- `Dalways.arc`: `boxa.brk` + `boxa.btk` (WW treasure-chest animations) byte-identical.

The other 9 shared-name arcs were fully rebuilt for TP (same name, zero identical
members) — the *naming conventions* carried over even where content didn't.

### 2.3 Full-disc SHA sweep (pass 2) — every remaining WW byte on the TP disc

A member-level hash cross-index of ALL 1,397 TP Object arcs against a 9,731-member
SHA pool of the entire WW extraction (Object + Stage). Beyond §2.1/2.2:

- **`Alink.arc` — Link's own arc carries two byte-identical WW Link textures**:
  `blur.bti` and `rock_mark.bti`, straight out of WW `Link.arc`. TP Link literally
  rolls with Toon Link's motion-blur texture.
- **`LogoUs.arc`**: `progressive_yes.bti` + `progressive_no.bti` are WW `Logo.arc`
  textures byte-identical (and `progressive_choice.bti` is a same-name/same-size
  re-render). The progressive-scan boot prompt (§5.1) was WW's prompt, reused.
- **`Obj_Bef.arc`** contains `kkiba_00.dzb` — a WW collision mesh byte-identical to
  WW `Kkiba_00.arc`'s (the shared-name arc from §2.2: its collision survived even
  though the models were rebuilt).
- Curios: TP `TKS.arc/tks_tubo_hide.bck` and `ykM3.arc/leaf_wait_b.bck` are both
  byte-identical to WW `Demo17.arc/taru_initial.bck` (a trivial hold-still track:
  TP's pot-hide anim IS WW's barrel-hide anim); a scatter of 192–224 B facial
  `.btp` tracks in TP Demo arcs match WW Demo-arc tracks (degenerate identical
  data, not meaningful reuse).
- **The Demo*.arc cutscene archives contain NO substantive WW content** — their
  treasure is TP's own beta material (trailer-era Link etc., see the Demo Leftover
  Viewer). TP's real WW payload is exactly: itemmdl (all 33 members), Always/
  Dalways (§2.2), the two Alink textures, the LogoUs prompt textures, and one
  collision mesh.

### 2.4 The animation vocabulary — TP Link is re-recorded WW Link

`AlAnm.arc` (TP Link's shared animation arc) contains **51 animations whose NAMES
are WW `LkAnm.arc` animation names** — the sword cuts (`cuta/cutl/cutr/cutrel/
cutrer/cutea/cuteb`), damage set (`dam/damf/damfb/damff/damfl/damfr` + `*up`
variants), rolls (`rollf/mrolll/mrollr/rollfmis`), grabs (`grabng/grabp/grabthrow/
grabup`), jumps (`vjmp/vjmpcha/vjmpchb/vjmpcl`), bottle moves (`binget/binswings/
binswingu`), boomerang (`boomcatch/boomthrow`), `slip/slidef/slideb/lie/take/
pickup/turnback/walks/dashs/crouch/hangl/hangup...` — every byte different (new
skeleton, new mocap) but the *entire move-set inventory* is WW's list, re-recorded.
The same pattern holds for enemies: `E_bs/E_kg/E_kk/E_mk/E_nz` keep WW's `Bs/Kg/
Kk/Mk/Nz` per-animation names (`*_wait01`, `*_walk`), and TP's dragon boss arc
`B_dr` carries `dr.bmd` — the same model name as WW's dragon (Valoo, `Dr2.arc`).
TP's bestiary wasn't just prefix-renamed from WW's (§4) — it was re-animated
against WW's shot lists.

---

## 3. Cut and stripped actors on the retail TP disc

### 3.1 The stub-rel roster (stripped at mastering, roster intact)

TP ships 623 `.rel` actor modules + ~135 actors statically linked in the DOL (all 758
symbol maps are present in `map\Final\Release\` — the map set is COMPLETE, including
maps for every stripped stub below; nothing is missing at the map layer). The
sub-500-byte rels are gutted shells whose actors were cut late:

| Stub rel | Size | What it was |
|---|---|---|
| **`d_a_npc_mk.rel`** | 308 B | **Makar** (WW's Korok sage — WW's real actor is 12.8 KB) |
| **`d_a_npc_p2.rel`** | 308 B | **Medli** (WW's Rito sage — WW's is 19.5 KB) |
| `d_a_npc_kdk.rel` | 276 B | unknown cut NPC |
| `d_a_npc_henna0.rel` | 276 B | cut NPC (relative of shipping Hena?) |
| `d_a_obj_testcube.rel` | 274 B | dev test primitive — its stage names `unitC00/unitS00/unitCy0` are STILL in the placeable-object table (`l_objectName`) |
| `d_a_obj_damCps.rel` | 273 B | cut object — but **222 lines of real code survive in the decomp** |
| `d_a_tag_arena.rel` | 380 B | ─┐ |
| `d_a_tag_escape.rel` | 380 B |  ├─ a cut tag family: arena / escape / instruction / |
| `d_a_tag_instruction.rel` | 380 B |  │  schedule — smells like a cut minigame or tutorial |
| `d_a_tag_schedule.rel` | 381 B | ─┘  system (decomp keeps 42–51-line shells for each) |
| `d_a_tag_setBall.rel` | 424 B | cut tag |
| `d_a_tag_assistance.rel` | 447 B | cut tag |
| `d_a_tag_gra.rel` | 452 B | cut tag |

Makar and Medli in TP's actor roster is the single strongest character-level trace of
the "Wind Waker 2" phase: the two WW sages were far enough into TP's plan to be in
the module list, then stripped to stubs for retail.

### 3.2 `d_a_mant` — not a manta: the cape that crossed two games

*(Corrected: research pass 1 misread "mant" as manta ray; the decomp settles it.)*
`mant` = **mantle**. In WW, `d_a_mant` is annotated as Phantom Ganon's / the
Darknut's **cape** object; in TP it is a ~830-line cloth-simulation actor driving a
169-vertex cloth grid over Ganondorf's mantle display-list data (`l_Egnd_mant*`),
**spawned as a child by the Ganondorf boss** (`d_a_b_gnd` →
`fopAcM_createChild(fpcNm_MANT_e, …)`), including horseback mode. So it's not cut
content at all — it's the clearest example of a WW *gameplay* actor surviving by
exact module name into TP's shipped final boss: WW's cape physics became Ganondorf's
cape. Also module-name-identical across both games: `d_a_movie_player.rel` (THP
player) and `d_a_title.rel` (title-screen lineage).

### 3.3 `E_ms` — the mislabeled cut enemy is TP-ORIGINAL

Community lore calls TP's cut `E_ms` a "Moblin." The data says otherwise:

- **Skeleton** (`E_ms.arc` → `ms.bmd`): quadruped — `F_L/R_leg_1-3`, `B_L/R_leg_2-4`,
  `backbone_1/2`, `head`, `mouth`, `waist`, **`tail_1-4`**.
- **Animations** (14): `ms_swim`, `ms_bite_die`, `ms_gnaw`, `ms_die_float`,
  `ms_jump_start/middle/end`, `ms_run/walk/wait/damage/die/find`, `ms_kyorokyoro`
  ("looking around").
- **Reserved audio** — the shipped sound tables allocate NINE ids:
  `Z2SE_EN_MS_{BITE, FALLWATER, FN_WATER, FOOTNOTE, SWIM, V_DAMAGE, V_DEATH, V_FIND,
  V_NAKU}` — wet footsteps, swimming, a cry. Audio ids surviving into the retail
  `.baa` means it was cut LATE.
- **Code**: `d_a_e_ms.cpp` is a substantial ~1,100-line actor (HIO tuning class
  included) still resolvable from the placeable table (`OBJNAME("E_ms")`).
- **No WW ancestor**: WW has no `d_a_ms` enemy; its Ms-named arcs (Msdan, Mshokki,
  Mspot, Msw) are props. WW's actual Moblin (`d_a_mo2`/`Mo2.arc`) and Bokoblin
  (`d_a_bk`/`Bk.arc`) never shipped on the TP disc at all.

Verdict — CORRECTED (2026-07-17, DZR placement sweep): **E_ms is not cut at all.**
A layer-blind byte-scan of all 79 stages finds **72 placements across 8 stages**
(Snowpeak Ruins D_MN10, sewer-class R_SP107/116, and heavy D_SB cave floors — 47 in
D_SB01 alone). Combined with the skeleton, swim/bite/gnaw anims, wet-footstep audio,
and biome pattern, E_ms is with high confidence the **shipped RAT family** ("ms" =
mouse). Both community layers of lore were wrong: not a Moblin, and not cut — nobody
documents it as unused because it was never unused. (The skeleton/audio evidence above
stands; only the "cut" classification is retired. tools/demo_cut_content/
dzr_placements.py reproduces the sweep.)

### 3.4 `E_dt` — CORRECTED: the shipped Deku Toad midboss

*(Corrected 2026-07-17 — pass-1 called this "the cut second frog"; the placement
sweep falsified that.)* `E_dt` = **Deku Toad** ("DT"), the shipped Lakebed midboss:
exactly **one retail placement** (a boss-arena-class room), a 2,592-line actor, and
`DT_OP_DEMO*` anims = its midboss ENTRANCE cutscene. `E_ot` ("ot" = *otama*,
tadpole) is its spawned minion, not "the shipped toad." The Wii-rev0 `E_base.arc`
leak's `DT_EGG` (§5.2) is therefore Deku Toad encounter material (the egg-drop
attack), a dev-template leftover of a SHIPPED fight — not cut-content evidence.

### 3.5 `Sample.arc` — a dev sample archive on every retail disc

Contents per its index: test `TXT_FILE0/1/2`, `BMD_FILE1/2`, a JStudio
`DAT_EVENT_LIST`, `DZB_FILE/FILE2` collision, and a BCK literally named `TEST`. The
engine's "hello world" archive, never removed.

**2026-07-17 identification (in-game visual + rig census): `file2.bmd` (59,648 B) is an
EARLY EPONA PROTOTYPE** — joint list identical name-for-name to shipped `Horse.arc/hs.bmd`
(backbone/leg chains, `neck1/2`, `kura1` saddle, `belt_L/R` + `abumi_L/R` stirrups, tail),
at under half the shipped model's size (131,264 B) and rendering untextured. Given WW2's
founding pitch was horseback riding (Takizawa), Sample.arc is plausibly the original
horseback-prototype sample and file2.bmd the oldest TP-era asset on the disc. **This
FALSIFIES TCRF's claim (§6.1) that Sample.arc contains the GDC 2005 early Shadow Beast**
— the data-level beta Shadow Beast is `E_s1.arc` (gen-2 prototype, never loaded; §3.6-adjacent),
visually confirmed in-game as the trailer-style beast. `file1.bmd` (2,784 B, `world_root`)
is a trivial test primitive.

**2026-07-18 — the horse LINEAGE, structured TEX1/JNT1 parse (verifies the community
"pixel-for-pixel" claim + the Ganon-steed question):**

- **`file2.bmd` doesn't just share Epona's rig — it names Epona's textures.** Its 12
  texture slots are literally `hs_body`/`hs_eye`(.1-.3)/`hs_hair`/`hs_tail` — the shipped
  Horse arc's own `hs_` prefix — in the SAME formats (CMPR/C8) and SAME dimensions
  (256×256 body, 4× 32×32 eyes, 128×128 hair, 64×128 tail) as shipped `hs.bmd`'s 12
  slots. Only the payloads differ (some intact → the correctly-textured patches seen
  in-game; some rotted/placeholder → the garbage patches). **The community claim that
  Epona's textures fit the prototype "pixel for pixel" is data-verified at the interface
  level: identical slot names, counts, formats, and dimensions.** file2.bmd isn't a
  different horse that resembles Epona — it IS an earlier revision of the same `hs`
  asset.
- **Ganondorf's steed (`B_hg.arc/hg.bmd`, loaded by `d_a_b_gnd` alongside B_gnd) IS a
  re-dressed member of the same lineage: the IDENTICAL 38-joint skeleton, name-for-name
  — including Epona's `kura1` saddle and `belt/abumi` stirrup joints, retained under the
  armor.** Its textures are its own (`hg_yoroi` 鎧 = armor, plus body/hair/tail at
  Epona's exact slot dimensions), and its animation set reuses Epona's core movement
  stems (`jump_start/middle/end`, `run_dash`, `stand`, renamed `hg_`) plus boss-only
  additions (`down`, `standturnl/r`, `wait`, `walk`, `stop`). One rig serves all three
  horses on the disc — prototype, Epona, and the boss steed — meaning their animations
  are cross-compatible.
- **`EN_HG` audio family attribution REFINED (corrects §3.9's correction): HG = "Horse
  Ganon" — the steed itself**, not beast-Ganon footfalls: the ids (`V_BREATH`,
  `FOOTNOTE_L/R`, `SLIP`, `LAND`, `V_STAND`, `V_STANDTURN`, `D28_STAND*`) align
  one-for-one with `hg_stand`/`hg_standturnl/r` anims and the Demo28 rampage cutscene.

### 3.6 What was NOT found

- **No test stages on retail TP GC USA**: 79 stages, systematic names, nothing like
  WW's 23+ dev rooms (`TEST`, `E3ROOP` — WW's E3 demo loop! — `K_Test2-9`, `ITest6x`,
  `VrTest`, `ENDumi`…). TP was mastered clean at the stage layer; WW never was.
  **AMENDED 2026-07-18: the stage DATA was mastered clean, but the dev-stage NETWORK
  survives in names — see §3.8.**
- **No second orc actor**: the 2004-trailer "beta Moblin" (tall, axe) matches nothing
  on retail; `E_OC/E_OC2` are one bokoblin proc with a mesh variant (earlier research,
  wind-waker-item-work.md §Cut enemies). Cut before GZ2E01.
- **The 135 map-only modules are NOT cut content** — they're the statically-linked
  DOL actors (verified: every one exists in the decomp's DOL source). TP's real
  DOL/REL split, documented to prevent future misreads.

### 3.7 Ghost placements + dormant WW mechanics (2026-07-17 sweeps)

- **`_partic` — the only true ghost placement on the disc.** Layer-aware DZR chunk
  parse of all 645 room/stage members (`tools/demo_cut_content/dzr_orphans.py`) diffed
  every placed actor name against `l_objectName` (916 entries): exactly one name is
  placed but unspawnable — `_partic`, ×3 (F_SP122/R08 layers d+e, F_SP123/R13 base),
  `params=FFFFFFFF`, neighbors `Coach`/`GWolf` ⇒ authored along the wagon-escort route,
  actor removed from the roster before ship. No matching symbol in TP or WW. Every
  other placement name on the disc resolves — TP's rooms were mastered that clean.
- **`getBokoFlamePos` — a Wind Waker Boko-stick mechanic dormant in shipped TP code.**
  The WW player base-class virtual (same mangled symbol in both games' maps) survives
  in TP (`d_a_player.h` stub, always FALSE; TP Link never overrides it) and TP ships a
  live caller: `d_a_obj_brg` (rope bridge, which also sways via WW's
  `dKyw_get_AllWind_vec` wind system). The dormant mechanic is complete: carried flame
  within 50 units of a plank's rope anchor → 30-frame burn timer + fire emitter →
  per-side rope bit cleared → plank dangles (one side) or drops collision (both).
  Retail placements exist (Forest Temple D_MN05/R01, Faron F_SP104). In WW Link
  carries flaming Boko sticks — Bokoblin weapon culture; TP kept the interface but cut
  the carryable. TP Link's lantern already tracks `mKandelaarFlamePos`, so a one-method
  override could revive the mechanic (Dusklight restoration candidate).

### 3.8 The cut dev-stage network (2026-07-18 — names survive, data does not)

Three independent shipped sources still name **~14 cut dev/test stages**:

1. **Twilight-capable stage table** (`d_kankyo_data.cpp` `l_darkworld_tbl`, in the DOL):
   `R_SP30` (Unknown Room), `T_SP05` (E3 2005 Warp Test, old), `T_SP05A` (Ito Warp
   Test), `T_WARP2`, `T_IMAI` (**Imai Shape Check — Level 2**), `T_IMAI0` (**Level 3**),
   `T_TOMI` ×3 (Tominaga Test), `T_SASA1` (Sasaki Test), `F_SP105` (**L1 Forest
   Field**), `F_SP05` (**Statue Forest**), `TEST11` (Local Map C). All flagged
   `KY_DARKLV_TEST`-era — these stages could hold TWILIGHT state.
2. **Live special-case code**: wolf-Link checks `checkStageName("R_SP30")` behind event
   bit M_010 (`d_a_alink_wolf.inc:7893`); the rain engine scripts R_SP30 room 4
   repeatedly (`d_kankyo_rain.cpp`); the env engine checks `T_SASA0` (`d_kankyo.cpp:8335`)
   — a 15th name that exists only in code.
3. **Shipped room exit tables (SCLS)** — new sweep, variable-length entry parse (name
   string + 5 bytes; NOT fixed-stride): retail rooms still hold exits pointing at
   `T_SP05` (**from Forest Temple D_MN05/R09**), `T_DEMO` (from R_SP01/R04),
   `T_OBJ01` (from Hyrule Field F_SP121/R06), `T_TOMI` (from Kakariko F_SP109/R00).
   `T_DEMO`/`T_OBJ01` appear in NO binary — placement-data-only survivals, likely
   previously unrecorded. (`OPENING` ×8 rooms = legit code-handled sentinel
   (`d_stage.cpp:44`), not a cut stage. An `NPC_GND` "destination" is junk-row tier —
   that token is the Ganondorf NPC actor name.)

**Room-number gaps** corroborate heavy late cuts inside shipped stages: Forest Temple
D_MN05 missing 10 room numbers (R06/08/13-18/20-21), F_SP122 missing 7 (R09-15),
D_MN04 missing 4, 13 stages with holes total.

**Beta-Moblin implication (2004 trailer axe-wielder, torch-lit dungeon corridor):** the
best-named candidate home for the "beta fire temple" is **`T_IMAI` (Shape Check
Level 2)** — Level 2 = the fire dungeon slot — with `T_SP05`/`F_SP105` covering the E3
demo forest content. The stage names, exit links, and env/wolf/rain scripting all
shipped; the stage archives themselves (and any enemy they contained) did not. If the
beta Moblin existed as data, this network is where it lived — and it was purged with
the stages, consistent with the Object-arc fingerprint NULL.

### 3.8b The WW crew on TP — everything the retail disc knows (2026-07-18)

**IDENTITY CORRECTION (same day, via in-game visual + WWHD naming + Cb.arc verification):
the stubs are NOT the sage pair.** WW's `Mk` = **Ivan of the Killer Bees** (Windfall kid;
`mk_jida*` = his foot-stamp tantrum) and WW's `P2` = **the small-pirate shared body with
THREE heads** (`p2head01/02/03`, book/telescope/daggers — Tetra's small crew). The real
sages are `Cb.arc` (Makar: `cb_face.bdl` leaf face, `cb_cello.bdl`, nut-sowing ceremony;
actor `d_a_npc_cb1`) and `Md.arc` (Medli, `d_a_npc_md`) — **TP has NO cb/md stubs; the
sages were never in TP's roster.** What TP's stub roster actually is: **WW-branch
RESIDUE** — a Windfall kid and a pirate body surviving as gutted actors in dolzel2's tree
all the way to mastering. Weaker romance, stronger branch evidence. Table below kept with
corrected identities:

| Evidence | Ivan (`npc_mk`, ex-"Makar") | Small pirates (`npc_p2`, ex-"Medli") |
|---|---|---|
| Spawn table (`l_objectName`) | `Mk` | **`P2a`, `P2b`, `P2c` — THREE placement subtypes (args 0x00-0x02): almost certainly the three pirate HEAD variants**, mirroring `p2head01/02/03` |
| Debug spawn roster (`f_pc_debug_sv.cpp`) | `NPC_MK` ✓ | `NPC_P2` ✓ |
| REL on disc | 308-byte safe stub | 308-byte safe stub |
| Model/anims on any TP master | **none** (SHA sweep) | **none** |
| Retail placements | none (one byte-scan hit = false positive) | none |
| Message-bank text (EN+JP, decompressed) | none | none |
| Symbol map | 5 × 8-byte return-0 shells (`Create/Delete/Execute/Draw/IsDelete`) — no behavior vocabulary survives | same |

The stub anatomy documents Nintendo's cut method: gut the REL to safe no-op entry
points, keep the profile and the spawn-table names — ghost placements would silently
fail rather than crash. (Dusklight phase-1 corollary: our port's stubs should be
hardened to match retail's safety.)

**Roster correction: `npc_kdk` is NOT WW crew** (no WW counterpart exists) — a
TP-original cut NPC, even barer than the sages: its rel is **276 bytes with NO methods
at all** (map = executor prologue + `g_profile_NPC_KDK` only; the sages at least kept
8-byte named stubs) — the NULL-method profile is exactly the TestCube crash class.
But it has the distinction the sages lack: **one real retail ghost placement — and the
location is remarkable: `R_SP107/R03` is ZELDA'S TOWER-TOP room** (base layer: `zdoor`
— the door to Zelda's chamber — a save point, two carryables). **Kdk stands alone on
layer 0 at (19676, 4500, 2633), 660 units from Zelda's door at the same floor height —
a cut NPC in the first-Zelda-meeting scene.** Params `000FAFFF` (undecodable — no code
survives). Retail survival hypothesis: that layer combination never activates in the
shipped story flow (the sewers sequence runs on other layers), so the methodless
profile never loads. Identity: **USER HYPOTHESIS (2026-07-18, promoted to evidence-alias candidate): Kdk =
the UNSEEN JAILER** — in retail, the cutscene after Midna and wolf Link first visit
Zelda confirms a guard/jailer approaching (light + footsteps, always off-camera;
Midna spirits Link away before he renders). That cutscene (Demo07 — robed-Zelda
`zelr*` models + flashback soldiers) plays in THIS room; Kdk stands at its door on a
layer the shipped flow may never activate. Location-exact, scene-exact — but
circumstantial until runtime evidence. **Phase-1 experiment (with safe-stub
hardening's create-attempt logging): replay the intro sequence to the Zelda visit —
if the game's layer state ever flips R03 to layer 0 during/after the scene, the
engine will literally attempt to spawn Kdk at the jailer's arrival beat, and the log
will show it. A positive = confirmation; a negative = the placement was already
orphaned by the layer flow when the scene was staged off-camera.**

**EXPERIMENT RESULTS (2026-07-18, both halves run):** (a) full intro replay — R03
loads ONLY layers 14/8/10; layer 0 never activates; no create-attempt ⇒ **Kdk is a
layer-orphaned ghost; retail never tries.** (b) forced layer-0 via debug warp
override — `[StubWatch] create-attempt proc=NPC_KDK stage=R_SP107 room=3 layer=0
params=000fafff pos=(19676,4500,2633)` — **Nintendo's authored Kdk placement executed
for the first time since mastering** (safe-declined; no payload exists). Mechanism
proven; identity (unseen-jailer hypothesis) remains circumstantial — the placement +
scene fit is the strongest evidence obtainable from the disc.

WW-side note (for the separate restoration mod, outside the base game): all relevant
arcs exist on the WW disc — `Cb.arc` (Makar), `Md.arc` (Medli), plus the residue pair
`Mk.arc` (Ivan) / `P2.arc` (pirates).

**THE COMPLETE WW-CHARACTER-IN-TP FORMULA (proven + MEASURED 2026-07-18 — Ivan
rendered 1:1 to his GC colors in Faron: shirt sampled `#609cd8` vs authored/WW target
`#6090cc`; R exact, G/B within 6-8% = TP scene-light contribution):**
1. **Offline arc adaptation** (`tools/ww_crew_restoration_skeleton/adapt_bdl_arcs.py`,
   Yaz0-aware, in-place RARC patch), FOUR passes per .bdl:
   - strip `MDL3` + retag `J3D2bdl4`→`J3D2bmd3` (the port's BDL loader is
     never-exercised decomp code and crashes);
   - `normalize_litmask`: clamp enabled channel litMask→0x01 (WW lights slots 0+1;
     TP feeds slot 0 — proven by the WW-boots overlay);
   - **`normalize_tevregs` (the ×0.50 killer): promote TEV register color
     (128,128,128) → white.** WW's runtime overwrites C-regs per frame with live
     light/shadow colors; the file's 50%-gray placeholder otherwise multiplies the
     whole character to exactly half brightness, immune to ambient;
   - keep `ZAtoon`/`ZBtoonEX` AUTHENTIC (hard 0/15 two-tone = WW's cel edge;
     whitening is a diagnostic flag `--flat-toon` only).
2. **Runtime draw = the WW boots/leaf cel branch, NOT MAJI** (`d_ww_itemmdl_pc.cpp`):
   `settingTevStruct(0)` → fixed ambient into tevStr → `applyBowMaterialAmbientOnly`
   → normal `entryDL`. Neutral gray ambient `0x5a` + live WREG_F(30/31/32) trim.
3. **Socket delivery**: Plan-R receiver (stub-socket adapter + mod-folder arc mount +
   manifest) — TP ships machinery only; characters live in the mod folder.
Diagnostic ladder for future cases: black-with-glowing-eyes = lit channels starved
(unlit materials render); "cleaner but still black" = toon ramp sampling dark end;
"uniform exactly-half, dial-immune" = the 128-gray TEV register placeholder.
**Forward implication (user): per-frame C-reg writes from tevStr (light+shadow
colors) = the road to FULL WW lighting (day/night-tracked cel) inside TP.**

### 3.9 The path to the Moblin — final synthesis (2026-07-18, all lanes exhausted)

Ten independent evidence lanes, run to completion:

| # | Lane | Result |
|---|------|--------|
| 1 | WW Mo2 fingerprint vs all 1,397 Object arcs | NULL |
| 2 | Symbol-map contents (all .map files) | NULL (only `getBokoFlamePos`, §3.7) |
| 3 | Orphan placement names (all 645 rooms) | `_partic` only — no enemy ghosts |
| 4 | Cut dev-stage network | names/exits/scripting survive; data purged (§3.8) |
| 5 | Cross-master Object roster diff (9 discs) | only the known Wii-rev0 `E_base` leak |
| 6 | Orphan-arc sweep (arcs no code names) | **`E_wpa` = unused `ef_warphole` warp effect** — every E_/B_ arc else accounted |
| 7 | E_OC2 identity | **SHIPS — 39/126 retail E_oc placements select OC2** (variant = placement rot.x byte 1 ≠ 0/FF; ToT R11/13/14, Cave of Ordeals D_SB01, Hyrule Field). Same 39,520 B body/joints as oc.bmd (retexture) + own 2× larger weapon `oc2_wepon.bmd`. HIO debug name: オーク "Orc". Folk label "Moblin mesh" dead. |
| 8 | Weapon-model sweep (all member names) | every axe accounted: `E_mf/mf_axe` = **Dynalfos** (shipped, 11 placements), `E_rdb/rb_ono` (斧) = **King Bulblin** (shipped), oc2 cleaver ships. No orphaned axe. |
| 9 | Stage-arc embedded models | door hardware only |
| 10 | Orphaned enemy-audio audit | **5 cut-enemy audio families, no moblin** (below) |

**CORRECTION (2026-07-18, same day — the "five cut-enemy audio families" claim is
RETRACTED.** The family-token→actor-filename matching method was flawed: audio family
names need not match actor names. Code-usage grep shows ALL FIVE belong to shipped
actors: **`EN_NS` = `d_a_e_s1` (the retail Shadow Beast — Cursor caught this first)**,
`EN_GF` = `d_a_e_gb` (**Deku Like** — eat/burst/open/close/bound + `demo_dropkey`),
`EN_ZZ` = `d_a_b_zant_mobile` (Zant's snorting head phase), `EN_GBA` = `d_a_e_gm`
(Gohma young; arc holds `goma_egg.bmd` — Armogohma's lay/hatch mechanic), `EN_HG` =
`d_a_b_gnd` (beast Ganon footfalls/stand). **Conclusion: ZERO orphaned enemy audio
families exist** — the audio lane is fully accounted, which makes the Moblin timing
argument STRONGER: no scar of any kind at the audio layer. Residue: individual unused
ids only (e.g. `EN_NS_V_DEMO04_HIPPARU` — the gen-2 "pull", unused by retail logic).)

**STB cutscene-script sweep + ActorDat (2026-07-18, final string sources):** all ~130
JStudio `.stb` scripts token-swept — no cut-stage refs, no unknown enemy tokens
(Demo90/98 hold nothing anomalous). BUT `res/ActorDat/ActorDat.bin` (1,984 B) —
outside every prior sweep — is **WIND WAKER's enemy item-drop database, BYTE-IDENTICAL
to WW's own copy, shipped on retail TP and read by nothing**: sections
ACFN/ACNA/TACDS/LARG, `N_ITEM0-15`/`percent` drop slots, and a WW enemy roster —
`nezumi, keeth, magtail, bbaba, p_hat, c_red/green/blue/black/kiiro` (chuchus),
`kuro_s, Stal, gmos, bable, amos/amos2, Fmaster, Rdead1/2, wiz_r/s/m/o, sea_hat,
Oship` — **and `mo2`: the Moblin, by name.** This falsifies §3.9-lane-1's "zero
`mo2` occurrences" as stated (that sweep covered Object arcs only) and adds ~2KB to
the §1 byte-identical WW payload. **The only Moblin trace on any TP master is this
one string — a fossil of WW's drop table, not TP-Moblin content.**

**Verdict — the Moblin's path (amended):** the 2004-trailer axe-wielder was a
vertical-slice asset whose home was the dev-stage network (§3.8; likeliest the
Level-2 shape-check / demo-dungeon tree). It was purged from the asset tree before
ANY of the nine shipped masters, before audio mastering froze (zero audio scar), and
survives on disc only as the string `mo2` inside WW's fossilized drop database.
Retail descendants are design-lineage only: King Bulblin (armor + `rb_ono` axe) and
the shipped OC2 heavy-cleaver orc. No disc-side restoration path; bringing the beta
Moblin back is mod-import work (WW Mo2 port or recreation), same class as
Makar/Medli. The hunt's yield: the dev-stage network, `E_wpa`, the OC2 correction,
the `mo2` fossil, and a fully-accounted audio map.

---

## 4. Naming archaeology — WW → TP conventions

19 actor tokens survive with prefix formalization: WW's loose names became TP's
`e_`(enemy) / `b_`(boss) / `npc_` / `obj_` / `tag_` scheme — e.g. WW `d_a_dk` → TP
`d_a_e_dk`, WW `d_a_ph` (Phantom Ganon) → TP `d_a_e_ph`, WW `d_a_pz`→`e_pz`,
`st`→`e_st`, `nz`→`e_nz`, plus `fan/kita/roten/firewall/waterfall` object/tag moves.
Particle files keep WW's `common.jpc` + `PsceneNNN.jpc` numbering convention with
fully rebuilt contents. The demo disc (DZDE01) differs from retail by only three
boot-flow arcs (adds `NNGC`, lacks `WarnJp/WranPal/WranUs`) — a late kiosk build.

Audio is the clean break: WW's JAudio (`JaiInit.aaf`, 67 `.aw`, `.afc` streams) vs
TP's Z2-era BAA (`Z2Sound.baa`, 232 `.aw`, `.ast` streams) — zero shared files, a
different sound-engine generation despite the shared lineage.

---

## 5. Cross-disc archaeology (demo / GC / Wii / regional / Shield)

Content-level diff of the resource indexes across all nine disc variants.

### 5.1 The "GC demo" is a post-Wii-launch build — and other reframes

- **DZDE01 (the GC kiosk demo) was mastered from the WII asset tree, after the Wii
  launch revision**: it carries the full Wii-only dirs (`LayoutRevo` with all 18
  Wii-Remote layouts, `WiiBannerIcon`, FR/SP text sets) and its UI files match Wii
  **rev2**, not rev0. It even ships `NNGC.arc` — the Wii pointer-cursor arc — on a
  GameCube disc. (Inside NNGC: the pointer screen is literally named
  `ZELDA_POINTING_CURSOR_NAVI` — the Wii cursor was internally "Navi".)
- **Zero gameplay-content churn across demo → GC retail → Wii**: all ~1,390 shared
  Object indexes are byte-identical on DZDE01, GZ2E01, and RZDE01_00. Every
  late-cycle change lived in boot/UI arcs only. Regionals likewise: **no JP/PAL
  exclusive content arcs exist** — only logo/title arcs differ.
- **PAL boot flow rework**: the demo/Wii tree had a progressive-scan Yes/No prompt
  (`PROGRESSIVE_CHOICE/YES/NO`); GC PAL mastering replaced it with the 50/60 Hz
  selector (`50_60_CHOICE`, `50_SET`, `60_SET`). Wii rev0 also shipped a
  `BPK_ZELDA_PRESS_START` palette anim in Title2D that rev2 deleted.
- **Cross-platform pollution both directions**: GC discs carry `LogoDuWii.arc`
  (a German Wii logo); Wii US rev0 carries PAL's `TitlePal.arc` (scrubbed in rev2;
  JP/PAL Wii discs equal rev2's tree — US rev0 is the lone outlier launch build).

### 5.2 `E_base.arc` — a dev enemy-template arc leaked on Wii rev0 ONLY

Wii USA rev0 ships (and rev2 scrubs) `E_base.arc`, absent from every other disc:
**`BMD_DT_EGG`** (with its own joint set), `BMD_HZ_HOLE`, `BMD_RD_CLUB`,
`BTI_A_BKWIRE`. **`DT_EGG` is an egg model for the cut `E_dt` frog (§3.4)** —
evidence the E_dt encounter had an egg/spawning mechanic (paralleling the shipped
E_ot toad's tadpole-spawning), prototyped in a shared "enemy base" template arc.
The clearest accidental dev leftover on any retail TP disc.

### 5.3 E3 and Shield traces

- **`clctresE3.arc`** — an "E3" build variant of the collection-screen layout —
  ships on EVERY disc from GC retail through the Shield port (which even patched
  its button glyphs). An E3-demo leftover never pruned across a decade.
- The Shield port: asset base follows Wii-rev2 lineage, adds Chinese
  text/anti-addiction notice + NVIDIA logo, and retrofits ~14 layout arcs with
  additive 32px gamepad-glyph textures (`DA32/DB32/...`). Shield vs ShieldD trees
  are 100% identical.

## 6. Community record — corroboration, expansion, and where we're ahead

Web lane (TCRF via search extraction — tcrf.net blocks this network's fetches; its
anti-scraper page even carries prompt-injection text, ignored — plus zeldaret decomps,
Unseen64, interviews). All claims sourced in the research transcript.

### 6.1 Corroborated by the public record

- **The engine story**: TCRF — TP "runs on a heavily modified version of the engine
  that was used to develop The Wind Waker… multiple assets carried over."
  zeldaret/tww README: "a significant amount of engine code is shared." The TP
  decomp's core headers are literally `include/d/dolzel.h` / `dolzel_rel.h`.
- **dolzel in-game**: TCRF's Unused Text documents leftover dev notes referencing
  the `dolzel` build tree by name ("old data remaining in dolzel… discussed with
  Iwawaki-san, Itou-san") — independent corroboration of §1.
- **itemmdl**: TCRF Unused Models counts the same "21 items from The Wind Waker."
- **Sample.arc**: TCRF adds two things we didn't see from the index alone — its
  `dir5/a` model is in the format of WW's unused early Hyrule Castle (`A_R00`), and
  Sample.arc holds the **early Shadow Beast from the GDC 2005 trailer** (fights Link
  in a forest; shares movements with the final Shadow Beast). Ties directly to the
  `E_s1` prototype lineage (§wind-waker-item-work).
- **Wind Waker 2 history**: GDC 2004 — Aonuma announces the next GC Zelda as
  *The Wind Waker 2*. Takizawa (Art & Artifacts): WW2 "would have taken place in a
  more land-based setting… Link gallop[ing] across the land" — Toon Link's
  proportions broke horseback, hence realistic Link (Nakano). Aonuma GDC 2007: NoA's
  cel-shade market pushback; Miyamoto's "do it right" + "120% Zelda"; the wolf came
  from Aonuma's dream of being a caged wolf; Miyamoto's "someone should ride the
  wolf" (four-legged animal from behind looked boring) → that rider became Midna.
  The sea content died with the WW2 concept — no evidence TP itself ever cut sailing.

### 6.2 Where this project's disc findings are AHEAD of the public record

Targeted searches found **no public documentation** of: the E_ms skeleton/audio
reclassification (still called a "Moblin" in community lore), E_dt/E_ot's frog
lineage or the `DT_EGG` in Wii-rev0's `E_base.arc`, the Makar/Medli 308-byte stub
rels, the `tag_arena/escape/instruction/schedule` cut family, `npc_kdk`,
`npc_henna0`, `obj_damCps`, or the byte-identical WW payload inventory (§2). TCRF's
TP coverage is asset-centric; the stripped-REL layer is unexplored there. The decomp
corroborates our reads (npc_mk is a pure `return true` stub with actor size 0x1;
d_a_e_ms is a full enemy with an HIO live-tuning class and a 450-unit aggro radius).

### 6.3 TCRF additions worth folding in (TP)

- **Unused items**: the **Surf Leaf** (item ID 0xF7, placeholder Hylian Shield icon)
  — TCRF: "probably the leaf used for snowboarding from Snowpeak"; the surfing
  mechanic's internal Japanese name is "**Shield Surfing**". A leaf ITEM concept
  existed in TP's item table — directly relevant to this project's Deku-Leaf-glide
  research (deku-leaf-glide-research.md). Also: a Light Arrow mislocalized as "Fire
  Arrow" (non-functional), Magic Potion, Green/Black Chu Jelly drops, fishing-rod
  bait variants, unused lantern-oil variants.
- **Unused models beyond ours**: a Bomb Flower "in the process of being reskinned
  from The Wind Waker before being scrapped" (a WW asset caught mid-conversion!);
  a WW-style fish (early Hylian Loach); the "Goron Golem" (Goron amalgam — possible
  early Dangoro/Fyrus); community-known Armos Titan + Twilight Assassin.
- **Unused rooms**: deleted maps still listed in `menu1.dat` (map-select remnant —
  the Wii kiosk demo retains the full three-level warp menu; the Shield port dropped
  the file); `F_SP118` room 02 = King Bulblin battle test area; Palace of Twilight
  ships extra Throne Room copies, one with **a unique Zant actor + cutscene**; a
  surviving early **Darkhammer** room (large hexagonal arena vs the final corridor).
- **E3 residue on retail**: `E3_2006.arc` (Wii US: images from both E3 demos) and
  the unused `Stageselect_e3_2006` music (the early/short Hyrule Field theme) —
  companions to our `clctresE3.arc` finding (§5.3).
- **Prerelease specifics**: the E3 2004 trailer was assembled by a US team on a
  barely-started game (camera tool + staged footage); the axe "beta Moblin" is
  unexplained and possibly became King Bulblin. E3 2005 demo: **Armogohma
  originally chased Link down a dark tunnel at the end of the Temple of Time**
  (relevant to this project's Armogohma phase work), Ook's parasite was a Shadow
  Insect, twilight was black-and-white. Unseen64: beta great-forest, cemetery in
  Faron, proto-Snowpeak wolf zone, intact Bridge of Eldin as the castle approach,
  the dropped "don't transform where NPCs can see you" rule.
- **The Shield port ships a debug build**: `RframeworkD.alf` + debug versions of
  every REL, OSReport/assertions on — the reason TP is now 100% decompiled
  (decomp.dev), while WW's decomp leans on the D44J01 kiosk-demo symbol maps.

### 6.4 WW-side records that complete the picture

- **E3ROOP** = "E3LOOP", an early Boating Course islands room — WW's own E3 artifact.
- WW's unused **Sage Stones for Makar and Medli** (plus a crumbling variant) carry
  an OoT/MM-style musical score — the sages' presence in TP's roster (§3.1) makes a
  neat pair: both games hold unused traces of the other's era.
- Aonuma (2013): **two WW dungeons were cut** and their parts "reused in later
  Zelda titles" — the canonical statement that WW's cut content dissolved into
  successors (a scrapped water dungeon was planned at Greatfish Isle).
- WW's Dec 2002 store-demo proto (built ~9 days before JP final): unmasked
  Jalhalla, failing Ganon's-Tower barrier, debug console on pads 3/4 — the WW
  counterpart of TP's kiosk-demo archaeology.

---

## 7. Beta Link and other leftovers inside Demo arcs (2026-07-17)

Retail `Object/Kmdl.arc` is **final** Hero’s Clothes. The 2004-trailer / early Link
face and companions were **not** deleted — they ride along inside cutscene
`Demo*.arc` packs. Community restore proof (adapted for gameplay, not a raw
extract): `D:\XXXXXXX\beta link\Kmdl.arc` (551 360 B raw RARC; SHA256
`ef036952…`). Same member names as retail `Kmdl` (`al.bmd`, `al_face`, …) but
different sizes — e.g. `al.bmd` beta **313 440** vs retail **140 448**. **Zero**
byte-identical SHA matches to Demo extracts ⇒ the playable pack is a remount /
retarget, which matches the finder note: “the model as-is isn’t usable, you have
to adapt it.”

### 7.1 Seed trail (verified on GZ2E01 extract)

| Claim | Disc evidence |
|---|---|
| Beta Link body | `Demo01_00.arc` → `demo01_link_cut00_bd_original_o.bmd` (380 576) + `…_high_o.bmd` (406 272) |
| Beta boar + knight same era | `demo01_wb_cut00_gp_original_o.bmd`, `demo01_ctz1_cut00_gp_talk_l.bmd` |
| First transformation faces | `Demo04_01.arc` → `…henkeiface…` (primitive/trailer) vs `…hiface…` (final) + face `btp` morphs (ShapeKey analogue) |
| Demo00 ancestry without `Demo00_01.arc` | `demo00_link_*_tmp` / `demo00_midna_*` crumbs inside later demos (e.g. `Demo04_01`, `Demo01_01`) |
| WW Moblin-style lantern (optional) | Separate from body; WW still has `Kantera` / Moblin gear under `Ex WW` |

### 7.2 Full-disc Demo mine (reproducible)

Toolkit: `tools/demo_cut_content/` — scans all **89** `Demo*.arc` under
`D:\XXXXXXX\Ex TP\files\res\Object`, tags `original` / `high` / `tmp` / `henkei` /
`hiface` / beta cast, writes CSV+MD, extracts priority models, and stages the
adapted pack as a Custom Models Layer-A overlay
(`files/res/Object/Kmdl.arc`).

```bat
python tools/demo_cut_content/scan_demo_leftovers.py --extract
python tools/demo_cut_content/stage_beta_kmdl_pack.py
```

Latest local run: **817** tagged leftover files across **60** demos; **210**
priority models extracted. See `tools/demo_cut_content/out/DEMO_LEFTOVERS.md`.

### 7.3 Pointed negatives (so we don’t chase ghosts)

- **No retail “beta forest” stage folder** — TP’s stage set is clean (~79 maps).
  “Beta geography” = unused rooms inside real stages / `menu1.dat` ghosts
  (`F_SP118` Bulblin test, PoT throne copies, early Darkhammer arena), not a
  missing GreatForest stage.
- **Trailer axe Moblin → Ook?** Unlikely on mesh grounds. Ook is monkey-family
  (`E_MB` / `E_MF` / `E_MK`); field “orc” is Bokoblin `E_OC`; WW Moblin `Mo2` is
  absent from retail TP. Closer lore guess in §6.3: King Bulblin, not Ook.

---

## 8. Why this matters for Dusklight

- **The WW item work stands on §2.1**: every itemmdl mesh is a bit-perfect WW asset —
  restoring them (vbow/vboot shipped; vhamm/viewer next) is genuine content
  archaeology, not approximation.
- **Demo-arc leftovers (§7)** are the second restore spine: beta Link / cast /
  transform faces are already on disc; gameplay needs the adapted `Kmdl` remount
  (or a fresh adapt), mountable via Custom Models without ISO surgery.
- **Cut-content restoration candidates** (revised after the placement-sweep
  corrections removed E_ms/E_dt from the cut list — both ship): `Sample.arc` curio;
  the tag_arena/escape/instruction/schedule family (shells only — names alone
  suggest a restorable minigame concept); `obj_damCps` (222 lines of real code,
  stripped rel); the TestCube (debug-gated, assets on disc).
- **The placeable table still knows several cut names** (`E_ms`, test cubes) — the
  level editor + future script-actor system can put them back in the world.
