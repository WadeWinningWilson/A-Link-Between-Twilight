// ============================================================
// §223 Pass 1 — direct-port shims (see d_ext_ww_actor_shims.h). First-pass
// bodies: functional where cheap (ground-angle flat), no-op where cosmetic or
// gated on unrestored mechanics (figurine gallery, grab/throw, attack-collision
// bits). Pass 2 wires the real bodies as the d_a_kb source drop-in needs them.
// ============================================================
// KIT-LINEAGE: bridge-owed:§223
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
#include "d/d_ext_ww_actor_shims.h"
#include "d/d_item_data.h"  // §226 dItemNo_NONE_e (neutral equip-query result)
#include "f_op/f_op_actor_mng.h"  // §227 fopAcM_fastCreateItem (port signature)
#include "JSystem/J3DGraphAnimator/J3DModel.h"  // §239 J3DModel::getWeightAnmMtx (setter adapter)
#include "m_Do/m_Do_ext.h"        // §253 mDoExt_McaMorf::setAnm/isStop (dLib_bcks_setAnm)
#include "d/d_com_inf_game.h"     // §253 dComIfG_getObjectRes (dLib_bcks_setAnm)

// Flat ground angle (no slope-lean yet — the pig walks level first pass).
BOOL fopAcM_getGroundAngle(fopAc_ac_c* actor, csXyz* p_angle) {
    (void)actor;
    if (p_angle != NULL) {
        p_angle->x = 0;
        p_angle->y = 0;
        p_angle->z = 0;
    }
    return TRUE;
}

// Nintendo-Gallery figurine registration — not restored.
void dSnap_RegistFig(u8 /*type*/, fopAc_ac_c* /*actor*/, f32 /*sx*/, f32 /*sy*/, f32 /*sz*/) {}

// Pig size/skin material swap — Pass 2 routes through the mount BMT path.
void dExtKb_setMaterialTable(J3DModelData* /*md*/, J3DMaterialTable* /*bmt*/) {}

// Ground material code under the pig — §227 returns 0, which equals none of the
// dBgS_Attr_* sentinels (1..4), so the pig's dig-material switch is inert.
s32 dExtKb_GetAttributeCode(const cBgS_PolyInfo& /*gnd*/) {
    return 0;
}

// Attack-collision bits — only matter when the pig is hittable (deferred).
void dExtKb_OnAtHitBit(dCcD_Sph* /*sph*/) {}
void dExtKb_ClrAtSet(fopAc_ac_c* /*actor*/) {}

// Grab/throw target — carry mechanic; no grab first pass.
fopAc_ac_c* dExtKb_getGrabMissActor() {
    return NULL;
}

// --- esa (bait) sea/water shims — no sea system → bait always lands on ground.
bool daSea_ChkArea(f32 /*x*/, f32 /*z*/) {
    return false;
}
f32 daSea_calcWave(f32 /*x*/, f32 /*z*/) {
    return -1.0e30f;  // effectively no wave
}
f32 dBgS_GetWaterHeight(cXyz& /*pos*/) {
    return -1.0e30f;  // no water under the bait → ground-land path (mState 1)
}

// ============================================================
// §225 Pass 2 — pig (d_a_kb) particle emitters the port lacks. Return NULL so the
// pig's `if(cb.getEmitter()==NULL) set*(...)` guard treats them as never-emitting;
// no visual, no crash. Deferred to the wave-emit pass (same missing WW bank as the
// ship wake). No behavior depends on these — they are dust/ripple cosmetics.
// ============================================================
JPABaseEmitter* dComIfGp_particle_setShipTail(u16 /*id*/, const cXyz* /*pos*/, const csXyz* /*angle*/,
                                              const cXyz* /*scale*/, u8 /*alpha*/, dPa_rippleEcallBack* /*cb*/) {
    return NULL;
}
JPABaseEmitter* dComIfGp_particle_setToon(u16 /*id*/, const cXyz* /*pos*/, const csXyz* /*angle*/,
                                          const cXyz* /*scale*/, u8 /*alpha*/, dPa_smokeEcallBack* /*cb*/, int /*roomNo*/) {
    return NULL;
}

