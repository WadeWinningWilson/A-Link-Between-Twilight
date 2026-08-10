#ifndef D_EXT_NPC_MOUNT_H
#define D_EXT_NPC_MOUNT_H

#if TARGET_PC

#include "f_op/f_op_actor_mng.h"
#include "m_Do/m_Do_ext.h"
#include "SSystem/SComponent/c_phase.h"
#include "d/d_bg_w.h"
#include "d/d_cc_d.h"

// Generic external-NPC / BG mount. Stub sockets delegate here when a mod
// provides a payload (manifest + arc). Character identity lives in manifests only.

static constexpr int kExtNpcMaxAttach = 2;
static constexpr int kExtNpcMaxSubtype = 4;
static constexpr int kExtNpcMaxSlavePairs = 8;

struct dExtNpcAttachSpec {
    char model[64];
    // №273: optional override arc for the attach model (generic key; package
    // names the arc). Empty = load from the mount's own `arc=` (prior behavior).
    char arc[64];
    char joint[32];
    // ============================================================================
    // №218 — optional donor-authored local transform for a held prop.
    // ============================================================================
    // The donor's own actors place hand props as jointMtx × T(offs) × R(rot)
    // (e.g. the Ls1 telescope: handR × T(5.5,-3,-2) × R(-7463,15109,-23665)).
    // Without this an attach can only sit exactly ON the joint origin. rot is in
    // raw s16 angle units, authored in decimal in the manifest.
    cXyz offs;
    cXyz rot;
    bool hasLocal;
    // ========================================================================
    // №250 — the DEMO-POSE variant (vanilla-truthful state split).
    // ========================================================================
    // The donor selects the prop's local transform by its demo-driven flag
    // (Ls1 m841: set while a storyboard owns her): in-demo T(5.7,-17.5,-1) vs
    // gameplay T(5.5,-3,-2). Shipping only one pose put the telescope through
    // her head in cutscenes. attach_offs_demo/attach_rot_demo author the
    // in-demo variant; absent = the base local is used in both states.
    cXyz offsDemo;
    cXyz rotDemo;
    bool hasDemoLocal;
    // ========================================================================
    // №262 — flag-gated prop lifetime (Grandma's clothes bundle).
    // ========================================================================
    // The donor's scene state machine: Ba1 CARRIES ba_cloth (hold.bck idle)
    // until the give, then never again. attach_unless_flag names the mod flag
    // that retires the attach the moment it is set (the give's set_flag).
    char unlessFlag[48];
};

struct dExtNpcSubtype {
    bool valid;
    int arg;  // matches params low byte
    char idle[64];
    char attachModel[64];
    char attachJoint[32];
    char displayName[64];
};

