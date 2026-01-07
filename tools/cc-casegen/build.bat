@echo off

cl -nologo -c -std:c++17 -EHsc -MD -TP -Zi casegen-main.cpp
link -nologo casegen-main.obj -out:app.exe
