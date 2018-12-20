<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@ taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<%@ page language="java" contentType="text/html; charset=EUC-KR" pageEncoding="EUC-KR"%>
<html>
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=EUC-KR">
    <!-- Bootstrap -->
    <link href="/resources/css/bootstrap.min.css" rel="stylesheet">
    <title>KSnet FCEX Service</title>
</head>
<body>
    <div class="d-flex justify-content-center align-items-center" style="height:500px;">
		<table>
			<!-- 타이틀 -->
			<tr>
				<td colspan="3" align="center"><br><h1>KSnet 환율 변환</h1><br></td>
			</tr>
			<tr>
				<!-- 원화 가격 -->
				<td align="left"><div class="form-group">
					<label for="wonPrice">원화 가격 (￦)</label>
					<input type="text" style="text-align: right;" class="form-control" name="wonPrice" id="wonPrice" max="2147483647" value="${wonPrice}">
				</div></td>
				<!-- 외화 종류 콤보박스 -->
				<td align="center"><div class="form-group">
					<label for="excType">외화 종류</label>
					<select class="form-control" name="excType" id="excType">
						<c:forEach var="i" begin="0" end="${fn:length(CurrencyCodename) - 1}" step="1">
							<c:set var="codename" value="${CurrencyCodename[i]}"/>
							<c:set var="codeSymbol" value="${CurrencyCodeSymbol[i]}"/>
							<option value="${codename}" <c:choose>
								<c:when test="${empty excType}">
									<c:if test="${i == 0}">selected</c:if>
								</c:when>
								<c:otherwise>
									<c:if test="${excType == CurrencyCodename[i]}">selected</c:if>
								</c:otherwise>
							</c:choose>>${codename}(${codeSymbol})</option>
						</c:forEach>
					</select>
				</div></td>
				<!-- 외화 환전결과 -->
				<td align="right"><div class="form-group">
					<label for="excPrice">외화 가격</label>
					<input type="text" style="text-align: right;" class="form-control" name="excPrice" id="excPrice" value="${excPrice}" readonly>
				</div></td>
			</tr>
			<!-- 환율 변환 버튼 -->
			<tr>
				<td colspan="3" align="center"><input type="button" class="btn btn-info btn-block" value="환율 변환" onclick="ajaxFcexResult()"></td>
			</tr>
			<!-- 반올림 체크박스 -->
			<tr>
				<td colspan="3" align="right" style="font-size: 12px;">
					<div class="radio">
						<c:set var="RoundOptionCodename" value="${['올림', '반올림', '내림']}"/>
						계산 결과를 (
						<c:forEach var="i" begin="0" end="${fn:length(RoundOptions) - 1}" step="1">
							<c:set var="roundOpsVal" value="${RoundOptions[i]}"/>
							<label><input type="radio" name="roundOps" value="${roundOpsVal}" <c:choose>
								<c:when test="${empty roundOps}">
									<c:if test="${i == 0}">checked</c:if>
								</c:when>
								<c:otherwise>
									<c:if test="${roundOps == roundOpsVal}">checked</c:if>
								</c:otherwise>
							</c:choose>>${RoundOptionCodename[i]}</option>
						</c:forEach>)
					</div>
				</td>
			</tr>
			<!-- 오류 메시지! -->
			<tr id="errorMsgRow" <c:if test="${empty errorMsg}">style="display: none;"</c:if>><td colspan="3"><div class="alert alert-danger">
				<strong>오류!</strong> <span id="errorMsg">${errorMsg}</span>
			</div></td></tr>
		</table>
    </div>
    <!-- jQuery (necessary for Bootstrap's JavaScript plugins) -->
    <script src="/resources/js/jquery331.js"></script>
    <!-- Include all compiled plugins (below), or include individual files as needed -->
    <script src="/resources/js/bootstrap.min.js"></script>
	<!-- JavaScript for ajax -->
	<script src="view/ajax/js/ajax.js"></script>
</body>
</html>