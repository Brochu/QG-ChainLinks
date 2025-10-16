@echo off

cl -nologo -c -std:c++17 -EHsc -MD -TP -Zi ./gen-main.cpp
link -nologo gen-main.obj -out:app.exe
