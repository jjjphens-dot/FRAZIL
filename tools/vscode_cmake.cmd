@echo off
call "%~dp0vscode_msvc_env.cmd"
if errorlevel 1 exit /b %errorlevel%
if "%~1"=="" (
    echo FRAZIL: configure preset is required.
    exit /b 2
)
cmake --preset "%~1"
exit /b %errorlevel%