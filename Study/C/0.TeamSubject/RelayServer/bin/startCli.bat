@echo off
setlocal
rem ===========================================================
call paths.bat
call param.bat
rem ===========================================================
cd %BIN_DIR%
cls

%EXE_OUT_PATH%/%CLI_EXE_NAME% ^
%FB_PARENT_COMP_NAME% %FB_PARENT_COMP_CODE% ^
%FB_PARENT_BANK_CODE_2% %FB_PARENT_BANK_CODE_3% ^
%FB_PARENT_ACCOUNT_NUMB% ^
%FB_DEPOSIT_BANK_CODE_2% %FB_DEPOSIT_BANK_CODE_3% ^
%SERVER_IP% %SERVER_PORT% ^
%IN_MSG_FILE_PATH% %OUT_MSG_FILE_PATH% %OUT_LOG_FILE_PATH% ^
%REUSABLE_SOCKET% %RECONNECT_TRY_CNT% ^
%AVG_SEND_SPEED_PER_SEC% %MAX_SEND_SPEED_PER_SEC%

cd %RES_DIR%/output
start.
cd %BIN_DIR%
rem ===========================================================
endlocal
rem ===========================================================