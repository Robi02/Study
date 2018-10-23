@echo off
rem ===========================================================
rem [ CMD Setting ]
setlocal
rem ===========================================================
rem [ Load batches ]
call paths.bat
rem ===========================================================
rem [ Compile and build .exe ]
cd %SRC_DIR%
gcc -o %EXE_OUT_PATH%/%EXE_NAME% ^
%C01% %C02% %EC01% %EC02% %CL01% ^
-I %SRC_DIR% -I %LIB_ENC_DIR%
cd %BIN_DIR%
rem ===========================================================
echo "[%EXE_NAME%] 컴파일 수행 완료. (Code:%ERRORLEVEL%)"
rem ===========================================================
rem [ CMD End setting ]
endlocal
rem ===========================================================