<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<%@ page language="java" contentType="text/html; charset=EUC-KR" pageEncoding="EUC-KR"%>

<html>
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=EUC-KR">
    <title>index</title>
	<style>
		.td_input {
			width: 650px;
		}
		.td_submit {
			width: 880px;
			height: 30px;
		}
	</style>
</head>
<body>
	<%!
		String realPath = null;
	%>
	
	<%
		realPath = application.getRealPath("/");
	%>
	
	<form name="ClientWorkForm" action="work.jsp" method="post">
		<h1>Welcome to Client!</h1>
		<table>
			<tr>
				<td>RECORD_FILE_PATH: </td>
				<td><input name="RECORD_FILE_PATH" class="td_input" type="text" readonly value="<%=realPath%>res/1)송신파일_35350081.180404104958"/></td>
			</tr>
			<tr>
				<td>OUTPUT_FILE_PATH: </td>
				<td><input name="OUTPUT_FILE_PATH" class="td_input" type="text" readonly value="<%=realPath%>res/output/1)송신파일_35350081.180404104958.rpy"/></td>
			</tr>
			<tr>
				<td>ATTRIBUTE_CONFIG_FILE_PATH: </td>
				<td><input name="ATTRIBUTE_CONFIG_FILE_PATH" class="td_input" type="text" readonly value="<%=realPath%>res/config/attrConfig.txt"/></td>
			</tr>
			<tr>
				<td>OUTPUT_LOG_PATH: </td>
				<td><input name="OUTPUT_LOG_PATH" class="td_input" type="text" readonly value="<%=realPath%>res/output/Client.out"/></td>
			</tr>
			<tr>
				<td>FB_RELAY_IP: </td>
				<td><input name="FB_RELAY_IP" class="td_input" type="text" readonly value="127.0.0.1"/></td>
			</tr>
			<tr>
				<td>FB_RELAY_PORT: </td>
				<td><input name="FB_RELAY_PORT" class="td_input" type="text" readonly value="9999"/></td>
			</tr>
			<tr>
				<td>FB_PARENT_BANK_CODE_3: </td>
				<td><input name="FB_PARENT_BANK_CODE_3" class="td_input" type="text" readonly value="039"/></td>
			</tr>
			<tr>
				<td>FB_PARENT_COMP_CODE: </td>
				<td><input name="FB_PARENT_COMP_CODE" class="td_input" type="text" readonly value="7770011"/></td>
			</tr>
			<tr>
				<td>FB_PARENT_ACCOUNT_NUMB: </td>
				<td><input name="FB_PARENT_ACCOUNT_NUMB" class="td_input" type="text" readonly value="86088800173"/></td>
			</tr>
			<tr>
				<td>FB_REQ_FILE: </td>
				<td><input name="FB_REQ_FILE" class="td_input" type="text" readonly value="35350081.180404104958"/></td>
			</tr>
			<tr>
				<td>FB_MSG_NUMB_S: </td>
				<td><input name="FB_MSG_NUMB_S" class="td_input" type="text" readonly value="0"/></td>
			</tr>
			<tr>
				<td>FB_PARENT_COMP_NAME: </td>
				<td><input name="FB_PARENT_COMP_NAME" class="td_input" type="text" readonly value="ＡＮＰ물품대금　　　"/></td>
			</tr>
			<tr>
				<td>REUSABLE_SOCKET_MODE: </td>
				<td><input name="REUSABLE_SOCKET_MODE" class="td_input" type="text" readonly value="FALSE"/></td>
			</tr>
			<tr>
				<td>SOCKET_CNT: </td>
				<td><input name="SOCKET_CNT" class="td_input" type="text" readonly value="2"/></td>
			</tr>
			<tr>
				<td>SOCKET_THREAD_TIMEOUT: </td>
				<td><input name="SOCKET_THREAD_TIMEOUT" class="td_input" type="text" readonly value="10000"/></td>
			</tr>
			<tr>
				<td>RECORD_RESEND_MAX_TRY: </td>
				<td><input name="RECORD_RESEND_MAX_TRY" class="td_input" type="text" readonly value="5"/></td>
			</tr>
			<tr>
				<td>RECORD_RESEND_DELAY: </td>
				<td><input name="RECORD_RESEND_DELAY" class="td_input" type="text" readonly value="5000"/></td>
			</tr>
			<tr>
				<td>RECORD_TGT_SEND_PER_SEC: </td>
				<td><input name="RECORD_TGT_SEND_PER_SEC" class="td_input" type="text" readonly value="15"/></td>
			</tr>
			<tr>
				<td>LOG_LEVEL: </td>
				<td><input name="LOG_LEVEL" class="td_input" type="text" readonly value="SERVICE"/></td>
			</tr>
			<tr>
				<td colspan="2"><p></p></td>
			</tr>
			<tr>
				<td align="center" colspan="2"><input class="td_submit" type="submit" value="전송 수행"/></td>
			</tr>
		</table>
	</form>
</body>
</html>