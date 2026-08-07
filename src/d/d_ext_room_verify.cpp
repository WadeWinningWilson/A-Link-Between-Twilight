/**
 * d_ext_room_verify.cpp
 * §389 — the ROOM MANIFEST VERIFIER: donor expectation vs live actuality.
 *
 * ============================================================================
 * WHY THIS EXISTS (user's question, and it changed the order of work):
 *
 * The "Ivan" — TP's same-named actor filling a slot the WW port failed to fill
 * — was an ACCIDENTAL detector. It made an absence VISIBLE (a lit TP torch
 * where WW's cooking fire belongs). Every other silent absence this campaign
 * hit was invisible until a human noticed it and remembered vanilla WW: arcs
 * staged to the wrong directory (§368), unported procs deferring with no
 * runtime trace (§367), materials drawn-but-invisible (§372), a particle latch
 * that failed once and stayed quiet (§368). None of that is measurement.
 *
 * If we ever remove the accident (e.g. by aliasing arcs so TP's actor stops
 * standing in), we lose the only indicator we had — so per the user's ruling
 * BOTH stay: the accidental canary AND this verifier, because a canary that
 * does not depend on my code being correct is worth keeping.
 *
 * WHAT IT DOES: reads the donor-derived expectation manifest
 * (<MOD>/npc/room_expect.csv, emitted by tools/.../room_expect.py from the
 * donor's own dzr rows) and, a few frames after a room settles, walks the LIVE
 * actor list (fopAcIt_Judge) to classify every expected row:
 *
 *   PRESENT     — an actor of the expected proc sits at the authored position
 *   SUBSTITUTED — the proc is there, but it is a TP actor standing in for a WW
 *                 row (the Ivan class — caught BEFORE anyone sees it)
 *   MISSING     — nothing filled the slot (the case the user asked about: no
 *                 visual anomaly, no error, previously undetectable)
 *   DEFERRED    — the row's name has no receiver OBJNAME yet (expected gap,
 *                 reported so it can never be mistaken for a pass)
 *
 * №31-C: if the manifest is absent or the room is unknown, it reports UNKNOWN
 * and verifies nothing — never CLEAN.
 * ============================================================================
 */
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN

#include "d/d_ext_room_verify.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "d/dolzel_rel.h"  // IWYU pragma: keep
#include "d/d_com_inf_game.h"
#include "f_op/f_op_actor_iter.h"
#include "f_op/f_op_actor_mng.h"
#include "dusk/logging.h"

extern aurora::Module DuskLog;

