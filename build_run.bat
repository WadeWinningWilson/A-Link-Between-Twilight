@echo off
cd /d "%~dp0"

REM ---------------------------------------------------------------------------
REM Build lock. Several chats/agents share this working tree, and two builds
REM running at once fight over the same object files: the loser dies on
REM "Permission denied" writing a .obj, and the exe that survives is a blend of
REM whatever each lane had half-written. That is expensive precisely because the
REM resulting binary still looks buildable - a fault in it cannot be attributed
REM to either lane.
REM
REM Serialising by remembering not to overlap is not a mechanism. This is.
REM mkdir is atomic, so it doubles as the lock: whoever creates the directory
REM owns the build, everyone else fails fast and is told when it started. A
REM stale lock (killed build, closed shell) is broken automatically after
REM LOCK_STALE_MIN minutes so nobody is left permanently blocked.
REM ---------------------------------------------------------------------------
setlocal enabledelayedexpansion
set "LOCKDIR=%~dp0build\.build-lock"
set "LOCK_STALE_MIN=30"

if not exist "%~dp0build" mkdir "%~dp0build" 2>nul

if exist "%LOCKDIR%" (
    set "STALE=0"
    for /f %%A in ('powershell -NoProfile -Command "if (((Get-Date) - (Get-Item '%LOCKDIR%').CreationTime).TotalMinutes -gt %LOCK_STALE_MIN%) { 1 } else { 0 }" 2^>nul') do set "STALE=%%A"
    if "!STALE!"=="1" (
        echo [build-lock] breaking stale lock ^(older than %LOCK_STALE_MIN% min^)
        rmdir /s /q "%LOCKDIR%" 2>nul
    )
)

mkdir "%LOCKDIR%" 2>nul
if errorlevel 1 (
    echo.
    echo ============================================================
    echo   BUILD REFUSED - another build is already running.
    echo ============================================================
    if exist "%LOCKDIR%\owner.txt" type "%LOCKDIR%\owner.txt"
    echo.
    echo   Two builds on one tree corrupt each other's object files and
    echo   produce a binary mixing both lanes' work. Wait for the other
    echo   build to finish, then run this again.
    echo.
    echo   If you are certain nothing is building, delete:
    echo     %LOCKDIR%
    echo ============================================================
    exit /b 2
)
>"%LOCKDIR%\owner.txt" echo   started: %DATE% %TIME%
>>"%LOCKDIR%\owner.txt" echo   host: %COMPUTERNAME%  user: %USERNAME%

call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" > nul 2>&1
if not exist "%~dp0build\windows-msvc-relwithdebinfo\build.ninja" (
    cmake --preset windows-msvc-relwithdebinfo
) else if not exist "%~dp0build\windows-msvc-relwithdebinfo\CMakeFiles\rules.ninja" (
    cmake --preset windows-msvc-relwithdebinfo
)

REM Hard-poison guard: empty RelWithDebInfo flags → ninja without /O2 → ~33 FPS.
findstr /C:"/O2" "%~dp0build\windows-msvc-relwithdebinfo\build.ninja" >nul
if errorlevel 1 (
    echo.
    echo ============================================================
    echo   BUILD REFUSED - RelWithDebInfo factory missing /O2
    echo ============================================================
    echo   build.ninja CXX FLAGS have no /O2. Do not feature-bisect.
    echo   Recover: tools\_factory_recover.bat  ^(pins + reconfigure + full rebuild^)
    echo ============================================================
    rmdir /s /q "%LOCKDIR%" 2>nul
    exit /b 98
)

REM §806: ww_donor_disc_package rides the canonical build — the mod DLL/.dusk
REM otherwise never rebuilds (its target is not a dusklight dependency), and a
REM stale donor-disc reader silently serves yesterday's roster.
ninja -j8 -C "%~dp0build\windows-msvc-relwithdebinfo" dusklight ww_donor_disc_package
set "BUILDERR=%ERRORLEVEL%"

REM Always release the lock - success, failure, or compile error.
rmdir /s /q "%LOCKDIR%" 2>nul

if not "%BUILDERR%"=="0" exit /b 1
for %%F in ("%~dp0build\windows-msvc-relwithdebinfo\dusklight.exe") do set EXESIZE=%%~zF
if "%EXESIZE%"=="0" (
    echo ERROR: dusklight.exe is 0 bytes - link failed or AV blocked the file. Try rebuild.bat
    exit /b 1
)
