<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@ page language="java" contentType="text/html; charset=EUC-KR" pageEncoding="EUC-KR"%>
<%@ page import="java.io.BufferedReader"%>
<%@ page import="java.io.InputStreamReader"%>
<%@ page import="java.io.OutputStream"%>
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
    public static final String[] RoundOptions = { "ROUND_UP", "HALF_ROUND_UP", "ROUND_DOWN" };	// Round options
	public static final String[] CurrencyCodeSymbol = {	"��",	"��",	"��",	"��"	};		// Currency symbols
    public static final String[] CurrencyCodename   = {	"USD",	"JPY",	"EUR",	"CNY"	};		// Currency codenames
	public static final long UpdateTermMillis = (1000 * 60 * 60 * 1);					// Default update term is '1-hour' (ms * s * m * h)
	//
    //--- [Changeable Constance End] ---//

    private static Map<String, Long> fcexMap    = new ConcurrentHashMap<String, Long>();	// Thread-safe HashMap for currency exchange data
	private static Map<String, Long> fcexMapCpy = new ConcurrentHashMap<String, Long>();	// Thread-safe HashMap to work concurrency
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
				
				// [1] Request hana-bank FCEX data from server to update 'Map<String, Long> fcexMap'
				try {
					System.out.println(String.format("fcexMap update begin! (%d)", System.currentTimeMillis())); // test
					byte[] sendByte = KsCommonLib.makeSendByte("ufce2", null);
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
					
					System.out.println(String.format("����:\n%s", new String(readByte)));
					byte[] recvByte = KsCommonLib.makeRecvByte(readByte);
					recvByte = KsCommonLib.hexString2ByteAry(new String(recvByte));
					fcexMap = (ConcurrentHashMap<String, Long>)KsCommonLib.byteAry2Obj(recvByte);
					
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
				
				// [2] Update 'Map<String, Long> fcexMapCpy' using copy data of fcexMap
				if (updateSuccess) {
					System.out.println(String.format("fcexMapCpy update begin! (%d)", System.currentTimeMillis())); // test
					fcexCpyUpdating = true;
					fcexMapCpy.clear();

					for (Entry<String, Long> entry : fcexMap.entrySet()) { // fcexMap deep copy
						fcexMapCpy.put(entry.getKey(), new Long(entry.getValue().toString()));
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
    public Long getFcex(String key) throws Exception {
		if (fcexMap.size() > 0 && !fcexUpdating) {
			System.out.println("������ ��� sz:" + fcexMap.size()); // test
			return fcexMap.get(key);
		}
		else if (fcexMapCpy.size() > 0 && !fcexCpyUpdating) {
			System.out.println("ī�Ǹ� ��� sz:" + fcexMapCpy.size()); // test
			return fcexMapCpy.get(key);
		}
		else {
			System.out.println("throwException"); // test
			throw new Exception(" [ȯ�������� �������Դϴ�. ���� �� �ٽ� �õ����ּ���.] ");
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
	String roundOpsParam   = request.getParameter("roundOps");
    boolean doExcWork      = true;
    long wonPrice = 0;
    long excPrice = 0;

    try {
		// [2-1] form parameter validation check
		double dbWonPrice = 0.00;
		if (wonPriceParam == null) {
			errorMsg += " [��ȭ���� �Է�] ";
			doExcWork = false;
		}
		else if (((wonPriceParam = wonPriceParam.replaceAll("[^0-9//.]", "")).length()) == 0) {
			errorMsg += " [�ùٸ� ��ȭ���� �Է�] ";
			doExcWork = false;
		}
		else if ((dbWonPrice = Double.parseDouble(wonPriceParam)) < 0.00) {
			errorMsg += " [0�� �̻��� ��ȭ���� �Է�] ";
			doExcWork = false;
		}
		else if (dbWonPrice > (double)Integer.MAX_VALUE) {
			errorMsg += " [�Ѱ� �ݾ� �ʰ�] ";
			doExcWork = false;
		}
		else {
			wonPrice = Long.parseLong(wonPriceParam);
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
				errorMsg += String.format(" [������ ��ȭ����(%s)] ", excTypeParam);
				doExcWork = false;
			}
		}
		
		if (roundOpsParam == null) {
			errorMsg += String.format(" [���� �ɼ� �̼���] ");
			roundOpsParam = RoundOptions[0];
			doExcWork = false;
		}
		else {
			boolean roundOpsContain = false;
			for (int i = 0; i < RoundOptions.length; ++i) {
				if (roundOpsParam.equals(RoundOptions[i])) {
					roundOpsContain = true;
					break; 
				}
			}
			
			if (!roundOpsContain) {
				errorMsg += String.format(" [���� �ɼ� ����(%s)] ", roundOpsParam);
				doExcWork = false;
			}
		}
		
		// [2-2] Exchange work
		if (doExcWork) {
			long excVal = getFcex(excTypeParam);
			
			excPrice = wonPrice * 100 / excVal;
			
			System.out.println(String.format("�Է�:%d / ȯ��:%d / �ݾ�:%d", wonPrice, excVal, excPrice)); // test
			
			if (roundOpsParam.equals(RoundOptions[0])) { // round up
				if (excPrice % excVal != 0) { ++excPrice; System.out.println("�ø� ����"); }
			}
			else if (roundOpsParam.equals(RoundOptions[1])) { // half round up
				if (excPrice % excVal >= 50) { ++excPrice; System.out.println("�ݿø� ����"); }
			}
			else if (roundOpsParam.equals(RoundOptions[2])) { // round down
				// round down will set default by divide(/) operator
			}
		}
    }
    catch (Exception e) {
        e.printStackTrace();
        errorMsg += String.format(" [���� �߻�(%s)] ", e.getMessage());
		doExcWork = false;
    }
    finally {
		// [3] Update html UI
		wonPriceParam = String.format("%d", wonPrice);
		excPriceParam = String.format("%d%s", excPrice, getCurrencySymbolByCodename(excTypeParam));
    }
%>

<body>
    <div class="d-flex justify-content-center align-items-center" style="height:500px;">
        <form action="./" method="POST">
            <table>
				<!-- Ÿ��Ʋ -->
                <tr>
                    <td colspan="3" align="right"><br><h1>KSnet ȯ�� ��ȯ</h1><br></td>
                </tr>
                <tr>
					<!-- ��ȭ ���� -->
                    <td align="left"><div class="form-group">
                        <label for="wonPrice">��ȭ ���� (��)</label>
                        <input type="text" style="text-align: right;" class="form-control" name="wonPrice" id="wonPrice" max="2147483647" value="<%=wonPriceParam%>">
                    </div></td>
					<!-- ��ȭ ���� �޺��ڽ� -->
                    <td align="center"><div class="form-group">
                        <label for="excType">��ȭ ����</label>
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
					<!-- ��ȭ ȯ����� -->
                    <td align="right"><div class="form-group">
                        <label for="excPrice">��ȭ ����</label>
                        <input type="text" style="text-align: right;" class="form-control" name="excPrice" id="excPrice" value="<%=excPriceParam%>" readonly>
                    </div></td>
                </tr>
				<!-- ȯ�� ��ȯ ��ư -->
                <tr>
                    <td colspan="3" align="center"><input type="submit" class="btn btn-info btn-block" value="ȯ�� ��ȯ"></td>
                </tr>
				<!-- �ݿø� üũ�ڽ� -->
				<tr>
					<td colspan="3" align="right" style="font-size: 12px;">
						<div class="radio">
							��� ����� (
							<%
								String[] roundOpStr = { "�ø�", "�ݿø�", "����" };
								for (int i = 0; i < roundOpStr.length; ++i) {
									out.println(String.format("<label><input type='radio' name='roundOps' value='%s' %s>%s</label>&nbsp;",
															  RoundOptions[i], roundOpsParam.equals(RoundOptions[i]) ? "checked" : "", roundOpStr[i]));
								}
							%>
							)
						</div>
					</td>
				</tr>
				<!-- ���� �޽��� -->
                <%
                    if (!doExcWork) {
                        out.println("<tr><td colspan=\"3\"><div class=\"alert alert-danger\">");
                        out.println("<strong>����!</strong> " + errorMsg);
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