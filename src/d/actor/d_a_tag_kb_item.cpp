// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_tag_kb_item.cpp + d/d_com_static.cpp:382-449 (kb_dig/dig_main)
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: MatchingFor
// KIT-PLUGIN: plugin-bound
// ============================================================
// WW PIG DIG-ITEM TAG (TAG_KB_ITEM, placement "TagKb") — §860 batch slot 5
// (recomposed §864: ki is enemy-class, obj_wood is d_wood-system-gated —
// both deferred with their gates named; this tag is the head's next clean
// row, 10 Outset placements). Donor WHOLE incl. the kb_dig/dig_main bodies
// from d_com_static (retail branch). LANDING THIS RETIRES the §225 pig
// shims: the fpcNm_TAG_KB_ITEM_e 0xFFFE sentinel + the daTagKbItem_c stub
// class in d_ext_ww_actor_shims.h (remove BOTH at land or the enum row
// redefines) — the pig's search_get_item then finds REAL tags and gains
// its dug-item food source, the intended donor behavior.
// SEAMS: [K1] item spawn through the §227 dExtKb_fastCreateItem boundary
// (the pig's own established item-spawn shim — same donor call shape);
// [K2] retail branch taken (switch-latched respawn law, VERSION>JPN).
// ============================================================

#include "d/actor/d_a_tag_kb_item_port.h"

#include "SSystem/SComponent/c_math.h"
#include "d/d_com_inf_game.h"
#include "d/d_ext_ww_actor_shims.h"   // [K1] dExtKb_fastCreateItem; NOTE: the
                                      // stub daTagKbItem_c + 0xFFFE sentinel in
                                      // this header RETIRE when this TU lands.
#include "f_op/f_op_actor_mng.h"
#include "dusk/logging.h"

#define fpcDwPi_TAG_KB_ITEM_e fpcDwPi_E_RD_e

void daTagKbItemPort_c::kb_dig(fopAc_ac_c* i_actor) {
    field_0x299 = 1;
    mpActor = i_actor;
    dig_main();
}

void daTagKbItemPort_c::dig_main() {
    // [K2] retail branch: switch-latched (VERSION > JPN).
    if (fopAcM_isSwitch(this, field_0x2a4)) {
        return;
    }
    if (field_0x299 == 0) {
        return;
    }
    if (field_0x298 != 0) {
        return;
    }

    cXyz scale_v = cXyz::Zero;
    csXyz angle = csXyz::Zero;
    bool r30 = false;
    if (!fopAcM_IsActor(mpActor)) {
        return;
    }
    if (fopAcM_GetName(mpActor) != fpcNm_KB_e) {
        // Not a pig.
        return;
    }
    if (field_0x2a0 != 0xFF) {
        angle.y = cM_rndF((f32)0x7FFE);
        s8 roomNo = current.roomNo;
        // [K1] the §227 boundary — the pig's own item-spawn shim carries the
        // donor call shape (pos, itemNo, room, angle, scale, speedF, speedY,
        // gravity); the save-bit rides the tag's own field like the donor.
        fopAc_ac_c* item = dExtKb_fastCreateItem(&mpActor->current.pos, field_0x2a0, roomNo,
                                                 &angle, &scale_v, cM_rndF(5.0f),
                                                 60.0f + cM_rndFX(5.0f), -6.0f);
        if (item) {
            fopAcM_OnStatus(item, fopAcStts_UNK4000_e);
        }
        if (field_0x2a4 != 0xFF) {
            fopAcM_onSwitch(this, field_0x2a4);
        } else {
            fopAcM_delete(this);
        }
        r30 = true;
    }

    if (field_0x2a1 != 0xFF) {
        mpActor->home.angle.z = field_0x2a1;
        fopAcM_onSwitch(this, field_0x2a4);
        fopAcM_delete(this);
        r30 = true;
    }

    if (r30) {
        field_0x298 = r30;
    }
    field_0x299 = 0;
}

bool daTagKbItemPort_c::_delete() {
    // [K2] retail branch.
    if (field_0x2a0 != 0xff && field_0x2a4 != 0xff) {
        dComIfGs_offSwitch(field_0x2a4, home.roomNo);
    }
    return true;
}

void daTagKbItemPort_c::CreateInit() {
    field_0x2a0 = fopAcM_GetParam(this);
    // receiver GetParamBit takes the ACTOR (§877 API shape).
    s8 paramBit = fopAcM_GetParamBit(this, 8, 8);
    field_0x29c = paramBit;
    field_0x2a1 = fopAcM_GetParamBit(this, 0x10, 8);
    field_0x2a4 = fopAcM_GetParamBit(this, 0x18, 8);
    field_0x298 = 0;
    field_0x299 = 0;
    mpActor = NULL;
}

cPhs_Step daTagKbItemPort_c::_create() {
    fopAcM_ct(this, daTagKbItemPort_c);

    CreateInit();
    if ((field_0x29c != 0x1f && fopAcM_isItem(this, field_0x29c)) ||
        (field_0x2a4 != 0xff && fopAcM_isSwitch(this, field_0x2a4))) {
        return cPhs_ERROR_e;
    }
    return cPhs_COMPLEATE_e;
}

bool daTagKbItemPort_c::_execute() {
    // [K2] retail branch: the JPN-only per-frame delete poll is compiled out.
    return true;
}

bool daTagKbItemPort_c::_draw() {
    return true;
}

static cPhs_Step daTagKbItem_Create(void* i_this) {
    return static_cast<daTagKbItemPort_c*>(i_this)->_create();
}

static BOOL daTagKbItem_Delete(void* i_this) {
    return static_cast<daTagKbItemPort_c*>(i_this)->_delete();
}

static BOOL daTagKbItem_Draw(void* i_this) {
    return static_cast<daTagKbItemPort_c*>(i_this)->_draw();
}

static BOOL daTagKbItem_Execute(void* i_this) {
    return static_cast<daTagKbItemPort_c*>(i_this)->_execute();
}

static BOOL daTagKbItem_IsDelete(void*) {
    return TRUE;
}

static actor_method_class daTagKbItemMethodTable = {
    (process_method_func)daTagKbItem_Create,
    (process_method_func)daTagKbItem_Delete,
    (process_method_func)daTagKbItem_Execute,
    (process_method_func)daTagKbItem_IsDelete,
    (process_method_func)daTagKbItem_Draw,
};

extern actor_process_profile_definition g_profile_TAG_KB_ITEM;

actor_process_profile_definition g_profile_TAG_KB_ITEM = {
    // donor g_profile_TAG_KB_ITEM (d_a_tag_kb_item.cpp:95).
    fpcLy_CURRENT_e,           // Layer ID
    7,                         // List ID (donor 0x0007)
    fpcPi_CURRENT_e,           // List Prio
    fpcNm_TAG_KB_ITEM_e,       // Proc Name (REAL enum row at land — shim sentinel retires)
    &g_fpcLf_Method.base,      // Proc SubMtd
    sizeof(daTagKbItemPort_c), // Size
    0,                         // Size Other
    0,                         // Parameters
    &g_fopAc_Method.base,      // Leaf SubMtd
    fpcDwPi_TAG_KB_ITEM_e,     // Draw Prio
    &daTagKbItemMethodTable,   // Actor SubMtd
    fopAcStts_UNK40000_e,      // Status (donor verbatim)
    fopAc_ACTOR_e,             // Group
    fopAc_CULLBOX_0_e,         // Cull Type
};
