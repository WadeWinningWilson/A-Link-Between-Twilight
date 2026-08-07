@echo off
REM ============================================================
REM  Ferry A / $147 - FPS BISECT run #waves
REM  Skips: usonami / wave-pane arming
REM  (tests the pane system's share)
REM
REM  IN GAME: warp to F_DL01 exterior (Outset), stand at the
REM  lookout facing the sea, note the FPS (HUD or log), quit.
REM
REM  AFTERWARD: check the newest log in
REM    %%AppData%%\TwilitRealm\Dusklight\logsREM  for:  [WwFoam] FerryA FPS_BISECT mode=waves
REM  and the periodic:  ... sample mode=waves fps=...
REM
REM  Run all three bats (palette / waves / btk), one at a time,
REM  restarting the exe each run (mode is cached at first read).
REM  Delete these three files when the FPS work is finished.
REM ============================================================
cd /d "%~dp0"

set "DUSK_WW_FPS_BISECT=waves"

set "EXE=%~dp0build\windows-msvc-relwithdebinfo\dusklight.exe"
if not exist "%EXE%" (
    echo ERROR: dusklight.exe not found at:
    echo   %EXE%
    echo Build first, then re-run this.
    pause
    exit /b 1
)

echo.
echo   DUSK_WW_FPS_BISECT=waves   (usonami / wave-pane arming skipped)
echo   launching: %EXE%
echo.

"%EXE%"

echo.
echo   Run finished. Note the FPS you saw and check the newest log for:
echo     [WwFoam] FerryA FPS_BISECT mode=waves
echo.
pause
