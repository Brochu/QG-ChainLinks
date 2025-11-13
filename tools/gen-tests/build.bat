@echo off

cl -nologo -c -std:c++17 -EHsc -MD -TP -Zi -I../../include/ ./gen-main.cpp
link -nologo gen-main.obj ../../libs/sqlite3.lib -debug -out:app.exe
