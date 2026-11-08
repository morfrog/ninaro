@echo off
rem Builds the golden-game harness with MSVC and writes the golden games
rem used by the web port's tests. Run from anywhere:
rem   Tools\golden\build.bat
rem Needs Visual Studio's C++ tools (vcvars64.bat) and Python.
setlocal
set HERE=%~dp0
set LEGACY=%HERE%..\..\Legacy
set BUILD=%HERE%build
set OUT=%HERE%..\..\WebApp\src\core\__golden__\games.json

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if "%VCVARS%"=="" for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find VC\Auxiliary\Build\vcvars64.bat`) do set VCVARS=%%i
if "%VCVARS%"=="" (echo Visual Studio C++ tools not found & exit /b 1)
rem vcvars64 may print a harmless "vswhere.exe is not recognized" message.
call "%VCVARS%" >nul 2>&1 || exit /b 1

if not exist "%BUILD%" mkdir "%BUILD%"
python "%HERE%extract.py" "%LEGACY%" "%BUILD%\ai_extracted.c" || exit /b 1

rem /J: char is unsigned, as in Watcom C. /TC: compile as C.
cl /nologo /TC /J /W2 /O2 /I "%LEGACY%" /I "%HERE%." /Fo"%BUILD%\\" /Fe"%BUILD%\harness.exe" ^
  "%HERE%harness.c" "%HERE%stubs.c" "%BUILD%\ai_extracted.c" ^
  "%LEGACY%\GRID.C" "%LEGACY%\CONNECTN.C" "%LEGACY%\PLAYER.C" "%LEGACY%\BINFILE.C" || exit /b 1

for %%F in ("%OUT%") do if not exist "%%~dpF" mkdir "%%~dpF"
"%BUILD%\harness.exe" "%LEGACY%\NINARO.TTL" > "%OUT%" || exit /b 1
echo Wrote %OUT%
