// ============================================================
// §417 WW CUMULUS DECODE — donor-equivalent bodies of the two /* Nonmatching */
// stubs in WW DP d_kankyo_rain.cpp:
//   vrkumo_move   (release 0x918 @ 800937BC-800940D4; debug map 0x904 @ 800cd828)
//   drawVrkumo    (release 0xE3C @ 8009AB88-8009B9C4; debug map 0x1238 @ 800d7090)
// EVIDENCE TIERS (user ruling: Winditor/binary = law, noclip = reference):
//   [LAW] WW DP structs: VRKUMO_EFF (d_kankyo_wether.h:244 — 0x00 status,
//         0x04 pos, 0x1C height, 0x20 alpha, 0x24 distFalloff, 0x28 speed;
//         0x10-0x1C unreferenced by the behavior — see [D4]) and the intact
//         wether-side driver (wether_move_vrkumo :763 — 50/100-cloud counts,
//         strength blends, status machine) constrain every field use below.
//   [LAW] Winditor EntityTemplates.cs:2346 Virt palette byte layout:
//         …4 unknown RGBA… HorizonCloudColor RGBA, CenterCloudColor RGBA,
//         SkyColor/FalseSeaColor/HorizonColor RGB — CenterCloudColor is
//         AUTHORED STAGE DATA (the field TP dropped as mVrkumoCenterColor).
//   [LAW] debug map frameworkD.map: symbol sizes + drawVrkumokage (cloud
//         SHADOW draw, 0xa18) exists as a third dropped function — ledgered,
//         not decoded here.
//   [REF] noclip d_kankyo_wether.ts (VRKUMO_EFF ctor :565, vrkumo_move :2515,
//         packet draw :618) — jstpierre's binary-derived reconstruction;
//         every constant below is carried from it verbatim.
// DECODE SEAMS:
//   [D1] noclip's packet field bounceAnimTimer (+200/frame, used as
//        sin(texIdx + 0.0001*t)) has no slot in WW DP's packet struct —
//        kept as a TU-static (donor storage location unresolved; behavior
//        identical).
//   [D2] donor drawVrkumo signature (Mtx, GXColor&, u8**) receives ONE color
//        (the packet's mColorTmp = horizon cloud color); the center color is
//        read from env light inside the body — the receiver carries it as
//        mWwVrkumoCenterColor (PC append; fed by Housing's §407 palette
//        conversion from the Winditor-lawed Virt slot).
//   [D3] noclip draws camera-pos-relative dome quads with radius
//        (farPlane - 10000); the donor's Mtx arg is the view matrix for the
//        same purpose. The receiver body uses the direct camera-pos form
//        (equivalent geometry; receiver wether draws already do this).
//   [D4] VRKUMO_EFF 0x10-0x1C (12 bytes) is untouched by the reconstructed
//        behavior — donor-padding or a field only drawVrkumokage uses;
//        left untouched here.
// Include from the sky TU (Housing §418): behavior swap point is the two
// functions behind dKyWw_isSkyHost().
// ============================================================

// ---------- per-cloud INIT (noclip VRKUMO_EFF ctor — donor spawn distribution)
static void dKyWwVrkumo_initEff(VRKUMO_EFF* kumo) {
    const f32 angle = cM_rndF(2.0f * M_PI);
    f32 dist = cM_rndF(18000.0f);
    if (dist > 15000.0f) {
        dist = 14000.0f + cM_rndF(1000.0f);
    }
    kumo->mPosition.x = dist * sinf(angle);
    kumo->mPosition.z = dist * cosf(angle);
    kumo->mAlpha = 0.0f;
    kumo->mSpeed = 0.5f + cM_rndF(4.0f);
    kumo->mHeight = 0.3f * cM_rndFX(0.3f);
}

