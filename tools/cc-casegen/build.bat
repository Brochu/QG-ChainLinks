@echo off
setlocal

set ENGINE_DIR=engine
set SRC_DIR=src
set ENGINE_INCLUDE_DIR=..\..\include
set LOCAL_INCLUDE_DIR=.\include
set LIB_DIR=.

rem Engine source files
set ENGINE_SRC=%ENGINE_DIR%\qg_mem.cpp %ENGINE_DIR%\qg_rand.cpp %ENGINE_DIR%\qg_parse.cpp %ENGINE_DIR%\qg_conf.cpp %ENGINE_DIR%\qg_bus.cpp

rem Case generator source files
set SRC_FILES=%SRC_DIR%\main.cpp %SRC_DIR%\world_setup.cpp %SRC_DIR%\sim_db.cpp %SRC_DIR%\name_cycle.cpp %SRC_DIR%\name_gen.cpp

rem All source files
set ALL_SRC=%SRC_FILES% %ENGINE_SRC%

if not exist obj mkdir obj

echo [BUILD] Compiling case generator...
cl -nologo -std:c++17 -EHsc -MD -Zi -I%ENGINE_DIR% -I%ENGINE_INCLUDE_DIR% -I%LOCAL_INCLUDE_DIR% -Foobj/ %ALL_SRC% -Fe:casegen.exe -link -LIBPATH:%LIB_DIR% SDL3.lib sqlite3.lib

if %ERRORLEVEL% NEQ 0 (
    echo [BUILD] Compilation failed!
    exit /b 1
)

echo [BUILD] Success! Output: casegen.exe
