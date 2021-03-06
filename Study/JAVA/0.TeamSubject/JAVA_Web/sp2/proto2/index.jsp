<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@ page language="java" contentType="text/html; charset=EUC-KR" pageEncoding="EUC-KR"%>
<%@ page import="java.io.BufferedReader"%>
<%@ page import="java.io.InputStreamReader"%>
<%@ page import="java.io.OutputStream"%>
<%@ page import="java.math.BigDecimal"%>
<%@ page import="java.net.HttpURLConnection"%>
<%@ page import="java.net.InetSocketAddress"%>
<%@ page import="java.net.URL"%>
<%@ page import="java.nio.ByteBuffer"%>
<%@ page import="java.nio.channels.SocketChannel"%>
<%@ page import="java.util.ArrayList"%>
<%@ page import="java.util.concurrent.ConcurrentHashMap"%> <%-- Thread-safe HashMap --%>
<%@ page import="java.util.List"%>
<%@ page import="java.util.LinkedList"%>
<%@ page import="java.util.Map"%>
<%@ page import="java.util.Map.Entry"%>
<%@ page import="java.util.regex.Matcher"%>
<%@ page import="java.util.regex.Pattern"%>
<%@ page import="com.ksnet.KsCommonLib"%>
<html>
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=EUC-KR">
    <!-- Bootstrap -->
    <link href="css/bootstrap.min.css" rel="stylesheet">
    <title>KSnet FCEX Service</title>
</head>
<%!
    //--- [Changeable Constance Begin] ---//
	//
    public static final String[] CurrencyCodeSymbol = {	"＄",	"￥",	"€",	"￥"	};	// Currency symbols
    public static final String[] CurrencyCodename   = {	"USD",	"JPY",	"EUR",	"CNY"	};	// Currency codenames
    public static final int ExchangePrecision = 2; 											// BigDecimal's floating point precision
	public static final long UpdateTermMillis = 20000; //(1000 * 60 * 60 * 1);						// Default update term is '1-hour' (ms * s * m * h)
	//
    //--- [Changeable Constance End] ---//

    private static Map<String, BigDecimal> fcexMap    = new ConcurrentHashMap<String, BigDecimal>();	// Thread-safe HashMap for currency exchange data
	private static Map<String, BigDecimal> fcexMapCpy = new ConcurrentHashMap<String, BigDecimal>();	// Thread-safe HashMap to work concurrency
	private static boolean fcexUpdating = false;														// While updating fcexMap, value must be 'true' otherwise 'false'
    private static boolean fcexCpyUpdating = false;														// While updating fcexCpyMap, value must be 'true' otherwise 'false'
	private static long nextFcexMapUpdateableTime = 0;													// The time that next updateable

    public static synchronized void updateFcex() { // 'FCEX' (Foreign Currency EXchange hashmap)
		long curTime = System.currentTimeMillis();

        if (curTime > nextFcexMapUpdateableTime) {
			nextFcexMapUpdateableTime = curTime + UpdateTermMillis; // next updateable time is after 1-hour from curTime
			fcexUpdating = true;
			fcexMap.clear();
				
			new Thread(()->{ // execute main logic with thread to prevent concurrency bottleneck
				boolean updateSuccess = false;
				
				// [1] Request hana-bank FCEX data from server to update 'Map<String, BigDecimal> fcexMap'
				try {
					System.out.println(String.format("fcexMap update begin! (%d)", System.currentTimeMillis())); // test
					byte[] sendByte = KsCommonLib.makeSendByte("ufcex", null);
					ByteBuffer writeBuf = ByteBuffer.allocate(sendByte.length);
					
					writeBuf.put(sendByte);
					writeBuf.clear();
					
					InetSocketAddress svrAddr = new InetSocketAddress("127.0.0.1", 9999);
					SocketChannel cliSocChannel = SocketChannel.open(svrAddr);
					cliSocChannel.write(writeBuf);
					
					ByteBuffer readBuf = ByteBuffer.allocate(5096);
					byte[] readByte = null;
					int readLen = -1;
					
					if ((readLen = cliSocChannel.read(readBuf)) != -1) {
						readByte = new byte[readLen]; 
						System.arraycopy(readBuf.array(), 0, readByte, 0, readByte.length);
					}
					
					System.out.println(String.format("수신:\n%s", new String(readByte)));
					byte[] recvByte = KsCommonLib.makeRecvByte(readByte);
					recvByte = KsCommonLib.hexString2ByteAry(new String(recvByte));
					fcexMap = (ConcurrentHashMap<String, BigDecimal>)KsCommonLib.byteAry2Obj(recvByte);
					
					updateSuccess = true; // only way for fcexMap updating 'success'
					cliSocChannel.close();
					System.out.println(String.format("fcexMap update complete! (%d)", System.currentTimeMillis())); // test
					System.out.println("fcexMap: " + fcexMap.toString()); // test
				}
				catch (Exception e) {
					e.printStackTrace();
					nextFcexMapUpdateableTime = 0; // init next update time when exception thrown
					return;
				}
				
				fcexUpdating = false;
				
				// [2] Update 'Map<String, BigDecimal> fcexMapCpy' using copy data of fcexMap
				if (updateSuccess) {
					System.out.println(String.format("fcexMapCpy update begin! (%d)", System.currentTimeMillis())); // test
					fcexCpyUpdating = true;
					fcexMapCpy.clear();

					for (Entry<String, BigDecimal> entry : fcexMap.entrySet()) { // fcexMap deep copy
						fcexMapCpy.put(entry.getKey(), new BigDecimal(entry.getValue().toString()));
					}
					
					fcexCpyUpdating = false;
					System.out.println(String.format("fcexMapCpy update complete! (%d)", System.currentTimeMillis())); // test
					System.out.println("fcexMapCpy: " + fcexMapCpy.toString()); // test
				}
			}).start(); 
		}

        return;
    }

	// fcexMap is ConcurrentHashMap(thread-safe) and this method does not change static value. so this method does not need 'synchronized' keyword
    public BigDecimal getFcex(String key) throws Exception {
		if (fcexMap.size() > 0 && !fcexUpdating) {
			System.out.println("useFcexMap sz:" + fcexMap.size()); // test
			return fcexMap.get(key);
		}
		else if (fcexMapCpy.size() > 0 && !fcexCpyUpdating) {
			System.out.println("useCpyFcexMap sz:" + fcexMapCpy.size()); // test
			return fcexMapCpy.get(key);
		}
		else {
			System.out.println("throwException"); // test
			throw new Exception(" [환율정보를 갱신중입니다. 몇초 후 다시 시도해주세요.] ");
		}
    }

    public String getCurrencySymbolByCodename(String codename) {
        if (codename != null && codename.length() > 0) {
			for (int i = 0; i < CurrencyCodename.length; ++i) {
				if (codename.equals(CurrencyCodename[i])) {
					return CurrencyCodeSymbol[i];
				}
			}
		}

        return "";
    }
