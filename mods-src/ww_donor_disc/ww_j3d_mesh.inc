// ============================================================================
// ww_j3d_mesh.inc - DECODE A J3D MODEL'S DISPLAY LISTS INTO TRIANGLES.
//
// WHY THIS EXISTS. J3D DOES NOT STORE TRIANGLES. Each J3DShape owns one or more
// J3DShapeDraw blocks holding a display list: a byte stream of commands and
// per-vertex attribute indices, with the vertices themselves in separate arrays
// on J3DVertexData. Nothing hands you a vertex/index buffer, so drawing this
// geometry WITHOUT GX means walking the stream yourself.
//
// THREE THINGS THIS FILE GOT WRONG BEFORE IT GOT RIGHT, all the same species -
// assuming the GameCube-era shape instead of reading what this PC port actually
// holds in memory. Recorded because the next reader will be tempted identically:
//
//   1. The lists are NOT raw GX. This port re-encodes them, and lwood's begin
//      with GX_AURORA (0x50). Symptom: the walk stopped on byte 0 and nothing
//      rendered at all.
//   2. Non-primitive GX commands are not end-of-stream; they are commands with
//      fixed sizes that must be SKIPPED.
//   3. The position index read out of each vertex record must be BOUNDS-CHECKED.
//      Without it a wrong offset reads arbitrary memory and yields geometry that
//      draws perfectly and means nothing.
//
// SCOPE: POSITIONS ONLY. No normals, colours, texture coordinates or materials.
// Enough to answer whether real lwood geometry at real vertex counts survives
// the GfxService route; not enough to look like a finished tree.
// ============================================================================

