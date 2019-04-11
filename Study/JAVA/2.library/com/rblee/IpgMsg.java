package com.rblee;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.LinkedList;
import java.util.Map;

public class IpgMsg {

    // 클래스 상수 //
    public static final String IPG_CHARSET = "EUC-KR";

    // 전문 속성값 상수 //
    public static final Map<String, MsgAttr> ReqArrovalNumMsgAttr;      // 승인번호 생성 요청 속성값 (상점정보 + Pay정보 + 승인번호)
    public static final Map<String, MsgAttr> RpyArrovalNumMsgAttr;      // 승인번호 생성 응답 속성값
    public static final Map<String, MsgAttr> ReqShopInfoMsgAttr;        // 상점정보 조회 요청 속성값 (상점정보 + Pay정보)
    public static final Map<String, MsgAttr> RpyShopInfoMsgAttr;        // 상점정보 조회 응답 속성값
    public static final Map<String, MsgAttr> ReqPayNotifyAttr;          // 결제정보 통보저장 요청 속성값
    public static final Map<String, MsgAttr> RpyPayNotifyAttr;          // 결제정보 통보저장 응답 속성값
    // 여기에 새로운 속성 추가...

    // 전문 속성값 상수 초기화 //
    static {
        // 승인번호 생성 요청 속성값 (상점정보 + Pay정보 + 승인번호)
        ReqArrovalNumMsgAttr = new HashMap<String, MsgAttr>();
        ReqArrovalNumMsgAttr.put("length"             , new MsgAttr(0,    "length",               0,      4,      '9',  null,               "전문길이: 길이필드 제외 전문길이"));
        ReqArrovalNumMsgAttr.put("stx"                , new MsgAttr(1,    "stx",                  4,      1,      'X',  new byte[]{0x02},   "구분자: STX(0x02)"));
        ReqArrovalNumMsgAttr.put("deal_type"          , new MsgAttr(2,    "deal_type",            5,      2,      'X',  "BI".getBytes(),    "거래구분: BI(위쳇/알리페이 승인번호생성)"));
        ReqArrovalNumMsgAttr.put("terminal_div_numb"  , new MsgAttr(3,    "terminal_div_numb",    7,      10,     'X',  null,               "단말기번호: DPT0F00000"));
        ReqArrovalNumMsgAttr.put("comp_info"          , new MsgAttr(4,    "comp_info",            17,     4,      'X',  null,               "업체정보: null"));
        ReqArrovalNumMsgAttr.put("msg_numb"           , new MsgAttr(5,    "msg_numb",             21,     12,     '9',  null,               "전문번호: null"));
        ReqArrovalNumMsgAttr.put("pos_entry_mode"     , new MsgAttr(6,    "pos_entry_mode",       33,     1,      'X',  "Q".getBytes(),     "PosEntryMode: 'Q'(QR결제)"));
        ReqArrovalNumMsgAttr.put("track2"             , new MsgAttr(7,    "track2",               34,     37,     'X',  null,               "TrackII: null"));
        ReqArrovalNumMsgAttr.put("fs"                 , new MsgAttr(8,    "fs",                   71,     1,      'X',  new byte[]{0x1C},   "FS: 0x1C"));
        ReqArrovalNumMsgAttr.put("approval_dist"      , new MsgAttr(9,    "approval_dist",        72,     2,      'X',  "MI".getBytes(),    "조회구분: MI"));
        ReqArrovalNumMsgAttr.put("total_amount"       , new MsgAttr(10,   "total_amount",         74,     9,      '9',  null,               "총금액: 총금액(달러승인인 경우 100 = 1$)"));
        ReqArrovalNumMsgAttr.put("cancle_amount"      , new MsgAttr(11,   "cancle_amount",        83,     9,      '9',  null,               "부분취소금액: 부분취소금액(달러승인인 경우 100 = 1$)"));
        ReqArrovalNumMsgAttr.put("vat_amount"         , new MsgAttr(12,   "vat_amount",           92,     9,      '9',  null,               "세금(부가세): 세금(부가세)(달러승인인 경우 100 = 1$)"));
        ReqArrovalNumMsgAttr.put("tax_amount"         , new MsgAttr(13,   "tax_amount",           101,    9,      '9',  null,               "과세금액: 과세금액(달러승인인 경우 100 = 1$)"));
        ReqArrovalNumMsgAttr.put("working_key_index"  , new MsgAttr(14,   "working_key_index",    110,    2,      'X',  "AA".getBytes(),    "WorkingKeyIndex: AA"));
        ReqArrovalNumMsgAttr.put("password"           , new MsgAttr(15,   "password",             112,    16,     'X',  null,               "비밀번호: null"));
        ReqArrovalNumMsgAttr.put("approval_numb"      , new MsgAttr(16,   "approval_numb",        128,    12,     'X',  null,               "원거래승인번호: null"));
        ReqArrovalNumMsgAttr.put("approval_date"      , new MsgAttr(17,   "approval_date",        140,    6,      'X',  null,               "원거래승인일자(YYMMDD): null"));
        ReqArrovalNumMsgAttr.put("user_info"          , new MsgAttr(18,   "user_info",            146,    13,     'X',  null,               "사용자정보: null"));
        ReqArrovalNumMsgAttr.put("shop_id"            , new MsgAttr(19,   "shop_id",              159,    2,      'X',  null,               "가맹점ID: null"));
        ReqArrovalNumMsgAttr.put("shop_reserved"      , new MsgAttr(20,   "shop_reserved",        161,    30,     'X',  null,               "가맹점사용필드: null"));
        ReqArrovalNumMsgAttr.put("point_dist"         , new MsgAttr(21,   "point_dist",           191,    4,      'X',  null,               "포인트구분: 페이사 구분(위쳇WX/알리:AL)"));
        ReqArrovalNumMsgAttr.put("ksnet_reserved"     , new MsgAttr(22,   "ksnet_reserved",       195,    20,     'X',  null,               "KSNET_Reserved: null"));
        ReqArrovalNumMsgAttr.put("dongul_dist"        , new MsgAttr(23,   "dongul_dist",          215,    1,      'X',  "N".getBytes(),     "동글구분: 'N' (동글 거래 아님)"));
        ReqArrovalNumMsgAttr.put("pay_div_type"       , new MsgAttr(24,   "pay_div_type",         216,    1,      'X',  "S".getBytes(),     "매체구분: 'S' (스마트폰 결제)"));
        ReqArrovalNumMsgAttr.put("telecom_dist"       , new MsgAttr(25,   "telecom_dist",         217,    1,      'X',  null,               "통신사구분: null"));
        ReqArrovalNumMsgAttr.put("credit_card_type"   , new MsgAttr(26,   "credit_card_type",     218,    1,      'X',  null,               "신용카드종류: null"));
        ReqArrovalNumMsgAttr.put("transaction_type"   , new MsgAttr(27,   "transaction_type",     219,    1,      'X',  null,               "거래형태: null"));
        ReqArrovalNumMsgAttr.put("use_sign"           , new MsgAttr(28,   "use_sign",             220,    1,      'X',  "N".getBytes(),     "전자서명유무 : 'N'"));
        ReqArrovalNumMsgAttr.put("platform_reserved"  , new MsgAttr(29,   "platform_reserved",    221,    200,    'X',  null,               "위쳇/알리/추가정보: null"));
        ReqArrovalNumMsgAttr.put("etx"                , new MsgAttr(30,   "etx",                  421,    1,      'X',  new byte[]{0x03},   "ETX: 0x03"));
        ReqArrovalNumMsgAttr.put("cr"                 , new MsgAttr(31,   "cr",                   422,    1,      'X',  new byte[]{0x0D},   "CR: 0x0D"));

        // 승인번호 생성 응답 속성값
        RpyArrovalNumMsgAttr = new HashMap<String, MsgAttr>();
        RpyArrovalNumMsgAttr.put("length"             , new MsgAttr(0,    "length",               0,      4,      '9',  null,               "전문길이: 길이필드 제외 전문길이"));
        RpyArrovalNumMsgAttr.put("stx"                , new MsgAttr(1,    "stx",                  4,      1,      'X',  null,               "STX: 0x02"));
        RpyArrovalNumMsgAttr.put("deal_type"          , new MsgAttr(2,    "deal_type",            5,      2,      'X',  null,               "거래구분: BJ(위쳇/알리페이)"));
        RpyArrovalNumMsgAttr.put("terminal_div_numb"  , new MsgAttr(3,    "terminal_div_numb",    7,      10,     'X',  null,               "단말기번호: null"));
        RpyArrovalNumMsgAttr.put("comp_info"          , new MsgAttr(4,    "comp_info",            17,     4,      'X',  null,               "업체정보: 위쳇(goods_id(4)), 알리(sec_mch_industry(4))"));
        RpyArrovalNumMsgAttr.put("msg_numb"           , new MsgAttr(5,    "msg_numb",             21,     12,     '9',  null,               "전문번호: null"));
        RpyArrovalNumMsgAttr.put("status"             , new MsgAttr(6,    "status",               33,     1,      'X',  null,               "Status: 'O'(정상), 'X'(거절)"));
        RpyArrovalNumMsgAttr.put("deal_date"          , new MsgAttr(7,    "deal_date",            34,     12,     'X',  null,               "거래일시: YYMMDDhhmmss"));
        RpyArrovalNumMsgAttr.put("card_type"          , new MsgAttr(8,    "card_type",            46,     1,      'X',  null,               "카드Type: null"));
        RpyArrovalNumMsgAttr.put("message1"           , new MsgAttr(9,    "message1",             47,     16,     'X',  null,               "메시지1: 승인시(카드종류명) / 거절시(거절메시지)"));
        RpyArrovalNumMsgAttr.put("message2"           , new MsgAttr(10,   "message2",             63,     16,     'X',  null,               "메시지2: 승인시(OK) / 거절시(거절메시지)"));
        RpyArrovalNumMsgAttr.put("approval_numb"      , new MsgAttr(11,   "approval_numb",        79,     12,     'X',  null,               "승인번호: 업무계 채번 고유번호(뒷자리6) or 거절코드(4)"));
        RpyArrovalNumMsgAttr.put("shop_numb"          , new MsgAttr(12,   "shop_numb",            91,     15,     'X',  null,               "가맹점번호: 위쳇(sub_mch_id), 알리(sec_mch_id)"));
        RpyArrovalNumMsgAttr.put("make_comp_gove_code", new MsgAttr(13,   "make_comp_gove_code",  106,    2,      'X',  null,               "발급사코드: 카드사 코드(Table참조)"));
        RpyArrovalNumMsgAttr.put("card_type_name"     , new MsgAttr(14,   "card_type_name",       108,    16,     'X',  null,               "카드종류명: null"));
        RpyArrovalNumMsgAttr.put("purc_comp_gove_code", new MsgAttr(15,   "purc_comp_gove_code",  124,    2,      'X',  null,               "매입사코드: null"));
        RpyArrovalNumMsgAttr.put("purc_comp_gove_name", new MsgAttr(16,   "purc_comp_gove_name",  126,    16,     'X',  null,               "매입사명: null"));
        RpyArrovalNumMsgAttr.put("working_key_index"  , new MsgAttr(17,   "working_key_index",    142,    2,      'X',  null,               "WorkingKeyIndex: 위쳇(AppID), 알리(ParterID)"));
        RpyArrovalNumMsgAttr.put("working_key"        , new MsgAttr(18,   "working_key",          144,    16,     'X',  null,               "WorkingKey: 위쳇(AppID), 알리(ParterID)"));
        RpyArrovalNumMsgAttr.put("spare_point"        , new MsgAttr(19,   "spare_point",          160,    9,      '9',  null,               "예비포인트: 환율정보 1USD기준 KRW금액(7.2) or All '0'인 경우 KRW승인"));
        RpyArrovalNumMsgAttr.put("point1"             , new MsgAttr(20,   "point1",               169,    9,      '9',  null,               "포인트1: null"));
        RpyArrovalNumMsgAttr.put("point2"             , new MsgAttr(21,   "point2",               178,    9,      '9',  null,               "포인트2: null"));
        RpyArrovalNumMsgAttr.put("point3"             , new MsgAttr(22,   "point3",               187,    9,      'X',  null,               "포인트3: null"));
        RpyArrovalNumMsgAttr.put("notice1"            , new MsgAttr(23,   "notice1",              196,    20,     'X',  null,               "Notice1: 위쳇(10:사업자번호+10:mch_id), 알리(10:사업자번호)"));
        RpyArrovalNumMsgAttr.put("notice2"            , new MsgAttr(24,   "notice2",              216,    40,     'X',  null,               "Notice2: 가맹점명 (영문)"));
        RpyArrovalNumMsgAttr.put("transaction_type"   , new MsgAttr(25,   "transaction_type",     256,    1,      'X',  null,               "거래형태: 0(환율조회없음), 1(금일환율), 2(전일환율)"));
        RpyArrovalNumMsgAttr.put("reserved"           , new MsgAttr(26,   "reserved",             257,    5,      'X',  null,               "Reserved: 통화코드(3)"));
        RpyArrovalNumMsgAttr.put("ksnet_reserved"     , new MsgAttr(27,   "ksnet_reserved",       262,    40,     'X',  null,               "KSNET_Reserved: 위쳇(32:API_Key), 알리(32:PrivateKey)"));
        RpyArrovalNumMsgAttr.put("shop_info"          , new MsgAttr(28,   "shop_info",            302,    242,    'X',  null,               "가맹점정보 : 위쳇: 가맹점명(50)+가맹점연락처(15)+가맹점폰번호(15)+가맹점주소(130)+{위쳇:APP_Secret(32)}"));
        RpyArrovalNumMsgAttr.put("etx"                , new MsgAttr(29,   "etx",                  544,    1,      'X',  null,               "ETX: 0x03"));
        RpyArrovalNumMsgAttr.put("cr"                 , new MsgAttr(30,   "cr",                   545,    1,      'X',  null,               "CR: 0x0D"));

        // 상점정보 조회 요청 속성값
        ReqShopInfoMsgAttr = new HashMap<String, MsgAttr>();
        ReqShopInfoMsgAttr.put("length"             , new MsgAttr(0,    "length",               0,      4,      '9',    null,               "전문길이: 길이필드 제외 전문길이"));
        ReqShopInfoMsgAttr.put("stx"                , new MsgAttr(1,    "stx",                  4,      1,      'X',    new byte[]{0x02},   "구분자: STX(0x02)"));
        ReqShopInfoMsgAttr.put("deal_type"          , new MsgAttr(2,    "deal_type",            5,      2,      'X',    "BI".getBytes(),    "거래구분: BI(위쳇/알리페이 상점조회)"));
        ReqShopInfoMsgAttr.put("terminal_div_numb"  , new MsgAttr(3,    "terminal_div_numb",    7,      10,     'X',    null,               "단말기번호: DPT0F00000"));
        ReqShopInfoMsgAttr.put("comp_info"          , new MsgAttr(4,    "comp_info",            17,     4,      'X',    null,               "업체정보: null"));
        ReqShopInfoMsgAttr.put("msg_numb"           , new MsgAttr(5,    "msg_numb",             21,     12,     '9',    null,               "전문번호: null"));
        ReqShopInfoMsgAttr.put("pos_entry_mode"     , new MsgAttr(6,    "pos_entry_mode",       33,     1,      'X',    "Q".getBytes(),     "PosEntryMode: 'Q'(QR결제)"));
        ReqShopInfoMsgAttr.put("track2"             , new MsgAttr(7,    "track2",               34,     37,     'X',    null,               "TrackII: null"));
        ReqShopInfoMsgAttr.put("fs"                 , new MsgAttr(8,    "fs",                   71,     1,      'X',    new byte[]{0x1C},   "FS: 0x1C"));
        ReqShopInfoMsgAttr.put("approval_dist"      , new MsgAttr(9,    "approval_dist",        72,     2,      'X',    "KI".getBytes(),    "조회구분: KI"));
        ReqShopInfoMsgAttr.put("total_amount"       , new MsgAttr(10,   "total_amount",         74,     9,      '9',    null,               "총금액: null"));
        ReqShopInfoMsgAttr.put("cancle_amount"      , new MsgAttr(11,   "cancle_amount",        83,     9,      '9',    null,               "부분취소금액: null"));
        ReqShopInfoMsgAttr.put("vat_amount"         , new MsgAttr(12,   "vat_amount",           92,     9,      '9',    null,               "세금(부가세): null"));
        ReqShopInfoMsgAttr.put("tax_amount"         , new MsgAttr(13,   "tax_amount",           101,    9,      '9',    null,               "과세금액: null"));
        ReqShopInfoMsgAttr.put("working_key_index"  , new MsgAttr(14,   "working_key_index",    110,    2,      'X',    "AA".getBytes(),    "WorkingKeyIndex: AA"));
        ReqShopInfoMsgAttr.put("password"           , new MsgAttr(15,   "password",             112,    16,     'X',    null,               "비밀번호: null"));
        ReqShopInfoMsgAttr.put("approval_numb"      , new MsgAttr(16,   "approval_numb",        128,    12,     'X',    null,               "원거래승인번호: null"));
        ReqShopInfoMsgAttr.put("approval_date"      , new MsgAttr(17,   "approval_date",        140,    6,      'X',    null,               "원거래승인일자(YYMMDD): null"));
        ReqShopInfoMsgAttr.put("user_info"          , new MsgAttr(18,   "user_info",            146,    13,     'X',    null,               "사용자정보: null"));
        ReqShopInfoMsgAttr.put("shop_id"            , new MsgAttr(19,   "shop_id",              159,    2,      'X',    null,               "가맹점ID: null"));
        ReqShopInfoMsgAttr.put("shop_reserved"      , new MsgAttr(20,   "shop_reserved",        161,    30,     'X',    null,               "가맹점사용필드: null"));
        ReqShopInfoMsgAttr.put("point_dist"         , new MsgAttr(21,   "point_dist",           191,    4,      'X',    null,               "포인트구분: 페이사 구분(위쳇WX/알리:AL)"));
        ReqShopInfoMsgAttr.put("ksnet_reserved"     , new MsgAttr(22,   "ksnet_reserved",       195,    20,     'X',    null,               "KSNET_Reserved: null"));
        ReqShopInfoMsgAttr.put("dongul_dist"        , new MsgAttr(23,   "dongul_dist",          215,    1,      'X',    "N".getBytes(),     "동글구분: 'N' (동글 거래 아님)"));
        ReqShopInfoMsgAttr.put("pay_div_type"       , new MsgAttr(24,   "pay_div_type",         216,    1,      'X',    "S".getBytes(),     "매체구분: 'S' (스마트폰 결제)"));
        ReqShopInfoMsgAttr.put("telecom_dist"       , new MsgAttr(25,   "telecom_dist",         217,    1,      'X',    null,               "통신사구분: null"));
        ReqShopInfoMsgAttr.put("credit_card_type"   , new MsgAttr(26,   "credit_card_type",     218,    1,      'X',    null,               "신용카드종류: null"));
        ReqShopInfoMsgAttr.put("transaction_type"   , new MsgAttr(27,   "transaction_type",     219,    1,      'X',    null,               "거래형태: null"));
        ReqShopInfoMsgAttr.put("use_sign"           , new MsgAttr(28,   "use_sign",             220,    1,      'X',    "N".getBytes(),     "전자서명유무 : 'N'"));
        ReqShopInfoMsgAttr.put("platform_reserved"  , new MsgAttr(29,   "platform_reserved",    221,    200,    'X',    null,               "위쳇/알리/추가정보: null"));
        ReqShopInfoMsgAttr.put("etx"                , new MsgAttr(30,   "etx",                  421,    1,      'X',    new byte[]{0x03},   "ETX: 0x03"));
        ReqShopInfoMsgAttr.put("cr"                 , new MsgAttr(31,   "cr",                   422,    1,      'X',    new byte[]{0x0D},   "CR: 0x0D"));

        // 상점정보 조회 응답 속성값
        RpyShopInfoMsgAttr = new HashMap<String, MsgAttr>();
        RpyShopInfoMsgAttr.put("length"             , new MsgAttr(0,    "length",               0,      4,      '9',  null,               "전문길이: 길이필드 제외 전문길이"));
        RpyShopInfoMsgAttr.put("stx"                , new MsgAttr(1,    "stx",                  4,      1,      'X',  null,               "STX: 0x02"));
        RpyShopInfoMsgAttr.put("deal_type"          , new MsgAttr(2,    "deal_type",            5,      2,      'X',  null,               "거래구분: BJ(위쳇/알리페이)"));
        RpyShopInfoMsgAttr.put("terminal_div_numb"  , new MsgAttr(3,    "terminal_div_numb",    7,      10,     'X',  null,               "단말기번호: null"));
        RpyShopInfoMsgAttr.put("comp_info"          , new MsgAttr(4,    "comp_info",            17,     4,      'X',  null,               "업체정보: 위쳇(goods_id(4)), 알리(sec_mch_industry(4))"));
        RpyShopInfoMsgAttr.put("msg_numb"           , new MsgAttr(5,    "msg_numb",             21,     12,     '9',  null,               "전문번호: null"));
        RpyShopInfoMsgAttr.put("status"             , new MsgAttr(6,    "status",               33,     1,      'X',  null,               "Status: 'O'(정상), 'X'(거절)"));
        RpyShopInfoMsgAttr.put("deal_date"          , new MsgAttr(7,    "deal_date",            34,     12,     'X',  null,               "거래일시: YYMMDDhhmmss"));
        RpyShopInfoMsgAttr.put("card_type"          , new MsgAttr(8,    "card_type",            46,     1,      'X',  null,               "카드Type: null"));
        RpyShopInfoMsgAttr.put("message1"           , new MsgAttr(9,    "message1",             47,     16,     'X',  null,               "메시지1: 승인시(카드종류명) / 거절시(거절메시지)"));
        RpyShopInfoMsgAttr.put("message2"           , new MsgAttr(10,   "message2",             63,     16,     'X',  null,               "메시지2: 승인시(OK) / 거절시(거절메시지)"));
        RpyShopInfoMsgAttr.put("approval_numb"      , new MsgAttr(11,   "approval_numb",        79,     12,     'X',  null,               "승인번호: 업무계 채번 고유번호(뒷자리6) or 거절코드(4)"));
        RpyShopInfoMsgAttr.put("shop_numb"          , new MsgAttr(12,   "shop_numb",            91,     15,     'X',  null,               "가맹점번호: 위쳇(sub_mch_id), 알리(sec_mch_id)"));
        RpyShopInfoMsgAttr.put("make_comp_gove_code", new MsgAttr(13,   "make_comp_gove_code",  106,    2,      'X',  null,               "발급사코드: 카드사 코드(Table참조)"));
        RpyShopInfoMsgAttr.put("card_type_name"     , new MsgAttr(14,   "card_type_name",       108,    16,     'X',  null,               "카드종류명: null"));
        RpyShopInfoMsgAttr.put("purc_comp_gove_code", new MsgAttr(15,   "purc_comp_gove_code",  124,    2,      'X',  null,               "매입사코드: null"));
        RpyShopInfoMsgAttr.put("purc_comp_gove_name", new MsgAttr(16,   "purc_comp_gove_name",  126,    16,     'X',  null,               "매입사명: null"));
        RpyShopInfoMsgAttr.put("working_key_index"  , new MsgAttr(17,   "working_key_index",    142,    2,      'X',  null,               "WorkingKeyIndex: 위쳇(AppID), 알리(ParterID)"));
        RpyShopInfoMsgAttr.put("working_key"        , new MsgAttr(18,   "working_key",          144,    16,     'X',  null,               "WorkingKey: 위쳇(AppID), 알리(ParterID)"));
        RpyShopInfoMsgAttr.put("spare_point"        , new MsgAttr(19,   "spare_point",          160,    9,      '9',  null,               "예비포인트: 환율정보 1USD기준 KRW금액(7.2) or All '0'인 경우 KRW승인"));
        RpyShopInfoMsgAttr.put("point1"             , new MsgAttr(20,   "point1",               169,    9,      '9',  null,               "포인트1: null"));
        RpyShopInfoMsgAttr.put("point2"             , new MsgAttr(21,   "point2",               178,    9,      '9',  null,               "포인트2: null"));
        RpyShopInfoMsgAttr.put("point3"             , new MsgAttr(22,   "point3",               187,    9,      'X',  null,               "포인트3: null"));
        RpyShopInfoMsgAttr.put("notice1"            , new MsgAttr(23,   "notice1",              196,    20,     'X',  null,               "Notice1: 위쳇(10:사업자번호+10:mch_id), 알리(10:사업자번호)"));
        RpyShopInfoMsgAttr.put("notice2"            , new MsgAttr(24,   "notice2",              216,    40,     'X',  null,               "Notice2: 가맹점명 (영문)"));
        RpyShopInfoMsgAttr.put("transaction_type"   , new MsgAttr(25,   "transaction_type",     256,    1,      'X',  null,               "거래형태: 0(환율조회없음), 1(금일환율), 2(전일환율)"));
        RpyShopInfoMsgAttr.put("reserved"           , new MsgAttr(26,   "reserved",             257,    5,      'X',  null,               "Reserved: 통화코드(3)"));
        RpyShopInfoMsgAttr.put("ksnet_reserved"     , new MsgAttr(27,   "ksnet_reserved",       262,    40,     'X',  null,               "KSNET_Reserved: 위쳇(32:API_Key), 알리(32:PrivateKey)"));
        RpyShopInfoMsgAttr.put("shop_info"          , new MsgAttr(28,   "shop_info",            302,    242,    'X',  null,               "가맹점정보 : 위쳇: 가맹점명(50)+가맹점연락처(15)+가맹점폰번호(15)+가맹점주소(130)+{위쳇:APP_Secret(32)}"));
        RpyShopInfoMsgAttr.put("etx"                , new MsgAttr(29,   "etx",                  544,    1,      'X',  null,               "ETX: 0x03"));
        RpyShopInfoMsgAttr.put("cr"                 , new MsgAttr(30,   "cr",                   545,    1,      'X',  null,               "CR: 0x0D"));

        // 결제정보 통보저장 요청 속성값
        ReqPayNotifyAttr = new HashMap<String, MsgAttr>();
        ReqPayNotifyAttr.put("length"               , new MsgAttr(0,    "length",               0,      4,      '9',    null,               "전문길이: 길이필드 제외 전문길이"));
        ReqPayNotifyAttr.put("stx"                  , new MsgAttr(1,    "stx",                  4,      1,      'X',    new byte[]{0x02},   "구분자: STX(0x02)"));
        ReqPayNotifyAttr.put("deal_type"            , new MsgAttr(2,    "deal_type",            5,      2,      'X',    "BI".getBytes(),    "거래구분: BI(위쳇/알리페이 거래통보)"));
        ReqPayNotifyAttr.put("terminal_div_numb"    , new MsgAttr(3,    "terminal_div_numb",    7,      10,     'X',    null,               "단말기번호: DPT0F00000"));
        ReqPayNotifyAttr.put("comp_info"            , new MsgAttr(4,    "comp_info",            17,     4,      'X',    null,               "업체정보: null"));
        ReqPayNotifyAttr.put("msg_numb"             , new MsgAttr(5,    "msg_numb",             21,     12,     '9',    null,               "전문번호: null"));
        ReqPayNotifyAttr.put("pos_entry_mode"       , new MsgAttr(6,    "pos_entry_mode",       33,     1,      'X',    "Q".getBytes(),     "PosEntryMode: 'Q'(QR결제)"));
        ReqPayNotifyAttr.put("track2"               , new MsgAttr(7,    "track2",               34,     37,     'X',    null,               "TrackII: null"));
        ReqPayNotifyAttr.put("fs"                   , new MsgAttr(8,    "fs",                   71,     1,      'X',    new byte[]{0x1C},   "FS: 0x1C"));
        ReqPayNotifyAttr.put("approval_dist"        , new MsgAttr(9,    "approval_dist",        72,     2,      'X',    "NI".getBytes(),    "조회구분: NI"));
        ReqPayNotifyAttr.put("total_amount"         , new MsgAttr(10,   "total_amount",         74,     9,      '9',    null,               "총금액: 총금액(달러승인인 경우 100 = 1$)"));
        ReqPayNotifyAttr.put("cancle_amount"        , new MsgAttr(11,   "cancle_amount",        83,     9,      '9',    null,               "부분취소금액: 부분취소금액(달러승인인 경우 100 = 1$)"));
        ReqPayNotifyAttr.put("tax_free_amount"      , new MsgAttr(12,   "tax_free_amount",      92,     9,      '9',    null,               "세금(부가세): 세금(부가세)(달러승인인 경우 100 = 1$)"));
        ReqPayNotifyAttr.put("tax_amount"           , new MsgAttr(13,   "tax_amount",           101,    9,      '9',    null,               "과세금액: 과세금액(달러승인인 경우 100 = 1$)"));
        ReqPayNotifyAttr.put("working_key_index"    , new MsgAttr(14,   "working_key_index",    110,    2,      'X',    "AA".getBytes(),    "WorkingKeyIndex: AA"));
        ReqPayNotifyAttr.put("password"             , new MsgAttr(15,   "password",             112,    16,     'X',    null,               "비밀번호: 중국결제시간(14:yyyymmddhhMMss)"));
        ReqPayNotifyAttr.put("approval_numb"        , new MsgAttr(16,   "approval_numb",        128,    12,     'X',    null,               "원거래승인번호: 00000000000000K00hhseq"));
        ReqPayNotifyAttr.put("approval_date"        , new MsgAttr(17,   "approval_date",        140,    6,      'X',    null,               "원거래승인일자: 한국시간(YYMMDD)"));
        ReqPayNotifyAttr.put("user_info"            , new MsgAttr(18,   "user_info",            146,    13,     'X',    null,               "사용자정보: null"));
        ReqPayNotifyAttr.put("shop_id"              , new MsgAttr(19,   "shop_id",              159,    2,      'X',    null,               "가맹점ID: null"));
        ReqPayNotifyAttr.put("shop_reserved"        , new MsgAttr(20,   "shop_reserved",        161,    30,     'X',    null,               "가맹점사용필드: 기관결제통화코드(3)+기관결제금액(7)+부가세(9)(달러승인인 경우 100 = 1$)"));
        ReqPayNotifyAttr.put("point_dist"           , new MsgAttr(21,   "point_dist",           191,    4,      'X',    null,               "포인트구분: 페이사 구분(위쳇WX/알리:AL)"));
        ReqPayNotifyAttr.put("ksnet_reserved"       , new MsgAttr(22,   "ksnet_reserved",       195,    20,     'X',    null,               "KSNET_Reserved: 알리:PartnerID"));
        ReqPayNotifyAttr.put("dongul_dist"          , new MsgAttr(23,   "dongul_dist",          215,    1,      'X',    "N".getBytes(),     "동글구분: 'N' (동글 거래 아님)"));
        ReqPayNotifyAttr.put("pay_div_type"         , new MsgAttr(24,   "pay_div_type",         216,    1,      'X',    "S".getBytes(),     "매체구분: 'S' (스마트폰 결제)"));
        ReqPayNotifyAttr.put("telecom_dist"         , new MsgAttr(25,   "telecom_dist",         217,    1,      'X',    null,               "통신사구분: null"));
        ReqPayNotifyAttr.put("credit_card_type"     , new MsgAttr(26,   "credit_card_type",     218,    1,      'X',    null,               "신용카드종류: null"));
        ReqPayNotifyAttr.put("transaction_type"     , new MsgAttr(27,   "transaction_type",     219,    1,      'X',    null,               "거래형태: null"));
        ReqPayNotifyAttr.put("use_sign"             , new MsgAttr(28,   "use_sign",             220,    1,      'X',    "N".getBytes(),     "전자서명유무 : 'N'"));
        ReqPayNotifyAttr.put("platform_reserved"    , new MsgAttr(29,   "platform_reserved",    221,    200,    'X',    null,               "위챗/알리/추가정보: 인증코드(128)+CNY고객청구금액(10.2)+플랫폼사거래번호(32)"));
        ReqPayNotifyAttr.put("etx"                  , new MsgAttr(30,   "etx",                  421,    1,      'X',    new byte[]{0x03},   "ETX: 0x03"));
        ReqPayNotifyAttr.put("cr"                   , new MsgAttr(31,   "cr",                   422,    1,      'X',    new byte[]{0x0D},   "CR: 0x0D"));

        // 결제정보 통보저장 응답 속성값
        RpyPayNotifyAttr = new HashMap<String, MsgAttr>();
        RpyPayNotifyAttr.put("length"               , new MsgAttr(0,    "length",               0,      4,      '9',    null,               "전문길이: 길이필드 제외 전문길이"));
        RpyPayNotifyAttr.put("stx"                  , new MsgAttr(1,    "stx",                  4,      1,      'X',    null,               "STX: 0x02"));
        RpyPayNotifyAttr.put("deal_type"            , new MsgAttr(2,    "deal_type",            5,      2,      'X',    null,               "거래구분: BJ(위쳇/알리페이 거래통보)"));
        RpyPayNotifyAttr.put("terminal_div_numb"    , new MsgAttr(3,    "terminal_div_numb",    7,      10,     'X',    null,               "단말기번호: null"));
        RpyPayNotifyAttr.put("comp_info"            , new MsgAttr(4,    "comp_info",            17,     4,      'X',    null,               "업체정보: null"));
        RpyPayNotifyAttr.put("msg_numb"             , new MsgAttr(5,    "msg_numb",             21,     12,     '9',    null,               "전문번호: null"));
        RpyPayNotifyAttr.put("status"               , new MsgAttr(6,    "status",               33,     1,      'X',    null,               "Status: 'O'(정상), 'X'(거절)"));
        RpyPayNotifyAttr.put("deal_date"            , new MsgAttr(7,    "deal_date",            34,     12,     'X',    null,               "거래일시: YYMMDDhhmmss"));
        RpyPayNotifyAttr.put("card_type"            , new MsgAttr(8,    "card_type",            46,     1,      'X',    null,               "카드Type: null"));
        RpyPayNotifyAttr.put("message1"             , new MsgAttr(9,    "message1",             47,     16,     'X',    null,               "메시지1: 승인시(카드종류명) / 거절시(거절메시지)"));
        RpyPayNotifyAttr.put("message2"             , new MsgAttr(10,   "message2",             63,     16,     'X',    null,               "메시지2: 승인시(OK) / 거절시(거절메시지)"));
        RpyPayNotifyAttr.put("approval_numb"        , new MsgAttr(11,   "approval_numb",        79,     12,     'X',    null,               "승인번호: 거절인 경우 거절코드(4)"));
        RpyPayNotifyAttr.put("shop_numb"            , new MsgAttr(12,   "shop_numb",            91,     15,     'X',    null,               "가맹점번호: 가맹점 번호"));
        RpyPayNotifyAttr.put("make_comp_gove_code"  , new MsgAttr(13,   "make_comp_gove_code",  106,    2,      'X',    null,               "발급사코드: 카드사 코드(Table참조)"));
        RpyPayNotifyAttr.put("card_type_name"       , new MsgAttr(14,   "card_type_name",       108,    16,     'X',    null,               "카드종류명: 발급사 카드종류명"));
        RpyPayNotifyAttr.put("purc_comp_gove_code"  , new MsgAttr(15,   "purc_comp_gove_code",  124,    2,      'X',    null,               "매입사코드: null"));
        RpyPayNotifyAttr.put("purc_comp_gove_name"  , new MsgAttr(16,   "purc_comp_gove_name",  126,    16,     'X',    null,               "매입사명: null"));
        RpyPayNotifyAttr.put("working_key_index"    , new MsgAttr(17,   "working_key_index",    142,    2,      'X',    null,               "WorkingKeyIndex: nul"));
        RpyPayNotifyAttr.put("working_key"          , new MsgAttr(18,   "working_key",          144,    16,     'X',    null,               "WorkingKey: null"));
        RpyPayNotifyAttr.put("spare_point"          , new MsgAttr(19,   "spare_point",          160,    9,      '9',    null,               "예비포인트: null"));
        RpyPayNotifyAttr.put("point1"               , new MsgAttr(20,   "point1",               169,    9,      '9',    null,               "포인트1: null"));
        RpyPayNotifyAttr.put("point2"               , new MsgAttr(21,   "point2",               178,    9,      '9',    null,               "포인트2: null"));
        RpyPayNotifyAttr.put("point3"               , new MsgAttr(22,   "point3",               187,    9,      'X',    null,               "포인트3: null"));
        RpyPayNotifyAttr.put("notice1"              , new MsgAttr(23,   "notice1",              196,    20,     'X',    null,               "Notice1: null"));
        RpyPayNotifyAttr.put("notice2"              , new MsgAttr(24,   "notice2",              216,    40,     'X',    null,               "Notice2: null"));
        RpyPayNotifyAttr.put("transaction_type"     , new MsgAttr(25,   "transaction_type",     256,    1,      'X',    null,               "거래형태: 'N'(고정)"));
        RpyPayNotifyAttr.put("reserved"             , new MsgAttr(26,   "reserved",             257,    5,      'X',    null,               "Reserved: null"));
        RpyPayNotifyAttr.put("ksnet_reserved"       , new MsgAttr(27,   "ksnet_reserved",       262,    40,     'X',    null,               "KSNET_Reserved: null"));
        RpyPayNotifyAttr.put("etx"                  , new MsgAttr(28,   "etx",                  302,    1,      'X',    null,               "ETX: 0x03"));
        RpyPayNotifyAttr.put("cr"                   , new MsgAttr(29,   "cr",                   303,    1,      'X',    null,               "CR: 0x0D"));

        // 여기에 새로운 속성값 초기화 추가...
        // ...
    }

