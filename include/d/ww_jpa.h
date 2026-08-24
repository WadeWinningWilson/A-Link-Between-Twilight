#ifndef D_WW_JPA_H
#define D_WW_JPA_H

#include "dolphin/types.h"

class JPAResourceManager;
class JKRHeap;

namespace ww_jpa {

struct Archive {
    bool parse(const u8*, u32) { return false; }
    u16 resourceCount() const { return 0; }
    u16 textureCount() const { return 0; }
};

inline bool bindResource(Archive&, u16, JPAResourceManager*, JKRHeap*) { return false; }

}  // namespace ww_jpa

#endif
