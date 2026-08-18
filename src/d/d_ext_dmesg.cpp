// ============================================================================
// §308 NATIVE WW dMesg — M1 boot residency.
// Loads the six donor archives Foundry staged (R2 overlay → res/Object/<name>.arc)
// and latches them into module slots the ported dMesg subsystem reads. Does NOT
// touch TP's MsgDtArchive (that is TP dialogue text). Fonts are M1b.
// ============================================================================
// KIT-LINEAGE: mixed
// KIT-DONOR: per-hunk
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: per-hunk
#include "d/d_ext_dmesg.h"

#include "d/d_com_inf_game.h"
#include "d/d_resorce.h"
#include "d/d_drawlist.h"                 // §308 M3 dDlst_blo_c (BLO screen draw wrapper)
#include "d/d_ext_save_guard.h"           // dExtWwSave_isWwHostStage
#include "d/d_ext_ww_actor_shims.h"       // §324 fix: dExtWwMsg_textByMsgNo (donor mMsgNo space)
#include "d/d_demo.h"                     // §308 M4b dDemo_c::getControl()->unsuspend(1)
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_controller_pad.h"    // §308 M4b mDoCPd_c::getTrigA/B (box advance)
#include "m_Do/m_Do_audio.h"             // §311(b) mDoAud_messageSePlay (message SE tag)
#include "dusk/logging.h"
#include "dusk/settings.h"                // §308 M5 {Reconstructed / Native} toggle
#include "JSystem/JUtility/JUTResFont.h"
#include "JSystem/JKernel/JKRExpHeap.h"   // JKRExpHeap → JKRHeap upcast (font heap)
#include "JSystem/J2DGraph/J2DTextBox.h"  // §308 M3 J2DTextBox setFont/setString
#include "JSystem/J2DGraph/J2DPicture.h"  // §308 M3b J2DPicture::changeTexture (BTI art)
#include "JSystem/JUtility/TColor.h"       // §311(b) dotAnime black/white colour pulse
#include "JSystem/JKernel/JKRArchive.h"    // §308 M3 JKRArchive::getResSize
#include "JSystem/JKernel/JKRDecomp.h"     // §308 M3 Yaz0 decode (BLO is compressed)
#include "JSystem/JSupport/JSUMemoryStream.h" // §308 M3 JSUMemoryInputStream
#include <cstring>
#include <cstdio>   // §308 M4c snprintf (out-font digit substitution)
#include <string>   // §308 M4c page buffers
#include <vector>

static JKRArchive* s_dmsg = NULL;   // dmsgres
static JKRArchive* s_bmg = NULL;    // bmgres  (zel_00.bmg)
static JKRArchive* s_bmgh = NULL;   // bmgresh (zel_01.bmg)
static JKRArchive* s_menu = NULL;   // menures
static JUTFont* s_font = NULL;      // M1b rock_24_20_4i_usa.bfn (main text)
static JUTFont* s_rfont = NULL;     // M1b hyrule.bfn (ruby; asserted non-NULL)
static const u8* s_zel00 = NULL;    // M2 parsed zel_00.bmg base
// Donor RGBA8 palette from `bmgres/color.bmc` (CLT1 body +0x0C), or NULL when
// the member is absent/malformed — in which case `kWwColor` still draws. This
// points INTO the mounted archive and is never copied, so it must not outlive
// the mount; it is re-captured on every init alongside `s_zel00`.
static const u8* s_wwPalette = NULL;

// The donor is big-endian. A struct overlay would byte-swap silently and the
// failure would look like a corrupt palette rather than a byte-order bug.
static inline u32 dExtDmesg_be32(const u8* p) {
    return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | (u32)p[3];
}
static bool s_requested = false;
static bool s_ready = false;

// ============================================================================
// §308 M2 — manual BMG resolution (the port lacks JSystem JMessage that the
// donor's dMesg_tControl extends). Foundry §308 decode: zel_00.bmg = 'MESGbmg1',
// big-endian; INF1 entries are 24 bytes with the DAT1 text offset (u32) at +0 and
// the MESSAGE ID (u16) at +4; the STB setMessageCode value IS that id, resolved by
// search (id 539 → INF1 index 2001). getMessageEntry(id) → pointer into DAT1.
// ============================================================================
static u32 be32(const u8* p) { return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]; }
static u16 be16(const u8* p) { return (u16)((p[0] << 8) | p[1]); }

static const u8* bmgFindSection(const u8* bmg, const char* magic) {
    const u32 numSec = be32(bmg + 0x0C);   // section count
    const u8* p = bmg + 0x20;              // sections begin at 0x20
    for (u32 i = 0; i < numSec; i++) {
        if (std::memcmp(p, magic, 4) == 0) {
            return p;
        }
        const u32 secSize = be32(p + 4);
        if (secSize == 0) {
            break;
        }
        p += secSize;
    }
    return NULL;
}

// Resolve a BMG message id → its text bytes (into DAT1). NULL if unresolved.
const char* dExtDmesg_getMessageById(u16 id) {
    if (s_zel00 == NULL) {
        return NULL;
    }
    const u8* inf1 = bmgFindSection(s_zel00, "INF1");
    const u8* dat1 = bmgFindSection(s_zel00, "DAT1");
    if (inf1 == NULL || dat1 == NULL) {
        return NULL;
    }
    // The STB setMessageCode value IS the direct INF1 entry index (DIAG3 receipt:
    // entry[539] = "I've been waiting for you…"). Each 24-byte entry holds the DAT1
    // text offset (u32) at +0; text is plain 1-byte (control codes embedded).
    const u16 count = be16(inf1 + 0x08);
    if (id >= count) {
        return NULL;
    }
    const u8* e = inf1 + 0x10 + (u32)id * 24;
    const u32 off = be32(e + 0);
    return (const char*)(dat1 + 0x08 + off);  // DAT1 data starts after its 8-byte header
}

// §311(c) — read a byte field of INF1 entry `id` (JMSMesgEntry_c, f_op_msg_mng.h:22): the
// text offset is +0x00, mMsgNo +0x04, mTextboxType +0x0C (fukiKind), mItemImage +0x0F.
static u8 dExtDmesg_getMessageEntryByte(u16 id, u32 fieldOff) {
    if (s_zel00 == NULL) {
        return 0;
    }
    const u8* inf1 = bmgFindSection(s_zel00, "INF1");
    if (inf1 == NULL) {
        return 0;
    }
    const u16 count = be16(inf1 + 0x08);
    if (id >= count) {
        return 0;
    }
    return (inf1 + 0x10 + (u32)id * 24)[fieldOff];
}

// mTextboxType (fukiKind): 9 = get-item box, else talk box. Foundry §311 byte-receipt.
static u8 dExtDmesg_getMessageTextboxType(u16 id) { return dExtDmesg_getMessageEntryByte(id, 0x0C); }
// mItemImage: the item-resource index for a get-item beat (0x32 FUKU / 0x37 NEW_FUKU).
static u8 dExtDmesg_getMessageItemImage(u16 id)   { return dExtDmesg_getMessageEntryByte(id, 0x0F); }