// ---------- vrkumo_move (donor 0x918 @ 800937BC)
static void dKyWwVrkumo_move() {
    dKankyo_vrkumo_Packet* pkt = g_env_light.mpVrkumoPacket;
    if (pkt == NULL) {
        return;
    }

    // §416 donor wind (Housing): vec + power.
    cXyz wind = dKyw_get_wind_vecpow();  // [R2] receiver accessor (§416-fed)

    f32 skyboxOffsY;
    if (strcmp(dComIfGp_getStartStageName(), "M_DragB") == 0) {
        wind.set(-1.0f, 0.0f, 0.0f);
        skyboxOffsY = 300.0f;
    } else {
        skyboxOffsY = 1000.0f + g_env_light.mVrkumoStrength * -500.0f;
    }
    // (donor also biases by -0.09 * (cameraY - FILI skyboxY); noclip carries it
    // disabled — kept off pending an in-world check against tall islands.)

    for (int i = 0; i < 100; i++) {
        VRKUMO_EFF* kumo = &pkt->mVrkumoEff[i]  /* [R1] receiver packet field name */;

        // [R3] the donor's case-0 spawn gate: wether_move_vrkumo zeroes every
        // mStatus at packet init, and the first move scatters each cloud once
        // (initEff) before the lifecycle runs. Without this every cloud sits
        // at the disc center where the overhead fade correctly zeroes it --
        // P11's st=0/spd=0/a=0 signature (log 155735).
        if (kumo->mStatus == 0) {
            dKyWwVrkumo_initEff(kumo);
            kumo->mStatus = 1;
        }

        f32 distXZ = sqrtf(kumo->mPosition.x * kumo->mPosition.x +
                           kumo->mPosition.z * kumo->mPosition.z);

        // Edge recycle: just past the rim → mirror through center (a cloud
        // leaves downwind, an identical one enters upwind); far past → rescatter.
        if (distXZ > 15000.0f) {
            if (distXZ <= 15100.0f) {
                kumo->mPosition.x *= -1.0f;
                kumo->mPosition.z *= -1.0f;
            } else {
                kumo->mPosition.x = cM_rndFX(14000.0f);
                kumo->mPosition.z = cM_rndFX(14000.0f);
                distXZ = sqrtf(kumo->mPosition.x * kumo->mPosition.x +
                               kumo->mPosition.z * kumo->mPosition.z);
            }
            kumo->mAlpha = 0.0f;
        }

        // Wind drift. Visible clouds move by falloff*speed; invisible ones
        // rush (index-staggered) so respawns reach their slot quickly.
        const f32 strengthVelocity = 4.0f + g_env_light.mVrkumoStrength * 4.3f;
        f32 velocity;
        if (kumo->mAlpha > 0.0f) {
            velocity = strengthVelocity * kumo->mDistFalloff * kumo->mSpeed;
        } else {
            velocity = strengthVelocity + (i / 1000.0f) * strengthVelocity;
        }
        kumo->mPosition.x += wind.x * velocity;
        kumo->mPosition.y += wind.y * velocity;
        kumo->mPosition.z += wind.z * velocity;

        const f32 distXZ01 = distXZ / 15000.0f > 1.0f ? 1.0f : distXZ / 15000.0f;

        // Height: index-laddered base + skybox offset + a center-weighted bulge
        // (cubic), lowered as storm strength rises.
        const f32 strengthY = 3000.0f + g_env_light.mVrkumoStrength * -1000.0f;
        const f32 centerAmtCubic = 1.0f - (distXZ01 * distXZ01 * distXZ01);
        kumo->mPosition.y = (500.0f * (i / 100.0f)) + skyboxOffsY + strengthY * centerAmtCubic;

        // Size/color falloff toward the rim (sextic).
        const f32 d3 = distXZ01 * distXZ01 * distXZ01;
        kumo->mDistFalloff = 1.0f - d3 * d3;

        // Alpha lifecycle.
        f32 alphaTarget;
        f32 alphaMaxVel = 1.0f;
        if (strcmp(dComIfGp_getStartStageName(), "M_DragB") == 0) {
            kumo->mAlpha = 1.0f;
            alphaTarget = 1.0f;
        } else if (i < g_env_light.mVrkumoCount) {
            alphaMaxVel = 0.1f;
            if (kumo->mDistFalloff >= 0.05f && kumo->mDistFalloff < 0.2f) {
                alphaTarget = (kumo->mDistFalloff - 0.05f) / 0.15f;   // rim fade-in band
            } else if (kumo->mDistFalloff < 0.2f) {
                alphaTarget = 0.0f;                                    // beyond the rim
            } else {
                alphaTarget = 1.0f + g_env_light.mVrkumoStrength * -0.55f;
            }
        } else {
            alphaTarget = 0.0f;      // over-count clouds bleed out slowly
            alphaMaxVel = 0.005f;
        }

        // Overhead fade: as a cloud nears dead-center the sphere projection
        // would give the illusion away — fade across centerAmtCubic 0.88→0.98.
        f32 overheadFade = (0.98f - centerAmtCubic) / (0.98f - 0.88f);
        if (overheadFade < 0.0f) overheadFade = 0.0f;
        if (overheadFade > 1.0f) overheadFade = 1.0f;
        alphaTarget *= overheadFade;

        cLib_addCalc(&kumo->mAlpha, alphaTarget, 0.2f, alphaMaxVel, 0.01f);
    }

    s_wwVrkumoBounceTimer += 200.0f;   // [D1]
}

