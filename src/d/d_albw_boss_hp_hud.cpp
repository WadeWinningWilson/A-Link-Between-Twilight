/**
 * d_albw_boss_hp_hud.cpp
 * ALBW Port — Boss health bar HUD.
 *
 * Bottom-centre red health bar with the boss name above it (matching the
 * approved reference). This file is DRAW-ONLY: it consumes a normalized
 * fillRatio (0..1) from the boss query layer and never reads health, hit
 * counters, or scalers itself. Geometry comes from computeLayout(); fill width
 * is layout.barW * fillRatio. All HP/scaling/phase logic (Boss HP scaler, Boss
 * Refinement, the two-phase composite) lives in the query layer, so it is not
 * duplicated here.
 *
 * Bar is built from the resident solid-block texture (tt_block8x8, msgres05)
 * drawn as three tinted rects (border / empty track / red fill); the name is
 * a J2DTextBox in the message font with a black outline.
 *
 * Prototype scope: Armogohma — phase 1 body (B_GM) then phase 2 core (E_GM TYPE_GOMA).
 * Visibility/query: dAlbwBoss_armogohmaQueryHealthBar().
 */

#if TARGET_PC

#include "d/d_albw_boss_hp_hud.h"
#include "d/d_albw_boss.h"
#include "d/d_albw_shade_boss.h"
#include "d/d_albw_hp_mult.h"
#include "d/d_com_inf_game.h"
#include "dusk/settings.h"
#include "dusk/frame_interpolation.h"
#include "f_op/f_op_actor.h"
#include <cstdlib>
#include <cstring>
#include "f_op/f_op_actor_mng.h"
#include "f_pc/f_pc_name.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_graphic.h"
#include "JSystem/J2DGraph/J2DGrafContext.h"
#include "JSystem/J2DGraph/J2DOrthoGraph.h"
#include "JSystem/J2DGraph/J2DPicture.h"
#include "JSystem/J2DGraph/J2DTextBox.h"
#include "JSystem/JUtility/TColor.h"
#include "JSystem/JKernel/JKRArchive.h"
#include "JSystem/JKernel/JKRHeap.h"

// ============================================
// NEW CODE — ALBW Port
// ============================================

// Bar geometry as fractions of the 2D ortho extent. Tunable.
// Phase 1 values per docs/albw-boss-hp-hud-tuning-brief.md (LoP-style: thin,
// wide, low, with a small attached name).
static constexpr f32 kBarWidthFrac  = 0.58f;   // bar width / screen width
static constexpr f32 kBarHeightFrac = 0.016f;  // bar height / screen height (thinner)
static constexpr f32 kBarTopFrac    = 0.908f;  // bar top Y / screen height (sit lower)
static constexpr f32 kBarBorder     = 1.0f;    // border thickness (ortho px)
static constexpr f32 kNameFontFrac  = 0.028f;  // name font height / screen height (~30px @1080p)
static constexpr f32 kNameGap       = 3.0f;    // gap between name and bar (attached, fixed px)
static constexpr f32 kNameOutline   = 1.0f;    // name outline offset (thinner stroke)

// ============================================
// NEW CODE — ALBW Port
// Intro-card styling for the boss name (reversible). When 1, the bar name uses
// the boss-intro title font (mDoExt_getRubyFont, same as zelda_boss_name.blo /
// dMsgScrnBoss_c) in the cream/gold card color. Flip to 0 to revert to the
// plain white "Armogohma" message-font label — no other changes needed.
// Applies to every boss bar (Armogohma + Hero's Shade), since they share the
// one J2DTextBox.
// ============================================
#define D_ALBW_BOSS_BAR_INTRO_STYLE 1

#if D_ALBW_BOSS_BAR_INTRO_STYLE
static const u32 kNameColorTop = 0xF6E8B0FF;  // cream — intro-card gold, glyph top
static const u32 kNameColorBot = 0xE0B84AFF;  // gold  — intro-card gradient, glyph bottom
#endif
// ============================================
// NEW CODE ENDS HERE
// ============================================

// Solid-block texture (resident in msgres05.arc) reused for every rect.
static const char* BLOCK_BTI = "tt_block8x8.bti";

static J2DPicture* sBlock = NULL;
static J2DTextBox* sName = NULL;

