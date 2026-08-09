@echo off
REM ===========================================================================
REM run_rdl02.bat -- boot straight into R_DL02 (native-rooms 3b pilot).
REM
REM Uses the --stage dev flag, NOT a warp row and NOT an SCLS edit:
REM   a warp row forces a BG payload, and that payload CRASHED in R_DL02
REM     (mDoExt_setupStageTexture: a dedicated bake carries no donor stage res)
REM   an SCLS edit would write a record Wind Waker does not have, into the
REM     very data the census and translators read
REM This touches no game data at all. Nothing to revert.
REM
REM EXPECT: [WwRoomSeam] lines for R_DL02 rooms 0/1, possible LGTV noise, and
REM VISIBLY MISSING PERFORMERS -- Ji1/Orca is not ported. A full house would be
REM the red flag, not the success.
REM
REM THE LOG IS THE VERDICT, not the screen.
REM ===========================================================================
setlocal
cd /d "%~dp0"
start "" "%~dp0build\windows-msvc-relwithdebinfo\dusklight.exe" --stage R_DL02
