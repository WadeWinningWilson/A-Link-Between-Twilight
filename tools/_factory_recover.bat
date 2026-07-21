@echo off
setlocal
cd /d "%~dp0.."
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
  echo vcvars failed
  exit /b 1
)
set ALLOW_CMAKE_RERUN=1
echo === cmake --preset windows-msvc-relwithdebinfo ===
cmake --preset windows-msvc-relwithdebinfo
if errorlevel 1 exit /b 1

set "BUILD_DIR=%cd%\build\windows-msvc-relwithdebinfo"
findstr /C:"/O2" "%BUILD_DIR%\build.ninja" >nul
if errorlevel 1 (
  echo FAIL: build.ninja still has no /O2
  exit /b 98
)
echo OK: /O2 present in build.ninja

echo === ninja clean dusklight ===
ninja -C "%BUILD_DIR%" -t clean dusklight
if errorlevel 1 echo clean returned %ERRORLEVEL% ^(continuing^)

del /q "%BUILD_DIR%\dusklight.ilk" 2>nul
del /q "%BUILD_DIR%\dusklight.exe" 2>nul

echo === ninja -j8 dusklight ^(full rebuild^) ===
ninja -j8 -C "%BUILD_DIR%" dusklight
if errorlevel 1 exit /b 1

for %%F in ("%BUILD_DIR%\dusklight.exe") do echo EXE_SIZE=%%~zF TIME=%%~tF
findstr /C:"/O2" "%BUILD_DIR%\build.ninja" >nul || (echo FAIL post-build no /O2 & exit /b 98)
echo FACTORY_RECOVER_OK
exit /b 0
