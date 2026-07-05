// ============================================
// NEW CODE — ALBW Port / Dusklight
// Repack-free custom-model override loader. See model_override.hpp.
// ============================================

#include "dusk/model_override.hpp"

#if TARGET_PC

#include "dusk/main.h"      // ConfigPath
#include "dusk/logging.h"
#include "d/d_resorce.h"    // dRes_info_c::loaderBasicBmd (engine-standard BMDV finish)

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>

namespace dusk::model_override {

// Resolve <arc>_<index>.bmd in the model_replacements root OR any immediate
// subfolder (one "folder per custom model" — e.g. "Armogohma Custom/"). Returns
// an empty path if not found. One level deep only, so a full-mod data tree
// (e.g. "Linkle Mod NSTC/files/...") isn't crawled here.
static std::filesystem::path resolve_override(const char* fname) {
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
        std::filesystem::path candidate = entry.path() / fname;
        if (std::filesystem::exists(candidate, ec)) {
            return candidate;
        }
    }
    return {};
}

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
        DuskLog.warn("[model_override] load failed: {}", path.string());
        return nullptr;
    }

    // Retain 'raw' intentionally — the model references it. Falls back to the arc
    // automatically if we ever return nullptr, so this path is safe by construction.
    DuskLog.info("[model_override] using {} ({} bytes)", path.string(), size);
    return model_data;
}

}  // namespace dusk::model_override

#else

class J3DModelData;
namespace dusk::model_override {
J3DModelData* try_load(const char*, int) { return nullptr; }
}  // namespace dusk::model_override

#endif  // TARGET_PC
