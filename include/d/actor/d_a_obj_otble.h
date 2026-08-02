#ifndef D_A_OBJ_OTBLE_H
#define D_A_OBJ_OTBLE_H

/**
 * ============================================================================
 * §329 WW WOODEN TABLE (Obj_Otble) — Actor-Kit port, header VERBATIM from the
 * WW donor (D:/XXXXXXX/WW DP/include/d/actor/d_a_obj_otble.h). Layout kept
 * donor-exact; TP-native bases (fopAc_ac_c, dBgS_*) are the same lineage.
 * ============================================================================
 */

#include "f_op/f_op_actor.h"
#include "d/d_bg_s_acch.h"

class dBgW;

namespace daObj_Otble {
    struct Attr_c {
        u8 m00;
        u8 m01;
    }; // size = 0x2

    class Act_c : public fopAc_ac_c {
    public:
        cPhs_Step _create();
        bool _delete();
        const Attr_c* attr() const { return &M_attr; }

        void set_mtx();
        BOOL _execute();
        BOOL _draw();
        BOOL _createHeap();
        void CreateInit();

        static const Attr_c M_attr;

    public:
        J3DModel* mModel;
        s32 m294;
        request_of_phase_process_class mPhase;
        dBgW* mBgW;
        Mtx m2A4;
        dBgS_ObjAcch mObjAcch;
        dBgS_AcchCir mObjAcchCir;
    };

    namespace Mthd {
        cPhs_Step Create(void*);
        BOOL Delete(void*);
        BOOL Execute(void*);
        BOOL Draw(void*);
        BOOL IsDelete(void*);
        extern actor_method_class Table;
    };
};

#endif /* D_A_OBJ_OTBLE_H */
