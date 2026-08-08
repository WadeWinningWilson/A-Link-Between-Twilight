@echo off
REM ===========================================================================
REM run_gate_off.bat — the V2 "reproduce with the gate off" test.
REM
REM WHAT THIS IS
REM V2's intake rule: every finding states whether it also reproduces with the
REM WW layer OFF. This launches the build that HAS no WW layer, so the answer
REM comes from running rather than from reading code.
REM
REM Step 19 is what made this cheap. Before it, "gate off" meant hunting a
REM per-feature switch; now the whole layer can be compiled out and the tree
REM still links.
REM
REM YOUR NORMAL EXE IS NOT TOUCHED.
REM   normal   build\windows-msvc-relwithdebinfo\dusklight.exe
REM   this     build\ww-exclude-test\dusklight.exe          (no WW layer)
REM
REM WHAT TO EXPECT
REM No Wind Waker content: no WW actors, no WW sky, no WW items, no ported
REM NPCs. That is correct, not a fault -- the layer is absent. Twilight
REM Princess itself should behave exactly as normal.
REM
REM HOW TO READ THE RESULT (for the night-sky stars specifically)
REM All three star TUs -- d_kankyo_rain.cpp, d_kankyo_wether.cpp, d_kankyo.cpp
REM -- are KEPT in this build. The star drawing code is IDENTICAL to the normal
REM exe. So:
REM
REM   stars STILL streaked  -> a TP-side defect. Identical code, no WW layer,
REM                            same bug. Nothing to do with the port.
REM   stars look CORRECT    -> the WW layer is corrupting shared render state
REM                            (GX/TEV, or a leg). Ours, and a much more
REM                            interesting finding.
REM
REM Either answer is worth having. The point of V2 is that this is decided by
REM running, not by reading -- reading the code got the provenance of
REM d_npc.cpp wrong twice in one session.
REM ===========================================================================
setlocal
cd /d "%~dp0"

set "EXE=%~dp0build\ww-exclude-test\dusklight.exe"

if not exist "%EXE%" (
    echo [FAIL] No gate-off build found at:
    echo        %EXE%
    echo.
    echo Build it with:
    echo   cmake --preset windows-msvc-relwithdebinfo -B build\ww-exclude-test -DDUSK_EXCLUDE_WW=ON
    echo   cmake --build build\ww-exclude-test
    exit /b 1
)

echo.
echo === GATE-OFF RUN — WW layer compiled OUT ===
echo   exe : %EXE%
echo   Your normal build is untouched.
echo.
echo   Load a night-time TP area and look at the stars.
echo     still streaked  = TP-side bug, not ours
echo     correct         = the WW layer is corrupting shared render state
echo.

start "" "%EXE%"
endlocal
