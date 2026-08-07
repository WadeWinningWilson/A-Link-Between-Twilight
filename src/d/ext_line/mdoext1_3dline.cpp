// KIT-LINEAGE: native-port
// KIT-DONOR: m_Do/m_Do_ext.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: NonMatching
// ============================================================================
// §484 MDoExt1 — donor 3D-line stack, ported VERBATIM. See the header for the
// campaign rationale and the donor line references.
//
// SEAM INDEX (every difference from donor source is listed; nothing silent):
//  [W0] LineMat1_c derives from the receiver's abstract mDoExt_3DlineMat_c so a
//       WW line can ride the existing sort packet. Pure-virtual only; no
//       behaviour inherited. Donor's own packet = phase 2, owed.
//  [W1] receiver member spelling: donor view->mLookat.mEye -> view->lookat.eye
//       (view_class holds lookat DIRECTLY; only camera_class nests it under .view).
//  [W2] receiver tevstr names for the donor's colour pair:
//       mColorC0 -> TevColor (GXColorS10), mColorK0 -> TevKColor (GXColor).
//       Verified by runtime data in §478-P68, not by assumption.
//  [W3] GXSetArray: the receiver's aurora exposes the donor's 3-arg form, so
//       the donor's calls are kept as written.
//  [W4] ADDITIVE, NOT A SUBSTITUTION (§482 rule): the donor's draw sets no
//       channel ambient because its surrounding frame supplies one — decomp
//       shows dKy_setLight_again loads LIGHT 0 only, and noclip's m_do_ext.ts
//       calls dKy_setLight__OnMaterialParams on this material. l_toonMat1DL's
//       stage 0 is TEXTURE-DISABLED on colour channel 0 with lighting enabled
//       and ambSrc=REGISTER (TREF 0x3C0000, XF 0x100E 0x0506), so without an
//       ambient register that stage renders black and multiplies the material
//       black. dKy_Global_amb_set -> GXSetChanAmbColor(GX_COLOR0A0) is this
//       receiver's supply for that context. It is ADDED, and no donor line is
//       removed to make room for it.
//  [W5] donor JUT_ASSERT on the size array is kept as a null-guard + one-shot
//       warning rather than an abort: a missing size array is a caller bug, and
//       this port must not introduce a new crash class.
// ============================================================================

#include "d/ext_line/mdoext1_3dline.h"

#include "JSystem/J3DGraphBase/J3DSys.h"
#include "SSystem/SComponent/c_lib.h"
#include "d/d_com_inf_game.h"
#include "d/d_kankyo.h"
#include "d/d_kankyo_tev_str.h"
#include "dusk/logging.h"