namespace {

// GX primitive opcodes (high 5 bits; low 3 select the VAT format).
enum {
    GXP_QUADS         = 0x80,
    GXP_TRIANGLES     = 0x90,
    GXP_TRIANGLESTRIP = 0x98,
    GXP_TRIANGLEFAN   = 0xA0,
};

// aurora's extended command. extern/aurora/include/dolphin/gx/GXCommandList.h:32
enum { GXP_AURORA = 0x50, GXP_AURORA_DRAW_INDEXED = 0x0041 };

struct WwMeshCache {
    const void* key;        // J3DModelData*
    float*      verts;      // xyz triplets, float, model space
    int         triCount;
    int         vertCap;
    int         shapes;
    int         prims;
    int         truncated;  // 1 = hit the cap; triCount is a FLOOR, not a total
    int         badOp;      // first unrecognised opcode, 0 = none
    int         badAt;      // byte offset where it appeared
    int         oobPos;     // rejected position indices: >0 means posOffset is wrong
    int         posComps;
    int         posType;
    u32         posStride;
    // THE ORACLE. An lwood should measure roughly +/-200 wide and ~600 tall.
    // Two axes with IDENTICAL symmetric extents is not what real geometry does;
    // it means indices are landing in the wrong place.
    float       lo[3];
    float       hi[3];
    // THE MODEL'S OWN DECLARED BOUNDS, unioned over its shapes (J3DShape::getMin
    // / getMax, straight out of the BDL). This is an INDEPENDENT oracle: it owes
    // nothing to my decoder. If decLo/decHi agree with lo/hi then the decode is
    // faithful and any remaining wrongness is downstream (transform, scale,
    // winding). If they disagree, the decoder is still lying.
    float       decLo[3];
    float       decHi[3];
    int         decValid;
    // RAW WINDOW over the first vertices of the position array, printed as-is.
    // The declared bounds say my components are rotated by one float; this shows
    // the ACTUAL bytes so the offset is read off the data instead of inferred
    // from a bounding box. Nine floats = three vertices if the stride is 12.
    float       raw[9];
    int         rawOk;
    int         perm[3];   // component permutation chosen against the declared box
    int         permScore; // residual after permuting; 0 = exact agreement
    // PER-SHAPE ACCOUNTING. `shapes` only ever counted shapes that SUCCEEDED, so
    // a shape whose vertex layout was refused vanished without trace - and a
    // silently dropped shape looks exactly like a model that has no such
    // geometry. These make the drop visible.
    int         shapesTotal;   // what the model actually has
    int         shapesSkipped; // layout refused
    int         groupsTotal;   // matrix groups seen across decoded shapes
    int         usesPnMtxIdx;  // any shape carrying PNMTXIDX => joint-space verts
    int         jointApplied;  // matrix groups that got a resolved joint matrix
};

WwMeshCache s_meshCache[4] = {};

// ---------------------------------------------------------------------------
// Vertex-record layout from the VCD. Every vertex in the stream carries one
// entry per ENABLED attribute in GXAttr order; one wrong width desynchronises
// everything after it, so unknown cases refuse rather than guess.
// ---------------------------------------------------------------------------
struct WwVtxLayout {
    int  posWidth;   // bytes of the POS index: 1 or 2
    int  posOffset;  // byte offset of the POS index within a vertex record
    int  stride;     // total bytes per vertex record
    bool ok;
};

WwVtxLayout wwBuildVtxLayout(const GXVtxDescList* desc) {
    WwVtxLayout L = {};
    L.ok = false;
    if (desc == nullptr) {
        return L;
    }
    int offset = 0;
    int posOff = -1;
    for (const GXVtxDescList* d = desc; d->attr != GX_VA_NULL; d++) {
        int w = 0;
        switch (d->type) {
        case GX_NONE:
            w = 0;
            break;
        case GX_DIRECT:
            // Only the *MTXIDX attributes are a bare byte. Any other GX_DIRECT
            // is inline vertex data sized by a GXVtxAttrFmtList; assuming one
            // byte there would desynchronise every following vertex.
            if (d->attr >= GX_VA_POS) {
                return L;
            }
            w = 1;
            break;
        case GX_INDEX8:
            w = 1;
            break;
        case GX_INDEX16:
            w = 2;
            break;
        default:
            return L;
        }
        if (d->attr == GX_VA_POS) {
            if (d->type != GX_INDEX8 && d->type != GX_INDEX16) {
                return L;   // inline positions unsupported
            }
            posOff = offset;
            L.posWidth = w;
        }
        offset += w;
    }
    if (posOff < 0 || offset <= 0) {
        return L;
    }
    L.posOffset = posOff;
    L.stride = offset;
    L.ok = true;
    return L;
}

// ---------------------------------------------------------------------------
// The POSITION array's real geometry. The shortcut accessors do not expose the
// component COUNT at all, and the array is not guaranteed tightly packed.
// ---------------------------------------------------------------------------
struct WwPosFmt {
    int   comps;    // 2 (GX_POS_XY) or 3 (GX_POS_XYZ)
    int   type;     // GXCompType
    float scale;    // 1 / 2^frac
    u32   stride;   // BYTES between consecutive positions
    u32   count;    // number of positions in the array
    bool  ok;
};

WwPosFmt wwResolvePosFmt(const J3DVertexData* vd) {
    WwPosFmt F = {};
    F.ok = false;
    if (vd == nullptr || vd->getVtxPosArray() == nullptr) {
        return F;
    }
    F.comps = 3;
    F.type = vd->getVtxPosType();
    F.scale = 1.0f / static_cast<float>(1u << vd->getVtxPosFrac());
    F.count = vd->getVtxNum();

    const GXVtxAttrFmtList* fmt = vd->getVtxAttrFmtList();
    if (fmt != nullptr) {
        for (const GXVtxAttrFmtList* f = fmt; f->attr != GX_VA_NULL; f++) {
            if (f->attr == GX_VA_POS) {
                F.comps = (f->cnt == GX_POS_XY) ? 2 : 3;
                F.type = f->type;
                F.scale = 1.0f / static_cast<float>(1u << f->frac);
                break;
            }
        }
    }

    F.stride = vd->getVtxArrStride(GX_VA_POS);
    if (F.stride == 0) {
        int elem = 4;
        if (F.type == GX_S16 || F.type == GX_U16) {
            elem = 2;
        } else if (F.type == GX_S8 || F.type == GX_U8) {
            elem = 1;
        }
        F.stride = static_cast<u32>(elem * F.comps);
    }
    F.ok = (F.stride > 0 && F.count > 0);
    return F;
}

bool wwReadPos(const J3DVertexData* vd, const WwPosFmt& F, u32 idx, float out[3]) {
    // BOUNDS CHECK FIRST. Its absence was an out-of-bounds READ, not merely a
    // missing diagnostic: the index comes from bytes at a computed offset inside
    // the vertex record, so a wrong offset makes it arbitrary.
    if (!F.ok || idx >= F.count) {
        return false;
    }
    const u8* base = static_cast<const u8*>(vd->getVtxPosArray()) + (idx * F.stride);
    out[0] = 0.0f;
    out[1] = 0.0f;
    out[2] = 0.0f;
    if (F.type == GX_F32) {
        const float* p = reinterpret_cast<const float*>(base);
        for (int c = 0; c < F.comps; c++) {
            out[c] = p[c];
        }
        return true;
    }
    if (F.type == GX_S16 || F.type == GX_U16) {
        const s16* p = reinterpret_cast<const s16*>(base);
        for (int c = 0; c < F.comps; c++) {
            out[c] = p[c] * F.scale;
        }
        return true;
    }
    if (F.type == GX_S8 || F.type == GX_U8) {
        const s8* p = reinterpret_cast<const s8*>(base);
        for (int c = 0; c < F.comps; c++) {
            out[c] = p[c] * F.scale;
        }
        return true;
    }
    return false;
}

inline u16 wwRead16(const u8* p) {
    return static_cast<u16>((p[0] << 8) | p[1]);   // headers are BIG-endian
}

inline u32 wwRead32(const u8* p) {
    return (static_cast<u32>(p[0]) << 24) | (static_cast<u32>(p[1]) << 16) |
           (static_cast<u32>(p[2]) << 8) | static_cast<u32>(p[3]);
}

// DRAW_INDEXED indices are NATIVE endian while its header fields are big-endian;
// aurora's own DrawCmd::index() memcpy's them without a swap (lib/gx/dl.cpp:105).
// Mixing the two up is silent - valid-looking indices pointing at wrong vertices.
inline u16 wwReadIdxNative(const u8* p) {
    u16 v;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

// Apply a J3D 3x4 (row-major) matrix to a point, in place.
void wwXformPoint(const float m[12], float p[3]) {
    const float x = p[0];
    const float y = p[1];
    const float z = p[2];
    p[0] = m[0] * x + m[1] * y + m[2] * z + m[3];
    p[1] = m[4] * x + m[5] * y + m[6] * z + m[7];
    p[2] = m[8] * x + m[9] * y + m[10] * z + m[11];
}

// Read the POS index out of one vertex record.
bool wwVertPosIndex(const u8* vtxp, u16 vtxCount, const WwVtxLayout& L, u32 seqVtx, u32* out) {
    if (seqVtx >= vtxCount) {
        return false;
    }
    const u8* vp = vtxp + (seqVtx * static_cast<u32>(L.stride));
    *out = (L.posWidth == 2) ? wwRead16(vp + L.posOffset) : vp[L.posOffset];
    return true;
}

// Append one triangle. A plain function rather than a macro: the macro form cost
// a round to line-continuation damage and was unreadable once flattened.
bool wwEmitTri(const u8* idxp, const u8* vtxp, u16 vtxCount, const WwVtxLayout& L,
               const J3DVertexData* vd, const WwPosFmt& F, const float* jointMtx,
               WwMeshCache* out, u32 a, u32 b, u32 c) {
    if (out->triCount * 9 + 9 > out->vertCap) {
        out->truncated = 1;
        return false;
    }
    float* w = out->verts + out->triCount * 9;
    const u32 seq[3] = {a, b, c};
    for (int k = 0; k < 3; k++) {
        const u16 vi = wwReadIdxNative(idxp + (seq[k] * 2u));
        u32 pi = 0;
        if (!wwVertPosIndex(vtxp, vtxCount, L, vi, &pi)) {
            out->oobPos++;
            return false;
        }
        if (!wwReadPos(vd, F, pi, w + k * 3)) {
            out->oobPos++;
            return false;
        }
    }
    // JOINT SPACE -> MODEL SPACE. PNMTXIDX on this model means each matrix
    // group's vertices are stored relative to ITS joint, not the model root.
    // The trunk sits on the root joint so it looked correct untransformed; the
    // canopy is bound to J_Alwd_ha, which is why it collapsed to that joint's
    // local origin instead of standing at Y 598.6..881.3 where the BDL's own
    // declared bounds say it belongs.
    if (jointMtx != nullptr) {
        for (int k = 0; k < 3; k++) {
            wwXformPoint(jointMtx, w + k * 3);
        }
    }
    out->triCount++;
    return true;
}

// ---------------------------------------------------------------------------
// GX_AURORA / DRAW_INDEXED. Layout taken field for field from aurora's own
// parser (extern/aurora/lib/gx/dl.cpp, the GX_AURORA case), not reverse-guessed:
//   [0]      opcode, masked with GX_OPCODE_MASK (0xF8)
//   [1..2]   u16 BE subcommand, must be GX_AURORA_DRAW_INDEXED
//   [3]      draw command; low 3 bits = GXVtxFmt, high bits = PRIMITIVE TYPE
//   [4..5]   u16 BE vertex count
//   [6..9]   u32 BE index count
//   [10..]   indexCount * u16 indices, NATIVE endian
//   then     vtxCount * stride vertex records
// The primitive type still lives in the draw command, so a DRAW_INDEXED strip is
// still a strip and must expand as one.
//
// Returns bytes consumed, or 0 to stop the walk.
// ---------------------------------------------------------------------------
u32 wwAuroraCmd(const u8* dl, u32 size, u32 at, const WwVtxLayout& L,
                const J3DVertexData* vd, const WwPosFmt& F, const float* jointMtx,
                WwMeshCache* out) {
    if (at + 10 > size) {
        return 0;
    }
    if (wwRead16(dl + at + 1) != GXP_AURORA_DRAW_INDEXED) {
        if (out->badOp == 0) {
            out->badOp = dl[at];
            out->badAt = static_cast<int>(at);
        }
        return 0;
    }
    const u8 drawCmd = dl[at + 3];
    const u16 vtxCount = wwRead16(dl + at + 4);
    const u32 idxCount = wwRead32(dl + at + 6);
    const u32 idxBytes = idxCount * 2u;
    const u32 cmdSize = 10u + idxBytes + (static_cast<u32>(vtxCount) * static_cast<u32>(L.stride));
    if (at + cmdSize > size) {
        return 0;
    }
    const u8* idxp = dl + at + 10;
    const u8* vtxp = idxp + idxBytes;
    const u8 prim = static_cast<u8>(drawCmd & 0xF8);
    out->prims++;

    if (prim == GXP_TRIANGLES) {
        for (u32 k = 0; k + 2 < idxCount; k += 3) {
            if (!wwEmitTri(idxp, vtxp, vtxCount, L, vd, F, jointMtx, out, k, k + 1, k + 2)) {
                break;
            }
        }
    } else if (prim == GXP_TRIANGLESTRIP) {
        for (u32 k = 0; k + 2 < idxCount; k++) {
            const bool even = ((k & 1u) == 0u);
            const u32 a = even ? k : k + 1;
            const u32 b = even ? k + 1 : k;
            if (!wwEmitTri(idxp, vtxp, vtxCount, L, vd, F, jointMtx, out, a, b, k + 2)) {
                break;
            }
        }
    } else if (prim == GXP_TRIANGLEFAN) {
        for (u32 k = 1; k + 1 < idxCount; k++) {
            if (!wwEmitTri(idxp, vtxp, vtxCount, L, vd, F, jointMtx, out, 0, k, k + 1)) {
                break;
            }
        }
    } else if (prim == GXP_QUADS) {
        for (u32 k = 0; k + 3 < idxCount; k += 4) {
            if (!wwEmitTri(idxp, vtxp, vtxCount, L, vd, F, jointMtx, out, k, k + 1, k + 2)) {
                break;
            }
            if (!wwEmitTri(idxp, vtxp, vtxCount, L, vd, F, jointMtx, out, k, k + 2, k + 3)) {
                break;
            }
        }
    } else {
        if (out->badOp == 0) {
            out->badOp = prim;
            out->badAt = static_cast<int>(at + 3);
        }
        return 0;
    }
    return cmdSize;
}

// ---------------------------------------------------------------------------
// Walk one display list. Non-primitive GX commands are SKIPPED by their fixed
// known sizes; anything unrecognised records its opcode and offset before
// stopping, because "drew nothing" and "stopped on opcode 0x50" are the same
// symptom with completely different fixes.
// ---------------------------------------------------------------------------
void wwWalkDisplayList(const u8* dl, u32 size, const WwVtxLayout& L,
                       const J3DVertexData* vd, const WwPosFmt& F, const float* jointMtx,
                       WwMeshCache* out) {
    u32 i = 0;
    while (i < size) {
        const u8 op = dl[i];
        if (op == 0x00) {           // NOP padding to the 32-byte boundary
            i++;
            continue;
        }
        if (op == 0x44) {           // invalidate vertex cache
            i += 1;
            continue;
        }
        if (op == 0x08) {           // load CP reg
            i += 6;
            continue;
        }
        if (op == 0x61) {           // load BP reg
            i += 5;
            continue;
        }
        if (op == 0x20 || op == 0x28 || op == 0x30 || op == 0x38) {
            i += 5;                 // load INDX A/B/C/D
            continue;
        }
        if (op == 0x10) {           // load XF regs
            if (i + 5 > size) {
                return;
            }
            i += 5 + ((static_cast<u32>(wwRead16(dl + i + 1)) + 1u) * 4u);
            continue;
        }
        if (op == GXP_AURORA) {
            const u32 adv = wwAuroraCmd(dl, size, i, L, vd, F, jointMtx, out);
            if (adv == 0) {
                return;
            }
            i += adv;
            continue;
        }

        const u8 prim = static_cast<u8>(op & 0xF8);
        if (prim != GXP_QUADS && prim != GXP_TRIANGLES &&
            prim != GXP_TRIANGLESTRIP && prim != GXP_TRIANGLEFAN) {
            if (out->badOp == 0) {
                out->badOp = op;
                out->badAt = static_cast<int>(i);
            }
            return;
        }
        // Raw (non-re-encoded) primitives. Retained for completeness; this port
        // has not been observed to produce them, so they are counted and skipped
        // rather than decoded on a path that has never been exercised.
        if (i + 3 > size) {
            return;
        }
        const u16 n = wwRead16(dl + i + 1);
        const u32 body = i + 3;
        const u32 need = static_cast<u32>(n) * static_cast<u32>(L.stride);
        if (body + need > size) {
            return;
        }
        out->prims++;
        i = body + need;
    }
}

// ---------------------------------------------------------------------------
// Decode a whole model once and cache it against its J3DModelData. Returns
// nullptr if nothing could be decoded; the caller MUST treat that as "no
// geometry", never as "zero triangles is fine".
// ---------------------------------------------------------------------------
WwMeshCache* wwDecodeModelMesh(J3DModel* model, int vertCapFloats) {
    if (model == nullptr) {
        return nullptr;
    }
    J3DModelData* data = model->getModelData();
    if (data == nullptr) {
        return nullptr;
    }
    for (int i = 0; i < 4; i++) {
        if (s_meshCache[i].key == static_cast<const void*>(data)) {
            return &s_meshCache[i];
        }
    }
    int slot = -1;
    for (int i = 0; i < 4; i++) {
        if (s_meshCache[i].key == nullptr) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        return nullptr;
    }

    WwMeshCache* c = &s_meshCache[slot];
    c->verts = static_cast<float*>(std::malloc(sizeof(float) * vertCapFloats));
    if (c->verts == nullptr) {
        return nullptr;
    }
    c->key = data;
    c->vertCap = vertCapFloats;
    c->triCount = 0;
    c->shapes = 0;
    c->prims = 0;
    c->truncated = 0;
    c->badOp = 0;
    c->badAt = -1;
    c->oobPos = 0;
    c->decValid = 0;
    c->rawOk = 0;
    c->perm[0] = 0;
    c->perm[1] = 1;
    c->perm[2] = 2;
    c->permScore = -1;
    c->shapesTotal = 0;
    c->shapesSkipped = 0;
    c->groupsTotal = 0;
    c->usesPnMtxIdx = 0;
    c->jointApplied = 0;
    for (int a = 0; a < 9; a++) {
        c->raw[a] = 0.0f;
    }
    for (int a = 0; a < 3; a++) {
        c->decLo[a] = 1e30f;
        c->decHi[a] = -1e30f;
    }
    for (int a = 0; a < 3; a++) {
        c->lo[a] = 1e30f;
        c->hi[a] = -1e30f;
    }

    const J3DVertexData* vd = &data->getVertexData();   // returns a REFERENCE
    const WwPosFmt PF = wwResolvePosFmt(vd);
    if (PF.ok && PF.type == GX_F32 && PF.count >= 3) {
        const float* fp = static_cast<const float*>(vd->getVtxPosArray());
        for (int a = 0; a < 9; a++) {
            c->raw[a] = fp[a];
        }
        c->rawOk = 1;
    }
    c->posComps = PF.comps;
    c->posType = PF.type;
    c->posStride = PF.stride;

    const u16 shapeNum = data->getShapeNum();
    c->shapesTotal = shapeNum;
    for (u16 s = 0; s < shapeNum; s++) {
        J3DShape* shp = data->getShapeNodePointer(s);
        if (shp == nullptr) {
            continue;
        }
        const WwVtxLayout L = wwBuildVtxLayout(shp->getVtxDesc());
        if (!L.ok) {
            c->shapesSkipped++;   // VISIBLE now; this used to vanish silently
            continue;
        }
        // PNMTXIDX present means the shape's vertices are in JOINT space and a
        // per-matrix-group transform is required to place them. The trunk sits
        // near the root joint so it looks right untransformed; leaf geometry
        // bound to J_Alwd_ha would not.
        for (const GXVtxDescList* d = shp->getVtxDesc(); d != nullptr && d->attr != GX_VA_NULL; d++) {
            if (d->attr == GX_VA_PNMTXIDX && d->type != GX_NONE) {
                c->usesPnMtxIdx = 1;
                break;
            }
        }
        c->shapes++;
        {
            const Vec* mn = shp->getMin();
            const Vec* mx = shp->getMax();
            if (mn != nullptr && mx != nullptr) {
                const float lo3[3] = {mn->x, mn->y, mn->z};
                const float hi3[3] = {mx->x, mx->y, mx->z};
                for (int a = 0; a < 3; a++) {
                    if (lo3[a] < c->decLo[a]) { c->decLo[a] = lo3[a]; }
                    if (hi3[a] > c->decHi[a]) { c->decHi[a] = hi3[a]; }
                }
                c->decValid = 1;
            }
        }
        const u16 groups = shp->getMtxGroupNum();
        c->groupsTotal += groups;
        for (u16 g = 0; g < groups; g++) {
            J3DShapeDraw* sd = shp->getShapeDraw(g);
            if (sd == nullptr) {
                continue;
            }
            // Resolve THIS matrix group's joint matrix. getUseMtxIndex names a
            // draw matrix; on a rigid model that index is the joint, and
            // getAnmMtx returns its MODEL-space matrix - which is what we want,
            // because the per-instance world placement still rides in the MVP.
            // If it cannot be resolved we pass null and the group stays in joint
            // space, which is the current (visibly wrong) behaviour rather than
            // a silent nudge toward looking right.
            const float* jointMtx = nullptr;
            J3DShapeMtx* smtx = shp->getShapeMtx(g);
            if (smtx != nullptr) {
                const u16 useIdx = smtx->getUseMtxIndex(0);
                MtxP am = model->getAnmMtx(useIdx);
                if (am != nullptr) {
                    jointMtx = reinterpret_cast<const float*>(am);
                    c->jointApplied++;
                }
            }
            const u8* dl = sd->getDisplayList();
            const u32 sz = sd->getDisplayListSize();
            if (dl != nullptr && sz > 0) {
                wwWalkDisplayList(dl, sz, L, vd, PF, jointMtx, c);
            }
        }
    }

    for (int t = 0; t < c->triCount * 9; t++) {
        const int ax = t % 3;
        if (c->verts[t] < c->lo[ax]) {
            c->lo[ax] = c->verts[t];
        }
        if (c->verts[t] > c->hi[ax]) {
            c->hi[ax] = c->verts[t];
        }
    }
    if (c->decValid == 0) {
        for (int a = 0; a < 3; a++) {
            c->decLo[a] = 0.0f;
            c->decHi[a] = 0.0f;
        }
    }
    if (c->triCount == 0) {
        for (int a = 0; a < 3; a++) {
            c->lo[a] = 0.0f;
            c->hi[a] = 0.0f;
        }
    }
    // ------------------------------------------------------------------------
    // CHOOSE THE COMPONENT ORDER FROM THE DECLARED BOX, DO NOT GUESS IT.
    //
    // The raw window showed component 0 constant across consecutive vertices
    // while the others varied - a horizontal ring, so component 0 is HEIGHT, not
    // X. Reading it as X also puts a vertex 1.2 units below the model's own
    // declared Y minimum, which a bounding box cannot legally do.
    //
    // Rather than hand-pick a permutation, score all six against the bounds the
    // BDL itself declares and take the best. That makes the choice a measurement
    // instead of my fourth assumption about this data - and if the residual is
    // not near zero, the receipt says so and none of this should be believed.
    //
    // HONEST LIMIT: two axes are both +/-660.6, so the box CANNOT distinguish X
    // from Z. Ties are broken by order, which may yield a MIRRORED tree. A
    // mirrored tree still validates the whole pipeline; it is a cosmetic fix
    // afterwards, and calling it out beats quietly shipping a reflection.
    // ------------------------------------------------------------------------
    if (c->triCount > 0 && c->decValid != 0) {
        static const int kPerms[6][3] = {
            {0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 0, 1}, {2, 1, 0},
        };
        int best = 0;
        float bestScore = 1e30f;
        for (int pi = 0; pi < 6; pi++) {
            float score = 0.0f;
            for (int a = 0; a < 3; a++) {
                const int src = kPerms[pi][a];
                float d0 = c->lo[src] - c->decLo[a];
                float d1 = c->hi[src] - c->decHi[a];
                if (d0 < 0.0f) { d0 = -d0; }
                if (d1 < 0.0f) { d1 = -d1; }
                score += d0 + d1;
            }
            if (score < bestScore) {
                bestScore = score;
                best = pi;
            }
        }
        c->perm[0] = kPerms[best][0];
        c->perm[1] = kPerms[best][1];
        c->perm[2] = kPerms[best][2];
        c->permScore = static_cast<int>(bestScore);

        if (best != 0) {
            for (int t = 0; t < c->triCount * 9; t += 3) {
                const float v[3] = {c->verts[t], c->verts[t + 1], c->verts[t + 2]};
                c->verts[t]     = v[kPerms[best][0]];
                c->verts[t + 1] = v[kPerms[best][1]];
                c->verts[t + 2] = v[kPerms[best][2]];
            }
            for (int a = 0; a < 3; a++) {
                c->lo[a] = 1e30f;
                c->hi[a] = -1e30f;
            }
            for (int t = 0; t < c->triCount * 9; t++) {
                const int ax = t % 3;
                if (c->verts[t] < c->lo[ax]) { c->lo[ax] = c->verts[t]; }
                if (c->verts[t] > c->hi[ax]) { c->hi[ax] = c->verts[t]; }
            }
        }
    }
    return c;
}

}  // namespace
