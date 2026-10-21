@echo off
setlocal
where cl >nul 2>nul
if errorlevel 1 (
  set "TASK_VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
  if not exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
    echo Install Visual Studio Build Tools with Desktop development with C++.
    exit /b 1
  )
  for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "TASK_VS=%%i"
)
if defined TASK_VS call "%TASK_VS%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b %errorlevel%
if defined TASK_VS set "PATH=%TASK_VS%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%TASK_VS%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
cmake -S "%~dp0." -B "%~dp0build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
if errorlevel 1 exit /b %errorlevel%
cmake --build "%~dp0build"
if errorlevel 1 exit /b %errorlevel%
ctest --test-dir "%~dp0build" --output-on-failure
exit /b %errorlevel%
