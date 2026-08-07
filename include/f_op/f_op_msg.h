#ifndef F_OP_F_OP_MSG_H
#define F_OP_F_OP_MSG_H

#include "SSystem/SComponent/c_xyz.h"
#include "f_pc/f_pc_leaf.h"

enum fopMsg_Mode_e {
    // ============================================================
    // §245 scope-message subsystem (WW fopMsgStts_* → port fopMsg_MODE_*).
    //   MSG_UNK0 (0x00) + CLOSE_WAIT (0x0A, alias of UNK_A) are the two
    //   scope states the port was missing. Needed by fopMsgM_forceSendOn /
    //   Aryll's (d_a_npc_ls1) telescope demo.
    // ============================================================
    fopMsg_MODE_MSG_UNK0_e      = 0x00,
    fopMsg_MODE_MSG_PREPARING_e = 0x01,
    fopMsg_MODE_BOX_OPENING_e   = 0x02,
    fopMsg_MODE_MSG_TYPING_e    = 0x06,
    fopMsg_MODE_CLOSE_WAIT_e    = 0x0A,  // §245 waiting for input before closing (== UNK_A)
    fopMsg_MODE_UNK_A_e         = 0x0A,
    // 512: donor names, learned at the re-baseline (donor f_op_msg.h:21-23).
    // Were UNK_B / UNK_D here. 0x0C is the donor's SCOPE_DEMO, added for
    // completeness -- unused by the port today, but it closes the gap rather
    // than leaving a hole someone re-derives later.
    fopMsg_MODE_SCOPE_ACTIVE_e  = 0x0B,  // scope active/idle
    fopMsg_MODE_SCOPE_DEMO_e    = 0x0C,  // scope demo (cutscene) mode
    fopMsg_MODE_SCOPE_WAIT_e    = 0x0D,  // scope waiting for event/demo
    fopMsg_MODE_MSG_DISPLAYED_e = 0x0E,
    fopMsg_MODE_MSG_CONTINUE_e  = 0x0F,
    fopMsg_MODE_MSG_END_e       = 0x10,
    fopMsg_MODE_BOX_CLOSING_e   = 0x11,
    fopMsg_MODE_BOX_CLOSED_e    = 0x12,
    fopMsg_MODE_MSG_DESTROYED_e = 0x13,
    fopMsg_MODE_UNK15_e         = 0x15,
};

class fopAc_ac_c;

struct msg_class {
    /* 0x00 */ leafdraw_class base;
    /* 0xC0 */ int type;
    /* 0xC4 */ create_tag_class draw_tag;
    /* 0xD8 */ leafdraw_method_class* sub_method;
    /* 0xDC */ fopAc_ac_c* talk_actor;
    /* 0xE0 */ cXyz pos;
    /* 0xEC */ u32 msg_idx;
    /* 0xF0 */ u32 field_0xf0;
    /* 0xF4 */ u32 field_0xf4;
    /* 0xF8 */ u16 mode;
    /* 0xFA */ u8 select_idx;
};  // Size: 0xFC

extern leafdraw_method_class g_fopMsg_Method;

namespace fopMsg {
    extern u8 MemCheck;
}

#endif /* F_OP_F_OP_MSG_H */
