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
    char joint[32];
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
    char talk1[64];
    char talk2[64];
    char btp[64];
    char displayName[64];
    char neckJoint[32];
    char dialogueKey[64];
    char modFolder[128];
    char companionModel[64];
    char companionIdle[64];
    // №36 A: 0 = synced base TRMtx + own BCK (mdarm); 1 = joint_slave (lshand / no BCK).
    u8 companionMode;
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
    bool isStatic;   // №32: prop/door — idle optional (NULL anm)
    bool doorAttention;  // №32 B1: A-press ENTER attention (not SPEAK)
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
void dExtNpcMount_pushPendingSpawn(const char* procName, const char* src, const char* headModel,
                                   const char* headJoint);
void dExtNpcMount_bindPendingSpawn(fpc_ProcID id, const char* procName, const char* src,
                                   const char* headModel, const char* headJoint);
// Take+erase: id map first, else FIFO front. NULL outs ignored.
bool dExtNpcMount_takePendingSpawn(fpc_ProcID id, char* procOut, u32 procBytes, char* srcOut,
                                   u32 srcBytes, char* headOut, u32 headBytes, char* jointOut,
                                   u32 jointBytes);
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
    int socketArg = -1;
    u8 headVariant = 0;
    bool valid = false;
};

bool dExtNpcMount_queryActor(const fopAc_ac_c* actor, dExtNpcIdentifyInfo* out);

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

// №62 Phase D: room-per-interior lane (exemplar LinkRM → host room 2).
// doors.ini lane=room + host_room=N registers proc→room; create/destroy bind to d_s_room.
void dExtNpcMount_registerRoomLane(const char* procName, int hostRoom);
bool dExtNpcMount_isRoomLaneProc(const char* procName);
int dExtNpcMount_roomLaneHostRoom(const char* procName);  // -1 if not room-lane
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

#endif  // TARGET_PC

#endif /* D_EXT_NPC_MOUNT_H */
