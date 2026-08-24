#pragma once

// ============================================
// NEW CODE — ALBW Port
// Live tuning for the Armogohma phase-3 laser HIT CAPSULE. Bound to sliders in the
// imgui "Collision View" panel (ImGuiMenuTools.cpp) so the capsule can be dialed in
// while watching its blue wireframe, no rebuild. Read each frame by d_a_b_gm.cpp's
// beam block. Defaults live in d_a_b_gm.cpp (the kAlbwArmoBeam* constants).
// ============================================
namespace dusk {

struct ArmoBeamTune {
    float radius;       // capsule radius: vertical reach (covers Link's body) + lateral bite
    float rise;         // pill-axis height above the boss floor (wall centre height)
    float forward;      // beam ground-track length -> capsule end distance from the eye
    float originRaise;  // beam source lift above the eye joint (visual origin)
};

extern ArmoBeamTune g_armoBeamTune;

}  // namespace dusk