namespace MDoExt1 {

// donor l_mat1DL — the NON-tevstr material list, 0x80 bytes as called.
// SOURCE: WW DP build asset include/assets/l_mat1DL.h (size 0x96,
// .data @ 0x803719A0). BIG-ENDIAN and kept so: aurora drains the FIFO with
// bigEndian=true, which is why §441b's LE swap broke the title reins.
//
// §485 CORRECTION, on the record: the first version of this array was TP's
// l_mat1DL copied out of the receiver and MISLABELLED "donor". They are NOT the
// same list -- donor XF 0x100E = 0x00000506 vs TP 0x00007F32 (a different
// channel-control, i.e. different LIGHTING), donor carries an XF 0x100A write
// TP lacks, and donor BP 0xC0 = 0x08FA8F vs TP 0x28FA8F. Sizes differ too
// (0x96 vs 141). Caught by diffing against the asset instead of trusting what
// was already in context -- the §443 rule applied to myself.
// KIT-DONOR-DATA: 150 display-list WW DP build asset include/assets/l_mat1DL.h
static u8 l_mat1DL[0x96] ATTRIBUTE_ALIGN(32) = {
    0x10, 0x00, 0x00, 0x10, 0x40, 0xFF, 0xFF, 0x42, 0x80, 0x08, 0x30, 0x3C, 0xF3, 0xCF, 0x00, 0x10,
    0x00, 0x00, 0x10, 0x18, 0x3C, 0xF3, 0xCF, 0x00, 0x10, 0x00, 0x00, 0x10, 0x0E, 0x00, 0x00, 0x05,
    0x06, 0x10, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x05, 0x00, 0x10, 0x00, 0x00, 0x10, 0x0A, 0x00,
    0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x0C, 0xFF, 0xFF, 0xFF, 0xFF, 0x61, 0x28, 0x38, 0x00,
    0x40, 0x61, 0xC0, 0x08, 0xFA, 0x8F, 0x61, 0xC1, 0x08, 0xFF, 0xF0, 0x61, 0x43, 0x00, 0x00, 0x41,
    0x61, 0x40, 0x00, 0x00, 0x17, 0x61, 0x41, 0x00, 0x00, 0x0C, 0x61, 0xF3, 0x7F, 0x00, 0x00, 0x10,
    0x00, 0x00, 0x10, 0x3F, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x10, 0x09, 0x00, 0x00, 0x00,
    0x01, 0x61, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// donor l_toonMat1DL — the TEVSTR material list, 0xA0 bytes as called. Source:
// WW DP build asset include/assets/l_toonMat1DL.h (0xA5, .data @ 0x80371A40).
// KIT-DONOR-DATA: 165 display-list WW DP build asset include/assets/l_toonMat1DL.h
static u8 l_toonMat1DL[0xA5] ATTRIBUTE_ALIGN(32) = {
    0x10, 0x00, 0x00, 0x10, 0x40, 0xFF, 0xFF, 0x42, 0x80, 0x08, 0x30, 0x3C, 0xF3, 0xCF, 0x00, 0x10,
    0x00, 0x00, 0x10, 0x18, 0x3C, 0xF3, 0xCF, 0x00, 0x10, 0x00, 0x00, 0x10, 0x0E, 0x00, 0x00, 0x05,
    0x06, 0x10, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x05, 0x00, 0x10, 0x00, 0x00, 0x10, 0x0A, 0x00,
    0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x0C, 0xFF, 0xFF, 0xFF, 0xFF, 0x61, 0x28, 0x3C, 0x00,
    0x00, 0x61, 0xC0, 0x08, 0x24, 0xAF, 0x61, 0xC1, 0x08, 0xFF, 0xF0, 0x61, 0x28, 0x3C, 0x00, 0x00,
    0x61, 0xC2, 0x08, 0xF0, 0x8F, 0x61, 0xC3, 0x08, 0xFF, 0xE0, 0x61, 0x43, 0x00, 0x00, 0x41, 0x61,
    0x40, 0x00, 0x00, 0x17, 0x61, 0x41, 0x00, 0x00, 0x0C, 0x61, 0xF3, 0x7F, 0x00, 0x00, 0x10, 0x00,
    0x00, 0x10, 0x3F, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x10, 0x09, 0x00, 0x00, 0x00, 0x01,
    0x61, 0x00, 0x00, 0x04, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00,
};

// donor setMaterial's static 2-entry normal array: +X and -X, indexed per
// vertex by GXNormal1x8(0/1). This is why the donor's NRM is INDEX8 and why it
// needs no per-vertex normal buffer at all.
// KIT-DONOR-DATA: 6 lookup-table m_Do/m_Do_ext.cpp mDoExt_3DlineMat1_c::setMaterial
static u8 l_normal[][3] ATTRIBUTE_ALIGN(32) = {
    {0x40, 0x00, 0x00},
    {0xC0, 0x00, 0x00},
};

// ---------------------------------------------------------------------------
// donor mDoExt_3Dline_c::init (m_Do_ext.cpp:1880-1924)
// ---------------------------------------------------------------------------
BOOL Line3D_c::init(u16 numSegments, BOOL hasSize, BOOL hasTex) {
    mpSegments = new cXyz[numSegments];
    if (mpSegments == NULL) {
        return FALSE;
    }

    if (hasSize) {
        mpSize = new u8[numSegments];
        if (mpSize == NULL) {
            return FALSE;
        }
    } else {
        mpSize = NULL;
    }

    u32 numArr = numSegments * 2;
    mPosArr[0] = new cXyz[numArr];
    if (mPosArr[0] == NULL) {
        return FALSE;
    }
    mPosArr[1] = new cXyz[numArr];
    if (mPosArr[1] == NULL) {
        return FALSE;
    }

    if (hasTex) {
        mTexArr[0] = new cXy[numArr];
        if (mTexArr[0] == NULL) {
            return FALSE;
        }
        mTexArr[1] = new cXy[numArr];
        if (mTexArr[1] == NULL) {
            return FALSE;
        }

        cXy* tex0 = mTexArr[0];
        cXy* tex1 = mTexArr[1];
        for (s32 i = 0; i < numSegments; i++) {
            tex0[0].x = 0.0f;
            tex1[0].x = 0.0f;
            tex0[1].x = 1.0f;
            tex1[1].x = 1.0f;
            tex0 += 2;
            tex1 += 2;
        }
    } else {
        mTexArr[0] = NULL;
        mTexArr[1] = NULL;
    }

    return TRUE;
}

// ---------------------------------------------------------------------------
// donor mDoExt_3DlineMat1_c::init (m_Do_ext.cpp:2192-2214)
// ---------------------------------------------------------------------------
BOOL LineMat1_c::init(u16 numLines, u16 numSegments, ResTIMG* i_img, BOOL hasSize) {
    mNumLines = numLines;
    mMaxSegments = numSegments;
    mpLines = new Line3D_c[numLines];
    if (mpLines == NULL) {
        return FALSE;
    }

    for (s32 i = 0; i < numLines; i++) {
        if (!mpLines[i].init(numSegments, hasSize, TRUE)) {
            return FALSE;
        }
    }

    field_0x4 = NULL;  // donor mpNextLineMat, named field_0x4 on the receiver base
    mCurArr = 0;

    GXInitTexObj(&mTexObj, (&i_img->format + i_img->imageOffset), i_img->width, i_img->height,
                 (GXTexFmt)i_img->format, (GXTexWrapMode)i_img->wrapS,
                 (GXTexWrapMode)i_img->wrapT, (GXBool)(i_img->mipmapCount > 1));
    GXInitTexObjLOD(&mTexObj, (GXTexFilter)i_img->minFilter, (GXTexFilter)i_img->magFilter,
                    i_img->minLOD * 0.125f, i_img->maxLOD * 0.125f, i_img->LODBias * 0.01f,
                    (GXBool)i_img->biasClamp, (GXBool)i_img->doEdgeLOD,
                    (GXAnisotropy)i_img->maxAnisotropy);

    return TRUE;
}

// ---------------------------------------------------------------------------
// donor mDoExt_3DlineMat1_c::setMaterial (m_Do_ext.cpp:2217-2249)
// ---------------------------------------------------------------------------
void LineMat1_c::setMaterial() {
    j3dSys.reinitGX();
    GXSetNumIndStages(0);
    dKy_setLight_again();

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX8);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 6);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    GXSetArray(GX_VA_NRM, l_normal, sizeof(l_normal), sizeof(*l_normal), true);  // [W3]

