@echo off
setlocal

set INCLUDE_DIR=..\..\include
set LIB_DIR=..\..\libs

set SRC_FILES=main.cpp sim_db.cpp phase_foundation.cpp phase_population.cpp phase_simulation.cpp phase_crime.cpp phase_selection.cpp phase_evidence.cpp phase_extraction.cpp phase_blanks.cpp case_output.cpp

echo [BUILD] Compiling case generator...
cl -nologo -std:c++17 -EHsc -MD -Zi -I%INCLUDE_DIR% -Foobj/ %SRC_FILES% -Fe:casegen.exe -link -LIBPATH:%LIB_DIR% sqlite3.lib

if %ERRORLEVEL% NEQ 0 (
    echo [BUILD] Compilation failed!
    exit /b 1
)

echo [BUILD] Success! Output: casegen.exe