// ----------------------------------------------------------------------------
// §324 FIX — TWO NUMBERING SPACES, and this file serves both. STB setMessageCode
// IS an INF1 index (receipt holds: no zel_00 entry has mMsgNo 539, so 539 could
// only be an index) — the STB path above stays index-keyed. An ACTOR's message
// id is an mMsgNo (donor getMessage 8002E4AC: linear scan comparing mMsgNo,
// skipping mDataOffs == 0). Feeding an mMsgNo to getMessageById resolved a
// wrong-but-valid entry, and because wrong-but-valid RETURNS TEXT it won over
// the correct catalog fallback — the postbox drew "Seven-Star Isles".
//
// This locator maps mMsgNo -> INF1 index so entry ATTRIBUTES (fukiKind) read
// from the entry actually addressed. TEXT is deliberately NOT resolved here:
// dExtWwMsg_textByMsgNo owns text and the Hylian archive selection with it,
// and that scan is not duplicated in a second place.
// ----------------------------------------------------------------------------
static int dExtDmesg_msgNoToIndex(u16 i_msgNo) {
    if (s_zel00 == NULL) {
        return -1;
    }
    const u8* inf1 = bmgFindSection(s_zel00, "INF1");
    if (inf1 == NULL) {
        return -1;
    }
    const u16 count = be16(inf1 + 0x08);
    for (u16 i = 0; i < count; i++) {
        const u8* e = inf1 + 0x10 + (u32)i * 24;
        if (be32(e + 0) == 0) {
            continue;  // mDataOffs == 0 — the donor's scan skips these (85 in zel_00)
        }
        if (be16(e + 0x04) == i_msgNo) {
            return (int)i;
        }
    }
    return -1;
}

// R2-overlaid arc names (Foundry §308/§311: mounted as res/Object/<name>.arc, stock loader).
// itemicon.arc holds the get-item textures (clothes.bti) — §311(c) staged donor-verbatim.
static const char* const kArcs[] = {
    "dmsgres", "bmgres", "bmgresh", "fontres", "rubyres", "menures", "itemicon",
};
static const int kArcNum = (int)(sizeof(kArcs) / sizeof(kArcs[0]));

static JKRArchive* getArc(const char* name) {
    dRes_info_c* info = dComIfG_getObjectResInfo(name);
    return info != NULL ? info->getArchive() : NULL;
}

bool dExtDmesg_ensureResident() {
    if (s_ready) {
        return true;
    }

    // Request all six once (async DVD load through the stock object-res path).
    if (!s_requested) {
        s_requested = true;
        for (int i = 0; i < kArcNum; i++) {
            dComIfG_setObjectRes(kArcs[i], 0, (JKRHeap*)NULL);
        }
        DuskLog.info("[dMesg] §308 M1 requested 6 archives (dmsgres/bmgres/bmgresh/"
                     "fontres/rubyres/menures) — awaiting residency");
        return false;
    }

    // Poll: every archive must be resident before we latch anything.
    for (int i = 0; i < kArcNum; i++) {
        if (getArc(kArcs[i]) == NULL) {
            return false;  // still loading — retry next frame
        }
    }

    // Latch the slots the ported dMesg reads. fontres/rubyres are fetched in M1b
    // (font load); we only confirm they are resident here.
    s_dmsg = getArc("dmsgres");
    s_bmg = getArc("bmgres");
    s_bmgh = getArc("bmgresh");
    s_menu = getArc("menures");
    if (s_dmsg == NULL || s_bmg == NULL || s_bmgh == NULL || s_menu == NULL) {
        return false;
    }

    // M1b — load the two WW fonts (JUTResFont from the resident arcs). dMsg_Create
    // asserts BOTH non-NULL even though the tale never renders ruby (§308 decode §5).
    {
        void* rf0 = dComIfG_getObjectRes("fontres", "rock_24_20_4i_usa.bfn");
        void* rf1 = dComIfG_getObjectRes("rubyres", "hyrule.bfn");
        if (rf0 != NULL && s_font == NULL) {
            s_font = new JUTResFont((ResFONT*)rf0, mDoExt_getZeldaHeap());
        }
        if (rf1 != NULL && s_rfont == NULL) {
            s_rfont = new JUTResFont((ResFONT*)rf1, mDoExt_getZeldaHeap());
        }
    }

    // M2 — capture zel_00.bmg base for id-resolution.
    s_zel00 = (const u8*)dComIfG_getObjectRes("bmgres", "zel_00.bmg");

    // ========================================================================
    // THE DONOR'S OWN COLOUR TABLE — `color.bmc`, the OTHER member of the
    // `bmgres` archive we already hold. No new mount and no new serve: this is
    // the consumer that was missing, not a new resource.
    //
    // WHY: `kWwColor` below is a hand-written approximation and SEVEN OF ITS
    // NINE ENTRIES DISAGREE with this file (measured against the user's own
    // disc: only white and blue match; e.g. red is FF6400 here and FF5A5A
    // there, cyan is 00FFFF here and 82FFFF there). Colour tags are 58.9% of
    // the 8,888 tags in the corpus, so the majority of WW coloured text has
    // been rendering in invented colours. The donor SHIPS this as data and
    // indexes it with INF1's `defaultColor`; approximating shipped donor data
    // is exactly what zero-bake forbids.
    //
    // FORMAT (measured): 'MGCLbmc1' · size field @0x08 in 32-BYTE UNITS
    // (34*32 = 1088, the second confirmation of that family convention after
    // the BMG header) · one 'CLT1' block at 0x20 · block body at +0x08 · a
    // 4-byte UNIDENTIFIED field there (observed 0x01000000) that is skipped
    // and deliberately NOT interpreted · palette base = block+0x0C, 256 x
    // RGBA8 big-endian.
    //
    // FALLS BACK TO `kWwColor`, IT DOES NOT REPLACE IT. If the member is
    // absent or malformed the old table still draws, so a bad archive degrades
    // to today's behaviour instead of to black text.
    // ========================================================================
    s_wwPalette = NULL;
    {
        const u8* bmc = (const u8*)dComIfG_getObjectRes("bmgres", "color.bmc");
        const char* why = "not requested";
        if (bmc == NULL) {
            why = "color.bmc absent from bmgres";
        } else if (memcmp(bmc, "MGCLbmc1", 8) != 0) {
            why = "not MGCLbmc1";
        } else {
            // Walk to CLT1 rather than assuming it is the first block.
            const u32 blocks = dExtDmesg_be32(bmc + 0x0C);
            u32 off = 0x20;
            why = "no CLT1 block";
            for (u32 i = 0; i < blocks; i++) {
                const u32 bsz = dExtDmesg_be32(bmc + off + 4);
                if (bsz == 0) {
                    why = "zero-size block (refused rather than spun)";
                    break;
                }
                if (memcmp(bmc + off, "CLT1", 4) == 0) {
                    if (bsz < 0x0C + 256 * 4) {
                        why = "CLT1 too small for 256 RGBA8 entries";
                    } else {
                        s_wwPalette = bmc + off + 0x0C;
                        why = "ok";
                    }
                    break;
                }
                off += bsz;
            }
        }
        DuskLog.info("[dMesg] §308 donor colour table: {} — {} (kWwColor is the "
                     "fallback, and it disagrees with the donor in 7 of 9 slots)",
                     s_wwPalette != NULL ? "LOADED" : "NOT LOADED", why);
    }

    s_ready = (s_font != NULL && s_rfont != NULL && s_zel00 != NULL);
    if (s_ready) {
        // Verify M2: id 539 = the tale greeting ("I've been waiting for you…").
        const char* t539 = dExtDmesg_getMessageById(539);
        char preview[41];
        preview[0] = '\0';
        if (t539 != NULL) {
            int n = 0;
            for (; n < 40 && t539[n] != '\0'; n++) {
                const unsigned char c = (unsigned char)t539[n];
                preview[n] = (c >= 0x20 && c < 0x7F) ? (char)c : '.';
            }
            preview[n] = '\0';
        }
        DuskLog.info("[dMesg] §308 M1b+M2 READY — fonts loaded (main+ruby), zel_00.bmg parsed. "
                     "getMessageById(539)={} preview=\"{}\"",
                     t539 != NULL ? "OK" : "NULL", preview);
    }
    return s_ready;
}

