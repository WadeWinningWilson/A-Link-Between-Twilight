// ============================================================================
// Level Editor — camera→cursor ray picking + model-tight highlight (Gate 11).
// See raycast.hpp. All math is row-major to match mDoLib_project's use of
// dComIfGd_getProjViewMtx() (world→clip); its inverse is therefore an exact
// screen→world of the same projection already used to draw the frame.
//
// Gate 11b (Path A): the pick is a broad-phase cull-volume reject followed by a
// narrow-phase test against the model's PER-JOINT bounding boxes — the loose
// cull box is only a coarse gate, the joints hug the mesh. The same per-joint
// volumes drive the selection highlight, so what you highlight is what you pick.
// ============================================================================

#include "dusk/leveledit/raycast.hpp"

#if TARGET_PC

#include "d/d_com_inf_game.h"
#include "d/d_debug_viewer.h"
#include "dusk/settings.h"
#include "f_op/f_op_actor.h"
#include "f_op/f_op_actor_mng.h"
#include "m_Do/m_Do_graphic.h"
#include "m_Do/m_Do_mtx.h"

#include "JSystem/J3DGraphAnimator/J3DJoint.h"
#include "JSystem/J3DGraphAnimator/J3DModel.h"
#include "JSystem/J3DGraphAnimator/J3DModelData.h"

#include <algorithm>
#include <cfloat>
#include <cmath>

namespace dusk::leveledit {
namespace {

// ============================================================================
// General 4x4 inverse via Gauss-Jordan with partial pivoting (row-major).
// Kept self-contained: the GC MTX lib only inverts 3x4 affine matrices, and
// the ProjView matrix is a full projective 4x4.
// ============================================================================
bool invert4x4(const f32 in[4][4], f32 out[4][4]) {
    f32 a[4][8];
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            a[r][c] = in[r][c];
            a[r][c + 4] = (r == c) ? 1.0f : 0.0f;
        }
    }
    for (int col = 0; col < 4; ++col) {
        int pivot = col;
        f32 best = std::fabs(a[col][col]);
        for (int r = col + 1; r < 4; ++r) {
            const f32 v = std::fabs(a[r][col]);
            if (v > best) {
                best = v;
                pivot = r;
            }
        }
        if (best < 1.0e-12f) {
            return false;  // singular
        }
        if (pivot != col) {
            for (int c = 0; c < 8; ++c) {
                std::swap(a[col][c], a[pivot][c]);
            }
        }
        const f32 inv = 1.0f / a[col][col];
        for (int c = 0; c < 8; ++c) {
            a[col][c] *= inv;
        }
        for (int r = 0; r < 4; ++r) {
            if (r == col) {
                continue;
            }
            const f32 f = a[r][col];
            if (f == 0.0f) {
                continue;
            }
            for (int c = 0; c < 8; ++c) {
                a[r][c] -= f * a[col][c];
            }
        }
    }
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            out[r][c] = a[r][c + 4];
        }
    }
    return true;
}

// Unproject an NDC point (nx, ny, nz) to world via the inverse ProjView.
void unproject(const f32 invPV[4][4], f32 nx, f32 ny, f32 nz, cXyz& out) {
    const f32 x = invPV[0][0] * nx + invPV[0][1] * ny + invPV[0][2] * nz + invPV[0][3];
    const f32 y = invPV[1][0] * nx + invPV[1][1] * ny + invPV[1][2] * nz + invPV[1][3];
    const f32 z = invPV[2][0] * nx + invPV[2][1] * ny + invPV[2][2] * nz + invPV[2][3];
    f32 w = invPV[3][0] * nx + invPV[3][1] * ny + invPV[3][2] * nz + invPV[3][3];
    if (std::fabs(w) < 1.0e-9f) {
        w = (w < 0.0f) ? -1.0e-9f : 1.0e-9f;
    }
    const f32 iw = 1.0f / w;
    out.set(x * iw, y * iw, z * iw);
}