    // 전문 맴버 변수 //
    private String  msgName;                    // 전문 이름
    private byte[]  msgStream;                  // 전문 바이트스트림
    private Map<String, MsgAttr> msgAttrMap;    // 전문 속성정보를 담은 맵
    
    private String traceId; // 추적 ID
    private Logger logger;  // 로거
    
    // 전문 맴버 함수 //
    public IpgMsg(byte[] _msgStream, Map<String, MsgAttr> _msgAttrMap, String traceId, Logger logger) {
        this.msgStream = _msgStream;
        this.msgAttrMap = _msgAttrMap;
        this.traceId = traceId;
        this.logger = logger;

        // 속성별 이름 초기화
        if (_msgAttrMap == ReqArrovalNumMsgAttr) {
            this.msgName = "ReqAprrovalNumMsg";
        }
        else if (_msgAttrMap == RpyArrovalNumMsgAttr) {
            this.msgName = "RpyAprrovalNumMsg";
        }
        else if (_msgAttrMap == ReqShopInfoMsgAttr) {
            this.msgName = "ReqShopInfoMsg";
        }
        else if (_msgAttrMap == RpyShopInfoMsgAttr) {
            this.msgName = "RpyShopInfoMsg";
        }
        else if (_msgAttrMap == ReqPayNotifyAttr) {
            this.msgName = "ReqPayNotify";
        }
        else if (_msgAttrMap == RpyPayNotifyAttr) {
            this.msgName = "RpyPayNotify";
        }
        else {
            this.msgName = "undefined_attr_msg";
        }

        // 속성값에 맞게 초기화 전문 생성
        if (this.msgStream == null && _msgAttrMap != null) {
            // 전문 속성을 읽어와서 길이 생성
            int szMsg = 0;

            for (String attrKey : _msgAttrMap.keySet()) {
                MsgAttr attr = _msgAttrMap.get(attrKey);
                szMsg += attr.length;
            }

            // 전문 바이트스트림 초기화
            genStreamAndInit(szMsg, _msgAttrMap);
            set("length", String.format("%04d", szMsg));
        }
    }

