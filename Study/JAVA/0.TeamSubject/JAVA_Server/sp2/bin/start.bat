@echo off
setlocal
rem ====================================
call local.bat
set LOGGING_PROP=-Djava.util.logging.config.file=%CONF_DIR%/logging.propertise
rem ====================================
set K01=TEST_KEY_1
set V01=TEST_VAL_1
set K02=TEST_KEY_2
set V02=TEST_VAL_2
set K03=SERVER_SOCKET_BIND_PORT
set V03=9999
set K04=HANA_FCEXT_URL
set V04=http://fx.kebhana.com/fxportal/jsp/RS/DEPLOY_EXRATE/fxrate_all.html
rem ====================================
cd %SRC_DIR%

java %LOGGING_PROP% -jar %JAR_NAME% ^
     %K01% %V01% ^
     %K02% %V02% ^
     %K03% %V03% ^
     %K04% %V04%

cd %BIN_DIR%
rem ====================================
endlocal