#ifndef D_EXT_PLUGIN_WW_IMPORT_GATE_H
#define D_EXT_PLUGIN_WW_IMPORT_GATE_H

// ============================================================================
// ROADMAP 19c — LOAD-TIME IMPORT GATE.
//
// THE GAP IT CLOSES (posture §8.1). `hook_install` already returns
// MOD_UNAVAILABLE when a target fails to resolve, so an unresolved hook does not
// crash. But that refusal is PER-CALL-SITE and LAZY: a mod loads clean, runs for
// an hour, and a hook that was never installed simply means a behaviour silently
// never happens. The service layer negotiates up front (struct_size / minor
// checks); symbol bindings have no equivalent. This gives them one.
//
// The gate validates the ENTIRE import list at load and refuses to initialise
// on MISSING or FOLDED, mirroring the posture the service layer already takes.
//
// ---------------------------------------------------------------------------
// WHY IT LIVES HERE AND NOT IN src/dusk/mods/
//
// The gate is OURS. `src/dusk/mods/` is dusklight's own mod subsystem, and
// placing WW-layer code inside it is precisely the crossing
// docs/WW Linked/ww-declared-crossings.md exists to police -- default deny, and
// a crossing that is neither gated nor declared is the defect. Foundry's
// acceptance probes glob `src/dusk/mods/**/*import*gate*`; that path is wrong
// for this reason and the probes want repointing here.
//
// ---------------------------------------------------------------------------
// SHAPE: a PURE DECISION over an injected resolver.
//
// The gate does not call the SDK directly. It takes a resolver callback, so the
// same decision logic runs against the real HookService at load AND against a
// stub in the self-test. A gate that can only be exercised by shipping it is a
// gate nobody can control -- and every instrument this campaign has trusted
// without a negative control has been wrong at least once.
// ============================================================================

#ifdef __cplusplus
extern "C" {
#endif

typedef enum WwGateOutcome {
    WW_GATE_PASS = 0,          // every import resolved, none folded
    WW_GATE_REFUSE_MISSING,    // at least one import did not resolve
    WW_GATE_REFUSE_FOLDED,     // at least one import shares an address (ICF)
    WW_GATE_UNKNOWN_MANIFEST,  // no/stale symbol manifest -- UNKNOWN, never clean
} WwGateOutcome;

// Per-symbol result the caller's resolver reports back.
typedef enum WwGateSymbolState {
    WW_GATE_SYM_OK = 0,
    WW_GATE_SYM_MISSING,       // MOD_UNAVAILABLE
    WW_GATE_SYM_FOLDED,        // HOOK_SYMBOL_MULTI_NAME without DISPLAY
    WW_GATE_SYM_NO_MANIFEST,   // MOD_UNSUPPORTED
} WwGateSymbolState;

// Resolver injection point. `user` is passed through untouched.
typedef WwGateSymbolState (*WwGateResolveFn)(const char* symbol, void* user);

typedef struct WwGateReport {
    int checked;
    int missing;
    int folded;
    int no_manifest;
    WwGateOutcome outcome;
    // First offender of each kind, for a log line that names a symbol rather
    // than a count. Empty when that kind did not occur.
    char first_missing[128];
    char first_folded[128];
} WwGateReport;

// Evaluate every symbol in `symbols` (count `n`). TOTAL, not short-circuiting:
// C6 requires the refusal be total rather than partial, so this keeps counting
// after the first failure and reports the whole picture. A gate that stops at
// the first problem tells you one symbol when the answer is "eleven".
WwGateOutcome dExtWwImportGate_evaluate(const char* const* symbols, int n,
                                        WwGateResolveFn resolve, void* user,
                                        WwGateReport* out_report);

// Runs the four controls (C1/C2/C4/C6) against stub resolvers and logs each.
// Returns 0 when all pass. Exists so the gate's behaviour is demonstrable
// without shipping it into a load path first.
int dExtWwImportGate_selftest(void);

#ifdef __cplusplus
}
#endif

#endif  // D_EXT_PLUGIN_WW_IMPORT_GATE_H
