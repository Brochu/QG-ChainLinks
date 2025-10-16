@echo off

cl -nologo -c -std:c++17 -EHsc -MD -TP -Zi -I ../../include ./ink-test-main.cpp
link -nologo ink-test-main.obj ../../libs/inkle/inkcpp.lib ../../libs/inkle/inkcpp_compiler.lib -out:app.exe
