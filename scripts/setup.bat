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

:: Set up the Visual Studio environment
echo Setting up Visual Studio environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

:: Configure the project using CMake preset
echo Configuring the project with CMake...
cmake .. --preset windows-base

:: Build the project using CMake preset
echo Building the project with CMake...
cmake --build ../out/build/windows-base --config Debug

endlocal
