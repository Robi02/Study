@echo off

cd src

SETLOCAL

SET CJ1=com/ksnet/util/Global.java
SET CJ2=com/ksnet/util/Logger.java
SET CJ3=com/ksnet/util/TransceiveLogger.java
SET CJ4=com/ksnet/net/Record.java
SET CJ5=com/ksnet/net/RecordConverter.java
SET CJ6=com/ksnet/net/RecordTransceiver.java
SET CJ7=com/ksnet/net/RecordPrinter.java
SET CJ8=com/ksnet/net/Attribute.java
SET CJ9=com/ksnet/net/AttributeManager.java
SET CJ10=com/ksnet/net/KsFileReader.java
SET CJ11=com/ksnet/net/KsFileWriter.java
SET CJ12=com/ksnet/main/client/ClientMain.java

SET CLIENT_JAR_NAME=../Client.jar

javac %CJ1% %CJ2% %CJ3% %CJ4% %CJ5% %CJ6% %CJ7% %CJ8% %CJ9% %CJ10% %CJ11% %CJ12%
jar -cf %CLIENT_JAR_NAME% com\ksnet\main\client\*.class com\ksnet\net\*.class com\ksnet\util\*.class

ENDLOCAL

cd ..