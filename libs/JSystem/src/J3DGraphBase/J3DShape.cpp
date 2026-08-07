#include "JSystem/JSystem.h" // IWYU pragma: keep

#include "JSystem/J3DGraphBase/J3DShape.h"

#include <dolphin/gd.h>
#include <gd.h>
#include <gx.h>
#include "JSystem/J3DGraphBase/J3DFifo.h"
#include "JSystem/J3DGraphBase/J3DPacket.h"
#include "JSystem/J3DGraphBase/J3DVertex.h"
#include "JSystem/JKernel/JKRHeap.h"
#include "tracy/Tracy.hpp"

void J3DGDSetVtxAttrFmtv(GXVtxFmt, GXVtxAttrFmtList const*, bool);
void J3DFifoLoadPosMtxImm(Mtx, u32);
void J3DFifoLoadNrmMtxImm(Mtx, u32);

void J3DShape::initialize() {
    mMaterial = NULL;
    mIndex = -1;
    mMtxGroupNum = 0;
    mFlags = 0;
    mRadius = 0.0f;
    mMin.x = 0.0f;
    mMin.y = 0.0f;
    mMin.z = 0.0f;
    mMax.x = 0.0f;
    mMax.y = 0.0f;
    mMax.z = 0.0f;
    mVtxDesc = NULL;
    mShapeMtx = NULL;
    mShapeDraw = NULL;
    mVertexData = NULL;
    mDrawMtxData = NULL;
    mScaleFlagArray = NULL;
    mDrawMtx = NULL;
    mNrmMtx = NULL;
    mCurrentViewNo = &j3dDefaultViewNo;
    mHasNBT = false;
    mHasPNMTXIdx = false;
}

void J3DShape::addTexMtxIndexInDL(GXAttr attr, u32 valueBase) {
    u32 kSize[] = {0, 1, 1, 2};

    s32 pnmtxidxOffs = -1;
    s32 attrOffs = -1;
    s32 stride = 0;
    bool found = false;

    for (GXVtxDescList* vtxDesc = getVtxDesc(); vtxDesc->attr != GX_VA_NULL; vtxDesc++) {
        if (vtxDesc->attr == GX_VA_PNMTXIDX)
            pnmtxidxOffs = stride;

        if (attr < vtxDesc->attr && !found) {
            attrOffs = stride;
            found = true;
        }

        stride = stride + kSize[vtxDesc->type];
    }

    if (pnmtxidxOffs == -1)
        return;

    for (u16 i = 0; i < (u16)getMtxGroupNum(); i++)
        getShapeDraw(i)->addTexMtxIndexInDL(stride, attrOffs, (s32)valueBase);
}

void J3DShape::addTexMtxIndexInVcd(GXAttr attr) {
    u32 kSize[] = {0, 1, 1, 2};  // stripped data

    s32 attrIdx = -1;
    s32 attrOffs = -1;
    s32 stride = 0;

    GXVtxDescList* vtxDesc = getVtxDesc();
    s32 attrCount = 0;

    for (; vtxDesc->attr != GX_VA_NULL; vtxDesc++) {
        if (vtxDesc->attr == GX_VA_PNMTXIDX) {
            attrIdx = stride;
        }
        attrCount++;
    }

    if (attrIdx == -1)
        return;

    GXVtxDescList* newVtxDesc = JKR_NEW_ARRAY(GXVtxDescList, attrCount + 2);
    bool inserted = false;

    vtxDesc = getVtxDesc();
    GXVtxDescList* dst = newVtxDesc;
    for (; vtxDesc->attr != GX_VA_NULL; vtxDesc++) {
        if ((attr < vtxDesc->attr) && !inserted) {
            dst->attr = attr;
            dst->type = GX_DIRECT;
            attrOffs = stride;
            dst++;

            inserted = true;
        }

        dst->attr = vtxDesc->attr;
        dst->type = vtxDesc->type;
        stride = stride + kSize[vtxDesc->type];
        dst++;
    }

    dst->attr = GX_VA_NULL;
    dst->type = GX_NONE;
    mVtxDesc = newVtxDesc;
    makeVcdVatCmd();
}

void J3DShape::calcNBTScale(const Vec& param_0, f32 (*param_1)[3][3], f32 (*param_2)[3][3]) {
    for (u16 i = 0; i < mMtxGroupNum; i++)
        mShapeMtx[i]->calcNBTScale(param_0, param_1, param_2);
}

