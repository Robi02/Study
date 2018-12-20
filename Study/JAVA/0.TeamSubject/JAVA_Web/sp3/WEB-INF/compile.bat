@echo off

setlocal

set CP1=C:/Program Files/apache-tomcat-9.0.7/lib/servlet-api.jar
set CP2=C:/Program Files/apache-tomcat-9.0.7/lib/KsCommonLib.jar

set JS1=src/com/ksnet/AjaxFcexService.java
set JS2=src/com/ksnet/IframeFcexService.java
set JS3=src/com/ksnet/JsonpFcexService.java

javac -cp "%CP1%;%CP2%" ^
      -d classes ^
      %JS1% %JS2% %JS3%
	  
endlocal