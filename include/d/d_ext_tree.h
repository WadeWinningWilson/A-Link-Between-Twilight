#ifndef D_EXT_TREE_H
#define D_EXT_TREE_H

// ============================================================
// §366 — the donor SWOOD packet (WW d_tree.cpp) — Foundry's §242 order.
// Housing's §231 corrections are the spec: d_tree (NOT d_wood), 5 DLs +
// l_modelStatus {0,1,2 / 3,1,4} + shadow pass; blobs staged decomp-exact
// at assets/veg/d_tree__*.bin. Class layout donor-verbatim (d_tree.h).
// Entry points for the vegetation dispatcher (kind == 1 rows):
//   dExtTree_newData(pos, roomNo)       — register a swood placement
//   dExtTree_execute(player, roomNo)    — per-frame logic (guarded 1/frame)
//   dExtTree_entryDraw()                — per-frame packet entry (1/frame)
//   dExtTree_deleteRoom(roomNo)         — room teardown
// ============================================================

#include "f_op/f_op_actor.h"
#include "SSystem/SComponent/c_xyz.h"
#include "d/d_cc_uty.h"
#include "JSystem/J3DGraphBase/J3DPacket.h"

class dExtTree_data_c;

class dExtTree_anm_c {
public:
    dExtTree_anm_c() { mState = 0; }
    /* donor dTree_anm_c, field-for-field (d_tree.h:46) */
    u8 mState;
    s16 field_0x02;
    s16 field_0x04;
    s16 field_0x06;
    s16 field_0x08;
    s16 field_0x0a;
    f32 mAnimTimer;
    f32 field_0x10;
    f32 field_0x14;
    f32 field_0x18;
    f32 field_0x1c;
    f32 field_0x20;
    Mtx field_0x24;
    Mtx field_0x54;
};

class dExtTree_data_c {
public:
    dExtTree_data_c();

    void WorkCo(fopAc_ac_c* i_actor, int i_roomNo);
    void WorkAt_NoCutAnim(fopAc_ac_c* i_actor, int i_roomNo, cCcD_Obj* i_hitObj);
    void WorkAt(fopAc_ac_c* i_actor, int i_roomNo, dCcMassS_HitInf* i_hitInf);
    BOOL hitCheck(fopAc_ac_c* i_player, int i_roomNo);
    void animation(int i_roomNo);

    /* donor dTree_data_c, field-for-field (d_tree.h:20) */
    u8 mState;
    u8 field_0x001;   // 0 = intact, 1 = cut (l_modelStatus row select)
    s8 mAnimIdx;
    u8 field_0x003;   // upper-part alpha (0xFF intact; fades after fall)
    cXyz mPos;
    Mtx field_0x010;  // view * upper(anim) mtx
    Mtx field_0x040;  // view * trunk mtx
    Mtx field_0x070;  // frozen fallen-upper world mtx
    Mtx mShadowMtx;
    Mtx field_0x0d0;  // view * shadow mtx
    dExtTree_data_c* mpNext;
};

class dExtTree_room_c {
public:
    dExtTree_room_c() { mpData = NULL; }
    void newData(dExtTree_data_c* i_data);
    void deleteData();
    dExtTree_data_c* getData() { return mpData; }
    dExtTree_data_c* mpData;
};

class dExtTree_packet_c : public J3DPacket {
public:
    dExtTree_packet_c();
    virtual ~dExtTree_packet_c() {}
    virtual void draw();  // §366 delta #1: donor update()'s view-mtx math runs
                          // HERE (draw phase) — Housing's stale-view law

    void calc(fopAc_ac_c* i_player, int i_roomNo);  // execute-phase logic
    dExtTree_data_c* newData(cXyz& i_pos, u8 i_cut, int i_roomNo);
    void setData(dExtTree_data_c* i_data, int i_nextIdx, cXyz& i_pos, u8 i_cut,
                 int i_roomNo);
    s32 newAnm(s16 i_baseAng);
    void setAnm(int i_no, s16 i_baseAng);
    void deleteAnm(int idx) { mAnm[idx].mState = 0; }
    void deleteRoom(int roomNo) { mRoom[roomNo].deleteData(); }
    dExtTree_anm_c* getAnm(int idx) { return &mAnm[idx]; }
    dExtTree_anm_c* getAnm() { return mAnm; }
    dExtTree_data_c* getData() { return mData; }
    s16 getPlayerSwordMoveAngY() { return mPlayerSwordMoveAngY; }

    /* donor dTree_packet_c capacities verbatim (d_tree.h:112) */
    u16 mNextIdx;
    dExtTree_data_c mData[64];
    dExtTree_anm_c mAnm[72];   // 0-7 ambient sway slots, 8+ dynamic
    dExtTree_room_c mRoom[64];
    s16 mPlayerSwordMoveAngY;
    cXyz mPlayerSwordTop;
};

// dispatcher surface (d_a_ext_vegetation kind == 1)
dExtTree_data_c* dExtTree_newData(cXyz& i_pos, int i_roomNo);
void dExtTree_execute(fopAc_ac_c* i_player, int i_roomNo);
void dExtTree_entryDraw();
void dExtTree_deleteRoom(int i_roomNo);
bool dExtTree_assetsReady();

#endif /* D_EXT_TREE_H */