JKRArchive* dExtDmesg_getDmsgArchive() { return s_dmsg; }
JKRArchive* dExtDmesg_getMsgArchive() { return s_bmg; }
JKRArchive* dExtDmesg_getMsgHArchive() { return s_bmgh; }
JKRArchive* dExtDmesg_getMenuArchive() { return s_menu; }
JUTFont* dExtDmesg_getFont() { return s_font; }
JUTFont* dExtDmesg_getRFont() { return s_rfont; }

// §308 M3b / §311(c) — Yaz0-aware BTI load from `arc`: getObjectRes + decompress if
// compressed. Returns a persistent ResTIMG* (decompressed buffers leak for the box's life).
static const ResTIMG* dExtDmesg_loadTimg(const char* arc, const char* name) {
    void* comp = dComIfG_getObjectRes(arc, name);
    if (comp == NULL) {
        return NULL;
    }
    const u8* raw = (const u8*)comp;
    if (raw[0] == 'Y' && raw[1] == 'a' && raw[2] == 'z' && raw[3] == '0') {
        const u32 exp = JKRDecompExpandSize((u8*)comp);
        u8* buf = new u8[exp];
        JKRDecomp::decodeSZS((u8*)comp, buf, exp, 0);
        return (const ResTIMG*)buf;
    }
    return (const ResTIMG*)raw;
}

// ============================================================================
// §308 M3/M4b — the native WW box. Builds the donor hukidashi_d00.blo screen via
// the port's dDlst_blo_c wrapper, binds the main text panes (tx02/txt0) to the WW
// font + the beat's real zel_00 text, and arts the Yaz0-compressed BTIs (box body
// ms00, next arrow yz00, close dot dt00). back.bti is referenced by d00 but exists
// nowhere in the donor tree (§308 decode §4); the release JUT_ASSERT is inert, so
// create tolerates it.
//
// M4b makes it a real renderer with a lifecycle: setMessage() shows the box for a
// storyboard beat, update() waits for A/B and — on dismiss — releases the STB's
// per-beat suspend(1) via dDemo_c::getControl()->unsuspend(1), the donor's own
// box-close contract (d_mesg.cpp:2112 / d_msg_object.cpp:1414). In Native style the
// native box is the SOLE owner of that release (§201's TP box + owe-resume are
// suppressed in dExtWw_handleDemoMessage), so the suspend accounting stays balanced.
// ============================================================================
// §311(c) — TWO box screens, selected per beat by the INF1 entry's mTextboxType (fukiKind):
// 0 → hukidashi_d00 talk box; 9 → hukidashi_d09 get-item box (frame + item icon + ring/rays).
static dDlst_blo_c s_boxTalk;   // hukidashi_d00 (fukiKind 0)
static dDlst_blo_c s_boxItem;   // hukidashi_d09 (fukiKind 9)
static bool s_talkBuilt = false;
static bool s_itemBuilt = false;
static dDlst_blo_c* s_activeBox = &s_boxTalk;  // the box the current beat renders into
static u64 s_textTag0 = MULTI_CHAR('tx02');    // active box front text pane
static u64 s_textTag1 = MULTI_CHAR('txt0');    // active box shadow text pane
static u16 s_currentMsgId = 539;   // §308: the storyboard's current message (539 = default)
static bool s_boxActive = false;   // §308 M4b: a beat is presenting (drives draw + poll)
static bool s_boxSawSuspend = false;  // the STB has frozen the timeline for this beat
static int s_boxFrames = 0;        // frames the current beat has been up (backstops)
static J2DPane* s_paneArrow = NULL;   // active box next-page arrow (yz00/yz90)
static J2DPane* s_paneDot = NULL;     // active box close dot (dt00/dt90)
static J2DPane* s_paneItem = NULL;    // §311(c) get-item icon pane (item; NULL for talk box)
static std::vector<std::string> s_pages;  // §308 M4c: the beat's text, split into pages
static int s_pageIdx = 0;                 // current page (0-based)

// ============================================================================
// §324 LIVE TALK state. Talk mode reuses the box/page machinery above but has a
// completely different completion contract: no STB, no suspend accounting — the
// native box owns A/B, and on final-page dismissal it feeds ONE synthetic "A"
// into the TP dMsgObject's isSend() funnel (see d_msg_object.cpp §324 gate) so
// the TP state machine (which every WW actor's talk()/next_msgStatus polls)
// advances at native pace with its visuals suppressed.
// ============================================================================
static bool s_talkMode = false;        // the active box is a live-talk box (not STB)
static u32 s_dmFrame = 0;              // module frame tick (update() cadence)
static bool s_talkRelease = false;     // final page dismissed; synthetic "A" pending
static u32 s_talkReleaseFrame = 0;     // frame the release was armed (2-frame validity)
static u32 s_talkStickyFrame = (u32)-1;  // frame the release was consumed (same-frame reads stay true)
static int s_talkChainFrames = 0;      // §324 chain window after a release (CONTINUE re-open)
// §227: gap tolerance between consecutive lines of one conversation (frames).
// Re-armed on every accepted chain message, so conversation LENGTH is unbounded;
// this only covers the pause between one line and the next.
static const int kTalkChainWindowFrames = 90;

bool dExtDmesg_isBoxActive() { return s_boxActive; }  // §313 — a tale beat is presenting

static int s_arrowTimer = 0;  // §311(b) arwAnime tick (next-page arrow squash-stretch)
static int s_dotTimer = 0;    // §311(b) dotAnime tick (close-dot pulse)
static bool s_dotFadedIn = false;         // §311(b) dot alpha fade-in done → colour pulse
static JUtility::TColor s_dotBlack;       // dot picture's base black/white (pulse endpoints)
static JUtility::TColor s_dotWhite;

static const int kLineMax = 4;  // §308 M4c: WW linemax — 4 authored lines per page (d_mesg.cpp:45)