// ============================================================
// §226 Pass 2 — pig (d_a_kb) compile-only bodies. All no-op/sentinel: deferred
// combat (cc_at_check, equip query) and cosmetic J3D tex/material swaps that the
// port's J3D layer doesn't expose. No pig behavior depends on these first pass.
// ============================================================

// Collision attack-check — the pig registers a Tg hit but takes no damage yet.
void cc_at_check(fopAc_ac_c* /*actor*/, CcAtInfo* /*atInfo*/) {}

// Equipped-item slot query — neutral value keeps the damage branches inert.
int dComIfGs_getSelectEquip(int /*slot*/) {
    return dItemNo_NONE_e;
}

// Eye-blink tex-pattern animator + skin/size material swap — cosmetic no-ops.
void dExtKb_setTexNoAnimator(J3DModelData* /*md*/, J3DAnmTexPattern* /*pat*/, J3DTexNoAnm* /*txm*/) {}
void dExtKb_setAnmTexPattern(J3DTexNoAnm* /*txm*/, J3DAnmTexPattern* /*pat*/) {}

// §227 Item drop on dig. The port's fopAcM_fastCreateItem takes pointer speeds,
// no gravity param, and trailing (itemBitNo=-1, param_9=0, createFunc=NULL) —
// see d_a_npc_kasi_hana / d_a_npc_ykw. Gravity is dropped (port applies its own).
fopAc_ac_c* dExtKb_fastCreateItem(const cXyz* pos, int itemNo, int roomNo,
                                  const csXyz* angle, const cXyz* scale,
                                  f32 speedF, f32 speedY, f32 /*gravity*/) {
    f32 sf = speedF;
    f32 sy = speedY;
    return fopAcM_fastCreateItem(pos, itemNo, roomNo, angle, scale, &sf, &sy, -1, 0, NULL);
}

// J3DTexNoAnm::setAnmIndex — no-op; the pig's tex-pattern animator is no-op'd,
// so the per-material index never feeds a live animator.
void dExtKb_setAnmIndex(J3DTexNoAnm* /*txm*/, u16 /*idx*/) {}

// ============================================================
// §232 Pass 2 — d_a_kamome (SEAGULL) donor-gap bodies. The seagull uses this only
// on the ship-following (heisou) path, which is dead in the port (no KoRL boat).
// "No enemy sound" is the neutral default that keeps the donor logic faithful.
// ============================================================
bool dComIfGp_att_chkEnemySound() {
    return false;
}

// ============================================================
// §239 Pass 1 — WW NPC framework (fopNpc_npc_c + dNpc_*) donor-gap bodies.
// See d_ext_ww_actor_shims.h §239. cLib_addCalcAngleL is VERBATIM donor; the
// reserve-item queries return the "nothing reserved" default; the weight-anm-mtx
// setter mirrors the port's own J3DMtxBuffer::setAnmMtx MTXCopy idiom.
// ============================================================

// VERBATIM WW donor (c_lib.cpp cLib_addCalcAngleL__FPlllll) — s32 twin of the
// port's s16 cLib_addCalcAngleS easing.
s32 cLib_addCalcAngleL(s32* pValue, s32 target, s32 scale, s32 maxStep, s32 minStep) {
    s32 diff = target - *pValue;
    if (*pValue != target) {
        scale = (diff) / scale;
        if (scale > minStep || scale < -minStep) {
            if (scale > maxStep) {
                scale = maxStep;
            }
            if (scale < -maxStep) {
                scale = -maxStep;
            }
            *pValue += scale;
        } else {
            if (0 <= diff) {
                *pValue += minStep;
                if (0 >= target - *pValue) {
                    *pValue = target;
                }
            } else {
                *pValue -= minStep;
                if (0 <= target - *pValue) {
                    *pValue = target;
                }
            }
        }
    }
    return diff;
}

// Reserved-item queries — the port has no reserve system → "nothing reserved".
BOOL dComIfGs_isGetItemReserve(u8 /*i_no*/) {
    return FALSE;
}
u8 dComIfGs_checkReserveItem(u8 /*i_itemNo*/) {
    return 0;
}

