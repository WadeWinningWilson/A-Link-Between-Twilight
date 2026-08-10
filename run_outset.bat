@echo off
REM ===========================================================================
REM run_outset.bat -- arm OUTSET as a warp destination.
REM
REM Outset Island is sea room 44 -- room 44 of a FIFTY-ROOM stage, because in
REM Wind Waker `sea` is every island at once. Under the zero-bake rule it cannot
REM be trimmed (trimming is editing), so the whole stage comes in.
REM
REM The arcs are staged BYTE-IDENTICAL from the disc, under their vanilla names
REM (Room44.arc, Stage.arc) at res/Stage/sea/. Nothing is repacked, renamed or
REM edited. `sea` is declared in the mod's ww_stages.ini, which is what switches
REM on the arc-filename alias -- the receiver asks for R44_00.arc and vanilla
REM ships Room44.arc.
REM
REM THIS DOES NOT TRAVEL BY ITSELF. Load a save, get somewhere settled, then:
REM     Warp tab  ->  section "Dev stage (--stage)"  ->  "sea  room 44  layer -1"
REM
REM EXPECT, and none of these are faults:
REM   - a SLOW first load; fifty rooms
REM   - a SPARSE island: 282 of 475 placements are unported actors and will
REM     no-op at proc -1. A fully populated Outset would be the red flag.
REM   - [WwRoomSeam] lines, including the first RCAM specimen in the campaign
REM
REM FILE FORMAT NOTE (S609): pure ASCII, CRLF only.
REM ===========================================================================
setlocal
cd /d "%~dp0"
start "" "%~dp0build\windows-msvc-relwithdebinfo\dusklight.exe" --stage sea,44