%>

<%
    // [1] When this page called, automatically update FCEX table every 'UpdateTermMillis'
    updateFcex();

    // [2] Update default or result UI
    request.setCharacterEncoding("EUC-KR");

	String errorMsg = "";
    String wonPriceParam   = request.getParameter("wonPrice");
    String excPriceParam   = request.getParameter("excPrice");
    String excTypeParam    = request.getParameter("excType");
	String useRoundUpParam = request.getParameter("useRoundUp");
	boolean useRoundUp     = (useRoundUpParam == null || !useRoundUpParam.equals("true") ? false : true); 
    boolean doExcWork      = true;
	BigDecimal zeroPirce   = new BigDecimal("0");
    BigDecimal wonPrice    = zeroPirce;
    BigDecimal excPrice    = zeroPirce;

    try {
		// [2-1] form parameter validation check
		double dbWonPrice = 0.00;
		if (wonPriceParam == null) {
			errorMsg += " [원화가격 입력] ";
			doExcWork = false;
		}
		else if (((wonPriceParam = wonPriceParam.replaceAll("[^0-9//.]", "")).length()) == 0) {
			errorMsg += " [올바른 원화가격 입력] ";
			doExcWork = false;
		}
		else if ((dbWonPrice = Double.parseDouble(wonPriceParam)) < 0.00) {
			errorMsg += " [0원 이상의 원화가격 입력] ";
			doExcWork = false;
		}
		else if (dbWonPrice > (double)Integer.MAX_VALUE) {
			errorMsg += " [한계 금액 초과] ";
			doExcWork = false;
		}
		else {
			wonPrice = new BigDecimal(wonPriceParam);
		}
		
		if (excTypeParam == null) {
			excTypeParam = CurrencyCodename[0];
		}
		else {
			boolean excContain = false;
			for (int i = 0; i < CurrencyCodename.length; ++i) {
				if (excTypeParam.equals(CurrencyCodename[i])) {
					excContain = true;
					break; 
				}
			}
			
			if (!excContain) {
				errorMsg += String.format(" [미지원 외화종류(%s)] ", excTypeParam);
				doExcWork = false;
			}
		}
		
		// [2-2] Exchange work
		if (doExcWork) {
			excPrice = wonPrice.divide(getFcex(excTypeParam), ExchangePrecision, useRoundUp ? BigDecimal.ROUND_HALF_UP : BigDecimal.ROUND_DOWN);
		}
    }
    catch (Exception e) {
        e.printStackTrace();
        errorMsg += String.format(" [예외 발생(%s)] ", e.getMessage());
		doExcWork = false;
    }
    finally {
		// [3] Update html UI
		wonPriceParam = String.format("%s", wonPrice.toString());
		excPriceParam = String.format("%s%s", excPrice.toString(), getCurrencySymbolByCodename(excTypeParam));
    }
