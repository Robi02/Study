package com.rblee;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class RpyMsgParser {
    // [클래스 공개 상수] //
    public static final int WECHAT_QR_V1 = 1;   // 위쳇페이 QR결제 전문
    public static final int ALIPAY_QR_V1 = 2;   // 알리페이 QR결제 전문

    // [클래스 내부 변수] //
    private IpgMsg rpyMsg;    // 파싱 대상 전문
    private String traceId;   // 트레이스 ID
    private Logger logger;    // 로거

    // [생성자] //
    public RpyMsgParser(IpgMsg rpyMsg, String traceId, Logger logger) {
        this.rpyMsg = rpyMsg;
        this.traceId = traceId;
        this.logger = logger;
    }

    // [메서드] //
    // 전문을 파싱하여 맵으로 반환
    public Map<String, String> parse(int msgType) {
        if (rpyMsg == null) {
            log(Logger.ERROR, "'rpyMsg' is null!");
            return null;
        }

        switch (msgType) {
            case WECHAT_QR_V1:
                return parse_WECHAT_QR_V1(); // 위쳇페이 QR결제 전문
            case ALIPAY_QR_V1:
                return parse_ALIPAY_QR_V1(); // 알리페이 QR결제 전문
            default:
                return null;
        }
    }

    // MsgParser 로그
    private void log(int logType, Object... logObjs) {
        if (this.logger != null) {
            logger.traceLog(Logger.ERROR, this.traceId, "[MsgParse]", logObjs);
        }
    }

    // WECHAT_QR_V1 전문 (2019.03.11)
    private Map<String, String> parse_WECHAT_QR_V1() {
        Map<String, String> rtMap = null;
        Map<String, IpgMsg.MsgAttr> msgAttr = rpyMsg.getMsgAttrMap();
        int excTrace = 0;

        try {
            // += [상점정보 조회 응답] ==========================================================================================+
            if (msgAttr == IpgMsg.RpyShopInfoMsgAttr) {
                String shop_name 		= null;	// 가맹점명
                String entr_numb		= null;	// 사업자번호
                String telephone		= null;	// 가맹점연락처
                String date_time		= null;	// 거래일자
                String currency_code	= null;	// 통화코드
                String appid			= null;	// WeChat-KSNET Service ID
                String mch_id			= null;	// WeChat-KSNET-Merchant ID
                String sub_mch_id		= null;	// WeChat-KSNET-Merchant-SubMerchant ID
                String api_key			= null;	// WeChat-KSNET API key (해시서명용)
                String secret			= null;	// WeChat-KSNET APP Secret (openid 획득용)
                String goods_id			= null; // WeChat-KSNET-Merchand의 산업군
                String status           = null; // 응답코드
        
                // shop_info_byte(shop_info) : 가맹점명(50)+가맹점전화번호(15)+가맹점휴대폰번호(15)+가맹점주소(130)+APP_Secret(32)
                byte[] shop_info_byte = rpyMsg.get("shop_info").getBytes();
                excTrace = 1;
                // shop_name(notice2) : 가맹점 명(영문)
                shop_name            = rpyMsg.get("notice2").trim();
                excTrace = 2;
                // entr_numb(notice1) : 사업자번호(xxxyyzzzzz) -> xxx-yy-zzzzz
                String entr_numb_raw = rpyMsg.get("notice1").trim();
                entr_numb			 = String.format("%s-%s-%s", entr_numb_raw.substring(0, 3),			// xxx
                                                                entr_numb_raw.substring(3, 5),			// yy
                                                                entr_numb_raw.substring(5, 10)).trim();	// zzzzz
                excTrace = 3;
                // telephone(shop_info(50-65)) : 가맹점 전화번호
                telephone		= new String(Arrays.copyOfRange(shop_info_byte, 50, 65)).trim();
                // date_time(deal_date) : 거래일시(yymmddhhMMss) -> 20yy-mm-dd hh:MM:ss
                String date_time_raw = rpyMsg.get("deal_date").trim();
                date_time			 = String.format("20%s-%s-%s %s:%s:%s", date_time_raw.substring(0, 2),		// yy
                                                                            date_time_raw.substring(2, 4),   	// mm
                                                                            date_time_raw.substring(4, 6),   	// dd
                                                                            date_time_raw.substring(6, 8),   	// hh
                                                                            date_time_raw.substring(8, 10),  	// MM
                                                                            date_time_raw.substring(10, 12));	// ss
                excTrace = 4;
                // currency_code : 통화코드 (410:KRW, 840:USD)
                currency_code	= rpyMsg.get("reserved").trim();
                excTrace = 5;
                // appid
                appid           = (rpyMsg.get("working_key_index") + rpyMsg.get("working_key")).trim();
                excTrace = 6;
                // mch_id
                mch_id		    = entr_numb_raw.substring(10, 20).trim();
                excTrace = 7;
                // sub_mch_id
                sub_mch_id	    = rpyMsg.get("shop_numb").trim();
                excTrace = 8;
                // api_key
                api_key		    = rpyMsg.get("ksnet_reserved").trim();
                excTrace = 9;
                // secret
                secret		    = new String(Arrays.copyOfRange(shop_info_byte, 210, 242)).trim();
                excTrace = 10;
                // goods_id
                goods_id	    = rpyMsg.get("comp_info").trim();
                excTrace = 11;
                // status
                status          = rpyMsg.get("status");
                excTrace = 12;

                // 맵에 넣어 반환
                rtMap = new HashMap<String, String>();
                rtMap.put("shop_name",      shop_name);
                rtMap.put("entr_numb",      entr_numb);
                rtMap.put("telephone",      telephone);
                rtMap.put("date_time",      date_time);
                rtMap.put("currency_code",  currency_code);
                rtMap.put("appid",          appid);
                rtMap.put("mch_id",         mch_id);
                rtMap.put("sub_mch_id",     sub_mch_id);
                rtMap.put("api_key",        api_key);
                rtMap.put("secret",         secret);
                rtMap.put("goods_id",       goods_id);
                rtMap.put("status",         status);
            }
            // += [승인번호 생성 응답] ==========================================================================================+
            else if (msgAttr == IpgMsg.RpyArrovalNumMsgAttr) {
                String shop_name 		= null;	// 가맹점명
                String entr_numb		= null;	// 사업자번호
                String telephone		= null;	// 가맹점연락처
                String date_time		= null;	// 거래일자
                String currency_code	= null;	// 통화코드
                String appid			= null;	// WeChat-KSNET Service ID
                String mch_id			= null;	// WeChat-KSNET-Merchant ID
                String sub_mch_id		= null;	// WeChat-KSNET-Merchant-SubMerchant ID
                String api_key			= null;	// WeChat-KSNET API key (해시서명용)
                String secret			= null;	// WeChat-KSNET APP Secret (openid 획득용)
                String goods_id			= null; // WeChat-KSNET-Merchand의 산업군
                String approval_numb    = null; // 승인번호(채번)
                String status           = null; // 응답코드
        
                // shop_info_byte(shop_info) : 가맹점명(50)+가맹점전화번호(15)+가맹점휴대폰번호(15)+가맹점주소(130)+APP_Secret(32)
                byte[] shop_info_byte = rpyMsg.get("shop_info").getBytes();
                excTrace = 1;
                // shop_name(notice2) : 가맹점 명(영문)
                shop_name 		= rpyMsg.get("notice2").trim();
                excTrace = 2;
                // entr_numb(notice1) : 사업자번호(xxxyyzzzzz) -> xxx-yy-zzzzz
                String entr_numb_raw = rpyMsg.get("notice1").trim();
                entr_numb			 = String.format("%s-%s-%s", entr_numb_raw.substring(0, 3),			// xxx
                                                                entr_numb_raw.substring(3, 5),			// yy
                                                                entr_numb_raw.substring(5, 10)).trim();	// zzzzz
                excTrace = 3;
                // telephone(shop_info(50-65)) : 가맹점 전화번호
                telephone		= new String(Arrays.copyOfRange(shop_info_byte, 50, 65)).trim();
                // date_time(deal_date) : 거래일시(yymmddhhMMss) -> 20yy-mm-dd hh:MM:ss
                String date_time_raw = rpyMsg.get("deal_date").trim();
                date_time			 = String.format("20%s-%s-%s %s:%s:%s", date_time_raw.substring(0, 2),		// yy
                                                                            date_time_raw.substring(2, 4),   	// mm
                                                                            date_time_raw.substring(4, 6),   	// dd
                                                                            date_time_raw.substring(6, 8),   	// hh
                                                                            date_time_raw.substring(8, 10),  	// MM
                                                                            date_time_raw.substring(10, 12));	// ss
                excTrace = 4;
                // currency_code : 통화코드 (410:KRW, 840:USD)
                currency_code	= rpyMsg.get("reserved").trim();
                excTrace = 5;
                // appid
                appid           = (rpyMsg.get("working_key_index") + rpyMsg.get("working_key")).trim();
                excTrace = 6;
                // mch_id
                mch_id		    = entr_numb_raw.substring(10, 20).trim();
                excTrace = 7;
                // sub_mch_id
                sub_mch_id	    = rpyMsg.get("shop_numb").trim();
                excTrace = 8;
                // api_key
                api_key		    = rpyMsg.get("ksnet_reserved").trim();
                excTrace = 9;
                // secret
                secret		    = new String(Arrays.copyOfRange(shop_info_byte, 210, 242)).trim();
                excTrace = 10;
                // goods_id
                goods_id	    = rpyMsg.get("comp_info").trim();
                excTrace = 11;
                // approval_numb
                approval_numb   = rpyMsg.get("approval_numb").trim();
                excTrace = 12;
                // status
                status          = rpyMsg.get("status");
                excTrace = 13;

                // 맵에 넣어 반환
                rtMap = new HashMap<String, String>();
                rtMap.put("shop_name",      shop_name);
                rtMap.put("entr_numb",      entr_numb);
                rtMap.put("telephone",      telephone);
                rtMap.put("date_time",      date_time);
                rtMap.put("currency_code",  currency_code);
                rtMap.put("appid",          appid);
                rtMap.put("mch_id",         mch_id);
                rtMap.put("sub_mch_id",     sub_mch_id);
                rtMap.put("api_key",        api_key);
                rtMap.put("secret",         secret);
                rtMap.put("goods_id",       goods_id);
                rtMap.put("approval_numb",  approval_numb);
                rtMap.put("status",         status);
            }
            // += [거래통보 응답] ==============================================================================================+
            else if (msgAttr == IpgMsg.RpyPayNotifyAttr) {
                String date_time    = null; // 통보 일시
                String message1     = null; // 메시지1
                String message2     = null; // 메시지2
                String status       = null; // 응답코드

                // date_time : 통보 일시(yymmddhhMMss)
                String date_time_raw = rpyMsg.get("deal_date").trim();
                date_time			 = String.format("20%s-%s-%s %s:%s:%s", date_time_raw.substring(0, 2),		// yy
                                                                            date_time_raw.substring(2, 4),   	// mm
                                                                            date_time_raw.substring(4, 6),   	// dd
                                                                            date_time_raw.substring(6, 8),   	// hh
                                                                            date_time_raw.substring(8, 10),  	// MM
                                                                            date_time_raw.substring(10, 12));	// ss
                excTrace = 1;
                // message1, message2
                message1             = rpyMsg.get("message1").trim();
                message2             = rpyMsg.get("message2").trim();
                excTrace = 2;
                // status
                status               = rpyMsg.get("status");
                excTrace = 3;

                // 맵에 넣어 반환
                rtMap = new HashMap<String, String>();
                rtMap.put("date_time",  date_time);
                rtMap.put("message1",   message1);
                rtMap.put("message2",   message2);
                rtMap.put("status",     status);
            }
            // += [파서내 미정의 전문속성] =======================================================================================+
            else {
                log(Logger.ERROR, "Undefined 'msgAttr' in parser! (MsgName: '" + rpyMsg.getMsgName() +  "', msgAttr: '" + msgAttr + "')");
                return null;
            }
        }
        catch (Exception e) {
            log(Logger.ERROR, "Exception! (MsgName: '" + rpyMsg.getMsgName() +  "', " + "ExcTrace:'" + excTrace + "')", e);
            return null;
        }

        return rtMap;
    }

    // ALIPAY_QR_V1 전문 (2019.03.11)
    private Map<String, String> parse_ALIPAY_QR_V1() {
        Map<String, String> rtMap = null;
        Map<String, IpgMsg.MsgAttr> msgAttr = rpyMsg.getMsgAttrMap();
        int excTrace = 0;

        try {
            // += [상점정보 조회 응답] ==========================================================================================+
            if (msgAttr == IpgMsg.RpyShopInfoMsgAttr) {
                String shop_name        = null; // 가맹점명
                String entr_numb        = null; // 사업자번호
                String telephone        = null; // 가맹점연락처
                String date_time        = null; // 거래일시
                String currency_code    = null; // 통화코드
                String partnerId        = null; // Alipay-KSNET Partner ID
                String privateKey       = null; // Alipay-KSNET Private Key
                String sec_mch_id       = null; // KSNET 가맹점 ID
                String sec_mch_industry = null; // 가맹점 산업군
                String status           = null; // 응답결과

                // shop_info_byte(shop_info) : 가맹점명(50)+가맹점전화번호(15)+가맹점휴대폰번호(15)+가맹점주소(130)
                byte[] shop_info_byte = rpyMsg.get("shop_info").getBytes();
                excTrace = 1;
                // shop_name(notice2) : 가맹점 명(영문)
                shop_name            = rpyMsg.get("notice2").trim();
                excTrace = 2;
                // entr_numb(notice1) : 사업자번호(xxxyyzzzzz) -> xxx-yy-zzzzz
                String entr_numb_raw = rpyMsg.get("notice1").trim();
                entr_numb			 = String.format("%s-%s-%s", entr_numb_raw.substring(0, 3),			    // xxx
                                                                 entr_numb_raw.substring(3, 5),			    // yy
                                                                 entr_numb_raw.substring(5, 10)).trim();	// zzzzz
                excTrace = 3;
                // telephone(shop_info(50-65)) : 가맹점 전화번호
                telephone		= new String(Arrays.copyOfRange(shop_info_byte, 50, 65)).trim();
                // date_time(deal_date) : 거래일시(yymmddhhMMss) -> 20yy-mm-dd hh:MM:ss
                String date_time_raw = rpyMsg.get("deal_date").trim();
                date_time			 = String.format("20%s-%s-%s %s:%s:%s", date_time_raw.substring(0, 2),		// yy
                                                                            date_time_raw.substring(2, 4),   	// mm
                                                                            date_time_raw.substring(4, 6),   	// dd
                                                                            date_time_raw.substring(6, 8),   	// hh
                                                                            date_time_raw.substring(8, 10),  	// MM
                                                                            date_time_raw.substring(10, 12));	// ss
                excTrace = 4;
                // currency_code : 통화코드 (410:KRW, 840:USD)
                currency_code       = rpyMsg.get("reserved").trim();
                excTrace = 5;
                // partnerId
                partnerId           = (rpyMsg.get("working_key_index") + rpyMsg.get("working_key")).trim();
                excTrace = 6;
                // privateKey
                privateKey          = rpyMsg.get("ksnet_reserved").trim();
                excTrace = 7;
                // sec_mch_id
                sec_mch_id          = rpyMsg.get("shop_numb").trim();
                excTrace = 8;
                // sec_mch_industry
                sec_mch_industry    = rpyMsg.get("comp_info").trim();
                excTrace = 9;
                // status
                status              = rpyMsg.get("status");
                excTrace = 10;

                // 맵에 넣어 반환
                rtMap = new HashMap<String, String>();
                rtMap.put("shop_name",          shop_name);
                rtMap.put("entr_numb",          entr_numb);
                rtMap.put("telephone",          telephone);
                rtMap.put("date_time",          date_time);
                rtMap.put("currency_code",      currency_code);
                rtMap.put("partnerId",          partnerId);
                rtMap.put("privateKey",         privateKey);
                rtMap.put("sec_mch_id",         sec_mch_id);
                rtMap.put("sec_mch_industry",   sec_mch_industry);
                rtMap.put("status",             status);
            }
            // += [승인번호 생성 응답] ==========================================================================================+
            else if (msgAttr == IpgMsg.RpyArrovalNumMsgAttr) {
                String shop_name        = null; // 가맹점명
                String entr_numb        = null; // 사업자번호
                String telephone        = null; // 가맹점연락처
                String date_time        = null; // 거래일시
                String currency_code    = null; // 통화코드
                String partnerId        = null; // Alipay-KSNET Partner ID
                String privateKey       = null; // Alipay-KSNET Private Key
                String sec_mch_id       = null; // KSNET 가맹점 ID
                String sec_mch_industry = null; // 가맹점 산업군
                String approval_numb    = null; // 승인번호(채번)
                String status           = null; // 응답결과

                // shop_info_byte(shop_info) : 가맹점명(50)+가맹점전화번호(15)+가맹점휴대폰번호(15)+가맹점주소(130)
                byte[] shop_info_byte = rpyMsg.get("shop_info").getBytes();
                excTrace = 1;
                // shop_name(notice2) : 가맹점 명(영문)
                shop_name            = rpyMsg.get("notice2").trim();
                excTrace = 2;
                // entr_numb(notice1) : 사업자번호(xxxyyzzzzz) -> xxx-yy-zzzzz
                String entr_numb_raw = rpyMsg.get("notice1").trim();
                entr_numb			 = String.format("%s-%s-%s", entr_numb_raw.substring(0, 3),			    // xxx
                                                                 entr_numb_raw.substring(3, 5),			    // yy
                                                                 entr_numb_raw.substring(5, 10)).trim();	// zzzzz
                excTrace = 3;
                // telephone(shop_info(50-65)) : 가맹점 전화번호
                telephone		= new String(Arrays.copyOfRange(shop_info_byte, 50, 65)).trim();
                // date_time(deal_date) : 거래일시(yymmddhhMMss) -> 20yy-mm-dd hh:MM:ss
                String date_time_raw = rpyMsg.get("deal_date").trim();
                date_time			 = String.format("20%s-%s-%s %s:%s:%s", date_time_raw.substring(0, 2),		// yy
                                                                            date_time_raw.substring(2, 4),   	// mm
                                                                            date_time_raw.substring(4, 6),   	// dd
                                                                            date_time_raw.substring(6, 8),   	// hh
                                                                            date_time_raw.substring(8, 10),  	// MM
                                                                            date_time_raw.substring(10, 12));	// ss
                excTrace = 4;
                // currency_code : 통화코드 (410:KRW, 840:USD)
                currency_code   = rpyMsg.get("reserved").trim();
                excTrace = 5;
                // partnerId
                partnerId           = (rpyMsg.get("working_key_index") + rpyMsg.get("working_key")).trim();
                excTrace = 6;
                // privateKey
                privateKey          = rpyMsg.get("ksnet_reserved").trim();
                excTrace = 7;
                // sec_mch_id
                sec_mch_id          = rpyMsg.get("shop_numb").trim();
                excTrace = 8;
                // sec_mch_industry
                sec_mch_industry    = rpyMsg.get("comp_info").trim();
                excTrace = 9;
                // approval_numb
                approval_numb       = rpyMsg.get("approval_numb").trim();
                excTrace = 10;
                // status
                status              = rpyMsg.get("status");
                excTrace = 11;

                // 맵에 넣어 반환
                rtMap = new HashMap<String, String>();
                rtMap.put("shop_name",          shop_name);
                rtMap.put("entr_numb",          entr_numb);
                rtMap.put("telephone",          telephone);
                rtMap.put("date_time",          date_time);
                rtMap.put("currency_code",      currency_code);
                rtMap.put("partnerId",          partnerId);
                rtMap.put("privateKey",         privateKey);
                rtMap.put("sec_mch_id",         sec_mch_id);
                rtMap.put("sec_mch_industry",   sec_mch_industry);
                rtMap.put("approval_numb",      approval_numb);
                rtMap.put("status",             status);
            }
            // += [거래통보 응답] ==============================================================================================+
            else if (msgAttr == IpgMsg.RpyPayNotifyAttr) {
                String date_time    = null; // 통보 일시
                String message1     = null; // 메시지1
                String message2     = null; // 메시지2
                String status       = null; // 응답코드

                // date_time : 통보 일시(yymmddhhMMss)
                String date_time_raw = rpyMsg.get("deal_date").trim();
                date_time			 = String.format("20%s-%s-%s %s:%s:%s", date_time_raw.substring(0, 2),		// yy
                                                                            date_time_raw.substring(2, 4),   	// mm
                                                                            date_time_raw.substring(4, 6),   	// dd
                                                                            date_time_raw.substring(6, 8),   	// hh
                                                                            date_time_raw.substring(8, 10),  	// MM
                                                                            date_time_raw.substring(10, 12));	// ss
                excTrace = 1;
                // message1, message2
                message1             = rpyMsg.get("message1").trim();
                message2             = rpyMsg.get("message2").trim();
                excTrace = 2;
                // status
                status               = rpyMsg.get("status");
                excTrace = 3;

                // 맵에 넣어 반환
                rtMap = new HashMap<String, String>();
                rtMap.put("date_time",  date_time);
                rtMap.put("message1",   message1);
                rtMap.put("message2",   message2);
                rtMap.put("status",     status);
            }
            // += [파서내 미정의 전문속성] =======================================================================================+
            else {
                log(Logger.ERROR, "Undefined 'msgAttr' in parser! (MsgName: '" + rpyMsg.getMsgName() +  "', msgAttr: '" + msgAttr + "')");
                return null;
            }
        }
        catch (Exception e) {
            log(Logger.ERROR, "Exception! (MsgName: '" + rpyMsg.getMsgName() +  "', " + "ExcTrace:'" + excTrace + "')", e);
            return null;
        }

        return rtMap;
    }

    // 템플릿
    /*private Map<String, String> parse_xxx_Vx() {
        Map<String, String> rtMap = null;
        Map<String, IpgMsg.MsgAttr> msgAttr = rpyMsg.getMsgAttrMap();
        int excTrace = 0;

        try {
            // += [상점정보 조회 응답] ==========================================================================================+
            if (msgAttr == IpgMsg.RpyShopInfoMsgAttr) {
                // ...
            }
            // += [승인번호 생성 응답] ==========================================================================================+
            else if (msgAttr == IpgMsg.RpyArrovalNumMsgAttr) {
                // ...
            }
            // += [거래통보 응답] ==============================================================================================+
            else if (msgAttr == IpgMsg.RpyPayNotifyAttr) {
                // ...
            }
            // += [파서내 미정의 전문속성] =======================================================================================+
            else {
                log(Logger.ERROR, "Undefined 'msgAttr' in parser! (MsgName: '" + rpyMsg.getMsgName() +  "', msgAttr: '" + msgAttr + "')");
                return null;
            }
        }
        catch (Exception e) {
            log(Logger.ERROR, "Exception! (MsgName: '" + rpyMsg.getMsgName() +  "', " + "ExcTrace:'" + excTrace + "')", e);
            return null;
        }

        return rtMap;
    }*/
}