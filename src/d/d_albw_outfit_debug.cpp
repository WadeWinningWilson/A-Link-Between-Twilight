// Temporary outfit quick-swap trace — remove when cycle/warp is stable.
#include "d/d_albw_outfit_debug.h"

#if TARGET_PC && D_ALBW_OUTFIT_SWAP_DEBUG

#include "dusk/logging.h"

#include <cstdarg>
#include <cstdio>
#include <filesystem>

namespace {

FILE* debugFile() {
    static FILE* sFile = nullptr;
    if (sFile != nullptr) {
        return sFile;
    }

    std::filesystem::path path = "outfit_swap_debug.txt";
    if (const char* logPath = dusk::GetLogFilePath()) {
        path = std::filesystem::path(logPath).parent_path() / "outfit_swap_debug.txt";
    }

    sFile = fopen(path.string().c_str(), "a");
    return sFile;
}

}  // namespace

void dAlbwOutfit_debugLog(const char* fmt, ...) {
    FILE* file = debugFile();
    if (file == nullptr) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(file, fmt, args);
    va_end(args);
    fputc('\n', file);
    fflush(file);
}

#endif
