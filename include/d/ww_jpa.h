#ifndef WW_JPA_H
#define WW_JPA_H

// ============================================================================
// WW JPA LANE — native JPAC1-00 ("jpa1") particle resources, read in place.
//
// WHY THIS EXISTS (user ruling, 2026-08-02: "We need to port the system. Never
// mounts."):
//
// The port previously reached WW particles through a hand-written JPAC1→JPAC2
// byte converter plus hand-authored GX passes. Both are BRIDGES under DN-9, and
// both failed repeatedly in ways that cost days: dropped blocks (§202), flag
// words whose meaning differs between the two JPA lineages (§212), a resolved
// bank id thrown away one line after it was computed (§223), and a
// reconstructed TEV that read an unset raster register and drew black (§231).
// Every one of those was an artefact of TRANSLATING donor data into receiver
// shapes instead of reading donor data the way the donor reads it.
//
// This lane does what the decomp does. WW's own *Arc classes
// (JPABaseShapeArc, JPAExtraShapeArc, JPAFieldBlockArc, …) are THIN ACCESSORS
// over the archive bytes — they never copy or convert; they point at the file
// and interpret it with WW's own field offsets and flag semantics. Ported the
// same way here: no conversion, no format invention, no flag translation.
// Sources: JSystem/JParticle/JPAEmitterLoader.cpp (v10 loader, :96-185) and the
// matching WW block headers; every offset below is cited to its header.
//
// ISOLATION (covenant + the user's own design ask): this lane owns WW resources
// only. TP's JPA is untouched — no shared state, no modified TP classes. If the
// mod folder is absent the lane simply has nothing to read, so it cannot leave
// residue in TP spaces.
//
// PHASES (this file = phase 1):
//   1. resource parse + accessors        ← here: replaces the byte converter
//   2. emitter + particle simulation     (JPAEmitter/JPAParticle/JPAField)
//   3. draw                              (JPADrawVisitor — the unlit donor path)
// ============================================================================

#include <dolphin/types.h>

#include <cstring>