// Ray vs axis-aligned box (slab method). Returns the nearest t >= 0 (entry, or
// exit if the origin is inside). d need not be unit — callers pass a world-unit
// ray transformed into local space, where the parameter t is invariant, so the
// returned t equals the world distance.
bool ray_aabb(const cXyz& o, const cXyz& d, const Vec& mn, const Vec& mx, f32& outT) {
    const f32 oa[3] = {o.x, o.y, o.z};
    const f32 da[3] = {d.x, d.y, d.z};
    const f32 la[3] = {mn.x, mn.y, mn.z};
    const f32 ha[3] = {mx.x, mx.y, mx.z};

    f32 tmin = -FLT_MAX;
    f32 tmax = FLT_MAX;
    for (int i = 0; i < 3; ++i) {
        if (std::fabs(da[i]) < 1.0e-8f) {
            if (oa[i] < la[i] || oa[i] > ha[i]) {
                return false;  // parallel and outside the slab
            }
            continue;
        }
        const f32 inv = 1.0f / da[i];
        f32 t1 = (la[i] - oa[i]) * inv;
        f32 t2 = (ha[i] - oa[i]) * inv;
        if (t1 > t2) {
            std::swap(t1, t2);
        }
        if (t1 > tmin) {
            tmin = t1;
        }
        if (t2 < tmax) {
            tmax = t2;
        }
        if (tmin > tmax) {
            return false;
        }
    }
    if (tmax < 0.0f) {
        return false;  // box entirely behind the ray
    }
    f32 t = tmin;
    if (t < 0.0f) {
        t = tmax;  // origin inside the box
    }
    if (t < 0.0f) {
        return false;
    }
    outT = t;
    return true;
}

// Ray vs oriented box: transform the world ray into the box's local space by
// the inverse of mtx, then a local AABB test. The ray parameter t is preserved
// by the affine map, so the returned t is still the world distance.
bool ray_obb(const cXyz& o, const cXyz& d, const Vec& mn, const Vec& mx, MtxP mtx, f32& outT) {
    cXyz lo = o;
    cXyz ld = d;
    if (mtx != nullptr) {
        Mtx inv;
        if (PSMTXInverse(mtx, inv) == 0) {
            return false;  // singular
        }
        lo.set(inv[0][0] * o.x + inv[0][1] * o.y + inv[0][2] * o.z + inv[0][3],
               inv[1][0] * o.x + inv[1][1] * o.y + inv[1][2] * o.z + inv[1][3],
               inv[2][0] * o.x + inv[2][1] * o.y + inv[2][2] * o.z + inv[2][3]);
        ld.set(inv[0][0] * d.x + inv[0][1] * d.y + inv[0][2] * d.z,
               inv[1][0] * d.x + inv[1][1] * d.y + inv[1][2] * d.z,
               inv[2][0] * d.x + inv[2][1] * d.y + inv[2][2] * d.z);
    }
    return ray_aabb(lo, ld, mn, mx, outT);
}

// Ray vs sphere; d assumed unit (world space).
bool ray_sphere(const cXyz& o, const cXyz& d, const cXyz& c, f32 r, f32& outT) {
    const f32 ox = o.x - c.x;
    const f32 oy = o.y - c.y;
    const f32 oz = o.z - c.z;
    const f32 b = ox * d.x + oy * d.y + oz * d.z;
    const f32 cc = ox * ox + oy * oy + oz * oz - r * r;
    const f32 disc = b * b - cc;
    if (disc < 0.0f) {
        return false;
    }
    const f32 s = std::sqrt(disc);
    f32 t = -b - s;
    if (t < 0.0f) {
        t = -b + s;  // origin inside the sphere
    }
    if (t < 0.0f) {
        return false;
    }
    outT = t;
    return true;
}

