@echo off
rem Run from an x86 Visual Studio developer command prompt. No Lua/game needed.
setlocal
cd /d "%~dp0.."
if not exist tests\build mkdir tests\build
cl /nologo /EHsc /W4 /std:c++14 /Fe:tests\build\aob.exe /Fo:tests\build\ AOB.cpp tests\aob_test.cpp /link /SECTION:.aobexec,ERW
if errorlevel 1 exit /b 1
tests\build\aob.exe
exit /b %errorlevel%