struct dExtNpcManifest {
    char proc[32];
    char socket[32];       // TP stub host, e.g. NPC_HENNA0 (default = proc)
    int socketArg;         // params low byte when sharing a socket (-1 = any/0)
    char arc[16];
    char model[64];
    char model2[64];
    char model3[64];
    char model2Btk[64];
    char collision[64];
    char idle[64];
    // ========================================================================
    // №262 — idle override while a flag-gated attach is still live (donor
    // state machine: Ba1 plays hold.bck while carrying ba_cloth, wait01 after
    // the give). Empty = plain idle in both states.
    // ========================================================================
    char idleAttached[64];
    // F-2 / №281b: opt-in arc for idle_attached BCK (Demo01 ba_wait_l ≠ Ba.arc).
    // Empty = load idle_attached from the actor arc (№262 default).
    char idleAttachedArc[32];
    // №263: one-shot PRESENT animation played at the get-item handoff (donor:
    // Ba1's hold.bck give motion). Distinct from idle_attached — this is the
    // moment of the give, not the carrying state.
    char presentAnim[64];
    char talk1[64];
    char talk2[64];
    char btp[64];
    // №188: idle blink texture-pattern (e.g. Aryll's `maba.btp`). Distinct from
    // `btp` because the semantics differ: `btp` LOOPS; a blink HOLDS eyes-open on
    // a random timer, then plays the close/open once. WW faces are texture planes
    // and "blink" is a BTP frame swap — the donor's `daNpc_Ls1_c::play_btp_anm`
    // with resID index 1 (`MABA` = mabataki). Empty = no idle blink.
    char blinkBtp[64];
    char displayName[64];
    char neckJoint[32];
    char dialogueKey[64];
    char modFolder[128];
    char companionModel[64];
    char companionIdle[64];
    // №36 A: 0 = synced base TRMtx + own BCK (mdarm); 1 = joint_slave (lshand / no BCK).
    u8 companionMode;
    // ========================================================================
    // №249 — companion hidden by default (presence axis 3 at attachment scale).
    // ========================================================================
    // Grandma's `ba_cloth.bdl` (the hero's-clothes bundle) mounts as her
    // companion but must only PRESENT during the give scene — unbound at her
    // base matrix it rendered as the "green model in her head" (№244). Hidden
    // means not drawn; the scene pass flips it with the hold-state.
    bool companionHidden;
    char brk[64];          // №36 C: Vlupy color anim
    char btk[64];
    f32 colorFrame;        // brk/btk frame; <0 ⇒ from create params
    bool pickupRupee;      // near player ⇒ execItemGet(pickupItemNo) + delete
    char doorOpenBck[64];  // №37: e.g. dooropenadoor.bck
    char doorVisual[64];   // №50-E: visible door mesh (default door_a.bdl); controller = model=
    char bodyBmt[64];      // №50-C: material override table (e.g. ko02.bmt)
    char warpLabel[64];    // empty ⇒ no warp-menu row
    char hostStage[16];
    int hostRoom;
    int hostLayer;
    cXyz hostPos;
    bool hasHostPos;
    bool hasAnchor;
    bool hasSpawnRel;
    bool hasSpawnRy;            // №54: Nintendo PLYR facing (s16 angle)
    bool hasReturnPos;          // №56: Nintendo outdoor return (WW world)
    bool hasReturnRy;
    bool hasExitDoorRel;        // №56: interior KNOB00 relative to host
    char populationCsv[64];     // relative to mod population/ (BG only)
    char populationStage[16];   // №32: when CSV has stage col, only rows matching this (e.g. LinkRM)
    char spawnIfFlag[64];       // R-O2e: require flag set to create
    char spawnUnlessFlag[64];   // R-O2e: refuse create if flag set
    dExtNpcAttachSpec attach[kExtNpcMaxAttach];
    int attachCount;
    dExtNpcSubtype subtypes[kExtNpcMaxSubtype];
    int subtypeCount;
    f32 cylRadius;
    f32 cylHeight;
    f32 scale;
    cXyz anchor;
    cXyz spawnRel;
    s16 spawnRy;                // №54: entry facing + exit-prop wall offset axis
    cXyz returnPos;             // №56: WW-world porch return
    s16 returnRy;
    cXyz exitDoorRel;           // №56: exit knob at room's real door
    u8 ambR;
    u8 ambG;
    u8 ambB;
    bool isBg;
    // When true: model baseTRMtx = translate(host); mBgMtx = translate(host−anchor).
    bool modelSpaceLocal;
    bool skipBtp;
    bool carryable;  // R-O2d: expose TP carry attention
    bool isBait;     // §222: this mount IS an esa bait actor (pig AI target)
    bool isStatic;   // №32: prop/door — idle optional (NULL anm)
    // №126: geometry comes from CODE + an extracted asset pack, not an arc
    // (the ported vegetation lane). Such a manifest legitimately has no arc=
    // and no model=, and exists only so the population spawner's payload and
    // lookup gates pass. Without this flag the validity check rejects it.
    bool isCodeGeom;
    bool doorAttention;  // №32 B1: A-press ENTER attention (not SPEAK)
    // №115: №58-B warm storm may create this BG after island COMPLEATE. Default true;
    // `warm=0` opts out (foreign-stage / heavy arcs). Engine also filters by host_stage.
    bool allowWarm;
    bool fromDvd;
    bool valid;
};