// ============================================================================
// §308/§311(a) — the WW dialogue-metrics HIO, ported as a named donor struct+consumer
// (NOT History constants). Foundry §311: the port never shipped WW's dMeter_msg_HIO_c —
// the only reference in the tree is this module — so there is nothing native to read;
// the donor's own ctor initializers (d_meter.cpp:371) ARE the authoritative retail
// values (HIO blocks are dev-tuning reflections with no data file to stage). The box
// applies them in dMesg_screenData_c::setCommonData at the END of createScreen (:1362),
// NOT per frame; the .blo baked TBX1 sizes are overridden here and are NOT authoritative.
// Porting the ctor + its consumer verbatim is what retires the "History-tuned" bridge:
// same numbers, but now sourced from the donor's own mechanism.
// ============================================================================
struct dExtDmesg_msgHIO_c {
    f32 mFontSize;      // field_0x70 = DEMO_SELECT(0x19,0x17) → RETAIL 23 (d_meter.cpp:427)
    f32 mCharSpace;     // field_0x5a = 0 (d_meter.cpp:380)
    f32 mRubyCharSpace; // field_0x5c = 0 (d_meter.cpp:381)
    f32 mLineSpace;     // field_0x5e = 0x1e = 30 (d_meter.cpp:382)
    f32 mRubySize;      // field_0x68 = 0xf = 15 (d_meter.cpp:384)
    // §317 (Foundry golden-trace) CORRECTION: retail is the SECOND DEMO_SELECT arg = 0x17 = 23;
    // 0x19=25 is the kiosk-DEMO value. Every box in the live donor run measured 23. The prior
    // 25 was a +2pt overrun — the true cause of the "text sometimes doesn't fit / mid-word wrap"
    // (not the box width; §316 widen was correctly reverted).
    dExtDmesg_msgHIO_c()
        : mFontSize(23.0f), mCharSpace(0.0f), mRubyCharSpace(0.0f),
          mLineSpace(30.0f), mRubySize(15.0f) {}
};
static dExtDmesg_msgHIO_c g_msgHIO;  // the WW-side g_msgHIO the port lacked

// §308 M4c — WW dialogue colour table (system tag 0xFF/0, d_mesg.cpp:688-698), RGBA8.
static const u32 kWwColor[9] = {
    0xFFFFFFFF,  // 0 white
    0xFF5A5AFF,  // 1 red
    0x78FF78FF,  // 2 green
    0x7878FFFF,  // 3 blue
    0xFFFF32FF,  // 4 yellow
    0x82FFFFFF,  // 5 cyan
    0xDC6EFFFF,  // 6 purple
    0xA0A0A0FF,  // 7 grey
    0xFF8200FF,  // 8 orange
};

static void dExtDmesg_bindText(const char* txt);  // defined below (font-bind both text panes)

// ============================================================================
// §308 M4c — port of dMesg_tSequenceProcessor's byte walk (d_mesg.cpp:303-761).
// WW dialogue text is a JMessage stream, NOT plain text: authored 0x0A line breaks,
// a 0x00 terminator, and 0x1A control tags (1A <size> <group> <codeHi><codeLo>
// <payload> — processor.cpp:141). WW does NO auto-wrap: every line break is authored,
// so a "page" is simply kLineMax authored lines. We consume tags by their size byte
// (substituting only the player-name and out-font-digit tags that carry visible text),
// honor 0x0A/0x00, and split every kLineMax lines into a page for the box to feed A/B.
// Colour/font-size/ruby/wait/sound/camera/anime tags are consumed silently for now
// (colour + timed-wait are the tracked M4c follow-ups).
// ============================================================================
static void dExtDmesg_paginate(const char* text, std::vector<std::string>& pages) {
    pages.clear();
    if (text == NULL) {
        return;
    }
    const u8* p = (const u8*)text;
    std::string page;
    int linesInPage = 0;  // authored newlines already in the current page (0..kLineMax-1)
    while (*p != 0x00) {
        const u8 c = *p;
        if (c == 0x1A) {
            const u8 size = p[1];
            const u8 group = p[2];
            const u16 code = (u16)((p[3] << 8) | p[4]);
            if (group == 0x00 && code == 0x00) {
                const char* nm = dComIfGs_getPlayerName();  // player-name (d_mesg.cpp:457)
                if (nm != NULL) {
                    page.append(nm);
                }
            } else if (group == 0x00 && code >= 10 && code <= 29) {
                char num[8];                                 // out-font digit (d_mesg.cpp:556)
                std::snprintf(num, sizeof(num), "%d", (int)code - 10);
                page.append(num);
            } else if (group == 0xFF && code == 0x00 && size >= 6) {
                // §308 M4c colour (system tag 0xFF/0, payload[0]=index — d_mesg.cpp:711).
                // Emit the J2D char+gradient colour escape the port's J2DPrint understands
                // (same format as jmessage_tRenderingProcessor::do_color, d_msg_class.cpp:3526).
                const u8 ci = p[5];
                // DONOR DATA FIRST, hardcode second (see the capture in init).
                // The donor table is a full 256 entries, so it needs no `< 9`
                // guard and no white default — indices 9-255 ARE white in the
                // shipped file, which is data rather than a rule we impose.
                // `kWwColor` remains as the labelled fallback for when the
                // member is missing; it is NOT deleted, and it is NOT correct
                // (7 of its 9 entries disagree with the donor).
                const u32 col = (s_wwPalette != NULL)
                    ? dExtDmesg_be32(s_wwPalette + (u32)ci * 4)
                    : ((ci < 9) ? kWwColor[ci] : 0xFFFFFFFFu);
                char esc[40];
                std::snprintf(esc, sizeof(esc), "\x1B" "CC[%08x]" "\x1B" "GC[%08x]", col, col);
                page.append(esc);
            } else if (group == 0x01) {
                // §311(b) message SE (group 1 → mDoAud_messageSePlay, d_mesg.cpp:614). The tale
                // authors one (0x00A7 in row 539). Fired at beat open — no typewriter to time it
                // to, so it plays when the box appears (close enough; the tale's SE is up front).
                mDoAud_messageSePlay(code, NULL, 0);
            }
            // Consumed silently (correct for the tale — Foundry §311 census: zero camera/anime
            // tags; the timed-waits are group-0 code-7 PAUSES, which are no-ops without a
            // typewriter): font-size (0xFF/1,6), ruby (0xFF/2), timed-wait (0/4,0/7), camera
            // (group 2), anime (group 3).
            p += (size < 5) ? 1 : size;  // malformed guard: never advance 0 (would loop)
            continue;
        }
        if (c == 0x0A) {
            if (linesInPage >= kLineMax - 1) {
                pages.push_back(page);   // page is full — this newline is the separator
                page.clear();
                linesInPage = 0;
            } else {
                page.push_back('\n');
                linesInPage++;
            }
            p++;
            continue;
        }
        page.push_back((char)c);
        p++;
    }
    if (!page.empty()) {
        pages.push_back(page);
    }
}

// Bind page `idx` to the box + set the arrow-vs-dot indicator (dMesg_outnowProc:
// more pages → next-arrow yz00; last page → close-dot dt00, d_mesg.cpp:2022).
static void dExtDmesg_showPage(int idx) {
    if (idx < 0 || idx >= (int)s_pages.size()) {
        return;
    }
    dExtDmesg_bindText(s_pages[idx].c_str());
    const bool lastPage = (idx + 1 >= (int)s_pages.size());
    if (s_paneArrow != NULL) {
        if (lastPage) {
            s_paneArrow->hide();
        } else {
            s_paneArrow->show();
            s_paneArrow->scale(1.0f, 1.0f);
            s_arrowTimer = 0;  // §311(b) restart the arrow bounce for this page
        }
    }
    if (s_paneDot != NULL) {
        if (lastPage) {
            s_paneDot->show();
            s_dotTimer = 0;    // §311(b) restart the close-dot fade-in + pulse
            s_dotFadedIn = false;
        } else {
            s_paneDot->hide();
        }
    }
}

