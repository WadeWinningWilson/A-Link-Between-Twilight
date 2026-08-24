#ifndef D_EXT_SAVE_FLAGS_H
#define D_EXT_SAVE_FLAGS_H

#include "global.h"

inline void dExtWwSv_reset() {}
inline u32 dExtWwSv_storeSize() { return 0; }
inline void dExtWwSv_store(u8*) {}
inline BOOL dExtWwSv_restore(const u8*) { return FALSE; }
inline u32 dExtWwSv_rawSize() { return 0; }
inline void dExtWwSv_exportRaw(u8*) {}
inline void dExtWwSv_importRaw(const u8*) {}
inline BOOL dExtWwSv_isEventBit(u16) { return FALSE; }
inline void dExtWwSv_onEventBit(u16) {}
inline void dExtWwSv_offEventBit(u16) {}
inline void dExtWwSv_setEventReg(u16, u8) {}
inline u8 dExtWwSv_getEventReg(u16) { return 0; }
inline BOOL dExtWwSvIo_restoreSlot(int) { return FALSE; }
inline void dExtWwSvIo_storeSlot(int) {}
inline void dExtWwSvIo_eraseSlot(int) {}
inline void dExtWwSvIo_copySlot(int, int) {}

#endif
