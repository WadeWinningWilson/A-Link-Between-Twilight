// ============================================================================
// phase5_include_probe.cpp - IS PHASE 5 BLOCKED AT THE INCLUDE LEVEL, OR NOT?
//
// A STANDALONE PROBE TU. It is NOT part of the plugin and must never be linked
// into it: its only job is to answer one question with a compiler instead of an
// argument - can a header-ful (typed) plugin TU see the draw/collision surface?
//
// WHY THIS EXISTS. Three instruments in that surface are unreachable BY NAME on
// an optimised build, so five hypotheses died on silence that could not be told
// apart from "the code never ran":
//   - daBg_c::draw                    INLINED-DEAD (registry.cpp:476)
//   - dStage_roomControl_c::setBgW    INLINE in d_stage.h:1222
//   - mDoLib_clipper::changeFar       INLINED - THE HOST ITSELF WARNS IT
//     ...and its sibling mDoLib_clipper::clip is AMBIGUOUS (2 raw entries).
// A TYPED plugin does not HOOK clip - it CALLS it. Going header-ful dissolves
// the observability class rather than working around it.
//
// THE MEASURED STARTING POINT (Integrator): the headers are ALREADY on the
// plugin's include path; adding them raw fails on PREREQUISITES, not
// availability - `d_bg_w.h(226): unknown override specifier`,
// `d_a_bg.h(16): missing ; before *`. Classic not-self-contained headers.
//
// AND THE RECEIVER ALREADY STATES THE FIX. Every game TU opens the same way -
// src/d/actor/d_a_bg.cpp:
//     #include "d/dolzel_rel.h"   // IWYU pragma: keep
//     #include "d/actor/d_a_bg.h"
//     #include "d/d_com_inf_game.h"
// The prerequisite header FIRST, with an explicit IWYU-keep so tooling cannot
// strip it. Nothing is authored here: this is the receiver's own header in the
// receiver's own order.
// ============================================================================
#include "d/dolzel_rel.h"  // IWYU pragma: keep - PREREQUISITE, MUST BE FIRST
#include "d/d_com_inf_game.h"

// The draw/collision surface the header-free plugin cannot observe.
#include "d/actor/d_a_bg.h"
#include "d/d_bg_s.h"
#include "d/d_bg_s_acch.h"
#include "d/d_bg_w.h"
#include "d/d_stage.h"
#include "m_Do/m_Do_lib.h"

// ----------------------------------------------------------------------------
// PROOF THE TYPES ARE USABLE, NOT MERELY PARSEABLE. A header that compiles but
// whose types cannot be named buys nothing, so this asserts the three specific
// capabilities Phase 5 needs. It is compile-time only; nothing runs.
// ----------------------------------------------------------------------------
namespace {

// (1) The room-binding accessor that is INLINE, and therefore unhookable by
//     name. A typed plugin CALLS it instead of hooking it - which is the whole
//     argument, so it must be nameable.
//
//     MEASURED CAVEAT, and it cost this probe its first run: `getBgW` EXISTS IN
//     OUR FORK (`include/d/d_stage.h:1226`) AND NOT ON VANILLA. A typed plugin
//     compiles against the VANILLA headers, so it sees VANILLA's API surface -
//     every fork-added accessor is invisible to it. Going header-ful dissolves
//     the INLINING blindness and introduces an API-DIVERGENCE boundary in its
//     place. That is a far better trade, but it is not a free one, and this is
//     the first measured instance of it.
void probe_room_binding() {
    dBgW_Base* bgw = NULL;
    dStage_roomControl_c::setBgW(0, bgw);  // vanilla HAS this (d_stage.h:1216)
}

// (2) The clipper. `clip` is ambiguous by name and `changeFar` is inlined away;
//     typed code calls both directly.
void probe_clipper() {
    mDoLib_clipper::changeFar(1000000.0f);
    mDoLib_clipper::resetFar();
}

// (3) The BG actor type itself - the thing whose draw is inlined-dead.
void probe_bg_type() {
    const size_t n = sizeof(daBg_c);
    (void)n;
}

}  // namespace
