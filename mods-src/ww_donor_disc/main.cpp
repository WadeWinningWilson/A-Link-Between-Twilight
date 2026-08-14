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
    const std::string stagesCsv = get_string_var(s_varStages);
    const std::vector<std::string> stages = split_csv(stagesCsv);
    uint64_t bytes = 0;
    int files = 0;
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
            }
        }
        if (files == before) {
            logf(LOG_LEVEL_WARN, "declared stage '%s' has no files under /%s on the disc",
                stage.c_str(), prefix.c_str());
        }
    }
    static const char* const kMsgPair[] = {"res/Msg/bmgres.arc", "res/Msg/bmgresh.arc"};
    for (const char* msgPath : kMsgPair) {
        int32_t index = -1;
        if (disc->find(mod_ctx, msgPath, &index) == MOD_OK && serve_file(disc, index, &bytes)) {
            files++;
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
        "serving %d donor file(s) (%.1f MiB) from disc for stage set '%s' — nothing staged, "
        "nothing shipped",
        files, double(bytes) / (1024.0 * 1024.0), stagesCsv.c_str());
}

void on_config_changed(ModContext*, ConfigVarHandle, const ConfigVarValue*, const ConfigVarValue*,
    void*) {
    rebuild();
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

    s_config->subscribe(mod_ctx, s_varIsoPath, on_config_changed, nullptr, nullptr);
    s_config->subscribe(mod_ctx, s_varStages, on_config_changed, nullptr, nullptr);
    s_config->subscribe(mod_ctx, s_varObjectArcs, on_config_changed, nullptr, nullptr);

    // A missing/wrong disc must NOT fail the mod load (19c: handled condition,
    // legible refusal) — rebuild() logs and leaves the layer dormant.
    rebuild();
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
    drop_served();
    wwDonorDisc_service()->detach(mod_ctx);
    return MOD_OK;
}
