#ifndef D_ALBW_OUTFIT_DEBUG_H
#define D_ALBW_OUTFIT_DEBUG_H

// Temporary outfit-cycle diagnostics. Set to 0 before ship.
#define D_ALBW_OUTFIT_SWAP_DEBUG 1

#if TARGET_PC && D_ALBW_OUTFIT_SWAP_DEBUG

void dAlbwOutfit_debugLog(const char* fmt, ...);

#else

#define dAlbwOutfit_debugLog(...) ((void)0)

#endif

#endif /* D_ALBW_OUTFIT_DEBUG_H */
