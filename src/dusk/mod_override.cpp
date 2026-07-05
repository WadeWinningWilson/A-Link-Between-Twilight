// ============================================
// NEW CODE — ALBW Port / Dusklight
// Phase 2a: whole-mod data-tree scan + path map (log-only). See mod_override.hpp.
// ============================================

#include "dusk/mod_override.hpp"

#if TARGET_PC

#include "dusk/main.h"      // ConfigPath
#include "dusk/logging.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace dusk::mod_override {
namespace {

// normalized game path ("res/object/kmdl.arc") -> absolute loose file path
std::unordered_map<std::string, std::string> s_map;

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

}  // namespace

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
            continue;  // not a full-mod tree (e.g. a single-BMD "Custom" folder)
        }

        ++mods;
        int count = 0;
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
            const std::string key = normalize(rel);
            s_map[key] = it->path().string();
            if (sample.empty()) {
                sample = key;
            }
            ++count;
        }
        DuskLog.info("[mod_override] '{}': {} override file(s) (e.g. {})",
                     modDir.path().filename().string(), count,
                     sample.empty() ? "-" : sample);
    }

    DuskLog.info("[mod_override] scan complete: {} data-tree mod(s), {} override path(s)",
                 mods, static_cast<int>(s_map.size()));
}

const char* find_override(const char* game_path) {
    if (game_path == nullptr || s_map.empty()) {
        return nullptr;
    }
    const auto it = s_map.find(normalize(game_path));
    return it == s_map.end() ? nullptr : it->second.c_str();
}

int count() {
    return static_cast<int>(s_map.size());
}

}  // namespace dusk::mod_override

#else

namespace dusk::mod_override {
void scan() {}
const char* find_override(const char*) { return nullptr; }
int count() { return 0; }
}  // namespace dusk::mod_override

#endif  // TARGET_PC