u16 J3DShape::countBumpMtxNum() const {
    u16 num = 0;
    for (u16 i = 0; i < mMtxGroupNum; i++)
        num += mShapeMtx[i]->getUseMtxNum();

    return num;
}

void J3DLoadCPCmd(u8 addr, u32 val) {
    GXCmd1u8(GX_LOAD_CP_REG);
    GXCmd1u8(addr);
    GXCmd1u32(val);
}

#if TARGET_PC
static void J3DLoadArrayBasePtr(GXAttr attr, void* data, u32 size, bool le) {
    u32 idx = (attr == GX_VA_NBT) ? 1 : (attr - GX_VA_POS);
    GXCmd1u8(GX_AURORA);
    GXCmd1u16(GX_AURORA_LOAD_ARRAYBASE | idx);
    GXCmd1u64((u64)data);
    GXCmd1u32(size);
    GXCmd1u8(le ? 1 : 0);
}
#else
static void J3DLoadArrayBasePtr(GXAttr attr, void* data) {
    u32 idx = (attr == GX_VA_NBT) ? 1 : (attr - GX_VA_POS);
    J3DLoadCPCmd(0xA0 + idx, ((uintptr_t)data & 0x7FFFFFFF));
}
#endif

void J3DShape::loadVtxArray() const {
#if TARGET_PC
    // TODO: these can very easily overcount if the data isn't in F32 format
    if (j3dSys.getVtxPos() != mVertexData->getVtxPosArray()) {
        J3DLoadArrayBasePtr(GX_VA_POS, j3dSys.getVtxPos(), j3dSys.mVtxPosNum * sizeof(Vec), true);
    }

    if (!mHasNBT && j3dSys.getVtxNrm() != mVertexData->getVtxNrmArray()) {
        J3DLoadArrayBasePtr(GX_VA_NRM, j3dSys.getVtxNrm(), j3dSys.mVtxNrmNum * sizeof(Vec), true);
    }

    if (j3dSys.getVtxCol() != mVertexData->getVtxColorArray(0)) {
        J3DLoadArrayBasePtr(GX_VA_CLR0, j3dSys.getVtxCol(), j3dSys.mVtxColNum * sizeof(GXColor), true);
    }
#else
    J3DLoadArrayBasePtr(GX_VA_POS, j3dSys.getVtxPos());

    if (!mHasNBT) {
        J3DLoadArrayBasePtr(GX_VA_NRM, j3dSys.getVtxNrm());
    }

    J3DLoadArrayBasePtr(GX_VA_CLR0, j3dSys.getVtxCol());
#endif
}

bool J3DShape::isSameVcdVatCmd(J3DShape* other) {
    u8* a = (u8*)other->getVcdVatCmd();
    u8* b = mVcdVatCmd;
    for (u32 i = 0; i < kVcdVatDLSize; i++)
        if (a[i] != b[i])
            return false;

    return true;
}

