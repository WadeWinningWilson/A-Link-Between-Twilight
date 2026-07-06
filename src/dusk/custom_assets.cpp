// ============================================
// NEW CODE — ALBW Port / Dusklight
// Custom Assets API implementation. See custom_assets.hpp for the layer model.
//   Layer A — Aurora DVD overlay (scan + install_overlays)
//   Layer B — loose single-BMD injection (try_load)
//   Shared  — per-folder enable toggle (gates both layers)
// ============================================

#include "dusk/custom_assets.hpp"

#if TARGET_PC

#include "dusk/main.h"      // ConfigPath
#include "dusk/logging.h"
#include "dusk/settings.h"  // game.customModelsDisabled
#include "d/d_resorce.h"    // dRes_info_c::loaderBasicBmd (engine-standard BMDV finish)

#include <aurora/dvd.h>     // aurora_dvd_overlay_files / _callbacks (virtual FST)

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace dusk::custom_assets {
namespace {

// normalized game path ("res/object/kmdl.arc") -> absolute loose file path
std::unordered_map<std::string, std::string> s_map;

// Bumped on each install_overlays() so resident-asset caches can detect a change.
int s_generation = 0;

std::string normalize(std::string p) {
    std::replace(p.begin(), p.end(), '\\', '/');
    while (!p.empty() && p.front() == '/') {
        p.erase(0, 1);
    }
    if (p.rfind("./", 0) == 0) {
        p.erase(0, 2);
    }
    std::transform(p.begin(), p.end(), p.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return p;
}

// Split the '|'-delimited disabled-folder list from the setting into tokens.
std::vector<std::string> disabled_list() {
    std::vector<std::string> out;
    const std::string s = getSettings().game.customModelsDisabled.getValue();
    std::string cur;
    for (char c : s) {
        if (c == '|') {
            if (!cur.empty()) out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

}  // namespace

// ============================================
// Shared — Custom Models editor list (folder enable/disable)
// ============================================
std::vector<std::string> list_folders() {
    std::vector<std::string> out;
    const std::filesystem::path root = ConfigPath / "model_replacements";
    std::error_code ec;
    if (!std::filesystem::exists(root, ec)) {
        return out;
    }
    for (const auto& entry : std::filesystem::directory_iterator(root, ec)) {
        if (entry.is_directory(ec)) {
            out.push_back(entry.path().filename().string());
        }
    }
    std::sort(out.begin(), out.end());
    return out;
}

bool is_folder_enabled(const char* folder) {
    if (folder == nullptr) {
        return true;
    }
    for (const std::string& d : disabled_list()) {
        if (d == folder) {
            return false;
        }
    }
    return true;
}

void toggle_folder(const char* folder) {
    if (folder == nullptr) {
        return;
    }
    std::vector<std::string> list = disabled_list();
    const auto it = std::find(list.begin(), list.end(), std::string(folder));
    if (it != list.end()) {
        list.erase(it);  // was disabled -> enable
    } else {
        list.emplace_back(folder);  // was enabled -> disable
    }
    std::string joined;
    for (size_t i = 0; i < list.size(); ++i) {
        if (i != 0) joined.push_back('|');
        joined += list[i];
    }
    getSettings().game.customModelsDisabled.setValue(joined);

    // Rebuild the map and re-register the overlay set so the change lands on the
    // next asset load (reload-scoped, texture-API style). Already-loaded assets
    // (mounted arcs, active audio banks) keep the old data until re-requested.
    scan();
    install_overlays();
}

// ============================================
// Layer A — whole-file DVD overlay
// ============================================
void scan() {
    s_map.clear();

    const std::filesystem::path root = ConfigPath / "model_replacements";
    std::error_code ec;
    if (!std::filesystem::exists(root, ec)) {
        return;
    }

    int mods = 0;
    for (const auto& modDir : std::filesystem::directory_iterator(root, ec)) {
        if (!modDir.is_directory(ec)) {
            continue;
        }
        const std::filesystem::path filesRoot = modDir.path() / "files";
        if (!std::filesystem::is_directory(filesRoot, ec)) {
            continue;  // not a full-mod tree (e.g. a single-BMD Layer-B folder)
        }
        if (!is_folder_enabled(modDir.path().filename().string().c_str())) {
            continue;  // folder disabled in the Custom Models editor list
        }

        ++mods;
        int count_ = 0;
        std::string sample;
        for (auto it = std::filesystem::recursive_directory_iterator(filesRoot, ec);
             it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
            if (ec) {
                break;
            }
            if (!it->is_regular_file(ec)) {
                continue;
            }
            const std::string rel =
                std::filesystem::relative(it->path(), filesRoot, ec).string();
            if (rel.empty()) {
                continue;
            }
            s_map[normalize(rel)] = it->path().string();
            if (sample.empty()) {
                sample = normalize(rel);
            }
            ++count_;
        }
        DuskLog.info("[custom_assets] '{}': {} override file(s) (e.g. {})",
                     modDir.path().filename().string(), count_,
                     sample.empty() ? "-" : sample);
    }

    DuskLog.info("[custom_assets] scan complete: {} data-tree mod(s), {} override path(s)",
                 mods, static_cast<int>(s_map.size()));
}

int count() {
    return static_cast<int>(s_map.size());
}

int overlay_generation() {
    return s_generation;
}

namespace {

// Program-lifetime backing store for the absolute loose paths handed to the
// overlay callbacks as userData. install_overlays() is re-runnable, so this pool
// is only ever appended to (deduped) — never cleared — which keeps every
// previously-issued c_str() pointer valid even across a rebuild. A std::deque
// never relocates its elements, so the pointers stay stable.
std::deque<std::string> s_pathPool;
std::unordered_map<std::string, const char*> s_pathCstr;

const char* stable_path(const std::string& abs) {
    const auto it = s_pathCstr.find(abs);
    if (it != s_pathCstr.end()) {
        return it->second;
    }
    s_pathPool.push_back(abs);
    const char* c = s_pathPool.back().c_str();
    s_pathCstr.emplace(abs, c);
    return c;
}

void* overlay_open(void* userdata) {
    return std::fopen(static_cast<const char*>(userdata), "rb");
}

void overlay_close(void* handle) {
    if (handle != nullptr) {
        std::fclose(static_cast<FILE*>(handle));
    }
}

std::int64_t overlay_read(void* handle, std::uint8_t* buf, std::size_t len) {
    FILE* fp = static_cast<FILE*>(handle);
    if (fp == nullptr) {
        return -1;
    }
    const std::size_t got = std::fread(buf, 1, len, fp);
    if (got < len && std::ferror(fp)) {
        return -1;
    }
    return static_cast<std::int64_t>(got);
}

std::int64_t overlay_seek(void* handle, std::int64_t offset, std::int32_t whence) {
    FILE* fp = static_cast<FILE*>(handle);
    if (fp == nullptr) {
        return -1;
    }
    if (std::fseek(fp, static_cast<long>(offset), whence) != 0) {
        return -1;
    }
    return static_cast<std::int64_t>(std::ftell(fp));
}

}  // namespace

void install_overlays() {
    // Callbacks are set exactly once, unconditionally, so that a later
    // re-registration (even one that STARTED from an empty set) never trips
    // Aurora's "callbacks-before-files" fatal check.
    static bool s_callbacksSet = false;
    if (!s_callbacksSet) {
        static const AuroraOverlayCallbacks kCallbacks = {
            overlay_open,
            overlay_close,
            overlay_read,
            overlay_seek,
        };
        aurora_dvd_overlay_callbacks(&kCallbacks);
        s_callbacksSet = true;
    }

    // Collect in one pass into a temp, then build the AuroraOverlayFile array so
    // every fileName pointer is stable before the (synchronous) hand-off to
    // Aurora. userData is backed by the persistent path pool so it survives
    // future rebuilds; the temp's name strings just need to outlive the call.
    struct Pending {
        std::string name;      // "/audiores/waves/...": disc-absolute
        const char* pathCstr;  // stable-pool userData
        std::size_t size;
    };
    std::vector<Pending> pending;
    pending.reserve(s_map.size());
    for (const auto& kv : s_map) {
        std::error_code ec;
        const auto sz = std::filesystem::file_size(kv.second, ec);
        if (ec) {
            continue;  // unreadable loose file — skip, disc entry stays
        }
        pending.push_back({"/" + kv.first, stable_path(kv.second),
                           static_cast<std::size_t>(sz)});
    }

    std::vector<AuroraOverlayFile> files;
    files.reserve(pending.size());
    for (const auto& p : pending) {
        AuroraOverlayFile f{};
        f.fileName = p.name.c_str();
        f.userData = const_cast<char*>(p.pathCstr);
        f.size = p.size;
        files.push_back(f);
    }

    // NOTE: passing 0 files is valid — it clears the overlay set (used when a
    // toggle disables the last remaining mod). Rebuilds the FST; reload-scoped.
    aurora_dvd_overlay_files(files.data(), files.size(), nullptr);
    ++s_generation;  // signal resident-asset caches (e.g. sumo body) to re-mount
    DuskLog.info("[custom_assets] installed {} DVD overlay file(s) (gen {})",
                 static_cast<int>(files.size()), s_generation);
}

// ============================================
// Layer B — loose single-BMD injection
// ============================================
namespace {

// Resolve <arc>_<index>.bmd in the model_replacements root OR any immediate
// ENABLED subfolder (one "folder per custom model", e.g. "Armogohma Custom/").
// Returns an empty path if not found. One level deep only.
std::filesystem::path resolve_override(const char* fname) {
    const std::filesystem::path root = ConfigPath / "model_replacements";
    std::error_code ec;
    if (!std::filesystem::exists(root, ec)) {
        return {};
    }
    std::filesystem::path direct = root / fname;
    if (std::filesystem::exists(direct, ec)) {
        return direct;
    }
    for (const auto& entry : std::filesystem::directory_iterator(root, ec)) {
        if (!entry.is_directory(ec)) {
            continue;
        }
        if (!is_folder_enabled(entry.path().filename().string().c_str())) {
            continue;  // folder disabled in the Custom Models editor list
        }
        std::filesystem::path candidate = entry.path() / fname;
        if (std::filesystem::exists(candidate, ec)) {
            return candidate;
        }
    }
    return {};
}

}  // namespace

J3DModelData* try_load(const char* arc_name, int res_index) {
    if (arc_name == nullptr) {
        return nullptr;
    }

    char fname[96];
    std::snprintf(fname, sizeof(fname), "%s_%d.bmd", arc_name, res_index);

    const std::filesystem::path path = resolve_override(fname);
    if (path.empty()) {
        return nullptr;  // no override — caller uses the arc
    }

    FILE* fp = std::fopen(path.string().c_str(), "rb");
    if (fp == nullptr) {
        return nullptr;
    }
    std::fseek(fp, 0, SEEK_END);
    const long size = std::ftell(fp);
    std::fseek(fp, 0, SEEK_SET);
    if (size <= 0) {
        std::fclose(fp);
        return nullptr;
    }

    // J3DModelLoaderDataBase fixes up pointers IN PLACE, so the buffer must be
    // >=32-byte aligned and must outlive the model (it binds to it as mpRawData).
    // Over-allocate and align; retain the raw block for the process lifetime.
    void* raw = std::malloc(static_cast<size_t>(size) + 32u);
    if (raw == nullptr) {
        std::fclose(fp);
        return nullptr;
    }
    void* buffer = reinterpret_cast<void*>(
        (reinterpret_cast<std::uintptr_t>(raw) + 31u) & ~static_cast<std::uintptr_t>(31u));

    const size_t got = std::fread(buffer, 1, static_cast<size_t>(size), fp);
    std::fclose(fp);
    if (got != static_cast<size_t>(size)) {
        std::free(raw);
        return nullptr;
    }

    J3DModelData* model_data = dRes_info_c::loaderBasicBmd('BMDV', buffer);
    if (model_data == nullptr) {
        std::free(raw);  // load failed; safe to release
        DuskLog.warn("[custom_assets] Layer-B load failed: {}", path.string());
        return nullptr;
    }

    // Retain 'raw' intentionally — the model references it. Falls back to the arc
    // automatically if we ever return nullptr, so this path is safe by construction.
    DuskLog.info("[custom_assets] Layer-B using {} ({} bytes)", path.string(), size);
    return model_data;
}

}  // namespace dusk::custom_assets

#else  // !TARGET_PC

class J3DModelData;
namespace dusk::custom_assets {
void scan() {}
void install_overlays() {}
J3DModelData* try_load(const char*, int) { return nullptr; }
std::vector<std::string> list_folders() { return {}; }
bool is_folder_enabled(const char*) { return true; }
void toggle_folder(const char*) {}
int count() { return 0; }
int overlay_generation() { return 0; }
}  // namespace dusk::custom_assets

#endif  // TARGET_PC
