// ============================================================================
// main.cpp — ww_donor_disc plugin glue (L2, re-homed plugin-side per L2a).
//
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
//
// WHAT THIS DOES. Declares its own config vars through the SDK config
// service (no settings.h/settings.cpp edit — the exact receiver leg L2a
// exists to avoid), attaches the user's own WW disc image through the
// service-shaped reader (donor_disc.h), and registers every donor file the
// declared stages need as runtime DVD overlays via the overlay service.
//
// THE RECEIVER IS UNTOUCHED. The WW layer's arc-filename alias
// (ww_room_loader.cpp) keeps asking Aurora for /res/Stage/sea/Room44.arc;
// these overlay registrations are what supply the bytes — same vanilla
// names, same bytes, different source. That is the whole trick: L2 needs no
// hook for the base path. (The wwRoom_aliasArcFileName hook seam stays a
// SEPARATE, uncommitted question pending Foundry's 19a/19b verdict on
// hook-reachability.)
//
// CONFIG KEYS (persisted in config.json by the host):
//   mod.wwDonorDisc.wwIsoPath  absolute path to a plain GZLE01 .iso ("" = off)
//   mod.wwDonorDisc.wwStages   comma-separated vanilla stage names to serve
//                                from /res/Stage/<name>/ (default "sea")
//
// 19c — THE GATE REFUSES LEGIBLY. Unconfigured / unreadable / compressed /
// wrong-game / OFF-ROSTER each produce one clear log line naming the fix.
// Never an assert; the WW layer simply stays dormant without its disc.
//
// PRELAUNCH: deliberately NO prelaunch surface — recommendation REVERSED
// (ttw-methods-review.md:248); a prelaunch field is a receiver leg. Quiet
// config key + this legible gate is the sanctioned shape.
// ============================================================================

#include "donor_disc.h"
#include "registry.h"
#include "ww_message.h"

#include <mods/service.hpp>
#include <mods/svc/config.h>
#include <mods/svc/log.h>
#include <mods/svc/overlay.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

DEFINE_MOD()

IMPORT_SERVICE(ConfigService, s_config);
IMPORT_SERVICE(LogService, s_log);
IMPORT_SERVICE(OverlayService, s_overlay);

