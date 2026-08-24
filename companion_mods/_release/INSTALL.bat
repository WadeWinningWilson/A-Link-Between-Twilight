@echo off
rem ALBT companion mods -- copies packs into portable data\ and AppData
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0install.ps1"
if errorlevel 1 pause