// §311(a) — port of dMesg_screenData_c::setCommonData (d_mesg.cpp:1163) for the box's text
// panes: bind the WW font + apply g_msgHIO font/char/line size, overriding the .blo baked
// TBX1 sizes. Run ONCE at box build (end of createScreen), per the donor.
static void dExtDmesg_setCommonData() {
    J2DPane* panes[2] = {s_activeBox->getPane(s_textTag0), s_activeBox->getPane(s_textTag1)};
    for (int i = 0; i < 2; i++) {
        if (panes[i] == NULL) {
            continue;
        }
        J2DTextBox* tb = (J2DTextBox*)panes[i];
        tb->setFont(s_font);
        tb->setFontSize(g_msgHIO.mFontSize, g_msgHIO.mFontSize);
        tb->setCharSpace(g_msgHIO.mCharSpace);
        tb->setLineSpace(g_msgHIO.mLineSpace);
        // §316 DIAGNOSTIC ONLY (no layout change) — WW wraps at the pane bounds like the port
        // does (J2DTextBox::drawSelf, :417), and authors its lines to fit; a mid-word wrap here
        // ("the you|ng") means the port renders wider than the donor at the same nominal metrics.
        // Log the real values to find the true cause (font size actually applied? bounds width?
        // font cell width?) rather than widening the box (a non-vanilla layout we must not add).
        {
            static bool s_logged = false;
            if (!s_logged) {
                s_logged = true;
                J2DTextBox::TFontSize fs;
                tb->getFontSize(fs);
                DuskLog.info("[dMesg] §316 textbox metrics — setFont={} realFontXY={}x{} boundsW={} "
                             "hbind={} cellW={}", (int)g_msgHIO.mFontSize, (int)fs.mSizeX,
                             (int)fs.mSizeY, (int)tb->getBounds().getWidth(),
                             (int)tb->getHBinding(),
                             s_font != NULL ? (int)s_font->getCellWidth() : -1);
            }
        }
    }
}

static void dExtDmesg_bindText(const char* txt) {
    // Metrics + font are set once by setCommonData at build; per page we only push text.
    J2DPane* panes[2] = {s_activeBox->getPane(s_textTag0), s_activeBox->getPane(s_textTag1)};
    for (int i = 0; i < 2; i++) {
        if (panes[i] != NULL) {
            ((J2DTextBox*)panes[i])->setString(txt != NULL ? txt : "");
        }
    }
}

// §311(c) — art one picture pane: decompress its (Yaz0) BTI + changeTexture, else hide it.
// EVERY J2DPicture pane in the screen must be arted or hidden — a pane left with its baked
// compressed texture reads a garbage size → CreateTexture(-1) GPU crash (the M3b lesson).
static bool dExtDmesg_artPane(dDlst_blo_c* box, u64 tag, const char* arc, const char* bti) {
    J2DPane* pp = box->getPane(tag);
    if (pp == NULL || pp->getTypeID() != 0x12) {  // 0x12 = J2DPicture
        return false;
    }
    const ResTIMG* timg = dExtDmesg_loadTimg(arc, bti);
    if (timg != NULL) {
        ((J2DPicture*)pp)->changeTexture(timg, 0);
        pp->show();
        return true;
    }
    pp->hide();  // no texture → hide to avoid the garbage-size crash
    return false;
}

// Build the box for `fukiKind` (0 talk / 9 item) once, decompress + setPriority + art its
// panes, and latch it as the active box (pane pointers + text tags + metrics). Idempotent.
static bool dExtDmesg_ensureBoxBuilt(int fukiKind) {
    const bool item = (fukiKind == 9);
    dDlst_blo_c* box = item ? &s_boxItem : &s_boxTalk;
    bool* built = item ? &s_itemBuilt : &s_talkBuilt;
    const char* blo = item ? "hukidashi_d09.blo" : "hukidashi_d00.blo";

    // Latch the active box + its text/arrow/dot/item tags every call (so the helpers target
    // the right screen even when it was already built and we're just switching back to it).
    s_activeBox = box;
    s_textTag0 = item ? MULTI_CHAR('tx90') : MULTI_CHAR('tx02');
    s_textTag1 = item ? MULTI_CHAR('tx92') : MULTI_CHAR('txt0');
    const u64 arrowTag = item ? MULTI_CHAR('yz90') : MULTI_CHAR('yz00');
    const u64 dotTag   = item ? MULTI_CHAR('dt90') : MULTI_CHAR('dt00');

    if (!*built) {
        // The BLO is Yaz0-compressed in dmsgres — decompress once into a persistent buffer,
        // then load the screen from the raw stream (getObjectRes does NOT auto-decompress).
        void* comp = dComIfG_getObjectRes("dmsgres", blo);
        if (comp == NULL) {
            return false;
        }
        const u8* raw = (const u8*)comp;
        const u32 rawSize = s_dmsg->getResSize(comp);
        const bool isYaz0 = (raw[0] == 'Y' && raw[1] == 'a' && raw[2] == 'z' && raw[3] == '0');
        const u8* bloData = raw;
        u32 bloSize = rawSize;
        if (isYaz0) {
            const u32 exp = JKRDecompExpandSize((u8*)comp);
            u8* buf = new u8[exp];  // persistent for the screen's life
            JKRDecomp::decodeSZS((u8*)comp, buf, exp, 0);
            bloData = buf;
            bloSize = exp;
        }
        JSUMemoryInputStream stream((void*)bloData, bloSize);
        if (!box->getScreen()->setPriority(&stream, 0x20000, s_dmsg)) {
            DuskLog.warn("[dMesg] §308 setPriority FAILED for {}", blo);
            return false;
        }
        *built = true;

        // §308 M3b / §311(c) — art every picture pane (all Yaz0-compressed). Shared frame/
        // arrow/dot BTIs live in dmsgres; d09 adds ring (ring_loght_d) + rays (back_light_d).
        // back.bti (d00's unnamed pane) exists nowhere in the donor tree (decode §4) → its
        // pane stays NULL/tolerated. Item icon is swapped in setMessage (per-beat item id).
        int arted = 0;
        if (item) {
            arted += dExtDmesg_artPane(box, MULTI_CHAR('ms90'), "dmsgres", "hukidashi_00_d.bti");
            arted += dExtDmesg_artPane(box, MULTI_CHAR('yz90'), "dmsgres", "yazirushi_00_d.bti");
            arted += dExtDmesg_artPane(box, MULTI_CHAR('dt90'), "dmsgres", "dot_02_d.bti");
            // ring flare (rin1-4) + 8 light rays (lig1-8) — the get-item radiance.
            const u64 rings[4] = {MULTI_CHAR('rin1'), MULTI_CHAR('rin2'),
                                  MULTI_CHAR('rin3'), MULTI_CHAR('rin4')};
            for (int r = 0; r < 4; r++) {
                arted += dExtDmesg_artPane(box, rings[r], "dmsgres", "ring_loght_d.bti");
            }
            const u64 rays[8] = {MULTI_CHAR('lig1'), MULTI_CHAR('lig2'), MULTI_CHAR('lig3'),
                                 MULTI_CHAR('lig4'), MULTI_CHAR('lig5'), MULTI_CHAR('lig6'),
                                 MULTI_CHAR('lig7'), MULTI_CHAR('lig8')};
            for (int r = 0; r < 8; r++) {
                arted += dExtDmesg_artPane(box, rays[r], "dmsgres", "back_light_d.bti");
            }
        } else {
            arted += dExtDmesg_artPane(box, MULTI_CHAR('ms00'), "dmsgres", "hukidashi_00_d.bti");
            arted += dExtDmesg_artPane(box, MULTI_CHAR('yz00'), "dmsgres", "yazirushi_00_d.bti");
            arted += dExtDmesg_artPane(box, MULTI_CHAR('dt00'), "dmsgres", "dot_02_d.bti");
        }
        DuskLog.info("[dMesg] §311 box BUILT — {} + {} BTI panes arted", blo, arted);
    }

    // (re)resolve the active-box pane pointers + apply the WW metrics.
    s_paneArrow = box->getPane(arrowTag);
    s_paneDot   = box->getPane(dotTag);
    s_paneItem  = item ? box->getPane(MULTI_CHAR('item')) : NULL;
    if (s_paneDot != NULL && s_paneDot->getTypeID() == 0x12) {  // §311(b) dotAnime endpoints
        s_dotBlack = ((J2DPicture*)s_paneDot)->getBlack();
        s_dotWhite = ((J2DPicture*)s_paneDot)->getWhite();
    }
    dExtDmesg_setCommonData();  // §311(a) — WW HIO metrics onto this box's text panes
    return true;
}

