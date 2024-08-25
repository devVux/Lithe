@echo off
setlocal enabledelayedexpansion

:: Default values
set CONFIGURE_PRESET=
set BUILD_PRESET=

:: Parse named parameters
:parse_args
if "%~1"=="" goto end_parse
if "%~1"=="-c" (
    set CONFIGURE_PRESET=%2
    shift
    shift
) else if "%~1"=="-b" (
    set BUILD_PRESET=%2
    shift
    shift
) else (
    echo Invalid option %~1
    exit /b 1
)
goto parse_args

:end_parse

:: Check if presets are set
if "%CONFIGURE_PRESET%"=="" (
    echo Configure preset is not specified
    exit /b 1
)
if "%BUILD_PRESET%"=="" (
    echo Build preset is not specified
    exit /b 1
)


:: Set up the Visual Studio environment
echo Setting up Visual Studio environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

echo Running CMake configuration with preset %CONFIGURE_PRESET%...
cmake --preset %CONFIGURE_PRESET%

echo Building project with preset %BUILD_PRESET%...
cmake --build --preset %BUILD_PRESET%

endlocal