void J3DShape::makeVtxArrayCmd() {
    GXVtxAttrFmtList* vtxAttr = mVertexData->getVtxAttrFmtList();

    u8 stride[12];
    void* array[12];
    for (u32 i = 0; i < 12; i++) {
        stride[i] = 0;
        array[i] = 0;
    }

    for (; vtxAttr->attr != GX_VA_NULL; vtxAttr++) {
        switch (vtxAttr->attr) {
        case GX_VA_POS: {
            if (vtxAttr->type == GX_F32)
                stride[vtxAttr->attr - GX_VA_POS] = 12;
            else
                stride[vtxAttr->attr - GX_VA_POS] = 6;

            array[vtxAttr->attr - GX_VA_POS] = mVertexData->getVtxPosArray();
            mVertexData->setVtxPosFrac((u8)vtxAttr->frac);
            mVertexData->setVtxPosType((GXCompType)vtxAttr->type);
        } break;
        case GX_VA_NRM: {
            if (vtxAttr->type == GX_F32)
                stride[vtxAttr->attr - GX_VA_POS] = 12;
            else
                stride[vtxAttr->attr - GX_VA_POS] = 6;

            array[vtxAttr->attr - GX_VA_POS] = mVertexData->getVtxNrmArray();
            mVertexData->setVtxNrmFrac((u8)vtxAttr->frac);
            mVertexData->setVtxNrmType((GXCompType)vtxAttr->type);
        } break;
        case GX_VA_CLR0:
        case GX_VA_CLR1: {
            stride[vtxAttr->attr - GX_VA_POS] = 4;
            array[vtxAttr->attr - GX_VA_POS] = mVertexData->getVtxColorArray(vtxAttr->attr - GX_VA_CLR0);
        } break;
        case GX_VA_TEX0:
        case GX_VA_TEX1:
        case GX_VA_TEX2:
        case GX_VA_TEX3:
        case GX_VA_TEX4:
        case GX_VA_TEX5:
        case GX_VA_TEX6:
        case GX_VA_TEX7: {
            if (vtxAttr->type == GX_F32)
                stride[vtxAttr->attr - GX_VA_POS] = 8;
            else
                stride[vtxAttr->attr - GX_VA_POS] = 4;

            array[vtxAttr->attr - GX_VA_POS] = mVertexData->getVtxTexCoordArray(vtxAttr->attr - GX_VA_TEX0);
        } break;
        default:
            break;
        }
    }

    GXVtxDescList* vtxDesc = mVtxDesc;
    mHasPNMTXIdx = false;
    for (; vtxDesc->attr != GX_VA_NULL; vtxDesc++) {
        if (vtxDesc->attr == GX_VA_NBT && vtxDesc->type != GX_NONE) {
            mHasNBT = true;
            stride[GX_VA_NRM - GX_VA_POS] *= 3;
            array[GX_VA_NRM - GX_VA_POS] = mVertexData->getVtxNBTArray();
            // TODO: How set array size here?
        } else if (vtxDesc->attr == GX_VA_PNMTXIDX && vtxDesc->type != GX_NONE) {
            mHasPNMTXIdx = true;
        }
    }

#if TARGET_PC
    for (u32 i = 0; i < 12; i++) {
        GXAttr attr = GXAttr(i + GX_VA_POS);
        if (array[i] != nullptr)
            GDSetArraySized(attr, array[i], mVertexData->getVtxArrByteSize(attr), mVertexData->getVtxArrStride(attr), true);
        else
            GDSetArraySized(attr, nullptr, 0, mVertexData->getVtxArrStride(attr), true);
    }
#else
    for (u32 i = 0; i < 12; i++) {
        if (array[i] != 0)
            GDSetArray((GXAttr)(i + GX_VA_POS), array[i], stride[i]);
        else
            GDSetArrayRaw((GXAttr)(i + GX_VA_POS), 0, stride[i]);
    }
#endif
}

void J3DShape::makeVcdVatCmd() {
    static BOOL sInterruptFlag = OSDisableInterrupts();
    OSDisableScheduler();

    GDLObj gdl_obj;
    GDInitGDLObj(&gdl_obj, mVcdVatCmd, kVcdVatDLSize);
    GDSetCurrent(&gdl_obj);
    GDSetVtxDescv(mVtxDesc);
    makeVtxArrayCmd();
    J3DGDSetVtxAttrFmtv(GX_VTXFMT0, mVertexData->getVtxAttrFmtList(), mHasNBT);
    GDPadCurr32();
    GDFlushCurrToMem();
    GDSetCurrent(NULL);
    OSEnableScheduler();
    OSRestoreInterrupts(sInterruptFlag);
}

void* J3DShape::sOldVcdVatCmd;

void J3DShape::loadCurrentMtx() const {
    mCurrentMtx.load();
}

void J3DShape::loadPreDrawSetting() const {
#if TARGET_PC
    // ========================================================================
    // [Housing] §471 P2 OWNER BREADCRUMB — DEBUG RESIDUE, ON THE HT-5 STRIP
    // LIST. Foundry's set_dl_owner records who submitted a display list, so an
    // anomaly can name its author instead of a bare pointer.
    // Placed OUTSIDE the sOldVcdVatCmd check ON PURPOSE: the failing shape is
    // exactly the one that SKIPS the VCD call (J3D's cache hit), so tagging
    // inside the branch would miss the only case we care about. The owner is
    // sticky, so it also covers the shape's own geometry list submitted later
    // by J3DShapeDraw::draw().
    // Diagnostic only — records an identity, changes no behaviour.
    // ========================================================================
    GXAuroraSetDlOwner(this, "J3DShape");
#endif
    if (sOldVcdVatCmd != mVcdVatCmd) {
        GXCallDisplayList(mVcdVatCmd, kVcdVatDLSize);
        sOldVcdVatCmd = mVcdVatCmd;
    }

    mCurrentMtx.load();
}

bool J3DShape::sEnvelopeFlag;