void dExtDmesg_drawTestBox() {
    if (!s_ready) {
        return;
    }
    const char* stg = dComIfGp_getStartStageName();
    if (stg == NULL || !dExtWwSave_isWwHostStage(stg)) {
        return;
    }
    // §308 M5: the native WW box only exists when the WW-dialogue toggle = Native.
    if (dusk::getSettings().game.wwDialogue.getValue() != dusk::WwDialogueStyle::Native) {
        return;
    }
    // §308 M4b: lifecycle-driven — draw ONLY while a beat is presenting (no longer the
    // always-on M3 overlay). The box shows on setMessage, hides on dismiss (update).
    if (!s_boxActive || s_activeBox == NULL) {
        return;
    }
    dComIfGd_set2DOpaTop(s_activeBox);
}

// §308 M4 — the storyboard drives the box's message. Called from the demo's message
// adaptor on WW host stages. M4b: in Native style this SHOWS the box for the beat and
// binds the donor zel_00 text directly (539→540→3095→…). Reconstructed style ignores it
// (§201's TP box owns display); we still latch s_currentMsgId so a mid-cutscene toggle
// picks up the right text.
void dExtDmesg_setMessage(unsigned short id) {
    s_currentMsgId = id;
    if (dusk::getSettings().game.wwDialogue.getValue() != dusk::WwDialogueStyle::Native) {
        return;
    }
    if (!s_ready) {
        return;
    }
    const char* stg = dComIfGp_getStartStageName();
    if (stg == NULL || !dExtWwSave_isWwHostStage(stg)) {
        return;
    }
    // §311(c) — pick the box by the INF1 entry's mTextboxType (fukiKind): 9 = get-item box
    // (hukidashi_d09), else talk box (hukidashi_d00). Byte-receipted by Foundry §311: 9 on
    // the clothes-get rows (3095/4410), 0 on every talk row.
    const u8 fukiKind = dExtDmesg_getMessageTextboxType(id);
    if (!dExtDmesg_ensureBoxBuilt((int)fukiKind)) {
        return;
    }
    // §311(c) — for the item box, swap the placeholder icon for the beat's real item texture.
    // Item id = mItemImage (INF1 +0x0F); for the clothes-get (0x32 FUKU / 0x37 NEW_FUKU) the
    // donor texture is clothes.bti in the staged itemicon.arc (readTypeResource, d_mesg.cpp:1540).
    if (fukiKind == 9 && s_paneItem != NULL && s_paneItem->getTypeID() == 0x12) {
        const u8 itemImage = dExtDmesg_getMessageItemImage(id);
        const char* bti = (itemImage == 0x32 || itemImage == 0x37) ? "clothes.bti" : NULL;
        if (bti != NULL) {
            const ResTIMG* timg = dExtDmesg_loadTimg("itemicon", bti);
            if (timg != NULL) {
                ((J2DPicture*)s_paneItem)->changeTexture(timg, 0);
                s_paneItem->show();
            } else {
                s_paneItem->hide();
                DuskLog.warn("[dMesg] §311(c) item icon '{}' unresolved in itemicon.arc", bti);
            }
        } else {
            s_paneItem->hide();  // no mapping yet for this item id → flagged debt, not a guess
            DuskLog.warn("[dMesg] §311(c) no icon mapping for mItemImage=0x{:X} (id={})",
                         (int)itemImage, (int)id);
        }
    }
    // §308 M4c — parse the WW BMG stream into pages (tags consumed, 0x0A/0x00 honored,
    // kLineMax lines per page). One STB message code = one message = possibly many pages;
    // the STB fires ONE suspend(1) for the whole message, released only when the LAST page
    // is dismissed (update), so the suspend accounting stays 1:1 across the page chain.
    const char* txt = dExtDmesg_getMessageById(id);
    dExtDmesg_paginate(txt, s_pages);
    if (s_pages.empty()) {
        // No authored text but the STB still suspended → show an empty box so A/B can
        // release it; never strand the tale on a missing/blank entry.
        s_pages.push_back("");
    }
    s_pageIdx = 0;
    dExtDmesg_showPage(0);
    s_boxActive = true;
    s_talkMode = false;  // §324: an STB beat always owns the box in STB mode (a live
                         // talk box being open when a cutscene fires cedes to it)
    s_boxSawSuspend = false;
    s_boxFrames = 0;
    DuskLog.info("[dMesg] §311 beat id={} fuki={} text={} pages={} → {} box shown", (int)id,
                 (int)fukiKind, txt != NULL ? "OK" : "NULL", (int)s_pages.size(),
                 fukiKind == 9 ? "ITEM" : "talk");
}

// §311(b) — port of dMesg_screenData_c::arwAnime (d_mesg.cpp:1231): the next-page arrow
// squash-stretches on a ~76-frame cycle (rest 0-60, bounce 60-76, repeat). Faithful scale
// tables; applied to the raw J2DPane via setScale (the donor animates a dMesg_pane wrapper,
// but the visible scale curve is identical).
// KIT-DONOR-HUNK: d/d_mesg.cpp NonMatching
static void dExtDmesg_arwAnime() {
    if (s_paneArrow == NULL || !s_paneArrow->isVisible()) {
        return;
    }
    // KIT-DONOR-DATA: 20 lookup-table d/d_mesg.cpp:1231 arwAnime scale table
    static const f32 sx[5] = {1.0f, 1.3f, 0.8f, 1.2f, 1.0f};
    // KIT-DONOR-DATA: 20 lookup-table d/d_mesg.cpp:1231 arwAnime scale table
    static const f32 sy[5] = {1.0f, 0.3f, 1.1f, 0.8f, 1.0f};
    static const int step[5] = {60, 67, 71, 74, 76};
    ++s_arrowTimer;
    const int t = s_arrowTimer;
    f32 fx = 1.0f, fy = 1.0f;
    if (t > step[0]) {
        int i = 1;
        while (i < 5 && t > step[i]) {
            ++i;
        }
        if (i >= 5) {
            s_arrowTimer = 0;  // cycle complete → back to rest
        } else {
            const f32 span = (f32)(step[i] - step[i - 1]);
            f32 u = (f32)(t - step[i - 1]);
            u = (u * u) / (span * span);
            fx = u * (sx[i] - sx[i - 1]) + sx[i - 1];
            fy = u * (sy[i] - sy[i - 1]) + sy[i - 1];
        }
    }
    s_paneArrow->scale(fx, fy);
}
// KIT-DONOR-HUNK-END

