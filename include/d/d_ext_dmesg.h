#ifndef D_EXT_DMESG_H
#define D_EXT_DMESG_H

class JKRArchive;
class JUTFont;

inline bool dExtDmesg_ensureResident() { return false; }
inline JKRArchive* dExtDmesg_getDmsgArchive() { return nullptr; }
inline JKRArchive* dExtDmesg_getMsgArchive() { return nullptr; }
inline JKRArchive* dExtDmesg_getMsgHArchive() { return nullptr; }
inline JKRArchive* dExtDmesg_getMenuArchive() { return nullptr; }
inline JUTFont* dExtDmesg_getFont() { return nullptr; }
inline JUTFont* dExtDmesg_getRFont() { return nullptr; }
inline const char* dExtDmesg_getMessageById(unsigned short) { return nullptr; }
inline void dExtDmesg_drawTestBox() {}
inline void dExtDmesg_setMessage(unsigned short) {}
inline void dExtDmesg_update() {}
inline bool dExtDmesg_isBoxActive() { return false; }
inline bool dExtDmesg_openTalk(unsigned short, const char*) { return false; }
inline bool dExtDmesg_isTalkActive() { return false; }
inline bool dExtDmesg_isTalkReleaseFrame() { return false; }
inline bool dExtDmesg_isTalkChainWindow() { return false; }
inline void dExtDmesg_rearmTalkChain() {}
inline void dExtDmesg_cancelTalk(const char*) {}

#endif
