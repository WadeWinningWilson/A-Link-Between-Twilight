@echo off
setlocal
echo.
echo ALBT companion mods -- manual install
echo.
echo Copy the pack folders from this directory into model_replacements:
echo.
echo   Portable alpha:
echo     ^<your unzip^>\build\windows-msvc-relwithdebinfo\data\model_replacements\
echo.
echo   Local Dusklight:
echo     %APPDATA%\TwilitRealm\Dusklight\model_replacements\
echo.
echo Packs: "Armogohma Custom"  "MM-SkullKid-Reskin"  "Wind Waker Skins"  "Wind Waker Deku Leaf"
echo.
echo Then turn on Boss Refinement and Custom Models in Settings.
echo.
if exist "%APPDATA%\TwilitRealm\Dusklight\model_replacements" (
  explorer "%APPDATA%\TwilitRealm\Dusklight\model_replacements"
)
explorer "%~dp0"
echo.
pause
