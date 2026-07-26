/**
 * d_ext_npc_mount.cpp — Plan R generic external-NPC mount (L1 + Slice I lighting/blink).
 */
#include "d/d_ext_npc_mount.h"

#if TARGET_PC

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <unordered_map>

#include "JSystem/J3DGraphAnimator/J3DJoint.h"
#include "JSystem/J3DGraphAnimator/J3DMaterialAnm.h"
#include "JSystem/J3DGraphAnimator/J3DMaterialAttach.h"
#include "JSystem/J3DGraphBase/J3DSys.h"
#include "JSystem/J3DGraphBase/J3DTransform.h"
#include "JSystem/J3DGraphLoader/J3DModelLoader.h"
#include "JSystem/JMath/JMath.h"
#include "SSystem/SComponent/c_lib.h"
#include "SSystem/SComponent/c_m3d.h"
#include "SSystem/SComponent/c_math.h"
#include <cmath>
#include "d/d_com_inf_game.h"
#include "d/d_stage.h"
#include "d/d_albw_dialogue.h"
#include "d/d_albw_outfit.h"  // №238: wardrobe-integrated clothes handover
#include "d/d_msg_flow.h"     // №248: native message flow for mount talk (Shade Watcher pattern)
#include "d/d_camera.h"  // №172: Stop camera so opening event path can own the view
#include "d/d_demo.h"  // §50: demo-truncation probe
#include "SSystem/SComponent/c_counter.h"  // §52: rate-limited read-back trace
#include "d/d_ext_mod_flags.h"
#include "d/d_ext_fado_door.h"
#include "d/d_ext_npc_doors.h"
#include "d/d_ext_npc_population.h"
#include "d/d_ext_save_guard.h"
#include "d/ext_seq/ja1_bank.h"
#include "d/d_item.h"
#include "d/d_item_data.h"
#include "d/d_bg_s_gnd_chk.h"
#include "d/d_drawlist.h"
#include "d/d_kankyo.h"
#include "d/d_kankyo_wether.h"
#include "d/d_meter2_info.h"
#include "d/d_s_play.h"
#include "global.h"
#include "d/actor/d_a_alink.h"
#include "dusk/custom_assets.hpp"
#include "dusk/logging.h"
#include "dusk/main.h"
#include "f_op/f_op_actor_iter.h"
#include "f_op/f_op_actor_mng.h"
#include "f_pc/f_pc_manager.h"
#include "f_pc/f_pc_name.h"
#include <vector>
#include "JSystem/JKernel/JKRHeap.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_lib.h"
#include "m_Do/m_Do_controller_pad.h"
#include "m_Do/m_Do_graphic.h"
#include "m_Do/m_Do_mtx.h"

// Live-tune NPC cel ambient offsets (additive on manifest amb_*).
// WREG_F(30)=R, WREG_F(31)=G, WREG_F(32)=B — e.g. -20 to pull bloom down.

namespace {

namespace fs = std::filesystem;

std::unordered_map<std::string, dExtNpcManifest> s_providers;
std::vector<std::string> s_providerOrder;  // stable UI order

// ModelData cache (GameHeap-pinned). Key = "arc/model" or "bg:arc/model".
// Actors hold their own J3DModel / McaMorf over shared cached ModelData — never
// store actor-solid-heap pointers in dRes slots (№18 / Layer-B lifetime).
// №73: NOT session-immortal — J3D is pointer-fixed into the archive buffer, so
// cache entries for an arc MUST be purged when that arc's last live mount
// resDeletes (room-lane unload). Shared arcs (Knob) use a live refcount.
std::unordered_map<std::string, J3DModelData*> s_modelDataCache;
// №50-C crash fix: J3D load pointer-fixes the dRes buffer in place. A second load of the
// same member (plain ko.bdl for Zill, then ko.bdl+ko02.bmt for Joel) AVs. Keep a pristine
// copy of each J3D2 blob before the first load and re-parse BMT variants from that.
std::unordered_map<std::string, std::vector<u8>> s_pristineJ3dRaw;
// ============================================================================
// №263 — the KOISI exit crash (J3D re-parse of a pointer-fixed buffer).
// ============================================================================
// Sequence: interior transition purged the arc's parsed cache AND its pristine
// copy, but dRes kept the (already pointer-fixed) buffer resident; the exterior
// re-create then re-parsed it → makeHierarchy AV. Two invariant fixes:
//   1. pristine copies are SESSION-LIVED (purge no longer erases them) — they
//      are the insurance FOR the re-parse case, erasing them defeated it;
//   2. every parse consumes a FRESH COPY of the pristine (below), so neither
//      the dRes buffer nor the stored pristine is ever pointer-fixed at all.
// The copies handed to J3D must outlive their models — kept here for the
// session (same lifetime class as the deliberately-leaked purged ModelData).
std::vector<std::vector<u8>> s_parsedRawKeep;

void* mountPristineParseSrc(const std::string& plainKey, void* res) {
    auto pit = s_pristineJ3dRaw.find(plainKey);
    if (pit == s_pristineJ3dRaw.end()) {
        return res;  // non-J3D2 / stash refused — first-touch parse of dRes
    }
    s_parsedRawKeep.push_back(pit->second);  // fresh copy; J3D fixes THIS one
    return s_parsedRawKeep.back().data();
}
// №73: mounts that have reached COMPLEATE and still own a live resLoad of `arc`.
std::unordered_map<std::string, int> s_arcLiveCount;
// №73 sweep ("arcs/mounts are permanent" assumptions invalidated by room-lane):
// - s_modelDataCache / s_pristineJ3dRaw — FIXED (retain/release + purge on last).
// - s_bgMountIds — already cleared on room unload / releaseRoomLaneMount.
// - warm interiors — intentionally permanent for PINNED doors (refcount keeps cache).
// - pending-spawn FIFO — №64 already drain-on-bind; leave alone.
// - door exit flags — №68 clearExitKnobForProc on unload.

u32 readBeU32(const void* p) {
    const u8* b = static_cast<const u8*>(p);
    return (u32(b[0]) << 24) | (u32(b[1]) << 16) | (u32(b[2]) << 8) | u32(b[3]);
}

void ensurePristineJ3dRaw(const char* arc, const char* modelName, void* res) {
    if (arc == NULL || modelName == NULL || res == NULL) {
        return;
    }
    const std::string key = std::string(arc) + "/" + modelName;
    if (s_pristineJ3dRaw.find(key) != s_pristineJ3dRaw.end()) {
        return;
    }
    const u8* bytes = static_cast<const u8*>(res);
    if (bytes[0] != 'J' || bytes[1] != '3' || bytes[2] != 'D' || bytes[3] != '2') {
        return;
    }
    const u32 size = readBeU32(bytes + 8);
    const u32 blocks = readBeU32(bytes + 12);
    if (size < 0x20 || size > 64u * 1024u * 1024u || blocks == 0 || blocks > 256) {
        return;
    }
    std::vector<u8> copy(size);
    std::memcpy(copy.data(), res, size);
    s_pristineJ3dRaw.emplace(key, std::move(copy));
}
bool s_bgWarpPending = false;
char s_bgWarpProc[32] = {};
char s_lastBgProc[32] = {};
// №90: COMPLEATE on WW host may precede player — drain when player exists.
char s_interiorBootstrapProc[32] = {};
// №94: increments on play-scene recreate / restart; spawn latches key off this.
u32 s_worldGeneration = 1;
char s_worldGenStage[12] = {};
bool s_bgSpawnOverrideValid = false;
cXyz s_bgSpawnOverride;
bool s_bgSpawnFacingValid = false;
s16 s_bgSpawnFacing = 0;
std::unordered_map<std::string, fpc_ProcID> s_bgMountIds;
// №62 Phase D: proc → host room (same-stage stream). Empty = pinned-only.
std::unordered_map<std::string, int> s_roomLaneRooms;
std::unordered_map<int, std::string> s_roomLaneProcByRoom;
// Rooms currently claimed by an active room-lane enter (protected from RTBL kill).
bool s_roomLaneClaimed[0x40] = {};
// Prevent phase_3+phase_4 double objectSetCheck from minting two BG mounts.
bool s_roomLaneMountCreating[0x40] = {};
// №68: set before room teardown; poll/draw/attention skip until cleared.
bool s_roomLaneUnloading[0x40] = {};
char s_roomLaneUnloadingProc[32] = {};
// №58-B: after island COMPLEATE, cold-create same-stage interior BGs (first press = warm).
// №115: abort on heap fail; foreign host_stage / warm=0 skipped; one create per N frames.
bool s_warmInteriors = false;
int s_warmProviderIndex = 0;
int s_warmCooldown = 0;
static constexpr int kWarmCooldownFrames = 30;  // ~0.5s @60 — one provider per N frames
// entrySolidHeap size 0x120000 is doubled on TARGET_PC; keep margin for models/cache.
static constexpr s32 kWarmBgHeapNeed = 0x280000;
bool s_doorDemoLocked = false;

// One native window is shared by all external mounts.  It is deliberately never
// actor-owned: actors can despawn while the UI draw list is still processing.
#if TARGET_PC_NATIVE_UI
dALBWDialogue_c* s_mountDialogue = NULL;
// ============================================================================
// №248 — mount talk goes through TP's NATIVE message flow.
// ============================================================================
// The Shade Watcher proved the pattern: dMsgFlow_c::initWord injects OUR text
// (no BMG asset) into the engine's 0x1324 code-text flow, and the ordered SPEAK
// EVENT owns Link for the duration. This retires the ALBW-postman box for mount
// talk — the box that cropped WW lines, ignored TP's centered formatting, and
// never locked the player (№242 defects 2+3). The section/data layer (ww_ref,
// flags, actions, next-chains) is untouched; only presentation changed.
dMsgFlow_c s_mountFlow;
// ============================================================================
// №252 — PAGINATION to TP's box rules (the Shade Watcher lesson, generalized).
// ============================================================================
// The Shade Watcher's injected lines were AUTHORED within TP's limits; donor
// rows are not — one catalog entry can hold several donor pages (WW's own
// page-break control codes were dropped at extraction, flattening multi-box
// messages). The native flow displays injected text verbatim, so we paginate:
// section text splits into pages of at most kMountMsgLinesPerPage lines, each
// page its own initWord, chained on doFlow completion BEFORE section-advance.
// Single conversation at a time (owner-guarded), so file-scope state is safe.
// №261 — TP-vanilla layout rules. The US talk box shows 4 lines ('n_e4line'
// layout) and hard-splits any overflowing line MID-WORD (the "youn / g"
// screenshot) — vanilla text never overflows because localization pre-wraps it.
// So WE pre-wrap: donor line breaks are WW-box-width formatting and get
// discarded; words re-flow to kMountMsgMaxCols with whole-word carry, and a
// sentence that cannot finish on the current page starts the next one instead
// (the Shade Watcher rule: word, line, and sentence carry over — never split).
constexpr int kMountMsgLinesPerPage = 4;   // vanilla US TP talk box line count
constexpr int kMountMsgMaxCols = 38;       // safe width under the observed 40-col overflow
std::vector<std::string> s_mountPages;
size_t s_mountPageIdx = 0;

void mountPaginate(const std::string& textIn) {
    // ========================================================================
    // №260 — the regenerated catalog decodes WW's name escape as a literal
    // "{player}" token. Substitute the save-file name here (R6: the sentence
    // stays donor data; only the insert comes from TP's own save, exactly the
    // donor's mechanic). Covers every text source, not just ww_ref.
    // ========================================================================
    std::string text = textIn;
    {
        const char* rawName = dComIfGs_getPlayerName();
        std::string name;
        for (int i = 0; i < 16 && rawName != NULL && rawName[i] != '\0'; ++i) {
            name += rawName[i];
        }
        if (name.empty()) {
            name = "Link";
        }
        size_t at;
        while ((at = text.find("{player}")) != std::string::npos) {
            text.replace(at, 8, name);
        }
    }
    s_mountPages.clear();
    s_mountPageIdx = 0;
    // ========================================================================
    // №261 — TP-vanilla re-wrap. Donor \n inside a paragraph = WW-width
    // formatting (discarded); a BLANK line = WW's own page break (hard flush).
    // Words flow whole onto lines (≤ kMountMsgMaxCols); a sentence that would
    // spill past the page's last line carries WHOLE to the next page when a
    // fresh page can hold it.
    // ========================================================================
    std::vector<std::string> pageLines;
    std::string curLine;
    auto flushLine = [&]() {
        if (!curLine.empty()) {
            pageLines.push_back(curLine);
            curLine.clear();
        }
    };
    auto flushPage = [&]() {
        flushLine();
        if (!pageLines.empty()) {
            std::string page;
            for (size_t i = 0; i < pageLines.size(); ++i) {
                if (i > 0) {
                    page += '\n';
                }
                page += pageLines[i];
            }
            s_mountPages.push_back(page);
            pageLines.clear();
        }
    };
    // Split into paragraphs on blank lines; within each, collect words.
    size_t pos = 0;
    while (pos <= text.size()) {
        // -- gather one paragraph (up to blank line / end) as a word list
        std::vector<std::string> words;
        bool sawBlank = false;
        std::string word;
        while (pos <= text.size()) {
            const char c = pos < text.size() ? text[pos] : '\n';
            if (c == '\n') {
                if (!word.empty()) {
                    words.push_back(word);
                    word.clear();
                }
                // peek: a second consecutive newline (ignoring the word we just
                // closed) = blank line = WW page break → paragraph ends
                if (pos + 1 <= text.size() &&
                    (pos + 1 == text.size() || text[pos + 1] == '\n')) {
                    sawBlank = true;
                    // consume the run of blank lines
                    while (pos < text.size() && (text[pos] == '\n' || text[pos] == '\r')) {
                        ++pos;
                    }
                    break;
                }
                ++pos;
                continue;
            }
            if (c == ' ' || c == '\t' || c == '\r') {
                if (!word.empty()) {
                    words.push_back(word);
                    word.clear();
                }
                ++pos;
                continue;
            }
            word += c;
            ++pos;
        }
        if (!word.empty()) {
            words.push_back(word);
        }
        if (pos >= text.size() && words.empty() && !sawBlank) {
            break;
        }
        // -- group words into sentences (break after terminal .!? — but not
        //    inside runs like "...!" — i.e. at the LAST of a punctuation run)
        std::vector<std::vector<std::string> > sentences;
        std::vector<std::string> sent;
        for (size_t w = 0; w < words.size(); ++w) {
            sent.push_back(words[w]);
            const char last = words[w][words[w].size() - 1];
            if (last == '.' || last == '!' || last == '?') {
                sentences.push_back(sent);
                sent.clear();
            }
        }
        if (!sent.empty()) {
            sentences.push_back(sent);
        }
        // -- flow each sentence; carry whole sentences across page boundaries
        for (size_t s = 0; s < sentences.size(); ++s) {
            const std::vector<std::string>& sw = sentences[s];
            // simulate the flow: would this sentence spill past the page's
            // last line? (linesUsed counts the line it starts on — curLine's
            // line when continuing, a fresh line otherwise.)
            {
                size_t col = curLine.size();
                int linesUsed = 1;
                size_t col2 = 0;
                int standaloneLines = 1;
                for (size_t w = 0; w < sw.size(); ++w) {
                    const size_t need = sw[w].size();
                    if (col == 0) {
                        col = need;
                    } else if (col + 1 + need <= (size_t)kMountMsgMaxCols) {
                        col += 1 + need;
                    } else {
                        col = need;
                        ++linesUsed;
                    }
                    if (col2 == 0) {
                        col2 = need;
                    } else if (col2 + 1 + need <= (size_t)kMountMsgMaxCols) {
                        col2 += 1 + need;
                    } else {
                        col2 = need;
                        ++standaloneLines;
                    }
                }
                const int totalLines = (int)pageLines.size() + linesUsed;
                if (totalLines > kMountMsgLinesPerPage &&
                    standaloneLines <= kMountMsgLinesPerPage) {
                    flushPage();  // sentence carries WHOLE to a fresh page
                }
            }
            for (size_t w = 0; w < sw.size(); ++w) {
                const std::string& wd = sw[w];
                if (curLine.empty()) {
                    curLine = wd;
                } else if (curLine.size() + 1 + wd.size() <= (size_t)kMountMsgMaxCols) {
                    curLine += ' ';
                    curLine += wd;
                } else {
                    flushLine();
                    if ((int)pageLines.size() == kMountMsgLinesPerPage) {
                        flushPage();
                    }
                    curLine = wd;
                }
            }
        }
        if (sawBlank) {
            flushPage();  // WW's own page break
        }
        if (pos >= text.size()) {
            break;
        }
    }
    flushPage();
    if (s_mountPages.empty()) {
        s_mountPages.push_back(std::string());
    }
    // §64 — H2-H7 output probe (LAW: sample the produced result). Log the page
    // COUNT and each page's line count + length, so on now-full text (№259) the
    // actual split is visible: overflow (H4 lines>cap), width run-on (H5 long
    // single line), or a good split all read straight off this.
    DuskLog.warn("[ExtNpcMount] §64 paginate: {} page(s) from {} chars", (int)s_mountPages.size(),
                 (int)text.size());
    for (size_t i = 0; i < s_mountPages.size(); ++i) {
        int nlines = 1;
        size_t longest = 0, cur = 0;
        for (char c : s_mountPages[i]) {
            if (c == '\n') { ++nlines; cur = 0; }
            else if (++cur > longest) longest = cur;
        }
        DuskLog.warn("[ExtNpcMount] §64   page {}: {} line(s), longest={} chars", (int)i, nlines,
                     (int)longest);
    }
}
#endif
dExtNpcMount_c* s_mountDialogueOwner = NULL;

const dCcD_SrcCyl s_mountCylSrc = {
    {{0x0, {{0x0, 0x0, 0x0}, {0, 0}, 0x79}},
     {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0x0},
     {dCcD_SE_NONE, 0x0, 0x0, 0x0, 0},
     {0x0}},
    {{{0.0f, 0.0f, 0.0f}, 0.0f, 0.0f}},
};

// №27 N3: refuse cross-rig anims (e.g. kohead01_wait on ko.bdl).
// Model stem must be a prefix of the anim stem, and must not be a strict
// shorter prefix of a longer sibling (ko vs kohead).
bool animMatchesModel(const char* modelName, const char* animName) {
    if (modelName == NULL || animName == NULL || modelName[0] == '\0' || animName[0] == '\0') {
        return false;
    }
    auto stem = [](const char* s, char* out, size_t n) {
        size_t i = 0;
        while (s[i] && s[i] != '.' && i + 1 < n) {
            char c = s[i];
            if (c >= 'A' && c <= 'Z') {
                c = (char)(c - 'A' + 'a');
            }
            out[i++] = c;
        }
        out[i] = '\0';
    };
    char mStem[64];
    char aStem[64];
    stem(modelName, mStem, sizeof(mStem));
    stem(animName, aStem, sizeof(aStem));
    const size_t mLen = std::strlen(mStem);
    if (mLen == 0 || std::strlen(aStem) < mLen) {
        return false;
    }
    for (size_t i = 0; i < mLen; ++i) {
        if (aStem[i] != mStem[i]) {
            return false;
        }
    }
    // Allow exact stem, stem_, or stem followed by digit — reject stem+letter that
    // starts a longer family (ko + head… → kohead).
    const char next = aStem[mLen];
    if (next == '\0' || next == '_' || (next >= '0' && next <= '9')) {
        return true;
    }
    return false;
}

// №47-A: WW body idles are often short names (`wait.bck`, `talk.bck`) — not model-prefixed.
// Keep N3's cross-rig refuse (kohead* on ko.bdl) but allow non-prefixed same-arc anims.
bool animAllowedOnBody(const char* modelName, const char* animName) {
    if (modelName == NULL || animName == NULL || modelName[0] == '\0' || animName[0] == '\0') {
        return false;
    }
    if (animMatchesModel(modelName, animName)) {
        return true;
    }
    auto stem = [](const char* s, char* out, size_t n) {
        size_t i = 0;
        while (s[i] && s[i] != '.' && i + 1 < n) {
            char c = s[i];
            if (c >= 'A' && c <= 'Z') {
                c = (char)(c - 'A' + 'a');
            }
            out[i++] = c;
        }
        out[i] = '\0';
    };
    char mStem[64];
    char aStem[64];
    stem(modelName, mStem, sizeof(mStem));
    stem(animName, aStem, sizeof(aStem));
    const size_t mLen = std::strlen(mStem);
    if (mLen == 0 || aStem[0] == '\0') {
        return false;
    }
    // Anim stem begins with model stem but failed the digit/_ boundary ⇒ sibling family.
    if (std::strlen(aStem) >= mLen) {
        bool prefix = true;
        for (size_t i = 0; i < mLen; ++i) {
            if (aStem[i] != mStem[i]) {
                prefix = false;
                break;
            }
        }
        if (prefix) {
            return false;
        }
    }
    return true;  // e.g. ym.bdl + wait.bck
}

void setMountAnimation(dExtNpcMount_c* a, const char* name, u8 mode) {
    if (a == NULL || a->mpMorf == NULL || name == NULL || name[0] == '\0') {
        return;
    }
    // №37: door-open BCK may not match door.bdl name prefix — allow static/door mounts.
    // №47-A: WW short idle/talk names allowed; cross-rig still refused.
    if (!a->mManifest.isStatic && !a->mManifest.doorAttention &&
        !animAllowedOnBody(a->mManifest.model, name)) {
        DuskLog.warn("[ExtNpcMount] N3 skip anim '{}' for model '{}' (cross-rig)", name,
                     a->mManifest.model);
        return;
    }
    J3DAnmTransform* anm = (J3DAnmTransform*)dComIfG_getObjectRes(a->mManifest.arc, name);
    if (anm != NULL) {
        a->mpMorf->setAnm(anm, mode, 3.0f, 1.0f, 0.0f, -1.0f, NULL);
    }
}

// №91: shared DoorK10 load (TP event archive for DEFAULT_KNOB_DOOR_*).
request_of_phase_process_class s_doorK10Phase;
int s_doorK10PhaseState = cPhs_INIT_e;  // COMPLEATE / ERROR once settled

bool pollDoorK10Ready() {
    if (s_doorK10PhaseState == cPhs_COMPLEATE_e) {
        return true;
    }
    if (s_doorK10PhaseState == cPhs_ERROR_e) {
        return false;
    }
    s_doorK10PhaseState = dComIfG_resLoad(&s_doorK10Phase, "DoorK10");
    if (s_doorK10PhaseState == cPhs_COMPLEATE_e) {
        DuskLog.info("[ExtNpcMount] №91 DoorK10 event archive ready");
        return true;
    }
    if (s_doorK10PhaseState == cPhs_ERROR_e) {
        DuskLog.warn("[ExtNpcMount] №91 DoorK10 load failed — falling back to №53 immediate warp");
        return false;
    }
    return false;
}

bool bindKnobDoorEvents(dExtNpcMount_c* a) {
    if (a == NULL || !a->mManifest.doorAttention) {
        return false;
    }
    if (a->mKnobEvBound) {
        return a->mKnobEvtFront >= 0 || a->mKnobEvtBack >= 0;
    }
    if (!pollDoorK10Ready()) {
        return false;
    }
    a->eventInfo.setArchiveName("DoorK10");
    a->mKnobEvtFront =
        dComIfGp_getEventManager().getEventIdx(a, "DEFAULT_KNOB_DOOR_F_OPEN", 0xff);
    a->mKnobEvtBack =
        dComIfGp_getEventManager().getEventIdx(a, "DEFAULT_KNOB_DOOR_B_OPEN", 0xff);
    a->mKnobEvBound = 1;
    a->mKnobEvtOrdered = -1;
    a->mKnobStaffId = -1;
    a->mKnobDoorAction = 0;
    a->mKnobOpenStarted = 0;
    DuskLog.info("[ExtNpcMount] №91 knob events bound proc='{}' F={} B={}", a->mManifest.proc,
                 (int)a->mKnobEvtFront, (int)a->mKnobEvtBack);
    return a->mKnobEvtFront >= 0 || a->mKnobEvtBack >= 0;
}

// daKnob20_c::frontCheck — 0 = front (F_OPEN), 1 = back (B_OPEN).
int knobFrontCheck(dExtNpcMount_c* a) {
    daPy_py_c* player = daPy_getPlayerActorClass();
    if (player == NULL || a == NULL) {
        return 0;
    }
    cXyz playerDist = player->current.pos - a->current.pos;
    mDoMtx_stack_c::YrotS(-a->current.angle.y);
    mDoMtx_stack_c::multVec(&playerDist, &playerDist);
    return playerDist.z > 0.0f ? 0 : 1;
}

void knobDoorSetEventPrm(dExtNpcMount_c* a) {
    if (a == NULL || !bindKnobDoorEvents(a)) {
        return;
    }
    const int side = knobFrontCheck(a);
    const s16 evt = (side == 0) ? a->mKnobEvtFront : a->mKnobEvtBack;
    if (evt < 0) {
        return;
    }
    a->mKnobEvtOrdered = evt;
    a->eventInfo.setArchiveName("DoorK10");
    a->eventInfo.setEventId(evt);
    a->eventInfo.setMapToolId(0xff);
    a->eventInfo.onCondition(dEvtCnd_CANDOOR_e);
}

int knobDoorGetDemoAction(dExtNpcMount_c* a) {
    static DUSK_CONSTEXPR char DUSK_CONST* action_table[16] = {
        "WAIT",
        "SETSTART",
        "SETANGLE",
        "ADJUSTMENT",
        "OPEN_PUSH",
        "OPEN_PULL",
        "OPEN_PUSH2",
        "OPEN_PULL2",
        "OPEN_PUSH_STOP",
        "OPEN_PULL_STOP",
        "TALK",
        "TALK_END",
        "SETSTART_PUSH",
        "SETSTART_PULL",
        "DEMO_OPEN",
        "DEMO_CLOSE",
    };
    return dComIfGp_evmng_getMyActIdx(a->mKnobStaffId, action_table, 16, 0, 0);
}

void knobDoorStartOpenAnim(dExtNpcMount_c* a) {
    if (a == NULL || a->mKnobOpenStarted || a->mpMorf == NULL) {
        return;
    }
    const char* bck = a->mManifest.doorOpenBck[0] ? a->mManifest.doorOpenBck : NULL;
    if (bck == NULL) {
        return;  // caller cutEnds when start leaves mKnobOpenStarted clear
    }
    J3DAnmTransform* anm = (J3DAnmTransform*)dComIfG_getObjectRes(a->mManifest.arc, bck);
    if (anm == NULL) {
        DuskLog.warn("[ExtNpcMount] №97 door open BCK '{}' missing in arc '{}'", bck,
                     a->mManifest.arc);
        return;
    }
    a->mpMorf->setAnm(anm, J3DFrameCtrl::EMode_NONE, 0.0f, 1.0f, 0.0f, -1.0f, NULL);
    a->mKnobOpenStarted = 1;
    DuskLog.info("[ExtNpcMount] №97 door OPEN anim '{}' proc='{}'", bck, a->mManifest.proc);
    // №31: never play TP Z2SE_OBJ_KNOB_DOOR_* in WW spaces.
}

bool knobDoorOpenAnimDone(dExtNpcMount_c* a) {
    if (a == NULL || a->mpMorf == NULL) {
        return true;
    }
    // №97: never treat "not started yet" as done — that cutEnd'd OPEN before the BCK.
    if (!a->mKnobOpenStarted) {
        return false;
    }
    return a->mpMorf->isStop() != 0;
}

// №91/№97: door owns SHUTTER_DOOR staff cuts (WW door_open_bck on open actions).
void knobDoorDemoProc(dExtNpcMount_c* a) {
    if (a == NULL || a->mKnobStaffId == -1) {
        return;
    }
    const int demoAction = knobDoorGetDemoAction(a);
    if (dComIfGp_evmng_getIsAddvance(a->mKnobStaffId) != 0) {
        switch (demoAction) {
        case 4:  // OPEN_PUSH
        case 5:  // OPEN_PULL
        case 6:  // OPEN_PUSH2
        case 7:  // OPEN_PULL2
            knobDoorStartOpenAnim(a);
            break;
        default:
            break;
        }
    }
    switch (demoAction) {
    case 4:
    case 5:
    case 6:
    case 7:
        // Belt-and-suspenders: start even if advance was missed this frame.
        if (!a->mKnobOpenStarted) {
            knobDoorStartOpenAnim(a);
            if (!a->mKnobOpenStarted) {
                // No door_open_bck in manifest — don't stall the cut.
                dComIfGp_evmng_cutEnd(a->mKnobStaffId);
            }
            break;
        }
        if (knobDoorOpenAnimDone(a)) {
            dComIfGp_evmng_cutEnd(a->mKnobStaffId);
        }
        break;
    default:
        // SETSTART / ADJUSTMENT / WAIT / … — Link side is event-driven; door cuts through.
        dComIfGp_evmng_cutEnd(a->mKnobStaffId);
        break;
    }
}

void knobDoorInitOpenDemo(dExtNpcMount_c* a) {
    if (a == NULL) {
        return;
    }
    a->mKnobStaffId = dComIfGp_evmng_getMyStaffId("SHUTTER_DOOR", NULL, 0);
    a->mKnobOpenStarted = 0;
    a->mKnobDoorAction = 1;
    DuskLog.info("[ExtNpcMount] №91 knob demo BEGIN proc='{}' staff={} evt={}", a->mManifest.proc,
                 a->mKnobStaffId, (int)a->mKnobEvtOrdered);
}

// Returns true when the door finished its owned event and fired the warp.
bool tickKnobDoorEvent(dExtNpcMount_c* a) {
    if (a == NULL || !a->mManifest.doorAttention) {
        return false;
    }
    // Keep trying to bind while DoorK10 loads; until then №53 doorCheck fallback remains.
    bindKnobDoorEvents(a);

    if (a->mKnobDoorAction == 1) {
        dMeter2Info_onGameStatus(2);
        if (a->mKnobEvtOrdered >= 0 && dComIfGp_evmng_endCheck(a->mKnobEvtOrdered)) {
            a->mKnobDoorAction = 0;
            a->mKnobStaffId = -1;
            dComIfGp_event_reset();
            DuskLog.info("[ExtNpcMount] №91 knob demo END (cutEnd owned) → warp proc='{}'",
                         a->mManifest.proc);
            dExtNpcDoors_tryNativeWarp(a, /*openAlreadyDone=*/true);
            a->mKnobEvtOrdered = -1;
            a->mKnobOpenStarted = 0;
            return true;
        }
        a->mKnobStaffId = dComIfGp_evmng_getMyStaffId("SHUTTER_DOOR", NULL, 0);
        knobDoorDemoProc(a);
        return true;
    }

    if (a->eventInfo.checkCommandDoor()) {
        if (a->mKnobEvtOrdered < 0) {
            // Event ordered without our bind (shouldn't happen) — let №53 path handle.
            return false;
        }
        knobDoorInitOpenDemo(a);
        knobDoorDemoProc(a);
        return true;
    }

    if (!dComIfGp_event_runCheck()) {
        if (a->mKnobEvBound && (a->mKnobEvtFront >= 0 || a->mKnobEvtBack >= 0)) {
            knobDoorSetEventPrm(a);
        } else {
            // DoorK10 not ready / idxs missing — keep №53 CANDOOR so doorCheck warps.
            fopAc_ac_c* player = dComIfGp_getPlayer(0);
            if (player != NULL && (player->current.pos - a->current.pos).absXZ() < 300.0f) {
                a->eventInfo.onCondition(dEvtCnd_CANDOOR_e);
            }
        }
    }
    return a->mKnobEvBound != 0 && (a->mKnobEvtFront >= 0 || a->mKnobEvtBack >= 0);
}

// №36 C: WW Vlupy tev frame (d_item_data mTevFrm) — item 1→0 … 6→5; silver 0x0F→6.
f32 vlupyColorFrame(u8 itemNo) {
    if (itemNo >= 1 && itemNo <= 6) {
        return (f32)(itemNo - 1);
    }
    if (itemNo == 0x0F) {
        return 6.0f;
    }
    return 0.0f;
}

// WW rupee item id → TP wallet item (silver differs: WW 0x0F → TP 0x07).
u8 tpRupeeGrantId(u8 wwItemNo) {
    if (wwItemNo >= dItemNo_GREEN_RUPEE_e && wwItemNo <= dItemNo_ORANGE_RUPEE_e) {
        return wwItemNo;
    }
    if (wwItemNo == 0x0F) {
        return (u8)dItemNo_SILVER_RUPEE_e;
    }
    return 0;
}

// №49 v4: REPLACE inside joint callback (param=0) BEFORE children recurse and
// BEFORE calcWeightEnvelopeMtx — Nintendo nodeCB pattern. Also patches
// j3dSys.mCurrentMtx so descendants + envelope see the new world mtx.
int extNpcSlaveJointCB(J3DJoint* joint, int timing) {
    if (timing != 0 || joint == NULL) {
        return 1;
    }
    J3DModel* model = j3dSys.getModel();
    if (model == NULL) {
        return 1;
    }
    dExtNpcMount_c* a = (dExtNpcMount_c*)model->getUserArea();
    if (a == NULL || a->mpMorf == NULL || a->mpMorf->getModel() == NULL) {
        return 1;
    }
    J3DModel* body = a->mpMorf->getModel();
    const u16 jnt = joint->getJntNo();
    MtxP src = NULL;

    if (a->mpCompanion != NULL && model == a->mpCompanion->getModel()) {
        if (a->mSlavePairCount > 0) {
            for (int i = 0; i < a->mSlavePairCount; ++i) {
                if (a->mSlaveCompJnt[i] == (s16)jnt) {
                    src = body->getAnmMtx(a->mSlaveBodyJnt[i]);
                    break;
                }
            }
        } else if (jnt == 0) {
            s16 hostJnt = a->mNeckJnt;
            if (hostJnt < 0 && body->getModelData() != NULL) {
                JUTNameTab* names = body->getModelData()->getJointTree().getJointName();
                hostJnt = names != NULL ? (s16)names->getIndex("head") : (s16)-1;
            }
            if (hostJnt >= 0) {
                src = body->getAnmMtx(hostJnt);
            }
        }
    } else {
        for (int i = 0; i < a->mAttachCount; ++i) {
            if (a->mpAttach[i] != model || jnt != 0 || a->mAttachJnt[i] < 0 ||
                a->mAttachSlave[i] == 0) {
                continue;
            }
            J3DModel* host = body;
            if (a->mAttachOnCompanion[i] && a->mpCompanion != NULL &&
                a->mpCompanion->getModel() != NULL) {
                host = a->mpCompanion->getModel();
            }
            src = host->getAnmMtx(a->mAttachJnt[i]);
            // ================================================================
            // №218 — donor local prop transform: jointMtx × T(offs) × R(rot),
            // the donor's own hand-prop recipe (Ls1 setMtx: transM then
            // XYZrotM under the hand joint's anm mtx). Stack top stays valid
            // through the MTXConcat below — nothing else touches it first.
            // ================================================================
            if (a->mAttachLocal[i]) {
                // №250: the donor selects the prop pose by its demo-driven
                // flag (m841) — use the demo variant while the storyboard
                // owns this mount.
                const bool useDemo = a->mDemoOwned && a->mAttachDemoLocal[i];
                const cXyz& off = useDemo ? a->mAttachOffsDemo[i] : a->mAttachOffs[i];
                const cXyz& rot = useDemo ? a->mAttachRotDemo[i] : a->mAttachRot[i];
                mDoMtx_stack_c::copy(src);
                mDoMtx_stack_c::transM(off.x, off.y, off.z);
                mDoMtx_stack_c::XYZrotM((s16)rot.x, (s16)rot.y, (s16)rot.z);
                src = mDoMtx_stack_c::get();
            }
            break;
        }
    }

    if (src == NULL) {
        return 1;
    }
    // №50-A v5 PARENT-COMPOSE: world = body_target × companion local bind.
    // Identity-root heads unchanged; Zill's authored −90/−90 bind is preserved.
    Mtx local;
    const J3DTransformInfo& ti = joint->getTransformInfo();
    J3DGetTranslateRotateMtx(ti, local);
    if (ti.mScale.x != 1.0f || ti.mScale.y != 1.0f || ti.mScale.z != 1.0f) {
        JMAMTXApplyScale(local, local, ti.mScale.x, ti.mScale.y, ti.mScale.z);
    }
    Mtx world;
    MTXConcat(src, local, world);
    model->setAnmMtx(jnt, world);
    MTXCopy(world, j3dSys.mCurrentMtx);
    return 1;
}

void installSlaveJointCallbacks(J3DModel* model, dExtNpcMount_c* owner) {
    if (model == NULL || owner == NULL) {
        return;
    }
    model->setUserArea((uintptr_t)owner);
    J3DModelData* data = model->getModelData();
    if (data == NULL) {
        return;
    }
    for (u16 i = 0; i < data->getJointNum(); ++i) {
        J3DJoint* j = data->getJointNodePointer(i);
        if (j != NULL) {
            j->setCallBack(extNpcSlaveJointCB);
        }
    }
}

void resolveSlaveMapPairs(dExtNpcMount_c* a) {
    if (a == NULL) {
        return;
    }
    a->mSlavePairCount = 0;
    if (a->mSlaveMap[0] == '\0' || a->mpCompanion == NULL || a->mpCompanion->getModel() == NULL ||
        a->mpMorf == NULL || a->mpMorf->getModel() == NULL) {
        return;
    }
    J3DModel* body = a->mpMorf->getModel();
    J3DModel* slave = a->mpCompanion->getModel();
    J3DModelData* hostData = body->getModelData();
    J3DModelData* slaveData = slave->getModelData();
    if (hostData == NULL || slaveData == NULL) {
        return;
    }
    JUTNameTab* hostNames = hostData->getJointTree().getJointName();
    JUTNameTab* slaveNames = slaveData->getJointTree().getJointName();
    if (hostNames == NULL || slaveNames == NULL) {
        return;
    }
    const char* p = a->mSlaveMap;
    while (*p && a->mSlavePairCount < kExtNpcMaxSlavePairs) {
        while (*p == ' ' || *p == ',') {
            ++p;
        }
        if (*p == '\0') {
            break;
        }
        char compName[32] = {};
        char bodyName[32] = {};
        int ci = 0;
        while (*p && *p != ':' && *p != ',' && ci + 1 < (int)sizeof(compName)) {
            if (*p != ' ') {
                compName[ci++] = *p;
            }
            ++p;
        }
        compName[ci] = '\0';
        if (*p != ':') {
            break;
        }
        ++p;
        int bi = 0;
        while (*p && *p != ',' && bi + 1 < (int)sizeof(bodyName)) {
            if (*p != ' ') {
                bodyName[bi++] = *p;
            }
            ++p;
        }
        bodyName[bi] = '\0';
        if (compName[0] == '\0' || bodyName[0] == '\0') {
            continue;
        }
        const s32 cIdx = slaveNames->getIndex(compName);
        const s32 hIdx = hostNames->getIndex(bodyName);
        if (cIdx < 0 || cIdx >= slaveData->getJointNum() || hIdx < 0 ||
            hIdx >= hostData->getJointNum()) {
            DuskLog.warn("[ExtNpcMount] slave_map miss '{}'→'{}' (c={} h={})", compName, bodyName,
                         (int)cIdx, (int)hIdx);
            continue;
        }
        const u8 slot = a->mSlavePairCount++;
        a->mSlaveCompJnt[slot] = (s16)cIdx;
        a->mSlaveBodyJnt[slot] = (s16)hIdx;
    }
}

// №25 F2: snap to ground only when probe is within 500 of authored home Y.
// Rejects seafloor under the island while Outset dzb is still settling.
bool tryGroundSnapSanity(dExtNpcMount_c* a) {
    if (a == NULL || a->mIsBg || a->mGroundSnapped) {
        return a != NULL && a->mGroundSnapped;
    }
    cXyz probe = a->current.pos;
    probe.y = a->home.pos.y + 200.0f;
    if (!fopAcM_gc_c::gndCheck(&probe)) {
        if (++a->mGroundSnapTries > 90) {
            a->current.pos.y = a->home.pos.y;
            a->old.pos = a->current.pos;
            a->mGroundSnapped = true;
        }
        return a->mGroundSnapped;
    }
    const f32 gy = fopAcM_gc_c::getGroundY();
    if (gy < a->home.pos.y - 500.0f) {
        if (++a->mGroundSnapTries > 90) {
            // Island never reported — keep authored (never seat in salvage zone).
            a->current.pos.y = a->home.pos.y;
            a->old.pos = a->current.pos;
            a->mGroundSnapped = true;
            DuskLog.warn("[ExtNpcMount] ground snap rejected seafloor y={} for {} (keep {})",
                         gy, a->mManifest.proc, a->home.pos.y);
        }
        return a->mGroundSnapped;
    }
    a->current.pos.y = gy;
    a->old.pos = a->current.pos;
    a->mGroundSnapped = true;
    return true;
}

// ============================================================================
// №251 — the get-item HANDOFF, fired at talk close (native talk→item pattern).
// ============================================================================
// Spawns the presentation item (Cursor's mature create path resolves the WW
// prop model behind this item number), then orders the change event — which
// REPLACES the still-active speak event. The caller must NOT event_reset when
// this fires; the ordered event owns the transition from here.
bool fireMountPresentDemo(dExtNpcMount_c* a) {
#if TARGET_PC
    if (a == NULL || a->mPresentDemoItemNo < 0) {
        return false;
    }
    const int itemNo = a->mPresentDemoItemNo;
    a->mPresentDemoItemNo = -1;
    // §66: Link's setTradeItemAnime present path passes argFlags 3 (0x1|0x2) —
    // 0x1 suppresses a second execItemGet; 0x2 is the raised-hands offset class.
    // Mount handoff used 0; match Link so the demo-item flag/show path aligns.
    const fpc_ProcID itemId =
        fopAcM_createItemForPresentDemo(&a->current.pos, itemNo, 3, -1, -1, NULL, NULL);
    if (itemId == fpcM_ERROR_PROCESS_ID_e) {
        DuskLog.warn("[ExtNpcMount] №251 present-demo spawn failed (item {})", itemNo);
        return false;
    }
    dComIfGp_event_setItemPartnerId(itemId);
    const s16 eventId = dComIfGp_getEventManager().getEventIdx(a, "DEFAULT_GETITEM", 0xff);
    if (eventId == -1) {
        DuskLog.warn("[ExtNpcMount] №251 DEFAULT_GETITEM not found — handoff aborted");
        return false;
    }
    dComIfGp_getEvent()->reset(a);
    fopAcM_orderChangeEventId(a, eventId, 1, 0xffff);
    // ========================================================================
    // №263 — the PRESENT motion (donor: hold.bck is Ba1's give animation, NOT
    // an idle — looping it as idle was the "constant present" bug). Play it
    // ONCE here at the handoff; execute() returns her to idle when it ends.
    // ========================================================================
    if (a->mManifest.presentAnim[0] != '\0') {
        setMountAnimation(a, a->mManifest.presentAnim, J3DFrameCtrl::EMode_NONE);
        a->mPresentAnimActive = true;
    }
    DuskLog.info("[ExtNpcMount] №251 get-item HANDOFF fired (item {}, event {})", itemNo,
                 (int)eventId);
    return true;
#else
    return false;
#endif
}

// ============================================================================
// №262 — flag-gated attach lifetime + carry-idle selection (donor state
// machine: Ba1 holds ba_cloth with hold.bck until the give flag sets, then
// wait01 with the bundle retired — the handover IS the state flip).
// ============================================================================
bool mountAttachLive(const dExtNpcMount_c* a, int slot) {
    const char* f = a->mManifest.attach[slot].unlessFlag;
    if (f[0] == '\0') {
        return true;
    }
    return !dExtModFlags_get(a->mManifest.modFolder, f);
}

const char* mountIdleBck(const dExtNpcMount_c* a) {
    if (a->mManifest.idleAttached[0] != '\0' && a->mManifest.attachCount > 0 &&
        mountAttachLive(a, 0)) {
        return a->mManifest.idleAttached;
    }
    return a->mManifest.idle;
}

void closeMountDialogue(dExtNpcMount_c* a) {
    if (a == NULL) {
        return;
    }
#if TARGET_PC_NATIVE_UI
    if (s_mountDialogue != NULL) {
        s_mountDialogue->hide();
    }
#endif
    // №251: a pending get-item consumes the event transition — do NOT reset.
    if (fireMountPresentDemo(a)) {
        // handoff owns the event from here
    } else if (a->mTalkEventActive) {
        dComIfGp_event_reset();
    }
    a->mTalking = false;
    a->mTalkEventActive = false;
    a->mTalkFrames = 0;
    a->mCloseCooldown = 12;
    if (s_mountDialogueOwner == a) {
        s_mountDialogueOwner = NULL;
    }
    // №262: idle re-picks by attach state. №263: unless the present motion is
    // in flight (fired above) — execute() restores idle when it completes.
    if (!a->mPresentAnimActive) {
        setMountAnimation(a, mountIdleBck(a), J3DFrameCtrl::EMode_LOOP);
    }
}

struct DialogueSectionParsed {
    std::string text;
    std::string ifFlag;
    std::string unlessFlag;
    std::string setFlag;
    std::string clearFlag;
    std::string next;
    std::string elseSection;
    std::string action;  // e.g. grant:62 or grant:0x3E
};

void applyDialogueAction(const dExtNpcMount_c* a, const std::string& action) {
    if (a == NULL || action.empty()) {
        return;
    }
    if (action.rfind("grant:", 0) == 0) {
        const char* spec = action.c_str() + 6;
        char* end = NULL;
        unsigned long id = std::strtoul(spec, &end, 0);
        if (end == spec || id > 255) {
            DuskLog.warn("[ExtNpcMount] bad grant action '{}'", action);
            return;
        }
        execItemGet(static_cast<u8>(id));
        DuskLog.info("[ExtNpcMount] grant item {} via '{}'", (int)id, a->mManifest.proc);
        return;
    }
    if (action.rfind("clear_flag:", 0) == 0) {
        dExtModFlags_set(a->mManifest.modFolder, action.c_str() + 11, false);
        return;
    }
    if (action.rfind("set_flag:", 0) == 0) {
        dExtModFlags_set(a->mManifest.modFolder, action.c_str() + 9, true);
        return;
    }
    // ========================================================================
    // №238 — grant_outfit:<kind> — the wardrobe-integrated clothes handover.
    // ========================================================================
    // Wraps the whole №232 contract in one dialogue action: record wardrobe
    // ownership (so store/own + the D-pad cycle pick it up), then request the
    // equip. The equip is async + transition-fenced (dAlbwOutfit_equip only
    // acts on OWNED outfits and self-blocks while a MESSAGE is open) — so
    // called mid-dialogue it QUEUES and drains after the box closes, which is
    // exactly the donor's visible order (box -> wearing -> "suit you
    // perfectly"). The wear-state design is LOCKED to TP-native Hero's Clothes
    // (D_ALBW_OUTFIT_HEROS -> dItemNo_WEAR_KOKIRI_e); vanilla-acquired clothes
    // still register via dAlbwOutfit_syncWornOwnership, so double-grant across
    // our scene and TP's own Faron beat is idempotent.
    if (action.rfind("grant_outfit:", 0) == 0) {
#if TARGET_PC
        const char* kindStr = action.c_str() + 13;
        dAlbwOutfitKind kind = D_ALBW_OUTFIT_COUNT;
        if (std::strcmp(kindStr, "heros") == 0) {
            kind = D_ALBW_OUTFIT_HEROS;
        } else {
            DuskLog.warn("[ExtNpcMount] grant_outfit unknown kind '{}'", kindStr);
            return;
        }
        const int itemNo = dAlbwOutfit_itemNoForKind(kind);
        dAlbwOutfit_recordOwnedByItemNo(itemNo);   // wardrobe ownership now true
        dAlbwOutfit_equip(kind);                    // queues; drains after the box
        // ====================================================================
        // №251 — the present-demo is DEFERRED to talk close. Ordering the item
        // event here (at section LOAD) put it under our still-running speak
        // event + the section's box, and closeMountDialogue's event_reset then
        // KILLED the ordered event — the "get-item never starts" of №242/this
        // run. The native pattern is a HANDOFF: the box closes, the item event
        // replaces the speak event, no reset in between. The pending flag is
        // consumed by the close path (fireMountPresentDemo).
        // ====================================================================
        dExtNpcMount_c* mount = const_cast<dExtNpcMount_c*>(a);
        mount->mPresentDemoItemNo = itemNo;
        DuskLog.info("[ExtNpcMount] grant_outfit {} (item {}) via '{}' — owned+equip; "
                     "present-demo deferred to talk close",
                     kindStr, itemNo, a->mManifest.proc);
#endif
        return;
    }
    DuskLog.warn("[ExtNpcMount] unknown dialogue action '{}'", action);
}

bool isDialogueDirective(const std::string& line, const char* key, std::string* valueOut) {
    const size_t keyLen = std::strlen(key);
    if (line.size() <= keyLen || line.compare(0, keyLen, key) != 0 || line[keyLen] != '=') {
        return false;
    }
    *valueOut = line.substr(keyLen + 1);
    return true;
}

bool loadDialogueSectionRaw(const dExtNpcMount_c* a, const char* sectionKey,
                            DialogueSectionParsed* out) {
    if (a == NULL || out == NULL || sectionKey == NULL || sectionKey[0] == '\0') {
        return false;
    }
    const fs::path dir = dusk::ConfigPath / "model_replacements" / a->mManifest.modFolder / "dialogue";
    std::error_code ec;
    if (!fs::is_directory(dir, ec)) {
        return false;
    }
    const std::string wanted = std::string("[") + sectionKey + "]";
    const auto trim = [](std::string& line) {
        while (!line.empty() &&
               (line.back() == '\r' || line.back() == '\n' || line.back() == ' ' ||
                line.back() == '\t')) {
            line.pop_back();
        }
        const size_t first = line.find_first_not_of(" \t");
        line.erase(0, first == std::string::npos ? line.size() : first);
    };
    for (fs::directory_iterator it(dir, ec); !ec && it != fs::directory_iterator(); it.increment(ec)) {
        if (!it->is_regular_file(ec) || it->path().extension() != ".txt") {
            continue;
        }
        std::ifstream in(it->path());
        std::string line;
        bool inSection = false;
        DialogueSectionParsed parsed;
        bool found = false;
        while (std::getline(in, line)) {
            trim(line);
            if (!inSection) {
                inSection = line == wanted;
                continue;
            }
            if (!line.empty() && line[0] == '[') {
                break;
            }
            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue;
            }
            found = true;
            std::string dirVal;
            if (isDialogueDirective(line, "if_flag", &dirVal)) {
                parsed.ifFlag = dirVal;
                continue;
            }
            if (isDialogueDirective(line, "unless_flag", &dirVal)) {
                parsed.unlessFlag = dirVal;
                continue;
            }
            if (isDialogueDirective(line, "set_flag", &dirVal)) {
                parsed.setFlag = dirVal;
                continue;
            }
            if (isDialogueDirective(line, "clear_flag", &dirVal)) {
                parsed.clearFlag = dirVal;
                continue;
            }
            if (isDialogueDirective(line, "next", &dirVal)) {
                parsed.next = dirVal;
                continue;
            }
            if (isDialogueDirective(line, "else", &dirVal)) {
                parsed.elseSection = dirVal;
                continue;
            }
            if (isDialogueDirective(line, "action", &dirVal)) {
                parsed.action = dirVal;
                continue;
            }
            // №27 N5: ww_ref=N pulls authentic BMG line from population/ww_dialogue_full.txt.
            if (isDialogueDirective(line, "ww_ref", &dirVal)) {
                const int idx = atoi(dirVal.c_str());
                const fs::path catalog = dusk::ConfigPath / "model_replacements" /
                                        a->mManifest.modFolder / "population" /
                                        "ww_dialogue_full.txt";
                std::ifstream cat(catalog);
                std::string catLine;
                char wanted[32];
                std::snprintf(wanted, sizeof(wanted), "[%d]", idx);
                while (std::getline(cat, catLine)) {
                    trim(catLine);
                    if (catLine.compare(0, std::strlen(wanted), wanted) != 0) {
                        continue;
                    }
                    std::string body = catLine.substr(std::strlen(wanted));
                    while (!body.empty() && (body[0] == ' ' || body[0] == '\t')) {
                        body.erase(0, 1);
                    }
                    // ========================================================
                    // №251 — the catalog stores ONE ENTRY ACROSS SEVERAL
                    // PHYSICAL LINES (the extractor wrapped at the donor's own
                    // line breaks). Reading only the [N] line truncated every
                    // multi-line message to its first fragment — the user's
                    // "Hanging the family" screenshot IS row 543's first line.
                    // Consume continuation lines until the next [entry] or a
                    // blank line, joining with real newlines.
                    // ========================================================
                    std::string contLine;
                    while (std::getline(cat, contLine)) {
                        trim(contLine);
                        if (contLine.empty() || contLine[0] == '[') {
                            break;
                        }
                        body += '\n';
                        body += contLine;
                    }
                    // №32 B6 / №29 C3: honor literal "\n", real newlines, and legacy " / ".
                    std::string flat;
                    for (size_t i = 0; i < body.size(); ++i) {
                        if (body[i] == '\\' && i + 1 < body.size() &&
                            (body[i + 1] == 'n' || body[i + 1] == 'N')) {
                            flat += '\n';
                            ++i;
                            continue;
                        }
                        if (body[i] == '/' && (i == 0 || body[i - 1] == ' ') &&
                            i + 1 < body.size() && body[i + 1] == ' ') {
                            flat += '\n';
                            ++i;  // skip space after /
                            continue;
                        }
                        flat += body[i];
                    }
                    if (!parsed.text.empty()) {
                        parsed.text += '\n';
                    }
                    // Keep Speaker: prefix if the section already started one; else raw.
                    parsed.text += flat;
                    break;
                }
                continue;
            }
            if (!parsed.text.empty()) {
                parsed.text += '\n';
            }
            for (size_t i = 0; i < line.size(); ++i) {
                if (line[i] == '\\' && i + 1 < line.size() && line[i + 1] == 'n') {
                    parsed.text += '\n';
                    ++i;
                } else {
                    parsed.text += line[i];
                }
            }
        }
        if (found) {
            *out = std::move(parsed);
            return true;
        }
    }
    return false;
}

bool sectionConditionsOk(const dExtNpcMount_c* a, const DialogueSectionParsed& sec) {
    const char* mod = a->mManifest.modFolder;
    if (!sec.ifFlag.empty() && !dExtModFlags_get(mod, sec.ifFlag.c_str())) {
        return false;
    }
    if (!sec.unlessFlag.empty() && dExtModFlags_get(mod, sec.unlessFlag.c_str())) {
        return false;
    }
    return true;
}

bool beginMountDialogue(dExtNpcMount_c* a, bool fromEvent) {
    // №248: an actor may re-enter to ADVANCE its own conversation (section
    // chain under one held speak event); only a DIFFERENT owner blocks.
    if (a == NULL || a->mCloseCooldown > 0 ||
        (s_mountDialogueOwner != NULL && s_mountDialogueOwner != a)) {
        return false;
    }
    const auto refuse = [&](const char* reason, bool playTalk) {
        DuskLog.warn("[ExtNpcMount] talk refused: {}", reason);
        if (playTalk) {
            setMountAnimation(a, a->mManifest.talk1, J3DFrameCtrl::EMode_LOOP);
        }
        if (fromEvent) {
            dComIfGp_event_reset();
        }
        a->mDialogueNext[0] = '\0';
        return false;
    };

    if (a->mDialogueSection[0] == '\0') {
        std::snprintf(a->mDialogueSection, sizeof(a->mDialogueSection), "%s",
                      a->mManifest.dialogueKey);
    }

    char key[64];
    std::snprintf(key, sizeof(key), "%s", a->mDialogueSection);
    DialogueSectionParsed sec;
    bool resolved = false;
    for (int depth = 0; depth < 8; ++depth) {
        DialogueSectionParsed loaded;
        if (!loadDialogueSectionRaw(a, key, &loaded)) {
            return refuse("dialogue section missing or empty", true);
        }
        if (!sectionConditionsOk(a, loaded)) {
            if (!loaded.elseSection.empty()) {
                std::snprintf(key, sizeof(key), "%s", loaded.elseSection.c_str());
                continue;
            }
            return refuse("dialogue gated by flag", true);
        }
        if (!loaded.setFlag.empty()) {
            dExtModFlags_set(a->mManifest.modFolder, loaded.setFlag.c_str(), true);
        }
        if (!loaded.clearFlag.empty()) {
            dExtModFlags_set(a->mManifest.modFolder, loaded.clearFlag.c_str(), false);
        }
        if (!loaded.action.empty()) {
            applyDialogueAction(a, loaded.action);
        }
        // Empty body + next= ⇒ silent hop (flag-only / router sections).
        if (loaded.text.empty() && !loaded.next.empty()) {
            std::snprintf(key, sizeof(key), "%s", loaded.next.c_str());
            continue;
        }
        sec = std::move(loaded);
        resolved = true;
        break;
    }
    if (!resolved || sec.text.empty()) {
        return refuse("dialogue section missing or empty", true);
    }

    if (dComIfGp_getMsgCommonArchive() == NULL) {
        return refuse("message archive unavailable", true);
    }
#if TARGET_PC_NATIVE_UI
    // ========================================================================
    // №248 — NATIVE presentation: inject the section text into the engine's
    // code-text flow (Shade Watcher pattern). The speak event ordered by the
    // talk trigger owns Link for the WHOLE conversation; the per-frame poll
    // drives s_mountFlow.doFlow and chains sections without releasing it.
    // №252 — the section text is PAGINATED to TP's box rules first; the poll
    // walks the pages before advancing to the next section.
    // ========================================================================
    mountPaginate(sec.text);
    s_mountFlow.initWord(a, s_mountPages[0].c_str(), 0xFF, 0, NULL);
    s_mountPageIdx = 1;
    if (!sec.next.empty()) {
        std::snprintf(a->mDialogueNext, sizeof(a->mDialogueNext), "%s", sec.next.c_str());
    } else {
        a->mDialogueNext[0] = '\0';
    }
    std::snprintf(a->mDialogueSection, sizeof(a->mDialogueSection), "%s", key);
    a->mTalking = true;
    a->mTalkEventActive = fromEvent;
    a->mTalkFrames = 0;
    s_mountDialogueOwner = a;
    setMountAnimation(a, a->mManifest.talk1, J3DFrameCtrl::EMode_LOOP);
    return true;
#else
    return refuse("native dialogue disabled", false);
#endif
}

// Match itemmdl BDL path: DoBdlMaterialCalc → readPatchedMaterial.
static constexpr u32 kExtNpcBdlFlags = 0x59020010u | 0x2000u;

void trimInPlace(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) {
        s.pop_back();
    }
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) {
        ++i;
    }
    if (i > 0) {
        s.erase(0, i);
    }
}

bool parseBoolVal(const std::string& val) {
    return val == "1" || val == "true" || val == "yes" || val == "on";
}

bool parseVec3(const std::string& value, cXyz* out) {
    return out != NULL &&
           sscanf(value.c_str(), "%f,%f,%f", &out->x, &out->y, &out->z) == 3;
}

bool parseManifestFile(const fs::path& path, const char* modFolder, dExtNpcManifest* out) {
    std::ifstream in(path);
    if (!in) {
        return false;
    }
    memset(out, 0, sizeof(*out));
    out->cylRadius = 40.0f;
    out->cylHeight = 100.0f;
    out->scale = 1.0f;
    out->socketArg = -1;
    out->hostRoom = 0;
    out->hostLayer = -1;
    out->colorFrame = -1.0f;  // №36 C: <0 ⇒ derive from create params / item id
    out->allowWarm = true;    // №115: warm=0 opts out of №58-B storm
    // Neutral gray ambient default (manifest may override).
    out->ambR = 90;
    out->ambG = 90;
    out->ambB = 90;
    snprintf(out->modFolder, sizeof(out->modFolder), "%s", modFolder);

    dExtNpcSubtype* subtype = NULL;
    std::string line;
    while (std::getline(in, line)) {
        trimInPlace(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        if (line[0] == '[') {
            subtype = NULL;
            int index = -1;
            if (sscanf(line.c_str(), "[subtype.%d]", &index) == 1 &&
                index >= 0 && index < kExtNpcMaxSubtype) {
                subtype = &out->subtypes[index];
                subtype->valid = true;
                if (index + 1 > out->subtypeCount) {
                    out->subtypeCount = index + 1;
                }
            }
            continue;
        }
        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        trimInPlace(key);
        trimInPlace(val);
        for (char& c : key) {
            if (c >= 'A' && c <= 'Z') {
                c = (char)(c - 'A' + 'a');
            }
        }
        auto set = [&](char* dst, size_t n) { snprintf(dst, n, "%s", val.c_str()); };
        if (subtype != NULL) {
            if (key == "arg") {
                subtype->arg = atoi(val.c_str());
            } else if (key == "idle") {
                set(subtype->idle, sizeof(subtype->idle));
            } else if (key == "attach_model") {
                set(subtype->attachModel, sizeof(subtype->attachModel));
            } else if (key == "attach_joint") {
                set(subtype->attachJoint, sizeof(subtype->attachJoint));
            } else if (key == "display_name") {
                set(subtype->displayName, sizeof(subtype->displayName));
            }
        } else if (key == "proc") {
            set(out->proc, sizeof(out->proc));
        } else if (key == "socket") {
            set(out->socket, sizeof(out->socket));
        } else if (key == "socket_arg") {
            out->socketArg = atoi(val.c_str());
        } else if (key == "warp_label") {
            set(out->warpLabel, sizeof(out->warpLabel));
        } else if (key == "host_stage") {
            set(out->hostStage, sizeof(out->hostStage));
        } else if (key == "host_room") {
            out->hostRoom = atoi(val.c_str());
        } else if (key == "host_layer") {
            out->hostLayer = atoi(val.c_str());
        } else if (key == "host_pos") {
            if (parseVec3(val, &out->hostPos)) {
                out->hasHostPos = true;
            }
        } else if (key == "arc") {
            set(out->arc, sizeof(out->arc));
        } else if (key == "model") {
            set(out->model, sizeof(out->model));
        } else if (key == "model2") {
            set(out->model2, sizeof(out->model2));
        } else if (key == "model3") {
            set(out->model3, sizeof(out->model3));
        } else if (key == "model2_btk") {
            set(out->model2Btk, sizeof(out->model2Btk));
        } else if (key == "collision") {
            set(out->collision, sizeof(out->collision));
        } else if (key == "type") {
            out->isBg = val == "bg";
        } else if (key == "model_space") {
            out->modelSpaceLocal = (val == "local");
        } else if (key == "anchor") {
            if (parseVec3(val, &out->anchor)) {
                out->hasAnchor = true;
            }
        } else if (key == "spawn_rel") {
            if (parseVec3(val, &out->spawnRel)) {
                out->hasSpawnRel = true;
            }
        } else if (key == "spawn_ry" || key == "spawn_angle") {
            out->spawnRy = (s16)atoi(val.c_str());
            out->hasSpawnRy = true;
        } else if (key == "return_pos") {
            if (parseVec3(val, &out->returnPos)) {
                out->hasReturnPos = true;
            }
        } else if (key == "return_ry") {
            out->returnRy = (s16)atoi(val.c_str());
            out->hasReturnRy = true;
        } else if (key == "exit_door_rel") {
            if (parseVec3(val, &out->exitDoorRel)) {
                out->hasExitDoorRel = true;
            }
        } else if (key == "idle") {
            set(out->idle, sizeof(out->idle));
        } else if (key == "talk1") {
            set(out->talk1, sizeof(out->talk1));
        } else if (key == "talk2") {
            set(out->talk2, sizeof(out->talk2));
        } else if (key == "btp") {
            set(out->btp, sizeof(out->btp));
        } else if (key == "blink_btp") {
            set(out->blinkBtp, sizeof(out->blinkBtp));  // №188
        } else if (key == "display_name") {
            set(out->displayName, sizeof(out->displayName));
        } else if (key == "neck_joint") {
            set(out->neckJoint, sizeof(out->neckJoint));
        } else if (key == "dialogue") {
            set(out->dialogueKey, sizeof(out->dialogueKey));
        } else if (key == "attach_model" || key == "attach_model2") {
            const int index = key == "attach_model2" ? 1 : 0;
            set(out->attach[index].model, sizeof(out->attach[index].model));
            if (index + 1 > out->attachCount) {
                out->attachCount = index + 1;
            }
        } else if (key == "attach_joint" || key == "attach_joint2") {
            const int index = key == "attach_joint2" ? 1 : 0;
            set(out->attach[index].joint, sizeof(out->attach[index].joint));
            if (index + 1 > out->attachCount) {
                out->attachCount = index + 1;
            }
        // ====================================================================
        // №218 — donor-authored local transform for a held prop attach.
        // attach_offs = x,y,z (model units); attach_rot = x,y,z (raw s16 angle
        // units, decimal). Either key alone marks the slot as having a local.
        // ====================================================================
        } else if (key == "attach_offs" || key == "attach_offs2") {
            const int index = key == "attach_offs2" ? 1 : 0;
            if (parseVec3(val, &out->attach[index].offs)) {
                out->attach[index].hasLocal = true;
            }
        } else if (key == "attach_rot" || key == "attach_rot2") {
            const int index = key == "attach_rot2" ? 1 : 0;
            if (parseVec3(val, &out->attach[index].rot)) {
                out->attach[index].hasLocal = true;
            }
        // №250: the in-demo pose variant (donor m841 split — Ls1 telescope).
        } else if (key == "attach_offs_demo" || key == "attach_offs_demo2") {
            const int index = key == "attach_offs_demo2" ? 1 : 0;
            if (parseVec3(val, &out->attach[index].offsDemo)) {
                out->attach[index].hasDemoLocal = true;
            }
        } else if (key == "attach_rot_demo" || key == "attach_rot_demo2") {
            const int index = key == "attach_rot_demo2" ? 1 : 0;
            if (parseVec3(val, &out->attach[index].rotDemo)) {
                out->attach[index].hasDemoLocal = true;
            }
        // №262: flag-gated prop lifetime + carry-idle (Grandma's bundle).
        } else if (key == "attach_unless_flag" || key == "attach_unless_flag2") {
            const int index = key == "attach_unless_flag2" ? 1 : 0;
            set(out->attach[index].unlessFlag, sizeof(out->attach[index].unlessFlag));
        } else if (key == "idle_attached") {
            set(out->idleAttached, sizeof(out->idleAttached));
        } else if (key == "present_anim") {
            set(out->presentAnim, sizeof(out->presentAnim));  // №263
        } else if (key == "companion_model") {
            set(out->companionModel, sizeof(out->companionModel));
        } else if (key == "companion_hidden") {
            // №249: presence axis 3 at attachment scale — mounted but not
            // presented until the scene pass flips it (Grandma's bundle).
            out->companionHidden = parseBoolVal(val);
        } else if (key == "companion_idle") {
            set(out->companionIdle, sizeof(out->companionIdle));
        } else if (key == "companion_mode") {
            // №36 A: joint_slave | synced (default)
            for (char& c : val) {
                if (c >= 'A' && c <= 'Z') {
                    c = (char)(c - 'A' + 'a');
                }
            }
            out->companionMode = (val == "joint_slave" || val == "slave" || val == "1") ? 1 : 0;
        } else if (key == "brk") {
            set(out->brk, sizeof(out->brk));
        } else if (key == "btk") {
            set(out->btk, sizeof(out->btk));
        } else if (key == "color_frame") {
            out->colorFrame = (f32)atof(val.c_str());
        } else if (key == "pickup_rupee") {
            out->pickupRupee = parseBoolVal(val);
        } else if (key == "door_open_bck") {
            set(out->doorOpenBck, sizeof(out->doorOpenBck));
        } else if (key == "cyl_radius") {
            out->cylRadius = (f32)atof(val.c_str());
        } else if (key == "cyl_height") {
            out->cylHeight = (f32)atof(val.c_str());
        } else if (key == "scale") {
            out->scale = (f32)atof(val.c_str());
            if (out->scale <= 0.01f) {
                out->scale = 1.0f;
            }
        } else if (key == "skip_btp") {
            out->skipBtp = parseBoolVal(val);
        } else if (key == "amb" || key == "ambient") {
            // amb=RRGGBB | #RRGGBB | R,G,B
            std::string v = val;
            if (!v.empty() && v[0] == '#') {
                v.erase(0, 1);
            }
            unsigned r = 90, g = 90, b = 90;
            if (v.find(',') != std::string::npos) {
                sscanf(v.c_str(), "%u,%u,%u", &r, &g, &b);
            } else if (v.size() >= 6) {
                sscanf(v.c_str(), "%02x%02x%02x", &r, &g, &b);
            }
            out->ambR = (u8)(r > 255 ? 255 : r);
            out->ambG = (u8)(g > 255 ? 255 : g);
            out->ambB = (u8)(b > 255 ? 255 : b);
        } else if (key == "amb_r") {
            out->ambR = (u8)(atoi(val.c_str()) & 0xFF);
        } else if (key == "amb_g") {
            out->ambG = (u8)(atoi(val.c_str()) & 0xFF);
        } else if (key == "amb_b") {
            out->ambB = (u8)(atoi(val.c_str()) & 0xFF);
        } else if (key == "source") {
            for (char& c : val) {
                if (c >= 'A' && c <= 'Z') {
                    c = (char)(c - 'A' + 'a');
                }
            }
            out->fromDvd = (val == "dvd" || val == "game" || val == "tp");
        } else if (key == "population") {
            set(out->populationCsv, sizeof(out->populationCsv));
        } else if (key == "population_stage") {
            set(out->populationStage, sizeof(out->populationStage));
        } else if (key == "spawn_if_flag") {
            set(out->spawnIfFlag, sizeof(out->spawnIfFlag));
        } else if (key == "spawn_unless_flag") {
            set(out->spawnUnlessFlag, sizeof(out->spawnUnlessFlag));
        } else if (key == "carryable") {
            out->carryable = parseBoolVal(val);
        } else if (key == "static") {
            out->isStatic = parseBoolVal(val);
        } else if (key == "codegen") {
            out->isCodeGeom = parseBoolVal(val);
        } else if (key == "door") {
            out->doorAttention = parseBoolVal(val);
        } else if (key == "door_visual") {
            set(out->doorVisual, sizeof(out->doorVisual));
        } else if (key == "body_bmt") {
            set(out->bodyBmt, sizeof(out->bodyBmt));
        } else if (key == "warm") {
            // №115: warm=0 / false — never №58-B cold-create this BG from the island.
            out->allowWarm = parseBoolVal(val);
        }
    }
    // №50-E: door controller (door.bdl) needs a visible variant; Nintendo default = door_a.
    if (out->doorAttention && out->doorVisual[0] == '\0') {
        snprintf(out->doorVisual, sizeof(out->doorVisual), "%s", "door_a.bdl");
    }
    // Phase M: no character dialogue defaults in code — require dialogue= or G1 refuse.
    if (out->socket[0] == '\0' && out->proc[0]) {
        snprintf(out->socket, sizeof(out->socket), "%s", out->proc);
    }
    // №32: static props/doors may omit idle (NULL anm). BG needs collision=.
    // №117: collision-only props (Akabe) — static + collision, model optional.
    if (out->isCodeGeom) {
        // №126: code-driven geometry (ported vegetation lane). No arc, no model
        // — the actor builds its own draw from an extracted asset pack, so the
        // only thing that must be present is the proc the spawner routes to.
        out->valid = out->proc[0] != '\0';
    } else if (out->isBg) {
        out->valid = out->proc[0] && out->arc[0] && out->collision[0] != '\0';
    } else {
        out->valid = out->proc[0] && out->arc[0] &&
                     (out->model[0] || (out->isStatic && out->collision[0])) &&
                     (out->idle[0] || out->isStatic);
    }
    return out->valid;
}

bool arcNameEqualsIgnoreCase(const std::string& stem, const char* arcName) {
    if (arcName == NULL || stem.size() != std::strlen(arcName)) {
        return false;
    }
    for (size_t i = 0; i < stem.size(); ++i) {
        char a = stem[i];
        char b = arcName[i];
        if (a >= 'A' && a <= 'Z') {
            a = static_cast<char>(a - 'A' + 'a');
        }
        if (b >= 'A' && b <= 'Z') {
            b = static_cast<char>(b - 'A' + 'a');
        }
        if (a != b) {
            return false;
        }
    }
    return true;
}

bool arcFilePresentInDir(const fs::path& arcsDir, const char* arcName) {
    std::error_code ec;
    if (!fs::is_directory(arcsDir, ec)) {
        return false;
    }
    const fs::path lower = arcsDir / (std::string(arcName) + ".arc");
    if (fs::is_regular_file(lower, ec)) {
        return true;
    }
    const fs::path upper = arcsDir / (std::string(arcName) + ".ARC");
    if (fs::is_regular_file(upper, ec)) {
        return true;
    }
    for (auto it = fs::directory_iterator(arcsDir, ec); it != fs::directory_iterator();
         it.increment(ec)) {
        if (ec || !it->is_regular_file(ec)) {
            continue;
        }
        const auto ext = it->path().extension().string();
        if (ext != ".arc" && ext != ".ARC") {
            continue;
        }
        if (arcNameEqualsIgnoreCase(it->path().stem().string(), arcName)) {
            return true;
        }
    }
    return false;
}

bool arcFilePresent(const fs::path& modRoot, const char* arcName) {
    if (arcName == NULL || arcName[0] == '\0') {
        return false;
    }
    // №52-C / №110: arcs/ first, then arcs_lib/ library fallback (never dump the lib).
    return arcFilePresentInDir(modRoot / "arcs", arcName) ||
           arcFilePresentInDir(modRoot / "arcs_lib", arcName);
}

bool shouldSkipModFolder(const std::string& modName) {
    // Bak / disabled twins must not scan as live providers (noisy "arcs missing" spam).
    if (modName.find(".SKELETON") != std::string::npos) {
        return true;
    }
    if (modName.size() >= 4 && modName.compare(modName.size() - 4, 4, "_BAK") == 0) {
        return true;
    }
    if (modName.size() >= 9 && modName.compare(modName.size() - 9, 9, ".DISABLED") == 0) {
        return true;
    }
    return false;
}

void stageLog(const char* stage, const char* detail) {
    DuskLog.info("[ExtNpcMount:D1] {} — {}", stage, detail != NULL ? detail : "");
}

// I1 (boots/leaf recipe — History №12c): WW lit channels ship litMask 0x03 (slots 0+1).
// TP actor/itemmdl host path only reliably fills slot 0; slot-1 garbage → black body
// (eyes stay bright because unlit). Working WW-boots overlay Kmdl_13.bmd uses 0x01.
// Clamp ENABLED channels to 0x01; leave unlit channels alone. Do NOT force 0xFF.
void applyActorLightMask(J3DModelData* modelData) {
    if (modelData == NULL) {
        return;
    }
    for (u16 i = 0; i < modelData->getMaterialNum(); i++) {
        J3DMaterial* material = modelData->getMaterialNodePointer(i);
        if (material == NULL || material->getColorChan(0) == NULL) {
            continue;
        }
        J3DColorChan* chan = material->getColorChan(0);
        if (chan->getEnable() == 0) {
            continue;  // unlit (eyes) — leave pristine
        }
        const u8 lightMask = chan->getLightMask();
        if (lightMask != 0x01) {
            chan->setLightMask(0x01);
            material->change();
        }
    }
}

// Load BDL/BMD into a fresh J3DModelData* without finish (shared-DL / light-mask).
J3DModelData* loadMountedModelDataOnly(void* res) {
    if (res == NULL) {
        stageLog("resolve", "FAIL res=NULL");
        return NULL;
    }
    const J3DModelFileData* header = (const J3DModelFileData*)res;
    char magicBuf[64];
    snprintf(magicBuf, sizeof(magicBuf), "magic1=%08x magic2=%08x", (u32)header->mMagic1,
             (u32)header->mMagic2);
    stageLog("resolve", magicBuf);

    if (header->mMagic1 == 'J3D2' &&
        (header->mMagic2 == 'bdl4' || header->mMagic2 == 'bdl3')) {
        stageLog("resolve", "path=loadBinaryDisplayList");
        return J3DModelLoaderDataBase::loadBinaryDisplayList(res, kExtNpcBdlFlags);
    }
    if (header->mMagic1 == 'J3D2' &&
        (header->mMagic2 == 'bmd3' || header->mMagic2 == 'bmd2')) {
        stageLog("resolve", "path=load (bmd)");
        return (J3DModelData*)J3DModelLoaderDataBase::load(res, 0x59020010);
    }
    stageLog("resolve", "path=preinstantiated J3DModelData*");
    J3DModelData* data = (J3DModelData*)res;
    if (data->getMaterialNum() == 0 || data->getMaterialNodePointer(0) == NULL) {
        stageLog("resolve", "FAIL preinstantiated model has no material[0]");
        return NULL;
    }
    return data;
}

J3DMaterialTable* loadMountedBmtTable(void* raw) {
    if (raw == NULL) {
        return NULL;
    }
    const J3DModelFileData* header = (const J3DModelFileData*)raw;
    if (header->mMagic1 != 'J3D2') {
        return NULL;
    }
    if (header->mMagic2 == 'bmt3') {
        J3DModelLoader_v26 loader;
        return loader.loadMaterialTable(raw);
    }
    if (header->mMagic2 == 'bmt2') {
        J3DModelLoader_v21 loader;
        return loader.loadMaterialTable(raw);
    }
    return NULL;
}

// №50-C: apply body_bmt before finish. ko02.bmt is TEX1-only (shirt textures);
// other BMTs may also carry MAT3 overrides.
bool applyBodyBmtToModelData(J3DModelData* data, const char* arc, const char* bmtName) {
    if (data == NULL || arc == NULL || bmtName == NULL || bmtName[0] == '\0') {
        return false;
    }
    void* raw = dComIfG_getObjectRes(arc, bmtName);
    if (raw == NULL) {
        DuskLog.warn("[ExtNpcMount] body_bmt '{}' missing in arc '{}'", bmtName, arc);
        return false;
    }
    J3DMaterialTable* bmt = loadMountedBmtTable(raw);
    if (bmt == NULL) {
        DuskLog.warn("[ExtNpcMount] body_bmt '{}' unparseable", bmtName);
        return false;
    }
    u16 matCopied = 0;
    if (bmt->getMaterialNum() > 0) {
        JUTNameTab* srcNames = bmt->getMaterialName();
        JUTNameTab* dstNames = data->getMaterialName();
        for (u16 i = 0; i < bmt->getMaterialNum(); ++i) {
            J3DMaterial* src = bmt->getMaterialNodePointer(i);
            if (src == NULL) {
                continue;
            }
            s32 dstIdx = -1;
            if (srcNames != NULL && dstNames != NULL) {
                const char* n = srcNames->getName(i);
                if (n != NULL && n[0]) {
                    dstIdx = dstNames->getIndex(n);
                }
            }
            if (dstIdx < 0 && i < data->getMaterialNum()) {
                dstIdx = (s32)i;
            }
            if (dstIdx < 0 || dstIdx >= (s32)data->getMaterialNum()) {
                continue;
            }
            J3DMaterial* dst = data->getMaterialNodePointer((u16)dstIdx);
            if (dst == NULL) {
                continue;
            }
            dst->copy(src);
            ++matCopied;
        }
    }
    bool texReplaced = false;
    if (bmt->getTexture() != NULL) {
        data->getMaterialTable().replaceTextures(bmt->getTexture(), bmt->getTextureName());
        texReplaced = true;
    }
    DuskLog.info("[ExtNpcMount] body_bmt '{}' applied mats={} tex={}", bmtName, matCopied,
                 texReplaced ? "yes" : "no");
    return matCopied > 0 || texReplaced;
}

J3DModelData* finishMountedModelData(J3DModelData* modelData) {
    stageLog("finish", "enter");
    if (modelData == NULL) {
        stageLog("finish", "FAIL modelData=NULL");
        return NULL;
    }
    const u16 matNum = modelData->getMaterialNum();
    char buf[96];
    snprintf(buf, sizeof(buf), "matNum=%u jointNum=%u", matNum, modelData->getJointNum());
    stageLog("finish", buf);
    if (matNum == 0) {
        stageLog("finish", "FAIL matNum=0");
        return NULL;
    }
    J3DMaterial* mat0 = modelData->getMaterialNodePointer(0);
    if (mat0 == NULL) {
        stageLog("finish", "FAIL material[0]=NULL");
        return NULL;
    }

    applyActorLightMask(modelData);

    for (u16 i = 0; i < matNum; i++) {
        J3DMaterial* material = modelData->getMaterialNodePointer(i);
        if (material == NULL) {
            snprintf(buf, sizeof(buf), "FAIL material[%u]=NULL", i);
            stageLog("finish", buf);
            return NULL;
        }
        material->change();
        J3DMaterialAnm* materialAnm = JKR_NEW J3DMaterialAnm();
        if (materialAnm == NULL) {
            stageLog("finish", "FAIL J3DMaterialAnm alloc");
            return NULL;
        }
        material->setMaterialAnm(materialAnm);
    }
    if (modelData->newSharedDisplayList(J3DMdlFlag_UseSingleDL) != kJ3DError_Success) {
        stageLog("finish", "FAIL newSharedDisplayList");
        return NULL;
    }
    modelData->simpleCalcMaterial(const_cast<MtxP>(j3dDefaultMtx));
    modelData->makeSharedDL();
    stageLog("finish", "ok");
    return modelData;
}

J3DModelData* resolveMountedModelUncached(void* res) {
    J3DModelData* loaded = loadMountedModelDataOnly(res);
    if (loaded == NULL) {
        return NULL;
    }
    // Preinstantiated live ModelData: light-mask only (already finished elsewhere).
    const J3DModelFileData* header = (const J3DModelFileData*)res;
    if (header != NULL && header->mMagic1 == 'J3D2' &&
        (header->mMagic2 == 'bdl4' || header->mMagic2 == 'bdl3' || header->mMagic2 == 'bmd3' ||
         header->mMagic2 == 'bmd2')) {
        return finishMountedModelData(loaded);
    }
    applyActorLightMask(loaded);
    return loaded;
}

// №26 F3: keys published during the current create — evicted on abort.
std::vector<std::string> s_createPublishedKeys;
bool s_createPublishTrack = false;

void beginCreateCacheTrack() {
    s_createPublishedKeys.clear();
    s_createPublishTrack = true;
}

void abortCreateCacheTrack() {
    if (!s_createPublishTrack) {
        return;
    }
    for (const std::string& key : s_createPublishedKeys) {
        auto it = s_modelDataCache.find(key);
        if (it != s_modelDataCache.end()) {
            DuskLog.warn("[ExtNpcMount] F3 cache eviction on abort: {}", key);
            s_modelDataCache.erase(it);
        }
    }
    s_createPublishedKeys.clear();
    s_createPublishTrack = false;
}

void commitCreateCacheTrack() {
    s_createPublishedKeys.clear();
    s_createPublishTrack = false;
}

// №73: keys for arc "LinkRM" are "LinkRM/…" or "bg:LinkRM/…".
static bool cacheKeyBelongsToArc(const std::string& key, const char* arc) {
    if (arc == NULL || arc[0] == '\0') {
        return false;
    }
    const std::string bgPrefix = std::string("bg:") + arc + "/";
    const std::string npcPrefix = std::string(arc) + "/";
    return key.rfind(bgPrefix, 0) == 0 || key.rfind(npcPrefix, 0) == 0;
}

static void purgeModelCacheForArc(const char* arc) {
    if (arc == NULL || arc[0] == '\0') {
        return;
    }
    int nModel = 0;
    for (auto it = s_modelDataCache.begin(); it != s_modelDataCache.end();) {
        if (cacheKeyBelongsToArc(it->first, arc)) {
            // Erase only — do not delete ModelData after/while archive dies (pointer-fixed
            // into the buffer; destructor would UAF). Matches F3 abort eviction.
            it = s_modelDataCache.erase(it);
            ++nModel;
        } else {
            ++it;
        }
    }
    // №263: pristine copies are SESSION-LIVED — do NOT erase them here. They
    // are the only safe parse source once the resident dRes buffer has been
    // pointer-fixed; erasing them on purge caused the KOISI exit crash.
    DuskLog.info("[ExtNpcMount] №73 purged model cache for arc '{}' (models={}, pristine kept)",
                 arc, nModel);
}

static void retainArcModels(const char* arc) {
    if (arc == NULL || arc[0] == '\0') {
        return;
    }
    const int n = ++s_arcLiveCount[arc];
    DuskLog.info("[ExtNpcMount] №73 retain arc '{}' live={}", arc, n);
}

// №100: true if any live mount (except `dying`) still references this arc.
static bool liveMountRefsArc(const char* arc, const fopAc_ac_c* dying) {
    if (arc == NULL || arc[0] == '\0') {
        return false;
    }
    struct Ctx {
        const char* arc;
        const fopAc_ac_c* dying;
        bool hit;
    } ctx{arc, dying, false};
    fopAcIt_Executor(
        [](void* actor, void* data) -> int {
            Ctx* c = (Ctx*)data;
            fopAc_ac_c* ac = (fopAc_ac_c*)actor;
            if (ac == NULL || ac == c->dying) {
                return 0;
            }
            const s16 name = fopAcM_GetName(ac);
            if (name != fpcNm_NPC_HENNA0_e && name != fpcNm_NPC_MK_e && name != fpcNm_NPC_P2_e &&
                name != fpcNm_NPC_KDK_e) {
                return 0;
            }
            dExtNpcMount_c* m = (dExtNpcMount_c*)ac;
            if (m->mManifest.arc[0] && std::strcmp(m->mManifest.arc, c->arc) == 0) {
                c->hit = true;
            }
            return 0;
        },
        &ctx);
    return ctx.hit;
}

// Call BEFORE dComIfG_resDelete. Shared arcs (Knob) only purge at last release.
// №100: `reason` names the caller; refuse purge while another live mount still refs the arc.
static void releaseArcModels(const char* arc, const char* reason, fopAc_ac_c* dying = NULL) {
    if (arc == NULL || arc[0] == '\0') {
        return;
    }
    const char* why = reason != NULL ? reason : "?";
    auto it = s_arcLiveCount.find(arc);
    if (it == s_arcLiveCount.end()) {
        if (liveMountRefsArc(arc, dying)) {
            DuskLog.warn(
                "[ExtNpcMount] №100 refuse purge arc '{}' — no retain but live mount refs "
                "(caller={})",
                arc, why);
            return;
        }
        DuskLog.info("[ExtNpcMount] №100 release arc '{}' (no retain) caller={} → purge", arc, why);
        purgeModelCacheForArc(arc);
        return;
    }
    --it->second;
    if (it->second > 0) {
        DuskLog.info("[ExtNpcMount] №73 release arc '{}' live={} (keep cache — shared) caller={}",
                     arc, it->second, why);
        return;
    }
    if (liveMountRefsArc(arc, dying)) {
        it->second = 1;  // restore — mount still drawing this arc
        DuskLog.warn(
            "[ExtNpcMount] №100 refuse purge arc '{}' — live mount still refs (caller={})", arc,
            why);
        return;
    }
    s_arcLiveCount.erase(it);
    DuskLog.info("[ExtNpcMount] №100 release arc '{}' live=0 caller={} → purge", arc, why);
    purgeModelCacheForArc(arc);
}

// GameHeap-pin + arc-scoped cache. Never mutates dRes slots; safe for N concurrent mounts.
// №26 F3: publish-on-success only; track keys for abort eviction.
// №50-C: optional body_bmt is part of the cache key so ko.bdl+ko02.bmt never poisons plain ko.bdl.
J3DModelData* acquireMountedModel(const char* arc, const char* modelName, void* res,
                                  const char* bodyBmt = NULL) {
    if (arc == NULL || modelName == NULL) {
        return NULL;
    }
    // Stash before any load — J3D parser pointer-fixes the dRes buffer in place.
    ensurePristineJ3dRaw(arc, modelName, res);

    std::string key = std::string(arc) + "/" + modelName;
    const bool wantBmt = bodyBmt != NULL && bodyBmt[0] != '\0';
    if (wantBmt) {
        key += "+";
        key += bodyBmt;
    }
    auto it = s_modelDataCache.find(key);
    if (it != s_modelDataCache.end()) {
        stageLog("resolve", "path=session-cache hit");
        return it->second;
    }

    JKRHeap* gameHeap = (JKRHeap*)mDoExt_getGameHeap();
    JKRHeap* prevHeap = gameHeap != NULL ? mDoExt_setCurrentHeap(gameHeap) : NULL;
    J3DModelData* data = NULL;
    if (wantBmt) {
        const std::string plainKey = std::string(arc) + "/" + modelName;
        auto pit = s_pristineJ3dRaw.find(plainKey);
        void* loadSrc = NULL;
        if (pit != s_pristineJ3dRaw.end()) {
            // №263: fresh copy — never mutate the stored pristine itself.
            loadSrc = mountPristineParseSrc(plainKey, NULL);
            stageLog("resolve", "path=bmt-load-from-pristine-copy");
        } else {
            // No pristine copy (already pointer-fixed before stash) — skip shirt, don't AV.
            DuskLog.warn(
                "[ExtNpcMount] body_bmt '{}' skipped — no pristine raw for {}/{} (would AV on "
                "re-parse)",
                bodyBmt, arc, modelName);
        }
        if (loadSrc != NULL) {
            data = loadMountedModelDataOnly(loadSrc);
            if (data != NULL) {
                applyBodyBmtToModelData(data, arc, bodyBmt);
                data = finishMountedModelData(data);
            }
        }
        if (data == NULL) {
            // Prefer already-finished plain body (never re-parse a pointer-fixed dRes buffer).
            const auto plainIt = s_modelDataCache.find(plainKey);
            if (plainIt != s_modelDataCache.end()) {
                data = plainIt->second;
                stageLog("resolve", "path=bmt-fallback-plain-cache");
            } else if (loadSrc != NULL) {
                // №263: loadSrc was consumed (fixed) by the failed bmt attempt
                // above — take ANOTHER fresh pristine copy for the fallback.
                data = resolveMountedModelUncached(mountPristineParseSrc(plainKey, res));
                stageLog("resolve", "path=bmt-fallback-plain-from-pristine");
            } else {
                data = resolveMountedModelUncached(res);
                stageLog("resolve", "path=bmt-fallback-plain-from-res");
            }
        }
    } else {
        // №263: parse a fresh pristine copy, never the shared dRes buffer —
        // a purged-then-recreated mount would otherwise re-parse a
        // pointer-fixed resident buffer (the KOISI exit crash).
        data = resolveMountedModelUncached(
            mountPristineParseSrc(std::string(arc) + "/" + modelName, res));
    }
    if (prevHeap != NULL) {
        mDoExt_setCurrentHeap(prevHeap);
    }
    if (data == NULL) {
        return NULL;
    }
    // Publish only after full resolve/finish succeeded.
    s_modelDataCache[key] = data;
    if (s_createPublishTrack) {
        s_createPublishedKeys.push_back(key);
    }
    DuskLog.info("[ExtNpcMount] model-data cache + {} heap={} (publish-on-success)", key,
                 gameHeap != NULL ? "GameHeap" : "?");
    return data;
}

// BG/room meshes: load raw J3D2 if dRes left the file unparsed (Outset BDLM path),
// but NEVER applyActorLightMask — that kills unlit/vertex-colored WW room mats.
J3DModelData* finishBgModelData(J3DModelData* modelData) {
    if (modelData == NULL || modelData->getMaterialNum() == 0 ||
        modelData->getMaterialNodePointer(0) == NULL) {
        return NULL;
    }
    for (u16 i = 0; i < modelData->getMaterialNum(); i++) {
        J3DMaterial* material = modelData->getMaterialNodePointer(i);
        if (material == NULL) {
            return NULL;
        }
        material->change();
        J3DMaterialAnm* materialAnm = JKR_NEW J3DMaterialAnm();
        if (materialAnm == NULL) {
            return NULL;
        }
        material->setMaterialAnm(materialAnm);
    }
    if (modelData->newSharedDisplayList(J3DMdlFlag_UseSingleDL) != kJ3DError_Success) {
        return NULL;
    }
    modelData->simpleCalcMaterial(const_cast<MtxP>(j3dDefaultMtx));
    modelData->makeSharedDL();
    return modelData;
}

J3DModelData* resolveBgModelUncached(void* res) {
    if (res == NULL) {
        return NULL;
    }
    const J3DModelFileData* header = (const J3DModelFileData*)res;
    if (header->mMagic1 == 'J3D2' &&
        (header->mMagic2 == 'bdl4' || header->mMagic2 == 'bdl3')) {
        J3DModelData* loaded =
            J3DModelLoaderDataBase::loadBinaryDisplayList(res, kExtNpcBdlFlags);
        return finishBgModelData(loaded);
    }
    if (header->mMagic1 == 'J3D2' &&
        (header->mMagic2 == 'bmd3' || header->mMagic2 == 'bmd2')) {
        J3DModelData* loaded = (J3DModelData*)J3DModelLoaderDataBase::load(res, 0x59020010);
        return finishBgModelData(loaded);
    }
    // Already a live J3DModelData* from dRes — use as-is (no NPC light-mask).
    J3DModelData* data = (J3DModelData*)res;
    if (data->getMaterialNum() == 0 || data->getMaterialNodePointer(0) == NULL) {
        return NULL;
    }
    return data;
}

J3DModelData* acquireBgModel(const char* arc, const char* modelName, void* res) {
    if (arc == NULL || modelName == NULL) {
        return NULL;
    }
    const std::string key = std::string("bg:") + arc + "/" + modelName;
    auto it = s_modelDataCache.find(key);
    if (it != s_modelDataCache.end()) {
        return it->second;
    }
    JKRHeap* gameHeap = (JKRHeap*)mDoExt_getGameHeap();
    JKRHeap* prevHeap = gameHeap != NULL ? mDoExt_setCurrentHeap(gameHeap) : NULL;
    J3DModelData* data = resolveBgModelUncached(res);
    if (prevHeap != NULL) {
        mDoExt_setCurrentHeap(prevHeap);
    }
    if (data == NULL) {
        return NULL;
    }
    s_modelDataCache[key] = data;
    if (s_createPublishTrack) {
        s_createPublishedKeys.push_back(key);
    }
    DuskLog.info("[ExtNpcMount] BG model-data cache + {} (publish-on-success)", key);
    return data;
}

// №108: WW sea-stage sky (mod arcs/WwSky.arc → Object/WwSky.arc). Camera-follow
// like WW daVrbox/daVrbox2. TP stub vrbox stays hidden (N6).
static constexpr int kWwSkyModelCount = 4;
static const char* const kWwSkyModelNames[kWwSkyModelCount] = {
    "vr_sky.bdl",         // VRBOX dome
    "vr_uso_umi.bdl",     // false-sea horizon
    "vr_kasumi_mae.bdl",  // haze
    "vr_back_cloud.bdl",  // clouds (+100 Y in WW)
};
static request_of_phase_process_class s_wwSkyPhase;
static int s_wwSkyPhaseState = cPhs_INIT_e;
static J3DModel* s_wwSkyModels[kWwSkyModelCount] = {};
static bool s_wwSkyReady = false;
static int s_wwSkyUsers = 0;

static bool mountWantsWwSky(const dExtNpcMount_c* a) {
    if (a == NULL || !a->mIsBg || a->mManifest.proc[0] == '\0') {
        return false;
    }
    // Outdoor F_DL* hosts only (Outset / forest). Interiors keep hide_vrbox alone.
    if (std::strcmp(a->mManifest.proc, "EXT_BG0") != 0 &&
        std::strcmp(a->mManifest.proc, "EXT_BG9") != 0) {
        return false;
    }
    const char* stage = dComIfGp_getStartStageName();
    return stage != NULL && dExtWwSave_isWwHostStage(stage) && stage[0] == 'F';
}

static void wwSkyRelease(const char* reason) {
    for (int i = 0; i < kWwSkyModelCount; ++i) {
        s_wwSkyModels[i] = NULL;
    }
    s_wwSkyReady = false;
    if (s_wwSkyPhaseState == cPhs_COMPLEATE_e) {
        purgeModelCacheForArc("WwSky");
        dComIfG_resDelete(&s_wwSkyPhase, "WwSky");
    }
    s_wwSkyPhase = {};
    s_wwSkyPhaseState = cPhs_INIT_e;
    DuskLog.info("[ExtNpcMount] №108 WwSky release ({})", reason != NULL ? reason : "?");
}

static bool wwSkyEnsure() {
    if (s_wwSkyReady) {
        return true;
    }
    s_wwSkyPhaseState = dComIfG_resLoad(&s_wwSkyPhase, "WwSky");
    if (s_wwSkyPhaseState != cPhs_COMPLEATE_e) {
        return false;
    }
    for (int i = 0; i < kWwSkyModelCount; ++i) {
        void* raw = dComIfG_getObjectRes("WwSky", kWwSkyModelNames[i]);
        J3DModelData* data = acquireBgModel("WwSky", kWwSkyModelNames[i], raw);
        if (data == NULL) {
            DuskLog.warn("[ExtNpcMount] №108 WwSky missing '{}'", kWwSkyModelNames[i]);
            wwSkyRelease("model-fail");
            return false;
        }
        // Same create flags as WW/TP vrbox (sky list / fog-friendly).
        s_wwSkyModels[i] = mDoExt_J3DModel__create(data, 0x80000, 0x11020202);
        if (s_wwSkyModels[i] == NULL) {
            DuskLog.warn("[ExtNpcMount] №108 WwSky create failed '{}'", kWwSkyModelNames[i]);
            wwSkyRelease("create-fail");
            return false;
        }
    }
    s_wwSkyReady = true;
    DuskLog.info("[ExtNpcMount] №108 WwSky ready (4 models)");
    return true;
}

static void wwSkyRetain() {
    if (s_wwSkyUsers++ == 0) {
        wwSkyEnsure();
    }
}

static void wwSkyReleaseUser() {
    if (s_wwSkyUsers <= 0) {
        return;
    }
    if (--s_wwSkyUsers == 0) {
        wwSkyRelease("last-user");
    }
}

// №121 / №145: WwSky is WW's dome set, not TP's mpSoraModel.
// WW d_a_vrbox / d_a_vrbox2 colour sky+uso+clouds via setTevKColor (TEV
// colorIn = KONST); only kasumi_mae uses setTevColor (C0). TP's
// daVrbox_color_set writes C0 because TP's models consume C0 — copying that
// onto WW's KONST stages left C0 unused and K stuck at file white → white dome.
// №145: back_cloud looked "correct" white because its stage is TEXC-only and
// the day-band kumo is white; that did not prove C0 was live.
static void wwSkyPaintMatC0(J3DMaterial* mat, s16 r, s16 g, s16 b, s16 a) {
    if (mat == NULL) {
        return;
    }
    mat->setCullMode(0);
    mat->change();
    J3DGXColorS10 color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    mat->setTevColor(0, &color);
}

static void wwSkyPaintMatK0(J3DMaterial* mat, u8 r, u8 g, u8 b, u8 a) {
    if (mat == NULL) {
        return;
    }
    mat->setCullMode(0);
    mat->change();
    J3DGXColor color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    mat->setTevKColor(0, &color);
}

// §127 — Room44 model1 authored tev registers (Bridge model1_mizu_colors.csv).
// Never inject pane seacolor (dif/amb): those are lerp endpoints for drawWave,
// not solid model1 colors (§126 white / §127 too-dark). Re-apply the BDL bake
// each draw so MAJI/other paths cannot leave a wrong value latched.
struct WwModel1AuthRegs {
    s16 c[4][4];  // C0..C3 RGBA (s10; donor may use a>255)
    u8 k[4][4];   // K0..K3 RGBA
};

static const WwModel1AuthRegs kWwModel1AuthRegs[8] = {
    // 0 SC_01_mizu — opaque KONST base
    {{{255, 255, 255, 255}, {70, 90, 150, 255}, {255, 255, 255, 255}, {0, 0, 0, 0}},
     {{70, 90, 150, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}}},
    // 1 SC_01_mizuB_v_x
    {{{255, 255, 255, 255}, {255, 255, 255, 279}, {255, 255, 255, 255}, {0, 0, 0, 0}},
     {{255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}}},
    // 2 SC_01_mizu_v
    {{{255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {0, 0, 0, 0}},
     {{255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}}},
    // 3 SC_01_mizu_v_x
    {{{255, 255, 255, 255}, {255, 255, 255, 279}, {200, 200, 255, 255}, {0, 0, 0, 0}},
     {{255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}}},
    // 4 SC_01_mizu_v(2)
    {{{255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {0, 0, 0, 0}},
     {{255, 255, 255, 255}, {100, 180, 180, 255}, {80, 100, 150, 255}, {255, 255, 255, 255}}},
    // 5 SC_01_mizu_v_x(2)
    {{{255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {0, 0, 0, 0}},
     {{255, 255, 255, 255}, {255, 255, 255, 50}, {255, 255, 255, 255}, {255, 255, 255, 255}}},
    // 6 SC_01_mizu_v(3)
    {{{255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {0, 0, 0, 0}},
     {{70, 90, 150, 255}, {100, 200, 200, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}}},
    // 7 SC_01_mizu_v(4)
    {{{0, 0, 0, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}, {0, 0, 0, 0}},
     {{255, 255, 255, 255}, {0, 0, 0, 50}, {255, 255, 255, 255}, {255, 255, 255, 255}}},
};

static void wwApplyModel1SeaPalette(J3DModelData* data) {
    if (data == NULL) {
        return;
    }
    const u16 n = data->getMaterialNum();
    const u16 lim = n < 8 ? n : 8;
    for (u16 i = 0; i < lim; i++) {
        J3DMaterial* mat = data->getMaterialNodePointer(i);
        if (mat == NULL) {
            continue;
        }
        mat->change();
        const WwModel1AuthRegs& auth = kWwModel1AuthRegs[i];
        for (int r = 0; r < 4; r++) {
            J3DGXColorS10 c;
            c.r = auth.c[r][0];
            c.g = auth.c[r][1];
            c.b = auth.c[r][2];
            c.a = auth.c[r][3];
            mat->setTevColor((u32)r, &c);
            J3DGXColor k;
            k.r = auth.k[r][0];
            k.g = auth.k[r][1];
            k.b = auth.k[r][2];
            k.a = auth.k[r][3];
            mat->setTevKColor((u32)r, &k);
        }
        // §128 composite sheet: mats 1–7 XLU z-update OFF; mat0 opaque keeps
        // z-write ON (donor). Do not clear mat0 depth — that is not the swoosh fix.
        J3DBlend* blend = mat->getBlend();
        J3DZMode* zmode = mat->getZMode();
        if (i > 0 && blend != NULL && zmode != NULL && blend->getBlendMode() == GX_BM_BLEND &&
            zmode->getUpdateEnable() != 0) {
            zmode->setUpdateEnable(0);
        }
    }
}

// One-shot fidelity probe vs Bridge mat-ref (VTX matSrc + XLU z-update=0).
static void wwProbeModel1Fidelity(J3DModelData* data) {
    if (data == NULL) {
        return;
    }
    static bool s_probed;
    if (s_probed) {
        return;
    }
    s_probed = true;
    JUTNameTab* names = data->getMaterialName();
    int vtxOk = 0;
    int zOk = 0;
    int n = data->getMaterialNum();
    for (u16 i = 0; i < n; i++) {
        J3DMaterial* mat = data->getMaterialNodePointer(i);
        if (mat == NULL) {
            continue;
        }
        J3DColorChan* chan = mat->getColorChan(0);
        const u8 matSrc = chan != NULL ? chan->getMatSrc() : 0xFF;
        J3DBlend* blend = mat->getBlend();
        J3DZMode* zmode = mat->getZMode();
        const u8 zUp = zmode != NULL ? zmode->getUpdateEnable() : 0xFF;
        const u8 bm = blend != NULL ? (u8)blend->getBlendMode() : 0xFF;
        if (matSrc == 1) {
            vtxOk++;
        }
        if (bm != GX_BM_BLEND || zUp == 0) {
            zOk++;
        }
        const char* name = names != NULL ? names->getName(i) : "?";
        J3DGXColor* k0 = mat->getTevKColor(0);
        DuskLog.info(
            "[WwFoam] §103 model1 mat[{}] '{}' matSrc={} blend={} zUp={} K0=({},{},{},{})",
            (int)i, name != NULL ? name : "?", (int)matSrc, (int)bm, (int)zUp,
            k0 != NULL ? (int)k0->r : -1, k0 != NULL ? (int)k0->g : -1,
            k0 != NULL ? (int)k0->b : -1, k0 != NULL ? (int)k0->a : -1);
    }
    DuskLog.info("[WwFoam] §127 model1 fidelity mats={} vtxMatSrc={} zWriteOk={} "
                 "(authored regs; mat0 K0/C1=(70,90,150) — no pane seacolor)",
                 n, vtxOk, zOk);
}

static void wwSkyApplyModel(J3DModel* model) {
    if (model == NULL) {
        return;
    }
    model->calc();
}

// №133/№144: g_env_light.vrbox_* was wrong (magenta kasumi / black kumo) while
// the stage VRB0 table was correct. Root causes:
//   (1) envcolor_init can latch dm* defaults; UseCol=room44 then indexes OOB —
//       rebind stage tables before draw.
//   (2) №144: converted VRB0 was authored at stride 0x18 but
//       sizeof(stage_vrboxcol_info_class)==0x15 — index 2 read mid-record.
//       Data fixed to 0x15; feed still drives colours from the Env0→Col0→PAL0→VRB0
//       chain (no addcol/ratio scale).
static bool wwSkyRebindStageLighting() {
    bool changed = false;
    auto tryRebind = [&](auto*& slot, auto* fresh, const char* name) {
        if (fresh != NULL && slot != fresh) {
            slot = fresh;
            changed = true;
            DuskLog.info("[WwSky] №133 rebound {} from stage", name);
        }
    };
    tryRebind(g_env_light.stage_envr_info, dComIfGp_getStageEnvrInfo(), "Env0");
    tryRebind(g_env_light.stage_pselect_info, dComIfGp_getStagePselectInfo(), "Col0");
    tryRebind(g_env_light.stage_palette_info, dComIfGp_getStagePaletteInfo(), "PAL0");
    tryRebind(g_env_light.stage_vrboxcol_info, dComIfGp_getStageVrboxcolInfo(), "VRB0");

    // Room-local VRB0 fallback if stage still on the 18-entry dm default.
    if (g_env_light.stage_vrboxcol_info == NULL ||
        g_env_light.stage_vrboxcol_info ==
            (stage_vrboxcol_info_class*)dKyd_dmvrbox_getp()) {
        const s8 stay = dComIfGp_roomControl_getStayNo();
        if (stay >= 0) {
            dStage_roomDt_c* roomDt = dComIfGp_roomControl_getStatusRoomDt(stay);
            if (roomDt != NULL) {
                stage_vrboxcol_info_class* roomVrb = roomDt->getVrboxcolInfo();
                if (roomVrb != NULL && g_env_light.stage_vrboxcol_info != roomVrb) {
                    g_env_light.stage_vrboxcol_info = roomVrb;
                    changed = true;
                    DuskLog.info("[WwSky] №133 rebound VRB0 from room{}", (int)stay);
                }
            }
        }
    }
    return changed;
}

static s16 wwSkyLerpU8(u8 a, u8 b, f32 t) {
    return (s16)(a + (b - a) * t);
}

static void wwSkyFeedEnvFromStageVrbox() {
    wwSkyRebindStageLighting();

    if (g_env_light.stage_envr_info == NULL || g_env_light.stage_pselect_info == NULL ||
        g_env_light.stage_palette_info == NULL || g_env_light.stage_vrboxcol_info == NULL) {
        return;
    }
    // Still on dm defaults → cannot safely index room 44.
    if (g_env_light.stage_envr_info == (stage_envr_info_class*)dKyd_dmenvr_getp() ||
        g_env_light.stage_vrboxcol_info ==
            (stage_vrboxcol_info_class*)dKyd_dmvrbox_getp()) {
        static bool s_warnedDm = false;
        if (!s_warnedDm) {
            s_warnedDm = true;
            DuskLog.warn(
                "[WwSky] №133 still on dm Env0/VRB0 — stage lighting tables not loaded "
                "(check Elst env-layer vs Env0-only inject)");
        }
        return;
    }

    int room = g_env_light.UseCol;
    if (room < 0) {
        room = dComIfGp_roomControl_getStayNo();
    }
    if (room < 0) {
        room = 0;
    }

    stage_envr_info_class* envr = &g_env_light.stage_envr_info[room];
    u8 weather = g_env_light.wether_pat0;
    if (weather > 7) {
        weather = 0;
    }
    u8 pselIdx = envr->pselect_id[weather];
    stage_pselect_info_class* psel = &g_env_light.stage_pselect_info[pselIdx];

    f32 daytime = g_env_light.getDaytime();
    u8 startL = 2;
    u8 endL = 2;
    f32 t = 0.0f;
    dKyd_lightSchejule* sch = dKyd_schejule_getp();
    if (sch != NULL) {
        for (int i = 0; i < 11; ++i) {
            if (daytime >= sch[i].startTime && daytime <= sch[i].endTime) {
                startL = sch[i].startTimeLight;
                endL = sch[i].endTimeLight;
                const f32 span = sch[i].endTime - sch[i].startTime;
                t = (span > 0.0001f) ? (daytime - sch[i].startTime) / span : 0.0f;
                if (t < 0.0f) {
                    t = 0.0f;
                }
                if (t > 1.0f) {
                    t = 1.0f;
                }
                break;
            }
        }
    }
    if (startL > 5) {
        startL = 2;
    }
    if (endL > 5) {
        endL = 2;
    }

    const u8 pal0 = psel->palette_id[startL];
    const u8 pal1 = psel->palette_id[endL];
    const u8 vr0 = g_env_light.stage_palette_info[pal0].vrboxcol_id;
    const u8 vr1 = g_env_light.stage_palette_info[pal1].vrboxcol_id;
    const stage_vrboxcol_info_class& A = g_env_light.stage_vrboxcol_info[vr0];
    const stage_vrboxcol_info_class& B = g_env_light.stage_vrboxcol_info[vr1];

    g_env_light.vrbox_sky_col.r = wwSkyLerpU8(A.sky_col.r, B.sky_col.r, t);
    g_env_light.vrbox_sky_col.g = wwSkyLerpU8(A.sky_col.g, B.sky_col.g, t);
    g_env_light.vrbox_sky_col.b = wwSkyLerpU8(A.sky_col.b, B.sky_col.b, t);
    g_env_light.vrbox_sky_col.a = 255;

    g_env_light.vrbox_kumo_top_col.r = wwSkyLerpU8(A.kumo_top_col.r, B.kumo_top_col.r, t);
    g_env_light.vrbox_kumo_top_col.g = wwSkyLerpU8(A.kumo_top_col.g, B.kumo_top_col.g, t);
    g_env_light.vrbox_kumo_top_col.b = wwSkyLerpU8(A.kumo_top_col.b, B.kumo_top_col.b, t);
    g_env_light.vrbox_kumo_top_col.a =
        wwSkyLerpU8(A.kumo_shadow_col.a, B.kumo_shadow_col.a, t);

    g_env_light.vrbox_kasumi_inner_col.r =
        wwSkyLerpU8(A.kasumi_inner_col.r, B.kasumi_inner_col.r, t);
    g_env_light.vrbox_kasumi_inner_col.g =
        wwSkyLerpU8(A.kasumi_inner_col.g, B.kasumi_inner_col.g, t);
    g_env_light.vrbox_kasumi_inner_col.b =
        wwSkyLerpU8(A.kasumi_inner_col.b, B.kasumi_inner_col.b, t);
    g_env_light.vrbox_kasumi_inner_col.a =
        wwSkyLerpU8(A.kasumi_inner_col.a, B.kasumi_inner_col.a, t);

    g_env_light.vrbox_kasumi_outer_col.r =
        wwSkyLerpU8(A.kasumi_outer_col.r, B.kasumi_outer_col.r, t);
    g_env_light.vrbox_kasumi_outer_col.g =
        wwSkyLerpU8(A.kasumi_outer_col.g, B.kasumi_outer_col.g, t);
    g_env_light.vrbox_kasumi_outer_col.b =
        wwSkyLerpU8(A.kasumi_outer_col.b, B.kasumi_outer_col.b, t);
    g_env_light.vrbox_kasumi_outer_col.a =
        wwSkyLerpU8(A.kasumi_outer_col.a, B.kasumi_outer_col.a, t);

    static bool s_fedOnce = false;
    if (!s_fedOnce) {
        s_fedOnce = true;
        DuskLog.info(
            "[WwSky] №144 feed daytime={:.1f} room={} weather={} pal=({},{}) vr=({},{}) "
            "sky=({},{},{}) kasumiIn=({},{},{}) kumoTop=({},{},{}) sizeof_vrb={}",
            daytime, room, (int)weather, (int)pal0, (int)pal1, (int)vr0, (int)vr1,
            g_env_light.vrbox_sky_col.r, g_env_light.vrbox_sky_col.g,
            g_env_light.vrbox_sky_col.b, g_env_light.vrbox_kasumi_inner_col.r,
            g_env_light.vrbox_kasumi_inner_col.g, g_env_light.vrbox_kasumi_inner_col.b,
            g_env_light.vrbox_kumo_top_col.r, g_env_light.vrbox_kumo_top_col.g,
            g_env_light.vrbox_kumo_top_col.b, (int)sizeof(stage_vrboxcol_info_class));
    }
}

static void wwSkyDraw() {
    if (!s_wwSkyReady || dComIfGd_getView() == NULL) {
        return;
    }
    wwSkyFeedEnvFromStageVrbox();
    f32 yOrigin = 0.0f;
    const s8 stay = dComIfGp_roomControl_getStayNo();
    if (stay >= 0) {
        dStage_roomDt_c* roomDt = dComIfGp_roomControl_getStatusRoomDt(stay);
        if (roomDt != NULL) {
            dStage_FileList_dt_c* fili = roomDt->getFileListInfo();
            if (fili != NULL) {
                yOrigin = dStage_FileList_dt_SeaLevel(fili);
            }
        }
    }
    const f32 yOff = (dComIfGd_getInvViewMtx()[1][3] - yOrigin) * 0.09f;
    const f32 cx = dComIfGd_getInvViewMtx()[0][3];
    const f32 cy = dComIfGd_getInvViewMtx()[1][3] - yOff;
    const f32 cz = dComIfGd_getInvViewMtx()[2][3];
    mDoMtx_stack_c::transS(cx, cy, cz);
    MtxP base = mDoMtx_stack_c::get();

    const GXColorS10& sky = g_env_light.vrbox_sky_col;
    const GXColorS10& kasumi = g_env_light.vrbox_kasumi_inner_col;
    const GXColorS10& kumo = g_env_light.vrbox_kumo_top_col;

    dComIfGd_setListSky();
    // WW order: sky → uso_umi → kasumi → back_cloud (+100 Y).
    // vr_sky.bdl (WW d_a_vrbox): mat0 K=kasumi, mat1 K=sky.
    if (s_wwSkyModels[0] != NULL) {
        s_wwSkyModels[0]->setBaseTRMtx(base);
        wwSkyApplyModel(s_wwSkyModels[0]);
        J3DModelData* data = s_wwSkyModels[0]->getModelData();
        if (data != NULL) {
            wwSkyPaintMatK0(data->getMaterialNodePointer(0), (u8)kasumi.r, (u8)kasumi.g,
                            (u8)kasumi.b, 255);
            wwSkyPaintMatK0(data->getMaterialNodePointer(1), (u8)sky.r, (u8)sky.g, (u8)sky.b,
                            255);
        }
        mDoExt_modelUpdateDL(s_wwSkyModels[0]);
    }
    // vr_uso_umi.bdl: K = uso/sky colour (WW mVrUsoUmiColor; we share sky_col).
    if (s_wwSkyModels[1] != NULL) {
        s_wwSkyModels[1]->setBaseTRMtx(base);
        wwSkyApplyModel(s_wwSkyModels[1]);
        J3DModelData* data = s_wwSkyModels[1]->getModelData();
        if (data != NULL) {
            wwSkyPaintMatK0(data->getMaterialNodePointer(0), (u8)sky.r, (u8)sky.g, (u8)sky.b,
                            255);
        }
        mDoExt_modelUpdateDL(s_wwSkyModels[1]);
    }
    // vr_kasumi_mae.bdl: C0 = kasumi (WW); K.a carries cloud alpha.
    if (s_wwSkyModels[2] != NULL) {
        s_wwSkyModels[2]->setBaseTRMtx(base);
        wwSkyApplyModel(s_wwSkyModels[2]);
        J3DModelData* data = s_wwSkyModels[2]->getModelData();
        if (data != NULL) {
            J3DMaterial* mat = data->getMaterialNodePointer(0);
            wwSkyPaintMatC0(mat, kasumi.r, kasumi.g, kasumi.b, kasumi.a);
            wwSkyPaintMatK0(mat, (u8)kumo.a, 0, 0, 0);
        }
        mDoExt_modelUpdateDL(s_wwSkyModels[2]);
    }
    // vr_back_cloud.bdl: K on every material (WW d_a_vrbox2); TEXC*K path.
    if (s_wwSkyModels[3] != NULL) {
        mDoMtx_stack_c::transS(cx, cy + 100.0f, cz);
        s_wwSkyModels[3]->setBaseTRMtx(mDoMtx_stack_c::get());
        wwSkyApplyModel(s_wwSkyModels[3]);
        J3DModelData* data = s_wwSkyModels[3]->getModelData();
        if (data != NULL) {
            const u16 n = data->getMaterialNum();
            for (u16 i = 0; i < n; ++i) {
                wwSkyPaintMatK0(data->getMaterialNodePointer(i), (u8)kumo.r, (u8)kumo.g,
                                (u8)kumo.b, 255);
            }
        }
        mDoExt_modelUpdateDL(s_wwSkyModels[3]);
    }
    dComIfGd_setList();
}

bool tryBindBtp(dExtNpcMount_c* a, J3DModelData* data) {
    a->mpBtp = NULL;
    a->mBtpBound = false;
    a->mDemoTexAnmLast = 0xFFFFFFFF;  // №186: sentinel — 0 is a valid resource id
    if (a->mManifest.skipBtp || a->mManifest.btp[0] == '\0') {
        stageLog("btp", "skipped (skip_btp or no btp=)");
        return true;
    }
    J3DAnmTexPattern* pat =
        (J3DAnmTexPattern*)dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.btp);
    if (pat == NULL) {
        DuskLog.warn("[ExtNpcMount] btp '{}' missing in arc '{}' — blink deferred",
                     a->mManifest.btp, a->mManifest.arc);
        stageLog("btp", "FAIL res NULL (cosmetic defer)");
        return true;  // non-fatal
    }
    a->mpBtp = JKR_NEW mDoExt_btpAnm();
    if (a->mpBtp == NULL) {
        stageLog("btp", "FAIL alloc");
        return true;
    }
    // Loop blink: attribute EMode_LOOP (2) like npc_ks relief path.
    if (a->mpBtp->init(data, pat, 1, J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1) == 0) {
        DuskLog.warn("[ExtNpcMount] btp '{}' bind failed — blink deferred", a->mManifest.btp);
        stageLog("btp", "FAIL init (cosmetic defer)");
        a->mpBtp = NULL;
        return true;
    }
    a->mBtpBound = true;
    stageLog("btp", "bound ok");
    return true;
}

// №188: IDLE BLINK — WW faces are texture planes; "blink" is a BTP frame swap.
//
// Ported from `daNpc_Ls1_c`: `setBtp` inits the pattern with EMode_NONE (the
// actor drives the frame, not a loop ctrl), and `play_btp_anm` with resID index
// 1 (MABA) holds eyes-open on a random 60-90 frame timer, then advances through
// the close/open once and re-arms. Bound as a SEPARATE `mpBlink` so it never
// fights the expression `mpBtp`. Assets confirmed present (`maba.btp` in Ls.arc).
bool tryBindBlink(dExtNpcMount_c* a, J3DModelData* data) {
    a->mpBlink = NULL;
    a->mBlinkBound = false;
    a->mBlinkTimer = 0;
    a->mBlinkFrame = 0.0f;
    if (a->mManifest.blinkBtp[0] == '\0') {
        return true;
    }
    J3DAnmTexPattern* pat =
        (J3DAnmTexPattern*)dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.blinkBtp);
    if (pat == NULL) {
        DuskLog.warn("[ExtNpcMount] №188 blink '{}' missing in arc '{}' — no blink",
                     a->mManifest.blinkBtp, a->mManifest.arc);
        return true;  // cosmetic, non-fatal
    }
    a->mpBlink = JKR_NEW mDoExt_btpAnm();
    if (a->mpBlink == NULL) {
        return true;
    }
    // EMode_NONE: we own the frame (donor sets attribute 0 in setBtp).
    if (a->mpBlink->init(data, pat, 1, J3DFrameCtrl::EMode_NONE, 1.0f, 0, -1) == 0) {
        DuskLog.warn("[ExtNpcMount] №188 blink '{}' init failed", a->mManifest.blinkBtp);
        a->mpBlink = NULL;
        return true;
    }
    a->mBlinkBound = true;
    a->mBlinkTimer = cLib_getRndValue<s16>(60, 30);  // first hold: 60..90 frames
    DuskLog.info("[ExtNpcMount] №188 blink bound '{}'", a->mManifest.blinkBtp);
    return true;
}

// Advance the blink one frame, donor `play_btp_anm` (index-1 branch) verbatim.
// Called every frame the actor is visible — including inside the demo branch,
// because a WW character blinks THROUGH its cutscene, not only outside it.
void dExtNpcMount_driveBlink(dExtNpcMount_c* a) {
    if (!a->mBlinkBound || a->mpBlink == NULL) {
        return;
    }
    const f32 frameMax = a->mpBlink->getEndFrame();
    // Hold eyes-open (frame 0) while the timer counts down; when it expires,
    // advance one frame per tick through the blink, then re-arm and reset.
    if (cLib_calcTimer(&a->mBlinkTimer) == 0) {
        a->mBlinkFrame += 1.0f;
        if (a->mBlinkFrame >= frameMax) {
            a->mBlinkTimer = cLib_getRndValue<s16>(60, 30);
            a->mBlinkFrame = 0.0f;
        }
    }
    a->mpBlink->setFrame(a->mBlinkFrame);
}

void applyModelAmbient(J3DModel* model, const GXColor& amb) {
    if (model == NULL || model->getModelData() == NULL) {
        return;
    }
    J3DModelData* data = model->getModelData();
    for (u16 i = 0; i < data->getMaterialNum(); ++i) {
        J3DMaterial* mat = data->getMaterialNodePointer(i);
        if (mat != NULL) {
            mat->change();
            mat->setAmbColor(0, (J3DGXColor*)&amb);
        }
    }
}

void logJointNames(J3DModelData* data) {
    JUTNameTab* names = data != NULL ? data->getJointTree().getJointName() : NULL;
    if (names == NULL) {
        DuskLog.warn("[ExtNpcMount] body has no joint-name table");
        return;
    }
    for (u16 i = 0; i < data->getJointNum(); ++i) {
        DuskLog.warn("[ExtNpcMount] body joint[{}]={}", i, names->getName(i));
    }
}

bool addAttachment(dExtNpcMount_c* a, J3DModelData* bodyData, const dExtNpcAttachSpec& spec) {
    if (spec.model[0] == '\0' || a->mAttachCount >= kExtNpcMaxAttach) {
        return true;
    }
    void* raw = dComIfG_getObjectRes(a->mManifest.arc, spec.model);
    J3DModelData* data = acquireMountedModel(a->mManifest.arc, spec.model, raw);
    if (data == NULL) {
        DuskLog.warn("[ExtNpcMount] attachment '{}' missing/unparseable in arc '{}'", spec.model,
                     a->mManifest.arc);
        return true;
    }

    // Prefer companion joint table when present.
    J3DModelData* jointHost = bodyData;
    bool onCompanion = false;
    if (a->mpCompanion != NULL && a->mpCompanion->getModel() != NULL) {
        J3DModelData* companionData = a->mpCompanion->getModel()->getModelData();
        JUTNameTab* cNames = companionData != NULL ? companionData->getJointTree().getJointName() : NULL;
        const char* joint = spec.joint[0] ? spec.joint : "head";
        if (cNames != NULL && cNames->getIndex(joint) >= 0) {
            jointHost = companionData;
            onCompanion = true;
        }
    }

    JUTNameTab* names = jointHost->getJointTree().getJointName();
    const char* joint = spec.joint[0] ? spec.joint : "head";
    const s32 jointIndex = names != NULL ? names->getIndex(joint) : -1;
    if (jointIndex < 0 || jointIndex >= jointHost->getJointNum()) {
        DuskLog.warn("[ExtNpcMount] attachment '{}' skipped: joint '{}' not found", spec.model, joint);
        logJointNames(jointHost);
        if (jointHost != bodyData) {
            logJointNames(bodyData);
        }
        return true;
    }
    J3DModel* model = mDoExt_J3DModel__create(data, 0, 1);
    if (model == NULL) {
        DuskLog.warn("[ExtNpcMount] attachment '{}' model allocation failed", spec.model);
        return true;
    }
    const int slot = a->mAttachCount++;
    a->mpAttach[slot] = model;
    a->mAttachJnt[slot] = (s16)jointIndex;
    // ========================================================================
    // №218 — carry the donor's local prop transform into the slot so both
    // placement paths (slave CB / setMtx) can compose jointMtx × T × R.
    // ========================================================================
    a->mAttachOffs[slot] = spec.offs;
    a->mAttachRot[slot] = spec.rot;
    a->mAttachLocal[slot] = spec.hasLocal ? 1 : 0;
    // №250: the in-demo pose variant (donor m841 split).
    a->mAttachOffsDemo[slot] = spec.offsDemo;
    a->mAttachRotDemo[slot] = spec.rotDemo;
    a->mAttachDemoLocal[slot] = spec.hasDemoLocal ? 1 : 0;
    a->mAttachOnCompanion[slot] = onCompanion ? 1 : 0;
    a->mAttachSlave[slot] = 1;
    // №49/№50: base = host BASE; parent-compose via joint callback during calc (envelope-safe).
    J3DModel* hostModel =
        onCompanion ? a->mpCompanion->getModel() : a->mpMorf->getModel();
    model->setBaseTRMtx(hostModel->getBaseTRMtx());
    installSlaveJointCallbacks(model, a);
    DuskLog.info("[ExtNpcMount] attachment '{}' joint_slave → {} ({}) on {}{}", spec.model, joint,
                 jointIndex, onCompanion ? "companion" : "body",
                 spec.hasLocal ? " +local" : "");
    return true;
}

// №50-E: Nintendo knob = door.bdl controller (BCK + DoorDummy) + visible door_a..h at DoorDummy.
bool addDoorVisual(dExtNpcMount_c* a, J3DModelData* bodyData) {
    if (a == NULL || bodyData == NULL || !a->mManifest.doorAttention ||
        a->mManifest.doorVisual[0] == '\0' || a->mAttachCount >= kExtNpcMaxAttach) {
        return true;
    }
    JUTNameTab* names = bodyData->getJointTree().getJointName();
    const s32 jointIndex = names != NULL ? names->getIndex("DoorDummy") : -1;
    if (jointIndex < 0 || jointIndex >= bodyData->getJointNum()) {
        DuskLog.warn("[ExtNpcMount] door_visual '{}' skipped: DoorDummy joint missing",
                     a->mManifest.doorVisual);
        logJointNames(bodyData);
        return true;
    }
    void* raw = dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.doorVisual);
    J3DModelData* data = acquireMountedModel(a->mManifest.arc, a->mManifest.doorVisual, raw);
    if (data == NULL) {
        DuskLog.warn("[ExtNpcMount] door_visual '{}' missing/unparseable in arc '{}'",
                     a->mManifest.doorVisual, a->mManifest.arc);
        return true;
    }
    J3DModel* model = mDoExt_J3DModel__create(data, 0, 1);
    if (model == NULL) {
        DuskLog.warn("[ExtNpcMount] door_visual '{}' model allocation failed",
                     a->mManifest.doorVisual);
        return true;
    }
    const int slot = a->mAttachCount++;
    a->mpAttach[slot] = model;
    a->mAttachJnt[slot] = (s16)jointIndex;
    a->mAttachOnCompanion[slot] = 0;
    a->mAttachSlave[slot] = 0;  // base@DoorDummy only — no joint CB
    if (a->mpMorf != NULL && a->mpMorf->getModel() != NULL) {
        model->setBaseTRMtx(a->mpMorf->getModel()->getAnmMtx((u16)jointIndex));
    }
    DuskLog.info("[ExtNpcMount] door_visual '{}' @ DoorDummy ({})", a->mManifest.doorVisual,
                 jointIndex);
    return true;
}

int useHeapInit(fopAc_ac_c* i_this) {
    dExtNpcMount_c* a = (dExtNpcMount_c*)i_this;

    // №117: collision-only static prop (Akabe.arc = dzb, no bdl). Invisible wall.
    if (a->mManifest.model[0] == '\0' && a->mManifest.isStatic && a->mManifest.collision[0]) {
        stageLog("heap", "collision-only (no model)");
        void* dzb = dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.collision);
        if (dzb == NULL) {
            DuskLog.warn("[ExtNpcMount] №117 collision-only '{}' missing in '{}'",
                         a->mManifest.collision, a->mManifest.arc);
            return 0;
        }
        a->mpBgW = JKR_NEW dBgW();
        if (a->mpBgW == NULL) {
            return 0;
        }
        MTXIdentity(a->mBgMtx);
        mDoMtx_stack_c::transS(a->current.pos.x, a->current.pos.y, a->current.pos.z);
        mDoMtx_stack_c::YrotM(a->current.angle.y);
        MTXCopy(mDoMtx_stack_c::get(), a->mBgMtx);
        if (a->mpBgW->Set((cBgD_t*)dzb, cBgW::MOVE_BG_e, &a->mBgMtx) == 1) {
            DuskLog.warn("[ExtNpcMount] №117 collision-only Set failed '{}'",
                         a->mManifest.collision);
            return 0;
        }
        a->mpBgW->SetCrrFunc(dBgS_MoveBGProc_Typical);
        a->mBgReady = true;
        a->mpMorf = NULL;
        stageLog("heap", "ok — collision-only");
        return 1;
    }

    stageLog("heap", "getObjectRes model");
    void* raw = dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.model);
    if (raw == NULL) {
        stageLog("heap", "FAIL model res NULL");
        return 0;
    }
    J3DModelData* data = acquireMountedModel(a->mManifest.arc, a->mManifest.model, raw,
                                             a->mManifest.bodyBmt[0] ? a->mManifest.bodyBmt : NULL);
    if (data == NULL) {
        DuskLog.warn("[ExtNpcMount] model '{}' missing/unparseable in arc '{}'", a->mManifest.model,
                     a->mManifest.arc);
        return 0;
    }

    // №47-A: bind idle on every non-static mount (WW short names allowed).
    stageLog("heap", "getObjectRes idle BCK");
    J3DAnmTransform* anm = NULL;
    if (a->mManifest.idle[0] &&
        (a->mManifest.isStatic || a->mManifest.doorAttention ||
         animAllowedOnBody(a->mManifest.model, a->mManifest.idle))) {
        anm = (J3DAnmTransform*)dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.idle);
        if (anm == NULL) {
            DuskLog.warn("[ExtNpcMount] idle '{}' missing — static pose", a->mManifest.idle);
        }
    } else if (a->mManifest.idle[0]) {
        DuskLog.warn("[ExtNpcMount] N3 refuse idle '{}' for model '{}' — static pose",
                     a->mManifest.idle, a->mManifest.model);
    }

    J3DMaterial* mat0 = data->getMaterialNodePointer(0);
    if (mat0 == NULL) {
        stageLog("heap", "FAIL pre-McaMorf material[0]=NULL");
        return 0;
    }

    stageLog("heap", "McaMorf ctor");
    a->mpMorf = JKR_NEW mDoExt_McaMorf(data, NULL, NULL, anm, J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1,
                                       1, NULL, 0x80000, 0x11000084);
    if (a->mpMorf == NULL || a->mpMorf->getModel() == NULL) {
        stageLog("heap", "FAIL McaMorf");
        a->mpMorf = NULL;
        return 0;
    }

    a->mpCompanion = NULL;
    if (a->mManifest.companionModel[0]) {
        void* cRaw = dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.companionModel);
        J3DModelData* cData =
            acquireMountedModel(a->mManifest.arc, a->mManifest.companionModel, cRaw);
        // №36 A / №49: joint_slave (or slave_map) — no hand BCK. Synced mode still wants idle.
        J3DAnmTransform* cAnm = NULL;
        const bool jointSlave = a->mManifest.companionMode == 1 || a->mSlaveMap[0] != '\0';
        if (jointSlave) {
            a->mManifest.companionMode = 1;
        }
        if (!jointSlave) {
            const char* cIdle = a->mManifest.companionIdle[0] ? a->mManifest.companionIdle :
                                                               a->mManifest.idle;
            if (cIdle[0]) {
                cAnm = (J3DAnmTransform*)dComIfG_getObjectRes(a->mManifest.arc, cIdle);
            }
        }
        if (cData != NULL && (jointSlave || cAnm != NULL)) {
            a->mpCompanion =
                JKR_NEW mDoExt_McaMorf(cData, NULL, NULL, cAnm, J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1,
                                       1, NULL, 0x80000, 0x11000084);
            if (a->mpCompanion == NULL || a->mpCompanion->getModel() == NULL) {
                DuskLog.warn("[ExtNpcMount] companion '{}' McaMorf failed — continuing without",
                             a->mManifest.companionModel);
                a->mpCompanion = NULL;
            } else {
                if (jointSlave) {
                    installSlaveJointCallbacks(a->mpCompanion->getModel(), a);
                    resolveSlaveMapPairs(a);
                }
                DuskLog.info("[ExtNpcMount] companion '{}' mode={} pairs={}",
                             a->mManifest.companionModel, jointSlave ? "joint_slave" : "synced",
                             (int)a->mSlavePairCount);
            }
        } else {
            DuskLog.warn("[ExtNpcMount] companion '{}' missing (or idle for synced) — skip",
                         a->mManifest.companionModel);
        }
    }

    // №36 C: optional brk/btk color (Vlupy rupees).
    a->mpBrk = NULL;
    a->mpColorBtk = NULL;
    if (a->mpMorf != NULL && a->mpMorf->getModel() != NULL) {
        J3DModelData* bodyData = a->mpMorf->getModel()->getModelData();
        if (a->mManifest.brk[0] && bodyData != NULL) {
            J3DAnmTevRegKey* brk =
                (J3DAnmTevRegKey*)dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.brk);
            if (brk != NULL) {
                a->mpBrk = JKR_NEW mDoExt_brkAnm();
                if (a->mpBrk != NULL &&
                    a->mpBrk->init(bodyData, brk, 1, J3DFrameCtrl::EMode_NONE, 1.0f, 0, -1)) {
                    f32 frame = a->mManifest.colorFrame;
                    if (frame < 0.0f) {
                        frame = vlupyColorFrame(a->mPickupItemNo);
                    }
                    a->mpBrk->setFrame(frame);
                } else {
                    a->mpBrk = NULL;
                }
            }
        }
        if (a->mManifest.btk[0] && bodyData != NULL) {
            J3DAnmTextureSRTKey* btk =
                (J3DAnmTextureSRTKey*)dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.btk);
            if (btk != NULL) {
                a->mpColorBtk = JKR_NEW mDoExt_btkAnm();
                if (a->mpColorBtk != NULL &&
                    a->mpColorBtk->init(bodyData, btk, 1, J3DFrameCtrl::EMode_NONE, 1.0f, 0, -1)) {
                    f32 frame = a->mManifest.colorFrame;
                    if (frame < 0.0f) {
                        frame = vlupyColorFrame(a->mPickupItemNo);
                    }
                    a->mpColorBtk->setFrame(frame);
                } else {
                    a->mpColorBtk = NULL;
                }
            }
        }
    }

    tryBindBtp(a, data);
    tryBindBlink(a, data);  // №188
    a->mAttachCount = 0;
    for (int i = 0; i < kExtNpcMaxAttach; ++i) {
        a->mpAttach[i] = NULL;
        a->mAttachJnt[i] = -1;
        a->mAttachOnCompanion[i] = 0;
        a->mAttachSlave[i] = 0;
        a->mAttachLocal[i] = 0;  // №218: identity unless the manifest authors one
        addAttachment(a, data, a->mManifest.attach[i]);
    }
    addDoorVisual(a, data);
    stageLog("heap", "ok — model bound");
    return 1;
}

void updateBgTransform(dExtNpcMount_c* a);

int useBgHeapInit(fopAc_ac_c* i_this) {
    dExtNpcMount_c* a = (dExtNpcMount_c*)i_this;
    // Housing §33 ruling (b): probe optional BG slots like daBg_c — use what
    // exists, skip what doesn't. Manifest model/model2/model3 are overrides;
    // empty keys fall back to model.bdl / model1.bdl / model3.bdl (donor table
    // also has model2; we only have three mount slots). Missing members are
    // WARN+continue, never FAIL — LinkUG/Pjavdou legitimately omit model1.
    static const char* const kProbeDefault[3] = {"model.bdl", "model1.bdl", "model3.bdl"};
    char slotName[3][64] = {};
    const char* override[3] = {a->mManifest.model, a->mManifest.model2, a->mManifest.model3};
    const bool anyOverride =
        override[0][0] != '\0' || override[1][0] != '\0' || override[2][0] != '\0';
    for (int i = 0; i < 3; ++i) {
        if (override[i][0] != '\0') {
            std::snprintf(slotName[i], sizeof(slotName[i]), "%s", override[i]);
        } else if (!anyOverride) {
            std::snprintf(slotName[i], sizeof(slotName[i]), "%s", kProbeDefault[i]);
        }
    }
    int loaded = 0;
    for (int i = 0; i < 3; ++i) {
        a->mpBgModels[i] = NULL;
        if (slotName[i][0] == '\0') {
            continue;
        }
        void* raw = dComIfG_getObjectRes(a->mManifest.arc, slotName[i]);
        J3DModelData* data = acquireBgModel(a->mManifest.arc, slotName[i], raw);
        if (data == NULL) {
            DuskLog.warn("[ExtNpcMount] BG model '{}' absent in '{}' — skip (probe)", slotName[i],
                         a->mManifest.arc);
            continue;
        }
        // Base differed flags match Obj_Fmobj / room BG. Slot 1 hosts model1.btk
        // (§128 shore crash): daBg ORs 0x1200 (TexGenNum=2 | TexGen) so
        // calcMaterial SRT reaches the DifferedDL. Without it, play()/entry()
        // advance frames but UVs stay frozen — "playback verified, no swoosh."
        u32 diffFlags = 0x11000084;
        if (i == 1) {
            diffFlags |= 0x1200;
        }
        a->mpBgModels[i] = mDoExt_J3DModel__create(data, 0x80000, diffFlags);
        if (a->mpBgModels[i] == NULL) {
            DuskLog.warn("[ExtNpcMount] BG model '{}' J3DModel__create failed — skip",
                         slotName[i]);
            continue;
        }
        ++loaded;
        DuskLog.info("[ExtNpcMount] BG model[{}] '{}' mats={} joints={} shapes={} diff={:#x}",
                     i, slotName[i], data->getMaterialNum(), data->getJointNum(),
                     data->getShapeNum(), diffFlags);
    }
    if (loaded == 0) {
        DuskLog.warn("[ExtNpcMount] BG '{}' — no models resolved", a->mManifest.arc);
        return 0;
    }
    a->mpBgBtk = NULL;
    if (a->mpBgModels[1] != NULL) {
        const char* btkName = a->mManifest.model2Btk[0] ? a->mManifest.model2Btk : "model1.btk";
        J3DAnmTextureSRTKey* btk =
            (J3DAnmTextureSRTKey*)dComIfG_getObjectRes(a->mManifest.arc, btkName);
        if (btk == NULL && a->mManifest.model2Btk[0] == '\0') {
            btk = (J3DAnmTextureSRTKey*)dComIfG_getObjectRes(a->mManifest.arc, "model.btk");
            btkName = "model.btk";
        }
        if (btk != NULL) {
            a->mpBgBtk = JKR_NEW mDoExt_btkAnm();
            if (a->mpBgBtk == NULL ||
                !a->mpBgBtk->init(a->mpBgModels[1]->getModelData(), btk, TRUE,
                                   J3DFrameCtrl::EMode_LOOP, 1.0f, 0, -1)) {
                DuskLog.warn("[ExtNpcMount] BG btk '{}' unavailable — skip", btkName);
                a->mpBgBtk = NULL;
            } else {
                DuskLog.info("[WwFoam] §128 model1 btk '{}' bound bindings={} "
                             "(DifferedDL TexGen 0x1200 — UV SRT live)",
                             btkName, (int)btk->getUpdateMaterialNum());
            }
        }
        wwProbeModel1Fidelity(a->mpBgModels[1]->getModelData());
    }
    // brakeeff / Obj_Fmobj pattern: Set against IDENTITY, then copy the real
    // host−anchor mtx and Move() AFTER Regist. GlobalVtx-at-Set with the final
    // translate left the dzb stranded at Great Sea (№20 hologram).
    MTXIdentity(a->mBgMtx);
    void* dzb = dComIfG_getObjectRes(a->mManifest.arc, a->mManifest.collision);
    if (dzb == NULL) {
        DuskLog.warn("[ExtNpcMount] BG collision '{}' missing in arc '{}'", a->mManifest.collision,
                     a->mManifest.arc);
        return 0;
    }
    a->mpBgW = JKR_NEW dBgW();
    if (a->mpBgW == NULL) {
        DuskLog.warn("[ExtNpcMount] BG dBgW alloc failed for '{}'", a->mManifest.collision);
        return 0;
    }
    // №98/№107: identity dzb transform (host − anchor == 0) = world collision
    // (GLOBAL_e), matching TP d_a_bg. MOVE_BG_e makes the space a "moving object"
    // ⇒ climb/ledge break. Outset identity is host=anchor=cell (not 0,0,0).
    a->mBgGlobal = false;
    {
        const char* stage = dComIfGp_getStartStageName();
        const bool onWwHost = stage != NULL && dExtWwSave_isWwHostStage(stage);
        const bool identity =
            a->mManifest.hasHostPos && a->mManifest.hasAnchor &&
            cM3d_IsZero(a->mManifest.hostPos.x - a->mManifest.anchor.x) &&
            cM3d_IsZero(a->mManifest.hostPos.y - a->mManifest.anchor.y) &&
            cM3d_IsZero(a->mManifest.hostPos.z - a->mManifest.anchor.z);
        if (onWwHost && identity) {
            if (a->mpBgW->Set((cBgD_t*)dzb, cBgW::GLOBAL_e, NULL) == 1) {
                DuskLog.warn("[ExtNpcMount] BG dBgW::Set GLOBAL failed for '{}'",
                             a->mManifest.collision);
                return 0;
            }
            a->mBgGlobal = true;
            // №256 / daBg parity: TP room collision is GLOBAL_e + PRIORITY_0
            // (d_a_bg.cpp). Default ClrDBgWBase leaves priority=2 — WallCorrect
            // still visits it, but stage-room attribute/SFX/ledge paths expect
            // world BGs at PRIORITY_0. Join that path; do not relax alink gates.
            // №257: setBgW + stub Release lands at Regist (create) — not here.
            a->mpBgW->SetPriority(dBgW_Base::PRIORITY_0);
            DuskLog.info(
                "[ExtNpcMount] №256 BG GLOBAL_e PRIORITY_0 (identity) proc='{}' arc='{}'",
                a->mManifest.proc, a->mManifest.arc);
            return 1;
        }
    }
    if (a->mpBgW->Set((cBgD_t*)dzb, cBgW::MOVE_BG_e, &a->mBgMtx) == 1) {
        DuskLog.warn("[ExtNpcMount] BG dBgW::Set failed for '{}'", a->mManifest.collision);
        return 0;
    }
    a->mpBgW->SetCrrFunc(dBgS_MoveBGProc_Typical);
    return 1;
}

void updateBgTransform(dExtNpcMount_c* a) {
    // DZB (world-baked): always host − anchor.
    mDoMtx_stack_c::transS(a->current.pos.x - a->mManifest.anchor.x,
                           a->current.pos.y - a->mManifest.anchor.y,
                           a->current.pos.z - a->mManifest.anchor.z);
    MTXCopy(mDoMtx_stack_c::get(), a->mBgMtx);

    // Models: №22 — WW sea-room meshes are cell-LOCAL; one shared mtx cannot align
    // them with a world-baked dzb. Local ⇒ translate(host) only.
    if (a->mManifest.modelSpaceLocal) {
        mDoMtx_stack_c::transS(a->current.pos.x, a->current.pos.y, a->current.pos.z);
    }
    MtxP modelBase = mDoMtx_stack_c::get();
    for (int i = 0; i < 3; ++i) {
        if (a->mpBgModels[i] != NULL) {
            a->mpBgModels[i]->setBaseTRMtx(modelBase);
            a->mpBgModels[i]->calc();  // without this, meshes stay at world origin
        }
    }
}

// №45/№48: static head/companion pins from actor_map, keyed by "PROC:arg".
struct HeadRegistryEntry {
    char headModel[64]{};
    char headJoint[32]{};
    char headFromParams[32]{};  // prefix → resolve at create from params low byte
    char slaveMap[160]{};       // №48 companion_slave_map
    char companionModel[64]{};  // №49: e.g. bmarm.bdl alongside head_model
    char bodyBmt[64]{};         // №50-C: e.g. ko02.bmt
};
std::unordered_map<std::string, HeadRegistryEntry> s_headRegistry;

void loadHeadRegistryForMod(const fs::path& modRoot) {
    const fs::path mapPath = modRoot / "population" / "actor_map.ini";
    std::ifstream in(mapPath);
    if (!in) {
        return;
    }
    std::string line;
    std::string section;
    HeadRegistryEntry cur{};
    char proc[32] = {};
    int arg = -1;
    bool haveSection = false;
    auto commit = [&]() {
        if (!haveSection || proc[0] == '\0') {
            return;
        }
        if (cur.headModel[0] == '\0' && cur.headFromParams[0] == '\0' && cur.slaveMap[0] == '\0' &&
            cur.companionModel[0] == '\0' && cur.bodyBmt[0] == '\0') {
            return;
        }
        const int useArg = arg >= 0 ? arg : 0;
        const std::string key = std::string(proc) + ":" + std::to_string(useArg);
        auto existing = s_headRegistry.find(key);
        if (existing != s_headRegistry.end()) {
            // Merge slave_map / companion / body_bmt into a prior head row (same proc:arg).
            if (existing->second.slaveMap[0] == '\0' && cur.slaveMap[0]) {
                snprintf(existing->second.slaveMap, sizeof(existing->second.slaveMap), "%s",
                         cur.slaveMap);
            }
            if (existing->second.companionModel[0] == '\0' && cur.companionModel[0]) {
                snprintf(existing->second.companionModel, sizeof(existing->second.companionModel),
                         "%s", cur.companionModel);
            }
            if (existing->second.bodyBmt[0] == '\0' && cur.bodyBmt[0]) {
                snprintf(existing->second.bodyBmt, sizeof(existing->second.bodyBmt), "%s",
                         cur.bodyBmt);
            }
            return;
        }
        if (cur.headJoint[0] == '\0' && (cur.headModel[0] || cur.headFromParams[0])) {
            snprintf(cur.headJoint, sizeof(cur.headJoint), "%s", "head");
        }
        s_headRegistry[key] = cur;
        DuskLog.debug(
            "[ExtNpcMount] head registry {} → {} / companion={} / slave_map={}", key,
            cur.headModel[0] ? cur.headModel : "-",
            cur.companionModel[0] ? cur.companionModel : "-",
            cur.slaveMap[0] ? cur.slaveMap : "-");
    };
    while (std::getline(in, line)) {
        while (!line.empty() &&
               (line.back() == '\r' || line.back() == '\n' || line.back() == ' ')) {
            line.pop_back();
        }
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        if (line[0] == '[') {
            commit();
            const size_t end = line.find(']');
            section = end == std::string::npos ? "" : line.substr(1, end - 1);
            cur = HeadRegistryEntry{};
            proc[0] = '\0';
            arg = -1;
            haveSection = section != "layers";
            continue;
        }
        if (!haveSection) {
            continue;
        }
        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        while (!key.empty() && key.back() == ' ') {
            key.pop_back();
        }
        while (!val.empty() && val[0] == ' ') {
            val.erase(0, 1);
        }
        for (char& c : key) {
            if (c >= 'A' && c <= 'Z') {
                c = static_cast<char>(c - 'A' + 'a');
            }
        }
        if (key == "proc") {
            snprintf(proc, sizeof(proc), "%s", val.c_str());
        } else if (key == "arg") {
            arg = std::atoi(val.c_str());
        } else if (key == "head_model") {
            snprintf(cur.headModel, sizeof(cur.headModel), "%s", val.c_str());
        } else if (key == "head_joint") {
            snprintf(cur.headJoint, sizeof(cur.headJoint), "%s", val.c_str());
        } else if (key == "head_from_params") {
            snprintf(cur.headFromParams, sizeof(cur.headFromParams), "%s", val.c_str());
        } else if (key == "companion_slave_map") {
            snprintf(cur.slaveMap, sizeof(cur.slaveMap), "%s", val.c_str());
        } else if (key == "companion") {
            snprintf(cur.companionModel, sizeof(cur.companionModel), "%s", val.c_str());
        } else if (key == "body_bmt") {
            snprintf(cur.bodyBmt, sizeof(cur.bodyBmt), "%s", val.c_str());
        }
    }
    commit();
}

bool pullHeadFromRegistry(const char* procName, int registryArg, u32 wwParams, char* headOut,
                          u32 headBytes, char* jointOut, u32 jointBytes) {
    if (procName == NULL || procName[0] == '\0' || headOut == NULL || headBytes == 0) {
        return false;
    }
    const std::string key = std::string(procName) + ":" + std::to_string(registryArg);
    auto it = s_headRegistry.find(key);
    if (it == s_headRegistry.end()) {
        return false;
    }
    const HeadRegistryEntry& e = it->second;
    if (e.headFromParams[0]) {
        int idx = static_cast<int>(wwParams & 0xFF);
        if (idx <= 0 || idx > 99 || idx == 0xFF) {
            idx = 1;
        }
        snprintf(headOut, headBytes, "%s%02d.bdl", e.headFromParams, idx);
    } else if (e.headModel[0]) {
        snprintf(headOut, headBytes, "%s", e.headModel);
    } else {
        return false;
    }
    if (jointOut != NULL && jointBytes > 0) {
        snprintf(jointOut, jointBytes, "%s", e.headJoint[0] ? e.headJoint : "head");
    }
    return true;
}

bool pullSlaveMapFromRegistry(const char* procName, int registryArg, char* mapOut, u32 mapBytes) {
    if (procName == NULL || procName[0] == '\0' || mapOut == NULL || mapBytes == 0) {
        return false;
    }
    mapOut[0] = '\0';
    const std::string key = std::string(procName) + ":" + std::to_string(registryArg);
    auto it = s_headRegistry.find(key);
    if (it == s_headRegistry.end() || it->second.slaveMap[0] == '\0') {
        return false;
    }
    snprintf(mapOut, mapBytes, "%s", it->second.slaveMap);
    return true;
}

bool pullCompanionFromRegistry(const char* procName, int registryArg, char* companionOut,
                               u32 companionBytes) {
    if (procName == NULL || procName[0] == '\0' || companionOut == NULL || companionBytes == 0) {
        return false;
    }
    companionOut[0] = '\0';
    const std::string key = std::string(procName) + ":" + std::to_string(registryArg);
    auto it = s_headRegistry.find(key);
    if (it == s_headRegistry.end() || it->second.companionModel[0] == '\0') {
        return false;
    }
    snprintf(companionOut, companionBytes, "%s", it->second.companionModel);
    return true;
}

bool pullBodyBmtFromRegistry(const char* procName, int registryArg, char* bmtOut, u32 bmtBytes) {
    if (procName == NULL || procName[0] == '\0' || bmtOut == NULL || bmtBytes == 0) {
        return false;
    }
    bmtOut[0] = '\0';
    const std::string key = std::string(procName) + ":" + std::to_string(registryArg);
    auto it = s_headRegistry.find(key);
    if (it == s_headRegistry.end() || it->second.bodyBmt[0] == '\0') {
        return false;
    }
    snprintf(bmtOut, bmtBytes, "%s", it->second.bodyBmt);
    return true;
}

}  // namespace

J3DModelData* dExtNpcMount_acquireModelData(const char* arc, const char* modelName) {
    if (arc == NULL || arc[0] == '\0' || modelName == NULL || modelName[0] == '\0') {
        return NULL;
    }
    void* res = dComIfG_getObjectRes(arc, modelName);
    if (res == NULL) {
        return NULL;
    }
    return acquireMountedModel(arc, modelName, res);
}

void dExtNpcMount_retainArc(const char* arc) {
    retainArcModels(arc);
}

void dExtNpcMount_releaseArc(const char* arc) {
    releaseArcModels(arc, "knob00-release");
}

void dExtNpcMount_rescanProviders() {
    s_providers.clear();
    s_providerOrder.clear();
    s_headRegistry.clear();
    // №73: do NOT treat s_modelDataCache as immortal. Room-lane mounts resDelete their
    // arcs; cache entries are purged on last releaseArcModels (see retain/release pair).
    // rescan leaves the cache alone — live mounts still own their arcs.
    std::error_code ec;
    const fs::path userRoot = dusk::ConfigPath / "model_replacements";
    if (!fs::is_directory(userRoot, ec)) {
        return;
    }
    // A2: socket+arg first-claim wins (directory walk order; top mod list is separate).
    std::unordered_map<std::string, std::string> claimedSocketArg;  // "SOCKET:arg" → proc
    for (auto it = fs::directory_iterator(userRoot, ec); it != fs::directory_iterator();
         it.increment(ec)) {
        if (ec || !it->is_directory(ec)) {
            continue;
        }
        const fs::path modRoot = it->path();
        const std::string modName = modRoot.filename().string();
        if (shouldSkipModFolder(modName)) {
            continue;
        }
        if (!dusk::custom_assets::is_folder_enabled(modName.c_str())) {
            continue;
        }
        const fs::path npcDir = modRoot / "npc";
        if (!fs::is_directory(npcDir, ec)) {
            continue;
        }
        for (auto nit = fs::directory_iterator(npcDir, ec); nit != fs::directory_iterator();
             nit.increment(ec)) {
            if (ec || !nit->is_regular_file(ec)) {
                continue;
            }
            if (nit->path().extension() != ".ini") {
                continue;
            }
            dExtNpcManifest man{};
            if (!parseManifestFile(nit->path(), modName.c_str(), &man)) {
                continue;
            }
            // №122: a manifest may legitimately name NO arc. Ported systems
            // (the vegetation lane) carry their geometry in an extracted asset
            // pack and create their own profile rather than mounting a model,
            // so there is nothing to look for under arcs/. Only enforce arc
            // presence when the manifest actually declares one.
            if (man.arc[0] != '\0' && !man.fromDvd && !arcFilePresent(modRoot, man.arc)) {
                DuskLog.debug(
                    "[ExtNpcMount] '{}' manifest ok but arc missing under {} — socket idle",
                    man.proc, (modRoot / "arcs" / (std::string(man.arc) + ".arc")).string());
                continue;
            }
            const char* sock = man.socket[0] ? man.socket : man.proc;
            const int sarg = man.socketArg >= 0 ? man.socketArg : 0;
            const std::string claimKey = std::string(sock) + ":" + std::to_string(sarg);
            auto claimed = claimedSocketArg.find(claimKey);
            if (claimed != claimedSocketArg.end() && claimed->second != man.proc) {
                DuskLog.warn(
                    "[ExtNpcMount] socket claim lost: {} arg={} kept proc={} rejected '{}' (mod {})",
                    sock, sarg, claimed->second, man.proc, modName);
                continue;
            }
            claimedSocketArg[claimKey] = man.proc;
            s_providers[man.proc] = man;
            s_providerOrder.push_back(man.proc);
            DuskLog.info("[ExtNpcMount] provider {} ← mod '{}' arc={} skip_btp={} btp={} scale={}",
                         man.proc, modName, man.arc, man.skipBtp ? 1 : 0,
                         man.btp[0] ? man.btp : "-", man.scale);
        }

        // №27 N4: overlay user-locked identities from population/identity.ini.
        const fs::path idPath = modRoot / "population" / "identity.ini";
        std::ifstream idIn(idPath);
        if (idIn) {
            std::string line;
            std::string section;
            while (std::getline(idIn, line)) {
                while (!line.empty() &&
                       (line.back() == '\r' || line.back() == '\n' || line.back() == ' ')) {
                    line.pop_back();
                }
                if (line.empty() || line[0] == '#' || line[0] == ';') {
                    continue;
                }
                if (line[0] == '[') {
                    const size_t end = line.find(']');
                    section = end == std::string::npos ? "" : line.substr(1, end - 1);
                    continue;
                }
                const size_t eq = line.find('=');
                if (eq == std::string::npos || section.empty()) {
                    continue;
                }
                std::string key = line.substr(0, eq);
                std::string val = line.substr(eq + 1);
                while (!key.empty() && key.back() == ' ') {
                    key.pop_back();
                }
                while (!val.empty() && val[0] == ' ') {
                    val.erase(0, 1);
                }
                if (key != "display_name" && key != "proc") {
                    continue;
                }
                // Section may be census name (Ls1) or proc (NPC_LS).
                std::string procKey = section;
                if (key == "proc") {
                    procKey = val;
                    continue;
                }
                // Prefer matching by proc; also try census→proc via common NPC_ prefix guess.
                auto pit = s_providers.find(procKey);
                if (pit == s_providers.end()) {
                    // Try NPC_<SECTION> uppercased stem.
                    std::string guess = "NPC_";
                    for (char c : section) {
                        if (c >= 'a' && c <= 'z') {
                            guess += (char)(c - 'a' + 'A');
                        } else if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
                            guess += c;
                        }
                    }
                    // Strip trailing digits for Ls1 → NPC_LS
                    while (!guess.empty() && guess.back() >= '0' && guess.back() <= '9') {
                        guess.pop_back();
                    }
                    pit = s_providers.find(guess);
                }
                if (pit != s_providers.end() && !val.empty()) {
                    snprintf(pit->second.displayName, sizeof(pit->second.displayName), "%s",
                             val.c_str());
                    DuskLog.info("[ExtNpcMount] N4 identity lock {} → '{}'", pit->first, val);
                }
            }
        }

        // №45: actor_map → (proc,arg) head pull registry (no transient handoff).
        loadHeadRegistryForMod(modRoot);
    }
    DuskLog.info("[ExtNpcMount] head registry entries={}", s_headRegistry.size());
}

bool dExtNpcMount_hasPayload(const char* procName) {
    return procName != NULL && s_providers.find(procName) != s_providers.end();
}

bool dExtNpcMount_lookup(const char* procName, dExtNpcManifest* out) {
    auto it = s_providers.find(procName != NULL ? procName : "");
    if (it == s_providers.end() || out == NULL) {
        return false;
    }
    *out = it->second;
    return true;
}

int dExtNpcMount_providerCount() {
    return (int)s_providerOrder.size();
}

bool dExtNpcMount_providerAt(int index, dExtNpcManifest* out) {
    if (out == NULL || index < 0 || index >= (int)s_providerOrder.size()) {
        return false;
    }
    return dExtNpcMount_lookup(s_providerOrder[index].c_str(), out);
}

s16 dExtNpcMount_socketActorId(const char* socketName) {
    if (socketName == NULL) {
        return -1;
    }
    if (strcmp(socketName, "NPC_MK") == 0) {
        return fpcNm_NPC_MK_e;
    }
    if (strcmp(socketName, "NPC_P2") == 0) {
        return fpcNm_NPC_P2_e;
    }
    if (strcmp(socketName, "NPC_HENNA0") == 0) {
        return fpcNm_NPC_HENNA0_e;
    }
    if (strcmp(socketName, "NPC_KDK") == 0) {
        return fpcNm_NPC_KDK_e;
    }
    if (strcmp(socketName, "EXT_BG10") == 0) {
        return fpcNm_EXT_BG10_e;
    }
    if (strcmp(socketName, "EXT_VEG") == 0) {
        return fpcNm_EXT_VEG_e;
    }
    if (strcmp(socketName, "EXT_SPAN") == 0) {
        return fpcNm_EXT_SPAN_e;
    }
    return -1;
}

static const char* providerSocket(const dExtNpcManifest& man) {
    return man.socket[0] ? man.socket : man.proc;
}

bool dExtNpcMount_hasSocketPayload(const char* socketName) {
    if (socketName == NULL || socketName[0] == '\0') {
        return false;
    }
    for (const auto& kv : s_providers) {
        if (strcmp(providerSocket(kv.second), socketName) == 0) {
            return true;
        }
    }
    return false;
}

// №38 E1 RESOLVER LAW: exact socket + exact arg, or REFUSE. No wildcard / first-match /
// default payload — a miss is a hole + log, never an imposter (Ganondorf = HENNA0 arg 1).
bool dExtNpcMount_resolveSocket(const char* socketName, int arg, char* procOut, u32 procOutBytes) {
    if (socketName == NULL || procOut == NULL || procOutBytes == 0) {
        return false;
    }
    procOut[0] = '\0';

    for (const auto& kv : s_providers) {
        const dExtNpcManifest& man = kv.second;
        if (strcmp(providerSocket(man), socketName) != 0) {
            continue;
        }
        if (man.socketArg == arg) {
            snprintf(procOut, procOutBytes, "%s", man.proc);
            return true;
        }
    }
    for (const auto& kv : s_providers) {
        const dExtNpcManifest& man = kv.second;
        if (strcmp(providerSocket(man), socketName) != 0) {
            continue;
        }
        for (int i = 0; i < man.subtypeCount; ++i) {
            if (man.subtypes[i].valid && man.subtypes[i].arg == arg) {
                snprintf(procOut, procOutBytes, "%s", man.proc);
                return true;
            }
        }
    }
    DuskLog.warn("[ExtNpcMount] E1 REFUSED socket='{}' arg={} — no exact payload (hole, not fallback)",
                 socketName, arg);
    return false;
}

static char s_forcedCreateProc[32] = {};
static bool s_forcedCreateProcSet = false;
static char s_forcedAttachModel[64] = {};
static char s_forcedAttachJoint[32] = {};
static bool s_forcedAttachSet = false;
static char s_forcedSpawnSrc[96] = {};
static bool s_forcedSpawnSrcSet = false;

struct PendingSpawn {
    char proc[32]{};
    char src[96]{};
    char headModel[64]{};
    char headJoint[32]{};
    // №130: identity token. The spawner reclaims its own entry by this value
    // after create, so an actor that forgets to consume cannot leave a stale
    // entry for the NEXT actor to pull. Order-based reaping was not safe —
    // popping the front can discard somebody else's entry.
    u32 seq{};
};
static u32 s_pendingSeq = 0;
static std::deque<u32> s_passWatermark;
static std::unordered_map<u32, PendingSpawn> s_pendingById;
static std::deque<PendingSpawn> s_pendingFifo;

static void fillPending(PendingSpawn* p, const char* procName, const char* src,
                        const char* headModel, const char* headJoint) {
    *p = PendingSpawn{};
    if (procName != NULL && procName[0]) {
        snprintf(p->proc, sizeof(p->proc), "%s", procName);
    }
    if (src != NULL && src[0]) {
        snprintf(p->src, sizeof(p->src), "%s", src);
    }
    if (headModel != NULL && headModel[0]) {
        snprintf(p->headModel, sizeof(p->headModel), "%s", headModel);
        snprintf(p->headJoint, sizeof(p->headJoint), "%s",
                 headJoint != NULL && headJoint[0] ? headJoint : "head");
    }
}

u32 dExtNpcMount_pushPendingSpawn(const char* procName, const char* src, const char* headModel,
                                  const char* headJoint) {
    PendingSpawn p{};
    fillPending(&p, procName, src, headModel, headJoint);
    p.seq = ++s_pendingSeq;
    s_pendingFifo.push_back(p);
    return p.seq;
}

// №130 STRUCTURAL GUARD. Every census spawn pushes an entry that the created
// actor is supposed to consume in its create. Historically that was a rule each
// actor author had to remember, and forgetting it was silent and catastrophic:
// the stale entry is pulled by the NEXT actor, so every later actor wears an
// earlier row's head. It has bitten twice (№64 orphan-entry, №129 ported
// actors) and the blast radius scales with cast size — one leak on a
// fully-populated Great Sea would shift hundreds of actors.
//
// So the queue is no longer trust-based. The spawner reclaims its own entry by
// token after create; anything unconsumed is dropped THERE, before it can be
// mis-served. Actors that consume correctly are unaffected (their entry is
// already gone). Actors that forget now degrade to "no head pinned" instead of
// corrupting the whole cast — a local, visible defect instead of a global,
// invisible one.
// №131 OVERRIDE. If the №130 guard itself turns out to be wrong, it must be
// switchable WITHOUT a rebuild — an escape hatch that needs a compile is not an
// escape hatch. Read mod-side: population/engine_overrides.ini, key
// `pending_spawn_guard` (default 1 = guard on; 0 = vanilla FIFO order only).
static int s_pendingGuard = -1;  // -1 = not yet read
bool dExtNpcMount_pendingGuardEnabled() {
    if (s_pendingGuard >= 0) {
        return s_pendingGuard != 0;
    }
    s_pendingGuard = 1;
    const fs::path root = fs::path(dusk::ConfigPath) / "model_replacements";
    std::error_code ec;
    if (fs::is_directory(root, ec)) {
        for (const auto& entry : fs::directory_iterator(root, ec)) {
            if (!entry.is_directory()) {
                continue;
            }
            std::ifstream in(entry.path() / "population" / "engine_overrides.ini");
            if (!in) {
                continue;
            }
            std::string line;
            while (std::getline(in, line)) {
                if (line.empty() || line[0] == '#' || line[0] == ';') {
                    continue;
                }
                const size_t eq = line.find('=');
                if (eq == std::string::npos) {
                    continue;
                }
                std::string key = line.substr(0, eq);
                std::string val = line.substr(eq + 1);
                while (!key.empty() && (key.back() == ' ' || key.back() == 0x0D)) key.pop_back();
                while (!val.empty() && (val.back() == ' ' || val.back() == 0x0D)) val.pop_back();
                if (key == "pending_spawn_guard" && (val == "0" || val == "off" || val == "false")) {
                    s_pendingGuard = 0;
                }
            }
        }
    }
    if (s_pendingGuard == 0) {
        DuskLog.warn(
            "[ExtNpcMount] №131 OVERRIDE ACTIVE — pending-spawn guard DISABLED via "
            "population/engine_overrides.ini. Head/identity binding falls back to vanilla "
            "FIFO ORDER, which is order-dependent: any actor that does not consume its entry "
            "will shift every actor created after it.");
        DuskLog.warn(
            "[ExtNpcMount] №131 BEFORE RELYING ON THIS BUILD: (1) snapshot the current engine "
            "+ mod-folder state so it can be restored, (2) re-verify actor identities AND "
            "placements on every affected space and story layer — a mis-bind is silent and "
            "keeps correct dialogue, so it will NOT announce itself.");
    }
    return s_pendingGuard != 0;
}

// №131: entries are keyed by actor id, and process ids are RECYCLED. An entry
// whose actor died without consuming would otherwise sit here until some later
// actor is handed the same id and pulls a dead row's head — the same corruption
// by a slower route, and story/layer changes (vanilla re-spawns the cast per
// dComIfG_play_c::getLayerNo) make repeats routine. Sweep before every pass.
void dExtNpcMount_sweepPendingById() {
    // №133: this used to ask vanilla whether each actor was still alive
    // (fopAcM_SearchByID / fpcM_IsCreating). That was the wrong instinct: it put
    // our bookkeeping inside vanilla's actor lifecycle, where it kept colliding
    // with meanings that are vanilla's to define — "not found" also means "still
    // creating", culling is a status bit and not a death, room unload is a real
    // death, and story layers re-spawn the whole cast. Every one of those had to
    // be re-taught to the guard, and each re-teaching was a chance to get
    // vanilla's semantics wrong.
    //
    // The guard has no business knowing any of that. It only needs to bound its
    // OWN side table, so it does that on its OWN clock: an entry that has
    // survived two full population passes was never going to be claimed. No
    // vanilla state is inspected, so vanilla's lifecycle — cull, drop, recall,
    // layer change, save/restore — proceeds exactly as it always did and simply
    // cannot be fought by this code.
    if (s_passWatermark.size() < 2) {
        return;
    }
    const u32 cutoff = s_passWatermark.front();
    for (auto it = s_pendingById.begin(); it != s_pendingById.end();) {
        it = (it->second.seq != 0 && it->second.seq < cutoff) ? s_pendingById.erase(it)
                                                             : std::next(it);
    }
}

// Ring of the last two pass-start token values. Purely our own clock.
void dExtNpcMount_markPendingPass() {
    s_passWatermark.push_back(s_pendingSeq);
    while (s_passWatermark.size() > 2) {
        s_passWatermark.pop_front();
    }
}

bool dExtNpcMount_reapPendingSpawn(u32 seq, fpc_ProcID id) {
    if (!dExtNpcMount_pendingGuardEnabled()) {
        return false;  // №131: vanilla FIFO order, by explicit override
    }
    if (seq == 0) {
        return false;
    }
    for (auto it = s_pendingFifo.begin(); it != s_pendingFifo.end(); ++it) {
        if (it->seq != seq) {
            continue;
        }
        // Still queued, so this actor's create has NOT consumed it yet.
        //
        // It matters enormously which way we resolve that. fopAcM_create may run
        // create synchronously OR defer it (that is why the push happens first —
        // see №45). Simply erasing here would be correct for a sync actor that
        // forgot, and CATASTROPHIC for a phase-based actor that was going to
        // consume on a later frame: we would delete the entry out from under it.
        //
        // So don't erase — RE-KEY. Move it out of the order-sensitive queue and
        // into the id map, where the owning actor can still claim it by its own
        // id whenever its create actually runs. Order-dependence, which is the
        // root of this entire bug class, disappears: a late or forgetful actor
        // can no longer be served somebody else's entry, and can no longer
        // cause somebody else to be served its own.
        PendingSpawn p = *it;
        char procName[32];
        snprintf(procName, sizeof(procName), "%s", p.proc);
        s_pendingFifo.erase(it);
        if (id != fpcM_ERROR_PROCESS_ID_e) {
            s_pendingById[static_cast<u32>(id)] = p;
        }
        return true;
    }
    return false;
}

void dExtNpcMount_bindPendingSpawn(fpc_ProcID id, const char* procName, const char* src,
                                   const char* headModel, const char* headJoint) {
    if (id == fpcM_ERROR_PROCESS_ID_e) {
        return;
    }
    fillPending(&s_pendingById[static_cast<u32>(id)], procName, src, headModel, headJoint);
    // №64: createBgMountAtHost push+bind left an orphan FIFO entry; the next HENNA0
    // create (population) then stole it (EXT_BG1 at the door / Ba1←Lamp). Drain one
    // matching FIFO slot now that the id-map owns the bind.
    if (procName != NULL && procName[0] != '\0') {
        for (auto it = s_pendingFifo.begin(); it != s_pendingFifo.end(); ++it) {
            if (std::strcmp(it->proc, procName) == 0) {
                s_pendingFifo.erase(it);
                break;
            }
        }
    }
}

bool dExtNpcMount_takePendingSpawn(fpc_ProcID id, char* procOut, u32 procBytes, char* srcOut,
                                   u32 srcBytes, char* headOut, u32 headBytes, char* jointOut,
                                   u32 jointBytes) {
    PendingSpawn p{};
    bool have = false;
    if (id != fpcM_ERROR_PROCESS_ID_e) {
        auto it = s_pendingById.find(static_cast<u32>(id));
        if (it != s_pendingById.end()) {
            p = it->second;
            s_pendingById.erase(it);
            have = true;
        }
    }
    if (!have && !s_pendingFifo.empty()) {
        p = s_pendingFifo.front();
        s_pendingFifo.pop_front();
        have = true;
    }
    if (!have) {
        return false;
    }
    if (procOut != NULL && procBytes > 0) {
        snprintf(procOut, procBytes, "%s", p.proc);
    }
    if (srcOut != NULL && srcBytes > 0) {
        snprintf(srcOut, srcBytes, "%s", p.src);
    }
    if (headOut != NULL && headBytes > 0) {
        snprintf(headOut, headBytes, "%s", p.headModel);
    }
    if (jointOut != NULL && jointBytes > 0) {
        snprintf(jointOut, jointBytes, "%s", p.headJoint);
    }
    return true;
}

void dExtNpcMount_forceNextSpawnSrc(const char* src) {
    if (src == NULL || src[0] == '\0') {
        s_forcedSpawnSrcSet = false;
        s_forcedSpawnSrc[0] = '\0';
        return;
    }
    snprintf(s_forcedSpawnSrc, sizeof(s_forcedSpawnSrc), "%s", src);
    s_forcedSpawnSrcSet = true;
}

void dExtNpcMount_forceNextCreateProc(const char* procName) {
    if (procName == NULL || procName[0] == '\0') {
        s_forcedCreateProcSet = false;
        s_forcedCreateProc[0] = '\0';
        return;
    }
    snprintf(s_forcedCreateProc, sizeof(s_forcedCreateProc), "%s", procName);
    s_forcedCreateProcSet = true;
}

bool dExtNpcMount_consumeForcedCreateProc(char* procOut, u32 procOutBytes) {
    if (!s_forcedCreateProcSet || procOut == NULL || procOutBytes == 0) {
        return false;
    }
    snprintf(procOut, procOutBytes, "%s", s_forcedCreateProc);
    s_forcedCreateProcSet = false;
    s_forcedCreateProc[0] = '\0';
    return true;
}

void dExtNpcMount_forceNextAttach(const char* modelName, const char* jointName) {
    if (modelName == NULL || modelName[0] == '\0') {
        s_forcedAttachSet = false;
        s_forcedAttachModel[0] = '\0';
        s_forcedAttachJoint[0] = '\0';
        return;
    }
    snprintf(s_forcedAttachModel, sizeof(s_forcedAttachModel), "%s", modelName);
    snprintf(s_forcedAttachJoint, sizeof(s_forcedAttachJoint), "%s",
             jointName != NULL && jointName[0] ? jointName : "head");
    s_forcedAttachSet = true;
}

bool dExtNpcMount_consumeForcedAttach(char* modelOut, u32 modelBytes, char* jointOut,
                                      u32 jointBytes) {
    if (!s_forcedAttachSet || modelOut == NULL || modelBytes == 0) {
        return false;
    }
    snprintf(modelOut, modelBytes, "%s", s_forcedAttachModel);
    if (jointOut != NULL && jointBytes > 0) {
        snprintf(jointOut, jointBytes, "%s", s_forcedAttachJoint);
    }
    s_forcedAttachSet = false;
    s_forcedAttachModel[0] = '\0';
    s_forcedAttachJoint[0] = '\0';
    return true;
}

bool dExtNpcMount_shouldSkipBtp(const char* arcName) {
    // Per-manifest, not a create-time global (№18a — interleaved creates stomped s_skipBtpArc).
    if (arcName == NULL || arcName[0] == '\0') {
        return false;
    }
    for (const auto& kv : s_providers) {
        if (kv.second.skipBtp && strcmp(kv.second.arc, arcName) == 0) {
            return true;
        }
    }
    return false;
}

// Manifest-driven BG warp latch (Phase M). Host stage/pos/spawn from ini.
enum {
    kBgWarpIdle = 0,
    kBgWarpWaitPlayer = 1,
    kBgWarpWaitIsland = 2,
    kBgWarpWaitBg = 3,
    kBgWarpHold = 4,
};
static int s_bgWarpPhase = kBgWarpIdle;
static fpc_ProcID s_bgIslandId = fpcM_ERROR_PROCESS_ID_e;
static cXyz s_bgHoldPos;
static cXyz s_bgHostPos;
static int s_bgHoldFrames = 0;
static bool s_bgFadeWatchdogFired = false;
static int s_bgSettleFrames = 0;
// №34 P4: frames waiting for BG ready; abort → fail-safe outdoor spawn.
static int s_bgReadyWaitFrames = 0;
static bool s_bgFailSafeValid = false;
static cXyz s_bgFailSafeSpawn;
// №58-B: abort only on true failure. Soft 180f mash-timeout retired while resLoad lives.
static constexpr int kBgReadyHardTimeout = 900;  // ~15s — genuine stuck
static constexpr int kBgReadyGoneTimeout = 45;   // actor vanished after create

// №83: last createBgMountAtHost failure reason (for room0 create FAILED lines).
static char s_bgCreateFailReason[96] = {};

// №265: identity GLOBAL mounts must stamp the HOST room (Outset exterior = 44),
// never the keep-slot / actor room 0. Default BgW m_roomId=0xFF falls through to
// GetGrpRoomId; SetRoomId(0) makes Link's underfoot room 0 and every room-keyed
// event pack (awake, doors) resolves -1.
static int resolveIdentityBgHostRoom(const dExtNpcMount_c* a) {
    if (a == NULL) {
        return -1;
    }
    const int lane = dExtNpcMount_roomLaneHostRoom(a->mManifest.proc);
    if (lane > 0 && lane < 0x40) {
        return lane;
    }
    if (a->mManifest.hostRoom > 0 && a->mManifest.hostRoom < 0x40) {
        return a->mManifest.hostRoom;
    }
    const int actorRoom = fopAcM_GetRoomNo(a);
    if (actorRoom > 0 && actorRoom < 0x40) {
        return actorRoom;
    }
    return -1;
}

static fpc_ProcID createBgMountAtHost(const dExtNpcManifest& man, const char* src,
                                      int forceRoomNo = -1) {
    s_bgCreateFailReason[0] = '\0';
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (!man.isBg) {
        std::snprintf(s_bgCreateFailReason, sizeof(s_bgCreateFailReason), "not_bg");
        return fpcM_ERROR_PROCESS_ID_e;
    }
    if (!man.hasHostPos) {
        std::snprintf(s_bgCreateFailReason, sizeof(s_bgCreateFailReason), "no_host_pos");
        return fpcM_ERROR_PROCESS_ID_e;
    }
    // Door/warm paths need the player for roomNo. Room-lane (forceRoomNo>=0) must
    // NOT — on stage enter the room can be ready before Link exists.
    if (forceRoomNo < 0 && player == NULL) {
        std::snprintf(s_bgCreateFailReason, sizeof(s_bgCreateFailReason), "no_player");
        return fpcM_ERROR_PROCESS_ID_e;
    }
    const char* socket = providerSocket(man);
    const s16 actorId = dExtNpcMount_socketActorId(socket);
    if (actorId < 0) {
        std::snprintf(s_bgCreateFailReason, sizeof(s_bgCreateFailReason), "bad_socket='%s'",
                      socket != NULL ? socket : "?");
        return fpcM_ERROR_PROCESS_ID_e;
    }
    const char* spawnSrc = (src != NULL && src[0]) ? src : "bg";
    csXyz angle;
    angle.set(0, 0, 0);
    cXyz scale(1.0f, 1.0f, 1.0f);
    const int roomNo = forceRoomNo >= 0 ? forceRoomNo : fopAcM_GetRoomNo(player);
    layer_class* savedLayer = fpcLy_CurrentLayer();
    base_process_class* playScene = fpcM_SearchByName(fpcNm_PLAY_SCENE_e);
    if (playScene != NULL) {
        fpcLy_SetCurrentLayer(&((process_node_class*)playScene)->layer);
    } else if (forceRoomNo >= 0) {
        // №85: room-ready runs under the new play scene's layer during create;
        // SearchByName can miss mid-phase. Stay on current layer (do not defer).
    }
    const u32 params = man.socketArg >= 0 ? (u32)man.socketArg : 0;
    // №163: `parameters` and `argument` are DIFFERENT fields, and until now only
    // the first was carried. `parameters` selects the manifest; `argument` is the
    // actor's CENSUS IDENTITY — `fopAcM_findObjectCB` matches proc AND argument,
    // and `dStage_getName2(prof, argument)` names it. Passing a hardcoded -1 left
    // every mounted actor anonymous: `dStage_searchName("Ls1")` yields (HENNA0, 5)
    // from its OBJNAME row, the live actor held -1, `5 == -1` failed, and the
    // storyboard reported no performer for an actor standing in plain sight.
    //
    // Arg is NOT interchangeable with -1 here: every islander shares the HENNA0
    // proc (Ls1=5, Ob1=7, Ko1=8), so -1 would match whichever one happened to be
    // found first — the identity-swap class of bug from №126/№129, re-entered
    // through a different door. The socket arg is what keeps them distinct.
    const s8 argument = man.socketArg >= 0 ? (s8)man.socketArg : (s8)-1;
    const u32 pendingSeq = dExtNpcMount_pushPendingSpawn(man.proc, spawnSrc, NULL, NULL);
    const fpc_ProcID id =
        fopAcM_create(actorId, params, &man.hostPos, roomNo, &angle, &scale, argument);
    fpcLy_SetCurrentLayer(savedLayer);
    dExtNpcMount_reapPendingSpawn(pendingSeq, id);  // №130
    if (id == fpcM_ERROR_PROCESS_ID_e) {
        std::snprintf(s_bgCreateFailReason, sizeof(s_bgCreateFailReason),
                      "fopAcM_create ERROR actorId=%d room=%d layer=-1 params=%08x", (int)actorId,
                      roomNo, params);
        return fpcM_ERROR_PROCESS_ID_e;
    }
    dExtNpcMount_bindPendingSpawn(id, man.proc, spawnSrc, NULL, NULL);
    return id;
}

void dExtNpcMount_registerRoomLane(const char* procName, int hostRoom) {
    if (procName == NULL || procName[0] == '\0' || hostRoom < 0 || hostRoom >= 0x40) {
        return;
    }
    s_roomLaneRooms[procName] = hostRoom;
    s_roomLaneProcByRoom[hostRoom] = procName;
    DuskLog.info("[ExtNpcMount] №62 room-lane register '{}' → room {}", procName, hostRoom);
}

bool dExtNpcMount_isRoomLaneProc(const char* procName) {
    return procName != NULL && s_roomLaneRooms.count(procName) != 0;
}

int dExtNpcMount_roomLaneHostRoom(const char* procName) {
    if (procName == NULL) {
        return -1;
    }
    auto it = s_roomLaneRooms.find(procName);
    return it != s_roomLaneRooms.end() ? it->second : -1;
}

bool dExtNpcMount_isRoomLaneRoom(int roomNo) {
    return roomNo >= 0 && roomNo < 0x40 && s_roomLaneProcByRoom.count(roomNo) != 0;
}

bool dExtNpcMount_isRoomLaneProtected(int roomNo) {
    return roomNo > 0 && roomNo < 0x40 && s_roomLaneClaimed[roomNo];
}

bool dExtNpcMount_isRoomLaneUnloading(int roomNo) {
    return roomNo > 0 && roomNo < 0x40 && s_roomLaneUnloading[roomNo];
}

static bool roomLaneMountIsUnloading(const dExtNpcMount_c* m) {
    if (m == NULL) {
        return false;
    }
    if (s_roomLaneUnloadingProc[0] != '\0' && m->mManifest.proc[0] != '\0' &&
        std::strcmp(m->mManifest.proc, s_roomLaneUnloadingProc) == 0) {
        return true;
    }
    return dExtNpcMount_isRoomLaneUnloading(fopAcM_GetRoomNo(m));
}

// №68/№69: soft-forget door/pop handles + latch. Does NOT Release collision yet —
// Link must leave the interior floor first (№69).
static void forgetRoomLaneSoftHandles(const char* procName, int hostRoom) {
    if (procName == NULL || procName[0] == '\0') {
        return;
    }
    if (hostRoom > 0 && hostRoom < 0x40) {
        s_roomLaneUnloading[hostRoom] = true;
    }
    std::snprintf(s_roomLaneUnloadingProc, sizeof(s_roomLaneUnloadingProc), "%s", procName);
    DuskLog.info("[ExtNpcMount] №69 forget soft handles '{}' room{}", procName, hostRoom);

    fopAcIt_Executor(
        [](void* actor, void* data) -> int {
            const char* proc = (const char*)data;
            fopAc_ac_c* ac = (fopAc_ac_c*)actor;
            if (ac == NULL) {
                return 0;
            }
            const s16 name = fopAcM_GetName(ac);
            if (name != fpcNm_NPC_HENNA0_e && name != fpcNm_NPC_MK_e && name != fpcNm_NPC_P2_e &&
                name != fpcNm_NPC_KDK_e) {
                return 0;
            }
            dExtNpcMount_c* m = (dExtNpcMount_c*)ac;
            if (!m->mManifest.doorAttention) {
                return 0;
            }
            const int r = fopAcM_GetRoomNo(m);
            const bool keyed =
                m->mDoorKey[0] &&
                (std::strstr(m->mDoorKey, "exit:") != NULL || std::strcmp(m->mDoorKey, proc) == 0);
            if (dExtNpcMount_isRoomLaneUnloading(r) || keyed) {
                m->attention_info.flags = 0;
                m->mManifest.doorAttention = false;
            }
            return 0;
        },
        (void*)procName);

    dExtNpcDoors_clearExitKnobForProc(procName);
    dExtNpcPopulation_clearForBg(procName);
}

// №69: Release interior dBgW only AFTER Link is off it, then delete the mount.
static void releaseRoomLaneMount(const char* procName) {
    if (procName == NULL || procName[0] == '\0') {
        return;
    }
    auto mid = s_bgMountIds.find(procName);
    if (mid == s_bgMountIds.end()) {
        return;
    }
    if (s_bgIslandId == mid->second) {
        s_bgIslandId = fpcM_ERROR_PROCESS_ID_e;
    }
    fopAc_ac_c* existing = fopAcM_SearchByID(mid->second);
    if (existing != NULL) {
        dExtNpcMount_c* mount = (dExtNpcMount_c*)existing;
        // Explicit Release before delete — never while the player stands on it.
        if (mount->mpBgW != NULL) {
            if (mount->mBgGlobal) {
                int roomNo = mount->mpBgW->GetRoomId();
                if (roomNo <= 0 || roomNo >= 0x40 || roomNo == 0xFF) {
                    roomNo = resolveIdentityBgHostRoom(mount);
                }
                if (roomNo > 0 && roomNo < 0x40 &&
                    dStage_roomControl_c::getBgW(roomNo) == mount->mpBgW) {
                    dStage_roomControl_c::setBgW(roomNo, NULL);
                }
            }
            dComIfG_Bgsp().Release(mount->mpBgW);
            mount->mpBgW = NULL;
            DuskLog.info("[ExtNpcMount] №69 Released interior BgW '{}'", procName);
        }
        fopAcM_delete(existing);
    }
    s_bgMountIds.erase(mid);
    DuskLog.info("[ExtNpcMount] №69 forgot mount '{}'", procName);
}

static void forceLinkGroundReprobe(fopAc_ac_c* player) {
    if (player == NULL) {
        return;
    }
    daAlink_c* link = (daAlink_c*)player;
    // №269: room-lane can place Link while door-open left FLAG_WALL_NONE set
    // (procDoorOpenInit) before procDoorOpen cleared it. WALL_NONE skips
    // WallCorrect entirely → ChkWallHit never true → ladders/ledges dead
    // (interior + progressive exterior). Restore the donor clear pair.
    const u32 before = link->mLinkAcch.GetFlags();
    link->mLinkAcch.ClrWallNone();
    link->mLinkAcch.OffLineCheckNone();
    link->mLinkAcch.ClrGroundHit();
    link->mLinkAcch.CrrPos(dComIfG_Bgsp());
    if ((before & dBgS_Acch::FLAG_WALL_NONE) != 0) {
        DuskLog.info("[ExtNpcMount] №269 ClrWallNone on place (acchFlags {:#x} → {:#x})",
                     (unsigned)before, (unsigned)link->mLinkAcch.GetFlags());
    }
}

static void clearRoomLaneUnloading(int hostRoom) {
    if (hostRoom > 0 && hostRoom < 0x40) {
        s_roomLaneUnloading[hostRoom] = false;
    }
    s_roomLaneUnloadingProc[0] = '\0';
}

static void activateWwHostRoom(int roomNo, const char* reason);

bool dExtNpcMount_ensureRoomLaneLoaded(int hostRoom) {
    if (hostRoom < 0 || hostRoom >= 0x40) {
        return false;
    }
    // №81: LinkRM on R_DL01 room 0 — native stage load owns room 0; no keep0+claim.
    if (hostRoom == 0) {
        return dComIfGp_roomControl_checkStatusFlag(0, 0x01) ||
               dComIfGp_roomControl_checkStatusFlag(0, 0x02);
    }
    s_roomLaneClaimed[hostRoom] = true;
    dStage_roomControl_c::setNextStayNo(hostRoom);
    // Mid-delete (0x04) — wait; another loadRoom would early-out and leave a void.
    if (dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x04)) {
        return false;
    }
    // Already live or still creating — do NOT re-call loadRoom (recreate loop → crash).
    if (dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x01)) {
        activateWwHostRoom(hostRoom, "ensure-loaded");
        return true;
    }
    if (dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x02)) {
        return true;
    }
    // Keep exterior (room 0) + claim the interior slot. BG bit (0x80) required for create.
    u8 rooms[2];
    rooms[0] = 0x80;
    rooms[1] = static_cast<u8>(0x80 | (hostRoom & 0x3f));
    const int rt = dComIfGp_roomControl_loadRoom(2, rooms, true);
    DuskLog.info("[ExtNpcMount] №62 loadRoom keep0+claim{} rt={}", hostRoom, rt);
    return rt != 0 || dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x01) ||
           dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x02);
}

bool dExtNpcMount_ensureRoomLaneUnloaded(int hostRoom) {
    if (hostRoom <= 0 || hostRoom >= 0x40) {
        return false;
    }
    // Drop claim first so loadRoom may schedule the kill (protect would skip it).
    s_roomLaneClaimed[hostRoom] = false;
    u8 rooms[1];
    rooms[0] = 0x80;  // exterior only
    dStage_roomControl_c::setNextStayNo(0);
    dStage_roomControl_c::setStayNo(0);
    // Fully gone (no live / creating / deleting).
    if (!dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x01) &&
        !dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x02) &&
        !dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x04)) {
        return true;
    }
    // Already deleting — just wait.
    if (dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x04) ||
        dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x08)) {
        return false;
    }
    const int rt = dComIfGp_roomControl_loadRoom(1, rooms, true);
    DuskLog.info("[ExtNpcMount] №62 unload room{} keep0 rt={}", hostRoom, rt);
    // rt==0 with kill flags set is NORMAL (schedule-then-wait). Done only when gone.
    return !dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x01) &&
           !dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x02) &&
           !dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x04);
}

// №83: defer room-lane mount until createBgMountAtHost can succeed (play scene /
// layer ready). Cleared on success or room unload.
static char s_roomMountPendingProc[32] = {};
static int s_roomMountPendingRoom = -1;
static int s_roomMountPendingTries = 0;

static void clearRoomMountPending() {
    s_roomMountPendingProc[0] = '\0';
    s_roomMountPendingRoom = -1;
    s_roomMountPendingTries = 0;
}

static bool tryCreateRoomLaneMount(const char* procName, int roomNo, const char* stageName) {
    if (procName == NULL || procName[0] == '\0' || roomNo < 0 || roomNo >= 0x40) {
        return false;
    }
    dExtNpcManifest man{};
    if (!dExtNpcMount_lookup(procName, &man) || !man.isBg || !man.hasHostPos) {
        DuskLog.warn("[ExtNpcMount] №83 room{} skip '{}' — lookup/bg/hostPos fail stage='{}'",
                     roomNo, procName, stageName != NULL ? stageName : "?");
        return false;
    }
    if (man.hostStage[0] != '\0' && stageName != NULL &&
        std::strcmp(stageName, man.hostStage) != 0) {
        return false;
    }
    auto mid = s_bgMountIds.find(procName);
    if (mid != s_bgMountIds.end()) {
        if (fopAcM_SearchByID(mid->second) != NULL) {
            clearRoomMountPending();
            return true;
        }
        s_bgMountIds.erase(mid);
    }
    if (s_roomLaneMountCreating[roomNo]) {
        return false;
    }
    s_roomLaneMountCreating[roomNo] = true;
    const s16 actorId = dExtNpcMount_socketActorId(providerSocket(man));
    const fpc_ProcID id = createBgMountAtHost(man, "room", roomNo);
    s_roomLaneMountCreating[roomNo] = false;
    if (id == fpcM_ERROR_PROCESS_ID_e) {
        DuskLog.warn(
            "[ExtNpcMount] №83 room{} create FAILED '{}' reason={} actorId={} roomNo={} "
            "host=({},{},{}) stage='{}' playScene={}",
            roomNo, procName, s_bgCreateFailReason[0] ? s_bgCreateFailReason : "?", (int)actorId,
            roomNo, man.hostPos.x, man.hostPos.y, man.hostPos.z, stageName != NULL ? stageName : "?",
            fpcM_SearchByName(fpcNm_PLAY_SCENE_e) != NULL ? 1 : 0);
        return false;
    }
    s_bgMountIds[procName] = id;
    clearRoomMountPending();
    DuskLog.info("[ExtNpcMount] №83 room{} mount '{}' id={:08x} stage='{}'", roomNo, procName,
                 (u32)id, stageName != NULL ? stageName : "?");
    return true;
}

// №83: when arriving on a WW host stage, re-bind room-lane procs whose manifests
// declare this host (doors.ini boot register may have been for the prior stage).
static void syncRoomLaneForCurrentStage() {
    const char* stage = dComIfGp_getStartStageName();
    if (stage == NULL || stage[0] == '\0') {
        return;
    }
    const int n = dExtNpcMount_providerCount();
    for (int i = 0; i < n; ++i) {
        dExtNpcManifest man{};
        if (!dExtNpcMount_providerAt(i, &man) || !man.isBg) {
            continue;
        }
        if (man.hostStage[0] == '\0' || std::strcmp(man.hostStage, stage) != 0) {
            continue;
        }
        if (man.hostRoom < 0 || man.hostRoom >= 0x40) {
            continue;
        }
        // Only re-register rooms already claimed as room-lane (doors.ini) OR
        // host_room explicitly set on a WW host stage (R_DL*).
        if (dExtNpcMount_isRoomLaneProc(man.proc) || dExtWwSave_isWwHostStage(stage)) {
            dExtNpcMount_registerRoomLane(man.proc, man.hostRoom);
        }
    }
}

void dExtNpcMount_onRoomObjectsReady(const char* stageName, int roomNo) {
    if (stageName == NULL || roomNo < 0 || roomNo >= 0x40) {
        return;
    }
    // №83: WW host stages re-assert lane bindings as rooms come up.
    if (dExtWwSave_isWwHostStage(stageName)) {
        syncRoomLaneForCurrentStage();
    }
    auto it = s_roomLaneProcByRoom.find(roomNo);
    if (it == s_roomLaneProcByRoom.end()) {
        return;
    }
    dExtNpcManifest man{};
    if (!dExtNpcMount_lookup(it->second.c_str(), &man) || !man.isBg || !man.hasHostPos) {
        return;
    }
    if (man.hostStage[0] != '\0' && std::strcmp(stageName, man.hostStage) != 0) {
        return;
    }
    if (tryCreateRoomLaneMount(it->second.c_str(), roomNo, stageName)) {
        return;
    }
    // Defer — pollBgWarps retries until play scene / create succeeds.
    std::snprintf(s_roomMountPendingProc, sizeof(s_roomMountPendingProc), "%s", it->second.c_str());
    s_roomMountPendingRoom = roomNo;
    s_roomMountPendingTries = 0;
    DuskLog.info("[ExtNpcMount] №83 room{} mount '{}' PENDING stage='{}' reason={}", roomNo,
                 s_roomMountPendingProc, stageName, s_bgCreateFailReason[0] ? s_bgCreateFailReason : "?");
}

static void pollPendingRoomLaneMount() {
    if (s_roomMountPendingProc[0] == '\0' || s_roomMountPendingRoom < 0) {
        return;
    }
    const char* stage = dComIfGp_getStartStageName();
    if (stage == NULL) {
        return;
    }
    if (!dComIfGp_roomControl_checkStatusFlag(s_roomMountPendingRoom, 0x01) &&
        !dComIfGp_roomControl_checkStatusFlag(s_roomMountPendingRoom, 0x02)) {
        return;
    }
    ++s_roomMountPendingTries;
    if (tryCreateRoomLaneMount(s_roomMountPendingProc, s_roomMountPendingRoom, stage)) {
        return;
    }
    if (s_roomMountPendingTries >= 300) {
        DuskLog.warn(
            "[ExtNpcMount] №83 room{} mount '{}' GAVE UP after {} tries lastReason={}",
            s_roomMountPendingRoom, s_roomMountPendingProc, s_roomMountPendingTries,
            s_bgCreateFailReason[0] ? s_bgCreateFailReason : "?");
        clearRoomMountPending();
    }
}

void dExtNpcMount_onRoomUnload(const char* stageName, int roomNo) {
    (void)stageName;
    if (roomNo < 0 || roomNo >= 0x40) {
        return;
    }
    if (s_roomMountPendingRoom == roomNo) {
        clearRoomMountPending();
    }
    s_roomLaneMountCreating[roomNo] = false;
    auto it = s_roomLaneProcByRoom.find(roomNo);
    if (it == s_roomLaneProcByRoom.end()) {
        return;
    }
    // №69: soft handles + mount Release should already be done (place-first exit).
    dExtNpcPopulation_clearForBg(it->second.c_str());
    dExtNpcDoors_clearExitKnobForProc(it->second.c_str());
    auto mid = s_bgMountIds.find(it->second);
    if (mid == s_bgMountIds.end()) {
        DuskLog.info("[ExtNpcMount] №69 room{} unload (mount already released) '{}'", roomNo,
                     it->second);
        return;
    }
    // Fallback path (non-exit unload): still Release before delete.
    if (s_bgIslandId == mid->second) {
        s_bgIslandId = fpcM_ERROR_PROCESS_ID_e;
    }
    fopAc_ac_c* existing = fopAcM_SearchByID(mid->second);
    if (existing != NULL) {
        dExtNpcMount_c* mount = (dExtNpcMount_c*)existing;
        if (mount->mpBgW != NULL) {
            if (mount->mBgGlobal) {
                const int r = fopAcM_GetRoomNo(mount);
                if (r >= 0 && r < 0x40 && dStage_roomControl_c::getBgW(r) == mount->mpBgW) {
                    dStage_roomControl_c::setBgW(r, NULL);
                }
            }
            dComIfG_Bgsp().Release(mount->mpBgW);
            mount->mpBgW = NULL;
        }
        fopAcM_delete(existing);
    }
    s_bgMountIds.erase(mid);
    DuskLog.info("[ExtNpcMount] №62 room{} unload drop '{}'", roomNo, it->second);
}

static void abortWarmInteriors(const char* reason) {
    if (!s_warmInteriors) {
        return;
    }
    s_warmInteriors = false;
    s_warmCooldown = 0;
    DuskLog.warn("[ExtNpcMount] №115 warm storm ABORT — {} (idx={})",
                 reason != NULL ? reason : "?", s_warmProviderIndex);
}

static bool warmHeapHasHeadroom() {
    JKRHeap* heap = (JKRHeap*)mDoExt_getGameHeap();
    if (heap == NULL) {
        return false;
    }
    return heap->getTotalFreeSize() >= kWarmBgHeapNeed;
}

static void pollWarmInteriors() {
    if (!s_warmInteriors || s_bgWarpPhase != kBgWarpIdle) {
        return;
    }
    if (s_warmCooldown > 0) {
        --s_warmCooldown;
        return;
    }
    // №115: abort the storm when the heap cannot host another BG solid heap.
    if (!warmHeapHasHeadroom()) {
        abortWarmInteriors("free-heap below headroom");
        return;
    }

    dExtNpcManifest island{};
    const bool haveIsland = dExtNpcMount_lookup("EXT_BG0", &island) && island.hostStage[0] != '\0';

    const int n = dExtNpcMount_providerCount();
    while (s_warmProviderIndex < n) {
        dExtNpcManifest man{};
        if (!dExtNpcMount_providerAt(s_warmProviderIndex++, &man) || !man.isBg ||
            man.proc[0] == '\0') {
            continue;
        }
        if (std::strcmp(man.proc, "EXT_BG0") == 0) {
            continue;
        }
        // Shelved Omori — skip warm (no door traffic).
        if (std::strcmp(man.proc, "EXT_BG7") == 0) {
            continue;
        }
        // №62: room-lane mounts bind to room load — do not eternal-warm.
        if (s_roomLaneRooms.count(man.proc) != 0) {
            continue;
        }
        // №115: data opt-out.
        if (!man.allowWarm) {
            continue;
        }
        // №115: foreign-stage BGs (forest/cave/fountain own stages) must not warm on island.
        if (haveIsland && man.hostStage[0] != '\0' &&
            std::strcmp(man.hostStage, island.hostStage) != 0) {
            continue;
        }
        auto it = s_bgMountIds.find(man.proc);
        if (it != s_bgMountIds.end()) {
            fopAc_ac_c* existing = fopAcM_SearchByID(it->second);
            if (existing != NULL) {
                continue;  // already created (ready or still loading)
            }
            s_bgMountIds.erase(it);
        }
        const fpc_ProcID id = createBgMountAtHost(man, "warm");
        if (id == fpcM_ERROR_PROCESS_ID_e) {
            DuskLog.warn("[ExtNpcMount] №58-B warm create FAILED '{}' reason='{}'", man.proc,
                         s_bgCreateFailReason[0] ? s_bgCreateFailReason : "?");
            // №115: first hard fail ends the storm (do not walk into heap exhaustion).
            abortWarmInteriors("create FAILED");
            return;
        }
        s_bgMountIds[man.proc] = id;
        s_warmCooldown = kWarmCooldownFrames;
        DuskLog.info("[ExtNpcMount] №58-B warm create '{}' id={:08x}", man.proc, (u32)id);
        return;
    }
    s_warmInteriors = false;
    DuskLog.info("[ExtNpcMount] №58-B warm interiors done ({} providers scanned)", n);
}

static bool beginBgWarp(const char* procName, bool hasSpawnOverride, const cXyz& spawnWorld) {
    dExtNpcManifest man{};
    if (procName == NULL || !dExtNpcMount_lookup(procName, &man) || !man.isBg ||
        !man.hasHostPos || !man.hasAnchor || !man.hasSpawnRel || man.hostStage[0] == '\0') {
        DuskLog.warn(
            "[ExtNpcMount] BG warp refused — need type=bg + host_stage + host_pos + "
            "anchor + spawn_rel");
        return false;
    }
    s_bgSpawnOverrideValid = hasSpawnOverride;
    if (hasSpawnOverride) {
        s_bgSpawnOverride = spawnWorld;
    } else {
        s_bgSpawnFacingValid = false;
    }
    s_bgWarpPending = true;
    s_bgWarpPhase = kBgWarpWaitPlayer;
    s_bgIslandId = fpcM_ERROR_PROCESS_ID_e;
    s_bgHoldFrames = 0;
    s_bgSettleFrames = 0;
    s_bgReadyWaitFrames = 0;
    s_bgFadeWatchdogFired = false;
    s_bgHostPos = man.hostPos;
    snprintf(s_bgWarpProc, sizeof(s_bgWarpProc), "%s", procName);
    DuskLog.info("[ExtNpcMount] BG warp '{}' → host {} r{} then relocate{}", procName,
                 man.hostStage, man.hostRoom, hasSpawnOverride ? " (spawn override)" : "");
    return true;
}

bool dExtNpcMount_requestBgWarp(const char* procName) {
    s_bgFailSafeValid = false;
    return beginBgWarp(procName, false, cXyz());
}

bool dExtNpcMount_requestBgWarpTo(const char* procName, const cXyz& spawnWorld) {
    s_bgFailSafeValid = false;
    s_bgSpawnFacingValid = false;
    return beginBgWarp(procName, true, spawnWorld);
}

bool dExtNpcMount_requestBgWarpTo(const char* procName, const cXyz& spawnWorld, s16 facing) {
    s_bgFailSafeValid = false;
    s_bgSpawnFacingValid = true;
    s_bgSpawnFacing = facing;
    return beginBgWarp(procName, true, spawnWorld);
}

bool dExtNpcMount_requestBgWarpGuarded(const char* procName, const cXyz& failSafeSpawn) {
    s_bgFailSafeValid = true;
    s_bgFailSafeSpawn = failSafeSpawn;
    return beginBgWarp(procName, false, cXyz());
}

const char* dExtNpcMount_lastBgProc() {
    return s_lastBgProc;
}

static void placeLinkAt(fopAc_ac_c* player, const cXyz& spawn);
static bool beginBgWarp(const char* procName, bool hasSpawnOverride, const cXyz& spawnWorld);

// №65/№66 — room-lane transport: cover → change room → place → lift (never beginBgWarp).
enum {
    kRoomTxIdle = 0,
    kRoomTxEnterCover = 1,  // №66-A: wait until screen is black before room work
    kRoomTxEnterWait = 2,   // load + mount + room draw ready
    kRoomTxEnterHold = 3,
    kRoomTxExitCover = 4,   // №66-A: black before unload/place
    kRoomTxExitUnload = 5,  // №66-B: wait until room fully gone
    kRoomTxExitHold = 6,
};
static int s_roomTxPhase = kRoomTxIdle;
static char s_roomTxProc[32] = {};
static cXyz s_roomTxFailSafe;
static bool s_roomTxFailSafeValid = false;
static cXyz s_roomTxReturn;
static s16 s_roomTxFacing = 0;
static bool s_roomTxHasFacing = false;
static cXyz s_roomTxHoldPos;
static int s_roomTxHoldFrames = 0;
static int s_roomTxWaitFrames = 0;
static bool s_roomTxFadeWatch = false;
static bool s_roomTxForgot = false;

static void roomTxForceCover() {
    JUTFader* fader = JFWDisplay::getManager()->getFader();
    if (fader == NULL) {
        return;
    }
    if (fader->getStatus() != JUTFader::FadeOut && fader->getStatus() != JUTFader::Wait) {
        mDoGph_gInf_c::startFadeOut(8);
    }
}

static bool roomTxScreenBlack() {
    JUTFader* fader = JFWDisplay::getManager()->getFader();
    if (fader == NULL) {
        return true;
    }
    // Wait = solid black. FadeOut alone is mid-cover — not enough to hide the place.
    return fader->getStatus() == JUTFader::Wait;
}

static void roomTxStartFadeIn() {
    JUTFader* fader = JFWDisplay::getManager()->getFader();
    if (fader != NULL &&
        (fader->getStatus() == JUTFader::FadeOut || fader->getStatus() == JUTFader::Wait)) {
        fader->setStatus(JUTFader::None, 0);
    }
    mDoGph_gInf_c::startFadeIn(15);
}

static void roomTxAssignPlayerRoom(fopAc_ac_c* player, int roomNo) {
    if (player == NULL || roomNo < 0 || roomNo >= 0x40) {
        return;
    }
    fopAcM_SetRoomNo(player, (s8)roomNo);
    player->tevStr.room_no = (s8)roomNo;
    // Layer move only when the room scene exists (else assert in setRoomLayer).
    if (dComIfGp_roomControl_checkStatusFlag(roomNo, 0x01)) {
        fopAcM_setRoomLayer(player, roomNo);
    }
}

static bool roomTxDrawable(int hostRoom) {
    return hostRoom > 0 && dComIfGp_roomControl_checkRoomDisp(hostRoom) != FALSE;
}

// №104: WW host rooms have no retail daBg (68-byte FILI+PLYR skeleton), so flag 0x10
// never gets set — rooms 1–5 stay invisible/non-solid. Mirror daBg_Create's onStatusFlag.
static void activateWwHostRoom(int roomNo, const char* reason) {
    if (roomNo < 0 || roomNo >= 0x40) {
        return;
    }
    dComIfGp_roomControl_offStatusFlag(roomNo, 0x08);  // clear hide
    dComIfGp_roomControl_onStatusFlag(roomNo, 0x10);   // drawable/solid (daBg)
    dStage_roomControl_c::setStayNo(roomNo);           // mDraw + stay
    DuskLog.info("[ExtNpcMount] №104 activate room{} ({}) disp={}", roomNo,
                 reason != NULL ? reason : "?",
                 dComIfGp_roomControl_checkRoomDisp(roomNo) != FALSE ? 1 : 0);
}

bool dExtNpcMount_roomLaneTransportBusy() {
    return s_roomTxPhase != kRoomTxIdle;
}

bool dExtNpcMount_bgWarpBusy() {
    return s_bgWarpPhase != kBgWarpIdle || s_roomTxPhase != kRoomTxIdle;
}

bool dExtNpcMount_requestRoomLaneEnter(const char* procName, const cXyz& failSafeSpawn) {
    dExtNpcManifest man{};
    if (procName == NULL || !dExtNpcMount_lookup(procName, &man) || !man.isBg ||
        !man.hasHostPos || !man.hasSpawnRel) {
        DuskLog.warn("[ExtNpcMount] №65 room-lane enter refused — incomplete manifest '{}'",
                     procName ? procName : "(null)");
        return false;
    }
    const int hostRoom = dExtNpcMount_roomLaneHostRoom(procName);
    if (hostRoom < 0) {
        DuskLog.warn("[ExtNpcMount] №65 room-lane enter refused — '{}' not registered", procName);
        return false;
    }
    if (s_roomTxPhase != kRoomTxIdle || s_bgWarpPhase != kBgWarpIdle) {
        DuskLog.warn("[ExtNpcMount] №65 room-lane enter refused — transport busy");
        return false;
    }
    std::snprintf(s_roomTxProc, sizeof(s_roomTxProc), "%s", procName);
    s_roomTxFailSafe = failSafeSpawn;
    s_roomTxFailSafeValid = true;
    s_roomTxWaitFrames = 0;
    s_roomTxFadeWatch = false;
    roomTxForceCover();
    s_roomTxPhase = kRoomTxEnterCover;
    DuskLog.info("[ExtNpcMount] №66 room-lane ENTER cover→load '{}' room={} (no BgWarp)",
                 procName, hostRoom);
    return true;
}

bool dExtNpcMount_requestRoomLaneExit(const char* procName, const cXyz& returnWorld, s16 facing,
                                       bool hasFacing) {
    if (procName == NULL || procName[0] == '\0') {
        return false;
    }
    if (s_roomTxPhase != kRoomTxIdle || s_bgWarpPhase != kBgWarpIdle) {
        DuskLog.warn("[ExtNpcMount] №65 room-lane exit refused — transport busy");
        return false;
    }
    std::snprintf(s_roomTxProc, sizeof(s_roomTxProc), "%s", procName);
    s_roomTxReturn = returnWorld;
    s_roomTxFacing = facing;
    s_roomTxHasFacing = hasFacing;
    s_roomTxFailSafeValid = false;
    s_roomTxWaitFrames = 0;
    s_roomTxFadeWatch = false;
    s_roomTxForgot = false;
    roomTxForceCover();
    s_roomTxPhase = kRoomTxExitCover;
    DuskLog.info("[ExtNpcMount] №66 room-lane EXIT cover→unload '{}' (no BgWarp)", procName);
    return true;
}

static void pollRoomLaneTransport() {
    if (s_roomTxPhase == kRoomTxIdle) {
        return;
    }

    // №66-A enter: stay black before touching room memory.
    if (s_roomTxPhase == kRoomTxEnterCover) {
        roomTxForceCover();
        ++s_roomTxWaitFrames;
        if (!roomTxScreenBlack() && s_roomTxWaitFrames < 45) {
            return;
        }
        const int hostRoom = dExtNpcMount_roomLaneHostRoom(s_roomTxProc);
        if (hostRoom >= 0) {
            dExtNpcMount_ensureRoomLaneLoaded(hostRoom);
        }
        s_roomTxWaitFrames = 0;
        s_roomTxPhase = kRoomTxEnterWait;
        DuskLog.info("[ExtNpcMount] №66 room-lane ENTER load '{}'", s_roomTxProc);
        return;
    }

    if (s_roomTxPhase == kRoomTxEnterWait) {
        dExtNpcManifest man{};
        if (!dExtNpcMount_lookup(s_roomTxProc, &man) || !man.isBg) {
            s_roomTxPhase = kRoomTxIdle;
            dExtNpcMount_endDoorDemoLock();
            return;
        }
        const int hostRoom = dExtNpcMount_roomLaneHostRoom(s_roomTxProc);
        if (hostRoom >= 0) {
            dExtNpcMount_ensureRoomLaneLoaded(hostRoom);
        }
        roomTxForceCover();
        fpc_ProcID mid = fpcM_ERROR_PROCESS_ID_e;
        auto it = s_bgMountIds.find(s_roomTxProc);
        if (it != s_bgMountIds.end()) {
            mid = it->second;
        }
        fopAc_ac_c* island = mid != fpcM_ERROR_PROCESS_ID_e ? fopAcM_SearchByID(mid) : NULL;
        dExtNpcMount_c* mount = island != NULL ? (dExtNpcMount_c*)island : NULL;
        const bool mountReady =
            mount != NULL && mount->mIsBg && mount->mBgReady && mount->mpBgW != NULL;
        // №66-B: wait for room draw bit (daBg sets 0x10) — mount alone is not enough.
        const bool roomReady = hostRoom < 0 || roomTxDrawable(hostRoom);
        if (!mountReady || !roomReady) {
            ++s_roomTxWaitFrames;
            if (s_roomTxFailSafeValid && s_roomTxWaitFrames >= kBgReadyHardTimeout) {
                DuskLog.warn(
                    "[Doors] enter {} → ABORT reason=room_tx_timeout after {}f "
                    "(mount={} roomDisp={}) — fail-safe",
                    s_roomTxProc, s_roomTxWaitFrames, mountReady ? 1 : 0, roomReady ? 1 : 0);
                const cXyz back = s_roomTxFailSafe;
                s_roomTxPhase = kRoomTxIdle;
                s_roomTxFailSafeValid = false;
                dExtNpcMount_endDoorDemoLock();
                if (hostRoom >= 0) {
                    dExtNpcMount_ensureRoomLaneUnloaded(hostRoom);
                }
                beginBgWarp("EXT_BG0", true, back);
                return;
            }
            return;
        }

        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player == NULL) {
            return;
        }
        updateBgTransform(mount);
        if (mount->mpBgW != NULL && !mount->mBgGlobal) {
            mount->mpBgW->Move();
        }
        cXyz spawn = man.hostPos + man.spawnRel;
        const f32 refY = spawn.y;
        if (!dExtNpcMount_localGroundSnap(&spawn, refY)) {
            spawn.y = refY + 50.0f;
        }
        placeLinkAt(player, spawn);
        forceLinkGroundReprobe(player);
        if (hostRoom >= 0) {
            roomTxAssignPlayerRoom(player, hostRoom);
            activateWwHostRoom(hostRoom, "room-lane-enter");
        }
        if (man.hasSpawnRy) {
            player->current.angle.y = man.spawnRy;
            player->shape_angle.y = man.spawnRy;
        }
        s_roomTxHoldPos = spawn;
        s_roomTxHoldFrames = 120;
        s_bgIslandId = mid;
        roomTxStartFadeIn();
        dExtNpcPopulation_spawnForBg(man);
        DuskLog.info(
            "[ExtNpcMount] №66 Link placed room-lane '{}' at spawn_rel → ({:.0f},{:.0f},{:.0f}) "
            "roomDisp={}",
            s_roomTxProc, spawn.x, spawn.y, spawn.z, roomReady ? 1 : 0);
        s_roomTxPhase = kRoomTxEnterHold;
        return;
    }

    // №66-A exit: black, then unload (symmetric wait), then place.
    if (s_roomTxPhase == kRoomTxExitCover) {
        roomTxForceCover();
        ++s_roomTxWaitFrames;
        if (!roomTxScreenBlack() && s_roomTxWaitFrames < 45) {
            return;
        }
        s_roomTxWaitFrames = 0;
        s_roomTxPhase = kRoomTxExitUnload;
        return;
    }

    if (s_roomTxPhase == kRoomTxExitUnload) {
        roomTxForceCover();
        const int hostRoom = dExtNpcMount_roomLaneHostRoom(s_roomTxProc);
        // №69: place Link on island FIRST → Release interior BgW → THEN free room.
        // Never Release a dBgW the player is standing on.
        if (!s_roomTxForgot) {
            fopAc_ac_c* player = dComIfGp_getPlayer(0);
            if (player == NULL) {
                s_roomTxPhase = kRoomTxIdle;
                dExtNpcMount_endDoorDemoLock();
                return;
            }
            forgetRoomLaneSoftHandles(s_roomTxProc, hostRoom);

            cXyz spawn = s_roomTxReturn;
            dExtNpcMount_localGroundSnap(&spawn, s_roomTxReturn.y);
            placeLinkAt(player, spawn);
            roomTxAssignPlayerRoom(player, 0);
            dStage_roomControl_c::setStayNo(0);
            if (s_roomTxHasFacing) {
                player->current.angle.y = s_roomTxFacing;
                player->shape_angle.y = s_roomTxFacing;
            }
            forceLinkGroundReprobe(player);
            s_roomTxHoldPos = spawn;
            DuskLog.info(
                "[ExtNpcMount] №69 room-lane EXIT place-first '{}' → porch "
                "({:.0f},{:.0f},{:.0f})",
                s_roomTxProc, spawn.x, spawn.y, spawn.z);

            releaseRoomLaneMount(s_roomTxProc);
            s_roomTxForgot = true;
            s_roomTxWaitFrames = 0;
        }
        bool gone = true;
        if (hostRoom >= 0) {
            gone = dExtNpcMount_ensureRoomLaneUnloaded(hostRoom);
        }
        ++s_roomTxWaitFrames;
        if (!gone && s_roomTxWaitFrames < 180) {
            return;
        }
        if (!gone) {
            DuskLog.warn(
                "[ExtNpcMount] №69 room-lane EXIT unload slow room{} after {}f — continue",
                hostRoom, s_roomTxWaitFrames);
        }
        clearRoomLaneUnloading(hostRoom);
        s_roomTxForgot = false;
        s_roomTxHoldFrames = 45;
        roomTxStartFadeIn();
        std::snprintf(s_lastBgProc, sizeof(s_lastBgProc), "EXT_BG0");
        // №257: releasing the interior cleared s_bgIslandId when it matched —
        // restore the live exterior mount id so soft-state matches "active EXT_BG0".
        {
            auto ext = s_bgMountIds.find("EXT_BG0");
            if (ext != s_bgMountIds.end() && fopAcM_SearchByID(ext->second) != NULL) {
                s_bgIslandId = ext->second;
            }
        }
        DuskLog.info("[ExtNpcMount] №69 room-lane EXIT complete — active EXT_BG0 island={:08x}",
                     (u32)s_bgIslandId);
        s_roomTxPhase = kRoomTxExitHold;
        return;
    }

    if (s_roomTxPhase == kRoomTxEnterHold || s_roomTxPhase == kRoomTxExitHold) {
        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player != NULL) {
            const f32 dx = player->current.pos.x - s_roomTxHoldPos.x;
            const f32 dz = player->current.pos.z - s_roomTxHoldPos.z;
            if ((dx * dx + dz * dz) > (5000.0f * 5000.0f)) {
                placeLinkAt(player, s_roomTxHoldPos);
            } else {
                player->speed.x = 0.0f;
                player->speed.z = 0.0f;
            }
        }
        if (s_roomTxPhase == kRoomTxEnterHold) {
            fopAc_ac_c* island = fopAcM_SearchByID(s_bgIslandId);
            if (island != NULL) {
                dExtNpcMount_c* mount = (dExtNpcMount_c*)island;
                if (mount->mpBgW != NULL && !mount->mBgGlobal) {
                    updateBgTransform(mount);
                    mount->mpBgW->Move();
                }
            }
        }
        if (!s_roomTxFadeWatch && s_roomTxHoldFrames <= 90) {
            JUTFader* fader = JFWDisplay::getManager()->getFader();
            if (fader != NULL &&
                (fader->getStatus() == JUTFader::None ||
                 fader->getStatus() == JUTFader::FadeOut)) {
                fader->setStatus(JUTFader::None, 0);
                if (mDoGph_gInf_c::startFadeIn(10)) {
                    s_roomTxFadeWatch = true;
                }
            } else if (fader != NULL && (fader->getStatus() == JUTFader::FadeIn ||
                                         fader->getStatus() == JUTFader::Wait)) {
                s_roomTxFadeWatch = true;
            }
        }
        if (--s_roomTxHoldFrames <= 0) {
            if (s_roomTxPhase == kRoomTxEnterHold) {
                std::snprintf(s_lastBgProc, sizeof(s_lastBgProc), "%s", s_roomTxProc);
            } else {
                std::snprintf(s_lastBgProc, sizeof(s_lastBgProc), "EXT_BG0");
            }
            DuskLog.info("[ExtNpcMount] №66 room-lane transport complete — active '{}'",
                         s_lastBgProc);
            s_roomTxPhase = kRoomTxIdle;
            dExtNpcMount_endDoorDemoLock();
        }
    }
}

// №84: after a native stage change back to the exterior, remount EXT_BG0 etc.
static char s_stageExitRemountProc[32] = {};
static cXyz s_stageExitRemountSpawn;
static s16 s_stageExitRemountFacing = 0;
static bool s_stageExitRemountHasFacing = false;
static bool s_stageExitRemountArmed = false;

void dExtNpcMount_cancelTransports() {
    if (s_bgWarpPhase != kBgWarpIdle) {
        DuskLog.info("[ExtNpcMount] №84 cancel BG transport phase={}", s_bgWarpPhase);
        s_bgWarpPhase = kBgWarpIdle;
        s_bgWarpPending = false;
        s_bgFailSafeValid = false;
        s_bgReadyWaitFrames = 0;
    }
    if (s_roomTxPhase != kRoomTxIdle) {
        DuskLog.info("[ExtNpcMount] №84 cancel room-lane transport phase={}", s_roomTxPhase);
        s_roomTxPhase = kRoomTxIdle;
        s_roomTxProc[0] = '\0';
        s_roomTxFailSafeValid = false;
        s_roomTxWaitFrames = 0;
    }
    clearRoomMountPending();
}

void dExtNpcMount_armStageExitRemount(const char* procName, const cXyz& spawnWorld, s16 facing,
                                       bool hasFacing) {
    if (procName == NULL || procName[0] == '\0') {
        s_stageExitRemountArmed = false;
        s_stageExitRemountProc[0] = '\0';
        return;
    }
    std::snprintf(s_stageExitRemountProc, sizeof(s_stageExitRemountProc), "%s", procName);
    s_stageExitRemountSpawn = spawnWorld;
    s_stageExitRemountFacing = facing;
    s_stageExitRemountHasFacing = hasFacing;
    s_stageExitRemountArmed = true;
    DuskLog.info("[ExtNpcMount] №84 arm stage-exit remount '{}' spawn=({:.1f},{:.1f},{:.1f})",
                 s_stageExitRemountProc, spawnWorld.x, spawnWorld.y, spawnWorld.z);
}

static void tryStageExitRemount() {
    if (!s_stageExitRemountArmed || s_stageExitRemountProc[0] == '\0') {
        return;
    }
    if (fpcM_SearchByName(fpcNm_PLAY_SCENE_e) == NULL || dComIfGp_getPlayer(0) == NULL) {
        return;
    }
    dExtNpcManifest man{};
    if (!dExtNpcMount_lookup(s_stageExitRemountProc, &man) || !man.isBg) {
        s_stageExitRemountArmed = false;
        return;
    }
    const char* stage = dComIfGp_getStartStageName();
    if (man.hostStage[0] != '\0' &&
        (stage == NULL || std::strcmp(stage, man.hostStage) != 0)) {
        return;  // not on the exterior host yet
    }
    s_stageExitRemountArmed = false;
    if (s_stageExitRemountHasFacing) {
        s_bgSpawnFacingValid = true;
        s_bgSpawnFacing = s_stageExitRemountFacing;
    } else {
        s_bgSpawnFacingValid = false;
    }
    const bool ok = beginBgWarp(s_stageExitRemountProc, true, s_stageExitRemountSpawn);
    DuskLog.info("[ExtNpcMount] №84 stage-exit remount '{}' ok={} stage='{}'",
                 s_stageExitRemountProc, ok ? 1 : 0, stage != NULL ? stage : "?");
}

u32 dExtNpcWorld_generation() {
    return s_worldGeneration;
}

void dExtNpcWorld_bump(const char* reason) {
    ++s_worldGeneration;
    if (s_worldGeneration == 0) {
        s_worldGeneration = 1;  // skip 0
    }
    dExtNpcDoors_clearSpawnLatches();
    dExtNpcPopulation_clearAll();
    s_interiorBootstrapProc[0] = '\0';
    // №257 / Verdict 2 reading (2): play-scene tear destroys BG actors but left
    // s_bgMountIds / s_bgIslandId pointing at recycled ProcIDs — reuse then
    // operated on the wrong BG (or a corpse). Soft handles die with the stage.
    if (!s_bgMountIds.empty() || s_bgIslandId != fpcM_ERROR_PROCESS_ID_e) {
        DuskLog.info(
            "[ExtNpcWorld] №257 drop stale BG soft-handles count={} island={:08x} ({})",
            (int)s_bgMountIds.size(), (u32)s_bgIslandId, reason != NULL ? reason : "?");
    }
    s_bgMountIds.clear();
    s_bgIslandId = fpcM_ERROR_PROCESS_ID_e;
    DuskLog.info("[ExtNpcWorld] №94 bump gen={} ({})", s_worldGeneration,
                 reason != NULL ? reason : "?");
}

// §46/№153: the crossing INTO the donor world, and the opening beat once there.
//
// The door lane shipped the mechanism (unlock flag + runtime warp command,
// `d_ext_fado_door.h`) and explicitly left "real callers" open. This is that
// caller. Destination is set as a COMMAND rather than hard-coded in the door,
// which is the shape that lane chose so the door stays generic.
static void dExtWw_wireFadoCrossing(const char* stage) {
    if (stage == NULL) {
        return;
    }
    // Ordon village: make the door usable and point it at the island.
    if (std::strcmp(stage, "F_SP103") == 0) {
        if (!dFadoDoor_isUnlocked()) {
            dFadoDoor_setUnlocked(true);
            DuskLog.info("[ExtWw] §46 Fado door unlocked (save-scoped)");
        }
        if (!dFadoDoor_hasWarpCommand()) {
            // Outset exterior host: F_DL01 room 44, the stage No.107 built.
            dFadoDoor_setWarpCommand("F_DL01", 44, 0, -1, -1);
            DuskLog.info("[ExtWw] §46 Fado warp command -> F_DL01 room 44 spawn 0");
        }
    }
}

// §46/№153: order the opening event on arrival. UNPROVEN — this is the first
// attempt to run donor cutscene data through the receiver's event system, and
// it is deliberately instrumented rather than assumed:
//   * `awake` was authored into F_DL01's own event_list.dat by merge_event.py
//     (№152) and reads back correctly, so getEventIdx SHOULD resolve it.
//   * whether the JStage adaptor then binds the storyboard's actors ('Link',
//     'Ls1') to ours is the real unknown (№151).
// Logging both the index and the order tells us which half failed, if either.
static bool s_openingOrdered = false;
static int s_openingWaitFrames = 0;  // №162: sparse-log counter for the order gate
/**
 * №170: two layers — do NOT collapse them.
 *   pending  = F_DL01 entry → opening done/abandoned  → defers №110 snap only
 *   hold     = after awake ORDER → demo end             → pauses G-guard too
 * Arming hold on stage-ready blocked residual clear; awake deferred forever.
 */
static bool s_openingPending = false;
static bool s_openingCameraHold = false;
static bool s_openingSawDemo = false;
static int s_openingHoldFrames = 0;

// №254 / №222: durable "opening finished" (donor UNK_3510). Statics reset on
// interior→exterior recreate; this mod-flag does not. Key is ours (covenant).
static constexpr const char* kOpeningDoneFlag = "ww.awake_played";

static const char* dExtWw_primaryModFolder() {
    for (const auto& kv : s_providers) {
        if (kv.second.modFolder[0] != '\0') {
            return kv.second.modFolder;
        }
    }
    return nullptr;
}

static bool dExtWw_openingAlreadyDone() {
    const char* mod = dExtWw_primaryModFolder();
    return mod != nullptr && dExtModFlags_get(mod, kOpeningDoneFlag);
}

static void dExtWw_markOpeningDone(const char* why) {
    const char* mod = dExtWw_primaryModFolder();
    if (mod == nullptr) {
        return;
    }
    if (dExtModFlags_get(mod, kOpeningDoneFlag)) {
        return;
    }
    dExtModFlags_set(mod, kOpeningDoneFlag, true);
    DuskLog.info("[ExtWw] №254 opening-done flag set ({}) — durable across transits",
                 why != nullptr ? why : "?");
}

bool dExtWw_openingOwnsCamera() {
    const char* stage = dComIfGp_getStartStageName();
    if (stage == NULL || std::strcmp(stage, "F_DL01") != 0) {
        return false;
    }
    return s_openingPending;
}

bool dExtWw_openingPauseArrivalGuard() {
    const char* stage = dComIfGp_getStartStageName();
    if (stage == NULL || std::strcmp(stage, "F_DL01") != 0) {
        return false;
    }
    return s_openingCameraHold;
}

static void dExtWw_clearOpeningCameraState() {
    s_openingPending = false;
    s_openingCameraHold = false;
    s_openingSawDemo = false;
    s_openingHoldFrames = 0;
}

/**
 * №171: №110 must not fire while we still intend to run `awake`, nor while it
 * owns the cam. Distinct from ownsCamera — pending must NOT pause G-guard
 * (№169: force-ending the leftover slot is what lets the order succeed).
 */
bool dExtWw_deferArrivalCameraSnap() {
    const char* stage = dComIfGp_getStartStageName();
    if (stage == NULL || std::strcmp(stage, "F_DL01") != 0) {
        return false;
    }
    if (s_openingCameraHold) {
        return true;
    }
    // Still trying to order (busy slot / arc load) — keep map-edge snap off.
    return !s_openingOrdered;
}

static void dExtWw_orderOpening(const char* stage) {
    if (stage == NULL || std::strcmp(stage, "F_DL01") != 0 || s_openingOrdered) {
        return;
    }
    // №254: durable gate (mirrors donor 0x3510). Interior door recreates the
    // play scene and clears s_openingOrdered; the save-side flag does not.
    if (dExtWw_openingAlreadyDone()) {
        s_openingOrdered = true;
        dExtWw_clearOpeningCameraState();
        DuskLog.info("[ExtWw] №254 opening skipped — '{}' already set", kOpeningDoneFlag);
        return;
    }
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player == NULL) {
        // №155: WAIT, do not give up. The first cut of this ran only from
        // onStageReady and returned here claiming it would "retry next
        // stage-ready" — but onStageReady fires ONCE per stage change, so there
        // was no next one. The player does not exist yet at that moment, so the
        // opening was never even attempted: the log showed the Fado lines and
        // no getEventIdx line at all. It is now also driven from the per-frame
        // poll, which is the retry that comment assumed.
        return;
    }
    const s16 idx = dComIfGp_getEventManager().getEventIdx(player, "awake", 0xff);
    // ========================================================================
    // №263 — do NOT latch failure on the FIRST unresolved frame. On the warp
    // entry path the event manager registers the stage list a beat after the
    // player exists, so the one-shot latch killed the intro for the whole
    // session (№155's own lesson, one level deeper: WAIT, do not give up).
    // Bounded retry; the warn fires only when it stays unresolved for 10s.
    // ========================================================================
    // DONOR-FAITHFUL (№263 rev 2, decomp-verified): WW actors resolve
    // getEventIdx TRUSTING success — the donor's load order guarantees the
    // stage list registers before actors run, so "resolution failed" is not a
    // state WW has (d_a_andsw0/auction/deku_item idiom: resolve, use, no
    // give-up branch). The receiver's warp path breaks that ordering
    // guarantee, so we WAIT for the donor-guaranteed state: retry every frame,
    // no give-up latch, periodic warn as pure diagnostic.
    static int s_openingResolveRetries = 0;
    if (idx < 0) {
        // ====================================================================
        // №266 — §67 proved the ROOT: BASE_STAGE never loads on the warp path
        // (create() ran before the stage resource was queryable; the engine
        // has no retry). Restore the donor guarantee: late-init the stage
        // list the moment the resource exists, then the room list, and let
        // the normal resolve above succeed next frame.
        // ====================================================================
        dEvent_manager_c& em66 = dComIfGp_getEventManager();
        const int late = em66.lateStageListInit();
        if (late == 2) {
            em66.roomInit(-1);
            DuskLog.warn("[ExtWw] №266 stage event list LATE-LOADED (+roomInit) after {} "
                         "failed resolves — donor availability guarantee restored",
                         s_openingResolveRetries);
            return;  // resolve retries next frame against the populated slots
        }
        if (++s_openingResolveRetries % 600 == 0) {
            DuskLog.warn(
                "[ExtWw] §46 'awake' unresolved after {} attempts (still retrying — donor "
                "guarantees the stage list; event_list.dat is present and round-trip "
                "verified №152)",
                s_openingResolveRetries);
            // ================================================================
            // §68 — №266 never fired (getStageRes NULL all session), so the
            // stage-res SLOT itself is the suspect. Two-way discriminator:
            //   dzs=NULL too  → whole Stage-archive slot unmounted/redirected
            //   dzs=OK, evt=NULL → a DIFFERENT (unmerged) F_DL01 Stage.arc is
            //                      mounted than №152's verified file
            // ================================================================
            const void* evtRes = dComIfG_getStageRes("event_list.dat");
            const void* dzsRes = dComIfG_getStageRes("stage.dzs");
            DuskLog.warn("[ExtWw] §68 stage-res slot: event_list={} stage.dzs={} stage='{}'",
                         evtRes != NULL ? "OK" : "NULL", dzsRes != NULL ? "OK" : "NULL",
                         dComIfGp_getStartStageName());
            // ================================================================
            // §67 probe — WHAT the event manager actually holds when the
            // resolve fails (room id is now correct per №265, so the defect
            // moved into the LIST itself). Per slot: loaded?, roomNo, count,
            // first names. One glance answers: wrong slot / evicted room list
            // / unmerged list / name mismatch.
            // ================================================================
            dEvent_manager_c& em67 = dComIfGp_getEventManager();
            const int playerRoom = fopAcM_GetRoomNo(player);
            DuskLog.warn("[ExtWw] §67 slots (player room {}):", playerRoom);
            for (int t = dEvent_manager_c::BASE_KEEP; t < dEvent_manager_c::BASE_MAX; ++t) {
                if (em67.probeSlot(t).getHeaderP() == NULL) {
                    DuskLog.warn("[ExtWw] §67   slot {}: (empty)", t);
                    continue;
                }
                const int n = (int)em67.probeSlot(t).getEventNum();
                std::string names;
                bool hasAwake = false;
                for (int e = 0; e < n; ++e) {
                    const char* nm = em67.probeSlot(t).getEventName(e);
                    if (nm != NULL && std::strcmp(nm, "awake") == 0) {
                        hasAwake = true;
                    }
                    if (e < 6 && nm != NULL) {
                        if (!names.empty()) {
                            names += ",";
                        }
                        names += nm;
                    }
                }
                DuskLog.warn("[ExtWw] §67   slot {}: roomNo={} num={} awake={} [{}{}]", t,
                             (int)em67.probeSlot(t).roomNo(), n, hasAwake ? "YES" : "no",
                             names, n > 6 ? ",..." : "");
            }
        }
        return;
    }
    DuskLog.info("[ExtWw] §46 opening 'awake' getEventIdx -> {} (after {} retries)", (int)idx,
                 s_openingResolveRetries);
    s_openingResolveRetries = 0;
    // §48/№161: the storyboard is NOT fetched from eventInfo's archive. The PLAY
    // cut calls `getStbDemoData("awake.stb")`, which looks the file up in
    // `dStage_roomControl_c::getDemoArcName()` — a name normally derived from the
    // room's LBNK chunk as "Demo%02d_%02d" and loaded via dComIfG_setObjectRes.
    //
    // Our authored room44.dzs has no LBNK, so that name stayed EMPTY: the lookup
    // took its "unknown archive name" branch, returned NULL, and (the assert being
    // inert in release) dDemo_c::start(NULL,...) did nothing. That is precisely
    // "ordered and accepted, but nothing plays".
    //
    // Rather than synthesise an LBNK chunk to encode a bank pair, name the demo
    // archive directly — getStbDemoData only does getObjectRes(name, file), so any
    // name works provided the archive is resident under it.
    //
    // The load is ASYNC. d_s_room.cpp:270 does not use the name the frame it is
    // set — it spins on dComIfG_syncObjectRes and returns 0 (retry) while the
    // phase is still positive. Ordering in the same frame we request the archive
    // reproduces the very symptom we are chasing: the order is accepted, then the
    // PLAY cut asks for awake.stb before the arc is resident and gets NULL.
    //
    // So this whole block is a GATE, not a fire-and-forget. It returns without
    // latching s_openingOrdered while the arc is in flight, and the per-frame
    // poll (№155) brings us straight back here next frame.
    {
        if (*dStage_roomControl_c::getDemoArcName() == '\0') {
            if (!dComIfG_setObjectRes("Demo02", 0, (JKRHeap*)NULL)) {
                DuskLog.warn(
                    "[ExtWw] §48 demo archive 'Demo02' would not load — the storyboard "
                    "cannot be fetched without it, so the event would order and play "
                    "nothing. Not ordering; will retry.");
                return;  // no latch: retry on the next poll
            }
            SAFE_SPRINTF(dStage_roomControl_c::getDemoArcName(), "%s", "Demo02");
            DuskLog.info("[ExtWw] §48 demo archive requested -> '{}'",
                         static_cast<const char*>(dStage_roomControl_c::getDemoArcName()));
        }

        const int phase = dComIfG_syncObjectRes(dStage_roomControl_c::getDemoArcName());
        if (phase > 0) {
            return;  // still loading — no latch, come back next frame
        }
        if (phase < 0) {
            DuskLog.warn("[ExtWw] §48 demo archive '{}' failed to load (phase {})",
                         static_cast<const char*>(dStage_roomControl_c::getDemoArcName()), phase);
            *dStage_roomControl_c::getDemoArcName() = '\0';  // as native does on failure
            return;
        }
        DuskLog.info("[ExtWw] §48 demo archive resident -> '{}'",
                     static_cast<const char*>(dStage_roomControl_c::getDemoArcName()));
    }

    // №162: the ORDER is a gate too, for the same reason the archive load is.
    //
    // `fopAcM_orderOtherEventId` refuses outright when `isOrderOK()` is false —
    // that is `mEventStatus == 0 || mEventStatus == 2`, so a non-zero status means
    // some OTHER event owns the slot this frame. The №161 arc gate pushed our order
    // one frame later than before, straight into a busy window: the order flipped
    // from accepted (-> 1, but with no arc) to refused (-> 0, with the arc resident).
    //
    // Latching on a REFUSAL is what turns a transient busy frame into a permanent
    // failure. So we do not latch until the manager actually takes it. Retries are
    // logged sparsely — a per-frame log would bury the run.
    if (!dComIfGp_getEvent()->isOrderOK()) {
        if ((s_openingWaitFrames++ % 60) == 0) {
            DuskLog.info(
                "[ExtWw] §46 opening deferred — another event holds the slot "
                "(isOrderOK false, waited {} frames). Not latching; will retry.",
                (int)s_openingWaitFrames);
        }
        return;  // no latch
    }

    // №156: the first cut only set a CONDITION. `onCondition(CANDEMO)` marks the
    // actor as ABLE to run a demo — for a knob door that pairs with an A-press.
    // Nothing was ever going to start it on arrival, which is why the log showed
    // a clean resolve and ordered-line while nothing played.
    //
    // The real sequence, from the NPC path (d_a_npc.cpp evtOrder) and
    // d_event.cpp: name the archive on BOTH the actor and the manager, then
    // actually ORDER the event.
    player->eventInfo.setArchiveName("Demo02");  // holds awake.stb + the cast anims
    dComIfGp_getEventManager().setObjectArchive(player->eventInfo.getArchiveName());
    player->eventInfo.setEventId(idx);
    player->eventInfo.setMapToolId(0xff);
    const s32 ok = fopAcM_orderOtherEventId(player, idx, 0xff, 0xffff, 40, 1);
    DuskLog.info("[ExtWw] §46 opening ORDER archive=Demo02 event={} -> {}", (int)idx,
                 (int)ok);
    if (ok == 0) {
        DuskLog.warn(
            "[ExtWw] §46 order REFUSED despite isOrderOK — event resolved (idx {}), stage "
            "list read, archive resident. This is `dComIfGp_event_order` itself declining, "
            "NOT a busy slot and NOT missing data. Not latching; will retry.",
            (int)idx);
        return;  // no latch: a refusal must not become permanent
    }
    s_openingOrdered = true;  // latch ONLY on an accepted order
    // №170: G-guard pause starts HERE — after order — so residual/G-guard can
    // still clear the busy slot that blocks awake (№169 log order).
    s_openingCameraHold = true;
    s_openingSawDemo = false;
    s_openingHoldFrames = 0;
    // №172: QuickStart leaves Active() true → Run() instead of NotRun().
    // Event CAMERA/STBWAIT only drives the view on the NotRun path (and store
    // was ignoring the demo cam while cameraPlay). Stop so the opening pan can
    // own the view from the first awake frame instead of the map-edge spawn.
    if (dCamera_c* cam = dCam_getBody()) {
        cam->Stop();
        DuskLog.info("[ExtWw] №172 camera Stop after awake ORDER — force event/demo view path");
    }
    DuskLog.info(
        "[ExtWw] №170 opening camera hold ARMED after ORDER — snap deferred, G-guard paused");
}

// №170: release pending/hold once the opening has run (or timed out).
static void dExtWw_pollOpeningCameraHold() {
    if (!s_openingPending && !s_openingCameraHold) {
        return;
    }
    const char* stage = dComIfGp_getStartStageName();
    if (stage == NULL || std::strcmp(stage, "F_DL01") != 0) {
        dExtWw_clearOpeningCameraState();
        return;
    }
    // Pending without an order yet: G-guard/residual must still run. Only the
    // post-ORDER hold watches demo lifetime.
    if (!s_openingCameraHold) {
        return;
    }
    ++s_openingHoldFrames;
    const bool running = dComIfGp_event_runCheck() != 0;
    const bool demoCam = dDemo_c::getCamera() != NULL;
    if (running || demoCam) {
        if (!s_openingSawDemo) {
            s_openingSawDemo = true;
            DuskLog.info("[ExtWw] №170 opening camera hold — awake demo live (frame {})",
                         (int)dDemo_c::getFrame());
        }
        // №254 / №222: donor sets UNK_3510 at msg-frame 0xC8 (title-card beat).
        // Latch our durable flag at the same demo-frame threshold.
        if (dDemo_c::getFrame() >= 0xC8u) {
            dExtWw_markOpeningDone("title-card frame 0xC8");
        }
        return;
    }
    if (s_openingSawDemo) {
        DuskLog.info(
            "[ExtWw] №170 opening camera hold RELEASE — awake ended (held {}f); arrival snap OK",
            s_openingHoldFrames);
        // №254: completion fallback if the demo ended before frame 0xC8 (skip).
        dExtWw_markOpeningDone("awake ended");
        // №176: №172 Stop()'d the camera for the pan; №110 was often disarmed when
        // the arrival G-guard finished before ORDER. Re-arm the behind-Link snap
        // explicitly — History §52 / owe / box-state probes untouched above.
        dExtNpcDoors_requestPostOpeningSnap(stage);
        dExtWw_clearOpeningCameraState();
        return;
    }
    // Ordered but never went live — abandon so №110 can still clear map-edge framing.
    constexpr int kHoldAbandonFrames = 300;  // ~5s @60
    if (s_openingHoldFrames >= kHoldAbandonFrames) {
        DuskLog.warn(
            "[ExtWw] №170 opening camera hold ABANDON — ordered but no demo after {}f",
            s_openingHoldFrames);
        dExtWw_clearOpeningCameraState();
    }
}

// §49/№165: donor storyboard MESSAGE ids.
//
// A donor storyboard's JMSG track fires DONOR message indices — decoded from
// awake.stb as 0x357, 0x358, 0x050, 0x359, 0x35A... on a timed schedule (waits
// of 535/285/430/554 frames between them). Those indices mean nothing in the
// receiver's message table: index 855 lands on an Ordon pumpkin line and 80 on
// a quest-log string. The track is working correctly against the wrong table.
//
// №31 APPLIED TO TEXT: a missing line is preferable to a foreign one. Showing
// the receiver's own dialogue in a donor scene is exactly the kind of bleed the
// space-purity law exists to prevent, so an unmapped id is SUPPRESSED, never
// passed through.
//
// R6: any replacement line is player-visible donor text and must come from
// DATA. It is read from `<mod>/dialogue/demo_messages.ini` (`id = line`), never
// from a string in this file. Rendering those lines is NOT wired yet — the
// folder-side dialogue path is bound to actor-talk flow, not to demo timing —
// so for now a mapped line is logged rather than drawn. That is deliberately
// short of the goal and is recorded as such.
namespace {
std::unordered_map<u32, std::string> s_demoMsgLines;
bool s_demoMsgLoaded = false;

void loadDemoMessageMap() {
    if (s_demoMsgLoaded) {
        return;
    }
    s_demoMsgLoaded = true;
    // Folder comes from the manifest store rather than a hardcoded name, so the
    // mod folder stays renameable.
    const char* mod = NULL;
    for (const auto& kv : s_providers) {
        if (kv.second.modFolder[0] != '\0') {
            mod = kv.second.modFolder;
            break;
        }
    }
    if (mod == NULL) {
        return;
    }
    const fs::path file =
        dusk::ConfigPath / "model_replacements" / mod / "dialogue" / "demo_messages.ini";
    std::error_code ec;
    if (!fs::is_regular_file(file, ec)) {
        return;
    }
    std::ifstream in(file);
    std::string line;
    while (std::getline(in, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
            line.pop_back();
        }
        const size_t hash = line.find_first_not_of(" \t");
        if (hash == std::string::npos || line[hash] == '#' || line[hash] == ';') {
            continue;
        }
        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        // Strip a trailing " #" comment. The seeded file annotates every id with
        // its frame wait, and without this the annotation would be DRAWN as part
        // of the line. Requires the space so a '#' inside real text survives.
        const size_t cmt = val.find(" #");
        if (cmt != std::string::npos) {
            val.erase(cmt);
        }
        const auto trim = [](std::string& s) {
            while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.erase(s.begin());
            while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.pop_back();
        };
        trim(key);
        trim(val);
        if (key.empty()) {
            continue;
        }
        const u32 id = (u32)std::strtoul(key.c_str(), NULL, 0);  // accepts 855 and 0x357
        s_demoMsgLines[id] = val;
    }
    DuskLog.info("[ExtWw] §49 demo message map loaded — {} line(s)", (int)s_demoMsgLines.size());
}
}  // namespace

// §49 RENDERING — the storyboard is TIMED, so the box must never block it.
//
// The JMSG track fires on explicit frame waits (535, 285, 430, 554, 111, 60,
// 120, 30, 90 — decoded №165). Some gaps are half a second. A box that waits
// for an A-press before the demo may continue would desynchronise the scene
// from its own camera cuts, so this deliberately does NOT gate the demo:
//
//   * a new line REPLACES the one on screen (that is what the short waits mean)
//   * A dismisses early, which is native-feeling but never required
//   * the line self-clears when the demo ends, so nothing outlives the scene
//
// Reuses `dALBWDialogue_c`, the same widget the mount talk path and the shop
// use — not a second text system.

/** №171: honour the native message↔storyboard suspend contract (UI or suppress). */
static void dExtWw_resumeDemoAfterMessage(const char* why) {
    if (dDemo_c::getControl() == NULL || !dDemo_c::getControl()->isSuspended()) {
        return;
    }
    DuskLog.info("[ExtWw] §51 {} — resuming storyboard (suspend {} -> {})", why,
                 (int)dDemo_c::getControl()->getSuspend(),
                 (int)dDemo_c::getControl()->getSuspend() - 1);
    dDemo_c::getControl()->unsuspend(1);
}

/**
 * №171: the STB may suspend *after* the MESSAGE op in the same track. An
 * immediate unsuspend in the adaptor can no-op (not yet suspended), then the
 * suspend lands and freezes forever. Debounce through the poll instead.
 */
static bool s_demoOwesResume = false;
static const char* s_demoOwesResumeWhy = NULL;

static void dExtWw_oweDemoResume(const char* why) {
    s_demoOwesResume = true;
    s_demoOwesResumeWhy = why;
}

static void dExtWw_pollOwedDemoResume() {
    if (!s_demoOwesResume) {
        return;
    }
    if (dDemo_c::getControl() == NULL || !dDemo_c::getControl()->isSuspended()) {
        return;  // wait until the STB's suspend lands
    }
    dExtWw_resumeDemoAfterMessage(s_demoOwesResumeWhy != NULL ? s_demoOwesResumeWhy
                                                              : "owed resume");
    s_demoOwesResume = false;
    s_demoOwesResumeWhy = NULL;
}

// №171 safety: if a mapped line never draws (UI not ready) or A is stolen by
// doors, frameNoMsg stalls with suspend==1 forever. Don't touch History's §50
// probe — just clear the debt after a short stuck window.
static void dExtWw_pollStuckMessageResume() {
    static u32 s_stuckAt = 0;
    static int s_stuckFrames = 0;
    if (dComIfGp_event_runCheck() == 0 || dDemo_c::getControl() == NULL ||
        !dDemo_c::getControl()->isSuspended()) {
        s_stuckAt = 0;
        s_stuckFrames = 0;
        return;
    }
    if (dDemo_c::getControl()->getSuspend() != 1) {
        s_stuckAt = 0;
        s_stuckFrames = 0;
        return;  // magnitude 100 = PLAY-finished; not our debt
    }
    const u32 fnm = dDemo_c::getFrameNoMsg();
    if (fnm == s_stuckAt) {
        ++s_stuckFrames;
    } else {
        s_stuckAt = fnm;
        s_stuckFrames = 0;
    }
    if (s_stuckFrames >= 90) {
        dExtWw_oweDemoResume("stuck frameNoMsg (suspend 1)");
        s_stuckFrames = 0;
    }
}

#if TARGET_PC_NATIVE_UI
namespace {
dALBWDialogue_c* s_demoDialogue = NULL;
std::string s_demoPendingLine;
bool s_demoLineShown = false;
int s_demoLineShownFrames = 0;  // №171/№168: auto-resume if A never comes
}  // namespace
#endif

void dExtWw_drawDemoMessage() {
#if TARGET_PC_NATIVE_UI
    // MUST be called from the DRAW pass. `registerDraw` ends in
    // `dComIfGd_set2DOpa`, and d_s_play records that queuing from Execute is
    // wiped every frame by BeforeOfDraw and never painted.
    if (s_demoDialogue != NULL && s_demoDialogue->isVisible()) {
        s_demoDialogue->registerDraw();
    }
#endif
}

void dExtWw_pollDemoMessage() {
    dExtWw_pollOpeningCameraHold();  // №170: release snap/G-guard hold after awake
    dExtWw_pollStuckMessageResume(); // №171: UI-miss / stolen-A safety (keeps §50 probe intact)
    dExtWw_pollOwedDemoResume();     // №171: pay suspend debt once STB has suspended
    // №169 PROBE: the opening ran ~535 storyboard frames, fired its first message,
    // then STOPPED — no further JMSG fires, and player control returned. The
    // G-guard is exonerated by log order (it force-ended the event that was
    // BLOCKING us, at 6155, before our order succeeded at 6160).
    //
    // "It stopped" and "it ended normally" look identical from outside, so this
    // reports the demo frame as it advances and, critically, the LAST frame
    // reached when it stops. A demo that dies at a fixed frame every run is a
    // different fault from one that dies at a varying frame.
    {
        static bool s_wasRunning = false;
        static u32 s_lastFrame = 0;
        const bool running = dComIfGp_event_runCheck() != 0;
        const u32 frame = dDemo_c::getFrame();
        if (running) {
            if (!s_wasRunning) {
                DuskLog.info("[ExtWw] §50 demo START (frame {})", (int)frame);
            } else if (frame != s_lastFrame && (frame % 120) == 0) {
                // №170: `m_frame` and `m_frameNoMsg` are DIFFERENT clocks, and the
                // difference is the whole question. `m_frame` ticks on every
                // forward(); `m_frameNoMsg` only advances while
                // `getSuspend() <= 0` (d_demo.cpp:1160-1165), and the WAIT cut
                // times off THAT one (d_event_data.cpp:807). A suspended
                // storyboard still raises the number the first probe printed,
                // which is exactly why it could not tell a running demo from a
                // frozen one.
                //
                // gap == 0 -> storyboard time is live; a missing message is the
                //             MESSAGE TRACK, not time.
                // gap  > 0 -> something suspended it, and when it starts
                //             diverging says what.
                // №171: the MAGNITUDE decides the fix. The message system resumes
                // with `unsuspend(1)` (d_msg_object.cpp:1412/1903); the PLAY cut
                // instead uses `suspend(100)` when the demo has already finished
                // (d_event_data.cpp:1311). A frozen clock looks the same either
                // way, but 1 means "waiting on a message we suppressed" and 100
                // means "the storyboard already ended" — opposite problems.
                DuskLog.info("[ExtWw] §50 demo running — frame {} frameNoMsg {} (gap {}) "
                             "suspend {}",
                             (int)frame, (int)dDemo_c::getFrameNoMsg(),
                             (int)(frame - dDemo_c::getFrameNoMsg()),
                             dDemo_c::getControl() != NULL
                                 ? (int)dDemo_c::getControl()->getSuspend()
                                 : -999);
            }
            s_lastFrame = frame;
        } else if (s_wasRunning) {
            DuskLog.info(
                "[ExtWw] §50 demo ENDED — last storyboard frame {}. The camera track alone "
                "runs 1129+ frames (№165), so an end far below that is a TRUNCATION, not a "
                "natural finish.",
                (int)s_lastFrame);
        }
        s_wasRunning = running;
    }
#if TARGET_PC_NATIVE_UI
    if (s_demoDialogue == NULL) {
        return;
    }
    // The scene ended — never let a line outlive the demo that scheduled it.
    if (!dComIfGp_event_runCheck() && s_demoDialogue->isVisible()) {
        s_demoDialogue->hide();
        s_demoLineShown = false;
        s_demoLineShownFrames = 0;
        s_demoPendingLine.clear();
        // If we still owe a resume, pay it — otherwise a late abort leaves the
        // next demo permanently suspended.
        dExtWw_resumeDemoAfterMessage("demo ended with line up");
        return;
    }
    if (!s_demoPendingLine.empty() && !s_demoLineShown) {
        s_demoDialogue->tryCreate();
        if (s_demoDialogue->isReady()) {
            if (s_demoDialogue->isVisible()) {
                // Replace: close the previous line's suspend before opening the next.
                s_demoDialogue->hide();
                dExtWw_resumeDemoAfterMessage("line replaced");
            }
            // §65 H4 probe — is the DEMO caption renderer showing text OUTSIDE
            // a demo (i.e. intercepting mount talk)? Logs every show.
            DuskLog.warn("[ExtNpcMount] §65 demo-caption SHOW: '{:.24}'", s_demoPendingLine);
            s_demoDialogue->showWithText(s_demoPendingLine.c_str());
            s_demoLineShown = true;
            s_demoLineShownFrames = 0;
        }
    }
    if (s_demoDialogue->isVisible() && s_demoLineShown) {
        ++s_demoLineShownFrames;
    }
    // №172: box-state trace. Log 040928 showed the storyboard frozen at suspend 1
    // with NO resume line from either path — so we could not tell whether the box
    // was still up (auto-resume should have fired), already hidden (dismiss fired
    // and swallowed the debt), or never shown at all. Those need different fixes,
    // and silence looks identical for all three.
    if (dComIfGp_event_runCheck() && (s_demoLineShownFrames % 60) == 0 &&
        (s_demoDialogue->isVisible() || s_demoLineShown || !s_demoPendingLine.empty())) {
        DuskLog.info("[ExtWw] §51 box state — visible={} shown={} frames={} pending='{}' "
                     "owes={} suspend={}",
                     s_demoDialogue->isVisible() ? 1 : 0, s_demoLineShown ? 1 : 0,
                     (int)s_demoLineShownFrames, s_demoPendingLine.c_str(),
                     s_demoOwesResume ? 1 : 0,
                     dDemo_c::getControl() != NULL
                         ? (int)dDemo_c::getControl()->getSuspend()
                         : -999);
    }
    if (s_demoDialogue->isVisible() && s_demoDialogue->checkDismiss()) {
        s_demoDialogue->hide();  // early dismiss: allowed, never required
        s_demoLineShown = false;
        s_demoLineShownFrames = 0;
        s_demoPendingLine.clear();
        // №171: RESUME THE STORYBOARD — this is the contract I broke in №166.
        //
        // The engine SUSPENDS the storyboard when a demo message goes up, and
        // the message system resumes it on close with exactly this call
        // (d_msg_object.cpp:1412 and :1903). By suppressing the native box I
        // removed the resume without removing the suspend, so `m_frameNoMsg`
        // froze at 640 and never moved again: the scene ran on visually while
        // its own clock stood still, and 11 of 12 messages never fired.
        //
        // Taking over the display means taking over the whole contract, not
        // just the visible half. Guarded so we only ever resume a storyboard
        // that is actually suspended — never push the counter negative and
        // break a demo that was legitimately paused for something else.
        // №172: OWE the resume, never pay-or-forget.
        //
        // This branch used to call resumeDemoAfterMessage() and then clear the
        // debt. But that call silently returns when `!isSuspended()`, and the
        // STB may suspend AFTER the MESSAGE op in the same track — the very race
        // the owe mechanism exists for. If checkDismiss() fires on the same frame
        // the box opens (page-build yields no text, or a stale A trigger is
        // pending), we hid the box, no-op'd the resume, and then cleared the
        // debt. The suspend landed a frame later with nobody owing a resume, and
        // the storyboard froze at suspend 1 forever — which is exactly what
        // log 040928 shows: box up, clock stuck at 640, no resume line at all.
        //
        // Owing is idempotent and the poll only pays once actually suspended, so
        // this can never over-resume. Debt is cleared by PAYMENT, never by having
        // attempted it.
        DuskLog.info("[ExtWw] §51 dialogue dismissed (input) after {} frames — owing resume",
                     (int)s_demoLineShownFrames);
        dExtWw_oweDemoResume("dialogue dismissed");
    } else if (s_demoDialogue->isVisible() && s_demoLineShown) {
        // №168: A is never required. Log 040047 shows the player mashing door
        // A while our box was up — checkDismiss never fired, clock stayed
        // frozen, camera pan fought the free player cam. Auto-resume after a
        // short readable beat so the storyboard (and its camera) continue.
        // Debounce via owe so we don't race the STB's suspend landing.
        constexpr int kAutoResumeFrames = 90;  // ~1.5s @60
        if (s_demoLineShownFrames >= kAutoResumeFrames) {
            s_demoDialogue->hide();
            s_demoLineShown = false;
            s_demoLineShownFrames = 0;
            s_demoPendingLine.clear();
            dExtWw_oweDemoResume("auto-resume (A never required)");
        }
    }
#endif
}

bool dExtWw_handleDemoMessage(u32 donorMsgId) {
    if (!dExtWwSave_isWwHostStage(dComIfGp_getStartStageName())) {
        return false;  // receiver scenes: native behaviour, untouched
    }
    // §87: donor message-tied CharVoice — SE 0x481F + port clip from package map.
    // 0x359 sound=0 stays silent by donor design. Map rides voice/manifest.ini.
    ExtSeq::ja1Voice_onDemoMessageOpen(donorMsgId);
    loadDemoMessageMap();
    const auto it = s_demoMsgLines.find(donorMsgId);
    if (it != s_demoMsgLines.end() && !it->second.empty()) {
        DuskLog.info("[ExtWw] §49 demo message {} (0x{:X}) -> \"{}\"", (int)donorMsgId,
                     (int)donorMsgId, it->second);
#if TARGET_PC_NATIVE_UI
        if (s_demoDialogue == NULL) {
            s_demoDialogue = JKR_NEW dALBWDialogue_c();
        }
        if (s_demoDialogue != NULL) {
            s_demoPendingLine = it->second;
            s_demoLineShown = false;  // the poll shows it next frame, in phase
        }
#endif
    } else {
        // Mapped-but-blank counts as unmapped: the seeded file ships every id
        // with an empty value so the ids are visible for authoring.
        DuskLog.info(
            "[ExtWw] §49 demo message {} (0x{:X}) suppressed — no line authored. The "
            "receiver's string at this index is unrelated text and must not show (№31).",
            (int)donorMsgId, (int)donorMsgId);
        // №171: suppression still owes the resume — the STB suspends for this
        // paragraph. No box means nothing will dismiss; debounce via poll so we
        // unsuspend after the suspend actually lands (not before).
        dExtWw_oweDemoResume("suppressed (no authored line)");
    }
    return true;  // suppress the native display either way
}

// §47: the donor cast's lighting recipe, applied to the PLAYER in donor spaces.
//
// Derived from this project's own tuned values rather than picked by eye:
//   * donor NPCs / BG render at a FIXED (90,90,90) with NO MAJI (manifest
//     amb=5a5a5a, applyModelAmbient)
//   * donor ITEMS render at a fixed warm (105,78,48), also no MAJI
//   * the player renders through MAJI on env ambient, measured (36,24,59)
//
// Two things follow. The item warm is WRONG for him — the mount already records
// that 105/78/48 "was the №14 orange bloom on skin/cloth", and the player is
// skin and cloth. And a brightness multiplier was the wrong lever entirely: his
// ambient is already LOWER than the cast's while he reads far brighter, so the
// light is coming from the MAJI path the cast never runs.
//
// Hence: pin his ambient to the cast's neutral 90 and skip MAJI, which is the
// same recipe the codebase already uses to make a model sit in the donor look.
bool dExtWw_applyPlayerDonorLook(J3DModel* model, dKy_tevstr_c* tevStr) {
    if (model == NULL || tevStr == NULL) {
        return false;
    }
    if (!dusk::getSettings().game.wwPlayerDonorLook.getValue()) {
        return false;  // default: untouched
    }
    if (!dExtWwSave_isWwHostStage(dComIfGp_getStartStageName())) {
        return false;  // receiver areas: never reached
    }
    // №162: pinning ambient alone only got HALFWAY, and the reason is visible in
    // the cast's own draw (see the mount's execute path). The cast does TWO things:
    //
    //     g_env_light.settingTevStruct(0, &pos, &tevStr);   // <-- repopulate ALL of it
    //     ... override AmbCol ...
    //
    // `settingTevStruct` rewrites the WHOLE tevStr at light-type 0 — light colours,
    // Color0/K0, fog — not just ambient. The first cut of this function skipped it
    // and pinned AmbCol onto a tevStr the ALINK draw had already filled with the
    // PLAYER's much hotter light-type values. So his ambient matched the cast while
    // his diffuse did not: matched on one channel, unchanged on the rest. That is
    // precisely "it did something, but only halfway".
    //
    // Running the same call the cast runs is what closes the gap. It is the cast's
    // recipe applied whole, rather than a brightness number picked to compensate.
    {
        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player != NULL) {
            g_env_light.settingTevStruct(0, &player->current.pos, tevStr);
        }
    }

    GXColor amb;
    amb.r = 90;  // 0x5a — the cast's own manifest value
    amb.g = 90;
    amb.b = 90;
    amb.a = 255;
    tevStr->AmbCol.r = amb.r;
    tevStr->AmbCol.g = amb.g;
    tevStr->AmbCol.b = amb.b;
    tevStr->AmbCol.a = amb.a;
    applyModelAmbient(model, amb);
    return true;  // caller SKIPS MAJI
}

void dExtNpcMount_onStageReady() {
    // №94: every play-scene Create tears down actors — bump so spawn latches re-arm.
    const char* stage = dComIfGp_getStartStageName();
    dExtWw_wireFadoCrossing(stage);
    // Re-arm on every stage change; the poll above does the actual ordering
    // once the player exists.
    if (stage != NULL && std::strcmp(stage, "F_DL01") == 0) {
        s_openingWaitFrames = 0;
        if (dExtWw_openingAlreadyDone()) {
            // №254: re-entry after interior must NOT re-arm the order gate.
            s_openingOrdered = true;
            dExtWw_clearOpeningCameraState();
            DuskLog.info(
                "[ExtWw] №254 F_DL01 re-entry — '{}' set, opening will not re-order",
                kOpeningDoneFlag);
        } else {
            // №170: pending defers snap only. Do NOT arm G-guard hold here — that
            // blocked residual clear and left awake deferred forever (log 035311).
            s_openingOrdered = false;
            s_openingPending = true;
            s_openingCameraHold = false;
            s_openingSawDemo = false;
            s_openingHoldFrames = 0;
            DuskLog.info("[ExtWw] №170 opening snap-pending on F_DL01 (G-guard still free)");
        }
    } else if (stage != NULL) {
        s_openingOrdered = false;
        s_openingWaitFrames = 0;
        dExtWw_clearOpeningCameraState();
    }
    char reason[48];
    if (stage != NULL && s_worldGenStage[0] != '\0' &&
        std::strcmp(stage, s_worldGenStage) == 0) {
        std::snprintf(reason, sizeof(reason), "recreate '%s'", stage);
    } else {
        std::snprintf(reason, sizeof(reason), "enter '%s'", stage != NULL ? stage : "?");
    }
    dExtNpcWorld_bump(reason);
    if (stage != NULL) {
        std::snprintf(s_worldGenStage, sizeof(s_worldGenStage), "%s", stage);
    } else {
        s_worldGenStage[0] = '\0';
    }
    // №83: re-bind room-lane procs for the stage we just entered (R_DL01 etc.).
    syncRoomLaneForCurrentStage();
    // §97b/§101: arm shore foam on WW field hosts; load package calm map.
    // flatInter polarity (donor pair): 0=calm/usonami ON, 1=chop/usonami OFF.
    if (stage != NULL && stage[0] == 'F' && dExtWwSave_isWwHostStage(stage)) {
        dKy_usonami_set(0.0f);
        dKyw_wave_calm_onStage(stage);
        DuskLog.info("[WwFoam] §97b/§101 usonami armed on '{}'", stage);
    } else {
        g_env_light.mWaveChan.mWaveCount = 0;
        g_env_light.mWaveChan.mWaveFlatInter = 1.0f;
        dKyw_wave_calm_onStage(NULL);
    }
    // №84: remount exterior after native stage change; release door demo lock.
    tryStageExitRemount();
    dExtNpcMount_endDoorDemoLock();
    // №90: warp (and any WW-host entry without a door arm) gets a lane-agnostic G-guard.
    if (dExtWwSave_isWwHostStage(stage)) {
        dExtNpcDoors_armArrivalGuard(stage);
    }
}

static void placeLinkAt(fopAc_ac_c* player, const cXyz& spawn) {
    player->current.pos = spawn;
    player->old.pos = spawn;
    player->home.pos = spawn;
    player->speed.x = 0.0f;
    player->speed.y = 0.0f;
    player->speed.z = 0.0f;
    player->speedF = 0.0f;
}

// №54-4: probe from refY+50 DOWN; accept only within ±250 of refY (rejects roofs/sea floor).
bool dExtNpcMount_localGroundSnap(cXyz* pos, f32 refY) {
    if (pos == NULL) {
        return false;
    }
    cXyz probe(pos->x, refY + 50.0f, pos->z);
    if (!fopAcM_gc_c::gndCheck(&probe)) {
        DuskLog.warn("[Doors] local probe miss at ({:.1f},{:.1f},{:.1f}) refY={:.1f}", probe.x,
                     probe.y, probe.z, refY);
        pos->y = refY + 50.0f;
        return false;
    }
    const f32 gy = fopAcM_gc_c::getGroundY() + 50.0f;
    if (gy > refY + 250.0f || gy < refY - 250.0f) {
        DuskLog.warn(
            "[Doors] local probe y={:.1f} outside ±250 of refY={:.1f} — keep authored", gy, refY);
        pos->y = refY + 50.0f;
        return false;
    }
    pos->y = gy;
    DuskLog.info("[ExtNpcMount] BG ground hit y={} probe=({:.1f},{:.1f},{:.1f}) local", pos->y,
                 probe.x, probe.y, probe.z);
    return true;
}

// №21 probes — classify draw vs dBgW vs cBgS routing (kept for BG warps).
static void logBgWarpProbes(dExtNpcMount_c* mount, const cXyz& hostVillage) {
    if (mount == NULL || mount->mpBgW == NULL) {
        DuskLog.warn("[ExtNpcMount:BG] probe aborted — mount/bgw null");
        return;
    }
    dBgW* bgw = mount->mpBgW;
    // №22: local models use cell-local village; world-baked path used absolute GS vtx.
    const cXyz probeVtx = mount->mManifest.modelSpaceLocal ?
                              cXyz(-1651.0f, 163.0f, 11989.0f) :
                              cXyz(-201651.0f, 163.0f, 311989.0f);
    const f32 expectX = hostVillage.x;
    const f32 expectY = hostVillage.y - 20.0f;  // spawn_rel has +20 clearance
    const f32 expectZ = hostVillage.z;

    // P0 / P0b — draw pipeline
    if (mount->mpBgModels[0] != NULL) {
        MtxP base = mount->mpBgModels[0]->getBaseTRMtx();
        cXyz drawn;
        cMtx_multVec(base, &probeVtx, &drawn);
        DuskLog.info(
            "[ExtNpcMount:P0] village vtx via model[0] baseTRMtx → ({}, {}, {}) "
            "expect≈({},{},{}) model_space={}",
            drawn.x, drawn.y, drawn.z, expectX, expectY, expectZ,
            mount->mManifest.modelSpaceLocal ? "local" : "world");
        DuskLog.info("[ExtNpcMount:P0b] model[0] baseTRMtx T=({}, {}, {})", base[0][3], base[1][3],
                     base[2][3]);
    } else {
        DuskLog.warn("[ExtNpcMount:P0] model[0] NULL");
    }
    DuskLog.info(
        "[ExtNpcMount:P0b] mBgMtx T=({}, {}, {}) pm_base={} same_ptr={} lock={} nocalc={} flags={:02x} "
        "moveCtr={} model_space={}",
        mount->mBgMtx[0][3], mount->mBgMtx[1][3], mount->mBgMtx[2][3], (void*)bgw->pm_base,
        bgw->pm_base == &mount->mBgMtx, bgw->ChkLock(), (int)bgw->ChkNoCalcVtx(), bgw->mFlags,
        bgw->mMoveCounter, mount->mManifest.modelSpaceLocal ? "local" : "world");

    // P1 — collision tree AABB location
    if (bgw->pm_grp != NULL) {
        const u16 root = bgw->m_rootGrpIdx;
        const cM3dGAab& aab = bgw->pm_grp[root].m_aab;
        DuskLog.info(
            "[ExtNpcMount:P1] rootGrp={} aab min=({}, {}, {}) max=({}, {}, {}) "
            "(host≈-60k/-90k, GS≈-200k/+315k)",
            root, aab.GetMinX(), aab.GetMinY(), aab.GetMinZ(), aab.GetMaxX(), aab.GetMaxY(),
            aab.GetMaxZ());
    } else {
        DuskLog.warn("[ExtNpcMount:P1] pm_grp NULL");
    }

    // P2 — direct dBgW GroundCross (bypass cBgS list)
    cXyz probe(hostVillage.x, hostVillage.y + 2000.0f, hostVillage.z);
    dBgS_ObjGndChk chk;
    chk.SetPos(&probe);
    chk.SetNowY(-G_CM3D_F_INF);
    const bool p2 = bgw->GroundCross(&chk);
    DuskLog.info("[ExtNpcMount:P2] direct GroundCross hit={} y={} probe=({}, {}, {})", p2,
                 chk.GetNowY(), probe.x, probe.y, probe.z);

    // P3 — same with PolyPassChk cleared (ti through-filter test)
    dBgS_ObjGndChk chk3;
    chk3.SetPos(&probe);
    chk3.SetNowY(-G_CM3D_F_INF);
    chk3.SetPolyPassChk(NULL);
    const bool p3 = bgw->GroundCross(&chk3);
    DuskLog.info("[ExtNpcMount:P3] GroundCross PolyPassChk=NULL hit={} y={}", p3, chk3.GetNowY());

    if (!p3 && bgw->pm_blk != NULL && bgw->pm_bgd != NULL) {
        int withGnd = 0;
        int emptyGnd = 0;
        const int bnum = bgw->pm_bgd->m_b_num;
        for (int i = 0; i < bnum; ++i) {
            if (bgw->pm_blk[i].m_gnd_idx != 0xFFFF) {
                ++withGnd;
            } else {
                ++emptyGnd;
            }
        }
        DuskLog.warn(
            "[ExtNpcMount:P3miss] blocks={} with_gnd={} empty_gnd={} vtx_num={} tri_num={} "
            "vtx_tbl={}",
            bnum, withGnd, emptyGnd, bgw->GetVtxNum(), (int)bgw->pm_bgd->m_t_num,
            (void*)bgw->GetVtxTbl());
    }
}

static void tryInteriorBootstrap() {
    if (s_interiorBootstrapProc[0] == '\0') {
        return;
    }
    if (dComIfGp_getPlayer(0) == NULL) {
        return;
    }
    dExtNpcManifest man{};
    if (!dExtNpcMount_lookup(s_interiorBootstrapProc, &man) || !man.isBg) {
        s_interiorBootstrapProc[0] = '\0';
        return;
    }
    const char* stage = dComIfGp_getStartStageName();
    if (man.hostStage[0] != '\0' &&
        (stage == NULL || std::strcmp(stage, man.hostStage) != 0)) {
        return;  // not on host yet
    }
    std::snprintf(s_lastBgProc, sizeof(s_lastBgProc), "%s", s_interiorBootstrapProc);
    // №104: activate host room before pop (rooms != 0 have no daBg to set 0x10).
    if (man.hostRoom >= 0 && man.hostRoom < 0x40 &&
        (dExtWwSave_isWwHostStage(stage) || dExtNpcMount_isRoomLaneProc(s_interiorBootstrapProc))) {
        activateWwHostRoom(man.hostRoom, "bootstrap");
        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player != NULL) {
            roomTxAssignPlayerRoom(player, man.hostRoom);
        }
    }
    dExtNpcPopulation_spawnForBg(man);
    if (std::strcmp(s_interiorBootstrapProc, "EXT_BG0") == 0) {
        dExtNpcDoors_spawnKnobs(man);
        DuskLog.info(
            "[ExtNpcMount] №104 exterior bootstrap '{}' stage='{}' — population + outdoor knobs",
            s_interiorBootstrapProc, stage != NULL ? stage : "?");
    } else {
        dExtNpcDoors_onInteriorBgReady(s_interiorBootstrapProc);
        DuskLog.info(
            "[ExtNpcMount] №90 interior bootstrap '{}' stage='{}' — population + exit knob",
            s_interiorBootstrapProc, stage != NULL ? stage : "?");
    }
    s_interiorBootstrapProc[0] = '\0';
}

// №94/№104: empty world after soft reload / exterior re-entry ⇒ re-run + log.
static void trySpawnSelfHeal() {
    if (dComIfGp_getPlayer(0) == NULL || s_lastBgProc[0] == '\0') {
        return;
    }
    dExtNpcManifest man{};
    if (!dExtNpcMount_lookup(s_lastBgProc, &man) || !man.isBg) {
        return;
    }
    if (std::strcmp(s_lastBgProc, "EXT_BG0") == 0) {
        const int want = dExtNpcDoors_wantOutdoorKnobCount();
        const int live = dExtNpcDoors_countLiveOutdoorKnobs();
        if (want > 0 && live == 0) {
            DuskLog.warn(
                "[ExtNpcMount] №94 self-heal doors — '{}' want={} live=0 latched={} → respawn",
                s_lastBgProc, want, dExtNpcDoors_knobsLatched() ? 1 : 0);
            dExtNpcDoors_clearSpawnLatches();
            dExtNpcDoors_spawnKnobs(man);
        }
    }
    if (man.populationCsv[0] == '\0') {
        return;
    }
    if (dExtNpcPopulation_countLiveCensus() != 0) {
        return;
    }
    // №104: post-bump exterior often never latched (spawn skipped) — still re-run.
    if (!dExtNpcPopulation_isLatched(s_lastBgProc)) {
        DuskLog.warn(
            "[ExtNpcMount] №104 self-heal census — unlatched '{}' live=0 → spawn (gen mismatch)",
            s_lastBgProc);
        dExtNpcPopulation_spawnForBg(man);
        return;
    }
    DuskLog.warn(
        "[ExtNpcMount] №94 self-heal census — COMPLEATE '{}' live=0 → re-run population",
        s_lastBgProc);
    dExtNpcPopulation_clearForBg(s_lastBgProc);
    dExtNpcPopulation_spawnForBg(man);
}

void dExtNpcMount_pollBgWarps() {
    // №83: drain deferred room-lane creates (room ready before play-scene/layer).
    pollPendingRoomLaneMount();
    // №84: exterior remount may arm before player exists in onStageReady.
    tryStageExitRemount();
    // №155: the opening needs the player to exist, which is not true at
    // stage-ready. Poll for it here until it lands (self-latching).
    dExtWw_orderOpening(dComIfGp_getStartStageName());
    // №90: population + exit knob for WW-host BG mounts (COMPLEATE may precede player).
    tryInteriorBootstrap();
    // №94: empty-world net — latched but no live actors after soft reload.
    trySpawnSelfHeal();
    if (s_roomTxPhase != kRoomTxIdle) {
        pollRoomLaneTransport();
        return;
    }
    if (s_bgWarpPhase == kBgWarpIdle) {
        pollWarmInteriors();
        return;
    }

    dExtNpcManifest man{};
    if (!dExtNpcMount_lookup(s_bgWarpProc, &man) || !man.isBg || !man.hasHostPos) {
        DuskLog.warn("[ExtNpcMount] BG warp aborted — payload '{}' missing/incomplete", s_bgWarpProc);
        s_bgWarpPhase = kBgWarpIdle;
        s_bgWarpPending = false;
        dExtNpcMount_endDoorDemoLock();
        return;
    }
    s_bgHostPos = man.hostPos;

    if (s_bgWarpPhase == kBgWarpWaitPlayer) {
        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player == NULL) {
            return;
        }
        const char* stage = dComIfGp_getStartStageName();
        if (stage == NULL || strcmp(stage, man.hostStage) != 0) {
            return;
        }

        // Reuse an existing BG mount for this proc (door house↔island).
        // №25 F1: refuse reuse unless mManifest.proc still matches.
        // №58-B: if warm/cold create is still loading, WAIT — do not erase+recreate (mash).
        {
            auto it = s_bgMountIds.find(s_bgWarpProc);
            if (it != s_bgMountIds.end()) {
                fopAc_ac_c* existing = fopAcM_SearchByID(it->second);
                if (existing != NULL) {
                    dExtNpcMount_c* mount = (dExtNpcMount_c*)existing;
                    if (mount->mIsBg && strcmp(mount->mManifest.proc, s_bgWarpProc) == 0) {
                        s_bgIslandId = it->second;
                        if (mount->mBgReady && mount->mpBgW != NULL) {
                            s_bgSettleFrames = 4;
                            s_bgWarpPhase = kBgWarpWaitBg;
                            DuskLog.info("[ExtNpcMount] BG reuse '{}' id={:08x}", s_bgWarpProc,
                                         (u32)s_bgIslandId);
                        } else {
                            s_bgWarpPhase = kBgWarpWaitIsland;
                            DuskLog.info(
                                "[ExtNpcMount] BG wait-load '{}' id={:08x} (warm/cold in flight)",
                                s_bgWarpProc, (u32)s_bgIslandId);
                        }
                        return;
                    }
                    DuskLog.warn(
                        "[ExtNpcMount] BG reuse rejected '{}' — cached id={:08x} proc='{}' "
                        "ready={}",
                        s_bgWarpProc, (u32)it->second, mount->mManifest.proc,
                        mount->mIsBg && mount->mBgReady);
                }
                s_bgMountIds.erase(it);
            }
        }

        // №62: room-lane mounts are created by d_s_room — wait, do not self-create.
        if (s_roomLaneRooms.count(s_bgWarpProc) != 0) {
            const int hostRoom = s_roomLaneRooms[s_bgWarpProc];
            dExtNpcMount_ensureRoomLaneLoaded(hostRoom);
            // Room already live but mount missing (e.g. prior kill) — rebind once.
            if (dComIfGp_roomControl_checkStatusFlag(hostRoom, 0x01) &&
                s_bgMountIds.count(s_bgWarpProc) == 0) {
                dExtNpcMount_onRoomObjectsReady(dComIfGp_getStartStageName(), hostRoom);
            }
            ++s_bgReadyWaitFrames;
            if (s_bgFailSafeValid && s_bgReadyWaitFrames >= kBgReadyHardTimeout) {
                DuskLog.warn(
                    "[Doors] enter {} → ABORT reason=room_lane_timeout after {}f — return to "
                    "fail-safe",
                    s_bgWarpProc, s_bgReadyWaitFrames);
                const cXyz back = s_bgFailSafeSpawn;
                s_bgFailSafeValid = false;
                s_bgWarpPhase = kBgWarpIdle;
                s_bgWarpPending = false;
                dExtNpcMount_endDoorDemoLock();
                beginBgWarp("EXT_BG0", true, back);
                return;
            }
            if ((s_bgReadyWaitFrames % 30) == 1) {
                DuskLog.info("[ExtNpcMount] №62 wait room-lane mount '{}' room={} f={}",
                             s_bgWarpProc, hostRoom, s_bgReadyWaitFrames);
            }
            return;
        }

        s_bgIslandId = createBgMountAtHost(man, "door");
        if (s_bgIslandId == fpcM_ERROR_PROCESS_ID_e) {
            DuskLog.warn("[ExtNpcMount] BG island fopAcM_create FAILED for '{}'", s_bgWarpProc);
            s_bgWarpPhase = kBgWarpIdle;
            s_bgWarpPending = false;
            dExtNpcMount_endDoorDemoLock();
            return;
        }
        s_bgMountIds[s_bgWarpProc] = s_bgIslandId;
        DuskLog.info(
            "[ExtNpcMount] BG create '{}' id={:08x} host=({}, {}, {}) spawn_rel=({}, {}, {})",
            s_bgWarpProc, (u32)s_bgIslandId, s_bgHostPos.x, s_bgHostPos.y, s_bgHostPos.z,
            man.spawnRel.x, man.spawnRel.y, man.spawnRel.z);
        s_bgWarpPhase = kBgWarpWaitIsland;
        return;
    }

    if (s_bgWarpPhase == kBgWarpWaitIsland) {
        fopAc_ac_c* island = fopAcM_SearchByID(s_bgIslandId);
        dExtNpcMount_c* mount =
            island != NULL ? (dExtNpcMount_c*)island : NULL;
        const bool ready =
            mount != NULL && mount->mIsBg && mount->mBgReady && mount->mpBgW != NULL;
        if (!ready) {
            ++s_bgReadyWaitFrames;
            // №58-B: hold black fade while resLoad is honestly in-flight.
            {
                JUTFader* fader = JFWDisplay::getManager()->getFader();
                if (fader != NULL && fader->getStatus() != JUTFader::FadeOut &&
                    fader->getStatus() != JUTFader::Wait) {
                    mDoGph_gInf_c::startFadeOut(5);
                }
            }
            const bool gone = island == NULL && s_bgReadyWaitFrames >= kBgReadyGoneTimeout;
            const bool hardStuck =
                s_bgFailSafeValid && s_bgReadyWaitFrames >= kBgReadyHardTimeout;
            if (gone || hardStuck) {
                DuskLog.warn(
                    "[Doors] enter {} → ABORT reason={} after {}f — return to fail-safe",
                    s_bgWarpProc, gone ? "create_gone" : "no_compleate", s_bgReadyWaitFrames);
                mDoGph_gInf_c::startFadeOut(10);
                const cXyz back = s_bgFailSafeSpawn;
                s_bgFailSafeValid = false;
                s_bgWarpPhase = kBgWarpIdle;
                s_bgWarpPending = false;
                s_bgIslandId = fpcM_ERROR_PROCESS_ID_e;
                s_bgReadyWaitFrames = 0;
                dExtNpcMount_endDoorDemoLock();
                beginBgWarp("EXT_BG0", true, back);
                return;
            }
            return;
        }
        // Keep fail-safe armed until ground probe in WaitBg (№37).
        s_bgReadyWaitFrames = 0;
        s_bgSettleFrames = 2;
        s_bgWarpPhase = kBgWarpWaitBg;
        return;
    }

    if (s_bgWarpPhase == kBgWarpWaitBg) {
        fopAc_ac_c* island = fopAcM_SearchByID(s_bgIslandId);
        if (island != NULL) {
            dExtNpcMount_c* mount = (dExtNpcMount_c*)island;
            if (mount->mpBgW != NULL && !mount->mBgGlobal) {
                updateBgTransform(mount);
                mount->mpBgW->Move();
            }
        }
        if (--s_bgSettleFrames > 0) {
            return;
        }

        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player == NULL) {
            return;
        }

        cXyz spawn =
            s_bgSpawnOverrideValid ? s_bgSpawnOverride : (s_bgHostPos + man.spawnRel);
        if (island != NULL && !s_bgSpawnOverrideValid) {
            logBgWarpProbes((dExtNpcMount_c*)island, spawn);
        }

        // №54-4: local probe (refY+50), never sky+2000 — sky hits roofs on island returns.
        const f32 refY = spawn.y;
        if (dExtNpcMount_localGroundSnap(&spawn, refY)) {
            s_bgFailSafeValid = false;
        } else {
            DuskLog.warn("[Doors] enter {} → local ground miss/reject at ({:.1f},{:.1f},{:.1f})",
                         s_bgWarpProc, spawn.x, spawn.y, spawn.z);
            if (s_bgFailSafeValid && !s_bgSpawnOverrideValid) {
                // Interior enter failed — abort to outdoor fail-safe (already porch-snapped).
                DuskLog.warn(
                    "[Doors] enter {} → ABORT reason=no_ground — return to fail-safe",
                    s_bgWarpProc);
                mDoGph_gInf_c::startFadeOut(10);
                const cXyz back = s_bgFailSafeSpawn;
                s_bgFailSafeValid = false;
                s_bgWarpPhase = kBgWarpIdle;
                s_bgWarpPending = false;
                s_bgIslandId = fpcM_ERROR_PROCESS_ID_e;
                dExtNpcMount_endDoorDemoLock();
                beginBgWarp("EXT_BG0", true, back);
                return;
            }
            DuskLog.warn("[ExtNpcMount] BG NO ground — placing at authored y (unguarded)");
            spawn.y = refY + 50.0f;
        }

        placeLinkAt(player, spawn);
        forceLinkGroundReprobe(player);
        // №54-5: Link faces INTO the room (Nintendo PLYR spawn_ry) on interior enter.
        // №56: exit/return override may carry return_ry facing.
        if (s_bgSpawnOverrideValid && s_bgSpawnFacingValid) {
            player->current.angle.y = s_bgSpawnFacing;
            player->shape_angle.y = s_bgSpawnFacing;
        } else if (!s_bgSpawnOverrideValid && man.hasSpawnRy) {
            player->current.angle.y = man.spawnRy;
            player->shape_angle.y = man.spawnRy;
        }
        s_bgHoldPos = spawn;
        s_bgHoldFrames = s_bgSpawnOverrideValid ? 45 : 120;
        s_bgSpawnOverrideValid = false;
        s_bgSpawnFacingValid = false;
        DuskLog.info("[ExtNpcMount] Link placed on BG spawn ({}, {}, {})", spawn.x, spawn.y,
                     spawn.z);
        // №52-A: door lane fades out on A-press; menu lane already fades in — match it here.
        // JUTFader::startFadeIn only accepts status None (post-FadeOut black). If still mid
        // FadeOut, snap to None first so fade-in can start.
        {
            JUTFader* fader = JFWDisplay::getManager()->getFader();
            if (fader != NULL && fader->getStatus() == JUTFader::FadeOut) {
                fader->setStatus(JUTFader::None, 0);
            }
            if (!mDoGph_gInf_c::startFadeIn(15)) {
                DuskLog.warn("[Doors] fade-in refused after Link placed ({}) — retry next hold",
                             s_bgWarpProc);
            } else {
                DuskLog.info("[Doors] fade-in after Link placed ({})", s_bgWarpProc);
            }
        }
        // №25 F2: populate once BG is ready + Link seated (not at create-time of the BG).
        dExtNpcPopulation_spawnForBg(man);
        // №27 N6: door-knob visual props at TGDR positions (island only).
        if (std::strcmp(s_bgWarpProc, "EXT_BG0") == 0) {
            dExtNpcDoors_spawnKnobs(man);
        }
        s_bgFadeWatchdogFired = false;
        s_bgWarpPhase = kBgWarpHold;
        return;
    }

    if (s_bgWarpPhase == kBgWarpHold) {
        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player != NULL) {
            const f32 dx = player->current.pos.x - s_bgHoldPos.x;
            const f32 dz = player->current.pos.z - s_bgHoldPos.z;
            if ((dx * dx + dz * dz) > (5000.0f * 5000.0f)) {
                DuskLog.warn("[ExtNpcMount] Link drifted off BG — re-anchoring");
                placeLinkAt(player, s_bgHoldPos);
            } else {
                player->speed.x = 0.0f;
                player->speed.z = 0.0f;
                if (player->speed.y < -50.0f) {
                    player->speed.y = -50.0f;
                }
            }
        }
        // Keep island MOVE_BG ticking while actors retry ground-snap.
        {
            fopAc_ac_c* island = fopAcM_SearchByID(s_bgIslandId);
            if (island != NULL) {
                dExtNpcMount_c* mount = (dExtNpcMount_c*)island;
                if (mount->mpBgW != NULL && !mount->mBgGlobal) {
                    updateBgTransform(mount);
                    mount->mpBgW->Move();
                }
            }
        }
        // №53: one-shot stuck-fade watchdog (was spamming every Hold frame).
        if (!s_bgFadeWatchdogFired && s_bgHoldFrames <= 90) {
            JUTFader* fader = JFWDisplay::getManager()->getFader();
            if (fader != NULL &&
                (fader->getStatus() == JUTFader::None ||
                 fader->getStatus() == JUTFader::FadeOut)) {
                fader->setStatus(JUTFader::None, 0);
                if (mDoGph_gInf_c::startFadeIn(10)) {
                    s_bgFadeWatchdogFired = true;
                    DuskLog.warn("[Doors] stuck-fade watchdog — force fade-in ({})", s_bgWarpProc);
                }
            } else if (fader != NULL && fader->getStatus() == JUTFader::FadeIn) {
                s_bgFadeWatchdogFired = true;  // fade-in already running
            } else if (fader != NULL && fader->getStatus() == JUTFader::Wait) {
                s_bgFadeWatchdogFired = true;  // clear screen
            }
        }
        if (--s_bgHoldFrames <= 0) {
            std::snprintf(s_lastBgProc, sizeof(s_lastBgProc), "%s", s_bgWarpProc);
            s_bgWarpPhase = kBgWarpIdle;
            s_bgWarpPending = false;
            dExtNpcMount_endDoorDemoLock();
            DuskLog.info("[ExtNpcMount] BG hold complete — active '{}'", s_lastBgProc);
            // №58-B: warm interiors once the island is live (first press = reuse, no mash).
            if (std::strcmp(s_lastBgProc, "EXT_BG0") == 0 && !s_warmInteriors) {
                s_warmInteriors = true;
                s_warmProviderIndex = 0;
                s_warmCooldown = 0;
                DuskLog.info("[ExtNpcMount] №58-B warm interiors start");
            }
        }
    }
}

void dExtNpcMount_beginDoorDemoLock() {
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player == NULL) {
        return;
    }
    daAlink_c* link = (daAlink_c*)player;
    link->changeOriginalDemo();
    // №53-A: native door-open proc through the fade (param0=0 → left swing).
    link->changeDemoMode(daPy_demo_c::DEMO_DOOR_OPEN_e, 0, 0, 0);
    player->speedF = 0.0f;
    player->speed.x = 0.0f;
    player->speed.y = 0.0f;
    player->speed.z = 0.0f;
    s_doorDemoLocked = true;
}

void dExtNpcMount_endDoorDemoLock() {
    if (!s_doorDemoLocked) {
        return;
    }
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player != NULL) {
        ((daPy_py_c*)player)->cancelOriginalDemo();
    }
    s_doorDemoLocked = false;
}

void dExtNpcMount_forceEndDoorEvent(const char* reason) {
    // №170: never truncate awake after it has ordered. Residual + pre-order
    // G-guard must still clear the busy slot that blocks the order (№169).
    if (dExtWw_openingPauseArrivalGuard() &&
        (s_openingSawDemo ||
         (reason != NULL && std::strcmp(reason, "arrival-G-guard") == 0))) {
        DuskLog.info(
            "[ExtNpcMount] №170 refuse force-end ({}) — awake hold active",
            reason != NULL ? reason : "?");
        return;
    }
    dExtNpcMount_endDoorDemoLock();
    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player != NULL) {
        ((daPy_py_c*)player)->cancelOriginalDemo();
    }
    dEvt_control_c* ev = dComIfGp_getEvent();
    if (ev == NULL) {
        return;
    }
    const bool active = ev->runCheck() || ev->getMode() != dEvt_mode_WAIT_e;
    if (!active) {
        return;
    }
    dComIfGp_getEventManager().cancelStaff("ALL");
    dComIfGp_getEventManager().setCameraPlay(0);
    ev->remove();
    DuskLog.info("[ExtNpcMount] №89 force-end event ({}) — control released",
                 reason != NULL ? reason : "?");
}

int dExtNpcMount_create(dExtNpcMount_c* i_this, const char* procName) {
    if (i_this == NULL || procName == NULL || procName[0] == '\0') {
        return cPhs_ERROR_e;
    }

    // №52: create is re-entrant across resLoad. One-time init + ledger once; later phases
    // only continue the phase machine (wiping pointers/keys mid-load left prop='?' ).
    const bool firstPhase = !i_this->mManifest.valid;
    if (firstPhase) {
        if (!dExtNpcMount_lookup(procName, &i_this->mManifest)) {
            return cPhs_ERROR_e;
        }

        // №164: stamp the CENSUS IDENTITY here, at the mount, not at the spawn.
        //
        // №163 set `argument` in our own `fopAcM_create` call and the bind STILL
        // failed, which proves the actor does not arrive through that call. It
        // cannot: the mount's whole design is that an actor of the SOCKET proc
        // mounts a manifest, and such actors are also placed by the stage's own
        // ACTR data — where `argument` comes from the placement byte
        // (`d_stage.cpp:1620`), not from us. One spawn path was patched; the
        // other was never touched.
        //
        // The mount is the single point EVERY mounted actor passes through no
        // matter how it was created, so the identity belongs here. Without it
        // `fopAcM_findObjectCB` (proc AND argument) cannot match, and the whole
        // mounted roster stays unfindable by census name.
        //
        // Arg stays EXACT (Ls1=5, Ob1=7, Ko1=8 share the HENNA0 proc). Widening
        // it to -1 would "fix" the lookup by matching whichever islander came
        // first — the identity-swap class of №126/№129.
        if (i_this->mManifest.socketArg >= 0) {
            i_this->argument = (s8)i_this->mManifest.socketArg;
            DuskLog.debug("[ExtNpcMount] №164 census identity '{}' argument={}", procName,
                          (int)i_this->argument);
        }

        // №24 D2: BG payloads require explicit anchor= (never invent 0).
        if (i_this->mManifest.isBg && !i_this->mManifest.hasAnchor) {
            DuskLog.warn("[ExtNpcMount] create '{}' refused — BG missing anchor=", procName);
            return cPhs_ERROR_e;
        }

        // R-O2e: folder-side spawn gates (missing flag = false).
        if (i_this->mManifest.spawnIfFlag[0] &&
            !dExtModFlags_get(i_this->mManifest.modFolder, i_this->mManifest.spawnIfFlag)) {
            DuskLog.debug("[ExtNpcMount] create '{}' refused — spawn_if_flag '{}'", procName,
                          i_this->mManifest.spawnIfFlag);
            return cPhs_ERROR_e;
        }
        if (i_this->mManifest.spawnUnlessFlag[0] &&
            dExtModFlags_get(i_this->mManifest.modFolder, i_this->mManifest.spawnUnlessFlag)) {
            DuskLog.debug("[ExtNpcMount] create '{}' refused — spawn_unless_flag '{}'", procName,
                          i_this->mManifest.spawnUnlessFlag);
            return cPhs_ERROR_e;
        }

        i_this->mpBtp = NULL;
        i_this->mpBlink = NULL;      // №188
        i_this->mBlinkBound = false;
        i_this->mDemoFaceActive = false;  // №194
        i_this->mDemoFaceFrame = 0.0f;    // №196
        i_this->mpDemoBtk = NULL;         // №197
        i_this->mDemoBtkBound = false;
        i_this->mDemoBtkLast = 0xFFFFFFFF;
        i_this->mDemoBtkFrame = 0.0f;
        i_this->mpCompanion = NULL;
        i_this->mpBrk = NULL;
        i_this->mpColorBtk = NULL;
        i_this->mpBgBtk = NULL;
        i_this->mpBgW = NULL;
        i_this->mIsBg = i_this->mManifest.isBg;
        i_this->mBgReady = false;
        // №36 C: WW item id in params low byte (Vlupy color + TP grant mapping).
        i_this->mPickupItemNo =
            i_this->mManifest.pickupRupee ? (u8)(fopAcM_GetParam(i_this) & 0xFF) : 0;
        for (int i = 0; i < 3; ++i) {
            i_this->mpBgModels[i] = NULL;
        }
        i_this->mBtpBound = false;
        i_this->mAttachCount = 0;
        i_this->mNeckJnt = -1;
        i_this->mLookYaw = 0;
        i_this->mLookPitch = 0;
        i_this->mTalking = false;
        i_this->mCcReady = false;
        i_this->mTalkEventActive = false;
        i_this->mTalkFrames = 0;
        i_this->mPresentDemoItemNo = -1;  // №251: no pending get-item handoff
        i_this->mDemoOwned = false;       // №250: demo-pose select flag
        i_this->mCloseCooldown = 0;
        i_this->mOrbitPhase = 0;
        i_this->mHeadVariant = 0;
        i_this->mSlaveMap[0] = '\0';
        i_this->mSlavePairCount = 0;
        // №52-B: do NOT clear mDoorKey/mSpawnSrc — stub Create stamps them from pending
        // before the first create phase.
        std::snprintf(i_this->mDialogueSection, sizeof(i_this->mDialogueSection), "%s",
                      i_this->mManifest.dialogueKey);
        i_this->mDialogueNext[0] = '\0';
        for (int i = 0; i < kExtNpcMaxAttach; ++i) {
            i_this->mpAttach[i] = NULL;
            i_this->mAttachJnt[i] = -1;
            i_this->mAttachOnCompanion[i] = 0;
            i_this->mAttachSlave[i] = 0;
        }

        const int arg = fopAcM_GetParam(i_this) & 0xFF;
        for (int i = 0; i < i_this->mManifest.subtypeCount; ++i) {
            const dExtNpcSubtype& subtype = i_this->mManifest.subtypes[i];
            if (!subtype.valid || subtype.arg != arg) {
                continue;
            }
            if (subtype.idle[0]) {
                snprintf(i_this->mManifest.idle, sizeof(i_this->mManifest.idle), "%s", subtype.idle);
            }
            if (subtype.displayName[0]) {
                snprintf(i_this->mManifest.displayName, sizeof(i_this->mManifest.displayName), "%s",
                         subtype.displayName);
            }
            if (subtype.attachModel[0] && i_this->mManifest.attachCount < kExtNpcMaxAttach) {
                const int slot = i_this->mManifest.attachCount++;
                snprintf(i_this->mManifest.attach[slot].model,
                         sizeof(i_this->mManifest.attach[slot].model), "%s", subtype.attachModel);
                snprintf(i_this->mManifest.attach[slot].joint,
                         sizeof(i_this->mManifest.attach[slot].joint), "%s",
                         subtype.attachJoint[0] ? subtype.attachJoint : "head");
            }
            DuskLog.info("[ExtNpcMount] {} subtype arg={} selected", procName, arg);
            break;
        }

        // №27 N2 / №44: optional pending/forced head (P1a vs P1b same arg, or FIFO pin).
        {
            char headModel[64];
            char headJoint[32];
            if (dExtNpcMount_consumeForcedAttach(headModel, sizeof(headModel), headJoint,
                                                 sizeof(headJoint))) {
                if (i_this->mManifest.attachCount < kExtNpcMaxAttach) {
                    const int slot = i_this->mManifest.attachCount++;
                    snprintf(i_this->mManifest.attach[slot].model,
                             sizeof(i_this->mManifest.attach[slot].model), "%s", headModel);
                    snprintf(i_this->mManifest.attach[slot].joint,
                             sizeof(i_this->mManifest.attach[slot].joint), "%s", headJoint);
                    const char* dot = std::strstr(headModel, ".bdl");
                    if (dot != NULL && dot >= headModel + 2 &&
                        dot[-2] >= '0' && dot[-2] <= '9' && dot[-1] >= '0' && dot[-1] <= '9') {
                        i_this->mHeadVariant = (u8)((dot[-2] - '0') * 10 + (dot[-1] - '0'));
                    } else {
                        i_this->mHeadVariant = 1;
                    }
                    DuskLog.info("[ExtNpcMount] {} pending/forced attach '{}' @ '{}'", procName,
                                 headModel, headJoint);
                }
            }
        }
        // №45/№48/№49 PULL: heads + companion + slave_map from actor_map (before heap).
        {
            const int regArg =
                i_this->mManifest.socketArg >= 0 ? i_this->mManifest.socketArg : arg;
            pullSlaveMapFromRegistry(procName, regArg, i_this->mSlaveMap, sizeof(i_this->mSlaveMap));
            if (i_this->mSlaveMap[0]) {
                DuskLog.info("[ExtNpcMount] {} slave_map={}", procName, i_this->mSlaveMap);
            }
            // №49: row may carry BOTH head_model and companion (Quill bmhead + bmarm).
            if (i_this->mManifest.companionModel[0] == '\0') {
                char companion[64] = {};
                if (pullCompanionFromRegistry(procName, regArg, companion, sizeof(companion))) {
                    snprintf(i_this->mManifest.companionModel,
                             sizeof(i_this->mManifest.companionModel), "%s", companion);
                    i_this->mManifest.companionMode = 1;
                    DuskLog.info("[ExtNpcMount] {} registry pull companion '{}'", procName,
                                 companion);
                }
            } else if (i_this->mSlaveMap[0]) {
                i_this->mManifest.companionMode = 1;
            }
            if (i_this->mManifest.bodyBmt[0] == '\0') {
                char bodyBmt[64] = {};
                if (pullBodyBmtFromRegistry(procName, regArg, bodyBmt, sizeof(bodyBmt))) {
                    snprintf(i_this->mManifest.bodyBmt, sizeof(i_this->mManifest.bodyBmt), "%s",
                             bodyBmt);
                    DuskLog.info("[ExtNpcMount] {} registry pull body_bmt '{}'", procName, bodyBmt);
                }
            }
            if (i_this->mManifest.attachCount == 0 && !i_this->mManifest.isBg) {
                char headModel[64] = {};
                char headJoint[32] = {};
                if (pullHeadFromRegistry(procName, regArg, fopAcM_GetParam(i_this), headModel,
                                         sizeof(headModel), headJoint, sizeof(headJoint))) {
                    if (i_this->mManifest.attachCount < kExtNpcMaxAttach) {
                        const int slot = i_this->mManifest.attachCount++;
                        snprintf(i_this->mManifest.attach[slot].model,
                                 sizeof(i_this->mManifest.attach[slot].model), "%s", headModel);
                        snprintf(i_this->mManifest.attach[slot].joint,
                                 sizeof(i_this->mManifest.attach[slot].joint), "%s",
                                 headJoint[0] ? headJoint : "head");
                        DuskLog.info("[ExtNpcMount] {} registry pull head '{}' @ '{}'", procName,
                                     headModel, headJoint[0] ? headJoint : "head");
                    }
                }
            }
        }

        // №38 E2 SPAWN LEDGER — after head pin so head= is authoritative.
        // №51: persist src on the actor (forceNextSpawnSrc is one-shot; door props need it).
        {
            if (s_forcedSpawnSrcSet && s_forcedSpawnSrc[0]) {
                if (i_this->mSpawnSrc[0] == '\0') {
                    std::snprintf(i_this->mSpawnSrc, sizeof(i_this->mSpawnSrc), "%s",
                                  s_forcedSpawnSrc);
                }
                if (i_this->mDoorKey[0] == '\0' && std::strncmp(s_forcedSpawnSrc, "door:", 5) == 0 &&
                    s_forcedSpawnSrc[5]) {
                    std::snprintf(i_this->mDoorKey, sizeof(i_this->mDoorKey), "%s",
                                  s_forcedSpawnSrc + 5);
                }
            }
            const char* src =
                i_this->mSpawnSrc[0] ? i_this->mSpawnSrc :
                (s_forcedSpawnSrcSet && s_forcedSpawnSrc[0] ? s_forcedSpawnSrc : "unknown");
            s_forcedSpawnSrcSet = false;
            s_forcedSpawnSrc[0] = '\0';
            char headBuf[96];
            const char* head = "(none)";
            if (i_this->mManifest.attachCount > 0 && i_this->mManifest.attach[0].model[0]) {
                std::snprintf(headBuf, sizeof(headBuf), "%s@%s", i_this->mManifest.attach[0].model,
                              i_this->mManifest.attach[0].joint[0] ?
                                  i_this->mManifest.attach[0].joint :
                                  "head");
                head = headBuf;
            } else if (i_this->mManifest.companionModel[0] && !i_this->mManifest.isBg &&
                       !i_this->mManifest.isStatic) {
                // Aryll hands etc. — companion is the attach proof, not a head miss.
                std::snprintf(headBuf, sizeof(headBuf), "companion:%s",
                              i_this->mManifest.companionModel);
                head = headBuf;
            } else if (i_this->mManifest.attachCount == 0 && !i_this->mManifest.isBg &&
                       !i_this->mManifest.isStatic) {
                head = "MISS:no_head_attach";
            }
            DuskLog.info("[Spawn] src={} proc={} arg={} mod={} head={}", src, procName,
                         i_this->mManifest.socketArg, i_this->mManifest.modFolder, head);
        }

        stageLog("create", "resLoad begin");
    } else if (i_this->mManifest.proc[0] == '\0') {
        // Re-entry without a stamped manifest — recover lookup once.
        if (!dExtNpcMount_lookup(procName, &i_this->mManifest)) {
            return cPhs_ERROR_e;
        }
    }

    // Already heap-bound (should not re-enter Create after COMPLEATE).
    // №117: collision-only statics set mBgReady without mpMorf / without mIsBg.
    if (i_this->mpMorf != NULL || i_this->mBgReady) {
        return cPhs_COMPLEATE_e;
    }

    int phase = dComIfG_resLoad(&i_this->mPhase, i_this->mManifest.arc);
    if (phase != cPhs_COMPLEATE_e) {
        if (phase == cPhs_ERROR_e) {
            DuskLog.info("[ExtNpcMount:D1] create — FAIL resLoad ERROR ({})", procName);
            dExtNpcMount_forceNextAttach(NULL, NULL);
        }
        return phase;
    }
    stageLog("create", "resLoad COMPLEATE → solid heap");

    beginCreateCacheTrack();
    if (i_this->mIsBg) {
        if (!fopAcM_entrySolidHeap(i_this, useBgHeapInit, 0x120000)) {
            DuskLog.warn("[ExtNpcMount] BG heap fail for {}", procName);
            // №115: warm-storm hang signature — first heap fail must stop further warms.
            if (std::strcmp(i_this->mSpawnSrc, "warm") == 0) {
                abortWarmInteriors("BG heap fail");
            }
            abortCreateCacheTrack();
            return cPhs_ERROR_e;
        }
        // Regist BEFORE first Move (History Phase O / №20). Matrix was identity at Set;
        // apply host−anchor now so Move() deltas the vtx table into diorama space.
        if (dComIfG_Bgsp().Regist(i_this->mpBgW, i_this)) {
            DuskLog.warn("[ExtNpcMount] BG Regist failed for {}", procName);
            abortCreateCacheTrack();
            return cPhs_ERROR_e;
        }
        updateBgTransform(i_this);
        if (!i_this->mBgGlobal) {
            i_this->mpBgW->Move();
        } else {
            // №256 / №265: daBg sets PRIORITY_0 + setBgW(room); it does NOT
            // SetRoomId (default 0xFF → GetRoomId falls through to GetGrpRoomId).
            // Stamping actor room when that room is the keep-slot 0 poisons Link's
            // underfoot room → event manager room-keyed packs (awake, doors) all
            // resolve -1. Always stamp the HOST room (Outset exterior = 44).
            const int roomNo = resolveIdentityBgHostRoom(i_this);
            if (roomNo > 0 && roomNo < 0x40) {
                if (fopAcM_GetRoomNo(i_this) != (s8)roomNo) {
                    fopAcM_SetRoomNo(i_this, (s8)roomNo);
                    i_this->home.roomNo = (s8)roomNo;
                }
                i_this->mpBgW->SetRoomId(roomNo);
                dBgW_Base* prev = dStage_roomControl_c::getBgW(roomNo);
                if (prev != NULL && prev != i_this->mpBgW) {
                    if (prev->ChkUsed()) {
                        dComIfG_Bgsp().Release(prev);
                    }
                    DuskLog.info(
                        "[ExtNpcMount] №257 released stub room{} BgW — mount '{}' owns "
                        "collision",
                        roomNo, procName);
                }
                dStage_roomControl_c::setBgW(roomNo, i_this->mpBgW);
                DuskLog.info("[ExtNpcMount] №265 SetRoomId({}) host for '{}'", roomNo,
                             procName);
            } else {
                // Leave m_roomId=0xFF (grp-room fallback). Never SetRoomId(0).
                DuskLog.warn(
                    "[ExtNpcMount] №265 no host room for '{}' actorRoom={} — leave "
                    "BgW roomId=0xFF",
                    procName, (int)fopAcM_GetRoomNo(i_this));
            }
        }
        i_this->mBgReady = true;
        if (i_this->mpBgModels[0] != NULL) {
            fopAcM_SetMtx(i_this, i_this->mpBgModels[0]->getBaseTRMtx());
        } else {
            // №117: model-less BG — bind actor mtx to collision transform.
            fopAcM_SetMtx(i_this, i_this->mBgMtx);
        }
        // Outset's authored span is ~100k units. Never frustum-cull the actor away.
        fopAcM_SetMin(i_this, -120000.0f, -20000.0f, -120000.0f);
        fopAcM_SetMax(i_this, 120000.0f, 20000.0f, 120000.0f);
        fopAcM_OffStatus(i_this, fopAcStts_CULL_e);
        dKy_tevstr_init(&i_this->tevStr, fopAcM_GetRoomNo(i_this), 0xFF);
        i_this->tevStr.room_no = fopAcM_GetRoomNo(i_this);
        // №27 N6 / №108: hide TP stub vrbox; outdoor F_DL* mounts WwSky instead.
        if (strcmp(procName, "EXT_BG0") == 0 || strcmp(procName, "EXT_BG9") == 0) {
            g_env_light.hide_vrbox = true;
            DuskLog.info("[ExtNpcMount] N6 hide_vrbox for '{}'", procName);
        }
        if (mountWantsWwSky(i_this)) {
            wwSkyRetain();
        }
        commitCreateCacheTrack();
        retainArcModels(i_this->mManifest.arc);
        DuskLog.info(
            "[ExtNpcMount] BG COMPLEATE {} — dzbT=({}, {}, {}) model_space={} host=({}, {}, {}) "
            "anchor=({}, {}, {})",
            procName, i_this->current.pos.x - i_this->mManifest.anchor.x,
            i_this->current.pos.y - i_this->mManifest.anchor.y,
            i_this->current.pos.z - i_this->mManifest.anchor.z,
            i_this->mManifest.modelSpaceLocal ? "local" : "world", i_this->current.pos.x,
            i_this->current.pos.y, i_this->current.pos.z, i_this->mManifest.anchor.x,
            i_this->mManifest.anchor.y, i_this->mManifest.anchor.z);
        // №90/№104: population (+ knobs) off EVERY BG COMPLEATE on its host — including
        // EXT_BG0 (exterior re-entry was skipping census; gen bump alone is not enough).
        // Deferred until player exists via poll.
        {
            const char* stage = dComIfGp_getStartStageName();
            const bool onHost = i_this->mManifest.hostStage[0] == '\0' ||
                                (stage != NULL &&
                                 std::strcmp(stage, i_this->mManifest.hostStage) == 0);
            const bool isExterior = std::strcmp(procName, "EXT_BG0") == 0;
            if (onHost && (isExterior || dExtWwSave_isWwHostStage(stage) ||
                           dExtNpcMount_isRoomLaneProc(procName))) {
                // №104: activate room immediately (daBg never runs on these shells).
                if (!isExterior && i_this->mManifest.hostRoom >= 0 &&
                    i_this->mManifest.hostRoom < 0x40) {
                    activateWwHostRoom(i_this->mManifest.hostRoom, "bg-compleate");
                }
                std::snprintf(s_interiorBootstrapProc, sizeof(s_interiorBootstrapProc), "%s",
                              procName);
                tryInteriorBootstrap();
            }
        }
        return cPhs_COMPLEATE_e;
    }

    if (!fopAcM_entrySolidHeap(i_this, useHeapInit, 0x80000)) {
        DuskLog.warn("[ExtNpcMount] heap fail for {}", procName);
        stageLog("create", "FAIL entrySolidHeap");
        abortCreateCacheTrack();
        return cPhs_ERROR_e;
    }

    const f32 s = i_this->mManifest.scale;
    i_this->scale.set(s, s, s);

    dKy_tevstr_init(&i_this->tevStr, fopAcM_GetRoomNo(i_this), 0xFF);
    i_this->tevStr.room_no = fopAcM_GetRoomNo(i_this);

    // №117: collision-only static — no model / morf; regist dzb and finish.
    if (i_this->mpMorf == NULL && i_this->mpBgW != NULL && i_this->mBgReady) {
        if (dComIfG_Bgsp().Regist(i_this->mpBgW, i_this)) {
            DuskLog.warn("[ExtNpcMount] №117 collision-only Regist failed for {}", procName);
            abortCreateCacheTrack();
            return cPhs_ERROR_e;
        }
        i_this->mpBgW->Move();
        fopAcM_SetMtx(i_this, i_this->mBgMtx);
        fopAcM_SetMin(i_this, -200.0f * s, -20.0f * s, -200.0f * s);
        fopAcM_SetMax(i_this, 200.0f * s, 400.0f * s, 200.0f * s);
        commitCreateCacheTrack();
        retainArcModels(i_this->mManifest.arc);
        DuskLog.info("[ExtNpcMount] №117 COMPLEATE collision-only {} arc={} dzb={}", procName,
                     i_this->mManifest.arc, i_this->mManifest.collision);
        return cPhs_COMPLEATE_e;
    }

    fopAcM_SetMtx(i_this, i_this->mpMorf->getModel()->getBaseTRMtx());
    fopAcM_SetMin(i_this, -80.0f * s, -20.0f * s, -80.0f * s);
    fopAcM_SetMax(i_this, 80.0f * s, 200.0f * s, 80.0f * s);
    // №32 B7: carryable = PICKUP only (never SPEAK). B1: door=1 → DOOR attention.
    if (i_this->mManifest.carryable) {
        i_this->attention_info.flags = fopAc_AttnFlag_CARRY_e;
        fopAcM_OnCarryType(i_this, fopAcM_CARRY_LIGHT);
    } else if (i_this->mManifest.doorAttention) {
        i_this->attention_info.flags = fopAc_AttnFlag_DOOR_e;
        // №91: resolve DoorK10 events when ready (async load may complete later in execute).
        i_this->mKnobEvtFront = -1;
        i_this->mKnobEvtBack = -1;
        i_this->mKnobEvtOrdered = -1;
        i_this->mKnobStaffId = -1;
        i_this->mKnobDoorAction = 0;
        i_this->mKnobOpenStarted = 0;
        i_this->mKnobEvBound = 0;
        bindKnobDoorEvents(i_this);
    } else if (i_this->mManifest.isStatic && i_this->mManifest.dialogueKey[0] == '\0') {
        i_this->attention_info.flags = 0;
    } else {
        i_this->attention_info.flags = fopAc_AttnFlag_TALK_e | fopAc_AttnFlag_SPEAK_e;
    }
    // Attention distances are table indices (daNpcT_getDistTableIdx formula), not world units.
    auto distIdx = [](int dist, int angle) -> u8 {
        return (u8)(dist + angle * 0x14 + 0x5e);
    };
    i_this->attention_info.distances[fopAc_attn_LOCK_e] = distIdx(5, 6);
    i_this->attention_info.distances[fopAc_attn_TALK_e] = distIdx(5, 6);
    i_this->attention_info.distances[fopAc_attn_SPEAK_e] = distIdx(3, 6);
    if (i_this->mManifest.carryable) {
        i_this->attention_info.distances[fopAc_attn_CARRY_e] = distIdx(3, 6);
    }
    if (i_this->mManifest.doorAttention) {
        i_this->attention_info.distances[fopAc_attn_DOOR_e] = distIdx(3, 6);
    }
    i_this->attention_info.position = i_this->current.pos;
    i_this->attention_info.position.y += i_this->mManifest.cylHeight * s * 0.85f;
    i_this->mCcStts.Init(0xFF, 0, i_this);
    i_this->mCyl.Set(s_mountCylSrc);
    i_this->mCyl.SetStts(&i_this->mCcStts);
    i_this->mCyl.SetR(i_this->mManifest.cylRadius * s);
    i_this->mCyl.SetH(i_this->mManifest.cylHeight * s);
    i_this->mCcReady = true;
    JUTNameTab* jointNames = i_this->mpMorf->getModel()->getModelData()->getJointTree().getJointName();
    if (jointNames != NULL && i_this->mManifest.neckJoint[0]) {
        const s32 index = jointNames->getIndex(i_this->mManifest.neckJoint);
        if (index >= 0 && index < i_this->mpMorf->getModel()->getModelData()->getJointNum()) {
            i_this->mNeckJnt = (s16)index;
        }
    }
    // №25 F2: authored home Y is the placement floor; never seat below it into seafloor.
    i_this->home.pos = i_this->current.pos;
    i_this->mGroundSnapped = false;
    i_this->mGroundSnapTries = 0;
    tryGroundSnapSanity(i_this);

    // N6: seagulls start orbiting after ground settle (phase ticks in execute).
    if (std::strcmp(i_this->mManifest.arc, "Kamome") == 0 ||
        std::strcmp(procName, "NPC_KAMOME") == 0) {
        i_this->mOrbitPhase = 1;
    }

    // №47-A: force idle start on every lane (heap may have bound NULL if N3 once refused).
    // №262: attach-aware — a mount still carrying its flag-gated prop starts
    // in the carry idle (Grandma: hold.bck until ba.clothes_given).
    if (!i_this->mManifest.isStatic && mountIdleBck(i_this)[0]) {
        setMountAnimation(i_this, mountIdleBck(i_this), J3DFrameCtrl::EMode_LOOP);
    }

    commitCreateCacheTrack();
    retainArcModels(i_this->mManifest.arc);
    DuskLog.info("[ExtNpcMount] COMPLEATE {} arc={} model={} btp={} scale={} mod={}", procName,
                 i_this->mManifest.arc, i_this->mManifest.model,
                 i_this->mBtpBound ? i_this->mManifest.btp : "(none)", s,
                 i_this->mManifest.modFolder);
    stageLog("create", "COMPLEATE");
    return cPhs_COMPLEATE_e;
}

int dExtNpcMount_delete(dExtNpcMount_c* i_this) {
    if (i_this != NULL && i_this->mIsBg) {
        if (mountWantsWwSky(i_this) ||
            (i_this->mManifest.proc[0] &&
             (std::strcmp(i_this->mManifest.proc, "EXT_BG0") == 0 ||
              std::strcmp(i_this->mManifest.proc, "EXT_BG9") == 0))) {
            // Stage may already have changed; always drop a user if we retained on COMPLEATE.
            wwSkyReleaseUser();
        }
        // №100: drop collision + draw refs before releasing the arc cache.
        // №257: clear room slot if we claimed it (daBg destructor does the same).
        if (i_this->mpBgW != NULL) {
            if (i_this->mBgGlobal) {
                // Prefer the room we stamped on the BgW; fall back to host resolve.
                int roomNo = i_this->mpBgW->GetRoomId();
                if (roomNo <= 0 || roomNo >= 0x40 || roomNo == 0xFF) {
                    roomNo = resolveIdentityBgHostRoom(i_this);
                }
                if (roomNo > 0 && roomNo < 0x40 &&
                    dStage_roomControl_c::getBgW(roomNo) == i_this->mpBgW) {
                    dStage_roomControl_c::setBgW(roomNo, NULL);
                }
            }
            dComIfG_Bgsp().Release(i_this->mpBgW);
            i_this->mpBgW = NULL;
        }
        for (int i = 0; i < 3; ++i) {
            i_this->mpBgModels[i] = NULL;
        }
        i_this->mBgReady = false;
        if (i_this->mManifest.arc[0]) {
            // №73/№100: purge cached J3D only when no other live mount refs this arc.
            releaseArcModels(i_this->mManifest.arc, "delete-bg", i_this);
            dComIfG_resDelete(&i_this->mPhase, i_this->mManifest.arc);
        }
        return 1;
    }
    // G4: the draw-list dialogue must be hidden before the owner actor goes away.
    if (i_this != NULL && (i_this->mTalking || s_mountDialogueOwner == i_this)) {
        closeMountDialogue(i_this);
    }
    if (i_this != NULL && i_this->mManifest.arc[0]) {
        releaseArcModels(i_this->mManifest.arc, "delete-npc", i_this);
        dComIfG_resDelete(&i_this->mPhase, i_this->mManifest.arc);
    }
    return 1;
}

// ============================================================================
// №219 — attach placement, shared by the normal path AND the demo branch.
// ============================================================================
// The demo branch (№173) rightly owns the actor and returns before the normal
// path's tail — but that tail was the ONLY place attaches were placed and
// calc'd. Result: during a storyboard, a held prop froze at its create-time
// base matrix (the actor's spawn point) and only looked attached again once
// gameplay resumed. Both paths now call this after body/companion modelCalc,
// so slave CBs pull FRESH host joint matrices in cutscenes too.
// №49/№50: slave attach → parent-compose CBs; door visual → base@DoorDummy.
// №218: non-slave joint attach composes jointMtx × T(offs) × R(rot).
// ============================================================================
static void dExtNpcMount_placeAttachments(dExtNpcMount_c* i_this) {
    if (i_this->mpMorf == NULL || i_this->mpMorf->getModel() == NULL) {
        return;
    }
    J3DModel* body = i_this->mpMorf->getModel();
    J3DModel* companion =
        i_this->mpCompanion != NULL ? i_this->mpCompanion->getModel() : NULL;
    for (int i = 0; i < i_this->mAttachCount; ++i) {
        if (i_this->mpAttach[i] == NULL) {
            continue;
        }
        if (!mountAttachLive(i_this, i)) {  // №262: retired by its flag
            continue;
        }
        J3DModel* host =
            (i_this->mAttachOnCompanion[i] && companion != NULL) ? companion : body;
        if (host == NULL) {
            continue;
        }
        if (i_this->mAttachSlave[i]) {
            i_this->mpAttach[i]->setBaseTRMtx(host->getBaseTRMtx());
        } else if (i_this->mAttachJnt[i] >= 0) {
            if (i_this->mAttachLocal[i]) {
                // №250: demo-pose variant while the storyboard owns this mount.
                const bool useDemo = i_this->mDemoOwned && i_this->mAttachDemoLocal[i];
                const cXyz& off = useDemo ? i_this->mAttachOffsDemo[i] : i_this->mAttachOffs[i];
                const cXyz& rot = useDemo ? i_this->mAttachRotDemo[i] : i_this->mAttachRot[i];
                mDoMtx_stack_c::copy(host->getAnmMtx((u16)i_this->mAttachJnt[i]));
                mDoMtx_stack_c::transM(off.x, off.y, off.z);
                mDoMtx_stack_c::XYZrotM((s16)rot.x, (s16)rot.y, (s16)rot.z);
                i_this->mpAttach[i]->setBaseTRMtx(mDoMtx_stack_c::get());
            } else {
                i_this->mpAttach[i]->setBaseTRMtx(host->getAnmMtx((u16)i_this->mAttachJnt[i]));
            }
        } else {
            i_this->mpAttach[i]->setBaseTRMtx(host->getBaseTRMtx());
        }
        i_this->mpAttach[i]->calc();
    }
}

int dExtNpcMount_execute(dExtNpcMount_c* i_this) {
    if (i_this == NULL) {
        return 1;
    }
    // №68: room teardown in flight — do not touch matrices / attention / bgw.
    if (roomLaneMountIsUnloading(i_this)) {
        return 1;
    }
    if (i_this->mIsBg) {
        if (mountWantsWwSky(i_this) || s_wwSkyUsers > 0) {
            wwSkyEnsure();
        }
        updateBgTransform(i_this);
        if (i_this->mpBgBtk != NULL) {
            i_this->mpBgBtk->play();
        }
        // №98: GLOBAL_e world collision has no move mtx — do not Move().
        if (i_this->mpBgW != NULL && !i_this->mBgGlobal) {
            i_this->mpBgW->Move();
        }
        return 1;
    }
    // №117: collision-only static prop — keep dzb aligned with actor pose.
    if (i_this->mpMorf == NULL) {
        if (i_this->mpBgW != NULL && i_this->mBgReady) {
            mDoMtx_stack_c::transS(i_this->current.pos.x, i_this->current.pos.y,
                                   i_this->current.pos.z);
            mDoMtx_stack_c::YrotM(i_this->current.angle.y);
            MTXCopy(mDoMtx_stack_c::get(), i_this->mBgMtx);
            i_this->mpBgW->Move();
        }
        return 1;
    }
    // №173: WHILE THE STORYBOARD DRIVES THIS ACTOR, IT OWNS THE ACTOR.
    //
    // The cast now BINDS (№164) and the storyboard's animation ids resolve
    // against our ported arc (№167: JACT 'Ls1' -> 0x10043/0x10045 ->
    // 47_ls_bwait_l.bck / 47_ls_kyoro_l.bck). But binding only gives the demo a
    // `dDemo_actor_c` to write into — SOMETHING has to read it back, and nothing
    // did. That is why the cast stands still at its authored spot through its own
    // cutscene: the storyboard was animating a record no one applied.
    //
    // `dDemo_setDemoData` is that read-back (d_demo.cpp:334). It copies trans /
    // rotate / scale off the demo actor and, given a morf, resolves the animation
    // by RESOURCE INDEX out of the demo archive and plays it. Flags mirror the
    // engine's own generic demo actor (d_a_demo00 uses 0x2A and 0x8E); we want
    // the full set since we pass a morf:
    //   TRANS|SCALE|ROTATE|ANM|ANM_FRAME|ANM_TRANSITION = 0xEE
    //
    // It returns 0 when this actor is not a demo performer, so the call doubles
    // as the test — no separate demoActorID check that could drift out of sync.
    //
    // RETURNING EARLY IS THE POINT, not an optimisation: ground snap, the orbit
    // walk and the talk logic below all write `current.pos`/animation, and every
    // one of them would fight the storyboard for the same fields. A demo-driven
    // actor must have exactly one author.
    // №174: TRACE, because №173 shipped blind and "she still does not move" was
    // ambiguous three ways: code not in the build / not a performer / performer
    // but the storyboard enabled none of the fields we asked for. Those need
    // completely different fixes and looked identical on screen.
    //
    // `checkEnable(mask)` returns the INTERSECTION of what we requested with what
    // the storyboard has actually set, so printing it against our 0xEE says
    // exactly which of trans/rot/scale/anm the demo is really driving.
    if (dComIfGp_event_runCheck() && (g_Counter.mCounter0 % 60) == 0) {
        dDemo_actor_c* da = dDemo_c::getActor(i_this->demoActorID);
        // Also print where the storyboard WANTS this actor. The user reports the
        // cast never enters the scene at all — no porch start, no beach run — so
        // the question is not only "is anything driving her" but "driving her
        // WHERE". This directly tests the №165 `OffsetPos` thread, still open:
        // `dDemo_c::start()` receives our merged event's offset (-220000, 0,
        // 320000) while the island sits near -195000. A target ~25,000 units off
        // means the demo is staging the cast into open ocean, which would look
        // exactly like "she never appears".
        cXyz want(0.0f, 0.0f, 0.0f);
        if (da != NULL) {
            want = da->getTrans();
        }
        DuskLog.info("[ExtWw] §52 demo read-back '{}' demoActorID={} actor={} enables=0x{:02X} "
                     "(asked 0xEE) at=({:.0f},{:.0f},{:.0f}) demoWants=({:.0f},{:.0f},{:.0f})",
                     i_this->mManifest.proc, (int)i_this->demoActorID,
                     da != NULL ? "bound" : "NONE",
                     da != NULL ? (int)da->checkEnable(0xFF) : 0, i_this->current.pos.x,
                     i_this->current.pos.y, i_this->current.pos.z, want.x, want.y, want.z);
    }
    // №181: FLAGS COME FROM THE DONOR'S OWN ACTOR, not from my reasoning.
    //
    // I picked 0xEE at №173 by arguing "we pass a morf, so enable everything".
    // The donor's Aryll (`d_a_npc_ls1.cpp daNpc_Ls1_c::demo()`) does:
    //
    //     dDemo_setDemoData(this, 106, mpMorf, mArcName, 0, NULL,
    //                       dBgS_GetGndMtrlSndId_Func(current.pos, 10.0f),
    //                       dComIfGp_getReverb(fopAcM_GetRoomNo(this)));
    //
    // 106 = 0x6A = TRANS|ROTATE|ANM|ANM_FRAME. It deliberately does NOT set
    // SCALE (4) or ANM_TRANSITION (128) — the two bits I added.
    //
    // SCALE is almost certainly why the cast was invisible: with ENABLE_SCALE_e
    // set, setDemoData assigns `i_actor->scale = demo_actor->getScale()`, and a
    // storyboard that never authors a scale hands back (0,0,0). A zero-scaled
    // model draws nothing — which is exactly "she is never there", and it also
    // explains why every position probe read correct while the screen stayed
    // empty. ANM_TRANSITION is separately unwanted: it writes `i_actor->gravity`.
    //
    // Reverb is the donor's too. The donor also passes a ground-material sound id
    // (`dBgS_GetGndMtrlSndId_Func`), which the receiver has no direct equivalent
    // for — its `GetMtrlSndId` wants a `cBgS_PolyInfo`. Passing 0 costs correct
    // footstep material SFX and NOTHING ELSE; it is not part of the visibility
    // fix, and is left as a known, scoped gap rather than faked.
    const s8 reverb = dComIfGp_getReverb(fopAcM_GetRoomNo(i_this));
    if (dDemo_setDemoData(i_this, 106, i_this->mpMorf, i_this->mManifest.arc, 0, NULL, 0,
                          reverb)) {
        // №177: the storyboard drives `current.pos`; THIS is what makes it visible.
        //
        // №173's early return was right about ownership and wrong about scope. It
        // stopped the competing WRITES below (ground snap, orbit, talk) — correct
        // — but it also skipped the `setBaseTRMtx` at the end of this function,
        // which is the only place `current.pos` becomes a rendered position. So
        // the cast walked the entire route logically while their models stayed
        // pinned to the last matrix they were handed.
        //
        // The §52 trace is what proved it: `at=` and `demoWants=` agreed exactly
        // and swept -204886 -> -201778 -> -200440 (y~138, beach level) -> -195400
        // (y=1650, lookout) — the porch/beach/hill route — while the user watched
        // her stand still. Logical position and drawn position had diverged, and
        // only the log could show that.
        //
        // Skipping a competing author must not mean skipping the bookkeeping the
        // engine still needs from us.
        const f32 ds = i_this->scale.x;
        mDoMtx_stack_c::transS(i_this->current.pos.x, i_this->current.pos.y,
                               i_this->current.pos.z);
        mDoMtx_stack_c::YrotM(i_this->shape_angle.y);
        mDoMtx_stack_c::scaleM(ds, ds, ds);
        MtxP demoBase = mDoMtx_stack_c::get();
        i_this->mpMorf->getModel()->setBaseTRMtx(demoBase);

        // №184: `modelCalc()` IS WHAT MAKES THE BASE MATRIX REAL.
        //
        // Setting `setBaseTRMtx` only records where the model SHOULD be.
        // `modelCalc()` is what recomputes the joint/world matrices the renderer
        // actually draws from — and the donor says so plainly: `daNpc_Ls1_c::setMtx`
        // does `setBaseScale` -> `setBaseTRMtx` -> **`mpMorf->calc()`** in that
        // order, every frame.
        //
        // Our normal path calls `modelCalc()` at what is now line ~6626, AFTER the
        // early return this branch takes. So a demo-driven actor had its base
        // matrix updated and its joints never recomputed — it rendered with STALE
        // joint matrices, i.e. exactly where it was last calculated: the lookout.
        //
        // That is why every probe read correct and the screen disagreed. Position,
        // scale, matrix and draw were all measuring the model's INTENT; the joints
        // were still describing the old pose. It also explains the user's sharpest
        // clue — her placement shifting slightly only AFTER the cutscene, when the
        // normal path resumed and finally calc'd the final demo position.
        //
        // Same defect class as №177: the early return was right to suppress
        // competing writers and wrong to drop the bookkeeping the engine needs.
        // That is TWICE from one `return`, which is the argument for mirroring the
        // donor's call sequence instead of hand-picking which parts to keep.
        dDemo_actor_c* demoActor = dDemo_c::getActor(i_this->demoActorID);

        // №194: (the №186 `mTexAnm`/`ENABLE_TEX_ANM` block was removed here — it
        // watched the wrong channel and never fired for Ls1, proven by the §54
        // probe. The demo face is driven below from the prm/getDemoIDData channel.)

        // №194: DRIVE THE DEMO FACE from the prm channel.
        //
        // The §54 probe (№193) settled the unknowns: the prm channel is live for
        // the performer, and each frame carries facial entries tagged triple
        // (0,0,1) — a BTP resID (e.g. 0x39 = 47_ls_bwait_l.btp) and a BTK resID
        // (0x4F). Critically they resolve from the DEMO ARCHIVE (Demo02), NOT the
        // actor's own arc — the probe's `arcRes=null` against Ls.arc proved that,
        // and Demo02 index 0x39 IS that btp.
        //
        // So: drain getDemoIDData (file-static iterator — one full while-loop
        // resets it), and for each facial entry resolve the pattern from the demo
        // arc and bind it to the mount's BTP. The donor's demo() does exactly
        // this via getP_BtpData(mArcName) — same data, its own arc name.
        //
        // BLINK ARBITRATION (the №192 conflict, resolved donor-faithfully): while
        // a demo face is active, the demo BTP owns the eye materials — the idle
        // blink (№188) is SUPPRESSED for the frame so the two never fight. The
        // demo idle face (`bwait`) carries its own blink, so nothing is lost.
        bool demoFaceActive = false;
        if (demoActor != NULL && demoActor->checkEnable(dDemo_actor_c::ENABLE_UNK_e)) {
            const char* demoArc = dStage_roomControl_c::getDemoArcName();
            int a0, a1, a2;
            u16 resID;
            int guard = 0;
            bool gotBtp = false;  // №196: the FIRST (0,0,1) entry is the BTP
            bool gotBtk = false;  // №197: the SECOND (0,0,1) entry is the BTK
            const bool logNow = (g_Counter.mCounter0 % 30) == 0;
            // №203 round-4 probe: collect the FULL per-beat texture set so the
            // laugh beat (garbled mouth) can be correlated. Symptom is SHAPE (pink
            // SQUARE not ovular), not colour (round 3 cleared the palette decode),
            // so the question is which texture/UV the mouth gets and whether its
            // BTK is applied. Logged once per BTP-resID CHANGE (each beat), with
            // each entry's resolved resource SIZE (a byte size distinguishes a
            // BTP vs BTK vs BCK, and 0 = not resident).
            struct { int a0, a1, a2; u16 id; u32 sz; } s_prmDump[16];
            int s_prmN = 0;
            u16 s_firstBtp = 0;
            const u32 prevBtp = i_this->mDemoTexAnmLast;  // №203: detect beat change
            while (demoActor->getDemoIDData(&a0, &a1, &a2, &resID, NULL) != 0) {
                if (++guard > 32) {
                    break;
                }
                if (s_prmN < 16) {
                    void* r = dComIfG_getObjectIDRes(
                        dStage_roomControl_c::getDemoArcName(), resID);
                    s_prmDump[s_prmN].a0 = a0;
                    s_prmDump[s_prmN].a1 = a1;
                    s_prmDump[s_prmN].a2 = a2;
                    s_prmDump[s_prmN].id = resID;
                    s_prmDump[s_prmN].sz = r != NULL ? 1u : 0u;  // resident?
                    if (a0 == 0 && a1 == 0 && a2 == 1 && s_firstBtp == 0) {
                        s_firstBtp = resID;
                    }
                    s_prmN++;
                }
                // Facial entries are tagged (0,0,1). The prm channel emits TWO per
                // frame with the same triple: the BTP (e.g. 0x39) FIRST, then the
                // BTK (0x4F). №194 mistakenly bound BOTH as BTP — loading a BTK
                // through a BTP init thrashes the eye material every frame, which
                // is the "rapid double-blinking". Take only the first as the BTP;
                // the BTK (secondary texture-SRT) is left for a later pass.
                if (a0 == 0 && a1 == 0 && a2 == 1 && !gotBtp) {
                    gotBtp = true;
                    J3DAnmTexPattern* pat =
                        (J3DAnmTexPattern*)dComIfG_getObjectIDRes(demoArc, resID);
                    if (pat != NULL) {
                        if (resID != i_this->mDemoTexAnmLast) {
                            if (i_this->mpBtp == NULL) {
                                i_this->mpBtp = JKR_NEW mDoExt_btpAnm();
                            }
                            // EMode_NONE: we own the frame (play-once-hold below),
                            // NOT a loop — looping re-blinks bwait continuously.
                            if (i_this->mpBtp != NULL &&
                                i_this->mpBtp->init(i_this->mpMorf->getModel()->getModelData(),
                                                    pat, 1, J3DFrameCtrl::EMode_NONE, 1.0f, 0,
                                                    -1) != 0) {
                                i_this->mBtpBound = true;
                                i_this->mDemoTexAnmLast = resID;
                                i_this->mDemoFaceFrame = 0.0f;  // new expression: replay once
                                if (logNow) {
                                    DuskLog.info(
                                        "[ExtWw] §54 demo face '{}' BTP resID=0x{:X} (re)bound",
                                        i_this->mManifest.proc, (int)resID);
                                }
                            }
                        }
                        demoFaceActive = true;
                    } else if (logNow) {
                        DuskLog.info("[ExtWw] §54 demo face '{}' resID=0x{:X} not in demo arc",
                                     i_this->mManifest.proc, (int)resID);
                    }
                } else if (a0 == 0 && a1 == 0 && a2 == 1 && gotBtp && !gotBtk) {
                    // №197: SECOND (0,0,1) entry = the BTK (texture SRT). Shapes the
                    // open-mouth/tongue texture into the mouth — without it the raw
                    // texture renders as a black+pink square (user report during the
                    // telescope beat). Donor pairs BTP+BTK per expression.
                    gotBtk = true;
                    J3DAnmTextureSRTKey* srt =
                        (J3DAnmTextureSRTKey*)dComIfG_getObjectIDRes(demoArc, resID);
                    if (srt != NULL) {
                        if (resID != i_this->mDemoBtkLast) {
                            if (i_this->mpDemoBtk == NULL) {
                                i_this->mpDemoBtk = JKR_NEW mDoExt_btkAnm();
                            }
                            if (i_this->mpDemoBtk != NULL &&
                                i_this->mpDemoBtk->init(i_this->mpMorf->getModel()->getModelData(),
                                                        srt, 1, J3DFrameCtrl::EMode_NONE, 1.0f, 0,
                                                        -1) != 0) {
                                i_this->mDemoBtkBound = true;
                                i_this->mDemoBtkLast = resID;
                                i_this->mDemoBtkFrame = 0.0f;
                            }
                        }
                    }
                }
            }
            // №203: dump the full prm set ONCE per beat (when the BTP resID
            // changed this frame). Each line shows every (arg-triple, resID,
            // resident?) so the laugh beat's texture set is visible, plus which
            // resID became BTP vs BTK and whether the BTK bound. Tells: is the
            // mouth's shaping BTK present at this beat, or is it a BTP-only frame
            // whose texture shows unshaped (square)?
            if (s_firstBtp != 0 && s_firstBtp != prevBtp) {
                DuskLog.info("[ExtWw] §56 BEAT '{}' frame={} n={} BTP=0x{:X} BTK=0x{:X} "
                             "btkBound={}",
                             i_this->mManifest.proc, (int)dDemo_c::getFrame(), s_prmN,
                             (int)s_firstBtp, (int)i_this->mDemoBtkLast,
                             i_this->mDemoBtkBound ? 1 : 0);
                for (int k = 0; k < s_prmN; ++k) {
                    DuskLog.info("[ExtWw] §56   [{}] arg=({},{},{}) resID=0x{:X} resident={}", k,
                                 s_prmDump[k].a0, s_prmDump[k].a1, s_prmDump[k].a2,
                                 (int)s_prmDump[k].id, (int)s_prmDump[k].sz);
                }
            }
        }

        i_this->mDemoFaceActive = demoFaceActive;  // draw reads this
        // №196: advance play-ONCE-and-hold, donor play_btp_anm (non-blink branch):
        // ++ to frameMax then hold. The expression plays through once when set and
        // then holds — no continuous re-blink. A new expression resets the frame
        // (above), so each face plays exactly once.
        if (demoFaceActive && i_this->mBtpBound && i_this->mpBtp != NULL) {
            const f32 fmax = i_this->mpBtp->getEndFrame();
            i_this->mDemoFaceFrame += 1.0f;
            if (i_this->mDemoFaceFrame >= fmax) {
                i_this->mDemoFaceFrame = fmax;  // hold on the final frame
            }
            i_this->mpBtp->setFrame(i_this->mDemoFaceFrame);
        }
        // №197: BTK advances the same way — play once, hold.
        if (demoFaceActive && i_this->mDemoBtkBound && i_this->mpDemoBtk != NULL) {
            const f32 fmax = i_this->mpDemoBtk->getEndFrame();
            i_this->mDemoBtkFrame += 1.0f;
            if (i_this->mDemoBtkFrame >= fmax) {
                i_this->mDemoBtkFrame = fmax;
            }
            i_this->mpDemoBtk->setFrame(i_this->mDemoBtkFrame);
        }
        // While the demo owns the face, DO NOT run the idle blink — it would
        // clobber the demo expression (both write the same eye materials).
        if (!demoFaceActive) {
            dExtNpcMount_driveBlink(i_this);  // №188: idle blink between demo faces
        }
        i_this->mpMorf->modelCalc();
        if (i_this->mpCompanion != NULL && i_this->mpCompanion->getModel() != NULL) {
            i_this->mpCompanion->getModel()->setBaseTRMtx(demoBase);
            i_this->mpCompanion->modelCalc();
        }
        // №219: attaches must ride the storyboard too — place them off the
        // matrices the demo just computed, or a held prop freezes at spawn.
        i_this->mDemoOwned = true;  // №250: demo pose variant selects
        dExtNpcMount_placeAttachments(i_this);

        if (i_this->mpBgW != NULL && i_this->mBgReady) {
            mDoMtx_stack_c::transS(i_this->current.pos.x, i_this->current.pos.y,
                                   i_this->current.pos.z);
            mDoMtx_stack_c::YrotM(i_this->current.angle.y);
            MTXCopy(mDoMtx_stack_c::get(), i_this->mBgMtx);
            i_this->mpBgW->Move();
        }
        return 1;
    }

    tryGroundSnapSanity(i_this);

    // №27 N6: simple seagull orbit around authored home (no AI arc — folder-side motion).
    if (i_this->mOrbitPhase > 0 && !i_this->mTalking) {
        ++i_this->mOrbitPhase;
        const s16 ang = (s16)(i_this->mOrbitPhase * 180);
        const s16 bob = (s16)(i_this->mOrbitPhase * 310);
        const f32 radius = 220.0f;
        i_this->current.pos.x = i_this->home.pos.x + radius * cM_scos(ang);
        i_this->current.pos.z = i_this->home.pos.z + radius * cM_ssin(ang);
        i_this->current.pos.y = i_this->home.pos.y + 40.0f * cM_ssin(bob);
        i_this->shape_angle.y = (s16)(ang + 0x4000);
        i_this->current.angle.y = i_this->shape_angle.y;
    }

    const f32 s = i_this->scale.x;
    i_this->attention_info.position = i_this->current.pos;
    i_this->attention_info.position.y += i_this->mManifest.cylHeight * s * 0.85f;
    if (i_this->mCcReady) {
        i_this->mCyl.SetC(i_this->current.pos);
        i_this->mCyl.SetR(i_this->mManifest.cylRadius * s);
        i_this->mCyl.SetH(i_this->mManifest.cylHeight * s);
        i_this->mCyl.ClrCoHit();
        dComIfG_Ccsp()->Set(&i_this->mCyl);
    }

    if (i_this->mCloseCooldown > 0) {
        --i_this->mCloseCooldown;
    }
    // №263: one-shot present motion finished → back to the state-aware idle
    // (post-give the attach flag is set, so this lands on the plain idle).
    if (i_this->mPresentAnimActive && i_this->mpMorf != NULL && i_this->mpMorf->isStop() != 0) {
        i_this->mPresentAnimActive = false;
        setMountAnimation(i_this, mountIdleBck(i_this), J3DFrameCtrl::EMode_LOOP);
    }
    if (i_this->mTalking) {
        ++i_this->mTalkFrames;
        // №33: keep native talk-event lock alive while the window is up (postman pattern).
        if (i_this->mTalkEventActive) {
            dMeter2Info_onGameStatus(2);
        }
#if TARGET_PC_NATIVE_UI
        // ====================================================================
        // №248 — native flow poll. doFlow returns nonzero when the current box
        // completes. Section ADVANCE re-inits the flow directly — the speak
        // event is NOT released between boxes (the old close-then-begin reset
        // the event after box 1, which is exactly why only the first box ever
        // locked Link, №242/№247). The event releases ONCE, at the true end.
        // ====================================================================
        if (s_mountFlow.doFlow(i_this, NULL, 0) != 0 || i_this->mTalkFrames >= 1800) {
            // №252: remaining PAGES of the current section come first — same
            // held speak event, next page injected, no section change.
            if (s_mountPageIdx < s_mountPages.size() && i_this->mTalkFrames < 1800) {
                s_mountFlow.initWord(i_this, s_mountPages[s_mountPageIdx].c_str(), 0xFF, 0, NULL);
                ++s_mountPageIdx;
            } else {
            char pendingNext[64];
            std::snprintf(pendingNext, sizeof(pendingNext), "%s", i_this->mDialogueNext);
            if (pendingNext[0] != '\0' && i_this->mTalkFrames < 1800) {
                std::snprintf(i_this->mDialogueSection, sizeof(i_this->mDialogueSection), "%s",
                              pendingNext);
                i_this->mDialogueNext[0] = '\0';
                const bool wasEvent = i_this->mTalkEventActive;
                if (!beginMountDialogue(i_this, wasEvent)) {
                    closeMountDialogue(i_this);
                    std::snprintf(i_this->mDialogueSection, sizeof(i_this->mDialogueSection),
                                  "%s", i_this->mManifest.dialogueKey);
                }
            } else {
                closeMountDialogue(i_this);
                // Next talk starts at the manifest entry key (else=/gates re-evaluate).
                std::snprintf(i_this->mDialogueSection, sizeof(i_this->mDialogueSection), "%s",
                              i_this->mManifest.dialogueKey);
            }
            }  // №252: end of pages-exhausted branch
        }
#else
        closeMountDialogue(i_this);
#endif
    } else if (i_this->mCloseCooldown == 0) {
        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        // №91: doorAttention mounts order DEFAULT_KNOB_DOOR_* and own cutEnd, then warp.
        // Until DoorK10 binds, №53 CANDOOR → immediate tryNativeWarp remains as fallback.
        if (i_this->mManifest.doorAttention) {
            if (player != NULL &&
                (player->current.pos - i_this->current.pos).absXZ() < 300.0f) {
                i_this->attention_info.flags = fopAc_AttnFlag_DOOR_e;
                i_this->attention_info.position = i_this->current.pos;
                i_this->attention_info.position.y += 80.0f;
            }
            tickKnobDoorEvent(i_this);
        } else {
            // №33: wrap mount dialogue in a real TP speak event (postman pattern).
            // Prefer checkCommandTalk; TrigA only orders the speak event (no orphan window).
            i_this->eventInfo.onCondition(dEvtCnd_CANTALK_e);
            if (i_this->eventInfo.checkCommandTalk()) {
                // §65 H9 probe — which entry path the talk takes (talk command
                // vs TrigA speak-order fallback) decides the event's actor
                // arrangement; log both so the caption-vs-box diff is pinned.
                DuskLog.warn("[ExtNpcMount] §65 talk-entry: checkCommandTalk ({})",
                             i_this->mManifest.proc);
                beginMountDialogue(i_this, /*fromEvent=*/true);
            } else if (player != NULL &&
                       (player->current.pos - i_this->current.pos).absXZ() < 160.0f &&
                       mDoCPd_c::getTrigA(PAD_1) != 0 &&
                       (i_this->attention_info.flags & fopAc_AttnFlag_SPEAK_e) != 0) {
                DuskLog.warn("[ExtNpcMount] §65 talk-entry: TrigA speak-order fallback ({})",
                             i_this->mManifest.proc);
                fopAcM_orderSpeakEvent(i_this, 0, 0);
            }
        }
    }

    mDoMtx_stack_c::transS(i_this->current.pos.x, i_this->current.pos.y, i_this->current.pos.z);
    mDoMtx_stack_c::YrotM(i_this->shape_angle.y);
    mDoMtx_stack_c::scaleM(s, s, s);
    MtxP base = mDoMtx_stack_c::get();
    i_this->mpMorf->getModel()->setBaseTRMtx(base);
    // №36 C: pickup rupee — near player ⇒ grant TP wallet item + delete WW visual.
    if (i_this->mManifest.pickupRupee) {
        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player != NULL) {
            const f32 dx = player->current.pos.x - i_this->current.pos.x;
            const f32 dy = player->current.pos.y - i_this->current.pos.y;
            const f32 dz = player->current.pos.z - i_this->current.pos.z;
            if (dx * dx + dz * dz < 100.0f * 100.0f && dy > -80.0f && dy < 160.0f) {
                const u8 grant = tpRupeeGrantId(i_this->mPickupItemNo);
                if (grant != 0) {
                    execItemGet(grant);
                    DuskLog.info("[ExtNpcMount] pickup grant ww={} → TP item {} (wallet)",
                                 (int)i_this->mPickupItemNo, (int)grant);
                }
                fopAcM_delete(i_this);
                return 1;
            }
        }
    }

    i_this->mpMorf->play(NULL, 0, 0);
    if (i_this->mBtpBound && i_this->mpBtp != NULL) {
        i_this->mpBtp->play();
    }
    dExtNpcMount_driveBlink(i_this);  // №188: idle blink (non-demo path)
    i_this->mpMorf->modelCalc();
    if (i_this->mNeckJnt >= 0) {
        fopAc_ac_c* player = dComIfGp_getPlayer(0);
        if (player != NULL) {
            cXyz from;
            mDoMtx_stack_c::copy(i_this->mpMorf->getModel()->getAnmMtx(i_this->mNeckJnt));
            mDoMtx_stack_c::multVecZero(&from);
            cXyz to = player->current.pos - from;
            s16 yaw = (s16)(to.atan2sX_Z() - i_this->shape_angle.y);
            s16 pitch = (s16)-to.atan2sY_XZ();
            if (yaw > 0x2000) yaw = 0x2000;
            if (yaw < -0x2000) yaw = -0x2000;
            if (pitch > 0x1000) pitch = 0x1000;
            if (pitch < -0x1000) pitch = -0x1000;
            cLib_addCalcAngleS(&i_this->mLookYaw, yaw, 8, 0x400, 0x20);
            cLib_addCalcAngleS(&i_this->mLookPitch, pitch, 8, 0x200, 0x20);
        } else {
            cLib_addCalcAngleS(&i_this->mLookYaw, 0, 8, 0x400, 0x20);
            cLib_addCalcAngleS(&i_this->mLookPitch, 0, 8, 0x200, 0x20);
        }
        mDoMtx_stack_c::copy(i_this->mpMorf->getModel()->getAnmMtx(i_this->mNeckJnt));
        mDoMtx_stack_c::YrotM(i_this->mLookYaw);
        mDoMtx_stack_c::XrotM(i_this->mLookPitch);
        i_this->mpMorf->getModel()->setAnmMtx(i_this->mNeckJnt, mDoMtx_stack_c::get());
    }
    if (i_this->mpCompanion != NULL && i_this->mpCompanion->getModel() != NULL) {
        i_this->mpCompanion->getModel()->setBaseTRMtx(base);
        if (i_this->mManifest.companionMode == 1) {
            // №49: joint callbacks inside modelCalc do REPLACE before envelope.
            i_this->mpCompanion->modelCalc();
        } else {
            i_this->mpCompanion->play(NULL, 0, 0);
            i_this->mpCompanion->modelCalc();
        }
    }
    i_this->mDemoOwned = false;  // №250: gameplay pose variant
    dExtNpcMount_placeAttachments(i_this);
    return 1;
}

static u8 clampAmbChannel(int base, f32 wregOffset) {
    int v = base + (int)wregOffset;
    if (v < 0) {
        return 0;
    }
    if (v > 255) {
        return 255;
    }
    return (u8)v;
}

int dExtNpcMount_draw(dExtNpcMount_c* i_this) {
    if (i_this == NULL) {
        return 1;
    }
    // №178: DRAW-SIDE probe. §52 covers EXECUTE only, and execute is now proven
    // correct — bound, enables 0xFF, `at` == `demoWants` == -204886, and №177 sets
    // the base matrix from it. Yet the cast reads as unmoved on screen.
    //
    // Execute being right while the screen disagrees leaves exactly two options,
    // and §52 cannot tell them apart: either draw is NOT RUNNING for this actor
    // during the demo (culled, torn down, or skipped upstream), or it runs and the
    // model is drawn somewhere other than `current.pos`. This says which.
    //
    // Silence from this line during a demo == draw never happened, which is itself
    // the answer.
    // №183: log EVERY mount of this proc, bound or not, with its INSTANCE POINTER.
    //
    // `MODELmtx` came back -204886 — the model's real render matrix is at the
    // porch — while the user sees Aryll on the lookout. Those cannot both be the
    // same object, so there is a second Aryll-shaped thing and my "one actor"
    // claim was wrong.
    //
    // The old gate (`demoActorID != 0`) is exactly why I never saw it: it only
    // ever logged the BOUND performer, so an unbound duplicate standing at the
    // lookout was invisible to the probe that was supposed to find it. Two
    // distinct pointers on the same frame proves the duplicate; one pointer means
    // the second figure is drawn by something that is not a mount at all.
    if (dComIfGp_event_runCheck() && (g_Counter.mCounter0 % 60) == 0) {
        // №179: read the MODEL'S OWN matrix, not `current.pos`.
        //
        // Everything so far has traced the actor's logical position and it has
        // been correct at every checkpoint — bound, enables 0xFF, `at` ==
        // `demoWants` == -204886, draw running, not culled, morf present. The
        // screen still disagrees, so the remaining suspect is the last link
        // nobody has actually looked at: the matrix the RENDERER uses.
        //
        // `getBaseTRMtx()[*][3]` is the translation column actually handed to
        // J3D. If it reads -204886 the model is genuinely drawn at the porch and
        // the figure at the lookout is not this actor. If it reads -195205 then
        // something resets the matrix between №177 setting it in execute and the
        // draw consuming it — and THAT is the bug, not the position.
        f32 mx = 0.0f, my = 0.0f, mz = 0.0f;
        if (i_this->mpMorf != NULL && i_this->mpMorf->getModel() != NULL) {
            Mtx& bm = i_this->mpMorf->getModel()->getBaseTRMtx();
            mx = bm[0][3];
            my = bm[1][3];
            mz = bm[2][3];
        }
        DuskLog.info("[ExtWw] §53 demo DRAW '{}' ptr={} id={} arg={} unloading={} isBg={} "
                     "morf={} scale=({:.2f},{:.2f},{:.2f}) at=({:.0f},{:.0f},{:.0f}) "
                     "MODELmtx=({:.0f},{:.0f},{:.0f})",
                     i_this->mManifest.proc, (void*)i_this, (int)i_this->demoActorID,
                     (int)i_this->argument, roomLaneMountIsUnloading(i_this) ? 1 : 0,
                     i_this->mIsBg ? 1 : 0, i_this->mpMorf != NULL ? 1 : 0, i_this->scale.x,
                     i_this->scale.y, i_this->scale.z, i_this->current.pos.x,
                     i_this->current.pos.y, i_this->current.pos.z, mx, my, mz);
    }
    // №68: skip draw while room handles are being torn down.
    if (roomLaneMountIsUnloading(i_this)) {
        return 1;
    }
    if (i_this->mIsBg) {
        // №108: sky before island geometry (camera-follow; sky draw list).
        if (std::strcmp(i_this->mManifest.proc, "EXT_BG0") == 0 ||
            std::strcmp(i_this->mManifest.proc, "EXT_BG9") == 0) {
            wwSkyDraw();
        }
        // Obj_Fmobj-style draw on the normal actor list. setListBG from an NPC draw
        // priority is too late — the BG list was already flushed (invisible island).
        // No per-draw logging — Outset BG is already the FPS hot path.
        mDoLib_clipper::changeFar(1000000.0f);
        g_env_light.settingTevStruct(0x40, &i_this->current.pos, &i_this->tevStr);
        for (int i = 0; i < 3; ++i) {
            if (i_this->mpBgModels[i] == NULL) {
                continue;
            }
            J3DModel* model = i_this->mpBgModels[i];
            J3DModelData* data = model->getModelData();
            // §128 / daBg order: BTK entry before calc so MaterialAnm SRT is live
            // when calcMaterial/diff patches the DifferedDL (needs create 0x1200).
            if (i == 1 && i_this->mpBgBtk != NULL && data != NULL) {
                i_this->mpBgBtk->entry(data);
            }
            model->calc();
            if (data != NULL) {
                for (u16 s = 0; s < data->getShapeNum(); ++s) {
                    J3DShape* shape = data->getShapeNodePointer(s);
                    if (shape != NULL) {
                        shape->show();
                    }
                }
            }
            g_env_light.setLightTevColorType_MAJI(model, &i_this->tevStr);
            // §127: model1 beach-crash — authored C/K from Bridge dump (not pane seacolor).
            if (i == 1 && data != NULL) {
                wwApplyModel1SeaPalette(data);
            }
            mDoExt_modelUpdateDL(model);
            if (i == 1 && i_this->mpBgBtk != NULL && data != NULL) {
                i_this->mpBgBtk->remove(data);
            }
        }
        mDoLib_clipper::resetFar();
        return 1;
    }
    if (i_this->mpMorf == NULL) {
        return 1;
    }

    J3DModel* model = i_this->mpMorf->getModel();

    // Boots/leaf recipe (no MAJI): settingTevStruct(0) → NPC ambient → entryDL.
    // Ambient is mount-owned (manifest amb= + WREG_F(30..32)), NOT bow warm tint —
    // that 105/78/48 cast was the №14 orange bloom on skin/cloth.
    g_env_light.settingTevStruct(0, &i_this->current.pos, &i_this->tevStr);

    GXColor amb_col;
    amb_col.r = clampAmbChannel(i_this->mManifest.ambR, WREG_F(30));
    amb_col.g = clampAmbChannel(i_this->mManifest.ambG, WREG_F(31));
    amb_col.b = clampAmbChannel(i_this->mManifest.ambB, WREG_F(32));
    amb_col.a = 255;
    i_this->tevStr.AmbCol.r = amb_col.r;
    i_this->tevStr.AmbCol.g = amb_col.g;
    i_this->tevStr.AmbCol.b = amb_col.b;
    i_this->tevStr.AmbCol.a = amb_col.a;

    applyModelAmbient(model, amb_col);

    if (i_this->mBtpBound && i_this->mpBtp != NULL) {
        i_this->mpBtp->entry(model->getModelData());
    }
    // №194: idle blink only when the demo is NOT driving the face — otherwise it
    // would overwrite the demo expression (entered last = wins per material).
    if (i_this->mBlinkBound && i_this->mpBlink != NULL && !i_this->mDemoFaceActive) {
        i_this->mpBlink->entry(model->getModelData());  // №188
    }
    // №197: demo BTK (texture SRT) — shapes the open-mouth/tongue texture.
    if (i_this->mDemoBtkBound && i_this->mpDemoBtk != NULL && i_this->mDemoFaceActive) {
        i_this->mpDemoBtk->entry(model->getModelData());
    }
    if (i_this->mpBrk != NULL) {
        i_this->mpBrk->entry(model->getModelData());
    }
    if (i_this->mpColorBtk != NULL) {
        i_this->mpColorBtk->entry(model->getModelData());
    }

    dComIfGd_setList();
    // №50-E: Nintendo draws only the visual door mesh — skip the controller (white triangle).
    bool drawController = true;
    if (i_this->mManifest.doorAttention) {
        for (int i = 0; i < i_this->mAttachCount; ++i) {
            if (i_this->mpAttach[i] != NULL && i_this->mAttachSlave[i] == 0) {
                drawController = false;
                break;
            }
        }
    }
    if (drawController) {
        i_this->mpMorf->entryDL();
    }
    if (i_this->mpCompanion != NULL && i_this->mpCompanion->getModel() != NULL &&
        !i_this->mManifest.companionHidden) {  // №249: mounted but not presented
        applyModelAmbient(i_this->mpCompanion->getModel(), amb_col);
        i_this->mpCompanion->entryDL();
    }
    for (int i = 0; i < i_this->mAttachCount; ++i) {
        if (i_this->mpAttach[i] != NULL && mountAttachLive(i_this, i)) {  // №262
            applyModelAmbient(i_this->mpAttach[i], amb_col);
            i_this->mpAttach[i]->entry();
        }
    }

    // I3: simple ground shadow (no Acch — use up-normal).
    cXyz gndNrm(0.0f, 1.0f, 0.0f);
    dComIfGd_setSimpleShadow(&i_this->current.pos, i_this->current.pos.y, 50.0f * i_this->scale.x,
                             &gndNrm, 0, 1.0f, dDlst_shadowControl_c::getSimpleTex());
#if TARGET_PC_NATIVE_UI
    if (s_mountDialogue != NULL && s_mountDialogue->isVisible()) {
        s_mountDialogue->registerDraw();
    }
#endif
    return 1;
}

// №27 N4 helpers — operate on the nearest live ExtNpc mount to `from`.
namespace {

dExtNpcMount_c* findNearestMount(const cXyz& from, f32 maxDist) {
    dExtNpcMount_c* best = NULL;
    f32 bestD2 = maxDist * maxDist;
    // Walk tracked providers' live actors via name search is awkward; scan room players' peers
    // via fopAcIt is heavier. Use a simple iterative search of known stub IDs.
    struct Ctx {
        const cXyz* from;
        f32 bestD2;
        dExtNpcMount_c* best;
    } ctx{&from, bestD2, NULL};
    fopAcIt_Executor(
        [](void* actor, void* data) -> int {
            fopAc_ac_c* ac = (fopAc_ac_c*)actor;
            Ctx* c = (Ctx*)data;
            if (ac == NULL) {
                return 0;
            }
            const s16 name = fopAcM_GetName(ac);
            if (name != fpcNm_NPC_HENNA0_e && name != fpcNm_NPC_MK_e && name != fpcNm_NPC_P2_e &&
                name != fpcNm_NPC_KDK_e) {
                return 0;
            }
            dExtNpcMount_c* m = (dExtNpcMount_c*)ac;
            if (m->mIsBg || m->mpMorf == NULL) {
                return 0;
            }
            const f32 dx = ac->current.pos.x - c->from->x;
            const f32 dz = ac->current.pos.z - c->from->z;
            const f32 d2 = dx * dx + dz * dz;
            if (d2 < c->bestD2) {
                c->bestD2 = d2;
                c->best = m;
            }
            return 0;
        },
        &ctx);
    return ctx.best;
}

bool rewriteIdentityIni(const char* modFolder, const char* proc, const char* displayName) {
    if (modFolder == NULL || proc == NULL || displayName == NULL) {
        return false;
    }
    const fs::path path =
        dusk::ConfigPath / "model_replacements" / modFolder / "population" / "identity.ini";
    std::error_code ec;
    fs::create_directories(path.parent_path(), ec);
    // Read existing, replace/append [proc] display_name=.
    std::string existing;
    {
        std::ifstream in(path);
        if (in) {
            existing.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        }
    }
    const std::string header = std::string("[") + proc + "]";
    std::string out;
    bool replaced = false;
    std::istringstream iss(existing);
    std::string line;
    bool inSection = false;
    while (std::getline(iss, line)) {
        std::string trimmed = line;
        while (!trimmed.empty() && (trimmed.back() == '\r' || trimmed.back() == '\n')) {
            trimmed.pop_back();
        }
        if (!trimmed.empty() && trimmed[0] == '[') {
            if (inSection && !replaced) {
                out += "display_name=";
                out += displayName;
                out += "\n";
                replaced = true;
            }
            inSection = (trimmed == header);
            out += trimmed;
            out += "\n";
            continue;
        }
        if (inSection) {
            size_t eq = trimmed.find('=');
            std::string key = eq == std::string::npos ? trimmed : trimmed.substr(0, eq);
            while (!key.empty() && key.back() == ' ') {
                key.pop_back();
            }
            if (key == "display_name") {
                out += "display_name=";
                out += displayName;
                out += "\n";
                replaced = true;
                continue;
            }
        }
        out += trimmed;
        out += "\n";
    }
    if (inSection && !replaced) {
        out += "display_name=";
        out += displayName;
        out += "\n";
        replaced = true;
    }
    if (!replaced) {
        out += "\n";
        out += header;
        out += "\ndisplay_name=";
        out += displayName;
        out += "\n";
    }
    std::ofstream ofs(path, std::ios::trunc);
    if (!ofs) {
        return false;
    }
    ofs << out;
    return true;
}

}  // namespace

dExtNpcMount_c* dExtNpcMount_nearestDoorAttention(const cXyz& from, f32 maxDist) {
    // 0x7FFF = no facing preference (pure nearest).
    return dExtNpcMount_facedDoorAttention(from, (s16)0x7FFF, maxDist, NULL);
}

dExtNpcMount_c* dExtNpcMount_facedDoorAttention(const cXyz& from, s16 facingYaw, f32 maxDist,
                                                   f32* outDistXZ) {
    struct Ctx {
        const cXyz* from;
        s16 facingYaw;
        bool preferFaced;
        f32 maxD2;
        f32 bestFacedD2;
        f32 bestAnyD2;
        dExtNpcMount_c* bestFaced;
        dExtNpcMount_c* bestAny;
    };
    const bool preferFaced = (facingYaw != (s16)0x7FFF);
    Ctx ctx{&from,
            facingYaw,
            preferFaced,
            maxDist * maxDist,
            maxDist * maxDist,
            maxDist * maxDist,
            NULL,
            NULL};
    fopAcIt_Executor(
        [](void* actor, void* data) -> int {
            fopAc_ac_c* ac = (fopAc_ac_c*)actor;
            Ctx* c = (Ctx*)data;
            if (ac == NULL) {
                return 0;
            }
            const s16 name = fopAcM_GetName(ac);
            if (name != fpcNm_NPC_HENNA0_e && name != fpcNm_NPC_MK_e && name != fpcNm_NPC_P2_e &&
                name != fpcNm_NPC_KDK_e) {
                return 0;
            }
            dExtNpcMount_c* m = (dExtNpcMount_c*)ac;
            if (m->mIsBg || m->mpMorf == NULL || !m->mManifest.doorAttention) {
                return 0;
            }
            if (roomLaneMountIsUnloading(m)) {
                return 0;
            }
            const f32 dx = ac->current.pos.x - c->from->x;
            const f32 dy = ac->current.pos.y - c->from->y;
            const f32 dz = ac->current.pos.z - c->from->z;
            if (dy > 250.0f || dy < -150.0f) {
                return 0;
            }
            const f32 d2 = dx * dx + dz * dz;
            if (d2 > c->maxD2) {
                return 0;
            }
            if (d2 < c->bestAnyD2) {
                c->bestAnyD2 = d2;
                c->bestAny = m;
            }
            if (!c->preferFaced) {
                return 0;
            }
            cXyz to(dx, 0.0f, dz);
            if (to.abs() < 1.0f) {
                c->bestFacedD2 = d2;
                c->bestFaced = m;
                return 0;
            }
            const s16 yawTo = to.atan2sX_Z();
            if (cLib_distanceAngleS(yawTo, c->facingYaw) <= 0x4000 && d2 < c->bestFacedD2) {
                c->bestFacedD2 = d2;
                c->bestFaced = m;
            }
            return 0;
        },
        &ctx);
    dExtNpcMount_c* best =
        (ctx.preferFaced && ctx.bestFaced != NULL) ? ctx.bestFaced : ctx.bestAny;
    if (outDistXZ != NULL) {
        if (best == NULL) {
            *outDistXZ = -1.0f;
        } else if (best == ctx.bestFaced) {
            *outDistXZ = std::sqrt(ctx.bestFacedD2);
        } else {
            *outDistXZ = std::sqrt(ctx.bestAnyD2);
        }
    }
    return best;
}

bool dExtNpcMount_playAnimNearest(const cXyz& from, f32 maxDist, const char* bckName) {
    if (bckName == NULL || bckName[0] == '\0') {
        return false;
    }
    dExtNpcMount_c* best = NULL;
    f32 bestD2 = maxDist * maxDist;
    struct Ctx {
        const cXyz* from;
        f32 bestD2;
        dExtNpcMount_c* best;
        bool preferDoor;
    } ctx{&from, bestD2, NULL, true};
    // Prefer doorAttention mounts (Knob); fall back to any nearby mount.
    for (int pass = 0; pass < 2; ++pass) {
        ctx.preferDoor = (pass == 0);
        ctx.best = NULL;
        ctx.bestD2 = bestD2;
        fopAcIt_Executor(
            [](void* actor, void* data) -> int {
                fopAc_ac_c* ac = (fopAc_ac_c*)actor;
                Ctx* c = (Ctx*)data;
                if (ac == NULL) {
                    return 0;
                }
                const s16 name = fopAcM_GetName(ac);
                if (name != fpcNm_NPC_HENNA0_e && name != fpcNm_NPC_MK_e &&
                    name != fpcNm_NPC_P2_e && name != fpcNm_NPC_KDK_e) {
                    return 0;
                }
                dExtNpcMount_c* m = (dExtNpcMount_c*)ac;
                if (m->mIsBg || m->mpMorf == NULL) {
                    return 0;
                }
                if (c->preferDoor && !m->mManifest.doorAttention) {
                    return 0;
                }
                const f32 dx = ac->current.pos.x - c->from->x;
                const f32 dz = ac->current.pos.z - c->from->z;
                const f32 d2 = dx * dx + dz * dz;
                if (d2 < c->bestD2) {
                    c->bestD2 = d2;
                    c->best = m;
                }
                return 0;
            },
            &ctx);
        if (ctx.best != NULL) {
            best = ctx.best;
            break;
        }
    }
    if (best == NULL) {
        return false;
    }
    const char* anim = bckName;
    if (best->mManifest.doorOpenBck[0]) {
        anim = best->mManifest.doorOpenBck;
    }
    // №55: callers may pass NULL and rely on the manifest; no anim resolved = no-op.
    if (anim == NULL || anim[0] == '\0') {
        return false;
    }
    setMountAnimation(best, anim, J3DFrameCtrl::EMode_NONE);
    DuskLog.info("[ExtNpcMount] playAnimNearest '{}' on '{}' ({})", anim, best->mManifest.proc,
                 best->mManifest.model);
    return true;
}

bool dExtNpcMount_cycleHeadNearest(const cXyz& from, f32 maxDist) {
    dExtNpcMount_c* m = findNearestMount(from, maxDist);
    if (m == NULL) {
        return false;
    }
    // Cycle 1→2→3→1 among attach slot 0 if it looks like a numbered head.
    if (m->mManifest.attachCount <= 0 || m->mManifest.attach[0].model[0] == '\0') {
        DuskLog.warn("[ExtNpcMount] N4 cycle: nearest '{}' has no head attach", m->mManifest.proc);
        return false;
    }
    char model[64];
    std::snprintf(model, sizeof(model), "%s", m->mManifest.attach[0].model);
    char* dot = std::strstr(model, ".bdl");
    if (dot == NULL || dot < model + 2) {
        return false;
    }
    int idx = (dot[-2] - '0') * 10 + (dot[-1] - '0');
    if (idx < 1) {
        idx = 1;
    }
    idx += 1;
    if (idx > 8) {
        idx = 1;
    }
    // Rewrite digits in place.
    char prefix[64];
    const size_t preLen = (size_t)(dot - model - 2);
    if (preLen >= sizeof(prefix)) {
        return false;
    }
    std::memcpy(prefix, model, preLen);
    prefix[preLen] = '\0';
    char nextModel[64];
    std::snprintf(nextModel, sizeof(nextModel), "%s%02d.bdl", prefix, idx);
    // Probe arc for the next head; if missing, wrap to 01.
    if (dComIfG_getObjectRes(m->mManifest.arc, nextModel) == NULL) {
        idx = 1;
        std::snprintf(nextModel, sizeof(nextModel), "%s%02d.bdl", prefix, idx);
        if (dComIfG_getObjectRes(m->mManifest.arc, nextModel) == NULL) {
            return false;
        }
    }
    std::snprintf(m->mManifest.attach[0].model, sizeof(m->mManifest.attach[0].model), "%s",
                  nextModel);
    m->mHeadVariant = (u8)idx;
    // Rebuild attach slot 0 model on the actor solid heap path is hard mid-frame;
    // force a soft rebind via acquireMountedModel + replace J3DModel.
    void* raw = dComIfG_getObjectRes(m->mManifest.arc, nextModel);
    J3DModelData* data = acquireMountedModel(m->mManifest.arc, nextModel, raw);
    if (data == NULL) {
        return false;
    }
    J3DModel* modelObj = mDoExt_J3DModel__create(data, 0, 1);
    if (modelObj == NULL) {
        return false;
    }
    m->mpAttach[0] = modelObj;
    DuskLog.info("[ExtNpcMount] N4 cycle head '{}' → {} (var {})", m->mManifest.proc, nextModel,
                 idx);
    return true;
}

bool dExtNpcMount_setDisplayNameNearest(const cXyz& from, f32 maxDist, const char* name) {
    dExtNpcMount_c* m = findNearestMount(from, maxDist);
    if (m == NULL || name == NULL || name[0] == '\0') {
        return false;
    }
    std::snprintf(m->mManifest.displayName, sizeof(m->mManifest.displayName), "%s", name);
    auto it = s_providers.find(m->mManifest.proc);
    if (it != s_providers.end()) {
        std::snprintf(it->second.displayName, sizeof(it->second.displayName), "%s", name);
    }
    const bool ok =
        rewriteIdentityIni(m->mManifest.modFolder, m->mManifest.proc, name);
    DuskLog.info("[ExtNpcMount] N4 lock identity '{}' → '{}' ({})", m->mManifest.proc, name,
                 ok ? "identity.ini ok" : "identity.ini FAIL");
    return ok;
}

const char* dExtNpcMount_nearestDisplayName(const cXyz& from, f32 maxDist) {
    dExtNpcMount_c* m = findNearestMount(from, maxDist);
    if (m == NULL) {
        return "";
    }
    return m->mManifest.displayName[0] ? m->mManifest.displayName : m->mManifest.proc;
}

bool dExtNpcMount_isMountActor(const fopAc_ac_c* actor) {
    if (actor == NULL) {
        return false;
    }
    const s16 nm = fopAcM_GetName(const_cast<fopAc_ac_c*>(actor));
    if (nm != fpcNm_NPC_HENNA0_e && nm != fpcNm_NPC_MK_e && nm != fpcNm_NPC_P2_e &&
        nm != fpcNm_NPC_KDK_e) {
        return false;
    }
    const dExtNpcMount_c* m = static_cast<const dExtNpcMount_c*>(actor);
    return m->mIsBg || m->mpMorf != NULL || m->mManifest.valid;
}

// §41: expose census code already retained on the mount (no new state).
static void fillCensusNameFromSpawnSrc(const char* spawnSrc, char* out, size_t n) {
    if (out == NULL || n == 0) {
        return;
    }
    out[0] = '\0';
    if (spawnSrc == NULL || spawnSrc[0] == '\0') {
        return;
    }
    if (std::strncmp(spawnSrc, "census:", 7) == 0) {
        const char* start = spawnSrc + 7;
        const char* at = std::strchr(start, '@');
        size_t len = at != NULL ? static_cast<size_t>(at - start) : std::strlen(start);
        if (len >= n) {
            len = n - 1;
        }
        if (len > 0) {
            std::memcpy(out, start, len);
        }
        out[len] = '\0';
        return;
    }
    // Door / warm / other lanes — keep the raw source; an unexpected src is information.
    std::snprintf(out, n, "%s", spawnSrc);
}

bool dExtNpcMount_queryActor(const fopAc_ac_c* actor, dExtNpcIdentifyInfo* out) {
    if (out == NULL) {
        return false;
    }
    *out = {};
    if (!dExtNpcMount_isMountActor(actor)) {
        return false;
    }
    const dExtNpcMount_c* m = static_cast<const dExtNpcMount_c*>(actor);
    const dExtNpcManifest& man = m->mManifest;
    out->valid = true;
    std::snprintf(out->proc, sizeof(out->proc), "%s", man.proc[0] ? man.proc : "?");
    std::snprintf(out->displayName, sizeof(out->displayName), "%s",
                  man.displayName[0] ? man.displayName : "-");
    std::snprintf(out->modFolder, sizeof(out->modFolder), "%s",
                  man.modFolder[0] ? man.modFolder : "-");
    fillCensusNameFromSpawnSrc(m->mSpawnSrc, out->censusName, sizeof(out->censusName));
    out->socketArg = man.socketArg;
    out->headVariant = m->mHeadVariant;
    if (man.attachCount > 0 && man.attach[0].model[0]) {
        std::snprintf(out->headModel, sizeof(out->headModel), "%s", man.attach[0].model);
        std::snprintf(out->headJoint, sizeof(out->headJoint), "%s",
                      man.attach[0].joint[0] ? man.attach[0].joint : "head");
    }
    return true;
}

// §41: Z-target identity probe — one pointer compare/frame; log only on target change.
void dExtNpcMount_pollIdentifyProbe() {
    static fopAc_ac_c* s_lastLockTarget = NULL;
    static int s_identifySeq = 0;

    dAttention_c* attn = dComIfGp_getAttention();
    fopAc_ac_c* target = attn != NULL ? attn->LockonTarget(0) : NULL;
    if (target == s_lastLockTarget) {
        return;
    }
    s_lastLockTarget = target;
    if (target == NULL || !dExtNpcMount_isMountActor(target)) {
        return;
    }
    dExtNpcIdentifyInfo info{};
    if (!dExtNpcMount_queryActor(target, &info)) {
        return;
    }
    ++s_identifySeq;
    DuskLog.info("[ExtNpcId] #{} census={} proc={} arg={} head={} display={}", s_identifySeq,
                 info.censusName[0] ? info.censusName : "-", info.proc, info.socketArg,
                 info.headModel[0] ? info.headModel : "-",
                 info.displayName[0] ? info.displayName : "-");
}

// §95 — mount cull probe (ride-along with §97b). Env DUSK_CULL_PROBE=1.
void dExtNpcMount_pollCullProbe() {
    static bool s_enabledChecked = false;
    static bool s_enabled = false;
    static int s_heartbeat = 0;
    if (!s_enabledChecked) {
        s_enabledChecked = true;
        const char* env = std::getenv("DUSK_CULL_PROBE");
        s_enabled = env != NULL && env[0] == '1';
        if (s_enabled) {
            DuskLog.info("[CullProbe] §95 enabled (DUSK_CULL_PROBE=1)");
        }
    }
    if (!s_enabled) {
        return;
    }

    fopAc_ac_c* player = dComIfGp_getPlayer(0);
    if (player == NULL) {
        return;
    }

    struct CullCtx {
        const cXyz* playerPos;
        int heartbeat;
        int logged;
    } ctx{&player->current.pos, s_heartbeat, 0};

    fopAcIt_Executor(
        [](void* actor, void* data) -> int {
            fopAc_ac_c* ac = (fopAc_ac_c*)actor;
            CullCtx* c = (CullCtx*)data;
            if (ac == NULL || !dExtNpcMount_isMountActor(ac)) {
                return 0;
            }
            dExtNpcMount_c* m = (dExtNpcMount_c*)ac;
            // Prefer static props / trees (BG mounts use huge boxes already).
            if (m->mIsBg) {
                return 0;
            }
            const f32 dx = ac->current.pos.x - c->playerPos->x;
            const f32 dz = ac->current.pos.z - c->playerPos->z;
            if ((dx * dx + dz * dz) > (5000.0f * 5000.0f)) {
                return 0;
            }

            const bool culled = fopAcM_cullingCheck(ac) != 0;
            // Per-actor edge memory: stash prior in unused attention bit via static map keyed by id.
            static std::unordered_map<u32, bool> s_wasCulled;
            const u32 id = (u32)fopAcM_GetID(ac);
            const bool prev = s_wasCulled.count(id) ? s_wasCulled[id] : false;
            s_wasCulled[id] = culled;

            const bool edge = culled && !prev;
            const bool beat = (c->heartbeat % 600) == 0 && culled;
            if (!edge && !beat) {
                return 0;
            }

            dExtNpcIdentifyInfo info{};
            dExtNpcMount_queryActor(ac, &info);
            DuskLog.warn(
                "[CullProbe] {} proc={} census={} box=({:.0f},{:.0f},{:.0f})→({:.0f},{:.0f},{:.0f}) "
                "far={:.2f} pos=({:.0f},{:.0f},{:.0f})",
                edge ? "CULLED" : "still", info.proc[0] ? info.proc : "?",
                info.censusName[0] ? info.censusName : "-", ac->cull.box.min.x, ac->cull.box.min.y,
                ac->cull.box.min.z, ac->cull.box.max.x, ac->cull.box.max.y, ac->cull.box.max.z,
                fopAcM_getCullSizeFar(ac), ac->current.pos.x, ac->current.pos.y, ac->current.pos.z);
            c->logged++;
            return 0;
        },
        &ctx);

    ++s_heartbeat;
}

// --- №81 EXTENSION-FIRST: native save write refuse ---------------------------------

bool dExtWwSave_isWwHostStage(const char* stageName) {
    // Neutral fork prefixes under /res/Stage/: R_DL* (interiors) + F_DL* (fields).
    if (stageName == NULL || stageName[1] != '_' || stageName[2] != 'D' || stageName[3] != 'L') {
        return false;
    }
    return stageName[0] == 'R' || stageName[0] == 'F';
}

bool dExtWwSave_isWwContentActive() {
    if (s_lastBgProc[0] != '\0' && std::strncmp(s_lastBgProc, "EXT_", 4) == 0) {
        return true;
    }
    if (!s_roomLaneRooms.empty()) {
        return true;
    }
    for (const auto& kv : s_bgMountIds) {
        if (kv.first.rfind("EXT_", 0) == 0) {
            return true;
        }
    }
    return false;
}

bool dExtWwSave_refuseNativeWrite(const char* api, int stageNo, int bit) {
    if (!dExtWwSave_isWwContentActive()) {
        return false;
    }
    const char* stage = dComIfGp_getStartStageName();
    // On a WW host stage: still refuse until the extension router lands (no vanilla
    // slot is the WW progression home under №81). Placeholder STAG values must not
    // accumulate real chest/switch/key bits.
    if (dExtWwSave_isWwHostStage(stage)) {
        DuskLog.warn(
            "[WwSave] REFUSED {} stageNo={} bit={} host='{}' bg='{}' — WW host waits on "
            "extension store (№81)",
            api != NULL ? api : "?", stageNo, bit, stage != NULL ? stage : "?", s_lastBgProc);
        return true;
    }
    // Vanilla host (F_SP115=LANAYRU today): refuse — WW must not touch vanilla memBit.
    DuskLog.warn(
        "[WwSave] REFUSED {} stageNo={} bit={} host='{}' bg='{}' — WW on vanilla host (№81)",
        api != NULL ? api : "?", stageNo, bit, stage != NULL ? stage : "?", s_lastBgProc);
    return true;
}

#endif  // TARGET_PC