void J3DShape::setArrayAndBindPipeline() const {
    J3DShapeMtx::setCurrentPipeline((mFlags & 0x1C) >> 2);
    loadVtxArray();
    j3dSys.setModelDrawMtx(mDrawMtx[*mCurrentViewNo]);
    j3dSys.setModelNrmMtx(mNrmMtx[*mCurrentViewNo]);
    J3DShapeMtx::sCurrentScaleFlag = mScaleFlagArray;
    J3DShapeMtx::sNBTFlag = mHasNBT;
    sEnvelopeFlag = mHasPNMTXIdx;
    J3DShapeMtx::sTexMtxLoadType = getTexMtxLoadType();
}

#if TARGET_PC
// §387 GX-STATE TAP B — the FAILING submission path (J3D shape draw). Armed by
// the room-model probe for a SPECIFIC material index so the snapshot lands on
// the plant material and nothing else; compared against tap A (raw-GX veg,
// same texture bytes) to name the differing GX field. Read-only; one shot.
extern "C" void aurora_gx_state_snapshot(char*, unsigned long);
int g_extWwGxTapShape = -1;   // shape index to tap (-1 = off), set by the probe
bool g_extWwGxTapFired = false;
#endif

void J3DShape::drawFast() const {
    ZoneScoped;
#if TARGET_PC
    if (g_extWwGxTapShape >= 0 && !g_extWwGxTapFired &&
        (int)getIndex() == g_extWwGxTapShape) {
        g_extWwGxTapFired = true;
        char buf[2048] = {};
        aurora_gx_state_snapshot(buf, sizeof(buf));
        OSReport("[GXTap] 387 B(J3D shape %d) %s\n", g_extWwGxTapShape, buf);
    }
#endif
#if TARGET_PC
    GXAuroraSetDlOwner(this, "J3DShape");  // §471 P2 breadcrumb (HT-5 strip list)
#endif
    if (sOldVcdVatCmd != mVcdVatCmd) {
        GXCallDisplayList(mVcdVatCmd, kVcdVatDLSize);
        sOldVcdVatCmd = mVcdVatCmd;
    }

    if (sEnvelopeFlag != 0 && !mHasPNMTXIdx)
        mCurrentMtx.load();

    setArrayAndBindPipeline();
    if (!checkFlag(J3DShpFlag_NoMtx)) {
        if (J3DShapeMtx::getLODFlag())
            J3DShapeMtx::resetMtxLoadCache();

        for (u16 n = mMtxGroupNum, i = 0; i < n; i++) {
            if (mShapeMtx[i] != NULL)
                mShapeMtx[i]->load();
            if (mShapeDraw[i] != NULL)
                mShapeDraw[i]->draw();
        }
    } else {
        J3DFifoLoadPosMtxImm(*j3dSys.getShapePacket()->getBaseMtxPtr(), GX_PNMTX0);
        J3DFifoLoadNrmMtxImm(*j3dSys.getShapePacket()->getBaseMtxPtr(), GX_PNMTX0);
        for (u16 n = mMtxGroupNum, i = 0; i < n; i++)
            if (mShapeDraw[i] != NULL)
                mShapeDraw[i]->draw();
    }
}

void J3DShape::draw() const {
    resetVcdVatCache();
    loadPreDrawSetting();
    drawFast();
}

void J3DShape::simpleDraw() const {
    resetVcdVatCache();
    loadPreDrawSetting();
    J3DShapeMtx::setCurrentPipeline((mFlags & 0x1C) >> 2);
    loadVtxArray();
    for (u16 n = mMtxGroupNum, i = 0; i < n; i++) {
        if (mShapeDraw[i] != NULL) {
            mShapeDraw[i]->draw();
        }
    }
}

void J3DShape::simpleDrawCache() const {
#if TARGET_PC
    GXAuroraSetDlOwner(this, "J3DShape");  // §471 P2 breadcrumb (HT-5 strip list)
#endif
    if (sOldVcdVatCmd != mVcdVatCmd) {
        GXCallDisplayList(mVcdVatCmd, kVcdVatDLSize);
        sOldVcdVatCmd = mVcdVatCmd;
    }

    if (sEnvelopeFlag && !mHasPNMTXIdx)
        mCurrentMtx.load();

    loadVtxArray();
    for (u16 n = mMtxGroupNum, i = 0; i < n; i++)
        if (mShapeDraw[i] != NULL)
            mShapeDraw[i]->draw();
}
