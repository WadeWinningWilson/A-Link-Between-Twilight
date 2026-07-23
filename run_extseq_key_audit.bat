@echo off
REM ============================================================
REM  §79 — ExtSeq KEY AUDIT run (runtime key-region thread)
REM  Double-click this instead of launching normally. It sets the
REM  audit flag, launches the game, and reminds you what to do.
REM
REM  IN GAME: enter Outset (i_linkin rig), stand still, listen
REM  (cascade ear-check), let it play ~2 minutes, quit.
REM
REM  AFTERWARD: the log lines Engine needs are in the newest
REM    %%AppData%%\TwilitRealm\Dusklight\logs\dusklight-*.log
REM  grep/search for:  keyAudit   and   tempoProbe
REM
REM  Delete this file when the audio work is finished.
REM ============================================================
cd /d "%~dp0"

set "DUSK_EXTSEQ_KEY_AUDIT=1"

set "EXE=%~dp0build\windows-msvc-relwithdebinfo\dusklight.exe"
if not exist "%EXE%" (
    echo ERROR: dusklight.exe not found at:
    echo   %EXE%
    echo Build first, then re-run this.
    pause
    exit /b 1
)

echo.
echo   DUSK_EXTSEQ_KEY_AUDIT=1
echo   launching: %EXE%
echo.
echo   IN GAME:
echo     1. enter Outset  (i_linkin plays - EAR CHECK the cascade fix)
echo     2. stand still ~2 minutes
echo     3. quit
echo.

"%EXE%"

echo.
echo   Run finished. Newest log:
for /f "delims=" %%f in ('dir /b /o-d "%APPDATA%\TwilitRealm\Dusklight\logs\dusklight-*.log" 2^>nul') do (
    echo     %APPDATA%\TwilitRealm\Dusklight\logs\%%f
    goto :done
)
:done
echo   Search it for:  keyAudit   /   tempoProbe
echo.
pause
