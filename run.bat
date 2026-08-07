@echo off
REM ============================================================
REM Head-loss diagnostic run (§head-dbg).
REM Repro: walk into an Outset interior via a door, then back out.
REM Then send the log. Grep line: [ExtNpcMount] §head-dbg
REM   gen=<N>  = world generation (first island load vs each reload)
REM   seat=(x,y,z)   -> head IS seated; compare to body=(x,y,z)
REM   seat=SRC_NULL   -> seat found no host matrix (mAttachJnt stale)
REM   (no §head-dbg line for a proc) -> callback never ran on respawn
REM Compare the FIRST load's gen vs the post-interior reload's gen.
REM ============================================================
setlocal
set "DUSK_HEAD_DEBUG=1"
set "EXE=%~dp0build\windows-msvc-relwithdebinfo\dusklight.exe"
if not exist "%EXE%" (
  echo missing %EXE% — run build_run.bat first
  exit /b 1
)
set "CACHE=%APPDATA%\TwilitRealm\Dusklight"
del /q "%CACHE%\dawn_cache.db*" "%CACHE%\pipeline_cache.db*" 2>nul
echo DUSK_HEAD_DEBUG=%DUSK_HEAD_DEBUG%
echo Launching %EXE%
echo Repro: Outset -^> interior door -^> back out. Then send the log ^(grep: head-dbg^).
start "" "%EXE%"
endlocal
