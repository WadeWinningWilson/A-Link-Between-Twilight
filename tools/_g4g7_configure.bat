@echo off
REM One-shot intentional reconfigure for the G4 (funchook) + G7 (aurora::gfx) bring-up.
REM Reuses the existing build dir and cache; does not wipe or re-run the preset.
cd /d "%~dp0.."
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cmake -S . -B build\windows-msvc-relwithdebinfo -DDUSK_ENABLE_CODE_MODS=ON -DDUSK_MODS_AURORA_GFX=ON
echo CONFIGURE_EXIT=%ERRORLEVEL%
