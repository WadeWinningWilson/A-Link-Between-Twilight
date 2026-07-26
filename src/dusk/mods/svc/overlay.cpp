#include "registry.hpp"
#include "slot_map.hpp"
#include "overlay_host.hpp"

#include "aurora/dvd.h"
#include "aurora/lib/logging.hpp"
#include "dusk/custom_assets.hpp"
#include "dusk/mods/loader/loader.hpp"
#include "mods/svc/overlay.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std::string_literals;

namespace dusk::mods::svc {
namespace {

aurora::Module Log("dusk::mods::overlay");

struct OverlayFileData {
    std::string bundlePath;
    std::shared_ptr<ModBundle> bundle;
    std::shared_ptr<const std::vector<u8> > buffer;
    std::string absPath;  // Layer-A loose file (fopen); empty for .dusk bundle/buffer
};

// Keyed by the id passed to Aurora as per-file userdata. Guarded by s_overlayMutex: Aurora may
// call cbOpen from a DVD thread while the game thread replaces the set in overlay_sync_files.
// The shared bundle/buffer pointer keeps a disabled/reloaded mod's data readable until the last
// open completes.
std::unordered_map<uintptr_t, OverlayFileData> s_overlayFiles;
uintptr_t s_nextOverlayId = 1;
std::mutex s_overlayMutex;

struct RuntimeOverlaySlot {
    std::string discPath;
    std::string bundlePath;                         // bundle-backed if non-empty
    std::shared_ptr<const std::vector<u8>> buffer;  // buffer-backed otherwise
    size_t size = 0;
    uint64_t order = 0;
};
SlotMap<RuntimeOverlaySlot> s_runtimeOverlays;
uint64_t s_nextRuntimeOrder = 0;
bool s_overlaysDirty = false;

// Aurora matches overlay paths against the disc case-insensitively and later entries win, so
// claims are tracked by lowercased path and re-claims by a different mod warn.
void claim_overlay_path(std::unordered_map<std::string, const LoadedMod*>& claims,
    const std::string& discPath, const LoadedMod& mod) {
    std::string key = discPath;
    for (auto& ch : key) {
        if (ch >= 'A' && ch <= 'Z') {
            ch += 'a' - 'A';
        }
    }
    const auto [it, inserted] = claims.try_emplace(std::move(key), &mod);
    if (!inserted && it->second != &mod) {
        Log.warn("Overlay conflict: '{}' is provided by both '{}' and '{}'; '{}' wins.", discPath,
            it->second->metadata.id, mod.metadata.id, mod.metadata.id);
        it->second = &mod;
    }
}

void find_overlay_files(std::vector<AuroraOverlayFile>& files, LoadedMod& mod,
    std::unordered_map<std::string, const LoadedMod*>& claims) {
    for (const auto& file : mod.bundle->getFileNames()) {
        if (!file.starts_with("overlay/")) {
            continue;
        }

        auto overlayPath = file.substr("overlay/"s.size());
        assert(!overlayPath.starts_with('/'));
        overlayPath.insert(0, "/");

        const auto size = mod.bundle->getFileSize(file);

        const auto id = s_nextOverlayId++;
        s_overlayFiles.emplace(id, OverlayFileData{file, mod.bundle, nullptr});
        claim_overlay_path(claims, overlayPath, mod);
        files.emplace_back(strdup(overlayPath.c_str()), reinterpret_cast<void*>(id), size);
    }
}

void append_runtime_overlays(std::vector<AuroraOverlayFile>& files, LoadedMod& mod,
    std::unordered_map<std::string, const LoadedMod*>& claims) {
    // Aurora resolves duplicate paths later-entry-wins, so emit in registration order (SlotMap
    // iteration is index order, and freed indices are reused).
    std::vector<const RuntimeOverlaySlot*> slots;
    s_runtimeOverlays.for_each([&](uint64_t, const auto& entry) {
        if (entry.owner == &mod) {
            slots.push_back(&entry.value);
        }
    });
    std::ranges::sort(slots, {}, &RuntimeOverlaySlot::order);

    for (const auto* slot : slots) {
        const auto id = s_nextOverlayId++;
        if (slot->buffer != nullptr) {
            s_overlayFiles.emplace(id, OverlayFileData{{}, nullptr, slot->buffer});
        } else {
            s_overlayFiles.emplace(id, OverlayFileData{slot->bundlePath, mod.bundle, nullptr});
        }
        claim_overlay_path(claims, slot->discPath, mod);
        files.emplace_back(strdup(slot->discPath.c_str()), reinterpret_cast<void*>(id), slot->size);
    }
}

struct OpenOverlayFile {
    std::vector<u8> ownedData;
    std::shared_ptr<const std::vector<u8> > shared;
    FILE* file = nullptr;
    size_t pos = 0;

