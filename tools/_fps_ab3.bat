@echo off
setlocal
set "BUILD=%~dp0..\build\windows-msvc-relwithdebinfo"
set "DIRTY=%~dp0..\build\fps-dirty-backup-20260719-1624"
set "CLEAN=%~dp0..\build\fps-good-backup-20260719-0827"
set "WW=%APPDATA%\TwilitRealm\Dusklight\model_replacements\WW-Crew-Restoration"
set "WWD=%APPDATA%\TwilitRealm\Dusklight\model_replacements\WW-Crew-Restoration.DISABLED"
set "CACHE=%APPDATA%\TwilitRealm\Dusklight"

if /I "%~1"=="" goto usage
if /I "%~1"=="1" goto t1
if /I "%~1"=="2" goto t2
if /I "%~1"=="3" goto t3
if /I "%~1"=="restore" goto restore
goto usage

:t1
echo === TEST 1: dirty exe + Ordon F_SP121 (WW folder left as-is) ===
call :stop_wipe
call :use_dirty
echo Launch game. Warp F_SP121 r0 p0. Report menu + field min/typ/max.
exit /b 0

:t2
echo === TEST 2: dirty exe + Outset, WW folder OFF ===
call :stop_wipe
call :use_dirty
if exist "%WW%" (
  ren "%WW%" "WW-Crew-Restoration.DISABLED"
  echo Renamed WW-Crew-Restoration -^> .DISABLED
) else if exist "%WWD%" (
  echo WW already .DISABLED
) else (
  echo WARNING: WW folder not found
)
echo Launch. Same Outset spot as the swing. Report menu + field min/typ/max.
exit /b 0

:t3
echo === TEST 3: CLEANr morning exe + Outset, WW folder ON ===
call :stop_wipe
if exist "%WWD%" (
  ren "%WWD%" "WW-Crew-Restoration"
  echo Restored WW-Crew-Restoration
)
call :use_clean
echo Launch. Same Outset spot. Report menu + field min/typ/max.
echo When done: tools\_fps_ab3.bat restore
exit /b 0

:restore
echo === RESTORE dirty play exe + WW ON ===
call :stop_wipe
if exist "%WWD%" ren "%WWD%" "WW-Crew-Restoration"
call :use_dirty
echo Restored.
exit /b 0

:use_dirty
copy /Y "%DIRTY%\dusklight.exe" "%BUILD%\dusklight.exe" >nul
if exist "%DIRTY%\dusklight.pdb" copy /Y "%DIRTY%\dusklight.pdb" "%BUILD%\dusklight.pdb" >nul
echo Using DIRTY backup exe
exit /b 0

:use_clean
copy /Y "%CLEAN%\dusklight.exe" "%BUILD%\dusklight.exe" >nul
if exist "%CLEAN%\dusklight.pdb" copy /Y "%CLEAN%\dusklight.pdb" "%BUILD%\dusklight.pdb" >nul
echo Using CLEANr backup exe
exit /b 0

:stop_wipe
taskkill /IM dusklight.exe /F >nul 2>&1
del /q "%CACHE%\dawn_cache.db*" >nul 2>&1
del /q "%CACHE%\pipeline_cache.db*" >nul 2>&1
exit /b 0

:usage
echo Usage: _fps_ab3.bat 1^|2^|3^|restore
echo   1       dirty + Ordon F_SP121
echo   2       dirty + Outset, WW OFF
echo   3       CLEANr + Outset, WW ON
echo   restore dirty exe + WW ON
exit /b 1
