/**
 * d_demo_leftover_viewer.cpp — ALBW editor Demo Leftover Viewer.
 *
 * Catalog: all unique Demo*.arc BMD/BDL (generated .inc). Filter by category,
 * spawn draws at Link's feet.
 */
#include "d/d_demo_leftover_viewer.h"

#if TARGET_PC

#include <cstring>

#include "JSystem/J3DGraphAnimator/J3DAnimation.h"
#include "JSystem/J3DGraphAnimator/J3DModel.h"
#include "JSystem/JKernel/JKRHeap.h"
#include "SSystem/SComponent/c_math.h"
#include "SSystem/SComponent/c_phase.h"
#include "d/actor/d_a_alink.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "f_op/f_op_actor_mng.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_mtx.h"
#include <stdio.h>

namespace dDemoLeftoverViewer {
namespace {

#include "d/d_demo_leftover_catalog.inc"

// Non-Demo retail leftovers (History playtest №2) — additive, disc names as keys.
static const Entry kExtraCatalog[] = {
    {0, "body",
     "[body] Sample/file2.bmd — early Epona (CONFIRMED; partial tex OK)", "Sample", "file2.bmd"},
    {0, "body",
     "[body] E_s1/s1.bmd — Shadow Beast gen-2 (CONFIRMED; cycles 24 BCKs)", "E_s1", "s1.bmd"},
};
static constexpr int kExtraCount = (int)(sizeof(kExtraCatalog) / sizeof(kExtraCatalog[0]));
static constexpr int kTotalCount = kCatalogCount + kExtraCount;

// E_s1.arc BCK cycle (retail never loads these on the E_S1 actor).
static const char* const kEs1Anims[] = {
    "s1_wait01.bck",    "s1_wait02.bck",    "s1_walk.bck",     "s1_dash01.bck",
    "s1_dash02.bck",    "s1_attack.bck",    "s1_attack02.bck", "s1_attack03.bck",
    "s1_damaged.bck",   "s1_defend.bck",    "s1_down.bck",     "s1_hook.bck",
    "s1_pull.bck",      "s1_stick.bck",     "s1_shrink.bck",   "s1_shrinkdown.bck",
    "s1_change.bck",    "s1_dead01.bck",    "s1_dead02.bck",   "s1_dead03.bck",
    "s1_dead04.bck",    "s1_deadwait02.bck","s1_deadwait03.bck","s1_deadwait04.bck",
};
static constexpr int kEs1AnimCount = (int)(sizeof(kEs1Anims) / sizeof(kEs1Anims[0]));

static constexpr u32 kHeapSize = 0x200000;

enum Phase {
    PHASE_IDLE = 0,
    PHASE_LOADING,
    PHASE_READY,
    PHASE_FAILED,
};

static int s_selected = 0;
static int s_categoryFilter = CAT_GOLD;
static int s_spawnIndex = -1;
static Phase s_phase = PHASE_IDLE;
static request_of_phase_process_class s_phaseReq;
static JKRSolidHeap* s_heap = NULL;
static J3DModel* s_model = NULL;
static mDoExt_McaMorf* s_morf = NULL;
static int s_es1AnimIndex = -1;
static char s_statusBuf[160] = "Idle — pick a model and Spawn at feet.";
static char s_loadedArc[16] = {};
static const char* s_status = s_statusBuf;
static cXyz s_pos;
static s16 s_angleY = 0;

// Rebuild when filter changes.
static int s_filterCat = -1;
static int s_filterIdx[512];
static int s_filterCount = 0;

void rebuildFilter() {
    if (s_filterCat == s_categoryFilter && s_filterCount > 0) {
        return;
    }
    s_filterCat = s_categoryFilter;
    s_filterCount = 0;
    for (int i = 0;
         i < kTotalCount && s_filterCount < (int)(sizeof(s_filterIdx) / sizeof(s_filterIdx[0]));
         ++i) {
        const Entry* e = (i < kCatalogCount) ? &kCatalog[i] : &kExtraCatalog[i - kCatalogCount];
        if (s_categoryFilter == CAT_ALL || e->category == s_categoryFilter) {
            s_filterIdx[s_filterCount++] = i;
        }
    }
    // Keep selection valid within filter
    bool ok = false;
    for (int i = 0; i < s_filterCount; ++i) {
        if (s_filterIdx[i] == s_selected) {
            ok = true;
            break;
        }
    }
    if (!ok && s_filterCount > 0) {
        s_selected = s_filterIdx[0];
    }
}

void clearModel() {
    s_model = NULL;
    s_morf = NULL;
    s_es1AnimIndex = -1;
    if (s_heap != NULL) {
        mDoExt_destroySolidHeap(s_heap);
        s_heap = NULL;
    }
    if (s_loadedArc[0] != '\0') {
        dComIfG_resDelete(&s_phaseReq, s_loadedArc);
        s_loadedArc[0] = '\0';
    }
    cPhs_Reset(&s_phaseReq);
    s_spawnIndex = -1;
    s_phase = PHASE_IDLE;
}

bool setEs1Anim(int index) {
    if (s_morf == NULL || index < 0 || index >= kEs1AnimCount) {
        return false;
    }
    J3DAnmTransform* anm =
        (J3DAnmTransform*)dComIfG_getObjectRes("E_s1", kEs1Anims[index]);
    if (anm == NULL) {
        return false;
    }
    s_morf->setAnm(anm, J3DFrameCtrl::EMode_NONE, 3.0f, 1.0f, 0.0f, -1.0f, NULL);
    s_es1AnimIndex = index;
    snprintf(s_statusBuf, sizeof(s_statusBuf), "E_s1 anim %d/%d: %s", index + 1, kEs1AnimCount,
             kEs1Anims[index]);
    s_status = s_statusBuf;
    return true;
}

bool createModelFromRes(const Entry& e) {
    J3DModelData* data = (J3DModelData*)dComIfG_getObjectRes(e.arcName, e.resName);
    if (data == NULL) {
        s_status = "Failed — resource missing from arc.";
        return false;
    }

    s_heap = mDoExt_createSolidHeapFromGame(kHeapSize, 0x20);
    if (s_heap == NULL) {
        s_status = "Failed — could not allocate viewer heap.";
        return false;
    }

    JKRHeap* const prev = JKRGetCurrentHeap();
    mDoExt_setCurrentHeap(s_heap);

    const bool isEs1 = (strcmp(e.arcName, "E_s1") == 0);
    if (isEs1) {
        J3DAnmTransform* anm0 =
            (J3DAnmTransform*)dComIfG_getObjectRes("E_s1", kEs1Anims[0]);
        s_morf = JKR_NEW mDoExt_McaMorf(data, NULL, NULL, anm0, J3DFrameCtrl::EMode_NONE, 1.0f, 0,
                                       -1, 1, NULL, 0x80000, 0x11000084);
        if (s_morf != NULL && s_morf->getModel() != NULL) {
            s_model = s_morf->getModel();
            s_es1AnimIndex = 0;
            snprintf(s_statusBuf, sizeof(s_statusBuf), "E_s1 anim 1/%d: %s", kEs1AnimCount,
                     kEs1Anims[0]);
            s_status = s_statusBuf;
        } else {
            s_morf = NULL;
            s_model = NULL;
        }
    } else {
        s_model = mDoExt_J3DModel__create(data, 0x80000, 0x11000084);
    }

    mDoExt_setCurrentHeap(prev);

    if (s_model == NULL) {
        mDoExt_destroySolidHeap(s_heap);
        s_heap = NULL;
        s_status = "Failed — J3DModel/McaMorf create returned null.";
        return false;
    }

    mDoExt_adjustSolidHeap(s_heap);
    return true;
}

void updateFollowPos(daAlink_c* link) {
    if (link == NULL) {
        return;
    }
    s_angleY = link->shape_angle.y;
    s_pos = link->current.pos;
    s_pos.x += cM_ssin(s_angleY) * 150.0f;
    s_pos.z += cM_scos(s_angleY) * 150.0f;
    if (fopAcM_gc_c::gndCheck(&s_pos)) {
        s_pos.y = fopAcM_gc_c::mGroundY;
    }
}

}  // namespace

const Entry* entryAt(int index) {
    if (index < 0 || index >= kTotalCount) {
        return NULL;
    }
    if (index < kCatalogCount) {
        return &kCatalog[index];
    }
    return &kExtraCatalog[index - kCatalogCount];
}

int entryCount() {
    return kTotalCount;
}

const Entry* entry(int index) {
    return entryAt(index);
}

int selectedIndex() {
    return s_selected;
}

void setSelectedIndex(int index) {
    if (index >= 0 && index < kTotalCount) {
        s_selected = index;
    }
}

int categoryFilter() {
    return s_categoryFilter;
}

void setCategoryFilter(int category) {
    if (category < 0 || (category >= CAT_COUNT && category != CAT_ALL)) {
        return;
    }
    s_categoryFilter = category;
    s_filterCat = -1;
    rebuildFilter();
}

const char* categoryName(int category) {
    switch (category) {
    case CAT_GOLD:
        return "Gold leftovers";
    case CAT_LINK:
        return "Link";
    case CAT_MIDNA:
        return "Midna";
    case CAT_WOLF:
        return "Wolf";
    case CAT_NPC:
        return "NPCs / cast";
    case CAT_BOSS:
        return "Boss / Zelda / Zant";
    case CAT_PROP:
        return "Props / other";
    case CAT_ALL:
        return "All (+ beta Sample/E_s1)";
    default:
        return "?";
    }
}

int filteredCount() {
    rebuildFilter();
    return s_filterCount;
}

int filteredCatalogIndex(int filteredIndex) {
    rebuildFilter();
    if (filteredIndex < 0 || filteredIndex >= s_filterCount) {
        return -1;
    }
    return s_filterIdx[filteredIndex];
}

void requestSpawn() {
    clearModel();
    s_spawnIndex = s_selected;
    s_phase = PHASE_LOADING;
    cPhs_Reset(&s_phaseReq);
    const Entry* e = entryAt(s_spawnIndex);
    if (e == NULL) {
        s_status = "Failed — bad catalog index.";
        s_phase = PHASE_FAILED;
        return;
    }
    strncpy(s_loadedArc, e->arcName, sizeof(s_loadedArc) - 1);
    s_loadedArc[sizeof(s_loadedArc) - 1] = '\0';
    s_status = "Loading arc… stand in the field.";
}

void requestDespawn() {
    clearModel();
    s_status = "Despawned.";
}

void tick() {
    if (s_phase != PHASE_LOADING && s_phase != PHASE_READY) {
        return;
    }

    daAlink_c* link = (daAlink_c*)daPy_getPlayerActorClass();
    if (link == NULL) {
        return;
    }

    if (s_phase == PHASE_READY) {
        updateFollowPos(link);
        if (s_morf != NULL) {
            s_morf->play(NULL, 0, 0);
            if (s_morf->isStop()) {
                const int next = (s_es1AnimIndex + 1) % kEs1AnimCount;
                setEs1Anim(next);
            }
        }
        return;
    }

    const Entry* e = entryAt(s_spawnIndex);
    if (e == NULL) {
        s_status = "Failed — bad catalog index.";
        clearModel();
        s_phase = PHASE_FAILED;
        return;
    }
    const int ph = dComIfG_resLoad(&s_phaseReq, e->arcName);
    if (ph == cPhs_ERROR_e) {
        s_status = "Failed — dComIfG_resLoad error.";
        clearModel();
        s_phase = PHASE_FAILED;
        return;
    }
    if (ph != cPhs_COMPLEATE_e) {
        return;
    }

    if (!createModelFromRes(*e)) {
        dComIfG_resDelete(&s_phaseReq, e->arcName);
        s_loadedArc[0] = '\0';
        s_phase = PHASE_FAILED;
        return;
    }

    updateFollowPos(link);
    s_phase = PHASE_READY;
    if (s_morf == NULL) {
        s_status = "Spawned at feet — Despawn to clear.";
    }
}

void draw() {
    if (s_phase != PHASE_READY || s_model == NULL) {
        return;
    }

    daAlink_c* link = (daAlink_c*)daPy_getPlayerActorClass();
    if (link == NULL) {
        return;
    }

    s_model->setBaseScale(cXyz(1.0f, 1.0f, 1.0f));
    mDoMtx_stack_c::transS(s_pos);
    mDoMtx_stack_c::YrotM(s_angleY);
    MTXCopy(mDoMtx_stack_c::now, s_model->mBaseTransformMtx);

    if (s_morf != NULL) {
        s_morf->modelCalc();
        g_env_light.setLightTevColorType_MAJI(s_model, &link->tevStr);
        dComIfGd_setList();
        s_morf->entryDL();
    } else {
        s_model->calc();
        g_env_light.setLightTevColorType_MAJI(s_model, &link->tevStr);
        dComIfGd_setList();
        mDoExt_modelUpdateDL(s_model);
    }
}

const char* status() {
    return s_status != NULL ? s_status : "";
}

}  // namespace dDemoLeftoverViewer

#endif  // TARGET_PC
