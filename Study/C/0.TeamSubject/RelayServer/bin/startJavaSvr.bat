@echo off
setlocal
rem ===========================================================
call paths.bat
rem ===========================================================
start
cd %BIN_DIR%/javaSvr2
mode con cols=200 lines=20
title FB0100Server v2
cls
call FB0100Server.bat
cd %BIN_DIR%
rem ===========================================================
endlocal
rem ===========================================================