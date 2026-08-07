@echo off
REM ============================================================
REM §231 P13 pig state-tap session launcher (Foundry §224 gate).
REM Sets DUSK_STATE_TAP=1 so d_a_kb.cpp emits [DuskLog] §P2 state lines
REM on every pig action/mode TRANSITION (m41E action + m420 mode),
REM builds via build_run.bat (lock + guards; build-only despite the
REM name), wipes the GPU/pipeline caches, then LAUNCHES the game with
REM the tap armed.
REM
REM Session: enter Outset EXTERIOR, exercise pigs (walk / carry /
REM throw / lure near water), then QUIT. Verify the capture with:
REM   D:\XXXXXXX\foundry-py312\Scripts\python.exe tools\foundry\state_gate.py ^
REM     "%APPDATA%\TwilitRealm\Dusklight\logs\<newest>.log" ^
REM     "docs\WW Linked\donor-statemap-pig-kb.md" state_gate_out.md
REM ============================================================
cd /d "%~dp0"
set DUSK_STATE_TAP=1
call build_run.bat
if errorlevel 1 (
    echo [state-tap] build failed - not launching.
    pause
    exit /b 1
)
REM Standing rule: wipe dawn/pipeline caches after every build.
del /q "%APPDATA%\TwilitRealm\Dusklight\dawn_cache.db*" 2>nul
del /q "%APPDATA%\TwilitRealm\Dusklight\pipeline_cache.db*" 2>nul
echo [state-tap] launching with DUSK_STATE_TAP=1 ...
start "" /D "%~dp0build\windows-msvc-relwithdebinfo" "%~dp0build\windows-msvc-relwithdebinfo\dusklight.exe"
