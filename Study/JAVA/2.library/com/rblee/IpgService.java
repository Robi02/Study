package com.rblee;

import java.util.Date;
import java.text.SimpleDateFormat;
import java.net.InetSocketAddress;

public class IpgService {

    // 클래스 내부 상수 //
    private static final Logger _logger = Logger.getInstance();
    private static final SimpleDateFormat DATE_FMT = new SimpleDateFormat("yyMMddHHmmss");

    public static String getCurTime() {
        return DATE_FMT.format(new Date());
    }

    // 상점정보 조회
    public static boolean shopInfoService(IpgRelayServer.Client cli, IpgMsg reqMsg, IpgMsg rpyMsg) {
        String cliAddrStr = cli.getSocAddr().toString();
        boolean svcResult = false;

        _logger.log(Logger.INFO, String.format("Client(%s):shopInfoService() 시작!", cliAddrStr));

        // 서비스 로직 시작 //
        String curTime = IpgService.getCurTime();

        // 가맹점 정보조회 요청 속성값 초기화
        String req_terminal_div_numb = new String(reqMsg.get("terminal_div_numb"));
        String svc_partner_platform = "";
        
        if (req_terminal_div_numb.equals("DPT0F50057")) {
            // 알리페이
            svcResult =true;
            svc_partner_platform = "alipay";
        }
        else if (req_terminal_div_numb.equals("DPT0F50200")) {
            // 위쳇페이
            svcResult =true;
            svc_partner_platform = "wechatpay";
        }
        else {
            // 미등록단말기
            svcResult = false;
        }

        // 가맹점 정보조회 응답 속성값 설정
        if (svcResult) {
            rpyMsg.set("terminal_div_numb"  , req_terminal_div_numb.getBytes());
            rpyMsg.set("comp_info"          , reqMsg.get("comp_info").getBytes());
            rpyMsg.set("msg_numb"           , reqMsg.get("msg_numb").getBytes());
            rpyMsg.set("deal_date"          , curTime.getBytes());
            rpyMsg.set("card_type"          , new byte[] { (byte)'1' });
            rpyMsg.set("shop_numb"          , new String("가맹점번호(15)").getBytes());
            rpyMsg.set("make_comp_gove_code", new byte[] { (byte)'0', (byte)'5'} );
            rpyMsg.set("card_type_name"     , new String("05가 어디더라?").getBytes());
            rpyMsg.set("purc_comp_gove_code", new byte[] { (byte)'0', (byte)'5'} );
            rpyMsg.set("purc_comp_gove_name", new String("05가 신한카드?").getBytes());
            rpyMsg.set("notice1"            , new String("TBOTW-Myeong-dong01S").getBytes());
            rpyMsg.set("transaction_type"   , new byte[] { (byte)'0' } );
            if (svc_partner_platform.equals("alipay")) {
                rpyMsg.set("notice2"            , new String("알리페이 API 키 (MAX:40바이트)").getBytes());
                rpyMsg.set("ksnet_reserved"     , new String("알리:PT_ID+SMC_ID").getBytes());
            }
            else if (svc_partner_platform.equals("wechatpay")) {
                rpyMsg.set("notice2"            , new String("위쳇이 API 키 (MAX:40바이트)").getBytes());
                rpyMsg.set("ksnet_reserved"     , new String("위쳇:APP_ID+MCH_ID").getBytes());
            }
            rpyMsg.set("shop_info"          , new String("1208197322천하제일상회-명동1점                              0234206807     01012344321    서울특별시 강남구 삼성동 168-26 테헤란로 107길 11 (삼성동, 성보타워) 5층 효주마켓 명품관").getBytes());
        }
        
        byte[] resultSym = new byte[1];
        byte[] resultMsg1, resultMsg2, approval_numb;
        
        if (svcResult) {
            resultSym[0] = (byte)'O';
            approval_numb = "C12345".getBytes();
            resultMsg1 = "카드종류명".getBytes();
            resultMsg2 = "OK: 승인번호8자".getBytes();
        }
        else {
            resultSym[0] = (byte)'X';
            approval_numb = "C011".getBytes();
            resultMsg1 = "단말기번호가,".getBytes();
            resultMsg2 = "등록안되있어유!".getBytes();
        }

        rpyMsg.set("status",   resultSym);
        rpyMsg.set("approval_numb", approval_numb);
        rpyMsg.set("message1", resultMsg1);
        rpyMsg.set("message2", resultMsg2);
        // 서비스 로직 종료 //

        _logger.log(Logger.INFO, String.format("Client(%s):shopInfoService() 종료! (Result:%b)", cliAddrStr, svcResult));
        return true;
    }