class dExtNpcMount_c : public fopAc_ac_c {
public:
    request_of_phase_process_class mPhase;
    mDoExt_McaMorf* mpMorf;
    mDoExt_McaMorf* mpCompanion;
    mDoExt_btpAnm* mpBtp;
    mDoExt_brkAnm* mpBrk;
    mDoExt_btkAnm* mpColorBtk;
    // §218 BTK auto-binder: true when mpColorBtk is an animated sibling <model>.btk
    // (looping texture-SRT) that must be play()'d every frame, vs a static
    // color-select frame (Vlupy). See the auto-binder socket in tryBindModel.
    bool mColorBtkPlay;
    // §222 pig↔bait AI (donor-pig-bait-contract). Bait actor: mIsBait + esa
    // state (0=falling,1=ground-available); mBaitClaim = donor field_0x298 claim
    // slot (0=unclaimed, else pig ID). Pig: mAiTargetId = claimed bait actor id.
    bool mIsBait;
    s16 mBaitState;
    u32 mBaitClaim;
    f32 mBaitGroundY;
    u32 mAiTargetId;
    bool mAiMoving;  // §222: walk anim active — switch walk/idle on transition only
    J3DModel* mpBgModels[3];
    mDoExt_btkAnm* mpBgBtk;
    dBgW* mpBgW;
    Mtx mBgMtx;
    bool mIsBg;
    bool mBgReady;
    // №98: identity WW-host rooms register GLOBAL_e (world) instead of MOVE_BG_e.
    bool mBgGlobal;
    J3DModel* mpAttach[kExtNpcMaxAttach];
    s16 mAttachJnt[kExtNpcMaxAttach];
    // №273: resLoad ownership for attach_arc overrides (≠ mount arc).
    request_of_phase_process_class mAttachArcPhase[kExtNpcMaxAttach];
    u8 mAttachArcOwned[kExtNpcMaxAttach];
    // F-2: resLoad ownership for idle_attached_arc (e.g. Demo01).
    request_of_phase_process_class mIdleAttachedArcPhase;
    u8 mIdleAttachedArcOwned;
    // ============================================================================
    // №218 — per-slot donor local transform (see dExtNpcAttachSpec::offs/rot).
    // ============================================================================
    cXyz mAttachOffs[kExtNpcMaxAttach];
    cXyz mAttachRot[kExtNpcMaxAttach];
    u8 mAttachLocal[kExtNpcMaxAttach];
    // №250: the in-demo local variant + whether the demo branch owns this frame.
    cXyz mAttachOffsDemo[kExtNpcMaxAttach];
    cXyz mAttachRotDemo[kExtNpcMaxAttach];
    u8 mAttachDemoLocal[kExtNpcMaxAttach];
    bool mDemoOwned;
    // №263: present animation in flight — execute() returns to idle at its end.
    bool mPresentAnimActive;
    // №251: pending get-item handoff — set by grant actions, consumed at talk
    // close (the item event REPLACES the speak event; no reset in between).
    int mPresentDemoItemNo;
    u8 mAttachOnCompanion[kExtNpcMaxAttach];
    // 1 = №49/50 joint_slave CB (heads/arms); 0 = №50-E door visual (base@DoorDummy only).
    u8 mAttachSlave[kExtNpcMaxAttach];
    int mAttachCount;
    dExtNpcManifest mManifest;
    bool mGroundSnapped;
    s16 mGroundSnapTries;  // №25 F2: retry island dzb before accepting seafloor
    s16 mOrbitPhase;       // №27 N6: Kamome orbit phase (0 = grounded/static)
    u8 mHeadVariant;       // №27 N2/N4: current head index (1-based; 0 = none)
    bool mBtpBound;
    // №186: last texture-anim id applied from the storyboard. PER-ACTOR on
    // purpose — a scene has several performers (Link and Ls1 both bind), and a
    // file-static would let one actor's id suppress another's re-init.
    u32 mDemoTexAnmLast;
    // №188: idle-blink state. `mpBlink` is a SEPARATE BTP from `mpBtp` (which is
    // for expression/loop); the two target the same eye material but only one
    // drives it at a time — the blink runs whenever no demo expression is active.
    mDoExt_btpAnm* mpBlink;
    bool mBlinkBound;
    s16 mBlinkTimer;
    f32 mBlinkFrame;
    // №194: set while the storyboard's prm channel is driving this actor's face.
    // Draw reads it to suppress the idle-blink entry so it can't clobber the
    // demo expression (both write the same eye materials).
    bool mDemoFaceActive;
    // №196: demo-face BTP frame — advanced play-ONCE-and-hold (donor
    // play_btp_anm non-blink branch), reset to 0 only when the expression
    // (resID) changes. NOT a loop — looping re-blinks bwait continuously.
    f32 mDemoFaceFrame;
    // №197: demo-face BTK (texture SRT). The prm channel's SECOND (0,0,1) entry.
    // Only visibly matters for the open-mouth/tongue — it scales the tongue
    // texture into the mouth; without it the raw texture shows as a black+pink
    // square. Same play-once-hold discipline as the BTP.
    mDoExt_btkAnm* mpDemoBtk;
    bool mDemoBtkBound;
    u32 mDemoBtkLast;
    f32 mDemoBtkFrame;
    dCcD_Stts mCcStts;
    dCcD_Cyl mCyl;
    s16 mNeckJnt;
    s16 mLookYaw;
    s16 mLookPitch;
    bool mTalking;
    bool mCcReady;
    bool mTalkEventActive;
    s16 mTalkFrames;
    s16 mCloseCooldown;
    // Phase O2: dialogue section cursor + pending next= hop after dismiss.
    char mDialogueSection[64];
    char mDialogueNext[64];
    u8 mPickupItemNo;  // №36 C: WW/TP rupee item id when pickupRupee
    // №48/№49: actor_map companion_slave_map; resolved pairs drive joint callbacks.
    char mSlaveMap[160];
    s16 mSlaveCompJnt[kExtNpcMaxSlavePairs];
    s16 mSlaveBodyJnt[kExtNpcMaxSlavePairs];
    u8 mSlavePairCount;
    // №51: doors.ini section name when spawned as a Knob (`door:<name>` pending src).
    char mDoorKey[32];
    // E2 spawn src persisted for the actor lifetime (forceNextSpawnSrc is one-shot).
    char mSpawnSrc[96];
    // №91: TP knob-door event ownership (DoorK10 DEFAULT_KNOB_DOOR_* + cutEnd).
    s16 mKnobEvtFront;     // DEFAULT_KNOB_DOOR_F_OPEN
    s16 mKnobEvtBack;      // DEFAULT_KNOB_DOOR_B_OPEN
    s16 mKnobEvtOrdered;   // event currently ordered (-1 = none)
    int mKnobStaffId;      // SHUTTER_DOOR staff
    u8 mKnobDoorAction;    // 0=wait 1=demo
    u8 mKnobOpenStarted;   // WW door_open_bck kicked this demo
    u8 mKnobEvBound;       // event idxs resolved for this actor
    // History Ba1_Get_Itm: mount owns the Ba1 staff (WAIT → cradle) while
    // Link's 011get_item + CAMERA GETITEM drive the raise/show cutscene.
    s16 mBa1GetEvtOrdered;  // -1 = none
    int mBa1StaffId;
    u8 mBa1GetActive;
};