    if (mpTevStr) {
        dKy_GxFog_tevstr_set(mpTevStr);
        GXCallDisplayList(l_toonMat1DL, 0xA0);
    } else {
        dKy_GxFog_set();
        GXCallDisplayList(l_mat1DL, 0x80);
    }

    GXLoadPosMtxImm(j3dSys.getViewMtx(), GX_PNMTX0);
    GXLoadNrmMtxImm(cMtx_getIdentity(), GX_PNMTX0);
}

// ---------------------------------------------------------------------------
// donor mDoExt_3DlineMat1_c::draw (m_Do_ext.cpp:2252-2285)
// ---------------------------------------------------------------------------
void LineMat1_c::draw() {
    GXLoadTexObj(&mTexObj, GX_TEXMAP0);
    u16 h = GXGetTexObjHeight(&mTexObj);
    u16 w = GXGetTexObjWidth(&mTexObj);
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, w, h);
    GXSetTevColor(GX_TEVREG2, mColor);
    if (mpTevStr != NULL) {
        GXSetTevColorS10(GX_TEVREG0, mpTevStr->TevColor);   // [W2] donor mColorC0
        GXSetTevColor(GX_TEVREG1, mpTevStr->TevKColor);     // [W2] donor mColorK0
        dKy_Global_amb_set(mpTevStr);                       // [W4] ADDED, see header
    }

    Line3D_c* line = mpLines;
    u16 numTriStrip = mNumSegments * 2;
    for (s32 i = 0; i < mNumLines; i++) {
        GXSetArray(GX_VA_POS, line->mPosArr[mCurArr], numTriStrip * sizeof(cXyz),
                   sizeof(cXyz), true);  // [W3]
        GXSetArray(GX_VA_TEX0, line->mTexArr[mCurArr], numTriStrip * sizeof(cXy),
                   sizeof(cXy), true);  // [W3]
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, numTriStrip);
        for (u16 j = 0; j < numTriStrip;) {
            GXPosition1x16(j);
            GXNormal1x8(0);
            GXTexCoord1x16(j++);

            GXPosition1x16(j);
            GXNormal1x8(1);
            GXTexCoord1x16(j++);
        }
        GXEnd();
        line++;
    }

    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 0, 0);
    mCurArr ^= 1;
}

