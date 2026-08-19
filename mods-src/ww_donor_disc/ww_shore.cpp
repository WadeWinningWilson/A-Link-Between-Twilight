// Shore crash BTK — donor daBg_btkAnm SC_01 ↔ wave-frame sync.
// See ww_shore.h.

#include "ww_shore.h"

#include "d/dolzel_rel.h"  // IWYU pragma: keep
#include "d/d_com_inf_game.h"
#include "JSystem/J3DGraphAnimator/J3DAnimation.h"
#include "JSystem/J3DGraphAnimator/J3DModelData.h"
#include "JSystem/JUtility/JUTNameTab.h"
#include "m_Do/m_Do_ext.h"
#include <mods/api.h>
#include <mods/svc/log.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>

extern const LogService* s_log;
extern ModContext* mod_ctx;

namespace {

// Vanilla daBg_btkAnm_c (d_a_bg.h): mpBtk + field_0x4. Donor stores `special`
// in the same spare byte — SC_01 shore crash.
struct BgBtkAnm {
    mDoExt_btkAnm* mpBtk;
    u8 special;
};

u16 s_waveFrame = 0;
int s_armed = 0;

void shoreLog(const char* fmt, ...) {
    if (s_log == nullptr) {
        return;
    }
    char buf[384];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    s_log->write(mod_ctx, LOG_LEVEL_INFO, buf);
}

bool nameIsSc01(const char* name) {
    return name != nullptr && name[0] == 'S' && name[1] == 'C' && name[2] == '_' &&
           name[3] == '0' && name[4] == '1';
}

}  // namespace

void wwShore_reset() {
    s_waveFrame = 0;
    s_armed = 0;
}

void wwShore_tick() {
    // Donor d_envse.cpp type-1 SOND: field_0xf8 cycles 0..99 then wraps.
    // Full envse audio path is not required for the BTK consume-boundary;
    // the frame counter IS the visual clock SC_01 reads.
    if (s_waveFrame >= 99) {
        s_waveFrame = 0;
    } else {
        s_waveFrame++;
    }
}

unsigned wwShore_waveFrame() {
    return s_waveFrame;
}

void wwShore_onBtkEntry(void* selfRaw, void* modelDataRaw) {
    BgBtkAnm* self = static_cast<BgBtkAnm*>(selfRaw);
    if (self == nullptr || self->mpBtk == nullptr) {
        return;
    }
    // Vanilla entry just zeroed field_0x4. Donor then sets special from SC_01.
    self->special = 0;
    J3DAnmTextureSRTKey* btk = self->mpBtk->getBtkAnm();
    const char* name = nullptr;
    if (btk != nullptr && btk->getUpdateMaterialNum() > 0) {
        name = btk->mUpdateMaterialName.getName(0);
    }
    if (name == nullptr && modelDataRaw != nullptr && btk != nullptr &&
        btk->getUpdateMaterialNum() > 0) {
        J3DModelData* data = static_cast<J3DModelData*>(modelDataRaw);
        JUTNameTab* tab = data->getMaterialName();
        const u16 mid = btk->getUpdateMaterialID(0);
        if (tab != nullptr && mid != 0xFFFF) {
            name = tab->getName(mid);
        }
    }
    if (!nameIsSc01(name)) {
        return;
    }
    self->special = 1;
    s_armed++;
    shoreLog("[WwShore] {\"ev\":\"sc01_arm\",\"n\":%d,\"mat\":\"%s\",\"bindings\":%d}",
             s_armed, name, btk != nullptr ? (int)btk->getUpdateMaterialNum() : 0);
}

bool wwShore_tryWavePlay(void* selfRaw) {
    BgBtkAnm* self = static_cast<BgBtkAnm*>(selfRaw);
    if (self == nullptr || self->mpBtk == nullptr || self->special != 1) {
        return false;
    }
    self->mpBtk->setFrame((f32)s_waveFrame);
    static int s_playLog = 0;
    s_playLog++;
    if (s_playLog == 1 || (s_playLog % 300) == 0) {
        shoreLog("[WwShore] {\"ev\":\"wave_frame\",\"n\":%d,\"frame\":%u}", s_playLog,
                 (unsigned)s_waveFrame);
    }
    return true;  // caller skips vanilla play()
}