// J3DModel::setWeightAnmMtx adapter — the port ships only the getter (returns the
// weight-envelope matrix pointer); copy into it exactly as J3DMtxBuffer::setAnmMtx
// does. Used by dNpc_setShadowModel to mirror the target model's skinning mtxs.
void dExtNpc_setWeightAnmMtx(J3DModel* self, int idx, MtxP mtx) {
    MTXCopy(mtx, self->getWeightAnmMtx(idx));
}

// ============================================================
// §246 d_a_npc_bm1 (Generic Ritos) donor-gap bodies. All faithful-in-effect:
// the WW subsystems they stand in for (symbols, beast counter, letter, demo,
// figurine, Rito feather FX) are not ported, and these Ritos do not spawn in the
// port, so every body here is on a runtime-dead path. See d_ext_ww_actor_shims.h.
// ============================================================
#include "m_Do/m_Do_ext.h"  // §246 mDoExt_McaMorf full type (entryDL adapter)
#include "f_op/f_op_msg.h"   // §246 fopMsg_MODE_* (msg-status shim)
#include "d/d_demo.h"        // §244 dDemo_c::getActor (real demo-actor lookup)
#include "d/d_stage.h"       // §244 dStage_roomControl_c::getDemoArcName (demo arc)
#include "d/d_com_inf_game.h" // §244 dComIfG_getObjectIDRes (resolve demo btp/btk)

// Message-state accessors — port msg_class is a stub; return inert values
// (runtime-dead: these Ritos never talk). "Still typing" is talk_1's no-op case.
u16 dExtNpcBm1_msgStatus(msg_class* /*m*/) {
    return fopMsg_MODE_MSG_TYPING_e;
}
s16 dExtNpcBm1_msgSelectNum(msg_class* /*m*/) {
    return 0;
}

// Sound-anim stop on delete — port McaMorf has no stopZelAnime → no-op.
void dExtNpcBm1_stopZelAnime(mDoExt_McaMorf* /*m*/) {}

// §251 Demo FACE-EXPRESSION reconstruction RETIRED — promoted to the true native
// subsystem. Foundry decoded the donor's getP_BtpData/getP_BtkData/getP_BrkData from
// the donor binary (docs/WW Linked/getP_BtpData-decode.md); the native methods now
// live on dDemo_actor_c (src/d/d_demo.cpp §251) and every cutscene actor (Aryll, bm1)
// calls them directly. The reconstruction here (dExtNpcBm1_getDemoBtp / dExtLs1_getDemoBtk)
// is deleted — standing directive: bridges get replaced by the native subsystem.
// (Confirmations from the decode: "rebind only on resID change" IS native donor
// behavior, and the id's 0x10000 bit natively selects the demo archive.)

// WW din-symbol query — no symbol system → "not shown".
BOOL dComIfGs_isSymbol(u8 /*i_no*/) {
    return FALSE;
}

// WW golden-feather / beast counter — none collected → 0.
u8 dComIfGs_getBeastNum(int /*i_beastIdx*/) {
    return 0;
}

// WW beast-count adjust — nothing to spend → no-op.
void dComIfGp_setItemBeastNumCount(int /*i_beastIdx*/, s16 /*num*/) {}

