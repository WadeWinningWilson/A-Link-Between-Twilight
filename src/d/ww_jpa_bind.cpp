// ============================================================================
// WW JPA LANE â€” phase 2 (Path B): bind WW resources to the receiver runtime.
//
// User ruling 2026-08-02 ("Go B"): rather than duplicate a JSystem runtime the
// codebase already has, feed the receiver's JPA with WW data â€” but do it at the
// STRUCT level, through named WW accessors, with WW semantics winning wherever
// the two JPA lineages disagree.
//
// WHY THIS IS NOT THE OLD CONVERTER. The converter worked at the BYTE level: it
// fabricated a whole JPAC2 archive (header, block walk, texture section) with
// offset arithmetic, and every silent mismatch cost a playtest â€” dropped blocks
// (Â§202), a flag word whose bits mean different things per lineage (Â§212), a
// resolved bank id discarded a line later (Â§223). Here there is no archive, no
// offsets and no block walk: each line is `dst->field = ww.accessor()`, so a
// wrong field cannot hide, and a missing block is a null pointer rather than a
// misparse. Every mapping cites the donor header it came from.
//
// Semantics rules, from the verification work:
//   * BEM1 flags   â€” bit layouts VERIFIED IDENTICAL (volumeType >>8&7, and the
//                    JPADynFlag_* enum values match header-to-header). Passed
//                    through.
//   * BSP1 flags   â€” VERIFIED IDENTICAL (type 0-3, dir 4-6, rot 7-9, basePlane
//                    10, tevColorArg 15-17, tevAlphaArg 18) â€” Â§207/Â§212.
//   * BSP1 clrFlg  â€” anm bits (0x02/0x08) and anm type (>>4&7) match; WW's
//                    extra "enabled" bits (0x01/0x04) are inert on TP.
//   * ESP1 flags   â€” DIFFER (WW: scale 0x100, alpha 0x01 â€¦ TP: scale 0x01,
//                    alpha 0x10000). TRANSLATED explicitly below; this is the
//                    one place a bit is rewritten, and it is why Â§212 happened.
//   * FLD1 flags   â€” VERIFIED IDENTICAL (type 0-3, velType >>8&3, stt >>16).
//   * TEX1         â€” WW's layout already matches TP's JPATextureData (name at
//                    +0x0C, ResTIMG at +0x20), so textures are registered from
//                    the donor bytes with NO conversion whatsoever.
// ============================================================================

#include "d/dolzel_rel.h"  // IWYU pragma: keep

#if TARGET_PC

#include "d/ww_jpa.h"

#include <cstring>

#include "JSystem/JKernel/JKRHeap.h"
#include "JSystem/JParticle/JPABaseShape.h"
#include "JSystem/JParticle/JPADynamicsBlock.h"
#include "JSystem/JParticle/JPAExtraShape.h"
#include "JSystem/JParticle/JPAFieldBlock.h"
#include "JSystem/JParticle/JPAResource.h"
#include "JSystem/JParticle/JPAResourceManager.h"
#include "JSystem/JParticle/JPATexture.h"
#include "dusk/logging.h"

