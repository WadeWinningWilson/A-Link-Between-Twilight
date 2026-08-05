#ifndef D_KANKYO_WW_H
#define D_KANKYO_WW_H

// ============================================================
// §404 WW LIGHTING PORT (user order; Housing's BDL deliberation named this
// the critical path). The donor's setLightTevColorType(_sub) is the per-frame
// write-path that WW materials are AUTHORED against:
//   - it REWRITES each material's color-channel light mask every frame
//     (clamps to slot 0; enables slot 1 only when the second-light color
//     mColorK1 is live, fed through TevKColor(1) + an extra TEV stage) —
//     the file's litMask 0x03 is just the authored default the runtime
//     manages, which is what staging's normalize_litmask imitated statically;
//   - it OVERWRITES TEV color/konst register 0 with the tevstr's live
//     light/shadow colors — the shipped 128-gray is a placeholder the donor
//     never draws, which is what normalize_tevregs imitated by whitening.
// The receiver's plain setLightTevColorType(J3DModel*,…) is an EMPTY STUB, so
// every WW-mounted model has been drawing with NO lighting writes at all.
// This TU is the donor function verbatim (WW DP d_kankyo.cpp:1763-1873).
// §405 CORRECTION (Housing): TP settingTevStruct fills AmbCol/fog/lights but
// NOT TevColor/TevKColor — TP computes material colors inside _MAJI instead,
// and the only tevstr writer of those two fields is the 12/13 leg (zeroing).
// The color SOURCE is therefore dKyWw_settingTevStruct below (the donor's
// own tail), fed by the №113-converted palettes' live blends. Once the WW actors call this, the two staging adapters can be
// retired and the 68-arc restage (Housing) proceeds without darkening.
// ============================================================

#include "dolphin/types.h"

class J3DModel;
class J3DMaterial;
class dKy_tevstr_c;

// ============================================================
// §406 WW TevType values, donor d_kankyo.h:133-149 VERBATIM — centralized.
// Four TUs each carried a private `#define TEV_TYPE_BG0 1` (knob00, lamp,
// toripost) / `BG1 2` (spc_item01): the trap-40 "knowledge didn't travel"
// pattern in miniature. One definition, here, next to the feeder that
// consumes them. TEV_TYPE_ACTOR (0) stays in d_ext_ww_actor_shims.h:134;
// guarded so either include order works.
// ============================================================
#ifndef TEV_TYPE_ACTOR
#define TEV_TYPE_ACTOR 0
#endif
#ifndef TEV_TYPE_BG0
#define TEV_TYPE_BG0 1
#define TEV_TYPE_BG1 2
#define TEV_TYPE_BG2 3
#define TEV_TYPE_BG3 4
#define TEV_TYPE_BG0_FULL 5
#define TEV_TYPE_BG1_FULL 6
#define TEV_TYPE_BG2_FULL 7
#define TEV_TYPE_BG3_FULL 8
#define TEV_TYPE_PLAYER 9
#define TEV_TYPE_UNK99 99
#endif

void dKyWw_setLightTevColorType_sub(J3DMaterial* i_material, dKy_tevstr_c* i_tevstr);
void dKyWw_setLightTevColorType(J3DModel* i_model, dKy_tevstr_c* i_tevstr);
// §408: per-material form, exported for draws that mix live palette colours
// with authored per-material registers (the mounted sea sheet keeps its baked
// C1-C3/K1-K3 and takes C0/K0 + fog from the tevstr through this).
void dKyWw_setLightTevColorType_sub(J3DMaterial* i_material, dKy_tevstr_c* i_tevstr);

// §405: the donor settingTevStruct TAIL — receiver settingTevStruct + the WW
// tevstr TevColor/TevKColor write TP moved into _MAJI. WW actors call THIS,
// not g_env_light.settingTevStruct directly, or §404's copy reads black.
class cXyz;
void dKyWw_settingTevStruct(int i_lightType, cXyz* i_pos, dKy_tevstr_c* i_tevstr);

// §411: "a WW sky is hosted" -- distinct from hide_vrbox ("no sky at all").
// Set by the mount alongside hide_vrbox; consulted by the wether gates.
void dKyWw_setSkyHost(bool i_active);
bool dKyWw_isSkyHost();
// §418: true once the NATIVE vrbox actors own the dome (mount draw retires).
void dKyWw_setDomeActorsLive(bool i_live);
bool dKyWw_domeActorsLive();

#endif /* D_KANKYO_WW_H */