    [[nodiscard]] const std::vector<u8>& data() const {
        return shared != nullptr ? *shared : ownedData;
    }
};

void* cbOpen(void* userdata) {
    const auto id = reinterpret_cast<uintptr_t>(userdata);
    OverlayFileData fileData;
    {
        std::lock_guard lock{s_overlayMutex};
        const auto it = s_overlayFiles.find(id);
        if (it == s_overlayFiles.end()) {
            // The overlay set was re-pushed between the FST lookup and this call.
            return nullptr;
        }
        fileData = it->second;
    }

    if (!fileData.absPath.empty()) {
        FILE* fp = std::fopen(fileData.absPath.c_str(), "rb");
        if (fp == nullptr) {
            Log.error("Failed to open Layer-A overlay file {}", fileData.absPath);
            return nullptr;
        }
        return new OpenOverlayFile{.file = fp};
    }

    if (fileData.buffer != nullptr) {
        return new OpenOverlayFile{.shared = std::move(fileData.buffer)};
    }

    try {
        auto fileContents = fileData.bundle->readFile(fileData.bundlePath);
        return new OpenOverlayFile{.ownedData = std::move(fileContents)};
    } catch (const std::runtime_error& e) {
        Log.error("Failed to read overlay file {}: {}", fileData.bundlePath, e.what());
        return nullptr;
    }
}

void cbClose(void* handle) {
    const auto openFile = static_cast<OpenOverlayFile*>(handle);
    if (openFile != nullptr && openFile->file != nullptr) {
        std::fclose(openFile->file);
        openFile->file = nullptr;
    }
    delete openFile;
}

int64_t cbRead(void* handle, uint8_t* buf, const size_t len) {
    auto& openFile = *static_cast<OpenOverlayFile*>(handle);
    if (openFile.file != nullptr) {
        const size_t got = std::fread(buf, 1, len, openFile.file);
        if (got < len && std::ferror(openFile.file)) {
            return -1;
        }
        return static_cast<int64_t>(got);
    }

    const auto remainingSpace = openFile.data().size() - openFile.pos;
    const auto toRead = std::min(remainingSpace, len);
    std::memcpy(buf, openFile.data().data() + openFile.pos, toRead);
    openFile.pos += toRead;
    return static_cast<int64_t>(toRead);
}

int64_t cbSeek(void* handle, int64_t offset, int32_t whence) {
    if (whence != 0) {
        Log.fatal("Invalid seek mode from aurora: {}", whence);
    }

    auto& openFile = *static_cast<OpenOverlayFile*>(handle);
    if (openFile.file != nullptr) {
        if (std::fseek(openFile.file, static_cast<long>(offset), SEEK_SET) != 0) {
            return -1;
        }
        return static_cast<int64_t>(std::ftell(openFile.file));
    }

    const auto posSigned =
        std::clamp(offset, static_cast<int64_t>(0), static_cast<int64_t>(openFile.data().size()));
    openFile.pos = static_cast<size_t>(posSigned);
    return posSigned;
}

constexpr AuroraOverlayCallbacks s_overlayCallbacks = {
    .open = cbOpen,
    .close = cbClose,
    .read = cbRead,
    .seek = cbSeek,
};

void free_strdup_file_names(const std::vector<AuroraOverlayFile>& files,
                            const std::vector<std::string>& layerOwnedPaths) {
    std::unordered_map<const char*, bool> layerOwned;
    for (const auto& s : layerOwnedPaths) {
        layerOwned[s.c_str()] = true;
    }
    for (const auto& file : files) {
        if (file.fileName != nullptr && layerOwned.find(file.fileName) == layerOwned.end()) {
            std::free(const_cast<char*>(file.fileName));
        }
    }
}

// Signature for skip-if-unchanged. Must NOT touch s_overlayFiles (find_overlay_files
// registers IDs — a probe that early-returns would leave the FST pointing at freed ids).
std::vector<std::string> united_overlay_sig(
    const std::vector<dusk::custom_assets::DvdOverlayFile>& layerA) {
    std::vector<std::string> sig;
    sig.reserve(layerA.size() + 32);
    for (const auto& entry : layerA) {
        sig.push_back("A|" + entry.discPath + "|" + std::to_string(entry.size) + "|" +
                      (entry.absPathCstr != nullptr ? entry.absPathCstr : ""));
    }
    for (auto& mod : ModLoader::instance().active_mods()) {
        if (mod.bundle == nullptr) {
            continue;
        }
        for (const auto& file : mod.bundle->getFileNames()) {
            if (!file.starts_with("overlay/")) {
                continue;
            }
            auto overlayPath = file.substr("overlay/"s.size());
            overlayPath.insert(0, "/");
            size_t size = 0;
            try {
                size = mod.bundle->getFileSize(file);
            } catch (...) {
                continue;
            }
            sig.push_back("D|" + overlayPath + "|" + std::to_string(size) + "|" + mod.metadata.id);
        }
        s_runtimeOverlays.for_each([&](uint64_t, const auto& entry) {
            if (entry.owner != &mod) {
                return;
            }
            sig.push_back("R|" + entry.value.discPath + "|" + std::to_string(entry.value.size) +
                          "|" + mod.metadata.id + "|" + std::to_string(entry.value.order));
        });
    }
    std::sort(sig.begin(), sig.end());
    return sig;
}

void overlay_sync_files_impl() {
    static bool callbacksRegistered = false;
    if (!callbacksRegistered) {
        aurora_dvd_overlay_callbacks(&s_overlayCallbacks);
        callbacksRegistered = true;
    }

    s_overlaysDirty = false;

    std::vector<dusk::custom_assets::DvdOverlayFile> layerA;
    dusk::custom_assets::collect_dvd_overlays(layerA);

    static std::vector<std::string> s_lastUnitedSig;
    auto sig = united_overlay_sig(layerA);
    if (sig == s_lastUnitedSig) {
        Log.info("United DVD overlay unchanged ({} Layer-A file(s)) — skip FST rebuild",
                 layerA.size());
        return;
    }

    std::vector<std::string> discPathStorage;
    std::vector<AuroraOverlayFile> files;
    std::unordered_map<std::string, const LoadedMod*> claims;
    {
        std::lock_guard lock{s_overlayMutex};
        s_overlayFiles.clear();

        // Layer A first (model_replacements — Linkle/Beta clothes RARCs, etc.).
        // .dusk package overlays append after and win on path conflicts (later entry).
        discPathStorage.reserve(layerA.size());
        for (const auto& entry : layerA) {
            discPathStorage.push_back(entry.discPath);
            const auto id = s_nextOverlayId++;
            s_overlayFiles.emplace(id, OverlayFileData{
                                           .bundlePath = {},
                                           .bundle = nullptr,
                                           .buffer = nullptr,
                                           .absPath = entry.absPathCstr != nullptr
                                                          ? std::string(entry.absPathCstr)
                                                          : std::string{},
                                       });
            files.emplace_back(discPathStorage.back().c_str(), reinterpret_cast<void*>(id),
                               entry.size);
        }

        for (auto& mod : ModLoader::instance().active_mods()) {
            find_overlay_files(files, mod, claims);
            append_runtime_overlays(files, mod, claims);
        }
    }

    Log.info("Registering united DVD overlays: {} Layer-A + {} total file(s)", layerA.size(),
             files.size());
    aurora_dvd_overlay_files(files.data(), files.size(), nullptr);
    s_lastUnitedSig = std::move(sig);
    dusk::custom_assets::commit_dvd_overlay_push(layerA);
    free_strdup_file_names(files, discPathStorage);
}

uint64_t overlay_add_file(
    LoadedMod& mod, std::string discPath, std::string bundlePath, size_t size) {
    const auto handle = s_runtimeOverlays.emplace(mod, RuntimeOverlaySlot{
                                                           .discPath = std::move(discPath),
                                                           .bundlePath = std::move(bundlePath),
                                                           .size = size,
                                                           .order = s_nextRuntimeOrder++,
                                                       });
    s_overlaysDirty = true;
    return handle;
}

uint64_t overlay_add_buffer(LoadedMod& mod, std::string discPath, std::vector<u8> data) {
    const auto size = data.size();
    const auto handle = s_runtimeOverlays.emplace(mod,
        RuntimeOverlaySlot{
            .discPath = std::move(discPath),
            .buffer = std::make_shared<const std::vector<u8>>(std::move(data)),
            .size = size,
            .order = s_nextRuntimeOrder++,
        });
    s_overlaysDirty = true;
    return handle;
}

bool overlay_remove(LoadedMod& mod, uint64_t handle) {
    if (!s_runtimeOverlays.erase_owned(handle, mod)) {
        return false;
    }
    s_overlaysDirty = true;
    return true;
}

void overlay_remove_mod(LoadedMod& mod) {
    if (s_runtimeOverlays.erase_all(mod) != 0) {
        s_overlaysDirty = true;
    }
}

bool consume_overlays_dirty() {
    return std::exchange(s_overlaysDirty, false);
}

constexpr size_t kMaxOverlayFileSize = UINT32_MAX;

bool is_valid_disc_path(const char* discPath) {
    if (discPath == nullptr) {
        return false;
    }
    const std::string_view path{discPath};
    return path.starts_with('/') && is_safe_resource_path(path.substr(1));
}

ModResult overlay_add_file(
    ModContext* context, const char* discPath, const char* bundlePath, OverlayHandle* outHandle) {
    if (outHandle != nullptr) {
        *outHandle = 0;
    }
    auto* mod = mod_from_context(context);
    if (mod == nullptr || !is_valid_disc_path(discPath) || bundlePath == nullptr ||
        !is_safe_resource_path(bundlePath))
    {
        return MOD_INVALID_ARGUMENT;
    }

    size_t size = 0;
    try {
        size = mod->bundle->getFileSize(bundlePath);
    } catch (const std::exception& e) {
        Log.error(
            "[{}] overlay add_file '{}' failed: {}", mod->metadata.id, bundlePath, e.what());
        return MOD_UNAVAILABLE;
    }
    if (size > kMaxOverlayFileSize) {
        Log.error("[{}] overlay add_file '{}' failed: file too large ({} bytes)",
            mod->metadata.id, bundlePath, size);
        return MOD_INVALID_ARGUMENT;
    }

    const auto handle = overlay_add_file(*mod, discPath, bundlePath, size);
    if (outHandle != nullptr) {
        *outHandle = handle;
    }
    return MOD_OK;
}

ModResult overlay_add_buffer(ModContext* context, const char* discPath, const void* data,
    size_t size, OverlayHandle* outHandle) {
    if (outHandle != nullptr) {
        *outHandle = 0;
    }
    auto* mod = mod_from_context(context);
    if (mod == nullptr || !is_valid_disc_path(discPath) || (data == nullptr && size != 0) ||
        size > kMaxOverlayFileSize)
    {
        return MOD_INVALID_ARGUMENT;
    }

    const auto* bytes = static_cast<const u8*>(data);
    const auto handle = overlay_add_buffer(*mod, discPath, std::vector<u8>{bytes, bytes + size});
    if (outHandle != nullptr) {
        *outHandle = handle;
    }
    return MOD_OK;
}

ModResult overlay_remove(ModContext* context, OverlayHandle handle) {
    auto* mod = mod_from_context(context);
    if (mod == nullptr || handle == 0) {
        return MOD_INVALID_ARGUMENT;
    }
    if (!overlay_remove(*mod, handle)) {
        Log.error("[{}] overlay remove failed: unknown handle {}", mod->metadata.id, handle);
        return MOD_INVALID_ARGUMENT;
    }
    return MOD_OK;
}

constexpr OverlayService s_overlayService{
    .header = SERVICE_HEADER(OverlayService, OVERLAY_SERVICE_MAJOR, OVERLAY_SERVICE_MINOR),
    .add_file = overlay_add_file,
    .add_buffer = overlay_add_buffer,
    .remove = overlay_remove,
};

}  // namespace

void overlay_sync_files() {
    overlay_sync_files_impl();
}

constinit const ServiceModule g_overlayModule{
    .id = OVERLAY_SERVICE_ID,
    .majorVersion = OVERLAY_SERVICE_MAJOR,
    .minorVersion = OVERLAY_SERVICE_MINOR,
    .service = &s_overlayService,
    .modDetached = overlay_remove_mod,
    .lifecycleApplied = overlay_sync_files,
    .frameEnd =
        [] {
            if (consume_overlays_dirty()) {
                overlay_sync_files();
            }
        },
};
}  // namespace dusk::mods::svc