    // 전문 스트림 할당 및 속성값('9', 'X', ...) 으로 채우기
    private void genStreamAndInit(int szMsg, Map<String, MsgAttr> msgAttrMap) {
        this.msgStream = new byte[szMsg];

        for (String key : msgAttrMap.keySet()) {
            MsgAttr attr = msgAttrMap.get(key);
            set(key, attr.defaults);
        }
    }

    // 전문 속성값을 문자열로 설정
    public boolean set(String key, String newStrStream) {
        byte[] newStream = null;

        if (newStrStream != null) {
            newStream = newStrStream.getBytes();
        }

        return this.set(key, newStream);
    }

    // 전문 속성값을 바이트로 설정
    public boolean set(String key, byte[] newStream) {
        boolean rtResult = false;

        if (this.msgStream == null || key == null) {
            return false;
        }

        try {
            final MsgAttr msgAttr = msgAttrMap.get(key);

            if (msgAttr == null) {
                log(Logger.ERROR, traceId, String.format("[%s] 키값 '%s'를 찾을 수 없음!", this.msgName, key));
                return false;
            }

            if (newStream == null) {
                newStream = msgAttr.defaults; // defaults 도 null일 수 있음
            }

            byte typeFiller = '?';
            int type9_Offset = 0;

            if (msgAttr.type == 'X') {
                typeFiller = ' ';
            }
            else if (msgAttr.type == '9') {
                typeFiller = '0';
            }
            else {
                log(Logger.WARNING, traceId, String.format("[%s] 미정의된 속성 타입 '%c'!", this.msgName, msgAttr.type));
            }

            // 일단 filler값으로 채움
            Arrays.fill(this.msgStream, msgAttr.offset, msgAttr.offset + msgAttr.length, typeFiller);

            // 데이터 존재 시 그 위에 덮어씀
            if (newStream != null) {
                int msgLoseLen = newStream.length - msgAttr.length;
                
                if (msgLoseLen > 0) {
                    String loseMsg = new String(Arrays.copyOfRange(newStream, msgAttr.length, msgAttr.length + msgLoseLen));
                    log(Logger.ERROR, traceId, String.format("[%s] 키값 '%s'의 바이트 길이 초과, [%s](%dbyte)전문 손상 발생!", this.msgName, key, loseMsg, msgLoseLen));
                }

                if (msgAttr.type == '9') {
                    type9_Offset = Math.min(msgAttr.length - newStream.length, 0);
                }

                int offset = msgAttr.offset + type9_Offset;
                System.arraycopy(newStream, 0, this.msgStream, offset, Math.min(newStream.length, msgAttr.length));
            }

            rtResult = true;
        }
        catch (Exception e) {
            log(Logger.ERROR, traceId, "Exception!", e);
        }
        finally {
            return rtResult;
        }
    }