// §311(b) — full port of dMesg_screenData_c::dotAnime (d_mesg.cpp:1284): the close-dot fades
// in over 10 frames (setAlpha), then colour-pulses on a 60-frame cycle — its black/white
// endpoints ramp toward (150,150,150)/(255,255,220) over 30f and back over 30f (setBlack/
// setWhite on the picture). Endpoints are the dot's own base colours captured at build.
static u8 dExtDmesg_lerp8(f32 base, f32 target, f32 t) {
    const f32 v = base + (target - base) * t;
    return (u8)(v < 0.0f ? 0.0f : (v > 255.0f ? 255.0f : v));
}
static void dExtDmesg_dotAnime() {
    if (s_paneDot == NULL || !s_paneDot->isVisible() || s_paneDot->getTypeID() != 0x12) {
        return;
    }
    J2DPicture* dot = (J2DPicture*)s_paneDot;
    ++s_dotTimer;
    if (!s_dotFadedIn) {
        s_paneDot->setAlpha((u8)(255 * (s_dotTimer < 10 ? s_dotTimer : 10) / 10));
        if (s_dotTimer >= 10) {
            s_dotFadedIn = true;
            s_dotTimer = 0;
        }
        return;
    }
    if (s_dotTimer >= 60) {  // cycle end → snap back to base, restart
        dot->setBlack(s_dotBlack);
        dot->setWhite(s_dotWhite);
        s_dotTimer = 0;
        return;
    }
    // valueIncrease(30, t, 0) = t/30 linear ramp; up for 0-30, mirrored 30-60.
    const f32 t = (s_dotTimer > 30) ? ((f32)(60 - s_dotTimer) / 30.0f) : ((f32)s_dotTimer / 30.0f);
    JUtility::TColor black, white;
    black.r = dExtDmesg_lerp8(s_dotBlack.r, 150.0f, t);
    black.g = dExtDmesg_lerp8(s_dotBlack.g, 150.0f, t);
    black.b = dExtDmesg_lerp8(s_dotBlack.b, 150.0f, t);
    black.a = s_dotBlack.a;
    white.r = dExtDmesg_lerp8(s_dotWhite.r, 255.0f, t);
    white.g = dExtDmesg_lerp8(s_dotWhite.g, 255.0f, t);
    white.b = dExtDmesg_lerp8(s_dotWhite.b, 220.0f, t);
    white.a = s_dotWhite.a;
    dot->setBlack(black);
    dot->setWhite(white);
}

// §308 M4b — per-frame lifecycle. Advance/dismiss on A/B and release the STB's per-beat
// suspend(1) on dismiss, mirroring the donor box close (getControl()->unsuspend(1)).
// §324: also hosts the live-talk branch (no STB coupling) — see the talk block below.
void dExtDmesg_update() {
    // §324 frame tick + release aging: the synthetic "A" stays valid for 2 update
    // frames (covers either execute order between this poll and the fopMsg proc),
    // then expires unconsumed.
    ++s_dmFrame;
    if (s_talkRelease && s_dmFrame - s_talkReleaseFrame > 2) {
        s_talkRelease = false;
    }
    if (s_talkChainFrames > 0) {
        --s_talkChainFrames;
    }
    if (!s_boxActive) {
        return;
    }
    if (dusk::getSettings().game.wwDialogue.getValue() != dusk::WwDialogueStyle::Native) {
        if (s_talkMode) {
            dExtDmesg_cancelTalk("style toggled mid-talk");
        }
        return;
    }
    dExtDmesg_arwAnime();  // §311(b) animate the page indicators while the box is up
    dExtDmesg_dotAnime();
    // ========================================================================
    // §324 LIVE-TALK branch: player-paced, no STB contract. A/B advances pages;
    // the final dismissal arms the one-shot release that isSend() feeds to the
    // TP dMsgObject. A generous backstop drops a stranded box (e.g. the talk
    // event force-ended underneath us) without releasing anything.
    // ========================================================================
    if (s_talkMode) {
        ++s_boxFrames;
        const bool tPressed =
            mDoCPd_c::getTrigA(PAD_1) != 0 || mDoCPd_c::getTrigB(PAD_1) != 0 ||
            mDoCPd_c::getTrigA(PAD_2) != 0 || mDoCPd_c::getTrigB(PAD_2) != 0 ||
            mDoCPd_c::getTrigA(PAD_3) != 0 || mDoCPd_c::getTrigB(PAD_3) != 0;
        if (tPressed) {
            if (s_pageIdx + 1 < (int)s_pages.size()) {
                ++s_pageIdx;
                dExtDmesg_showPage(s_pageIdx);
                s_boxFrames = 0;
                DuskLog.info("[dMesg] §324 talk page → {}/{} (id={})", s_pageIdx + 1,
                             (int)s_pages.size(), (int)s_currentMsgId);
            } else {
                s_boxActive = false;
                s_talkMode = false;
                s_talkRelease = true;
                s_talkReleaseFrame = s_dmFrame;
                s_talkStickyFrame = (u32)-1;
                // ============================================================
                // §227 — the CONTINUE-chain window was 10 frames (~1/6 s).
                // An actor whose next message arrives later than that (a talk
                // proc with a state delay, an event beat, a slower flow) missed
                // it and the conversation fell back to a TP box mid-sentence —
                // the postbox's "first line WW, rest TP" symptom.
                //
                // The window is re-armed on every accepted chain message (see
                // dExtDmesg_rearmTalkChain), so a long conversation stays on
                // the native box for its whole length rather than racing a
                // one-shot timer. This value is only the gap tolerance BETWEEN
                // consecutive lines, not a budget for the conversation.
                // ============================================================
                s_talkChainFrames = kTalkChainWindowFrames;
                DuskLog.info("[dMesg] §324 talk dismissed id={} (last of {} pages) → "
                             "synthetic A to TP dMsgObject", (int)s_currentMsgId,
                             (int)s_pages.size());
            }
        }
        // No time backstop here: a talk box legitimately waits forever (donor
        // behaviour); cancelling would strand the suppressed TP object open.
        // Aborts are event-driven only (select handback / style toggle).
        return;
    }
    // Demo ended mid-beat → drop the box. No release: the control track is gone, so its
    // suspend went with it; unsuspending a torn-down control would underflow the counter.
    if (!dComIfGp_event_runCheck()) {
        s_boxActive = false;
        return;
    }
    ++s_boxFrames;
    JStudio::stb::TControl* ctrl = dDemo_c::getControl();
    const int susp = (ctrl != NULL) ? (int)ctrl->getSuspend() : 0;
    if (susp > 0) {
        s_boxSawSuspend = true;  // the STB has frozen the timeline for this beat
    }
    // Direct pad read (the exact read the donor's dMesg_outwaitProc uses — un-gated by
    // event_runCheck, so it works during a demo). A OR B across PAD_1..PAD_3 covers the
    // §194 H11 "player's pad may not be PAD_1 during a demo" uncertainty + the DualSense
    // face-button mapping.
    const bool pressed =
        mDoCPd_c::getTrigA(PAD_1) != 0 || mDoCPd_c::getTrigB(PAD_1) != 0 ||
        mDoCPd_c::getTrigA(PAD_2) != 0 || mDoCPd_c::getTrigB(PAD_2) != 0 ||
        mDoCPd_c::getTrigA(PAD_3) != 0 || mDoCPd_c::getTrigB(PAD_3) != 0;

    // Donor contract (dMesg_outwaitProc:2035 advances pages on A/B; dMesg_closeProc:2112
    // unsuspends only when the whole message is done). Accept the press once the STB has
    // actually frozen the timeline (susp>0): advance to the next page WITHOUT releasing,
    // and unsuspend exactly once on the final page's dismissal — so the one STB suspend(1)
    // for this message is balanced by our one unsuspend(1), no matter the page count.
    // §317 oracle item (2), closed 2026-08-16: the release is NOT gated on
    // `susp > 0` — that test was a non-donor clamp. The donor's TControl is a
    // plain signed accumulate (JStudio/stb.h:71 `suspend(s32 v) { _20 += v; }`,
    // unsuspend(n) == suspend(-n), :145) with no floor: a fast dismissal is
    // ALLOWED to drive the counter to −1 so the authored suspend that follows
    // cancels it back to 0. Gating release on the counter's sign prevented the
    // excursion and thereby the correction — once at −1, unsuspend never fired
    // and the accounting never recovered. Owed-ness is OUR beat state
    // (s_boxSawSuspend: one STB suspend observed → one release owed), not the
    // counter's current value. `susp > 0` remains the right question only for
    // the latch above (the donor's own isSuspended() is `> 0`, stb.h:138).
    if (pressed && s_boxSawSuspend) {
        if (s_pageIdx + 1 < (int)s_pages.size()) {
            ++s_pageIdx;
            dExtDmesg_showPage(s_pageIdx);
            s_boxFrames = 0;  // restart the per-page backstop
            DuskLog.info("[dMesg] §308 M4c page → {}/{} (id={})", s_pageIdx + 1,
                         (int)s_pages.size(), (int)s_currentMsgId);
        } else {
            if (ctrl != NULL) {
                ctrl->unsuspend(1);
            }
            s_boxActive = false;
            DuskLog.info("[dMesg] §308 M4c dismissed id={} (last of {} pages) → unsuspend(1) "
                         "(susp {}→{})", (int)s_currentMsgId, (int)s_pages.size(), susp,
                         susp - 1);
        }
        return;
    }
    // Anti-hang backstops (the tale always suspends per beat — these only fire on a
    // malformed beat, and never leave the timeline suspended by us):
    //  * a beat that never suspends → A/B drops the box with no release after a grace.
    if (pressed && !s_boxSawSuspend && s_boxFrames > 120) {
        s_boxActive = false;
        DuskLog.info("[dMesg] §308 M4b dropped id={} (no STB suspend seen in {}f)",
                     (int)s_currentMsgId, s_boxFrames);
        return;
    }
    //  * absolute backstop: force-close a stuck beat, paying the release if still owed.
    //    Owed = this beat suspended (s_boxSawSuspend) and we have not released yet —
    //    NOT `susp > 0`: the counter may already sit at 0/−1 with our release still
    //    unpaid, and skipping the payment there is the §317 clamp again.
    if (s_boxFrames > 3600) {
        if (s_boxSawSuspend && ctrl != NULL) {
            ctrl->unsuspend(1);
        }
        s_boxActive = false;
        DuskLog.warn("[dMesg] §308 M4b backstop force-close id={} (susp={})",
                     (int)s_currentMsgId, susp);
    }
}

