# ============================================================================
# ww_donor_disc — THE Wind Waker plugin (fork-local, NEVER pushed).
#
# ONE PLUGIN, NOT A FAMILY (Integrator assignment, CALLS row 570). This was
# three artifacts — a disc reader, an actor registry, and a hook-smoke
# instrument — because each was built to answer a different question: does
# donor data serve, do by-name hooks fire, does above-enum dispatch route.
# All three are answered, so the split has outlived its purpose. The registry
# is merged in (registry.cpp); the instrument is retired.
#
# Built IN-TREE from the root CMakeLists (guarded on NOT DUSK_EXCLUDE_WW), so
# one normal build produces the .dusk next to the exe (build/<preset>/mods/)
# where the loader's "mods next to the app" search dir finds it.
#
# FEATURES game IS NEW HERE AND IT IS A REAL CHANGE, not a formality: the
# registry's hooks require it (sdk/include/mods/hook.hpp:3 hard-errors
# without DUSK_MOD_FEATURE_GAME), so this target now links against
# dusklight_imports.lib. That is the build-order coupling this mod was
# originally written to AVOID, and the merge accepts it deliberately.
#
# THE COUPLING IS BUILD-TIME ONLY, AND IT IS MEASURED, NOT ASSUMED:
# `dumpbin /imports` on the same FEATURES game + DEFINE_HOOK_SYMBOL shape
# (ww_hook_smoke.dll, H10, run 180329) showed KERNEL32, VCRUNTIME140 and two
# api-ms-win-crt DLLs — and NOTHING from dusklight.exe. The emitted plugin
# carries no thunk into the host and stays version-independent; only the
# BUILD needs the import library present.
# ============================================================================
add_mod(ww_donor_disc
    MOD_JSON mod.json
    FEATURES game
    # ww_message.cpp is its OWN TU deliberately (CALLS §837): the message
    # reader is a new concern, and putting it in registry.cpp — the file two
    # lanes collided in on D1 — would repeat that for no benefit.
    # ww_jmessage.cpp is the WwJMessage:: base layer (§801-admissible port of
    # the donor's NONMATCHING processor.cpp). Its own TU for the same reason
    # ww_message.cpp is: a new concern does not go in the file two lanes have
    # already collided in twice.
    SOURCES main.cpp donor_disc.cpp registry.cpp ww_message.cpp ww_jmessage.cpp ww_dmesg.cpp
)
