@echo off
setlocal
rem ====================================
call local.bat
rem ====================================
set J01=com/ksnet/ServerMain.java
set J02=com/ksnet/Server.java
set J03=com/ksnet/Global.java
set L01=com/ksnet/KsCommonLib
rem ====================================
cd %SRC_DIR%

javac %J01% %J02% %J03%
jar -cfe %JAR_NAME% %JAR_ENTRY% ^
         com/ksnet/*.class

javac %L01%.java
jar -cf %LIB_NAME% ^
        %L01%.class
		 
cd %BIN_DIR%
rem ====================================
endlocal