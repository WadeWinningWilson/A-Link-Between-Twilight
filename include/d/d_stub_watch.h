#ifndef D_STUB_WATCH_H
#define D_STUB_WATCH_H

#if TARGET_PC

#include <cstring>

#include "SSystem/SComponent/c_phase.h"
#include "d/d_com_inf_game.h"
#include "dusk/logging.h"
#include "f_op/f_op_actor_mng.h"

// Jailer / cut-stub experiment: log create attempts then refuse safely.
inline int dStubWatch_refuseCreate(fopAc_ac_c* a, const char* procName) {
    const char* stage = dComIfGp_getStartStageName();
    DuskLog.debug(
        "[StubWatch] create-attempt proc={} stage={} room={} layer={} params={:08x} pos=({:.0f},{:.0f},{:.0f})",
        procName != NULL ? procName : "?", stage != NULL ? stage : "?",
        (int)fopAcM_GetRoomNo(a), dComIfG_play_c::getLayerNo(0), (u32)fopAcM_GetParam(a),
        a->current.pos.x, a->current.pos.y, a->current.pos.z);
    OS_REPORT("[StubWatch] create-attempt proc=%s stage=%s room=%d layer=%d params=%08x pos=(%d,%d,%d)\n",
              procName != NULL ? procName : "?", stage != NULL ? stage : "?",
              (int)fopAcM_GetRoomNo(a), dComIfG_play_c::getLayerNo(0), (u32)fopAcM_GetParam(a),
              (int)a->current.pos.x, (int)a->current.pos.y, (int)a->current.pos.z);
    return cPhs_ERROR_e;
}

inline void dStubWatch_logLayerR_SP107(int roomNo) {
    const char* stage = dComIfGp_getStartStageName();
    if (stage != NULL && strcmp(stage, "R_SP107") == 0) {
        const int layer = dComIfG_play_c::getLayerNo(0);
        DuskLog.debug("[StubWatch] R_SP107 room={} layer={}", roomNo, layer);
        OS_REPORT("[StubWatch] R_SP107 room=%d layer=%d\n", roomNo, layer);
    }
}

#endif  // TARGET_PC

#endif /* D_STUB_WATCH_H */
