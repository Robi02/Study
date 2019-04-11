package com.rblee;

import java.util.HashMap;

public class ServletCommonVar {
    // 서블릿 공용변수 //
    public String _wtId, _wtPw;						    // 웹토큰, 토큰암호
	public String _reqURI;								// 페이지 URI
	public HashMap<String, String> _reqMap, _rpyMap;	// reqeust, response를 담은 HashMap
	public boolean _jspResult;							// 서블릿 결과

    public ServletCommonVar() {
        this._wtId = null;
        this._wtPw = null;
        this._reqURI = null;
        this._reqMap = null;
        this._rpyMap = null;
        this._jspResult = false;
    }
}