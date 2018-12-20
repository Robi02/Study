<%@ page language="java" contentType="text/html; charset=EUC-KR" pageEncoding="EUC-KR"%>
<%@ page import="com.ksnet.main.client.*" %>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=EUC-KR">
    <title>index</title>
</head>
<body>
    <%
            String realPath = application.getRealPath("/");
            String[] args = {
            realPath + "res/1)송신파일_35350081.180404104958",
            realPath + "res/output/1)송신파일_35350081.180404104958.rpy",
            realPath + "res/config/attrConfig.txt",
			realPath + "res/output/Client.out",
            "127.0.0.1",
            "9999",
            "039",
            "7770011",
            "86088800173",
            "35350081.180404104958",
            "0",
            "ＡＮＰ물품대금　　　",
            "FALSE",
            "2",
            "10000",
            "5",
            "5000",
            "15",
            "SERVICE"
        };
        
        ClientMain.main(args);
    %>
    <h1>Hello World!</h1>
</body>
</html>