// Broad phase: ray vs the actor's loose cull volume (box or sphere). Cheap.
bool ray_hits_cull(const fopAc_ac_c* actor, const cXyz& o, const cXyz& d, f32& outT) {
    const int cullType = fopAcM_GetCullSize(actor);
    MtxP mtx = fopAcM_GetMtx(actor);

    if (fopAcM_CULLSIZE_IS_BOX(cullType)) {
        Vec mn;
        Vec mx;
        if (cullType == fopAc_CULLBOX_CUSTOM_e) {
            mn = actor->cull.box.min;
            mx = actor->cull.box.max;
        } else {
            const cull_box& box = l_cullSizeBox[fopAcM_CULLSIZE_IDX(cullType)];
            mn = box.min;
            mx = box.max;
        }
        return ray_obb(o, d, mn, mx, mtx, outT);
    }

    f32 radius;
    Vec centerLocal;
    if (cullType == fopAc_CULLSPHERE_CUSTOM_e) {
        radius = actor->cull.sphere.radius;
        centerLocal = actor->cull.sphere.center;
    } else {
        const cull_sphere& sph = l_cullSizeSphere[fopAcM_CULLSIZE_Q_IDX(cullType)];
        radius = sph.radius;
        centerLocal = sph.center;
    }
    cXyz centerWorld;
    if (mtx != nullptr) {
        centerWorld.set(
            mtx[0][0] * centerLocal.x + mtx[0][1] * centerLocal.y + mtx[0][2] * centerLocal.z + mtx[0][3],
            mtx[1][0] * centerLocal.x + mtx[1][1] * centerLocal.y + mtx[1][2] * centerLocal.z + mtx[1][3],
            mtx[2][0] * centerLocal.x + mtx[2][1] * centerLocal.y + mtx[2][2] * centerLocal.z + mtx[2][3]);
    } else {
        centerWorld = centerLocal;
    }
    return ray_sphere(o, d, centerWorld, radius, outT);
}

// A joint whose bounding box has zero extent carries no geometry (null/control
// joint) — skip it.
bool joint_box_empty(const Vec& mn, const Vec& mx) {
    return mn.x == mx.x && mn.y == mx.y && mn.z == mx.z;
}

// Corner order matches fopAcM_DrawCullingBox / dDbVw_drawCube8pXlu's strip.
void joint_box_corners(const Vec& mn, const Vec& mx, cXyz out[8]) {
    out[0].set(mn.x, mx.y, mn.z);
    out[1].set(mx.x, mx.y, mn.z);
    out[2].set(mn.x, mx.y, mx.z);
    out[3].set(mx.x, mx.y, mx.z);
    out[4].set(mn.x, mn.y, mn.z);
    out[5].set(mx.x, mn.y, mn.z);
    out[6].set(mn.x, mn.y, mx.z);
    out[7].set(mx.x, mn.y, mx.z);
}

}  // namespace

bool build_pick_ray(f32 cursorGameX, f32 cursorGameY, cXyz& outOrigin, cXyz& outDir) {
    if (dComIfGd_getView() == nullptr) {
        return false;
    }
    Mtx44* pv = dComIfGd_getProjViewMtx();
    if (pv == nullptr) {
        return false;
    }

    f32 invPV[4][4];
    if (!invert4x4(*pv, invPV)) {
        return false;
    }

    const f32 minX = mDoGph_gInf_c::getMinXF();
    const f32 minY = mDoGph_gInf_c::getMinYF();
    const f32 width = mDoGph_gInf_c::getWidthF();
    const f32 height = mDoGph_gInf_c::getHeightF();
    if (width <= 1.0f || height <= 1.0f) {
        return false;
    }

    // Cursor game-screen → NDC. Matches mDoLib_project's forward mapping,
    // including its mirror-mode x flip and its y flip (dst.y uses -calcFloat).
    const f32 tx = (cursorGameX - minX) / width;
    const f32 ty = (cursorGameY - minY) / height;
    f32 ndcX = 2.0f * tx - 1.0f;
    if (dusk::getSettings().game.enableMirrorMode.getValue()) {
        ndcX = 1.0f - 2.0f * tx;
    }
    const f32 ndcY = 1.0f - 2.0f * ty;

    cXyz nearW;
    cXyz farW;
    unproject(invPV, ndcX, ndcY, -1.0f, nearW);
    unproject(invPV, ndcX, ndcY, 1.0f, farW);

    const f32 dx = farW.x - nearW.x;
    const f32 dy = farW.y - nearW.y;
    const f32 dz = farW.z - nearW.z;
    const f32 len = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (len < 1.0e-4f) {
        return false;
    }
    const f32 inv = 1.0f / len;
    outDir.set(dx * inv, dy * inv, dz * inv);
    outOrigin = nearW;
    return true;
}