// ============================================================================
// §324 LIVE TALK API — see d_ext_dmesg.h. Text resolution order is native-first:
// the donor BMG by the actor's mMsgNo (dExtWwMsg_textByMsgNo — the donor's own
// scan, Hylian archive selection included), then the §256 catalog line the
// caller already resolved (ids the WW-Crew text pack remapped). The actor's id
// is an mMsgNo, NOT an INF1 index; feeding it to getMessageById was the §324
// mis-key that drew "Seven-Star Isles" at the postbox.
// ============================================================================
bool dExtDmesg_openTalk(unsigned short i_id, const char* i_fallbackText) {
    if (dusk::getSettings().game.wwDialogue.getValue() != dusk::WwDialogueStyle::Native) {
        return false;
    }
    if (!s_ready) {
        return false;
    }
    const char* stg = dComIfGp_getStartStageName();
    if (stg == NULL || !dExtWwSave_isWwHostStage(stg)) {
        return false;
    }
    if (s_boxActive && !s_talkMode) {
        return false;  // an STB beat owns the box — never steal a cutscene's box
    }
    const char* txt = dExtWwMsg_textByMsgNo(i_id);
    const bool fromBmg = txt != NULL;
    if (txt == NULL) {
        txt = i_fallbackText;
    }
    if (txt == NULL || txt[0] == '\0') {
        return false;  // nothing to show → caller stays Reconstructed
    }
    // Box choice mirrors the STB path — but keyed in the talk path's own space:
    // the fukiKind comes from the zel_00 entry CARRYING mMsgNo i_id, not from
    // INF1[i_id] (the same mis-key chose box shapes from unrelated entries, and
    // since Hylian detection rides on mTextboxType == 12, could silently
    // mis-mark a line Hylian).
    const int bmgIdx = fromBmg ? dExtDmesg_msgNoToIndex(i_id) : -1;
    const u8 fukiKind = (bmgIdx >= 0) ? dExtDmesg_getMessageTextboxType((u16)bmgIdx) : (u8)0;
    if (!dExtDmesg_ensureBoxBuilt((int)fukiKind)) {
        return false;
    }
    s_currentMsgId = i_id;
    dExtDmesg_paginate(txt, s_pages);
    if (s_pages.empty()) {
        return false;
    }
    s_pageIdx = 0;
    dExtDmesg_showPage(0);
    s_boxActive = true;
    s_talkMode = true;
    s_talkRelease = false;
    s_boxFrames = 0;
    DuskLog.info("[dMesg] §324 talk open msgNo={} inf1={} fuki={} src={} pages={}", (int)i_id,
                 bmgIdx, (int)fukiKind, fromBmg ? "bmg(msgNo)" : "catalog", (int)s_pages.size());
    return true;
}

bool dExtDmesg_isTalkActive() { return s_talkMode && s_boxActive; }

bool dExtDmesg_isTalkReleaseFrame() {
    // Consume-once with same-frame stickiness: the first isSend() that reads the
    // release consumes it, and every further read THIS update-frame still sees it
    // (a proc may branch on isSend() more than once in one frame).
    if (s_talkRelease && s_dmFrame - s_talkReleaseFrame <= 2) {
        s_talkRelease = false;
        s_talkStickyFrame = s_dmFrame;
        return true;
    }
    return s_talkStickyFrame == s_dmFrame;
}

bool dExtDmesg_isTalkChainWindow() { return s_talkChainFrames > 0; }

// §227: re-arm the gap window each time a chained line is accepted, so the
// native box owns the WHOLE conversation instead of only the first hop. Without
// this the window is a one-shot from the first dismissal and any conversation
// longer than two lines drifts back to TP boxes.
void dExtDmesg_rearmTalkChain() { s_talkChainFrames = kTalkChainWindowFrames; }

void dExtDmesg_cancelTalk(const char* i_why) {
    if (!s_talkMode) {
        return;
    }
    s_talkMode = false;
    s_boxActive = false;
    s_talkRelease = false;
    s_talkStickyFrame = (u32)-1;
    DuskLog.info("[dMesg] §324 talk cancelled ({}) id={}", i_why != NULL ? i_why : "?",
                 (int)s_currentMsgId);
}
