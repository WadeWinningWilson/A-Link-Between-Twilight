// ============================================================
// §303 — donor event-flag namespace implementation.
// Logic is the donor's dSv_event_c, LINE FOR LINE (d_save.cpp:1189-1214);
// only the storage identity differs (this block, not TP's table).
// ============================================================
// KIT-LINEAGE: mixed
// KIT-DONOR: per-hunk
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: per-hunk
#include "d/d_ext_save_flags.h"

#include <cstring>

static dExtSvFlags_event_c s_wwEventFlags;

// magic 'WWEV' + version; bumps only if the layout ever changes (it is the
// donor's frozen layout, so it should not).
static const u32 kMagic = 0x57574556;  // 'WWEV'
static const u16 kVersion = 1;

// KIT-DONOR-HUNK: d/d_save.cpp Matching
void dExtSvFlags_event_c::init() {
    std::memset(mFlags, 0, sizeof(mFlags));  // donor init = zeroed
}

void dExtSvFlags_event_c::onEventBit(u16 i_no) {
    mFlags[i_no >> 8] |= (i_no & 0xFF);      // donor d_save.cpp:1190
}

void dExtSvFlags_event_c::offEventBit(u16 i_no) {
    mFlags[i_no >> 8] &= ~(i_no & 0xFF);     // donor d_save.cpp:1195
}

BOOL dExtSvFlags_event_c::isEventBit(u16 i_no) {
    return mFlags[i_no >> 8] & (i_no & 0xFF) ? TRUE : FALSE;  // :1200
}

void dExtSvFlags_event_c::setEventReg(u16 i_reg, u8 i_no) {
    mFlags[i_reg >> 8] &= ~(u8)i_reg;        // donor d_save.cpp:1205
    mFlags[i_reg >> 8] |= i_no;
}

u8 dExtSvFlags_event_c::getEventReg(u16 i_reg) {
    return mFlags[i_reg >> 8] & (u8)i_reg;   // donor d_save.cpp:1210 shape
}

// KIT-DONOR-HUNK-END
BOOL dExtWwSv_isEventBit(u16 i_no) { return s_wwEventFlags.isEventBit(i_no); }
void dExtWwSv_onEventBit(u16 i_no) { s_wwEventFlags.onEventBit(i_no); }
void dExtWwSv_offEventBit(u16 i_no) { s_wwEventFlags.offEventBit(i_no); }
void dExtWwSv_setEventReg(u16 i_reg, u8 i_no) {
    s_wwEventFlags.setEventReg(i_reg, i_no);
}
u8 dExtWwSv_getEventReg(u16 i_reg) { return s_wwEventFlags.getEventReg(i_reg); }

void dExtWwSv_reset() {
    s_wwEventFlags.init();
}

u32 dExtWwSv_storeSize() {
    return 4 + 2 + 2 + sizeof(s_wwEventFlags.mFlags);  // magic+ver+pad+block
}

void dExtWwSv_store(u8* o_buf) {
    std::memcpy(o_buf, &kMagic, 4);
    std::memcpy(o_buf + 4, &kVersion, 2);
    o_buf[6] = 0;
    o_buf[7] = 0;
    std::memcpy(o_buf + 8, s_wwEventFlags.mFlags,
                sizeof(s_wwEventFlags.mFlags));
}

u32 dExtWwSv_rawSize() {
    return sizeof(s_wwEventFlags.mFlags);
}

void dExtWwSv_exportRaw(u8* o_buf) {
    std::memcpy(o_buf, s_wwEventFlags.mFlags, sizeof(s_wwEventFlags.mFlags));
}

void dExtWwSv_importRaw(const u8* i_buf) {
    std::memcpy(s_wwEventFlags.mFlags, i_buf, sizeof(s_wwEventFlags.mFlags));
}

BOOL dExtWwSv_restore(const u8* i_buf) {
    u32 magic;
    u16 version;
    std::memcpy(&magic, i_buf, 4);
    std::memcpy(&version, i_buf + 4, 2);
    if (magic != kMagic || version != kVersion) {
        dExtWwSv_reset();  // fail safe = new-game donor state
        return FALSE;
    }
    std::memcpy(s_wwEventFlags.mFlags, i_buf + 8,
                sizeof(s_wwEventFlags.mFlags));
    return TRUE;
}
