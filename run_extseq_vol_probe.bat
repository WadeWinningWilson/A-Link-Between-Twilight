@echo off
REM Live §C.1 absolute-level / composedVolume probe against field ExtSeq (i_link/house).
REM Set DUSK_EXTSEQ_VOL_PROBE=1, wipe GPU caches, launch RelWithDebInfo build.
REM Grep logs for: [ExtSeq] §C.1 volProbe
setlocal
set "DUSK_EXTSEQ_VOL_PROBE=1"
set "EXE=%~dp0build\windows-msvc-relwithdebinfo\dusklight.exe"
if not exist "%EXE%" (
  echo missing %EXE% — run build_run.bat first
  exit /b 1
)
set "CACHE=%APPDATA%\TwilitRealm\Dusklight"
del /q "%CACHE%\dawn_cache.db*" "%CACHE%\pipeline_cache.db*" 2>nul
echo DUSK_EXTSEQ_VOL_PROBE=%DUSK_EXTSEQ_VOL_PROBE%
echo Launching %EXE%
echo Play confirmed Outset theme ^(i_link^); copy volProbe lines from the log.
start "" "%EXE%"
endlocal