// №37: play door-open BCK on nearest doorAttention mount (Knob).
bool dExtNpcMount_playAnimNearest(const cXyz& from, f32 maxDist, const char* bckName);

bool dExtNpcMount_hasPayload(const char* procName);
bool dExtNpcMount_lookup(const char* procName, dExtNpcManifest* out);

// Socket helpers (Phase M): resolve which proc a stub should mount.
bool dExtNpcMount_hasSocketPayload(const char* socketName);
bool dExtNpcMount_resolveSocket(const char* socketName, int arg, char* procOut, u32 procOutBytes);
s16 dExtNpcMount_socketActorId(const char* socketName);

// №24 D1: pin the next stub Create to a proc key (BG warp / population).
// Consumed once by the stub Create path — bypasses socket-arg ambiguity.
// Prefer bindPendingSpawn when fopAcM_create returns an id (async-safe).
void dExtNpcMount_forceNextCreateProc(const char* procName);
bool dExtNpcMount_consumeForcedCreateProc(char* procOut, u32 procOutBytes);

// №27 N2: pin the next Create's head/prop attach (population head_from_params).
void dExtNpcMount_forceNextAttach(const char* modelName, const char* jointName);
bool dExtNpcMount_consumeForcedAttach(char* modelOut, u32 modelBytes, char* jointOut,
                                      u32 jointBytes);

