# The game ABI surface shared by the main build and the mod SDK (sdk/CMakeLists.txt)
include_guard(GLOBAL)

get_filename_component(_game_root "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

# PARTIAL_DEBUG makes debug and release share one struct/vtable ABI so a mod binary loads into either.
# TARGET_PC_NATIVE_UI is ALBT-local: several d_albw_* headers are gated entirely on it, so a mod
# must see the same value as the host or it sees a different set of declarations.
set(_game_compile_defs TARGET_PC=1 TARGET_PC_NATIVE_UI=1 WIDESCREEN_SUPPORT=1 AVOID_UB=1 VERSION=0
        MTX_USE_PS=1 PARTIAL_DEBUG=1)
if (ANDROID)
    list(APPEND _game_compile_defs TARGET_ANDROID=1)
endif ()

# Public game headers.
# ${_game_root}/src is NOT on main's ABI surface. It is here because this tree still keeps its
# dusk headers under include/dusk (main moved them to src/dusk), and several of those public
# headers include src-only ones (include/dusk/settings.h -> dusk/ui/controls.hpp). Dropping src
# again means relocating those headers first.
set(_game_abi_include_dirs
        ${_game_root}/include
        ${_game_root}/src
        ${_game_root}/assets/GZ2E01
        ${_game_root}/libs/JSystem/include
        ${_game_root}/libs
        ${_game_root}/extern/aurora/include/dolphin
        ${_game_root}/extern/aurora/include
        ${_game_root}/sdk/include
)

# Internal game headers
set(_game_include_dirs
        ${_game_abi_include_dirs}
        ${_game_root}/src
        ${_game_root}/extern
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Mod API, including services
add_library(dusklight_mod_api INTERFACE)
target_include_directories(dusklight_mod_api INTERFACE ${_game_root}/sdk/include)

# Full internal headers used to build the game
add_library(dusklight_game_headers INTERFACE)
target_include_directories(dusklight_game_headers INTERFACE ${_game_include_dirs})
target_compile_definitions(dusklight_game_headers INTERFACE ${_game_compile_defs})

# Public game ABI for mods
add_library(dusklight_game_abi_headers INTERFACE)
target_include_directories(dusklight_game_abi_headers INTERFACE ${_game_abi_include_dirs})
target_compile_definitions(dusklight_game_abi_headers INTERFACE ${_game_compile_defs})

# Mod feature targets
add_library(dusklight_mod_feature_game INTERFACE)
target_link_libraries(dusklight_mod_feature_game INTERFACE
        dusklight_mod_api
        dusklight_game_abi_headers)
target_compile_definitions(dusklight_mod_feature_game INTERFACE DUSK_MOD_FEATURE_GAME=1)
# Game headers assume global.h comes first in the translation unit (it defines DUSK_GAME_DATA
# and friends); force-include it so mods don't depend on include order.
if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    target_compile_options(dusklight_mod_feature_game INTERFACE
            "$<$<COMPILE_LANGUAGE:CXX>:/FIglobal.h>")
else ()
    target_compile_options(dusklight_mod_feature_game INTERFACE
            "$<$<COMPILE_LANGUAGE:CXX>:SHELL:-include global.h>")
endif ()
target_sources(dusklight_mod_feature_game INTERFACE
        ${_game_root}/sdk/src/game_feature.cpp)

add_library(dusklight_mod_feature_webgpu INTERFACE)
target_link_libraries(dusklight_mod_feature_webgpu INTERFACE dusklight_mod_api)
target_compile_definitions(dusklight_mod_feature_webgpu INTERFACE DUSK_MOD_FEATURE_WEBGPU=1)
