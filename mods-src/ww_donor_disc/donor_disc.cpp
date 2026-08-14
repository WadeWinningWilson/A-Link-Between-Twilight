// ============================================================================
// donor_disc.cpp — GCM image reader behind the service-shaped boundary (L2).
//
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
//
// WHAT THIS IS. Plain plugin-side I/O over the user's own GC disc image
// (L2a: no receiver code, no host mount API — none is needed). It parses the
// standard GCM layout:
//
//   0x000  game id (6 chars: "GZLE01" = The Wind Waker, USA)
//   0x01C  GC disc magic 0xC2339F3D (big-endian)
//   0x424  FST offset   (big-endian u32)
//   0x428  FST size     (big-endian u32)
//
// FST entries are 12 bytes: {u8 flags; u24 nameOffset; u32 offset; u32 length}
// with entry 0 the root (its length = total entry count) and the string table
// immediately after the entries. GC offsets are byte offsets (no Wii >>2).
//
// THE R3 WRONG-DISC GATE (§332, "OFF-ROSTER verdict"). The checked-in donor
// roster (tools/ww_crew_restoration_skeleton/donor_roster_GZLE01.csv) pins
// SHA-256 for every file of the sanctioned dump — including sys/boot.bin
// (the first 0x440 bytes, which CONTAIN the FST offset/size fields) and
// sys/fst.bin (which contains every file's name/offset/length). Verifying
// those two therefore pins the location and identity of everything this
// reader will ever serve, for the cost of hashing ~58 KB at attach time.
// The two hashes are embedded below verbatim from the roster; the roster
// file remains the source of truth (regenerate → update these constants).
//
// A hash mismatch is OFF-ROSTER: the reader refuses to serve. Legibly, never
// an assert — a wrong or corrupt image is a normal handled condition (19c).
//
// NO Yaz0 PASS, DELIBERATELY. This reader serves bytes UNTOUCHED (zero-bake:
// the donor layer is immutable). The receiver's stock loaders already handle
// compressed members exactly as they would from any disc; decompressing here
// would be editing the donor layer and is not this reader's business.
// ============================================================================

#include "donor_disc.h"

#include <mods/service.hpp>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace {

// ============================================================================
// Compact SHA-256 (FIPS 180-4). Self-contained so the plugin needs no host
// crypto surface; ~58 KB hashed once per attach, performance is irrelevant.
// ============================================================================
struct Sha256 {
    uint32_t state[8];
    uint64_t bitlen = 0;
    uint8_t buffer[64];
    size_t buflen = 0;

    static constexpr uint32_t k[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4,
        0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe,
        0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f,
        0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
        0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
        0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116,
        0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7,
        0xc67178f2};

    Sha256() {
        state[0] = 0x6a09e667; state[1] = 0xbb67ae85; state[2] = 0x3c6ef372;
        state[3] = 0xa54ff53a; state[4] = 0x510e527f; state[5] = 0x9b05688c;
        state[6] = 0x1f83d9ab; state[7] = 0x5be0cd19;
    }

    static uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }

    void transform(const uint8_t* data) {
        uint32_t w[64];
        for (int i = 0; i < 16; i++) {
            w[i] = (uint32_t(data[i * 4]) << 24) | (uint32_t(data[i * 4 + 1]) << 16) |
                   (uint32_t(data[i * 4 + 2]) << 8) | uint32_t(data[i * 4 + 3]);
        }
        for (int i = 16; i < 64; i++) {
            const uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            const uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
        uint32_t e = state[4], f = state[5], g = state[6], h = state[7];
        for (int i = 0; i < 64; i++) {
            const uint32_t s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            const uint32_t ch = (e & f) ^ (~e & g);
            const uint32_t t1 = h + s1 + ch + k[i] + w[i];
            const uint32_t s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            const uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            const uint32_t t2 = s0 + maj;
            h = g; g = f; f = e; e = d + t1;
            d = c; c = b; b = a; a = t1 + t2;
        }
        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
        state[4] += e; state[5] += f; state[6] += g; state[7] += h;
    }

    void update(const uint8_t* data, size_t len) {
        bitlen += uint64_t(len) * 8;
        while (len > 0) {
            const size_t take = (len < 64 - buflen) ? len : 64 - buflen;
            std::memcpy(buffer + buflen, data, take);
            buflen += take;
            data += take;
            len -= take;
            if (buflen == 64) {
                transform(buffer);
                buflen = 0;
            }
        }
    }

    // Hex digest, lowercase — the roster CSV's format, compared as strings.
    std::string final_hex() {
        uint8_t pad[72] = {0x80};
        const uint64_t bits = bitlen;
        size_t padlen = (buflen < 56) ? 56 - buflen : 120 - buflen;
        update(pad, padlen);
        uint8_t lenbytes[8];
        for (int i = 0; i < 8; i++) {
            lenbytes[i] = uint8_t(bits >> (56 - i * 8));
        }
        // update() would re-count these 8 bytes into bitlen, but bits was
        // latched above, so the digest is over the original message only.
        update(lenbytes, 8);
        char hex[65];
        for (int i = 0; i < 8; i++) {
            std::snprintf(hex + i * 8, 9, "%08x", state[i]);
        }
        return std::string(hex, 64);
    }
};

