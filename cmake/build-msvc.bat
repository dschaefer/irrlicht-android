@echo off
REM Build script for Visual C++
REM you need CMake in your path as well as the Visual C++ env vars
REM The Visual C++ env vars are handled in CDT automatically. CMake is not.

if exist msvc goto checkmk
mkdir msvc

:checkmk
cd msvc
if exist Makefile goto dobuild
@echo on
cmake ..\..

:dobuild
@echo on
jom %*