namespace ww_jpa {

// --- big-endian readers: the archive is donor-endian, we read it in place ----
inline u16 be16(const u8* p) { return (u16)((p[0] << 8) | p[1]); }
inline u32 be32(const u8* p) {
    return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | (u32)p[3];
}
inline s16 bes16(const u8* p) { return (s16)be16(p); }
inline f32 bef32(const u8* p) {
    const u32 v = be32(p);
    f32 f;
    std::memcpy(&f, &v, 4);
    return f;
}

// ============================================================================
// Block accessors — WW layouts, WW flag semantics. Data pointer is the BLOCK
// start; payload begins at +0x0C (JPAEmitterBlockHeader_v10::blockData).
// ============================================================================
struct DynamicsBlock {  // BEM1 — JPADynamicsBlockData (WW JPADynamicsBlock.h:6)
    const u8* d;        // → payload
    u32 flags() const { return be32(d + 0x00); }
    u32 volumeType() const { return (flags() >> 8) & 0x07; }
    f32 volumeSweep() const { return bef32(d + 0x04); }
    f32 volumeMinRad() const { return bef32(d + 0x08); }
    u16 volumeSize() const { return be16(d + 0x0C); }
    u16 divNumber() const { return be16(d + 0x0E); }
    f32 rate() const { return bef32(d + 0x10); }
    f32 rateRndm() const { return bef32(d + 0x14); }
    u8 rateStep() const { return d[0x18]; }
    s16 maxFrame() const { return bes16(d + 0x1A); }
    s16 startFrame() const { return bes16(d + 0x1C); }
    s16 lifeTime() const { return bes16(d + 0x1E); }
    f32 lifeTimeRndm() const { return bef32(d + 0x20); }
    f32 initVelOmni() const { return bef32(d + 0x24); }
    f32 initVelAxis() const { return bef32(d + 0x28); }
    f32 initVelRndm() const { return bef32(d + 0x2C); }
    f32 initVelDir() const { return bef32(d + 0x30); }
    f32 initVelRatio() const { return bef32(d + 0x34); }
    f32 spread() const { return bef32(d + 0x38); }
    f32 airResist() const { return bef32(d + 0x3C); }
    f32 airResistRndm() const { return bef32(d + 0x40); }
    f32 moment() const { return bef32(d + 0x44); }
    f32 momentRndm() const { return bef32(d + 0x48); }
    f32 accel() const { return bef32(d + 0x4C); }
    f32 accelRndm() const { return bef32(d + 0x50); }
    f32 emitterScl(int i) const { return bef32(d + 0x54 + 4 * i); }
    f32 emitterTrs(int i) const { return bef32(d + 0x60 + 4 * i); }
    f32 emitterDir(int i) const { return bef32(d + 0x6C + 4 * i); }
    s16 emitterRot(int i) const { return bes16(d + 0x78 + 2 * i); }
};

struct BaseShapeBlock {  // BSP1 — JPABaseShapeData (WW JPABaseShape.h:100-121)
    const u8* d;
    u32 flags() const { return be32(d + 0x00); }
    u32 type() const { return flags() & 0x0F; }
    u32 dirType() const { return (flags() >> 4) & 0x07; }
    u32 rotType() const { return (flags() >> 7) & 0x07; }
    u32 basePlaneType() const { return (flags() >> 10) & 0x01; }
    u32 tevColorArgSel() const { return (flags() >> 15) & 0x07; }
    u32 tevAlphaArgSel() const { return (flags() >> 18) & 0x01; }
    s16 prmAnmDataOffs() const { return bes16(d + 0x04); }
    s16 envAnmDataOffs() const { return bes16(d + 0x06); }
    f32 baseSizeX() const { return bef32(d + 0x08); }
    f32 baseSizeY() const { return bef32(d + 0x0C); }
    u16 blendMode() const { return be16(d + 0x12); }
    // WW colour flags: 0x01 prm on, 0x02 prm anm, 0x04 env on, 0x08 env anm,
    // (>>4)&7 anm type. NOTE these bits differ from TP's — never share a word.
    u8 colorFlags() const { return d[0x1B]; }
    bool prmEnabled() const { return (colorFlags() & 0x01) != 0; }
    bool envEnabled() const { return (colorFlags() & 0x04) != 0; }
    u8 prmAnmKeyNum() const { return d[0x1C]; }
    u8 envAnmKeyNum() const { return d[0x1D]; }
    s16 colorAnmMaxFrm() const { return bes16(d + 0x1E); }
    void prmColor(u8* rgba) const { for (int i = 0; i < 4; i++) rgba[i] = d[0x20 + i]; }
    void envColor(u8* rgba) const { for (int i = 0; i < 4; i++) rgba[i] = d[0x24 + i]; }
    // ========================================================================
    // §241 — TEXTURE-COORDINATE MATRIX FIELDS (WW JPABaseShape.h:189-201).
    //
    // These have NO counterpart in TP's JPABaseShapeData: TP stores the same
    // ten scalars in a trailing table read at `block + sizeof(data)` and gets
    // its tiling from FLAG BITS 25/26 instead of these two floats. WW keeps
    // them as named struct fields. Carrying them is mandatory whenever
    // texScrollAnm (flag bit 24) is set — see bindBsp.
    // ========================================================================
    bool texScrollAnmEnabled() const { return (flags() & 0x01000000) != 0; }
    f32 tilingX() const { return bef32(d + 0x28); }
    f32 tilingY() const { return bef32(d + 0x2C); }
    f32 texStaticTransX() const { return bef32(d + 0x30); }
    f32 texStaticTransY() const { return bef32(d + 0x34); }
    f32 texStaticScaleX() const { return bef32(d + 0x38); }
    f32 texStaticScaleY() const { return bef32(d + 0x3C); }
    f32 texScrollTransX() const { return bef32(d + 0x40); }
    f32 texScrollTransY() const { return bef32(d + 0x44); }
    f32 texScrollScaleX() const { return bef32(d + 0x48); }
    f32 texScrollScaleY() const { return bef32(d + 0x4C); }
    f32 texScrollRotate() const { return bef32(d + 0x50); }
};

struct ExtraShapeBlock {  // ESP1 — JPAExtraShapeData (WW JPAExtraShape.h:6-32)
    const u8* d;
    u32 flags() const { return be32(d + 0x00); }
    // WW bits (JPAExtraShape.h:83-117) — deliberately NOT TP's.
    bool scaleEnabled() const { return (flags() & 0x100) != 0; }
    bool scaleXYDiff() const { return (flags() & 0x200) != 0; }
    bool alphaEnabled() const { return (flags() & 0x01) != 0; }
    bool sinWaveEnabled() const { return (flags() & 0x02) != 0; }
    bool rotateEnabled() const { return (flags() & 0x01000000) != 0; }
    f32 alphaInTiming() const { return bef32(d + 0x08); }
    f32 alphaOutTiming() const { return bef32(d + 0x0C); }
    f32 alphaInValue() const { return bef32(d + 0x10); }
    f32 alphaBaseValue() const { return bef32(d + 0x14); }
    f32 alphaOutValue() const { return bef32(d + 0x18); }
    f32 scaleInTiming() const { return bef32(d + 0x2C); }
    f32 scaleOutTiming() const { return bef32(d + 0x30); }
    f32 scaleInValueX() const { return bef32(d + 0x34); }
    f32 scaleOutValueX() const { return bef32(d + 0x38); }
    f32 scaleInValueY() const { return bef32(d + 0x3C); }
    f32 scaleOutValueY() const { return bef32(d + 0x40); }
    f32 randomScale() const { return bef32(d + 0x44); }
    s16 anmCycleX() const { return bes16(d + 0x48); }
    s16 anmCycleY() const { return bes16(d + 0x4A); }
    f32 rotateAngle() const { return bef32(d + 0x4C); }
    f32 rotateSpeed() const { return bef32(d + 0x50); }
};

struct FieldBlock {  // FLD1 — JPAFieldBlockData (WW JPAFieldBlock.h:6-21)
    const u8* d;
    u32 flags() const { return be32(d + 0x00); }
    u8 type() const { return (u8)(flags() & 0x0F); }
    u32 velType() const { return (flags() >> 8) & 0x03; }
    u32 sttFlag() const { return (flags() >> 16) & 0xFFFF; }
    f32 mag() const { return bef32(d + 0x04); }
    f32 magRndm() const { return bef32(d + 0x08); }
    f32 maxDist() const { return bef32(d + 0x0C); }
    f32 pos(int i) const { return bef32(d + 0x10 + 4 * i); }
    f32 dir(int i) const { return bef32(d + 0x1C + 4 * i); }
    f32 val1() const { return bef32(d + 0x28); }
    f32 val2() const { return bef32(d + 0x2C); }
    f32 val3() const { return bef32(d + 0x30); }
    f32 fadeIn() const { return bef32(d + 0x34); }
    f32 fadeOut() const { return bef32(d + 0x38); }
    f32 enTime() const { return bef32(d + 0x3C); }
    f32 disTime() const { return bef32(d + 0x40); }
    u8 cycle() const { return d[0x44]; }
};

// ============================================================================
// One emitter resource — pointers INTO the archive (nothing copied).
// Mirrors JPADataBlockLinkInfo (JPAEmitterLoader.cpp:108-167).
// ============================================================================
struct Resource {
    u16 resId = 0;
    u8 keyNum = 0;
    u8 fldNum = 0;
    u8 textureNum = 0;
    const u8* bem1 = nullptr;   // block starts (payload = +0x0C)
    const u8* bsp1 = nullptr;
    const u8* esp1 = nullptr;
    const u8* ssp1 = nullptr;
    const u8* etx1 = nullptr;
    const u8* fld1[8] = {};
    const u8* kfa1[8] = {};
    const u8* texIdxTable = nullptr;  // TDB1 payload: u16 per texture

