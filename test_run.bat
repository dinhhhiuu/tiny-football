@echo off
echo Starting Tiny Football...
set PATH=%~dp0lib\SDL2\bin;C:\msys64\mingw64\bin;%PATH%
"%~dp0tiny_football.exe"
if errorlevel 1 (
    echo Program exited with error code: %errorlevel%
)
pause
