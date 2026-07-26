@echo off
set "DIR=%AppData%\TwilitRealm\Dusklight"
echo === before:
dir /b "%DIR%\dawn_cache.db*" "%DIR%\pipeline_cache.db*" 2>nul
del /q "%DIR%\dawn_cache.db*" 2>nul
del /q "%DIR%\pipeline_cache.db*" 2>nul
echo === after:
dir /b "%DIR%\dawn_cache.db*" "%DIR%\pipeline_cache.db*" 2>nul
echo done