namespace {

ConfigVarHandle s_varIsoPath = 0;
ConfigVarHandle s_varStages = 0;
ConfigVarHandle s_varObjectArcs = 0;
ConfigVarHandle s_varWarpFileNames = 0;

// ============================================================================
// §716 (WAVE-1 rows 20/21) — the mount-retirement serve set.
//
// The R2 mount (model_replacements/WW-Crew-Restoration/arcs/) served ~90
// curated archives as res/Object/<Name>.arc. Auditing that set against the
// donor FST partitioned it three ways:
//   - 61 are PRISTINE DISC FILES under res/Object/ (incl. the VshiN casing) —
//     served from disc below, so the staged copies retire;
//   - 7 are disc files under res/Msg/ that the mount RE-HOMED to res/Object/
//     (dmsgres/fontres/rubyres/menures/itemicon + the bmg pair). The §308
//     consumer asks for the res/Object path, so the alias is kept — a
//     consumption-boundary path translation, bytes verbatim from disc;
//   - 15 are PROJECT-ASSEMBLED (WwSky/WwAlways/WwDalways, interior packs like
//     Ojhous/LinkRM/Cave09, Outset) — they exist on no disc and stay with the
//     mount until each gets its own derive-from-disc step (№116 ceremony).
// ============================================================================
const char* const kDefaultObjectArcs =
    "Ah,Aj,Ajav,Akabe,Auzu,Ba,Bb,Bk,Bm,Bridge,Cb,Cc,Demo01,Demo02,Dk,Ebrock,"
    "Ekao,Gnd,Hbox2,Hseki,Jb,Ji,Kamome,Kanban,Kb,Kmi00x,Kmtub_00,Kn,Knob,Ko,"
    "Krock_01,Ksaku_00,Kt,Ktaru_01,Kusa,Lamp,Ls,Lwood,Md,Mk,Mo2,Mshokki,Ob,"
    "Odokuro,Okioke,Okmono,Opaper,Otana,Oyashi,P1,P2,Piwa,Plant,Ppos,Pt,Ptubo,"
    "Rflw,Sitem,Table,Toripost,Vdora,Vfuku,Vhkak,Vhutu,VkeyN,Vlupy,VshiN,"
    "Yaflw00,Ym,Yw,Zl";
// §806 growth (tsubo port, tale §805/§806): Hbox2, Hseki, Kmi00x, Kmtub_00,
// Ktaru_01, Odokuro, Okioke, Sitem — the donor tsubo family's arcs, all
// verified free of receiver-side consumers 2026-08-12 (grep across src/
// include; Okmono/Ptubo were already on the roster).
// §822 growth: Jb — Jabun (donor d_a_npc_jb1); collision-free.
// §837 growth: Ppos — the poster type of Obj_Paper (Opaper/Piwa/Plant were
// already served); collision-free.

// The 7 res/Msg archives the mount re-homed to res/Object/ (see banner above).
const char* const kMsgToObjectAlias[] = {
    "bmgres", "bmgresh", "dmsgres", "fontres", "itemicon", "menures", "rubyres",
};

// §806: donor arcs whose NAMES collide with the receiver's own res/Object
// archives — served with the SAME i_asPath rename the Msg aliases use (bytes
// verbatim from disc; only the mount point differs, so nothing TP-side is
// shadowed). Consumers translate at the boundary via dExtWw_objectArcAlias
// (d_ext_ww_actor_shims.h) — the two tables MUST stay in lockstep.
//   Always   → WwAlways   (TP core archive: alink/bg_obj/demo00/...)
//   Kkiba_00 → WwKkiba00  (TP box archive: obj_carry/burnbox/movebox)
struct TpCollisionAlias {
    const char* discStem;   // donor disc stem under res/Object/
    const char* serveStem;  // receiver-visible mount stem
};
const TpCollisionAlias kTpCollisionAlias[] = {
    {"Always", "WwAlways"},
    {"Kkiba_00", "WwKkiba00"},
    // §809: donor player arc — Aryll's telescope prop lives here (the donor's
    // own recipe: d_a_npc_ls1 resolves telescope.bdl from res/Object/Link.arc,
    // which is donor-RESIDENT; the receiver serves it as an object arc). The
    // name is too load-bearing TP-side to risk unaliased.
    {"Link", "WwLink"},
};
std::vector<OverlayHandle> s_served;
std::string s_attachedPath;  // what the current overlays were built from

void logf(LogLevel level, const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    s_log->write(mod_ctx, level, buf);
}

std::string get_string_var(ConfigVarHandle var) {
    size_t len = 0;
    if (s_config->get_string(mod_ctx, var, nullptr, 0, &len) != MOD_OK) {
        return {};
    }
    std::string value(len, '\0');
    if (len > 0 && s_config->get_string(mod_ctx, var, value.data(), len + 1, nullptr) != MOD_OK) {
        return {};
    }
    return value;
}

std::vector<std::string> split_csv(const std::string& csv) {
    std::vector<std::string> out;
    size_t start = 0;
    while (start <= csv.size()) {
        size_t end = csv.find(',', start);
        if (end == std::string::npos) {
            end = csv.size();
        }
        size_t b = start;
        size_t e = end;
        while (b < e && (csv[b] == ' ' || csv[b] == '\t')) b++;
        while (e > b && (csv[e - 1] == ' ' || csv[e - 1] == '\t')) e--;
        if (e > b) {
            out.push_back(csv.substr(b, e - b));
        }
        start = end + 1;
    }
    return out;
}

// Whole-string case-insensitive compare. Its sibling below matches a PREFIX,
// which is wrong for a leaf test: `Stage.arc` must not match `Stage.arc.bak`
// or any longer name that merely starts the same way.
bool str_eq_ci(const char* a, const char* b) {
    while (*a != '\0' && *b != '\0') {
        char x = *a, y = *b;
        if (x >= 'A' && x <= 'Z') x += 'a' - 'A';
        if (y >= 'A' && y <= 'Z') y += 'a' - 'A';
        if (x != y) {
            return false;
        }
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

bool path_has_prefix_ci(const char* path, const char* prefix) {
    while (*prefix != '\0') {
        char a = *path, b = *prefix;
        if (a >= 'A' && a <= 'Z') a += 'a' - 'A';
        if (b >= 'A' && b <= 'Z') b += 'a' - 'A';
        if (a != b) {
            return false;
        }
        ++path;
        ++prefix;
    }
    return true;
}

// ============================================================================
// Register one disc file as a runtime overlay. Bytes are read once and handed
// to the host (add_buffer copies) — the whole declared payload for `sea` is
// ~13 MiB (measured against the roster), so residency is a non-cost.
// ============================================================================
// i_asPath: optional overlay-path override (default = the disc's own path).
// Used only for the §716 res/Msg → res/Object aliases; bytes are ALWAYS the
// disc's, verbatim — the override renames the mount point, never the content.
// ============================================================================
// PARENTAGE FROM THE DISC — the `SCLS` exit table (warp-menu grouping).
//
// WW ENCODES WHICH INTERIORS BELONG TO WHICH ISLAND, and it was reported as
// data the donor does not have. It does: every room's `.dzr` carries an
// `SCLS` chunk listing the stages reachable from it. `sea/Room44.dzr` names
// exactly LinkRM / Ojhous / Ojhous2 / Omasao / Onobuta / A_mori / Pjavdou /
// Cave09 / LinkUG — Outset's interior set, enumerated by the donor.
//
// MEASURED ACROSS ALL 49 SEA ROOMS: 31 carry exits, yielding 54 destination
// stages each with a parent island. The method cross-checks itself on pairings
// nobody supplied — Atorizk->Dragon Roost, Ekaze->Gale Isle, Edaichi->
// Headstone, Siren->Tower of the Gods. So the grouping needs NOTHING typed.
//
// COST IS ZERO EXTRA I/O. This runs inside serve_file() on bytes already read
// for the overlay; the alternative (a later pass) would re-read up to 12 MB.
//
// CHUNK LAYOUT (same DZR table the actor pass uses): u32 count at +0x00, then
// count x 12-byte headers {tag[4], u32 entries, u32 offset}. SCLS entries are
// 0xC bytes: an 8-byte stage name, then spawn id and room number.
// ============================================================================
struct WwStageParent {
    char stage[20];
    signed char seaRoom;
};
WwStageParent s_stageParents[96];
int s_stageParentCount = 0;
bool s_stageParentsSaturated = false;

void noteStageParent(const char* stage, int seaRoom) {
    for (int i = 0; i < s_stageParentCount; i++) {
        if (std::strcmp(s_stageParents[i].stage, stage) == 0) {
            return;   // FIRST parent wins; see the LinkRM note in the log below
        }
    }
    if (s_stageParentCount >= (int)(sizeof(s_stageParents) / sizeof(s_stageParents[0]))) {
        s_stageParentsSaturated = true;   // never drop silently
        return;
    }
    WwStageParent& p = s_stageParents[s_stageParentCount++];
    std::strncpy(p.stage, stage, sizeof(p.stage) - 1);
    p.stage[sizeof(p.stage) - 1] = 0;
    p.seaRoom = (signed char)seaRoom;
}

int stageParentRoom(const char* stage) {
    for (int i = 0; i < s_stageParentCount; i++) {
        if (std::strcmp(s_stageParents[i].stage, stage) == 0) {
            return s_stageParents[i].seaRoom;
        }
    }
    return -1;
}

// Read `STAG` out of a served `Stage.arc` and record the donor's own stage
// type. `dStage_stagInfo_GetSTType()` is `(mStageTypeAndSchbit >> 16) & 7`,
// and `mStageTypeAndSchbit` sits at **+0x0C inside the donor's 0x20 STAG** —
// INSIDE the struct, unlike `mMsgGroup` at 0x28 which is 8 bytes past its end
// and cost a crash. Reading past 0x1C here would repeat that exactly.
void sniffStageType(const char* discPath, const void* arcBytes, uint32_t size) {
    if (discPath == nullptr || arcBytes == nullptr) { return; }
    const char* p = std::strstr(discPath, "Stage/");
    if (p == nullptr || std::strstr(discPath, "/Stage.arc") == nullptr) { return; }
    char stage[20] = {0};
    if (std::sscanf(p + 6, "%19[^/]", stage) != 1 || stage[0] == 0) { return; }
    uint32_t dzsLen = 0;
    const unsigned char* dzs =
        (const unsigned char*)wwMessage_rarcFind(arcBytes, size, "stage.dzs", &dzsLen);
    if (dzs == nullptr || dzsLen < 4) { return; }
    const uint32_t chunks = ((uint32_t)dzs[0] << 24) | ((uint32_t)dzs[1] << 16) |
                            ((uint32_t)dzs[2] << 8) | (uint32_t)dzs[3];
    if (chunks == 0 || chunks > 256) { return; }
    for (uint32_t i = 0; i < chunks; i++) {
        const uint32_t h = 4 + i * 12;
        if (h + 12 > dzsLen) { break; }
        if (std::memcmp(dzs + h, "STAG", 4) != 0) { continue; }
        const uint32_t off = ((uint32_t)dzs[h+8] << 24) | ((uint32_t)dzs[h+9] << 16) |
                             ((uint32_t)dzs[h+10] << 8) | (uint32_t)dzs[h+11];
        if (off + 0x10 > dzsLen) { break; }   // refuse rather than read past
        const uint32_t v = ((uint32_t)dzs[off+0x0C] << 24) | ((uint32_t)dzs[off+0x0D] << 16) |
                           ((uint32_t)dzs[off+0x0E] << 8) | (uint32_t)dzs[off+0x0F];
        wwRegistry_setStageType(stage, (int)((v >> 16) & 7));
        return;
    }
}

// Read `SCLS` out of a served sea room and record every destination's parent.
// No-op for anything that is not a sea room arc.
void sniffSeaExits(const char* discPath, const void* arcBytes, uint32_t size) {
    if (discPath == nullptr || arcBytes == nullptr) {
        return;
    }
    // Only `res/Stage/sea/RoomNN.arc`. The parent axis is the ISLAND, so an
    // interior's own exits (which point back out to sea) must not be read as
    // parentage — that would make every island a child of its own interior.
    const char* sea = std::strstr(discPath, "Stage/sea/Room");
    if (sea == nullptr) {
        return;
    }
    int room = -1;
    if (std::sscanf(sea + 14, "%d", &room) != 1 || room < 1 || room > 49) {
        return;
    }
    uint32_t dzrLen = 0;
    const unsigned char* dzr =
        (const unsigned char*)wwMessage_rarcFind(arcBytes, size, "room.dzr", &dzrLen);
    if (dzr == nullptr || dzrLen < 4) {
        return;   // rarcFind already logged a named reason
    }
    const uint32_t chunks = ((uint32_t)dzr[0] << 24) | ((uint32_t)dzr[1] << 16) |
                            ((uint32_t)dzr[2] << 8) | (uint32_t)dzr[3];
    if (chunks == 0 || chunks > 256) {
        return;   // refuse a nonsense table rather than walk it
    }
    for (uint32_t i = 0; i < chunks; i++) {
        const uint32_t h = 4 + i * 12;
        if (h + 12 > dzrLen) {
            break;
        }
        if (std::memcmp(dzr + h, "SCLS", 4) != 0) {
            continue;
        }
        const uint32_t cnt = ((uint32_t)dzr[h+4] << 24) | ((uint32_t)dzr[h+5] << 16) |
                             ((uint32_t)dzr[h+6] << 8) | (uint32_t)dzr[h+7];
        const uint32_t off = ((uint32_t)dzr[h+8] << 24) | ((uint32_t)dzr[h+9] << 16) |
                             ((uint32_t)dzr[h+10] << 8) | (uint32_t)dzr[h+11];
        for (uint32_t k = 0; k < cnt; k++) {
            const uint32_t e = off + k * 0xC;
            if (e + 9 > dzrLen) {
                break;
            }
            char nm[9];
            std::memcpy(nm, dzr + e, 8);
            nm[8] = 0;
            // A blank or non-printable name is a padding row, not a stage.
            bool ok = nm[0] != 0;
            for (int c = 0; ok && nm[c] != 0; c++) {
                if (nm[c] < 0x20 || nm[c] >= 0x7F) {
                    ok = false;
                }
            }
            if (ok && std::strcmp(nm, "sea") != 0) {
                noteStageParent(nm, room);
                // Publish to the menu's sort keys as well. Local copy
                // stays for the boot receipt; the registry needs it to
                // nest this stage under its island.
                wwRegistry_setStageParent(nm, room);
            }
        }
    }
}

bool serve_file(const DonorDiscService* disc, int32_t index, uint64_t* io_bytes,
                const char* i_asPath = nullptr) {
    const char* path = nullptr;
    uint32_t size = 0;
    if (disc->file_info(mod_ctx, index, &path, &size) != MOD_OK) {
        return false;
    }
    std::vector<uint8_t> bytes(size);
    uint32_t got = 0;
    if (disc->read(mod_ctx, index, 0, bytes.data(), size, &got) != MOD_OK || got != size) {
        logf(LOG_LEVEL_ERROR, "read failed for '%s' (%u of %u bytes)", path, got, size);
        return false;
    }
    const std::string discPath = std::string("/") + (i_asPath != nullptr ? i_asPath : path);
    OverlayHandle handle = 0;
    if (s_overlay->add_buffer(mod_ctx, discPath.c_str(), bytes.data(), bytes.size(), &handle) !=
        MOD_OK)
    {
        logf(LOG_LEVEL_ERROR, "overlay registration failed for '%s'", discPath.c_str());
        return false;
    }
    s_served.push_back(handle);
    *io_bytes += size;
    // The bytes are in hand RIGHT HERE and about to go out of scope. Reading
    // the room's exit table now costs a chunk-table walk and ZERO extra I/O;
    // doing it later would mean re-reading up to 12 MB of room arcs to
    // recover something we already had. See sniffSeaExits().
    sniffSeaExits(path, bytes.data(), size);
    sniffStageType(path, bytes.data(), size);
    return true;
}

void drop_served() {
    for (OverlayHandle handle : s_served) {
        s_overlay->remove(mod_ctx, handle);
    }
    s_served.clear();
}

// ============================================================================
// (Re)build the served set from the current config. Called at initialize and
// from the config-change subscriptions.
// ============================================================================
// ============================================================================
// WAVE-1 row 14 (user ruling 2026-08-11): the USER-FACING disc field is the
// HOST's generic `backend.extraIsoPath` — in dusklight, agnostic naming, no
// WW anywhere in it. This plugin CONSUMES that field (plain plugin-side I/O
// on config.json, same as its other disc reads); the mod-scoped
// mod.wwDonorDisc.wwIsoPath remains as a developer override/fallback only.
// Host key wins when set.
// ============================================================================
std::string read_host_extra_iso() {
    const char* appdata = getenv("APPDATA");
    if (appdata == NULL) {
        return {};
    }
    const std::string cfg = std::string(appdata) + "\\TwilitRealm\\Dusklight\\config.json";
    FILE* f = std::fopen(cfg.c_str(), "rb");
    if (f == NULL) {
        return {};
    }
    std::string text;
    char buf[4096];
    size_t got;
    while ((got = std::fread(buf, 1, sizeof(buf), f)) > 0) {
        text.append(buf, got);
    }
    std::fclose(f);
    const std::string key = "\"backend.extraIsoPath\"";
    size_t pos = text.find(key);
    if (pos == std::string::npos) {
        return {};
    }
    pos = text.find(':', pos + key.size());
    if (pos == std::string::npos) {
        return {};
    }
    pos = text.find('"', pos);
    if (pos == std::string::npos) {
        return {};
    }
    size_t end = pos + 1;
    std::string out;
    while (end < text.size() && text[end] != '"') {
        if (text[end] == '\\' && end + 1 < text.size()) {
            ++end;  // JSON escape: keep the escaped char (covers \\ in paths)
        }
        out.push_back(text[end]);
        ++end;
    }
    return out;
}

void rebuild() {
    const DonorDiscService* disc = wwDonorDisc_service();
    drop_served();

    std::string isoPath = read_host_extra_iso();
    if (!isoPath.empty()) {
        logf(LOG_LEVEL_INFO, "using host backend.extraIsoPath (row-14 generic field)");
    } else {
        isoPath = get_string_var(s_varIsoPath);
    }
    if (isoPath.empty()) {
        disc->detach(mod_ctx);
        s_attachedPath.clear();
        // 19c: dormant is a legible, normal state — one line, names the key.
        logf(LOG_LEVEL_INFO,
            "donor disc not configured — donor stages stay dormant. Set "
            "mod.wwDonorDisc.wwIsoPath to your own Wind Waker (USA) .iso to serve them "
            "from disc.");
        return;
    }

    if (isoPath != s_attachedPath || disc->verdict(mod_ctx) != DONOR_DISC_ON_ROSTER) {
        ModError err = MOD_ERROR_INIT;
        if (disc->attach(mod_ctx, isoPath.c_str(), &err) != MOD_OK) {
            s_attachedPath.clear();
            // 19c: the refusal reason comes from the reader, already legible.
            logf(LOG_LEVEL_ERROR, "%s", err.message);
            return;
        }
        s_attachedPath = isoPath;
        logf(LOG_LEVEL_INFO, "attached '%s' — GZLE01, roster verdict ON-ROSTER "
            "(sys/boot.bin + sys/fst.bin match the sanctioned dump), %d files in FST",
            isoPath.c_str(), disc->file_count(mod_ctx));
    }

    // ------------------------------------------------------------------------
    // Serve /res/Stage/<stage>/* for each declared stage, plus the donor
    // message pair the WW message system mounts (/res/Msg/bmgres.arc +
    // bmgresh.arc — one archive set for the whole donor game, no per-stage
    // groups). Vanilla names throughout; the WW layer's alias does the rest.
    // ------------------------------------------------------------------------
    // The warp menu's destination tree is DERIVED FROM THIS SERVE, not typed
    // in the menu. Cleared first because a config change re-runs the whole
    // pass and stale routes would offer stages that are no longer mounted.
    wwRegistry_clearRoutes();

    const std::string stagesCsv = get_string_var(s_varStages);
    const std::vector<std::string> stages = split_csv(stagesCsv);
    uint64_t bytes = 0;
    int files = 0;
    int aliases = 0;        // second mount points added, not files served
    int aliasesBefore = 0;  // per-stage watermark for the no-alias warning
    const int32_t count = disc->file_count(mod_ctx);
    for (const std::string& stage : stages) {
        const std::string prefix = "res/Stage/" + stage + "/";
        int before = files;
        for (int32_t i = 0; i < count; i++) {
            const char* path = nullptr;
            if (disc->file_info(mod_ctx, i, &path, nullptr) != MOD_OK) {
                continue;
            }
            if (path_has_prefix_ci(path, prefix.c_str()) && serve_file(disc, i, &bytes)) {
                files++;
                // ------------------------------------------------------------
                // ARC-NAME ALIASES — a SECOND mount for the same bytes, never
                // a replacement. The vanilla mount happened on the line above
                // and is untouched, which is the user's requirement stated
                // back: WW keeps reading its own files under its own names,
                // guaranteed by construction rather than by care.
                //
                // WHY A SECOND CALL AND NOT AN `asPath` ON THE FIRST: that
                // loop is what mounts all 224 files under their donor names.
                // Passing an override there MOVES the mount point instead of
                // adding one, and WW would stop finding its own data. One
                // serve_file call = one mount point; two calls = two names,
                // same bytes (`serve_file` copies into the overlay, so the
                // duplicate costs memory — flagged below, accepted here).
                //
                // AND WHY THIS IS NOT THE `res/Object` PATTERN: those aliases
                // exist because donor names COLLIDE with the receiver's own
                // (`Always.arc`), so there only one name may be served. Stage
                // arcs are DIRECTORY-SCOPED — `/res/Stage/sea/Stg_00.arc`
                // cannot collide with `/res/Stage/F_SP103/Stg_00.arc` — so
                // here both names are wanted, and copying that pattern would
                // have made us drop the donor one.
                //
                // BOTH LEAF FORMS ARE MEASURED AT SOURCE, NOT INFERRED:
                //   · the `.arc` suffix — `d_resorce.cpp:78` builds the real
                //     open as `snprintf(path, "%s%s.arc", i_path, arcName)`,
                //     so the receiver asking for `Stg_00` opens `Stg_00.arc`.
                //   · the room form — `d_com_inf_game.cpp:2931` is
                //     `SAFE_SPRINTF(buf, "R%02d_00", i_roomNo)`, so room 44 is
                //     `R44_00` and the padding is two digits, not a guess off
                //     the two samples (`R00_00`, `R01_00`) we had in a log.
                // ------------------------------------------------------------
                const char* leaf = std::strrchr(path, '/');
                leaf = (leaf != nullptr) ? leaf + 1 : path;
                char aliasLeaf[32] = {0};
                int roomNo = -1;
                if (str_eq_ci(leaf, "Stage.arc")) {
                    std::snprintf(aliasLeaf, sizeof(aliasLeaf), "Stg_00.arc");
                } else if (std::sscanf(leaf, "Room%d.arc", &roomNo) == 1 && roomNo >= 0) {
                    std::snprintf(aliasLeaf, sizeof(aliasLeaf), "R%02d_00.arc", roomNo);
                    // b3: this room is now MOUNTED, so it is a real warp
                    // destination. Recording it here rather than in the menu is
                    // what makes the tree disc-derived — the alias pass already
                    // knows every room the user's own disc provides, and the
                    // hardcoded menu knew only `sea` room 44 while serving ~49
                    // more. A destination the plugin cannot serve is never
                    // offered, and one it CAN serve is never hidden.
                    wwRegistry_addRoute(stage.c_str(), roomNo);
                }
                if (aliasLeaf[0] != '\0') {
                    const std::string aliasPath = prefix + aliasLeaf;
                    if (serve_file(disc, i, &bytes, aliasPath.c_str())) {
                        aliases++;
                        logf(LOG_LEVEL_INFO,
                            "arc-name alias: disc '%s' ALSO served as '/%s' "
                            "(bytes verbatim, donor mount kept)",
                            path, aliasPath.c_str());
                    } else {
                        logf(LOG_LEVEL_ERROR,
                            "arc-name alias FAILED for '%s' -> '/%s' — the "
                            "receiver will ask for this name and miss",
                            path, aliasPath.c_str());
                    }
                }
            }
        }
        if (files == before) {
            logf(LOG_LEVEL_WARN, "declared stage '%s' has no files under /%s on the disc",
                stage.c_str(), prefix.c_str());
        }
        // A declared stage that mounts files but produces NO stage alias can
        // never be entered: the receiver opens `<stage>/Stg_00.arc` first and
        // nothing else is tried. Warn at MOUNT rather than let it surface as a
        // black screen later — that silence is what cost seven boots.
        if (files > before && aliases == aliasesBefore) {
            logf(LOG_LEVEL_WARN,
                "declared stage '%s' served %d file(s) but produced NO arc-name "
                "alias — no 'Stage.arc' on the disc under /%s, so the receiver's "
                "'%sStg_00.arc' will miss and the stage cannot be entered",
                stage.c_str(), files - before, prefix.c_str(), prefix.c_str());
        }
        aliasesBefore = aliases;
    }
    // ------------------------------------------------------------------------
    // REMOVED 2026-08-16 BY USER ORDER (§1023/§1024) — the `kMsgPair` serve.
    //
    // WHAT IT DID: mounted donor `res/Msg/bmgres.arc` + `bmgresh.arc` at the
    // RECEIVER'S OWN PATHS, unconditionally at mount time, with no WW-stage
    // gate. The overlay is last-one-wins, and TP's own comment
    // (`d_msg_object.cpp:1846`) names `/res/Msg/bmgres.arc` as its whole-game
    // message archive — so this replaced the receiver's messages for EVERY
    // stage, from boot, including pure TP play.
    //
    // ⚠ WITHDRAWN 2026-08-16 BY THE INTEGRATOR — THIS PARAGRAPH WAS WRONG ON
    // BOTH CLAIMS, AND IT WAS MY ERROR THAT HOUSING QUOTED IN GOOD FAITH.
    // Left in place rather than deleted, because three lanes read it and the
    // withdrawal has to be visible where the claim was.
    //
    // IT SAID: "WHAT IT COST: the Outset crash … that pointer ends in `0C` and
    // is therefore not 8-byte aligned, so it was never a `JKRArchive*`."
    //
    // BOTH HALVES ARE FALSE, MEASURED:
    //   (1) THE REMOVAL DID NOT FIX THE CRASH. Run 105922, with this serve
    //       gone, crashed identically — same `zel_00.bmg`, same fault address,
    //       same stack. The causal claim was never tested before it was
    //       written, and I had explicitly bounded it as unproven.
    //   (2) THE ALIGNMENT ARGUMENT IS NONSENSE. Run 111600 is a pure TP boot
    //       with no warp: `zel_00.bmg` resolves to `…9204980C` — ALSO ending
    //       in `0C`, ALSO not 8-aligned — TWICE, and the game runs fine.
    //       **A misaligned value there is NORMAL for this pointer.** I built a
    //       diagnosis on an invariant I never checked against a control.
    //
    // WHAT THE BASELINE ACTUALLY SHOWS, and it is a better lead: on TP both
    // `zel_00.bmg` lookups return the SAME pointer; on the WW stage two
    // lookups of the same slot returned DIFFERENT pointers. `mMsgDtArchive[0]`
    // is written in exactly one place — `d_s_logo.cpp:928`, at boot — so a
    // value that cannot change, changing, means the memory holding it is
    // clobbered during the WW stage load. **Stability, not alignment.**
    //
    // THE REMOVAL ITSELF STANDS. A colliding mount on the receiver's own
    // whole-game message path is wrong on its own terms, which is why it was
    // ordered independently of the crash theory.
    //
    // WHY "IT WORKED BEFORE" WAS NOT EVIDENCE OF SAFETY, recorded because it
    // is the reasoning error that let this survive two rounds as a mere
    // suspect: TP stages DID load under the same override earlier in the same
    // run. The override is survivable right up until something RE-FETCHES
    // messages on a scene change — which is what METER2 creation does.
    //
    // WHY IT IS WRONG IN PRINCIPLE AND NOT ONLY HERE: this is the `res/Object`
    // COLLISION case, not the stage-arc case. Stage arcs are directory-scoped
    // so both names can coexist; these names collide outright, so only one may
    // win — and on the receiver's own path, the receiver must win.
    //
    // AND THE FIX IS NOT AN ALIAS FOR `zel_00.bmg`: that name is TP's and the
    // donor set has no equivalent. The fix is to stop occupying the path.
    //
    // NOTHING IS LOST FOR WW: the same two stems are still served through the
    // `kMsgToObjectAlias` re-home below, at `res/Object/<stem>.arc`, which is
    // where the WW dialogue consumer actually reads them. That block is
    // deliberately untouched.
    // ------------------------------------------------------------------------

    // ------------------------------------------------------------------------
    // W1a SELF-PROOF — open the donor's real message archive off the user's own
    // disc and report what the reader found (CALLS §837).
    //
    // WHY THIS EXISTS RATHER THAN A UNIT TEST: every value the reader depends
    // on was measured against THIS disc — INF1's table at 0x10, the 32-byte
    // size unit, entrySize 0x18 read from the file. A test against synthetic
    // bytes would prove the code matches my understanding, which is the thing
    // in doubt. This proves it against the artifact.
    //
    // AND IT MUST NOT BE A GREEN LINE THAT MEANS NOTHING: a refusal logs its
    // reason by name, and the counters distinguish "reader never ran" from
    // "reader ran and found zero" — the same distinction that made
    // `set_stage_res_calls: 0` unreadable for a whole boot.
    //
    // READ-ONLY AND NON-FATAL. Nothing downstream depends on it yet; if the
    // archive is absent the plugin serves exactly as before.
    {
        static const char* const kBmgArc = "res/Msg/bmgres.arc";
        static const char* const kBmgMember = "zel_00.bmg";
        // The SECOND member of the same archive — the donor's own RGBA8
        // palette. Named here beside the BMG because they ship together and
        // are read from one buffer.
        static const char* const kBmcMember = "color.bmc";
        int32_t bmgIdx = -1;
        if (disc->find(mod_ctx, kBmgArc, &bmgIdx) != MOD_OK) {
            logf(LOG_LEVEL_WARN,
                "[WwMsg] {\"ev\":\"selfproof_skipped\",\"why\":\"'%s' not on the "
                "disc — reader UNEXERCISED, which is not the same as failing\"}",
                kBmgArc);
        } else {
            const char* bmgPath = nullptr;
            uint32_t bmgSize = 0;
            if (disc->file_info(mod_ctx, bmgIdx, &bmgPath, &bmgSize) == MOD_OK &&
                bmgSize > 0u)
            {
                // RETAINED FOR THE SESSION (was a local vector): the vanilla
                // slot-1 owned mount (registry.cpp msg-group seam) mem-mounts
                // these bytes per WW scene — JKRArchive::mount(void*,heap,dir)
                // dedupes by buffer pointer and constructs with break-flag 0,
                // so the receiver NEVER frees this buffer; one copy serves
                // every scene. The self-proof semantics below are unchanged
                // (the reader is still closed again after the pass — holding
                // the BYTES is not holding parser pointers into them).
                static std::vector<uint8_t> arcBytes;
                arcBytes.resize(bmgSize);
                uint32_t got = 0;
                if (disc->read(mod_ctx, bmgIdx, 0, arcBytes.data(), bmgSize, &got) == MOD_OK &&
                    got == bmgSize)
                {
                    wwRegistry_setBmgArcBytes(arcBytes.data(), bmgSize);
                    if (wwMessage_openFromArc(arcBytes.data(), bmgSize, kBmgMember)) {
                        WwMessageEntry e0 = {};
                        const bool got0 = wwMessage_get(0, &e0);
                        logf(LOG_LEVEL_INFO,
                            "[WwMsg] {\"ev\":\"selfproof\",\"arc\":\"%s\",\"member\":\"%s\","
                            "\"entries\":%u,\"entry0_resolved\":%d,\"entry0_dataOffset\":%u,"
                            "\"entry0_textboxType\":%u,\"entry0_textLen\":%u,"
                            "\"entry0_strlen\":%u,\"note\":\"parsed from the user's own "
                            "disc, not a fixture; textLen vs strlen is TRAP 7 - they "
                            "DIFFER whenever the message carries a colour tag, and the "
                            "strlen value is the truncated one\"}",
                            kBmgArc, kBmgMember, wwMessage_count(), got0 ? 1 : 0,
                            e0.dataOffset, e0.textboxType, e0.textLen,
                            (got0 && e0.text != nullptr)
                                ? (unsigned)std::strlen(e0.text) : 0u);
                    }
                    // ====================================================
                    // THE COLOUR TABLE, FROM THE SAME ARCHIVE. `color.bmc`
                    // is the OTHER member of `bmgres.arc`, already in this
                    // buffer — no new serve, no second mount, just the
                    // consumer the estate was missing.
                    //
                    // WHY IT IS PROVEN HERE RATHER THAN ASSUMED: the
                    // receiver's `kWwColor[9]` (d_ext_dmesg.cpp:313) is a
                    // hand-written approximation that disagrees with this
                    // file in 7 of 9 slots, and colour is 58.9% of the
                    // corpus tags. This line is what makes the donor's own
                    // palette a measured fact at boot instead of a claim.
                    // ====================================================
                    if (wwMessageColor_openFromArc(arcBytes.data(), bmgSize,
                                                   kBmcMember)) {
                        logf(LOG_LEVEL_INFO,
                            "[WwMsg] {\"ev\":\"selfproof_color\",\"member\":\"%s\","
                            "\"distinct\":%u,\"idx0\":\"%08X\",\"idx1\":\"%08X\","
                            "\"idx3\":\"%08X\",\"idx8\":\"%08X\","
                            "\"hardcoded_idx1\":\"FF5A5AFF\","
                            "\"note\":\"idx1/idx8 are two of the SEVEN slots where "
                            "the hardcoded kWwColor disagrees with the donor; "
                            "idx0 white and idx3 blue are the only two that match\"}",
                            kBmcMember, wwMessageColor_distinctCount(),
                            wwMessageColor_get(0), wwMessageColor_get(1),
                            wwMessageColor_get(3), wwMessageColor_get(8));
                    }
                    // ====================================================
                    // NAME THE WARP DESTINATIONS FROM THE DONOR'S OWN TABLE.
                    //
                    // MEASURED, not a table anyone typed: the island name for
                    // `sea` room N is message `3295 + N`. Anchored on room 44
                    // resolving to "Outset Island", which is independently
                    // known to be Outset's room in the 50-room `sea` stage —
                    // that anchor is what turns a contiguous block into a
                    // proven mapping.
                    //
                    // TAGS MUST BE STRIPPED AND THIS IS NOT OPTIONAL: six of
                    // the 49 names BEGIN with a 0x1A colour tag (Forsaken
                    // Fortress, Windfall, Dragon Roost, Greatfish, Forest
                    // Haven and Outset — the donor colours its story-critical
                    // islands). A colour tag's code high byte IS a NUL, so a
                    // `%s` of the raw text prints EMPTY for exactly those six
                    // — the six a player looks for first. Hence textLen and
                    // the tag-aware copy below.
                    // ====================================================
                    int named = 0;
                    for (int room = 1; room <= 49; room++) {
                        WwMessageEntry e = {};
                        if (!wwMessage_get((uint32_t)(3295 + room), &e) ||
                            e.text == nullptr || e.textLen == 0) {
                            continue;
                        }
                        char nm[32];
                        unsigned o = 0;
                        for (unsigned k = 0; k < e.textLen && o + 1 < sizeof(nm);) {
                            const unsigned char c = (unsigned char)e.text[k];
                            if (c == 0x1A) {                 // consume tag whole
                                const unsigned char sz =
                                    (k + 1 < e.textLen) ? (unsigned char)e.text[k + 1] : 0;
                                k += (sz >= 5) ? sz : 1;
                                continue;
                            }
                            if (c >= 0x20 && c < 0x7F) {
                                nm[o++] = (char)c;
                            }
                            k++;
                        }
                        nm[o] = 0;
                        if (o > 0) {
                            wwRegistry_setRouteName("sea", room, nm);
                            named++;
                        }
                    }
                    logf(LOG_LEVEL_INFO,
                        "[WwMsg] {\"ev\":\"warp_names\",\"sea_rooms_named\":%d,"
                        "\"formula\":\"msgID = 3295 + room\",\"room44\":\"%s\","
                        "\"note\":\"names read from the donor's own table; a "
                        "room that never mounted is skipped, so fewer than 49 "
                        "is normal and not a failure\"}",
                        named, named > 0 ? "resolved" : "NOT RESOLVED");

                    // Both readers closed with the buffer they point into.
                    wwMessageColor_close();
                    wwMessage_close();

                    // ====================================================
                    // OUTSET INTERIORS — AUTHORED NAMES (user ruling
                    // 2026-08-16: "always authored names").
                    //
                    // THE DONOR HAS NO NAMES FOR THESE. Measured, not
                    // assumed: zero labels for any Outset resident across
                    // all 4,411 corpus entries, and all 12 interior stage
                    // archives hold no text but `event_list.dat` (cutscene
                    // definitions). DN-10 step 1 has no answer here, so
                    // these are step 3 — authored, with the user's go.
                    //
                    // THE WORDING IS AUTHORED; THE MAPPING IS MEASURED. Each
                    // house was identified by parsing its own room `.dzr`
                    // ACTR chunks and looking the actors up in the donor's
                    // source: Ji1=Orca, Aj1=Sturgeon, Ob1=Rose,
                    // Ym1="NPC - Mesa & Abe", Ba1=Grandma, Yw1=Sue-Belle.
                    // Nothing here was recalled.
                    //
                    // `Ojhous` vs `Ojhous2`: DIFFERENT BUILDINGS, not
                    // variants — every model/collision/layout member differs
                    // by hash, only Ojhous2 has Sue-Belle, and the donor
                    // itself branches on "Ojhous2" in its SKYBOX
                    // (d_a_vrbox2.cpp:130) and RAIN (d_kankyo_rain.cpp:1350)
                    // code, so that one sees the sky. The split is measured;
                    // WHICH is Orca's and which is Sturgeon's is the one
                    // inference in this block.
                    // ====================================================
                    struct WwInteriorName { const char* stage; const char* name; };
                    static const WwInteriorName kInteriors[] = {
                        {"LinkRM",  "Link's House"},
                        {"Ojhous",  "Orca's House"},
                        {"Ojhous2", "Sturgeon's House"},
                        {"Omasao",  "Mesa's House"},
                        {"Onobuta", "Abe & Rose's House"},
                    };
                    // ====================================================
                    // THE PARENT MAP, REPORTED. `sniffSeaExits()` filled this
                    // during the serve pass at zero extra I/O; this is where
                    // it becomes visible and checkable.
                    //
                    // DELIBERATELY NOT FOLDED INTO THE LABEL HERE. Label
                    // composition lives in `composeRouteLabel()` in
                    // registry.cpp, which another lane is actively building
                    // (the file-name toggle). Two lanes already collided in
                    // that exact file this session — once producing a latent
                    // link error and once a silent shadow — so the DATA is
                    // published and the COMPOSITION is left to its owner.
                    // ====================================================
                    {
                        const int p44 = stageParentRoom("Ojhous");
                        logf(LOG_LEVEL_INFO,
                            "[WwMsg] {\"ev\":\"warp_parents\",\"stages_mapped\":%d,"
                            "\"saturated\":%d,\"Ojhous_parent_room\":%d,"
                            "\"note\":\"parent island per stage, read from each sea "
                            "room's SCLS exit table during serve - zero extra I/O. "
                            "Ojhous should read 44 (Outset). 0 mapped means the "
                            "SCLS walk found nothing and grouping is UNAVAILABLE, "
                            "not empty\"}",
                            s_stageParentCount, s_stageParentsSaturated ? 1 : 0, p44);
                    }
                    for (const WwInteriorName& in : kInteriors) {
                        // Interiors are small; name every room they mount.
                        // A (stage, room) that never mounted is skipped by
                        // the setter, so over-supplying costs nothing.
                        for (int r = 0; r <= 1; r++) {
                            wwRegistry_setRouteName(in.stage, r, in.name);
                        }
                    }

                    // ====================================================
                    // ORDER THE MENU. Runs LAST, after every route is added
                    // (serve pass), named (islands + interiors above) and
                    // given its donor type/parent - every sort key must
                    // exist before the permutation, or rows sort into a
                    // group not yet assigned.
                    //
                    // THIS IS THE PART THAT COST A RUN: a flat 81-row list
                    // meant the user could not find Outset, warped somewhere
                    // else, and the draw probe never reached `R44_00`.
                    // ====================================================
                    wwRegistry_finalizeRoutes();
                } else {
                    logf(LOG_LEVEL_ERROR,
                        "[WwMsg] {\"ev\":\"selfproof_read_failed\",\"path\":\"%s\","
                        "\"got\":%u,\"want\":%u}", kBmgArc, got, bmgSize);
                }
            }
        }
    }

    // ------------------------------------------------------------------------
    // THE MESSAGE-BOX SCREEN ARCHIVES — `msgres.arc` + `tmsgres.arc`.
    //
    // WHY THEY ARE HERE AT ALL: History's §1011 decode located W3's data and it
    // is NOT code to write — `dMesg_screenData_c::createScreen()` builds a
    // `J2DScreen` from donor BLO archives. `msgres.arc` holds the 30
    // `hukidashi_*.blo` speech-bubble screens; `tmsgres.arc` holds
    // `baton_input.blo`, the conductor screen behind the `INPUT_e`/`TACT_e`/
    // `DEMO_e` states that were recorded as unmappable. **Neither was served.**
    // `dmsgres` (the demo variants) already rides the alias table below, so the
    // split was: demo screens present, main screens absent.
    //
    // WHY AT THE DONOR'S OWN PATH AND NOT RE-HOMED TO `res/Object/` LIKE THE
    // SEVEN BELOW — and this is the distinction the `kMsgPair` removal just
    // taught us, applied deliberately rather than by pattern-matching:
    //   · that re-home exists for the FORK's `d_ext_dmesg`, which requests
    //     those seven stems as OBJECT arcs. `d_ext_dmesg` is fork-only and
    //     MISSING on vanilla, so on a clean dusklight the rename buys nothing.
    //   · a rename puts another donor archive into the receiver's `res/Object/`
    //     namespace, which is the COLLISION surface. Donor-path serving keeps
    //     them off it entirely.
    //   · zero-bake: the asset is never renamed, so the lookup moves only when
    //     it must. Here it must not.
    //
    // AND THE SAFETY ARGUMENT IS THE EXACT ONE `bmgres` FAILED, run in reverse:
    // a donor mount is only dangerous where the RECEIVER READS THAT PATH. TP
    // owns and reads `/res/Msg/bmgres.arc` (`d_msg_object.cpp:1846`) — which
    // is why that mount crashed Outset. **No receiver code path requests
    // `msgres` or `tmsgres` at any path**, so these add files rather than
    // shadowing them. The overlay contract is explicit that a path absent from
    // the disc is added as a new file.
    //
    // MOUNT ONLY. This serves the bytes; nothing reads them yet. The consumer
    // is the W3 half of the WW message port, which stays BLOCKED—PASS 2 —
    // but the data being present lets that half be de-risked before any
    // behaviour is written, instead of discovering an empty mount later.
    // ------------------------------------------------------------------------
    static const char* const kWwScreenArcs[] = {"msgres", "tmsgres"};
    for (const char* stem : kWwScreenArcs) {
        const std::string screenPath = std::string("res/Msg/") + stem + ".arc";
        int32_t index = -1;
        if (disc->find(mod_ctx, screenPath.c_str(), &index) == MOD_OK &&
            serve_file(disc, index, &bytes))
        {
            files++;
            logf(LOG_LEVEL_INFO,
                "message-box screens: '%s' served at the donor's own path "
                "(bytes verbatim, no rename — the receiver reads no such path)",
                screenPath.c_str());
        } else {
            logf(LOG_LEVEL_WARN,
                "message-box screen archive '%s' NOT on the disc — W3's screen "
                "data will be absent and the message box will have nothing to "
                "build from",
                screenPath.c_str());
        }
    }

    // ------------------------------------------------------------------------
    // §716: the res/Object serve set (mount retirement — banner at the top).
    // Curated stems only, never the full Object directory (hundreds of arcs
    // the receiver would never request). Case-insensitive match against the
    // FST so config casing never silently misses (the VshiN lesson).
    // ------------------------------------------------------------------------
    const std::vector<std::string> objStems = split_csv(get_string_var(s_varObjectArcs));
    int objFiles = 0;
    for (const std::string& stem : objStems) {
        const std::string discPath = "res/Object/" + stem + ".arc";
        bool found = false;
        for (int32_t i = 0; i < count; i++) {
            const char* path = nullptr;
            if (disc->file_info(mod_ctx, i, &path, nullptr) != MOD_OK) {
                continue;
            }
            if (path_has_prefix_ci(path, discPath.c_str()) &&
                std::strlen(path) == discPath.size() && serve_file(disc, i, &bytes))
            {
                objFiles++;
                found = true;
                break;
            }
        }
        if (!found) {
            logf(LOG_LEVEL_WARN, "declared Object arc '%s' is not on the disc — check "
                "mod.wwDonorDisc.wwObjectArcs (assembled arcs stay with the mount)",
                stem.c_str());
        }
    }
    for (const char* stem : kMsgToObjectAlias) {
        const std::string msgPath = std::string("res/Msg/") + stem + ".arc";
        const std::string objPath = std::string("res/Object/") + stem + ".arc";
        int32_t index = -1;
        if (disc->find(mod_ctx, msgPath.c_str(), &index) == MOD_OK &&
            serve_file(disc, index, &bytes, objPath.c_str()))
        {
            objFiles++;
        }
    }
    // §806: TP-collision aliases — donor bytes verbatim, renamed mount point.
    for (const TpCollisionAlias& alias : kTpCollisionAlias) {
        const std::string discPath = std::string("res/Object/") + alias.discStem + ".arc";
        const std::string asPath = std::string("res/Object/") + alias.serveStem + ".arc";
        int32_t index = -1;
        if (disc->find(mod_ctx, discPath.c_str(), &index) == MOD_OK &&
            serve_file(disc, index, &bytes, asPath.c_str()))
        {
            objFiles++;
            logf(LOG_LEVEL_INFO, "collision alias: disc '%s' served as '%s' (bytes verbatim)",
                discPath.c_str(), asPath.c_str());
        } else {
            logf(LOG_LEVEL_WARN, "collision alias '%s' not served — donor '%s' missing?",
                alias.serveStem, discPath.c_str());
        }
    }
    files += objFiles;
    logf(LOG_LEVEL_INFO, "res/Object serve set: %d archive(s) from disc (%d curated + %d "
        "Msg-alias) — the R2 mount's staged copies are superseded for these names",
        objFiles, (int)objStems.size(), (int)(sizeof(kMsgToObjectAlias) / sizeof(char*)));

    logf(LOG_LEVEL_INFO,
        "serving %d donor file(s) (%.1f MiB) from disc [+%d arc-name alias mount(s), "
        "bytes duplicated] for stage set '%s' — nothing staged, "
        "nothing shipped",
        files, double(bytes) / (1024.0 * 1024.0), aliases, stagesCsv.c_str());
}

void on_config_changed(ModContext*, ConfigVarHandle, const ConfigVarValue*, const ConfigVarValue*,
    void*) {
    rebuild();
}

// The file-name toggle does NOT rebuild the serve set - it only relabels an
// existing menu. Calling `rebuild()` here would remount every archive to
// change a string, which is the kind of over-reaction that turns a display
// preference into a load-bearing operation.
void on_warp_filenames_changed(ModContext*, ConfigVarHandle,
    const ConfigVarValue* /*prev*/, const ConfigVarValue* next, void*) {
    wwRegistry_setShowFileNames(next != nullptr && next->bool_value);
}

}  // namespace

MOD_EXTERN_C MOD_EXPORT ModResult mod_initialize(ModError* out_error) {
    ConfigVarDesc isoDesc = CONFIG_VAR_DESC_INIT;
    isoDesc.name = "wwIsoPath";
    isoDesc.type = CONFIG_VAR_STRING;
    isoDesc.default_string = "";
    if (s_config->register_var(mod_ctx, &isoDesc, &s_varIsoPath) != MOD_OK) {
        return mods::set_error(out_error, MOD_ERROR, "failed to register wwIsoPath");
    }

    ConfigVarDesc stagesDesc = CONFIG_VAR_DESC_INIT;
    stagesDesc.name = "wwStages";
    stagesDesc.type = CONFIG_VAR_STRING;
    stagesDesc.default_string = "sea";
    if (s_config->register_var(mod_ctx, &stagesDesc, &s_varStages) != MOD_OK) {
        return mods::set_error(out_error, MOD_ERROR, "failed to register wwStages");
    }

    ConfigVarDesc objDesc = CONFIG_VAR_DESC_INIT;
    objDesc.name = "wwObjectArcs";
    objDesc.type = CONFIG_VAR_STRING;
    objDesc.default_string = kDefaultObjectArcs;
    if (s_config->register_var(mod_ctx, &objDesc, &s_varObjectArcs) != MOD_OK) {
        return mods::set_error(out_error, MOD_ERROR, "failed to register wwObjectArcs");
    }

    // ========================================================================
    // WARP-MENU FILE-NAME TOGGLE — user ruling 2026-08-16: *"warp rows should
    // present by their in-canon names first with the toggle revealing their
    // file names alongside them. Just like dusklight does for TP rows."*
    //
    // DEFAULT OFF, so the menu reads as canon names — that is the ruling's
    // "first". The token is never lost, only hidden: turning this on appends
    // `[stage room]`, and a route with no established canon name shows its
    // token ALONE regardless of this setting, because an unnamed row must
    // degrade to UNLABELLED and never to a guess.
    // ========================================================================
    ConfigVarDesc fileNamesDesc = CONFIG_VAR_DESC_INIT;
    fileNamesDesc.name = "wwWarpShowFileNames";
    fileNamesDesc.type = CONFIG_VAR_BOOL;
    fileNamesDesc.default_bool = false;
    if (s_config->register_var(mod_ctx, &fileNamesDesc, &s_varWarpFileNames) != MOD_OK) {
        return mods::set_error(out_error, MOD_ERROR,
                               "failed to register wwWarpShowFileNames");
    }
    s_config->subscribe(mod_ctx, s_varWarpFileNames, on_warp_filenames_changed,
                        nullptr, nullptr);

    // ========================================================================
    // DIAGNOSTIC PROBES — DEFAULT OFF, and the default is the whole point.
    // The collision/scene investigation attached hooks to genuinely HOT paths
    // (the ground query, the per-node tree walk, the per-triangle check, the
    // per-frame create/draw pumps). Those earned their keep while the fault
    // was unlocated, and they cost the user real FPS on BOTH images once the
    // trace moved into per-frame code. A silenced handler still pays for its
    // trampoline, so this switch is read at INSTALL and the hot hooks are not
    // ATTACHED at all when it is off. Flip it on for one diagnostic boot.
    // ========================================================================
    ConfigVarDesc diagDesc = CONFIG_VAR_DESC_INIT;
    diagDesc.name = "wwDiagProbes";
    diagDesc.type = CONFIG_VAR_BOOL;
    diagDesc.default_bool = false;
    ConfigVarHandle varDiag = {};
    bool diagOn = false;
    if (s_config->register_var(mod_ctx, &diagDesc, &varDiag) == MOD_OK) {
        bool v = false;
        if (s_config->get_bool(mod_ctx, varDiag, &v) == MOD_OK) {
            diagOn = v;
        }
    }
    wwRegistry_setDiagProbes(diagOn);

    s_config->subscribe(mod_ctx, s_varIsoPath, on_config_changed, nullptr, nullptr);
    s_config->subscribe(mod_ctx, s_varStages, on_config_changed, nullptr, nullptr);
    s_config->subscribe(mod_ctx, s_varObjectArcs, on_config_changed, nullptr, nullptr);

    // A missing/wrong disc must NOT fail the mod load (19c: handled condition,
    // legible refusal) — rebuild() logs and leaves the layer dormant.
    rebuild();

    // ------------------------------------------------------------------------
    // THE ACTOR REGISTRY, merged in from the former `ww_registry.dusk`
    // (Integrator assignment, CALLS row 570). Its result is deliberately NOT
    // propagated: a registry that cannot resolve its hooks leaves the actor
    // vehicle inert, which is exactly the state this plugin shipped in before
    // the merge — and failing the whole mod load would take the DISC READER
    // down with it, losing a service that works for the sake of one that did
    // not start. Same reasoning as the missing-disc case just above.
    // ------------------------------------------------------------------------
    wwRegistry_initialize();
    return MOD_OK;
}

// ============================================================================
// mod_update is a REQUIRED export (the loader refuses the DLL without it —
// "missing required mod API exports"). This mod is event-driven: everything
// happens at initialize and on config-change callbacks, so the frame tick has
// nothing to do.
// ============================================================================
MOD_EXTERN_C MOD_EXPORT ModResult mod_update(ModError* /*out_error*/) {
    return MOD_OK;
}

MOD_EXTERN_C MOD_EXPORT ModResult mod_shutdown(ModError* /*out_error*/) {
    wwRegistry_shutdown();
    drop_served();
    wwDonorDisc_service()->detach(mod_ctx);
    return MOD_OK;
}
