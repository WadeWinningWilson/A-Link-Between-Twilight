@echo off
rem ALBT companion mods -- double-click launcher for install.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0install.ps1"
if errorlevel 1 pause
