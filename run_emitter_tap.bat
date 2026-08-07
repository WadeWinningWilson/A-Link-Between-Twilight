@echo off
REM ============================================================
REM Foundry P2 emitter-tap session launcher (bus §207/§208).
REM Sets DUSK_EMITTER_TAP=1 so d_particle.cpp's emitter-parity tap
REM logs every simple-emitter creation, builds via build_run.bat
REM (lock + guards; build-only despite the name), then LAUNCHES the
REM game with the tap armed. Play ~5 min of WW Outset EXTERIOR,
REM quit, tell Foundry — the differ does the rest.
REM ============================================================
cd /d "%~dp0"
set DUSK_EMITTER_TAP=1
call build_run.bat
if errorlevel 1 (
    echo [emitter-tap] build failed - not launching.
    pause
    exit /b 1
)
echo [emitter-tap] launching with DUSK_EMITTER_TAP=1 ...
start "" /D "%~dp0build\windows-msvc-relwithdebinfo" "%~dp0build\windows-msvc-relwithdebinfo\dusklight.exe"
