@ECHO OFF

SETLOCAL

SET JAVA_HOME=C:\Program Files\Java\jdk1.8.0_162

SET LISTEN_PORT=19988

"%JAVA_HOME%\bin\java.exe" -DCNPG_FB0100 -cp  .\fb0100_study.jar FB0100Server %LISTEN_PORT%> FB0100Server.out.txt
