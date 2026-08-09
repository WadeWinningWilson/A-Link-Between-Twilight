@echo off
REM ===========================================================================
REM run_rdl02.bat -- arm R_DL02 as a warp destination (native-rooms 3b pilot).
REM
REM THIS DOES NOT TRAVEL BY ITSELF (changed 2026-08-09 on user ruling).
REM It launches with --stage, which adds ONE button to the warp window:
REM     Warp tab  ->  section "Dev stage (--stage)"  ->  "R_DL02  room 0  layer -1"
REM Load a save, get somewhere settled, then press it.
REM
REM WHY NOT AUTOMATIC ANYMORE
REM The first revision fired ~1s into the play scene, which landed on top of the
REM save's entry demo. Moving during whatever the save was mid-way through makes
REM a fault in the DESTINATION indistinguishable from a fault caused by
REM interrupting the ORIGIN. Warping on purpose keeps the destination the only
REM variable.
REM
REM WHY --stage AND NOT A WARP ROW OR AN SCLS EDIT
REM   a warp row forces a BG payload -- rows are only valid with arc= and
REM     collision= -- so it mounts geometry into the stage instead of going to it
REM   an SCLS edit would write a record Wind Waker does not have, into the very
REM     data the census and the translators read
REM This touches no game data at all. Nothing to revert.
REM
REM EXPECT: [BootStage] armed at launch, [BootStage] WARP when you press it, then
REM [WwRoomSeam] lines for the rooms you visit. VISIBLY MISSING PERFORMERS are
REM correct -- Ji1/Orca is not ported. A full house would be the red flag.
REM
REM KNOWN, NOT YET EXPLAINED: the room draws but everything is dark. Being
REM chased separately; it is not the crash and not the node-type fix.
REM
REM FILE FORMAT NOTE (S609): pure ASCII, CRLF only. An LF-only bat makes cmd.exe
REM execute comment fragments as commands.
REM ===========================================================================
setlocal
cd /d "%~dp0"
start "" "%~dp0build\windows-msvc-relwithdebinfo\dusklight.exe" --stage R_DL02
