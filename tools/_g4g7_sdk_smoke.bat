@echo off
REM Out-of-tree Mod SDK smoke test: configure + build albw-meter against THIS tree's sdk/ and the
REM import library setup_windows_exports() generated. Scratch build dir under %TEMP%.
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
set "HOST=%USERPROFILE%\Documents\dusklight"
set "BD=%TEMP%\albw-meter-albt-sdk"
cmake -S %USERPROFILE%\Documents\albw-meter -B "%BD%" -G Ninja ^
  -DDUSKLIGHT_DIR=%HOST% ^
  -DDUSK_GAME_EXE=%HOST%/build/windows-msvc-relwithdebinfo/dusklight_imports.lib
echo SDK_CONFIGURE_EXIT=%ERRORLEVEL%
cmake --build "%BD%" > "%TEMP%\albw_build.log" 2>&1
echo SDK_BUILD_EXIT=%ERRORLEVEL%
