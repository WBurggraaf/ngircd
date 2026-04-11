@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /d C:\develop\ngircd\build
ctest --output-on-failure -C Debug > C:\develop\ngircd\test_out.txt 2>&1
echo CTEST_EXIT:%ERRORLEVEL% >> C:\develop\ngircd\test_out.txt