%>

<body>
    <div class="d-flex justify-content-center align-items-center" style="height:500px;">
        <form action="./" method="POST">
            <table>
				<!-- 타이틀 -->
                <tr>
                    <td colspan="3" align="center"><br><h1>KSnet 환율 변환</h1><br></td>
                </tr>
                <tr>
					<!-- 원화 가격 -->
                    <td align="left"><div class="form-group">
                        <label for="wonPrice">원화 가격 (￦)</label>
                        <input type="text" style="text-align: right;" class="form-control" name="wonPrice" id="wonPrice" max="2147483647" value="<%=wonPriceParam%>">
                    </div></td>
					<!-- 외화 종류 콤보박스 -->
                    <td align="center"><div class="form-group">
                        <label for="excType">외화 종류</label>
                        <select class="form-control" name="excType" id="excType">
							<%
								for (int i = 0; i < CurrencyCodename.length; ++i) {
									String codename = CurrencyCodename[i];
									String codeSymbol = CurrencyCodeSymbol[i];
									out.println(String.format("<option value=\"%s\" %s>%s(%s)</option>", codename, (codename.equals(excTypeParam) ? "selected" : " "), codename, codeSymbol));
								}
							%>
                        </select>
                    </div></td>
					<!-- 외화 환전결과 -->
                    <td align="right"><div class="form-group">
                        <label for="excPrice">외화 가격</label>
                        <input type="text" style="text-align: right;" class="form-control" name="excPrice" id="excPrice" value="<%=excPriceParam%>" readonly>
                    </div></td>
                </tr>
				<!-- 환율 변환 버튼 -->
                <tr>
                    <td colspan="3" align="center"><input type="submit" class="btn btn-info btn-block" value="환율 변환"></td>
                </tr>
				<!-- 반올림 체크박스 -->
				<tr>
					<td colspan="3" align="right" style="font-size: 12px;">
						<label class="checkbox-inline">
						<%
							out.println(String.format("<input name=\"useRoundUp\" type=\"checkbox\" value=\"true\" %s> 소수점 %d번째 자리에서 반올림(v)/내림( )</label>", (useRoundUp ? "checked" : " "), (ExchangePrecision + 1)));
						%>
					</td>
				</tr>
				<!-- 오류 메시지 -->
                <%
                    if (!doExcWork) {
                        out.println("<tr><td colspan=\"3\"><div class=\"alert alert-danger\">");
                        out.println("<strong>오류!</strong> " + errorMsg);
                        out.println("</div></td></tr>");
                    }
                %>
            </table>
        </form>
    </div>

    <!-- jQuery (necessary for Bootstrap's JavaScript plugins) -->
    <script src="js/jquery331.js"></script>
    <!-- Include all compiled plugins (below), or include individual files as needed -->
    <script src="js/bootstrap.min.js"></script>
</body>
</html>