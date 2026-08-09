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
bool s_fired = false;
char s_stage[16] = {};
int s_room = 0;
int s_layer = -1;
int s_settle = 0;

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
    s_armed = true;
    DuskLog.info("[BootStage] armed: stage='{}' room={} layer={} "
                 "(fires once, after the play scene is up)",
                 s_stage, s_room, s_layer);
}

void dBootStage_poll(void) {
    if (!s_armed || s_fired) {
        return;
    }
    // Let the first play scene settle before requesting a change. Firing during
    // boot would race the very stage load we are trying to replace, and a
    // half-initialised change reads as a crash in the new stage rather than as
    // a mistimed request here.
    if (++s_settle < 60) {  // ~1s at 60fps
        return;
    }
    if (dComIfGp_isEnableNextStage()) {
        return;  // a change is already queued; do not stack another
    }

    const char* cur = dComIfGp_getStartStageName();
    if (cur != NULL && std::strcmp(cur, s_stage) == 0) {
        // Already there — nothing to do, and saying so is better than silently
        // disarming, because "the flag did nothing" and "the flag worked" look
        // identical from the outside otherwise.
        DuskLog.info("[BootStage] already in '{}' — nothing to do", s_stage);
        s_fired = true;
        return;
    }

    // Same call the warp menu uses (warp.cpp:413), which is the proven path.
    // Point 0: authored PLYR spawn, never -1 (№90).
    mDoGph_gInf_c::offFade();
    dComIfGp_setNextStage(s_stage, 0, static_cast<s8>(s_room),
                          static_cast<s8>(s_layer));
    DuskLog.info("[BootStage] FIRED -> stage='{}' room={} layer={}", s_stage,
                 s_room, s_layer);
    s_fired = true;
}

#else

void dBootStage_arm(const char*) {}
void dBootStage_poll(void) {}

#endif  // TARGET_PC
