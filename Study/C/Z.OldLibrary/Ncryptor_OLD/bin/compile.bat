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
%C01%.c ^
%NLC01%.c %NLC02%.c %NLC03%.c %NLC04%.c %NLC05%.c %NLC06%.c %NLC07%.c %NLC08%.c ^
%CLC01%.c ^
-I %SRC_DIR% -I %INC_DIR_NCRYPT_HOME%
cd %BIN_DIR%
rem ===========================================================
echo "[%EXE_NAME%] 컴파일 수행 완료. (Code:%ERRORLEVEL%)"
rem ===========================================================
rem [ CMD End setting ]
endlocal
rem ===========================================================