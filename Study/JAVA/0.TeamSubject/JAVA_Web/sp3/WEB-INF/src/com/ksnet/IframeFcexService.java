package com.ksnet;

import java.io.IOException;
import java.io.PrintWriter;
import javax.servlet.RequestDispatcher;
import javax.servlet.ServletConfig;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.URL;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.util.ArrayList;
import java.util.concurrent.ConcurrentHashMap;
import java.util.List;
import java.util.LinkedList;
import java.util.Map;
import java.util.Map.Entry;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.ksnet.KsCommonLib;

public class IframeFcexService extends HttpServlet {
	//--- [Changeable Constance Begin] ---//
	//
    public static final String[] RoundOptions        = { "ROUND_UP", "HALF_ROUND_UP", "ROUND_DOWN" };	// Round options
	//
	public static final String[] CurrencyCodeSymbol  = { "＄",	"￥",	"€",	"￥"  };				// Currency symbols
    public static final String[] CurrencyCodename    = { "USD",	"JPY",	"EUR",	"CNY" };				// Currency codenames
	//
	public static final long UpdateTermMillis = (1000 * 60 * 60 * 1);	// Default update term is '1-hour' (ms * s * m * h)
	//
    //--- [Changeable Constance End] ---//

    private Map<String, Long> fcexMap;		// Thread-safe HashMap for currency exchange data
	private Map<String, Long> fcexMapCpy;	// Thread-safe HashMap to work concurrency
	private boolean fcexUpdating; 			// While updating fcexMap, value must be 'true' otherwise 'false'
    private boolean fcexCpyUpdating;		// While updating fcexCpyMap, value must be 'true' otherwise 'false'
	private long nextFcexMapUpdateableTime;	// The time that next updateable

	// Update 'FCEX' (Foreign Currency EXchange hashmap)
    protected synchronized void updateFcex() {
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
					
					System.out.println(String.format("수신:\n%s", new String(readByte)));
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
    protected Long getFcex(String key) throws Exception {
		if (fcexMap.size() > 0 && !fcexUpdating) {
			System.out.println("원본맵 사용 sz:" + fcexMap.size()); // test
			return fcexMap.get(key);
		}
		else if (fcexMapCpy.size() > 0 && !fcexCpyUpdating) {
			System.out.println("카피맵 사용 sz:" + fcexMapCpy.size()); // test
			return fcexMapCpy.get(key);
		}
		else {
			System.out.println("throwException"); // test
			throw new Exception("환율정보를 갱신중입니다. 몇초 후 다시 시도해주세요.");
		}
    }

	// Get codeSymbol from codename
    protected String getCurrencySymbolByCodename(String codename) {
        if (codename != null && codename.length() > 0) {
			for (int i = 0; i < CurrencyCodename.length; ++i) {
				if (codename.equals(CurrencyCodename[i])) {
					return CurrencyCodeSymbol[i];
				}
			}
		}

        return "";
    }
	
	@Override
	public void init(ServletConfig config) throws ServletException {
		super.init(config);
		
		fcexMap = new ConcurrentHashMap<String, Long>();
		fcexMapCpy = new ConcurrentHashMap<String, Long>();
		fcexUpdating = false;
		fcexCpyUpdating = false;
		nextFcexMapUpdateableTime = 0;
		
		System.out.println("IframeFcexService 초기화 완료!");
	}
	
	@Override
	protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		doGet(request, response);
	}
	
	@Override
	// URL: [GET] /sp3/iframe/
	protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
		System.out.println("IframeFcexService doGet 시작!");
		request.setCharacterEncoding("EUC-KR");
		
		// [1] When this page called, automatically update FCEX table every 'UpdateTermMillis'
		updateFcex();

		// [2] Calculate exchange result by 'ajax'
		request.setCharacterEncoding("EUC-KR");

		String errorMsg = "";
		String wonPriceParam   = request.getParameter("wonPrice");
		String excPriceParam   = null;
		String excTypeParam    = request.getParameter("excType");
		String roundOpsParam   = request.getParameter("roundOps");
		boolean doExcWork      = true;
		long wonPrice = 0;
		long excPrice = 0;

