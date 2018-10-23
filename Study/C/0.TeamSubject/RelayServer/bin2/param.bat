rem ===========================================================
rem [ CLIENT PARAM ]
set FB_PARENT_COMP_NAME=ＡＮＰ물품대금
set FB_PARENT_COMP_CODE=7770011
set FB_PARENT_BANK_CODE_2=39
set FB_PARENT_BANK_CODE_3=039
set FB_PARENT_ACCOUNT_NUMB=86088800173
set FB_DEPOSIT_BANK_CODE_2=03
set FB_DEPOSIT_BANK_CODE_3=003
rem ===========================================================
set SERVER_IP=127.0.0.1
set SERVER_PORT=7777
set IN_MSG_FILE_PATH=../res/input/35350081_2.180404104958
set OUT_MSG_FILE_PATH=../res/output/cli/35350081.180404104958_2.rpy
set OUT_LOG_FILE_PATH=../res/output/cli/fbclilog2.log
set REUSABLE_SOCKET=N
set RECONNECT_TRY_CNT=5
set AVG_SEND_SPEED_PER_SEC=50
set MAX_SEND_SPEED_PER_SEC=25
rem ===========================================================
rem [ RELAY SERVER PARAM ]
set SVR_SERVER_IP=127.0.0.1
set SVR_SERVER_PORT=19988
set SVR_RELAY_PORT=7777
set SVR_CLI_SOC_TIMEOUT_DELAY=2000
set SVR_IN_LOG_FILE_PATH=../res/output/svr/fbrsvrlog.log
rem ===========================================================