bool ray_hits_actor(const fopAc_ac_c* actor, const cXyz& origin, const cXyz& dir, f32& outT) {
    if (actor == nullptr) {
        return false;
    }

    // Broad phase: reject via the loose cull volume before touching joints.
    f32 broadT = FLT_MAX;
    if (!ray_hits_cull(actor, origin, dir, broadT)) {
        return false;
    }

    // Narrow phase: per-joint bounding boxes (hug the mesh). Broad-phase gate
    // keeps this to the 1-3 actors under the cursor.
    J3DModel* model = actor->model;
    J3DModelData* md = model != nullptr ? model->getModelData() : nullptr;
    if (md != nullptr && md->getJointNum() > 0) {
        const u16 n = md->getJointNum();
        f32 best = FLT_MAX;
        bool hit = false;
        bool anyTestable = false;
        for (u16 i = 0; i < n; ++i) {
            J3DJoint* jnt = md->getJointNodePointer(i);
            if (jnt == nullptr) {
                continue;
            }
            const Vec mn = *jnt->getMin();
            const Vec mx = *jnt->getMax();
            if (joint_box_empty(mn, mx)) {
                continue;
            }
            anyTestable = true;
            f32 t = FLT_MAX;
            if (ray_obb(origin, dir, mn, mx, model->getAnmMtx(i), t) && t < best) {
                best = t;
                hit = true;
            }
        }
        if (hit) {
            outT = best;
            return true;
        }
        if (anyTestable) {
            return false;  // over the loose cull box but not the actual actor
        }
        // No usable joint boxes → fall through to the broad volume.
    }

    outT = broadT;
    return true;
}

void draw_actor_volume_highlight(const fopAc_ac_c* actor, u8 r, u8 g, u8 b, u8 a) {
    const GXColor color = {r, g, b, a};
    J3DModel* model = actor != nullptr ? actor->model : nullptr;
    J3DModelData* md = model != nullptr ? model->getModelData() : nullptr;
    if (md != nullptr && md->getJointNum() > 0) {
        const u16 n = md->getJointNum();
        bool drew = false;
        for (u16 i = 0; i < n; ++i) {
            J3DJoint* jnt = md->getJointNodePointer(i);
            if (jnt == nullptr) {
                continue;
            }
            const Vec mn = *jnt->getMin();
            const Vec mx = *jnt->getMax();
            if (joint_box_empty(mn, mx)) {
                continue;
            }
            cXyz corners[8];
            joint_box_corners(mn, mx, corners);
            MtxP jm = model->getAnmMtx(i);
            if (jm != nullptr) {
                cMtx_multVecArray(jm, corners, corners, 8);
            }
            dDbVw_drawCube8pXlu(corners, color);
            drew = true;
        }
        if (drew) {
            return;
        }
    }
    // Fallback: no model / no joint boxes → the loose cull box.
    if (actor != nullptr) {
        fopAcM_DrawCullingBox(actor, color);
    }
}

}  // namespace dusk::leveledit

#else

namespace dusk::leveledit {

bool build_pick_ray(f32, f32, cXyz&, cXyz&) {
    return false;
}
bool ray_hits_actor(const fopAc_ac_c*, const cXyz&, const cXyz&, f32&) {
    return false;
}
void draw_actor_volume_highlight(const fopAc_ac_c*, u8, u8, u8, u8) {}

}  // namespace dusk::leveledit

#endif