// ============================================================================
// WW MESSAGE ANIME-TAG CHANNEL — PORTED FROM THE DONOR'S OWN MECHANISM.
//
// WAS: `getMesgAnimeTagInfo() { return 0xFF; }` beside a no-op setter, with the
// honest reason recorded at the shim — *"the WW dialogue-anim-tag mechanism has
// no port driver, so getter stays 0xFF and Aryll's chngAnmTag() path is inert."*
// THAT REASON IS NOW OBSOLETE: the derived message tier's group-3 (anime) tag IS
// the driver, and a constant getter would make it fail SILENTLY — no fault, no
// log, the NPC simply never changes animation on a dialogue beat.
//
// THE DONOR'S MECHANISM, read at source rather than inferred:
//   field     `u8 mMesgAnimeTagInfo` @ 0x493C (donor d_com_inf_game.h:826)
//   accessors getNowAnimeID / setNowAnimeID / clearNowAnimeID   (:684-686)
//   sentinel  **0xFF means EMPTY** — clearNowAnimeID() writes exactly that
//
// IT IS A ONE-SLOT MAILBOX, and the CONSUMER does the clearing. Donor consumer,
// d_a_npc_ls1.cpp:1076 (npc_ba1, bm1, hi1, jb1, km1, zl1 all identical):
//     tag = dComIfGp_getMesgAnimeTagInfo();
//     if (tag != 0xFF && tag != mMesgAnimeTag) {
//         dComIfGp_setMesgAnimeTagInfo(0xFF);   // consume
//         mMesgAnimeTag = tag;
//         chngAnmTag();
//     }
// So the setter is called by BOTH ends — the engine posts an ID, the actor
// posts 0xFF back to consume it. A no-op setter breaks both halves, which is
// why the getter alone could never have been enough.
//
// MODULE-STATIC, NOT A GAME-INFO FIELD — the offset-stable law (same rule that
// keeps §308's archives in module slots). The donor's 0x493C does not exist in
// the port's fixed struct and must not be carved into it.
//
// STILL A SHIM IN ONE RESPECT, stated so it is not mistaken for done: this
// ports the CHANNEL, not the tag SOURCE. Until the derived tier writes group-3
// tags, nothing posts an ID and the mailbox stays empty — behaviour identical to
// today. The difference is that it is now WIRED rather than welded shut.
// ============================================================================
static u8 s_mesgAnimeTagInfo = 0xFF;  // 0xFF = empty, per the donor's sentinel

// ============================================================================
// WW BMG MESSAGE LOOKUP **BY mMsgNo** — the donor's own resolution, ported.
//
// WHY THIS EXISTS SEPARATELY FROM §308's `dExtDmesg_getMessageById`: THEY ARE
// TWO DIFFERENT NUMBERING SPACES AND BOTH ARE REAL.
//   · STB `setMessageCode` IS an INF1 ROW INDEX. Proven, not assumed: INF1[539]
//     is "I've been waiting for you..." exactly as §308's receipt says, AND no
//     entry in zel_00.bmg carries mMsgNo == 539 at all. §308 indexes; correct.
//   · An ACTOR's message id (what it hands fopMsgM_messageSet) is an **mMsgNo**.
//     Donor `fopMsgM_itemMsgGet_c::getMessage` (8002E4AC) LINEAR-SCANS INF1
//     comparing mMsgNo and SKIPPING entries whose mDataOffs == 0 (85 such in
//     zel_00). getMesgHeader -> that scan. There is no index arithmetic anywhere.
//
// Measured cost of conflating them, on the user's disc: id 0xCE5 (the postbox)
// indexes to INF1[3301] = "Seven-Star Isles" but SCANS to INF1[924] =
// "Gooood moooorrrning!". A wrong-but-valid index returns TEXT, so the failure
// is silent and the wrong line beats the correct fallback.
//
// NO TABLE. The donor needs none and a table is the thing that drifts.
// ============================================================================
static const u8* dExtWwMsg_section(const u8* bmg, const char* magic) {
    if (bmg == NULL) {
        return NULL;
    }
    u32 off = 0x20;  // past the BMG header; blocks are magic + u32 size
    for (int guard = 0; guard < 64; guard++) {
        const u8* b = bmg + off;
        const u32 size = ((u32)b[4] << 24) | ((u32)b[5] << 16) | ((u32)b[6] << 8) | b[7];
        if (b[0] == magic[0] && b[1] == magic[1] && b[2] == magic[2] && b[3] == magic[3]) {
            return b;
        }
        if (size == 0) {
            break;
        }
        off += size;
    }
    return NULL;
}