    // 전문 속성값을 문자열로 반환
    public String get(String key) {
        String rtStr = null;

        if (this.msgStream == null || key == null) {
            return null;
        }

        try {
            final MsgAttr msgAttr = msgAttrMap.get(key);

            if (msgAttr == null) {
                log(Logger.ERROR, traceId, "msgAttr == null");
                return null;
            }

            int bgnIdx = msgAttr.offset, endIdx = msgAttr.offset + msgAttr.length;

            if (bgnIdx > endIdx) {
                log(Logger.ERROR, traceId, String.format("'%s'전문의 속성값'%s'의 offset(%d)과 length(%d)검사!", this.msgName, key, msgAttr.offset, msgAttr.length));             
                return null;
            }
            
            byte[] keyStream = Arrays.copyOfRange(this.msgStream, msgAttr.offset, msgAttr.offset + msgAttr.length);

            // 이스케이프 문자 교체
            for (int i = 0; i < keyStream.length; ++i) {
                if (keyStream[i] > (byte)0xFF && keyStream[i] < (byte)0x20) { // 0x00 ~ 0x1F 까지의 제어 문자 (오버플로 제어)
                    keyStream[i] = (byte)0x3F; // '?'
                }
            }

            rtStr = new String(keyStream, IPG_CHARSET);
        }
        catch (Exception e) {
            log(Logger.ERROR, traceId, "Exception!", e);
        }
        finally {
            return rtStr;
        }
    }

