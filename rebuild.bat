@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cmake --build build > C:\develop\ngircd\build_out.txt 2>&1
echo BUILD_EXIT:%ERRORLEVEL% >> C:\develop\ngircd\build_out.txt
