@echo off
setlocal
cd /d "%~dp0"
if not exist build\ngircd.exe (
  echo build\ngircd.exe not found. Build first.
  exit /b 1
)
if not exist runtime mkdir runtime
if exist runtime\ngircd.pid del /f /q runtime\ngircd.pid
build\ngircd.exe --debug --config "%~dp0ngircd-win32.conf" --nodaemon > "%~dp0ngircd-root.log" 2>&1
