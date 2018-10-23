package com.ksnet.net;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Map;
import java.util.HashMap;
import java.util.ArrayList;

import com.ksnet.util.*;

public class AttributeManager {
	
	public static final String KEYWORD_ATTRFILES = "attrFiles:";
	
	private static AttributeManager attributeManager; // 싱글턴 클래스
	private static HashMap<String, HashMap<String, Attribute>> attributeMapMap;	// Attribute정보들을 담은 맵을 담은 맵
	private static HashMap<String, Integer> attributeSizeMap; // Attribute들의 크기를 담은 맵
	
	// 생성자
	private AttributeManager() {}
	
	// 초기화
	public static void InitManager(String configFilePath) throws Exception {
		// 싱글턴 객체 생성
		attributeManager = new AttributeManager();
		
		// 해시맵 초기화
		attributeMapMap = new HashMap<String, HashMap<String, Attribute>>();
		attributeSizeMap = new HashMap<String, Integer>();
		
		// 설정파일 내용 문자열 리스트로 반환
		KsFileReader ksFileReader = new KsFileReader(configFilePath);
		ArrayList<String> configStrList = ksFileReader.readLines();
		
		// 설정파일 KEYWORD_ATTRFILES 키워드 검색
		ArrayList<String> configFilePathList = new ArrayList<String>();
		for (String configStr : configStrList) {
			int attrFilesBeginIndex = configStr.indexOf(KEYWORD_ATTRFILES);
			
			if (attrFilesBeginIndex != -1) {
				String configFilePathStr = configStr.substring(attrFilesBeginIndex + KEYWORD_ATTRFILES.length(), configStr.length());
				
				for (String filePath : configFilePathStr.split(",")) {
					configFilePathList.add(filePath.trim());
				}

				break;
			}
		}

		// 속성파일 읽어서 AttributeMapMap 채워넣음
		//for (String attrFilePath : configFilePathList) {
		//	updateAttributeMapMap(attrFilePath);
		//}
		
		// 임시로 하드코딩
		HashMap<String, Attribute> attrMap = new HashMap<String, Attribute>();
		attrMap.put("h_idCode",					new Attribute(1,	"식별코드",		"h_idCode",					"X",	0,	1,	"S".getBytes()			));
		attrMap.put("h_taskComp",				new Attribute(2,	"업무구분",		"h_taskComp",				"X",	1,	2,	"10".getBytes()			));
		attrMap.put("h_bankCode",				new Attribute(3,	"은행코드",		"h_bankCode",				"9",	3,	3,	"081".getBytes()		));
		attrMap.put("h_companyCode",			new Attribute(4,	"업체코드",		"h_companyCode",			"X",	6,	8,	"KSANP001".getBytes()	));
		attrMap.put("h_comissioningDate",		new Attribute(5,	"이체의뢰일자",		"h_comissioningDate",		"9",	14,	6,	"180404".getBytes()		));
		attrMap.put("h_processingDate",			new Attribute(6,	"이체처리일자",		"h_processingDate",			"9",	20,	6,	null					));
		attrMap.put("h_motherAccountNum",		new Attribute(7,	"모계좌번호",		"h_motherAccountNum",		"9",	26,	14,	"25791005094404".getBytes()	));
		attrMap.put("h_transferType",			new Attribute(8,	"이체종류",		"h_transferType",			"9",	40,	2,	"51".getBytes()			));
		attrMap.put("h_companyNum",				new Attribute(9,	"회사번호",		"h_companyNum",				"9",	42,	6,	"000000".getBytes()		));
		attrMap.put("h_resultNotifyType",		new Attribute(10,	"처리결과통보구분",	"h_resultNotifyType",		"X",	48,	1,	"1".getBytes()			));
		attrMap.put("h_transferCnt",			new Attribute(11,	"전송차수",		"h_transferCnt",			"X",	49,	1,	"1".getBytes()			));
		attrMap.put("h_password",				new Attribute(12,	"비밀번호",		"h_password",				"X",	50,	8,	"4380".getBytes()		));
		attrMap.put("h_blank",					new Attribute(13,	"공란",			"h_blank",					"X",	58,	19,	null					));
		attrMap.put("h_format",					new Attribute(14,	"Format",		"h_format",					"X",	77,	1,	"1".getBytes()			));
		attrMap.put("h_van",					new Attribute(15,	"VAN",			"h_van",					"X",	78,	2,	null					));
		attrMap.put("h_newLine",				new Attribute(16,	"개행문자",		"h_newLine",				"X",	80,	2,	null					));
		attributeMapMap.put("HanaAttrClient_Head", attrMap);
		
		attrMap = new HashMap<String, Attribute>();
		attrMap.put("d_idCode",					new Attribute(1,	"식별코드",		"d_idCode",					"X",	0,	1,	"D".getBytes()			));
		attrMap.put("d_dataSerialNum",			new Attribute(2,	"데이터 일련번호",	"d_dataSerialNum",			"9",	1,	6,	null					));
		attrMap.put("d_bankCode",				new Attribute(3,	"은행코드",		"d_bankCode",				"9",	7,	3,	null					));
		attrMap.put("d_accountNum",				new Attribute(4,	"계좌번호",		"d_accountNum",				"X",	10,	14,	null					));
		attrMap.put("d_requestTransferPrice",	new Attribute(5,	"이체요청금액",		"d_requestTransferPrice",	"9",	24,	11,	null					));
		attrMap.put("d_realTransferPrice",		new Attribute(6,	"실제이체금액",		"d_realTransferPrice",		"9",	35,	11,	null					));
		attrMap.put("d_recieverIdNum",			new Attribute(7,	"주민/사업자번호",	"d_recieverIdNum",			"X",	46,	13,	null					));
		attrMap.put("d_processingResult",		new Attribute(8,	"처리결과",		"d_processingResult",		"X",	59,	1,	null					));
		attrMap.put("d_disableCode",			new Attribute(9,	"불능코드",		"d_disableCode",			"X",	60,	4,	null					));
		attrMap.put("d_briefs",					new Attribute(10,	"적요",			"d_briefs",					"X",	64,	12,	null					));
		attrMap.put("d_blank",					new Attribute(11,	"공란",			"d_blank",					"X",	76,	4,	null					));
		attrMap.put("d_newLine",				new Attribute(12,	"개행문자",		"d_newLine",				"X",	80,	2,	null					));
		attributeMapMap.put("HanaAttrClient_Data", attrMap);
		
		attrMap = new HashMap<String, Attribute>();
		attrMap.put("t_idCode",					new Attribute(1,	"식별코드",		"t_idCode",					"X",	0,	1,	"E".getBytes()			));
		attrMap.put("t_totalRequestCnt",		new Attribute(2,	"총의뢰건수",		"t_totalRequestCnt",		"9",	1,	7,	null					));
		attrMap.put("t_totalRequestPrice",		new Attribute(3,	"총의뢰금액",		"t_totalRequestPrice",		"9",	8,	13,	null					));
		attrMap.put("t_normalProcessingCnt",	new Attribute(4,	"정상처리건수",		"t_normalProcessingCnt",	"9",	21,	7,	null					));
		attrMap.put("t_normalProcessingPrice",	new Attribute(5,	"정상처리금액",		"t_normalProcessingPrice",	"9",	28,	13,	null					));
		attrMap.put("t_disableProcessingCnt",	new Attribute(6,	"불능처리건수",		"t_disableProcessingCnt",	"9",	41,	7,	null					));
		attrMap.put("t_disableProcessingPrice",	new Attribute(7,	"불능처리금액",		"t_disableProcessingPrice",	"9",	48,	13,	null					));
		attrMap.put("t_recoveryCode",			new Attribute(8,	"복기부호",		"t_recoveryCode",			"X",	61,	8,	"3706".getBytes()		));
		attrMap.put("t_blank",					new Attribute(9,	"공란",			"t_blank",					"X",	69,	11,	null					));
		attrMap.put("t_newLine",				new Attribute(10,	"개행문자",		"t_newLine",				"X",	80,	2,	null					));
		attributeMapMap.put("HanaAttrClient_Tail", attrMap);
		
		attrMap = new HashMap<String, Attribute>();
		attrMap.put("h_idCode",							new Attribute(1,	"식별코드",		"h_idCode",							"C",	0,		9,	null));
		attrMap.put("h_companyCode",					new Attribute(2,	"업체코드",		"h_companyCode",					"C",	9,		8,	null));
		attrMap.put("h_bankCode2",						new Attribute(3,	"은행코드2",		"h_bankCode2",						"C",	17,		2,	null));
		attrMap.put("h_msgCode",						new Attribute(4,	"메시지코드",		"h_msgCode",						"C",	19,		4,	null));
		attrMap.put("h_workTypeCode",					new Attribute(5,	"업무구분코드",		"h_workTypeCode",					"C",	23,		3,	null));
		attrMap.put("h_transferCnt",					new Attribute(6,	"송신횟수",		"h_transferCnt",					"C",	26,		1,	null));
		attrMap.put("h_msgNum",							new Attribute(7,	"전문번호",		"h_msgNum",							"N",	27,		6,	null));
		attrMap.put("h_transferDate",					new Attribute(8,	"전송일자",		"h_transferDate",					"D",	33,		8,	null));
		attrMap.put("h_transferTime",					new Attribute(9,	"전송시간",		"h_transferTime",					"T",	41,		6,	null));
		attrMap.put("h_responseCode",					new Attribute(10,	"응답코드",		"h_responseCode",					"C",	47,		4,	null));
		attrMap.put("h_bankResponseCode",				new Attribute(11,	"은행 응답코드",		"h_bankResponseCode",				"C",	51,		4,	null));
		attrMap.put("h_lookupDate",						new Attribute(12,	"조회일자",		"h_lookupDate",						"D",	55,		8,	null));
		attrMap.put("h_lookupNum",						new Attribute(13,	"조회번호",		"h_lookupNum",						"N",	63,		6,	null));
		attrMap.put("h_bankMsgNum",						new Attribute(14,	"은행전문번호",		"h_bankMsgNum",						"C",	69,		15,	null));
		attrMap.put("h_bankCode3",						new Attribute(15,	"은행코드3",		"h_bankCode3",						"C",	84,		3,	null));
		attrMap.put("h_spare",							new Attribute(16,	"예비",			"h_spare",							"C",	87,		13,	null));
		attrMap.put("dt_withdrawalAccountNum",			new Attribute(17,	"출금 계좌번호",		"dt_withdrawalAccountNum",			"C",	100,	15,	null));
		attrMap.put("dt_bankBookPassword",				new Attribute(18,	"통장 비밀번호",		"dt_bankBookPassword",				"C",	115,	8,	null));
		attrMap.put("dt_recoveryCode",					new Attribute(19,	"복기부호",		"dt_recoveryCode",					"C",	123,	6,	null));
		attrMap.put("dt_withdrawalAmount",				new Attribute(20,	"출금 금액",		"dt_withdrawalAmount",				"N",	129,	13,	null));
		attrMap.put("dt_afterWithdrawalBalanceSign",	new Attribute(21,	"출금 후 잔액부호",	"dt_afterWithdrawalBalanceSign",	"C",	142,	1,	null));
		attrMap.put("dt_afterWithdrawalBalance",		new Attribute(22,	"출금 후 잔액",		"dt_afterWithdrawalBalance",		"N",	143,	13,	null));
		attrMap.put("dt_depositBankCode2",				new Attribute(23,	"입금 은행코드2",	"dt_depositBankCode2",				"C",	156,	2,	null));
		attrMap.put("dt_depositAccountNum",				new Attribute(24,	"입금 계좌번호",		"dt_depositAccountNum",				"C",	158,	15,	null));
		attrMap.put("dt_fees",							new Attribute(25,	"수수료",			"dt_fees",							"N",	173,	9,	null));
		attrMap.put("dt_transferTime",					new Attribute(26,	"이체 시각",		"dt_transferTime",					"T",	182,	6,	null));
		attrMap.put("dt_depositAccountBriefs",			new Attribute(27,	"입금 계좌 적요",	"dt_depositAccountBriefs",			"C",	188,	20,	null));
		attrMap.put("dt_cmsCode",						new Attribute(28,	"CMS코드",		"dt_cmsCode",						"C",	208,	16,	null));
		attrMap.put("dt_identificationNum",				new Attribute(29,	"신원확인번호",		"dt_identificationNum",				"C",	224,	13,	null));
		attrMap.put("dt_autoTransferClassification",	new Attribute(30,	"자동이체 구분",		"dt_autoTransferClassification",	"C",	237,	2,	null));
		attrMap.put("dt_withdrawalAccountBriefs",		new Attribute(31,	"출금 계좌 적요",	"dt_withdrawalAccountBriefs",		"C",	239,	20,	null));
		attrMap.put("dt_depositBankCode3",				new Attribute(32,	"입금 은행코드3",	"dt_depositBankCode3",				"C",	259,	3,	null));
		attrMap.put("dt_salaryClassification",			new Attribute(33,	"급여 구분",		"dt_salaryClassification",			"C",	262,	1,	null));
		attrMap.put("dt_spare",							new Attribute(34,	"예비",			"dt_spare",							"C",	263,	37,	null));
		attributeMapMap.put("HanaAttrServer", attrMap);
	}
	
