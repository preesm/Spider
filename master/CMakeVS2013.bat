@echo off
mkdir bin
cd bin
cmake ..\ -G "Visual Studio 12" -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE
pause