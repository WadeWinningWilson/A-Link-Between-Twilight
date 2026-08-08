@echo off
REM ===========================================================================
REM build_ww_excluded.bat — the WW-layer separability REGRESSION GATE.
REM
REM WHAT THIS IS FOR — AND WHAT CHANGED
REM This began as step 11's diagnostic: nobody knew whether the tree still
REM LINKED with the WW layer's files excluded, and a link FAILURE was the
REM deliverable, because the unresolved-symbol list named the legs.
REM
REM STEP 19 ANSWERED THAT. 129 of 129 inbound symbols were severed (§589) and
REM the receiver links with the WW layer entirely absent. So the meaning of
REM this script's result is now INVERTED:
REM
REM   LINKS      = the expected state. Separability holds.
REM   DOES NOT   = a REGRESSION. Something re-bound the WW layer into the
REM                receiver. The unresolved list names what.
REM
REM THIS IS A DIAGNOSTIC BUILD, NOT A PLAY BUILD.
REM If it links, the exe has the WW layer compiled OUT. Do not expect WW
REM content in it. Do not replace your normal exe with it.
REM
REM YOUR NORMAL BUILD IS UNTOUCHED. Different directory, different exe:
REM   normal : build\windows-msvc-relwithdebinfo\dusklight.exe   (unchanged)
REM   this   : build\ww-exclude-test\dusklight.exe
REM
REM DIRECTORY NOTE — READ THIS BEFORE TRUSTING AN OLD LOG.
REM The step-19 green result was proven in build\ww-exclude-test, and
REM run_gate_off.bat already treats that path as the canonical no-WW exe.
REM This script formerly built build\ww-excluded and wrote
REM build\ww-excluded-link.log. Those artifacts are PRE-SEVERANCE and SUPER-
REM SEDED: that log ends in "LNK1120: 130 unresolved externals" and reads as
REM though step 19 were still open. It is not. Do not cite it.
REM ===========================================================================
setlocal
cd /d "%~dp0"

set "BUILDDIR=build\ww-exclude-test"
set "LOGFILE=build\ww-exclude-test-link.log"

echo.
echo === WW separability gate: building with DUSK_EXCLUDE_WW=ON ===
echo     output : %BUILDDIR%
echo     log    : %LOGFILE%
echo     Your normal build and exe are NOT touched.
echo.

call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" > nul 2>&1
if errorlevel 1 (
    echo [FAIL] Could not initialise the MSVC environment. Is Visual Studio installed
    echo        at the path in this script?
    exit /b 1
)

echo --- configuring ---
cmake --preset windows-msvc-relwithdebinfo -B "%BUILDDIR%" -DDUSK_EXCLUDE_WW=ON
if errorlevel 1 (
    echo.
    echo [FAIL] Configure failed. If it says the exclusion list is STALE or missing,
    echo        regenerate it and re-run:
    echo          python tools\ww_crew_restoration_skeleton\ww_layer_manifest.py --emit-cmake
    exit /b 1
)

echo.
echo --- building (this takes a while; full output goes to the log) ---
cmake --build "%BUILDDIR%" > "%LOGFILE%" 2>&1
set "RC=%ERRORLEVEL%"

echo.
if "%RC%"=="0" (
    echo ============================================================
    echo   RESULT: LINKED CLEAN — the expected state since step 19.
    echo   The WW layer is separable at link time. No regression.
    echo   Exe: %BUILDDIR%\dusklight.exe  ^(WW layer compiled OUT -- not a play build^)
    echo ============================================================
) else (
    echo ============================================================
    echo   RESULT: DID NOT LINK ^(exit %RC%^) -- this is a REGRESSION.
    echo   Step 19 severed all 129 inbound symbols; if any are back, the
    echo   receiver has re-bound the WW layer. The unresolved list below
    echo   names exactly which references did it.
    echo ============================================================
    echo.
    echo   --- unresolved externals ^(first 40^) ---
    findstr /C:"unresolved external" "%LOGFILE%" 2>nul | more +0
    echo.
    echo   Full log: %LOGFILE%
)

echo.
echo Your normal build is still at build\windows-msvc-relwithdebinfo\dusklight.exe
exit /b %RC%