// №38 E2: pin spawn ledger src= for the next Create (e.g. census:Ym1@pos / door:linkrm).
void dExtNpcMount_forceNextSpawnSrc(const char* src);

// №44/№45: pin proc/src/head for the next Create. Prefer pushPendingSpawn BEFORE
// fopAcM_create (FIFO — Create may run before an id bind lands). Id bind is a backup.
u32 dExtNpcMount_pushPendingSpawn(const char* procName, const char* src, const char* headModel,
                                   const char* headJoint);
void dExtNpcMount_bindPendingSpawn(fpc_ProcID id, const char* procName, const char* src,
                                   const char* headModel, const char* headJoint);
// Take+erase: id map first, else FIFO front. NULL outs ignored.
// №130: if this spawn's entry is still queued after create, move it OUT of the
// order-sensitive FIFO and re-key it to the owning actor id. Kills the whole
// leak/misassignment class: a late or forgetful actor can neither be served
// another's entry nor cause another to be served its own. Safe for deferred
// creates — the entry survives, it just stops being order-dependent.
bool dExtNpcMount_reapPendingSpawn(u32 seq, fpc_ProcID id);
// №131: is the №130 guard active? Overridable mod-side without a rebuild via
// population/engine_overrides.ini (`pending_spawn_guard=0` ⇒ vanilla FIFO order).
bool dExtNpcMount_pendingGuardEnabled();
// №131: drop id-keyed entries whose actor is gone. Process ids are recycled, and
// vanilla re-spawns the cast on every story/layer change, so this must run each
// population pass or a recycled id can inherit a dead row's head.
void dExtNpcMount_sweepPendingById();
// №133: record a pass boundary on the guard's own clock (no vanilla state read).
void dExtNpcMount_markPendingPass();
bool dExtNpcMount_takePendingSpawn(fpc_ProcID id, char* procOut, u32 procBytes, char* srcOut,
                                   u32 srcBytes, char* headOut, u32 headBytes, char* jointOut,
                                   u32 jointBytes);
// §334: WW→TP collision-attribute repack (bus §332/§333) — call on every
// WW-sourced cBgD_t BEFORE dBgW::Set. Idempotent (pointer-keyed); staged arcs
// stay byte-verbatim (in-memory parsed copy only).
struct cBgD_t;
void dExtWw_repackDzbAttributes(cBgD_t* bgd, const char* tag);
// №38/№51: nearest doorAttention Knob within maxDist. If `facingYaw` non-NULL, prefer
// props in front of that yaw (≤90°), else pure nearest. Writes *outDistXZ when non-NULL.
dExtNpcMount_c* dExtNpcMount_nearestDoorAttention(const cXyz& from, f32 maxDist);
dExtNpcMount_c* dExtNpcMount_facedDoorAttention(const cXyz& from, s16 facingYaw, f32 maxDist,
                                                   f32* outDistXZ);

int dExtNpcMount_create(dExtNpcMount_c* i_this, const char* procName);
int dExtNpcMount_delete(dExtNpcMount_c* i_this);
int dExtNpcMount_execute(dExtNpcMount_c* i_this);
int dExtNpcMount_draw(dExtNpcMount_c* i_this);

void dExtNpcMount_rescanProviders();
bool dExtNpcMount_shouldSkipBtp(const char* arcName);

// №54-4: local ground snap (refY+50 probe, ±250 clamp). Used by door porch returns.
bool dExtNpcMount_localGroundSnap(cXyz* pos, f32 refY);

// №27 N4: identity audition — cycle head / lock display name on nearest mount.
bool dExtNpcMount_cycleHeadNearest(const cXyz& from, f32 maxDist);
bool dExtNpcMount_setDisplayNameNearest(const cXyz& from, f32 maxDist, const char* name);
const char* dExtNpcMount_nearestDisplayName(const cXyz& from, f32 maxDist);

// Level Editor / general pick identify — query mount metadata for a live actor (click-only).
struct dExtNpcIdentifyInfo {
    char proc[32]{};
    char displayName[64]{};
    char modFolder[128]{};
    char headModel[64]{};
    char headJoint[32]{};
    char censusName[32]{};  // §41: from mSpawnSrc — "census:<Name>@…" → "<Name>"; else raw src
    int socketArg = -1;
    u8 headVariant = 0;
    bool valid = false;
};

