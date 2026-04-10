@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake -B build -S . -DBUILD_TESTING=ON -G "NMake Makefiles"
if %ERRORLEVEL% neq 0 (
    echo CMAKE CONFIGURE FAILED with code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)
cmake --build build
echo BUILD EXIT CODE: %ERRORLEVEL%
