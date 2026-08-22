@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=amd64
cd /d C:\Users\xxxxx\Documents\dusklight\mods-src\ww_donor_disc\standalone\build
ninja ww_donor_disc_package
if errorlevel 1 exit /b 1
copy /Y mods\ww_donor_disc.dusk C:\Users\xxxxx\Documents\dusklight-main\build\windows-msvc-relwithdebinfo\mods\ww_donor_disc.dusk
copy /Y mods\ww_donor_disc.dusk C:\Users\xxxxx\Documents\dusklight\build\windows-msvc-relwithdebinfo\mods\ww_donor_disc.dusk
del /Q "%APPDATA%\TwilitRealm\Dusklight\dawn_cache.db*" 2>nul
del /Q "%APPDATA%\TwilitRealm\Dusklight\pipeline_cache.db*" 2>nul
echo DONE