constexpr uint32_t Sha256::k[64];

// ============================================================================
// R3 roster constants — verbatim from donor_roster_GZLE01.csv (§332).
// boot.bin = first 0x440 bytes; fst.bin = [fstOffset, fstOffset + fstSize).
// ============================================================================
constexpr size_t kBootBinSize = 0x440;  // roster: sys/boot.bin,1088
constexpr const char* kBootBinSha256 =
    "bdd9be0dad36f29c0769830fe600e6ba9023063d159b7e5a3430af10941d6909";
constexpr const char* kFstBinSha256 =
    "b9738ae497893f9deb814af0ad2837b4d10f85967e64c32c453ff5cadc74f951";

constexpr const char* kSanctionedGameId = "GZLE01";
constexpr uint32_t kGcDiscMagic = 0xC2339F3D;  // big-endian at 0x1C

uint32_t be32(const uint8_t* p) {
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}

struct FstFile {
    std::string path;  // "res/Stage/sea/Room44.arc" — disc-relative, no leading slash
    uint32_t offset;
    uint32_t size;
};

// ============================================================================
// Reader state. One attached image per module (L2b: one plugin, one mounted
// disc handle in one module's state).
// ============================================================================
FILE* s_image = nullptr;
std::string s_imagePath;
DonorDiscVerdict s_verdict = DONOR_DISC_UNATTACHED;
std::vector<FstFile> s_files;

bool read_at(uint64_t offset, void* buf, size_t len) {
    if (s_image == nullptr) {
        return false;
    }
#if defined(_WIN32)
    if (_fseeki64(s_image, (long long)offset, SEEK_SET) != 0) {
        return false;
    }
#else
    if (std::fseek(s_image, (long)offset, SEEK_SET) != 0) {
        return false;
    }
#endif
    return std::fread(buf, 1, len, s_image) == len;
}

void reset_state() {
    if (s_image != nullptr) {
        std::fclose(s_image);
        s_image = nullptr;
    }
    s_imagePath.clear();
    s_verdict = DONOR_DISC_UNATTACHED;
    s_files.clear();
}

bool ieq(char a, char b) {
    if (a >= 'A' && a <= 'Z') a += 'a' - 'A';
    if (b >= 'A' && b <= 'Z') b += 'a' - 'A';
    return a == b;
}