// ---------------------------------------------------------------------------
// donor update, 5-arg: uniform width tapering over `space` (m_Do_ext.cpp:2287)
// ---------------------------------------------------------------------------
void LineMat1_c::update(u16 segs, f32 size, GXColor& newColor, u16 space,
                        dKy_tevstr_c* pTevStr) {
    cXyz eyeDelta;
    cXyz delta;
    cXyz nextP0;
    cXyz nextP1;

    mColor = newColor;
    mpTevStr = pTevStr;
    mNumSegments = segs;
    if (mNumSegments > mMaxSegments) {
        mNumSegments = mMaxSegments;
    }

    view_class* view = dComIfGd_getView();
    Line3D_c* line = mpLines;

    f32 spacing;
    if (space != 0) {
        spacing = size / space;
    } else {
        spacing = 0.0f;
    }

    u32 posArrSize = mNumSegments * 2 * sizeof(cXyz);
    u32 texArrSize = mNumSegments * 2 * sizeof(cXy);

    f32 r_size;
    f32 dist = 0.0f;
    for (s32 i = 0; i < mNumLines; i++) {
        cXyz* pos = line->mpSegments;
        cXyz* dstPos = line->mPosArr[mCurArr];
        cXy* dstTex = line->mTexArr[mCurArr];
        r_size = size;

        dstTex[0].y = dist;
        dstTex[1].y = dist;

        cXy* r_dstTex = dstTex;
        r_dstTex += 2;

        delta = pos[1] - pos[0];
        f32 mag = delta.abs();

        dist += mag * 0.1f;
        eyeDelta = pos[0] - view->lookat.eye;  // [W1]
        delta = delta.outprod(eyeDelta);
        f32 scale = delta.abs();
        if (scale != 0.0f) {
            scale = size / scale;
            delta *= scale;
        }

        dstPos[0] = pos[0] + delta;
        dstPos[1] = pos[0] - delta;

        cXyz* r_dstPos = dstPos;
        r_dstPos += 2;

        pos++;
        nextP0 = pos[0] + delta;
        nextP1 = pos[0] - delta;

        for (s32 j = mNumSegments - 2; j > 0; j--) {
            if (j < space) {
                r_size -= spacing;
            }

            r_dstTex[0].y = dist;
            r_dstTex[1].y = dist;
            r_dstTex += 2;

            delta = pos[1] - pos[0];
            f32 mag2 = delta.abs();

            dist += mag2 * 0.1f;
            eyeDelta = pos[0] - view->lookat.eye;  // [W1]
            delta = delta.outprod(eyeDelta);
            scale = delta.abs();
            if (scale != 0.0f) {
                scale = r_size / scale;
            }

            delta *= scale;
            nextP0 += pos[0] + delta;
            nextP1 += pos[0] - delta;

            r_dstPos[0] = nextP0 * 0.5f;
            r_dstPos[1] = nextP1 * 0.5f;
            r_dstPos += 2;

            pos++;

            nextP0 = pos[0] + delta;
            nextP1 = pos[0] - delta;
        }

        r_dstTex[0].y = dist;
        r_dstTex[1].y = dist;

        if (space != 0) {
            r_dstPos[0] = pos[0];
            r_dstPos[1] = pos[0];
        } else {
            r_dstPos[0] = nextP0;
            r_dstPos[1] = nextP1;
        }

        DCStoreRangeNoSync(dstPos, posArrSize);
        DCStoreRangeNoSync(dstTex, texArrSize);
        line++;
    }
}

