@echo off
setlocal

cd /d "%~dp0"

if exist legacy (
  echo legacy already exists. Nothing to do.
  exit /b 0
)

if not exist original (
  echo original folder not found.
  exit /b 1
)

move original legacy
if errorlevel 1 (
  echo Failed to rename original to legacy.
  exit /b 1
)

echo Renamed original to legacy.
