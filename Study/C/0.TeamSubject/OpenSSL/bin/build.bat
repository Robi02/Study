@echo off
setlocal
rem ================================================================
set PWD_DIR=%cd%
set SRC_DIR=%PWD_DIR%/../src
set INC_DIR=%PWD_DIR%/../include
set LIB_DIR=%PWD_DIR%/../lib
rem ================================================================
gcc -o %PWD_DIR%/openssl_block_test.exe %SRC_DIR%/openssl_block_main.c ^
-I%INC_DIR% -I%INC_DIR%/ncoder ^
-L%LIB_DIR% -lcommrblib -lncoder -llibcrypto -llibssl -lws2_32
rem ================================================================
gcc -o %PWD_DIR%/openssl_rsa_test.exe %SRC_DIR%/openssl_rsa_main.c ^
-I%INC_DIR% -I%INC_DIR%/ncoder ^
-L%LIB_DIR% -lcommrblib -lncoder -llibcrypto -llibssl -lws2_32
rem ================================================================
gcc -o %PWD_DIR%/openssl_hash_test.exe %SRC_DIR%/openssl_hash_main.c ^
-I%INC_DIR% -I%INC_DIR%/ncoder ^
-L%LIB_DIR% -lcommrblib -lncoder -llibcrypto -llibssl -lws2_32
rem ================================================================
gcc -o %PWD_DIR%/crc_test.exe %SRC_DIR%/crc_main.c ^
-I%INC_DIR% -I%INC_DIR%/ncoder ^
-L%LIB_DIR% -lcommrblib -lncoder
rem ================================================================
echo "Compile Done!"
endlocal