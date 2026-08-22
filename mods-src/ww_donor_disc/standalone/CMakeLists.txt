# ============================================================================
# ww_donor_disc — STANDALONE (out-of-tree) build against the epoch-2 mod SDK.
#
# WHY THIS EXISTS. On 2026-08-17 the clean vanilla at dusklight-main was
# fast-forwarded 39 commits to c880d46fb5, which crossed the Game ABI epoch
# (GAME_SERVICE_MAJOR 1u -> 2u). Every game-coupled mod on that build now fails
# at load:
#
#   [ERROR | wwDonorDisc] failed: Required service unavailable:
#                         dev.twilitrealm.dusklight.game@1 (2)
#
# That is the documented guard, NOT a defect: dusklight's own shadow_mod and
# albw_meter fail identically on the same boot. Mods that touch game code are
# invalidated per ABI epoch, by design.
#
# THE FORK IS DELIBERATELY NOT MERGED (standing user instruction). This target
# therefore builds the SAME sources in ../ against the SDK in the clean vanilla
# checkout, producing a game@2 .dusk without moving the fork one commit.
#
# THIS IS ROUTE (A), AND IT IS SHAPED AS ROUTE (B). The user's sequencing is
# "A, then B ultimately" -- A = build the fork's sources against vanilla's SDK;
# B = the plugin becomes its own project, outside the fork. The SDK's own
# documented entry point is the out-of-tree form:
#
#   add_subdirectory(<dusk>/sdk dusk-sdk EXCLUDE_FROM_ALL)
#   add_mod(my_mod FEATURES game SOURCES ... MOD_JSON mod.json)
#
# so A is written that way on purpose. Reaching B is then a `git mv` of the
# sources next to this file plus dropping the `../` prefixes -- not a rewrite.
#
# NOT WIRED INTO THE FORK'S ROOT BUILD, and that is intentional: the root build
# is epoch-1 and must stay buildable. Configure this directory directly.
# ============================================================================
cmake_minimum_required(VERSION 3.25)
project(ww_donor_disc_standalone CXX)

# ---------------------------------------------------------------------------
# Where the clean vanilla checkout lives. Override with -DDUSK_ROOT=... .
# ---------------------------------------------------------------------------
set(DUSK_ROOT "C:/Users/xxxxx/Documents/dusklight-main" CACHE PATH
        "Clean dusklight checkout supplying the mod SDK")

if (NOT EXISTS "${DUSK_ROOT}/sdk/CMakeLists.txt")
    message(FATAL_ERROR
            "DUSK_ROOT does not look like a dusklight checkout: ${DUSK_ROOT}\n"
            "Expected ${DUSK_ROOT}/sdk/CMakeLists.txt")
endif ()

# ---------------------------------------------------------------------------
# Windows mods link against the host's import library. ModSDK.cmake would
# otherwise DOWNLOAD a version-independent link stub; we point it at the
# import library our own epoch-2 build produced, so the link is against the
# exact binary this plugin is being built for.
#
# Must be set BEFORE add_subdirectory(sdk) -- sdk/CMakeLists.txt declares
# DUSK_GAME_EXE as a cache entry and an unset cache var wins afterwards.
# ---------------------------------------------------------------------------
if (WIN32 AND NOT DUSK_GAME_EXE)
    set(_implib "${DUSK_ROOT}/build/windows-msvc-relwithdebinfo/dusklight_imports.lib")
    if (NOT EXISTS "${_implib}")
        message(FATAL_ERROR
                "Import library not found: ${_implib}\n"
                "Build dusklight in ${DUSK_ROOT} first, or set -DDUSK_GAME_EXE=<path to .lib>")
    endif ()
    set(DUSK_GAME_EXE "${_implib}" CACHE FILEPATH "Game import library" FORCE)
endif ()

add_subdirectory("${DUSK_ROOT}/sdk" dusk-sdk EXCLUDE_FROM_ALL)

# ---------------------------------------------------------------------------
# Same sources, same mod.json, same FEATURES as the in-tree target. The source
# list is duplicated from ../CMakeLists.txt rather than included, because that
# file calls add_mod() itself and would define a second target of the same
# name. KEEP THE TWO LISTS IN SYNC until route (B) retires the in-tree one.
# ---------------------------------------------------------------------------
add_mod(ww_donor_disc
    MOD_JSON ../mod.json
    FEATURES game webgpu
    SOURCES
        ../main.cpp
        ../donor_disc.cpp
        ../registry.cpp
        ../ww_gfx_spike.cpp
        ../ww_message.cpp
        ../ww_jmessage.cpp
        ../ww_dmesg.cpp
        ../ww_kankyo.cpp
        ../ww_kankyo_wind.cpp
        ../ww_vrbox.cpp
        ../ww_sky.cpp
        ../ww_vrkumo.cpp
        ../ww_wave.cpp
        ../ww_shore.cpp
        ../ww_cam_data.cpp
        ../ww_cam_select.cpp
        ../ww_cam_crawl.cpp
)
