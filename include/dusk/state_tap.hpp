#pragma once
// ============================================================
// §P2 action-state tap (Foundry, bus §224 — History's acceptance-gate request).
// Drop-in for ported donor actors (pig first): call duskStateTap at every
// action/mode WRITE site (the transition points the donor statemap cites) and
// the differ gates the port against the donor law.
//   [DuskLog] §P2 state t=<ms> tag=<actor> act=<action> mode=<mode>
// Toggle DUSK_STATE_TAP=1 (default OFF; low volume — transitions only, but
// capture-session discipline still applies).
// Verify: tools/foundry/state_gate.py <log> <donor-statemap.md> <out.md>
// ============================================================
#include <chrono>
#include <cstdlib>
#include "dusk/logging.h"

inline bool duskStateTapEnv() {
    static int s_on = -1;
    if (s_on < 0) {
        const char* v = std::getenv("DUSK_STATE_TAP");
        s_on = (v != NULL && v[0] != '\0' && !(v[0] == '0' && v[1] == '\0')) ? 1 : 0;
    }
    return s_on == 1;
}

// §238 NPC variant: WW NPC states are member-function pointers, not ints —
// log the ACTION-METHOD NAME at each set_action site; state_gate --npc checks
// it against the statemap's method roster.
inline void duskStateTapS(const char* i_tag, const char* i_method) {
    if (!duskStateTapEnv()) {
        return;
    }
    static const std::chrono::steady_clock::time_point s_t0s =
        std::chrono::steady_clock::now();
    const long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - s_t0s)
                             .count();
    DuskLog.info("[DuskLog] §P2 staten t={} tag={} fn={}", ms, i_tag, i_method);
}

inline void duskStateTap(const char* i_tag, int i_action, int i_mode) {
    if (!duskStateTapEnv()) {
        return;
    }
    static const std::chrono::steady_clock::time_point s_t0 =
        std::chrono::steady_clock::now();
    const long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - s_t0)
                             .count();
    DuskLog.info("[DuskLog] §P2 state t={} tag={} act={} mode={}", ms, i_tag,
                 i_action, i_mode);
}
