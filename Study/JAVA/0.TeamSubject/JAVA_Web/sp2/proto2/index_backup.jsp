<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@ page language="java" contentType="text/html; charset=EUC-KR" pageEncoding="EUC-KR"%>
<%@ page import="java.io.BufferedReader"%>
<%@ page import="java.io.InputStreamReader"%>
<%@ page import="java.io.OutputStream"%>
<%@ page import="java.math.BigDecimal"%>
<%@ page import="java.net.HttpURLConnection"%>
<%@ page import="java.net.URL"%>
<%@ page import="java.util.ArrayList"%>
<%@ page import="java.util.concurrent.ConcurrentHashMap"%> <%-- Thread Safe HashMap --%>
<%@ page import="java.util.List"%>
<%@ page import="java.util.Map"%>
<%@ page import="java.util.regex.Matcher"%>
<%@ page import="java.util.regex.Pattern"%>
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
    public static final String[] CurrencyCodeSymbol = { "＄", "￥", "€", "￥" };		// Currency symbols
    public static final String[] CurrencyCodename   = { "USD", "JPY", "EUR", "CNY" };	// Currency codenames
    public static final int ExchangePrecision = 2; 										// BigDecimal's floating point precision
	public static final long UpdateTermMillis = (1000 * 60 * 60);						// Default update term is '1-hour'
	//
    //--- [Changeable Constance End] ---//

    private static Map<String, BigDecimal> fcexMap = new ConcurrentHashMap<String, BigDecimal>();	// thread-safe HashMap for currency exchagne data
    private static long nextExcUpdateableTime = 0;													// The time that next updateable

    public static synchronized boolean updateFcex() { // 'FCEX' (Foreign Currency EXchange hashmap)
        long curTime = System.currentTimeMillis();
        StringBuilder htmlResponse = new StringBuilder(); // updateExc() is 'synchronized' method, so we can use both 'StringBuilder' and 'StringBuffer'

        if (curTime > nextExcUpdateableTime) {
            try {
                nextExcUpdateableTime = curTime + UpdateTermMillis; // next updateable time is after 1-hour from curTime

                // [1] Get hanabank-FCEX html data
                URL url = new URL("http://fx.kebhana.com/fxportal/jsp/RS/DEPLOY_EXRATE/fxrate_all.html");
                HttpURLConnection con = (HttpURLConnection)url.openConnection();
                
                con.setRequestMethod("POST");
                con.setDoInput(true);
                con.setDoOutput(true);
                con.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");

                // OutputStream os = con.getOutputStream();
                // os.write(); os.flush(); os.close(); // in case of want to send some data

                BufferedReader br = new BufferedReader(new InputStreamReader(con.getInputStream(), "EUC-KR"), con.getContentLength());
                String htmlLineStr = null;

                while ((htmlLineStr = br.readLine()) != null) {
                    htmlResponse.append(htmlLineStr);
                }
                br.close();
                
                // [2] Parse FCEX table 'html' using regex, and update 'Map<String, Float> fcexMap'
                // > Java regex doc - https://docs.oracle.com/javase/8/docs/api/java/util/regex/Pattern.html
				String html = htmlResponse.toString();
                Pattern pattern_tbody = Pattern.compile("(<tbody>)(.+?)(</tbody>)"); // search all <tbody> ~ </tbody>
                Matcher matcher_tbody = pattern_tbody.matcher(html);
                
                while (matcher_tbody.find()) {
                    html = matcher_tbody.group();
                    break; // This jsp only need the first <tbody> tag's innerHTML that type of 'cash'
                }
				
				Pattern pattern_unit = Pattern.compile("(?<=(alt='' />))(.+?)(?=(</td>))"); // searcch all <img ... alt='' /> ~ </td>
				Matcher matcher_unit = pattern_unit.matcher(html);
				List<String> curUnitList = new ArrayList<String>();
				
				while (matcher_unit.find()) {
					curUnitList.add(matcher_unit.group().replaceAll("[^0-9]", "")); // (100￥ -> 100), (1USD -> 1)
				}

                Pattern pattern_td = Pattern.compile("(?<=(<td class='sell'>))(.+?)(?=(</td>))"); // search all <td class='sell'> ~ </td>
                Matcher matcher_td = pattern_td.matcher(html);
                int loopI = 0;

                while (matcher_td.find()) {
					BigDecimal number = new BigDecimal(matcher_td.group());
					BigDecimal unit = new BigDecimal(curUnitList.get(loopI));
                    fcexMap.put(CurrencyCodename[loopI], number.divide(unit, ExchangePrecision, BigDecimal.ROUND_DOWN));
                    ++loopI;
                }
            }
            catch (Exception e) {
                e.printStackTrace();
            }

            return true;
        }

        return false;
    }

    public BigDecimal getFcex(String key) { // fcexMap is ConcurrentHashMap(thread-safe) so this method does not need 'synchronized' keyword
        return fcexMap.get(key);
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
    String wonPriceStr   = request.getParameter("wonPrice");
    String excPriceStr   = request.getParameter("excPrice");
    String excType       = request.getParameter("excType");
	String useRoundUpStr = request.getParameter("useRoundUp");
	boolean useRoundUp   = (useRoundUpStr == null || !useRoundUpStr.equals("true") ? false : true); 
    boolean doExcWork    = true;
	BigDecimal zeroPirce = new BigDecimal("0");
    BigDecimal wonPrice = zeroPirce;
    BigDecimal excPrice = zeroPirce;

    try {
		// [2-1] form parameter validation check
		double dbWonPrice = 0.00;
		if (wonPriceStr == null || wonPriceStr.length() == 0) {
			errorMsg += " [원화가격 입력] ";
			doExcWork = false;
		}
		else if ((dbWonPrice = Double.parseDouble(wonPriceStr)) < 0.00) {
			errorMsg += " [0원 이상의 원화가격 입력] ";
			doExcWork = false;
		}
		else if (dbWonPrice > (double)Integer.MAX_VALUE) {
			errorMsg += " [한계 금액 초과] ";
			doExcWork = false;
		}
		else {
			wonPrice = new BigDecimal(wonPriceStr);
		}
		
		if (excType == null) {
			errorMsg += " [외화종류 선택] ";
			doExcWork = false;
		}
		else {
			boolean excContain = false;
			for (int i = 0; i < CurrencyCodename.length; ++i) {
				if (excType.equals(CurrencyCodename[i])) {
					excContain = true;
					break;
				}
			}
			
			if (!excContain) {
				errorMsg += String.format(" [미지원 외화종류(%s)] ", excType);
				doExcWork = false;
			}
		}
		
		// [2-2] Exchange work
		if (doExcWork) {
			excPrice = wonPrice.divide(getFcex(excType), ExchangePrecision, useRoundUp ? BigDecimal.ROUND_HALF_UP : BigDecimal.ROUND_DOWN);
		}
    }
    catch (Exception e) {
        e.printStackTrace();
        errorMsg += String.format(" [예외 발생(%s)] ", e.getMessage());
    }
    finally {
		// [3] Update html UI
		wonPriceStr = String.format("%s", wonPrice.toString());
		excPriceStr = String.format("%s%s", excPrice.toString(), getCurrencySymbolByCodename(excType));
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
                        <input type="number" class="form-control" name="wonPrice" id="wonPrice" max="2147483647" value="<%=wonPriceStr%>">
                    </div></td>
					<!-- 외화 종류 콤보박스 -->
                    <td align="center"><div class="form-group">
                        <label for="excType">외화 종류</label>
                        <select class="form-control" name="excType" id="excType">
							<%
								for (int i = 0; i < CurrencyCodename.length; ++i) {
									String codename = CurrencyCodename[i];
									String codeSymbol = CurrencyCodeSymbol[i];
									out.println(String.format("<option value=\"%s\" %s>%s(%s)</option>", codename, (codename.equals(excType) ? "selected" : " "), codename, codeSymbol));
								}
							%>
                        </select>
                    </div></td>
					<!-- 외화 환전결과 -->
                    <td align="right"><div class="form-group">
                        <label for="excPrice">외화 가격</label>
                        <input type="text" class="form-control" name="excPrice" id="excPrice" value="<%=excPriceStr%>" readonly>
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
							out.println(String.format("<input name=\"useRoundUp\" type=\"checkbox\" value=\"true\" %s>소숫점 %d번째 자리에서 반올림(v)/내림( )</label>", (useRoundUp ? "checked" : " "), (ExchangePrecision + 1)));
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