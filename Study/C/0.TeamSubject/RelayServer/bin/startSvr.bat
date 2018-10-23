@echo off
setlocal
rem ===========================================================
call paths.bat
call param.bat
rem ===========================================================
cd %BIN_DIR%
cls

%EXE_OUT_PATH%/%SVR_EXE_NAME% ^
%SVR_SERVER_IP% %SVR_SERVER_PORT% ^
%SVR_RELAY_PORT% %SVR_CLI_SOC_TIMEOUT_DELAY% ^
%SVR_IN_LOG_FILE_PATH%

cd %RES_DIR%/output
start.
cd %BIN_DIR%
rem ===========================================================
endlocal
rem ===========================================================