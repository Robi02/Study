@echo off

cd src

SETLOCAL

SET CJ1=com/ksnet/util/Global.java
SET CJ2=com/ksnet/util/Logger.java
SET CJ3=com/ksnet/util/TransceiveLogger.java

SET RJ1=com/ksnet/main/server/RelayServerMain.java

SET RELAY_JAR_NAME=../RelayServer.jar

javac %CJ1% %CJ2% %CJ3% %RJ1%
jar -cfe %RELAY_JAR_NAME% com.ksnet.main.server.RelayServerMain com\ksnet\main\server\*.class com\ksnet\util\*.class

ENDLOCAL

cd ..