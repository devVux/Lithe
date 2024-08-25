@echo off
setlocal

:: Set the directory for vcpkg
set VCPKG_DIR=..\vendor

:: Check if vcpkg exists, if not, bootstrap it
if not exist "%VCPKG_DIR%\vcpkg\vcpkg.exe" (
    echo Bootstrapping vcpkg...
    "%VCPKG_DIR%\bootstrap-vcpkg.bat"
)

:: Install dependencies using vcpkg
echo Installing dependencies with vcpkg...
"%VCPKG_DIR%\vcpkg\vcpkg.exe" install

endlocal
