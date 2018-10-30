@echo off
setlocal
rem ================================================================
set PWD_DIR=%cd%
set SRC_DIR=%PWD_DIR%/../src
set INC_DIR=%PWD_DIR%/../include
set LIB_DIR=%PWD_DIR%/../lib
rem ================================================================
gcc -o %PWD_DIR%/openssl_test.exe %SRC_DIR%/openssl_main.c ^
-I%INC_DIR% -I%INC_DIR%/ncoder ^
-L%LIB_DIR% -lcommrblib -lncoder -llibcrypto -llibssl -lws2_32
rem ================================================================
echo "Compile Done!"
endlocal