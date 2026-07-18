#ifndef D_EXT_NPC_MOUNT_H
#define D_EXT_NPC_MOUNT_H

#if TARGET_PC

#include "f_op/f_op_actor_mng.h"
#include "m_Do/m_Do_ext.h"
#include "SSystem/SComponent/c_phase.h"

// Plan R — generic external-NPC mount (L1: model + idle BCK + ground snap + light + blink).
// Stub sockets (NPC_MK / NPC_P2) delegate here when a mod provides arc + manifest.

struct dExtNpcManifest {
    char proc[32];
    char arc[16];
    char model[64];
    char idle[64];
    char talk1[64];
    char talk2[64];
    char btp[64];          // optional blink BTP member
    char displayName[64];
    char neckJoint[32];
    char modFolder[128];
    f32 cylRadius;
    f32 cylHeight;
    f32 scale;             // actor uniform scale (child NPCs ~1.0)
    bool skipBtp;          // leave BTP unparsed during resLoad
    bool fromDvd;          // load arc from game DVD (no mod arcs/ required)
    bool valid;
};

class dExtNpcMount_c : public fopAc_ac_c {
public:
    request_of_phase_process_class mPhase;
    mDoExt_McaMorf* mpMorf;
    mDoExt_btpAnm* mpBtp;
    dExtNpcManifest mManifest;
    bool mGroundSnapped;
    bool mBtpBound;
};

// True when an enabled mod has arcs/<arc>.arc + a matching npc/*.ini for this proc.
bool dExtNpcMount_hasPayload(const char* procName);

// Fill manifest for proc (NPC_MK / NPC_P2). Returns false if payload absent.
bool dExtNpcMount_lookup(const char* procName, dExtNpcManifest* out);

// Create-phase for a socket actor already allocated as dExtNpcMount_c.
// Returns cPhs_COMPLEATE_e / cPhs_ERROR_e / intermediate load phases.
int dExtNpcMount_create(dExtNpcMount_c* i_this, const char* procName);
int dExtNpcMount_delete(dExtNpcMount_c* i_this);
int dExtNpcMount_execute(dExtNpcMount_c* i_this);
int dExtNpcMount_draw(dExtNpcMount_c* i_this);

// Rebuild provider table from enabled mods (call from custom_assets::scan).
void dExtNpcMount_rescanProviders();

// True while an ExtNpc create is resLoading an arc that asked skip_btp=1.
bool dExtNpcMount_shouldSkipBtp(const char* arcName);

#endif  // TARGET_PC

#endif /* D_EXT_NPC_MOUNT_H */
