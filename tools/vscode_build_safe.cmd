@echo off
call "%~dp0vscode_msvc_env.cmd"
if errorlevel 1 exit /b %errorlevel%
python "%~dp0build_safe.py" %*
exit /b %errorlevel%