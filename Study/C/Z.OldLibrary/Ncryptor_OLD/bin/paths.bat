rem ===========================================================
rem [ PATH ]
set BIN_DIR=%cd%
set PRJ_DIR=%BIN_DIR%/..
set SRC_DIR=%PRJ_DIR%/src
set RES_DIR=%PRJ_DIR%/res
set LIB_DIR=%PRJ_DIR%/lib
set INC_DIR_NCRYPT_HOME=%SRC_DIR%/ncrypt
set INC_DIR_NCRYPT_KISA=%INC_DIR_NCRYPT_HOME%/KISA
set INC_DIR_NCRYPT_ARIA=%INC_DIR_NCRYPT_KISA%/ARIA
set INC_DIR_NCRYPT_HIGHT=%INC_DIR_NCRYPT_KISA%/HIGHT
set INC_DIR_NCRYPT_SEED=%INC_DIR_NCRYPT_KISA%/SEED
rem ===========================================================
rem [ OUTPUT PATH ]
set EXE_OUT_PATH=%BIN_DIR%
set EXE_NAME=ncrypt.exe
set LIB_OUT_PATH=%LIB_DIR%
set LIB_NAME=ncrypt.lib
rem ===========================================================
rem [ COMMON_CODE ]
set C01=ncryptor_main
rem [ NCRYPT_LIB_CODE ]
set NLC01=%INC_DIR_NCRYPT_HOME%/ncrypt
set NLC02=%INC_DIR_NCRYPT_HOME%/ncrypt_common
set NLC03=%INC_DIR_NCRYPT_ARIA%/ncrypt_aria
set NLC04=%INC_DIR_NCRYPT_ARIA%/KISA_ARIA
set NLC05=%INC_DIR_NCRYPT_HIGHT%/ncrypt_hight
set NLC06=%INC_DIR_NCRYPT_HIGHT%/KISA_HIGHT
set NLC07=%INC_DIR_NCRYPT_SEED%/ncrypt_seed
set NLC08=%INC_DIR_NCRYPT_SEED%/KISA_SEED
rem [ COMMON_LIB_CODE ]
set CLC01=rbmath
rem ===========================================================