// KIT-LINEAGE: native-port
// KIT-DONOR: JSystem/JParticle/JPAEmitterLoader.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: Matching
// ============================================================================
// WW JPA LANE — phase 1: JPAC1-00 archive parse (native, no conversion).
//
// Direct port of JPAEmitterArchiveLoader_v10::load()
// (WW JSystem/JParticle/JPAEmitterLoader.cpp:96-185), with the same block walk,
// the same header fields, and the same in-place block pointers. Where the donor
// allocates *Arc accessor objects, this keeps raw pointers and reads through the
// accessors in ww_jpa.h — identical semantics, no per-emitter allocation.
//
// This replaces the JPAC1→JPAC2 byte converter in d_particle.cpp. The converter
// had to invent a second format and translate every field and flag word; this
// reads the donor archive the way the donor reads it, so there is nothing to
// translate and nothing to get wrong.
// ============================================================================

#include "d/dolzel_rel.h"  // IWYU pragma: keep

#if TARGET_PC

#include "d/ww_jpa.h"

#include <cstring>

namespace ww_jpa {

// Archive header (JPAEmitterArchiveData_v10, JPAEmitterLoader.cpp:69-74):
//   0x00 'JPAC' · 0x04 '1-00' · 0x08 u16 emtrResNum · 0x0A u16 texResNum
// Per-emitter header (JPAEmitterParticleHeader_v10, :76-86):
//   0x00 'JEFF' · 0x04 'jpa1' · 0x0C u32 blockNum · 0x14 keyNum · 0x15 fldNum
//   0x16 textureNum · 0x18 u16 resID   (payload blocks begin at +0x20)
// Block header (JPAEmitterBlockHeader_v10, :88-93):
//   0x00 magic · 0x04 u32 size · 0x0C blockData
bool Archive::parse(const u8* data, u32 size) {
    mData = nullptr;
    mResN = mTexN = 0;
    if (data == nullptr || size < 0x20) {
        return false;
    }
    if (std::memcmp(data, "JPAC", 4) != 0 || std::memcmp(data + 4, "1-00", 4) != 0) {
        return false;  // not a donor v10 archive — caller keeps its own path
    }
    mResCount = be16(data + 0x08);
    mTexCount = be16(data + 0x0A);

    u32 offs = 0x20;
    for (u16 i = 0; i < mResCount; i++) {
        if (offs + 0x20 > size || mResN >= kMaxRes) {
            return false;
        }
        const u8* ptcl = data + offs;
        Resource r;
        r.keyNum = ptcl[0x14];
        r.fldNum = ptcl[0x15];
        r.textureNum = ptcl[0x16];
        r.resId = be16(ptcl + 0x18);
        const u32 blockNum = be32(ptcl + 0x0C);

        u32 fldN = 0;
        u32 keyN = 0;
        u32 blockOffs = offs + 0x20;
        for (u32 j = 0; j < blockNum; j++) {
            if (blockOffs + 8 > size) {
                return false;
            }
            const u8* block = data + blockOffs;
            const u32 bsize = be32(block + 4);
            if (bsize == 0 || blockOffs + bsize > size) {
                return false;  // malformed: never loop on a zero-size block
            }
            if (std::memcmp(block, "FLD1", 4) == 0) {
                if (fldN < 8) {
                    r.fld1[fldN++] = block;
                }
            } else if (std::memcmp(block, "KFA1", 4) == 0) {
                if (keyN < 8) {
                    r.kfa1[keyN++] = block;
                }
            } else if (std::memcmp(block, "BEM1", 4) == 0) {
                r.bem1 = block;
            } else if (std::memcmp(block, "BSP1", 4) == 0) {
                r.bsp1 = block;
            } else if (std::memcmp(block, "ESP1", 4) == 0) {
                r.esp1 = block;
            } else if (std::memcmp(block, "SSP1", 4) == 0) {
                r.ssp1 = block;
            } else if (std::memcmp(block, "ETX1", 4) == 0) {
                r.etx1 = block;
            } else if (std::memcmp(block, "TDB1", 4) == 0) {
                r.texIdxTable = block + 0x0C;  // donor: &block->blockData
            }
            blockOffs += bsize;
        }
        mRes[mResN++] = r;
        offs = blockOffs;  // donor advances to the block walk's end, not a stride
    }

    // Texture section follows the last emitter, one JUTDataBlockHeader each.
    for (u16 i = 0; i < mTexCount; i++) {
        if (offs + 8 > size || mTexN >= kMaxTex) {
            break;
        }
        const u32 tsize = be32(data + offs + 4);
        if (tsize == 0 || offs + tsize > size) {
            break;
        }
        Texture t;
        t.block = data + offs;
        mTex[mTexN++] = t;
        offs += tsize;
    }

    mData = data;
    mSize = size;
    return true;
}

const Resource* Archive::find(u16 resId) const {
    for (int i = 0; i < mResN; i++) {
        if (mRes[i].resId == resId) {
            return &mRes[i];
        }
    }
    return nullptr;
}

const Texture* Archive::texture(u16 idx) const {
    if ((int)idx >= mTexN) {
        return nullptr;
    }
    return &mTex[idx];
}

}  // namespace ww_jpa

#endif  // TARGET_PC
