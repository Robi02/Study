rem ===========================================================
rem [ PATH ]
set BIN_DIR=%cd%
set PRJ_DIR=%BIN_DIR%/..
set SRC_DIR=%PRJ_DIR%/src
set RES_DIR=%PRJ_DIR%/res
rem ===========================================================
rem [ OUTPUT PATH ]
set EXE_OUT_PATH=%BIN_DIR%
set CLI_EXE_NAME=HanaClient.exe
set SVR_EXE_NAME=HanaRelayServer.exe
rem ===========================================================
rem [ COMMON_CODE ]
set C01=commonlib.c
set C02=msgfileio.c
rem ===========================================================
rem [ CLI_CODE ]
set CC01=cli/climain.c
set CC02=cli/clirecord.c
set CC03=cli/climsgsocket.c
set CC04=cli/cliglobal.c
rem ===========================================================
rem [ SVR_CODE ]
set SC01=svr/svrmain.c
set SC02=svr/svrglobal.c
set SC03=svr/svrsocket.c
rem ===========================================================
rem [ COMMON_LIB ]
set L01=-lws2_32
rem ===========================================================