namespace ww_jpa {

namespace {

// Allocate a zeroed block buffer with its JSystem block header filled in.
u8* newBlock(JKRHeap* heap, const char* magic, u32 size) {
    u8* p = (u8*)heap->alloc(size, 0x20);
    if (p == NULL) {
        return NULL;
    }
    std::memset(p, 0, size);
    std::memcpy(p, magic, 4);
    // mSize is BE(u32) at +4 in every JSystem data block.
    p[4] = (u8)(size >> 24);
    p[5] = (u8)(size >> 16);
    p[6] = (u8)(size >> 8);
    p[7] = (u8)size;
    return p;
}

}  // namespace

// ---------------------------------------------------------------------------
// BEM1 â€” dynamics. WW JPADynamicsBlock.h:6 â†’ TP JPADynamicsBlockData.
// ---------------------------------------------------------------------------
static JPADynamicsBlock* bindDyn(const Resource& r, JKRHeap* heap) {
    u8* blk = newBlock(heap, "BEM1", sizeof(JPADynamicsBlockData));
    if (blk == NULL) {
        return NULL;
    }
    JPADynamicsBlockData* d = (JPADynamicsBlockData*)blk;
    const DynamicsBlock w = r.dyn();

    d->mFlags = w.flags();  // layouts verified identical
    d->mResUserWork = 0;
    d->mEmitterScl.x = w.emitterScl(0);
    d->mEmitterScl.y = w.emitterScl(1);
    d->mEmitterScl.z = w.emitterScl(2);
    d->mEmitterTrs.x = w.emitterTrs(0);
    d->mEmitterTrs.y = w.emitterTrs(1);
    d->mEmitterTrs.z = w.emitterTrs(2);
    d->mEmitterDir.x = w.emitterDir(0);
    d->mEmitterDir.y = w.emitterDir(1);
    d->mEmitterDir.z = w.emitterDir(2);
    d->mInitialVelOmni = w.initVelOmni();
    d->mInitialVelAxis = w.initVelAxis();
    d->mInitialVelRndm = w.initVelRndm();
    d->mInitialVelDir = w.initVelDir();
    d->mSpread = w.spread();
    d->mInitialVelRatio = w.initVelRatio();
    d->mRate = w.rate();
    d->mRateRndm = w.rateRndm();
    d->mLifeTimeRndm = w.lifeTimeRndm();
    d->mVolumeSweep = w.volumeSweep();
    d->mVolumeMinRad = w.volumeMinRad();
    d->mAirResist = w.airResist();
    d->mMoment = w.moment();
    d->mEmitterRot.x = w.emitterRot(0);
    d->mEmitterRot.y = w.emitterRot(1);
    d->mEmitterRot.z = w.emitterRot(2);
    d->mMaxFrame = w.maxFrame();
    d->mStartFrame = w.startFrame();
    d->mLifeTime = w.lifeTime();
    d->mVolumeSize = w.volumeSize();
    d->mDivNumber = w.divNumber();
    d->mRateStep = w.rateStep();
    return JKR_NEW_ARGS(heap, 0) JPADynamicsBlock(blk);
}

// ---------------------------------------------------------------------------
// BSP1 â€” base shape. WW JPABaseShape.h:100 â†’ TP JPABaseShapeData.
// ---------------------------------------------------------------------------
static JPABaseShape* bindBsp(const Resource& r, JKRHeap* heap) {
    u8* blk = newBlock(heap, "BSP1", sizeof(JPABaseShapeData));
    if (blk == NULL) {
        return NULL;
    }
    JPABaseShapeData* d = (JPABaseShapeData*)blk;
    const BaseShapeBlock w = r.bsp();
    const u8* raw = w.d;

    d->mFlags = w.flags();  // type/dir/rot/basePlane/tev args â€” verified identical
    d->mBaseSizeX = w.baseSizeX();
    d->mBaseSizeY = w.baseSizeY();
    d->mBlendModeCfg = w.blendMode();
    // WW packs alpha-compare / z-mode / tex config as bytes at +0x14..0x19,
    // in the same order TP reads them at +0x1A..0x1F.
    d->mAlphaCompareCfg = raw[0x14];
    d->mAlphaRef0 = raw[0x15];
    d->mAlphaRef1 = raw[0x16];
    d->mZModeCfg = raw[0x17];
    d->mTexIdx = 0;  // TDB1 supplies the real index
    // ========================================================================
    // §234 — ANIMATION TABLES ARE NOT CARRIED IN PHASE 2 (and must be disabled,
    // not merely ignored).
    //
    // JPABaseShape's constructor WALKS the colour/texture animation tables when
    // the key counts are non-zero, reading them at mClrPrmAnmOffset /
    // mClrEnvAnmOffset — offsets that are relative to the DONOR's block layout.
    // This block is a freshly allocated JPABaseShapeData with no anim payload
    // after it, so honouring those counts sends the constructor reading past
    // the buffer into the heap: memory corruption, and a hard crash with no
    // handler output (exactly what the grass cut produced — windline survived
    // only because it has no anim keys).
    //
    // So the counts, the offsets and the anm ENABLE bits are all cleared here.
    // Consequence, stated honestly: WW colour/texture animation over a
    // particle's life is not reproduced yet — the base colours are. Carrying
    // the tables is phase 3 work (they need to be copied and re-based, which is
    // real porting, not a flag flip).
    // ========================================================================
    d->mClrFlg = (u8)(w.colorFlags() & ~0x0A);  // drop prm-anm (0x02) + env-anm (0x08)
    d->prmAnmKeyNum = 0;
    d->envAnmKeyNum = 0;
    d->mClrPrmAnmOffset = 0;
    d->mClrEnvAnmOffset = 0;
    d->texAnmKeyNum = 0;
    d->mTexFlg = (u8)(raw[0x18] & ~0x01);  // drop isTexAnm
    d->mClrAnmFrmMax = 0;
    u8 prm[4];
    u8 env[4];
    w.prmColor(prm);
    w.envColor(env);
    d->mClrPrm.r = prm[0];
    d->mClrPrm.g = prm[1];
    d->mClrPrm.b = prm[2];
    d->mClrPrm.a = prm[3];
    d->mClrEnv.r = env[0];
    d->mClrEnv.g = env[1];
    d->mClrEnv.b = env[2];
    d->mClrEnv.a = env[3];
    return JKR_NEW_ARGS(heap, 0) JPABaseShape(blk, heap);
}

// ---------------------------------------------------------------------------
// ESP1 â€” extra shape. THE ONE BLOCK WHOSE FLAGS MUST BE REWRITTEN (Â§212).
// ---------------------------------------------------------------------------
static JPAExtraShape* bindEsp(const Resource& r, JKRHeap* heap) {
    u8* blk = newBlock(heap, "ESP1", sizeof(JPAExtraShapeData));
    if (blk == NULL) {
        return NULL;
    }
    JPAExtraShapeData* d = (JPAExtraShapeData*)blk;
    const ExtraShapeBlock w = r.esp();

    u32 tf = 0;
    if (w.scaleEnabled()) tf |= 0x00000001;   // TP isEnableScaleAnm
    if (w.scaleXYDiff()) tf |= 0x00000002;    // TP isScaleXYDiff
    if (w.alphaEnabled()) tf |= 0x00010000;   // TP isEnableAlphaAnm
    if (w.sinWaveEnabled()) tf |= 0x00020000; // TP isEnableAlphaFlick
    if (w.rotateEnabled()) tf |= 0x01000000;  // same bit in both
    // scale anm type / centre: WW >>18,>>19 (1 bit) and >>14,>>16 (2 bit)
    // land at TP >>8,>>10 and >>12,>>14.
    const u32 wf = w.flags();
    tf |= ((wf >> 18) & 0x01) << 8;
    tf |= ((wf >> 19) & 0x01) << 10;
    tf |= ((wf >> 14) & 0x03) << 12;
    tf |= ((wf >> 16) & 0x03) << 14;
    d->mFlags = tf;

    d->mScaleInTiming = w.scaleInTiming();
    d->mScaleOutTiming = w.scaleOutTiming();
    d->mScaleInValueX = w.scaleInValueX();
    d->mScaleOutValueX = w.scaleOutValueX();
    d->mScaleInValueY = w.scaleInValueY();
    d->mScaleOutValueY = w.scaleOutValueY();
    d->mScaleOutRandom = w.randomScale();
    d->mScaleAnmCycleX = w.anmCycleX();
    d->mScaleAnmCycleY = w.anmCycleY();
    d->mAlphaInTiming = w.alphaInTiming();
    d->mAlphaOutTiming = w.alphaOutTiming();
    d->mAlphaInValue = w.alphaInValue();
    d->mAlphaBaseValue = w.alphaBaseValue();
    d->mAlphaOutValue = w.alphaOutValue();
    d->mRotateAngle = w.rotateAngle();
    d->mRotateSpeed = w.rotateSpeed();
    return JKR_NEW_ARGS(heap, 0) JPAExtraShape(blk);
}

// ---------------------------------------------------------------------------
// FLD1 â€” field. WW JPAFieldBlock.h:6 â†’ TP JPAFieldBlockData.
// ---------------------------------------------------------------------------
static JPAFieldBlock* bindFld(const FieldBlock& w, JKRHeap* heap) {
    u8* blk = newBlock(heap, "FLD1", sizeof(JPAFieldBlockData));
    if (blk == NULL) {
        return NULL;
    }
    JPAFieldBlockData* d = (JPAFieldBlockData*)blk;
    d->mFlags = w.flags();  // type/velType/sttFlag â€” verified identical
    d->mPos.x = w.pos(0);
    d->mPos.y = w.pos(1);
    d->mPos.z = w.pos(2);
    d->mDir.x = w.dir(0);
    d->mDir.y = w.dir(1);
    d->mDir.z = w.dir(2);
    d->mMag = w.mag();
    d->mMagRndm = w.magRndm();
    d->mVal1 = w.val1();
    d->mFadeInTime = w.fadeIn();
    d->mFadeOutTime = w.fadeOut();
    d->mEnTime = w.enTime();
    d->mDisTime = w.disTime();
    d->mCycle = w.cycle();
    return JKR_NEW_ARGS(heap, 0) JPAFieldBlock(blk, heap);
}

// ---------------------------------------------------------------------------
// Public: bind one WW resource (and the archive's textures) into a receiver
// resource manager. Textures are registered from the DONOR BYTES directly.
// ---------------------------------------------------------------------------
bool bindResource(const Archive& arc, u16 resId, JPAResourceManager* mgr, JKRHeap* heap) {
    if (mgr == NULL || heap == NULL) {
        return false;
    }
    const Resource* r = arc.find(resId);
    if (r == NULL || !r->valid()) {
        DuskLog.error("[wwJPA] bind {:#06x}: resource missing or incomplete", (unsigned)resId);
        return false;
    }

    JPAResource* res = JKR_NEW_ARGS(heap, 0) JPAResource();
    if (res == NULL) {
        return false;
    }
    res->mUsrIdx = resId;
    res->pDyn = bindDyn(*r, heap);
    res->pBsp = bindBsp(*r, heap);
    res->pEsp = (r->esp1 != NULL) ? bindEsp(*r, heap) : NULL;
    res->pCsp = NULL;
    res->pEts = NULL;
    res->keyNum = 0;
    res->ppKey = NULL;
    res->fldNum = r->fldNum;
    res->ppFld = NULL;
    if (res->fldNum != 0) {
        res->ppFld = JKR_NEW_ARRAY_ARGS(JPAFieldBlock*, res->fldNum, heap, 0);
        for (u8 i = 0; i < res->fldNum; i++) {
            res->ppFld[i] = bindFld(r->fld(i), heap);
        }
    }
    res->texNum = r->textureNum;
    res->mpTDB1 = (BE(u16) const*)r->texIdxTable;  // donor bytes, BE u16s

    if (res->pDyn == NULL || res->pBsp == NULL) {
        DuskLog.error("[wwJPA] bind {:#06x}: block alloc failed", (unsigned)resId);
        return false;
    }

    // Textures: register the archive's whole table in order, so the donor's own
    // TDB1 indices remain valid without remapping.
    if (mgr->texRegNum == 0) {
        for (u16 i = 0; i < arc.textureCount(); i++) {
            const Texture* t = arc.texture(i);
            if (t == NULL) {
                break;
            }
            JPATexture* tex = JKR_NEW_ARGS(heap, 0) JPATexture(t->block);
            if (tex != NULL) {
                mgr->registTex(tex);
            }
        }
    }

    res->init(heap);
    // ========================================================================
    // §238 — WW resources take the CLASSIC draw path, not the batch path.
    //
    // §236 proved the resource is complete (drawP=4, calcP=4) and that
    // particles ARE created (ptclNum/emitterNum both rising), and the capped
    // 30 fps test ruled out time-scaling — yet nothing is visible. That leaves
    // HOW the particles are drawn. `batch=1` means this resource is eligible
    // for the port's batched draw, which builds geometry from a cached
    // BatchInfo template rather than running the per-particle draw functions;
    // it is a receiver-side optimisation the donor never had, and it is the one
    // stage in the chain we have not exercised for WW data.
    //
    // Opting WW resources out puts them on the same per-particle path TP's
    // non-batchable particles use every frame. Set DUSK_WW_BATCH=1 to re-enable
    // and compare.
    // ========================================================================
    {
        static const bool s_batchOk = [] {
            const char* v = std::getenv("DUSK_WW_BATCH");
            return v != NULL && v[0] == '1';
        }();
        if (!s_batchOk) {
            res->mBatchInfo.supported = false;
        }
    }
    // ========================================================================
    // §236 — post-init readout. init() builds the calc/draw function lists from
    // BSP1's flags; if our flag combination produces an EMPTY draw list, the
    // resource binds cleanly and then renders nothing, silently. These four
    // numbers separate "bound but has no draw function" from "draws but never
    // emits", which are the only two possibilities left after §235.
    // ========================================================================
    DuskLog.warn("[wwJPA] {:#06x} init: drawP={} calcP={} drawE={} calcE={} batch={} "
                 "bspType={} tevSel={} blend={:#06x}",
                 (unsigned)resId, (int)res->mpDrawParticleFuncListNum,
                 (int)res->mpCalcParticleFuncListNum, (int)res->mpDrawEmitterFuncListNum,
                 (int)res->mpCalcEmitterFuncListNum, res->mBatchInfo.supported ? 1 : 0,
                 (int)res->pBsp->getType(), (int)res->pBsp->getTevColorArgSel(),
                 (unsigned)r->bsp().blendMode());
    mgr->registRes(res);
    DuskLog.info("[wwJPA] bound {:#06x} â€” dyn/bsp{}{} fld={} tex={} (native WW accessors)",
                 (unsigned)resId, res->pEsp != NULL ? "/esp" : "", "", (int)res->fldNum,
                 (int)res->texNum);
    return true;
}

}  // namespace ww_jpa

#endif  // TARGET_PC