// ---------- drawVrkumo (donor 0xE3C @ 8009AB88) — geometry per cloud per layer
// Donor call shape: dKankyo_vrkumo_Packet::draw() →
//   drawVrkumo(j3dSys.getViewMtx(), mColorTmp, &mpCloudTx1);
// clr = horizon cloud color; center color from env light ([D2]); three texture
// layers drawn back-to-front (2→0); per-cloud quad on the sky dome ([D3]).
static void dKyWwVrkumo_drawLayerQuad(const VRKUMO_EFF* kumo, int i, int textureIdx,
                                      f32 strength, f32 domeRadius, const cXyz& camPos,
                                      GXColor clr /* pre-lerped, alpha set */) {
    const f32 size = kumo->mDistFalloff *
                     (1.0f - cubef(((textureIdx + i) & 0x0F) / 16.0f)) *
                     (0.45f + strength * 0.55f);
    const f32 bounceAnim = sinf(textureIdx + 0.0001f * s_wwVrkumoBounceTimer);
    const f32 sizeAnim = size + (0.06f * size) * bounceAnim * kumo->mDistFalloff;
    const f32 height = sizeAnim + sizeAnim * kumo->mHeight;
    const f32 m0 = 0.15f * sizeAnim;
    const f32 m1 = 0.65f * sizeAnim;

    // Layer scatter: 4 arrangement classes by (i & 3); layer 0 never offsets.
    f32 polarOffs = 0.0f, azimuthalOffs = 0.0f;
    if (textureIdx != 0) {
        switch (i & 3) {
        case 0:
            if (textureIdx == 2) { polarOffs = m1;  azimuthalOffs = m0; }
            break;
        case 1:
            if (textureIdx == 1) { polarOffs = -m0; azimuthalOffs = m0; }
            else if (textureIdx == 2) { polarOffs = -m1; azimuthalOffs = m1; }
            break;
        case 2:
            if (textureIdx == 1) { polarOffs = m1;  azimuthalOffs = -m1; }
            else if (textureIdx == 2) { polarOffs = m0; azimuthalOffs = -m1; }
            break;
        case 3:
            if (textureIdx == 1) { polarOffs = -m1; }
            else if (textureIdx == 2) { polarOffs = -m0; azimuthalOffs = m0; }
            break;
        }
    }

    const f32 distXZ = sqrtf(kumo->mPosition.x * kumo->mPosition.x +
                             kumo->mPosition.z * kumo->mPosition.z);
    const f32 polarY1 = atan2f(kumo->mPosition.y, distXZ) + polarOffs;

    // Elevation-cubed widening — THE anti-balloon term. TP's 6-cloud tuning
    // runs the same shape but WW's 50-100 count + WW polar ladder needs the
    // donor min() clamps below or overhead quads balloon (observed §414).
    f32 np = polarY1 / 1.9f;
    if (np > 1.0f) np = 1.0f;
    const f32 normalPitch = np * np * np;

    const f32 azimuthal = atan2f(kumo->mPosition.x, kumo->mPosition.z) + azimuthalOffs;
    const f32 azimuthalOffsY0 = 0.6f * sizeAnim * (1.0f + 16.0f * normalPitch);
    const f32 azimuthalOffsY1 = 0.6f * sizeAnim * (1.0f + 2.0f * normalPitch);

    f32 polarY0 = polarY1 + 0.9f * height * (1.0f + -4.0f * normalPitch);
    if (polarY0 > 1.21f) polarY0 = 1.21f;

    // Quad corners on the dome (top-left, top-right, bottom-right, bottom-left),
    // UVs (0,0)(1,0)(1,1)(0,1); world = dome*dir + camera pos.
    struct { f32 polar, azi; s16 s, t; } corners[4] = {
        { polarY0, azimuthal + azimuthalOffsY0, 0, 0 },
        { polarY0, azimuthal - azimuthalOffsY0, 1, 0 },
        { polarY1, azimuthal - azimuthalOffsY1, 1, 1 },
        { polarY1, azimuthal + azimuthalOffsY1, 0, 1 },
    };
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    for (int c = 0; c < 4; c++) {
        const f32 x = cosf(corners[c].polar) * sinf(corners[c].azi);
        const f32 y = sinf(corners[c].polar);
        const f32 z = cosf(corners[c].polar) * cosf(corners[c].azi);
        GXPosition3f32(camPos.x + x * domeRadius, camPos.y + y * domeRadius,
                       camPos.z + z * domeRadius);
        GXColor4u8(clr.r, clr.g, clr.b, clr.a);
        GXTexCoord2s16(corners[c].s, corners[c].t);
    }
    GXEnd();
}
// Layer loop (the drawVrkumo body proper): for textureIdx 2→0: load
// cloudTx[textureIdx]; for each of 100 clouds with alpha > 0.000001:
//   clr.rgb = lerp(vrKumoCol, mWwVrkumoCenterColor, kumo->mDistFalloff) [D2]
//   clr.a   = kumo->mAlpha * 255
//   dKyWwVrkumo_drawLayerQuad(...)
// TEV per donor material: C1=0 (black), stage0: (C1, RASC, TEXC, ZERO) ADD →
// PREV; alpha (ZERO, RASA, TEXA, ZERO); blend SRCALPHA/INVSRCALPHA; Z LEQUAL
// no-write; dome radius = farPlane - 10000.
