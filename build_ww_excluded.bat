@echo off
REM ===========================================================================
REM build_ww_excluded.bat — roadmap step 11, the LINK-TIME leg.
REM
REM WHAT THIS IS FOR
REM Configure-level exclusion is already proven (§572: 48 of 48 listed TUs
REM removed, both refusal guards fire). What is NOT known is whether the tree
REM still LINKS with those 48 files gone. Legs -- donor lines living inside
REM receiver-owned TUs -- may reference symbols defined in the excluded files,
REM and the exclusion is documented PARTIAL precisely because it cannot see
REM them. This build answers that and nothing else.
REM
REM THIS IS A DIAGNOSTIC BUILD, NOT A PLAY BUILD.
REM If it links, the exe has the WW layer compiled OUT. Do not expect WW
REM content in it. Do not replace your normal exe with it.
REM
REM YOUR NORMAL BUILD IS UNTOUCHED. Different directory, different exe:
REM   normal : build\windows-msvc-relwithdebinfo\dusklight.exe   (unchanged)
REM   this   : build\ww-excluded\dusklight.exe
REM
REM A LINK FAILURE HERE IS THE RESULT, NOT A BREAKAGE. The unresolved-symbol
REM list names exactly which legs bind the WW layer into the receiver, which is
REM the measurement step 11 has been missing.
REM ===========================================================================
setlocal
cd /d "%~dp0"

set "BUILDDIR=build\ww-excluded"
set "LOGFILE=build\ww-excluded-link.log"

echo.
echo === Step 11 link-time test: building with DUSK_EXCLUDE_WW=ON ===
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
    echo   RESULT: LINKED CLEAN.
    echo   The WW stacks are separable at link time. Step 11 discharges.
    echo   Exe: %BUILDDIR%\dusklight.exe  ^(WW layer compiled OUT -- not a play build^)
    echo ============================================================
) else (
    echo ============================================================
    echo   RESULT: DID NOT LINK ^(exit %RC%^) -- this is a MEASUREMENT, not a break.
    echo   The unresolved symbols below name the legs that bind the WW layer
    echo   into the receiver. That list is the deliverable.
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