// ---------------------------------------------------------------------------
// donor update, 3-arg: PER-VERTEX u8 widths (m_Do_ext.cpp:2408)
// ---------------------------------------------------------------------------
void LineMat1_c::update(u16 segs, GXColor& newColor, dKy_tevstr_c* pTevStr) {
    cXyz eyeDelta;
    cXyz delta;
    cXyz nextP0;
    cXyz nextP1;

    mColor = newColor;
    mpTevStr = pTevStr;
    mNumSegments = segs;
    if (mNumSegments > mMaxSegments) {
        mNumSegments = mMaxSegments;
    }

    view_class* view = dComIfGd_getView();
    Line3D_c* line = mpLines;

    u32 posArrSize = mNumSegments * 2 * sizeof(cXyz);
    u32 texArrSize = mNumSegments * 2 * sizeof(cXy);

    f32 dist = 0.0f;
    for (s32 i = 0; i < mNumLines; i++) {
        cXyz* pos = line->mpSegments;
        u8* size_p = line->mpSize;
        if (size_p == NULL) {
            // [W5] donor JUT_ASSERTs here. A missing size array is a caller
            // bug; report once and skip rather than add a new abort path.
            static bool s_warned = false;
            if (!s_warned) {
                s_warned = true;
                DuskLog.warn("[MDoExt1] 484 per-vertex update on a line with NO size array "
                             "(init hasSize=0) -- line skipped");
            }
            line++;
            continue;
        }

        cXyz* dstPos = line->mPosArr[mCurArr];
        cXy* dstTex = line->mTexArr[mCurArr];

        dstTex[0].y = dist;
        dstTex[1].y = dist;

        cXy* r_dstTex = dstTex;
        r_dstTex += 2;

        delta = pos[1] - pos[0];
        f32 mag = delta.abs();

        dist += mag * 0.1f;
        eyeDelta = pos[0] - view->lookat.eye;  // [W1]
        delta = delta.outprod(eyeDelta);
        f32 scale = delta.abs();
        if (scale != 0.0f) {
            scale = *size_p / scale;
            delta *= scale;
        }

        dstPos[0] = pos[0] + delta;
        dstPos[1] = pos[0] - delta;

        cXyz* r_dstPos = dstPos;
        r_dstPos += 2;
        size_p++;

        pos++;
        nextP0 = pos[0] + delta;
        nextP1 = pos[0] - delta;

        for (s32 j = mNumSegments - 2; j > 0; j--) {
            r_dstTex[0].y = dist;
            r_dstTex[1].y = dist;
            r_dstTex += 2;

            delta = pos[1] - pos[0];
            f32 mag2 = delta.abs();

            dist += mag2 * 0.1f;
            eyeDelta = pos[0] - view->lookat.eye;  // [W1]
            delta = delta.outprod(eyeDelta);
            scale = delta.abs();
            if (scale != 0.0f) {
                scale = *size_p / scale;
            }

            delta *= scale;
            nextP0 += pos[0] + delta;
            nextP1 += pos[0] - delta;

            r_dstPos[0] = nextP0 * 0.5f;
            r_dstPos[1] = nextP1 * 0.5f;
            r_dstPos += 2;

            pos++;
            size_p++;

            nextP0 = pos[0] + delta;
            nextP1 = pos[0] - delta;
        }

        r_dstTex[0].y = dist;
        r_dstTex[1].y = dist;

        r_dstPos[0] = nextP0;
        r_dstPos[1] = nextP1;

        DCStoreRangeNoSync(dstPos, posArrSize);
        DCStoreRangeNoSync(dstTex, texArrSize);
        line++;
    }
}

}  // namespace MDoExt1
