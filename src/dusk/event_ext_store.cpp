// ============================================================
// §305 — sidecar persistence for the donor event-flag block (wire ①).
//
// One file, three slots — one 0x100 donor block per quest-log slot,
// keyed by the SAME dataNum dSv_info_c::memory_to_card/card_to_memory
// use. Lives beside the card data, NOT inside it:
//   <data>/<USA|EUR|JAP>/event_ext.sav
//   (region rule mirrors aurora GetCardRegion(): diskID gameName[3]
//    'E'->USA 'P'->EUR 'J'->JAP — same identity as the Card A dir;
//    the region dir is on dusk::data's user-data migration allowlist,
//    so the sidecar relocates with the card.)
//
// Why sidecar (History's finding, confirmed): mSaveBuffer is exactly
// QUEST_LOG_SIZE*3 with no trailing slack, and the in-slot bytes are
// checksum-covered (mDoMemCdRWm_SetCheckSumGameData) — there is no
// clean in-card home. The sidecar fails safe on its own magic:
// missing/corrupt file -> donor new-game state; the TP save is never
// touched by any failure here (every entry point is exception-walled).
//
// №106 filename law: no "ww" in shipping paths — event_ext.sav,
// event_ext_store.cpp.
// ============================================================
#include "d/d_ext_save_flags.h"

#include <cstring>
#include <filesystem>
#include <vector>

#include "data.hpp"
#include "dusk/io.hpp"
#include "dusk/logging.h"
#include "dusk/version.hpp"

namespace {

aurora::Module Log{"dusk::event_ext"};

constexpr int kSlotCount = 3;          // TP quest-log slots
constexpr u32 kSlotSize = 0x100;       // donor dSv_event_c, verbatim
constexpr u32 kHeaderSize = 8;         // magic + version + slot count
constexpr u32 kFileSize = kHeaderSize + kSlotCount * kSlotSize;
constexpr u8 kMagic[4] = {'W', 'W', 'E', 'V'};
constexpr u16 kVersion = 1;

std::filesystem::path sidecar_path() {
    const char region = dusk::version::getDiskID().gameName[3];
    const char* dir = (region == 'P') ? "EUR" : (region == 'J') ? "JAP" : "USA";
    return dusk::data::base_path_relative(std::filesystem::path(dir) /
                                          "event_ext.sav");
}

struct FileImage {
    u8 slots[kSlotCount][kSlotSize];
};

// Missing/short/bad-magic file -> zeroed image (donor new-game state for
// every slot) and FALSE; the caller decides whether that matters.
bool read_image(FileImage& o_image) {
    std::memset(&o_image, 0, sizeof(o_image));
    try {
        const std::filesystem::path path = sidecar_path();
        if (!std::filesystem::exists(path)) {
            return false;
        }
        const std::vector<u8> bytes = dusk::io::FileStream::ReadAllBytes(path);
        if (bytes.size() < kFileSize || std::memcmp(bytes.data(), kMagic, 4) != 0) {
            Log.warn("event_ext.sav rejected (size {} / bad magic) — donor "
                     "flags fall back to new-game state",
                     bytes.size());
            return false;
        }
        u16 version;
        std::memcpy(&version, bytes.data() + 4, 2);
        if (version != kVersion) {
            Log.warn("event_ext.sav version {} != {} — donor flags fall back "
                     "to new-game state",
                     version, kVersion);
            return false;
        }
        std::memcpy(o_image.slots, bytes.data() + kHeaderSize,
                    sizeof(o_image.slots));
        return true;
    } catch (const std::exception& e) {
        Log.warn("event_ext.sav read failed: {}", e.what());
        return false;
    }
}

void write_image(const FileImage& i_image) {
    try {
        u8 bytes[kFileSize];
        std::memcpy(bytes, kMagic, 4);
        std::memcpy(bytes + 4, &kVersion, 2);
        bytes[6] = kSlotCount;
        bytes[7] = 0;
        std::memcpy(bytes + kHeaderSize, i_image.slots, sizeof(i_image.slots));
        dusk::io::FileStream stream =
            dusk::io::FileStream::Create(sidecar_path());
        stream.Write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
        stream.Flush();
    } catch (const std::exception& e) {
        Log.warn("event_ext.sav write failed: {}", e.what());
    }
}

bool slot_ok(int i_dataNum) {
    return i_dataNum >= 0 && i_dataNum < kSlotCount;
}

}  // namespace

BOOL dExtWwSvIo_restoreSlot(int i_dataNum) {
    dExtWwSv_reset();  // fail-safe floor: donor new-game state
    if (!slot_ok(i_dataNum)) {
        return FALSE;
    }
    FileImage image;
    if (!read_image(image)) {
        return FALSE;
    }
    dExtWwSv_importRaw(image.slots[i_dataNum]);
    return TRUE;
}

void dExtWwSvIo_storeSlot(int i_dataNum) {
    if (!slot_ok(i_dataNum)) {
        return;
    }
    FileImage image;
    read_image(image);  // keep the other slots; zeroed image if no file yet
    dExtWwSv_exportRaw(image.slots[i_dataNum]);
    write_image(image);
}

void dExtWwSvIo_eraseSlot(int i_dataNum) {
    if (!slot_ok(i_dataNum)) {
        return;
    }
    FileImage image;
    if (!read_image(image)) {
        return;  // nothing persisted -> nothing to erase
    }
    std::memset(image.slots[i_dataNum], 0, kSlotSize);
    write_image(image);
}

void dExtWwSvIo_copySlot(int i_from, int i_to) {
    if (!slot_ok(i_from) || !slot_ok(i_to) || i_from == i_to) {
        return;
    }
    FileImage image;
    read_image(image);  // a zeroed source copies as zeroed — mirrors the card
    std::memcpy(image.slots[i_to], image.slots[i_from], kSlotSize);
    write_image(image);
}
