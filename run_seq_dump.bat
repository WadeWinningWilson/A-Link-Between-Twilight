@echo off
REM ============================================================
REM  ASK 17 / §59 — ExtSeq event dump run
REM  Double-click this. It sets the dump flag and launches the
REM  game so the engine writes its BMS event stream for diffing
REM  against Bridge's independent decode.
REM
REM  Output lands beside Bridge's goldens in:
REM    %%APPDATA%%\TwilitRealm\Dusklight\model_replacements\
REM      WW-Crew-Restoration\audio\ww_jaudio1\
REM        seq_events_engine_i_link.csv
REM        seq_events_engine_house.csv
REM
REM  Delete this file when the audio timing work is finished.
REM ============================================================
cd /d "%~dp0"

set "DUSK_EXTSEQ_EVENT_DUMP=1"

set "EXE=%~dp0build\windows-msvc-relwithdebinfo\dusklight.exe"
if not exist "%EXE%" (
    echo ERROR: dusklight.exe not found at:
    echo   %EXE%
    echo Build first, then re-run this.
    pause
    exit /b 1
)

echo.
echo   DUSK_EXTSEQ_EVENT_DUMP=1
echo   launching: %EXE%
echo.
echo   IN GAME:
echo     1. warp to Outset  ^(exterior music starts -^> i_link dump^)
echo     2. enter Grandma's house ^(interior music starts -^> house dump^)
echo     3. quit
echo.

"%EXE%"

echo.
echo   Run finished. Checking for dumps...
set "OUT=%APPDATA%\TwilitRealm\Dusklight\model_replacements\WW-Crew-Restoration\audio\ww_jaudio1"
if exist "%OUT%\seq_events_engine_i_link.csv" (
    echo   FOUND: seq_events_engine_i_link.csv
) else (
    echo   MISSING: seq_events_engine_i_link.csv  ^(did Outset music start?^)
)
if exist "%OUT%\seq_events_engine_house.csv" (
    echo   FOUND: seq_events_engine_house.csv
) else (
    echo   MISSING: seq_events_engine_house.csv  ^(did interior music start?^)
)
echo.
pause
