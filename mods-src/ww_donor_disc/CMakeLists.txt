# ============================================================================
# ww_donor_disc — L2 donor disc reader plugin (fork-local, NEVER pushed).
#
# Built IN-TREE from the root CMakeLists (guarded on NOT DUSK_EXCLUDE_WW), so
# one normal build produces the .dusk next to the exe (build/<preset>/mods/)
# where the loader's "mods next to the app" search dir finds it. Services
# only — no FEATURES game — so it links against nothing of the exe and works
# regardless of build order.
# ============================================================================
add_mod(ww_donor_disc
    MOD_JSON mod.json
    SOURCES main.cpp donor_disc.cpp
)
