@echo off
REM /O2 factory proof for the RelWithDebInfo ninja + cache (see .cursor/rules/build-fps-safe.mdc)
set "BD=%~dp0..\build\windows-msvc-relwithdebinfo"
echo === build.ninja lines containing /O2:
findstr /C:"/O2" "%BD%\build.ninja" | find /c /v ""
echo === build.ninja FLAGS lines total:
findstr /C:"FLAGS = " "%BD%\build.ninja" | find /c /v ""
echo === build.ninja FLAGS lines with /O2:
findstr /C:"FLAGS = " "%BD%\build.ninja" | findstr /C:"/O2" | find /c /v ""
echo === cache flags:
findstr /B /C:"CMAKE_CXX_FLAGS_RELWITHDEBINFO:" /C:"CMAKE_C_FLAGS_RELWITHDEBINFO:" "%BD%\CMakeCache.txt"
echo === code-mod cache entries:
findstr /B /C:"DUSK_ENABLE_CODE_MODS:" /C:"DUSK_MODS_AURORA_GFX:" /C:"DUSK_HAS_FUNCHOOK:" /C:"DUSK_HAS_PREPATCH:" "%BD%\CMakeCache.txt"
