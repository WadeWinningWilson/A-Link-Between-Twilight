// ============================================================================
// boot_stage.cpp — the --stage dev entry. See include/dusk/boot_stage.h for
// why this exists and why it is not a warp row or an SCLS edit.
//
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
//
// WW-AGNOSTIC BY CONSTRUCTION. Nothing here names a WW stage, reads a WW
// manifest, or depends on the WW layer. It warps to whatever stage id it is
// given, which is why it is receiver-side plumbing rather than a WW leg and why
// it survives DUSK_EXCLUDE_WW untouched.
// ============================================================================
#include "dusk/boot_stage.h"

#if TARGET_PC

#include <cstdio>
#include <cstring>

#include "d/d_com_inf_game.h"
#include "dusk/logging.h"
#include "m_Do/m_Do_graphic.h"

namespace {

bool s_armed = false;
char s_stage[16] = {};
char s_label[64] = {};
int s_room = 0;
int s_layer = -1;

}  // namespace

void dBootStage_arm(const char* spec) {
    if (spec == NULL || spec[0] == '\0') {
        return;
    }
    // NAME[,room[,layer]] — parsed here rather than by the option library so the
    // stage id stays a plain runtime string that is never a compiled literal.
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%s", spec);
    char* comma = std::strchr(buf, ',');
    if (comma != NULL) {
        *comma++ = '\0';
        char* comma2 = std::strchr(comma, ',');
        if (comma2 != NULL) {
            *comma2++ = '\0';
            s_layer = std::atoi(comma2);
        }
        s_room = std::atoi(comma);
    }
    if (buf[0] == '\0') {
        return;
    }
    std::snprintf(s_stage, sizeof(s_stage), "%s", buf);
    std::snprintf(s_label, sizeof(s_label), "%s  room %d  layer %d", s_stage,
                  s_room, s_layer);
    s_armed = true;
    DuskLog.info("[BootStage] armed: stage='{}' room={} layer={} "
                 "(warp window → Dev stage; nothing happens until you press it)",
                 s_stage, s_room, s_layer);
}

const char* dBootStage_target(void) {
    return s_armed ? s_stage : NULL;
}

const char* dBootStage_label(void) {
    return s_armed ? s_label : NULL;
}

bool dBootStage_warp(void) {
    if (!s_armed) {
        return false;
    }
    if (dComIfGp_isEnableNextStage()) {
        // A change is already queued. Stacking a second one is how you get a
        // fault in a stage you never asked for.
        DuskLog.warn("[BootStage] refused: a stage change is already queued");
        return false;
    }
    const char* cur = dComIfGp_getStartStageName();
    if (cur != NULL && std::strcmp(cur, s_stage) == 0) {
        // Saying so beats silently doing nothing: "the button did nothing" and
        // "the button worked" are otherwise identical from the outside.
        DuskLog.info("[BootStage] already in '{}' — nothing to do", s_stage);
        return false;
    }

    // Same call the warp menu's own rows use (warp.cpp:413), which is the proven
    // path. Point 0: authored PLYR spawn, never -1 (№90).
    mDoGph_gInf_c::offFade();
    dComIfGp_setNextStage(s_stage, 0, static_cast<s8>(s_room),
                          static_cast<s8>(s_layer));
    DuskLog.info("[BootStage] WARP -> stage='{}' room={} layer={}", s_stage,
                 s_room, s_layer);
    return true;
}

#else

void dBootStage_arm(const char*) {}
const char* dBootStage_target(void) { return NULL; }
const char* dBootStage_label(void) { return NULL; }
bool dBootStage_warp(void) { return false; }

#endif  // TARGET_PC