bool path_ieq(const char* a, const char* b) {
    while (*a != '\0' && *b != '\0') {
        if (!ieq(*a, *b)) {
            return false;
        }
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

// ============================================================================
// FST parse. Bounds-checked throughout: a truncated or lying FST refuses the
// attach rather than reading wild — though the R3 gate makes that mostly
// theoretical (the FST bytes are hash-pinned before this runs).
// ============================================================================
bool parse_fst(const std::vector<uint8_t>& fst, std::string* err) {
    if (fst.size() < 12) {
        *err = "FST too small";
        return false;
    }
    const uint32_t numEntries = be32(fst.data() + 8);
    const uint64_t tableEnd = uint64_t(numEntries) * 12;
    if (numEntries < 1 || tableEnd > fst.size()) {
        *err = "FST entry count out of range";
        return false;
    }
    const uint8_t* strings = fst.data() + tableEnd;
    const size_t stringsLen = fst.size() - size_t(tableEnd);

    // Directory nesting: (endIndex, pathPrefix). Root covers everything.
    std::vector<std::pair<uint32_t, std::string>> dirs;
    dirs.emplace_back(numEntries, std::string());

    for (uint32_t i = 1; i < numEntries; i++) {
        while (i >= dirs.back().first) {
            dirs.pop_back();
        }
        const uint8_t* e = fst.data() + size_t(i) * 12;
        const bool isDir = e[0] != 0;
        const uint32_t nameOff = (uint32_t(e[1]) << 16) | (uint32_t(e[2]) << 8) | uint32_t(e[3]);
        if (nameOff >= stringsLen) {
            *err = "FST name offset out of range";
            return false;
        }
        // NUL-termination inside the string table, verified not assumed.
        const void* nul = std::memchr(strings + nameOff, '\0', stringsLen - nameOff);
        if (nul == nullptr) {
            *err = "FST name unterminated";
            return false;
        }
        const char* name = reinterpret_cast<const char*>(strings + nameOff);
        if (isDir) {
            const uint32_t endIdx = be32(e + 8);
            if (endIdx <= i || endIdx > numEntries) {
                *err = "FST directory span out of range";
                return false;
            }
            dirs.emplace_back(endIdx, dirs.back().second + name + "/");
        } else {
            FstFile f;
            f.path = dirs.back().second + name;
            f.offset = be32(e + 4);
            f.size = be32(e + 8);
            s_files.push_back(std::move(f));
        }
    }
    return true;
}

// ============================================================================
// Service implementation.
// ============================================================================
ModResult svc_attach(ModContext* /*ctx*/, const char* image_path, ModError* out_error) {
    reset_state();
    if (image_path == nullptr || image_path[0] == '\0') {
        return mods::set_error(out_error, MOD_INVALID_ARGUMENT, "no image path given");
    }

    FILE* f = std::fopen(image_path, "rb");
    if (f == nullptr) {
        char msg[MOD_ERROR_MESSAGE_SIZE];
        std::snprintf(msg, sizeof(msg),
            "cannot open donor disc image '%s' (missing path or no read permission)", image_path);
        return mods::set_error(out_error, MOD_UNAVAILABLE, msg);
    }
    s_image = f;
    s_imagePath = image_path;

    uint8_t boot[kBootBinSize];
    if (!read_at(0, boot, sizeof(boot))) {
        reset_state();
        return mods::set_error(
            out_error, MOD_ERROR, "donor disc image is too small to be GC media (truncated?)");
    }

    // Compressed-container magics first, so the refusal can say HOW to fix it.
    if (std::memcmp(boot, "RVZ\x01", 4) == 0 || std::memcmp(boot, "WIA\x01", 4) == 0) {
        reset_state();
        return mods::set_error(out_error, MOD_UNSUPPORTED,
            "donor disc image is a compressed RVZ/WIA — a PLAIN .iso is required "
            "(Dolphin: right-click the game -> Convert File -> ISO)");
    }
    if (be32(boot + 0x1C) != kGcDiscMagic) {
        reset_state();
        return mods::set_error(out_error, MOD_UNSUPPORTED,
            "not a plain GameCube disc image (GC magic missing at 0x1C)");
    }

    char gameId[7];
    std::memcpy(gameId, boot, 6);
    gameId[6] = '\0';
    if (std::memcmp(gameId, kSanctionedGameId, 6) != 0) {
        reset_state();
        char msg[MOD_ERROR_MESSAGE_SIZE];
        std::snprintf(msg, sizeof(msg),
            "wrong disc: game id '%s' — the sanctioned donor is %s (The Wind Waker, USA)", gameId,
            kSanctionedGameId);
        return mods::set_error(out_error, MOD_UNAVAILABLE, msg);
    }

    // ------------------------------------------------------------------------
    // R3 wrong-disc gate (§332). boot.bin pins the FST fields read below;
    // fst.bin pins everything this reader will ever serve.
    // ------------------------------------------------------------------------
    Sha256 bootHash;
    bootHash.update(boot, sizeof(boot));
    const std::string bootHex = bootHash.final_hex();

    const uint32_t fstOffset = be32(boot + 0x424);
    const uint32_t fstSize = be32(boot + 0x428);
    std::string fstHex;
    std::vector<uint8_t> fst;
    if (bootHex == kBootBinSha256) {
        fst.resize(fstSize);
        if (!read_at(fstOffset, fst.data(), fst.size())) {
            reset_state();
            return mods::set_error(
                out_error, MOD_ERROR, "donor disc image truncated inside the FST");
        }
        Sha256 fstHash;
        fstHash.update(fst.data(), fst.size());
        fstHex = fstHash.final_hex();
    }
    if (bootHex != kBootBinSha256 || fstHex != kFstBinSha256) {
        s_verdict = DONOR_DISC_OFF_ROSTER;
        char msg[MOD_ERROR_MESSAGE_SIZE];
        std::snprintf(msg, sizeof(msg),
            "OFF-ROSTER (R3 / bus §332): this %s image is not the sanctioned dump "
            "(sys/%s.bin SHA-256 mismatch) — refusing to serve donor files. "
            "Re-dump your own disc, or check the image for modification/corruption.",
            kSanctionedGameId, bootHex != kBootBinSha256 ? "boot" : "fst");
        std::fclose(s_image);
        s_image = nullptr;
        return mods::set_error(out_error, MOD_CONFLICT, msg);
    }

    std::string parseErr;
    if (!parse_fst(fst, &parseErr)) {
        // ON-ROSTER bytes that fail to parse would mean the parser is wrong,
        // not the disc — say so rather than blaming the image.
        reset_state();
        char msg[MOD_ERROR_MESSAGE_SIZE];
        std::snprintf(msg, sizeof(msg),
            "FST parse failed AFTER the roster hashes matched (%s) — this is a reader bug, "
            "not a bad disc; report it",
            parseErr.c_str());
        return mods::set_error(out_error, MOD_ERROR, msg);
    }

    s_verdict = DONOR_DISC_ON_ROSTER;
    return MOD_OK;
}

void svc_detach(ModContext* /*ctx*/) {
    reset_state();
}

DonorDiscVerdict svc_verdict(ModContext* /*ctx*/) {
    return s_verdict;
}

int32_t svc_file_count(ModContext* /*ctx*/) {
    return s_verdict == DONOR_DISC_ON_ROSTER ? int32_t(s_files.size()) : 0;
}

ModResult svc_file_info(
    ModContext* /*ctx*/, int32_t index, const char** out_path, uint32_t* out_size) {
    if (s_verdict != DONOR_DISC_ON_ROSTER || index < 0 || size_t(index) >= s_files.size()) {
        return MOD_INVALID_ARGUMENT;
    }
    if (out_path != nullptr) {
        *out_path = s_files[size_t(index)].path.c_str();
    }
    if (out_size != nullptr) {
        *out_size = s_files[size_t(index)].size;
    }
    return MOD_OK;
}

ModResult svc_find(ModContext* /*ctx*/, const char* path, int32_t* out_index) {
    if (s_verdict != DONOR_DISC_ON_ROSTER || path == nullptr || out_index == nullptr) {
        return MOD_INVALID_ARGUMENT;
    }
    for (size_t i = 0; i < s_files.size(); i++) {
        if (path_ieq(s_files[i].path.c_str(), path)) {
            *out_index = int32_t(i);
            return MOD_OK;
        }
    }
    return MOD_UNAVAILABLE;
}

ModResult svc_read(ModContext* /*ctx*/, int32_t index, uint32_t offset, void* buffer,
    uint32_t length, uint32_t* out_read) {
    if (s_verdict != DONOR_DISC_ON_ROSTER || index < 0 || size_t(index) >= s_files.size() ||
        buffer == nullptr)
    {
        return MOD_INVALID_ARGUMENT;
    }
    const FstFile& f = s_files[size_t(index)];
    if (offset >= f.size) {
        if (out_read != nullptr) {
            *out_read = 0;
        }
        return MOD_OK;
    }
    const uint32_t take = (length < f.size - offset) ? length : f.size - offset;
    if (!read_at(uint64_t(f.offset) + offset, buffer, take)) {
        return MOD_ERROR;
    }
    if (out_read != nullptr) {
        *out_read = take;
    }
    return MOD_OK;
}

constexpr DonorDiscService s_service = {
    .header = SERVICE_HEADER(
        DonorDiscService, WW_DONOR_DISC_SERVICE_MAJOR, WW_DONOR_DISC_SERVICE_MINOR),
    .attach = svc_attach,
    .detach = svc_detach,
    .verdict = svc_verdict,
    .file_count = svc_file_count,
    .file_info = svc_file_info,
    .find = svc_find,
    .read = svc_read,
};

}  // namespace

const DonorDiscService* wwDonorDisc_service(void) {
    return &s_service;
}
