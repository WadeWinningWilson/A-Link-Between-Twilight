@echo off
REM ============================================================
REM  Ferry C / $151 - FPS BISECT run #wavedraw
REM  Waves ARMED + wave_move runs; drawWave SKIPPED.
REM  Answers: is the cost draw or move/churn?
REM
REM  IN GAME: warp to F_DL01 exterior (Outset), stand at the
REM  lookout facing the sea, note the FPS (HUD or log), quit.
REM
REM  AFTERWARD: check the newest log in
REM    %AppData%\TwilitRealm\Dusklight\logs
REM  for:  [WwFoam] FerryA FPS_BISECT mode=wavedraw
REM        [WwFoam] FerryC FPS_BISECT wavedraw SKIP draw panes=N
REM  and the periodic:  ... sample mode=wavedraw fps=...
REM
REM  Verdict: ~270 = DRAW (STEP 2A); ~40-50 = MOVE (STEP 2B).
REM  Delete with the other fps_bisect bats when FPS work ends.
REM ============================================================
cd /d "%~dp0"

set "DUSK_WW_FPS_BISECT=wavedraw"

set "EXE=%~dp0build\windows-msvc-relwithdebinfo\dusklight.exe"
if not exist "%EXE%" (
    echo ERROR: dusklight.exe not found at:
    echo   %EXE%
    echo Build first, then re-run this.
    pause
    exit /b 1
)

echo.
echo   DUSK_WW_FPS_BISECT=wavedraw   (wave_move ON, drawWave OFF)
echo   launching: %EXE%
echo.

"%EXE%"

echo.
echo   Run finished. Note the FPS you saw and check the newest log for:
echo     [WwFoam] FerryA FPS_BISECT mode=wavedraw
echo     [WwFoam] FerryC FPS_BISECT wavedraw SKIP draw panes=...
echo.
pause
