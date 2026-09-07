@echo off
set "FRAZIL_VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%FRAZIL_VSWHERE%" (
    echo FRAZIL: vswhere.exe was not found at the standard Visual Studio Installer location.
    exit /b 1
)

set "FRAZIL_VS_INSTALL="
for /f "usebackq delims=" %%I in (`"%FRAZIL_VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "FRAZIL_VS_INSTALL=%%I"
if not defined FRAZIL_VS_INSTALL (
    echo FRAZIL: an MSVC x64 Visual Studio installation was not found.
    exit /b 1
)
if not exist "%FRAZIL_VS_INSTALL%\Common7\Tools\VsDevCmd.bat" (
    echo FRAZIL: VsDevCmd.bat was not found under "%FRAZIL_VS_INSTALL%".
    exit /b 1
)

call "%FRAZIL_VS_INSTALL%\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
if errorlevel 1 (
    echo FRAZIL: VsDevCmd.bat failed to initialize the MSVC x64 environment.
    exit /b 1
)

set "FRAZIL_VSWHERE="
set "FRAZIL_VS_INSTALL="
echo FRAZIL: MSVC x64 developer environment is ready.