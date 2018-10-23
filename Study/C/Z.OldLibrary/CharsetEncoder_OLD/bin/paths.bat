rem ===========================================================
rem [ PATH ]
set BIN_DIR=%cd%
set PRJ_DIR=%BIN_DIR%/..
set SRC_DIR=%PRJ_DIR%/src
set RES_DIR=%PRJ_DIR%/res
set LIB_ENC_DIR=%SRC_DIR%/encode
rem ===========================================================
rem [ OUTPUT PATH ]
set EXE_OUT_PATH=%BIN_DIR%
set EXE_NAME=encode.exe
rem ===========================================================
rem [ COMMON_CODE ]
set C01=main.c
set C02=encoder.c
rem [ ENC_CODE ]
set EC01=%LIB_ENC_DIR%/enc_hexa.c
set EC02=%LIB_ENC_DIR%/enc_base64.c
rem [ COMMON_LIBRARY ]
set CL01=rbmath.c
rem ===========================================================