// Donor-faithful mMsgNo -> DAT1 text. NULL when the archive is not resident or
// no entry carries the number. Public so the §324 talk path can be corrected
// without a second implementation being written somewhere else.
const char* dExtWwMsg_textByMsgNo(u16 i_msgNo) {
    // Hylian branch, donor fopMsgM_hyrule_language_check (8002AD4C): before the
    // game is cleared, fifteen specific messages come from zel_01.bmg instead.
    // Selected DATA-SIDE here rather than from a transcribed list of fifteen
    // numbers: those entries are exactly the ones carrying mTextboxType == 12,
    // measured set-equal against the donor's own switch and against zel_01's
    // INF1. Porting the mechanism beats copying the table.
    const u8* zel00 = (const u8*)dComIfG_getObjectRes("bmgres", "zel_00.bmg");
    const u8* inf1 = dExtWwMsg_section(zel00, "INF1");
    const u8* dat1 = dExtWwMsg_section(zel00, "DAT1");
    if (inf1 == NULL || dat1 == NULL) {
        return NULL;
    }
    const u16 count = (u16)(((u16)inf1[0x08] << 8) | inf1[0x09]);
    for (u16 i = 0; i < count; i++) {
        const u8* e = inf1 + 0x10 + (u32)i * 24;
        const u32 dataOffs = ((u32)e[0] << 24) | ((u32)e[1] << 16) | ((u32)e[2] << 8) | e[3];
        if (dataOffs == 0) {
            continue;  // donor skips these explicitly
        }
        const u16 msgNo = (u16)(((u16)e[0x04] << 8) | e[0x05]);
        if (msgNo != i_msgNo) {
            continue;
        }
        if (e[0x0C] == 12 && dComIfGs_getClearCount() == 0) {
            const u8* zel01 = (const u8*)dComIfG_getObjectRes("bmgresh", "zel_01.bmg");
            const u8* hi = dExtWwMsg_section(zel01, "INF1");
            const u8* hd = dExtWwMsg_section(zel01, "DAT1");
            if (hi != NULL && hd != NULL) {
                const u16 hn = (u16)(((u16)hi[0x08] << 8) | hi[0x09]);
                for (u16 j = 0; j < hn; j++) {
                    const u8* he = hi + 0x10 + (u32)j * 24;
                    const u16 hno = (u16)(((u16)he[0x04] << 8) | he[0x05]);
                    const u32 ho = ((u32)he[0] << 24) | ((u32)he[1] << 16) | ((u32)he[2] << 8) | he[3];
                    if (hno == i_msgNo && ho != 0) {
                        return (const char*)(hd + 0x08 + ho);
                    }
                }
            }
            // Hylian variant missing → fall through to the readable line rather
            // than returning nothing. A missing variant must not mute the text.
        }
        return (const char*)(dat1 + 0x08 + dataOffs);
    }
    return NULL;
}

