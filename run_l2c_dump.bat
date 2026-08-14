@echo off
REM ============================================================
REM tale §792/§794 — L2C EQUIVALENCE CORPUS RUN.
REM Launches Dusklight with DUSK_L2C_DUMP=1 armed: every J3D model the
REM WW resolver consumes is written (runtime-adapted, throwaway copy) to
REM   %APPDATA%\TwilitRealm\Dusklight\l2c_dump\<Arc>__<member>
REM One Outset pass (walk around, open a door or two) is enough, then
REM close the game — Foundry's l2c_equiv.py diffs the dump against the
REM retired references. The var lives only in this window; normal runs
REM via launch.bat / run.bat stay dump-free.
REM ============================================================
cd /d "%~dp0"
set DUSK_L2C_DUMP=1
call launch.bat
