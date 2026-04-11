@echo off
echo STEP1 > C:\develop\ngircd\rb2_out.txt
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >> C:\develop\ngircd\rb2_out.txt 2>&1
echo STEP2 >> C:\develop\ngircd\rb2_out.txt
cmake --build C:\develop\ngircd\build >> C:\develop\ngircd\rb2_out.txt 2>&1
echo BUILD_EXIT:%ERRORLEVEL% >> C:\develop\ngircd\rb2_out.txt