    // 승인정보 조회
    public static boolean approvalInfoService(IpgRelayServer.Client cli, IpgMsg reqMsg, IpgMsg rpyMsg) {
        String cliAddrStr = cli.getSocAddr().toString();
        boolean svcResult = false;

        _logger.log(Logger.INFO, String.format("Client(%s):approvalInfoService() 시작!", cliAddrStr));

        // 서비스 로직 시작 //
        String curTime = IpgService.getCurTime();

        // 가맹점 정보조회 요청 속성값 초기화
        String req_terminal_div_numb = new String(reqMsg.get("terminal_div_numb"));
        String svc_partner_platform = "";
        
        if (req_terminal_div_numb.equals("DPT0F50057")) {
            // 알리페이
            svcResult =true;
            svc_partner_platform = "alipay";
        }
        else if (req_terminal_div_numb.equals("DPT0F50200")) {
            // 위쳇페이
            svcResult =true;
            svc_partner_platform = "wechatpay";
        }
        else {
            // 미등록단말기
            svcResult = false;
        }

        // 승인정보 조회 응답 속성값 설정
        if (svcResult) {
            rpyMsg.set("terminal_div_numb"  , req_terminal_div_numb.getBytes());
            rpyMsg.set("comp_info"          , reqMsg.get("comp_info").getBytes());
            rpyMsg.set("msg_numb"           , reqMsg.get("msg_numb").getBytes());
            rpyMsg.set("deal_date"          , curTime.getBytes());
            rpyMsg.set("card_type"          , new byte[] { (byte)'1' });
            rpyMsg.set("shop_numb"          , new String("가맹점번호(15)").getBytes());
            rpyMsg.set("make_comp_gove_code", new byte[] { (byte)'0', (byte)'5'} );
            rpyMsg.set("card_type_name"     , new String("05가 어디더라?").getBytes());
            rpyMsg.set("purc_comp_gove_code", new byte[] { (byte)'0', (byte)'5'} );
            rpyMsg.set("purc_comp_gove_name", new String("05가 신한카드?").getBytes());
            rpyMsg.set("point1"             , new String("7654321").getBytes());
            rpyMsg.set("point2"             , new String("1234567").getBytes());
            rpyMsg.set("point3"             , new String("KRW").getBytes());
            rpyMsg.set("notice1"            , new String("TBOTW-Myeong-dong01S").getBytes());
            rpyMsg.set("transaction_type"   , new byte[] { (byte)'0' } );
            if (svc_partner_platform.equals("alipay")) {
                rpyMsg.set("ksnet_reserved"     , new String("CNY123456700").getBytes());
            }
            else if (svc_partner_platform.equals("wechatpay")) {
                rpyMsg.set("ksnet_reserved"     , new String("위쳇:APP_ID+MCH_ID").getBytes());
            }
        }
        
        byte[] resultSym = new byte[1];
        byte[] resultMsg1, resultMsg2, approval_numb;
        
        if (svcResult) {
            resultSym[0] = (byte)'O';
            approval_numb = "C12345".getBytes();
            resultMsg1 = "카드종류명".getBytes();
            resultMsg2 = "OK: 승인번호8자".getBytes();
        }
        else {
            resultSym[0] = (byte)'X';
            approval_numb = "C011".getBytes();
            resultMsg1 = "단말기번호가,".getBytes();
            resultMsg2 = "등록안되있어유!".getBytes();
        }

        rpyMsg.set("status", resultSym);
        rpyMsg.set("approval_numb", approval_numb);
        rpyMsg.set("message1", resultMsg1);
        rpyMsg.set("message2", resultMsg2);
        // 서비스 로직 종료 //

        _logger.log(Logger.INFO, String.format("Client(%s):approvalInfoService() 종료! (Result:%b)", cliAddrStr, svcResult));
        return true;
    }

    // 결제내역 통보
    public static boolean payNotifyService(IpgRelayServer.Client cli, IpgMsg reqMsg, IpgMsg rpyMsg) {
        String cliAddrStr = cli.getSocAddr().toString();
        boolean svcResult = false;

        _logger.log(Logger.INFO, String.format("Client(%s):payNotifyService() 시작!", cliAddrStr));

        // 서비스 로직 시작 //
        String curTime = IpgService.getCurTime();

        // 결제내역 통보 응답 속성값 초기화
        String req_terminal_div_numb = new String(reqMsg.get("terminal_div_numb"));
        String svc_partner_platform = "";
        
        if (req_terminal_div_numb.equals("DPT0F50057")) {
            // 알리페이
            svcResult =true;
            svc_partner_platform = "alipay";
        }
        else if (req_terminal_div_numb.equals("DPT0F50200")) {
            // 위쳇페이
            svcResult =true;
            svc_partner_platform = "wechatpay";
        }
        else {
            // 미등록단말기
            svcResult = false;
        }

        // 결제내역 통보 응답 속성값 설정
        if (svcResult) {
            rpyMsg.set("terminal_div_numb"  , req_terminal_div_numb.getBytes());
            rpyMsg.set("comp_info"          , reqMsg.get("comp_info").getBytes());
            rpyMsg.set("msg_numb"           , reqMsg.get("msg_numb").getBytes());
            rpyMsg.set("deal_date"          , curTime.getBytes());
            rpyMsg.set("card_type"          , new byte[] { (byte)'1' });
            rpyMsg.set("shop_numb"          , new String("가맹점번호(15)").getBytes());
            rpyMsg.set("make_comp_gove_code", new byte[] { (byte)'0', (byte)'5'} );
            rpyMsg.set("card_type_name"     , new String("05가 어디더라?").getBytes());
            rpyMsg.set("purc_comp_gove_code", new byte[] { (byte)'0', (byte)'5'} );
            rpyMsg.set("purc_comp_gove_name", new String("05가 신한카드?").getBytes());
            rpyMsg.set("transaction_type"   , new byte[] { (byte)'0' } );
        }
        
        byte[] resultSym = new byte[1];
        byte[] resultMsg1, resultMsg2, approval_numb;
        
        if (svcResult) {
            resultSym[0] = (byte)'O';
            resultMsg1 = "카드종류명".getBytes();
            resultMsg2 = "OK: ".getBytes();
            approval_numb = "C12345".getBytes();
        }
        else {
            resultSym[0] = (byte)'X';
            resultMsg1 = "단말기번호가,".getBytes();
            resultMsg2 = "등록안되있어유!".getBytes();
            approval_numb = "C011".getBytes();
        }

        rpyMsg.set("status",   resultSym);
        rpyMsg.set("message1", resultMsg1);
        rpyMsg.set("message2", resultMsg2);
        rpyMsg.set("approval_numb", approval_numb);
        // 서비스 로직 종료 //

        _logger.log(Logger.INFO, String.format("Client(%s):payNotifyService() 종료! (Result:%b)", cliAddrStr, svcResult));
        return true;
    }

}