namespace {

struct ExpectRow {
    std::string hostStage;
    int hostRoom = 0;
    std::string name;      // donor dzr name (e.g. "bonbori")
    std::string layer;     // "-" or layer digit
    s16 procIdx = -1;      // receiver proc index, -1 = deferred
    f32 x = 0.f, y = 0.f, z = 0.f;
};

std::vector<ExpectRow> s_expect;
bool s_loaded = false;
bool s_manifestOk = false;

// WW-PORT PROC ROSTER. A proc index in this set means "the receiver's actor at
// this name IS the WW direct port", so PRESENT means present-as-WW. A proc NOT
// in the set that still fills a WW row is a SUBSTITUTION (the Ivan class).
// Kept explicit and cited rather than inferred — runtime cannot see which TU an
// actor came from.
struct WwPortProc {
    const char* sym;
    const char* note;
};
const WwPortProc kWwPortProcs[] = {
    {"fpcNm_NPC_BA1_e", "§261 Grandma"},
    {"fpcNm_NPC_LS1_e", "§244 Aryll"},
    {"fpcNm_NPC_ZL1_e", "§254 Tetra"},
    {"fpcNm_OBJ_TORIPOST_e", "§253 Rito postbox"},
    {"fpcNm_LAMP_e", "§327 WW lamp"},
    {"fpcNm_Obj_Mshokki_e", "§327 tableware"},
    {"fpcNm_SPC_ITEM01_e", "§327 wall shield"},
    {"fpcNm_OBJ_OTBLE_e", "§329 wooden table"},
    {"fpcNm_KAMOME_e", "§232 seagull"},
    {"fpcNm_KB_e", "WW pig port"},
    {"fpcNm_EXT_VEG_e", "§366 swood (donor d_tree packet) + №122 grass/flowers"},
};

std::filesystem::path manifestPath() {
    const char* appdata = getenv("APPDATA");
    if (appdata == NULL) {
        return {};
    }
    return std::filesystem::path(appdata) / "TwilitRealm" / "Dusklight" /
           "model_replacements" / "WW-Crew-Restoration" / "npc" /
           "room_expect.csv";
}

void loadManifest() {
    if (s_loaded) {
        return;
    }
    s_loaded = true;
    const std::filesystem::path p = manifestPath();
    std::ifstream in(p);
    if (!in) {
        DuskLog.warn("[RoomVerify] §389 UNKNOWN — no expectation manifest at "
                     "'{}' (nothing verified; this is not a pass)",
                     p.string());
        return;
    }
    std::string line;
    std::getline(in, line);  // header
    while (std::getline(in, line)) {
        // host_stage,host_room,donor_stage,donor_room,chunk,layer,name,
        // params,x,y,z,proc,proc_idx
        std::vector<std::string> f;
        std::string cur;
        for (char c : line) {
            if (c == ',') {
                f.push_back(cur);
                cur.clear();
            } else if (c != '\r') {
                cur.push_back(c);
            }
        }
        f.push_back(cur);
        if (f.size() < 13) {
            continue;
        }
        ExpectRow r;
        r.hostStage = f[0];
        r.hostRoom = atoi(f[1].c_str());
        r.layer = f[5];
        r.name = f[6];
        r.x = (f32)atof(f[8].c_str());
        r.y = (f32)atof(f[9].c_str());
        r.z = (f32)atof(f[10].c_str());
        r.procIdx = (s16)atoi(f[12].c_str());
        s_expect.push_back(r);
    }
    s_manifestOk = !s_expect.empty();
    DuskLog.info("[RoomVerify] §389 manifest loaded: {} expected rows",
                 s_expect.size());
}

// --- live-actor sweep -------------------------------------------------------
struct Sweep {
    s16 wantProc;
    f32 x, y, z;
    int matchesProc;      // actors of that proc anywhere
    int matchesHere;      // ...within tolerance of the authored position
};

void* judgeActor(void* i_actor, void* i_data) {
    fopAc_ac_c* ac = (fopAc_ac_c*)i_actor;
    Sweep* s = (Sweep*)i_data;
    if (ac == NULL || s == NULL) {
        return NULL;
    }
    if (fopAcM_GetName(ac) != s->wantProc) {
        return NULL;  // keep walking
    }
    s->matchesProc++;
    const f32 dx = ac->current.pos.x - s->x;
    const f32 dy = ac->current.pos.y - s->y;
    const f32 dz = ac->current.pos.z - s->z;
    // Tolerance is generous on purpose: authored rows and settled actors differ
    // by ground-snap and gravity (the donor's own checkGroundY moves y).
    if (dx * dx + dz * dz < 60.0f * 60.0f && dy > -400.0f && dy < 400.0f) {
        s->matchesHere++;
    }
    return NULL;  // never stop early — we want the count
}

// §389b CALIBRATION 2 — rows a RECEIVER proc legitimately serves BY RULING.
// Reporting these as SUBSTITUTED cried wolf on the first real run (5 of 5 were
// this class except bonbori). Each entry cites why it is sanctioned.
bool isRuledReceiverHost(s16 procIdx) {
    switch (procIdx) {
        case fpcNm_KNOB00_e:        // §333 user ruling: interior doors stay
                                    // port-wired (exteriors already were)
        case fpcNm_TAG_EVENT_e:     // TP's own event-trigger tag hosts the
                                    // donor TagEv volumes (§312 chain)
        case fpcNm_KYTAG01_e:       // TP environment tag hosts ky_tag1
            return true;
        default:
            return false;
    }
}

bool isWwPortProc(s16 procIdx) {
    // Compare by index resolved from the receiver's own enum at build time.
    // (Kept as a switch on the enum symbols so a rename breaks the build rather
    // than silently mis-classifying.)
    switch (procIdx) {
        case fpcNm_NPC_BA1_e:
        case fpcNm_NPC_LS1_e:
        case fpcNm_NPC_ZL1_e:
        case fpcNm_OBJ_TORIPOST_e:
        case fpcNm_LAMP_e:
        case fpcNm_Obj_Mshokki_e:
        case fpcNm_SPC_ITEM01_e:
        case fpcNm_OBJ_OTBLE_e:
        case fpcNm_KAMOME_e:
        case fpcNm_KB_e:
        // §394: the census-routed vegetation actor IS a WW direct port (donor
        // d_tree for swood, d_grass/d_flower for the rest) — not a receiver
        // stand-in. Without this row the verifier would report the two swood
        // placements as SUBSTITUTED the moment they started spawning, i.e. cry
        // wolf on the exact fix it exists to confirm.
        case fpcNm_EXT_VEG_e:
            return true;
        default:
            return false;
    }
}

}  // namespace

