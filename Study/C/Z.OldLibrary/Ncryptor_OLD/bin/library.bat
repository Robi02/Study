@echo off
rem ===========================================================
rem [ CMD Setting ]
setlocal
rem ===========================================================
rem [ Load batches ]
call paths.bat
rem ===========================================================
rem [ Compile ]
cd %SRC_DIR%
gcc -c ^
%NLC01%.c %NLC02%.c %NLC03%.c %NLC04%.c %NLC05%.c %NLC06%.c %NLC07%.c %NLC08%.c ^
%CLC01%.c ^
-I %SRC_DIR% -I %INC_DIR_NCRYPT_HOME%
rem [ Build Library ]
cd %SRC_DIR%
ar crv %LIB_OUT_PATH%/%LIB_NAME% ^
ncrypt.o ncrypt_common.o ncrypt_aria.o KISA_ARIA.o ^
ncrypt_hight.o KISA_HIGHT.o ncrypt_seed.o KISA_SEED.o ^
rbmath.o
del ^
ncrypt.o ncrypt_common.o ncrypt_aria.o KISA_ARIA.o ^
ncrypt_hight.o KISA_HIGHT.o ncrypt_seed.o KISA_SEED.o ^
rbmath.o
rem ===========================================================
echo "[%LIB_NAME%] 라이브러리 빌드 완료. (Code:%ERRORLEVEL%)"
rem ===========================================================
rem [ CMD End setting ]
endlocal
rem ===========================================================