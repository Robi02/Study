<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@ page language="java" contentType="text/html; charset=EUC-KR" pageEncoding="EUC-KR"%>
<%@ page import="java.io.BufferedReader"%>
<%@ page import="java.io.InputStreamReader"%>
<%@ page import="java.io.OutputStream"%>
<%@ page import="java.net.HttpURLConnection"%>
<%@ page import="java.net.URL"%>
<%@ page import="java.util.concurrent.ConcurrentHashMap"%> <%-- Thread Safe HashMap --%>
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
    public static final String[] CurrencyCodeSymbol = { "＄", "￥", "€", "￥" };
    public static final String[] CurrencyCodename   = { "USD", "JPY", "EUR", "CNY" };
    public static final long UpdateTermMillis = (1000 * 60 * 60); // 1-hour
	//
    //--- [Changeable Constance End] ---//

    private static Map<String, Float> fcexMap = new ConcurrentHashMap<String, Float>(); // thread-safe HashMap
    private static long nextExcUpdateableTime = 0;

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
                // > Java regex doc - https://docs.oracle.com/javase/7/docs/api/java/util/regex/Pattern.html
				String html = htmlResponse.toString();
                Pattern pattern_tbody = Pattern.compile("(<tbody>)(.+?)(</tbody>)"); // search all <tbody> ... </tbody>
                Matcher matcher_tbody = pattern_tbody.matcher(html);
                
                while (matcher_tbody.find()) {
                    html = matcher_tbody.group();
                    break; // This jsp only need the first <tbody> tag's innerHTML that type of 'cash'
                }

                Pattern pattern_td = Pattern.compile("(?<=(<td class='sell'>))(.+?)(?=(</td>))"); // search all <td class='sell'> ... </td>
                Matcher matcher_td = pattern_td.matcher(html);
                int loopI = 0;

                while (matcher_td.find()) {
                    Float number = Float.parseFloat(matcher_td.group());
                    fcexMap.put(CurrencyCodename[loopI], number);
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

    public Float getFcex(String key) { // fcexMap is ConcurrentHashMap(thread-safe) so this method does not need 'synchronized' keyword
        return fcexMap.get(key);
    }

    public String getCurrencySymbolByCodename(String codename) {
        for (int i = 0; i < CurrencyCodename.length; ++i) {
            if (codename.equals(CurrencyCodename[i])) {
                return CurrencyCodeSymbol[i];
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

    final String zeroPriceStr = "0";
    boolean doExcWork = true;
    String wonPriceStr = request.getParameter("wonPrice");
    String excPriceStr = request.getParameter("excPrice");
    String excType     = request.getParameter("excType");
    String errorMsg = " ";
    float wonPrice = 0;
    float excPrice = 0;

    try {
        if (wonPriceStr == null || wonPriceStr.length() == 0) {
            wonPriceStr = zeroPriceStr;
            doExcWork = false;
            errorMsg += " 원화 가격이 비어있음 ";
        }
        else {
            if ((wonPrice = Float.parseFloat(wonPriceStr)) <= 0.00f) {
                doExcWork = false;
                errorMsg += " 원화 가격이 0 이하 ";
            }
        }

        if (excPriceStr == null || excPriceStr.length() == 0) {
            excPriceStr = zeroPriceStr;
        }

        if (excType == null || excType.length() == 0) {
            excType = CurrencyCodename[0];
            doExcWork = false;
            errorMsg += " 선택 외화가 비어있음 ";
        }
        else {
            boolean contains = false;
            for (int i = 0; i < CurrencyCodename.length; ++i) {
                if (excType.equals(CurrencyCodename[i])) {
                    contains = true;
                    break;
                }
            }

            if (!contains) {
                excType = CurrencyCodename[0];
                doExcWork = false;
                errorMsg += " 지원하지 않는 외화 ";
            }
        }

        if (doExcWork) {
            Float fcexVal = getFcex(excType);

            if (fcexVal == null || fcexVal <= 0.00f) {
                throw new Exception("Error! fcexVal is " + fcexVal);
            }

            excPrice = (wonPrice / fcexVal);
        }
    }
    catch (Exception e) {
        e.printStackTrace();
        wonPrice = 0.00f;
        excPrice = 0.00f;
        excType = CurrencyCodename[0];
        errorMsg += " 예외 발생 ";
    }
    finally {
        if (wonPrice % 1.00f != 0) {
            wonPriceStr = String.format("%.2f", wonPrice);
        }
        else {
            wonPriceStr = String.format("%d", (int)wonPrice);
        }

        if (excPrice != 0.00f) {
            excPriceStr = String.format("%.2f%s", excPrice, getCurrencySymbolByCodename(excType));
        }
    }
%>

<body>
    <div class="d-flex justify-content-center align-items-center" style="height:500px;">
        <form action="./" method="POST">
            <table>
                <tr>
                    <td colspan="3" align="center"><br><h1>KSnet 환율 변환</h1><br></td>
                </tr>
                <tr>
                    <td align="left"><div class="form-group">
                        <label for="wonPrice">원화 가격 (￦)</label>
                        <input type="number" class="form-control" name="wonPrice" id="wonPrice" value="<%=wonPriceStr%>">
                    </div></td>
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
                    <td align="right"><div class="form-group">
                        <label for="excPrice">외화 가격</label>
                        <input type="text" class="form-control" name="excPrice" id="excPrice" value="<%=excPriceStr%>" readonly>
                    </div></td>
                </tr>
                <tr>
                    <td colspan="3" align="center"><input type="submit" class="btn btn-info btn-block" value="환율 변환"></td>
                </tr>
                <%
                    if (!doExcWork) {
                        out.println("<tr><td colspan=\"3\"><p><div class=\"alert alert-danger\">");
                        out.println("<strong>오류!</strong> (" + errorMsg + ")");
                        out.println("</div></p></td></tr>");
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