	public static AttributeManager getInst() {
		return attributeManager;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	public HashMap<String, Attribute> copyOfAttributeMap(String attributeMapName) {
		HashMap<String, Attribute> rtMap = null;
		HashMap<String, Attribute> attributeMap = null;
			
		if ((attributeMap = attributeMapMap.get(attributeMapName)) != null) {
			rtMap = new HashMap<String, Attribute>();
			
			for (Map.Entry<String, Attribute> entry : attributeMap.entrySet()) { // 딥카피 수행
				rtMap.put(entry.getKey(), new Attribute(entry.getValue()));
			}
		}
		else {
			Logger.logln(Logger.LogType.LT_WARN, "\"" + attributeMapName + "\"값을 키로 갖는 attributeMapMap이 없습니다.");
		}

		return rtMap;
	}
	
	public int getRecordSizeFromAttributeMap(String attributeMapName) {
		int rtInt = -1;
		HashMap<String, Attribute> attributeMap = null;

		if ((attributeSizeMap.get(attributeMapName)) != null) {
			rtInt = attributeSizeMap.get(attributeMapName);
		}
		else {
			if ((attributeMap = attributeMapMap.get(attributeMapName)) != null) {
				rtInt = 0;

				for (Map.Entry<String, Attribute> entry : attributeMap.entrySet()) {
					rtInt += entry.getValue().getByteLength();
				}
				
				attributeSizeMap.put(attributeMapName, rtInt);
			}
			else {
				Logger.logln(Logger.LogType.LT_WARN, "\"" + attributeMapName + "\"값을 키로 갖는 attributeMapMap이 없습니다.");
			}
		}
		
		return rtInt;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	// .attr 파일 파싱
	private void updateAttributeMapMap(String attrFilePath) throws Exception {
		final int META_ROW_CNT = 2;
		final String KEY_NEWLINE = "\r\n";
		final String KEY_COMMENT = "COMMENT";
		final String KEY_STRING = "STRING";
		final String KEY_INT = "INT";
		final String KEY_BYTE = "BYTE";
		
		// 문자열 행렬로 파일 읽기
		final String[][] strAry2D = cvtAttrFile2StringAry2D(attrFilePath);
		final int rowCnt = strAry2D.length - META_ROW_CNT;
		final int colCnt = strAry2D[0].length;

		// 행렬 데이터의 헤더에 따른 열 자료형 리스트 생성
		// (지금은 AttrRecord가 자료형이 정해져있는 '정적'인 클래스지만, 추후 .attr파일 가장 상단의 자료형에 맞춰
		//  '동적'으로 속성들을 관리할 수 있도록 하기 위해 각 자료형이 무엇인지 파악하여 리스트에 저장해 둠.)
		ArrayList<Integer> colIndexList = new ArrayList<Integer>();
		ArrayList<String> colTypeList = new ArrayList<String>();
		
		for (int col = 0; col < colCnt; ++col) {
			String keyWord = strAry2D[0][col];

			// 주석 컬럼
			if (keyWord.equals(KEY_COMMENT)) {}
			// 문자열 컬럼
			else if (keyWord.equals(KEY_STRING)) {
				colIndexList.add(col);
				colTypeList.add(KEY_STRING);
			}
			// 정수 컬럼
			else if (keyWord.equals(KEY_INT)) {
				colIndexList.add(col);
				colTypeList.add(KEY_INT);
			}
			// 바이트 컬럼
			else if (keyWord.equals(KEY_BYTE)) {
				colIndexList.add(col);
				colTypeList.add(KEY_BYTE);
			}
			// 오류 (미정의 키워드)
			else {
				throw new Exception("[오류: 알 수 없는 키워드 (File: " + attrFilePath + "\"" + keyWord + "\", row: " + 0 + ", col: " + col + ")]");
			}
		}
		
		// attrMapMap에 attrMap추가 (HanaAttrClient, HanaAttrServer 전용. 추후 범용 Attribute 클래스로 개선 필요...)
		final String attrFileName = attrFilePath.substring(attrFilePath.lastIndexOf("/") + 1, attrFilePath.lastIndexOf("."));
		int orderCnt = 0;
		HashMap<String, Attribute> attributeMap = new HashMap<String, Attribute>();

		//for (int i = 0; i < colIndexList.size(); ++i) { // (추후 이런식으로...)
			//String varType = colTypeList.get(i);
			//int col = colIndexList.get(i);
			// ......
		//}	
		
		try {
			if (attrFileName.equals("HanaAttrClient") || attrFileName.equals("HanaAttrServer")) { // 하나은행 전문 (클라용/서버용)
				for (int row = 2; row < rowCnt; ++row) {
					String name = strAry2D[row][1];
					String codeName = strAry2D[row][2];
					String type = strAry2D[row][3];
					int beginIndex = Integer.parseInt(strAry2D[row][4]);
					int byteLength = Integer.parseInt(strAry2D[row][5]);
					byte[] defaultValue = strAry2D[row][6].getBytes();

					attributeMap.put(codeName, new Attribute(row - 1, name, codeName, type, beginIndex, byteLength, defaultValue));
				}
			}
			else {
				throw new Exception("[오류: (" + attrFileName + ")은 미지원 전문 파일입니다.]");
			}
		}
		catch (Exception e) {
			Logger.logln(Logger.LogType.LT_ERR, e);
		}
		
		attributeMapMap.put(attrFileName, attributeMap);
	}
	
	private String[][] cvtAttrFile2StringAry2D(String attrFilePath) {
		// .attr파일 문자열로 가공
		KsFileReader ksFileReader = new KsFileReader(attrFilePath);
		ArrayList<String> attrFileStrList = ksFileReader.readLines();
		
		StringBuilder strBuilder = new StringBuilder();
		for (String lineStr : attrFileStrList) {
			System.out.print(">" + lineStr + "\r\n");
			strBuilder.append(lineStr).append("\r\n");
		}
		System.out.println();
		
		// 행렬의 크기 구하기
		String fileStr = strBuilder.toString();
		String[] rowStrAry = fileStr.split("\r\n");			// 열 데이터 배열
		final int rowCnt = rowStrAry.length - 1;			// 행의 개수 (마지막 빈 행 제외)
		final int colCnt = rowStrAry[0].split("\t").length;	// 열의 개수
		String[][] strAry2D = new String[rowCnt][colCnt];	// 행렬 데이터 배열
		
		// 원본 데이터를 행렬화
		for (int row = 0; row < rowCnt; ++row) {
			String[] colStrAry = rowStrAry[row].split("\t");
			
			for (int col = 0; col < colCnt; ++col) {
				if (col < colStrAry.length) {
					strAry2D[row][col] = colStrAry[col];
				}
				else {
					strAry2D[row][col] = "";
				}
			}
		}
		
		return strAry2D;
	}
}