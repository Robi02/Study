@echo off
setlocal
rem ===========================================================
call paths.bat
call param.bat
rem ===========================================================
cls

cd %BIN_DIR%
start startCli.bat

cd %BIN_DIR%2
start startCli.bat

cd %BIN_DIR%3
start startCli.bat

cd %BIN_DIR%4
start startCli.bat

cd %BIN_DIR%5
start startCli.bat

cd %RES_DIR%/output
start.
cd %BIN_DIR%
rem ===========================================================
endlocal
rem ===========================================================