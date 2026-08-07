@echo off
REM ============================================================
REM Plain build + launch — the general "build and play" runner.
REM build_run.bat is BUILD-ONLY (lock + guards); this calls it, wipes the
REM GPU/pipeline caches (standing rule), then LAUNCHES the game with NO
REM diagnostic env var. Use run_state_tap.bat / run.bat / run_*_tap.bat
REM instead when you need a specific DUSK_* probe armed.
REM ============================================================
cd /d "%~dp0"
call build_run.bat
if errorlevel 1 (
    echo [launch] build failed - not launching.
    pause
    exit /b 1
)
set "CACHE=%APPDATA%\TwilitRealm\Dusklight"
del /q "%CACHE%\dawn_cache.db*" "%CACHE%\pipeline_cache.db*" 2>nul
echo [launch] launching dusklight.exe ...
start "" /D "%~dp0build\windows-msvc-relwithdebinfo" "%~dp0build\windows-msvc-relwithdebinfo\dusklight.exe"