bool dExtNpcMount_queryActor(const fopAc_ac_c* actor, dExtNpcIdentifyInfo* out);
// §41: Z-target identity probe — log once per LockonTarget change (d_s_play poll).
void dExtNpcMount_pollIdentifyProbe();
#if TARGET_PC
// §95: tree/mount cull probe — env DUSK_CULL_PROBE=1; edge + heartbeat logs.
void dExtNpcMount_pollCullProbe();
#endif

// Manifest-driven BG warps.
bool dExtNpcMount_requestBgWarp(const char* procName);
// Same as requestBgWarp, but place Link at absolute world spawn (door returns).
bool dExtNpcMount_requestBgWarpTo(const char* procName, const cXyz& spawnWorld);
// №56: return porch + optional facing (return_ry).
bool dExtNpcMount_requestBgWarpTo(const char* procName, const cXyz& spawnWorld, s16 facing);
// №34 P4: door enter with black-void guard — if target BG never becomes ready,
// abort and return Link to failSafeSpawn (outdoor door).
bool dExtNpcMount_requestBgWarpGuarded(const char* procName, const cXyz& failSafeSpawn);
void dExtNpcMount_onStageReady();
void dExtNpcMount_pollBgWarps();
// Faithful TagEv region-fire (no native d_a_tag_event): consume
// population/region_triggers.ini ([tale_loft] → TALE_DEMO / tale.stb).
void dExtNpcMount_pollRegionTriggers();
// №269 / §158: ClrWallNone + OffLineCheckNone + ground reprobe (donor procDoorOpen pair).
// Call on place after room-lane OR native-stage arrival into WW interiors.
void dExtNpcMount_forceLinkGroundReprobe(fopAc_ac_c* player);
// Last successfully completed BG warp proc (empty if none); for door triggers.
const char* dExtNpcMount_lastBgProc();
bool dExtNpcMount_bgWarpBusy();

// №94: world generation — spawn latches are keyed to this; bump on play-scene
// recreate / restart so doors+census re-run after void-fall / death / reload.
u32 dExtNpcWorld_generation();
void dExtNpcWorld_bump(const char* reason);

// №84: abort in-flight room-lane / BG transports before a native setNextStage
// (foreign-stage entry must not room-create while the play scene tears down).
void dExtNpcMount_cancelTransports();
// After setNextStage back to an exterior host: remount proc (usually EXT_BG0)
// once the new play scene exists, then place Link at spawn.
void dExtNpcMount_armStageExitRemount(const char* procName, const cXyz& spawnWorld, s16 facing,
                                       bool hasFacing);

// №58-B / №53-A: lock Link (door-open demo) through fade; release on hold complete/abort.
void dExtNpcMount_beginDoorDemoLock();
void dExtNpcMount_endDoorDemoLock();
// №89: nuclear clear — cancel demo + remove stuck event (arrival end / G-guard).
void dExtNpcMount_forceEndDoorEvent(const char* reason);
// §270: stand the arrival-G-guard down so a native WW NPC's freshly-ordered event (the
// tale) isn't force-ended as if it were a stuck arrival residual. Re-arms on next arrival.
void dExtNpcMount_clearArrivalGuard(const char* why);
// §297: the tale trigger is native (daAlink reads the donor loft-point PLYR param →
// evmng_startDemo). The port only supplies the demo-arc residency the native gets from
// LBNK; that lives entirely inside d_ext_npc_mount.cpp (dExtWw_pollTaleEntryDemo), no
// pending-id wire. (The former dExtWw_setPendingTaleDemo bridge is removed.)

// §278: make the tale storyboard arc (Demo01/tale.stb) resident + named before ba1
// orders the tale event. Returns true only when getStbDemoData(stbName) resolves.
bool dExtWw_ensureTaleArcResident(const char* stbName);

// §281: true if the running event is the tale storyboard under ANY of its names
// (TALE_DEMO/TALE_DEMO2 or ba1's native tale_1/tale_2). Used by the presentation gates.
bool dExtWw_isTaleRunEvent(const char* runEvt);

