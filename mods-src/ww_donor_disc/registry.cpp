// ============================================================================
// ww_registry — the WW actor registration vehicle (USER RULING: §968 option 1,
// "above-enum indices + 2 hooks", taken 2026-08-14).
//
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-STATUS: n/a (delivery mechanism, not donor content)
//
// WHAT THIS IS. A plugin-side registration layer so WW actors can claim
// profile indices WITHOUT any edit to the user's dusklight. It is the
// mechanism only: the registry table below is EMPTY, so today every index
// falls through to the receiver and behaviour is bit-identical to not
// loading this mod at all. Actors get added later, one table row each.
//
// WHY ABOVE-ENUM AND NOT A FREE SLOT (§968, and the reason is arithmetic not
// preference). The user's dusklight ships the vanilla profile list — 829
// entries, 13 free (§954) — and we can never change THEIR binary. Widening
// the array, reclaiming the 13, or shadowing unused TP rows all require
// recompiling the user's exe, which is fork delivery: the exact thing
// "download the plugin, point it at your ISO" replaces. Above-enum indices
// are not the cheapest option, they are the only one that ships.
//
// THE TWO HOOKS, and why exactly two. `fpcNm_MAX_NUM` bounds precisely two
// things tree-wide (§968, verified by this lane at source):
//   f_pc_profile.cpp:51   `fpcPf_Get`'s OOB guard — already exists (§706),
//                         returns NULL for out-of-range instead of wild-reading
//   c_dylink.cpp:807      `static DynamicModuleControlBase* DMC[fpcNm_MAX_NUM]`
// DMC is indexed at 7 sites; on TARGET_PC only `cDyl_IsLinked` (:889) can ever
// see a plugin index — the registration path walks a static DECLARED table a
// plugin index never enters, :867 is a bounded loop, and cDyl_Unlink /
// cDyl_LinkASync both `#if TARGET_PC` return early before touching DMC[].
//
// WHY cDyl_IsLinked ANSWERS **TRUE** — this was specified backwards first and
// the correction is load-bearing (§969, and this lane's row 202 independently).
// The function's own shape is:
//     d = DMC[i]; if (d != NULL) rt = d->isLinked(); else rt = TRUE;
// so "no dynamic-module control for this index" ALREADY MEANS "treat it as
// linked" — correct on PC where everything is statically linked. And the
// consumer chain forces the same answer from the other end:
//     fpcFCtRq_Request (f_pc_fstcreate_req.cpp:31)  `if (!fpcLd_Use(n)) return NULL;`
//     fpcLd_Use (f_pc_load.cpp:10)  = fpcLd_IsLoaded(n) && fpcLd_Load(n) == COMPLEATE
//     fpcLd_IsLoaded (:17)          = cDyl_IsLinked(n)   <- straight pass-through
//     fpcLd_Load -> cDyl_LinkASync  = COMPLEATE early on PC, never touches DMC[]
// A FALSE would short-circuit fpcLd_Use, fpcFCtRq_Request would return NULL,
// and every WW actor would SILENTLY FAIL TO SPAWN — no crash, no log. So this
// hook is not "answer specially for our range": it reproduces the function's
// own NULL branch for indices the array cannot hold. Nothing about behaviour
// changes; the only thing avoided is the wild read.
//
// WHY A BASE INDEX RATHER THAN `>= fpcNm_MAX_NUM`. The spec said "indices >=
// fpcNm_MAX_NUM", but a plugin cannot see that constant without a receiver
// header, and hardcoding today's 829 would silently mis-fire on any dusklight
// whose proc list grew. Testing against OUR OWN base is version-independent
// and strictly safer: everything below the base is the receiver's business and
// falls through untouched, whatever the host's real array size is.
// ============================================================================

// ============================================================
// THE RECEIVER'S OWN PROCESS LAYOUT, NOT A HAND-ROLLED OFFSET
// ============================================================
// This file carries a standing "NO STRUCT OFFSETS" note whose reasoning
// is right and whose conclusion no longer binds: it rejected reading
// `profname` at a HARD-CODED 0x0E ("probably safe is exactly the
// reasoning that produced the byte-reversed J3D2 magic"). Including the
// receiver's own header is the opposite move - the layout is the
// receiver's, checked by the compiler, and drifts with it.
// Needed because the capped delete receipts are sampled BY TIME and the
// grass cohort is created late: the first 10 victims all sat at pid
// 78-103 while GRASS_e spans pid 23-276, so that sample can never answer
// "was this class ever deleted".
// ============================================================
#include "f_pc/f_pc_base.h"
#include "registry.h"
#include "ww_kankyo.h"
#include "ww_kankyo_wind.h"
#include "ww_vrbox.h"
#include "ww_sky.h"
#include "ww_shore.h"
#include "ww_wave.h"
#include "ww_cam_data.h"
#include "ww_cam_select.h"
#include "ww_cam_crawl.h"

#include "d/d_camera.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "JSystem/J3DGraphAnimator/J3DModel.h"
#include "JSystem/J3DGraphAnimator/J3DModelData.h"
#include "JSystem/J3DGraphBase/J3DShape.h"
#include "JSystem/J3DGraphBase/J3DMaterial.h"
#include "JSystem/J3DGraphBase/J3DPacket.h"
// Integrator 2026-08-21: H11 build. `J3DDrawBuffer::entryNum` is read at :3189
// and :5398; J3DPacket.h only FORWARD-DECLARES the class. Vanilla defines it at
// libs/JSystem/include/JSystem/J3DGraphBase/J3DDrawBuffer.h - a missing include,
// NOT an API divergence.
#include "JSystem/J3DGraphBase/J3DDrawBuffer.h"
#include "JSystem/J3DGraphBase/J3DShapeDraw.h"
#include "JSystem/JUtility/JUTNameTab.h"
#include "JSystem/JUtility/JUTTexture.h"
#include "SSystem/SComponent/c_counter.h"
#include "SSystem/SComponent/c_math.h"

#include <mods/hook.hpp>
#include <mods/meta.hpp>
#include <mods/service.hpp>
#include <mods/svc/hook.h>
#include <mods/svc/log.h>
#include <mods/svc/ui.h>

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>

// NO `DEFINE_MOD()` HERE — `main.cpp` owns it, and this file used to be a mod
// of its own. That macro emits `mod_ctx`, `mod_meta` and the
// `mod_meta_bounds_begin/end` pair; leaving it in place put two of each in one
// binary and the link failed with four LNK2005s (Integrator, CALLS row 574).
//
// THE IMPORT_SERVICE LINES BELOW STAY, and that is a NARROWER subtraction than
// the one suggested — deliberately. Removing them would leave `s_hook` and
// `s_log` undefined in this file, which is exactly the "a wrong subtraction
// silently drops a service" hazard the report warned about. None of the four
// duplicate symbols in that link error came from them.
//
// ============================================================================
// EPOCH-2 AMENDMENT — INTEGRATOR 2026-08-17. THE PARAGRAPH ABOVE USED TO SAY
// `IMPORT_SERVICE` EXPANDS TO A **STATIC** POINTER, SO IT "CANNOT COLLIDE
// ACROSS TRANSLATION UNITS". THAT WAS TRUE AND IT IS NOW FALSE.
//
// It cited `sdk/include/mods/service.hpp:49`. That line still exists and it
// changed:
//     epoch 1: static const service_type* variable = nullptr;
//     epoch 2:        const service_type* variable = nullptr;
//
// One dropped keyword turns internal linkage into external, and `s_log` —
// imported here AND in `main.cpp` — becomes LNK2005. `docs/modding.md` now
// states the contract plainly: "A service must be imported in only one file
// (usually your mod.cpp). Other files may simply use `svc_log`."
//
// So on epoch 2 `main.cpp` owns the LogService import and this file declares
// it. `s_hook` is imported ONLY here, so it is unaffected either way.
//
// The epoch-1 arm is kept, not replaced: the fork's own in-tree build is still
// epoch 1 and must stay buildable (the fork is deliberately NOT merged). The
// discriminator is `<mods/svc/hook.hpp>`, which is one of the headers the
// 39-commit fast-forward ADDED — `MOD_ABI_VERSION` is 1u in both and cannot
// tell them apart. Route (B) deletes the epoch-1 arm.
// ============================================================================
IMPORT_SERVICE(HookService, s_hook);
#if defined(__has_include) && __has_include(<mods/svc/hook.hpp>)
extern const LogService* s_log;  // epoch 2+: imported once, in main.cpp
#else
IMPORT_SERVICE(LogService, s_log);
#endif
// The UI service, imported for ONE observation and nothing more (§1011 rider).
// NOTE FOR THE RECORD: §662 said this also needs "a `mod.json` version entry".
// It does not — checked rather than followed: no mod.json under `mods-src/`
// declares a services block, and `HookService`/`LogService` above resolve today
// with no such entry. IMPORT_SERVICE is the whole declaration.
//
// ============================================================================
// OPTIONAL, NOT REQUIRED — INTEGRATOR GATE FIX 2026-08-15, one token, and it is
// the difference between degrading and dying.
//
// This was `IMPORT_SERVICE`, which expands through IMPORT_SERVICE_VERSION to
// `SERVICE_IMPORT_REQUIRED`. `service.hpp:44` states it plainly: **"Required
// imports are guaranteed non-null (the mod fails to load otherwise)."**
//
// So on any image that does not register UiService, the WHOLE MOD fails to
// load — taking the 224-file donor serve, the actor registry and every hook
// with it, in exchange for a menu tab. That trades a working artifact for a
// cosmetic one.
//
// AND IT MADE THE AUTHOR'S OWN GUARD DEAD CODE: the `if (s_ui != nullptr)`
// check below carries the comment "the UI service may be absent on an image
// that predates it — and that absence is a reportable answer, not a crash."
// That is exactly the right intent. Under a REQUIRED import the mod never
// reaches `mod_initialize` to run it, so the intent was unreachable. This makes
// the guard live and the comment true.
//
// Version risk is nil either way: UiService is v1.0, so a required import would
// have demanded `major 1, minor >= 0` — satisfied by any 1.x host. The failure
// mode being fixed is ABSENCE, not staleness.
// ============================================================================
IMPORT_OPTIONAL_SERVICE(UiService, s_ui);

// Mangled names only. Duplicated mangled names measure ZERO manifest-wide
// (parameter types are IN the mangled name, so overloads disambiguate by
// construction) against 5,076 duplicated plain names — and both targets are
// verified SAFE on our fork AND vanilla upstream via
// `upstream_conformance.py --symbol`. That check is this lane's standing gate:
// no by-name hook is written here without a two-image SAFE verdict first.
DEFINE_HOOK_SYMBOL("?fpcPf_Get@@YAPEBUprocess_profile_definition@@F@Z", void*(short), PfGet);
DEFINE_HOOK_SYMBOL("?cDyl_IsLinked@@YAHF@Z", int(short), DylIsLinked);
// Third leg of the §747 chain: the PLACEMENT ROUTE. Keys on an OBJECT NAME
// string, not a room number, so gate 0-C does not bind it (Integrator, row 283).
DEFINE_HOOK_SYMBOL("?dStage_searchName@@YAPEAUdStage_objectNameInf@@PEBD@Z",
                   void*(const char*), StageSearchName);

// ---------------------------------------------------------------------------
// THE STAGE SEAM — HOOK WITHDRAWN 03:1x, REPLACED BY A READ. Measured, not
// reasoned: run 030100 armed the override with `sea` and reported
// `startstage_answers: 0`. The hook installed MOD_OK and was NEVER CALLED.
//
// WHY: `dComIfGp_getStartStageName` is declared INLINE (`d_com_inf_game.h:467`)
// and every consumer inlines it — `d_resorce.cpp:1111`, `d_stage.cpp:364/404/
// 581/1863/2011/2223/2248`. A by-name hook patches the out-of-line copy that
// nobody calls. `--symbol SAFE` proved the symbol exists; it never proved a
// call site reaches it. That is the §480 rule one level up, and the receipt
// was written in advance to say exactly this if answers came back 0.
//
// AND THE FINDING THAT MAKES THE HOOK UNNECESSARY RATHER THAN JUST BROKEN:
// THE RECEIVER ALREADY COPIES NEXT -> START ITSELF. `dScnPly_c::phase_1`
// (`d_s_play.cpp:1308`) runs `dComIfGp_setStartStage(dComIfGp_getNextStartStage())`
// on play-scene creation, then `offEnableNextStage()` at :1317. So after a
// successful transition the start stage IS the WW stage, natively, with no
// interception at all. We were about to fake a value the receiver sets for us.
//
// SO THE QUESTION IS NO LONGER "how do we answer" BUT "does that copy run, and
// with what name" — and that is a READ, not a hook. We resolve the same
// out-of-line getter as a CALLABLE and ask the receiver what it currently
// thinks its start stage is. It reads the same `mStartStage` the inlined
// copies read, so the answer is the receiver's own truth rather than ours.
// ---------------------------------------------------------------------------
typedef const char* (*FnGetStartStageName)();
FnGetStartStageName s_fnGetStartStageName = nullptr;
// NEXT-stage name, same signature, resolved alongside (Integrator, §1016).
// START alone cannot separate "the 1308 copy never ran" from "setNextStage
// never stuck" — reading both in one boot answers which.
FnGetStartStageName s_fnGetNextStageName = nullptr;

// ---------------------------------------------------------------------------
// THE LAST UNMEASURED LINK — `dStage_dt_c_stageLoader`, PURE OBSERVER.
//
// WHAT RUN 031649 BRACKETED, and why this is the only place left to look:
// the shutdown read returned `startstage_now: "sea"` — a FRESH call to the
// receiver's own getter, not a cached value. So `dScnPly_c::phase_1`
// (`d_s_play.cpp:1308`) ran, copied next -> start, and the receiver's start
// stage genuinely IS the WW stage. **And `search_calls_total` was 87 for the
// THIRD run running** — no stage parsing, no `/res/Stage/sea/` request.
//
// So the request lands, the copy runs, the name is right, and the loader
// still never executes. That eliminates both fixes on the table: §1015's
// `getNextStartStage` hook (the copy already works) and my own withdrawn
// `getStartStageName` seam (the value was already correct). **The fault is
// downstream of the start-stage assignment**, which is the first time it has
// been bracketed on both sides rather than guessed at.
//
// SO: does the stage loader run at all? `dStage_dt_c_stageLoader` is SAFE on
// both images. This hook COUNTS AND RETURNS — no answer, no skip, no retval
// touched. A probe that changed loader behaviour while measuring whether the
// loader runs would be measuring itself.
//
// AND ITS SILENCE IS INFORMATIVE, WHICH IS WHY IT IS WORTH A BOOT: because
// the install result is reported, `loader_calls: 0` means the loader was
// never invoked — it cannot be confused with "the hook failed", which is the
// ambiguity that made three black screens read the same way.
//
// NOT USED: `dStage_dt_c_roomLoader`. It is DIVERGENT (donor 2 params /
// receiver 3, §623) and stays out on principle even here, where a pre-hook
// reads no arguments — the rule is not conditional on whether this particular
// use would survive it.
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// THE MULT CHUNK — port the donor's own handler for WW stages (§1020).
//
// DN-10 STEP 1, and the comparison is unusually clean. Donor
// (`WW DP/src/d/d_stage.cpp:1959`) is THREE LINES:
//     dStage_Multi_c* multi = (dStage_Multi_c*)((char*)i_data + 4);
//     i_stage->setMulti(multi);
//     return 1;
// WW NEVER WALKS THE MULT CHUNK AT LOAD. The receiver's version has the same
// first two lines and then ADDS `dStage_initRoomKeepDoorInfo()`, two
// `readMult()` passes and `setRoomNo(-1)`. The crash is TP's added eager walk
// applied to donor data that was never meant to be walked at load time —
// `readMult` -> `OffsetPtr::setBase` -> `be32s` rebasing a base-relative
// offset to a wild address. So this is not a guard around a receiver bug; it
// is the donor's own handler, run for donor data.
//
// SIGNATURE IS THE RECEIVER'S, NOT THE DONOR'S. `d_stage.cpp` declares
// `(dStage_dt_c* i_stage, void* i_data, int entryNum, void* param_3)` — four
// parameters, STAGE FIRST. The donor's is `(void* i_data, dStage_dt_c*)`.
// Binding the donor shape here would put i_data where i_stage belongs, which
// is the `fopAcM_GetParamBit` failure exactly: right name, wrong call.
//
// WHY IT CANNOT BE INLINED AWAY (the trap that killed the getStartStageName
// seam): its address is TAKEN in the chunk dispatch table (`{"MULT",
// dStage_multInfoInit}`), so a call must go through the pointer. The
// Integrator additionally resolved it on the USER'S ACTUAL IMAGE —
// `rva=0x1b7d80 [CODE|LOCAL]`, no MULTI_NAME — so the LOCAL linkage of the
// `static` definition does not prevent binding.
// ---------------------------------------------------------------------------
DEFINE_HOOK_SYMBOL("dStage_multInfoInit", int(void*, void*, int, void*), MultInfoInit);

// ============================================================================
// THE PER-STAGE MESSAGE GROUP — the donor has no such concept, so for a WW
// stage the receiver's mount must not run (§1025).
//
// ROOT CAUSE, MEASURED IN BOTH TREES, NOT INFERRED: the receiver reads
// `mMsgGroup` at **0x28** of `stage_stag_info_class`; the DONOR's struct is
// **0x20 bytes long** (`WW DP/include/d/d_stage.h:66`, fields end 0x1C,
// `// Size: 0x20`). **Eight bytes past the end, every WW stage load.** The
// first 0x14 bytes genuinely agree — near/far planes, camera type — which is
// why it hid: a struct that matches for 20 bytes and then diverges passes
// every cheap sanity check.
//
// THE CHAIN: `readMessageGroupLocal` -> `dStage_stagInfo_GetMsgGroup` reads OOB
// -> `sprintf(arcName, "/res/Msguk/bmgres%d.arc", garbage)` -> mount produces
// nothing usable -> `setMsgDtArchive(1, cmd->getArchive())` -> the meter
// dereferences it -> wild jump.
//
// DN-10 STEP 1 GIVES THE FIX AND IT IS NOT A TRANSLATION: **WW HAS NO
// `mMsgGroup` FIELD**, so there is nothing to map and no value to pick. The
// donor's per-stage message-group mount does not exist, so for a WW stage the
// receiver's must not run. **Second instance of the MULT pattern: donor chunk,
// receiver parser, fields the donor never had.**
//
// AND WE DO NOT INVENT A GROUP. Picking 0 would mount a real TP archive onto a
// WW stage and look like it worked, which is worse than crashing.
//
// SIGNATURE: non-static member, so `this` IS present — arg 0 is the
// `dMsgObject_c*`, arg 1 is `mDoDvdThd_mountXArchive_c** p_arcMount`. That is
// the opposite of `getGlbResource` (static, no `this`) and getting it backwards
// writes NULL through the object pointer.
// ============================================================================
// MANGLED — INTEGRATOR GATE FIX 2026-08-16. Bare `readMessageGroupLocal` is
// `[CODE|MULTI_NAME] (+1 more)` on the user's image (`dMsgObject_c::` has both
// `readMessageGroup` and `readMessageGroupLocal` under overlapping bare names),
// so it would have failed to install exactly as `setStageRes` did — and a
// hook that does not install reports ZERO and reads as evidence.
//
// THIRD TIME THIS EXACT CHECK HAS CAUGHT A BARE MEMBER BINDING (`setStageRes`,
// `getRes`, now this). The signature was already right: `QEAA` is a NON-static
// member, so `this` IS a real first parameter and `void(void*, void**)` is
// correct — unlike `getRes`/`getGlbResource`, which were `SA` (static) and had
// a phantom `this`. **The two cases look identical and differ by one letter in
// the mangling; that is why the mangling gets read, not guessed.**
DEFINE_HOOK_SYMBOL("?readMessageGroupLocal@dMsgObject_c@@QEAAXPEAPEAVmDoDvdThd_mountXArchive_c@@@Z",
                   void(void*, void**), ReadMsgGroupLocal);

// ============================================================================
// THE J3D PARSE SEAM — and the reason DN-3 may not bind here at all.
//
// DN-3 forbids ADDING BDL parsing at arc-mount, because on the FORK a
// consume-time layer (`ExtNpcMount` / `acquireBgModel` / `s_modelDataCache`)
// already parses, and a second pass re-fixes an already-pointer-fixed buffer.
// **On VANILLA there is no consume-time layer.** And `dRes_info_c::loadResource`
// (`d_resorce.cpp:455-467`) ALREADY calls `J3DModelLoaderDataBase::load` and
// `::loadBinaryDisplayList` at mount, in the RELEASE build — that is the
// receiver's own native behaviour, not something we add.
//
// So the question is NOT "port a parser". It is which of three things is true,
// and these want completely different answers:
//   (a) the parse never runs on our served file  -> why? a type/flag gate we
//       may be able to satisfy from the plugin, with no receiver change;
//   (b) it runs and returns NULL                 -> TP's loader rejects WW's
//       J3D block layout, and that is a format question;
//   (c) it runs and returns a model              -> the parse is fine and
//       `mDoExt_setupStageTexture` is failing on something else downstream.
//
// PURE OBSERVERS, POST-SHAPED so the RESULT is visible: a log of "we were
// asked to parse" cannot separate (b) from (c), and (b)-vs-(c) is the whole
// question. Both bound MANGLED: bare `loadBinaryDisplayList` is
// `[CODE|MULTI_NAME] (+3 more)` on the user's image, and both are `SA`
// (STATIC) — `void*(const void*, unsigned int)`, NO `this`. Fourth and fifth
// time that pair of traps has been checked rather than guessed.
// ============================================================================
DEFINE_HOOK_SYMBOL("?loadBinaryDisplayList@J3DModelLoaderDataBase@@SAPEAVJ3DModelData@@PEBXI@Z",
                   void*(const void*, unsigned int), J3DLoadBDL);
DEFINE_HOOK_SYMBOL("?load@J3DModelLoaderDataBase@@SAPEAVJ3DModelData@@PEBXI@Z",
                   void*(const void*, unsigned int), J3DLoadBMD);
// 2N′: locked/patched BDL materials no-op makeSharedDisplayList. During WW
// finish only, run J3DMaterial's base bake so the shared DL is not empty.
DEFINE_HOOK_SYMBOL("?makeSharedDisplayList@J3DLockedMaterial@@UEAAXXZ",
                   void(void*), LockedMakeSharedDL);
DEFINE_HOOK_SYMBOL("?makeSharedDisplayList@J3DPatchedMaterial@@UEAAXXZ",
                   void(void*), PatchedMakeSharedDL);

// ============================================================================
// THE MISSING `'BMDL'` DISPATCH CASE — supplied plugin-side.
// USER RULING 2026-08-16 ("Build the hook"), per the fork-to-plugin ROADMAP:
// *"it has two cheaper answers (supply the dispatch case via hook; or origin
// un-gates it)"* and *"build the hook as though the proposal will never land."*
//
// WHAT IS ACTUALLY MISSING, and why this is a GATED case and not a port:
// `dRes_info_c::loadResource` (`d_resorce.cpp:312`) dispatches on the RARC
// directory node 4CC. The `'BMDL'` arm EXISTS at :452 and is wrapped in
// `#if DEBUG`, so a release image has no branch for it: the slot keeps the RAW
// buffer, and the first consumer to cast it to `J3DModelData*` faults. That is
// the `daBg_c::createHeap` crash. Nothing is absent from the receiver — one
// dispatch arm is compiled out (Foundry's EXISTS/GATED/ABSENT taxonomy, §1013).
//
// WHY THIS IS NOT DN-3. DN-3 forbids ADDING A SECOND PARSE LAYER: on the fork,
// ExtNpcMount already parses at consume time, so a mount-time parse would parse
// the same buffer twice and the second pass re-fixes an already-pointer-fixed
// buffer. **On a clean dusklight that consume-time layer DOES NOT EXIST — it is
// fork code.** There is no second parser to collide with, so the double-parse
// mechanism DN-3 exists to stop cannot occur. DN-3's own amendment states the
// condition ("requires REMOVING the consume-time layer in the same change"),
// and here it is satisfied by construction rather than by an edit.
//
// AND THE SINGLE-PARSE GUARD IS DN-3'S OWN VERIFICATION SIGNATURE: we act only
// on a slot whose first 4 bytes are still `'J3D2'` (0x4A334432). A parsed slot
// holds a heap `J3DModelData` whose head is a vtable pointer, so a second pass
// cannot re-enter. DN-3: *"`modelData` head reads a heap vtable, never
// 0x4A334432 (raw `J3D2`)."* The check that proves the fix is the check that
// makes it idempotent.
//
// **NO-OP ON VANILLA CONTENT BY CONSTRUCTION.** A slot is only touched if it is
// STILL raw after the receiver's own dispatch ran — i.e. the receiver had no
// branch for that node type. Every stock TP resource is handled by an existing
// arm, so on unmodded content this hook finds nothing and changes nothing.
// This is the same seam the fork opened as `dRes_info_c::setRes(i, res)` (§634,
// *"a donor archive can file a resource under a node type this receiver has no
// branch for"*) — that was a receiver edit; this is the plugin-side equivalent
// that needs no receiver change at all.
//
// FIELD OFFSETS ARE MEASURED, NEVER COMPUTED. `cl /d1`-style probe compiled
// against BOTH trees with each tree's own ninja flags (2026-08-16):
//   dRes_info_c::mArchive          0x18   | JKRArchive::mArcInfoBlock   0x78
//   dRes_info_c::mRes              0x30   | SArcDataInfo::num_file_entries 0x08
// mRes/mArchive are IDENTICAL on fork and clean, so this binds on either.
// (`sizeof(dRes_info_c)` DIFFERS — 64 clean vs 56 fork — because the fork
// narrowed `mSize`'s guard from `PARTIAL_DEBUG || DEBUG` to `DEBUG`. We never
// take the size, only these two offsets, so the divergence cannot reach us.
// It is filed separately: PARTIAL_DEBUG exists so debug and release share one
// struct ABI, and that promise is broken for this class on the fork.)
//
// `num_file_entries` is `BE(u32)` — BIG-ENDIAN in the archive image — so it is
// byteswapped on read. Reading it natively yields a huge count and walks off
// the array; that is the whole reason it is called out here.
// ============================================================================
DEFINE_HOOK_SYMBOL("?loadResource@dRes_info_c@@QEAAHXZ", int(void*), ResLoadResource);

// ============================================================================
// OUTSET VISIBILITY PROBE — the draw-entry question, asked at the ONE call
// that decides it.
//
// STATE THIS ANSWERS. As of run 125821 the WW stage LOADS and PARSES: `sea` is
// the start stage, `model.bdl` arrives raw, the consume-parse fires on that
// exact pointer and returns a model, three BDLs parse, four `fpcNm_BG_e`
// actors create, 275 textures resolve with ZERO misses, and it runs to frame
// 2700 with no crash, no `resLoad ERROR` and no arc purge. **Geometry loads.
// Nothing reaches the screen.**
//
// `daBg_c::draw()` walks `mBgParts` and calls `mDoExt_modelEntryDL(model)` per
// part (`d_a_bg.cpp:452`). That call IS the submission. So one binary split
// settles which half of the pipeline is broken:
//
//   FIRES  -> geometry IS submitted; the fault is DOWNSTREAM (camera, viewport,
//             draw-list flush, clipping) and the BG actor is doing its job.
//   SILENT -> nothing is submitted; the fault is UPSTREAM, in the parts array
//             or the room parameter, and no amount of render debugging helps.
//
// WHY THIS SYMBOL AND NOT `daBg_c::draw`: hooking the draw member would need
// `mBgParts`' offset inside `daBg_c` — another measured constant, another
// chance to be wrong. This needs NONE: one argument, `J3DModel*`, and the
// question is simply whether it arrives. Fewer numbers, same answer.
//
// `void(J3DModel*)` — the return type matters: my first gate attempt mangled
// it as `H` (int) and `upstream_conformance` reported MISSING on BOTH images,
// which is leg 1 catching a wrong signature rather than a wrong name.
// ============================================================================
DEFINE_HOOK_SYMBOL("?mDoExt_modelEntryDL@@YAXPEAVJ3DModel@@@Z",
                   void(void*), ModelEntryDL);

// ============================================================================
// THE GX CENSUS — plugin-side, no aurora change, and it tests the ONE mechanism
// still standing.
//
// WHY THIS AND NOT MORE aurora WORK. The user's constraint is that the fix must
// live plugin-side, and the tracked measurement says ~90% of the fork's aurora
// work-set is patch-only. But the Dolphin GX API is a different story: the host
// exports 247 GX entry points including `GXCallDisplayList`,
// `GXBeginDisplayList` and `GXEndDisplayList` (verified via dumpbin /EXPORTS).
// Those are resolvable by name at runtime, so the plugin can MEASURE the GX
// stream without touching the backend at all.
//
// WHAT IT TESTS. `fifo::publish()` returns early while `sInDisplayList` is true
// (`fifo.cpp:173`), and `begin_display_list` sets that latch while
// `end_display_list` clears it. AN UNBALANCED begin THEREFORE STOPS THE FIFO
// FROM EVER DRAINING - which is exactly the runaway signature: `drain()` runs
// per frame from `aurora.cpp:251`, so death ~8-11 frames in means the drain is
// not doing its job rather than that geometry is heavy.
//
// THE PREDICTION IS FALSIFIABLE AND CHEAP: if `begins != ends` and the gap grows
// per frame, that is the desync and its author is whoever left it open. If they
// stay balanced, THE RECORDED MECHANISM IS WRONG and I will say so - this lane
// has already retracted three mechanisms tonight and a fourth costs nothing but
// a build.
//
// NOTE ON SIGNATURES: these are C entry points, so the symbol names are
// undecorated. A wrong signature here is caught by the hook layer at install,
// which is why each install result is reported rather than assumed.
// ============================================================================
DEFINE_HOOK_SYMBOL("GXCallDisplayList", void(const void*, unsigned int), GxCallDL);
DEFINE_HOOK_SYMBOL("GXBeginDisplayList", void(void*, unsigned int), GxBeginDL);
DEFINE_HOOK_SYMBOL("GXEndDisplayList", unsigned int(void), GxEndDL);

// ============================================================================
// THE PACKET-CYCLE PROBE — catch the self-loop AS IT IS CREATED.
//
// `lwood_pkt_census` proved a cycle exists (clean through draw 41, `cycle:1`
// from draw 42, dead at 44). This names the instruction that makes it.
//
// `J3DMatPacket::addShapePacket` PREPENDS (`J3DPacket.cpp:199`):
//     pShape->setNextPacket(mpShapePacket);
//     mpShapePacket = pShape;
// If `pShape` IS ALREADY THE HEAD, that writes `pShape->next = pShape` — a
// one-line self-loop, and any traversal of it runs forever.
//
// How that can happen: `entryMatSort` merges same-material packets by walking
// its bucket and testing `packet->isSame(pMatPacket)`. A matPacket ENTERED
// TWICE IN ONE FRAME finds ITSELF in that bucket, and the merge then reads
// `A->addShapePacket(A->getShapePacket())` — head prepended onto itself.
// `J3DJoint::entryIn` resets the head each frame (`J3DJoint.cpp:179`,
// `matPacket->setShapePacket(shapePacket)`), which is what normally keeps this
// impossible — so a model that skips or double-runs that path is the suspect.
//
// The probe CHANGES NOTHING. It reports the condition at the moment of creation,
// with both pointers, so the author can be named instead of inferred.
// ============================================================================
DEFINE_HOOK_SYMBOL("?addShapePacket@J3DMatPacket@@QEAAXPEAVJ3DShapePacket@@@Z",
                   void(void*, void*), AddShapePacket);

// The entry side. `entryMatSort` is the ONLY caller that can make a matPacket
// merge with itself, so recording its current subject lets the self-loop probe
// say WHICH call created the cycle instead of leaving it to inference.
DEFINE_HOOK_SYMBOL("?entryMatSort@J3DDrawBuffer@@QEAAHPEAVJ3DMatPacket@@@Z",
                   int(void*, void*), EntryMatSort);

// THE REAL FRAME BOUNDARY. `dDlst_list_c::reset()` calls `frameInit()` on all 21
// draw buffers (`d_drawlist.cpp:1930`), and `frameInit` NULLs every bucket
// (`J3DDrawBuffer.cpp:47`). That is the only point at which a matPacket stops
// being in its bucket.
//
// The previous probe cleared its duplicate tally in `wwGxCensusTick`, which
// fires once per LWOOD DRAW - four times a frame - so its window was far
// narrower than a frame and its `pkt_double_entry:0` was NOT evidence.
// Anchoring to reset() makes the count mean what it says.
DEFINE_HOOK_SYMBOL("?reset@dDlst_list_c@@QEAAXXZ", void(void*), DlstReset);

// ============================================================================
// THE UPSTREAM HALF — run 132033 answered the binary split and it is UPSTREAM.
//
// That run REACHED Outset (`R44_00` looked up 18 times), PARSED its geometry
// (`bmdl_consume_parse` x3) and submitted NOTHING (`draw_entry` 0). So the
// fault is not camera, viewport or flush — geometry never reaches the list.
//
// `daBg_c::createHeap` turns parsed DATA into a drawable INSTANCE at
// `d_a_bg.cpp:~197`:
//
//     bgPart->model = mDoExt_J3DModel__create(modelData, 0, modelFlags);
//     if (bgPart->model == NULL) { return 0; }
//
// **`return 0` from createHeap DELETES THE ACTOR.** So a NULL here explains
// every symptom at once: no model, no draw entry, and a BG that logged its
// resource lookups and then quietly went away. `fpcBs_Create OK` does NOT
// contradict that — process creation and heap creation are separate phases,
// and only the first was ever in the log.
//
// THREE OUTCOMES, all worth having:
//   called + non-NULL  -> instances exist; the fault is between here and the
//                         draw list (parts array, room param, actor lifetime)
//   called + NULL      -> construction REJECTS the donor model data, and the
//                         parse succeeding told us less than it appeared to
//   never called       -> createHeap bails BEFORE construction, upstream again
//
// `J3DModel*(J3DModelData*, u32, u32)` — mangled, and the FIRST mangling I
// tried was the right one only by luck: the second (`KK` for the flag pair)
// reported MISSING, which is leg 1 distinguishing a wrong parameter type from
// a wrong name.
// ============================================================================
DEFINE_HOOK_SYMBOL("?mDoExt_J3DModel__create@@YAPEAVJ3DModel@@PEAVJ3DModelData@@II@Z",
                   void*(void*, unsigned int, unsigned int), J3DModelCreate);

// ============================================================================
// THE LAST SPLIT. Run 132503 removed every remaining upstream candidate:
// Outset reached (18 lookups), geometry parsed (3 BDLs), and **12 J3DModel
// instances constructed with ZERO NULLs** - so `createHeap` did NOT bail, the
// models exist, and they are attached to their parts. And `draw_entry` is
// STILL 0.
//
// That leaves exactly one question: **is `daBg_c::draw()` ever called?**
//
//   CALLED    -> it runs and skips the entry loop; the parts array or an
//                internal gate is the fault, and it is inside one function.
//   NEVER     -> the actor is never asked to draw. Then the fault is the
//                process/room layer, and the user's `Room: -1` is no longer a
//                sibling symptom but the lead - TP draws actors per ROOM, and
//                a BG whose room is not active is never invited.
//
// `?draw@daBg_c@@QEAAHXZ` - NON-virtual member (`inline int draw()` in the
// header, reached through the process-method table at `d_a_bg.cpp:639`). The
// `U` and `M` manglings both report MISSING, which is leg 1 telling the
// difference between a virtual and a plain member rather than a bad name.
// ============================================================================
DEFINE_HOOK_SYMBOL("?draw@daBg_c@@QEAAHXZ", int(void*), BgDraw);
// Donor daBg_btkAnm_c: SC_01 materials sync to wave frame (d_a_bg.cpp:72-87).
DEFINE_HOOK_SYMBOL("?entry@daBg_btkAnm_c@@QEAAXPEAVJ3DModelData@@@Z",
                   void(void*, void*), BgBtkEntry);
DEFINE_HOOK_SYMBOL("?play@daBg_btkAnm_c@@QEAAXXZ", void(void*), BgBtkPlay);

// Sky list flush. dComIfGd_drawOpaListSky is header-inline onto this.
DEFINE_HOOK_SYMBOL("?drawOpaDrawList@dDlst_list_c@@QEAAXPEAVJ3DDrawBuffer@@@Z",
                   void(void*, void*), DrawOpaList);

// WW camera selector. Vanilla has no dCamera_setSelectHook. nextType is the
// first call of the type/mode/style block in Run(), after setMapToolData.
DEFINE_HOOK_SYMBOL("?nextType@dCamera_c@@QEAAHH@Z", int(void*, int), CamNextType);
DEFINE_HOOK_SYMBOL("?nextMode@dCamera_c@@QEAAHH@Z", int(void*, int), CamNextMode);
DEFINE_HOOK_SYMBOL("?onStyleChange@dCamera_c@@QEAA_NHH@Z", bool(void*, int, int),
                   CamOnStyleChange);
// CRAWL socket: extra-engine 20 remapped onto test2 (engine_tbl[19]).
DEFINE_HOOK_SYMBOL("?test2Camera@dCamera_c@@QEAA_NH@Z", bool(void*, int), CamTest2);

// ============================================================================
// THE INLINE DISAMBIGUATOR - because `bg_draw: 0` is NOT yet an answer.
//
// Run 132759: Outset reached, 12 models constructed, `draw_entry` 0, and
// `bg_draw` 0 with the hook reporting INSTALLED **and** ATTACHED. Tempting to
// conclude "draw() is never called". **But `daBg_c::draw()` is `inline int
// draw()` in the header, and the process table registers the LOCAL static
// wrapper `daBg_Draw` which calls it.** If the compiler inlined draw() into
// that wrapper, my hook sits on an out-of-line copy nobody calls - dead, with
// no warning, reporting a silence that means the opposite of what it looks
// like. That is `setMsgDtArchive` one level subtler, and it already cost a
// boot this week.
//
// `mDoLib_clipper::changeFar()` is STATIC and out-of-line, `d_a_bg.cpp` calls
// it EXACTLY ONCE - at the top of draw(), before the parts loop. So:
//
//   changeFar fires -> draw() IS running; my BgDraw hook is inlined-dead, and
//                      the fault is INSIDE draw(), in the parts loop or a gate
//   changeFar silent -> draw() genuinely never runs; the actor is never asked,
//                      and `Room: -1` becomes the lead
//
// Other actors call it too (d_a_ep, d_a_mirror, d_bg_parts), so the count is
// an upper bound - but on a WW stage a NON-ZERO count with `draw_entry` still
// 0 is the discriminating fact, not the exact number.
// ============================================================================
DEFINE_HOOK_SYMBOL("?changeFar@mDoLib_clipper@@SAXM@Z", void(float), ClipChangeFar);

// ============================================================================
// WHO ACTUALLY GETS DRAWN - the membership question, asked without guessing.
//
// ESTABLISHED so far: `daBg_c::draw()` never runs (changeFar silent on a run
// that reached Outset); the draw LOOP does run every frame; `fpcDw_Execute`
// gates only on pause; and the iteration is
// `fpcLyIt_OnlyHere(fpcLy_RootLayer(), ...)` where `cTrIt_Method` walks that
// layer's OWN `mpLists` and DOES NOT RECURSE. "OnlyHere" is literal.
//
// So drawing only ever visits processes in the ROOT layer's lists - and the
// log shows two layers live: `0x7ff6a161a920` (module-range, the static root)
// and a HEAP layer `0x2063...`. If the BG lands in the heap one it is never
// visited, which would explain every symptom.
//
// **THAT LAST STEP IS STILL INFERENCE.** `fpcBs_Create` does not log a layer,
// unlike `fpcNdRq_phase_Create`, so BG membership is unproven. This settles it
// by enumerating what IS drawn.
//
// NO STRUCT OFFSETS: logging the raw `base_process_class*` is enough, because
// the receiver ALREADY prints `fpcBs_Create OK proc=0x...` beside each
// profile name. Cross-referencing two existing logs beats measuring a layout
// I would have to trust. (`profname` at 0x0E is scalar-only and would probably
// be safe - "probably safe" is exactly the reasoning that produced the
// byte-reversed J3D2 magic this morning.)
// ============================================================================
DEFINE_HOOK_SYMBOL("?fpcDw_Execute@@YAHPEAUbase_process_class@@@Z",
                   int(void*), DwExecute);

// ============================================================================
// THE MEMBERSHIP GATE ITSELF. Measured chain so far, each step from a log:
//   131 processes created after the warp, EXACTLY 1 ever drawn (an OVERLAP0)
//   TP draws 40 distinct incl. BG_e, ALINK_e, CAMERA_e, PLAY_SCENE_e
//   EXECUTE iterates `g_fpcLn_Queue` - a GLOBAL queue, layer-independent
//   DRAW iterates `fpcLyIt_OnlyHere(fpcLy_RootLayer())` - ONE layer, no recursion
// So WW processes execute (resources load, models build) and never draw,
// because execution ignores layer membership and drawing is nothing else.
//
// `fpcLyTg_ToQueue` is the gate that grants membership, and its first line is:
//     if (i_layer_tag->layer == NULL && i_layerID == fpcLy_NONE_e) return 0;
// `fpcLy_NONE_e` == 0xFFFFFFFF. A process with a NULL layer tag AND layer id -1
// is never queued into any list -> never drawn.
//
// HOOKED HERE RATHER THAN AT `fpcEx_ToExecuteQ` DELIBERATELY: the executor
// passes `i_proc->priority.current_info.layer_id`, and reaching that from
// outside needs THREE PPC-derived offsets (proc->priority, ->current_info,
// ->layer_id). This function takes the layer id as **arg1**, plainly, and its
// RETURN says whether the process was granted membership. Zero offsets, and
// the answer is the return value rather than an inference from it.
//
// READS: on a WW stage, `ret=0` with `layer=0xFFFFFFFF` confirms the gate is
// the mechanism. `ret=1` means membership is granted and the fault is further
// on - which would be worth knowing before anyone "fixes" the layer id.
// ============================================================================
DEFINE_HOOK_SYMBOL("?fpcLyTg_ToQueue@@YAHPEAUlayer_management_tag_class@@IGG@Z",
                   int(void*, unsigned int, unsigned short, unsigned short), LyTgToQueue);

// ============================================================================
// PROBE SET #20 — THE SCENE-CREATE PHASE LADDER (run 140618 ruled the queue
// gate OUT: `lytg_toqueue` on the WW stage granted membership every time,
// zero refusals against a 12-slot budget. So processes ARE queued and still
// never drawn — which moves the question from "who refused membership" to
// "did dScnPly's CREATE ever finish". A scene stuck in its phase machine
// executes its children and never runs its own draw, which is EXACTLY the
// measured shape: 131 created, 1 drawn, the 1 being the warp's OVERLAP wipe.)
//
// The phase functions themselves are `static` in d_s_play.cpp — unhookable.
// Each phase has an EXTERNAL-LINKAGE callee, so the ladder is observed from
// the callees, all four verified in the user's own image manifest:
//   phase_1  -> dStage_dt_c_stageLoader          (ALREADY HOOKED, StageLoader)
//   phase_01 -> Z2SceneMgr::setSceneName          + load1stDynamicWave
//   phase_2  -> dComIfG_syncAllObjectRes          (ret>0 repeated = res stall)
//   phase_3  -> Z2SceneMgr::check1stDynamicWave   (true repeated = wave stall)
//
// HYPOTHESES THIS ONE BUILD DISCRIMINATES (multi-hypothesis rule):
//   H1 phase_01 stall — setSceneName never fires / load1st never follows
//   H2 phase_2 stall  — syncAllObjectRes returns >0 forever (WW object arcs
//      requested by TP's res system and never arriving)
//   H3 phase_3 stall  — check1stDynamicWave returns true forever (audio has
//      no scene named "sea"; `load1stWait` or a wave request stuck at 1)
//   H4 ladder COMPLETES — all milestones pass and BgDraw still never fires:
//      the fault is in the scene-draw/draw-list domain, not scene create
//   H5 ladder never STARTS — StageLoader silent post-warp: create never began
//   H6 sync returns negative — the error path, a different fault entirely
// READS: the LAST ladder event in the log names the stalled phase directly.
// ============================================================================
DEFINE_HOOK_SYMBOL("?setSceneName@Z2SceneMgr@@QEAAXPEADHH@Z",
                   void(void*, char*, int, int), AudSetScene);
DEFINE_HOOK_SYMBOL("?load1stDynamicWave@Z2SceneMgr@@QEAAXXZ",
                   void(void*), AudLoad1st);
DEFINE_HOOK_SYMBOL("?check1stDynamicWave@Z2SceneMgr@@QEAA_NXZ",
                   bool(void*), AudCheck1st);
// dComIfG_syncAllObjectRes is inline → syncAllRes(mObjectInfo, …). The free
// function symbol never exists; hook the real callee or the probe is silent.
DEFINE_HOOK_SYMBOL("?syncAllRes@dRes_control_c@@SAHPEAVdRes_info_c@@H@Z",
                   int(void*, int), SyncObjRes);

// ============================================================================
// PROBE SET #21 — THE DRAWABILITY DOOR. Run 142432 pushed the create ladder
// through phase_4 on the WW stage (msgarc_set start:"sea") with no stall
// alarms, while the draw census still reads WW:1 vs TP:40. The mechanism was
// then found in source, and it is one line — f_op_actor.cpp:543:
//     if (ret == cPhs_COMPLEATE_e) { fopDwTg_ToDrawQ(&actor->draw_tag, ...); }
// An actor joins the DRAW queue ONLY when its own create method returns
// COMPLEATE (4). A create looping INIT (0) executes every frame and never
// draws — which reproduces every number measured so far: 131 executing,
// layer membership granted, one drawn.
//
// So the question is no longer "which scene phase" but "WHICH ACTORS never
// finish create, and with what status". Two hooks:
//   fpcMtd_Create  (post) — per-process create status, the cPhs return.
//     Census: distinct procs by LAST status; a transition (INIT->COMPLEATE,
//     INIT->ERROR) logs once. ERROR (5) actors get condition 0x10 and vanish
//     from execute — loopers stay INIT forever.
//   fopDwTg_ToDrawQ (post) — the door itself; counts admissions per side.
//     WW admissions ~1 while TP shows ~40 confirms the door is the choke.
// cPhs values (SComponent/c_phase.h, by position): INIT=0 LOADING=1 NEXT=2
// UNK3=3 COMPLEATE=4 ERROR=5.
// Pointers cross-reference the loader's own `fpcBs_Create OK proc=` lines —
// deliberately NO struct offsets read from the proc (the PPC-offset trap).
// ============================================================================
DEFINE_HOOK_SYMBOL("?fpcMtd_Create@@YAHPEBUprocess_method_class@@PEAX@Z",
                   int(const void*, void*), MtdCreate);
DEFINE_HOOK_SYMBOL("?fopDwTg_ToDrawQ@@YAXPEAUcreate_tag_class@@H@Z",
                   void(void*, int), DwTgToDrawQ);

// ============================================================================
// PROBE SET #22 — THE REMOVAL SIDE OF THE DRAW QUEUE. Run 143410 proved:
//   · the WW PLAY_SCENE's create COMPLETED (census transition 0->4)
//   · 28 actors COMPLEATED, >=6 admissions logged through the ToDrawQ door
//   · the frame walk STILL hands fpcDw_Execute exactly ONE proc (OVERLAP0)
// The iterator (f_op_draw_iter.cpp) walks EVERY bucket of the ONE global tree
// `g_fopDwTg_Queue` with no layer filter and no skip — so a tag admitted and
// still resident WOULD be drawn. Admitted-but-not-drawn therefore means
// REMOVED between admission and the frame. Exactly two removal doors exist:
//   fopDwTg_DrawQTo     — the single-tag CUT (actor deletion path)
//   fopDwTg_CreateQueue — cTr_Create over the static lists = a TREE WIPE
//                         (scene-transition reset; anything admitted BEFORE
//                         the wipe is orphaned — a lost-in-the-wash race)
// READS: create_queue AFTER ww admissions = the wipe race is the black screen.
// drawq_cut for recently admitted ww tags = completed actors are being
// DELETED, and the next trace is who deletes them (room control is the
// suspect — the player's room is -1). Neither = the walk model is wrong and
// the next probe counts fopDwIt_Begin directly.
// ============================================================================
DEFINE_HOOK_SYMBOL("?fopDwTg_DrawQTo@@YAXPEAUcreate_tag_class@@@Z",
                   void(void*), DwTgCut);
DEFINE_HOOK_SYMBOL("?fopDwTg_CreateQueue@@YAXXZ",
                   void(), DwTgWipe);

// ============================================================================
// INSTRUMENT-GUARD (NOT A FIX) — THE NULL-PLAYER ATTENTION CRASH, run 144259.
// Symbolicated chain: dScnPly_Execute -> dAttention_c::Run ->
// setOwnerAttentionPos -> daPy_py_c::checkNoResetFlg1, fault addr 0x66c =
// NULL + offset. `d_attention.cpp:645` dereferences `mpPlayer` UNGUARDED,
// while `SelectAttention` ten lines below guards `mpPlayer == NULL` expressly
// — the receiver knows the state is real; this one site doesn't check.
// `mpPlayer` is captured at phase_4 construction where the player pointer has
// JUST been nulled; on TP the race is won when ALINK's create completes and
// re-registers. On a WW stage ALINK's create loops FOREVER (census 143410),
// so the crash is GUARANTEED once the scene's execute reaches attention. It
// fired on plain F_SP102 this time — a mainline-TP race, not WW-scoped.
//
// THE GUARD: pre-hook, skip the call iff the receiver's own exported player
// getter (`daPy_getPlayerActorClass`) returns NULL — the IDENTICAL predicate
// SelectAttention uses, applied one call earlier, zero struct offsets. When
// the player exists the original always runs.
// LABELLED A BRIDGE: the real fix is the attention/player lifecycle (fork-
// side receiver work, or ALINK completing create on WW). On the owed list.
// ============================================================================
DEFINE_HOOK_SYMBOL("?setOwnerAttentionPos@dAttention_c@@QEAAXXZ",
                   void(void*), AttnSetOwnerPos);

// ============================================================================
// PROBE SET #23 — THE ADMISSION'S RETURN VALUE. Run 145820 exhausted the
// removal side: ONE wipe at boot (before any WW admission), ZERO cuts of WW
// tags — nothing removes the admitted tags, and they still never reach
// fpcDw_Execute. That leaves the door itself, and the source names a silent
// void the admission counter cannot see:
//   cTr_Addition (c_tree.cpp:14): `if (listIdx >= tree->mNumLists) return 0;`
//   — the tree has EXACTLY 1000 buckets, an out-of-range priority is REFUSED
//   WITH NO LOG, and `fopDwTg_ToDrawQ` is void so the refusal is DISCARDED.
// A hook on ToDrawQ counts CALLS; a refused insertion is indistinguishable
// from a successful one from there. This hook reads the one place the truth
// exists: cTg_AdditionToTree's return (0 = refused / already-in-use) plus the
// listIdx actually passed.
// ALSO NOTE the second hazard found while reading: `listIdx` is SIGNED and
// only the UPPER bound is checked — a negative priority would index
// mpLists[negative]. If refusals show negative indices, that is a separate,
// worse finding.
// READS: refused=1 rows on the WW side with their listIdx = the black
// screen's mechanism, and the bad priority is named in the same line.
// ============================================================================
DEFINE_HOOK_SYMBOL("?cTg_AdditionToTree@@YAHPEAUnode_lists_tree_class@@HPEAUcreate_tag_class@@@Z",
                   int(void*, int, void*), TgAddToTree);

// ============================================================================
// PROBE SET #24 — THE NODE WALK. Run 150345 exonerated the global tag tree
// completely: admissions succeed (ret=1, indices 0..689), the one wipe is at
// boot BEFORE the warp, zero cuts — and still fpcDw_Execute receives exactly
// OVERLAP0 on WW frames. A tag resident in a tree the interp loop walks
// unconditionally CANNOT be skipped, so the loop is not what draws these
// frames: the live path must be the NATIVE branch (fpcNd_DrawMethod — the
// per-NODE, layer-based walk), and OVERLAP0 reaches it through its node.
// The question is one hop up: IS THE WW SCENE'S NODE EVER DRAWN by its
// parent? This hook censuses fpcNd_Draw's node argument per stage side —
// the same distinct-set shape as the dw_exec census, one level higher.
// READS: WW side showing root-only (or missing the WW scene node) = the
// scene node never entered its parent's draw structure; the insert for
// NODES is the next and likely final trace. WW scene node present = the
// walk reaches it and the fault drops INTO its own layer iteration.
// ============================================================================
DEFINE_HOOK_SYMBOL("?fpcNd_Draw@@YAHPEAUprocess_node_class@@@Z",
                   int(void*), NdDraw);

// ============================================================================
// PROBE SET #25 — THE OVERLAP GATE. Run 153846 (read together with
// dusklight's own fopScnRq debug lines) located the stall precisely:
//   · fpcNd_Draw visits LOGO -> OPENING -> NAME as each becomes the CURRENT
//     scene, then NEVER fires again after the warp — the WW play scene never
//     joins the current-scene chain, so nothing under it can draw.
//   · post-warp, fopScnM_CreateReq fires (two ROOM_SCENE requests) but their
//     fopScnRq_phase_Execute lines NEVER print — the request phase machine
//     stalls BEFORE Execute, i.e. inside the fade path's overlap waits:
//     IsDoingOverlap / IsDoneOverlap gate on fopOvlpM, and ClearOfReq gates
//     the finish. The wipe being the ONLY thing drawn is the same fact from
//     the other side: the screen shows a transition that never ends.
// These three predicates ARE the gate. Log their returns post-warp, bounded:
// a predicate pinned at 0 names the stuck step; all three cycling normally
// would mean the stall is in fpcNdRq_Execute instead (next candidate).
// ============================================================================
DEFINE_HOOK_SYMBOL("?fopOvlpM_IsDoingReq@@YAHXZ", int(), OvlpIsDoing);
DEFINE_HOOK_SYMBOL("?fopOvlpM_IsDone@@YAHXZ",     int(), OvlpIsDone);
DEFINE_HOOK_SYMBOL("?fopOvlpM_ClearOfReq@@YAHXZ", int(), OvlpClear);

// ============================================================================
// PROBE SET #26 — THE OVERLAP'S OWN STATE MACHINE (d_ovlp_fade.cpp, sole
// holder of fpcNm_OVERLAP0_e). Run 154838 sharpened #25's answer: ClearOfReq
// ret=1 means the clear was REQUESTED (`cReq_Create(base, 2)`), not
// performed. The overlap runs Create -> FadeIn -> Wait -> FadeOut -> Done,
// and the wipe drawn forever = one of the first three never ends. The three
// states leak distinct external calls, so which state it is stuck in reads
// straight off which probes fire post-warp:
//   FadeIn: loops until `JFWDisplay::startFadeOut(speed)` accepts — a false
//     return forever = stuck at the very first step (the JUTFader refusing)
//   Wait:   polls `fopOvlpM_IsOutReq` — polled-and-0 forever = the clear
//     request never surfaces as an out-request
//   FadeOut: calls `fopOvlpM_SceneIsStart()` EVERY frame — and this is very
//     likely the signal that makes the new scene CURRENT for the node walk,
//     which would tie the whole black screen to one missing call
// `fopOvlpM_Done` marks each leg's completion; counting it tells legs apart.
// ============================================================================
DEFINE_HOOK_SYMBOL("?fopOvlpM_IsOutReq@@YAHPEAUoverlap_task_class@@@Z",
                   int(void*), OvlpIsOutReq);
DEFINE_HOOK_SYMBOL("?fopOvlpM_SceneIsStart@@YAHXZ", int(), OvlpSceneStart);
DEFINE_HOOK_SYMBOL("?fopOvlpM_Done@@YAXPEAUoverlap_task_class@@@Z",
                   void(void*), OvlpDone);
DEFINE_HOOK_SYMBOL("?startFadeOut@JFWDisplay@@QEAA_NH@Z",
                   bool(void*, int), JfwStartFadeOut);

// ============================================================================
// PROBE SET #27 — THE PUMPS AND THE COMMAND. Run 155713 named the stuck leg:
// the overlap sits in Wait (one ovlp_done = fade-to-black finished; IsOutReq
// polled and 0; SceneIsStart never fires; ClearOfReq never even flips). The
// handoff that should free it is fopOvlpReq_phase_WaitOfFadeout
// (f_op_overlap_req.cpp:49): decrement peektime, and when the scene-request
// machine has armed the clear (flag2==2) issue `cReq_Command(task->req, 2)`
// — THE single event that moves the overlap to FadeOut and fires
// SceneIsStart. The phases are static; the pumps and the command are not:
//   cReq_Command  — cmd==2 on the WW warp = the handoff HAPPENED (then the
//     stall is inside the overlap's FadeOut leg after all);  absent = the
//     handoff never fired, and the pump/flag side is the fault
//   fpcNdRq_Handler — the scene/node request pump, counted per side; a pump
//     that stops post-warp starves every request machine at once
// ============================================================================
DEFINE_HOOK_SYMBOL("?cReq_Command@@YAXPEAUrequest_base_class@@E@Z",
                   void(void*, unsigned char), CReqCommand);
DEFINE_HOOK_SYMBOL("?fpcNdRq_Handler@@YAHXZ", int(), NdRqHandler);
// #27c — the ARMING event. Run 161231 (uncapped cmd>=2) proved the wipe-out
// command is NEVER issued post-warp. The command's precondition is
// `flag2 == 2`, armed by exactly one call: OverlapClr's `cReq_Create(base, 2)`
// (f_op_overlap_req.cpp:181). Post-warp create=2 present = armed but the
// WaitOfFadeout pump is dead (pump-wiring fault); absent = the scene-request
// machine never reached ClearOverlap (its position is next).
DEFINE_HOOK_SYMBOL("?cReq_Create@@YAXPEAUrequest_base_class@@E@Z",
                   void(void*, unsigned char), CReqCreate);
// #28 — THE REQUEST CONSTRUCTORS. Run 162602: post-warp there is NO cmd=1 on
// even the STATIC overlap base — but fopOvlpReq_Request issues one
// UNCONDITIONALLY when creating a request. Ergo the warp's overlap request
// was NEVER CREATED. Suspect: the `l_fopScnRq_IsUsingOfOverlap` latch
// (f_op_scene_req.cpp) — cleared only in a scene-request's phase_Done; if a
// boot transition never finished, the latch holds, the warp's FadeRequest
// returns NULL, and fopScnRq_Request DELETES the whole scene request and
// returns ERROR. These two hooks watch the constructors themselves:
//   fopOvlpM_Request  — every overlap-request creation (procname, and NULL
//     return = slot busy), the event whose absence run 162602 inferred
//   fopScnM_ChangeReq — the menu's warp entry; its RETURN is the verdict
//     (fpcM_ERROR_PROCESS_ID_e = the latch/delete path ran)
// ============================================================================
DEFINE_HOOK_SYMBOL("?fopOvlpM_Request@@YAPEAVoverlap_request_class@@FG@Z",
                   void*(short, unsigned short), OvlpMRequest);
DEFINE_HOOK_SYMBOL("?fopScnM_ChangeReq@@YAHPEAVscene_class@@FFG@Z",
                   int(void*, short, short, unsigned short), ScnMChangeReq);
// #29 — THE CHANGE REQUEST'S INNARDS. Run 163224 killed the latch theory:
// post-warp ALL constructors succeed (ovlp_request returns the slot,
// scn_change_req ret=1, params identical to working boot transitions), the
// wipe task fades to black, the play scene gets created — and ClearOverlap
// still never runs. The scene-request fade machine sits at Execute, which
// forwards fpcNdRq_Execute: for a CHANGE request that phase also DELETES THE
// OLD SCENE and returns NEXT only when the whole swap completes. Two reads:
//   fpcNdRq_Execute return — INIT forever post-warp = the swap is stuck
//   fpcBs_Delete return    — 0 = a process REFUSING to die; the old NAME
//     scene refusing deletion would hold Execute at INIT indefinitely
DEFINE_HOOK_SYMBOL("?fpcNdRq_Execute@@YAHPEAUnode_create_request@@@Z",
                   int(void*), NdRqExecute);
// ============================================================
// H11 (folded into the mode-15 set) - DELETIONS ARE UNLOGGED
// ============================================================
// The receiver logs `fpcBs_Create:` and has NO delete line at all - the
// only "delete" strings in a run are this plugin's own hook_manifest
// naming the symbols. So "0 deletes after the warp" is VACUOUS, not
// evidence that pre-warp actors persist into the WW stage. This hook is
// already INSTALLED/ATTACHED, so the counter costs one receipt.
// ============================================================
DEFINE_HOOK_SYMBOL("?fpcBs_Delete@@YAHPEAUbase_process_class@@@Z",
                   int(void*), BsDelete);

// ============================================================================
// PROBE SET #30 — THE TEN-HYPOTHESIS NET (user-ordered; the standing
// multi-hypothesis rule). One build, every branch of "the swap never
// finishes" instrumented at once. With #29's two probes and the live sets,
// the next boot discriminates ALL of:
//   H1  old scene refuses deletion            -> bs_delete refusals (#29)
//   H2  swap request unpumped                 -> ndrq_exec call counter (#29)
//   H3  play scene stuck on the CREATING list -> fpcM_IsCreating returns
//       (create COMPLETED per census, but if the bookkeeping list never
//       drops it, the swap's wait-created phase spins forever)
//   H4  swap died on a silent ERROR return    -> ndrq_exec flip catches 3/5
//   H5  machine position bracket              -> ScnPause Enable/Disable:
//       Enable fires at post-create, Disable only at phase_Done — an Enable
//       with no Disable post-warp brackets the stall between them
//   H6  the WIPE TASK itself got deleted      -> fpcM_Delete victim log
//       (a dead overlap task leaves nobody to reveal; cross-ref pid census)
//   H7  the scene-manager pump stops          -> fopScnM_Management counter
//       per side (fapGm_After's first call each frame)
//   H8  ERROR propagated to the request       -> same as H4, distinct value
//   H9  control: the clear stays unarmed      -> creq_create (#27c, live)
//   H10 the menu RE-REQUESTS forever          -> fpcNdRq_ReRequest events
//       (a re-request re-arms the phase machine; a spam of them means the
//       transition restarts every frame and can never advance)
// ============================================================================
DEFINE_HOOK_SYMBOL("?fpcM_IsCreating@@YAHI@Z", int(unsigned int), MIsCreating);
DEFINE_HOOK_SYMBOL("?fopScnPause_Enable@@YAHPEAVscene_class@@@Z",
                   int(void*), ScnPauseEnable);
DEFINE_HOOK_SYMBOL("?fopScnPause_Disable@@YAHPEAVscene_class@@@Z",
                   int(void*), ScnPauseDisable);
DEFINE_HOOK_SYMBOL("?fopScnM_Management@@YAXXZ", void(), ScnMManagement);
DEFINE_HOOK_SYMBOL("?fpcNdRq_ReRequest@@YAHIFPEAX@Z",
                   int(unsigned int, short, void*), NdRqReRequest);
DEFINE_HOOK_SYMBOL("?fpcM_Delete@@YAHPEAX@Z", int(void*), MDelete);

// ============================================================================
// PROBE SET #31 — THE CREATE-QUEUE POP. The ten-net's verdict (run 164328):
// H3 CONFIRMED — fpcM_IsCreating returns 1 for pids 137 (wipe task), 138
// (play scene), 149 (room scene) though their creates returned COMPLEATE,
// while siblings 109/148 cleared normally. Completed-but-still-queued is the
// whole stall: the swap's wait-created phase polls IsCreating forever.
// The pop lives in fpcCtRq_Do: phase_handler returns COMPLEATE ->
// fpcEx_ToExecuteQ -> Delete (pop) on success, Cancel on 0. Three reads:
//   fpcCtRq_Do return + its request's pump count — an entry pumped forever
//     means the WRAPPER phase handler never returns COMPLEATE upward even
//     though the inner create did (wrapper-level wait: heap/layer/child);
//     an entry NEVER pumped means the create ITERATOR skips its layer
//   fpcEx_ToExecuteQ return — 0 = the Cancel branch (silent kill path)
//   fpcCtRq_Handler count — the pump itself, per side
// ============================================================================
DEFINE_HOOK_SYMBOL("?fpcCtRq_Do@@YAHPEAUcreate_request@@@Z",
                   int(void*), CtRqDo);
DEFINE_HOOK_SYMBOL("?fpcEx_ToExecuteQ@@YAHPEAUbase_process_class@@@Z",
                   int(void*), ExToExecuteQ);
DEFINE_HOOK_SYMBOL("?fpcCtRq_Handler@@YAHXZ", int(), CtRqHandler);
// ============================================================================
// ROUND #31 TOPPED TO THE FULL TEN (user rule: EVERY round carries 10 —
// survivors keep slots, new hypotheses fill the set until the answer).
// Survivor axis: completed procs stuck on the creating list (run 164328).
//   H1 wrapper never returns COMPLEATE upward     -> ctrq_do pump census
//   H2 iterator never pumps the entry             -> ctrq_do absence + ct_pump
//   H3 execute-queue admission refused (cancel)   -> ex_toq_refused
//   H4 pop ran but Delete REFUSED (delete_method) -> ctrq_delete ret=0
//   H5 DUPLICATE requests for one process (the pumped one popped, a twin
//      lingers; the menu's pre-warp re-request spam is the suspect twin-maker)
//                                                 -> ctrq_enqueue log
//   H6 layer creating/created message imbalance   -> ly_mesg net counter
//   H7 zombie cancel (is_cancel latched, Cancel returned FALSE mid-teardown,
//      entry never re-pumped)                     -> ctrq_cancel ret log
//   H8 entries orphaned under the DELETED old scene's layer, unreachable by
//      the walk                                   -> ly_delete victims + H2
//   H9 IsCreating FALSE-POSITIVE (freed node still linked; the id was popped
//      correctly and the flag lies)               -> composite: stuck id with
//      NO enqueue, NO pumps, yet IsCreating=1 reads as H9
//   H10 reentrant pump corrupting the walk        -> ctrq_do depth counter
// ============================================================================
DEFINE_HOOK_SYMBOL("?fpcCtRq_Delete@@YAHPEAUcreate_request@@@Z",
                   int(void*), CtRqDelete);
DEFINE_HOOK_SYMBOL("?fpcCtRq_Cancel@@YAHPEAUcreate_request@@@Z",
                   int(void*), CtRqCancel);
DEFINE_HOOK_SYMBOL("?fpcCtRq_ToCreateQ@@YAXPEAUcreate_request@@@Z",
                   void(void*), CtRqEnqueue);
DEFINE_HOOK_SYMBOL("?fpcLy_CreatingMesg@@YAXPEAUlayer_class@@@Z",
                   void(void*), LyCreatingMesg);
DEFINE_HOOK_SYMBOL("?fpcLy_CreatedMesg@@YAXPEAUlayer_class@@@Z",
                   void(void*), LyCreatedMesg);
DEFINE_HOOK_SYMBOL("?fpcLy_Delete@@YAHPEAUlayer_class@@@Z",
                   int(void*), LyDelete);
// ============================================================================
// PROBE SET #33 — COLLISION REGISTRATION RECEIPTS (rows 956/958 + the
// receiver read). d_a_bg.cpp:212-225 is the donor chain verbatim and every
// failure exit in it is SILENT. Two receipts make it speak:
//   cBgW::Set(dzb, ...)      — ret (bool: nonzero = FAILED per the call site)
//   roomControl::setBgW(room, bgw) — the registration itself, with roomNo
// On the next vanilla WW boot: no Set call = the dzb lookup returned NULL
// (see the widened room.* getRes log); Set ret=1 = parse-side refusal;
// setBgW with sane roomNo = registration ran and the fault is in the QUERY
// side (GroundCross/AABB), which the 958 cross-check made unlikely.
// ============================================================================
DEFINE_HOOK_SYMBOL("?Set@cBgW@@QEAA_NPEAUcBgD_t@@IPEAY123M@Z",
                   bool(void*, void*, unsigned int, void*), CBgWSet);
DEFINE_HOOK_SYMBOL("?setBgW@dStage_roomControl_c@@SAXHPEAVdBgW_Base@@@Z",
                   void(int, void*), RoomSetBgW);
// #33b — run 173429: dzb FOUND on R44_00, cBgW::Set SUCCEEDED on that exact
// pointer, and setBgW's zero receipts are an INLINING artifact (host warned).
// The searchable set is dBgS: daBg's later create phase calls
// `dComIfG_Bgsp().Regist(mpBgW, this)` (d_a_bg.cpp:599) and RETURNS ERROR on
// failure — which silently deletes the whole BG. dBgS has a FIXED slot table,
// so a WW sea stage can plausibly exhaust it. This receipt is the last link:
// ret nonzero on the WW side = the declining line, and the fix is sizing or
// admission policy at the receiver's own table.
DEFINE_HOOK_SYMBOL("?Regist@dBgS@@QEAA_NPEAVdBgW_Base@@PEAVfopAc_ac_c@@@Z",
                   bool(void*, void*, void*), DBgSRegist);
// #33c — the GROUND QUERY itself, the receipt that ends ambiguity: on the WW
// side, the FIRST GroundCross return that is not -INF is logged (ground
// exists), plus a bounded -INF census. Float return (M in the mangling).
DEFINE_HOOK_SYMBOL("?GroundCross@cBgS@@QEAAMPEAVcBgS_GndChk@@@Z",
                   float(void*, void*), GroundCross);
// #35 — THE WALK RECEIPT (row 997's oracle, runtime half). History proved
// offline that ALL 24 spawn columns have floor-facing room-terrain polygons
// at their record heights — the data is right and the runtime walk misses
// SOME columns. Their ranked suspect: the block/tree spatial index walking
// wrong (invisible to any "does ground exist somewhere" test). This hook
// logs every tree-node index the walk visits FOR QUERIES IN LINK'S COLUMN
// (GndChk m_pos within 60 units of the point-11 spawn), so History's offline
// oracle can compare visited-vs-expected node sets and name the divergence.
DEFINE_HOOK_SYMBOL("?GroundCrossRp@cBgW@@QEAA_NPEAVcBgS_GndChk@@H@Z",
                   bool(void*, void*, int), BgWGroundCrossRp);

// #34 — the PER-BgW query (virtual, UEAA): with registration fully green and
// the manager-level query pinned at sentinel, the last box is whether the
// room BgW's own GroundCross is (a) reached at all and (b) ever true. Bool
// return; `this` is the BgW — pointers cross-ref the cbgw_set receipts.
DEFINE_HOOK_SYMBOL("?GroundCross@cBgW@@UEAA_NPEAVcBgS_GndChk@@@Z",
                   bool(void*, void*), BgWGroundCross);
// #36 — THE TRIANGLE LEVEL, the last layer standing. Run 222652 settled the
// two above it: AABBs are SOUND (node0 = -211453.9/-4991.0/310611.8 ..
// -188296.1/-4989.0/328111.2, matching two lanes' offline derivation to the
// decimal, needs_full_transform=1 so no translate-arm confound) and the WALK
// REACHES THE FLOOR LEAF (node 115 visited twice in Link's column). Yet the
// query from his exact column still returns -INF. So the rejection happens
// INSIDE the leaf: `RwgGroundCheckGnd` walks the rwg chain from the block's
// gnd index and each candidate goes through `RwgGroundCheckCommon`. This
// receipt logs, for queries in Link's column, the poly index entering the
// chain and what the check returned — naming the triangle that should be 410
// and whether it was even considered.
DEFINE_HOOK_SYMBOL("?RwgGroundCheckGnd@cBgW@@QEAA_NGPEAVcBgS_GndChk@@@Z",
                   bool(void*, unsigned short, void*), RwgGndCheck);
// #37 — THE PLAYER'S ACTUAL GROUND PATH. Run 223203 proved the leaf-level
// chain WORKS at Link's column: poly 410 (the floor triangle) is considered
// and RwgGroundCheckGnd returns TRUE. Yet ALINK's create gate still fails —
// and the gate does not read the raw query at all: `d_a_alink.cpp:5008-5009`
// calls `mLinkAcch.CrrPos(dComIfG_Bgsp())` then tests
// `mLinkAcch.GetGroundH() == -G_CM3D_F_INF`. That is the dBgS_Acch machinery,
// which builds its OWN GndChk (radius, wall/spline flags, actor exclusion)
// around the query I have been watching. This receipt measures the value the
// gate ACTUALLY reads, one call per actor per frame — and it replaces three
// per-triangle/per-node probes, so it is cheaper than what it supersedes.
DEFINE_HOOK_SYMBOL("?CrrPos@dBgS_Acch@@QEAAXAEAVdBgS@@@Z",
                   void(void*, void*), AcchCrrPos);

typedef void* (*FnJ3DParse)(const void*, unsigned int);
FnJ3DParse s_fnJ3DLoadBDL = nullptr;   // loadBinaryDisplayList — bdl4 path
// ============================================================
// ANIMATION LOADER - the choke's missing arm (History/Bridge
// surfaced it on the Salvage port; registry.cpp is this lane's).
// TWO-ARG BY NECESSITY: the receiver's convenience overload
// `load(p) { return load(p, J3DLOADER_UNK_FLAG0); }`
// (J3DAnmLoader.h:44) is an inline - a symbol resolved by name has
// NO default argument to inherit, so the flag is passed EXPLICITLY.
// J3DLOADER_UNK_FLAG0 = 0, read from the enum, not assumed.
// ============================================================
typedef void* (*FnAnmLoad)(const void*, int);
FnAnmLoad s_fnJ3DAnmLoad = nullptr;
FnJ3DParse s_fnJ3DLoadBMD = nullptr;   // load                  — bmd3 path

// ============================================================================
// THE DZB CONVERSION ARM — the collision root, run 174047 + source, and it is
// the BMDL GAP'S TWIN. `dRes_info_c::loadResource` (d_resorce.cpp:512-515)
// byte-swaps `DZB `-typed nodes at arc load via the receiver's own
// `cBgS::ConvDzb` — and that walk never reaches plugin-served WW arcs (the
// same proven gap that made model.bdl arrive raw). MEASURED CHAIN: R44
// room.dzb FOUND · cBgW::Set ret=0 · dBgS::Regist ret=0 (ADMITTED) · ALINK
// still -INF — because Set/Regist read the HEADER through BE() wrappers
// while the QUERY math (SetVtx aliases `pm_vtx_tbl = pm_bgd->m_v_tbl`, row
// 953) reads raw vertices natively: big-endian garbage, AABB nowhere near
// Link, every GroundCross rejects.
//
// FIX = the DN-3-sanctioned consume-time shape ALREADY SHIPPED for bdl4/bmd3:
// when a WW-stage lookup returns a `.dzb` resource, hand it to the RECEIVER'S
// OWN ConvDzb. Single-conversion is enforced by the receiver itself —
// ConvDzb checks its own 0x80000000 converted flag (idempotent by design),
// so a double call cannot re-swap. Fork boots defer (§836): the host's own
// serve/convert layer owns it there.
// ============================================================================
typedef void* (*FnConvDzb)(void*);
FnConvDzb s_fnConvDzb = nullptr;

// ============================================================================
// HOST-CAPABILITY DETECTOR (CALLS §836, user-mandated before ANY fork boot).
// The plugin never probed whether the image it loaded into ALREADY PROVIDES
// what it provides. On a FORK boot both provision paths run: the fork's
// in-tree l_objectName WW rows AND our name hook; the fork's in-tree stage
// declarations AND our stage machinery; and — the sharpest instance — the
// fork's dExtNpcMount consume-time BDL parser AND our on_getRes consume-time
// parser over the SAME buffers, which is DN-3's forbidden double-parse with a
// recorded memory-corruption symptom.
//
// DETECTION: six stage-hosting symbols, gate-verified 6/6 RESOLVED on the
// fork image and 6/6 MISSING on vanilla (2026-08-16). One resolve pass at
// install (top of wwRegistry_initialize); ANY hit => fork-hosted. The failure
// direction is chosen deliberately: misreading vanilla as fork makes the
// plugin DEFER (WW merely stops working); misreading fork as vanilla
// double-provisions (corruption). Defer is the safe wrong answer, so the
// threshold is >= 1, not == 6.
//
// DEFERS WIRED (each logs when it declines): the on_getRes consume parse,
// the loadResource BMDL arm, the l_objectName name hook, the mult-info stage
// declaration. Probes/observers stay live on both hosts — reading collides
// with nothing. Declared HERE, above the first deferring handler, because
// C2065 is the compiler's opinion of a ledger entry below its first reader.
// ============================================================================
bool s_hostIsFork = false;
// Diagnostics master switch - see wwRegistry_setDiagProbes. Default OFF so a
// normal boot pays nothing for the investigation's instrumentation.
bool s_diagProbes = false;
int s_hostCaps = 0;
int s_deferNotes = 0;   // one-time defer announcements, bounded
void noteDefer(const char* which);   // body below logf — the CRT's math logf
                                     // shadows ours above line 1402 (C2660)

// Receipt counters, reported on the bindings line like every other seam here.
int s_bmdlSeen = 0;      // slots still raw after the receiver's own dispatch
int s_bmdlFixed = 0;     // slots parsed and republished by us
int s_bmdlFailed = 0;    // parser ran and returned NULL — a FORMAT answer

// ---------------------------------------------------------------------------
// THE RESOURCE-LOOKUP SEAM — `dRes_control_c::getRes(arc, resName, info, num)`.
//
// §1021 asks what archive/resource the meter requests and whether it resolved.
// The obvious targets are BOTH INLINE and would repeat the mistake that killed
// the getStartStageName seam: `dComIfG_getObjectRes` (`d_com_inf_game.h:4601`)
// forwards to `dRes_control_c::getObjectRes` (`d_resorce.h:129`), which is
// ALSO inline, and that one finally forwards to `getRes` — out-of-line at
// `d_resorce.cpp:993`. **That is the choke point every by-name lookup passes
// through, and the only one of the three that a by-name hook can reach.**
//
// PURE OBSERVER, POST-SHAPED: it reports the ANSWER, not just the question. A
// log of what was asked cannot distinguish "asked and got it" from "asked and
// got NULL", and NULL is the whole hypothesis here — a TP consumer reading a
// WW archive and finding nothing.
//
// EXPECT AMBIGUITY ON THE BARE NAME AND READ IT AS INFORMATION: `getRes` has
// TWO overloads (`:975` by index, `:993` by name), so the bare symbol may come
// back MOD_CONFLICT exactly as `dComIfGp_setNextStage` did. The install result
// is reported, so that outcome names itself at boot and the fix is the mangled
// form — rather than a silent zero, which is what `setStageRes` cost us.
// ---------------------------------------------------------------------------
// ============================================================================
// MANGLED + SIGNATURE CORRECTED BY THE INTEGRATOR AT THE GATE, 2026-08-16.
// Housing predicted the ambiguity in the comment above and was right; the
// image confirms it AND turns up a second, worse problem.
//
// (1) BARE NAME IS UNBINDABLE. `--find` on the USER'S image:
//     `RESOLVED* getRes rva=0x1d78a0 [CODE|MULTI_NAME] as dRes_info_c::getRes (+4 more)`
//     — the bare name resolves to a DIFFERENT CLASS entirely.
//
// (2) IT IS `static`, SO THERE IS NO `this` — and the old declaration had one.
//     `d_resorce.h:87`: `static void* getRes(char const* i_arcName, char const*
//     i_resName, dRes_info_c* i_resInfo, int i_infoNum);` and the mangling says
//     the same: `SA` = static, params `PEBD 0 PEAVdRes_info_c@@ H`.
//     **FOUR parameters, not five.**
//
// WHAT THE OLD DECLARATION WOULD HAVE DONE, which is why this is not pedantry:
// every argument shifts by one, so `arg(2)` — read and printed as `%s` — would
// actually have been the `dRes_info_c*`. **The probe would have formatted a
// struct pointer as a string and crashed inside the instrument**, producing a
// crash that looks like the bug it was built to find. Arg indices below are
// corrected to 0/1 to match.
// ============================================================================
DEFINE_HOOK_SYMBOL("?getRes@dRes_control_c@@SAPEAXPEBD0PEAVdRes_info_c@@H@Z",
                   void*(const char*, const char*, void*, int), GetRes);

// ============================================================================
// THE CRASHING CALL — `JKRArchive::getGlbResource`, PRE-hook (§1022).
//
// THE getRes PROBE ABOVE IS RETIRED FROM THE INSTALL LIST BY THIS ROW, AND THE
// REASON IS A DESIGN FAULT OF MINE RATHER THAN BAD LUCK — two independent
// reasons, either alone fatal:
//   (1) IT IS A POST HOOK, AND A POST HOOK CANNOT OBSERVE A CALL THAT DOES NOT
//       RETURN. The crash happens INSIDE the lookup, so the callback never
//       runs and never logs. I chose post deliberately — "the answer is the
//       point" — which is right for counting misses and wrong for catching a
//       fatal call.
//   (2) IT WAS THE WRONG DOOR. The crash is in `JKRArchive::getGlbResource`,
//       which `JKRGetTypeResource` (`JKRArchive.h:256`) forwards straight to.
//       `dRes_control_c::getRes` is a different path.
//
// BOTH BINDING TRAPS ARE LIVE ON THIS SYMBOL AND THE INTEGRATOR VERIFIED BOTH
// ON THE USER'S IMAGE, so the mangled string is used VERBATIM, not re-derived:
//   · bare `getGlbResource` is `[CODE|MULTI_NAME] (+4 more)` — `JKRFileLoader`
//     has one too. That is the `setStageRes` trap.
//   · the mangling reads `SA` = STATIC, so there is NO `this` and the
//     signature is THREE parameters. That is the `getRes` phantom-`this` trap,
//     which in that probe would have printed a struct pointer as `%s` and
//     crashed inside the instrument.
//
// AND IT LOGS THE ARC POINTER VALUE, not just the names: crash PC == fault
// address (`0xffffffea60000000`) is a JUMP through a bad pointer rather than a
// bad read, so the pointer itself is the evidence and the last line written
// before the log ends is the call that made it.
// ============================================================================
DEFINE_HOOK_SYMBOL("?getGlbResource@JKRArchive@@SAPEAXIPEBDPEAV1@@Z",
                   void*(unsigned int, const char*, void*), GlbResource);

// ============================================================================
// THE MESSAGE-ARCHIVE SLOTS — INTEGRATOR 2026-08-16. Two instruments, because
// the last three theories each died to a control I had not run.
//
// WHY THIS PAIR AND NOT MORE GUESSING: `glb_res` logs the `arc` ARGUMENT, and
// slot 0 and slot 1 get asked for the SAME STRING ("zel_00.bmg", because
// `bmg_filename[0]` is that name and `d_meter2_draw.cpp:99` indexes it by
// groupID). So two identical-looking lines carry two different archives and
// the log cannot tell them apart. **I read that as one slot changing, and it
// was not.** These two make the slot self-identifying instead of inferred.
//
// READER: `?dComIfGp_getMsgDtArchive@@YAPEAVJKRArchive@@H@Z` — the FREE
// function form, `YA` (not a member, no `this`), `JKRArchive*(int)`. Called,
// never hooked, so inlining elsewhere is irrelevant — the `getStartStageName`
// lesson applied deliberately rather than rediscovered.
//
// WRITER: `?dComIfGp_setMsgDtArchive@@YAXHPEAVJKRArchive@@@Z`, `void(int,
// JKRArchive*)`. This is the one that matters: `d_s_play.cpp:1419-1422` fills
// slot 1 from a command's archive and calls `destroy()` ON THE NEXT LINE,
// every stage load. History's decode shows `TResource`/`TControl` cache six
// raw pointers INTO that buffer across frames without revalidating, so
// set-then-destroy is the arrangement that makes those dangle. **Hooking the
// write records what slot 1 becomes, and when, instead of inferring it.**
//
// Both bare names are MULTI_NAME on the user's image (`dComIfG_play_c::`
// members share them), so both are bound mangled. Checked with `--find`, not
// assumed.
// ============================================================================
DEFINE_HOOK_SYMBOL("?dComIfGp_setMsgDtArchive@@YAXHPEAVJKRArchive@@@Z",
                   void(int, void*), SetMsgDtArc);
typedef void* (*FnGetMsgDtArc)(int);
FnGetMsgDtArc s_fnGetMsgDtArc = nullptr;
// The SETTER, as a CALLABLE. `dComIfGp_setMsgDtArchive` is `inline` in the
// receiver header, so its call sites inline it and the hook we installed never
// fires — but the out-of-line copy still EXISTS and calling it works. Same
// call-don't-hook pattern as `getStartStageName`, applied deliberately.
typedef void (*FnSetMsgDtArc)(int, void*);
FnSetMsgDtArc s_fnSetMsgDtArc = nullptr;
// Owned-mount path (slot-1 ownership contract), MEM-MOUNT SHAPE — the
// Integrator's run-170618 finding retired the DVD-path shape (the plugin
// serves donor paths at the ARC layer; DVDConvertPathToEntrynum refuses, so
// the preflighted mDoDvdThd command never fired anywhere). The vehicle now is
// the receiver's exported `JKRArchive::mount(void* ptr, JKRHeap*, dir)`
// factory over a PLUGIN-RETAINED copy of the bmgres.arc bytes:
//   · receiver-side allocation (no plugin sizeof-of-receiver-class trap),
//   · dedupe BY BUFFER POINTER (JKRArchivePub check_mount_already) — every
//     scene mounts the same retained buffer, refcounts balance per scene,
//   · constructed with break-flag 0 → the dtor NEVER frees our buffer,
//   · removeResourceAll is a NO-OP on MOUNT_MEM (JKRMemArchive.cpp:178) —
//     the scene teardown's dangerous half is inert by construction.
typedef void* (*FnJkrMountMem)(void*, void*, int);
FnJkrMountMem s_fnJkrMountMem = nullptr;
bool s_msgMountResolveTried = false;
// Session-retained bmgres.arc bytes, handed over by main.cpp's disc read.
const unsigned char* s_bmgArcBuf = nullptr;
uint32_t s_bmgArcSize = 0;

DEFINE_HOOK_SYMBOL("dStage_dt_c_stageLoader", void*(void*, void*), StageLoader);
DEFINE_HOOK_SYMBOL("?dStage_dt_c_roomLoader@@YAXPEAXPEAVdStage_dt_c@@H@Z",
                   void(void*, void*, int), RoomLoader);
DEFINE_HOOK_SYMBOL("?dKy_setLight@@YAXXZ", void(), KySetLight);
DEFINE_HOOK_SYMBOL(
    "?setLightTevColorType_MAJI@dScnKy_env_light_c@@QEAAXPEAVJ3DModelData@@PEAVdKy_tevstr_c@@@Z",
    void(void*, void*, void*), KyMajiModelData);
DEFINE_HOOK_SYMBOL(
    "?settingTevStruct@dScnKy_env_light_c@@QEAAXHPEAVcXyz@@PEAVdKy_tevstr_c@@@Z",
    void(void*, int, void*, void*), KySettingTev);
DEFINE_HOOK_SYMBOL("?dStage_Create@@YAXXZ", void(), StageCreate);

// AND THE SITE ONE STEP UPSTREAM — §1017's target, which is the better of the
// two and is taken as well rather than instead. `dRes_control_c::setStageRes`
// is where `/res/Stage/%s/` is actually built (`d_resorce.cpp:1109-1113`), so
// it answers PATH REQUESTED directly, while `stageLoader` answers DATA PARSED.
// Taking BOTH in one build brackets the chain: fires/fires = the path went out
// and the parse ran; fires/silent = requested and never parsed; silent/silent
// = the load phase never ran at all and the fault is scene phase order, not
// the name. **Four boots have each answered one bit; this one answers two.**
// `this` is the first parameter on x64 — read as void*, never dereferenced.
// ============================================================================
// MANGLED, NOT BARE — INTEGRATOR GATE FIX 2026-08-15. The bare name DID NOT
// RESOLVE and the hook silently never installed: run 032202 logged
// `hook target 'setStageRes' did not resolve (symbol 'setStageRes' not found)`
// and `setStageRes_hook: MOD_UNAVAILABLE`, so `set_stage_res_calls: 0` meant
// "never hooked", not "never called". A dead probe reporting a zero is worse
// than no probe: it reads as evidence.
//
// AND MY GATE PASSED IT, WHICH IS THE PART WORTH RECORDING. I ran
// `upstream_conformance --symbol setStageRes` and got SAFE on BOTH images, so
// I approved a bare binding. The manifest tells the truth on a closer look:
//     RESOLVED* setStageRes  rva=0x1131f0 [CODE|MULTI_NAME]  (+2 more)
// FOUR symbols carry that substring — the member AND a free-function wrapper:
//     ?setStageRes@dRes_control_c@@QEAAHPEBDPEAVJKRHeap@@@Z   <- this one
//     ?dComIfG_setStageRes@@YAHPEBDPEAVJKRHeap@@@Z            <- wrapper
// So `--symbol SAFE` can report SAFE for a name the RUNTIME RESOLVER refuses.
// That is a THIRD face of the §480 rule: SAFE proves the manifest resolves the
// name, not that the call sites reach it (inlining), and not that the loader
// can bind it (MULTI_NAME). **Check `--find` on the image, not just `--symbol`,
// before approving any bare member-function binding.**
//
// The SIGNATURE was already correct and is unchanged: `QEAA H PEBD
// PEAVJKRHeap@@` is `int(const char*, JKRHeap*)` plus the x64 `this`, which is
// exactly `int(void*, const char*, void*)`. Only the name was wrong.
// ============================================================================
DEFINE_HOOK_SYMBOL("?setStageRes@dRes_control_c@@QEAAHPEBDPEAVJKRHeap@@@Z",
                   int(void*, const char*, void*), SetStageRes);

// ---------------------------------------------------------------------------
// RETIRED — b2's AUTOFIRE TRIGGER HOOKS (`fopScnM_ChangeReq`, `fpcBs_Create`).
// USER RULING 2026-08-15 (§1009): *"it should not auto warp me: I should be
// able to select it via the warp menu/enter through Fado's door like in our
// Fork."* The hook surface goes back to the three of the §747 chain.
//
// WHAT THEY WERE AND WHY THEY ARE NOT COMING BACK: both bound and worked —
// `b2_bind` reported `ChangeReq_hook: MOD_OK, BsCreate_hook: MOD_OK` by BARE
// name on run 012112, which settled the open question that SAFE-means-one-entry
// makes mangling unnecessary for these two. They are removed because AUTOFIRE
// itself is retired, not because they failed.
//
// AND THE MEASUREMENT THEY PRODUCED IS THE ARGUMENT FOR THE RULING, so it is
// kept here rather than lost with the code: `changereq_calls: 114` in one
// session. A stage-change boundary that fires 114 times is a poor proxy for
// "the player arrived somewhere", so ANY autofire built on it is approximate.
// Selection is exact. A player should not be teleported by a counter reaching
// a number.
//
// The TRANSPORT survives this retirement: `?dComIfGp_setNextStage@@YAXPEBDFCC@Z`
// stays bound below, because the warp-menu and Fado-door paths need exactly
// that call — with the layer argument it already carries.
// ---------------------------------------------------------------------------

namespace {

// ============================================================================
// THE PLUGIN'S OWN INDEX RANGE.
//
// Must sit above the host's `fpcNm_MAX_NUM` (829 as measured at §954) and stay
// inside s16, since every consumer in the chain takes `s16 i_procName`. 4096
// leaves ~5x headroom over today's value and ~28k usable indices below the
// s16 ceiling. Anything BELOW this base is the receiver's and falls through.
// ============================================================================
const short kWwProfileBase = 4096;

inline bool isWwIndex(short i_procName) {
    return i_procName >= kWwProfileBase;
}

// ============================================================================
// THE REGISTRY — deliberately EMPTY.
//
// This mod ships the MECHANISM, not content. With no rows, `fpcPf_Get` always
// falls through and the profile hook is a no-op, so loading this mod changes
// nothing observable. Each WW actor adds one row when it is ported mod-side.
//
// HAZARD FOR WHOEVER ADDS THE FIRST ROW — READ BEFORE FILLING THIS IN:
// `fpcPf_Get` returns a `process_profile_definition*`, and this plugin has NO
// receiver headers by design (that is what makes it version-independent). A
// row must therefore supply a LAYOUT-COMPATIBLE struct authored here, and a
// layout mismatch is a silent field-misread, not a compile error. Do not
// eyeball it: derive the layout from the receiver and state the derivation in
// the CALLS row that adds it.
// ALSO: whatever is returned must OUTLIVE THE CALL — plugin-owned static or
// table storage, never a stack temporary.
// ============================================================================
// ============================================================================
// THE PROFILE LAYOUT — derived, and the DECOMP OFFSETS ARE A TRAP.
//
// `include/f_pc/f_pc_profile.h:15` annotates the struct with /* 0x00 */,
// /* 0x0C */, /* 0x10 */ … — those are GAMECUBE 32-BIT offsets, where
// `methods` is a 4-byte pointer. On our x64 target the pointer is 8 bytes and
// EVERY offset from `methods` onward moves. Copying the commented offsets
// would misread process_size, unk_size and parameters silently — a wrong heap
// size and a wrong parameter word, with no crash at the boundary.
//
// THE SAFE DERIVATION is not arithmetic: declare the SAME FIELDS in the SAME
// ORDER with the same types and let the compiler lay them out. The plugin and
// the receiver are built by the same compiler for the same ABI, so a
// field-for-field declaration is layout-identical BY CONSTRUCTION rather than
// by my counting. The static_asserts below then LOCK that reasoning — if the
// ABI or the receiver struct ever changes, this fails the BUILD instead of
// misreading a field at runtime. That is the whole point of asserting a layout
// you believe is already correct.
//
// Receiver sources this mirrors (do NOT include them — zero receiver headers
// is what makes this plugin version-independent):
//   process_profile_definition   f_pc_profile.h:15
//   process_method_class         f_pc_method.h:8   (4 method pointers)
//   leaf_process_profile_definition f_pc_leaf.h:32 (base + sub_method + priority)
// ============================================================================
struct WwMethodClass {
    void* create_method;
    void* delete_method;
    void* execute_method;
    void* is_delete_method;
};
static_assert(sizeof(WwMethodClass) == 32, "process_method_class: 4 pointers on x64");

struct WwProfileDef {
    unsigned int layer_id;
    unsigned short list_id;
    unsigned short list_priority;
    short name;
    const WwMethodClass* methods;
    unsigned int process_size;
    unsigned int unk_size;
    unsigned int parameters;
};
static_assert(offsetof(WwProfileDef, layer_id) == 0, "layer_id");
static_assert(offsetof(WwProfileDef, list_id) == 4, "list_id");
static_assert(offsetof(WwProfileDef, list_priority) == 6, "list_priority");
static_assert(offsetof(WwProfileDef, name) == 8, "name");
static_assert(offsetof(WwProfileDef, methods) == 16, "methods — 16 on x64, NOT the 0x0C in the header");
static_assert(offsetof(WwProfileDef, process_size) == 24, "process_size");
static_assert(offsetof(WwProfileDef, unk_size) == 28, "unk_size");
static_assert(offsetof(WwProfileDef, parameters) == 32, "parameters");
static_assert(sizeof(WwProfileDef) == 40, "process_profile_definition on x64");

// Actors are LEAF processes: the profile the game reads is this, and
// fpcPf_Get returns a pointer to its `base`. Returning a bare WwProfileDef for
// an actor would leave the draw sub-method and priority off the end of the
// allocation — read as garbage the first time the actor draws.
struct WwLeafProfileDef {
    WwProfileDef base;
    const void* sub_method;
    short priority;
};
static_assert(offsetof(WwLeafProfileDef, base) == 0, "leaf base");
static_assert(offsetof(WwLeafProfileDef, sub_method) == 40, "leaf sub_method");
static_assert(offsetof(WwLeafProfileDef, priority) == 48, "leaf priority");
static_assert(sizeof(WwLeafProfileDef) == 56, "leaf_process_profile_definition on x64");

// ============================================================================
// ACTORS ARE A FOURTH TIER, not the leaf one — checked against the real pilot.
//
// The donor's `g_profile_TAG_SO` (WW d_a_tag_so.cpp:100) is an
// `actor_process_profile_definition`, whose receiver shape is
// `include/f_op/f_op_actor.h:16`:
//     leaf_process_profile_definition base;  actor_method_class* sub_method;
//     u32 status;  u8 group;  u8 cullType;
// So the chain is profile -> leaf -> actor, and `fpcPf_Get` hands back a
// pointer to the OUTERMOST object's `base.base`. Returning anything shorter
// leaves the actor fields off the end of the allocation.
//
// AND THE PROFILE IS NOT SELF-CONTAINED — this is the part that decides
// whether a plugin can host an actor at all. The donor profile stores
// `&g_fpcLf_Method.base` and `&g_fopAc_Method.base`: addresses of RECEIVER
// GLOBALS. A plugin cannot author those; it must RESOLVE them from the host
// at runtime. Both are resolvable as DATA symbols (verified, see
// s_leafMethod/s_actorMethod below), which is what makes the pilot possible.
// ============================================================================
struct WwActorProfileDef {
    WwLeafProfileDef base;
    const void* sub_method;  // actor_method_class* — the actor's own vtable-ish
    unsigned int status;
    unsigned char group;
    unsigned char cullType;
};
static_assert(offsetof(WwActorProfileDef, base) == 0, "actor base");
static_assert(offsetof(WwActorProfileDef, sub_method) == 56, "actor sub_method");
static_assert(offsetof(WwActorProfileDef, status) == 64, "actor status");
static_assert(offsetof(WwActorProfileDef, group) == 68, "actor group");
static_assert(offsetof(WwActorProfileDef, cullType) == 69, "actor cullType");
// THE LAST UNGUARDED DERIVED NUMBER IN THIS FILE, NOW CLOSED. Until Foundry
// measured it this was the only mirror here with offsetof asserts and NO size
// assert — I had derived 72 by counting (cullType at 69, padded to align 8)
// and nobody had checked it against the receiver. Foundry measured
// `sizeof(actor_process_profile_definition)` = 0x48 = 72, compiler-verified
// with a failing control, and it matches. Deriving it correctly was luck as
// much as arithmetic: the same reasoning applied to `actor_method_class` was
// 16 bytes short, and that one shipped as a latent overread until someone
// else's assert battery caught it. A derived number that happens to be right
// is still unguarded until it is asserted.
static_assert(sizeof(WwActorProfileDef) == 72,
              "actor_process_profile_definition == 0x48 on x64 (Foundry, measured)");

// Receiver method tables the profile must POINT AT. Resolved by name at
// init — mangled forms taken from the manifest, both DATA symbols:
//   ?g_fpcLf_Method@@3Uleafdraw_method_class@@B   (leaf draw methods)
//   ?g_fopAc_Method@@3Uactor_method_class@@A      (actor methods)
// A NULL here means the host did not yield them and NO actor row may be
// registered — which is why they are resolved and reported before any row is
// used, rather than assumed at table-authoring time.
const void* s_leafMethod = nullptr;
const void* s_actorMethod = nullptr;

struct WwProfileRow {
    short index;
    const void* profile;  // outermost profile's &base.base
    // Is `profile` a real leaf-derived profile whose fields may be read? The
    // self-test sentinel is a bare int compared by ADDRESS and never
    // dereferenced, so the priority validation below must skip it. Without
    // this flag that check would itself become the wild read it exists to
    // prevent.
    bool is_profile;
};

// ============================================================================
// SELF-TEST SENTINEL — closes the "dispatch unproven" gap named by run 183312.
//
// That run proved the hooks INSTALL on stock dusklight, but `pf_hits:0` meant
// the ANSWER path was never exercised: nothing in the game asks for an index
// in our range until an actor is placed, so dispatch stayed untested.
//
// This row fixes that without needing an actor OR a correct profile layout.
// It registers ONE index with a sentinel pointer, and mod_initialize calls the
// hooked function with that index and checks the returned pointer is the
// sentinel. Identity is all we check — nothing dereferences it — so the
// process_profile_definition layout question stays deferred, which is exactly
// where it belongs until a real actor lands.
//
// WHY THIS IS SAFE TO CALL AT INIT: the index is ours, so our pre-hook answers
// and returns HOOK_SKIP_ORIGINAL before the original runs. No game state is
// touched and `fpcPf_Get`'s own body is never entered.
//
// WHY IT CANNOT COLLIDE WITH ANYTHING: the game never requests this index —
// that is the whole premise of above-enum allocation — so the row is
// unreachable except from the self-test below.
// ============================================================================
const short kSelfTestIndex = kWwProfileBase;  // 4096
const int kSelfTestSentinel = 0x5754;         // "WT" — never dereferenced

// ============================================================================
// THE PILOT ACTOR — `tag_so`. IT BEGAN AS A LINKAGE PROBE AND BECAME A REAL
// PORT AT RIDER 5; the banner records the history because either claim alone
// is misleading.
//
// WHAT IT WAS: a bare probe borrowing tag_so's shape (the smallest actor
// surface — no model, no collision, no message flow, no particles) and
// implementing NONE of its behaviour, to answer one question no static
// measurement can: does an above-enum index actually spawn a live process
// through the receiver's own create path? It was labelled "NOT a tag_so port"
// for as long as that was true, because calling it one would have been a
// substitution dressed as a port.
//
// WHAT IT IS NOW: the donor's actual body. `WwTagSo_ct` reproduces the
// `fopAcM_ct` expansion including both OSPanic guards; `WwTagSo_getArg`
// reproduces the param fetch, the three bit extractions and the 0xff radius
// rule; the profile carries the donor's `status` and `cullType`. The gaps are
// declared, not silent — `debugDraw` (unreachable in a shipping build) and
// `fpcDwPi_TAG_SO_e` (a receiver-owned bucket index that cannot transfer,
// tale §995).
//
// THE LABEL WAS STALE FOR ONE RIDER AND FOUNDRY CAUGHT IT (CALLS row 557).
// It under-claimed, which is the rarer and more confusing direction: an
// auditor asking "has a real WW actor landed?" would read the old banner and
// conclude no. A stale honesty label is worse than none, because it is
// trusted — and this profile-fill is the path every future WW actor copies,
// so a copied false disclaimer would propagate.
//
// PROCESS SIZE IS EXACT, NOT BOUNDED. `sizeof(fopAc_ac_c)` = 0x668 (1640) on
// this target — MEASURED by making the compiler print it, after the header's
// `STATIC_ASSERT(... == 0x568)` turned out to be compiled out on MSVC
// (global.h:62-73 makes it an empty macro off __MWERKS__/GCN_USA). The header
// figure is short by 0x100; a profile built on it would under-allocate EVERY
// actor by 256 bytes — silent at the boundary, fatal on first member write.
//
// `process_size` is consumed at exactly two sites, both in fpcBs_Create
// (f_pc_base.cpp:150-158): memalignB and sBs_ClearArea. Allocation only —
// which is why an exact figure is a bonus rather than a requirement, but there
// is no reason to carry slack now that the real number is known.
// ============================================================================
const unsigned int kActorBaseSize = 0x668;  // measured sizeof(fopAc_ac_c), x64

struct WwPilotMembers {
    unsigned char state;
    unsigned char pad[7];
};
static_assert(sizeof(WwPilotMembers) == 8, "pilot member block");

// ============================================================================
// TAG_SO — the donor's behaviour, ported. Donor: WW `d_a_tag_so.cpp`.
//
// EVERY CONSTANT HERE IS MEASURED, NOT READ OFF A HEADER COMMENT:
//   sizeof(fopAc_ac_c)                  = 0x668 (1640)
//   offsetof(fopAc_ac_c, layer_tag)     = 40   (0x28)
//   offsetof(fopAc_ac_c, layer_tag.layer) = 80 (0x50)  <- absolute
//   fopAcCnd_INIT_e                     = 0x08 (f_op_actor.h:77)
// The first three came from the compiler via the ShowSize/ShowA probe; the
// GC-era `// Size: 0x568` in the header is short by 0x100 and its guarding
// STATIC_ASSERT is an empty macro on MSVC.
//
// FOUR BINDINGS, EACH WITH TWO VERDICTS (name-SAFE on both images AND
// signature-compared donor-to-receiver): fopAcM_GetParam · fopAcM_CheckCondition
// · fopAcM_OnCondition · OSPanic.
//
// AND ONE DELIBERATE NON-BINDING: `fopAcM_GetParamBit` resolves SAFE but the
// donor's takes a u32 VALUE while the receiver's takes an ACTOR POINTER it
// dereferences. Binding it would pass a small integer as a pointer — compiles
// clean, wild-reads at runtime. The donor's version is pure arithmetic, so it
// is reproduced inline below and nothing is bound.
// ============================================================================
typedef unsigned int (*FnGetParam)(const void*);
typedef int (*FnCondition)(void*, unsigned int);
typedef void (*FnOSPanic)(const char*, int, const char*, ...);

FnGetParam  s_fnGetParam = nullptr;
FnCondition s_fnCheckCondition = nullptr;
FnCondition s_fnOnCondition = nullptr;
FnOSPanic   s_fnOSPanic = nullptr;

const unsigned int kFopAcCnd_INIT = 0x08;
const unsigned int kOffLayerTagLayer = 80;  // measured, absolute from actor base

// Donor `fopAcM_GetParamBit` (WW f_op_actor_mng.h:150) — VALUE in, not actor.
inline unsigned int wwGetParamBit(unsigned int param, unsigned char shift,
                                  unsigned char bit) {
    return (param >> shift) & ((1u << bit) - 1u);
}

// Members sit immediately after the actor base. Donor offsets 0x290/0x294/0x298
// are relative to the DONOR's smaller base; what is faithful is the ORDER and
// the types, not the donor's absolute numbers.
struct TagSoMembers {
    unsigned char m290;
    unsigned char pad1[3];
    float mRadius;
    unsigned char m298;
    unsigned char pad2[7];
};
static_assert(sizeof(TagSoMembers) == 16, "tag_so member block");

inline TagSoMembers* tagSoMembers(void* self) {
    return reinterpret_cast<TagSoMembers*>(static_cast<char*>(self) + kActorBaseSize);
}

// Donor `getArg` (d_a_tag_so.cpp:39). Faithful: the donor fetches the param
// ONCE and then extracts bits from the VALUE — which is why the receiver's
// pointer-taking GetParamBit must not be used here.
void WwTagSo_getArg(void* self) {
    if (s_fnGetParam == nullptr) {
        return;
    }
    const unsigned int param = s_fnGetParam(self);
    TagSoMembers* m = tagSoMembers(self);
    m->m290 = (unsigned char)wwGetParamBit(param, 0, 8);
    const unsigned int paramRadius = wwGetParamBit(param, 8, 8);
    m->m298 = (unsigned char)wwGetParamBit(param, 16, 8);
    // Donor: 0xff means "default", else radius = value * 100.
    m->mRadius = (paramRadius == 0xff) ? 1600.0f : (float)(paramRadius * 100);
}

// Donor `fopAcM_ct` expansion (receiver f_op_actor_mng.h:25-31), reproduced
// rather than omitted — the two OSPanic guards are the donor's own and dropping
// them would be a substitution.
void WwTagSo_ct(void* self) {
    void** layerp = reinterpret_cast<void**>(
        static_cast<char*>(self) + kOffLayerTagLayer);
    if (*layerp == nullptr && s_fnOSPanic != nullptr) {
        s_fnOSPanic(__FILE__, __LINE__, "UH OH");
    }
    if (s_fnCheckCondition != nullptr && !s_fnCheckCondition(self, kFopAcCnd_INIT)) {
        // fopAcM_ct_placement: zero the member block. The receiver already
        // cleared the whole allocation in fpcBs_Create (sBs_ClearArea), so this
        // is the donor's placement-new reduced to what it actually does for a
        // POD member block — no vtable, no base re-init.
        std::memset(tagSoMembers(self), 0, sizeof(TagSoMembers));
        if (s_fnOnCondition != nullptr) {
            s_fnOnCondition(self, kFopAcCnd_INIT);
        }
    }
    if (*layerp == nullptr && s_fnOSPanic != nullptr) {
        s_fnOSPanic(__FILE__, __LINE__, "Oh come on");
    }
}

// The five methods, donor-faithful. `_execute`, `_draw`, `_delete` and
// `_isDelete` return TRUE in the donor; `_draw`'s debugDraw branch is gated on
// a HIO flag that defaults to 0, so it never runs and is not ported.
//
// ============================================================================
// CREATE RETURNS cPhs_COMPLEATE_e (4) — THE `return 0` HERE WAS FOUR OF THE
// SEVEN PINS HOLDING THE BLACK SCREEN SHUT (round #31, run 165541).
// 0 is cPhs_INIT_e = "still creating, pump me again forever": every pilot
// actor looped on the creating list eternally, its layer's creating-count
// never drained (`ly_mesg_net` +18), and `fpcSCtRq_phase_IsComplete` holds a
// SCENE on the creating list until its layer's count is zero — so the play
// scene could never finish creating, the swap never completed, the wipe's
// reveal was never armed. DONOR-VERIFIED: `daTag_So_c::_create()`
// (d_a_tag_so.cpp:56-59) returns cPhs_COMPLEATE_e. The remaining pins
// (ALINK + two unnamed) are receiver-side and separately owned.
// ============================================================================
int WwPilot_create(void* self)  { WwTagSo_ct(self); WwTagSo_getArg(self); return 4; }
int WwPilot_delete(void*)   { return 1; }
int WwPilot_execute(void*)  { return 1; }
int WwPilot_isDelete(void*) { return 1; }
int WwPilot_draw(void*)     { return 1; }

// ============================================================================
// actor_method_class — CORRECTED. My first version was 16 bytes SHORT.
//
// I wrote it as five pointers (40 bytes) from the donor's initialiser list
// (`d_a_tag_so.cpp:92` supplies exactly five). **The receiver type is bigger.**
// `include/f_op/f_op_actor.h:11`:
//     struct actor_method_class {
//         /* 0x00 */ leafdraw_method_class base;   // 5 method pointers
//         /* 0x14 */ u8 field_0x14[0xC];           // "Likely padding"
//     };
// On x64 that is 40 + 12 = 52, padded to **56 (0x38)** — which is exactly what
// the Integrator's §989 assert battery measured (`sizeof == 0x38`,
// `offsetof(field_0x14) == 0x28`).
//
// WHY THIS WAS A REAL BUG AND NOT A TIDINESS ISSUE: the receiver holds this
// object through an `actor_method_class*` and is entitled to touch all 56
// bytes. A 40-byte plugin object handed over as that type means any read of
// `field_0x14` runs 16 bytes past the end into whatever static follows it.
// "Likely padding" is not a guarantee, and I do not get to rely on the
// receiver never reading a field its own struct declares.
//
// THE LESSON I HAD BEEN GIVING OTHERS AND MISSED HERE: an initialiser list
// shows what the DONOR writes, not how big the RECEIVER type is. The donor's
// five entries are the base; the tail is invisible at the call site.
// ============================================================================
struct WwActorMethodClass {
    void* create_method;
    void* delete_method;
    void* execute_method;
    void* is_delete_method;
    void* draw_method;
    unsigned char field_0x14[12];
};
static_assert(offsetof(WwActorMethodClass, draw_method) == 32, "draw_method @0x20");
static_assert(offsetof(WwActorMethodClass, field_0x14) == 40, "field_0x14 @0x28");
static_assert(sizeof(WwActorMethodClass) == 56, "actor_method_class == 0x38 on x64");

WwActorMethodClass s_pilotMethods = {
    (void*)&WwPilot_create, (void*)&WwPilot_delete, (void*)&WwPilot_execute,
    (void*)&WwPilot_isDelete, (void*)&WwPilot_draw, {0},
};

// Filled at init: `methods`, `sub_method` (leaf) and the actor sub_method are
// RECEIVER addresses resolved by name — a plugin cannot author them.
WwActorProfileDef s_pilotProfile = {};

const short kPilotIndex = kWwProfileBase + 1;  // 4097

// ============================================================================
// DRAW PRIORITY IS RANGE-CONSTRAINED HERE, PLUGIN-SIDE — the Integrator's
// ruling on the `cTr_Addition` hazard I routed (CALLS row, tale §995).
//
// THE RECEIVER DEFECT, recorded and deliberately NOT patched: `c_tree.cpp:14`
// checks `listIdx >= mNumLists` and nothing else, so a NEGATIVE draw priority
// indexes before `mpLists` and writes there. The value can reach it because
// `leaf_process_profile_definition::priority` is an s16 — signed.
//
// WHY THE FIX LIVES HERE AND NOT THERE (DN-10 step 2): nothing in the receiver
// produces a negative priority, so a guard added to that hot path would be
// instance-authored defence over a system nobody has shown to be broken. WE
// are the only party that can author an above-enum profile, so we are the only
// party that can introduce the value — constrain it where it is CREATED.
//
// COMPILE-TIME FIRST: a literal priority fails the BUILD, not the frame,
// which is the same property the 27 layout asserts already give us.
// ============================================================================
// ============================================================================
// PORT #1 OF THE 20-CARD BATCH (queue 146) — daObjAkabe, the invisible
// red-wall collision actor. KIT-DONOR: d_a_obj_akabe.cpp (208 lines,
// **Matching for GZLJ01/GZLE01/GZLP01** — citable verbatim for the user's
// GZLE01 disc). KIT-DONOR-STATUS: BEHAVIOURALLY FAITHFUL, BYTE-DIVERGENT
// (MSVC x64 vs donor CW PPC). Donor logic carried whole: the four
// type-selected arcs, phased resLoad, solid-heap dBgW with MOVE_BG_e + the
// actor matrix, Bgsp registration, param-bit appear/scale rules, the
// execute-side despawn. Divergences, each labelled at its line:
//   · dBgW is CRT-allocated + receiver-ctor'd (the donor allocates from the
//     actor's solid heap; the plugin cannot run inside JKR heap switching) —
//     freed symmetrically at delete after Release.
//   · dComIfGs_checkCollect (inline over gameInfo, unresolvable) — the
//     swSave==0xFF branch defaults to APPEAR with a one-time WARN receipt.
//     Donor semantics: hidden once collect-item 0 is taken; Outset's single
//     placement uses a real switch, so the bridge branch is dormant there.
//   · mDoMtx_copy is inline — 12-float copy reproduced locally (pure math).
// ============================================================================
// Forward declaration — the plugin's logf is DEFINED below (after the CRT
// shadow window); without this, calls in the akabe section resolve to the
// CRT's math logf. Same trap that bit noteDefer, same cure.
void logf(LogLevel level, const char* fmt, ...);

// ============================================================================
// DZB ATTRIBUTE TRANSLATION AT CONSUME — DN-10 step 2, not a bake, not a mount.
//
// ConvDzb already rebases OffsetPtrs and one-time-swaps vertices. It does NOT
// translate poly-info bits. WW packs through/exit/att on a different layout
// than TP reads:
//   m_info0 through cluster: WW bits 16-26 vs TP bits 14-23 (Link-through)
//   m_info0 exit id:         WW bits 13-18 vs TP bits 0-5
//   m_info1 att/ground:      WW att at bits 16-20 vs TP att0/att1/groundCode
// Untouched donor dzb => every triangle is Link-through. Plain GroundCross
// still hits; player acch (SetLink) misses; ALINK create stays INIT on -INF.
// Measured historically on Outset: 85/85 ti entries carried 0x3FF in that
// cluster. Boot 172311 matches that signature (walker hits, player -INF).
//
// STEP 1 cannot apply: this is a genuine donor/receiver FORMAT difference at
// the TP consumption boundary, not a missing WW system. STEP 2 is the
// sanctioned answer — the same shape as the already-shipped ConvDzb arm.
//
// THE CODE PORTED is fork `dExtWw_repackDzbAttributes`
// (`src/d/d_ext_npc_mount.cpp:3709`, §654/§729/§334). That function lives
// inside the mount layer. DN-9 forbids calling it. The translation itself is
// the named format map; it is applied here, after ConvDzb, on vanilla only
// (fork already runs the same map tree-side). Header-free: walk cBgD_t by the
// offsets in `include/d/d_bg_w.h` and resolve OffsetPtrs the same way
// `include/dusk/offset_ptr.h` does after setBase. Idempotent by construction
// (second pass: through cluster already clear => no rewrite; slip signature
// preserved).
// ============================================================================
static std::uint32_t wwBe32Load(const void* p) {
    const unsigned char* b = static_cast<const unsigned char*>(p);
    return (static_cast<std::uint32_t>(b[0]) << 24) |
           (static_cast<std::uint32_t>(b[1]) << 16) |
           (static_cast<std::uint32_t>(b[2]) << 8) |
           static_cast<std::uint32_t>(b[3]);
}

static void wwBe32Store(void* p, std::uint32_t v) {
    unsigned char* b = static_cast<unsigned char*>(p);
    b[0] = static_cast<unsigned char>(v >> 24);
    b[1] = static_cast<unsigned char>(v >> 16);
    b[2] = static_cast<unsigned char>(v >> 8);
    b[3] = static_cast<unsigned char>(v);
}

static void* wwOffsetPtrResolve(void* field) {
    // After ConvDzb setBase: bit 31 = relocated, remaining bits = self-relative
    // offset. Same decode as OffsetPtr::operator T*() (offset_ptr.h:17-31).
    const std::int32_t swapped = static_cast<std::int32_t>(wwBe32Load(field));
    if (swapped == 0) {
        return nullptr;
    }
    const std::int32_t realOffset =
        (swapped & 0x40000000) ? swapped : (swapped & 0x7FFFFFFF);
    return static_cast<char*>(field) + realOffset;
}

static void wwDzbRepackAttributes(void* dzb, const char* tag) {
    if (dzb == nullptr || s_hostIsFork) {
        return;
    }
    unsigned char* base = static_cast<unsigned char*>(dzb);
    const int tiNum = static_cast<int>(wwBe32Load(base + 0x28));  // cBgD_t::m_ti_num
    const int tNum = static_cast<int>(wwBe32Load(base + 0x08));   // cBgD_t::m_t_num
    if (tiNum <= 0 || tiNum > 65536) {
        return;
    }
    unsigned char* ti =
        static_cast<unsigned char*>(wwOffsetPtrResolve(base + 0x2C));  // m_ti_tbl
    if (ti == nullptr) {
        return;
    }

    int standable = 0;
    int slip = 0;
    int through = 0;
    for (int i = 0; i < tiNum; i++) {
        unsigned char* rec = ti + (i * 16);  // sizeof(cBgD_Ti_t)
        const std::uint32_t inf0 = wwBe32Load(rec + 0);
        const std::uint32_t cleared = inf0 & ~0x00FFC000u;
        if (cleared != inf0) {
            const std::uint32_t wwExit = (inf0 >> 13) & 0x3F;
            std::uint32_t out0 = cleared & ~(0x0000003Fu | (1u << 13));
            out0 |= wwExit;
            wwBe32Store(rec + 0, out0);
            ++through;
        }

        const std::uint32_t inf1 = wwBe32Load(rec + 4);
        const std::uint32_t wwAtt = (inf1 >> 16) & 0x1F;
        std::uint32_t out = inf1 & ~0x00FFF000u;
        if (wwAtt == 0 && ((inf1 >> 19) & 0x1F) == 4 && ((inf1 >> 12) & 0xF) == 0) {
            out |= (4u << 19);
            ++slip;
            wwBe32Store(rec + 4, out);
            continue;
        }
        switch (wwAtt) {
            case 0x06:  // LAVA
            case 0x08:  // VOID
            case 0x09:  // DAMAGE
            case 0x15:  // FREEZE
            case 0x16:  // ELECTRICITY
                out |= (4u << 19);
                ++slip;
                break;
            default:
                ++standable;
                break;
        }
        wwBe32Store(rec + 4, out);
    }

    static int shown = 0;
    if (shown < 8) {
        shown++;
        const std::uint32_t post = wwBe32Load(ti + 4);
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"dzb_attr_repack\",\"tag\":\"%s\",\"standable\":%d,"
            "\"slip\":%d,\"through_cleared\":%d,\"ti_num\":%d,\"t_num\":%d,"
            "\"ti0_inf1\":\"0x%08x\",\"reads\":\"WW->TP through/exit/att at consume; "
            "ported from dExtWw_repackDzbAttributes, not the mount\"}",
            tag != nullptr ? tag : "?", standable, slip, through, tiNum, tNum, post);
    }
}

// Actor field offsets — MEASURED from the image's own PDB (llvm-pdbutil type
// stream, LF_MEMBER rows for `fopAc_ac_c`), not authored: current @1460,
// shape_angle @1480, scale @1488; `actor_place` carries pos at +0
// (f_op_actor.h:245 + PDB size 52 agree). The §970 hazard is exactly what
// this note exists to prevent recurring.
const unsigned int kOffCurrentPos = 1460;
const unsigned int kOffShapeAngle = 1480;
const unsigned int kOffScale = 1488;
static_assert(kOffScale + 12 <= 0x668, "inside measured sizeof(fopAc_ac_c)");

// ============================================================================
// WW-ACTOR SUPPORT LAYER — the shared plumbing the ports past `akabe` need.
//
// WHY: akabe shipped because it is a pure `fopAc_ac_c` + `dBgW` actor. The
// rest of the batch needs MEMBER OBJECTS whose APIs are header-inline and so
// carry no linkable symbol (`switem`: dCcD_Cyl/dCcD_Stts; `dk`: AcchCir/
// ObjAcch), and `npc_ah` is blocked outright — the receiver has no
// `fopNpc_npc_c` at all. Deriving this once makes the rest cheap.
//
// EVERY NUMBER BELOW IS MEASURED FROM THE IMAGE'S OWN PDB TYPE STREAM
// (LF_CLASS sizes, LF_BCLASS base offsets, LF_MEMBER field offsets) — the
// method History/Bridge independently corroborated. The base chain for the
// cylinder was DERIVED rather than guessed:
//     dCcD_Cyl  --(LF_BCLASS +376)-->  cCcD_CylAttr
//     cCcD_CylAttr --(LF_BCLASS +40)-->  cM3dGCyl
//     cM3dGCyl: mCenter@8 · mRadius@20 · mHeight@24
// so the header-inline SetC/SetR/SetH are writes at +424 / +436 / +440.
//
// AND THE STANDARD THE DAY'S THREE INSTRUMENT ERRORS TAUGHT (the 83-roots
// premise, my block-table misread, the two-armed +INF): AN OFFSET PROVES
// WHERE A FIELD IS, NEVER WHAT IT MEANS. So `wwSupportVerify()` writes known
// values and reads them back before any actor trusts them, and refuses the
// whole layer on mismatch rather than letting a wrong offset ship silently.
// ============================================================================
const unsigned int kSizeofDCcDCyl      = 448;   // LF_CLASS `dCcD_Cyl`
const unsigned int kSizeofDCcDStts     = 64;    // conservative; ctor-filled
const unsigned int kSizeofDBgSObjAcch  = 736;   // LF_CLASS `dBgS_ObjAcch`
const unsigned int kSizeofDBgSAcchCir  = 128;   // conservative; ctor-filled
const unsigned int kOffCylCenter = 424;         // +376 CylAttr +40 Cyl +8
const unsigned int kOffCylRadius = 436;         // ... +20
const unsigned int kOffCylHeight = 440;         // ... +24

typedef void (*FnVoidThis)(void*);
typedef void (*FnSttsInit)(void*, int, int, void*);
typedef void (*FnCylSet)(void*, const void*);
typedef void (*FnSetWall)(void*, float, float);
FnVoidThis  s_fnCylCtor = nullptr;
FnVoidThis  s_fnSttsCtor = nullptr;
FnVoidThis  s_fnObjAcchCtor = nullptr;
FnVoidThis  s_fnAcchCirCtor = nullptr;
FnSttsInit  s_fnSttsInit = nullptr;
FnCylSet    s_fnCylSet = nullptr;
FnSetWall   s_fnAcchSetWall = nullptr;
bool s_supportReady = false;

// Inline-API shims: the receiver's own setters are header-inline, so they are
// reached by measured offset. Reading is the same field, which is what makes
// the verification below possible at all.
void wwCyl_setCenter(void* cyl, float x, float y, float z) {
    float* c = reinterpret_cast<float*>(static_cast<char*>(cyl) + kOffCylCenter);
    c[0] = x; c[1] = y; c[2] = z;
}
void wwCyl_setRadius(void* cyl, float r) {
    *reinterpret_cast<float*>(static_cast<char*>(cyl) + kOffCylRadius) = r;
}
void wwCyl_setHeight(void* cyl, float h) {
    *reinterpret_cast<float*>(static_cast<char*>(cyl) + kOffCylHeight) = h;
}

// WRITE-THEN-READ-BACK. Runs once at install on a scratch cylinder the
// receiver itself constructed: if a constructor stamps these fields, or an
// offset is wrong, the readback disagrees and the layer disables itself —
// loudly — instead of handing actors a cylinder with its radius written into
// someone else's member.
void wwSupportVerify() {
    if (s_fnCylCtor == nullptr) {
        logf(LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"support_verify\",\"ok\":0,"
            "\"why\":\"dCcD_Cyl ctor unresolved - layer stays DISABLED\"}");
        return;
    }
    void* scratch = std::calloc(1, kSizeofDCcDCyl);
    if (scratch == nullptr) { return; }
    s_fnCylCtor(scratch);
    wwCyl_setCenter(scratch, 1234.5f, -678.25f, 9012.75f);
    wwCyl_setRadius(scratch, 31.5f);
    wwCyl_setHeight(scratch, 62.25f);
    const float* c = reinterpret_cast<const float*>(
        static_cast<char*>(scratch) + kOffCylCenter);
    const float r = *reinterpret_cast<const float*>(
        static_cast<char*>(scratch) + kOffCylRadius);
    const float h = *reinterpret_cast<const float*>(
        static_cast<char*>(scratch) + kOffCylHeight);
    const bool ok = (c[0] == 1234.5f && c[1] == -678.25f && c[2] == 9012.75f &&
                     r == 31.5f && h == 62.25f);
    s_supportReady = ok;
    logf(ok ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"support_verify\",\"ok\":%d,\"center\":[%.2f,%.2f,%.2f],"
        "\"radius\":%.2f,\"height\":%.2f,\"reads\":\"write-then-readback on a "
        "receiver-constructed cylinder; a mismatch means a derived offset is "
        "wrong and the layer REFUSES rather than shipping it\"}",
        ok ? 1 : 0, c[0], c[1], c[2], r, h);
    std::free(scratch);
}

struct AkabeMembers {
    unsigned char phs[0x80];   // request_of_phase_process_class, oversized —
                               // the receiver writes within ITS sizeof
    void* bgw;                 // dBgW*, CRT-allocated, receiver-constructed
    float mtx[3][4];
    int type;                  // 0 Akabe · 1 AkabeD · 2 AkabeK · 3 NBOX
    unsigned char appear;
};
AkabeMembers* akabeMembers(void* self) {
    return reinterpret_cast<AkabeMembers*>(static_cast<char*>(self) + kActorBaseSize);
}
const char* const kAkabeArcs[4] = {"Akabe", "AkabeD", "AkabeK", "NBOX"};
// dRes_INDEX_*_DZB_* from the donor's generated res headers (assets/D44J01):
// every one of the four arcs carries its dzb at index 3.
const int kAkabeDzbIndex = 3;

// Callables, resolved in the init block with the others. Every one of these
// was gate-verified on the user's image before this section was written.
typedef int   (*FnResLoad)(void*, const char*);
typedef int   (*FnResDelete)(void*, const char*);
typedef void* (*FnGetObjRes)(const char*, int);
typedef bool  (*FnEntrySolidHeap)(void*, int (*)(void*), unsigned int);
typedef void  (*FnDBgWCtor)(void*);
typedef bool  (*FnBgwSet)(void*, void*, unsigned int, void*);
typedef bool  (*FnBgsRegist)(void*, void*, void*);
typedef void  (*FnBgsRelease)(void*, void*);
typedef void  (*FnSetCrrFunc)(void*, void*);
typedef void  (*FnSetPriority)(void*, int);
typedef void  (*FnSetMtx)(void*, void*);
typedef void  (*FnCullBox)(void*, float, float, float, float, float, float);
typedef int   (*FnIsSwitch)(const void*, int);
typedef int   (*FnActorDelete)(void*);
typedef void  (*FnMtxTransS)(float, float, float);
typedef void  (*FnMtxZXYrotM)(short, short, short);
typedef void  (*FnMtxScaleM)(float, float, float);
typedef void* (*FnMtxGet)();
FnResLoad        s_fnResLoad = nullptr;
FnResDelete      s_fnResDelete = nullptr;

// Forward decls: the parse-once table is defined further down (it needs the
// J3D loader binds), but its INVALIDATORS are called from delete paths above it.
// ========================================================================
// THE REST OF THE RECEIVER'S MODEL CONVERSION - I had only done step ONE.
// ========================================================================
// `d_resorce.cpp` converts a model in a SEQUENCE, and EVERY arm is the same
// four steps: load -> newSharedDisplayList(UseSingleDL) -> simpleCalcMaterial
// (j3dDefaultMtx) -> makeSharedDL(). I called `loadBinaryDisplayList` and
// treated that as "converted", so the model reached `modelUpdateDL` with NO
// shared display list built and materials never calculated.
// R2 says "convert at the fetch site with the RECEIVER'S OWN loaders" and I
// used ONE of them. A conversion that is a sequence is not satisfied by its
// first step - and the failure is invisible on the CPU, because nothing
// faults: a malformed model is only rejected later, by the GPU.
// ========================================================================
typedef void (*FnModelCalc)(void* model);
typedef void (*FnSetBaseScale)(void* model, const void* vec);
FnModelCalc    s_fnModelCalc    = nullptr;
FnSetBaseScale s_fnSetBaseScale = nullptr;
typedef int  (*FnNewSharedDL)(void* modelData, unsigned int flag);
typedef void (*FnSimpleCalcMat)(void* modelData, const void* mtx);
typedef void (*FnMakeSharedDL)(void* modelData);
FnNewSharedDL   s_fnNewSharedDL   = nullptr;
FnSimpleCalcMat s_fnSimpleCalcMat = nullptr;
FnMakeSharedDL  s_fnMakeSharedDL  = nullptr;
void*           s_j3dDefaultMtx   = nullptr;
typedef void (*FnMatMakeSharedDL)(void* material);
FnMatMakeSharedDL s_fnJ3DMatMakeSharedDL = nullptr;
int s_wwFinishDepth = 0;
// READ, NOT AUTHORED: J3DModel.h:16 `J3DMdlFlag_UseSingleDL = 0x40000`.
// I first wrote 0x20000 from memory - the FOURTH authored-constant slip of
// the session, and the first one caught BEFORE it shipped, by reading the
// header instead of trusting the recollection.
const unsigned int kJ3DMdlFlag_UseSingleDL = 0x40000;
// WW d_resorce.cpp 'BDL ' arm: loadBinaryDisplayList(res, 0x00002020).
// 0x2020 = UsePostTexMtx|DoBdlMaterialCalc — NOT TP BMD 0x59020010, NOT 0x1010.
const unsigned int kWwBdlLoadFlags = 0x00002020u;
// Brief A/B gate for stretch suspect #3 (WW consume: 0x2020 + setToonTex +
// keep MDL3 SharedDL). TEMP default ON (finish) so a normal launch tests the
// gate — last run logged mode=ww because WW_BDL_CONSUME was unset. Restore WW
// with WW_BDL_CONSUME=ww. Does NOT delete the WW path — diagnostic only.
const unsigned int kFinishBdlLoadFlags = 0x59020010u;

static int wwBdlConsumeFinishGate() {
    static int s_gate = -1;
    if (s_gate < 0) {
        // Default ON for this A/B install. Env "ww" / "0" / "off" restores
        // current WW consume; "finish" / "gate" / "1" keeps the gate.
        s_gate = 1;
        const char* e = std::getenv("WW_BDL_CONSUME");
        if (e != nullptr && e[0] != '\0') {
            if (std::strcmp(e, "ww") == 0 || std::strcmp(e, "0") == 0 ||
                std::strcmp(e, "off") == 0) {
                s_gate = 0;
            } else if (std::strcmp(e, "finish") == 0 ||
                       std::strcmp(e, "gate") == 0 ||
                       std::strcmp(e, "1") == 0) {
                s_gate = 1;
            } else if (std::strcmp(e, "finish_toon") == 0 ||
                       std::strcmp(e, "2") == 0) {
                // ================================================
                // THE THIRD ARM THIS FILE'S OWN TIP ASKED FOR AND
                // NOBODY HAD RUN: "split finish vs flags vs
                // skip-setToonTex".
                // ================================================
                // The stretch fix bundled TWO changes - always
                // finish, AND drop setToonTex - and shipped them as
                // one switch. Donor law is 0x2020 + setToonTex
                // (d_resorce.cpp). A material that samples the toon
                // texture and never receives it renders BLACK, which
                // is what the user reports seeing on the donor
                // vegetation. This arm keeps the finish consume that
                // cured the stretch and RESTORES setToonTex, so the
                // two halves of that bundle can be told apart.
                // Outcome is informative either way: black gone +
                // stretch gone = the bundle was over-broad; stretch
                // back = the two are genuinely coupled and the cost
                // is a real trade rather than an oversight.
                // ================================================
                s_gate = 2;
            }
        }
        logf(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"bdl_consume_gate\",\"mode\":\"%s\","
             "\"reads\":\"TEMP default=finish (gate #3); set WW_BDL_CONSUME=ww "
             "to restore 0x2020+setToonTex+keep MDL3\"}",
             s_gate == 2 ? "finish_toon" : (s_gate ? "finish" : "ww"));
    }
    return s_gate;
}

static unsigned int wwBdlLoadFlags() {
    return wwBdlConsumeFinishGate() ? kFinishBdlLoadFlags : kWwBdlLoadFlags;
}

// finishBgModelData / wwParseModelOnce: load is step one of four. getRes and
// loadResource were stopping at load — empty shared DLs → Aurora tcg src 21.
void wwFinishModelData(void** pParsed, const char* tag) {
    if (pParsed == nullptr || *pParsed == nullptr) {
        return;
    }
    void* parsed = *pParsed;
    if (s_fnNewSharedDL == nullptr) {
        logf(LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"model_finish\",\"tag\":\"%s\",\"ok\":0,"
            "\"why\":\"newSharedDisplayList unresolved\"}",
            tag != nullptr ? tag : "?");
        return;
    }
    s_wwFinishDepth++;
    const int sharedRet = s_fnNewSharedDL(parsed, kJ3DMdlFlag_UseSingleDL);
    if (sharedRet == 0) {
        if (s_fnSimpleCalcMat != nullptr && s_j3dDefaultMtx != nullptr) {
            s_fnSimpleCalcMat(parsed, s_j3dDefaultMtx);
        }
        if (s_fnMakeSharedDL != nullptr) {
            s_fnMakeSharedDL(parsed);
        }
    } else {
        logf(LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"model_sharedDL_fail\",\"tag\":\"%s\",\"ret\":%d,"
            "\"reads\":\"receiver returns -1 here; refusing a half-built model\"}",
            tag != nullptr ? tag : "?", sharedRet);
        *pParsed = nullptr;
    }
    s_wwFinishDepth--;
    static int shown = 0;
    if (shown < 8) {
        shown++;
        logf(*pParsed != nullptr ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"model_finish\",\"tag\":\"%s\",\"ok\":%d,"
            "\"sharedDL\":%d,\"binds\":\"%d%d%d\",\"reads\":\"load already done; "
            "this is newSharedDisplayList+simpleCalcMaterial+makeSharedDL; "
            "2N bake runs inside makeSharedDL while finish-depth>0\"}",
            tag != nullptr ? tag : "?", *pParsed != nullptr ? 1 : 0, sharedRet,
            s_fnNewSharedDL != nullptr, s_fnSimpleCalcMat != nullptr,
            s_fnMakeSharedDL != nullptr);
    }
}

// §678 (fork ww_room_loader): daBg clips each shape with viewMtx × LOCAL
// min/max and never applies the MULT baseTRMtx. sea room 44 sits at
// ~(-200000, 300000); local boxes stay at the origin, so a camera that is
// actually on Outset hides every island shape. Transform parsed bounds into
// world XZ once per first parse (Y unchanged — MULT is XZ + Y-rot only).
// §682 (fork d_a_bg.cpp): donor daBg clips whole models only. Vanilla always
// per-shape hides. Worldize (§678) puts rock AABBs in the frustum; thin
// overlay shapes (grass cards / model1 xlu) still fail the clip because the
// view at list-build is not the view at present. Undo hide at EntryDL PRE —
// after the clip loop, before packets. clipper::clip and shape->hide are
// inline; this is the consume-boundary of "do not hide."
const int kMaxWorldized = 32;
void* s_worldized[kMaxWorldized];
int s_worldizedN = 0;

void wwRememberWorldized(void* i_parsed) {
    if (i_parsed == nullptr) {
        return;
    }
    for (int i = 0; i < s_worldizedN; i++) {
        if (s_worldized[i] == i_parsed) {
            return;
        }
    }
    if (s_worldizedN < kMaxWorldized) {
        s_worldized[s_worldizedN++] = i_parsed;
    }
}

int wwIsWorldized(void* i_parsed) {
    if (i_parsed == nullptr) {
        return 0;
    }
    for (int i = 0; i < s_worldizedN; i++) {
        if (s_worldized[i] == i_parsed) {
            return 1;
        }
    }
    return 0;
}

void wwWorldizeShapeBounds(void* i_parsed, const char* i_arc) {
    if (i_parsed == nullptr || i_arc == nullptr) {
        return;
    }
    if (!(i_arc[0] == 'R' && i_arc[1] >= '0' && i_arc[1] <= '9' && i_arc[2] >= '0' &&
          i_arc[2] <= '9' && i_arc[3] == '_' && i_arc[4] == '0' && i_arc[5] == '0' &&
          i_arc[6] == '\0')) {
        return;
    }
    const int roomNo = (i_arc[1] - '0') * 10 + (i_arc[2] - '0');
    f32 tx = 0.0f;
    f32 tz = 0.0f;
    s16 angle = 0;
    if (!dComIfGp_getMapTrans(roomNo, &tx, &tz, &angle)) {
        logf(LOG_LEVEL_WARN,
             "[WwRegistry] {\"ev\":\"shape_worldize\",\"arc\":\"%s\",\"room\":%d,"
             "\"ok\":0,\"why\":\"getMapTrans miss (MULT not live yet)\"}",
             i_arc, roomNo);
        return;
    }
    J3DModelData* model = static_cast<J3DModelData*>(i_parsed);
    const f32 s = cM_ssin(angle);
    const f32 c = cM_scos(angle);
    const u16 n = model->getShapeNum();
    for (u16 j = 0; j < n; j++) {
        J3DShape* shape = model->getShapeNodePointer(j);
        if (shape == nullptr) {
            continue;
        }
        Vec* mn = shape->getMin();
        Vec* mx = shape->getMax();
        const f32 xs[2] = {mn->x, mx->x};
        const f32 zs[2] = {mn->z, mx->z};
        f32 minX = 1e30f, minZ = 1e30f, maxX = -1e30f, maxZ = -1e30f;
        for (int a = 0; a < 2; a++) {
            for (int b = 0; b < 2; b++) {
                const f32 wx = c * xs[a] + s * zs[b] + tx;
                const f32 wz = -s * xs[a] + c * zs[b] + tz;
                if (wx < minX) {
                    minX = wx;
                }
                if (wx > maxX) {
                    maxX = wx;
                }
                if (wz < minZ) {
                    minZ = wz;
                }
                if (wz > maxZ) {
                    maxZ = wz;
                }
            }
        }
        mn->x = minX;
        mn->z = minZ;
        mx->x = maxX;
        mx->z = maxZ;
    }
    wwRememberWorldized(i_parsed);
    logf(LOG_LEVEL_INFO,
         "[WwRegistry] {\"ev\":\"shape_worldize\",\"arc\":\"%s\",\"room\":%d,"
         "\"shapes\":%u,\"tx\":%.1f,\"tz\":%.1f,\"rot\":%d}",
         i_arc, roomNo, (unsigned)n, tx, tz, (int)angle);
}

void wwInvalidateParsedModels(const char* arc);
void wwInvalidateParsedAnims(const char* arc);
void wwResetParsedAnims();
void* wwParseAnimOnce(void* raw, const char* arc, int index);
void wwInvalidateParsedModelsAll(const char* why);
FnGetObjRes      s_fnGetObjRes = nullptr;
FnEntrySolidHeap s_fnEntrySolidHeap = nullptr;
FnDBgWCtor       s_fnDBgWCtor = nullptr;
FnBgwSet         s_fnBgwSet = nullptr;
FnBgsRegist      s_fnBgsRegist = nullptr;
FnBgsRelease     s_fnBgsRelease = nullptr;
FnSetCrrFunc     s_fnSetCrrFunc = nullptr;
FnSetPriority    s_fnSetPriority = nullptr;
FnSetMtx         s_fnSetMtx = nullptr;
FnCullBox        s_fnCullBox = nullptr;
FnIsSwitch       s_fnIsSwitch = nullptr;
FnActorDelete    s_fnActorDelete = nullptr;
FnMtxTransS      s_fnMtxTransS = nullptr;
FnMtxZXYrotM     s_fnMtxZXYrotM = nullptr;
FnMtxScaleM      s_fnMtxScaleM = nullptr;
FnMtxGet         s_fnMtxGet = nullptr;
// The dBgS instance, captured from the DBgSRegist post-hook's arg0 — the
// receiver hands us its own `this` on every TP boot registration, long
// before any WW actor can create. Zero offsets, zero new resolves.
void* s_bgspInstance = nullptr;

// Donor param bits (d_a_obj_akabe.h Prm_e): (shift, width) per field.
int akabePrm(void* self, int shift, int width) {
    if (s_fnGetParam == nullptr) { return 0; }
    return (int)wwGetParamBit(s_fnGetParam(self), shift, width);
}

// Donor current.pos / shape_angle / scale — via the receiver accessors the
// pilot already established (kOffCurrentPos etc. verified by the §989 assert
// battery); reuse those offsets rather than re-deriving.
int WwAkabe_solidHeapCB(void* self) {
    AkabeMembers* m = akabeMembers(self);
    if (s_fnDBgWCtor == nullptr || s_fnBgwSet == nullptr) { return 0; }
    void* bgw = std::calloc(1, 0x400);       // DIVERGENCE: CRT alloc, see header
    if (bgw == nullptr) { return 0; }
    s_fnDBgWCtor(bgw);
    void* dzb = (s_fnGetObjRes != nullptr)
                    ? s_fnGetObjRes(kAkabeArcs[m->type], kAkabeDzbIndex) : nullptr;
    if (dzb == nullptr) { std::free(bgw); return 0; }
    // ====================================================================
    // COVERAGE GAP CLOSED (Foundry, CALLS 1045 — a defect in my own port).
    // The plugin's conversion arm matches on a resource NAME ending `.dzb`.
    // THIS fetch is BY INDEX, so no name exists to match and the buffer
    // reached `Set` UNCONVERTED — un-setBase'd OffsetPtrs resolve relative
    // to the field's own address: deterministic garbage, no crash, AABB
    // junk. Exactly the class of fault the Outset trace is hunting, in my
    // own code. Converted explicitly here; ConvDzb's own 0x80000000 latch
    // makes a second call on an already-converted buffer a no-op, so this
    // is safe whether or not something else got there first.
    // ====================================================================
    if (s_fnConvDzb != nullptr && !s_hostIsFork) {
        void* conv = s_fnConvDzb(dzb);
        if (conv != nullptr) { dzb = conv; }
    }
    // ========================================================================
    // CONVERT BEFORE Set — THIS FETCH IS BY INDEX, SO THE NAME-KEYED ARM
    // CANNOT SEE IT (Foundry's coverage read). The getRes POST hook matches on
    // a resource NAME ending `.dzb` and replaces the return value; a fetch with
    // no name never meets it, and an un-rebased dzb handed to Set resolves its
    // OffsetPtrs against each field's own address — deterministic garbage, no
    // crash, AABB junk, -INF. That is the symptom the collision hunt is chasing,
    // and this call site was one of its unguarded sources.
    //
    // Safe to call unconditionally: the receiver's ConvDzb carries a 0x80000000
    // latch, so a buffer converted elsewhere is a no-op here. Cheaper than
    // proving which path got there first.
    // ========================================================================
    if (s_fnConvDzb != nullptr) {
        void* conv = s_fnConvDzb(dzb);
        if (conv != nullptr) {
            dzb = conv;
        }
        static bool s_akabeConvLogged = false;
        if (!s_akabeConvLogged) {
            s_akabeConvLogged = true;
            logf(LOG_LEVEL_INFO,
                "[WwRegistry] {\"ev\":\"dzb_conv_byindex\",\"site\":\"akabe solidHeap\","
                "\"arc\":\"%s\",\"idx\":%d,\"note\":\"index fetch bypasses the name-keyed "
                "arm; converted explicitly before Set (ConvDzb latch makes it idempotent)\"}",
                kAkabeArcs[m->type], (int)kAkabeDzbIndex);
        }
    }
    wwDzbRepackAttributes(dzb, kAkabeArcs[m->type]);
    // donor: if (!Set(dzb, MOVE_BG_e, &mMtx)) ret = true;  (Set false = OK)
    if (s_fnBgwSet(bgw, dzb, 0x1u /* cBgW::MOVE_BG_e */, m->mtx)) {
        std::free(bgw);
        return 0;
    }
    m->bgw = bgw;
    return 1;
}

int WwAkabe_create(void* self) {
    WwTagSo_ct(self);                        // fopAcM_ct expansion, shared
    AkabeMembers* m = akabeMembers(self);
    const int arg0 = akabePrm(self, 16, 4);
    m->type = (arg0 >= 1 && arg0 <= 3) ? arg0 : 0;
    // chk_appear, donor rules; the 0xFF branch is the labelled bridge.
    if (m->appear == 0) {
        const int always = akabePrm(self, 12, 1);
        const int swSave = akabePrm(self, 0, 8);
        bool appear;
        if (always != 0) { appear = true; }
        else if (swSave == 0xFF) {
            static int warned = 0;
            if (!warned) { warned = 1;
                logf(LOG_LEVEL_WARN,
                    "[WwRegistry] {\"ev\":\"akabe_bridge\",\"why\":\"checkCollect "
                    "inline-unreachable; swSave==0xFF defaults to APPEAR\"}");
            }
            appear = true;
        } else {
            appear = (s_fnIsSwitch == nullptr) || !s_fnIsSwitch(self, swSave);
        }
        if (!appear) { return 5; }           // cPhs_ERROR_e: donor returns ERROR
        m->appear = 1;
    }
    if (s_fnResLoad == nullptr || s_fnEntrySolidHeap == nullptr) { return 5; }
    const int rt = s_fnResLoad(m->phs, kAkabeArcs[m->type]);
    if (rt != 4) { return rt; }              // INIT/LOADING pass through
    // init_scale (donor): scl bits at (8,2)
    {
        float* scl = reinterpret_cast<float*>(static_cast<char*>(self) + kOffScale);
        const int s = akabePrm(self, 8, 2);
        if (s == 1)      { scl[0] *= 10.0f; scl[1] *= 10.0f; scl[2] = 1.0f; }
        else if (s == 3) { scl[0] *= 10.0f; scl[1] *= 10.0f; scl[2] *= 10.0f; }
        else if (s != 2) { scl[2] = 1.0f; }
    }
    // init_mtx (donor): trans * ZXYrot * scale -> mMtx
    if (s_fnMtxTransS != nullptr) {
        const float* pos = reinterpret_cast<const float*>(
            static_cast<const char*>(self) + kOffCurrentPos);
        const short* ang = reinterpret_cast<const short*>(
            static_cast<const char*>(self) + kOffShapeAngle);
        const float* scl = reinterpret_cast<const float*>(
            static_cast<const char*>(self) + kOffScale);
        s_fnMtxTransS(pos[0], pos[1], pos[2]);
        s_fnMtxZXYrotM(ang[0], ang[1], ang[2]);
        s_fnMtxScaleM(scl[0], scl[1], scl[2]);
        const float* src = static_cast<const float*>(s_fnMtxGet());
        std::memcpy(m->mtx, src, sizeof(m->mtx));   // mDoMtx_copy, pure math
    }
    static const unsigned int kHeap[4] = {0x200, 0x200, 0x200, 0x3E0};
    if (!s_fnEntrySolidHeap(self, WwAkabe_solidHeapCB, kHeap[m->type])) {
        return 5;
    }
    if (s_bgspInstance != nullptr && s_fnBgsRegist != nullptr && m->bgw != nullptr) {
        s_fnBgsRegist(s_bgspInstance, m->bgw, self);
        // SetCrrFunc(NULL) omitted: header-inline, and calloc+ctor already
        // leave m_crr_func zero — the donor line is satisfied by construction.
        if (s_fnSetPriority != nullptr) { s_fnSetPriority(m->bgw, 1); }
    }
    if (s_fnSetMtx != nullptr) { s_fnSetMtx(self, m->mtx); }
    if (s_fnCullBox != nullptr) {
        if (m->type == 3) { s_fnCullBox(self, -51.f, -1.f, -51.f, 51.f, 101.f, 51.f); }
        else              { s_fnCullBox(self, -51.f, -1.f, -1.f, 51.f, 101.f, 1.f); }
    }
    static int created = 0;
    if (created < 4) { created++;
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"akabe_created\",\"type\":%d,\"bgw\":\"%p\","
            "\"reads\":\"port #1 of the 20-card batch is ALIVE and registered\"}",
            m->type, m->bgw);
    }
    return 4;                                // cPhs_COMPLEATE_e — the pilot lesson
}
int WwAkabe_delete(void* self) {
    AkabeMembers* m = akabeMembers(self);
    if (m->appear) {
        if (m->bgw != nullptr && s_bgspInstance != nullptr && s_fnBgsRelease != nullptr) {
            s_fnBgsRelease(s_bgspInstance, m->bgw);
        }
        std::free(m->bgw);
        m->bgw = nullptr;
        wwInvalidateParsedModels(kAkabeArcs[m->type]);
        wwInvalidateParsedAnims(kAkabeArcs[m->type]);
        if (s_fnResDelete != nullptr) { s_fnResDelete(m->phs, kAkabeArcs[m->type]); }
    }
    return 1;
}
int WwAkabe_execute(void* self) {
    // donor: if (!chk_appear()) fopAcM_delete(this) — the switch-driven despawn
    AkabeMembers* m = akabeMembers(self);
    const int always = akabePrm(self, 12, 1);
    const int swSave = akabePrm(self, 0, 8);
    if (!always && swSave != 0xFF && s_fnIsSwitch != nullptr &&
        s_fnIsSwitch(self, swSave) && m->appear && s_fnActorDelete != nullptr) {
        s_fnActorDelete(self);
    }
    return 1;
}
int WwAkabe_isDelete(void*) { return 1; }
int WwAkabe_draw(void*)     { return 1; }   // donor _draw: invisible wall

WwActorMethodClass s_akabeMethods = {
    (void*)&WwAkabe_create, (void*)&WwAkabe_delete, (void*)&WwAkabe_execute,
    (void*)&WwAkabe_isDelete, (void*)&WwAkabe_draw, {0},
};
WwActorProfileDef s_akabeProfile = {};

WwActorMethodClass s_vrboxMethods = {
    (void*)&WwVrbox_create, (void*)&WwVrbox_delete, (void*)&WwVrbox_execute,
    (void*)&WwVrbox_isDelete, (void*)&WwVrbox_draw, {0},
};
WwActorProfileDef s_vrboxProfile = {};

WwActorMethodClass s_vrbox2Methods = {
    (void*)&WwVrbox2_create, (void*)&WwVrbox2_delete, (void*)&WwVrbox2_execute,
    (void*)&WwVrbox2_isDelete, (void*)&WwVrbox2_draw, {0},
};
WwActorProfileDef s_vrbox2Profile = {};

// ============================================================================
// PORT — daKytag01 (WAVE_INFO / coast foam kill). Donor d_a_kytag01.cpp
// MATCHING. Placement name "ky_tag1" (WW d_stage.cpp:660). Room44 SCOB×1.
// Foam channel is plugin-owned (s_chan); Create only registers WAVE_INFO into
// wwWave mpWaveInfl table. Does NOT call donor wave_make / zero WaveCount —
// that would tear down foam owned by ww_wave.
// ============================================================================
struct Kytag01Members {
    WwWaveInfl wave;  // donor WAVE_INFO at actor+base
};
static_assert(sizeof(Kytag01Members) == 24, "WAVE_INFO layout");

Kytag01Members* kytag01Members(void* self) {
    return reinterpret_cast<Kytag01Members*>(static_cast<char*>(self) + kActorBaseSize);
}

int WwKytag01_create(void* self) {
    WwTagSo_ct(self);
    Kytag01Members* m = kytag01Members(self);
    const float* pos =
        reinterpret_cast<const float*>(static_cast<const char*>(self) + kOffCurrentPos);
    const float* scl =
        reinterpret_cast<const float*>(static_cast<const char*>(self) + kOffScale);
    m->wave.mPos.set(pos[0], pos[1], pos[2]);
    m->wave.mInnerRadius = scl[0] * 5000.0f;
    m->wave.mOuterRadius = scl[2] * 5000.0f;
    m->wave.field_0x14 = 0.0f;
    const f32 defaultOuter = m->wave.mInnerRadius + 500.0f;
    if (defaultOuter >= m->wave.mOuterRadius) {
        m->wave.mOuterRadius = defaultOuter;
    }
    wwWave_registerInfl(&m->wave);
    static int created = 0;
    if (created < 4) {
        created++;
        logf(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"kytag01_created\",\"pos\":[%.1f,%.1f,%.1f],"
             "\"inner\":%.0f,\"outer\":%.0f,\"reads\":\"donor daKytag01_Create\"}",
             m->wave.mPos.x, m->wave.mPos.y, m->wave.mPos.z, m->wave.mInnerRadius,
             m->wave.mOuterRadius);
    }
    return 4;  // cPhs_COMPLEATE_e
}

int WwKytag01_delete(void* self) {
    wwWave_unregisterInfl(&kytag01Members(self)->wave);
    return 1;
}
int WwKytag01_execute(void*) { return 1; }
int WwKytag01_isDelete(void*) { return 1; }
int WwKytag01_draw(void*) { return 1; }

WwActorMethodClass s_kytag01Methods = {
    (void*)&WwKytag01_create, (void*)&WwKytag01_delete, (void*)&WwKytag01_execute,
    (void*)&WwKytag01_isDelete, (void*)&WwKytag01_draw, {0},
};
WwActorProfileDef s_kytag01Profile = {};

// Model/draw callables used by lwood (and retained for later props).
// Declared before the lwood body so create/draw can see them.
typedef void* (*FnJ3DModelCreate)(void*, unsigned int, unsigned int);
typedef void  (*FnCullFar)(void*, float);
typedef float (*FnRndF)(float);
typedef void  (*FnBgwMove)(void*);
typedef void  (*FnModelUpdateDL)(void*);
FnJ3DModelCreate s_fnJ3DModelCreateC = nullptr;
FnCullFar        s_fnCullFar = nullptr;
FnRndF           s_fnRndF = nullptr;
FnBgwMove        s_fnBgwMove = nullptr;
FnModelUpdateDL  s_fnModelUpdateDL = nullptr;
// ============================================================
// MODE 12 - THE RECEIVER'S OWN SUBMITTER
// ============================================================
// daBg_c::draw() submits room models with mDoExt_modelEntryDL
// (d_a_bg.cpp:452) and Room44's model.bdl - the SAME 448 B/mat
// SharedDL class as Alwd - survives every frame that way.
// lwood submits with mDoExt_modelUpdateDL instead, and the two
// differ in one decisive place: EntryDL carries a TARGET_PC
// frame-interp early-out (`if (!is_sim_frame()) { diff(); return; }`)
// and UpdateDL has NONE - so lwood submits on EVERY frame, sim or
// interpolated, while every receiver model submits only on sim
// frames. DN-10 step 1: use the donor-faithful path the survivor
// already uses rather than inventing a fourth suppression mode.
FnModelUpdateDL  s_fnModelEntryDL = nullptr;
const unsigned int kLwoodOffTevStr = 496;

// ============================================================================
// PORT — daLwood (normal tree). Donor d_a_lwood.cpp MATCHING (GZLE01).
//
// TRACE / PRIOR FAILURES → RESOLUTIONS (this ship):
//   R2  by-index DZB never hits name-keyed ConvDzb → call ConvDzb + att
//       repack before Set (same as akabe).
//   R3  getObjRes returns raw forever → wwParseModelOnce + evict on
//       resDelete / stage load (memo already in this file).
//   R4  authored constants are DATA: BDL=4, DZB=7 (GZLE01 Lwood.h /
//       RARC table). Prior crash wrote DZB=5. TEV_TYPE_BG0=1 not 0.
//   R5  CreateInit is NOT in the method table — call it from create
//       after solid heap (SetMtx, cull box/far, calc, Regist, set_mtx,
//       Move). Method-table-only ports dropped these.
//   R6  bind counts ≠ coverage — every CreateInit call site is reached
//       and logged once.
//   DN-1 Regist(bgw, this) only — never SetRoomId.
//   Dawn draw: mode1/5 PASS · mode4 FAIL even locked:1 AND private ModelData.
//       Private-copy was invented — REVERTED (DN-10). WW d_resorce 'BDL ':
//       loadBinaryDisplayList(0x2020) + setToonTex; NO newSharedDisplayList.
//       Leaf sway HELD until mode 0 PASSes.
//   Move bind: ?Move@dBgW@@QEAAXXZ (imports.lib), not UEAA.
// ============================================================================
void* wwParseModelOnce(void* raw, const char* arc, int index);  // defined below

const char kLwoodArc[] = "Lwood";
// Authority: D:\XXXXXXX\WW DP\assets\GZLE01\res\Object\Lwood.h
const int kLwoodBdlIndex = 4;  // dRes_INDEX_LWOOD_BDL_ALWD_e
const int kLwoodDzbIndex = 7;  // dRes_INDEX_LWOOD_DZB_ALWD_e — NOT 5 (R4)
const unsigned int kOffCurrentAngle = 1472;  // current.pos@1460 + 12
const unsigned int kTevTypeBg0 = 1;          // R4: 0 is TEV_TYPE_ACTOR
const unsigned int kLwoodOffHeap = 448;      // PDB; kOffHeap defined later

struct LwoodMembers {
    unsigned char phs[0x80];
    J3DModel* model;
    void* bgw;
    float mtx[3][4];
    short timer;
    short pad;
    float yureScale;
};

LwoodMembers* lwoodMembers(void* self) {
    return reinterpret_cast<LwoodMembers*>(static_cast<char*>(self) + kActorBaseSize);
}

void lwoodSetMoveBgMtx(void* self) {
    LwoodMembers* m = lwoodMembers(self);
    if (s_fnMtxTransS == nullptr || s_fnMtxZXYrotM == nullptr || s_fnMtxScaleM == nullptr ||
        s_fnMtxGet == nullptr) {
        return;
    }
    const float* pos =
        reinterpret_cast<const float*>(static_cast<const char*>(self) + kOffCurrentPos);
    const short* ang =
        reinterpret_cast<const short*>(static_cast<const char*>(self) + kOffCurrentAngle);
    const float* scl =
        reinterpret_cast<const float*>(static_cast<const char*>(self) + kOffScale);
    s_fnMtxTransS(pos[0], pos[1], pos[2]);
    s_fnMtxZXYrotM(ang[0], ang[1], ang[2]);
    s_fnMtxScaleM(scl[0], scl[1], scl[2]);
    std::memcpy(m->mtx, s_fnMtxGet(), sizeof(m->mtx));
}

void lwoodSetMtx(void* self) {
    LwoodMembers* m = lwoodMembers(self);
    if (m->model == nullptr || s_fnMtxTransS == nullptr || s_fnMtxZXYrotM == nullptr ||
        s_fnMtxGet == nullptr) {
        return;
    }
    const float* pos =
        reinterpret_cast<const float*>(static_cast<const char*>(self) + kOffCurrentPos);
    const short* ang =
        reinterpret_cast<const short*>(static_cast<const char*>(self) + kOffCurrentAngle);
    const float* scl =
        reinterpret_cast<const float*>(static_cast<const char*>(self) + kOffScale);
    Vec scale = {scl[0], scl[1], scl[2]};
    m->model->setBaseScale(scale);
    s_fnMtxTransS(pos[0], pos[1], pos[2]);
    s_fnMtxZXYrotM(ang[0], ang[1], ang[2]);
    m->model->setBaseTRMtx(*reinterpret_cast<Mtx*>(s_fnMtxGet()));
}

int WwLwood_isDelete(void*);
static int lwoodDrawMode();
static void lwoodCreateFlags(unsigned int* modelFlag, unsigned int* differedFlag);

int WwLwood_solidHeapCB(void* self) {
    LwoodMembers* m = lwoodMembers(self);
    if (s_fnGetObjRes == nullptr || s_fnJ3DModelCreateC == nullptr || s_fnDBgWCtor == nullptr ||
        s_fnBgwSet == nullptr) {
        logf(LOG_LEVEL_ERROR, "[WwRegistry] {\"ev\":\"lwood_heap\",\"ok\":0,\"why\":\"bind\"}");
        return 0;
    }
    void* raw = s_fnGetObjRes(kLwoodArc, kLwoodBdlIndex);
    void* parsed = wwParseModelOnce(raw, kLwoodArc, kLwoodBdlIndex);
    if (parsed == nullptr) {
        logf(LOG_LEVEL_ERROR,
             "[WwRegistry] {\"ev\":\"lwood_heap\",\"ok\":0,\"why\":\"bdl\",\"idx\":%d}",
             kLwoodBdlIndex);
        return 0;
    }
    // Donor: mDoExt_J3DModel__create(modelData, 0x80000, 0x11000022)
    unsigned int modelFlag = 0x80000u;
    unsigned int differedFlag = 0x11000022u;
    lwoodCreateFlags(&modelFlag, &differedFlag);
    m->model = static_cast<J3DModel*>(s_fnJ3DModelCreateC(parsed, modelFlag, differedFlag));
    if (m->model == nullptr) {
        logf(LOG_LEVEL_ERROR, "[WwRegistry] {\"ev\":\"lwood_heap\",\"ok\":0,\"why\":\"model\"}");
        return 0;
    }
    m->model->setUserArea(reinterpret_cast<uintptr_t>(self));
    lwoodSetMoveBgMtx(self);

    void* dzb = s_fnGetObjRes(kLwoodArc, kLwoodDzbIndex);
    if (dzb == nullptr) {
        logf(LOG_LEVEL_ERROR,
             "[WwRegistry] {\"ev\":\"lwood_heap\",\"ok\":0,\"why\":\"dzb\",\"idx\":%d}",
             kLwoodDzbIndex);
        return 0;
    }
    if (s_fnConvDzb != nullptr) {
        void* conv = s_fnConvDzb(dzb);
        if (conv != nullptr) {
            dzb = conv;
        }
    }
    wwDzbRepackAttributes(dzb, "alwd.dzb");
    void* bgw = std::calloc(1, 0x400);
    if (bgw == nullptr) {
        return 0;
    }
    s_fnDBgWCtor(bgw);
    if (s_fnBgwSet(bgw, dzb, 0x1u /* MOVE_BG_e */, m->mtx)) {
        std::free(bgw);
        logf(LOG_LEVEL_ERROR, "[WwRegistry] {\"ev\":\"lwood_heap\",\"ok\":0,\"why\":\"bgw_set\"}");
        return 0;
    }
    m->bgw = bgw;
    J3DModelData* md = static_cast<J3DModelData*>(parsed);
    logf(LOG_LEVEL_INFO,
         "[WwRegistry] {\"ev\":\"lwood_heap\",\"ok\":1,\"bdl\":%d,\"dzb\":%d,"
         "\"locked\":%d,\"flags\":\"0x%08X\","
         "\"reads\":\"shared ModelData (WW); BDL flags from WW_BDL_CONSUME\"}",
         kLwoodBdlIndex, kLwoodDzbIndex,
         (md != nullptr && md->isLocked()) ? 1 : 0, wwBdlLoadFlags());
    return 1;
}

void WwLwood_createInit(void* self) {
    LwoodMembers* m = lwoodMembers(self);
    if (m->model == nullptr) {
        return;
    }
    // R5 — full CreateInit body (not in method table).
    if (s_fnSetMtx != nullptr) {
        s_fnSetMtx(self, m->model->getBaseTRMtx());
    }
    if (s_fnCullBox != nullptr) {
        s_fnCullBox(self, -600.0f, -0.0f, -600.0f, 600.0f, 900.0f, 600.0f);
    }
    if (s_fnCullFar != nullptr) {
        s_fnCullFar(self, 2.37f);
    }
    if (s_fnRndF != nullptr) {
        m->timer = (short)s_fnRndF(0x8000);
        m->yureScale = s_fnRndF(0.4f) + 0.8f;
    } else {
        m->timer = 0;
        m->yureScale = 1.0f;
    }
    // Leaf sway callback HELD — see banner. Joint name still logged for proof.
    {
        J3DModelData* data = m->model->getModelData();
        JUTNameTab* names = data != nullptr ? data->getJointName() : nullptr;
        int found = 0;
        if (names != nullptr && data != nullptr) {
            for (u16 i = 0; i < data->getJointNum(); i++) {
                const char* n = names->getName(i);
                if (n != nullptr && std::strcmp(n, "J_Alwd_ha") == 0) {
                    found = 1;
                    break;
                }
            }
        }
        logf(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"lwood_init\",\"leaf_joint\":%d,\"leaf_sway\":0,"
             "\"reads\":\"CreateInit full; sway HELD until draw PASS\"}",
             found);
    }
    if (s_fnModelCalc != nullptr) {
        s_fnModelCalc(m->model);
    }
    // DN-1 sanctioned Regist — no room stamp.
    if (s_bgspInstance != nullptr && s_fnBgsRegist != nullptr && m->bgw != nullptr) {
        s_fnBgsRegist(s_bgspInstance, m->bgw, self);
        if (s_fnSetPriority != nullptr) {
            s_fnSetPriority(m->bgw, 1);
        }
    }
    lwoodSetMtx(self);
    if (s_fnBgwMove != nullptr && m->bgw != nullptr) {
        s_fnBgwMove(m->bgw);
    }
}

int WwLwood_create(void* self) {
    WwTagSo_ct(self);
    LwoodMembers* m = lwoodMembers(self);
    std::memset(m, 0, sizeof(*m));
    if (s_fnResLoad == nullptr || s_fnEntrySolidHeap == nullptr) {
        return 5;
    }
    const int rt = s_fnResLoad(m->phs, kLwoodArc);
    if (rt != 4) {
        return rt;
    }
    if (!s_fnEntrySolidHeap(self, WwLwood_solidHeapCB, 0x0e40u)) {
        logf(LOG_LEVEL_ERROR, "[WwRegistry] {\"ev\":\"lwood_create\",\"ok\":0,\"why\":\"heap\"}");
        return 5;
    }
    WwLwood_createInit(self);
    static int created = 0;
    if (created < 8) {
        created++;
        const float* pos =
            reinterpret_cast<const float*>(static_cast<const char*>(self) + kOffCurrentPos);
        logf(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"lwood_created\",\"n\":%d,\"pos\":[%.1f,%.1f,%.1f],"
             "\"bgw\":\"%p\",\"model\":\"%p\"}",
             created, pos[0], pos[1], pos[2], m->bgw, (void*)m->model);
    }
    return 4;
}

int WwLwood_delete(void* self) {
    LwoodMembers* m = lwoodMembers(self);
    if (m->bgw != nullptr && s_bgspInstance != nullptr && s_fnBgsRelease != nullptr) {
        const unsigned char* heap =
            reinterpret_cast<const unsigned char*>(static_cast<const char*>(self) + kLwoodOffHeap);
        // Donor: if (heap != NULL) Release — heap ptr at actor+448
        void* heapPtr = *reinterpret_cast<void* const*>(heap);
        if (heapPtr != nullptr) {
            s_fnBgsRelease(s_bgspInstance, m->bgw);
        }
        std::free(m->bgw);
        m->bgw = nullptr;
    }
    wwInvalidateParsedModels(kLwoodArc);
    wwInvalidateParsedAnims(kLwoodArc);
    if (s_fnResDelete != nullptr) {
        s_fnResDelete(m->phs, kLwoodArc);
    }
    return 1;
}

int WwLwood_execute(void* self) {
    lwoodMembers(self)->timer++;
    return 1;
}

int WwLwood_isDelete(void*) { return 1; }

// Draw bisect (SPEC-patcher-requirements / HANDOFF §7.4). Dawn device-lost
// after CPU draw returns — no Fault marker. Env WW_LWOOD_DRAW:
//   0 full · 1 none (PASS 133253) · 2 UpdateDL only (FAIL 134152) ·
//   3 tev+lists, no submit · 4 lists+UpdateDL, no tev ·
//   5 lists+UpdateDL once then skip (PASS 134723) ·
//   6 hide shapes (skips MatPacket entry entirely — no SharedDL, no drawFast)
//   7 = mode4 + WW_LWOOD_SKIP_PATCH implied at load (see wwSetToonTex)
//   8 = mode4 but only 2 submits (n=2 probe; 3 dies)
//   9 = mode4; create uses UseSharedDL not Differed (see lwoodCreateFlags)
//  10 = mode4 but SharedDL mSize forced 0 (shapes still drawFast)
//  11 = mode4 but ShapePacket Hidden (SharedDL callDL still runs; no drawFast)
// Default 1 — playable Outset while Alwd draw is bisected. Fork ExtNpc ≠ mode.
static int lwoodDrawMode() {
    static int s_mode = -1;
    if (s_mode < 0) {
        const char* e = std::getenv("WW_LWOOD_DRAW");
        s_mode = (e != nullptr && e[0] != '\0') ? std::atoi(e) : 1;
        if (s_mode < 0 || s_mode > 18) {
            s_mode = 1;
        }
        logf(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"lwood_draw_mode\",\"mode\":%d,"
             "\"reads\":\"0=full 1=none 2=UDL 3=tev 4=lists+UDL 5=one-submit "
             "6=hideAll(no entry) 7=skipPatch 8=two-submit 9=noDifferedCreate "
             "10=zeroSharedDL 11=hideShapePackets 12=entryDL(receiver submitter) "
             "13=census(full submit, chain logged BEFORE it) "
             "14=restoreShapePacket(entryIn contract, PROVEN NO-OP) "
             "15=probe set (H1-H11) 16=FIX CANDIDATE: clear the stale next; "
             "default 1 (playable; room MDL3 same class survives)\"}",
             s_mode);
    }
    return s_mode;
}

// Create-flag bisect (WW donor is 0x80000 + 0x11000022). Mode 9 / env
// WW_LWOOD_CREATE=shared forces UseSharedDL (0x20000) so DifferedDL is not
// allocated — diagnostic only; not WW-faithful for ship.
static void lwoodCreateFlags(unsigned int* modelFlag, unsigned int* differedFlag) {
    *modelFlag = 0x80000u;
    *differedFlag = 0x11000022u;
    const int mode = lwoodDrawMode();
    const char* e = std::getenv("WW_LWOOD_CREATE");
    const bool wantShared =
        mode == 9 || (e != nullptr && (std::strcmp(e, "shared") == 0 || std::strcmp(e, "9") == 0));
    if (wantShared) {
        *modelFlag = 0x20000u;  // J3DMdlFlag_UseSharedDL
        *differedFlag = 0u;
        logf(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"lwood_create_flags\",\"model\":\"0x20000\","
             "\"differed\":\"0\",\"reads\":\"diagnostic UseSharedDL; no DifferedDL\"}");
    } else if (e != nullptr && std::strcmp(e, "bgdiff") == 0) {
        *differedFlag = 0x11000084u;
        logf(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"lwood_create_flags\",\"model\":\"0x80000\","
             "\"differed\":\"0x11000084\",\"reads\":\"daBg-like differed bits\"}");
    }
}

static void lwoodHideAllShapes(J3DModel* model) {
    if (model == nullptr) {
        return;
    }
    J3DModelData* data = model->getModelData();
    if (data == nullptr) {
        return;
    }
    const u16 n = data->getShapeNum();
    for (u16 i = 0; i < n; i++) {
        J3DShape* shp = data->getShapeNodePointer(i);
        if (shp != nullptr) {
            shp->hide();
        }
    }
}

// Mode 10: keep MatPacket entry + drawFast, but SharedDL callDL writes 0 bytes.
// ============================================================
// Mode 10 arm — REPORTS WHAT IT DID, because a silent arm makes
// the bisect unreadable in BOTH directions: a PASS would read as
// "SharedDL was the killer" and a FAIL as "so it is the vtx path",
// when the honest third case is "this changed nothing." The caller
// used to log `lwood_zero_shareddl` unconditionally after this ran,
// so all three looked identical. Returns the zeroed count; the
// caller shouts when it is 0. (Alwd carries 448 B SharedDL per
// material — see _research/alwd_mdl3_decode.txt — so 0 is a defect,
// not a valid reading.)
// ============================================================
static int lwoodZeroSharedDlSizes(J3DModel* model, int* outMats, unsigned* outBytes) {
    if (outMats != nullptr) {
        *outMats = 0;
    }
    if (outBytes != nullptr) {
        *outBytes = 0;
    }
    if (model == nullptr) {
        return -1;
    }
    J3DModelData* data = model->getModelData();
    if (data == nullptr) {
        return -2;
    }
    int zeroed = 0;
    const u16 n = data->getMaterialNum();
    if (outMats != nullptr) {
        *outMats = (int)n;
    }
    for (u16 i = 0; i < n; i++) {
        J3DMaterial* mat = data->getMaterialNodePointer(i);
        if (mat == nullptr) {
            continue;
        }
        J3DDisplayListObj* dl = mat->getSharedDisplayListObj();
        if (dl != nullptr) {
            if (outBytes != nullptr) {
                *outBytes += (unsigned)dl->mSize;
            }
            dl->mSize = 0;
            zeroed++;
        }
    }
    return zeroed;
}

// Mode 11: SharedDL still callDL'd from MatPacket::draw; ShapePacket::drawFast
// early-outs on J3DShpFlag_Hidden (packet flag, not shape Visible).
// ============================================================
// Mode 11 arm — per-INSTANCE (ShapePackets belong to the J3DModel,
// not the shared ModelData), and it now returns the packet count so
// the caller can prove EVERY instance was armed. The kill needs
// n>=3 trees submitting, so "armed once, logged once" cannot tell
// an armed fleet from one armed tree and two live ones.
// ============================================================
static int lwoodHideShapePackets(J3DModel* model) {
    if (model == nullptr) {
        return -1;
    }
    J3DModelData* data = model->getModelData();
    if (data == nullptr) {
        return -2;
    }
    int hidden = 0;
    const u16 n = data->getShapeNum();
    for (u16 i = 0; i < n; i++) {
        J3DShapePacket* pkt = model->getShapePacket(i);
        if (pkt != nullptr) {
            pkt->onFlag(J3DShpFlag_Hidden);
            hidden++;
        }
    }
    return hidden;
}

// Defined further down with the other GX census pieces; declared here because
// WwLwood_draw is the tick site and sits above that definition.
void wwGxCensusTick();
extern int s_lwoodDrawsThisFrame;

#include "ww_j3d_mesh.inc"

int WwLwood_draw(void* self) {
    LwoodMembers* m = lwoodMembers(self);
    if (m->model == nullptr) {
        return 1;
    }
    const int mode = lwoodDrawMode();

    wwGxCensusTick();   // GX traffic census - counters only, no behaviour change

    // ------------------------------------------------------------------------
    // THE BRANCH PROBE — which arm of mDoExt_modelUpdateDL does lwood take?
    //
    // Prompted by the user's question: WW waves, stars and grass all render, so
    // why is lwood alone a problem? That reframes it — the difficulty is not
    // "WW content cannot render here", it is specific to lwood, and Room 44's
    // own model.bdl is ALSO a donor J3D model that draws every frame without
    // complaint.
    //
    // `mDoExt_modelUpdateDL` (m_Do_ext.cpp:334) chooses:
    //     sharedDL(mat0) != NULL && !isLocked()  ->  calc(); diff(); entry();
    //     otherwise                              ->  unlock(); update(); lock();
    //
    // THE SECOND ARM CALLS update(), WHICH REGENERATES THE MODEL'S DISPLAY
    // LISTS. If the condition stays false, that happens EVERY FRAME - a display
    // list rebuilt per model per frame, into a buffer whose overflow is exactly
    // the crash we have been chasing. The first arm only re-enters an existing
    // packet and costs nothing.
    //
    // Two booleans decide it and neither has ever been measured. This logs them.
    // ------------------------------------------------------------------------
    {
        static int s_branchN = 0;
        s_branchN++;
        if (s_branchN <= 4) {
            J3DModelData* md = m->model->getModelData();
            J3DMaterial* mat0 = (md != nullptr) ? md->getMaterialNodePointer(0) : nullptr;
            const int hasShared =
                (mat0 != nullptr && mat0->getSharedDisplayListObj() != nullptr) ? 1 : 0;
            const int locked = (md != nullptr && md->isLocked()) ? 1 : 0;
            logf(LOG_LEVEL_INFO,
                 "[WwRegistry] {\"ev\":\"lwood_dl_branch\",\"n\":%d,\"shared_dl0\":%d,"
                 "\"locked\":%d,\"takes\":\"%s\","
                 "\"reads\":\"THE WHOLE QUESTION IN TWO BOOLEANS. takes=diff means the cheap "
                 "arm: calc/diff/entry, re-enters an existing packet, writes no display list. "
                 "takes=UPDATE means unlock/update/lock - IT REGENERATES THE DISPLAY LIST EVERY "
                 "FRAME, which is a per-model-per-frame writer into the very buffer whose "
                 "overflow kills this actor at draw 44. If lwood reads UPDATE and Room 44's "
                 "model reads diff, that is the difference between them and the bypass was "
                 "never needed\"}",
                 s_branchN, hasShared, locked,
                 (hasShared != 0 && locked == 0) ? "diff" : "UPDATE");
        }
    }

    // ------------------------------------------------------------------------
    // MODE 17 — THE GfxService ROUTE. The actual lwood test.
    //
    // This is the whole point of the spike: the GX submit below dies on a
    // `fifo::write_data: buffer size overflow`, and mode 0 proved a
    // FORK-IDENTICAL draw shape dies too, so the draw shape is not the variable.
    // Mode 17 delivers the same instances, at the same world positions, on the
    // same frame cadence, through a route that never touches `aurora::gx::fifo`.
    //
    // It RETURNS EARLY: no `dComIfGd_setListBG`, no `modelUpdateDL`, nothing
    // enters the GX path for this actor. A mode that did both would prove
    // nothing, because the GX submit alone is enough to trigger the overflow.
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    // MODE 18 — THE REAL LWOOD MESH THROUGH GfxService.
    //
    // Mode 17 answered "does the route work" with a 36-corner box that uploaded
    // nothing. That was the wrong test to stop on: the GX failure is a DATA
    // VOLUME failure, and a box exercises almost none of the data path. This
    // mode decodes the model's own GX display lists into a triangle list and
    // uploads it per instance per frame — deliberately the naive shape, because
    // stressing the buffer is the entire point.
    //
    // Decode is CACHED per J3DModelData: display lists are immutable resource
    // data, so all four lwood share one decode. The per-frame cost is the
    // UPLOAD, which is what we want to measure.
    // ------------------------------------------------------------------------
    if (mode == 18) {
        static int s_meshN = 0;
        // 64k floats = ~7.2k triangles; an lwood is far under this. If the cap is
        // ever hit the receipt says so, because a silently truncated tree that
        // renders fine is exactly how a decoder bug survives review.
        WwMeshCache* mesh = wwDecodeModelMesh(m->model, 64 * 1024);
        bool ok = false;
        if (mesh != nullptr && mesh->triCount > 0) {
            ok = wwGfxSpike_submitMesh(
                mesh->verts, mesh->triCount * 3,
                reinterpret_cast<const float*>(m->model->getBaseTRMtx()));
        }
        s_meshN++;
        if (s_meshN <= 4 || (s_meshN % 900) == 0) {
            logf(LOG_LEVEL_INFO,
                 "[WwRegistry] {\"ev\":\"lwood_mesh_decode\",\"n\":%d,\"tris\":%d,"
                 "\"shapes\":%d,\"prims\":%d,\"truncated\":%d,\"submitted\":%d,"
                 "\"bad_op\":\"0x%02X\",\"bad_at\":%d,"
                 "\"pos_comps\":%d,\"pos_type\":%d,\"pos_stride\":%u,"
                 "\"oob_pos\":%d,"
                 "\"dec_lo\":[%.1f,%.1f,%.1f],\"dec_hi\":[%.1f,%.1f,%.1f],\"dec_ok\":%d,"
                 "\"raw\":[%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f],\"raw_ok\":%d,"
                 "\"perm\":[%d,%d,%d],\"perm_resid\":%d,"
                 "\"shapes_total\":%d,\"shapes_skipped\":%d,\"groups\":%d,\"pnmtxidx\":%d,"
                 "\"joint_applied\":%d,"
                 "\"bbox_lo\":[%.1f,%.1f,%.1f],\"bbox_hi\":[%.1f,%.1f,%.1f],"
                 "\"reads\":\"THE BBOX IS THE ORACLE. An lwood should measure roughly +/-200 "
                 "wide and ~600 tall. Tens of thousands, or wild asymmetry, means the position "
                 "array was read with the wrong endianness or fixed-point frac and the geometry "
                 "is noise - which would still RENDER, just as a scribble. tris=0 means the "
                 "display lists were not understood at all - and bad_op then names the exact GX "
                 "opcode that stopped the walk, which is the difference between a decoder bug "
                 "and a model with no geometry. dec_lo/dec_hi are the MODEL'S OWN DECLARED bounds "
                 "from the BDL and owe NOTHING to my decoder: if they match bbox_lo/bbox_hi the "
                 "decode is faithful and the fault is downstream, if they differ the decoder is "
                 "still lying\"}",
                 s_meshN, mesh != nullptr ? mesh->triCount : -1,
                 mesh != nullptr ? mesh->shapes : -1, mesh != nullptr ? mesh->prims : -1,
                 mesh != nullptr ? mesh->truncated : -1, ok ? 1 : 0,
                 mesh != nullptr ? mesh->badOp : 0, mesh != nullptr ? mesh->badAt : -1,
                 mesh != nullptr ? mesh->posComps : -1, mesh != nullptr ? mesh->posType : -1,
                 mesh != nullptr ? (unsigned)mesh->posStride : 0u,
                 mesh != nullptr ? mesh->oobPos : -1,
                 mesh != nullptr ? mesh->decLo[0] : 0.0f, mesh != nullptr ? mesh->decLo[1] : 0.0f,
                 mesh != nullptr ? mesh->decLo[2] : 0.0f, mesh != nullptr ? mesh->decHi[0] : 0.0f,
                 mesh != nullptr ? mesh->decHi[1] : 0.0f, mesh != nullptr ? mesh->decHi[2] : 0.0f,
                 mesh != nullptr ? mesh->decValid : -1,
                 mesh != nullptr ? mesh->raw[0] : 0.0f, mesh != nullptr ? mesh->raw[1] : 0.0f,
                 mesh != nullptr ? mesh->raw[2] : 0.0f, mesh != nullptr ? mesh->raw[3] : 0.0f,
                 mesh != nullptr ? mesh->raw[4] : 0.0f, mesh != nullptr ? mesh->raw[5] : 0.0f,
                 mesh != nullptr ? mesh->raw[6] : 0.0f, mesh != nullptr ? mesh->raw[7] : 0.0f,
                 mesh != nullptr ? mesh->raw[8] : 0.0f, mesh != nullptr ? mesh->rawOk : -1,
                 mesh != nullptr ? mesh->perm[0] : -1, mesh != nullptr ? mesh->perm[1] : -1,
                 mesh != nullptr ? mesh->perm[2] : -1, mesh != nullptr ? mesh->permScore : -1,
                 mesh != nullptr ? mesh->shapesTotal : -1,
                 mesh != nullptr ? mesh->shapesSkipped : -1,
                 mesh != nullptr ? mesh->groupsTotal : -1,
                 mesh != nullptr ? mesh->usesPnMtxIdx : -1,
                 mesh != nullptr ? mesh->jointApplied : -1,
                 mesh != nullptr ? mesh->lo[0] : 0.0f, mesh != nullptr ? mesh->lo[1] : 0.0f,
                 mesh != nullptr ? mesh->lo[2] : 0.0f, mesh != nullptr ? mesh->hi[0] : 0.0f,
                 mesh != nullptr ? mesh->hi[1] : 0.0f, mesh != nullptr ? mesh->hi[2] : 0.0f);
        }
        return 1;
    }

    if (mode == 17) {
        static int s_gfxN = 0;
        const bool ok =
            wwGfxSpike_submitLwood(reinterpret_cast<const float*>(m->model->getBaseTRMtx()));
        s_gfxN++;
        if (s_gfxN <= 3 || (s_gfxN % 600) == 0) {
            logf(LOG_LEVEL_INFO,
                 "[WwRegistry] {\"ev\":\"lwood_draw_gfx\",\"n\":%d,\"submitted\":%d,"
                 "\"reads\":\"mode 17: this instance went to GfxService and NOT to GX. "
                 "submitted=0 means the spike declined it and lwood_gfx_reject says why; a run "
                 "of zeros here is a clean run that proves NOTHING about the FIFO\"}",
                 s_gfxN, ok ? 1 : 0);
        }
        return 1;
    }
    if (mode == 1) {
        static int s_skipN = 0;
        s_skipN++;
        if (s_skipN <= 3 || (s_skipN % 300) == 0) {
            logf(LOG_LEVEL_INFO,
                 "[WwRegistry] {\"ev\":\"lwood_draw_skip\",\"n\":%d,\"mode\":1}",
                 s_skipN);
        }
        return 1;
    }

    static int s_submitBudget = -1;
    if (s_submitBudget < 0) {
        if (mode == 5) {
            s_submitBudget = 1;
        } else if (mode == 8) {
            s_submitBudget = 2;
        } else {
            s_submitBudget = 1000000;
        }
    }

    dKy_tevstr_c* tev =
        reinterpret_cast<dKy_tevstr_c*>(static_cast<char*>(self) + kLwoodOffTevStr);
    cXyz* pos = reinterpret_cast<cXyz*>(static_cast<char*>(self) + kOffCurrentPos);
    const bool doTev = (mode == 0 || mode == 3);
    const bool doLists =
        (mode == 0 || mode == 3 || mode == 4 || mode == 5 || mode == 7 || mode == 8 ||
         mode == 9 || mode == 10 || mode == 11 || mode == 12 || mode == 13 ||
         mode == 14 || mode == 15 ||
         mode == 16);
    // Mode 6: hide shapes only — do NOT UpdateDL. 162313 proved UpdateDL while
    // hidden survives Dawn but spams frame_interp (viewMtx stretch / input lag).
    bool doSubmit =
        (mode == 0 || mode == 2 || mode == 4 || mode == 5 || mode == 7 || mode == 8 ||
         mode == 9 || mode == 10 || mode == 11 || mode == 12 || mode == 13 ||
         mode == 14 || mode == 15 ||
         mode == 16);
    if (doSubmit && s_submitBudget <= 0) {
        doSubmit = false;
        static int s_capSkip = 0;
        s_capSkip++;
        if (s_capSkip <= 3 || (s_capSkip % 300) == 0) {
            logf(LOG_LEVEL_INFO,
                 "[WwRegistry] {\"ev\":\"lwood_draw_cap\",\"n\":%d,\"mode\":%d}",
                 s_capSkip, mode);
        }
        return 1;
    }
    if (mode == 6) {
        static int s_hid = 0;
        if (s_hid == 0) {
            lwoodHideAllShapes(m->model);
            s_hid = 1;
            logf(LOG_LEVEL_INFO,
                 "[WwRegistry] {\"ev\":\"lwood_hide_all\",\"shapes\":%u}",
                 m->model->getModelData() != nullptr
                     ? (unsigned)m->model->getModelData()->getShapeNum()
                     : 0u);
        }
    }
    if (mode == 10) {
        // Once is correct HERE and only here: SharedDL hangs off the SHARED
        // ModelData (lwood_heap: "shared ModelData (WW)"), so zeroing it once
        // covers every instance. Contrast mode 11 below, which must arm each.
        static int s_z = 0;
        if (s_z == 0) {
            s_z = 1;
            int mats = 0;
            unsigned bytes = 0;
            const int zeroed = lwoodZeroSharedDlSizes(m->model, &mats, &bytes);
            logf(zeroed > 0 ? LOG_LEVEL_INFO : LOG_LEVEL_WARN,
                 "[WwRegistry] {\"ev\":\"lwood_zero_shareddl\",\"zeroed\":%d,"
                 "\"mats\":%d,\"bytes_was\":%u,\"armed\":%d,\"reads\":\"mode10; "
                 "zeroed=0 means THE ARM DID NOTHING and neither a PASS nor a FAIL "
                 "may be read as evidence about SharedDL\"}",
                 zeroed, mats, bytes, zeroed > 0 ? 1 : 0);
        }
    }
    if (mode == 11) {
        // ShapePackets are per J3DModel — must arm every instance, not once.
        const int hidden = lwoodHideShapePackets(m->model);
        static int s_hpCalls = 0;
        static int s_hpPkts = 0;
        static int s_hpFail = 0;
        s_hpCalls++;
        if (hidden > 0) {
            s_hpPkts += hidden;
        } else {
            s_hpFail++;
        }
        // The kill needs n>=3 submitting trees, so the fleet count is the
        // reading — one armed tree beside two live ones looks like "armed".
        if (s_hpCalls <= 4 || (s_hpCalls % 300) == 0 || hidden <= 0) {
            logf(s_hpFail == 0 ? LOG_LEVEL_INFO : LOG_LEVEL_WARN,
                 "[WwRegistry] {\"ev\":\"lwood_hide_shapepackets\",\"call\":%d,"
                 "\"hidden\":%d,\"pkts_total\":%d,\"unarmed_calls\":%d,"
                 "\"reads\":\"mode11; unarmed_calls>0 means some instance still "
                 "draws its shapes and a PASS is not attributable\"}",
                 s_hpCalls, hidden, s_hpPkts, s_hpFail);
        }
    }
    if (doTev) {
        dKyWw_settingTevStruct((int)kTevTypeBg0, pos, tev);
        dKyWw_setLightTevColorType(m->model, tev);
    }
    if (doLists) {
        dComIfGd_setListBG();
    }
    // ============================================================
    // MODE 13 - THE PACKET-CHAIN CENSUS, LOGGED BEFORE THE SUBMIT
    // ============================================================
    // Modes 10, 11 and 12 all FAILED at n=3: not SharedDL content, not
    // drawFast, not the submitter. The one mode that PASSES while models
    // still exist is 6 - and 6 is the only one that skips the MatPacket
    // ENTRY. So the suspect is the entry itself. MatPackets are per
    // J3DModel (`J3DModel::mMatPacket`, 0xC0), but J3DDrawBuffer MERGES
    // packets it considers `isSame`, and all lwood instances share one
    // ModelData - so they share MATERIALS. This census reports whether
    // the chain the third instance enters is already circular.
    // Logged BEFORE the submit on purpose: the run dies inside it, and a
    // receipt written after the call is a receipt that never arrives.
    // ============================================================
    // ============================================================
    // MODE 14 - RESTORE THE PACKET'S OWN SHAPE, PER FRAME
    // ============================================================
    // ROOT CAUSE, read out of the receiver's own source rather than guessed:
    //   * J3DDrawBuffer::entryMatSort MERGES packets whose material isSame -
    //     `packet->addShapePacket(pMatPacket->getShapePacket())` - and
    //     addShapePacket PREPENDS without clearing.
    //   * The per-frame reset that makes that safe lives in
    //     J3DJoint::entryIn(): `matPacket->setShapePacket(shapePacket)`
    //     restores each MatPacket to ITS OWN shape before entry.
    //   * That joint walk runs on the `i_model->entry()` branch. lwood does
    //     NOT take it: mDoExt_modelEntryDL/UpdateDL pick `mDoExt_modelDiff`
    //     when `SharedDL != NULL && !modelData->isLocked()`, and lwood is
    //     exactly that - `lwood_heap` reports **locked:0** and mode 10
    //     measured 448 B of SharedDL per material.
    //   * So the chain is never reset, each frame prepends onto the last
    //     frame's chain, and it eventually CLOSES - census `cycle:1`. A draw
    //     walker on a circular list never returns: the GX FIFO grows until
    //     `fifo::write_data: buffer size overflow` at 4 GB.
    // The fix is the receiver's own line, applied at the consume boundary -
    // not a new invention, and not a guard around the symptom.
    // ============================================================
    if (mode == 14) {
        struct OwnShape { void* model; void* mat[4]; void* shape[4]; };
        static OwnShape s_own[8];
        static int s_ownN = 0;
        J3DModelData* md14 = m->model->getModelData();
        const u16 nm14 = (md14 != nullptr) ? md14->getMaterialNum() : 0;
        OwnShape* rec = nullptr;
        for (int k = 0; k < s_ownN; k++) {
            if (s_own[k].model == (void*)m->model) { rec = &s_own[k]; break; }
        }
        if (rec == nullptr && s_ownN < 8) {
            // FIRST SIGHT: the packet still points at its OWN shape, because
            // nothing has merged into it yet. That is the value to restore.
            rec = &s_own[s_ownN++];
            rec->model = (void*)m->model;
            for (int i = 0; i < 4; i++) { rec->mat[i] = nullptr; rec->shape[i] = nullptr; }
            for (u16 i = 0; i < nm14 && i < 4; i++) {
                J3DMatPacket* mp = m->model->getMatPacket(i);
                rec->mat[i] = (void*)mp;
                rec->shape[i] = (mp != nullptr) ? (void*)mp->getShapePacket() : nullptr;
            }
            logf(LOG_LEVEL_INFO,
                 "[WwRegistry] {\"ev\":\"lwood_shapepkt_learn\",\"model\":\"%p\",\"mats\":%u,"
                 "\"reads\":\"mode14; captured each MatPacket's OWN ShapePacket at first sight\"}",
                 (void*)m->model, (unsigned)nm14);
        }
        if (rec != nullptr) {
            int restored = 0, alreadyOk = 0;
            for (u16 i = 0; i < nm14 && i < 4; i++) {
                J3DMatPacket* mp = m->model->getMatPacket(i);
                if (mp == nullptr || rec->shape[i] == nullptr) { continue; }
                if ((void*)mp->getShapePacket() != rec->shape[i]) {
                    mp->setShapePacket((J3DShapePacket*)rec->shape[i]);
                    restored++;
                } else {
                    alreadyOk++;
                }
            }
            static int s_restN = 0;
            s_restN++;
            if (s_restN <= 4 || (s_restN % 300) == 0) {
                logf(LOG_LEVEL_INFO,
                     "[WwRegistry] {\"ev\":\"lwood_shapepkt_restore\",\"call\":%d,"
                     "\"restored\":%d,\"already\":%d,\"reads\":\"mode14; restored=0 every call "
                     "would mean the chain was never being clobbered and this mode proves nothing\"}",
                     s_restN, restored, alreadyOk);
            }
        }
    }

    // ============================================================
    // MODE 15 - THE TEN-HYPOTHESIS PROBE SET, ONE RUN
    // ============================================================
    // Six single-hypothesis modes cost six boots and produced two
    // retractions. This set instruments every branch that could produce
    // the cycle, so one run discriminates instead of one run eliminating.
    //   H1  PHASE      - draws since the last observed frame turnover
    //   H2  DOUBLE     - draws per instance per frame (>1 = entered twice)
    //   H3  STATE      - MatPacket isLocked / isChanged at entry time
    //   H4  SLOT       - texture identity feeding entryMatSort's hash
    //   H5  SAME       - isSame() against instance 0 (the merge precondition)
    //   H6  STALE      - ShapePacket->getNextPacket() BEFORE the submit
    //   H7  BUFFER     - which draw buffer (opa vs xlu) the material sorts to
    //   H8  MTX        - anm matrix present (entryIn's early paths)
    //   H9  CONTROL    - the SURVIVING room model measured with the SAME fields
    //   H10 ENTRYNUM   - J3DDrawBuffer::entryNum growth per frame
    // H9 is the point: the room model survives every frame on the same
    // backend, so any field where lwood and room AGREE cannot be the cause.
    // ============================================================
    if (mode == 15) {
        static int s_p15 = 0;
        static void* s_firstModel = nullptr;
        static J3DMatPacket* s_firstMat = nullptr;
        static int s_lastEntryNum = 0;
        s_p15++;
        J3DModelData* md15 = m->model->getModelData();
        const u16 nm15 = (md15 != nullptr) ? md15->getMaterialNum() : 0;
        if (s_firstModel == nullptr) {
            s_firstModel = (void*)m->model;
            s_firstMat = (nm15 > 0) ? m->model->getMatPacket(0) : nullptr;
        }
        const int entryNow = J3DDrawBuffer::entryNum;
        // ------------------------------------------------------------
        // H1 (PHASE) and H2 (DOUBLE ENTRY) - REWRITTEN. The first version
        // shipped H1 not at all and H2 as `same_model_as_first`, which is
        // an IDENTITY field, not an entry counter: it could never have
        // shown a model entered twice in one frame. A probe leg that
        // cannot produce the reading it is named for is a placebo, and
        // eliminating on it would retire a hypothesis nobody tested.
        //
        // The frame boundary is `g_Counter.mCounter0` - the receiver's own
        // sim tick, already used by this file's painter gate. NOT
        // J3DDrawBuffer::entryNum: that is only ever ++'d (J3DJoint.cpp:184)
        // and never reset, so it is monotonic and cannot mark a frame.
        // ------------------------------------------------------------
        const unsigned int tick15 = g_Counter.mCounter0;
        static unsigned int s_tickPrev = 0xFFFFFFFFu;
        static int s_drawsThisTick = 0;
        static void* s_seenThisTick[8] = {nullptr};
        static int s_seenN = 0;
        static int s_maxPerTick = 0;
        if (tick15 != s_tickPrev) {
            s_tickPrev = tick15;
            s_drawsThisTick = 0;
            s_seenN = 0;
        }
        s_drawsThisTick++;
        int dupThisTick = 0;
        for (int k = 0; k < s_seenN; k++) {
            if (s_seenThisTick[k] == (void*)m->model) { dupThisTick++; }
        }
        if (s_seenN < 8) { s_seenThisTick[s_seenN++] = (void*)m->model; }
        if (s_drawsThisTick > s_maxPerTick) { s_maxPerTick = s_drawsThisTick; }
        for (u16 i = 0; i < nm15 && i < 2; i++) {
            J3DMatPacket* mp = m->model->getMatPacket(i);
            if (mp == nullptr) { continue; }
            J3DShapePacket* sp = mp->getShapePacket();
            J3DMaterial* mat = mp->getMaterial();
            logf(LOG_LEVEL_INFO,
                 "[WwRegistry] {\"ev\":\"lwood_h15\",\"p\":%d,\"model\":\"%p\",\"mat\":%u,"
                 "\"h1_tick\":%u,\"h1_draws_this_tick\":%d,\"h1_max_per_tick\":%d,"
                 "\"h2_dup_entries_this_tick\":%d,\"h3_locked\":%d,\"h3_changed\":%d,"
                 "\"h4_texno\":%u,\"h5_same_as_first\":%d,\"h6_stale_next\":\"%p\","
                 "\"h7_opa_texedge\":%d,\"h8_has_anm\":%d,\"h10_entrynum\":%d,"
                 "\"h10_delta\":%d,\"reads\":\"mode15 H1-H10; compare EVERY field against the "
                 "lwood_h15_room control - a field where they AGREE cannot be the cause\"}",
                 s_p15, (void*)m->model, (unsigned)i,
                 tick15, s_drawsThisTick, s_maxPerTick, dupThisTick,
                 mp->isLocked() ? 1 : 0,
                 mp->isChanged() ? 1 : 0,
                 (mat != nullptr) ? (unsigned)mat->getTexNo(0) : 0xFFFFu,
                 (s_firstMat != nullptr && mp != s_firstMat) ? (s_firstMat->isSame(mp) ? 1 : 0) : -1,
                 (sp != nullptr) ? (void*)sp->getNextPacket() : nullptr,
                 (mat != nullptr) ? (mat->isDrawModeOpaTexEdge() ? 1 : 0) : -1,
                 // H8 guarded: getAnmMtx() dereferences mMtxBuffer with no null
                 // check of its own. A probe that faults is worse than a probe
                 // missing a field - and this one would have faulted inside the
                 // very draw it was built to observe.
                 (m->model->getMtxBuffer() != nullptr &&
                  m->model->getAnmMtx(0) != nullptr) ? 1 : 0,
                 entryNow, entryNow - s_lastEntryNum);
        }
        s_lastEntryNum = entryNow;
    }

    // ============================================================
    // MODE 16 - THE FIX CANDIDATE. Clear the packet's OWN stale `next`.
    // ============================================================
    // This targets the ONE thing that was actually measured, and nothing
    // it was inferred to mean:
    //   * `h6_stale_next` is NON-NULL on 75/75 post-first-draw samples and
    //     NULL on all 8 first-draw samples - an in-flight transition.
    //   * Followed by identity, those links close: 8 of 8 chains form two
    //     per-material rings (0->2->1->0, model[3] dangling in), 6 of 6
    //     target packets are themselves victims.
    //   * A walker on a circular packet list never returns, which is what
    //     drives `fifo::write_data: buffer size overflow` at 4 GB.
    //
    // WHY THIS IS NOT MODE 14 AGAIN. Mode 14 restored the MatPacket's HEAD
    // (`setShapePacket`) and measured `restored:0, already:2` - a proven
    // no-op, because `J3DJoint::entryIn` already sets the head every frame.
    // It never touched the SHAPE PACKET'S OWN `next`, which is the field
    // the census actually finds non-null. Different field, different site.
    //
    // WHY IT IS THE RECEIVER'S OWN OPERATION, not an invention: this is
    // exactly what `J3DDrawBuffer::entryMatSort` does on the packet it is
    // entering - `pMatPacket->getShapePacket()->drawClear()` - applied at
    // the point where THIS plugin submits, because lwood's instances reach
    // the buffer through a path where that clear lands on a packet that a
    // previous merge has already re-pointed.
    //
    // WHAT IT DOES NOT CLAIM: a cause. The user's standard is a CONFIRMED
    // DRAW ON A NON-CRASHED OUTSET, and this is a candidate until that
    // exists. It reports whether it changed anything, so a PASS cannot be
    // read as "the fix worked" when the field was already NULL.
    // ============================================================
    if (mode == 16) {
        J3DModelData* md16 = m->model->getModelData();
        const u16 nm16 = (md16 != nullptr) ? md16->getMaterialNum() : 0;
        int cleared = 0, alreadyNull = 0;
        for (u16 i = 0; i < nm16 && i < 4; i++) {
            J3DMatPacket* mp = m->model->getMatPacket(i);
            if (mp == nullptr) { continue; }
            J3DShapePacket* sp = mp->getShapePacket();
            if (sp == nullptr) { continue; }
            if (sp->getNextPacket() != nullptr) {
                sp->drawClear();   // the receiver's own call, same as entryMatSort
                cleared++;
            } else {
                alreadyNull++;
            }
        }
        static int s_fixN = 0;
        s_fixN++;
        if (s_fixN <= 4 || (s_fixN % 300) == 0) {
            logf(LOG_LEVEL_INFO,
                 "[WwRegistry] {\"ev\":\"lwood_fix_clear_next\",\"call\":%d,\"cleared\":%d,"
                 "\"already_null\":%d,\"reads\":\"mode16 FIX CANDIDATE. cleared>0 means the "
                 "stale link this run's census measures was actually broken here; cleared=0 on "
                 "every call means the field was already NULL and a PASS says NOTHING about "
                 "the fix - the same no-op trap mode 14 fell into\"}",
                 s_fixN, cleared, alreadyNull);
        }
    }

    // WW_LWOOD_CENSUS=1 rides ANY mode: the census found the merge, so it must
    // travel to the mode that tests the FIX, not stay welded to mode 13.
    static int s_censusEnv = -1;
    if (s_censusEnv < 0) {
        const char* ce = std::getenv("WW_LWOOD_CENSUS");
        s_censusEnv = (ce != nullptr && ce[0] == '1') ? 1 : 0;
    }
    if (mode == 13 || s_censusEnv == 1) {
        static int s_censusN = 0;
        s_censusN++;
        J3DModelData* md = m->model->getModelData();
        const u16 nmat = (md != nullptr) ? md->getMaterialNum() : 0;
        for (u16 i = 0; i < nmat && i < 4; i++) {
            J3DMatPacket* mp = m->model->getMatPacket(i);
            if (mp == nullptr) {
                continue;
            }
            // Walk the packet chain, bounded, and report a repeat as a CYCLE.
            const J3DPacket* seen[24];
            int len = 0;
            bool cycle = false;
            for (const J3DPacket* q = mp->getShapePacket() != nullptr
                                         ? (const J3DPacket*)mp->getShapePacket()
                                         : nullptr;
                 q != nullptr && len < 24; q = q->getNextPacket()) {
                for (int k = 0; k < len; k++) {
                    if (seen[k] == q) { cycle = true; break; }
                }
                if (cycle) { break; }
                seen[len++] = q;
            }
            logf(cycle ? LOG_LEVEL_WARN : LOG_LEVEL_INFO,
                 "[WwRegistry] {\"ev\":\"lwood_pkt_census\",\"draw\":%d,\"model\":\"%p\","
                 "\"mat\":%u,\"matpkt\":\"%p\",\"shapepkt\":\"%p\",\"chain\":%d,"
                 "\"cycle\":%d,\"dl_size\":%u,\"reads\":\"mode13; chain=24 means it hit the "
                 "walk cap, not that the list ends there; cycle=1 is the kill\"}",
                 s_censusN, (void*)m->model, (unsigned)i, (void*)mp,
                 (void*)mp->getShapePacket(), len, cycle ? 1 : 0,
                 (unsigned)(mp->getDisplayListObj() != nullptr ? mp->getDisplayListSize() : 0u));
        }
    }

    // Mode 12 routes through the RECEIVER's own submitter (see s_fnModelEntryDL).
    // Falls back loudly rather than silently drawing via the wrong function: a
    // mode that quietly ran the arm it was meant to replace would read as a
    // result for the path it never took.
    bool entryPath = false;
    if (doSubmit) {
        if (mode == 12) {
            if (s_fnModelEntryDL != nullptr) {
                s_fnModelEntryDL(m->model);
                entryPath = true;
            } else {
                static int s_noEntry = 0;
                if (s_noEntry++ == 0) {
                    logf(LOG_LEVEL_WARN,
                         "[WwRegistry] {\"ev\":\"lwood_entrydl_unresolved\",\"reads\":\"mode12 "
                         "asked for mDoExt_modelEntryDL and the symbol did not resolve - THIS RUN "
                         "SAYS NOTHING ABOUT THE RECEIVER PATH\"}");
                }
            }
        } else if (s_fnModelUpdateDL != nullptr) {
            s_fnModelUpdateDL(m->model);
        }
        if (mode == 5) {
            s_submitBudget--;
        }
    }
    if (doLists) {
        dComIfGd_setList();
    }
    static int s_drawN = 0;
    s_drawN++;
    if (s_drawN <= 3 || (s_drawN % 300) == 0) {
        logf(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"lwood_draw\",\"n\":%d,\"mode\":%d,"
             "\"tev\":%d,\"lists\":%d,\"submit\":%d,\"submit_fn\":\"%s\","
             "\"reads\":\"submit alive; submit_fn names WHICH receiver function ran - "
             "entry = daBg's own path (frame-interp aware), update = the plugin's\"}",
             s_drawN, mode, doTev ? 1 : 0, doLists ? 1 : 0, doSubmit ? 1 : 0,
             entryPath ? "entry" : (doSubmit ? "update" : "none"));
    }
    return 1;
}

WwActorMethodClass s_lwoodMethods = {
    (void*)&WwLwood_create, (void*)&WwLwood_delete, (void*)&WwLwood_execute,
    (void*)&WwLwood_isDelete, (void*)&WwLwood_draw, {0},
};
WwActorProfileDef s_lwoodProfile = {};

// ============================================================================
                                // says 7. Indices are DATA, never inferred.

// Actor fields, PDB-measured like the rest: heap@448, tevStr@496.
const unsigned int kOffHeap   = 448;
const unsigned int kOffTevStr = 496;

typedef void* (*FnBgWNewSet)(void*, unsigned int, void*);
typedef void  (*FnModelSetBaseTR)(void*, void*);
typedef void  (*FnTevSetting)(void*, int, const void*, void*);
typedef void  (*FnTevColorType)(void*, void*, void*);
typedef void  (*FnVoidVoid)();
FnBgWNewSet      s_fnBgWNewSet = nullptr;
FnModelSetBaseTR s_fnSetBaseTRMtx = nullptr;
FnTevSetting     s_fnTevSetting = nullptr;
FnTevColorType   s_fnTevColorType = nullptr;
FnVoidVoid       s_fnSetListBG = nullptr;
FnVoidVoid       s_fnSetList = nullptr;
void* s_envLight = nullptr;

// ========================================================================
// wwParseModelOnce - one parse per raw image, for ALL by-index consumers
// ========================================================================
// Keyed on the raw buffer address, which is stable for the lifetime of the
// loaded arc. A hit returns the previously parsed object; a miss parses with
// the RECEIVER'S OWN loader (never a reimplementation) and records it. The
// table is small and never evicts: an arc that unloads and reloads at a
// different address simply takes a new slot, and a stale slot can only be hit
// by an address that is live again, in which case it is the same image.
// ========================================================================
struct WwParsedModel {
    const char* arc;
    int         index;
    void*       raw;
    void*       parsed;
};
const int kMaxParsedModels = 64;
WwParsedModel s_parsedModels[kMaxParsedModels] = {};
int s_parsedModelCount = 0;

// ========================================================================
// INVALIDATION - because an ADDRESS IS NOT A DURABLE KEY (History/Bridge).
// ========================================================================
// My first version keyed on the raw buffer address alone and I defended it
// in a comment: "a stale slot can only be hit by an address that is live
// again, in which case it is the same image." FALSE, and it is the estate's
// oldest J3D law wearing a new hat - "never free an arc while parsed data is
// cached" (the sumo BMT crash and the room-lane mesh corruption). Free the
// arc and a later allocation reuses the address; the table then answers HIT
// and hands out a J3DModelData* into reused memory. It would not present as
// a parse bug - it would present as random model corruption two stages
// later, which is EXACTLY how the room-lane corruption presented.
//
// And the exposure is not exotic: `WwLwood_delete` itself calls
// dComIfG_resDelete(phs, "Lwood") on room exit, so the dangle happens in
// ordinary play, not only on a stage change.
//
// Two guards, because they cover different failures:
//   (a) the key is (arc, index, raw) - ALL THREE must match. A different
//       arc landing on the same address can no longer collide, because its
//       arc name does not match.
//   (b) explicit invalidation at both release seams - per-arc on resDelete,
//       and the whole table on stage load. This is the one that covers the
//       remaining case (a) cannot: the SAME arc unmounted and remounted at
//       the SAME address, where all three key fields still match but the
//       parsed object is dead.
// ========================================================================
void wwInvalidateParsedModels(const char* arc) {
    if (arc == nullptr) { return; }
    int w = 0;
    int dropped = 0;
    for (int i = 0; i < s_parsedModelCount; i++) {
        const bool same = (s_parsedModels[i].arc == arc) ||
                          (s_parsedModels[i].arc != nullptr &&
                           std::strcmp(s_parsedModels[i].arc, arc) == 0);
        if (same) { dropped++; continue; }
        s_parsedModels[w++] = s_parsedModels[i];
    }
    s_parsedModelCount = w;
    if (dropped > 0) {
        logf(LOG_LEVEL_INFO, "[WwRegistry] {\"ev\":\"model_parse_evict\","
             "\"arc\":\"%s\",\"dropped\":%d,\"left\":%d,\"reads\":\"the "
             "arc is being released; a parsed object over its buffer is dead "
             "the moment it is freed\"}", arc, dropped, s_parsedModelCount);
    }
}

void wwInvalidateParsedModelsAll(const char* why) {
    if (s_parsedModelCount > 0) {
        logf(LOG_LEVEL_INFO, "[WwRegistry] {\"ev\":\"model_parse_evict_all\","
             "\"why\":\"%s\",\"dropped\":%d}", why != nullptr ? why : "?",
             s_parsedModelCount);
    }
    s_parsedModelCount = 0;
    s_worldizedN = 0;
    // Animations share the arcs models live in, so they share the release
    // seam. Evicting one table and not the other would leave parsed anim
    // objects pointing into a buffer the model table just declared dead.
    wwResetParsedAnims();
}

// ========================================================================
// wwParseModelOnce - one parse per (arc, index, raw), for by-index consumers
// ========================================================================
// A by-index fetch returns the SAME RAW IMAGE on every call - the plugin's
// conversion arms are keyed on resource NAME, so nothing replaces the arc
// entry. "I parsed it" does not mean "it is parsed": the next consumer
// starts over on a buffer the first parse already endian-fixed IN PLACE,
// and the second pass is destructive rather than merely wasteful (DN-3).
// Parsing uses the RECEIVER'S OWN loader, never a reimplementation.
// ========================================================================
// KIT-DONOR: d/d_resorce.cpp setToonTex(J3DModelData*) MatchingFor (8006D990).
// WW 'BDL ' conversion is loadBinaryDisplayList + THIS — not TP newSharedDL.
// Pale has no dDlst_list_c::getToonImage; ZA*/ZB* rebind skipped.
// DL patch uses receiver mDoExt_modelTexturePatch (same GD patch as WW).
typedef void (*FnModelTexturePatch)(J3DModelData*);
FnModelTexturePatch s_fnModelTexturePatch = nullptr;

static void wwSetToonTex(J3DModelData* pModel) {
    if (pModel == nullptr) {
        return;
    }
    J3DTexture* pTexture = pModel->getTexture();
    if (pTexture == nullptr) {
        return;
    }
    JUTNameTab* pTextureName = pModel->getTextureName();
    int zaSeen = 0;
    if (pTextureName != nullptr) {
        for (u16 i = 0; i < pTexture->getNum(); i++) {
            const char* pName = pTextureName->getName(i);
            if (pName != nullptr && pName[0] == 'Z' && (pName[1] == 'A' || pName[1] == 'B')) {
                zaSeen++;
            }
        }
    }
    j3dSys.setTexture(pTexture);
    const s32 isBDL = (pModel->getModelDataType() == 1) ? 1 : 0;
    for (u16 i = 0; i < pModel->getMaterialNum(); i++) {
        J3DMaterial* pMaterial = pModel->getMaterialNodePointer(i);
        if (pMaterial == nullptr) {
            continue;
        }
        J3DTevBlock* pTevBlock = pMaterial->getTevBlock();
        if (pTevBlock == nullptr) {
            continue;
        }
        J3DGXColorS10* pTev3 = pMaterial->getTevColor(3);
        if (pTev3 != nullptr) {
            pTev3->a = pTevBlock->getTevStageNum();
        }
    }
    // Mode 7 / WW_LWOOD_SKIP_PATCH=1: tev3.a only — isolate SharedDL GD patch.
    static int s_skipPatch = -1;
    if (s_skipPatch < 0) {
        const char* skip = std::getenv("WW_LWOOD_SKIP_PATCH");
        s_skipPatch = (skip != nullptr && skip[0] == '1') ? 1 : 0;
        if (lwoodDrawMode() == 7) {
            s_skipPatch = 1;
        }
    }
    const int didPatch =
        (isBDL && s_skipPatch == 0 && s_fnModelTexturePatch != nullptr) ? 1 : 0;
    if (didPatch) {
        s_fnModelTexturePatch(pModel);
    }
    static int s_toonN = 0;
    if (s_toonN < 8) {
        s_toonN++;
        logf(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"set_toon_tex\",\"bdl\":%d,\"za_names\":%d,"
             "\"patch_fn\":%d,\"patched\":%d,\"mats\":%u,"
             "\"reads\":\"WW setToonTex; ZA skipped; patch=mDoExt_modelTexturePatch\"}",
             isBDL, zaSeen, s_fnModelTexturePatch != nullptr ? 1 : 0, didPatch,
             (unsigned)pModel->getMaterialNum());
    }
}

// After loadBinaryDisplayList: WW setToonTex; TP-style finish only if no baked DL.
// Returns 1 if wwFinishModelData ran.
// WW_BDL_CONSUME=finish gates suspect #3: always finish, no setToonTex.
static int wwAfterBdlLoad(void** pParsed, const char* tag) {
    if (pParsed == nullptr || *pParsed == nullptr) {
        return 0;
    }
    const int gate = wwBdlConsumeFinishGate();
    if (gate == 2) {
        // finish_toon: the finish consume that cured stretch, PLUS the
        // donor's own setToonTex. Order matches the donor arm below -
        // toon first, then the finish - so the only difference from
        // `finish` is the presence of the toon texture.
        wwSetToonTex(static_cast<J3DModelData*>(*pParsed));
        wwFinishModelData(pParsed, tag);
        return 1;
    }
    if (gate) {
        wwFinishModelData(pParsed, tag);
        return 1;
    }
    J3DModelData* md = static_cast<J3DModelData*>(*pParsed);
    wwSetToonTex(md);
    J3DMaterial* m0 = (md->getMaterialNum() > 0) ? md->getMaterialNodePointer(0) : nullptr;
    J3DDisplayListObj* sdl =
        (m0 != nullptr) ? m0->getSharedDisplayListObj() : nullptr;
    if (m0 == nullptr || sdl == nullptr) {
        wwFinishModelData(pParsed, tag);
        return 1;
    }
    return 0;
}

// ============================================================================
// wwParseAnimOnce - THE ANIMATION ARM OF THE getObjectRes CHOKE
// ============================================================================
// The receiver replaces raw bytes with a PARSED object at arc-load for
// animations exactly as it does for geometry (d_resorce.cpp:506-510). The
// plugin served models through wwParseModelOnce and handed animations back
// RAW, so a consumer casting to J3DAnmTevRegKey* was casting file bytes.
//
// DISPATCH IS ON THE FILE MAGIC, not on the arc's node type: at this choke we
// hold (arc, index, raw) and not the node table, and the magic is authoritative
// anyway - same reasoning as the model arm's 'J3D2'/'bdl4' test.
//
// SEVEN TYPES, NOT SIX: the receiver's else-if also routes 'BXA '.
//
// BCK IS DELIBERATELY NOT HANDLED HERE, AND SAYS SO OUT LOUD. Its path is not
// a bare load(): d_resorce.cpp:485-505 rebases a chunk offset at +0x1C and
// constructs mDoExt_transAnmBas, whose constructor DOES NOT RESOLVE in the
// stock exe (measured: zero hits for ??0mDoExt_transAnmBas@@). Returning raw
// with a WARN is the honest failure - a silent pass-through is what created
// this defect class in the first place.
//
// Memo discipline is the model table's, for the same reasons written there:
// three-field key (arc, index, raw), and eviction at both release seams,
// because an address is not a durable key and a parsed object over a freed
// arc is dead the moment the arc goes.
// ============================================================================
struct WwParsedAnim {
    const char* arc;
    int         index;
    void*       raw;
    void*       parsed;
};
const int kMaxParsedAnims = 64;
WwParsedAnim s_parsedAnims[kMaxParsedAnims] = {};
int s_parsedAnimCount = 0;

void wwInvalidateParsedAnims(const char* arc) {
    if (arc == nullptr) { return; }
    int w = 0, dropped = 0;
    for (int i = 0; i < s_parsedAnimCount; i++) {
        const bool same = (s_parsedAnims[i].arc == arc) ||
                          (s_parsedAnims[i].arc != nullptr &&
                           std::strcmp(s_parsedAnims[i].arc, arc) == 0);
        if (same) { dropped++; continue; }
        s_parsedAnims[w++] = s_parsedAnims[i];
    }
    s_parsedAnimCount = w;
    if (dropped > 0) {
        logf(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"anim_parse_evict\",\"arc\":\"%s\",\"dropped\":%d,"
             "\"left\":%d,\"reads\":\"same law as the model table - a parsed anim over a "
             "released arc is dead the moment the arc is freed\"}",
             arc, dropped, s_parsedAnimCount);
    }
}

void wwResetParsedAnims() { s_parsedAnimCount = 0; }

void* wwParseAnimOnce(void* raw, const char* arc, int index) {
    if (raw == nullptr) { return nullptr; }
    for (int i = 0; i < s_parsedAnimCount; i++) {
        if (s_parsedAnims[i].raw == raw &&
            s_parsedAnims[i].index == index &&
            s_parsedAnims[i].arc != nullptr && arc != nullptr &&
            std::strcmp(s_parsedAnims[i].arc, arc) == 0) {
            return s_parsedAnims[i].parsed;
        }
    }
    // 'J3D1' - not a raw J3D animation image => already parsed, pass through.
    if (*static_cast<const unsigned int*>(raw) != 0x3144334Au) { return raw; }
    const unsigned int sub = *reinterpret_cast<const unsigned int*>(
        static_cast<const unsigned char*>(raw) + 4);
    // 'btk1' 'brk1' 'btp1' 'bpk1' 'blk1' 'bva1' 'bxa1' - the seven the
    // receiver routes to J3DAnmLoaderDataBase::load.
    const bool loadFamily =
        (sub == 0x316B7462u || sub == 0x316B7262u || sub == 0x31707462u ||
         sub == 0x316B7062u || sub == 0x316B6C62u || sub == 0x31617662u ||
         sub == 0x31617862u);
    if (sub == 0x316B6362u) {   // 'bck1' - the unreachable path, named not hidden
        static int s_bckWarn = 0;
        if (s_bckWarn++ == 0) {
            logf(LOG_LEVEL_WARN,
                 "[WwRegistry] {\"ev\":\"anim_parse_bck_unsupported\",\"arc\":\"%s\","
                 "\"index\":%d,\"reads\":\"BCK needs the chunk rebase + mDoExt_transAnmBas "
                 "(d_resorce.cpp:485-505) and that constructor does not resolve by name in the "
                 "stock exe - returning RAW and saying so, because a silent pass-through is "
                 "exactly the defect this arm exists to fix\"}",
                 arc != nullptr ? arc : "?", index);
        }
        return raw;
    }
    if (!loadFamily || s_fnJ3DAnmLoad == nullptr) {
        static int s_unkWarn = 0;
        if (s_unkWarn++ == 0) {
            logf(LOG_LEVEL_WARN,
                 "[WwRegistry] {\"ev\":\"anim_parse_skip\",\"arc\":\"%s\",\"sub\":%u,"
                 "\"loader\":%d,\"reads\":\"J3D1 image the receiver does not route to load(), "
                 "or the loader symbol never resolved - raw returned unchanged\"}",
                 arc != nullptr ? arc : "?", sub, s_fnJ3DAnmLoad != nullptr ? 1 : 0);
        }
        return raw;
    }
    void* parsed = s_fnJ3DAnmLoad(raw, 0);   // J3DLOADER_UNK_FLAG0
    if (parsed != nullptr && s_parsedAnimCount < kMaxParsedAnims) {
        s_parsedAnims[s_parsedAnimCount].arc    = arc;
        s_parsedAnims[s_parsedAnimCount].index  = index;
        s_parsedAnims[s_parsedAnimCount].raw    = raw;
        s_parsedAnims[s_parsedAnimCount].parsed = parsed;
        s_parsedAnimCount++;
    }
    logf(parsed != nullptr ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
         "[WwRegistry] {\"ev\":\"anim_parse\",\"arc\":\"%s\",\"index\":%d,\"sub\":%u,"
         "\"ok\":%d,\"cached\":%d,\"reads\":\"receiver's own J3DAnmLoaderDataBase::load, "
         "flag J3DLOADER_UNK_FLAG0 passed explicitly (no default arg through a resolved symbol)\"}",
         arc != nullptr ? arc : "?", index, sub, parsed != nullptr ? 1 : 0, s_parsedAnimCount);
    return parsed;
}

void* wwParseModelOnce(void* raw, const char* arc, int index) {
    if (raw == nullptr) { return nullptr; }
    for (int i = 0; i < s_parsedModelCount; i++) {
        if (s_parsedModels[i].raw == raw &&
            s_parsedModels[i].index == index &&
            s_parsedModels[i].arc != nullptr && arc != nullptr &&
            std::strcmp(s_parsedModels[i].arc, arc) == 0) {
            return s_parsedModels[i].parsed;
        }
    }
    // Not a raw J3D2 image => already a parsed object; pass it straight through.
    if (*static_cast<const unsigned int*>(raw) != 0x3244334Au) { return raw; }

    const unsigned int sub = *reinterpret_cast<const unsigned int*>(
        static_cast<const unsigned char*>(raw) + 4);
    void* parsed = nullptr;
    int finishRan = 0;
    if (sub == 0x346C6462u && s_fnJ3DLoadBDL != nullptr) {          // 'bdl4'
        parsed = s_fnJ3DLoadBDL(raw, wwBdlLoadFlags());
        if (parsed != nullptr) {
            finishRan = wwAfterBdlLoad(&parsed, arc);
        }
    } else if (sub == 0x33646D62u && s_fnJ3DLoadBMD != nullptr) {   // 'bmd3'
        parsed = s_fnJ3DLoadBMD(raw, 0x29020030);
        wwFinishModelData(&parsed, arc);
        finishRan = 1;
    } else {
        wwFinishModelData(&parsed, arc);
        finishRan = 1;
    }
    wwWorldizeShapeBounds(parsed, arc);
    if (parsed != nullptr && s_parsedModelCount < kMaxParsedModels) {
        s_parsedModels[s_parsedModelCount].arc   = arc;
        s_parsedModels[s_parsedModelCount].index = index;
        s_parsedModels[s_parsedModelCount].raw   = raw;
        s_parsedModels[s_parsedModelCount].parsed = parsed;
        s_parsedModelCount++;
    }
    {
        J3DModelData* md = static_cast<J3DModelData*>(parsed);
        const int locked = (md != nullptr && md->isLocked()) ? 1 : 0;
        J3DMaterial* m0 =
            (md != nullptr && md->getMaterialNum() > 0) ? md->getMaterialNodePointer(0) : nullptr;
        J3DDisplayListObj* sdl =
            (m0 != nullptr) ? m0->getSharedDisplayListObj() : nullptr;
        const unsigned int sdlSize = (sdl != nullptr) ? sdl->getDisplayListSize() : 0u;
        logf(parsed != nullptr ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
             "[WwRegistry] {\"ev\":\"model_parse_once\",\"arc\":\"%s\","
             "\"idx\":%d,\"sub\":\"0x%08X\",\"raw\":\"%p\",\"parsed\":\"%p\","
             "\"n\":%d,\"flags\":\"0x%08X\",\"locked\":%d,\"finish_ran\":%d,"
             "\"shared_dl\":%d,\"shared_dl_size\":%u,"
             "\"reads\":\"WW_BDL_CONSUME=ww|finish; finish_ran under ww only if SharedDL null\"}",
             arc != nullptr ? arc : "?", index, sub, raw, parsed, s_parsedModelCount,
             wwBdlLoadFlags(), locked, finishRan, sdl != nullptr ? 1 : 0, sdlSize);
    }
    return parsed;
}




const short kAkabeIndex = kWwProfileBase + 2;   // 4098
const short kVrboxIndex = kWwProfileBase + 3;   // 4099
const short kVrbox2Index = kWwProfileBase + 4;  // 4100
const short kKytag01Index = kWwProfileBase + 5; // 4101
const short kLwoodIndex = kWwProfileBase + 6;   // 4102

const short kPilotDrawPriority = 0;  // see tale §995: donor's 101 is untransferable
static_assert(kPilotDrawPriority >= 0,
              "draw priority indexes fopDwTg's bucket array; c_tree.cpp:14 "
              "bound-checks only the UPPER end, so a negative value writes "
              "before mpLists");

const WwProfileRow kRowsStorage[] = {
    {kSelfTestIndex, static_cast<const void*>(&kSelfTestSentinel), false},
    {kPilotIndex, static_cast<const void*>(&s_pilotProfile), true},
    {kAkabeIndex, static_cast<const void*>(&s_akabeProfile), true},
    {kVrboxIndex, static_cast<const void*>(&s_vrboxProfile), true},
    {kVrbox2Index, static_cast<const void*>(&s_vrbox2Profile), true},
    {kKytag01Index, static_cast<const void*>(&s_kytag01Profile), true},
    {kLwoodIndex, static_cast<const void*>(&s_lwoodProfile), true},
};
const WwProfileRow* const kRows = kRowsStorage;
const int kRowCount = (int)(sizeof(kRowsStorage) / sizeof(kRowsStorage[0]));

// ============================================================================
// THE PLACEMENT ROUTE — dStage_searchName's return type, authored plugin-side.
//
// LAYOUT DERIVED, NOT GUESSED (the §970 hazard applies to any struct we author
// without receiver headers). Receiver `include/d/d_stage.h:1316-1320`:
//     struct dStage_objectNameInf {
//         /* 0x00 */ char name[8];
//         /* 0x08 */ s16  procname;
//         /* 0x0A */ s8   argument;
//     };  // Size: 0xC
// Natural alignment reproduces it exactly: name@0, procname@8 (2-byte aligned),
// argument@10, tail pad to 12. The static_assert below is the guard — if this
// ever stops being 12 bytes the mismatch is a COMPILE error, not a field
// misread at runtime. That is the whole reason to assert rather than comment.
//
// UNLIKE process_profile_definition this struct is small, fully visible and
// stable, which is why the placement leg can land now while the profile leg's
// table still waits for a derived layout.
// ============================================================================
struct WwObjectNameInf {
    char name[8];
    short procname;
    signed char argument;
};
static_assert(sizeof(WwObjectNameInf) == 0xC,
              "WwObjectNameInf must match dStage_objectNameInf (d_stage.h:1316, size 0xC)");

// ============================================================================
// THE OBJECT-NAME TABLE — deliberately EMPTY, same discipline as the profile
// half. Rows are plugin-owned STATIC storage, which satisfies §970's lifetime
// constraint by construction: no consumer caches the pointer, but it MUST
// outlive the call, and a static table cannot fail that.
//
// One row per WW placement name when actors land. The name field is 8 bytes
// and is NOT required to be NUL-terminated at 8 chars — the receiver compares
// with strcmp against its own 8-byte field, so an 8-char name works exactly as
// it does receiver-side.
// ============================================================================
const WwObjectNameInf kObjectNames[] = {
    // ------------------------------------------------------------------------
    // THE DONOR'S OWN PLACEMENT NAMES — taken from WW `d_stage.cpp:570-571`:
    //     OBJNAME("TagSo",  fpcNm_TAG_SO_e, 0, 0)
    //     OBJNAME("TagMSo", fpcNm_TAG_SO_e, 1, 0)
    // TWO names for ONE proc, differing in the ARGUMENT field (0 vs 1) — and
    // `argument` is a real member of dStage_objectNameInf that the receiver
    // hands to the actor, so the pair is not redundant.
    //
    // WHY THIS MATTERS MORE THAN IT LOOKS: a real WW DZR places "TagSo" or
    // "TagMSo". It never emits "WwPilot" — that name is mine. Without these two
    // rows the actor is reachable only by a fixture we invent, which would test
    // our own plumbing rather than the donor's data. With them, genuine WW
    // placement data routes to a plugin-hosted actor.
    // ------------------------------------------------------------------------
    {"TagSo", kPilotIndex, 0},
    {"TagMSo", kPilotIndex, 1},
    // Retained as the SELF-TEST name only: the game never places it, so it
    // exercises the route at init without colliding with donor data.
    //
    // ARGUMENT IS 2, NOT 0, AND THAT IS LOAD-BEARING (§1026). With 0 this row
    // was IDENTICAL TO `TagSo` IN BOTH KEY FIELDS — `(kPilotIndex, 0)` — so
    // the receiver's REVERSE lookup could not tell them apart. `dStage_getName`
    // (`d_stage.cpp:1786-1812`) returns the FIRST exact match, so today it
    // answers `"TagSo"` by luck of array order, not by construction: append a
    // row, sort the table, or iterate differently in a future plugin-side
    // reverse hook, and **a genuine donor-placed `TagSo` starts naming itself
    // `WwPilot`** — a plugin-invented name reported for donor data, which is
    // precisely the silent substitution zero-bake exists to prevent. Nothing
    // would fail; the name would just quietly become ours.
    //
    // THE FIX MOVES THE ROW WE INVENTED AND LEAVES THE DONOR'S ALONE (DN-10):
    // `TagSo` 0 and `TagMSo` 1 keep the donor's own values verbatim from WW
    // `d_stage.cpp:570-571`. Only this name is ours, so only this one moves,
    // and no donor data can collide with 2 because the game never places it.
    // `(proc, argument)` is now unique BY CONSTRUCTION rather than by ordering.
    //
    // SAFE TODAY, BY INSPECTION: the forward path matches on NAME only
    // (`strncmp`), so the self-test is unaffected — and `argument` reaches an
    // actor only through creation, which this row never undergoes.
    {"WwPilot", kPilotIndex, 2},
    // Port #1 (queue 146): donor WW `d_stage.cpp:1088` OBJNAME("Akabe",
    // fpcNm_Obj_Akabe_e, 0, 0) — name and argument verbatim.
    {"Akabe", kAkabeIndex, 0},
    // Donor d_stage.cpp:1260-1261. argument 255 as s8 is -1 (vanilla's Vrbox
    // argument). Intercepted only on WW stages — TP owns these names too.
    {"Vrbox", kVrboxIndex, -1},
    {"Vrbox2", kVrbox2Index, -1},
    // Donor d_stage.cpp:660 OBJNAME("ky_tag1", fpcNm_KYTAG01_e, 255, 0).
    {"ky_tag1", kKytag01Index, -1},
    // Donor d_stage.cpp OBJNAME("lwood", fpcNm_Lwood_e, …).
    {"lwood", kLwoodIndex, 0},
};
const int kObjectNameCount = (int)(sizeof(kObjectNames) / sizeof(kObjectNames[0]));

int s_pfHits = 0;
int s_dylHits = 0;
int s_nameHits = 0;
int s_searchCallsTotal = 0;  // every dStage_searchName call — stage-load proxy

// ============================================================================
// ROW OWNERSHIP TEST — does this pointer name one of OUR rows?
//
// ADDED 2026-08-15 (tale §1005). The placement self-test's fall-through half
// used to compare the receiver's answer against a SINGLE element, so it passed
// for any answer that was not that one element — including a wrong answer of
// another of our own rows, i.e. exactly the shadowing case its comment claimed
// to rule out. Ownership is a property of the whole table, so it is tested
// against the whole table.
// ============================================================================
bool is_our_row(const void* p) {
    for (int i = 0; i < kObjectNameCount; i++) {
        if (p == static_cast<const void*>(&kObjectNames[i])) {
            return true;
        }
    }
    return false;
}

void logf(LogLevel level, const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    s_log->write(mod_ctx, level, buf);
}

// Declared with the detector globals (§836 block above); defined here because
// only from this point does `logf` mean OUR writer and not the CRT's math fn.
void noteDefer(const char* which) {
    if (s_deferNotes >= 6) { return; }
    s_deferNotes++;
    logf(LOG_LEVEL_WARN,
        "[WwRegistry] {\"ev\":\"defer_to_host\",\"subsystem\":\"%s\","
        "\"reads\":\"fork-hosted image detected; the host's own layer answers this - "
        "running both would double-provision (CALLS 836)\"}", which);
}

// ============================================================================
// The hand-off choke point — every profile we give the receiver leaves through
// here, so the priority constraint is enforced here as well as at compile time.
//
// RUNTIME AS WELL AS COMPILE-TIME because the two catch different things: the
// static_assert covers a literal in this file, and this covers a profile
// filled at init from a resolved address or a future data-driven row. A
// negative value would reach `cTr_Addition`'s unchecked lower bound.
//
// REFUSAL IS `nullptr`, which is already this hook's "not mine" answer — so a
// rejected row degrades to the receiver's own graceful creation failure
// (fpcBs_Create, f_pc_base.cpp:138) instead of inventing a second refusal path.
// The actor does not spawn; nothing is corrupted.
// ============================================================================
bool s_priorityRefused = false;

const void* lookup(short i_procName) {
    for (int i = 0; i < kRowCount; i++) {
        if (kRows[i].index != i_procName) {
            continue;
        }
        const WwProfileRow& row = kRows[i];
        if (row.is_profile && row.profile != nullptr) {
            const short priority =
                static_cast<const WwLeafProfileDef*>(row.profile)->priority;
            if (priority < 0) {
                if (!s_priorityRefused) {  // once — this runs per creation
                    s_priorityRefused = true;
                    logf(LOG_LEVEL_ERROR,
                        "[WwRegistry] {\"ev\":\"priority_refused\",\"index\":%d,"
                        "\"priority\":%d,\"why\":\"negative index into "
                        "fopDwTg bucket array (c_tree.cpp:14 checks upper "
                        "bound only)\"}",
                        (int)i_procName, (int)priority);
                }
                return nullptr;
            }
        }
        return row.profile;
    }
    return nullptr;
}

// (The b2 autofire counter that used to be forward-declared here is retired
// with the autofire path — user ruling §1009. See the retirement note above
// the DEFINE_HOOK_SYMBOL block.)
// Defined with the start-stage reader below; declared here so the sample site
// stays inside the hook it rides on rather than in a separate pass.
void sampleStartStage();

int s_loaderCalls = 0;
int s_setStageResCalls = 0;
int s_glbCalls = 0;

// PRE-hook — logs BEFORE the call, because the call may not come back.
//
// WW-GATED so a TP session is untouched and the volume stays readable. The
// gate cannot hide the target: the start stage is `sea` at the moment of the
// crash, confirmed by `startstage_now` in the last three runs.
//
// DELIBERATELY NOT CAPPED. Every other probe in this file rate-limits, and
// that was right for counters; here a cap would hide the fatal call if it fell
// past the limit — which is exactly how `set_stage_res_calls <= 4` hid call #5
// Defined below the destination table; declared here because the resource
// probe gates on it and sits above that table.
bool startStageIsWw();
bool stageBecomingWw();   // start OR next — see the MULT gate (Room -1 fix)

// for a whole boot. The run ends seconds after the first WW stage entry, so
// the volume is bounded by the crash and the LAST LINE is the evidence.
int s_msgArcWrites = 0;

// PURE OBSERVER on every write to a message-archive slot. Not gated on the WW
// stage: the TP writes are the control, and a control I skipped is how the
// last three theories died.
HookAction on_setMsgDtArc(ModContext*, void* args, void*, void*) {
    const int idx = mods::arg<int>(args, 0);
    void* arc = mods::arg<void*>(args, 1);
    s_msgArcWrites++;
    logf(LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"msgarc_set\",\"n\":%d,\"slot\":%d,"
        "\"arc\":\"%p\",\"start\":\"%s\"}",
        s_msgArcWrites, idx, arc,
        (s_fnGetStartStageName != nullptr && s_fnGetStartStageName() != nullptr)
            ? s_fnGetStartStageName() : "(unreadable)");
    return HOOK_CONTINUE;
}

// ============================================================================
// ARCHIVE LIVENESS PEEK — INTEGRATOR 2026-08-16.
//
// A `JKRArchive*` is a C++ object, so its first 8 bytes are a VTABLE POINTER,
// and a live one points INTO THE IMAGE. Freed-and-reused memory does not.
//
// WHY THIS AND NOT ANOTHER REDIRECT: slot 1 has held the same value since the
// TITLE SCREEN (`F_SP102`) — through the Ordon load, which runs fine, and into
// `sea`, where it kills us. **An unchanged pointer proves nothing about the
// memory behind it**, which is the error that killed my dangling-pointer
// theory: I compared pointer VALUES and called them healthy. This compares the
// OBJECT.
//
// slot 0 is the control. It is the boot/global archive, it is used successfully
// on the same WW stage one call earlier (n=7), and it must therefore read as
// live. **If slot 0's vptr looks like an image address and slot 1's does not,
// slot 1 has been overwritten and the question moves to what allocated over
// it.** If BOTH look live, the archives are innocent and I have been chasing
// the wrong object for five theories.
//
// Reading 8 bytes through the pointer is no more dangerous than what the game
// does with it microseconds later, and this runs BEFORE that — so the line is
// on disk even if the read itself faults.
// ============================================================================
unsigned long long peek64(void* p) {
    if (p == nullptr) {
        return 0ULL;
    }
    return *static_cast<unsigned long long*>(p);
}

int s_j3dBdl = 0;
int s_j3dBmd = 0;

// POST hooks: `retval` carries the parsed `J3DModelData*`, and NULL there is
// the answer that separates "TP rejected WW's layout" from "parsed fine".
// ============================================================================
// `on_resLoadResource` — the supplied `'BMDL'` arm, run AFTER the receiver's
// own dispatch so we see exactly what it declined to handle.
//
// POST, not PRE, deliberately: pre-hooking would mean reimplementing the whole
// 300-line `loadResource` (the class-B "hook the host entire" cost). Running
// after it costs nothing and leaves every arm the receiver DOES have untouched.
// ============================================================================
// EVERY EXIT IS NAMED. Run 225743 bound MOD_OK and printed NOTHING, and I could
// not tell "never called" from "called and found zero" from "bailed at the count
// check" — because all seven early returns were silent. That is the dead-probe
// defect I filed three rows about the same night. One unconditional receipt per
// call, carrying the reason, tests all ten hypotheses in ONE boot.
int s_bmdlCalls = 0;
// RUN 231654 ANSWERED THE WRONG QUESTION BECAUSE I NEVER LOGGED **WHICH ARCHIVE**.
// 40 passes, all raw:0 — but with no name on any of them I could not tell
// "the stage arc was walked and had nothing raw" from "the stage arc was never
// walked at all". Those are opposite diagnoses. `mArchiveName` is a char[11] at
// offset 0x00 of dRes_info_c (measured), so the name was free the whole time.
void bmdlReceipt(void* self, const char* why, unsigned int count, int raw, int nonNull) {
    // CAP RAISED 60 -> 4000 AND THE REASON IS A FAILURE, NOT A PREFERENCE.
    // Run 031024 logged passes 1..60 and stopped at EXACTLY the cap — TP's own
    // boot arcs (LogoUs, Always, Alink, Title, Ordon props) consumed every slot,
    // and the WW stage arcs load AFTER them. **The cap hid the precise thing the
    // arc name was added to reveal.** Same shape as the tier2 C-class list and
    // the B-class host list: a display limit eating the signal while the run
    // looks complete. A pass receipt is one short line; 4000 of them is cheap
    // next to losing a user's boot, and `raw` is the only field that matters
    // when it is non-zero.
    if (s_bmdlCalls > 4000) { return; }
    const char* arcName = (self != nullptr) ? static_cast<const char*>(self) : "?";
    char safeName[12];
    for (int k = 0; k < 11; k++) {
        const char c = arcName[k];
        safeName[k] = (c >= 32 && c < 127) ? c : '\0';
        if (safeName[k] == '\0') { break; }
    }
    safeName[11] = '\0';
    logf(LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"bmdl_pass\",\"n\":%d,\"arc\":\"%s\",\"why\":\"%s\","
        "\"count\":%u,\"nonNull\":%d,\"raw\":%d}",
        s_bmdlCalls, safeName, why, count, nonNull, raw);
}

void on_resLoadResource(ModContext*, void* args, void* retval, void*) {
    s_bmdlCalls++;
    // DEFER-TO-HOST (§836): same double-parse hazard as the getRes arm.
    if (s_hostIsFork) { noteDefer("bmdl_arm_loadResource"); return; }
    // `retval` is the receiver's own return; a failed load is not ours to fix.
    const int rc = (retval != nullptr) ? *static_cast<int*>(retval) : 0;
    if (rc < 0) { bmdlReceipt(nullptr, "receiver_load_failed", 0, 0, 0); return; }

    void* self = mods::arg<void*>(args, 0);          // `this` (member fn, arg0)
    if (self == nullptr) { bmdlReceipt(nullptr, "self_null", 0, 0, 0); return; }
    if (s_fnJ3DLoadBDL == nullptr && s_fnJ3DLoadBMD == nullptr) {
        bmdlReceipt(self, "no_parser_resolved", 0, 0, 0); return;   // prime suspect
    }

    unsigned char* base = static_cast<unsigned char*>(self);
    void* arc = *reinterpret_cast<void**>(base + 0x18);        // mArchive
    void** slots = *reinterpret_cast<void***>(base + 0x30);    // mRes
    if (arc == nullptr) { bmdlReceipt(self, "arc_null", 0, 0, 0); return; }
    if (slots == nullptr) { bmdlReceipt(self, "slots_null", 0, 0, 0); return; }

    void* info = *reinterpret_cast<void**>(static_cast<unsigned char*>(arc) + 0x78);
    if (info == nullptr) { bmdlReceipt(self, "arcinfo_null", 0, 0, 0); return; }

    // BIG-ENDIAN in the archive image — swap before use (see block comment).
    const unsigned int beCount =
        *reinterpret_cast<unsigned int*>(static_cast<unsigned char*>(info) + 0x08);
    const unsigned int count =
        ((beCount & 0x000000FFu) << 24) | ((beCount & 0x0000FF00u) << 8) |
        ((beCount & 0x00FF0000u) >> 8)  | ((beCount & 0xFF000000u) >> 24);
    // Sanity bound: a plausible archive, never a swap that silently went wrong.
    // Report the RAW value too — if the offset is wrong, the number itself says so.
    if (count == 0u || count > 4096u) {
        if (s_bmdlCalls <= 40) {
            logf(LOG_LEVEL_WARN,
                "[WwRegistry] {\"ev\":\"bmdl_pass\",\"n\":%d,\"why\":\"count_implausible\","
                "\"beRaw\":\"0x%08X\",\"swapped\":%u}", s_bmdlCalls, beCount, count);
        }
        return;
    }

    // Always/WwAlways: skip BMDL supply walk. This hook exists for J3D models;
    // Always is mostly TIMG. Mid-expand walks corrupted boots (f_shipicon
    // truncated). Celestial + whitecaps use getObjectRes after COMPLEATE.
    {
        const char* arcName = static_cast<const char*>(self);
        char n0 = arcName != nullptr ? arcName[0] : 0;
        if (n0 == 'A' || n0 == 'W') {
            char safe[12];
            for (int k = 0; k < 11; k++) {
                const char c = arcName[k];
                safe[k] = (c >= 32 && c < 127) ? c : '\0';
                if (safe[k] == '\0') {
                    break;
                }
            }
            safe[11] = '\0';
            if (std::strcmp(safe, "Always") == 0 || std::strcmp(safe, "WwAlways") == 0) {
                static int s_skipN = 0;
                if ((++s_skipN % 50) == 1) {
                    logf(LOG_LEVEL_INFO,
                         "[WwRegistry] {\"ev\":\"bmdl_skip_always\",\"n\":%d,\"arc\":\"%s\"}",
                         s_skipN, safe);
                }
                return;
            }
        }
    }

    int nonNull = 0, rawHere = 0;
    for (unsigned int i = 0; i < count; i++) {
        void* p = slots[i];
        if (p == nullptr) { continue; }
        nonNull++;
        // Sample the first few heads unconditionally: "walked 200 slots and none
        // were raw" and "walked 200 slots of garbage" print identically without
        // this, and they are opposite diagnoses.
        if (s_bmdlCalls <= 3 && nonNull <= 6) {
            logf(LOG_LEVEL_INFO,
                "[WwRegistry] {\"ev\":\"bmdl_slot\",\"n\":%d,\"i\":%u,\"p\":\"%p\","
                "\"head\":\"0x%08X\",\"sub\":\"0x%08X\"}",
                s_bmdlCalls, i, p, *static_cast<const unsigned int*>(p),
                *reinterpret_cast<const unsigned int*>(static_cast<unsigned char*>(p) + 4));
        }
        // Still raw => the receiver had no arm for this node type. A parsed
        // slot's head is a heap vtable, so this can never re-fire (DN-3's own
        // verification signature, used here as the single-parse guard).
        // 'J3D2' AS A LITTLE-ENDIAN u32 IS 0x3244334A, NOT 0x4A334432.
        // The reversed constant is what made every pass report `raw:0` while
        // the same archive was handing out raw buffers: the enumeration was
        // correct and the COMPARISON could never match. I derived `bdl4` and
        // `bmd3` from the LOG and got them right; I derived this one from
        // ASCII in my head and got it backwards (runs 031720 / 031936).
        const unsigned int magic = *static_cast<const unsigned int*>(p);
        if (magic != 0x3244334Au) { continue; }                // 'J3D2'
        rawHere++;
        const unsigned int sub =
            *reinterpret_cast<const unsigned int*>(static_cast<unsigned char*>(p) + 4);
        s_bmdlSeen++;

        void* parsed = nullptr;
        if (sub == 0x626D6433u) {                              // 'bmd3'
            if (s_fnJ3DLoadBMD != nullptr) { parsed = s_fnJ3DLoadBMD(p, 0x29020030); }
            wwFinishModelData(&parsed, "loadResource");
        } else if (sub == 0x62646C34u) {                       // 'bdl4'
            if (s_fnJ3DLoadBDL != nullptr) { parsed = s_fnJ3DLoadBDL(p, wwBdlLoadFlags()); }
            wwAfterBdlLoad(&parsed, "loadResource");
        } else {
            continue;   // J3D2 but not a model container — not ours.
        }
        if (parsed != nullptr) {
            slots[i] = parsed;      // republish, exactly as the DEBUG arm does
            s_bmdlFixed++;
        } else {
            s_bmdlFailed++;         // parser REJECTED it — a format answer
        }
        if (s_bmdlSeen <= 8) {
            logf(parsed != nullptr ? LOG_LEVEL_INFO : LOG_LEVEL_WARN,
                "[WwRegistry] {\"ev\":\"bmdl_supplied\",\"slot\":%u,"
                "\"sub\":\"0x%08X\",\"in\":\"%p\",\"out\":\"%p\"}",
                i, sub, p, parsed);
        }
    }
    // THE LINE THAT WAS MISSING. Fires on EVERY pass, including the all-zero
    // one — so "walked the arc, nothing was raw" becomes a statement instead
    // of a silence.
    bmdlReceipt(self, "walked", count, rawHere, nonNull);
}

// Draw submissions are per-frame and per-part, so an ungated log would bury
// the answer in its own volume — the throttle failure that ate the signal
// three times earlier this week. Report the FIRST few, then a periodic
// heartbeat, and gate on WW stages so TP's own drawing is not counted.
int s_drawEntries = 0;
// POST, because the ANSWER is the return value: did construction yield an
// instance, or NULL? A pre hook sees only that it was attempted.
int s_bgDraws = 0;
// ============================================================================
// COUNT BEFORE YOU GATE. The previous cut returned on !startStageIsWw() BEFORE
// touching the counter, so `bg_draw` receipts = 0 had TWO readings that are
// opposite answers and were indistinguishable:
//
//   (a) daBg_c::draw NEVER RUNS on this host - the actor is never asked, and
//       any fix that re-shows shapes is fixing nothing; or
//   (b) it runs FINE and the WW predicate suppressed every receipt.
//
// The vanilla run reported ZERO. That silence is exactly the shape that has
// killed five hypotheses on this board - a gated instrument reading as absence.
// So: count EVERY call unconditionally, and report the predicate's value
// alongside. `calls` answers (a); `ww` answers (b); the pair cannot be misread.
// ============================================================================
HookAction on_bgDraw(ModContext*, void* args, void*, void*) {
    static int s_bgDrawCallsAll = 0;   // EVERY call, gate or no gate
    s_bgDrawCallsAll++;

    const bool ww = startStageIsWw();
    if (ww) { s_bgDraws++; }

    // ------------------------------------------------------------------
    // SAMPLE IN THE RIGHT ERA. The previous cut threw its whole sample away
    // on the WRONG STAGE and read as a finding: all twelve receipts landed
    // at log lines 1307-2448 while the first `start="sea"` was at 2489, so
    // every `ww:0` was CORRECT and about the PRE-WARP BG - the title actor,
    // not Outset's. `calls:1200` looks damning until you ask WHEN frame 1200
    // was.
    //
    // A throttle is a sampling decision, and a sampling decision made before
    // the event under study is a decision to miss it. So the counters RESET
    // when the WW predicate flips, and the throttle restarts on the far side
    // of the warp. `ww_calls` now counts from the stage we actually care
    // about, and `era` says which side of the flip a receipt came from.
    // ------------------------------------------------------------------
    static bool s_sawWw = false;
    if (ww && !s_sawWw) {
        s_sawWw = true;
        s_bgDrawCallsAll = 1;   // restart sampling ON the WW stage
        s_bgDraws = 1;
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"bg_draw_era\",\"reads\":\"WW predicate flipped "
            "TRUE - bg_draw counters reset here. Receipts BELOW this line are the "
            "only ones about the WW stage; anything above is pre-warp and says "
            "nothing about Outset\"}");
    }

    // Emit on the first few of EITHER counter so a host where the predicate is
    // false still proves the hook fires at all.
    if (s_bgDrawCallsAll <= 6 || (s_bgDrawCallsAll % 600) == 0 ||
        (ww && (s_bgDraws <= 6 || (s_bgDraws % 60) == 0))) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"bg_draw\",\"calls\":%d,\"ww_calls\":%d,\"ww\":%d,"
            "\"self\":\"%p\",\"entries_so_far\":%d,"
            "\"reads\":\"READ THE ERA FIRST - a receipt ABOVE bg_draw_era is "
            "PRE-WARP and says NOTHING about the WW stage (that mistake was made "
            "once already: 12 receipts, all before the stage flipped, read as a "
            "finding). BELOW the era line: ww==1 with entries==0 = the actor is "
            "asked to draw and has NO PARTS - upstream of any culling. ww==1 with "
            "entries>0 = parts exist and the question moves to visibility. Event "
            "ABSENT entirely = daBg_c::draw is never reached\"}",
            s_bgDrawCallsAll, s_bgDraws, ww ? 1 : 0,
            mods::arg<void*>(args, 0), s_drawEntries);
    }
    return HOOK_CONTINUE;
}

HookAction on_drawOpaList(ModContext*, void* args, void*, void*) {
    wwVrbox_onSkyOpaDraw(mods::arg<void*>(args, 1));
    return HOOK_CONTINUE;
}

int s_toQ = 0;
void on_lyTgToQueue(ModContext*, void* args, void* retval, void*) {
    const bool ww = startStageIsWw();
    const unsigned int layerId = mods::arg<unsigned int>(args, 1);
    const int ret = (retval != nullptr) ? *static_cast<int*>(retval) : -1;
    // Report REFUSALS always (they are the finding) and a few grants for
    // contrast. A refusal is silent in the game and is the whole bug.
    static int shownWw = 0, shownTp = 0;
    int& shown = ww ? shownWw : shownTp;
    s_toQ++;
    if (ret == 0 ? (shown < 12) : (shown < 4)) {
        shown++;
        logf(ret == 0 ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"lytg_toqueue\",\"ww\":%d,\"layer_id\":\"0x%08X\","
            "\"ret\":%d,\"reads\":\"ret=0 means NOT QUEUED into any layer list, so the process executes but is NEVER DRAWN\"}",
            ww ? 1 : 0, layerId, ret);
    }
}

// ============================================================================
// PROBE SET #20 HANDLERS — the phase ladder. Each fires bounded (first few +
// milestone transitions), tagged ww/tp so the working TP path is the control.
// The stall signature is a RET that never changes across a growing call count,
// so the periodic heartbeat carries the count and the last return together.
// ============================================================================
int s_audSetScene = 0;
void on_audSetScene(ModContext*, void* args, void*, void*) {
    s_audSetScene++;
    const char* spot = mods::arg<char*>(args, 1);
    char safe[24]; snprintf(safe, sizeof(safe), "%s", spot ? spot : "(null)");
    if (s_audSetScene <= 6) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"phase_ladder\",\"step\":\"01_setSceneName\","
            "\"n\":%d,\"ww\":%d,\"spot\":\"%s\",\"room\":%d,\"layer\":%d,"
            "\"reads\":\"phase_01 REACHED; an unknown spot resolves to Z2SCENE_NONE, not a stall by itself\"}",
            s_audSetScene, startStageIsWw() ? 1 : 0, safe,
            mods::arg<int>(args, 2), mods::arg<int>(args, 3));
    }
}

int s_audLoad1st = 0;
void on_audLoad1st(ModContext*, void*, void*, void*) {
    s_audLoad1st++;
    if (s_audLoad1st <= 6) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"phase_ladder\",\"step\":\"01_load1stWave\","
            "\"n\":%d,\"ww\":%d,\"reads\":\"phase_01 PASSED (mDoAud wrapper returns 1 unconditionally)\"}",
            s_audLoad1st, startStageIsWw() ? 1 : 0);
    }
}

// phase_3's gate. `true` = still loading = phase_3 loops cPhs_INIT. A handful
// of trues at scene start is NORMAL (load1stWait counts down from 40); the
// finding is trues WITHOUT END — hence log the first few, every 120th, and
// the FIRST false (the moment phase_3 unblocks, or never).
int s_audCheck = 0, s_audCheckTrue = 0, s_audCheckFalseSeen = 0;
void on_audCheck1st(ModContext*, void*, void* retval, void*) {
    s_audCheck++;
    const bool still = (retval != nullptr) && *static_cast<bool*>(retval);
    if (still) { s_audCheckTrue++; }
    const bool firstFalse = (!still && s_audCheckFalseSeen == 0);
    if (firstFalse) { s_audCheckFalseSeen = 1; }
    if (s_audCheck <= 4 || firstFalse || (still && (s_audCheck % 120) == 0)) {
        logf((still && s_audCheck >= 120) ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"phase_ladder\",\"step\":\"3_check1stWave\","
            "\"n\":%d,\"ww\":%d,\"still_loading\":%d,\"trues\":%d,"
            "\"reads\":\"true forever = phase_3 STALL (H3); a first false = phase_3 passed\"}",
            s_audCheck, startStageIsWw() ? 1 : 0, still ? 1 : 0, s_audCheckTrue);
    }
}

// Logo dvdWaitDraw / play sync gate. Hook is syncAllRes (inline target of
// dComIfG_syncAllObjectRes). ret>0 = still syncing.
//
// Do NOT open WwAlways here. Full disc Always→WwAlways (§806) at logo alongside
// TP Always+Alink left the archive heap too tight for sea Akabe solid heaps
// (113931: always_resident OK, then dbgs_regist n=3 → crash before akabe_created).
// Boot window opens from wwSky_move when the outdoor host is live.
int s_syncObj = 0, s_syncZeroSeen = 0;
void on_syncObjRes(ModContext*, void*, void* retval, void*) {
    s_syncObj++;
    const int rt = (retval != nullptr) ? *static_cast<int*>(retval) : -999;
    if (wwSky_alwaysReady()) {
        // no-op
    } else if (wwSky_bootWindowOpen()) {
        wwSky_pollAlways();
        if (!wwSky_alwaysReady() && retval != nullptr) {
            *static_cast<int*>(retval) = 1;
        }
    }
    const int outRt = (retval != nullptr) ? *static_cast<int*>(retval) : rt;
    const bool done = (outRt <= 0);
    const bool firstDone = (done && s_syncZeroSeen == 0);
    if (firstDone) {
        s_syncZeroSeen = 1;
    }
    if (s_syncObj <= 4 || firstDone || (!done && (s_syncObj % 120) == 0)) {
        logf((!done && s_syncObj >= 120) ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"phase_ladder\",\"step\":\"2_syncObjectRes\","
            "\"n\":%d,\"ww\":%d,\"ret\":%d,"
            "\"reads\":\"ret>0 forever = phase_2 STALL (H2, object arcs never arrive); negative = error path (H6)\"}",
            s_syncObj, startStageIsWw() ? 1 : 0, outRt);
    }
}

// ============================================================================
// PROBE SET #21 HANDLERS — see the DEFINE block for the mechanism. The census
// keys on the proc POINTER and stores only its last cPhs status; a status
// TRANSITION is the loggable event, so a looper (INIT every frame) costs one
// line at first sight and one more only if it ever resolves. The heartbeat
// aggregates by status so a hundred loopers read as one number, not a flood.
// ============================================================================
int s_mtdCalls = 0;
void on_mtdCreate(ModContext*, void* args, void* retval, void*) {
    if (!startStageIsWw()) { return; }     // TP's own count is known: 40 drawn
    s_mtdCalls++;
    void* proc = mods::arg<void*>(args, 1);
    const int st = (retval != nullptr) ? *static_cast<int*>(retval) : -1;
    static void* procs[192]; static signed char stat[192]; static int n = 0;
    int i = 0;
    for (; i < n; i++) { if (procs[i] == proc) { break; } }
    const bool fresh = (i == n);
    if (fresh && n < 192) {
        procs[n] = proc; stat[n] = (signed char)st; n++;
        // FIRST-SIGHT ERRORS WERE INVISIBLE: run 143410 counted 99 of them and
        // could name none, because only TRANSITIONS logged and an actor whose
        // FIRST return is ERROR(5) never transitions. Bounded to 16 — the
        // pattern (which profiles die at birth) shows in far fewer.
        static int shownBornDead = 0;
        if (st == 5 && shownBornDead < 16) {
            shownBornDead++;
            logf(LOG_LEVEL_ERROR,
                "[WwRegistry] {\"ev\":\"create_born_dead\",\"n\":%d,\"proc\":\"%p\","
                "\"reads\":\"create returned ERROR on its FIRST call - cross-ref "
                "against fpcBs_Create pid/profname lines to name the actor\"}",
                shownBornDead, proc);
        }
    }
    else if (!fresh && stat[i] != (signed char)st) {
        // The transition IS the finding: INIT->COMPLEATE names an actor that
        // finished; INIT->ERROR names one that silently died (condition 0x10).
        logf(st == 5 ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"create_status\",\"proc\":\"%p\",\"from\":%d,"
            "\"to\":%d,\"reads\":\"0=INIT loop,4=COMPLEATE(drawable),5=ERROR(deleted)\"}",
            proc, (int)stat[i], st);
        stat[i] = (signed char)st;
    }
    if ((s_mtdCalls % 900) == 0) {         // ~once per 15s of looping creates
        int byStat[8] = {0};
        for (int k = 0; k < n; k++) {
            const int s = stat[k];
            byStat[(s >= 0 && s < 8) ? s : 7]++;
        }
        // The loopers ARE the black screen; name the first few for the
        // fpcBs_Create cross-reference.
        char who[160]; int w = 0; who[0] = '\0';
        for (int k = 0; k < n && w < 5; k++) {
            if (stat[k] == 0) {
                char one[28];
                snprintf(one, sizeof(one), "%s%p", who[0] ? "," : "", procs[k]);
                strncat(who, one, sizeof(who) - strlen(who) - 1);
                w++;
            }
        }
        logf(LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"create_census\",\"distinct\":%d,\"init_looping\":%d,"
            "\"loading\":%d,\"compleate\":%d,\"error\":%d,\"first_loopers\":\"%s\","
            "\"reads\":\"init_looping>0 sustained = actors that will never draw; "
            "cross-ref pointers against fpcBs_Create OK lines\"}",
            n, byStat[0], byStat[1], byStat[4], byStat[5], who);
    }
}

int s_drawQAdmitWw = 0, s_drawQAdmitTp = 0;
// #22 shares the admission ledger: remember recent WW admissions so a CUT can
// say whether it hit one of them. 64 slots ring — admissions on a stage are
// tens, not thousands.
void* s_admitRing[64]; int s_admitN = 0;
void on_dwTgToDrawQ(ModContext*, void* args, void*, void*) {
    const bool ww = startStageIsWw();
    int& c = ww ? s_drawQAdmitWw : s_drawQAdmitTp;
    c++;
    if (ww) { s_admitRing[s_admitN++ & 63] = mods::arg<void*>(args, 0); }
    if (c <= 6 || (c % 50) == 0) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"drawq_admit\",\"ww\":%d,\"n\":%d,\"tag\":\"%p\","
            "\"reads\":\"each admission = one actor whose create COMPLEATED; "
            "ww stuck at ~1 while tp reaches ~40 = the door is the choke\"}",
            ww ? 1 : 0, c, mods::arg<void*>(args, 0));
    }
}

// ============================================================================
// PROBE SET #22 HANDLERS — see the DEFINE block. The wipe is rare and always
// loggable; the cut is per-deletion, so report cuts of REMEMBERED WW
// admissions always and others only as a bounded sample.
// ============================================================================
// The attention guard's player getter, resolved as a CALLABLE at install
// (same pattern as s_fnGetStartStageName). NULL until resolve succeeds —
// and an unresolved getter means the guard NEVER skips, which fails toward
// stock behavior, not toward silently suppressing attention.
typedef void* (*FnGetPlayer)();
FnGetPlayer s_fnGetPlayerActor = nullptr;
int s_attnGuardSkips = 0;
HookAction on_attnSetOwnerPos(ModContext*, void*, void*, void*) {
    if (s_fnGetPlayerActor == nullptr) { return HOOK_CONTINUE; }
    if (s_fnGetPlayerActor() != nullptr) { return HOOK_CONTINUE; }
    s_attnGuardSkips++;
    if (s_attnGuardSkips <= 4 || (s_attnGuardSkips % 600) == 0) {
        logf(LOG_LEVEL_WARN,
            "[WwRegistry] {\"ev\":\"attn_null_player_skip\",\"n\":%d,\"ww\":%d,"
            "\"reads\":\"receiver's own SelectAttention NULL-guard applied one call "
            "earlier; each skip is a frame that would have been crash 144259\"}",
            s_attnGuardSkips, startStageIsWw() ? 1 : 0);
    }
    return HOOK_SKIP_ORIGINAL;
}

// ============================================================================
// PROBE SET #23 HANDLER — the admission's return. Refusals are the finding
// and log always (bounded); grants log a few per side for contrast. The tag
// is checked against the WW admission ring so a refusal can say it hit one
// of OUR completed actors' tags.
// ============================================================================
int s_tgAdds = 0, s_tgRefused = 0;
void on_tgAddToTree(ModContext*, void* args, void* retval, void*) {
    s_tgAdds++;
    const int idx = mods::arg<int>(args, 1);
    const int ret = (retval != nullptr) ? *static_cast<int*>(retval) : -1;
    void* tag = mods::arg<void*>(args, 2);
    const bool ww = startStageIsWw();
    const bool refused = (ret == 0);
    if (refused) { s_tgRefused++; }
    static int shownRef = 0, shownOkWw = 0, shownOkTp = 0;
    bool show = false;
    if (refused && shownRef < 16) { shownRef++; show = true; }
    else if (!refused && ww && shownOkWw < 6) { shownOkWw++; show = true; }
    else if (!refused && !ww && shownOkTp < 4) { shownOkTp++; show = true; }
    if (show) {
        logf(refused ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"tg_add\",\"ww\":%d,\"listIdx\":%d,\"ret\":%d,"
            "\"tag\":\"%p\",\"reads\":\"ret=0 = REFUSED (idx>=1000, NEGATIVE idx, "
            "or tag already in use) - refusal is silent in the game; the void "
            "the admission counter cannot see\"}",
            ww ? 1 : 0, idx, ret, tag);
    }
}

// #24 — the node-walk census. Same distinct-set shape as dw_exec, one level
// up. Node pointers cross-ref via census_join against fpcBs_Create lines.
int s_ndDraws = 0;
HookAction on_ndDraw(ModContext*, void* args, void*, void*) {
    const bool ww = startStageIsWw();
    s_ndDraws++;

    // ------------------------------------------------------------------
    // PERIODIC PER-ERA TOTAL — because the dedup below makes this probe
    // STRUCTURALLY UNABLE to report per-frame activity. It logs each DISTINCT
    // node once, capped at 24 per era, so a whole boot yielded 3 receipts and
    // "0 after the WW flip" was read as "the draw walker stopped". It means
    // no NEW node, which is a different claim entirely.
    //
    // That is the fourth instrument on this surface whose LIMIT was read as a
    // property of the subject. So: an unconditional running total per era,
    // emitted on a cadence the dedup cannot suppress. `ww_total` climbing
    // after the flip = the draw walker IS running on the WW stage and the BG
    // simply is not in it. `ww_total` flat = the walker itself stops.
    // ------------------------------------------------------------------
    static int s_ndAllTp = 0, s_ndAllWw = 0;
    int& eraTotal = ww ? s_ndAllWw : s_ndAllTp;
    eraTotal++;
    if ((eraTotal % 120) == 0 || eraTotal == 1) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"nd_draw_total\",\"ww\":%d,\"era_total\":%d,"
            "\"tp_total\":%d,\"ww_total\":%d,"
            "\"reads\":\"UNCONDITIONAL running total - the nd_draw receipts below "
            "are DEDUPED per distinct node (cap 24/era) and cannot show per-frame "
            "activity. ww_total climbing = the draw walker RUNS on the WW stage; "
            "ww_total absent/flat = the walker itself is not reached\"}",
            ww ? 1 : 0, eraTotal, s_ndAllTp, s_ndAllWw);
    }

    void* node = mods::arg<void*>(args, 0);
    static void* seenWw[24]; static int nWw = 0;
    static void* seenTp[24]; static int nTp = 0;
    void** seen = ww ? seenWw : seenTp;
    int& n = ww ? nWw : nTp;
    for (int i = 0; i < n; i++) { if (seen[i] == node) { return HOOK_CONTINUE; } }
    if (n < 24) { seen[n++] = node; }
    logf(LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"nd_draw\",\"ww\":%d,\"distinct\":%d,\"node\":\"%p\","
        "\"reads\":\"census of NODES the draw walk visits; a WW scene node absent "
        "here was never inserted into its parent's draw structure\"}",
        ww ? 1 : 0, n, node);
    return HOOK_CONTINUE;
}

// #25 — the overlap gate's three predicates. One shared reporter: log each
// predicate's first few returns per stage side, then only TRANSITIONS (a
// predicate flipping value is the event; a pinned value is the finding and
// shows as first-values-then-silence plus the periodic pinned heartbeat).
// #27c: set at the warp_request moment (see the emission site) — the
// startStage flag lags to the new scene's phase_1 and cannot separate
// pre-warp from mid-transition.
bool s_warpMoment = false;

void ovlpReport(const char* which, int idx, void* retval) {
    // Sized for BOTH probe sets that share this reporter (#25 uses 0..2, #26
    // adds 3..4) — the original [3] would have been silent stack corruption
    // the moment #26 landed, inside the instrument hunting corruption.
    static int shown[8] = {0};
    static int lastVal[8] = {-99, -99, -99, -99, -99, -99, -99, -99};
    static int calls[8] = {0};
    // FIRST CALL AFTER THE WARP logs unconditionally, per predicate — the
    // flip reporter cannot distinguish "never called post-warp" from "called
    // with unchanged value", and that ambiguity has now cost two boot cycles.
    static bool postWarpSeen[8] = {false};
    if (s_warpMoment && !postWarpSeen[idx]) {
        postWarpSeen[idx] = true;
        const int v0 = (retval != nullptr) ? *static_cast<int*>(retval) : -1;
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"ovlp_gate\",\"which\":\"%s\",\"post_warp_first\":1,"
            "\"ret\":%d,\"reads\":\"this predicate IS called after the warp; absence "
            "of this line for a predicate means it is never reached\"}", which, v0);
    }
    const int v = (retval != nullptr) ? *static_cast<int*>(retval) : -1;
    calls[idx]++;
    const bool flip = (v != lastVal[idx]);
    lastVal[idx] = v;
    if ((flip && shown[idx] < 10) || (calls[idx] % 900) == 0) {
        if (flip) { shown[idx]++; }
        logf((v == 0 && calls[idx] >= 900) ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"ovlp_gate\",\"which\":\"%s\",\"ww\":%d,\"ret\":%d,"
            "\"calls\":%d,\"reads\":\"the scene-request fade path waits on these; "
            "one pinned at 0 post-warp = the step holding the black screen\"}",
            which, startStageIsWw() ? 1 : 0, v, calls[idx]);
    }
}
void on_ovlpIsDoing(ModContext*, void*, void* retval, void*) { ovlpReport("IsDoingReq", 0, retval); }
void on_ovlpIsDone(ModContext*, void*, void* retval, void*)  { ovlpReport("IsDone",     1, retval); }
void on_ovlpClear(ModContext*, void*, void* retval, void*)   { ovlpReport("ClearOfReq", 2, retval); }
// #26 — the overlap's states, through the same flip-reporter (indices 3..6).
void on_ovlpIsOutReq(ModContext*, void*, void* retval, void*)   { ovlpReport("IsOutReq",     3, retval); }
void on_ovlpSceneStart(ModContext*, void*, void* retval, void*) { ovlpReport("SceneIsStart", 4, retval); }
void on_ovlpDone(ModContext*, void*, void*, void*) {
    static int n = 0;
    n++;
    logf(LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"ovlp_done\",\"n\":%d,\"ww\":%d,"
        "\"reads\":\"leg completion; 1=fade-to-black done, 2=reveal done\"}",
        n, startStageIsWw() ? 1 : 0);
}
void on_jfwStartFadeOut(ModContext*, void* args, void* retval, void*) {
    static int shown = 0; static int lastV = -9; static int calls = 0;
    calls++;
    const int v = (retval != nullptr) ? (int)*static_cast<bool*>(retval) : -1;
    const bool flip = (v != lastV) && (shown < 10);
    if (flip) { shown++; }
    if (flip || (calls % 900) == 0) {
        lastV = v;
        logf((v == 0 && calls >= 900) ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"jfw_fadeout\",\"ww\":%d,\"accepted\":%d,\"speed\":%d,"
            "\"calls\":%d,\"reads\":\"false forever = overlap stuck at its FIRST step, "
            "the JUTFader refusing the wipe\"}",
            startStageIsWw() ? 1 : 0, v, mods::arg<int>(args, 1), calls);
    }
}

// #27 — the command and the pump.
void on_cReqCommand(ModContext*, void* args, void*, void*) {
    static int n = 0;
    n++;
    const unsigned cmd = (unsigned)mods::arg<unsigned char>(args, 1);
    // Run 160935: the flat n<=12 budget was spent by BOOT'S OWN transitions
    // (LOGO/OPENING/NAME burned all twelve) and the warp's commands - the one
    // question the probe existed to answer - fell past the cap unrecorded.
    // Third cap bite this session. cmd>=2 (the wipe-out handoff) is a
    // handful per SESSION, so it logs UNCONDITIONALLY; only the chatty cmd=1
    // stays budgeted.
    if (cmd >= 2 || n <= 12) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"creq_command\",\"n\":%d,\"ww\":%d,\"req\":\"%p\","
            "\"cmd\":%u,\"reads\":\"cmd=2 is the wipe-out handoff; one on the WW warp "
            "means the overlap was TOLD to reveal\"}",
            n, startStageIsWw() ? 1 : 0, mods::arg<void*>(args, 0), cmd);
    }
}

void on_cReqCreate(ModContext*, void* args, void*, void*) {
    static int n = 0;
    n++;
    const unsigned cmd = (unsigned)mods::arg<unsigned char>(args, 1);
    if (cmd >= 2 || n <= 8) {           // same shape as the command probe
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"creq_create\",\"n\":%d,\"ww\":%d,\"req\":\"%p\","
            "\"cmd\":%u,\"reads\":\"cmd=2 = the clear was ARMED (OverlapClr ran); "
            "armed-but-no-command = the WaitOfFadeout pump is dead\"}",
            n, startStageIsWw() ? 1 : 0, mods::arg<void*>(args, 0), cmd);
    }
}

// #28 — the constructors. Rare events; log every one.
void on_ovlpMRequest(ModContext*, void* args, void* retval, void*) {
    static int n = 0;
    n++;
    void* ret = (retval != nullptr) ? *static_cast<void**>(retval) : nullptr;
    logf(ret == nullptr ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"ovlp_request\",\"n\":%d,\"post_warp\":%d,\"procname\":%d,"
        "\"peektime\":%u,\"ret\":\"%p\",\"reads\":\"NULL ret = slot busy, fade refused; "
        "absence of this event on a warp = FadeRequest latch short-circuited before here\"}",
        n, s_warpMoment ? 1 : 0, (int)mods::arg<short>(args, 0),
        (unsigned)mods::arg<unsigned short>(args, 1), ret);
}

void on_scnMChangeReq(ModContext*, void* args, void* retval, void*) {
    static int n = 0;
    n++;
    const int rc = (retval != nullptr) ? *static_cast<int*>(retval) : -999;
    logf(rc < 0 ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"scn_change_req\",\"n\":%d,\"post_warp\":%d,\"proc\":%d,"
        "\"fade\":%d,\"peek\":%u,\"ret\":%d,\"reads\":\"negative ret = the request was "
        "DELETED (FadeRequest NULL, the l_fopScnRq_IsUsingOfOverlap latch) - the warp "
        "silently did not happen at the scene-manager level\"}",
        n, s_warpMoment ? 1 : 0, (int)mods::arg<short>(args, 1),
        (int)mods::arg<short>(args, 2), (unsigned)mods::arg<unsigned short>(args, 3), rc);
}

// #29 handlers.
void on_ndRqExecute(ModContext*, void* args, void* retval, void*) {
    static int lastRet = -99; static int shown = 0; static int calls = 0;
    static bool postFirst = false;
    calls++;
    const int rt = (retval != nullptr) ? *static_cast<int*>(retval) : -1;
    const bool pw = s_warpMoment && !postFirst;
    if (pw) { postFirst = true; }
    const bool flip = (rt != lastRet) && (shown < 12);
    if (flip) { shown++; }
    if (pw || flip || (calls % 900) == 0) {
        lastRet = rt;
        logf((rt == 0 && calls >= 900) ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"ndrq_exec\",\"post_warp\":%d,\"ret\":%d,\"calls\":%d,"
            "\"req\":\"%p\",\"reads\":\"0=INIT (swap still working), 2=NEXT (done); "
            "INIT forever post-warp = the delete-old/create-new swap is the stall\"}",
            s_warpMoment ? 1 : 0, rt, calls, mods::arg<void*>(args, 0));
    }
}

// ============================================================
// H11 AGGREGATE TOTALS (Integrator, 2026-08-21). The capped receipts above
// answer WHICH processes died; they cannot answer HOW MANY. The purge
// question ("were the 118 GRASS_e + 11 ITEM_e ever deleted?") needs totals,
// and both hooks were already counting in function-local statics that died
// unread. Promoted to file scope, emitted in the shutdown line. No behavior
// change to the capped receipts.
// ============================================================
int s_bsDelPost = 0;    // fpcBs_Delete calls post-warp (grants + refusals)
int s_bsDelRefused = 0; // of those, ret==0 refusals
int s_mDelPost = 0;     // fpcM_Delete calls post-warp

// ============================================================
// PER-CLASS DELETE CENSUS - the uncapped answer the totals cannot give
// ============================================================
// `bs_del_post` says HOW MANY died; the capped receipts say WHICH TEN died
// FIRST. Neither answers "did the purge reach GRASS_e", because the cap
// closes before that class is created. Counting by `profname` costs one
// table and no sampling: a class with count 0 was never deleted, and that
// is a fact about the CLASS rather than about the window.
// No enum values are hard-coded - the name comes from the process itself.
// ============================================================
const int kMaxDelClasses = 48;
short s_delName[kMaxDelClasses] = {0};
int   s_delCount[kMaxDelClasses] = {0};
int   s_delClasses = 0;
int   s_delOverflow = 0;

// ORDERING, because the CAPPED receipts cannot supply it and I tried to use
// them for exactly that. I attempted to test "the purge is a point-in-time
// sweep, so actors created later survive" by treating the LAST `bs_delete`
// receipt as the sweep's end - and the receipts stop at TEN by cap, not at
// the end of deleting. The arithmetic came out inverted and the test was
// vacuous. Third time the cap has bitten a reading tonight.
// `s_delFirstAt`/`s_delLastAt` record the `bs_del_post` INDEX at which each
// class was first and last deleted: ordering with no flood and no cap.
int s_delFirstAt[kMaxDelClasses] = {0};
int s_delLastAt[kMaxDelClasses] = {0};

void wwNoteDeleteClass(void* procRaw) {
    base_process_class* bp = static_cast<base_process_class*>(procRaw);
    if (bp == nullptr) { s_delOverflow++; return; }
    const short pn = bp->profname;
    for (int i = 0; i < s_delClasses; i++) {
        if (s_delName[i] == pn) {
            s_delCount[i]++;
            s_delLastAt[i] = s_bsDelPost;
            return;
        }
    }
    if (s_delClasses < kMaxDelClasses) {
        s_delName[s_delClasses] = pn;
        s_delCount[s_delClasses] = 1;
        s_delFirstAt[s_delClasses] = s_bsDelPost;
        s_delLastAt[s_delClasses] = s_bsDelPost;
        s_delClasses++;
    } else {
        s_delOverflow++;   // table full - REPORTED, never silently dropped
    }
}

void on_bsDelete(ModContext*, void* args, void* retval, void*) {
    if (!s_warpMoment) { return; }       // boot deletions are known-good noise
    const int rt = (retval != nullptr) ? *static_cast<int*>(retval) : -1;
    // Refusals always (they are the hang); grants first 10 (the grass wave).
    static int shownRefuse = 0, shownOk = 0;
    s_bsDelPost++;
    if (rt == 0) { s_bsDelRefused++; }
    wwNoteDeleteClass(mods::arg<void*>(args, 0));
    const int nPost = s_bsDelPost;
    if (rt == 0 ? (shownRefuse < 12) : (shownOk < 10)) {
        (rt == 0 ? shownRefuse : shownOk)++;
        logf(rt == 0 ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"bs_delete\",\"n\":%d,\"proc\":\"%p\",\"ret\":%d,"
            "\"reads\":\"ret=0 = the process REFUSED deletion this frame; the same proc "
            "refusing repeatedly is the swap's hostage - cross-ref via census_join\"}",
            nPost, mods::arg<void*>(args, 0), rt);
    }
}

// ============================================================================
// PROBE SET #30 HANDLERS — all bounded post-warp; every reader keys on
// s_warpMoment (the warp instant), never the laggy stage flag.
// ============================================================================
// ============================================================================
// PER-ID, NOT ONE SHARED lastRet - AND A FULL CENSUS, NOT TEN FLIP LINES.
//
// THE DEFECT THIS REPLACES, because the old output was READ AS A FINDING and
// published: the first cut compared `rt` against a SINGLE static `lastRet`
// shared across ALL ids, and capped output at 10 lines. An id returning 1
// immediately after another id returned 1 therefore NEVER LOGGED. The set of
// ids that appeared was an artifact of return-value INTERLEAVING, not a census
// of what was pinned - so "three ids are pinned, whatever is stuck is not one
// scene" was a claim the instrument could not support in either direction.
//
// A census answers "how many are pinned" in one line. A flip stream never can.
// ============================================================================
void on_mIsCreating(ModContext*, void* args, void* retval, void*) {
    if (!s_warpMoment) { return; }
    const unsigned int id = mods::arg<unsigned int>(args, 0);
    const int rt = (retval != nullptr) ? *static_cast<int*>(retval) : -1;

    static unsigned int ids[32]; static int lastRet[32]; static int hits[32];
    static int n = 0; static int calls = 0; static bool overflowed = false;
    calls++;

    int i = 0;
    for (; i < n; i++) { if (ids[i] == id) { break; } }
    if (i == n) {
        if (n >= 32) {
            // LOUD, not silent: a dropped id is a hole in the census and the
            // census is the whole point.
            if (!overflowed) {
                overflowed = true;
                logf(LOG_LEVEL_ERROR,
                    "[WwRegistry] {\"ev\":\"m_is_creating_overflow\",\"cap\":32,"
                    "\"reads\":\"more than 32 distinct ids - the census below is "
                    "TRUNCATED and its counts are a floor, not a total\"}");
            }
            return;
        }
        ids[n] = id; lastRet[n] = -99; hits[n] = 0; n++;
    }
    hits[i]++;
    const bool flip = (rt != lastRet[i]);
    lastRet[i] = rt;

    if (flip) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"m_is_creating\",\"id\":%u,\"ret\":%d,\"hits\":%d,"
            "\"reads\":\"PER-ID flip. ret=1 pinned for an id whose create completed "
            "= stuck on the creating list\"}", id, rt, hits[i]);
    }

    // FULL CENSUS - every id and its CURRENT ret, so 'how many are pinned' is
    // read off one line instead of inferred from a flip stream.
    if ((calls % 240) == 0) {
        char buf[512]; int off = 0; int pinned = 0;
        for (int k = 0; k < n && off < (int)sizeof(buf) - 24; k++) {
            if (lastRet[k] == 1) { pinned++; }
            off += snprintf(buf + off, sizeof(buf) - off, "%s%u:%d",
                            (k == 0) ? "" : ",", ids[k], lastRet[k]);
        }
        buf[sizeof(buf) - 1] = '\0';
        logf((pinned > 0) ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"m_is_creating_census\",\"ids\":%d,\"pinned\":%d,"
            "\"calls\":%d,\"map\":\"%s\",\"trunc\":%d,"
            "\"reads\":\"map is id:lastRet. pinned = COUNT of ids whose last ret was "
            "1. THIS is the number; the flip lines above are not a census\"}",
            n, pinned, calls, buf, overflowed ? 1 : 0);
    }
}

void on_scnPauseEnable(ModContext*, void* args, void*, void*) {
    static int n = 0;
    n++;
    if (n <= 8) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"scn_pause\",\"which\":\"ENABLE\",\"n\":%d,"
            "\"post_warp\":%d,\"scene\":\"%p\",\"reads\":\"H5: fires at post-create; "
            "Enable with no later Disable brackets the stall\"}",
            n, s_warpMoment ? 1 : 0, mods::arg<void*>(args, 0));
    }
}
void on_scnPauseDisable(ModContext*, void* args, void*, void*) {
    static int n = 0;
    n++;
    if (n <= 8) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"scn_pause\",\"which\":\"DISABLE\",\"n\":%d,"
            "\"post_warp\":%d,\"scene\":\"%p\",\"reads\":\"H5: fires only at the "
            "request's phase_Done - its presence post-warp means the machine FINISHED\"}",
            n, s_warpMoment ? 1 : 0, mods::arg<void*>(args, 0));
    }
}

int s_scnMgmtPre = 0, s_scnMgmtPost = 0;
HookAction on_scnMManagement(ModContext*, void*, void*, void*) {
    int& c = s_warpMoment ? s_scnMgmtPost : s_scnMgmtPre;
    c++;
    if (c == 1 || (c % 900) == 0) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"scnm_pump\",\"post_warp\":%d,\"calls\":%d,"
            "\"reads\":\"H7: the per-frame scene-manager pump; a frozen post-warp "
            "count starves every transition at once\"}",
            s_warpMoment ? 1 : 0, c);
    }
    return HOOK_CONTINUE;
}

void on_ndRqReRequest(ModContext*, void* args, void*, void*) {
    static int n = 0;
    n++;
    if (n <= 10 || (n % 300) == 0) {
        logf(n > 10 ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"ndrq_rerequest\",\"n\":%d,\"post_warp\":%d,"
            "\"id\":%u,\"proc\":%d,\"reads\":\"H10: a re-request RE-ARMS the phase "
            "machine; a spam of these = the transition restarts every frame and can "
            "never advance\"}",
            n, s_warpMoment ? 1 : 0, mods::arg<unsigned int>(args, 0),
            (int)mods::arg<short>(args, 1));
    }
}

void on_mDelete(ModContext*, void* args, void* retval, void*) {
    if (!s_warpMoment) { return; }
    s_mDelPost++;
    const int n = s_mDelPost;
    const int rt = (retval != nullptr) ? *static_cast<int*>(retval) : -1;
    if (n <= 12) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"m_delete\",\"n\":%d,\"victim\":\"%p\",\"ret\":%d,"
            "\"reads\":\"H6: every post-warp deletion by pointer; the OVERLAP task "
            "among the victims = the wipe was killed with nobody left to reveal\"}",
            n, mods::arg<void*>(args, 0), rt);
    }
}

// #31 handlers. The census keys on the REQUEST pointer: per distinct request
// post-warp, log the first pump, any return change, and a spin count at 300.
int s_ctRqDepth = 0;   // H10: incremented by the PRE hook, decremented in the
                       // POST — depth > 1 at post time means a nested pump
                       // is still on the stack beneath us.
HookAction on_ctRqDoPre(ModContext*, void*, void*, void*) {
    s_ctRqDepth++;
    return HOOK_CONTINUE;
}
void on_ctRqDo(ModContext*, void* args, void* retval, void*) {
    static int reentrySeen = 0;
    if (s_ctRqDepth > 1 && reentrySeen < 4) {
        reentrySeen++;
        logf(LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"ctrq_reentry\",\"depth\":%d,"
            "\"reads\":\"H10: the create pump reentered itself\"}", s_ctRqDepth);
    }
    s_ctRqDepth--;
    if (!s_warpMoment) { return; }
    void* req = mods::arg<void*>(args, 0);
    const int rt = (retval != nullptr) ? *static_cast<int*>(retval) : -1;
    // ========================================================================
    // PUMP COUNTS WERE UNOBSERVABLE BELOW 300, AND THE 33rd REQUEST VANISHED.
    //
    // THE DEFECT THIS REPLACES, because its output was read as a finding:
    // emission was `flip || (pumps % 300) == 0`, and `lastRt` starts at the
    // sentinel -99 so the FIRST sighting of ANY request always flips and always
    // logs at `pumps:1`. A request pumped ONCE and a request pumped 250 times
    // with a stable ret produced the IDENTICAL LINE. So "pumps is not climbing,
    // many entries each pumped once" described the EMISSION RULE, not the
    // runtime - and "one stuck entry re-pumped" vs "many entries pumped once"
    // are different faults with different fixes.
    //
    // Fix: a periodic FULL TALLY of every tracked request with its pump count,
    // plus a LOUD overflow. The tally is the discriminator; the flip line is not.
    // ========================================================================
    static void* reqs[32]; static int lastRt[32]; static int pumps[32];
    static int n = 0; static int calls = 0; static bool overflowed = false;
    calls++;
    int i = 0;
    for (; i < n; i++) { if (reqs[i] == req) { break; } }
    if (i == n) {
        if (n >= 32) {
            if (!overflowed) {
                overflowed = true;
                logf(LOG_LEVEL_ERROR,
                    "[WwRegistry] {\"ev\":\"ctrq_overflow\",\"cap\":32,"
                    "\"reads\":\"more than 32 distinct requests - the tally below is "
                    "TRUNCATED. The old build dropped these SILENTLY, which is why a "
                    "141-request census looked like 32\"}");
            }
            return;
        }
        reqs[n] = req; lastRt[n] = -99; pumps[n] = 0; n++;
    }
    pumps[i]++;
    const bool flip = (rt != lastRt[i]);
    lastRt[i] = rt;
    if (flip) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"ctrq_do\",\"req\":\"%p\",\"ret\":%d,\"pumps\":%d,"
            "\"reads\":\"a FLIP only. Do NOT read pumps:1 here as 'pumped once' - "
            "first sighting always flips off the sentinel. Use ctrq_tally\"}",
            req, rt, pumps[i]);
    }
    if ((calls % 240) == 0) {
        char buf[512]; int off = 0; int maxPumps = 0; int stuck = 0;
        for (int k = 0; k < n && off < (int)sizeof(buf) - 28; k++) {
            if (pumps[k] > maxPumps) { maxPumps = pumps[k]; }
            if (lastRt[k] == 1) { stuck++; }
            off += snprintf(buf + off, sizeof(buf) - off, "%s%d/%d",
                            (k == 0) ? "" : ",", pumps[k], lastRt[k]);
        }
        buf[sizeof(buf) - 1] = '\0';
        logf(LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"ctrq_tally\",\"reqs\":%d,\"calls\":%d,"
            "\"max_pumps\":%d,\"ret1\":%d,\"pumps_ret\":\"%s\",\"trunc\":%d,"
            "\"reads\":\"THE DISCRIMINATOR. max_pumps HIGH with few reqs = ONE entry "
            "re-pumped and never completing. max_pumps LOW with many reqs = MANY "
            "entries each pumped once and abandoned. These are different faults\"}",
            n, calls, maxPumps, stuck, buf, overflowed ? 1 : 0);
    }
}

void on_exToExecuteQ(ModContext*, void* args, void* retval, void*) {
    if (!s_warpMoment) { return; }
    const int rt = (retval != nullptr) ? *static_cast<int*>(retval) : -1;
    static int shown = 0;
    if (rt == 0 && shown < 8) {          // refusals only — the silent-kill branch
        shown++;
        logf(LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"ex_toq_refused\",\"proc\":\"%p\","
            "\"reads\":\"execute-queue admission REFUSED -> fpcCtRq_Cancel kills the "
            "process silently\"}", mods::arg<void*>(args, 0));
    }
}

// H4/H7 — pop refusals and zombie cancels. Rare; refusals log always.
void on_ctRqDelete(ModContext*, void* args, void* retval, void*) {
    if (!s_warpMoment) { return; }
    const int rt = (retval != nullptr) ? *static_cast<int*>(retval) : -1;
    static int shown = 0;
    if (rt == 0 && shown < 8) {
        shown++;
        logf(LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"ctrq_delete_refused\",\"req\":\"%p\","
            "\"reads\":\"H4: the pop's Delete returned FALSE (delete_method vetoed) - "
            "the entry may linger half-removed\"}", mods::arg<void*>(args, 0));
    }
}
void on_ctRqCancel(ModContext*, void* args, void* retval, void*) {
    if (!s_warpMoment) { return; }
    const int rt = (retval != nullptr) ? *static_cast<int*>(retval) : -1;
    static int n = 0;
    n++;
    if (n <= 8 || rt == 0) {
        logf(rt == 0 ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"ctrq_cancel\",\"n\":%d,\"req\":\"%p\",\"ret\":%d,"
            "\"reads\":\"H7: ret=0 = cancel stalled mid-teardown, is_cancel latched, "
            "entry zombie\"}", n, mods::arg<void*>(args, 0), rt);
    }
}

// H5 — every post-warp enqueue, so duplicates for one process become visible
// by pairing these against ctrq_do's request pointers.
void on_ctRqEnqueue(ModContext*, void* args, void*, void*) {
    if (!s_warpMoment) { return; }
    static int n = 0;
    n++;
    if (n <= 24) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"ctrq_enqueue\",\"n\":%d,\"req\":\"%p\","
            "\"reads\":\"H5: post-warp creating-queue entry; two entries whose pumps "
            "resolve to one process = the duplicate\"}", n, mods::arg<void*>(args, 0));
    }
}

// H6 — the layer creating/created message balance. A net that never returns
// to zero post-warp is a leaked creating-count on some layer.
int s_lyMesgNet = 0;
void on_lyCreatingMesg(ModContext*, void*, void*, void*) {
    if (s_warpMoment) { s_lyMesgNet++; }
}
void on_lyCreatedMesg(ModContext*, void*, void*, void*) {
    if (!s_warpMoment) { return; }
    s_lyMesgNet--;
    static int reports = 0;
    static int calls = 0;
    calls++;
    if ((calls % 120) == 0 && reports < 8) {
        reports++;
        logf(s_lyMesgNet > 0 ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"ly_mesg_net\",\"net\":%d,"
            "\"reads\":\"H6: creating-minus-created since the warp; a positive value "
            "that never decays = leaked creating-count\"}", s_lyMesgNet);
    }
}

// H8 — layer deletions; a layer dying while entries queued under it would
// orphan them from the walk.
void on_lyDelete(ModContext*, void* args, void* retval, void*) {
    if (!s_warpMoment) { return; }
    static int n = 0;
    n++;
    if (n <= 8) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"ly_delete\",\"n\":%d,\"layer\":\"%p\",\"ret\":%d,"
            "\"reads\":\"H8: a layer deleted post-warp; stuck entries queued under it "
            "are orphaned from the create walk\"}",
            n, mods::arg<void*>(args, 0),
            (retval != nullptr) ? *static_cast<int*>(retval) : -1);
    }
}

// #33 — registration receipts. Rare per stage; log every WW-side event.
// ============================================================================
// THE SET-TIME AABB RECEIPT — the last box in the Outset ground trace.
// `dzb_walk_sim.py` proved the DISC index sound: brute-force and index-walk
// both reach the floor at Link's exact column (y=580). The one element no
// disc-side measurement can check is `pm_node_tree` — the AABB array cBgW::Set
// BUILDS AT RUNTIME from the vertex table. A wrong AABB set is the only
// remaining thing that explains "some columns answer, others don't" with
// healthy data underneath, because it rejects a subtree before any triangle
// is examined.
// This POST hook reads, from the just-Set BgW: the vertex count/table it
// latched, and node 0's computed min/max. Offsets MEASURED from the image's
// own PDB (cBgW: pm_vtx_tbl@184, pm_bgd@192, pm_node_tree@216; cM3dGAab:
// mMin@8, mMax@20) — never authored. Compare the printed box against the
// same node computed offline from the disc: agreement clears the AABBs and
// moves the hunt to the query side; disagreement IS the bug, located.
// ============================================================================
void logSetAabb(void* self) {
    unsigned char* b = static_cast<unsigned char*>(self);
    void* bgd = *reinterpret_cast<void**>(b + 192);
    void* vtx = *reinterpret_cast<void**>(b + 184);
    void* tree = *reinterpret_cast<void**>(b + 216);
    if (bgd == nullptr || tree == nullptr) {
        logf(LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"set_aabb\",\"bgd\":\"%p\",\"node_tree\":\"%p\","
            "\"reads\":\"Set left a NULL structure - nothing to query\"}", bgd, tree);
        return;
    }
    // cBgD_t header through the receiver's own BE convention: v_num is a
    // big-endian int at +0 (the disc form the collision path consumes).
    const unsigned char* h = static_cast<const unsigned char*>(bgd);
    const unsigned int vRaw = *reinterpret_cast<const unsigned int*>(h);
    const unsigned int vNum = ((vRaw & 0xFFu) << 24) | ((vRaw & 0xFF00u) << 8) |
                              ((vRaw & 0xFF0000u) >> 8) | ((vRaw & 0xFF000000u) >> 24);
    const float* n0 = reinterpret_cast<const float*>(
        static_cast<unsigned char*>(tree) + 8);      // cM3dGAab::mMin
    const float* n0max = reinterpret_cast<const float*>(
        static_cast<unsigned char*>(tree) + 20);     // cM3dGAab::mMax
    // THE DISAMBIGUATOR (Foundry, 2026-08-17): +INF/-INF has TWO causes, not
    // one. `MakeBlckBnd` has two arms and only one reads vertices; when
    // `mNeedsFullTransform == 0` it takes MakeBlckTransMinMax, which merely
    // ADDS mTransVel to the existing box — and +INF + finite == +INF, so a
    // freshly-cleared box down that arm is BIT-IDENTICAL to the pre-swap
    // outcome. Logging the flag turns a coin flip into a diagnosis.
    // cBgW::mFlags@156, mNeedsFullTransform@157 — PDB-measured.
    const unsigned char flags = *(b + 156);
    const unsigned char needsFull = *(b + 157);
    logf(LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"set_aabb\",\"flags\":%u,\"needs_full_transform\":%u,"
        "\"v_num_be\":%u,\"vtx_tbl\":\"%p\","
        "\"node0_min\":[%.1f,%.1f,%.1f],\"node0_max\":[%.1f,%.1f,%.1f],"
        "\"reads\":\"v_num must read 3722 for R44. EXPECTED node0 box (two lanes, "
        "two parsers, agreeing with the MakeBlckBnd +/-1 pad): min(-211453.9, "
        "-4991.0, 310611.8) max(-188296.1, -4989.0, 328111.2). NOT the "
        "whole-table extent - node 0 is ONE OF 83 ROOTS (the tree is a FOREST), "
        "so a box spanning only ~23k x 17k is CORRECT, not truncated. THE "
        "DISCRIMINATOR IS THE Y AXIS: min.y=+INF / max.y=-INF is a COLLAPSED box "
        "(pre-swap vertices at Set time, every Y compare failing, sentinels "
        "surviving) and rejects every query; -4991/-4989 means the AABBs are "
        "sound and the fault is downstream. AND IF Y IS +INF, READ "
        "needs_full_transform FIRST: 0 means the box went down the TRANSLATE "
        "arm (MakeBlckTransMinMax adds mTransVel and never reads vertices; "
        "+INF+finite==+INF) which is a DIFFERENT fault from pre-swap vertices\"}",
        (unsigned)flags, (unsigned)needsFull,
        vNum, vtx, n0[0], n0[1], n0[2], n0max[0], n0max[1], n0max[2]);
}

void on_cBgWSet(ModContext*, void* args, void* retval, void*) {
    if (!startStageIsWw()) { return; }
    static int n = 0;
    n++;
    const int rt = (retval != nullptr) ? (int)*static_cast<bool*>(retval) : -1;
    if (n <= 4 && rt == 0) { logSetAabb(mods::arg<void*>(args, 0)); }
    if (n <= 12) {
        logf(rt != 0 ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"cbgw_set\",\"n\":%d,\"dzb\":\"%p\",\"ret\":%d,"
            "\"reads\":\"the collision wrap; ret nonzero = FAILED at the call site "
            "(d_a_bg.cpp:220 early-returns silently on it)\"}",
            n, mods::arg<void*>(args, 1), rt);
    }
}
void on_dBgSRegist(ModContext*, void* args, void* retval, void*) {
    // The akabe port's Bgsp instance capture — the receiver hands us its own
    // `this` on every registration (TP boot included), long before any WW
    // actor can create. Zero offsets, zero extra resolves.
    if (s_bgspInstance == nullptr) {
        s_bgspInstance = mods::arg<void*>(args, 0);
    }
    if (!startStageIsWw()) { return; }
    static int n = 0;
    n++;
    const int rt = (retval != nullptr) ? (int)*static_cast<bool*>(retval) : -1;
    if (n <= 16) {
        logf(rt != 0 ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"dbgs_regist\",\"n\":%d,\"bgw\":\"%p\",\"ret\":%d,"
            "\"reads\":\"the SEARCHABLE set's admission; ret nonzero = REFUSED and "
            "daBg returns cPhs_ERROR (silent BG deletion); zero WW events = the "
            "phase never reached :599\"}",
            n, mods::arg<void*>(args, 1), rt);
    }
}
void on_groundCross(ModContext*, void* args, void* retval, void*) {
    if (!startStageIsWw()) { return; }
    static int calls = 0, infs = 0, firstHit = 0, missShown = 0;
    calls++;
    const float y = (retval != nullptr) ? *static_cast<float*>(retval) : -1.0e9f;
    // SENTINEL CORRECTED (run 181622): G_CM3D_F_INF is 1e9 (c_m3d.h:22), not
    // the 1e17 I guessed — the first receipt logged the sentinel itself as
    // "ground exists" four times. An optimistic instrument is worse than none.
    const bool inf = (y <= -0.99e9f);
    // Query POSITION (run 183144's paradox: ground lives at y=580 while ALINK
    // still loops — WHOSE queries hit and whose miss is now the question).
    // cBgS_GndChk::m_pos @ +64, MEASURED from the image's PDB type stream —
    // not authored.
    const float* qp = reinterpret_cast<const float*>(
        static_cast<const char*>(mods::arg<void*>(args, 1)) + 64);
    if (inf) { infs++; }
    if (!inf && firstHit < 6) {
        firstHit++;
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"ground_query\",\"hit\":1,\"y\":%.1f,"
            "\"from\":[%.0f,%.0f,%.0f],\"gndchk\":\"%p\",\"calls\":%d,"
            "\"reads\":\"a HIT, where cast from, and WHICH query object\"}",
            y, qp[0], qp[1], qp[2], mods::arg<void*>(args, 1), calls);
    }
    if (inf && (missShown < 6 || (calls % 600) == 0)) {
        missShown++;
        logf(LOG_LEVEL_WARN,
            "[WwRegistry] {\"ev\":\"ground_query\",\"hit\":0,\"from\":[%.0f,%.0f,%.0f],"
            "\"gndchk\":\"%p\",\"infs\":%d,\"calls\":%d,\"reads\":\"a MISS with its "
            "query object - cross-reference gndchk against rwg_gnd: same pointer with "
            "poly 410 accepted means the acceptance is being DISCARDED downstream\"}",
            qp[0], qp[1], qp[2], mods::arg<void*>(args, 1), infs, calls);
    }
}
// PRECISE NOW — the scan version worked but could not tell WHOSE acch it was
// reading (it reported inf_words=3 on every call while plausible Ys drifted
// between actors). Fields taken from the image's PDB instead of scanned:
//   dBgS_Acch::m_flags @72 · m_my_ac @224 · m_ground_h @256
// (`GetGroundH()` is literally `return m_ground_h`, d_bg_s_acch.h:125), and
// FLAG_GROUND_HIT/FIND live in m_flags. Reporting the owning actor lets the
// player's own acch be picked out of the crowd.
// ============================================================================
// PHASE 4a — THE ROOF-CLAMP SEAM, RE-SITED OFF A RECEIVER EDIT (Integrator,
// 2026-08-17, user-approved). FIRST seam moved onto the host's OWN switch.
//
// REPLACES `d_bg_s_acch.cpp:170` (fork tale §818), which added a WW-host test
// beside TP's thin-ceiling roof clamp:
//     if (!wwHost818 && !ChkGndThinCellingOff()) { ...roof clamp... }
// Its own comment gives the reason: the donor's GroundCheck has NO roof check
// for any actor, donor placements sit at EXACTLY floor Y, so the clamp lands
// the ground-query start ON the plane and the actor falls through geometry.
//
// DN-10 STEP 1 — THE RECEIVER ALREADY HAS THIS SWITCH. `SetGndThinCellingOff`
// sets FLAG_GND_THIN_CELLING_OFF; the clamp is skipped when it is set. So
// `wwHost818 == true` and the flag being set are the SAME EFFECT. We author no
// behaviour; we throw the receiver's own switch. Both setters verified SAFE
// (resolve to exactly one) in the SHIPPING VANILLA manifest.
//
// AND THE FORK TRIED THIS FIRST: §796 set the flag for the PLAYER only; §817
// found the pots dying the same way in Sturgeon's room; §818 gave up on the
// flag and went stage-wide. THE MECHANISM WAS NEVER WRONG — COVERAGE WAS. An
// edit sets the flag where you edited it; a hook on the shared choke point
// sets it for every consumer. §818's coverage via §796's mechanism.
//
// WHY A SECOND CALLBACK ON A HOOK THAT ALREADY EXISTS, rather than folding
// this into `on_acchCrrPos` below: that one is a MEASUREMENT INSTRUMENT, gated
// on `s_diagProbes` and switchable off. This gate must run whenever the plugin
// is active. Folding a behaviour into a probe makes the probe un-disableable
// and hides the behaviour inside a diagnostic. Add and label; do not
// substitute.
//
// THE CLR ARM IS LOAD-BEARING, NOT SYMMETRY: `|=` never clears itself, so
// without it a TP stage entered after a WW stage inherits donor ground
// semantics on every acch that ever visited one — a TP-content bug caused by
// WW code, which is the exact shape the WW shared-path scoping rule exists to
// stop.
//
// NOT MEASURED: the per-frame trampoline cost. `CrrPos` has 444 call sites
// across 293 files. Phase 5 warns hot per-frame subsystems may belong
// tree-side; this is the first candidate to measure, and it is why the counters
// below exist.
// ============================================================================
typedef void (*FnAcchFlag)(void*);
FnAcchFlag s_fnAcchSetThinCeilOff = nullptr;
FnAcchFlag s_fnAcchClrThinCeilOff = nullptr;
bool s_acchFlagResolveTried = false;
unsigned s_acchSetCalls = 0;
unsigned s_acchClrCalls = 0;

// Call-don't-hook, the same pattern as `getStartStageName`: these are setters
// we INVOKE on the receiver's own object, not seams we intercept.
void resolveAcchFlagFns() {
    if (s_acchFlagResolveTried) { return; }
    s_acchFlagResolveTried = true;
    void* a = nullptr;
    if (s_hook->resolve(mod_ctx, "?SetGndThinCellingOff@dBgS_Acch@@QEAAXXZ", &a, nullptr) == MOD_OK) {
        s_fnAcchSetThinCeilOff = reinterpret_cast<FnAcchFlag>(a);
    }
    a = nullptr;
    if (s_hook->resolve(mod_ctx, "?ClrGndThinCellingOff@dBgS_Acch@@QEAAXXZ", &a, nullptr) == MOD_OK) {
        s_fnAcchClrThinCeilOff = reinterpret_cast<FnAcchFlag>(a);
    }
    logf(LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"acch_thinceil_resolve\",\"set\":%d,\"clr\":%d,"
        "\"reads\":\"both 1 => the roof-clamp seam is re-sited onto the host's own "
        "flag and d_bg_s_acch.cpp:170 is not needed; a 0 means the gate is INERT "
        "and the receiver edit is still load-bearing\"}",
        s_fnAcchSetThinCeilOff != nullptr ? 1 : 0,
        s_fnAcchClrThinCeilOff != nullptr ? 1 : 0);
}

// PRE hooks return HookAction (a pre hook may suppress the original); POST
// hooks return void. This one never suppresses: the receiver's CrrPos must
// always run, we only set a flag it will read.
HookAction on_acchCrrPosGate(ModContext*, void* args, void*, void*) {
    resolveAcchFlagFns();
    void* self = mods::arg<void*>(args, 0);
    if (self == nullptr) { return HOOK_CONTINUE; }
    if (startStageIsWw()) {
        if (s_fnAcchSetThinCeilOff != nullptr) { s_fnAcchSetThinCeilOff(self); s_acchSetCalls++; }
    } else if (s_fnAcchClrThinCeilOff != nullptr) {
        s_fnAcchClrThinCeilOff(self);
        s_acchClrCalls++;
    }
    // ------------------------------------------------------------------
    // INVOCATION RECEIPT — added after the 211106 boot, which proved the
    // gate INSTALLED (r:0) and RESOLVED (set:1 clr:1) and could not show
    // whether it ever FIRED. Announce != record != applied != effective;
    // the first two had receipts and the third did not, so "green" was
    // being read one step further than the evidence reached.
    // Log-once at 1, then sparsely, so a hot path cannot flood the log.
    // ------------------------------------------------------------------
    // THRESHOLD CHOICE, corrected after boot 211829. Totals of 1/100/10000 all
    // landed in menu frames, where startStageIsWw() is false: the log read
    // "set":0,"clr":100 and said nothing about the arm that matters. A counter
    // keyed on VOLUME reports when the hot path is hot; the interesting event
    // is the TRANSITION into the WW arm. Fire on the FIRST set, and on the
    // first clr, then sparsely.
    static bool s_firstSetShown = false;
    const unsigned total = s_acchSetCalls + s_acchClrCalls;
    const bool firstSet = (s_acchSetCalls == 1 && !s_firstSetShown);
    if (firstSet) { s_firstSetShown = true; }
    if (firstSet || total == 1 || total == 10000) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"acch_thinceil_fired\",\"total\":%u,\"set\":%u,"
            "\"clr\":%u,\"reads\":\"the gate is INVOKED, not merely installed; set>0 on a WW "
            "stage means the receiver's own thin-ceiling flag is being thrown for every acch "
            "user, which is what d_bg_s_acch.cpp:170 was edited to do\"}",
            total, s_acchSetCalls, s_acchClrCalls);
    }
    return HOOK_CONTINUE;
}

void on_acchCrrPos(ModContext*, void* args, void*, void*) {
    if (!startStageIsWw()) { return; }
    static int shown = 0;
    if (shown >= 20) { return; }
    const unsigned char* b = static_cast<const unsigned char*>(mods::arg<void*>(args, 0));
    float groundH; std::memcpy(&groundH, b + 256, 4);
    unsigned int flags; std::memcpy(&flags, b + 72, 4);
    void* owner; std::memcpy(&owner, b + 224, 8);
    // Only report the PLAYER's acch (and a couple of others for contrast).
    const bool isPlayer = (s_fnGetPlayerActor != nullptr) &&
                          (owner != nullptr) && (owner == s_fnGetPlayerActor());
    static int others = 0;
    if (!isPlayer && others >= 3) { return; }
    if (!isPlayer) { others++; }
    shown++;
    logf(LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"acch_ground\",\"n\":%d,\"is_player\":%d,"
        "\"owner\":\"%p\",\"ground_h\":%.1f,\"flags\":\"0x%08X\","
        "\"reads\":\"is_player=1 is THE value ALINK's create gate tests. "
        "-1e9 means the acch layer lost a result the raw query FOUND (poly 410 "
        "returned true); ~580 means acch resolved and the gate fails elsewhere\"}",
        shown, isPlayer ? 1 : 0, owner, groundH, flags);
}

void on_rwgGndCheck(ModContext*, void* args, void* retval, void*) {
    if (!startStageIsWw()) { return; }
    const float* qp = reinterpret_cast<const float*>(
        static_cast<const char*>(mods::arg<void*>(args, 2)) + 64);
    if (qp[0] > -205656.f && qp[0] < -205536.f &&
        qp[2] > 316502.f && qp[2] < 316622.f) {
        static int shown = 0;
        if (shown < 24) {
            shown++;
            logf(LOG_LEVEL_INFO,
                "[WwRegistry] {\"ev\":\"rwg_gnd\",\"n\":%d,\"poly\":%u,\"ret\":%d,"
                "\"gndchk\":\"%p\",\"reads\":\"gndchk IDENTIFIES THE QUERY. poly 410 "
                "returns TRUE here, yet the player's query reports -INF - if these "
                "gndchk pointers differ, the accepted triangle belongs to a DIFFERENT "
                "query and the player's own never sees it\"}",
                shown, (unsigned)mods::arg<unsigned short>(args, 1),
                (retval != nullptr) ? (int)*static_cast<bool*>(retval) : -1,
                mods::arg<void*>(args, 2));
        }
    }
}

HookAction on_bgwGroundCrossRp(ModContext*, void* args, void*, void*) {
    if (!startStageIsWw()) { return HOOK_CONTINUE; }
    const float* qp = reinterpret_cast<const float*>(
        static_cast<const char*>(mods::arg<void*>(args, 1)) + 64);
    // Link's column: point 11 spawn (-205596, 316562), 60-unit box.
    if (qp[0] > -205656.f && qp[0] < -205536.f &&
        qp[2] > 316502.f && qp[2] < 316622.f) {
        static int shown = 0;
        if (shown < 48) {
            shown++;
            logf(LOG_LEVEL_INFO,
                "[WwRegistry] {\"ev\":\"walk_node\",\"n\":%d,\"node\":%d,\"bgw\":\"%p\","
                "\"reads\":\"tree node visited for a query in Link's column - compare "
                "this set against History's offline expected-path oracle\"}",
                shown, mods::arg<int>(args, 2), mods::arg<void*>(args, 0));
        }
    }
    return HOOK_CONTINUE;
}

void on_bgwGroundCross(ModContext*, void* args, void* retval, void*) {
    if (!startStageIsWw()) { return; }
    static int calls = 0, trues = 0, shownDistinct = 0;
    static void* seen[8]; static int nSeen = 0;
    calls++;
    const int rt = (retval != nullptr) ? (int)*static_cast<bool*>(retval) : -1;
    if (rt == 1) { trues++; }
    void* self = mods::arg<void*>(args, 0);
    bool fresh = true;
    for (int i = 0; i < nSeen; i++) { if (seen[i] == self) { fresh = false; break; } }
    if (fresh && nSeen < 8) { seen[nSeen++] = self; }
    if ((fresh && shownDistinct < 8) || (rt == 1 && trues <= 3) || (calls % 900) == 0) {
        if (fresh) { shownDistinct++; }
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"bgw_ground\",\"bgw\":\"%p\",\"ret\":%d,\"calls\":%d,"
            "\"trues\":%d,\"distinct\":%d,\"reads\":\"per-BgW query; the ROOM BgW absent "
            "from distinct = never reached by the element loop; present with trues=0 = "
            "its internal walk (block tree / RwgCheck) rejects everything\"}",
            self, rt, calls, trues, nSeen);
    }
}
void on_roomSetBgW(ModContext*, void* args, void*, void*) {
    if (!startStageIsWw()) { return; }
    static int n = 0;
    n++;
    if (n <= 12) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"room_set_bgw\",\"n\":%d,\"roomNo\":%d,\"bgw\":\"%p\","
            "\"reads\":\"REGISTRATION RAN for this room; NULL bgw = the teardown "
            "mirror; roomNo must be the REAL room (DN-1)\"}",
            n, mods::arg<int>(args, 0), mods::arg<void*>(args, 1));
    }
}

int s_ctPumpPre = 0, s_ctPumpPost = 0;
void on_ctRqHandler(ModContext*, void*, void*, void*) {
    int& c = s_warpMoment ? s_ctPumpPost : s_ctPumpPre;
    c++;
    if (c == 1 || (c % 900) == 0) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"ct_pump\",\"post_warp\":%d,\"calls\":%d,"
            "\"reads\":\"the create-queue pump; frozen post-warp = entries never "
            "pumped at all (iterator-side fault)\"}", s_warpMoment ? 1 : 0, c);
    }
}

int s_ndRqPumpWw = 0, s_ndRqPumpTp = 0;
void on_ndRqHandler(ModContext*, void*, void*, void*) {
    int& c = startStageIsWw() ? s_ndRqPumpWw : s_ndRqPumpTp;
    c++;
    if (c == 1 || (c % 900) == 0) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"ndrq_pump\",\"ww\":%d,\"calls\":%d,"
            "\"reads\":\"the request pump; a side whose count freezes is a starved "
            "request machine\"}", startStageIsWw() ? 1 : 0, c);
    }
}

int s_dwWipes = 0;
HookAction on_dwTgWipe(ModContext*, void*, void*, void*) {
    s_dwWipes++;
    logf(LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"drawq_wipe\",\"n\":%d,\"ww\":%d,\"ww_admits_so_far\":%d,"
        "\"reads\":\"tree recreated; every earlier admission is orphaned - a wipe "
        "AFTER ww admissions is the lost-in-the-wash race\"}",
        s_dwWipes, startStageIsWw() ? 1 : 0, s_drawQAdmitWw);
    return HOOK_CONTINUE;
}

int s_dwCuts = 0;
void on_dwTgCut(ModContext*, void* args, void*, void*) {
    s_dwCuts++;
    void* tag = mods::arg<void*>(args, 0);
    bool wasWwAdmit = false;
    for (int i = 0; i < 64; i++) { if (s_admitRing[i] == tag) { wasWwAdmit = true; break; } }
    static int shownWw = 0, shownOther = 0;
    if (wasWwAdmit ? (shownWw < 16) : (shownOther < 4)) {
        (wasWwAdmit ? shownWw : shownOther)++;
        logf(wasWwAdmit ? LOG_LEVEL_ERROR : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"drawq_cut\",\"n\":%d,\"tag\":\"%p\",\"ww_admitted_tag\":%d,"
            "\"reads\":\"ww_admitted_tag=1 means a COMPLEATED WW actor was removed from "
            "the draw tree - the deleter is the next trace (room control suspect)\"}",
            s_dwCuts, tag, wasWwAdmit ? 1 : 0);
    }
}

int s_dwExec = 0;
// UNGATED FROM WW-ONLY 2026-08-16. The WW draw set is 1 process; that is
// only meaningful against the TP number, and I do not have it. Worse, I
// nearly reported `layer: -1` as the cause - until `src/dusk/ui/warp.cpp:20`
// showed dusklight's own TP warp defaults to the SAME -1 and renders fine.
// A value shared by the working case cannot be the differentiator.
// So: count BOTH sides. One line per stage, distinct sets, no throttle race.
HookAction on_dwExecute(ModContext*, void* args, void*, void*) {
    const bool ww = startStageIsWw();
    s_dwExec++;
    // One frame's worth is the whole answer: the set of processes drawn is
    // stable, so enumerate it once rather than every frame forever.
    // Distinct processes per stage-kind, reported once each. A COUNT is the
    // comparison; the pointers are how they get named against
    // `fpcBs_Create OK proc=...` afterwards.
    static void* seenWw[64]; static int nWw = 0;
    static void* seenTp[64]; static int nTp = 0;
    void* pr = mods::arg<void*>(args, 0);
    void** seen = ww ? seenWw : seenTp;
    int&   n    = ww ? nWw : nTp;
    for (int i = 0; i < n; i++) { if (seen[i] == pr) { return HOOK_CONTINUE; } }
    if (n < 64) { seen[n++] = pr; }
    if (n <= 40) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"dw_exec\",\"ww\":%d,\"distinct\":%d,\"proc\":\"%p\","
            "\"reads\":\"cross-reference with fpcBs_Create OK proc=... to name it; a BG proc absent here is never asked to draw; compare WW distinct vs TP distinct\"}",
            ww ? 1 : 0, n, pr);
    }
    return HOOK_CONTINUE;
}

int s_clipFar = 0;
HookAction on_clipChangeFar(ModContext*, void* args, void*, void*) {
    if (!startStageIsWw()) { return HOOK_CONTINUE; }
    s_clipFar++;
    if (s_clipFar <= 4 || (s_clipFar % 600) == 0) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"clip_far\",\"n\":%d,\"far\":%.1f,"
            "\"bg_draws\":%d,\"draw_entries\":%d,\"reads\":\"fires => draw() runs and BgDraw is inlined-dead; silent => draw() never runs\"}",
            s_clipFar, (double)mods::arg<float>(args, 0), s_bgDraws, s_drawEntries);
    }
    return HOOK_CONTINUE;
}


int s_modelCreates = 0, s_modelNulls = 0;
void on_j3dModelCreate(ModContext*, void* args, void* retval, void*) {
    if (!startStageIsWw()) { return; }
    void* out = (retval != nullptr) ? *static_cast<void**>(retval) : nullptr;
    void* data = mods::arg<void*>(args, 0);
    s_modelCreates++;
    if (out == nullptr) { s_modelNulls++; }
    if (s_modelCreates <= 12 || out == nullptr) {
        logf(out != nullptr ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"model_create\",\"n\":%d,\"data\":\"%p\","
            "\"out\":\"%p\",\"nulls\":%d,\"note\":\"NULL here makes "
            "daBg_c::createHeap return 0, which DELETES the actor\"}",
            s_modelCreates, data, out, s_modelNulls);
    }
}

// ============================================================================
// GX CENSUS HANDLERS. Counters only - no behaviour change, nothing suppressed.
// Reported per WW-stage frame so the per-frame TREND is visible: a one-off
// imbalance is noise, a gap that grows every frame is the desync.
// ============================================================================
int s_gxCallDlCalls = 0;
unsigned long long s_gxCallDlBytes = 0;
int s_gxBeginDl = 0;
int s_gxEndDl = 0;
int s_gxFrames = 0;

int s_addShapeCalls = 0;
int s_selfLoops = 0;
void* s_entrySubject = nullptr;   // matPacket currently inside entryMatSort
int s_entryCalls = 0;

// Per-lwood-frame entry tally: how many times each matPacket is entered.
// A matPacket entered TWICE in one frame is what puts it in its own bucket.
void* s_entrySeen[32];
int s_entrySeenN = 0;
int s_entryDupes = 0;

int s_dlstResets = 0;
int s_entriesThisFrame = 0;
int s_lwoodDrawsThisFrame = 0;
int s_prevFrameEntries = 0;
int s_prevFrameLwoodDraws = 0;
int s_peakFrameEntries = 0;

HookAction on_dlstReset(ModContext*, void*, void*, void*) {
    // Every bucket is about to be NULLed, so nothing can still be "already in
    // its bucket" past this point. This is the honest frame boundary.
    s_dlstResets++;   // RESTORED: dropped when this block was repaired, which is
                      // why every receipt read resets:0 and the baseline logged
                      // every frame instead of every 60th.
    // Snapshot the frame that just ENDED before clearing. The baseline matters:
    // if the whole draw list replays, this total roughly DOUBLES; if only lwood
    // re-enters, it rises by a handful. Those are different bugs with different
    // fixes, and the number separates them without another guess.
    s_prevFrameEntries = s_entriesThisFrame;
    s_prevFrameLwoodDraws = s_lwoodDrawsThisFrame;
    if (s_entriesThisFrame > s_peakFrameEntries) {
        s_peakFrameEntries = s_entriesThisFrame;
    }
    // Report periodically AND whenever a frame runs far past the steady-state
    // 8 entries. The failing frame reached 45-50 before anything else fired, so
    // the anomaly is the frame BOUNDARY going missing, not the draw pass looping
    // - and a threshold catches that on the frame it happens.
    const bool anomalous = (s_entriesThisFrame > 16);
    if (startStageIsWw() && (anomalous || (s_dlstResets % 60) == 0)) {
        logf(anomalous ? LOG_LEVEL_WARN : LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"frame_entries\",\"reset\":%d,\"entries\":%d,"
             "\"lwood_draws\":%d,\"peak\":%d,"
             "\"reads\":\"BASELINE. entries = entryMatSort calls in the frame that just "
             "ended; lwood_draws = WwLwood_draw calls in it. On a steady WW stage both should "
             "be FLAT. The failing frame reported entries_this_frame 31-36 with six double "
             "entries - compare against this baseline: roughly DOUBLE means THE WHOLE DRAW "
             "PASS REPLAYED, a small rise means only lwood re-entered. Those are different "
             "bugs with different fixes and this number separates them\"}",
             s_dlstResets, s_entriesThisFrame, s_lwoodDrawsThisFrame, s_peakFrameEntries);
    }
    s_entrySeenN = 0;
    s_entriesThisFrame = 0;
    s_lwoodDrawsThisFrame = 0;
    return HOOK_CONTINUE;
}

HookAction on_entryMatSort(ModContext*, void* args, void*, void*) {
    s_entryCalls++;
    void* mp = mods::arg<void*>(args, 1);   // arg0 = this (J3DDrawBuffer)
    s_entrySubject = mp;
    if (startStageIsWw() && mp != nullptr) {
        s_entriesThisFrame++;
        for (int i = 0; i < s_entrySeenN; i++) {
            if (s_entrySeen[i] == mp) {
                s_entryDupes++;
                if (s_entryDupes <= 6) {
                    logf(LOG_LEVEL_WARN,
                         "[WwRegistry] {\"ev\":\"pkt_double_entry\",\"n\":%d,"
                         "\"matpkt\":\"%p\",\"entry_call\":%d,\"resets\":%d,\"entries_this_frame\":%d,\"lwood_draws\":%d,"
                         "\"reads\":\"THIS matPacket IS BEING ENTERED INTO THE DRAW BUFFER A "
                         "SECOND TIME WITHIN ONE FRAME. entryMatSort walks the bucket and merges on "
                         "isSame(), so the second entry FINDS ITSELF and runs "
                         "A->addShapePacket(A->getShapePacket()) - head prepended onto head, which "
                         "is the self-loop. If this fires alongside pkt_self_loop with the same "
                         "matpkt, the double entry is the CAUSE and the fix is to stop submitting "
                         "the model twice per frame\"}",
                         s_entryDupes, mp, s_entryCalls, s_dlstResets, s_entriesThisFrame,
                         s_lwoodDrawsThisFrame);
                }
                return HOOK_CONTINUE;
            }
        }
        if (s_entrySeenN < 32) {
            s_entrySeen[s_entrySeenN++] = mp;
        }
    }
    return HOOK_CONTINUE;
}

HookAction on_addShapePacket(ModContext*, void* args, void*, void*) {
    s_addShapeCalls++;
    J3DMatPacket* mp = static_cast<J3DMatPacket*>(mods::arg<void*>(args, 0));
    J3DShapePacket* sp = static_cast<J3DShapePacket*>(mods::arg<void*>(args, 1));
    if (mp == nullptr || sp == nullptr) {
        return HOOK_CONTINUE;
    }
    // THE SELF-LOOP CONDITION, read BEFORE the prepend happens.
    if (mp->getShapePacket() == sp) {
        s_selfLoops++;
        if (s_selfLoops <= 8) {
            logf(LOG_LEVEL_WARN,
                 "[WwRegistry] {\"ev\":\"pkt_self_loop\",\"n\":%d,\"call\":%d,"
                 "\"matpkt\":\"%p\",\"shapepkt\":\"%p\",\"ww_stage\":%d,\"self_merge\":%d,\"resets\":%d,\"entries_this_frame\":%d,"
                 "\"reads\":\"addShapePacket IS ABOUT TO PREPEND A PACKET ONTO ITSELF - "
                 "pShape already IS the chain head, so this writes pShape->next = pShape and "
                 "every later traversal of that chain runs FOREVER. This is the cycle "
                 "lwood_pkt_census sees from draw 42, caught at the instruction that creates "
                 "it. ww_stage says whether it is OUR stage; a self-loop on a TP stage would "
                 "mean this is a receiver-wide behaviour and not the plugin's doing\"}",
                 s_selfLoops, s_addShapeCalls, (void*)mp, (void*)sp,
                 startStageIsWw() ? 1 : 0, (mp == s_entrySubject) ? 1 : 0,
                 s_dlstResets, s_entriesThisFrame);
        }
    }
    return HOOK_CONTINUE;
}

HookAction on_gxCallDL(ModContext*, void* args, void*, void*) {
    s_gxCallDlCalls++;
    s_gxCallDlBytes += mods::arg<unsigned int>(args, 1);
    return HOOK_CONTINUE;
}

HookAction on_gxBeginDL(ModContext*, void*, void*, void*) {
    s_gxBeginDl++;
    return HOOK_CONTINUE;
}

HookAction on_gxEndDL(ModContext*, void*, void*, void*) {
    s_gxEndDl++;
    return HOOK_CONTINUE;
}

// Called once per lwood draw pass so the census has a frame boundary to report
// against; lwood is the actor under investigation, so its cadence is the right
// clock for this measurement.
void wwGxCensusTick() {
    s_gxFrames++;
    s_lwoodDrawsThisFrame++;
    if (s_gxFrames <= 6 || (s_gxFrames % 120) == 0) {
        logf(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"gx_census\",\"tick\":%d,\"callDL\":%d,"
             "\"callDL_bytes\":%llu,\"beginDL\":%d,\"endDL\":%d,\"open\":%d,"
             "\"reads\":\"CUMULATIVE GX traffic. `open` = beginDL minus endDL. "
             "aurora's publish() returns early while sInDisplayList is true, and "
             "begin_display_list sets that latch, so AN OPEN COUNT THAT GROWS PER TICK "
             "MEANS THE FIFO CAN NEVER DRAIN - which is the runaway. open staying 0 "
             "FALSIFIES the unbalanced-display-list mechanism outright and callDL_bytes "
             "then says whether the volume is real traffic instead\"}",
             s_gxFrames, s_gxCallDlCalls, s_gxCallDlBytes, s_gxBeginDl, s_gxEndDl,
             s_gxBeginDl - s_gxEndDl);
    }
}

HookAction on_modelEntryDL(ModContext*, void* args, void*, void*) {
    if (!startStageIsWw()) { return HOOK_CONTINUE; }
    void* model = mods::arg<void*>(args, 0);
    if (model != nullptr) {
        J3DModel* j3d = static_cast<J3DModel*>(model);
        J3DModelData* data = j3d->getModelData();
        if (wwIsWorldized(data)) {
            const u16 n = data->getShapeNum();
            for (u16 i = 0; i < n; i++) {
                J3DShape* shape = data->getShapeNodePointer(i);
                if (shape != nullptr) {
                    shape->show();
                }
            }
        }
    }
    // ============================================================
    // H9 - THE CONTROL LEG OF THE MODE-15 SET
    // ============================================================
    // The room model survives EVERY frame on this backend. Measuring it with
    // the SAME fields as lwood is what makes the other nine hypotheses
    // decidable: any field where the survivor and the victim AGREE cannot be
    // the cause, however plausible it looks in isolation. Gated on the same
    // env as the probe so a normal run pays nothing.
    // ============================================================
    {
        static int s_h9env = -1;
        if (s_h9env < 0) {
            const char* he = std::getenv("WW_LWOOD_DRAW");
            s_h9env = (he != nullptr && he[0] == '1' && he[1] == '5' && he[2] == '\0') ? 1 : 0;
        }
        if (s_h9env == 1 && model != nullptr) {
            // ------------------------------------------------------------
            // PER-MODEL SAMPLING, NOT FIRST-N-CALLS. The first version logged
            // `s_h9n <= 6`, and there are more than six room models - so the
            // control observed SIX MODELS EXACTLY ONCE EACH. A first
            // observation is precisely where the VICTIMS also read a NULL
            // `next`, so `0/6 survivors have a stale next` was what that
            // control MUST produce whether or not survivors ever develop one.
            // The survivor side of H6 was structurally uninformative and the
            // discriminator rested on the victim side alone.
            // Breadth-over-depth is the defect: a control that never sees a
            // SECOND frame cannot contradict an IN-FLIGHT transition.
            // ------------------------------------------------------------
            static void* s_h9Models[16] = {nullptr};
            static int s_h9Counts[16] = {0};
            static int s_h9Known = 0;
            int slot = -1;
            for (int k = 0; k < s_h9Known; k++) {
                if (s_h9Models[k] == model) { slot = k; break; }
            }
            if (slot < 0 && s_h9Known < 16) {
                slot = s_h9Known++;
                s_h9Models[slot] = model;
                s_h9Counts[slot] = 0;
            }
            const int perModel = (slot >= 0) ? ++s_h9Counts[slot] : 0;
            static int s_h9n = 0;
            s_h9n++;
            if (perModel <= 6 || (perModel % 300) == 0) {
                J3DModel* rm = static_cast<J3DModel*>(model);
                J3DModelData* rd = rm->getModelData();
                if (rd != nullptr && rd->getMaterialNum() > 0) {
                    J3DMatPacket* rmp = rm->getMatPacket(0);
                    // h5 needs a reference on the CONTROL side too - the first
                    // surviving MatPacket seen - or the field can only ever be -1.
                    static J3DMatPacket* s_h9FirstMat = nullptr;
                    if (s_h9FirstMat == nullptr) { s_h9FirstMat = rmp; }
                    J3DShapePacket* rsp = (rmp != nullptr) ? rmp->getShapePacket() : nullptr;
                    J3DMaterial* rmat = (rmp != nullptr) ? rmp->getMaterial() : nullptr;
                    logf(LOG_LEVEL_INFO,
                         "[WwRegistry] {\"ev\":\"lwood_h15_room\",\"n\":%d,\"obs_of_this_model\":%d,"
                         "\"model\":\"%p\","
                         "\"h3_locked\":%d,\"h3_changed\":%d,\"h4_texno\":%u,"
                         "\"h5_same_as_first\":%d,\"h6_stale_next\":\"%p\","
                         "\"h7_opa_texedge\":%d,\"h8_has_anm\":%d,\"h10_entrynum\":%d,"
                         "\"reads\":\"H9 CONTROL - the model that SURVIVES, same fields as "
                         "lwood_h15; agreement on a field ELIMINATES that hypothesis. h5/h8 "
                         "ADDED 2026-08-21: the first control leg omitted them, so those two "
                         "hypotheses had no survivor side and were structurally un-eliminable "
                         "- a control narrower than the victim leg silently shrinks the test\"}",
                         s_h9n, perModel, (void*)rm,
                         (rmp != nullptr && rmp->isLocked()) ? 1 : 0,
                         (rmp != nullptr && rmp->isChanged()) ? 1 : 0,
                         (rmat != nullptr) ? (unsigned)rmat->getTexNo(0) : 0xFFFFu,
                         (s_h9FirstMat != nullptr && rmp != nullptr && rmp != s_h9FirstMat)
                             ? (s_h9FirstMat->isSame(rmp) ? 1 : 0) : -1,
                         (rsp != nullptr) ? (void*)rsp->getNextPacket() : nullptr,
                         (rmat != nullptr) ? (rmat->isDrawModeOpaTexEdge() ? 1 : 0) : -1,
                         (rm->getMtxBuffer() != nullptr && rm->getAnmMtx(0) != nullptr) ? 1 : 0,
                         J3DDrawBuffer::entryNum);
                }
            }
        }
    }

    s_drawEntries++;
    // The head of a live J3DModel is a heap vtable; `0x3244334A` there would
    // mean a RAW 'J3D2' buffer reached the draw list, which is a different
    // and much worse bug than not drawing at all.
    unsigned int head = 0;
    if (model != nullptr) { head = *static_cast<const unsigned int*>(model); }
    if (s_drawEntries <= 8 || (s_drawEntries % 600) == 0) {
        logf(model != nullptr ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"draw_entry\",\"n\":%d,\"model\":\"%p\","
            "\"head\":\"0x%08X\",\"raw_j3d2\":%d}",
            s_drawEntries, model, head, head == 0x3244334Au ? 1 : 0);
    }
    return HOOK_CONTINUE;
}

void on_j3dLoadBDL(ModContext*, void* args, void* retval, void*) {
    if (!startStageIsWw()) { return; }
    s_j3dBdl++;
    if (s_j3dBdl > 24) { return; }
    const void* in = mods::arg<const void*>(args, 0);
    const unsigned int flags = mods::arg<unsigned int>(args, 1);
    void* out = (retval != nullptr) ? *static_cast<void**>(retval) : nullptr;
    logf(out != nullptr ? LOG_LEVEL_INFO : LOG_LEVEL_WARN,
        "[WwRegistry] {\"ev\":\"j3d_bdl\",\"n\":%d,\"in\":\"%p\","
        "\"flags\":\"0x%08X\",\"out\":\"%p\",\"magic\":\"0x%08X\"}",
        s_j3dBdl, in, flags, out,
        in != nullptr ? *static_cast<const unsigned int*>(in) : 0u);
}

void on_j3dLoadBMD(ModContext*, void* args, void* retval, void*) {
    if (!startStageIsWw()) { return; }
    s_j3dBmd++;
    if (s_j3dBmd > 24) { return; }
    const void* in = mods::arg<const void*>(args, 0);
    const unsigned int flags = mods::arg<unsigned int>(args, 1);
    void* out = (retval != nullptr) ? *static_cast<void**>(retval) : nullptr;
    logf(out != nullptr ? LOG_LEVEL_INFO : LOG_LEVEL_WARN,
        "[WwRegistry] {\"ev\":\"j3d_bmd\",\"n\":%d,\"in\":\"%p\","
        "\"flags\":\"0x%08X\",\"out\":\"%p\",\"magic\":\"0x%08X\"}",
        s_j3dBmd, in, flags, out,
        in != nullptr ? *static_cast<const unsigned int*>(in) : 0u);
}

HookAction on_lockedMakeSharedDL(ModContext*, void* args, void*, void*) {
    if (s_wwFinishDepth <= 0) {
        return HOOK_CONTINUE;
    }
    void* self = mods::arg<void*>(args, 0);
    if (s_fnJ3DMatMakeSharedDL != nullptr && self != nullptr) {
        s_fnJ3DMatMakeSharedDL(self);
    }
    return HOOK_SKIP_ORIGINAL;
}

HookAction on_patchedMakeSharedDL(ModContext*, void* args, void*, void*) {
    if (s_wwFinishDepth <= 0) {
        return HOOK_CONTINUE;
    }
    void* self = mods::arg<void*>(args, 0);
    if (s_fnJ3DMatMakeSharedDL != nullptr && self != nullptr) {
        s_fnJ3DMatMakeSharedDL(self);
    }
    return HOOK_SKIP_ORIGINAL;
}

HookAction on_glbResource(ModContext*, void* args, void*, void*) {
    // ========================================================================
    // TP BASELINE ADDED — INTEGRATOR 2026-08-16. The WW-only gate was right for
    // suppressing TP's benign resource probing, and WRONG here: with no TP
    // sample there is nothing to compare the WW `arc` value against, so the
    // probe can say "this pointer is garbage" but not "this pointer CHANGED".
    //
    // WHY THAT IS THE WHOLE QUESTION NOW: `mMsgDtArchive[0]` is filled in
    // exactly ONE place — `d_s_logo.cpp:928`, the LOGO scene at boot — and
    // nothing re-sets it on a stage change. So either it was already bad
    // before the warp (a boot/mount problem) or the WW stage load freed the
    // memory under it (a lifetime problem). **Those want opposite fixes**, and
    // one TP-stage sample of the same call separates them.
    //
    // Cheap by construction: the first 6 calls only, then WW-gated as before.
    // TP asks for `zel_00.bmg` on every meter create, so 6 is plenty for a
    // baseline and cannot become the firehose the WW gate was added to stop.
    // ========================================================================
    const bool ww = startStageIsWw();
    if (!ww && s_glbCalls >= 6) {
        return HOOK_CONTINUE;
    }
    s_glbCalls++;
    // Indices 0/1/2 with NO `this` — the mangling says `SA` (static). Reading
    // 1/2/3 here would print the name as the tag and the arc as the name.
    const unsigned int tag = mods::arg<unsigned int>(args, 0);
    const char* name = mods::arg<const char*>(args, 1);
    void* arc = mods::arg<void*>(args, 2);
    // The tag is a fourcc. Printed both as hex and as characters so neither a
    // byte-order surprise nor an unprintable value costs a read.
    char fourcc[5] = {0};
    fourcc[0] = (char)((tag >> 24) & 0xFF);
    fourcc[1] = (char)((tag >> 16) & 0xFF);
    fourcc[2] = (char)((tag >> 8) & 0xFF);
    fourcc[3] = (char)(tag & 0xFF);
    for (int i = 0; i < 4; i++) {
        if (fourcc[i] < 32 || fourcc[i] > 126) {
            fourcc[i] = '.';
        }
    }
    logf(LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"glb_res\",\"n\":%d,\"ww\":%d,"
        "\"tag\":\"0x%08X\",\"fourcc\":\"%s\",\"name\":\"%s\","
        "\"arc\":\"%p\",\"slot0\":\"%p\",\"slot1\":\"%p\","
        "\"arc_vptr\":\"0x%016llX\",\"slot0_vptr\":\"0x%016llX\","
        "\"slot1_vptr\":\"0x%016llX\",\"start\":\"%s\"}",
        s_glbCalls, ww ? 1 : 0, tag, fourcc, name != nullptr ? name : "(null)",
        arc,
        s_fnGetMsgDtArc != nullptr ? s_fnGetMsgDtArc(0) : nullptr,
        s_fnGetMsgDtArc != nullptr ? s_fnGetMsgDtArc(1) : nullptr,
        peek64(arc),
        peek64(s_fnGetMsgDtArc != nullptr ? s_fnGetMsgDtArc(0) : nullptr),
        peek64(s_fnGetMsgDtArc != nullptr ? s_fnGetMsgDtArc(1) : nullptr),
        (s_fnGetStartStageName != nullptr && s_fnGetStartStageName() != nullptr)
            ? s_fnGetStartStageName() : "(unreadable)");
    return HOOK_CONTINUE;
}
int s_resCalls   = 0;
int s_resNull    = 0;  // all NULL lookups, both games
int s_resNullWw  = 0;  // NULL lookups while a WW stage is the start stage

// Forward-declared HERE rather than below, because `on_getRes` now gates on it
// and the definition sits after the destination table (Integrator, gate fix).

// Post-hook: `retval` holds the answer, which is the half that matters.
void on_getRes(ModContext*, void* args, void* retval, void*) {
    s_resCalls++;
    void* got = (retval != nullptr) ? *static_cast<void**>(retval) : nullptr;
    // ========================================================================
    // THE BG'S OWN LOOKUP, LOGGED ON SUCCESS — added 2026-08-16 after run
    // 231654 showed the crash is NOT an unparsed model (raw:0 on all 40 passes)
    // but a bad TEXTURE BLOCK inside a model that DID parse
    // (`m_Do_ext.cpp:628` = `texture_p->getNum()`, reached only after
    // `getTexture()` returned non-NULL).
    //
    // `daBg_c::createHeap` asks for `model.bmd` then falls back to `model.bdl`
    // (`d_a_bg.cpp:122-123`). This prints WHICH arc answered, WHICH name won,
    // and the first 8 bytes of what came back — so "parsed by the right arm",
    // "parsed by the WRONG arm", and "not a model at all" stop being
    // indistinguishable. Static (`SA`): arg0=arcName, arg1=resName, no `this`.
    // ========================================================================
    {
        const char* rn = mods::arg<const char*>(args, 1);
        // ====================================================================
        // DZB CONVERSION ARM — REVERTED AND RESTORED WITHIN THE HOUR, and the
        // whiplash is recorded because the reconciliation is the lesson:
        //   · I reverted on endian.h:77 (`BE<T>` swaps on every read, ergo
        //     "the path is designed for raw BE, conversion double-swaps").
        //   · History's row 965 read ConvDzb IN FULL: on PC it ALSO does the
        //     OFFSET->POINTER REBASE (`setBase` on all six tables).
        //   · offset_ptr.h:23 is the decider: an un-setBase'd OffsetPtr
        //     resolves relative to THE FIELD'S OWN ADDRESS — file offsets
        //     land a few bytes off INSIDE the buffer. Deterministic garbage,
        //     no crash, AABB junk, -INF. The rebase is REQUIRED; the BE
        //     wrappers coexist (vertices are plain-and-swapped-once by
        //     ConvDzb, index fields are BE-wrapped). Both header reads were
        //     true; each of us had half the function.
        // Run 174716 (arm active) still ended black — the arm alone may not
        // finish ALINK's compound gate; the ground_query receipt below makes
        // the next boot name whether ground now resolves.
        // ====================================================================
        if (rn != nullptr && got != nullptr && startStageIsWw() && !s_hostIsFork &&
            s_fnConvDzb != nullptr) {
            const char* dot = std::strrchr(rn, '.');
            if (dot != nullptr && dot[1] == 'd' && dot[2] == 'z' && dot[3] == 'b' &&
                dot[4] == '\0') {
                void* conv = s_fnConvDzb(got);
                static int shownDzb = 0;
                if (shownDzb < 6) {
                    shownDzb++;
                    logf(LOG_LEVEL_INFO,
                        "[WwRegistry] {\"ev\":\"dzb_conv\",\"res\":\"%s\",\"in\":\"%p\","
                        "\"out\":\"%p\",\"reads\":\"receiver's own ConvDzb (rebase + "
                        "one-time vertex swap); its 0x80000000 latch makes repeats no-ops\"}",
                        rn, got, conv);
                }
                if (conv != nullptr && retval != nullptr) {
                    *static_cast<void**>(retval) = conv;
                }
                wwDzbRepackAttributes(conv != nullptr ? conv : got, rn);
            }
        }
        // COLLISION VISIBILITY (rows 956/958): `room.dzb` requests were
        // invisible — this probe logged only `mode*` names, so the one lookup
        // that decides whether the receiver's own d_a_bg.cpp:212 registration
        // chain engages was never observed. `room.*` now logs the same line.
        if (rn != nullptr && ((rn[0] == 'm' && rn[1] == 'o' && rn[2] == 'd' && rn[3] == 'e') ||
                              (rn[0] == 'r' && rn[1] == 'o' && rn[2] == 'o' && rn[3] == 'm'))) {
            const char* an = mods::arg<const char*>(args, 0);
            unsigned int h0 = 0, h1 = 0;
            if (got != nullptr) {
                h0 = *static_cast<const unsigned int*>(got);
                h1 = *reinterpret_cast<const unsigned int*>(
                         static_cast<const unsigned char*>(got) + 4);
            }
            logf(LOG_LEVEL_INFO,
                "[WwRegistry] {\"ev\":\"bg_model_lookup\",\"arc\":\"%s\",\"res\":\"%s\","
                "\"got\":\"%p\",\"head\":\"0x%08X\",\"sub\":\"0x%08X\"}",
                an != nullptr ? an : "?", rn, got, h0, h1);
        }
    }
    // ========================================================================
    // THE SUPPLIED 'BMDL' ARM, MOVED TO CONSUME TIME — run 031720 proved this
    // is where it belongs.
    //
    // WHAT THE RUN SHOWED: `R44_00 / model.bdl` -> head `0x3244334A`, sub
    // `0x346C6462` = **"J3D2" "bdl4"**. `daBg_c::createHeap` receives a RAW,
    // UNPARSED donor BDL, casts it to `J3DModelData*`, and `getTexture()` walks
    // garbage. That is the crash, and it means the gated `'BMDL'` arm WAS the
    // right diagnosis after all.
    //
    // WHY NOT THE loadResource WALK I BUILT FIRST: it reported `raw:0` for this
    // very archive while this lookup returns raw bytes from it. Its slot
    // enumeration does not reach `model.bdl` (count 26 / nonNull 9 for an arc
    // that plainly holds more). Rather than repair an enumeration to chase a
    // consumer, hook the consumer — it is the only place the pointer is
    // guaranteed to arrive.
    //
    // AND THIS IS DN-3'S OWN SANCTIONED SHAPE, NOT AN EXCEPTION TO IT: *"parse
    // a BDL exactly ONCE, at CONSUME time, through the port's cached
    // resolver."* On a clean dusklight there is no second parser to collide
    // with — the fork's ExtNpcMount layer is fork code — so the double-parse
    // mechanism DN-3 forbids cannot occur.
    //
    // SINGLE PARSE IS ENFORCED TWO WAYS: the same raw pointer is returned by
    // every later lookup of the same resource, so results are cached by INPUT
    // ADDRESS; and a parsed result is a heap object whose head is a vtable, so
    // the `J3D2` test cannot re-fire on it. DN-3's own verification signature.
    // ========================================================================
    if (got != nullptr) {
        const unsigned int head = *static_cast<const unsigned int*>(got);
        // DEFER-TO-HOST (§836): on a fork-hosted image the host's own
        // dExtNpcMount layer parses at consume time — a second parser over the
        // same buffer is DN-3's double-parse exactly. Observation above stays.
        if (head == 0x3244334Au && s_hostIsFork) { noteDefer("consume_parse_getRes"); }
        if (head == 0x3244334Au && !s_hostIsFork) {     // 'J3D2' — still raw
            const unsigned int sub = *reinterpret_cast<const unsigned int*>(
                static_cast<const unsigned char*>(got) + 4);
            static void* s_rawKey[32] = {nullptr};
            static void* s_parsed[32] = {nullptr};
            static int s_cacheN = 0;
            void* out = nullptr;
            for (int i = 0; i < s_cacheN; i++) {
                if (s_rawKey[i] == got) { out = s_parsed[i]; break; }
            }
            if (out == nullptr) {
                if (sub == 0x346C6462u && s_fnJ3DLoadBDL != nullptr) {        // 'bdl4'
                    out = s_fnJ3DLoadBDL(got, wwBdlLoadFlags());
                    wwAfterBdlLoad(&out, mods::arg<const char*>(args, 1));
                } else if (sub == 0x33646D62u && s_fnJ3DLoadBMD != nullptr) { // 'bmd3'
                    out = s_fnJ3DLoadBMD(got, 0x29020030);
                    wwFinishModelData(&out, mods::arg<const char*>(args, 1));
                }
                wwWorldizeShapeBounds(out, mods::arg<const char*>(args, 0));
                if (out != nullptr && s_cacheN < 32) {
                    s_rawKey[s_cacheN] = got;
                    s_parsed[s_cacheN] = out;
                    s_cacheN++;
                }
                logf(out != nullptr ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
                    "[WwRegistry] {\"ev\":\"bmdl_consume_parse\",\"res\":\"%s\","
                    "\"sub\":\"0x%08X\",\"in\":\"%p\",\"out\":\"%p\"}",
                    mods::arg<const char*>(args, 1), sub, got, out);
            }
            if (out != nullptr && retval != nullptr) {
                *static_cast<void**>(retval) = out;   // hand back the PARSED model
            }
        }
        // ================================================================
        // THE ANIMATION ARM, AT THE SAME CHOKE AND FOR THE SAME REASON
        // ================================================================
        // 'J3D1' is the animation magic; the block above only ever tested
        // 'J3D2', so every BTK/BRK/BTP/BPK/BLK/BVA/BXA came back RAW and the
        // consumer cast file bytes to a parsed type. The receiver parses
        // these at arc-load exactly as it parses geometry
        // (d_resorce.cpp:506-510) - so this is the same defect class as the
        // model arm, one file family over.
        //
        // Deferred on a fork host for the identical DN-3 reason as above: the
        // host's own layer parses at consume time and a second parser over
        // one buffer is the double-parse this file already forbids.
        // ================================================================
        if (head == 0x3144334Au && s_hostIsFork) { noteDefer("consume_parse_anim"); }
        if (head == 0x3144334Au && !s_hostIsFork) {
            // INDEX -1 ON PURPOSE: this is the BY-NAME overload
            // (arg0 = arc, arg1 = resource NAME, no index exists here). I
            // first wrote `mods::arg<int>(args, 1)` and that reads the name
            // POINTER as an integer - a garbage key that would have made
            // every lookup a miss and re-parsed the same buffer forever,
            // which is the exact double-parse DN-3 forbids. By-name callers
            // key on (arc, -1, raw); by-index callers keep their real index.
            void* anm = wwParseAnimOnce(got, mods::arg<const char*>(args, 0), -1);
            if (anm != nullptr && anm != got && retval != nullptr) {
                *static_cast<void**>(retval) = anm;   // hand back the PARSED anim
            }
        }
        return;  // resolved — not our business
    }
    s_resNull++;
    // ========================================================================
    // WW STAGES ONLY — INTEGRATOR GATE FIX 2026-08-16, and it is the THIRD time
    // a rate limit has eaten the one event we were hunting.
    //
    // Run 100000: the 40-miss cap was fully spent by log line 809, every one of
    // them on `F_SP102`, and the warp is at line 2119. **Zero misses survived
    // to the WW stage.** TP probes constantly for optional resources
    // (`model1.brk`, `event_list.dat`, `demo00_*_blend.bmd`) and every probe is
    // a benign NULL, so TP's noise exhausted the budget before the signal
    // existed.
    //
    // Gating on the stage is better than a bigger cap: it makes the log SPARSE
    // AND MEANINGFUL rather than large. A miss on a TP stage is normal
    // behaviour; a miss on a WW stage is the hypothesis. Same gate every other
    // WW-specific behaviour in this file uses, so TP is untouched either way.
    // ========================================================================
    if (!startStageIsWw()) {
        return;
    }
    s_resNullWw++;
    // Indices 0/1, NOT 1/2 — `getRes` is a STATIC member, so there is no `this`
    // in the argument list (Integrator gate fix; see the DEFINE_HOOK_SYMBOL
    // banner). With the old off-by-one, arg(2) was the `dRes_info_c*` and it
    // was printed with `%s`.
    const char* arc = mods::arg<const char*>(args, 0);
    const char* res = mods::arg<const char*>(args, 1);
    // Every MISS is logged, capped only far enough out to survive a storm.
    // A miss is rare in a healthy run and is the thing we are hunting; the
    // silence around one is what cost seven boots on the arc-name bug.
    if (s_resNullWw <= 40) {
        logf(LOG_LEVEL_WARN,
            "[WwRegistry] {\"ev\":\"res_miss\",\"arc\":\"%s\",\"res\":\"%s\","
            "\"miss\":%d,\"of_calls\":%d,\"start\":\"%s\"}",
            arc != nullptr ? arc : "(null)", res != nullptr ? res : "(null)",
            s_resNullWw, s_resCalls,
            (s_fnGetStartStageName != nullptr && s_fnGetStartStageName() != nullptr)
                ? s_fnGetStartStageName() : "(unreadable)");
    }
}

int s_msgGroupSkipped = 0;

// The WW msg-group seam, three tiers (the banner history below records how it
// got here — "do nothing" alone left slot 1 dangling and the repoint violated
// the slot's ownership contract):
//   FORK    -> defer to host (§836); the host's WW message machinery owns it.
//   VANILLA -> slot 1 OWNS A REAL MOUNT of the donor's one archive
//              (preflighted; JKR refcounting balances create/teardown).
//   BRIDGE  -> the slot-0 repoint, WARN-labelled, only if the mount path
//              cannot resolve — see the slot-1 contract CALLS row.
HookAction on_readMsgGroupLocal(ModContext* mod_ctx, void* args, void*, void*) {
    if (!startStageIsWw()) {
        return HOOK_CONTINUE;  // TP stage — the receiver's own load, untouched
    }
    // DEFER-TO-HOST (§836 + CALLS row 931): the msg-arc seam was MISSING from
    // the original four defer gates. On a fork boot the host's own WW message
    // machinery manages slot 1; this handler repointing it too is the same
    // two-writers shape — and the fork's A_mori exit crash (run 161726,
    // -1-sentinel archive in dScnPly_Delete's slot-1 teardown) is exactly
    // where such a collision would surface.
    if (s_hostIsFork) { noteDefer("msg_group_seam"); return HOOK_CONTINUE; }
    // arg 1, not 0: this is a NON-STATIC member, so arg 0 is `this`.
    void** p_arcMount = mods::arg<void**>(args, 1);
    if (p_arcMount != nullptr) {
        *p_arcMount = nullptr;
    }
    // ====================================================================
    // PRIMARY — SLOT 1 OWNS A REAL MOUNT (the ownership-contract fix; CALLS:
    // History's slot-1 decode + the Integrator's flagged-not-fixed item).
    //
    // THE CONTRACT, from the receiver's own debug strings (f_ap_game.cpp:477
    // "message data (STAGE)"): slot 1 is the per-STAGE archive and the SCENE
    // OWNS IT — dScnPly_Delete does removeResourceAll + JKRUnmountArchive on
    // it. Handing it the GLOBAL archive (the repoint below) means a COMPLETED
    // teardown unmounts the whole game's message data; the fork crash was the
    // lucky outcome.
    //
    // WW has no per-stage archive — its one archive maps onto the receiver's
    // contract by giving EVERY WW SCENE ITS OWN MOUNT of it. JKR mounts are
    // REFCOUNTED (JKRArchivePub.cpp:21 — an already-mounted archive answers
    // with mMountCount++), so the scene's teardown decrements exactly what its
    // create incremented and no module-side mount is ever destroyed by a
    // stage change. The receiver's own machinery does the rest: phase_3 syncs
    // the command, phase_4 stores slot 1 and destroys the command — the
    // receiver cannot tell the stage is foreign, which is the test of a
    // boundary translation.
    //
    // PREFLIGHTED, because the failure mode of an unservable path is worse
    // than the fallback: a created-but-doomed command makes phase_4 store
    // getArchive() == NULL into slot 1, and VANILLA's teardown derefs slot 1
    // unguarded. DVDConvertPathToEntrynum >= 0 proves the receiver's DVD
    // stack (plugin serve included) can open the path BEFORE the scene is
    // committed to owning a mount of it.
    // ====================================================================
    if (!s_msgMountResolveTried) {
        s_msgMountResolveTried = true;
        void* a = nullptr;
        if (s_hook->resolve(mod_ctx,
                "?mount@JKRArchive@@SAPEAV1@PEAXPEAVJKRHeap@@W4EMountDirection@1@@Z",
                &a, nullptr) == MOD_OK) {
            s_fnJkrMountMem = reinterpret_cast<FnJkrMountMem>(a);
        }
    }
    if (s_fnJkrMountMem != nullptr && s_bmgArcBuf != nullptr) {
        // NULL heap = JKR's current-heap convention (the mount(entryNum) twin
        // passes heap through the same way); 1 = MOUNT_DIRECTION_HEAD, used
        // only for the allocation alignment sign. The scene's teardown will
        // removeResourceAll (no-op on MOUNT_MEM) + JKRUnmountArchive (refcount
        // to 0, receiver deletes ITS object, our buffer survives — break-flag
        // 0 in the factory). Slot 1 is set HERE; *p_arcMount stays NULL so
        // phase_3/phase_4 skip their command handling cleanly.
        void* arc = s_fnJkrMountMem(const_cast<unsigned char*>(s_bmgArcBuf), nullptr, 1);
        if (arc != nullptr && s_fnSetMsgDtArc != nullptr) {
            s_fnSetMsgDtArc(1, arc);
            s_msgGroupSkipped++;
            if (s_msgGroupSkipped == 1) {
                logf(LOG_LEVEL_INFO,
                    "[WwRegistry] {\"ev\":\"msggroup_owned_mount\",\"stage\":\"%s\","
                    "\"arc\":\"bmgres.arc (mem-mount, %u bytes, plugin-retained)\","
                    "\"why\":\"slot 1 is an OWNED per-stage slot (scene teardown "
                    "unmounts it); WW's one archive maps onto that contract as one "
                    "refcounted mem-mount per scene - create/teardown balance, "
                    "removeResourceAll is a MOUNT_MEM no-op, nothing global is "
                    "ever unmounted and our buffer is never freed\"}",
                    (s_fnGetStartStageName != nullptr && s_fnGetStartStageName() != nullptr)
                        ? s_fnGetStartStageName() : "(unreadable)",
                    s_bmgArcSize);
            }
            return HOOK_SKIP_ORIGINAL;
        }
    }
    // ====================================================================
    // FALLBACK, LABELLED BRIDGE — create/preflight unavailable. The repoint
    // below keeps the meter alive but VIOLATES the ownership contract (a
    // completed teardown would unmount the GLOBAL archive); it survives only
    // because the alternative on an unguarded vanilla teardown is an
    // immediate crash. Logged at WARN so it can never read as native.
    // ====================================================================
    // REPOINT SLOT 1 AT SLOT 0 — INTEGRATOR 2026-08-16, and this is the FIX,
    // proven rather than hoped.
    //
    // MEASURED (run 171959, vtable-pointer peek): slot 1's POINTER VALUE never
    // changes across the warp, but the VTABLE POINTER of the object it names
    // does — `0x7FF62A984440` on TP, `0x7FF62A983240` on `sea`. Both are valid
    // image addresses, so this is not corruption: **the archive was FREED
    // during the WW stage load and its memory reallocated as a DIFFERENT
    // CLASS.** Slot 0's vptr is identical at every sample — the control held.
    //
    // Skipping the mount alone was not enough and made it worse: with no mount,
    // slot 1 is GUARANTEED to keep naming freed memory. It must be given
    // something live.
    //
    // SLOT 0 IS THE RIGHT ANSWER, not NULL: it is the boot/global message
    // archive, it holds the `zel_*.bmg` family the meter asks for, it is proven
    // live on the same WW stage one call earlier (n=7 uses it and survives),
    // and it is never freed by a stage change. NULL would make
    // `getGlbResource` fall back to a global search — safe, but it would also
    // silently change WHICH archive answers, and a quiet behaviour change is
    // how a plausible success gets shipped.
    //
    // WW-STAGE ONLY. TP never reaches this branch, so its per-stage mount and
    // its slot 1 are untouched.
    // ====================================================================
    if (s_fnSetMsgDtArc != nullptr && s_fnGetMsgDtArc != nullptr) {
        void* live = s_fnGetMsgDtArc(0);
        s_fnSetMsgDtArc(1, live);
    }
    s_msgGroupSkipped++;
    if (s_msgGroupSkipped == 1) {
        logf(LOG_LEVEL_WARN,
            "[WwRegistry] {\"ev\":\"msggroup_repoint_bridge\",\"stage\":\"%s\","
            "\"why\":\"owned-mount path unavailable (create or DVD preflight "
            "unresolved) - slot 1 repointed at the GLOBAL archive as a bridge\","
            "\"warn\":\"OWNERSHIP-CONTRACT VIOLATION held at bay: a completed "
            "scene teardown would removeResourceAll+unmount the global message "
            "archive. Bridge, not native - see the slot-1 contract CALLS row\"}",
            (s_fnGetStartStageName != nullptr && s_fnGetStartStageName() != nullptr)
                ? s_fnGetStartStageName() : "(unreadable)");
    }
    return HOOK_SKIP_ORIGINAL;
}

int s_multWw = 0;   // MULT chunks handled the donor's way
int s_multTp = 0;   // MULT chunks left entirely to the receiver

// Is the receiver's CURRENT start stage one we serve? Gates every WW-specific
// behaviour in this file on the same question, so a TP stage is never touched.
// Reads the receiver's own getter rather than any cached value of ours.

// vt[74] — `setMulti`, annotated identically at d_stage.h:532 (pure virtual)
// and :975 (the concrete `mMulti = i_Multi`). TWO INDEPENDENT ANNOTATIONS
// AGREE, WHICH IS NOT THE SAME AS A MEASUREMENT: this is a DERIVED NUMBER in
// a shipping path, the same class as the profile `sizeof` that Foundry made me
// assert. It cannot be static_assert'd from a header-free plugin, and a wrong
// slot calls the wrong virtual — so it is flagged in the queue row as the one
// thing to verify on the image before this builds.
// ============================================================================
// CORRECTED 74 -> 72 BY THE INTEGRATOR AT THE GATE, 2026-08-16. Housing asked
// for exactly this check ("the one thing to verify on the image before this
// builds") and it was wrong.
//
// THE RULE, and it applies to EVERY vtable index this plugin will ever take:
// **the decomp's `/* vt[N] */` annotations are PPC/Itanium-numbered, where the
// ABI reserves two leading vtable entries. MSVC x64 reserves NONE — the first
// virtual sits at slot 0. So MSVC slot = annotation - 2.**
//
// MEASURED, not reasoned: in the USER'S OWN TREE (`dusklight-main`, which is
// NOT this repo) `d_stage.h:526` annotates `setMulti` as `vt[74]`, and counting
// declarations from `class dStage_dt_c` it is the **73rd** virtual -> 0-based
// slot **72**. The annotations start at `vt[2]` for the first virtual (`init`),
// which is the two-slot offset made visible.
//
// WHAT 74 WOULD HAVE CALLED, which is why this is not a cosmetic fix:
//     slot 72 = setMulti(dStage_Multi_c*)   <- wanted
//     slot 73 = getMulti(void) const
//     slot 74 = setOldMulti(void)           <- would have been called
// `setOldMulti` takes NO argument. The hook would have invoked it with the
// multi pointer, left `getMulti()` stale, mutated old-multi state, and then
// returned 1 as if the donor handler had succeeded. **A fix that appears to
// work and corrupts state, with the crash arriving later wearing a different
// face.** Two independent header annotations agreeing is not a measurement —
// they agreed with each other and both used the donor ABI's numbering.
// ============================================================================
const int kVtSetMulti = 72;

HookAction on_multInfoInit(ModContext*, void* args, void* retval, void*) {
    // Room -1 fix: MULT is decided DURING the load, so ask the early question.
    if (!stageBecomingWw()) {
        s_multTp++;
        return HOOK_CONTINUE;  // TP stage — the receiver's own handler, untouched
    }
    // DEFER-TO-HOST (§836): the fork declares WW stages in-tree; running the
    // donor's mult-init on top of the host's own is the doubled-provision path.
    if (s_hostIsFork) { noteDefer("mult_info_init"); return HOOK_CONTINUE; }
    void* stage = mods::arg<void*>(args, 0);
    void* data  = mods::arg<void*>(args, 1);
    if (stage == nullptr || data == nullptr) {
        s_multTp++;
        return HOOK_CONTINUE;  // nothing to do the donor's way; let them decide
    }

    // The donor's three lines, in order.
    void* multi = static_cast<void*>(static_cast<char*>(data) + 4);
    void** vtbl = *reinterpret_cast<void***>(stage);
    typedef void (*FnSetMulti)(void*, void*);
    const FnSetMulti setMulti = reinterpret_cast<FnSetMulti>(vtbl[kVtSetMulti]);
    if (setMulti == nullptr) {
        s_multTp++;
        return HOOK_CONTINUE;
    }
    setMulti(stage, multi);

    s_multWw++;
    if (s_multWw == 1) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"mult_donor\",\"vt_slot\":%d,\"setMulti\":\"%p\","
            "\"note\":\"donor handler: setMulti then return; TP's readMult walk "
            "skipped for WW stages only\"}",
            kVtSetMulti, (void*)setMulti);
    }
    *static_cast<int*>(retval) = 1;  // donor returns 1
    return HOOK_SKIP_ORIGINAL;
}

// PURE OBSERVER. Logs the arc name it was asked for and the start stage the
// path will be built from — the two halves of `/res/Stage/<start>/<arc>`.
HookAction on_setStageRes(ModContext*, void* args, void*, void*) {
    s_setStageResCalls++;
    const char* arc = mods::arg<const char*>(args, 1);
    // THROTTLE REMOVED — INTEGRATOR GATE FIX 2026-08-15. It was `calls <= 4 ||
    // calls % 50 == 0`, tuned against a firehose that does not exist: run
    // 032634 made FIVE calls in an entire session. Four of them logged and
    // **the fifth — the only one after the warp, the only one anybody wanted —
    // fell in the single gap between the two clauses.** A rate limit that hides
    // exactly one event will hide the interesting one, because the interesting
    // event is by definition the rare one. Five lines a session is not a
    // firehose; log every call.
    if (true) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"set_stage_res\",\"calls\":%d,\"arc\":\"%s\","
            "\"start\":\"%s\",\"note\":\"path is /res/Stage/<start>/<arc>\"}",
            s_setStageResCalls, arc != nullptr ? arc : "(null)",
            (s_fnGetStartStageName != nullptr && s_fnGetStartStageName() != nullptr)
                ? s_fnGetStartStageName() : "(unreadable)");
    }
    return HOOK_CONTINUE;
}

// PRE observer + Pale bind. Does not change whether the loader runs.
// Vanilla never looks for the donor "Pale" chunk (it wants PAL0). Binding
// here is consume-boundary translation of the DZS we already relocated.
HookAction on_stageLoader(ModContext*, void* args, void*, void*) {
    s_loaderCalls++;
    // ====================================================================
    // THE SEAM THE PER-ARC EVICTION CANNOT COVER.
    // ====================================================================
    // Per-arc eviction handles "this actor released its arc". It CANNOT
    // handle the same arc being unmounted and remounted at the same
    // address, where every key field still matches but the parsed object
    // died with the old buffer. A stage load is the coarse boundary where
    // that is possible, so the whole table goes. Re-parsing a handful of
    // models on a stage change costs nothing; a stale J3DModelData* costs
    // a corruption two stages later that looks like anything but this.
    // ====================================================================
    wwInvalidateParsedModelsAll("stage load");
    wwKankyo_reset();
    dKyWw_wind_reset();
    wwVrbox_resetSpawn();
    wwSky_reset();
    wwWave_reset();
    wwShore_reset();
    void* dzs = mods::arg<void*>(args, 0);
    if (stageBecomingWw() && dzs != nullptr) {
        const int n = wwKankyo_bindDzs(dzs);
        const int fili = dKyWw_bindFili(dzs);
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"pale_bind\",\"n\":%d,\"virt\":%d,\"envr\":%d,\"colo\":%d,"
            "\"fili\":%d,\"start\":\"%s\"}",
            n, wwKankyo_virtN(), wwKankyo_envrN(), wwKankyo_coloN(), fili,
            (s_fnGetStartStageName != nullptr && s_fnGetStartStageName() != nullptr)
                ? s_fnGetStartStageName() : "(unreadable)");
    }
    if (s_loaderCalls == 1 || (s_loaderCalls % 50) == 0) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"stage_loader\",\"calls\":%d,\"start\":\"%s\"}",
            s_loaderCalls,
            (s_fnGetStartStageName != nullptr && s_fnGetStartStageName() != nullptr)
                ? s_fnGetStartStageName() : "(unreadable)");
    }
    return HOOK_CONTINUE;
}

HookAction on_roomLoader(ModContext*, void* args, void*, void*) {
    if (!stageBecomingWw()) {
        return HOOK_CONTINUE;
    }
    void* dzr = mods::arg<void*>(args, 0);
    const int fili = dKyWw_bindFili(dzr);
    if (fili >= 0) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"fili_bind\",\"level\":%d,\"room\":%d,\"sea\":%.1f}",
            fili, mods::arg<int>(args, 2), dKyWw_filiSeaLevel());
    }
    return HOOK_CONTINUE;
}

void on_bgBtkEntry(ModContext*, void* args, void*, void*) {
    if (!startStageIsWw()) {
        return;
    }
    wwShore_onBtkEntry(mods::arg<void*>(args, 0), mods::arg<void*>(args, 1));
}

HookAction on_bgBtkPlay(ModContext*, void* args, void*, void*) {
    if (!startStageIsWw()) {
        return HOOK_CONTINUE;
    }
    if (wwShore_tryWavePlay(mods::arg<void*>(args, 0))) {
        return HOOK_SKIP_ORIGINAL;
    }
    return HOOK_CONTINUE;
}

HookAction on_kySetLight(ModContext*, void*, void*, void*) {
    if (!startStageIsWw()) {
        return HOOK_CONTINUE;
    }
    // Painter can fire more than once per sim tick (frame interp). Donor
    // wether_move is once per tick — double move = 2× wind + bounce pulse.
    static u32 s_lastCounter = 0xFFFFFFFFu;
    const u32 tick = g_Counter.mCounter0;
    const bool simTick = (tick != s_lastCounter);
    if (simTick) {
        s_lastCounter = tick;
        dKyWw_wind_set();
        wwSky_move();
        // Whitecaps — donor wether_move_wave shape; Always gate inside wwWave.
        wwWave_move();
        // Once per Counter0 only. setLight can fire multiple times per tick;
        // re-entryImm of the same wave packet → circular J3D list (114950).
        wwWave_drawQueue();
        // Shore SC_01 BTK clock (donor d_envse type-1 field_0xf8 0..99).
        wwShore_tick();
    }
    wwKankyo_virt_set();
    static int s_windTicks = 0;
    if (simTick) {
        s_windTicks++;
        if (s_windTicks == 1 || (s_windTicks % 300) == 0) {
            cXyz* v = dKyWw_get_wind_vec();
            logf(LOG_LEVEL_INFO,
                "[WwRegistry] {\"ev\":\"wind_tick\",\"n\":%d,\"pow\":%.3f,"
                "\"vec\":[%.3f,%.3f,%.3f],\"fili\":%d,\"stagFar\":%.0f}",
                s_windTicks, dKyWw_get_wind_pow(),
                v != nullptr ? v->x : 0.0f, v != nullptr ? v->y : 0.0f,
                v != nullptr ? v->z : 0.0f, dKyWw_filiLevel(), dKyWw_discStagFar());
        }
    }
    return HOOK_CONTINUE;
}

HookAction on_kyMaji(ModContext*, void* args, void*, void*) {
    if (!startStageIsWw()) {
        return HOOK_CONTINUE;
    }
    void* data = mods::arg<void*>(args, 1);
    void* tevstr = mods::arg<void*>(args, 2);
    int type = -1;
    if (!dKyWw_tryWwMaji(data, tevstr, &type)) {
        return HOOK_CONTINUE;
    }
    static int s_dabg = 0;
    s_dabg++;
    if (s_dabg == 1 || (s_dabg % 300) == 0) {
        logf(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"ww_light\",\"n\":%d,\"type\":%d,\"pale\":%d}",
             s_dabg, type, wwKankyo_paletN());
    }
    return HOOK_SKIP_ORIGINAL;
}

void on_kySettingTev(ModContext*, void* args, void*, void*) {
    if (!startStageIsWw()) {
        return;
    }
    const int type = mods::arg<int>(args, 1);
    dKyWw_overlayTevStruct(type, static_cast<dKy_tevstr_c*>(mods::arg<void*>(args, 3)));
}

void on_stageCreate(ModContext*, void*, void*, void*) {
    if (startStageIsWw()) {
        wwVrbox_trySpawn(kVrboxIndex, kVrbox2Index);
    }
}

HookAction on_camNextType(ModContext*, void* args, void* retval, void*) {
    dCamera_c* cam = static_cast<dCamera_c*>(mods::arg<void*>(args, 0));
    if (cam == nullptr || !wwCamSelect(cam)) {
        return HOOK_CONTINUE;
    }
    *static_cast<int*>(retval) = cam->mCurType;
    return HOOK_SKIP_ORIGINAL;
}

HookAction on_camNextMode(ModContext*, void* args, void* retval, void*) {
    dCamera_c* cam = static_cast<dCamera_c*>(mods::arg<void*>(args, 0));
    if (cam == nullptr || !wwCam_isAttached()) {
        return HOOK_CONTINUE;
    }
    *static_cast<int*>(retval) = cam->mCurMode;
    return HOOK_SKIP_ORIGINAL;
}

HookAction on_camOnStyleChange(ModContext*, void*, void* retval, void*) {
    if (!wwCam_isAttached() || wwCam_inSelectLatch()) {
        return HOOK_CONTINUE;
    }
    *static_cast<bool*>(retval) = false;
    return HOOK_SKIP_ORIGINAL;
}

HookAction on_camTest2(ModContext*, void* args, void* retval, void*) {
    dCamera_c* cam = static_cast<dCamera_c*>(mods::arg<void*>(args, 0));
    if (cam == nullptr || !wwCam_isAttached()) {
        return HOOK_CONTINUE;
    }
    const int style = mods::arg<int>(args, 1);
    *static_cast<bool*>(retval) = wwCam_runCrawl(cam, style);
    return HOOK_SKIP_ORIGINAL;
}


// ============================================================================
// fpcPf_Get — answer for OUR indices, fall through for everything else.
//
// NULL is the correct "not mine" signal on THIS function specifically:
// fpcBs_Create (f_pc_base.cpp:138) does `pprofile = fpcPf_Get(...); if
// (pprofile == NULL) { ... }` and treats it as a graceful creation failure.
// That is NOT true of dStage_searchName, whose consumer asserts non-NULL —
// the two hooks of the §747 chain have OPPOSITE null contracts, which is why
// this comment exists rather than a shared helper.
// ============================================================================
HookAction on_pfGet(ModContext*, void* args, void* retval, void*) {
    // The start-stage sampler rides HERE, before the range check, because this
    // hook fires on every process creation — including the OVERLAP0 and
    // PLAY_SCENE processes a stage change creates. That is the window a failed
    // warp actually has, and the one `dStage_searchName` does not reach.
    sampleStartStage();

    const short procName = mods::arg<short>(args, 0);
    if (!isWwIndex(procName)) {
        return HOOK_CONTINUE;  // receiver's index — untouched
    }
    const void* profile = lookup(procName);
    if (profile == nullptr) {
        // Ours by range but unregistered: let the receiver's own §706 OOB
        // guard answer NULL rather than inventing a second refusal path.
        return HOOK_CONTINUE;
    }
    s_pfHits++;
    *static_cast<const void**>(retval) = profile;
    return HOOK_SKIP_ORIGINAL;
}

// ============================================================================
// cDyl_IsLinked — return TRUE for our range, reproducing the array's own
// NULL branch. MUST skip the original: HOOK_CONTINUE would run it and index
// DMC[] with an out-of-range value, which is the wild read this exists to
// prevent.
// ============================================================================
HookAction on_dylIsLinked(ModContext*, void* args, void* retval, void*) {
    const short procName = mods::arg<short>(args, 0);
    if (!isWwIndex(procName)) {
        return HOOK_CONTINUE;
    }
    s_dylHits++;
    *static_cast<int*>(retval) = 1;  // TRUE — nothing to link; code is resident
    return HOOK_SKIP_ORIGINAL;
}

// ============================================================================
// dStage_searchName — answer for OUR object names, FALL THROUGH for everything
// else. The null contract here is the OPPOSITE of fpcPf_Get's and getting it
// backwards is not survivable (§970):
//
//   fpcPf_Get      NULL is a valid "not mine" — fpcBs_Create (f_pc_base.cpp:138)
//                  treats it as a graceful creation failure.
//   dStage_searchName  NULL is NOT — d_event_data.cpp:977 does
//                  `JUT_ASSERT(1073, objectName)` immediately after the call.
//
// So this hook NEVER returns NULL. A miss is HOOK_CONTINUE, which lets the
// receiver's own search run and produce its own NULL + OS_REPORT exactly as
// today. We only ever ADD answers; we never suppress the receiver's.
// ============================================================================

HookAction on_stageSearchName(ModContext*, void* args, void* retval, void*) {
    // TOTAL calls, ours and the receiver's alike — the STAGE-LOADED leg of the
    // §1014 receipt. This function is driven by stage/room data parsing, so a
    // count that climbs after a warp means the receiver got far enough to walk
    // the new stage's object tables; a count that stays flat means it never
    // started. `name_hits` below counts only OUR answers and cannot tell those
    // apart, which is why both ship.
    s_searchCallsTotal++;
    // sampleStartStage() USED TO RIDE HERE AND IT WAS THE WRONG HOOK (§1016).
    // `dStage_searchName` fires only during STAGE/ROOM PARSING — which is
    // precisely the thing that never happens on a failed warp. The sampler
    // therefore went quiet at exactly the moment it was needed, and two runs
    // reported `search_calls_total: 87` identically because every one of those
    // calls came from the boot loads. Moved to `fpcPf_Get`, which fires on
    // process creation and keeps ticking through a scene change.
    const char* objName = mods::arg<const char*>(args, 0);
    if (objName == NULL) {
        return HOOK_CONTINUE;
    }
    // DEFER-TO-HOST (§836): the fork's in-tree l_objectName carries the WW
    // rows; answering here too would give two authorities for one name.
    if (s_hostIsFork) { noteDefer("object_name_hook"); return HOOK_CONTINUE; }
    for (int i = 0; i < kObjectNameCount; i++) {
        // strncmp bounded at 8: the receiver's field is char[8] and an 8-char
        // name carries no terminator, so an unbounded compare would read past it.
        if (std::strncmp(kObjectNames[i].name, objName, sizeof(kObjectNames[i].name)) == 0) {
            // Vrbox/Vrbox2 exist in the receiver's table. Only steal them on
            // a WW stage; otherwise Ordon's sky would become ours.
            if ((kObjectNames[i].procname == kVrboxIndex ||
                 kObjectNames[i].procname == kVrbox2Index) &&
                !startStageIsWw() && !stageBecomingWw()) {
                return HOOK_CONTINUE;
            }
            s_nameHits++;
            *static_cast<const void**>(retval) = &kObjectNames[i];
            return HOOK_SKIP_ORIGINAL;
        }
    }
    return HOOK_CONTINUE;  // receiver's name — untouched, including its NULL path
}

// ============================================================================
// b2 — THE STAGE-CHANGE REQUEST. "Asking is cheap; surviving the answer is the
// problem" (Bridge, §997). This section does the ASKING. b3 is the surviving.
//
// WHY THE MANGLED 4-PARAM FORM AND NOT THE BARE NAME. `dComIfGp_setNextStage`
// is AMBIGUOUS by bare name — 2 raw entries under 1 name (§606), so a by-name
// resolve returns MOD_CONFLICT and binds nothing. The receiver declares two
// overloads (d_com_inf_game.h:1225 and :1227); we want the SHORT one, and
// `?dComIfGp_setNextStage@@YAXPEBDFCC@Z` is it — PEBD/F/C/C reads as
// (const char*, s16, s8, s8), matching :1227 exactly.
//
// WHY THE SHORT OVERLOAD IS THE RIGHT CALL AND NOT A SHORTCUT. It is not a
// reduced-fidelity variant: d_com_inf_game.cpp:1570 forwards to the 11-param
// form with the receiver's OWN defaults (0.0f, 0, 1, 0, 0, 1, 0). Every
// shipped actor warp in this tree uses the short form — d_a_b_zant.cpp:4481,
// d_a_e_mk.cpp:2346, and our own d_a_albw_shade_watcher.cpp:1903. Calling the
// long form with hand-written defaults would be authoring what the receiver
// already authors (DN-10 step 2 when step 1 already answers).
//
// FOUNDRY'S DIVERGENCE VERDICT IS NOT A BLOCKER HERE AND THIS IS WHY. §623
// reports `dComIfGp_setNextStage` arity 8 (donor) vs 11 (receiver), no
// compatible pair. TRUE AND NOT APPLICABLE: that measures whether a
// DONOR-SHAPED call is safe. We are not making one. This is a RECEIVER call
// with a receiver signature, picked by the receiver's own mangling — which is
// precisely their instruction: "pick the mangled form by the RECEIVER
// signature you are actually calling, not by the donor's."
//
// AND THE ENTRY POINT WE ARE NOT USING: `dStage_dt_c_roomLoader` is DIVERGENT
// (donor 2 params, receiver 3) and stays OUT of b2 per §623/§626, despite
// reading clean under `--symbol`. Name-SAFE is not call-SAFE.
//
// NO SECOND CALL IS NEEDED, VERIFIED AT SOURCE RATHER THAN ASSUMED. The
// receiver's own warps are a single setNextStage and nothing else; d_s_play
// consumes it (`dComIfGp_isEnableNextStage`, d_s_play.cpp:613/636). The
// fopScnM_ChangeReq + ReRequest pairing at d_com_inf_game.cpp:1301 belongs to
// the OPENING-scene path, which is a different transition. Adding it here
// would be inventing a sequence the receiver does not use.
// ============================================================================
typedef void (*FnSetNextStage)(const char*, short, signed char, signed char);
FnSetNextStage s_fnSetNextStage = nullptr;


// THE TRIGGER, REVISED — and the comment that stood here is retired rather
// than deleted, because it was wrong in an instructive way.
//
// WHAT IT SAID: "adding a fourth hook purely to schedule a probe would widen
// the hook surface for no measurement", and it justified counting `fpcPf_Get`
// as "a proxy for the game building a populated scene" with a threshold of
// 20,000.
//
// WHY IT WAS WRONG, MEASURED ACROSS TWO USER BOOTS (Integrator §1007): that
// counter tracks ACTOR CREATIONS, not time or frames — 46 calls / 79 creations
// on a title screen and 121 / 224 in loaded Ordon, a stable ~0.55 ratio, with
// frames rising 4x while the counter followed creations at 2.6x. So 20,000 is
// roughly 250 stage loads, and — the part that makes it unfixable by waiting —
// IDLING CANNOT ADVANCE IT, because creation stops once a stage settles. It
// was a threshold picked against a unit nobody had measured.
//
// AND THE HOOK-SURFACE ARGUMENT WAS RIGHT WHEN NEITHER HOOK WAS PRE-FLIGHTED
// AND IS WRONG NOW THAT BOTH ARE. `fopScnM_ChangeReq` and `fpcBs_Create` each
// clear BOTH verdicts on BOTH images — SAFE (one entry) and `sig_diff` SAME
// (4 and 3 params identical) — verified by Foundry and re-derived independently
// by the Integrator at the gate. The widening 3 -> 5 buys a MEASURED unit and a
// per-load reset, which is the entire defect. Approved at gate, row 167.
//
// THE SHAPE: mark the stage-load boundary at `fopScnM_ChangeReq`, then count
// `fpcBs_Create` FROM that mark and fire at 145 — the creations one settled
// in-game stage load produces. A from-boundary count fires once per load.
//
// NO FALLBACK, DELIBERATELY AND RATIFIED AT THE GATE. An absolute-count
// backstop would be one more number nobody measured. If the boundary never
// fires, b2 never fires and the receipt says which of the two it was — a
// legible non-event beats a blind one.
// ============================================================================
// THE WW WARP MENU — a working warp to Outset, reached from the menu bar.
//
// USER RULING 2026-08-15: *"Will I be able to warp to outset with the menu? If
// that isn't set up yet, set it up now. I don't want to spend time on simply
// seeing a tab show up."* This started as a render-site probe and is now the
// delivery, because a boot that only proves a tab exists is a wasted boot.
//
// THREE SDK CALLS, NO RECEIVER EDIT AND NO HOOK. `register_menu_tab` puts a
// top-level `WW Warp` tab on the MAIN MENU BAR (render site
// `src/dusk/ui/menu_bar.cpp:73-77`, inline with the host's own tabs, after
// `Achievements` and before `Reset`); selecting it `window_push`es a two-pane
// window; the panes take `pane_add_section`/`pane_add_control`. All three
// entry points measured present on VANILLA UPSTREAM identical to our fork, so
// this needs no rebuilt dusklight.
//
// WHY THIS IS NOT THE SAME-PAGE SHAPE, STATED SO IT IS NOT MISTAKEN FOR IT:
// building into dusklight's OWN warp page is blocked by an ownership check
// (`resolve()` -> `find_owned`, `src/dusk/mods/svc/ui.cpp:123`), which no mod
// can satisfy for a host pane. This is the same two-column SHAPE, one tab
// over. That distinction cost three withdrawn specs tonight.
//
// AND THE HOST'S OWN `Warp` TAB IS GATED BEHIND `enableAdvancedSettings`
// (`menu_bar.cpp:59-60`) while a mod menu tab is unconditional — so this is
// visible to every user even where dusklight's native Warp tab is not.
// ============================================================================
const char* result_name(ModResult r);  // defined below; used by the warp window

UiMenuTabHandle s_wwMenuTab = 0;
UiWindowHandle  s_wwWarpWindow = 0;

// ---------------------------------------------------------------------------
// THE DESTINATION TABLE. Donor-native stage names ONLY — no invented display
// labels, and that is a decision rather than laziness.
//
// §1010's spec wanted friendly names like "Grandma's House", and flagged in
// the same breath that WHICH interior is which house is UNVERIFIED. A display
// name is exactly the thing that gets typed from memory and never re-checked,
// so this table ships the names the disc actually uses. They are also what the
// user needs in order to check the warp against the donor.
//
// `sea` room 44 is Outset's exterior — under zero-bake Outset is not its own
// stage but room 44 of the 50-room `sea`. The interiors are the served set,
// confirmed live in every boot log: 224 donor files across
// `sea, LinkRM, Ojhous, Ojhous2, Omasao, Onobuta, A_mori, Pjavdou, Cave09, LinkUG`.
// Nothing here is guessed: every stage in this table is one the disc serves.
//
// ROOM 0 FOR THE INTERIORS IS THE DONOR'S OWN CONVENTION for single-room
// interior stages, not an assumption about their contents.
// ---------------------------------------------------------------------------
struct WwDest {
    const char* label;   // what the SELECT shows (points into s_routeLabels)
    const char* stage;   // donor-native stage name, byte-true
    signed char room;
};

// ---------------------------------------------------------------------------
// THE ROUTE STORE — filled at mount time from the disc, not typed here.
//
// WHAT THE HARDCODED TABLE GOT WRONG, and it was live: `sea` was pinned to
// room 44, so the ~49 OTHER sea rooms the plugin already mounts were
// unreachable from the menu. The alias pass in main.cpp walks every
// `Room<NN>.arc` on the user's disc to mint `R<NN>_00.arc`; the same walk now
// reports each (stage, room) it served, so the menu offers exactly what was
// mounted — no more, and no less.
//
// STATIC STORAGE, DELIBERATELY. `UiControlDesc.options` is a
// `const char* const*` the host reads across frames; labels built into a
// caller's buffer would dangle. Same lifetime rule as the object-name table.
//
// CAPACITY IS BOUNDED AND SATURATION IS LOUD. The served set today is ~91
// mounts across 10 stages; 192 leaves room for a wider `wwStages` without
// silently dropping destinations — and if it ever fills, it SAYS SO. A
// truncated list that reports nothing is the "0 means not measured" failure
// wearing different clothes.
// ---------------------------------------------------------------------------
const int kMaxRoutes = 192;
WwDest s_routes[kMaxRoutes];
char   s_routeLabels[kMaxRoutes][40];
char   s_routeStages[kMaxRoutes][20];  // OWNED copy - see addRoute
// Human-readable destination name, empty until established. Islands come from
// the donor's OWN table (`messageID = 3295 + room`, verified: room 44 =
// "Outset Island"); interiors are authored per the user's ruling, against
// actor identities read from the donor's source (Ji1=Orca, Aj1=Sturgeon,
// Ob1=Rose, Ym1=Mesa&Abe, Ba1=Grandma, Yw1=Sue-Belle). The TOKEN is never
// discarded — the final label carries BOTH, because "which file is this"
// stays answerable and never-substitute applies to labels too.
char   s_routeNames[kMaxRoutes][32];
const char* s_routeOptions[kMaxRoutes];
int    s_routeCount = 0;
bool   s_routesSaturated = false;

// ============================================================================
// GROUPING METADATA — the sort keys, all DONOR-DERIVED (grouping pass, user
// assignment 2026-08-16). None of this is typed: `type` is the donor's own
// `dStageType_*` read from STAG +0x0C, `groupRoom` is the parent island the
// donor's own `SCLS` exit table names.
//
// WHY THIS EXISTS AT ALL: a flat 81-entry list cost the user a diagnostic run
// — they could not find Outset, warped elsewhere, and the probe never reached
// `R44_00`. Legibility here is a functional requirement, not polish.
//
// `groupRoom` semantics, and the sea/interior duality is the whole trick:
//   · a `sea` room is its OWN group head    -> groupRoom = its room number
//   · an interior is a CHILD of its island  -> groupRoom = the island's room
// so sorting by (groupRoom, isChild, ...) puts every interior directly under
// the island it belongs to, with the island itself first.
// -1 means UNKNOWN — those sort last as their own bucket rather than being
// silently folded into island 0.
// ============================================================================
// Presentation-pass group table (row 961, STATIC-API branch): groups are
// CONTIGUOUS after the finalize sort, so each is a (start, count) slice of
// s_routeOptions. Filled in finalize step 3; consumed by the tab builder.
int s_groupStart[64];
int s_groupCount[64];
int s_groupN = 0;
signed char s_routeGroup[kMaxRoutes];    // parent sea room, or own room for sea
bool        s_routeIsChild[kMaxRoutes];  // false = the island/group head
signed char s_routeType[kMaxRoutes];     // donor dStageType_*, -1 unknown

// Stage-keyed inputs, filled from the disc before finalize runs.
struct WwStageMeta {
    char stage[20];
    signed char parentRoom;   // island room from SCLS, -1 none
    signed char type;         // dStageType_*, -1 unknown
};
WwStageMeta s_stageMeta[96];
int s_stageMetaCount = 0;
bool s_stageMetaSaturated = false;

WwStageMeta* stageMetaFor(const char* stage, bool create) {
    for (int i = 0; i < s_stageMetaCount; i++) {
        if (std::strcmp(s_stageMeta[i].stage, stage) == 0) {
            return &s_stageMeta[i];
        }
    }
    if (!create) {
        return nullptr;
    }
    if (s_stageMetaCount >= (int)(sizeof(s_stageMeta) / sizeof(s_stageMeta[0]))) {
        s_stageMetaSaturated = true;   // never drop silently
        return nullptr;
    }
    WwStageMeta& m = s_stageMeta[s_stageMetaCount++];
    std::strncpy(m.stage, stage, sizeof(m.stage) - 1);
    m.stage[sizeof(m.stage) - 1] = 0;
    m.parentRoom = -1;
    m.type = -1;
    return &m;
}

// ============================================================================
// LABEL COMPOSITION — user ruling 2026-08-16: *"warp rows should present by
// their in-canon names first with the toggle revealing their file names
// alongside them. Just like dusklight does for TP rows."*
//
// So the canon name LEADS and the file token FOLLOWS, exactly matching
// dusklight's own model — `MapEntry { const char* mapName; const char* mapFile; }`
// under `RegionEntry` — rather than inventing a second convention beside it.
//
//   toggle OFF :  Outset Island
//   toggle ON  :  Outset Island  [sea 44]
//   no name yet:  sea  room 44            <- token ALONE, never a guess
//
// THE TOKEN IS NEVER DISCARDED, only demoted. A route whose canon name is not
// established shows its token and nothing else — that is the donor-native
// fallback, and it is why a missing name degrades to "unlabelled" instead of
// to "wrong". Room 0 is the live case: `3295 + 0` resolves to the UI sentence
// "You have no maps.", NOT an island, so the island block starts at room 1 and
// room 0 must never be auto-named from the formula.
// ============================================================================
bool s_showFileNames = false;

void composeRouteLabel(int i) {
    if (i < 0 || i >= kMaxRoutes) { return; }
    const bool named = s_routeNames[i][0] != 0;
    if (!named) {
        std::snprintf(s_routeLabels[i], sizeof(s_routeLabels[i]),
                      "%s  room %d", s_routeStages[i], s_routes[i].room);
    } else if (s_showFileNames) {
        std::snprintf(s_routeLabels[i], sizeof(s_routeLabels[i]),
                      "%s  [%s %d]", s_routeNames[i], s_routeStages[i],
                      s_routes[i].room);
    } else {
        std::snprintf(s_routeLabels[i], sizeof(s_routeLabels[i]),
                      "%s", s_routeNames[i]);
    }
}



// Selection state. Point and layer are NUMBER controls rather than SELECTs
// because WW's spawn points come from the disc's DZR/DZS tables and NOBODY HAS
// READ THEM YET — enumerating a plausible-looking range would be inventing
// data. A stepper lets the user try one and see, and says so in its help text.
bool startStageIsWw() {
    if (s_fnGetStartStageName == nullptr) {
        return false;
    }
    const char* now = s_fnGetStartStageName();
    if (now == nullptr) {
        return false;
    }
    for (int i = 0; i < s_routeCount; i++) {
        if (std::strcmp(now, s_routes[i].stage) == 0) {
            return true;
        }
    }
    return false;
}

// ============================================================================
// THE SAME QUESTION, ASKED EARLY ENOUGH — the Room -1 root cause (Integrator,
// 2026-08-17). `startStageIsWw()` reports the stage that IS current. Some
// decisions are made while the stage is still being REQUESTED, and for those
// it answers false at exactly the moment it matters.
//
// MEASURED, not theorised: the user's overlay showed **Room: -1** on a `sea`
// boot. `registry.cpp:232` records that the RECEIVER's `dStage_multInfoInit`
// adds `setRoomNo(-1)` on top of the donor's three-line handler (WW never
// walks the MULT chunk at load). We already port the donor handler — but the
// shutdown receipt read **`mult_donor:1, mult_receiver:2`**: the substitution
// fired ONCE and the receiver's ran TWICE, because MULT is processed DURING
// the load, BEFORE the start-stage name flips. So we handed WW data to the
// walker that sets room -1.
//
// CORROBORATION FROM AN UNRELATED SEAM, which is why this is a property of the
// predicate and not of MULT: the Phase-4a acch gate logged **1,264 CLR calls
// before its first SET**. Same late flip, different consumer. I read that as
// "menu frames" and moved on; it was this.
//
// DELIBERATELY A SECOND FUNCTION, NOT A CHANGE TO `startStageIsWw()`: that
// predicate is consumed all over this file, and several of those consumers
// WANT "is current", not "is becoming". Widening it would silently re-time
// every one of them. Add and label; do not substitute.
//
// `s_fnGetNextStageName` is already resolved alongside the start getter
// (§1016) — this reads the receiver's own answer, it does not invent one.
// ============================================================================
bool stageBecomingWw() {
    if (startStageIsWw()) {
        return true;
    }
    if (s_fnGetNextStageName == nullptr) {
        return false;
    }
    const char* nxt = s_fnGetNextStageName();
    if (nxt == nullptr) {
        return false;
    }
    for (int i = 0; i < s_routeCount; i++) {
        if (std::strcmp(nxt, s_routes[i].stage) == 0) {
            return true;
        }
    }
    return false;
}

int s_destIdx = 0;
int s_point   = 0;
int s_layer   = -1;  // -1 = Auto, mirroring TP's own layer control default

void getDest(ModContext*, void*, UiControlValue* v) { v->int_value = s_destIdx; }
// ============================================================================
// PER-DESTINATION SPAWN-POINT DEFAULT (the last pin of run 183144): point
// indices are MODES, not slots — the receiver's own dStage_playerInit matches
// `(u8)angle.z == point`, and Outset exterior's point 0 is the SEA-LEVEL
// boat-mode spawn (y=173.1, param 0x00), 400 units under the island's terrain
// (y=580). History's disc read of all 24 points names **point 23** as the
// stand-on-Outset spawn: (-205596, 580, 316562), door-mode param 0x50, y equal
// to the measured ground. Defaulting the warp there is applying a MEASURED
// record, not authoring one. Every other destination keeps 0 (unmeasured —
// interiors carry few points and a wrong index is the receiver's own assert).
// The stepper still overrides; this only sets the default on selection.
// ============================================================================
static int wwDefaultPointFor(int destIdx) {
    if (destIdx >= 0 && destIdx < s_routeCount &&
        std::strcmp(s_routeStages[destIdx], "sea") == 0 &&
        s_routes[destIdx].room == 44) {
        // POINT 11 — the stand-on-TERRAIN spawn, id read from History's RECORDS
        // table: sea/Room44 point=11 pos=(-205596, 580, 316562), y == the
        // measured ground. The journey recorded: 23 was the ARRAY SLOT (the
        // documented slot-vs-id trap, committed in the first default), and
        // point 1 (y=720, door mode) put Link over a HOLE - door spawns
        // stand on BUILDING collision and Outset's buildings are the 35
        // unported actors (run 191421: his ray missed at his exact spot
        // while terrain answered 580 nearby). Also learned from the FATAL:
        // "23" was the ARRAY SLOT of the y=580 record, not its id (the exact
        // slot-vs-id trap History documented, committed in this very
        // default), AND the receiver's matcher compares the FULL SIGNED
        // angle.z where the donor masks (u8) — its valid list shows 99 as
        // -157, 128 as -128, 206 as -50 (same low byte, angle high bits
        // intact). Any spawn with high bits set is unreachable by plain id
        // until that seam is masked (filed to Housing). Point 1 is valid in
        // BOTH readings: plain id 1 in the receiver's own list, donor y=720
        // door-mode (0xA0) standing spawn.
        return 11;
    }
    return 0;
}

void setDest(ModContext*, void*, const UiControlValue* v) {
    const int i = (int)v->int_value;
    if (i >= 0 && i < s_routeCount && i != s_destIdx) {
        s_destIdx = i;
        s_point = wwDefaultPointFor(i);
    }
}
void getPoint(ModContext*, void*, UiControlValue* v) { v->int_value = s_point; }
void setPoint(ModContext*, void*, const UiControlValue* v) { s_point = (int)v->int_value; }
void getLayer(ModContext*, void*, UiControlValue* v) { v->int_value = s_layer; }
void setLayer(ModContext*, void*, const UiControlValue* v) { s_layer = (int)v->int_value; }

// The warp is disabled rather than silently doing nothing when the transport
// did not bind — a greyed button with a reason beats a button that lies.
bool warpDisabled(ModContext*, void*) { return s_fnSetNextStage == nullptr; }

// ---------------------------------------------------------------------------
// WHAT THE RECEIVER THINKS ITS START STAGE IS. A read, not an override.
//
// The override that used to live here (armed name + release button) is gone
// with the hook: it existed to make the loader see `sea`, and the receiver
// already sets that itself at `d_s_play.cpp:1308`. Faking a value the host
// assigns for us was the wrong shape, and the 0-answer receipt is what proved
// it rather than an argument.
// ---------------------------------------------------------------------------
char s_lastSeenStage[16] = {0};
int  s_stageReads = 0;

// Called from the stage-search hook, which fires during stage/room parsing —
// so this samples the receiver's own start-stage name exactly while a load is
// happening. Rate-limited: the search hook is hot.
void sampleStartStage() {
    if (s_fnGetStartStageName == nullptr) {
        return;
    }
    const char* now = s_fnGetStartStageName();
    if (now == nullptr) {
        return;
    }
    if (std::strncmp(now, s_lastSeenStage, sizeof(s_lastSeenStage) - 1) != 0) {
        std::strncpy(s_lastSeenStage, now, sizeof(s_lastSeenStage) - 1);
        s_lastSeenStage[sizeof(s_lastSeenStage) - 1] = '\0';
        s_stageReads++;
        // CHANGE-ONLY, so this is one line per real stage transition rather
        // than one per parse call. If a WW name ever appears here, the
        // receiver adopted our destination on its own.
        //
        // NEXT is logged BESIDE start (Integrator, §1016) because the two
        // together are what discriminate:
        //   next=sea, start!=sea  -> setNextStage stuck; the d_s_play.cpp:1308
        //                            copy never ran. Fault is in the scene phase.
        //   next!=sea             -> setNextStage never stuck at all; the fault
        //                            is upstream of the scene entirely.
        //   next=sea, start=sea   -> the name is right and the black screen is
        //                            further down the load, not the name.
        // One boot answers all three instead of three boots answering one each.
        const char* nextName = (s_fnGetNextStageName != nullptr)
                                   ? s_fnGetNextStageName() : nullptr;
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"startstage_now\",\"name\":\"%s\","
            "\"next\":\"%s\",\"change\":%d}",
            s_lastSeenStage, nextName != nullptr ? nextName : "(unreadable)",
            s_stageReads);
    }
}

void onWwWarpPressed(ModContext*, void*) {
    if (s_fnSetNextStage == nullptr) {
        logf(LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"warp_refused\",\"why\":\"setNextStage "
            "unresolved on this image\"}");
        return;
    }
    if (s_destIdx < 0 || s_destIdx >= s_routeCount) {
        logf(LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"warp_refused\",\"why\":\"no routes; the disc "
            "served no stage arcs, so there is nothing to warp to\"}");
        return;
    }
    const WwDest& d = s_routes[s_destIdx];
    // Logged BEFORE the call: if the transition faults, this line is the only
    // record of what was asked for.
    logf(LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"warp_request\",\"stage\":\"%s\",\"room\":%d,"
        "\"point\":%d,\"layer\":%d,\"source\":\"WW Warp menu\"}",
        d.stage, (int)d.room, s_point, s_layer);
    // #27c: the ovlp reporter keys "post-warp" on THIS moment, not on
    // startStageIsWw() — the stage flag flips only at the new scene's
    // phase_1 copy, so transition-window events read ww:0 and the flag
    // cannot separate pre-warp from mid-transition (bit run 155713's read).
    s_warpMoment = true;
    s_fnSetNextStage(d.stage, (short)s_point, d.room, (signed char)s_layer);
    logf(LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"warp_returned\",\"note\":\"setNextStage "
        "returned; the transition is now the receiver's to run\"}");
}

// Row-961 grouped selects: each island section's select spans that group's
// contiguous slice; `user_data` carries the slice start so one get/set pair
// serves every section. A select whose group does not hold the live selection
// shows its own head (option 0) rather than a lie about the global state.
void getDestGrouped(ModContext*, void* user_data, UiControlValue* out) {
    const int start = (int)(intptr_t)user_data;
    int idx = s_destIdx - start;
    // count lives in the desc the host read at build time; clamp via table
    int count = 0;
    for (int g = 0; g < s_groupN; g++) {
        if (s_groupStart[g] == start) { count = s_groupCount[g]; break; }
    }
    if (idx < 0 || idx >= count) { idx = 0; }
    out->int_value = idx;
}
static int wwDefaultPointFor(int destIdx);   // defined below with its rationale

void setDestGrouped(ModContext*, void* user_data, const UiControlValue* v) {
    const int start = (int)(intptr_t)user_data;
    s_destIdx = start + (int)v->int_value;
    // THE POINT-0 BLACK SCREEN'S LAST LINK (user screenshot, 2026-08-17):
    // Housing's measured default (Outset -> PLYR point 23, y == ground) hooks
    // setDest — the FLAT handler — and this grouped handler bypassed it, so
    // every sectioned-menu warp still sailed with point 0, four hundred units
    // under the island. Same default, both doors.
    s_point = wwDefaultPointFor(s_destIdx);
}

ModResult buildWwWarpTab(ModContext* ctx, UiWindowHandle, UiElementHandle left,
                         UiElementHandle right, void*, ModError*) {
    // ========================================================================
    // USER RULING (2026-08-16, second warp test): the row-961 sectioned build
    // is REVERTED to the previous single flat select — reorganized by
    // EVENT/STORY ORDER (islands in story sequence, each island's interiors
    // immediately after it; see the AUTHORED rank table in finalizeRoutes).
    // The native-WarpWindow join remains the endgame, deferred by the same
    // ruling to "the first patch". The grouped accessors above stay for the
    // join's provider to reuse; nothing here consumes them today.
    // ========================================================================
    s_ui->pane_add_section(ctx, left, "Wind Waker — Great Sea");

    UiControlDesc dest = UI_CONTROL_DESC_INIT;
    dest.kind = UI_CONTROL_SELECT;
    dest.label = "Destination";
    dest.binding = UI_BINDING_CALLBACKS;
    dest.get = getDest;
    dest.set = setDest;
    dest.options = s_routeOptions;
    dest.option_count = (size_t)s_routeCount;
    dest.help_rml =
        "<p>Donor-native stage names, served byte-identical from your own "
        "Wind Waker disc. Islands run in story order — Outset first, its "
        "interiors beneath it — then the rest of the sea ascending.</p>";
    s_ui->pane_add_control(ctx, left, &dest, nullptr);

    s_ui->pane_add_section(ctx, left, "Warp");

    UiControlDesc point = UI_CONTROL_DESC_INIT;
    point.kind = UI_CONTROL_NUMBER;
    point.label = "Spawn point";
    point.binding = UI_BINDING_CALLBACKS;
    point.get = getPoint;
    point.set = setPoint;
    point.min = 0; point.max = 63; point.step = 1;
    point.help_rml =
        "<p>Point indices are the donor's own spawn MODES (matched on "
        "<code>angle.z</code>, not array position). Outset's exterior defaults "
        "to <em>point 23</em> — the measured stand-on-land spawn; point 0 there "
        "is the sea-level boat spawn, 400 units under the island. Other "
        "destinations default to 0 until their tables are read.</p>";
    s_ui->pane_add_control(ctx, left, &point, nullptr);

    UiControlDesc layer = UI_CONTROL_DESC_INIT;
    layer.kind = UI_CONTROL_NUMBER;
    layer.label = "Room layer override";
    layer.binding = UI_BINDING_CALLBACKS;
    layer.get = getLayer;
    layer.set = setLayer;
    layer.min = -1; layer.max = 15; layer.step = 1;
    layer.prefix = "layer ";
    layer.help_rml =
        "<p><code>-1</code> is Auto, mirroring Twilight Princess's own layer "
        "control. The layer argument is carried by the same "
        "<code>setNextStage</code> call the host uses for its own warps.</p>";
    s_ui->pane_add_control(ctx, left, &layer, nullptr);

    UiControlDesc go = UI_CONTROL_DESC_INIT;
    go.kind = UI_CONTROL_BUTTON;
    go.label = "Warp";
    go.on_pressed = onWwWarpPressed;
    go.is_disabled = warpDisabled;
    go.help_rml =
        "<p>Calls the host's own <code>dComIfGp_setNextStage</code>. Disabled "
        "if that symbol did not resolve on this image.</p>";
    s_ui->pane_add_control(ctx, left, &go, nullptr);

    s_ui->pane_add_section(ctx, right, "Wind Waker layer");
    s_ui->pane_add_text(ctx, right,
        "Stage data is served straight from your Wind Waker (USA) disc image. "
        "Nothing is staged and nothing is shipped.", nullptr);
    return MOD_OK;
}

void onWwWarpTabSelected(ModContext* ctx, void*) {
    UiTabDesc tab = UI_TAB_DESC_INIT;
    tab.title = "Outset";
    tab.build = buildWwWarpTab;

    UiWindowDesc win = UI_WINDOW_DESC_INIT;
    win.tabs = &tab;
    win.tab_count = 1;

    const ModResult r = s_ui->window_push(ctx, &win, &s_wwWarpWindow);
    logf(r == MOD_OK ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"warp_window\",\"push\":\"%s\"}",
        result_name(r));
}

// ============================================================================
// D1 EMISSION — the whole-manifest hook load gate (original 19c).
//
// RESTORED BY THE INTEGRATOR 2026-08-16, and the incident belongs in the file
// because it is the cheapest way to stop it recurring: this function EXISTED,
// I added a SECOND worse copy without reading the file — the editor warned me
// it "contains other changes not in your context" and I proceeded anyway —
// then removed my duplicate with a read-modify-write that raced a concurrent
// edit and took the original with it. The other side then defensively removed
// the CALL to keep the tree green and left the wiring point marked. Two
// failures, one cause: writing to a file I had not opened.
//
// WHY IT EXISTS, with tonight's receipt: `setStageRes` was bound BARE, is
// `MULTI_NAME` on the user's image, and the hook was NEVER INSTALLED. The probe
// then logged `set_stage_res_calls: 0`, and that zero was read as a
// MEASUREMENT. A dead probe reporting 0 is worse than no probe.
//
// NO NEW REGISTRATION MECHANISM WAS NEEDED — the design question D1 flagged:
// `DEFINE_HOOK_SYMBOL` already emits a `HookNameRecord` into `modmeta$d`, the
// host resolves it at load, and `DEFINE_MOD()` publishes the `modmeta$a`/`$z`
// bounds. Only the emission was missing. Walking begin->end in `rec.size`
// steps visits every record in declaration order; `size` is documented as a
// multiple of 8, and a zero or unaligned size would spin, so the walk refuses.
//
// AND THE ONE THING THIS MUST NOT BECOME: a green line that means nothing. A
// record whose `resolved` is NULL prints NOT-INSTALLED **with the name**, and
// the summary counts both sides, so a zero is legible as a measurement rather
// than as silence.
// ============================================================================
// ============================================================================
// ATTACH LEDGER — the second axis D1 never had (SPEC-boot-join, CALLS
// 2026-08-16). D1 reports whether a DECLARATION'S SYMBOL RESOLVED. It cannot
// report whether a CALLBACK WAS ATTACHED, because nothing recorded that. A
// hook declared with DEFINE_HOOK_SYMBOL and never passed to `hook_add_*`
// reports INSTALLED forever and can never fire — that is what `on_getRes` did
// across two boots while its INSTALLED line was read as proof it was alive.
//
// KEYED BY RECORD ADDRESS, NOT BY NAME, and that is the point: the macro
// (`sdk/include/mods/hook.hpp:152`) emits `mod_meta_hook_##alias` as the very
// record the walker will encounter, so `&mod_meta_hook_PfGet` IS the join key.
// A pointer compare cannot mis-join the way a mangled-name string compare can
// — and mangled names are exactly where MULTI_NAME ambiguity already bit us.
//
// A hook with NO note is NOT reported as unattached-and-fine: it is reported
// as NOT-ATTACHED with its name, because that is the failure this exists to
// catch. UNKNOWN is reserved for the case the ledger overflowed, which is
// reported separately rather than silently truncating (№31-C: a check that
// cannot run says UNKNOWN, never clean).
// ============================================================================
const int kMaxAttachNotes = 128;
struct AttachNote {
    const void* rec;
    ModResult   result;
};
AttachNote s_attachNotes[kMaxAttachNotes];
int  s_attachNoteCount = 0;
bool s_attachLedgerOverflowed = false;

void noteAttach(const void* rec, ModResult r) {
    if (s_attachNoteCount >= kMaxAttachNotes) {
        s_attachLedgerOverflowed = true;   // never silently drop
        return;
    }
    s_attachNotes[s_attachNoteCount].rec = rec;
    s_attachNotes[s_attachNoteCount].result = r;
    s_attachNoteCount++;
}

const AttachNote* findAttach(const void* rec) {
    for (int i = 0; i < s_attachNoteCount; ++i) {
        if (s_attachNotes[i].rec == rec) {
            return &s_attachNotes[i];
        }
    }
    return nullptr;
}

int emitHookManifest() {
    // BOUNDS COME FROM THE EXPORTED `mod_meta`, NOT `MOD_META_BOUNDS_BEGIN`.
    // Those macros expand to `&mod_meta_bounds_begin/_end`, which are DEFINED
    // BY `DEFINE_MOD()` — and that lives in `main.cpp`, so they are undeclared
    // identifiers in this TU (`error C2065`, hit on the first build of this
    // restore). `DEFINE_MOD()` publishes the same two addresses through the
    // exported `ModMeta`, which api.h declares `extern` for every TU. Same
    // pointers, reachable from here.
    const unsigned char* p =
        reinterpret_cast<const unsigned char*>(mod_meta.records_begin);
    const unsigned char* end =
        reinterpret_cast<const unsigned char*>(mod_meta.records_end);
    int installed = 0, missing = 0, records = 0;
    int attachMissing = 0;   // resolved-or-not, but no MOD_OK callback registered
    if (p == nullptr || end == nullptr || end < p) {
        logf(LOG_LEVEL_WARN,
            "[WwRegistry] {\"ev\":\"hook_manifest_summary\",\"records\":0,"
            "\"why\":\"modmeta bounds unavailable — enumeration did not run, "
            "which is NOT the same as zero hooks\"}");
        return -1;
    }

    while (p + sizeof(ModMetaRecord) <= end) {
        const ModMetaRecord* rec = reinterpret_cast<const ModMetaRecord*>(p);
        const unsigned size = rec->size;
        // THE SENTINELS ARE NOT RECORDS. `DEFINE_MOD()` places an 8-byte zero
        // `unsigned long long` in `modmeta$a` and another in `$z` purely to mark
        // the section bounds, so the FIRST eight bytes at `records_begin` read
        // as size 0 — and run 031024 aborted on them at record 0 and reported
        // `hooks_installed: 0`. That is the exact failure this walker exists to
        // prevent, produced by the walker itself: a zero that looks measured.
        // Zero-size means padding or a sentinel; step over it. Only a NON-ZERO
        // unaligned size is genuinely malformed and worth refusing.
        if (size == 0u) {
            p += 8;
            continue;
        }
        if (size < sizeof(ModMetaRecord) || (size & 7u) != 0u) {
            logf(LOG_LEVEL_ERROR,
                "[WwRegistry] {\"ev\":\"hook_manifest_abort\",\"at_record\":%d,"
                "\"size\":%u,\"why\":\"size is zero, sub-header or unaligned; "
                "walk refused rather than spun\"}",
                records, size);
            break;
        }
        records++;
        if (rec->kind == MOD_META_HOOK_NAME) {
            const mods::detail::HookNameRecord<8>* h =
                reinterpret_cast<const mods::detail::HookNameRecord<8>*>(p);
            const bool ok = h->resolved != nullptr;
            ok ? installed++ : missing++;
            // TWO AXES, REPORTED SEPARATELY. `state` is RESOLUTION (the
            // loader found the symbol). `attach` is REGISTRATION (a callback
            // was actually passed to hook_add_*). They are independent, and
            // collapsing them is the bug this line exists to stop repeating:
            // RESOLVED + NOT-ATTACHED is a hook that can never fire while
            // reporting INSTALLED.
            const AttachNote* an = findAttach(p);
            const char* attachState =
                (an == nullptr)
                    ? (s_attachLedgerOverflowed
                           ? "UNKNOWN(attach ledger overflowed - not a clean result)"
                           : "NOT-ATTACHED(declared, never passed to hook_add_*)")
                    : (an->result == MOD_OK ? "ATTACHED" : result_name(an->result));
            const bool attachedOk = (an != nullptr && an->result == MOD_OK);
            if (!attachedOk) {
                attachMissing++;
            }
            logf((ok && attachedOk) ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
                "[WwRegistry] {\"ev\":\"hook_manifest\",\"name\":\"%s\","
                "\"state\":\"%s\",\"attach\":\"%s\",\"resolved\":\"%p\","
                "\"read_me\":\"state=symbol resolved; attach=callback registered. "
                "A hook can be INSTALLED and NOT-ATTACHED, and then it never "
                "fires - a zero counter for it is NOT a measurement\"}",
                h->name, ok ? "INSTALLED" : "NOT-INSTALLED(unresolved at load)",
                attachState, h->resolved);
        }
        p += size;
    }

    // SEVERITY SPANS BOTH AXES. Gating this on `missing` alone would report
    // INFO for a hook that resolved and was never attached — a green summary
    // over a hook that can never fire, which is precisely the false clean this
    // whole change exists to remove. A ledger overflow is also not-clean: it
    // means the attach axis is UNKNOWN, and unknown is never green (№31-C).
    const bool clean = (missing == 0 && attachMissing == 0
                        && !s_attachLedgerOverflowed);
    logf(clean ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"hook_manifest_summary\",\"records\":%d,"
        "\"hooks_installed\":%d,\"hooks_not_installed\":%d,"
        "\"hooks_not_attached\":%d,\"attach_notes\":%d,\"ledger_overflow\":%d,"
        "\"read_me\":\"TWO AXES. installed/not_installed = the loader RESOLVED "
        "the symbol. not_attached = no callback was registered for it. A hook "
        "can be INSTALLED and NOT-ATTACHED, in which case it never fires and a "
        "counter reading 0 for it is NOT a measurement. Both names are printed "
        "per hook above so a zero is always attributable\"}",
        records, installed, missing, attachMissing, s_attachNoteCount,
        s_attachLedgerOverflowed ? 1 : 0);
    // RETURN NOW COVERS BOTH AXES (was `missing` alone). The single caller
    // discards it today, but a caller that gates on "0 == fine" must not be
    // handed a zero that only means "every symbol resolved".
    return missing + attachMissing;
}

const char* result_name(ModResult r) {
    switch (r) {
    case MOD_OK: return "MOD_OK";
    case MOD_UNSUPPORTED: return "MOD_UNSUPPORTED(no manifest embedded/stale)";
    case MOD_UNAVAILABLE: return "MOD_UNAVAILABLE(symbol not found)";
    case MOD_CONFLICT: return "MOD_CONFLICT(ambiguous name)";
    default: return "other";
    }
}

}  // namespace

bool wwRegistry_isWwHostStage() {
    return startStageIsWw();
}

void wwFinishParsedModel(void** pParsed, const char* tag) {
    wwFinishModelData(pParsed, tag);
}

ModResult wwRegistry_initialize() {
    // ------------------------------------------------------------------------
    // HOST-CAPABILITY DETECTION FIRST — the defer flags below must be set
    // before any handler can fire. Per-symbol results are logged individually
    // (an aggregate count is exactly the silent-zero shape D1 exists to kill).
    // ------------------------------------------------------------------------
    {
        static const char* const kHostProbes[6] = {
            "?dExtWwSave_isWwHostStage@@YA_NPEBD@Z",
            "?dExtWwSave_registerWwStage@@YAXPEBD@Z",
            "?dExtWwSave_isDeclaredWwStage@@YA_NPEBD@Z",
            "?dExtWwSave_isWwContentActive@@YA_NXZ",
            "?dExtWwSave_refuseNativeWrite@@YA_NPEBDHH@Z",
            "?dExtWwCam_SkipSmoother@@YAXXZ",
        };
        for (int i = 0; i < 6; i++) {
            void* addr = nullptr;
            const ModResult r = s_hook->resolve(mod_ctx, kHostProbes[i], &addr, nullptr);
            const bool hit = (r == MOD_OK && addr != nullptr);
            if (hit) { s_hostCaps++; }
            logf(LOG_LEVEL_INFO,
                "[WwRegistry] {\"ev\":\"host_probe\",\"i\":%d,\"sym\":\"%.40s\",\"hit\":%d}",
                i, kHostProbes[i], hit ? 1 : 0);
        }
        s_hostIsFork = (s_hostCaps >= 1);
        logf(s_hostIsFork ? LOG_LEVEL_WARN : LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"host_capability\",\"fork_hosted\":%d,\"hits\":%d,"
            "\"of\":6,\"reads\":\"fork_hosted=1 => name/stage/consume-parse subsystems "
            "DEFER TO HOST (CALLS 836); 0 => full provisioning, vanilla\"}",
            s_hostIsFork ? 1 : 0, s_hostCaps);
    }
    const ModResult rPf = mods::hook_add_pre<PfGet>(s_hook, on_pfGet);
    const ModResult rDyl = mods::hook_add_pre<DylIsLinked>(s_hook, on_dylIsLinked);
    const ModResult rName = mods::hook_add_pre<StageSearchName>(s_hook, on_stageSearchName);
    // The two load-chain observers. Both pure counters; a failure to install
    // either costs nothing that works today, and both install results are
    // reported so a zero count can never be confused with a dead hook.
    const ModResult rMlt = mods::hook_add_pre<MultInfoInit>(s_hook, on_multInfoInit);
    const ModResult rMsg = mods::hook_add_pre<ReadMsgGroupLocal>(s_hook, on_readMsgGroupLocal);

    // D1 WIRING POINT — the whole-manifest INSTALLED/NOT-INSTALLED walk
    // belongs HERE, after every `hook_add_*` above, because the loader fills
    // each record's `resolved` during install; emitting earlier would report
    // NOT-INSTALLED for hooks that are about to succeed, and a gate that lies
    // early is worse than no gate.
    //
    // CALL RESTORED 2026-08-16 — the definition is back above (I was the one
    // who deleted it; see the block comment there). Leaving the placement
    // marked rather than re-deriving it was the right call by whoever wrote
    // this note: it meant the fix was a one-line re-enable instead of a second
    // competing walker, which is exactly the duplicate-manifest outcome D1's
    // own design question warned about.
    // ⚠ CALL MOVED TO THE END OF THIS FUNCTION 2026-08-16 — AND THE NOTE ABOVE
    // WAS NOT WRONG, IT WAS RIGHT FOR A ONE-AXIS REPORT. Resolution is filled
    // by the LOADER before this function runs, so for a resolved/unresolved
    // manifest the placement genuinely did not matter. It matters now:
    // ATTACHMENT is recorded by the `hook_add_*` calls themselves, and EIGHT OF
    // THE THIRTEEN happen BELOW this point (GlbResource, GetRes, ResLoadResource,
    // J3DLoadBDL, J3DLoadBMD, SetMsgDtArc, StageLoader, SetStageRes).
    // Emitting here would report 8 of 13 as NOT-ATTACHED — a false alarm, which
    // is the same class of lie as the false clean and no better.
    // The walk now runs after the last `hook_add_*`, where BOTH axes are true.
    // POST hook: the answer is what we need, not the question.
    // §1022: the getRes POST probe is retired in favour of a PRE hook on the
    // call that actually crashes. A post hook cannot see a call that does not
    // return, and this one does not.
    const ModResult rRes = mods::hook_add_pre<GlbResource>(s_hook, on_glbResource);
    // ========================================================================
    // `GetRes` RE-ATTACHED 2026-08-16 — and the §1022 retirement above stays
    // CORRECT for the reason it gave. It retired a post-hook on the call that
    // CRASHES (`getGlbResource`), because a post-hook cannot observe a call
    // that never returns. **`dRes_control_c::getRes` is a different call and it
    // DOES return** — `daBg_c::createHeap` gets its pointer back and dies later,
    // inside `mDoExt_setupStageTexture`. So the reason to retire the one does
    // not apply to the other.
    //
    // WHY THIS IS BEING FIXED AT ALL: I added the `bg_model_lookup` probe into
    // `on_getRes` and run 031403 produced ZERO lines from it — because the
    // handler was DEFINED AND NEVER REGISTERED. I then read D1's
    // `"name":"?getRes@...","state":"INSTALLED"` as proof the probe was alive.
    //
    // ⚠ IT IS NOT, AND THIS IS A REAL LIMIT OF D1 AS BUILT: `INSTALLED` means
    // the DECLARATION'S SYMBOL RESOLVED, not that a CALLBACK IS ATTACHED. A
    // hook declared with DEFINE_HOOK_SYMBOL and never passed to `hook_add_*`
    // reports INSTALLED and can never fire. That is the same silent-zero class
    // D1 exists to kill, one level up — and it fooled the person who built it.
    // ========================================================================
    const ModResult rGetRes = mods::hook_add_post<GetRes>(s_hook, on_getRes);
    logf(rGetRes == MOD_OK ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"getres_attach\",\"result\":\"%s\","
        "\"note\":\"ATTACHED is not the same as D1 INSTALLED; this line reports "
        "the callback, D1 reports the symbol\"}",
        result_name(rGetRes));
    // The supplied 'BMDL' dispatch arm (user ruling 2026-08-16, "Build the
    // hook"). POST so the receiver's own arms run first and we only see what
    // it declined. See the block comment at the DEFINE_HOOK_SYMBOL above.
    const ModResult rBmdl = mods::hook_add_post<ResLoadResource>(s_hook, on_resLoadResource);
    // The Outset visibility probe. PRE: the question is whether the call is
    // REACHED, and a post hook on a void function tells us nothing extra.
    const ModResult rDraw = mods::hook_add_pre<ModelEntryDL>(s_hook, on_modelEntryDL);
    const ModResult rAsp = mods::hook_add_pre<AddShapePacket>(s_hook, on_addShapePacket);
    const ModResult rEms = mods::hook_add_pre<EntryMatSort>(s_hook, on_entryMatSort);
    const ModResult rDlr = mods::hook_add_pre<DlstReset>(s_hook, on_dlstReset);
    const ModResult rGxC = mods::hook_add_pre<GxCallDL>(s_hook, on_gxCallDL);
    const ModResult rGxB = mods::hook_add_pre<GxBeginDL>(s_hook, on_gxBeginDL);
    const ModResult rGxE = mods::hook_add_pre<GxEndDL>(s_hook, on_gxEndDL);
    logf(LOG_LEVEL_INFO,
         "[WwRegistry] {\"ev\":\"gx_census_install\",\"addShapePacket\":%d,\"entryMatSort\":%d,\"dlstReset\":%d,\"callDL\":%d,\"beginDL\":%d,"
         "\"endDL\":%d,"
         "\"reads\":\"0=MOD_OK. These are UNDECORATED C symbols; the host exports 247 GX "
         "entry points so they should resolve. A non-zero here means the census never ran "
         "and a clean gx_census line would prove NOTHING\"}",
         (int)rAsp, (int)rEms, (int)rDlr, (int)rGxC, (int)rGxB, (int)rGxE);
    const ModResult rMk = mods::hook_add_post<J3DModelCreate>(s_hook, on_j3dModelCreate);
    const ModResult rBgD = mods::hook_add_pre<BgDraw>(s_hook, on_bgDraw);
    const ModResult rBgBtkE = mods::hook_add_post<BgBtkEntry>(s_hook, on_bgBtkEntry);
    const ModResult rBgBtkP = mods::hook_add_pre<BgBtkPlay>(s_hook, on_bgBtkPlay);
    logf((rBgBtkE == MOD_OK && rBgBtkP == MOD_OK) ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
         "[WwRegistry] {\"ev\":\"shore_btk_attach\",\"entry\":\"%s\",\"play\":\"%s\"}",
         result_name(rBgBtkE), result_name(rBgBtkP));
    const ModResult rSkyFlush = mods::hook_add_pre<DrawOpaList>(s_hook, on_drawOpaList);
    const ModResult rCamT = mods::hook_add_pre<CamNextType>(s_hook, on_camNextType);
    const ModResult rCamM = mods::hook_add_pre<CamNextMode>(s_hook, on_camNextMode);
    const ModResult rCamS = mods::hook_add_pre<CamOnStyleChange>(s_hook, on_camOnStyleChange);
    const ModResult rCamC = mods::hook_add_pre<CamTest2>(s_hook, on_camTest2);
    logf((rCamT == MOD_OK && rCamM == MOD_OK && rCamS == MOD_OK && rCamC == MOD_OK)
             ? LOG_LEVEL_INFO
             : LOG_LEVEL_ERROR,
         "[WwRegistry] {\"ev\":\"wwcam_attach\",\"nextType\":\"%s\",\"nextMode\":\"%s\","
         "\"onStyle\":\"%s\",\"test2\":\"%s\"}",
         result_name(rCamT), result_name(rCamM), result_name(rCamS), result_name(rCamC));
    const ModResult rCF = mods::hook_add_pre<ClipChangeFar>(s_hook, on_clipChangeFar);
    const ModResult rDwE = (s_diagProbes ? mods::hook_add_pre<DwExecute>(s_hook, on_dwExecute) : MOD_OK);
    const ModResult rTQ = mods::hook_add_post<LyTgToQueue>(s_hook, on_lyTgToQueue);
    // Probe set #20 — the phase ladder. POST throughout: three of the four
    // answers ARE return values, and setSceneName's answer (the spot string)
    // is unchanged by when it is read.
    const ModResult rASS = mods::hook_add_post<AudSetScene>(s_hook, on_audSetScene);
    const ModResult rAL1 = mods::hook_add_post<AudLoad1st>(s_hook, on_audLoad1st);
    const ModResult rAC1 = mods::hook_add_post<AudCheck1st>(s_hook, on_audCheck1st);
    const ModResult rSOR = mods::hook_add_post<SyncObjRes>(s_hook, on_syncObjRes);
    // Probe set #21 — the drawability door. Both POST: the create STATUS is
    // the return value, and an admission has no information before it happens.
    const ModResult rMtd = (s_diagProbes ? mods::hook_add_post<MtdCreate>(s_hook, on_mtdCreate) : MOD_OK);
    const ModResult rDwQ = (s_diagProbes ? mods::hook_add_post<DwTgToDrawQ>(s_hook, on_dwTgToDrawQ) : MOD_OK);
    // Probe set #22 — the removal side. Cut is POST (the tag is still readable
    // either way); wipe is PRE so the count of prior admissions is reported
    // BEFORE the tree forgets them.
    const ModResult rCut = mods::hook_add_post<DwTgCut>(s_hook, on_dwTgCut);
    const ModResult rWipe = mods::hook_add_pre<DwTgWipe>(s_hook, on_dwTgWipe);
    logf((rCut == MOD_OK && rWipe == MOD_OK) ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"drawq_removal_attach\",\"cut\":\"%s\",\"wipe\":\"%s\"}",
        result_name(rCut), result_name(rWipe));
    // The 144259 crash guard. PRE with skip; see the DEFINE block. The getter
    // resolve happens in the callable block below — attach order is safe
    // because the handler treats an unresolved getter as "never skip".
    const ModResult rAtG = mods::hook_add_pre<AttnSetOwnerPos>(s_hook, on_attnSetOwnerPos);
    logf(rAtG == MOD_OK ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"attn_guard_attach\",\"result\":\"%s\","
        "\"reads\":\"guards crash 144259 (NULL player in dAttention Run)\"}",
        result_name(rAtG));
    // Probe set #23 — the admission's return. POST: the return IS the answer.
    const ModResult rTgA = (s_diagProbes ? mods::hook_add_post<TgAddToTree>(s_hook, on_tgAddToTree) : MOD_OK);
    logf(rTgA == MOD_OK ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"tg_add_attach\",\"result\":\"%s\"}", result_name(rTgA));
    // Probe set #24 — the node walk. PRE: the census is of REACHING the node.
    const ModResult rNdD = (s_diagProbes ? mods::hook_add_pre<NdDraw>(s_hook, on_ndDraw) : MOD_OK);
    logf(rNdD == MOD_OK ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"nd_draw_attach\",\"result\":\"%s\"}", result_name(rNdD));
    // Probe set #25 — the overlap gate. POST throughout: the returns ARE the data.
    const ModResult rOv1 = mods::hook_add_post<OvlpIsDoing>(s_hook, on_ovlpIsDoing);
    const ModResult rOv2 = mods::hook_add_post<OvlpIsDone>(s_hook, on_ovlpIsDone);
    const ModResult rOv3 = mods::hook_add_post<OvlpClear>(s_hook, on_ovlpClear);
    logf((rOv1 == MOD_OK && rOv2 == MOD_OK && rOv3 == MOD_OK)
             ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"ovlp_gate_attach\",\"isDoing\":\"%s\",\"isDone\":\"%s\","
        "\"clear\":\"%s\"}", result_name(rOv1), result_name(rOv2), result_name(rOv3));
    // Probe set #26 — the overlap's state machine. POST throughout.
    const ModResult rOv4 = mods::hook_add_post<OvlpIsOutReq>(s_hook, on_ovlpIsOutReq);
    const ModResult rOv5 = mods::hook_add_post<OvlpSceneStart>(s_hook, on_ovlpSceneStart);
    const ModResult rOv6 = mods::hook_add_post<OvlpDone>(s_hook, on_ovlpDone);
    const ModResult rOv7 = mods::hook_add_post<JfwStartFadeOut>(s_hook, on_jfwStartFadeOut);
    logf((rOv4 == MOD_OK && rOv5 == MOD_OK && rOv6 == MOD_OK && rOv7 == MOD_OK)
             ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"ovlp_state_attach\",\"isOutReq\":\"%s\",\"sceneStart\":\"%s\","
        "\"done\":\"%s\",\"jfwFadeOut\":\"%s\"}",
        result_name(rOv4), result_name(rOv5), result_name(rOv6), result_name(rOv7));
    // Probe set #27 — the command and the pump. POST.
    const ModResult rCmd = mods::hook_add_post<CReqCommand>(s_hook, on_cReqCommand);
    const ModResult rPmp = (s_diagProbes ? mods::hook_add_post<NdRqHandler>(s_hook, on_ndRqHandler) : MOD_OK);
    const ModResult rCrt = mods::hook_add_post<CReqCreate>(s_hook, on_cReqCreate);
    logf((rCmd == MOD_OK && rPmp == MOD_OK && rCrt == MOD_OK)
             ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"pump_attach\",\"creq_command\":\"%s\",\"ndrq_pump\":\"%s\","
        "\"creq_create\":\"%s\"}",
        result_name(rCmd), result_name(rPmp), result_name(rCrt));
    // Probe set #28 — the constructors. POST; returns are the data.
    const ModResult rOvR = mods::hook_add_post<OvlpMRequest>(s_hook, on_ovlpMRequest);
    const ModResult rChR = mods::hook_add_post<ScnMChangeReq>(s_hook, on_scnMChangeReq);
    logf((rOvR == MOD_OK && rChR == MOD_OK) ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"ctor_attach\",\"ovlp_request\":\"%s\",\"change_req\":\"%s\"}",
        result_name(rOvR), result_name(rChR));
    // Probe set #29 — the swap innards. POST; returns are the data.
    const ModResult rNqE = (s_diagProbes ? mods::hook_add_post<NdRqExecute>(s_hook, on_ndRqExecute) : MOD_OK);
    const ModResult rBsD = mods::hook_add_post<BsDelete>(s_hook, on_bsDelete);
    logf((rNqE == MOD_OK && rBsD == MOD_OK) ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"swap_attach\",\"ndrq_exec\":\"%s\",\"bs_delete\":\"%s\"}",
        result_name(rNqE), result_name(rBsD));
    // Probe set #30 — the ten-hypothesis net.
    const ModResult rN1 = (s_diagProbes ? mods::hook_add_post<MIsCreating>(s_hook, on_mIsCreating) : MOD_OK);
    const ModResult rN2 = mods::hook_add_post<ScnPauseEnable>(s_hook, on_scnPauseEnable);
    const ModResult rN3 = mods::hook_add_post<ScnPauseDisable>(s_hook, on_scnPauseDisable);
    const ModResult rN4 = (s_diagProbes ? mods::hook_add_pre<ScnMManagement>(s_hook, on_scnMManagement) : MOD_OK);
    const ModResult rN5 = mods::hook_add_post<NdRqReRequest>(s_hook, on_ndRqReRequest);
    const ModResult rN6 = mods::hook_add_post<MDelete>(s_hook, on_mDelete);
    logf((rN1 == MOD_OK && rN2 == MOD_OK && rN3 == MOD_OK && rN4 == MOD_OK &&
          rN5 == MOD_OK && rN6 == MOD_OK) ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"net_attach\",\"isCreating\":\"%s\",\"pauseEn\":\"%s\","
        "\"pauseDis\":\"%s\",\"scnMgmt\":\"%s\",\"reReq\":\"%s\",\"mDelete\":\"%s\"}",
        result_name(rN1), result_name(rN2), result_name(rN3), result_name(rN4),
        result_name(rN5), result_name(rN6));
    // Probe set #31 — the create-queue pop. POST.
    const ModResult rC1p = (s_diagProbes ? mods::hook_add_pre<CtRqDo>(s_hook, on_ctRqDoPre) : MOD_OK);
    (void)rC1p;   // depth-only; shares CtRqDo's ledger line
    const ModResult rC1 = (s_diagProbes ? mods::hook_add_post<CtRqDo>(s_hook, on_ctRqDo) : MOD_OK);
    const ModResult rC2 = mods::hook_add_post<ExToExecuteQ>(s_hook, on_exToExecuteQ);
    const ModResult rC3 = (s_diagProbes ? mods::hook_add_post<CtRqHandler>(s_hook, on_ctRqHandler) : MOD_OK);
    const ModResult rC4 = mods::hook_add_post<CtRqDelete>(s_hook, on_ctRqDelete);
    const ModResult rC5 = mods::hook_add_post<CtRqCancel>(s_hook, on_ctRqCancel);
    const ModResult rC6 = mods::hook_add_post<CtRqEnqueue>(s_hook, on_ctRqEnqueue);
    const ModResult rC7 = mods::hook_add_post<LyCreatingMesg>(s_hook, on_lyCreatingMesg);
    const ModResult rC8 = mods::hook_add_post<LyCreatedMesg>(s_hook, on_lyCreatedMesg);
    const ModResult rC9 = mods::hook_add_post<LyDelete>(s_hook, on_lyDelete);
    // Probe set #33 — collision registration receipts. POST.
    const ModResult rG1 = mods::hook_add_post<CBgWSet>(s_hook, on_cBgWSet);
    const ModResult rG2 = mods::hook_add_post<RoomSetBgW>(s_hook, on_roomSetBgW);
    const ModResult rG3 = mods::hook_add_post<DBgSRegist>(s_hook, on_dBgSRegist);
    const ModResult rG4 = (s_diagProbes ? mods::hook_add_post<GroundCross>(s_hook, on_groundCross) : MOD_OK);
    const ModResult rG5 = (s_diagProbes ? mods::hook_add_post<BgWGroundCross>(s_hook, on_bgwGroundCross) : MOD_OK);
    const ModResult rG6 = (s_diagProbes ? mods::hook_add_pre<BgWGroundCrossRp>(s_hook, on_bgwGroundCrossRp) : MOD_OK);
    const ModResult rG7 = (s_diagProbes ? mods::hook_add_post<RwgGndCheck>(s_hook, on_rwgGndCheck) : MOD_OK);
    const ModResult rG8 = (s_diagProbes ? mods::hook_add_post<AcchCrrPos>(s_hook, on_acchCrrPos) : MOD_OK);
    // Phase 4a roof-clamp gate: PRE arm, NOT gated on s_diagProbes — it is
    // behaviour, not measurement. Rides the AcchCrrPos symbol already declared.
    const ModResult rG8p = mods::hook_add_pre<AcchCrrPos>(s_hook, on_acchCrrPosGate);
    logf(LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"acch_thinceil_gate_install\",\"r\":%d}", (int)rG8p);
    (void)rG8;
    (void)rG7;   // leaf-level receipt; shares the bg_receipt attach semantics
    (void)rG6;   // walk receipt; shares the bg_receipt attach line semantics
    logf((rG1 == MOD_OK && rG2 == MOD_OK && rG3 == MOD_OK && rG4 == MOD_OK && rG5 == MOD_OK)
             ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"bg_receipt_attach\",\"cbgw_set\":\"%s\",\"set_bgw\":\"%s\","
        "\"dbgs_regist\":\"%s\",\"ground\":\"%s\",\"bgw_ground\":\"%s\"}",
        result_name(rG1), result_name(rG2), result_name(rG3), result_name(rG4),
        result_name(rG5));
    logf((rC1 == MOD_OK && rC2 == MOD_OK && rC3 == MOD_OK && rC4 == MOD_OK &&
          rC5 == MOD_OK && rC6 == MOD_OK && rC7 == MOD_OK && rC8 == MOD_OK &&
          rC9 == MOD_OK) ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"ctq_attach\",\"ctrq_do\":\"%s\",\"ex_toq\":\"%s\","
        "\"ct_pump\":\"%s\",\"delete\":\"%s\",\"cancel\":\"%s\",\"enqueue\":\"%s\","
        "\"lyCreating\":\"%s\",\"lyCreated\":\"%s\",\"lyDelete\":\"%s\"}",
        result_name(rC1), result_name(rC2), result_name(rC3), result_name(rC4),
        result_name(rC5), result_name(rC6), result_name(rC7), result_name(rC8),
        result_name(rC9));
    logf((rMtd == MOD_OK && rDwQ == MOD_OK) ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"create_door_attach\",\"fpcMtd_Create\":\"%s\","
        "\"fopDwTg_ToDrawQ\":\"%s\"}", result_name(rMtd), result_name(rDwQ));
    logf((rASS == MOD_OK && rAL1 == MOD_OK && rAC1 == MOD_OK && rSOR == MOD_OK)
             ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"phase_ladder_attach\",\"setSceneName\":\"%s\","
        "\"load1st\":\"%s\",\"check1st\":\"%s\",\"syncObjRes\":\"%s\"}",
        result_name(rASS), result_name(rAL1), result_name(rAC1), result_name(rSOR));
    logf(rDraw == MOD_OK ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"draw_probe\",\"attach\":\"%s\","
        "\"reads\":\"FIRES = geometry submitted, fault is downstream; "
        "SILENT = nothing submitted, fault is upstream in parts/room\"}",
        result_name(rDraw));
    const ModResult rBdl = mods::hook_add_post<J3DLoadBDL>(s_hook, on_j3dLoadBDL);
    const ModResult rBmd = mods::hook_add_post<J3DLoadBMD>(s_hook, on_j3dLoadBMD);
    const ModResult rLockDL = mods::hook_add_pre<LockedMakeSharedDL>(s_hook, on_lockedMakeSharedDL);
    const ModResult rPatchDL = mods::hook_add_pre<PatchedMakeSharedDL>(s_hook, on_patchedMakeSharedDL);
    logf((rBdl == MOD_OK && rBmd == MOD_OK && rBmdl == MOD_OK)
             ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"j3d_hooks\",\"loadBinaryDisplayList\":\"%s\","
        "\"load\":\"%s\",\"bmdl_arm\":\"%s\",\"locked_makeSharedDL\":\"%s\","
        "\"patched_makeSharedDL\":\"%s\"}",
        result_name(rBdl), result_name(rBmd), result_name(rBmdl),
        result_name(rLockDL), result_name(rPatchDL));
    const ModResult rMsgArc = mods::hook_add_pre<SetMsgDtArc>(s_hook, on_setMsgDtArc);
    logf(rMsgArc == MOD_OK ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
        "[WwRegistry] {\"ev\":\"msgarc_hook\",\"setMsgDtArchive\":\"%s\","
        "\"getMsgDtArchive_readable\":%d}",
        result_name(rMsgArc), s_fnGetMsgDtArc != nullptr);
    const ModResult rLdr = mods::hook_add_pre<StageLoader>(s_hook, on_stageLoader);
    const ModResult rRoomLdr = mods::hook_add_pre<RoomLoader>(s_hook, on_roomLoader);
    const ModResult rKyLt = mods::hook_add_pre<KySetLight>(s_hook, on_kySetLight);
    const ModResult rMaji = mods::hook_add_pre<KyMajiModelData>(s_hook, on_kyMaji);
    const ModResult rTev = mods::hook_add_post<KySettingTev>(s_hook, on_kySettingTev);
    const ModResult rStCr = mods::hook_add_post<StageCreate>(s_hook, on_stageCreate);
    const ModResult rSSR = mods::hook_add_pre<SetStageRes>(s_hook, on_setStageRes);

    // ========================================================================
    // RECORD ATTACHMENT FOR ALL THIRTEEN, THEN WALK THE MANIFEST.
    //
    // This block is the whole of the second axis, and it is deliberately ONE
    // place rather than 13 edits at the call sites: every `ModResult` above is
    // still in scope here, so the ledger is filled from the ACTUAL returned
    // results and cannot drift from them. Adding a 14th hook without adding
    // its line here makes it report NOT-ATTACHED — LOUD, and the correct
    // direction to fail in.
    //
    // The key is `&mod_meta_hook_<alias>`, the record the macro emits and the
    // exact object the walker will encounter (`sdk/include/mods/hook.hpp:153`).
    // ========================================================================
    noteAttach(&mod_meta_hook_PfGet, rPf);
    noteAttach(&mod_meta_hook_DylIsLinked, rDyl);
    noteAttach(&mod_meta_hook_StageSearchName, rName);
    noteAttach(&mod_meta_hook_MultInfoInit, rMlt);
    noteAttach(&mod_meta_hook_ReadMsgGroupLocal, rMsg);
    noteAttach(&mod_meta_hook_GlbResource, rRes);
    noteAttach(&mod_meta_hook_GetRes, rGetRes);
    noteAttach(&mod_meta_hook_ResLoadResource, rBmdl);
    noteAttach(&mod_meta_hook_J3DLoadBDL, rBdl);
    noteAttach(&mod_meta_hook_J3DLoadBMD, rBmd);
    noteAttach(&mod_meta_hook_LockedMakeSharedDL, rLockDL);
    noteAttach(&mod_meta_hook_PatchedMakeSharedDL, rPatchDL);
    noteAttach(&mod_meta_hook_SetMsgDtArc, rMsgArc);
    noteAttach(&mod_meta_hook_StageLoader, rLdr);
    noteAttach(&mod_meta_hook_RoomLoader, rRoomLdr);
    noteAttach(&mod_meta_hook_KySetLight, rKyLt);
    noteAttach(&mod_meta_hook_KyMajiModelData, rMaji);
    noteAttach(&mod_meta_hook_KySettingTev, rTev);
    noteAttach(&mod_meta_hook_StageCreate, rStCr);
    noteAttach(&mod_meta_hook_SetStageRes, rSSR);
    // 14th — the Outset draw probe. Added on Housing's heads-up: attachment
    // comes from THIS ledger, not from the loader, so a hook that is declared
    // and genuinely attached still reports NOT-ATTACHED without a line here.
    // That would be a FALSE ALARM from the gate built to end false cleans, and
    // it is the one failure mode worse than the silence it replaced.
    noteAttach(&mod_meta_hook_ModelEntryDL, rDraw);
    noteAttach(&mod_meta_hook_J3DModelCreate, rMk);
    noteAttach(&mod_meta_hook_BgDraw, rBgD);
    noteAttach(&mod_meta_hook_BgBtkEntry, rBgBtkE);
    noteAttach(&mod_meta_hook_BgBtkPlay, rBgBtkP);
    noteAttach(&mod_meta_hook_DrawOpaList, rSkyFlush);
    noteAttach(&mod_meta_hook_CamNextType, rCamT);
    noteAttach(&mod_meta_hook_CamNextMode, rCamM);
    noteAttach(&mod_meta_hook_CamOnStyleChange, rCamS);
    noteAttach(&mod_meta_hook_CamTest2, rCamC);
    noteAttach(&mod_meta_hook_ClipChangeFar, rCF);
    noteAttach(&mod_meta_hook_DwExecute, rDwE);
    noteAttach(&mod_meta_hook_LyTgToQueue, rTQ);
    noteAttach(&mod_meta_hook_AudSetScene, rASS);
    noteAttach(&mod_meta_hook_AudLoad1st, rAL1);
    noteAttach(&mod_meta_hook_AudCheck1st, rAC1);
    noteAttach(&mod_meta_hook_SyncObjRes, rSOR);
    noteAttach(&mod_meta_hook_MtdCreate, rMtd);
    noteAttach(&mod_meta_hook_DwTgToDrawQ, rDwQ);
    noteAttach(&mod_meta_hook_DwTgCut, rCut);
    noteAttach(&mod_meta_hook_DwTgWipe, rWipe);
    noteAttach(&mod_meta_hook_AttnSetOwnerPos, rAtG);
    noteAttach(&mod_meta_hook_TgAddToTree, rTgA);
    noteAttach(&mod_meta_hook_NdDraw, rNdD);
    noteAttach(&mod_meta_hook_OvlpIsDoing, rOv1);
    noteAttach(&mod_meta_hook_OvlpIsDone, rOv2);
    noteAttach(&mod_meta_hook_OvlpClear, rOv3);
    noteAttach(&mod_meta_hook_OvlpIsOutReq, rOv4);
    noteAttach(&mod_meta_hook_OvlpSceneStart, rOv5);
    noteAttach(&mod_meta_hook_OvlpDone, rOv6);
    noteAttach(&mod_meta_hook_JfwStartFadeOut, rOv7);
    noteAttach(&mod_meta_hook_CReqCommand, rCmd);
    noteAttach(&mod_meta_hook_NdRqHandler, rPmp);
    noteAttach(&mod_meta_hook_CReqCreate, rCrt);
    noteAttach(&mod_meta_hook_OvlpMRequest, rOvR);
    noteAttach(&mod_meta_hook_ScnMChangeReq, rChR);
    noteAttach(&mod_meta_hook_NdRqExecute, rNqE);
    noteAttach(&mod_meta_hook_BsDelete, rBsD);
    noteAttach(&mod_meta_hook_MIsCreating, rN1);
    noteAttach(&mod_meta_hook_ScnPauseEnable, rN2);
    noteAttach(&mod_meta_hook_ScnPauseDisable, rN3);
    noteAttach(&mod_meta_hook_ScnMManagement, rN4);
    noteAttach(&mod_meta_hook_NdRqReRequest, rN5);
    noteAttach(&mod_meta_hook_MDelete, rN6);
    noteAttach(&mod_meta_hook_CtRqDo, rC1);
    noteAttach(&mod_meta_hook_ExToExecuteQ, rC2);
    noteAttach(&mod_meta_hook_CtRqHandler, rC3);
    noteAttach(&mod_meta_hook_CtRqDelete, rC4);
    noteAttach(&mod_meta_hook_CtRqCancel, rC5);
    noteAttach(&mod_meta_hook_CtRqEnqueue, rC6);
    noteAttach(&mod_meta_hook_LyCreatingMesg, rC7);
    noteAttach(&mod_meta_hook_LyCreatedMesg, rC8);
    noteAttach(&mod_meta_hook_LyDelete, rC9);
    noteAttach(&mod_meta_hook_CBgWSet, rG1);
    noteAttach(&mod_meta_hook_RoomSetBgW, rG2);
    noteAttach(&mod_meta_hook_DBgSRegist, rG3);
    noteAttach(&mod_meta_hook_GroundCross, rG4);
    noteAttach(&mod_meta_hook_BgWGroundCross, rG5);
    noteAttach(&mod_meta_hook_BgWGroundCrossRp, rG6);
    noteAttach(&mod_meta_hook_RwgGndCheck, rG7);
    noteAttach(&mod_meta_hook_AcchCrrPos, rG8);
    emitHookManifest();

    // ------------------------------------------------------------------------
    // ACTOR PREREQUISITES. An actor profile must point at the receiver's own
    // leaf-draw and actor method tables; a plugin cannot author them. Resolve
    // them here and REPORT, so the first actor row is written against a proven
    // address rather than a hope. Failure is not fatal — the vehicle stays
    // inert exactly as it is today — but it is the gate on registering actors.
    // ------------------------------------------------------------------------
    HookSymbolFlags leafFlags = (HookSymbolFlags)0, actorFlags = (HookSymbolFlags)0;
    void* leafAddr = nullptr;
    void* actorAddr = nullptr;
    const ModResult rLeaf = s_hook->resolve(
        mod_ctx, "?g_fpcLf_Method@@3Uleafdraw_method_class@@B", &leafAddr, &leafFlags);
    const ModResult rActor = s_hook->resolve(
        mod_ctx, "?g_fopAc_Method@@3Uactor_method_class@@A", &actorAddr, &actorFlags);
    s_leafMethod = leafAddr;
    s_actorMethod = actorAddr;
    // ------------------------------------------------------------------------
    // BUILD THE PILOT PROFILE from the resolved receiver addresses. Done here
    // and not as a static initialiser precisely because two of its fields are
    // HOST addresses that do not exist until resolve() succeeds. If either is
    // missing the profile is left zeroed and the row is inert — better a
    // pilot that never spawns than one pointing at NULL method tables.
    // ------------------------------------------------------------------------
    // Bind tag_so's four verified receiver calls. Each was name-SAFE on both
    // images AND signature-compared donor-to-receiver before a line was written.
    {
        void* a = nullptr;
        // The start-stage reader. Resolved as a CALLABLE, not hooked: the
        // out-of-line copy reads the same `mStartStage` every inlined copy
        // reads, so calling it returns the receiver's own truth. Hooking it
        // was the mistake run 030100 disproved.
        if (s_hook->resolve(mod_ctx, "dComIfGp_getStartStageName", &a, nullptr) == MOD_OK)
            s_fnGetStartStageName = reinterpret_cast<FnGetStartStageName>(a);
        a = nullptr;
        // The attention guard's predicate — the receiver's own exported player
        // getter, gate-verified `?daPy_getPlayerActorClass@@YAPEAVdaPy_py_c@@XZ`
        // on the user's image. Resolved as a CALLABLE; the guard treats
        // resolve-failure as "never skip" (stock behavior).
        if (s_hook->resolve(mod_ctx, "?daPy_getPlayerActorClass@@YAPEAVdaPy_py_c@@XZ",
                            &a, nullptr) == MOD_OK)
            s_fnGetPlayerActor = reinterpret_cast<FnGetPlayer>(a);
        a = nullptr;
        // The DZB conversion arm's converter — the receiver's own, static SA.
        if (s_hook->resolve(mod_ctx, "?ConvDzb@cBgS@@SAPEAXPEAX@Z",
                            &a, nullptr) == MOD_OK)
            s_fnConvDzb = reinterpret_cast<FnConvDzb>(a);
        a = nullptr;
        // Message-archive slot READER, mangled free-function form (Integrator).
        if (s_hook->resolve(mod_ctx, "?dComIfGp_getMsgDtArchive@@YAPEAVJKRArchive@@H@Z",
                            &a, nullptr) == MOD_OK)
            s_fnGetMsgDtArc = reinterpret_cast<FnGetMsgDtArc>(a);
        a = nullptr;
        if (s_hook->resolve(mod_ctx, "?dComIfGp_setMsgDtArchive@@YAXHPEAVJKRArchive@@@Z",
                            &a, nullptr) == MOD_OK)
            s_fnSetMsgDtArc = reinterpret_cast<FnSetMsgDtArc>(a);
        a = nullptr;
        // ====================================================================
        // THE TWO J3D PARSERS AS CALLABLES — needed because supplying the
        // 'BMDL' arm means CALLING the loader, not merely observing it. Both
        // are already bound above as post-hook OBSERVERS; these are the
        // resolve-then-CALL half of the same symbols. Static (`SA`) in the
        // mangling, so `void*(const void*, unsigned int)` with NO `this`.
        // ====================================================================
        if (s_hook->resolve(mod_ctx,
                "?loadBinaryDisplayList@J3DModelLoaderDataBase@@SAPEAVJ3DModelData@@PEBXI@Z",
                &a, nullptr) == MOD_OK)
            s_fnJ3DLoadBDL = reinterpret_cast<FnJ3DParse>(a);
        a = nullptr;
        if (s_hook->resolve(mod_ctx,
                "?load@J3DModelLoaderDataBase@@SAPEAVJ3DModelData@@PEBXI@Z",
                &a, nullptr) == MOD_OK)
            s_fnJ3DLoadBMD = reinterpret_cast<FnJ3DParse>(a);
        a = nullptr;
        if (s_hook->resolve(mod_ctx,
                "?makeSharedDisplayList@J3DMaterial@@UEAAXXZ",
                &a, nullptr) == MOD_OK)
            s_fnJ3DMatMakeSharedDL = reinterpret_cast<FnMatMakeSharedDL>(a);
        a = nullptr;
        // RECEIPT on the resolves themselves. Run 225743 could not distinguish
        // "both parsers resolved and the walk found nothing" from "neither
        // resolved, so the handler returned on its first check" — because a
        // failed resolve left a null pointer and printed nothing at all.
        logf((s_fnJ3DLoadBDL != nullptr && s_fnJ3DLoadBMD != nullptr)
                 ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"bmdl_parsers\",\"loadBinaryDisplayList\":\"%p\","
            "\"load\":\"%p\",\"mat_makeSharedDL\":\"%p\"}",
            (void*)s_fnJ3DLoadBDL, (void*)s_fnJ3DLoadBMD, (void*)s_fnJ3DMatMakeSharedDL);
        // ====================================================================
        // NEXT-stage name, added by the INTEGRATOR at the gate 2026-08-15
        // (§1016 ask, second half). Housing moved the sampler to `fpcPf_Get`
        // — the important half — but shipped only the START read.
        //
        // WHY IT RIDES THIS BUILD RATHER THAN THE NEXT ONE: with START alone,
        // a result of "still F_SP103" cannot distinguish "the copy at
        // d_s_play.cpp:1308 never ran" from "setNextStage never stuck". That
        // is two boots for one question, and the standing user rule is one
        // probe testing many hypotheses at once.
        //
        // SAFE BY THE SAME REASONING AS THE START READ: `getNextStageName`
        // returns `const char*` directly, so there is NO dStage_startStage_c
        // layout assumption — which matters because that class is MISSING from
        // the manifest on both images and a field-misread would be silent.
        // Measured SAFE/SAFE on fork and vanilla. We CALL it, never hook it,
        // so inlining at other call sites is irrelevant.
        // ====================================================================
        if (s_hook->resolve(mod_ctx, "dComIfGp_getNextStageName", &a, nullptr) == MOD_OK)
            s_fnGetNextStageName = reinterpret_cast<FnGetStartStageName>(a);
        a = nullptr;
        if (s_hook->resolve(mod_ctx, "fopAcM_GetParam", &a, nullptr) == MOD_OK)
            s_fnGetParam = reinterpret_cast<FnGetParam>(a);
        a = nullptr;
        if (s_hook->resolve(mod_ctx, "fopAcM_CheckCondition", &a, nullptr) == MOD_OK)
            s_fnCheckCondition = reinterpret_cast<FnCondition>(a);
        a = nullptr;
        if (s_hook->resolve(mod_ctx, "fopAcM_OnCondition", &a, nullptr) == MOD_OK)
            s_fnOnCondition = reinterpret_cast<FnCondition>(a);
        a = nullptr;
        if (s_hook->resolve(mod_ctx, "OSPanic", &a, nullptr) == MOD_OK)
            s_fnOSPanic = reinterpret_cast<FnOSPanic>(a);
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"tagso_bindings\",\"GetParam\":%d,"
            "\"CheckCondition\":%d,\"OnCondition\":%d,\"OSPanic\":%d,"
            "\"getStartStageName_readable\":%d,"
            "\"stageLoader_hook\":\"%s\",\"setStageRes_hook\":\"%s\","
            "\"multInfoInit_hook\":\"%s\",\"glbResource_hook\":\"%s\",\"msgGroup_hook\":\"%s\"}",
            s_fnGetParam != nullptr, s_fnCheckCondition != nullptr,
            s_fnOnCondition != nullptr, s_fnOSPanic != nullptr,
            s_fnGetStartStageName != nullptr,
            result_name(rLdr), result_name(rSSR), result_name(rMlt), result_name(rRes), result_name(rMsg));
    }

    // ------------------------------------------------------------------------
    // b2 — BIND THE STAGE-CHANGE ENTRY POINT. Reported with the resolve RESULT
    // and not just a boolean: MOD_CONFLICT here would mean the mangled form is
    // ambiguous too, which is a different failure from MOD_UNAVAILABLE and
    // sends b2 somewhere different. `result_name` distinguishes them.
    // ------------------------------------------------------------------------
    {
        void* a = nullptr;
        const ModResult rNext = s_hook->resolve(
            mod_ctx, "?dComIfGp_setNextStage@@YAXPEBDFCC@Z", &a, nullptr);
        if (rNext == MOD_OK) {
            s_fnSetNextStage = reinterpret_cast<FnSetNextStage>(a);
        }
        // THE TRANSPORT, reported on its own now that autofire is retired.
        // This binding is what the warp-menu and Fado-door selection paths
        // will call; it carries the LAYER argument those paths need, which is
        // the half of the work the user named as remaining (§1009).
        // Register the WW Warp menu-bar tab. Guarded on `s_ui` because the UI
        // service may be absent on an image that predates it — and that
        // absence is a reportable answer, not a crash.
        if (s_ui != nullptr) {
            UiMenuTabDesc tab = UI_MENU_TAB_DESC_INIT;
            tab.label = "WW Warp";
            tab.on_selected = onWwWarpTabSelected;
            const ModResult rTab =
                s_ui->register_menu_tab(mod_ctx, &tab, &s_wwMenuTab);
            logf(rTab == MOD_OK ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
                "[WwRegistry] {\"ev\":\"ww_warp_menu\",\"register\":\"%s\","
                "\"label\":\"WW Warp\",\"destinations\":%d,\"warp_bound\":%d,"
                "\"note\":\"selecting the tab pushes the Outset warp window\"}",
                result_name(rTab), s_routeCount, s_fnSetNextStage != nullptr);
        } else {
            logf(LOG_LEVEL_ERROR,
                "[WwRegistry] {\"ev\":\"ww_warp_menu\",\"register\":\"NO UI "
                "SERVICE\",\"note\":\"this image does not carry "
                "dev.twilitrealm.dusklight.ui\"}");
        }

        logf(s_fnSetNextStage != nullptr ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"b2_bind\",\"setNextStage\":\"%s\","
            "\"addr\":\"%p\",\"autofire\":\"RETIRED (user ruling 1009)\","
            "\"note\":\"transport bound and idle; awaiting a selection path\"}",
            result_name(rNext), a);
    }

    if (rLeaf == MOD_OK && rActor == MOD_OK) {
        s_pilotProfile.base.base.layer_id = 0;          // fpcLy_CURRENT_e
        s_pilotProfile.base.base.list_id = 0x0002;      // as donor tag_so
        s_pilotProfile.base.base.list_priority = 0;     // fpcPi_CURRENT_e
        s_pilotProfile.base.base.name = kPilotIndex;
        s_pilotProfile.base.base.methods =
            reinterpret_cast<const WwMethodClass*>(s_leafMethod);
        s_pilotProfile.base.base.process_size = kActorBaseSize + sizeof(TagSoMembers);
        s_pilotProfile.base.base.unk_size = 0;
        s_pilotProfile.base.base.parameters = 0;
        s_pilotProfile.base.sub_method = s_actorMethod;
        s_pilotProfile.base.priority = kPilotDrawPriority;
        s_pilotProfile.sub_method = &s_pilotMethods;
        // ====================================================================
        // STATUS + CULL TYPE — the donor's own values, previously dropped to 0.
        //
        // History's §991 spec listed the donor profile field-by-field
        // (`d_a_tag_so.cpp:106-114`) and two of them were zero here:
        //     /* Status    */ fopAcStts_UNK40000_e
        //     /* Cull Type */ fopAc_CULLBOX_4_e
        //
        // Both transfer as NUMBERS, verified in BOTH enums rather than assumed:
        //   · status is a BITMASK — donor `UNK40000` is literally 0x40000, and
        //     the receiver's `fopAcStts_UNK_0x40000_e` (f_op_actor.h:49) is
        //     `1 << 18` = the same bit. Value-identical, not merely same-named.
        //   · cullType indexes `fopAc_Cull_e`, which is POSITIONAL in both
        //     trees and agrees element-for-element through CULLBOX_7
        //     (donor f_op_actor.h:91-99, receiver :81-88), so CULLBOX_4 == 4
        //     on both sides.
        //
        // NEITHER IS OBSERVABLE FOR THIS ACTOR, and I am carrying them anyway.
        // tag_so's `_execute` returns TRUE unconditionally and its `_draw`
        // renders nothing, so no cull decision and no status bit can change
        // what a player sees. Under DN-10 that is not a licence to drop them:
        // a zero here is a value the donor did not write, and the next actor
        // to reuse this profile-fill path inherits the habit.
        //
        // NOT TRANSFERRED — `fpcDwPi_TAG_SO_e`, and this one is deliberate.
        // Draw priority is an UNVALUED positional enum in both trees (donor
        // f_pc_draw_priority.h:117 = 0x0065; the receiver's enum is likewise
        // bare, 789 entries, its own ordering). The donor's 101 names a
        // different slot in the receiver's list, so copying the number would
        // import a wrong ordering key while LOOKING faithful.
        //
        // MECHANISM (traced to the consumer): f_pc_leaf.cpp:66 ->
        // f_op_actor.cpp:559 -> f_op_draw_tag.cpp:14 -> c_tree.cpp:13-17. Draw
        // priority is an INDEX INTO A 1000-SLOT BUCKET ARRAY, not a sort key,
        // so a number's meaning depends on which RECEIVER actors sit in the
        // neighbouring buckets. 101 is a valid index here — it would not fail
        // loudly, it would place the actor at a WW-derived position in a
        // TP-ordered queue.
        //
        // 0 is right for THIS actor specifically, not merely safe: tag_so's
        // `_draw` returns TRUE without drawing, so its bucket is never
        // observable. A receiver-side mapping is needed only for an actor whose
        // draw does something. Full write-up: tale §995; the unchecked-negative
        // hazard this uncovered in c_tree.cpp:14 is routed as its own CALLS row.
        // ====================================================================
        s_pilotProfile.status = 0x40000;                // fopAcStts_UNK40000_e
        s_pilotProfile.group = 0;                       // fopAc_ACTOR_e
        s_pilotProfile.cullType = 4;                    // fopAc_CULLBOX_4_e
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"pilot_ready\",\"index\":%d,\"process_size\":%u,"
            "\"base\":\"0x668 measured\",\"status\":\"0x%x\",\"cull\":%d,"
            "\"note\":\"tag_so port: ct+getArg live, drawPrio NOT transferred\"}",
            (int)kPilotIndex, s_pilotProfile.base.base.process_size,
            s_pilotProfile.status, (int)s_pilotProfile.cullType);

        // ====================================================================
        // PORT #1 — the akabe profile, donor values field-for-field
        // (d_a_obj_akabe.cpp profile block): List ID 3, NOCULLEXEC|CULL|
        // UNK40000 status, ACTOR group, CULLBOX_CUSTOM. Draw priority NOT
        // transferred (same positional-enum reasoning as the pilot, §995).
        // ====================================================================
        s_akabeProfile.base.base.layer_id = 0;          // fpcLy_CURRENT_e
        s_akabeProfile.base.base.list_id = 0x0003;      // donor List ID
        s_akabeProfile.base.base.list_priority = 0;     // fpcPi_CURRENT_e
        s_akabeProfile.base.base.name = kAkabeIndex;
        s_akabeProfile.base.base.methods =
            reinterpret_cast<const WwMethodClass*>(s_leafMethod);
        s_akabeProfile.base.base.process_size = kActorBaseSize + sizeof(AkabeMembers);
        s_akabeProfile.base.base.unk_size = 0;
        s_akabeProfile.base.base.parameters = 0;
        s_akabeProfile.base.sub_method = s_actorMethod;
        s_akabeProfile.base.priority = 0;
        s_akabeProfile.sub_method = &s_akabeMethods;
        s_akabeProfile.status = 0x10 | 0x8 | 0x40000;   // NOCULLEXEC|CULL|UNK40000
        s_akabeProfile.group = 0;                       // fopAc_ACTOR_e
        s_akabeProfile.cullType = 0;                    // CULLBOX_CUSTOM = 0

        // ====================================================================
        // PORT — kytag01 (WAVE_INFO). Donor d_a_kytag01.cpp profile:
        // List ID 7, UNK40000, CULLBOX_0. Foam infl only — no wave_make.
        // ====================================================================
        s_kytag01Profile.base.base.layer_id = 0;
        s_kytag01Profile.base.base.list_id = 0x0007;
        s_kytag01Profile.base.base.list_priority = 0;
        s_kytag01Profile.base.base.name = kKytag01Index;
        s_kytag01Profile.base.base.methods =
            reinterpret_cast<const WwMethodClass*>(s_leafMethod);
        s_kytag01Profile.base.base.process_size =
            kActorBaseSize + sizeof(Kytag01Members);
        s_kytag01Profile.base.base.unk_size = 0;
        s_kytag01Profile.base.base.parameters = 0;
        s_kytag01Profile.base.sub_method = s_actorMethod;
        s_kytag01Profile.base.priority = 0;
        s_kytag01Profile.sub_method = &s_kytag01Methods;
        s_kytag01Profile.status = 0x40000;  // fopAcStts_UNK40000_e
        s_kytag01Profile.group = 0;
        s_kytag01Profile.cullType = 0;  // CULLBOX_0
        logf(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"kytag01_ready\",\"index\":%d,"
             "\"process_size\":%u,\"reads\":\"Room44 SCOB ky_tag1 → WAVE_INFO\"}",
             (int)kKytag01Index, s_kytag01Profile.base.base.process_size);

        // ====================================================================
        // PORT — lwood. Donor profile: List 7, NOCULLEXEC|CULL|UNK40000,
        // CULLBOX_CUSTOM. Draw prio NOT transferred (§995).
        // ====================================================================
        s_lwoodProfile.base.base.layer_id = 0;
        s_lwoodProfile.base.base.list_id = 0x0007;
        s_lwoodProfile.base.base.list_priority = 0;
        s_lwoodProfile.base.base.name = kLwoodIndex;
        s_lwoodProfile.base.base.methods =
            reinterpret_cast<const WwMethodClass*>(s_leafMethod);
        s_lwoodProfile.base.base.process_size =
            kActorBaseSize + sizeof(LwoodMembers);
        s_lwoodProfile.base.base.unk_size = 0;
        s_lwoodProfile.base.base.parameters = 0;
        s_lwoodProfile.base.sub_method = s_actorMethod;
        s_lwoodProfile.base.priority = 0;
        s_lwoodProfile.sub_method = &s_lwoodMethods;
        s_lwoodProfile.status = 0x10 | 0x8 | 0x40000;
        s_lwoodProfile.group = 0;
        s_lwoodProfile.cullType = 0;  // CULLBOX_CUSTOM
        logf(LOG_LEVEL_INFO,
             "[WwRegistry] {\"ev\":\"lwood_ready\",\"index\":%d,\"process_size\":%u,"
             "\"bdl\":%d,\"dzb\":%d,\"leaf_sway\":0,"
             "\"reads\":\"R2-R6 applied; sway HELD for Dawn draw bisect\"}",
             (int)kLwoodIndex, s_lwoodProfile.base.base.process_size, kLwoodBdlIndex,
             kLwoodDzbIndex);

        // The akabe callable set — every mangling gate-verified on the user's
        // image this session before the port was written.
        {
            void* a = nullptr;
            struct Bind { const char* sym; void** slot; };
            const Bind kBinds[] = {
                {"?dComIfG_resLoad@@YAHPEAUrequest_of_phase_process_class@@PEBD@Z",
                 reinterpret_cast<void**>(&s_fnResLoad)},
                {"?dComIfG_resDelete@@YAHPEAUrequest_of_phase_process_class@@PEBD@Z",
                 reinterpret_cast<void**>(&s_fnResDelete)},
                // The three conversion steps beyond `load` (d_resorce.cpp).
                // J3DModel matrix/calc callables — retained for future ports
                // (the lwood plugin port that introduced them was reverted to
                // the tree on 2026-08-17; the binds are harmless if unused).
                {"?calc@J3DModel@@UEAAXXZ",
                 reinterpret_cast<void**>(&s_fnModelCalc)},
                {"?setBaseScale@J3DModel@@QEAAXAEBUVec@@@Z",
                 reinterpret_cast<void**>(&s_fnSetBaseScale)},
                {"?newSharedDisplayList@J3DModelData@@QEAAHI@Z",
                 reinterpret_cast<void**>(&s_fnNewSharedDL)},
                {"?simpleCalcMaterial@J3DModelData@@QEAAXQEAY03M@Z",
                 reinterpret_cast<void**>(&s_fnSimpleCalcMat)},
                {"?makeSharedDL@J3DModelData@@QEAAXXZ",
                 reinterpret_cast<void**>(&s_fnMakeSharedDL)},
                {"?j3dDefaultMtx@@3QAY03$$CBMA",
                 reinterpret_cast<void**>(&s_j3dDefaultMtx)},
                {"?dComIfG_getObjectRes@@YAPEAXPEBDH@Z",
                 reinterpret_cast<void**>(&s_fnGetObjRes)},
                {"?fopAcM_entrySolidHeap@@YA_NPEAVfopAc_ac_c@@P6AH0@ZI@Z",
                 reinterpret_cast<void**>(&s_fnEntrySolidHeap)},
                {"??0dBgW@@QEAA@XZ", reinterpret_cast<void**>(&s_fnDBgWCtor)},
                {"?Set@cBgW@@QEAA_NPEAUcBgD_t@@IPEAY123M@Z",
                 reinterpret_cast<void**>(&s_fnBgwSet)},
                {"?Regist@dBgS@@QEAA_NPEAVdBgW_Base@@PEAVfopAc_ac_c@@@Z",
                 reinterpret_cast<void**>(&s_fnBgsRegist)},
                // Corrected after run-time miss receipts (akabe_bind_miss):
                // Release is bool-returning over dBgW_Base. SetCrrFunc is
                // DROPPED — it is header-inline (`{ m_crr_func = func; }`,
                // d_bg_w.h:344) and the donor sets NULL right after
                // construction; our calloc+ctor already leaves the member
                // zero, so the call was a no-op wearing a bind.
                {"?Release@cBgS@@QEAA_NPEAVdBgW_Base@@@Z",
                 reinterpret_cast<void**>(&s_fnBgsRelease)},
                {"?SetPriority@dBgW_Base@@QEAAXW4PRIORITY@1@@Z",
                 reinterpret_cast<void**>(&s_fnSetPriority)},
                {"?fopAcM_SetMtx@@YAXPEAVfopAc_ac_c@@PEAY03M@Z",
                 reinterpret_cast<void**>(&s_fnSetMtx)},
                {"?fopAcM_setCullSizeBox@@YAXPEAVfopAc_ac_c@@MMMMMM@Z",
                 reinterpret_cast<void**>(&s_fnCullBox)},
                {"?fopAcM_isSwitch@@YAHPEBVfopAc_ac_c@@H@Z",
                 reinterpret_cast<void**>(&s_fnIsSwitch)},
                {"?fopAcM_delete@@YAHPEAVfopAc_ac_c@@@Z",
                 reinterpret_cast<void**>(&s_fnActorDelete)},
                {"?transS@mDoMtx_stack_c@@SAXMMM@Z",
                 reinterpret_cast<void**>(&s_fnMtxTransS)},
                {"?ZXYrotM@mDoMtx_stack_c@@SAXFFF@Z",
                 reinterpret_cast<void**>(&s_fnMtxZXYrotM)},
                {"?scaleM@mDoMtx_stack_c@@SAXMMM@Z",
                 reinterpret_cast<void**>(&s_fnMtxScaleM)},
                {"?get@mDoMtx_stack_c@@SAPEAY03MXZ",
                 reinterpret_cast<void**>(&s_fnMtxGet)},
                // lwood draw/create extras (R5/R6 coverage)
                {"?mDoExt_J3DModel__create@@YAPEAVJ3DModel@@PEAVJ3DModelData@@II@Z",
                 reinterpret_cast<void**>(&s_fnJ3DModelCreateC)},
                {"?mDoExt_modelUpdateDL@@YAXPEAVJ3DModel@@@Z",
                 reinterpret_cast<void**>(&s_fnModelUpdateDL)},
                {"?load@J3DAnmLoaderDataBase@@SAPEAVJ3DAnmBase@@PEBXW4J3DAnmLoaderDataBaseFlag@@@Z",
                 reinterpret_cast<void**>(&s_fnJ3DAnmLoad)},
                {"?mDoExt_modelEntryDL@@YAXPEAVJ3DModel@@@Z",
                 reinterpret_cast<void**>(&s_fnModelEntryDL)},
                {"?mDoExt_modelTexturePatch@@YAXPEAVJ3DModelData@@@Z",
                 reinterpret_cast<void**>(&s_fnModelTexturePatch)},
                {"?fopAcM_setCullSizeFar@@YAXPEAVfopAc_ac_c@@M@Z",
                 reinterpret_cast<void**>(&s_fnCullFar)},
                {"?cM_rndF@@YAMM@Z", reinterpret_cast<void**>(&s_fnRndF)},
                // READ from dusklight_imports.lib — QEAA not UEAA (virtual).
                {"?Move@dBgW@@QEAAXXZ", reinterpret_cast<void**>(&s_fnBgwMove)},
            };
            int bound = 0, missing = 0;
            for (const Bind& b : kBinds) {
                a = nullptr;
                if (s_hook->resolve(mod_ctx, b.sym, &a, nullptr) == MOD_OK && a != nullptr) {
                    *b.slot = a; bound++;
                } else {
                    missing++;
                    logf(LOG_LEVEL_ERROR,
                        "[WwRegistry] {\"ev\":\"akabe_bind_miss\",\"sym\":\"%.60s\"}", b.sym);
                }
            }
            // WW-actor support layer binds, resolved beside akabe's set and
            // verified immediately (see wwSupportVerify).
            struct SBind { const char* sym; void** slot; };
            const SBind kSupport[] = {
                {"??0dCcD_Cyl@@QEAA@XZ", reinterpret_cast<void**>(&s_fnCylCtor)},
                {"??0dCcD_Stts@@QEAA@XZ", reinterpret_cast<void**>(&s_fnSttsCtor)},
                {"??0dBgS_ObjAcch@@QEAA@XZ", reinterpret_cast<void**>(&s_fnObjAcchCtor)},
                {"??0dBgS_AcchCir@@QEAA@XZ", reinterpret_cast<void**>(&s_fnAcchCirCtor)},
                {"?Init@dCcD_Stts@@QEAAXHHPEAVfopAc_ac_c@@@Z",
                 reinterpret_cast<void**>(&s_fnSttsInit)},
                {"?Set@dCcD_Cyl@@QEAAXAEBUdCcD_SrcCyl@@@Z",
                 reinterpret_cast<void**>(&s_fnCylSet)},
                {"?SetWall@dBgS_AcchCir@@QEAAXMM@Z",
                 reinterpret_cast<void**>(&s_fnAcchSetWall)},
            };
            int sBound = 0;
            for (const SBind& sb : kSupport) {
                void* sa = nullptr;
                if (s_hook->resolve(mod_ctx, sb.sym, &sa, nullptr) == MOD_OK && sa != nullptr) {
                    *sb.slot = sa; sBound++;
                }
            }
            logf(LOG_LEVEL_INFO,
                "[WwRegistry] {\"ev\":\"support_bind\",\"bound\":%d,\"of\":7,"
                "\"reads\":\"WW-actor support layer callables; the offset half is "
                "verified separately by support_verify\"}", sBound);
            wwSupportVerify();
            logf(missing == 0 ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
                "[WwRegistry] {\"ev\":\"akabe_ready\",\"index\":%d,\"bound\":%d,"
                "\"missing\":%d,\"reads\":\"port #1 of queue 146; a missing bind "
                "degrades that call to a guarded no-op, never a crash\"}",
                (int)kAkabeIndex, bound, missing);
        }

        // PORT — donor vrbox / vrbox2. List ID 7, UNK4000|UNK40000, CULLBOX_0.
        // Draw prio is the RECEIVER's fpcDwPi_VRBOX_e / VRBOX2_e (7 / 4), which
        // happen to match the donor's early-enum slots — these are TP's sky
        // buckets, not a WW number copied into a TP-ordered queue.
        s_vrboxProfile.base.base.layer_id = 0;
        s_vrboxProfile.base.base.list_id = 0x0007;
        s_vrboxProfile.base.base.list_priority = 0;
        s_vrboxProfile.base.base.name = kVrboxIndex;
        s_vrboxProfile.base.base.methods =
            reinterpret_cast<const WwMethodClass*>(s_leafMethod);
        s_vrboxProfile.base.base.process_size = kActorBaseSize + sizeof(void*);
        s_vrboxProfile.base.base.unk_size = 0;
        s_vrboxProfile.base.base.parameters = 0;
        s_vrboxProfile.base.sub_method = s_actorMethod;
        s_vrboxProfile.base.priority = 7;  // fpcDwPi_VRBOX_e
        s_vrboxProfile.sub_method = &s_vrboxMethods;
        s_vrboxProfile.status = 0x4000 | 0x40000;
        s_vrboxProfile.group = 0;
        s_vrboxProfile.cullType = 0;  // CULLBOX_0

        s_vrbox2Profile.base.base.layer_id = 0;
        s_vrbox2Profile.base.base.list_id = 0x0007;
        s_vrbox2Profile.base.base.list_priority = 0;
        s_vrbox2Profile.base.base.name = kVrbox2Index;
        s_vrbox2Profile.base.base.methods =
            reinterpret_cast<const WwMethodClass*>(s_leafMethod);
        s_vrbox2Profile.base.base.process_size = kActorBaseSize + sizeof(void*) * 3;
        s_vrbox2Profile.base.base.unk_size = 0;
        s_vrbox2Profile.base.base.parameters = 0;
        s_vrbox2Profile.base.sub_method = s_actorMethod;
        s_vrbox2Profile.base.priority = 4;  // fpcDwPi_VRBOX2_e
        s_vrbox2Profile.sub_method = &s_vrbox2Methods;
        s_vrbox2Profile.status = 0x4000 | 0x40000;
        s_vrbox2Profile.group = 0;
        s_vrbox2Profile.cullType = 0;

        wwVrbox_bind(s_hook);
        dExtWwCam_installData();
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"vrbox_ready\",\"index\":[%d,%d],\"prio\":[7,4]}",
            (int)kVrboxIndex, (int)kVrbox2Index);
    }

    logf((rLeaf == MOD_OK && rActor == MOD_OK) ? LOG_LEVEL_INFO : LOG_LEVEL_WARN,
        "[WwRegistry] {\"ev\":\"actor_prereq\",\"g_fpcLf_Method\":\"%s\",\"addr\":\"%p\","
        "\"g_fopAc_Method\":\"%s\",\"addr2\":\"%p\","
        "\"note\":\"both DATA symbols; required before any actor row\"}",
        result_name(rLeaf), leafAddr, result_name(rActor), actorAddr);

    // MOD_UNSUPPORTED here means the host carries no embedded symbol manifest,
    // which kills by-name resolution for EVERY plugin — a build-configuration
    // fault, not a missing symbol. Kept distinct from MOD_UNAVAILABLE so the
    // two are never reported as one line.
    logf(LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"install\",\"fpcPf_Get\":\"%s\",\"cDyl_IsLinked\":\"%s\","
        "\"dStage_searchName\":\"%s\",\"base\":%d,\"rows\":%d,\"names\":%d}",
        result_name(rPf), result_name(rDyl), result_name(rName),
        (int)kWwProfileBase, kRowCount, kObjectNameCount);

    // ------------------------------------------------------------------------
    // DISPATCH SELF-TEST. Calls through the TARGET address (which now carries
    // the trampoline), NOT through g_orig — calling the original would bypass
    // the detour and prove nothing. A pass means the full path ran: trampoline
    // -> dispatch_pre -> our callback -> HOOK_SKIP_ORIGINAL -> our pointer.
    // ------------------------------------------------------------------------
    if (rPf == MOD_OK && PfGet::target != nullptr) {
        typedef void* (*PfGetFn)(short);
        const PfGetFn through_detour = reinterpret_cast<PfGetFn>(PfGet::target);
        const void* got = through_detour(kSelfTestIndex);
        const bool pass = (got == static_cast<const void*>(&kSelfTestSentinel));
        logf(pass ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"selftest\",\"index\":%d,\"dispatch\":\"%s\","
            "\"expect\":\"sentinel returned via detour\"}",
            (int)kSelfTestIndex, pass ? "PASS" : "FAIL");
    } else {
        logf(LOG_LEVEL_WARN,
            "[WwRegistry] {\"ev\":\"selftest\",\"dispatch\":\"SKIPPED\","
            "\"why\":\"hook not installed\"}");
    }

    // ========================================================================
    // PLACEMENT-ROUTE SELF-TEST — REPAIRED 2026-08-15 (tale §1005), by user
    // ruling, IN FRONT OF b2 rather than alongside it.
    //
    // BOTH HALVES OF THE PREVIOUS VERSION WERE WRONG, and the one that was
    // wrong in the more dangerous direction is the one that reported PASS:
    //   · answer_ours compared the answer against `&kObjectNames[0]`, but the
    //     hook returns the row it MATCHED and "WwPilot" is the THIRD row. It
    //     failed by construction — a perfectly working hook failed it.
    //   · fallthrough_theirs was `theirs != &kObjectNames[0]`, which passes for
    //     any answer that is not row 0, INCLUDING a wrong row-2 answer that had
    //     shadowed the receiver's entire object table. It could not fail
    //     informatively, so its PASS carried no information.
    //
    // Net effect: the name -> row route — the ONLY route by which a placement
    // reaches the pilot — was untested in both directions while appearing to be
    // half-tested. b2 is creation, so b2 would have been built on top of it.
    //
    // THE REPAIR, and why each piece is shaped this way:
    //   · the expected row is resolved BY NAME, not by a literal index, so
    //     reordering `kObjectNames` cannot silently re-break this test;
    //   · fall-through is tested against the WHOLE table via `is_our_row`;
    //   · `s_nameHits` is SAMPLED ACROSS BOTH CALLS, so a green line proves the
    //     hook BODY ran and that the receiver's name did NOT enter it. A
    //     pointer comparison alone cannot distinguish "our hook answered" from
    //     "something else happened to return that address".
    // ========================================================================
    if (rName == MOD_OK && StageSearchName::target != nullptr) {
        typedef void* (*SearchFn)(const char*);
        const SearchFn through_detour = reinterpret_cast<SearchFn>(StageSearchName::target);

        const WwObjectNameInf* expected = nullptr;
        for (int i = 0; i < kObjectNameCount; i++) {
            if (std::strncmp(kObjectNames[i].name, "WwPilot",
                             sizeof(kObjectNames[i].name)) == 0) {
                expected = &kObjectNames[i];
                break;
            }
        }

        const int hitsBefore = s_nameHits;
        const void* mine = through_detour("WwPilot");
        const int hitsAfterOurs = s_nameHits;
        const bool hit = (expected != nullptr
                          && mine == static_cast<const void*>(expected)
                          && hitsAfterOurs == hitsBefore + 1);

        // A name the receiver owns: the answer must land OUTSIDE our whole
        // table, and our hook must not have counted a hit for it.
        const void* theirs = through_detour("Link");
        const bool fellThrough = (!is_our_row(theirs) && s_nameHits == hitsAfterOurs);

        logf((hit && fellThrough) ? LOG_LEVEL_INFO : LOG_LEVEL_ERROR,
            "[WwRegistry] {\"ev\":\"selftest_name\",\"answer_ours\":\"%s\","
            "\"fallthrough_theirs\":\"%s\",\"expect_row\":%d,\"name_hits\":%d}",
            hit ? "PASS" : "FAIL", fellThrough ? "PASS" : "FAIL",
            expected != nullptr ? (int)(expected - &kObjectNames[0]) : -1,
            s_nameHits);
    } else {
        logf(LOG_LEVEL_WARN,
            "[WwRegistry] {\"ev\":\"selftest_name\",\"route\":\"SKIPPED\","
            "\"why\":\"name hook not installed\"}");
    }

    logf(LOG_LEVEL_INFO,
        "[WwRegistry] registry carries the tag_so pilot at index 4097 plus an unreachable "
        "self-test sentinel. The game reaches the pilot only via a TagSo/TagMSo placement.");
    return MOD_OK;
}

// ============================================================================
// SHUTDOWN RECEIPT — `name_hits` ADDED 2026-08-15 (tale §1005).
//
// `s_nameHits` was declared and incremented and NEVER PRINTED, so the receipt
// carried the two counters that were not in question and omitted the one that
// was. With the self-test repaired but no placement yet made, this counter is
// the only thing that will distinguish "a real TagSo/TagMSo placement reached
// our row" from "init ran the self-test and nothing else ever asked". Expect
// exactly 1 on a boot with no placement — the self-test's own "WwPilot" call.
// Anything above 1 on a b2 run is the donor's placement data arriving.
// ============================================================================
// ============================================================================
// ROUTE INTAKE — called by main.cpp's serve pass, once per mounted (stage,
// room). See registry.h for the contract; the short version is that the disc
// decides the menu, and static storage keeps the SELECT's labels alive.
// ============================================================================
void wwRegistry_clearRoutes() {
    s_routeCount = 0;
    s_routesSaturated = false;
    s_destIdx = 0;
}

// MOVED OUT OF THE ANONYMOUS NAMESPACE 2026-08-16. Defined beside the route
// store, these had INTERNAL LINKAGE and `main.cpp` could not link against
// them (LNK2019 on setShowFileNames). They belong here with the rest of the
// public intake surface; the statics they touch stay file-local, which is
// the point of the split.
void wwRegistry_setDiagProbes(bool on) { s_diagProbes = on; }

void wwRegistry_setShowFileNames(bool on) {
    if (s_showFileNames == on) { return; }
    s_showFileNames = on;
    for (int i = 0; i < s_routeCount; i++) { composeRouteLabel(i); }
    logf(LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"warp_filenames\",\"on\":%d,\"relabelled\":%d}",
        on ? 1 : 0, s_routeCount);
}

// ============================================================================
// THE GROUPING PASS (user assignment 2026-08-16, relayed via the Integrator).
//
// DEFINED HERE, BELOW THE ANONYMOUS NAMESPACE, DELIBERATELY. Everything
// declared in registry.h must be defined at global scope: an extern
// declaration over an anon-namespace definition compiles clean and fails at
// LINK, and `cl /Zs` cannot see it. That trap cost a build this morning and
// this file is where it happened.
//
// WHAT IT COMPOSES — three donor-derived inputs, none typed:
//   TYPE      `dStageType_*`, STAG +0x0C, `(mStageTypeAndSchbit >> 16) & 7`
//   PARENT    the `SCLS` exit table of each sea room
//   NAME      already set via wwRegistry_setRouteName()
//
// THE ORDER IT PRODUCES, and it is chosen to answer one question fast —
// "which row is Outset?":
//   islands ascending by ROOM NUMBER (the donor's own island order), each
//   immediately followed by its own interiors; then everything with no known
//   island, bucketed by donor stage type; then anything still unknown.
// A flat 81-row list is what cost the user a run; this makes the island a
// visible header with its houses beneath it.
// ============================================================================

// Session hand-over from main.cpp's disc read: the raw bmgres.arc bytes the
// vanilla msg-group seam mem-mounts per scene. The buffer is main.cpp's static
// vector — session lifetime by construction; the receiver never frees it
// (break-flag 0 in the mount factory).
void wwRegistry_setBmgArcBytes(const void* buf, unsigned int size) {
    s_bmgArcBuf = static_cast<const unsigned char*>(buf);
    s_bmgArcSize = size;
}

void wwRegistry_setStageType(const char* stage, int type) {
    if (stage == nullptr || type < 0 || type > 7) { return; }
    WwStageMeta* m = stageMetaFor(stage, true);
    if (m != nullptr) { m->type = (signed char)type; }
}

void wwRegistry_setStageParent(const char* stage, int parentSeaRoom) {
    if (stage == nullptr || parentSeaRoom < 1 || parentSeaRoom > 49) { return; }
    WwStageMeta* m = stageMetaFor(stage, true);
    if (m == nullptr) { return; }
    // ------------------------------------------------------------------
    // FIRST PARENT WINS — except that plain first-wins was WRONG here, and
    // only for `LinkRM`.
    //
    // `LinkRM` is named by TWO islands' `SCLS`: Forest Haven (room 41) and
    // Outset (room 44). Every other stage in the corpus is named by exactly
    // one. Rooms are parsed in ascending order, so 41 arrives first and plain
    // first-wins would file **Link's House under Forest Haven** — visibly
    // wrong, and wrong in the one place a player looks first.
    //
    // OUTSET WINS, and this is an AUTHORED tie-break, not a derived one: the
    // donor's data says "both" and cannot say which is home. Labelled as
    // authored under the user's "always authored names" ruling, which covers
    // exactly this case — where the donor is ambiguous rather than silent.
    // Recorded here rather than in a doc because this line is where a future
    // reader will ask why 44 beat 41.
    // ------------------------------------------------------------------
    const bool isLinkRM = (std::strcmp(stage, "LinkRM") == 0);
    if (isLinkRM && parentSeaRoom == 44) {
        m->parentRoom = 44;          // authored: Link's House is on Outset
        return;
    }
    if (isLinkRM && m->parentRoom == 44) {
        return;                      // never let 41 overwrite the decision
    }
    if (m->parentRoom < 0) { m->parentRoom = (signed char)parentSeaRoom; }
}

// ============================================================================
// AUTHORED STORY ORDER (user ruling 2026-08-16, second warp test): islands
// sort by the game's event sequence, not by sea-grid room number — "Outset
// Island, Link House, Orca's House ... Forsaken Fortress, [its] interiors".
// The ranks are AUTHORED CONTENT under that ruling — the donor's data cannot
// say what order the story visits its islands. Rooms 44 (Outset) and 41
// (Forest Haven) are session-measured; the spine between them is the game's
// canonical progression. An island absent from this table follows the story
// set in ascending room order (rank 100+room). Sea room 0 ("You have no
// maps.", not a destination) sorts dead last rather than being index-excluded
// — the flat select's indices must stay aligned with s_routes.
// ============================================================================
static int wwStoryRank(int seaRoom) {
    switch (seaRoom) {
    case 44: return 0;       // Outset Island (measured: msg 3295+44)
    case 1:  return 1;       // Forsaken Fortress
    case 11: return 2;       // Windfall Island
    case 13: return 3;       // Dragon Roost Island
    case 41: return 4;       // Forest Haven (measured: the LinkRM tie-break)
    case 23: return 5;       // Greatfish Isle
    case 26: return 6;       // Tower of the Gods
    case 0:  return 100000;  // "You have no maps." — last, never excluded
    default: return 100 + seaRoom;
    }
}

void wwRegistry_finalizeRoutes() {
    if (s_routeCount <= 0) { return; }

    // ---- 1. resolve each route's group + child flag from the stage meta ----
    int grouped = 0, ungrouped = 0;
    for (int i = 0; i < s_routeCount; i++) {
        const WwStageMeta* m = stageMetaFor(s_routeStages[i], false);
        s_routeType[i] = (m != nullptr) ? m->type : (signed char)-1;
        if (std::strcmp(s_routeStages[i], "sea") == 0) {
            // A sea room heads its own group. Room 0 is NOT an island (msg
            // 3295+0 is the UI line "You have no maps."), so it groups as
            // itself but is never treated as a parent.
            s_routeGroup[i] = s_routes[i].room;
            s_routeIsChild[i] = false;
            grouped++;
        } else if (m != nullptr && m->parentRoom >= 1) {
            s_routeGroup[i] = m->parentRoom;
            s_routeIsChild[i] = true;
            grouped++;
        } else {
            s_routeGroup[i] = -1;      // UNKNOWN - sorts last, never folded into 0
            s_routeIsChild[i] = true;
            ungrouped++;
        }
    }

    // ---- 2. selection sort into display order (kMaxRoutes is 192; an O(n^2)
    // pass over <=192 rows runs once at attach and needs no allocation) ----
    for (int a = 0; a < s_routeCount - 1; a++) {
        int best = a;
        for (int b = a + 1; b < s_routeCount; b++) {
            // ordering key, most significant first:
            //   known group before unknown · group STORY RANK (authored, user
            //   ruling — table above; unranked fall back to room ascending) ·
            //   head before children · room number · stage name
            const bool bUnk = s_routeGroup[b] < 0, cUnk = s_routeGroup[best] < 0;
            if (bUnk != cUnk) { if (!bUnk) { best = b; } continue; }
            if (!bUnk && s_routeGroup[b] != s_routeGroup[best]) {
                // Story rank, not room number (user ruling; table above).
                // Ranks are injective over distinct rooms, so strict < works.
                if (wwStoryRank(s_routeGroup[b]) < wwStoryRank(s_routeGroup[best])) { best = b; }
                continue;
            }
            if (bUnk && s_routeType[b] != s_routeType[best]) {
                if (s_routeType[b] < s_routeType[best]) { best = b; }
                continue;
            }
            if (s_routeIsChild[b] != s_routeIsChild[best]) {
                if (!s_routeIsChild[b]) { best = b; }
                continue;
            }
            const int cmp = std::strcmp(s_routeStages[b], s_routeStages[best]);
            if (cmp != 0) { if (cmp < 0) { best = b; } continue; }
            if (s_routes[b].room < s_routes[best].room) { best = b; }
        }
        if (best == a) { continue; }
        // Swap EVERY parallel array together. `s_routes[].label`/`.stage` point
        // INTO these buffers, so they are re-pointed after the swap rather than
        // carried across it — a swapped pointer would alias another row's text.
        char tl[40], ts[20], tn[32];
        std::memcpy(tl, s_routeLabels[a], sizeof(tl));
        std::memcpy(ts, s_routeStages[a], sizeof(ts));
        std::memcpy(tn, s_routeNames[a], sizeof(tn));
        const signed char trm = s_routes[a].room, tg = s_routeGroup[a], tt = s_routeType[a];
        const bool tc = s_routeIsChild[a];
        std::memcpy(s_routeLabels[a], s_routeLabels[best], sizeof(tl));
        std::memcpy(s_routeStages[a], s_routeStages[best], sizeof(ts));
        std::memcpy(s_routeNames[a], s_routeNames[best], sizeof(tn));
        s_routes[a].room = s_routes[best].room;
        s_routeGroup[a] = s_routeGroup[best];
        s_routeType[a] = s_routeType[best];
        s_routeIsChild[a] = s_routeIsChild[best];
        std::memcpy(s_routeLabels[best], tl, sizeof(tl));
        std::memcpy(s_routeStages[best], ts, sizeof(ts));
        std::memcpy(s_routeNames[best], tn, sizeof(tn));
        s_routes[best].room = trm;
        s_routeGroup[best] = tg;
        s_routeType[best] = tt;
        s_routeIsChild[best] = tc;
    }

    // ---- 3. re-point and relabel. INDENT RETIRED (row 961): hierarchy now
    // lives in per-island SECTIONS in the tab builder, so labels stay clean —
    // "  - " prefixes in a select were text pretending to be structure. ----
    // The list now opens on the story-first route (Outset), so the point
    // default must track the initial selection too, not only setDest changes.
    s_point = wwDefaultPointFor(s_destIdx);
    for (int i = 0; i < s_routeCount; i++) {
        s_routes[i].label = s_routeLabels[i];
        s_routes[i].stage = s_routeStages[i];
        composeRouteLabel(i);
        s_routeOptions[i] = s_routeLabels[i];
    }

    // ---- 4. group table (row 961, STATIC branch): contiguous slices. A new
    // group starts at every HEAD row; the unknown bucket (group < 0, sorted
    // last) collapses into one trailing "(unplaced)" group. ----
    s_groupN = 0;
    for (int i = 0; i < s_routeCount; i++) {
        const bool startsGroup =
            (i == 0) || (!s_routeIsChild[i]) ||
            (s_routeGroup[i] < 0 && s_routeGroup[i - 1] >= 0);
        if (startsGroup && s_groupN < 64) {
            s_groupStart[s_groupN] = i;
            s_groupCount[s_groupN] = 0;
            s_groupN++;
        }
        if (s_groupN > 0) { s_groupCount[s_groupN - 1]++; }
    }

    // Initial selection's point default too — a warp pressed without ever
    // touching the destination select must not sail with a stale point 0.
    s_point = wwDefaultPointFor(s_destIdx);

    logf(LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"warp_grouped\",\"routes\":%d,\"grouped\":%d,"
        "\"ungrouped\":%d,\"stage_meta\":%d,\"meta_saturated\":%d,"
        "\"first\":\"%s\",\"read_me\":\"islands in AUTHORED STORY ORDER (user "
        "ruling 2026-08-16; Outset first), each followed by its own interiors; "
        "unranked islands ascend by room; ungrouped sort last by stage type. "
        "ungrouped > 0 is NOT an error - it means those stages are not named by "
        "any sea room's SCLS\"}",
        s_routeCount, grouped, ungrouped, s_stageMetaCount,
        s_stageMetaSaturated ? 1 : 0, s_routeOptions[0]);
}

// Establish a route's canon name. REFUSES an empty name rather than storing a
// blank that would render as a nameless row — an empty label is the silent
// failure this menu exists to remove.
void wwRegistry_setRouteName(const char* stage, int room, const char* name) {
    if (stage == nullptr || name == nullptr || name[0] == 0) { return; }
    for (int i = 0; i < s_routeCount; i++) {
        if (s_routes[i].room == room &&
            std::strcmp(s_routeStages[i], stage) == 0) {
            std::snprintf(s_routeNames[i], sizeof(s_routeNames[i]), "%s", name);
            composeRouteLabel(i);
            return;
        }
    }
}

void wwRegistry_addRoute(const char* stage, int room) {
    if (stage == nullptr || room < 0 || room > 255) {
        return;
    }
    if (s_routeCount >= kMaxRoutes) {
        // ONCE, not per drop: a saturated list must announce itself, but a
        // line per rejected route would bury the announcement it is making.
        if (!s_routesSaturated) {
            s_routesSaturated = true;
            logf(LOG_LEVEL_ERROR,
                "[WwRegistry] {\"ev\":\"routes_saturated\",\"cap\":%d,"
                "\"why\":\"more mounted rooms than the menu can hold; "
                "destinations are being DROPPED, not merely unshown\"}",
                kMaxRoutes);
        }
        return;
    }
    const int i = s_routeCount;
    // Donor-native stage name verbatim; the room number is the disc's own.
    // This is the FALLBACK label and it is still the right default: a route
    // with no established name shows its token rather than a guess.
    // `wwRegistry_setRouteName()` upgrades it once a name is KNOWN — island
    // names come from the donor's own table, interior names are authored on
    // the user's explicit ruling ("always authored names") against actor
    // identities read out of the donor's source. Neither is typed from memory.
    std::snprintf(s_routeLabels[i], sizeof(s_routeLabels[i]), "%s  room %d", stage, room);
    s_routeNames[i][0] = 0;   // no display name until one is established
    // COPY the stage name, never alias it. main.cpp builds its stage list with
    // `split_csv()` into a `std::vector<std::string>` LOCAL to `rebuild()`;
    // holding that `c_str()` would dangle the moment rebuild returns, and the
    // symptom would be a warp to a garbage stage name long after the fact.
    std::strncpy(s_routeStages[i], stage, sizeof(s_routeStages[i]) - 1);
    s_routeStages[i][sizeof(s_routeStages[i]) - 1] = 0;
    s_routes[i].label = s_routeLabels[i];
    s_routes[i].stage = s_routeStages[i];
    s_routes[i].room = (signed char)room;
    s_routeOptions[i] = s_routeLabels[i];
    s_routeCount++;
}

// ============================================================================
// ROUTE NAMING - PUBLIC. These two were authored inside the anonymous
// namespace while `registry.h` declared them EXTERNALLY, which compiles
// clean and then fails at LINK - `/Zs` cannot see it. Lifted to global
// scope so the definitions match the declarations. `composeRouteLabel` stays
// internal and is still reachable: anonymous-namespace names are visible to
// the rest of the TU.
//
// A SECOND DEFINITION OF `setRouteName` (mine) WAS REMOVED HERE. It sat at
// global scope and hardcoded `Name [stage room]`, so it SHADOWED this one
// and silently ignored the file-name toggle. Two lanes, one file, same
// hazard as the D1 manifest walker - this version is kept because it is the
// one that honours the toggle.
// ============================================================================

ModResult wwRegistry_shutdown() {
    // Teardown removes the tab anyway (svc/ui.h: "removed when unregistered or
    // the mod is torn down"), but unregistering explicitly keeps the probe
    // symmetrical — and if the handle is ever reused for real UI, the release
    // path is already where it belongs.
    if (s_ui != nullptr && s_wwMenuTab != 0) {
        s_ui->unregister_menu_tab(mod_ctx, s_wwMenuTab);
        s_wwMenuTab = 0;
    }

    // b2 AUTOFIRE FIELDS REMOVED 2026-08-15 with the autofire path itself
    // (user ruling 1009). What they measured is recorded at the retirement
    // note above the hook block rather than carried as dead counters here:
    // `changereq_calls: 114` in one session, which is why a stage-change
    // boundary is a poor proxy for "the player arrived somewhere".
    //
    // `name_hits` STAYS and is now the only b2-relevant number on this line.
    // Expect exactly 1 with no placement — the self-test's own "WwPilot"
    // call. Anything above 1 is donor placement data reaching our row, and
    // that remains true whether the stage was entered by a warp-menu
    // selection, Fado's door, or anything else.
    // ------------------------------------------------------------
    // PER-CLASS DELETE CENSUS, emitted BEFORE the shutdown line so a
    // truncated tail cannot eat it. One row per class; a class that never
    // appears was never deleted, which is the whole question for GRASS_e.
    // ------------------------------------------------------------
    for (int i = 0; i < s_delClasses; i++) {
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"del_by_class\",\"profname\":%d,\"deleted\":%d,"
            "\"first_at\":%d,\"last_at\":%d,"
            "\"classes\":%d,\"overflow\":%d,\"reads\":\"UNCAPPED count of post-warp "
            "fpcBs_Delete calls for this profname. Cross-ref the number against the "
            "loader's own 'profname=fpcNm_X (N)' lines. A class ABSENT from this list "
            "was NEVER deleted - unlike the capped bs_delete receipts, which only show "
            "the first ten by TIME and close before late-created classes exist.\"}",
            (int)s_delName[i], s_delCount[i], s_delFirstAt[i], s_delLastAt[i],
            s_delClasses, s_delOverflow);
    }

    logf(LOG_LEVEL_INFO,
        "[WwRegistry] {\"ev\":\"shutdown\",\"bs_del_post\":%d,\"bs_del_refused\":%d,\"m_del_post\":%d,\"pf_hits\":%d,\"dyl_hits\":%d,"
        "\"name_hits\":%d,\"name_hits_from_selftest\":1,"
        "\"startstage_now\":\"%s\",\"stage_changes\":%d,"
        "\"search_calls_total\":%d,"
        "\"set_stage_res_calls\":%d,\"stage_loader_calls\":%d,"
        "\"mult_donor\":%d,\"mult_receiver\":%d,"
        // `res_calls`/`res_misses` REMOVED with the getRes probe's retirement
        // (§1022). An uninstalled hook's counters print 0, and a 0 that means
        // "not measured" is indistinguishable from a 0 that means "none" —
        // which is exactly what `set_stage_res_calls: 0` did to a whole boot
        // when its hook failed to bind. Better absent than falsely quiet.
        "\"glb_res_calls\":%d,\"msggroup_skipped\":%d,"
        "\"read_me\":\"startstage_now = FRESH read of the receiver's own start "
        "stage. Chain: set_stage_res_calls>0 = path was BUILT and requested; "
        "stage_loader_calls>0 = stage data was PARSED. both 0 after a warp = "
        "the load phase never ran and the fault is scene phase order, not the "
        "name - which is already proven correct\"}",
        // H11 aggregate totals FIRST - the format string leads with them,
        // and the order comment below is there because order was wrong once.
        s_bsDelPost, s_bsDelRefused, s_mDelPost,
        s_pfHits, s_dylHits, s_nameHits,
        // FRESH READ, not the cache. The previous receipt printed
        // `s_lastSeenStage`, which is only updated when the sampler runs — so
        // on a run where the sampler went quiet it reported a STALE VALUE
        // dressed as a reading (§1016 caught this; `F_SP103` in run 030942 was
        // exactly that). Calling the getter here asks the receiver directly at
        // shutdown, so this field is always current or honestly "(unreadable)".
        (s_fnGetStartStageName != nullptr && s_fnGetStartStageName() != nullptr)
            ? s_fnGetStartStageName() : "(unreadable)",
        // ORDER MATTERS AND IT WAS WRONG ONCE: the format runs
        // search_calls_total -> set_stage_res_calls -> stage_loader_calls, and
        // these three were supplied in a different order, which would have
        // printed the search count under the setStageRes field. Caught by
        // reading the receipt back against its own format string rather than
        // by the compiler, which cannot check a printf argument it is handed
        // as a matching type.
        s_stageReads, s_searchCallsTotal, s_setStageResCalls, s_loaderCalls,
        s_multWw, s_multTp, s_glbCalls, s_msgGroupSkipped);

    // ------------------------------------------------------------------
    // PER-CLASS DELETE TABLE (Integrator, 2026-08-21). History/Bridge
    // authored the counting; the table was filled and NEVER PRINTED - the
    // same counters-die-unread defect as the totals this afternoon, one
    // iteration later. One line, profname:count pairs; profname is the
    // numeric enum value - census_join maps it to a name downstream.
    // ------------------------------------------------------------------
    {
        char dcbuf[1024]; int off = 0;
        for (int i = 0; i < s_delClasses && off < (int)sizeof(dcbuf) - 24; i++) {
            off += std::snprintf(dcbuf + off, sizeof(dcbuf) - off, "%s\"%d\":%d",
                                 (i ? "," : ""), (int)s_delName[i], s_delCount[i]);
        }
        logf(LOG_LEVEL_INFO,
            "[WwRegistry] {\"ev\":\"del_classes\",\"classes\":%d,\"overflow\":%d,"
            "\"counts\":{%s},\"reads\":\"post-warp fpcBs_Delete by profname enum; a class "
            "ABSENT here was NEVER deleted - that is a fact about the class, not a window\"}",
            s_delClasses, s_delOverflow, dcbuf);
    }
    return MOD_OK;
}