    bool valid() const { return bem1 != nullptr && bsp1 != nullptr; }
    DynamicsBlock dyn() const { return DynamicsBlock{bem1 + 0x0C}; }
    BaseShapeBlock bsp() const { return BaseShapeBlock{bsp1 + 0x0C}; }
    ExtraShapeBlock esp() const { return ExtraShapeBlock{esp1 + 0x0C}; }
    FieldBlock fld(int i) const { return FieldBlock{fld1[i] + 0x0C}; }
    u16 texIndex(int i) const { return texIdxTable ? be16(texIdxTable + 2 * i) : 0; }
};

// A texture entry: TEX1 block. Name at +0x0C (20 bytes), BTI image at +0x20 —
// same layout TP's JPATextureData describes, so the bytes are directly usable.
struct Texture {
    const u8* block = nullptr;
    const char* name() const { return (const char*)(block + 0x0C); }
    const u8* bti() const { return block + 0x20; }
};

// ============================================================================
// Archive — parses a JPAC1-00 image in place. No allocation of donor bytes.
// ============================================================================
class Archive {
public:
    bool parse(const u8* data, u32 size);
    bool isValid() const { return mData != nullptr; }
    u16 resourceCount() const { return mResCount; }
    u16 textureCount() const { return mTexCount; }
    // Find a resource by its donor id (JPAEmitterParticleHeader_v10::resID).
    const Resource* find(u16 resId) const;
    const Texture* texture(u16 idx) const;

private:
    const u8* mData = nullptr;
    u32 mSize = 0;
    u16 mResCount = 0;
    u16 mTexCount = 0;
    static const int kMaxRes = 256;
    static const int kMaxTex = 128;
    Resource mRes[kMaxRes];
    Texture mTex[kMaxTex];
    int mResN = 0;
    int mTexN = 0;
};

// ============================================================================
// Phase 2 (Path B) — bind a parsed WW resource into a receiver JPAResourceManager.
// Struct-level population through the accessors above: no archive is fabricated,
// no offsets are computed, and WW semantics win wherever the lineages differ.
// Implemented in ww_jpa_bind.cpp.
// ============================================================================
bool bindResource(const Archive& arc, u16 resId, class JPAResourceManager* mgr, class JKRHeap* heap);

}  // namespace ww_jpa

#endif /* WW_JPA_H */