// №62 Phase D: room-per-interior lane (exemplar LinkRM → host room 2).
// doors.ini lane=room + host_room=N registers proc→room; create/destroy bind to d_s_room.
void dExtNpcMount_registerRoomLane(const char* procName, int hostRoom);
bool dExtNpcMount_isRoomLaneProc(const char* procName);
int dExtNpcMount_roomLaneHostRoom(const char* procName);  // -1 if not room-lane
// №257: true if a room-lane BG mount is registered to own this room's collision
// (daBg must not also Regist a stub room.dzb on WW hosts).
bool dExtNpcMount_isRoomLaneRoom(int roomNo);
// True while a room-lane interior is claimed — dStage::loadRoom must not kill it
// (alink RoomCheck RTBL only knows room 0 and would otherwise recreate-loop).
bool dExtNpcMount_isRoomLaneProtected(int roomNo);
// №68: true while room-lane teardown is in flight (skip poll/draw/attention).
bool dExtNpcMount_isRoomLaneUnloading(int roomNo);
// Keep exterior room 0 + claim hostRoom (forced loadRoom; bypasses RTBL adjacency).
bool dExtNpcMount_ensureRoomLaneLoaded(int hostRoom);
// Drop claimed interior room; keep room 0.
bool dExtNpcMount_ensureRoomLaneUnloaded(int hostRoom);
void dExtNpcMount_onRoomObjectsReady(const char* stageName, int roomNo);
void dExtNpcMount_onRoomUnload(const char* stageName, int roomNo);

// №65/№66: room-lane PLAYER TRANSPORT (no requestBgWarp*).
// Cover → load/unload → place → lift. Waits for room draw (checkRoomDisp) on enter.
bool dExtNpcMount_requestRoomLaneEnter(const char* procName, const cXyz& failSafeSpawn);
bool dExtNpcMount_requestRoomLaneExit(const char* procName, const cXyz& returnWorld, s16 facing,
                                       bool hasFacing);
bool dExtNpcMount_roomLaneTransportBusy();

// Provider enumeration for UI (Cut Actors / Warp).
int dExtNpcMount_providerCount();
bool dExtNpcMount_providerAt(int index, dExtNpcManifest* out);

// §27: WW bmd3/bdl arcs need ExtNpc load+finish (never cast getObjectRes → J3DModelData*).
// acquire pins ModelData in the session cache; retain/release keep the arc buffer alive (№73).
J3DModelData* dExtNpcMount_acquireModelData(const char* arc, const char* modelName);
// §630: the same resolver, sourced from the STAGE res control instead of the
// object one — for donor ROOM arcs staged byte-identical. A vanilla WW room arc
// files its models under RARC node type 'BDL ', which dRes_info_c has no branch
// for, so they are never mount-parsed and getStageRes hands back a raw buffer.
// This is DN-3's PRESCRIBED route for that case, not an exception to it: one
// cached parse at consume time, from a pristine byte copy, so the donor buffer
// is never pointer-fixed in place and stays byte-identical in memory as well as
// on disk.
J3DModelData* dExtNpcMount_acquireStageModelData(const char* arc, const char* modelName);
// §229 direct-port helper: acquire a model with a body BMT baked in (parse-at-consume,
// cache-keyed by model+bmt). For WW actors whose COLOR lives in a .bmt swap (pig pg_*.bmt)
// — a raw getObjectRes bmt can't be applied, and the base model renders untextured/black.
J3DModelData* dExtNpcMount_acquireModelDataBmt(const char* arc, const char* modelName,
                                               const char* bmtName);
// §181 (Housing Approach A): consume-time BDL parse for daDemo00 cutscene doubles.
// The SHAPE model comes by resource ID (getObjectIDRes) from the demo arc, RAW —
// route it through the same cache/pristine machinery so it parses once and drops
// with the demo arc (purge is erase-only; the arc owns the buffer). See d_ext_npc_mount.cpp.
J3DModelData* dExtNpcMount_acquireDemoModel(const char* arc, u16 id, void* res);
void dExtNpcMount_retainArc(const char* arc);
void dExtNpcMount_releaseArc(const char* arc);

#endif  // TARGET_PC

#endif /* D_EXT_NPC_MOUNT_H */
