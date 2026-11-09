@echo off

if exist "PercRipper.exe" (
    PercRipper.exe
) else if exist "cmake-build-release\PercRipper\PercRipper.exe" (
    cmake-build-release\PercRipper\PercRipper.exe
) else (
    cmake-build-debug\PercRipper\PercRipper.exe
)

pause
