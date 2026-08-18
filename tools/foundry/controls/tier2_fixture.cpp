// ============================================================================
// NEGATIVE/POSITIVE CONTROL FIXTURE for tier2_census.py — do not "fix".
// Every case below has a KNOWN classification; controls/tier2_selftest.py
// asserts them. If the census misreads any, its counts are worthless.
// (Deliberately not valid, linkable C++ — same arrangement as the other
// fixtures in this directory; nothing in the build reaches tools/.)
// ============================================================================

// CASE 1 — port-track dWw call inside a SINGLE-LINE TP host: must be B.
void dDemoFixture_tpHostSingle(int x) {
    dWwProfileRegister_lookup(x);        // EXPECT B
}

// CASE 2 — the same call inside a MULTI-LINE TP host signature: must be B.
// (The pre-repair scanner lost wrapped hosts and filed these as A.)
void dDemoFixture_tpHostWrapped(int alpha,
                                int beta,
                                int gamma) {
    dExtWwSave_isWwHostStage("sea");     // EXPECT B
}

// CASE 3 — skins API in a TP host: EXCLUDED BY USER RULING (ALBW track).
void dDemoFixture_skins(int x) {
    dWwItemmdl_heldSkinActive(x);        // EXPECT not counted
}

// CASE 4 — vanilla TP false-positive guards: must NOT count (first-run FPs).
void dDemoFixture_vanilla(void) {
    int a = dRes_ID_ALANM_BCK_DAMSWW_e;  // EXPECT not counted
    const char* p = "ZI_S_ww_blur_a.jpa"; // EXPECT not counted
}

// CASE 5 — WW row in a static table: must be C.
static int l_fixtureTable[] = {
    fpcNm_WW_FIXTURE_e,                  // EXPECT C
};

// CASE 6 — added WW function (WW-named host): must be A.
void dExtWwFixture_added(void) {
    dExtWw_handleDemoMessage(0);         // EXPECT A
}