// ---------------------------------------------------------------------------
// Boss name for a profile (prototype: Armogohma only).
// ---------------------------------------------------------------------------
static const char* bossDisplayName(s16 profName) {
    switch (profName) {
    case fpcNm_B_BQ_e:
        return "Twilit Parasite DIABABA";
    case fpcNm_B_GM_e:
#if D_ALBW_BOSS_BAR_INTRO_STYLE
        return "Twilit Arachnid Armogohma";
#else
        return "Armogohma";
#endif
    case fpcNm_NPC_KN_e:
        return "Hero of Time";  // the combat Hero's Shade
    case fpcNm_B_ZANT_e:
        return "Usurper King Zant";
    default:
        return NULL;
    }
}

static bool ensureResources() {
    if (sBlock == NULL) {
        JKRArchive* arc = dComIfGp_getMsgArchive(5);
        if (arc == NULL) {
            return false;
        }
        ResTIMG* timg = static_cast<ResTIMG*>(arc->getResource('TIMG', BLOCK_BTI));
        if (timg == NULL) {
            return false;
        }
        sBlock = JKR_NEW J2DPicture(timg);
        if (sBlock == NULL) {
            return false;
        }
    }
    if (sName == NULL) {
        sName = JKR_NEW J2DTextBox();
        if (sName == NULL) {
            return false;
        }
#if D_ALBW_BOSS_BAR_INTRO_STYLE
        sName->setFont(mDoExt_getRubyFont());  // boss-intro title font
#else
        sName->setFont(mDoExt_getMesgFont());
#endif
    }
    return true;
}

// Draw a tinted rectangle using the white block texture. Alpha lets the empty
// track be translucent (LoP "see the floor through the empty bar").
static void drawRect(f32 x, f32 y, f32 w, f32 h, u8 r, u8 g, u8 b, u8 a = 255) {
    if (w <= 0.0f || h <= 0.0f) {
        return;
    }
    sBlock->setBlackWhite(JUtility::TColor(0, 0, 0, 0), JUtility::TColor(r, g, b, a));
    sBlock->setAlpha(a);
    sBlock->draw(x, y, w, h, false, false, false);
}

// ============================================
// NEW CODE — ALBW Port
// Layout is derived once per frame from screen-size fractions, so every tuning-
// brief knob (bar width/height/position, name size/gap) lives here and never
// touches meter or boss logic. The fill is layout.barW * fillRatio — no HP math.
// ============================================
struct BossHpHudLayout {
    f32 barX, barY, barW, barH;
    f32 nameY, fontSz;
    f32 border;
};

static BossHpHudLayout computeLayout(f32 minX, f32 minY, f32 scrW, f32 scrH) {
    BossHpHudLayout L;
    L.barW   = scrW * kBarWidthFrac;
    L.barH   = scrH * kBarHeightFrac;
    L.barX   = minX + (scrW - L.barW) * 0.5f;
    L.barY   = minY + scrH * kBarTopFrac;
    L.border = kBarBorder;
    L.fontSz = scrH * kNameFontFrac;
    L.nameY  = L.barY - kNameGap - L.fontSz;
    return L;
}
// ============================================
// NEW CODE ENDS HERE
// ============================================

