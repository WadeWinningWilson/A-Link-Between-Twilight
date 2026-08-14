#ifndef D_A_OBJ_SHELF_H
#define D_A_OBJ_SHELF_H

// ============================================================
// WW WOODEN SHELF (daObjShelf) — donor header mirrored whole on the
// receiver's own dBgS_MoveBgActor base (same lineage class, present
// natively). Port: tale §835. Solid MoveBG geometry: 8 placements in
// Sturgeon's room (Ojhous2/Room1) — §817-2.
// ============================================================

#include "d/d_bg_s_movebg_actor.h"
#include "d/d_a_obj.h"
#include "SSystem/SComponent/c_phase.h"

namespace daObjShelf {
    class Act_c : public dBgS_MoveBgActor {
    public:
        static Mtx M_tmp_mtx;

        enum Prm_e {
            PRM_GROUNDMA_W = 0x01,
            PRM_GROUNDMA_S = 0x00,
        };

        bool prm_get_groundma() const {
            return daObj::PrmAbstract<int>(this, PRM_GROUNDMA_W, PRM_GROUNDMA_S) != 0;
        }

        virtual int CreateHeap();
        virtual int Create();
        cPhs_Step Mthd_Create();
        int Delete_();   // port: base has no virtual Delete slot by this name
        BOOL Mthd_Delete();
        void hold_event() const;
        void mode_wait_init();
        void mode_wait();
        void mode_vib_init();
        void mode_vib();
        void mode_rot_init();
        void mode_rot_init2();
        void mode_rot_init3();
        void mode_rot();
        void mode_fell_init();
        void mode_fell();
        void set_mtx();
        void init_mtx();
        virtual int Execute(Mtx**);
        virtual int Draw();

    public:
        static const char M_arcname[6];

        request_of_phase_process_class mPhs;
        J3DModel* mpModel;
        int mMode;
        float mRotSpeed;
        short mTargetAngle;
        short mTimer;
        short mVibY;
        short mVibX;
        short mVibZ;
        s8 mCurBounce;
        bool m2e7;
    };
};

#endif /* D_A_OBJ_SHELF_H */
