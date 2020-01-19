@echo off

set brp="..\build-results"
set exe="3g-rebooter.exe"

set config=x32-debug
echo config: %config%...
copy /y "%~dp0%brp%\%config%\%exe%" "%~dp0%config%\" /b
echo.

set config=x32-release
echo config: %config%...
copy /y "%~dp0%brp%\%config%\%exe%" "%~dp0%config%\" /b
echo.

set config=x64-debug
echo config: %config%...
copy /y "%~dp0%brp%\%config%\%exe%" "%~dp0%config%\" /b
echo.

set config=x64-release
echo config: %config%...
copy /y "%~dp0%brp%\%config%\%exe%" "%~dp0%config%\" /b
echo.
