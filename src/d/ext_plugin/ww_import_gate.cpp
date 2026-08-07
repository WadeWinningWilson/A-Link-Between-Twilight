// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
// ============================================================================
// ROADMAP 19c — load-time import gate. See the header for why it lives here and
// why the resolver is injected rather than called directly.
// ============================================================================
#include "d/ext_plugin/ww_import_gate.h"

#include <cstring>

#if TARGET_PC
#include "dusk/logging.h"
#endif

namespace {

void copy_first(char* dst, size_t cap, const char* src) {
    if (dst[0] != '\0' || src == nullptr) {
        return;  // keep the FIRST offender, not the last
    }
    std::strncpy(dst, src, cap - 1);
    dst[cap - 1] = '\0';
}

}  // namespace

WwGateOutcome dExtWwImportGate_evaluate(const char* const* symbols, int n,
                                        WwGateResolveFn resolve, void* user,
                                        WwGateReport* out_report) {
    WwGateReport rep;
    std::memset(&rep, 0, sizeof(rep));

    if (resolve == nullptr) {
        // Cannot check. UNKNOWN, never clean (№31-C). A gate that reports PASS
        // because it could not run is the failure mode this whole campaign has
        // been paying for.
        rep.outcome = WW_GATE_UNKNOWN_MANIFEST;
        if (out_report != nullptr) {
            *out_report = rep;
        }
        return rep.outcome;
    }

    for (int i = 0; i < n; ++i) {
        const char* sym = symbols != nullptr ? symbols[i] : nullptr;
        if (sym == nullptr || sym[0] == '\0') {
            continue;
        }
        ++rep.checked;
        switch (resolve(sym, user)) {
        case WW_GATE_SYM_OK:
            break;
        case WW_GATE_SYM_MISSING:
            ++rep.missing;
            copy_first(rep.first_missing, sizeof(rep.first_missing), sym);
            break;
        case WW_GATE_SYM_FOLDED:
            ++rep.folded;
            copy_first(rep.first_folded, sizeof(rep.first_folded), sym);
            break;
        case WW_GATE_SYM_NO_MANIFEST:
            ++rep.no_manifest;
            break;
        }
        // NOTE: no `break` out of the loop on failure. C6 -- the refusal is
        // TOTAL. Stopping at the first offender would report one symbol when
        // the truth is eleven, and the operator would fix one and re-run.
    }

    // Ordering matters. "No manifest" outranks everything: without one we did
    // not test the other symbols meaningfully, so claiming MISSING would assert
    // a result we did not measure.
    if (rep.no_manifest > 0) {
        rep.outcome = WW_GATE_UNKNOWN_MANIFEST;
    } else if (rep.missing > 0) {
        rep.outcome = WW_GATE_REFUSE_MISSING;
    } else if (rep.folded > 0) {
        rep.outcome = WW_GATE_REFUSE_FOLDED;
    } else {
        rep.outcome = WW_GATE_PASS;
    }

    if (out_report != nullptr) {
        *out_report = rep;
    }
    return rep.outcome;
}

// ============================================================================
// SELF-TEST — the four controls, run against stubs.
//
// C2 is the one that matters most and is easiest to omit: a gate that refuses
// everything passes C1, C4 and C6 and is useless. Both directions get a case.
// ============================================================================
namespace {

WwGateSymbolState stub_all_ok(const char*, void*) { return WW_GATE_SYM_OK; }
WwGateSymbolState stub_no_manifest(const char*, void*) { return WW_GATE_SYM_NO_MANIFEST; }

WwGateSymbolState stub_one_missing(const char* s, void*) {
    return std::strcmp(s, "cM_ssin") == 0 ? WW_GATE_SYM_MISSING : WW_GATE_SYM_OK;
}

WwGateSymbolState stub_two_missing_one_folded(const char* s, void*) {
    if (std::strcmp(s, "cM_ssin") == 0 || std::strcmp(s, "cM_scos") == 0) {
        return WW_GATE_SYM_MISSING;
    }
    if (std::strcmp(s, "push_back") == 0) {
        return WW_GATE_SYM_FOLDED;
    }
    return WW_GATE_SYM_OK;
}

}  // namespace

int dExtWwImportGate_selftest(void) {
    // Self-guarding: callers should not have to own a `ran` flag, and a
    // control that spams every load teaches people to filter it out.
    static bool s_ran = false;
    static int s_result = 0;
    if (s_ran) {
        return s_result;
    }
    s_ran = true;

    static const char* const kSyms[] = {"setBaseTRMtx", "push_back", "cM_ssin", "cM_scos"};
    const int kN = 4;
    int failures = 0;
    WwGateReport rep;

    struct Case {
        const char* name;
        WwGateResolveFn fn;
        WwGateOutcome expect;
    };
    const Case cases[] = {
        {"C2 clean list PASSES", stub_all_ok, WW_GATE_PASS},
        {"C1 refuses on MISSING", stub_one_missing, WW_GATE_REFUSE_MISSING},
        {"C4 absent manifest -> UNKNOWN", stub_no_manifest, WW_GATE_UNKNOWN_MANIFEST},
        {"C6 refusal is TOTAL", stub_two_missing_one_folded, WW_GATE_REFUSE_MISSING},
    };

    for (const Case& c : cases) {
        const WwGateOutcome got = dExtWwImportGate_evaluate(kSyms, kN, c.fn, nullptr, &rep);
        bool ok = (got == c.expect);
        // C6 additionally demands the count, not just the verdict: a total
        // refusal must have COUNTED every offender, so 2 missing + 1 folded.
        if (std::strcmp(c.name, "C6 refusal is TOTAL") == 0) {
            ok = ok && rep.missing == 2 && rep.folded == 1 && rep.checked == kN;
        }
        if (!ok) {
            ++failures;
        }
#if TARGET_PC
        DuskLog.info("[WwGate] 19c {} — {} (outcome={} checked={} missing={} folded={})",
                     c.name, ok ? "PASS" : "FAIL", (int)got, rep.checked, rep.missing,
                     rep.folded);
#endif
    }

    // A null resolver must also be UNKNOWN, never PASS.
    if (dExtWwImportGate_evaluate(kSyms, kN, nullptr, nullptr, &rep) != WW_GATE_UNKNOWN_MANIFEST) {
        ++failures;
    }

#if TARGET_PC
    DuskLog.info("[WwGate] 19c self-test: {} — C5 (resolve-then-CALL record kind) is an SDK "
                 "gap and is NOT covered here", failures == 0 ? "ALL PASS" : "FAILURES");
#endif
    s_result = failures;
    return failures;
}