// ============================================================================
// WW PASSWORD RENDER — donor `fopMsgM_passwordGet` (8002BE04), ported.
//
// Renders a message to PLAIN TEXT with the player-name tag substituted and all
// other tags stripped. knob00 compares the result against the entered password,
// so a stub returning "" made every comparison fail — the §328 banner said so.
//
// DONOR SEMANTICS, kept exactly:
//   · 0x1A escape: read size at +1, group at +2, code at +3..4 (u16 BE). Group 0
//     / code 0 substitutes the player name; every other tag is SKIPPED, not
//     rendered. Advance by the tag's own size byte.
//   · USA possessive: when the PAL language is 1 and the message number is one
//     of SIX specific values, append "'" after a trailing s/S/z/Z/x/X else "s".
//     Same six the donor lists, and the same six d_mesg.cpp:457 uses.
//   · Shift-JIS lead bytes (high nibble 8 or 9) copy TWO bytes, not one.
//
// READ BYTE-WISE ON PURPOSE: the donor does `*(u32*)(++src)` because GameCube is
// big-endian. That cast byte-swaps on x64 and would compare the tag against a
// reversed value, matching nothing — the class of bug that looks like "the tag
// is just never present".
// ============================================================================
void dExtWwMsg_passwordGet(char* o_buf, u32 i_msgNo) {
    if (o_buf == NULL) {
        return;
    }
    o_buf[0] = '\0';
    const char* src = dExtWwMsg_textByMsgNo((u16)i_msgNo);
    if (src == NULL) {
        return;  // not resident / no such message — caller sees an empty compare
    }

    const u8* p = (const u8*)src;
    char* dst = o_buf;
    while (*p != 0x00) {
        if (*p == 0x1A) {
            const u8 size = p[1];
            const u8 group = p[2];
            const u16 code = (u16)(((u16)p[3] << 8) | p[4]);
            if (group == 0x00 && code == 0x0000) {
                char name[24];
                const char* nm = dComIfGs_getPlayerName();
                int n = 0;
                for (; nm != NULL && nm[n] != '\0' && n < 20; n++) {
                    name[n] = nm[n];
                }
                name[n] = '\0';
                if (dComIfGs_getPalLanguage() == 1 &&
                    (i_msgNo == 0x33B || i_msgNo == 0xC8B || i_msgNo == 0x1D21 ||
                     i_msgNo == 0x31D7 || i_msgNo == 0x37DD || i_msgNo == 0x37DE) &&
                    n > 0) {
                    const char last = name[n - 1];
                    const bool sibilant = (last == 's' || last == 'S' || last == 'z' ||
                                           last == 'Z' || last == 'x' || last == 'X');
                    name[n++] = sibilant ? '\'' : 's';
                    name[n] = '\0';
                }
                for (int k = 0; k < n; k++) {
                    *dst++ = name[k];
                }
            }
            p += (size != 0) ? size : 1;  // size 0 would spin forever
            continue;
        }
        const u8 hi = (u8)((*p >> 4) & 0xF);
        if (hi == 8 || hi == 9) {
            *dst++ = (char)*p++;  // Shift-JIS lead byte: two bytes travel together
            if (*p == 0x00) {
                break;
            }
        }
        *dst++ = (char)*p++;
    }
    *dst = '\0';
}

u8 dComIfGp_getMesgAnimeTagInfo() {
    return s_mesgAnimeTagInfo;
}

void dComIfGp_setMesgAnimeTagInfo(u8 i_id) {
    s_mesgAnimeTagInfo = i_id;
}

// ⚠ LIFETIME GAP — NAMED, THEN CLOSED. WIRED 2026-08-16.
//
// The donor's mMesgAnimeTagInfo lives INSIDE g_dComIfG_gameInfo, so it dies and
// resets with the game-info. A module-static does NOT: it outlives every stage
// change for the life of the process.
//
// THE WINDOW: engine posts an ID -> stage changes BEFORE the NPC consumes it ->
// the stale ID survives -> the first NPC to poll in the NEXT stage sees a value
// that is neither 0xFF nor its own last tag, and plays ONE WRONG ANIMATION.
// Narrow, but it is exactly the class that reads as "a weird one-off glitch"
// and costs a debugging session to trace back to a mailbox that never emptied.
//
// CALLED FROM ww_stage_loader.cpp:467 (dExtWwStage_loadStageDzs), inside
// DUSK_WW_STAGE_SEAM, immediately after the tale-§773 model-cache eviction.
// That site was already the estate's answer to "stage-scoped state dies here",
// so this rides an established lifecycle rather than inventing one — and it is
// WW-scoped by construction, which was the reason it was parked in the first
// place. The parking was right; the assumption that no WW-only seam existed
// was not.
void dExtWwShims_resetMesgAnimeTag() {
    s_mesgAnimeTagInfo = 0xFF;
}

// WW postman-0 status-map clear — status-map not restored → no-op.
void fopAcM_ClearStatusMap(fopAc_ac_c* /*actor*/) {}

// WW letter send — dLetter subsystem not ported → no-op (event_proc case 2 dead).
void dLetter_send(u16 /*i_no*/) {}

