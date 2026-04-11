@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /d C:\develop\ngircd\build
nmake -f CMakeFiles\server_app.dir\build.make CMakeFiles/server_app.dir/src/app/server_app/server_app.c.obj > C:\develop\ngircd\compile_out.txt 2>&1
echo COMPILE_EXIT:%ERRORLEVEL% >> C:\develop\ngircd\compile_out.txt
nmake -f CMakeFiles\server_app.dir\build.make server_app.dll >> C:\develop\ngircd\compile_out.txt 2>&1
echo LINK_EXIT:%ERRORLEVEL% >> C:\develop\ngircd\compile_out.txt