		try {
			// [2-1] form parameter validation check
			double dbWonPrice = 0.00;
			if (wonPriceParam == null) {
				doExcWork = false;
			}
			else if (((wonPriceParam = wonPriceParam.replaceAll("[^0-9//.//-]", "")).length()) == 0) {
				errorMsg += " [올바른 원화가격 입력] ";
				doExcWork = false;
			}
			else if ((dbWonPrice = Double.parseDouble(wonPriceParam)) <= 0.00) {
				errorMsg += " [0원 이상의 원화가격 입력] ";
				doExcWork = false;
			}
			else if (dbWonPrice > (double)Integer.MAX_VALUE) {
				errorMsg += " [한계 금액 초과] ";
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
					errorMsg += String.format(" [미지원 외화종류(%s)] ", excTypeParam);
					doExcWork = false;
				}
			}
			
			if (roundOpsParam == null) {
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
					errorMsg += String.format(" [라운딩 옵션 오류(%s)] ", roundOpsParam);
					doExcWork = false;
				}
			}
			
			// [2-2] Exchange work
			if (doExcWork) {
				long excVal = getFcex(excTypeParam);
				
				excPrice = wonPrice * 100 / excVal;
				
				System.out.println(String.format("입력:%d / 환율:%d / 금액:%d", wonPrice, excVal, excPrice)); // test
				
				if (roundOpsParam.equals(RoundOptions[0])) { // round up
					if (excPrice % excVal != 0) { ++excPrice; System.out.println("올림 적용"); }
				}
				else if (roundOpsParam.equals(RoundOptions[1])) { // half round up
					if (excPrice % excVal >= 50) { ++excPrice; System.out.println("반올림 적용"); }
				}
				else if (roundOpsParam.equals(RoundOptions[2])) { // round down
					// round down will set default by divide(/) operator
				}
			}
		}
		catch (Exception e) {
			e.printStackTrace();
			errorMsg += String.format(" [%s] ", e.getMessage());
			doExcWork = false;
		}
		finally {
			// [3] Update html UI
			String[] url = request.getRequestURL().toString().split("/");
			String dir = url[url.length - 1];
			
			// URL: [GET] sp3/iframe
			if (!dir.equals("result")) {
				request.setCharacterEncoding("EUC-KR");
				request.setAttribute("RoundOptions", RoundOptions);
				request.setAttribute("CurrencyCodename", CurrencyCodename);
				request.setAttribute("CurrencyCodeSymbol", CurrencyCodeSymbol);
				request.setAttribute("wonPrice", "0");
				request.setAttribute("excPrice", "0" + CurrencyCodeSymbol[0]);
				request.setAttribute("excType", CurrencyCodename[0]);
				request.setAttribute("roundOps", RoundOptions[0]);
				request.setAttribute("errorMsg", null);
				request.getRequestDispatcher("/view/iframe/fcex_input.jsp").forward(request, response);
			}
			// URL: [GET] sp3/iframe/result
			else {
				wonPriceParam = String.format("%d", wonPrice);
				excPriceParam = String.format("%d%s", excPrice, getCurrencySymbolByCodename(excTypeParam));
				request.setCharacterEncoding("EUC-KR");
				request.setAttribute("RoundOptions", RoundOptions);
				request.setAttribute("CurrencyCodename", CurrencyCodename);
				request.setAttribute("CurrencyCodeSymbol", CurrencyCodeSymbol);
				request.setAttribute("wonPrice", wonPriceParam);
				request.setAttribute("excPrice", excPriceParam);
				request.setAttribute("excType", excTypeParam);
				request.setAttribute("roundOps", roundOpsParam);
				request.setAttribute("errorMsg", errorMsg);
				request.getRequestDispatcher("/view/iframe/fcex_result.jsp").forward(request, response);
			}	
			
			System.out.println("IframeFcexService doGet 완료!");
		}
	}
}