    public byte[] getMsgStream() {
        return this.msgStream;
    }

    public void setMsgStream(byte[] msgStream) {
        this.msgStream = msgStream;
    }

    public String getMsgName() {
        return this.msgName;
    }

    public Map<String, MsgAttr> getMsgAttrMap() {
        return this.msgAttrMap;
    }

    // 모든 속성값을 문자열로 반환
    public String getAllAttrStr() {
        List<MsgAttr> attrList = new LinkedList<MsgAttr>(this.msgAttrMap.values());
        StringBuilder sb = new StringBuilder();

        Collections.sort(attrList);

        sb.append('[').append(this.msgName).append("':'").append(attrList.size()).append(']');

        for (MsgAttr msgAttr : attrList) {            
            sb.append("('").append(msgAttr.name).append("':'").append(this.get(msgAttr.name)).append("')");
        }

        return sb.toString();
    }

    // 스트림을 문자열로 반환
    public String getMsgStreamToStr() {
        byte[] byteStreamCopy = Arrays.copyOfRange(this.msgStream, 0, this.msgStream.length);

        for (int i = 0; i < byteStreamCopy.length; ++i) {
            byte b = byteStreamCopy[i];

            if (b > (byte)0xFF && b < (byte)0x20) {
                byteStreamCopy[i] = (byte)0x3F; // '?'
            }
        }

        return new String(byteStreamCopy);
    }

    public String getMsgStreamToHexaStr() {
        StringBuilder hexaSb = new StringBuilder();

        for (byte b : this.msgStream) {
            hexaSb.append(String.format("%02X ", b));
        }

        return hexaSb.toString();
    }

    // IpgMsg 로그생성
    private void log(int logType, String traceId, Object... logObjs) {
        if (logger != null) {
           logger.traceLog(logType, traceId, "[IpgMsg]", logObjs);
        }
	}

    // 전문 속성값 내부정적 클래스 //
    public static class MsgAttr implements Comparable<MsgAttr> {

        public int      number;     // 순번
        public String   name;       // 항목명
        public int      offset;     // 바이트 오프셋
        public int      length;     // 바이트 길이
        public char     type;       // 속성
        public byte[]   defaults;    // 기본값
        public String   desc;       // 내용

        public MsgAttr(int _number, String _name, int _offset, int _length, char _type, byte[] _defaults, String _desc) {
            number      = _number;
            name        = _name;
            offset      = _offset;
            length      = _length;
            type        = _type;
            defaults    = _defaults;
            desc        = _desc;
        }

        @Override
        public int compareTo(MsgAttr msgAttr) {
            return (this.number - msgAttr.number); // 오름차순
        }
    }
}