#ifndef D_EXT_SAVE_GUARD_H
#define D_EXT_SAVE_GUARD_H

// №81 EXTENSION-FIRST: refuse native stage-progression writes while WW content is
// live on a vanilla host (today: F_SP115 → Lanayru bits). Thin header so call
// sites in d_com_inf_game / d_save do not pull the full mount type graph.
// №106: filename neutral (was d_ext_ww_save_guard.h) — avoid ww in shipping paths.

#if TARGET_PC

// True if stage name is a Dusklight WW host (R_DL* interiors / F_DL* fields).
bool dExtWwSave_isWwHostStage(const char* stageName);
// §632: declare a VANILLA-NAMED donor stage as a WW host at runtime, from mod
// data (<mod>/ww_stages.ini). Lets /res/Stage/sea/Room44.arc be reached under
// its own name without the exe carrying a WW place-name (History A1).
void dExtWwSave_registerWwStage(const char* stageName);
// §637: NARROWER than isWwHostStage — true only for stages DECLARED in mod
// data, i.e. those whose arcs are staged byte-identical under the DONOR's own
// filenames. The neutral R_DL*/F_DL* fork stages are repackaged under the
// receiver's names and must NOT be aliased. Use this for on-disk LAYOUT
// questions; use isWwHostStage for donor DATA semantics.
bool dExtWwSave_isDeclaredWwStage(const char* stageName);
// §47: give the player the donor cast's LIGHTING RECIPE inside donor spaces.
// Returns true when it handled the lighting, in which case the caller must SKIP
// setLightTevColorType_MAJI — bypassing MAJI is the point, not a side effect.
// Always false outside donor stages and at the default setting, so receiver
// areas take an early-out and are untouched.
class dKy_tevstr_c;
class J3DModel;
bool dExtWw_applyPlayerDonorLook(J3DModel* model, dKy_tevstr_c* tevStr);

// №165: storyboard message IDs. A donor storyboard's JMSG track fires DONOR
// message indices (awake.stb: 0x357, 0x358, 0x050, 0x359, 0x35A...). Those
// indices are meaningless in the receiver's message table, where they land on
// unrelated shipped strings — which is why the opening showed an Ordon pumpkin
// line and a quest-log string rather than dialogue.
//
// Returns TRUE when the caller must SUPPRESS the native display: showing the
// receiver's string for a donor index is always wrong, so a missing line is
// preferable to a foreign one (№31 applied to text).
bool dExtWw_handleDemoMessage(u32 donorMsgId);

// §256 native WW-actor talk-text injection (empty-box fix). Returns true if it
// resolved + injected a WW catalog line for i_talkActor's i_msgIdx (caller then
// targets kWwCodeTextIndex). See d_ext_npc_mount.cpp.
bool dExtWw_injectTalkText(fopAc_ac_c* i_talkActor, u32 i_msgIdx);

// §226: true when this proc is one of OUR WW-restored actors (derived from the
// loaded provider manifests, plus the legacy proc-range as a safety net).
// Base rule: every WW NPC takes the native WW box; per-actor exceptions (signs
// on the parchment style, etc.) are declared inside this function.
bool dExtWw_isWwTalkProc(s16 i_procName);

// §324: CONTINUE-chained talk message (actor-less 2-arg fopMsgM_messageSet) —
// re-opens the native WW dMesg box for the next message in a conversation.
// Returns true if taken (caller then targets the code-text entry 4900).
bool dExtWw_injectTalkChain(u32 i_msgIdx);

// §195: true on a WW host stage — used by dDemo_c::update to gate the storyboard
// advance on the message-box suspend (restore the vanilla step-in-step freeze).
bool dExtWw_isWwDemoStage();

// §49 rendering. The storyboard is TIMED, so the box never gates the demo:
// a new line replaces the old, A dismisses early but is never required, and
// the line self-clears when the scene ends.
void dExtWw_pollDemoMessage();   // per-frame (execute)
void dExtWw_drawDemoMessage();   // per-frame (DRAW pass only — set2DOpa)

/**
 * №170: defer №110 arrival snap while F_DL01 still intends to run `awake`
 * (pending from host entry until the opening ends or is abandoned).
 */
bool dExtWw_openingOwnsCamera();

/**
 * №170: pause arrival G-guard / refuse force-end only AFTER `awake` has
 * ordered. Arming this earlier blocked residual clear and left a permanent
 * busy slot (№169's G-guard never got to free the lane).
 */
bool dExtWw_openingPauseArrivalGuard();

/**
 * §281: true if the running event is the tale storyboard under ANY name
 * (TALE_DEMO/TALE_DEMO2 or ba1's native tale_1/tale_2). Defined in d_ext_npc_mount.cpp;
 * declared here too so the camera-drive gate (d_camera.cpp) can see it.
 */
bool dExtWw_isTaleRunEvent(const char* runEvt);

/**
 * №171: №110 snap defer — hold after ORDER, or still trying to order.
 * Prefer this over openingOwnsCamera for arrival snap.
 */
bool dExtWw_deferArrivalCameraSnap();

// True if a WW BG / room-lane interior is currently active.
bool dExtWwSave_isWwContentActive();

// Returns true when the write must be refused (caller should no-op).
// stageNo may be -1 when the write targets the current stage memory.
bool dExtWwSave_refuseNativeWrite(const char* api, int stageNo, int bit);

#endif  // TARGET_PC

#endif /* D_EXT_SAVE_GUARD_H */
