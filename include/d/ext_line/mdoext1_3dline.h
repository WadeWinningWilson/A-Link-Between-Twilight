#ifndef D_EXT_LINE_MDOEXT1_3DLINE_H
#define D_EXT_LINE_MDOEXT1_3DLINE_H

// ============================================================================
// §484 MDoExt1 — THE DONOR'S 3D-LINE STACK, PORTED AS A PARALLEL WW TYPE.
//
// USER ORDER (2026-08-06): port the stack that makes the ropes appear from WW
// code, after §483 conceded that §460's shape — subclass TP's class, inherit
// its body, diverge by editing — is exactly what a parallel donor stack exists
// to prevent. This follows the JAudio1:: / JEvent1:: precedent: the donor's own
// classes live in their own namespace with their own data, their own display
// lists and their own draw. Nothing of TP's body is inherited or edited.
//
// WHAT IS *NOT* PARALLEL YET, declared rather than glossed:
//   [W0] LineMat1_c still derives from the receiver's abstract
//        mDoExt_3DlineMat_c. That is the ONE shared contract, and it exists
//        only so a WW line can ride the receiver's existing
//        mDoExt_3DlineMatSortPacket / dComIfGd_set3DlineMat plumbing. The base
//        is pure virtual (getMaterialID/setMaterial/draw) plus one link
//        pointer — no behaviour is inherited. Porting the donor's own sort
//        packet and draw-list entry is PHASE 2 and remains owed.
//
// DONOR SOURCE, read directly (WW DP), not from summaries:
//   m_Do_ext.h:570-633   class layouts (note mpSize is u8*, NOT f32*)
//   m_Do_ext.cpp:1880    mDoExt_3Dline_c::init
//   m_Do_ext.cpp:2192    mDoExt_3DlineMat1_c::init
//   m_Do_ext.cpp:2217    setMaterial (l_normal, INDEX8 normals, tevstr branch)
//   m_Do_ext.cpp:2252    draw
//   m_Do_ext.cpp:2287    update (5-arg, uniform width — the rail)
//   m_Do_ext.cpp:2408    update (3-arg, per-vertex u8 widths — the hangers)
// ============================================================================

#include "SSystem/SComponent/c_xyz.h"
#include "m_Do/m_Do_ext.h"

class dKy_tevstr_c;
struct ResTIMG;

namespace MDoExt1 {

// donor mDoExt_3Dline_c (m_Do_ext.h:570-584) — per-line buffers, double
// -buffered for the GP/CPU split. [W1] receiver spelling for cXy/cXyz only.
class Line3D_c {
public:
    Line3D_c() {}
    ~Line3D_c() {}

    BOOL init(u16 numSegments, BOOL hasSize, BOOL hasTex);

    /* 0x00 */ cXyz* mpSegments;
    /* 0x04 */ u8* mpSize;  // donor: u8 widths, NOT the receiver's f32
    /* 0x08 */ cXyz* mPosArr[2];
    /* 0x10 */ cXy* mTexArr[2];
};

// donor mDoExt_3DlineMat1_c (m_Do_ext.h:610-633). Own data, own DLs, own draw.
class LineMat1_c : public mDoExt_3DlineMat_c {
public:
    LineMat1_c() : mpLines(NULL), mpTevStr(NULL), mNumLines(0), mMaxSegments(0),
                   mNumSegments(0), mCurArr(0) {}
    ~LineMat1_c() {}

    BOOL init(u16 numLines, u16 numSegments, ResTIMG* i_img, BOOL hasSize);
    void update(u16 segs, f32 size, GXColor& newColor, u16 space, dKy_tevstr_c* pTevStr);
    void update(u16 segs, GXColor& newColor, dKy_tevstr_c* pTevStr);

    int getMaterialID() override { return 1; }
    void setMaterial() override;
    void draw() override;

    cXyz* getPos(int i_idx) { return mpLines[i_idx].mpSegments; }
    u8* getSize(int i_idx) { return mpLines[i_idx].mpSize; }

private:
    /* 0x08 */ TGXTexObj mTexObj;
    /* 0x28 */ GXColor mColor;
    /* 0x2C */ dKy_tevstr_c* mpTevStr;
    /* 0x30 */ u16 mNumLines;
    /* 0x32 */ u16 mMaxSegments;
    /* 0x34 */ u16 mNumSegments;
    /* 0x36 */ u8 mCurArr;
    /* 0x38 */ Line3D_c* mpLines;
};

}  // namespace MDoExt1

#endif /* D_EXT_LINE_MDOEXT1_3DLINE_H */
