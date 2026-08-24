#ifndef D_A_SPC_ITEM01_H
#define D_A_SPC_ITEM01_H

// ============================================================
// §327 WW WALL SHIELD / SPECIAL PLACED ITEM (SPC_ITEM01, OBJNAME "SPitem")
// DIRECT PORT — header VERBATIM from the WW donor
// (D:/XXXXXXX/WW DP/include/d/actor/d_a_spc_item01.h).
//
// BASE CLASS: the port's NATIVE daItemBase_c (include/d/actor/d_a_itembase.h —
// TP's own itembase, the SAME lineage as WW's: DeleteBase/DrawBase/animPlay/
// mpModel/mAcch/mAcchCir/m_itemNo/m_timer all present). Port-name deltas the
// .cpp adapts at call sites: mPhs->mPhase, mStts->mCcStts, mCyl->mCcCyl,
// getHeight/getR->getCollisionH/getCollisionR (wrapped below), no
// mpModelArrow[2] member (WW arrow-bundle doubles — see .cpp §327 note).
//
// The port base reads the TP dItem_data tables through VIRTUAL per-item getters
// (chkFlag/getTevFrm/getBtpFrm/getShadowSize/getCollisionH/getCollisionR).
// m_itemNo here carries WW DZR numbering, so this class OVERRIDES all six with
// the WW donor item_info values (subset table in the .cpp).
// ============================================================

#include "d/actor/d_a_itembase.h"
#include "SSystem/SComponent/c_phase.h"   // §327 cPhs_Step

class daSpcItem01_c : public daItemBase_c {
public:
    void set_mtx();
    BOOL _delete();
    cPhs_Step _create();  // §327 WW cPhs_State -> port cPhs_Step
    BOOL CreateInit();
    BOOL _execute();
    void set_effect();
    void scale_anim();
    void anim_play();
    void move();
    void rotate_item();
    void setCol();
    BOOL _draw();
    void setTevStr();  // §327 virtual override (port base declares it virtual)

    // §327 WW daItemBase_c::setLoadError VERBATIM (d_a_itembase_static.cpp:87:
    // cLib_onBit(mDrawFlags, 0x04)) — the port base dropped the method but kept
    // the flags byte (field_0x92b). No port consumer reads bit 4; kept faithful.
    void setLoadError() { field_0x92b |= 0x04; }

    // §327 donor-name wrappers: WW itembase getHeight/getR == port virtual
    // getCollisionH/getCollisionR (overridden in the .cpp with WW item_info).
    u8 getHeight() { return getCollisionH(); }
    u8 getR() { return getCollisionR(); }

    // §327 WW-numbered per-item getters (port base virtuals; TP tables misindex
    // for WW item numbers — see .cpp WW subset table).
    virtual BOOL chkFlag(int flag);
    virtual s8 getTevFrm();
    virtual s8 getBtpFrm();
    virtual u8 getShadowSize();
    virtual u8 getCollisionH();
    virtual u8 getCollisionR();

    /* Place member variables here */
    f32 field_0x63C;
    s16 field_0x640;
    u8 field_0x642;
    u8 field_0x643;
    u16 field_0x644;
    u8 field_0x646;
    u8 field_0x647;
};
// §327 donor STATIC_ASSERT(sizeof(daSpcItem01_c) == 0x648) dropped (x64 port).

#endif /* D_A_SPC_ITEM01_H */