// ---------------------------------------------------------------------------
// dAlbwBossHpHud_draw — public draw tick.
// ---------------------------------------------------------------------------
void dAlbwBossHpHud_draw() {
    if (!dusk::getSettings().game.bossHealthBars.getValue()) {
        return;
    }

    // Pick the active boss. Armogohma (composite) first, then Diababa, then the
    // Hero's Shade. Arenas never coexist; only {current,max,name}/fillRatio
    // selection differs — bar render is shared.
    // ============================================
    // MODIFIED CODE — ALBW Port
    // Fill comes from a normalized fillRatio, not raw {current,max}. Armogohma's
    // query already composites both phases into fillRatio; Diababa / Shade
    // (single pool) derive it as current/max. Early-out is fillRatio <= 0 so a
    // half-full composite bar (e.g. phase-1 empty at 0.5) is never hidden.
    // ============================================
    f32 fillRatio = 0.0f;
    const char* name = NULL;

    dAlbwBoss_ArmogohmaBarState bar = {};
    if (dAlbwBoss_armogohmaQueryHealthBar(&bar) && bar.visible && bar.fillRatio > 0.0f) {
        fillRatio = bar.fillRatio;
        name = bossDisplayName(fpcNm_B_GM_e);
    } else {
        int sCur = 0;
        int sMax = 0;
        if (dAlbwBoss_diababaQueryHealthBar(&sCur, &sMax) && sMax > 0 && sCur > 0) {
            fillRatio = static_cast<f32>(sCur) / static_cast<f32>(sMax);
            name = bossDisplayName(fpcNm_B_BQ_e);
        } else if (dShadeBoss_queryHealthBar(&sCur, &sMax) && sMax > 0 && sCur > 0) {
            fillRatio = static_cast<f32>(sCur) / static_cast<f32>(sMax);
            name = bossDisplayName(fpcNm_NPC_KN_e);
        } else if (dAlbwBoss_zantQueryHealthBar(&sCur, &sMax) && sMax > 0 && sCur > 0) {
            // ============================================
            // NEW CODE — ALBW Port (Zant per-phase bar; see dAlbwBoss_zantQueryHealthBar)
            // ============================================
            fillRatio = static_cast<f32>(sCur) / static_cast<f32>(sMax);
            name = bossDisplayName(fpcNm_B_ZANT_e);
        }
    }

    if (name == NULL || fillRatio <= 0.0f || !ensureResources()) {
        return;
    }
    // ============================================
    // MODIFIED CODE ENDS HERE
    // ============================================

    J2DGrafContext* gfx = dComIfGp_getCurrentGrafPort();
    if (gfx == NULL) {
        return;
    }

    // Match the footer-text pattern: explicit widescreen ortho so the bar sits
    // bottom-centre at any aspect ratio.
#ifdef TARGET_PC
    dusk::frame_interp::set_ui_tick_pending(true);
#endif
    const f32 minX = mDoGph_gInf_c::getMinXF();
    const f32 minY = mDoGph_gInf_c::getMinYF();
    const f32 scrW = mDoGph_gInf_c::getWidthF();
    const f32 scrH = mDoGph_gInf_c::getHeightF();

    J2DOrthoGraph* ortho = static_cast<J2DOrthoGraph*>(gfx);
    ortho->setOrtho(minX, minY, scrW, scrH, -1.0f, 1.0f);
    ortho->setup2D();
    ortho->setPort();

    // ============================================
    // MODIFIED CODE — ALBW Port
    // All geometry now comes from computeLayout(); the fill is layout.barW *
    // fillRatio (no phase branches, no HP math). Tuning-brief edits touch only
    // computeLayout and the color literals below.
    // ============================================
    const BossHpHudLayout layout = computeLayout(minX, minY, scrW, scrH);

    if (fillRatio < 0.0f) {
        fillRatio = 0.0f;
    } else if (fillRatio > 1.0f) {
        fillRatio = 1.0f;
    }
    const f32 fillW = layout.barW * fillRatio;

    // Border (near-black 1px frame), empty track (translucent charcoal — lost
    // health reads black/empty with the floor faintly visible), deep crimson fill.
    drawRect(layout.barX - layout.border, layout.barY - layout.border,
             layout.barW + layout.border * 2.0f, layout.barH + layout.border * 2.0f,
             32, 32, 36, 255);
    drawRect(layout.barX, layout.barY, layout.barW, layout.barH, 18, 18, 22, 168);
    drawRect(layout.barX, layout.barY, fillW, layout.barH, 176, 8, 8, 255);

    // Boss name, centred above the bar, white with a black outline.
    sName->setFontSize(layout.fontSz, layout.fontSz);
    sName->setString(name);

    sName->setCharColor(0x000000FF);
    sName->setGradColor(0x000000FF);
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            sName->draw(layout.barX + dx * kNameOutline, layout.nameY + dy * kNameOutline,
                        layout.barW, HBIND_CENTER);
        }
    }
#if D_ALBW_BOSS_BAR_INTRO_STYLE
    sName->setCharColor(kNameColorTop);
    sName->setGradColor(kNameColorBot);
#else
    sName->setCharColor(0xFFFFFFFF);
    sName->setGradColor(0xFFFFFFFF);
#endif
    sName->draw(layout.barX, layout.nameY, layout.barW, HBIND_CENTER);
    // ============================================
    // MODIFIED CODE ENDS HERE
    // ============================================
}

// ============================================
// NEW CODE ENDS HERE
// ============================================

#endif // TARGET_PC
