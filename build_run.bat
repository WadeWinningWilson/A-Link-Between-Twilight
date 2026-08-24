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

REM ---------------------------------------------------------------------------
REM B1 SEAM GATE (roadmap Phase B, Bridge). Baseline-diff + ownership classifier,
REM IN THE TOOLCHAIN rather than behind a role, so it cannot be "forgotten to run".
REM Blocks on exactly two things: a tracker row that was baselined and is now gone
REM with no retirement entry (the deletion bypass), and a changed WW-layer file
REM whose ownership category nobody has classified.
REM
REM Its refusals name FILE + WHY + FIX (B2) — a failure a builder cannot act on
REM trains them to bypass the gate, and a bypassed gate protects nothing.
REM
REM ESCAPE HATCH, deliberate and visible: set DUSK_SKIP_SEAM_GATE=1 to proceed.
REM It prints a loud line when used, because an ADVERTISED bypass that leaves a
REM trace beats an unadvertised one that gets added under deadline and never
REM removed. If this gate is ever wrong, skip it and say so on CALLS.
REM ---------------------------------------------------------------------------
if not "%DUSK_SKIP_SEAM_GATE%"=="1" (
    python "%~dp0tools\foundry\seam_gate.py" check
    if errorlevel 1 (
        echo.
        echo   BUILD REFUSED by the B1 seam gate ^(above^). Each problem names its fix.
        echo   Override for one build: set DUSK_SKIP_SEAM_GATE=1
        rmdir /s /q "%LOCKDIR%" 2>nul
        exit /b 97
    )
) else (
    echo [seam-gate] SKIPPED via DUSK_SKIP_SEAM_GATE=1 - row/ownership checks did NOT run.
)

REM CMakeLists is the graph. Plugin is OFF there, so this is dusklight only —
REM do not name ww_donor_disc_package; ninja would fail on a target that does
REM not exist. Plugin work is standalone/ in the WW repo.
ninja -j8 -C "%~dp0build\windows-msvc-relwithdebinfo" dusklight
set "BUILDERR=%ERRORLEVEL%"

REM Always release the lock - success, failure, or compile error.
rmdir /s /q "%LOCKDIR%" 2>nul

if not "%BUILDERR%"=="0" exit /b 1
for %%F in ("%~dp0build\windows-msvc-relwithdebinfo\dusklight.exe") do set EXESIZE=%%~zF
if "%EXESIZE%"=="0" (
    echo ERROR: dusklight.exe is 0 bytes - link failed or AV blocked the file. Try rebuild.bat
    exit /b 1
)

REM ===========================================================================
REM PDB ARCHIVE (tale S945/S968, user-ruled in with a retention cap).
REM MSVC keeps the PDB GUID stable across incremental links and bumps the AGE,
REM rewriting dusklight.pdb IN PLACE - so a crash log from the previous build
REM becomes unsymbolicatable the moment the NEXT link finishes. That is exactly
REM how the 08-12 crash was lost: nobody deleted its PDB, the next build
REM overwrote it. This must run AFTER a successful link and BEFORE the next one.
REM Prunes to the newest 10 (~2.3 GB ceiling; each PDB is ~234 MB) and keeps the
REM index row for pruned builds so they report PRUNED rather than vanishing.
REM Never fails the build: archiving is diagnostics, not a gate.
REM ===========================================================================
python "%~dp0tools\foundry\build_ledger.py" record || echo   (build ledger skipped - non-fatal)
python "%~dp0tools\foundry\build_identity.py" archive || echo   (PDB archive skipped - non-fatal)

