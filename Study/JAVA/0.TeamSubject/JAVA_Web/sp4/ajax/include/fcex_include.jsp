<%@ page language="java" contentType="text/html; charset=EUC-KR" pageEncoding="EUC-KR"%>
<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@ taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<%@ page import="java.io.BufferedReader"%>
<%@ page import="java.io.IOException"%>
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
<%!
	//--- [Changeable Constance Begin] ---//
	//
	public static final String[] RoundOptions       = { "ROUND_UP", "HALF_ROUND_UP", "ROUND_DOWN" };	// Round options
	public static final String[] RoundOptionSymbol  = { "올림", "반올림", "내림" };							// Round option symbols
	//
	public static final String[] CurrencyCodeSymbol = { "＄",	"￥",	"€",	"￥"  };				// Currency symbols
	public static final String[] CurrencyCodename   = { "USD",	"JPY",	"EUR",	"CNY" };				// Currency codenames
	//
	public static final long UpdateTermMillis = ( 1000 * 60 * 60 * 1 );	// Default is 1-hour update term
	//
	//--- [Changeable Constance End] ---//
%>