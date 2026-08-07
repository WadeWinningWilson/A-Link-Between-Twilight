@echo off
REM ============================================================
REM Foundry BTK-parity tap session launcher (bus §219).
REM Sets DUSK_BTK_TAP=4 (log every 4th tap call) so d_ext_npc_mount's
REM dExtBtkTapLog records effective texture-SRT for the scene model1
REM btk and every §218 auto-bound sibling btk. Builds via build_run.bat
REM (lock + guards; build-only despite the name), then LAUNCHES.
REM Mission: stand at the Outset shore ~30 s, quit, tell Foundry.
REM ============================================================
cd /d "%~dp0"
set DUSK_BTK_TAP=4
call build_run.bat
if errorlevel 1 (
    echo [btk-tap] build failed - not launching.
    pause
    exit /b 1
)
echo [btk-tap] launching with DUSK_BTK_TAP=4 ...
start "" /D "%~dp0build\windows-msvc-relwithdebinfo" "%~dp0build\windows-msvc-relwithdebinfo\dusklight.exe"