void dExtRoomVerify_run(const char* i_hostStage, int i_hostRoom) {
    loadManifest();
    if (!s_manifestOk) {
        return;
    }
    int present = 0, missing = 0, substituted = 0, deferred = 0,
        variant = 0, considered = 0;
    for (const ExpectRow& r : s_expect) {
        if (r.hostStage != i_hostStage || r.hostRoom != i_hostRoom) {
            continue;
        }
        considered++;
        if (r.procIdx < 0) {
            deferred++;
            DuskLog.warn("[RoomVerify] §389 DEFERRED  '{}' (layer {}) — no "
                         "receiver OBJNAME; donor authors it at "
                         "({:.0f},{:.0f},{:.0f})",
                         r.name, r.layer, r.x, r.y, r.z);
            continue;
        }
        Sweep s = {r.procIdx, r.x, r.y, r.z, 0, 0};
        fopAcM_Search(judgeActor, &s);
        if (s.matchesHere > 0) {
            if (isWwPortProc(r.procIdx) || isRuledReceiverHost(r.procIdx)) {
                present++;
            } else {
                substituted++;
                DuskLog.warn("[RoomVerify] §389 SUBSTITUTED '{}' at "
                             "({:.0f},{:.0f},{:.0f}) — proc {} filled the slot "
                             "but is neither a WW port nor a ruled receiver "
                             "host (the Ivan class)",
                             r.name, r.x, r.y, r.z, (int)r.procIdx);
            }
        } else if (s.matchesProc > 0) {
            // §389b CALIBRATION 1 — STATE-SELECTED VARIANT. Several rows share a
            // name (Grandma authors Ba1 four times, one per story state) and the
            // donor spawns exactly ONE. An unfilled sibling row is the engine
            // choosing, not a missing actor — report it, never count it MISSING.
            variant++;
            DuskLog.info("[RoomVerify] §389 VARIANT-OFF '{}' (layer {}) at "
                         "({:.0f},{:.0f},{:.0f}) — {} live elsewhere; "
                         "state-selected row not chosen (not a fault)",
                         r.name, r.layer, r.x, r.y, r.z, s.matchesProc);
        } else {
            missing++;
            DuskLog.warn("[RoomVerify] §389 MISSING    '{}' (layer {}) proc {} "
                         "— donor authors it at ({:.0f},{:.0f},{:.0f}); NONE of "
                         "that proc exist anywhere (real absence)",
                         r.name, r.layer, (int)r.procIdx, r.x, r.y, r.z);
        }
    }
    if (considered == 0) {
        DuskLog.warn("[RoomVerify] §389 UNKNOWN — no expected rows for {} room "
                     "{} (nothing verified; not a pass)",
                     i_hostStage, i_hostRoom);
        return;
    }
    DuskLog.info("[RoomVerify] §389 {} room {}: {} PRESENT / {} MISSING / "
                 "{} SUBSTITUTED / {} VARIANT-OFF / {} DEFERRED  (of {} donor "
                 "rows)",
                 i_hostStage, i_hostRoom, present, missing, substituted,
                 variant, deferred, considered);
}