// WW demo-actor lookup. Maps an actor's demoActorID → its bound dDemo_actor_c via
// the port's own accessor (d_demo.cpp:338 uses dDemo_c::getActor identically). §246
// bm1's Ritos never bind (id 0 → NULL, matching the old stub). §244 Aryll's awake /
// telescope demo DOES bind (id != 0 → the real actor) — returning NULL there NULL-
// derefs in daNpc_Ls1_c::demo()->checkEnable(). id 0 is guarded (unbound → NULL).
dDemo_actor_c* dComIfGp_demo_getActor(u8 id) {
    if (id == 0) {
        return NULL;
    }
    return dDemo_c::getActor(id);
}

// Material-table entryDL adapter — port McaMorf::entryDL() takes no arg. The
// recolor bmt (bm02/bmarm02) is DEFERRED like the pig's BMT swap; submit the base
// DL so the arm-having Rito variants still render (base palette, not the variant).
void dExtNpcBm1_entryDL_mat(mDoExt_McaMorf* morf, J3DMaterialTable* /*bmt*/) {
    if (morf != NULL) {
        morf->entryDL();
    }
}

// WW Rito feather particle emit — no feather bank in the port → no-op (NULL
// emitter; each call site null-checks the result). Deferred cosmetic FX.
JPABaseEmitter* dExtNpcBm1_particleSet(u16 /*id*/, const void* /*pos*/, const void* /*rot*/,
                                       const void* /*scale*/, int /*alpha*/, const void* /*cb*/,
                                       s8 /*room*/, const void* /*prmColor*/,
                                       const void* /*envColor*/) {
    return NULL;
}
JPABaseEmitter* dExtNpcBm1_particleSet(u16 /*id*/, const void* /*pos*/, const void* /*rot*/,
                                       const void* /*scale*/, int /*alpha*/, const void* /*cb*/,
                                       s8 /*room*/) {
    return NULL;
}

// ============================================================================
// §253  Rito Postbox (d_a_obj_toripost) shim bodies.
// ============================================================================

// dLib anim-param BCK driver — VERBATIM from the WW donor (src/d/d_lib.cpp). Generic
// McaMorf BCK setter; no toripost-specific behavior. Native port McaMorf setAnm/isStop.
void dLib_bcks_setAnm(const char* arcName, mDoExt_McaMorf* morf, s8* pAnmIdx, s8* pPrmIdx,
                      s8* pOldPrmIdx, const int* bcksTbl, const dLib_anm_prm_c* anmPrmTbl, bool force) {
    if ((*pOldPrmIdx != *pPrmIdx && anmPrmTbl[*pPrmIdx].mAnmIdx != -1) || force == true) {
        *pAnmIdx = anmPrmTbl[*pPrmIdx].mAnmIdx;
        J3DAnmTransform* bck = (J3DAnmTransform*)dComIfG_getObjectRes(arcName, bcksTbl[*pAnmIdx]);
        morf->setAnm(bck, anmPrmTbl[*pPrmIdx].mLoopMode, anmPrmTbl[*pPrmIdx].mMorf,
                     anmPrmTbl[*pPrmIdx].mPlaySpeed, 0.0f, -1.0f, NULL);
    }

    *pOldPrmIdx = *pPrmIdx;

    if (morf->isStop()) {
        if (anmPrmTbl[*pPrmIdx].mNextPrmIdx != -1 && anmPrmTbl[*pPrmIdx].mLoopMode == J3DFrameCtrl::EMode_NONE) {
            *pPrmIdx = anmPrmTbl[*pPrmIdx].mNextPrmIdx;
        }
    }
}

// WW letter subsystem — EMPTY-faithful stubs (BRIDGE-OWED: real WW mail port required).
bool dLetter_isStock(u16 /*i_no*/) { return false; }
void dLetter_read(u16 /*i_no*/) {}
void dLetter_autoStock(u16 /*i_no*/) {}
bool dLetter_isDelivery(u16 /*i_no*/) { return false; }

// Cosmetic / inert no-ops (deferred FX + unported present-demo/reserve supply).
void dExtTpost_HitEff_kikuzu(fopAc_ac_c* /*actor*/, void* /*cyl*/) {}
void dExtTpost_onLetterReadEyeMove() {}
void dComIfGp_evmng_CancelPresent() {}
void dComIfGs_setReserveItemEmpty() {}
