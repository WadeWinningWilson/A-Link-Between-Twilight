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

// ============================================================================
// §638: a LIST, not a single destination.
//
// One --stage armed one row, so reaching a second dev stage meant relaunching.
// Worse, a stage the mod already DECLARES in data had no route at all unless
// the command line happened to name it too — the declaration knew about the
// stage and the menu did not.
//
// Now anything can add a destination: --stage takes a ';'-separated list, and a
// content layer calls dBootStage_add for stages it declares. The stage id is
// still never a compiled literal — every entry arrives as a runtime string.
// ============================================================================
struct BootTarget {
    char stage[16];
    char label[64];
    int room;
    int layer;
    bool fromData;   // declared by a mod vs. named on the command line
};

const int kMaxTargets = 16;
BootTarget s_targets[kMaxTargets];
int s_count = 0;

// NAME[,room[,layer]] — parsed here rather than by the option library so the
// stage id stays a plain runtime string.
void addOne(const char* spec, bool fromData) {
    if (spec == NULL || spec[0] == '\0' || s_count >= kMaxTargets) {
        return;
    }
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%s", spec);
    int room = 0;
    int layer = -1;
    char* comma = std::strchr(buf, ',');
    if (comma != NULL) {
        *comma++ = '\0';
        char* comma2 = std::strchr(comma, ',');
        if (comma2 != NULL) {
            *comma2++ = '\0';
            layer = std::atoi(comma2);
        }
        room = std::atoi(comma);
    }
    if (buf[0] == '\0') {
        return;
    }
    for (int i = 0; i < s_count; i++) {
        if (std::strcmp(s_targets[i].stage, buf) == 0 && s_targets[i].room == room) {
            return;  // already listed — a mod declaring what --stage also named
        }
    }
    BootTarget& t = s_targets[s_count];
    std::snprintf(t.stage, sizeof(t.stage), "%s", buf);
    t.room = room;
    t.layer = layer;
    t.fromData = fromData;
    std::snprintf(t.label, sizeof(t.label), "%s  room %d  layer %d", t.stage, room, layer);
    s_count++;
    DuskLog.info("[BootStage] target {}: '{}' room={} layer={} ({})", s_count - 1,
                 t.stage, room, layer, fromData ? "declared in mod data" : "--stage");
}

}  // namespace

void dBootStage_arm(const char* spec) {
    if (spec == NULL) {
        return;
    }
    // ';'-separated so one flag can arm several: --stage "sea,44;R_DL02"
    char buf[256];
    std::snprintf(buf, sizeof(buf), "%s", spec);
    char* p = buf;
    while (p != NULL && *p != '\0') {
        char* semi = std::strchr(p, ';');
        if (semi != NULL) {
            *semi++ = '\0';
        }
        addOne(p, false);
        p = semi;
    }
}

void dBootStage_add(const char* spec) {
    addOne(spec, true);
}

int dBootStage_count(void) {
    return s_count;
}

const char* dBootStage_labelAt(int i) {
    return (i >= 0 && i < s_count) ? s_targets[i].label : NULL;
}

bool dBootStage_warpAt(int i) {
    if (i < 0 || i >= s_count) {
        return false;
    }
    const BootTarget& t = s_targets[i];
    if (dComIfGp_isEnableNextStage()) {
        // A change is already queued. Stacking a second one is how you get a
        // fault in a stage you never asked for.
        DuskLog.warn("[BootStage] refused: a stage change is already queued");
        return false;
    }
    const char* cur = dComIfGp_getStartStageName();
    if (cur != NULL && std::strcmp(cur, t.stage) == 0) {
        // Saying so beats silently doing nothing: "the button did nothing" and
        // "the button worked" are otherwise identical from the outside.
        DuskLog.info("[BootStage] already in '{}' — nothing to do", t.stage);
        return false;
    }
    // Same call the warp menu's own rows use (warp.cpp:413), which is the proven
    // path. Point 0: authored PLYR spawn, never -1 (№90).
    mDoGph_gInf_c::offFade();
    dComIfGp_setNextStage(t.stage, 0, static_cast<s8>(t.room), static_cast<s8>(t.layer));
    DuskLog.info("[BootStage] WARP -> stage='{}' room={} layer={}", t.stage, t.room,
                 t.layer);
    return true;
}

#else

void dBootStage_arm(const char*) {}
void dBootStage_add(const char*) {}
int dBootStage_count(void) { return 0; }
const char* dBootStage_labelAt(int) { return NULL; }
bool dBootStage_warpAt(int) { return false; }

#endif  // TARGET_PC
