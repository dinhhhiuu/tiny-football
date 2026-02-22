@echo off
REM --- set SDL2 bin folder vào PATH tạm thời ---
set THIS_DIR=%~dp0
set PATH=%THIS_DIR%lib\SDL2\bin;%PATH%

REM --- chạy exe ---
"%THIS_DIR%tiny_football.exe"

pause
