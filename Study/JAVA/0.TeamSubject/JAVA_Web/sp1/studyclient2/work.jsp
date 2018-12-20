<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<%@ page language="java" contentType="text/html; charset=EUC-KR" pageEncoding="EUC-KR"%>

<%@ page import="java.util.Enumeration"%>
<%@ page import="java.io.BufferedReader"%>
<%@ page import="java.io.File"%>
<%@ page import="java.io.FileOutputStream"%>
<%@ page import="java.io.FileReader"%>
<%@ page import="java.io.FileWriter"%>
<%@ page import="java.io.InputStream"%>
<%@ page import="java.io.IOException"%>
<%@ page import="java.io.OutputStream"%>
<%@ page import="java.io.RandomAccessFile"%>
<%@ page import="java.net.Socket"%>
<%@ page import="java.net.SocketAddress"%>
<%@ page import="java.nio.channels.IllegalBlockingModeException"%>
<%@ page import="java.nio.file.Files"%>
<%@ page import="java.nio.file.StandardCopyOption"%>
<%@ page import="java.text.SimpleDateFormat"%>
<%@ page import="java.util.Arrays"%>
<%@ page import="java.util.ArrayList"%>
<%@ page import="java.util.Collections"%>
<%@ page import="java.util.Date"%>
<%@ page import="java.util.Map"%>
<%@ page import="java.util.HashMap"%>
<%@ page import="java.util.List"%>
<%@ page import="java.util.LinkedList"%>
<%@ page import="java.util.Scanner"%>

<html>
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=EUC-KR">
    <title>index</title>
</head>
<body>
    <%!	
		////////////////////////////////////////////////////////////////////////////////////////////////
		//[ClientMain.java]/////////////////////////////////////////////////////////////////////////////
		
		public static class ClientMain {	
			// 메인
			public static void main(String[] args) {
				// 변수
				long beginTime = 0, endTime = 0, runTime = 0;
				HashMap<String, byte[]> envVarMap = new HashMap<String, byte[]>();
				KsFileReader ksFileReader = null;
				KsFileWriter ksFileWriter = null;
				RecordConverter svrRecordConverter = null, cliRecordConverter = null;
				RecordTransceiver recordTransceiver = null;
				RecordPrinter recordPrinter = null;
				
				// 초기화
				try {
					System.out.println("========================================================================");

					beginTime = System.currentTimeMillis();
					
					init(envVarMap, args);
					
					endTime = System.currentTimeMillis();
					runTime = endTime - beginTime;
					
					System.out.println("> 초기화 완료 : " + (runTime / 1000.0) + "초");
					beginTime = System.currentTimeMillis();
					
					////////////////////////////////////////////////////////////////////////////////////////////////////
					
					// 파일 읽기 작업과 비동기 쓰기 작업을 위한 클래스 (비동기 쓰기 작업중, 서버->클라 레코드 변환 수행)
					String inFilePath = new String(envVarMap.get("INPUT_FILE_PATH"));
					String outFilePath = new String(envVarMap.get("OUTPUT_FILE_PATH"));
					
					KsFileWriter.copyFromFile(new File(inFilePath), new File(outFilePath)); // 원본 복사
					
					ksFileReader = new KsFileReader(inFilePath);
					ksFileWriter = new KsFileWriter(outFilePath, AttributeManager.getInst().getRecordSizeFromAttributeMap("HanaAttrClient_Data"));
					
					// 클라->서버->클라 레코드 변환을 위한 클래스
					svrRecordConverter = new RecordConverter(null, "HanaRecordServer", "", envVarMap); // 서버로 변환
					cliRecordConverter = new RecordConverter(null, "HanaRecordClient", "", envVarMap); // 클라로 변환
					
					// 서버에 데이터를 전송할 클래스
					String svrIp = new String(envVarMap.get("FB_IP"));
					int svrPort = Integer.parseInt(new String(envVarMap.get("FB_PORT")));
					

					boolean reusableSocketMode = new String(envVarMap.get("REUSABLE_SOCKET_MODE")).toUpperCase().equals("TRUE") ? true : false;
					int socketCnt = Integer.parseInt(new String(envVarMap.get("SOCKET_CNT")));
					
					recordTransceiver = new RecordTransceiver(reusableSocketMode, svrIp, svrPort, socketCnt, 50,
															  cliRecordConverter, ksFileWriter, envVarMap);
					
					// 레코드 출력을 위한 클래스
					recordPrinter = new RecordPrinter(null);
					
					// 출력 파일 표제부 쓰기
					cliRecordConverter.setOutRecordSubTypeName("Head");
					ksFileWriter.write(cliRecordConverter.convert(new Record("HanaRecordServer", "", 0, null)).toByteAry(), 0);
				}
				catch (Exception e) {
					Logger.logln(Logger.LogType.LT_CRIT, e);
					finishProgram(recordTransceiver, cliRecordConverter, svrRecordConverter, ksFileReader, ksFileWriter, 0);
				}
				
				// 전문 파일을 라인별로 읽으면서 변환 및 전송, 파일로 쓰기 수행
				int lineCnt = 0;
				
				try {
					for (; ; ++lineCnt) {
						// 전문 파일 라인별 읽기
						String recordStr = ksFileReader.readLine();
						
						// 더이상 읽을 라인이 없으면 탈출
						if (recordStr == null) break;

						// 읽은 라인으로 레코드 생성
						Record cliRecord = new Record("HanaRecordClient", lineCnt, recordStr.getBytes());
						
						// 서버 레코드로 변환
						Record toSvrRecord = svrRecordConverter.convert(cliRecord);
						
						// 데이터부만 서버로 전송
						while (true) {
							sleep(1);
							
							if (recordTransceiver.isSendWaittingListFull()) {
								continue;
							}
							else {
								if (toSvrRecord != null) {
									recordTransceiver.send(toSvrRecord);
									
									if (lineCnt % 10 == 0) {
										recordTransceiver.printWorkLeft();
									}
								}

								break;
							}
						}
					}
					
					// 모든 레코드의 전송과 파일 쓰기가 완료될 때까지 대기
					long printInterval = 1000;
					long nextSysMsgTime = 0;
					
					while (!recordTransceiver.checkTransceiverFinished()) {
						if (System.currentTimeMillis() > nextSysMsgTime) {
							recordTransceiver.printWorkLeft();
							nextSysMsgTime = System.currentTimeMillis() + printInterval;
						}
						
						sleep(1);
					}
				}
				catch (Exception e) {
					Logger.logln(Logger.LogType.LT_CRIT, e);
				}
				finally {
					finishProgram(recordTransceiver, cliRecordConverter, svrRecordConverter, ksFileReader, ksFileWriter, lineCnt);

					////////////////////////////////////////////////////////////////////////////////////////////////////
					
					// 종료
					endTime = System.currentTimeMillis();
					runTime = endTime - beginTime;
					System.out.println("> 서버 송수신 완료 : " + (runTime / 1000.0) + "초");
					System.out.println("========================================================================");
				}
			}
			
			// 초기화
			public static void init(HashMap<String, byte[]> envVarMap, String[] args) {
				// 환경변수 해시 초기화
				if (envVarMap == null) return;
				
				envVarMap.put("INPUT_FILE_PATH", args[0].getBytes()); 
				envVarMap.put("OUTPUT_FILE_PATH", args[1].getBytes());
				envVarMap.put("ATTR_CONFIG_FILE_PATH", args[2].getBytes());
				envVarMap.put("OUTPUT_LOG_PATH", args[3].getBytes());

				envVarMap.put("FB_IP", args[4].getBytes());
				envVarMap.put("FB_PORT", args[5].getBytes());
				envVarMap.put("FB_PARENT_BANK_CODE_3", args[6].getBytes());
				envVarMap.put("FB_PARENT_COMP_CODE", args[7].getBytes());
				envVarMap.put("FB_PARENT_ACCOUNT_NUMB", args[8].getBytes());
				envVarMap.put("FB_REQ_FILE", args[9].getBytes());
				envVarMap.put("FB_MSG_NUMB_S", args[10].getBytes());
				envVarMap.put("FB_PARENT_COMP_NAME", args[11].getBytes());
				
				envVarMap.put("REUSABLE_SOCKET_MODE", args[12].getBytes());
				envVarMap.put("SOCKET_CNT", args[13].getBytes());
				envVarMap.put("SOCKET_THREAD_TIMEOUT", args[14].getBytes());
				envVarMap.put("RECORD_RESEND_MAX_TRY", args[15].getBytes());
				envVarMap.put("RECORD_RESEND_DELAY", args[16].getBytes());
				envVarMap.put("RECORD_TGT_SEND_PER_SEC", args[17].getBytes());
				envVarMap.put("LOG_LEVEL", args[18].getBytes());

				envVarMap.put("MessageCode_0100", "0100".getBytes());
				envVarMap.put("MessageCode_0600", "0600".getBytes());
				envVarMap.put("WorkTypeCode_100", "100".getBytes());
				envVarMap.put("WorkTypeCode_101", "101".getBytes());
				envVarMap.put("WorkTypeCode_300", "300".getBytes());
				envVarMap.put("WorkTypeCode_400", "400".getBytes());
				envVarMap.put("ProcessingResultOk", "0000".getBytes());
				
				// 매니저 초기화
				try {
					AttributeManager.InitManager(new String(envVarMap.get("ATTR_CONFIG_FILE_PATH")));
				}
				catch (Exception e) {
					Logger.logln(Logger.LogType.LT_ERR, e);
				}
				
				// 로거 초기화
				String logLevel = new String(envVarMap.get("LOG_LEVEL"));
				
				Logger.setVisibleByLogLevel(logLevel);
			}
			
			// 프로그램 마무리
			public static void finishProgram(RecordTransceiver recordTransceiver, RecordConverter cliRecordConverter, RecordConverter svrRecordConverter, KsFileReader ksFileReader, KsFileWriter ksFileWriter, int lineCnt) {
				try {
					// 읽은 파일 닫기
					if (ksFileReader != null) ksFileReader.close();
					
					// 전송기 종료
					if (recordTransceiver != null) recordTransceiver.close();
					
					// 출력 파일 종료부 쓰기
					if (cliRecordConverter != null) cliRecordConverter.setOutRecordSubTypeName("Tail");
					if (ksFileWriter != null) ksFileWriter.write(cliRecordConverter.convert(new Record("HanaRecordServer", "", lineCnt, null)).toByteAry(), lineCnt - 1);
					
					// 레코드 변환기 및 파일 닫기
					if (cliRecordConverter != null) cliRecordConverter.close();
					if (ksFileWriter != null) ksFileWriter.close();
				}
				catch (Exception e) {
					System.out.println("========================================================================");
					System.exit(-1);
				}
			}
			
			// 스레드 대기
			public static void sleep(int ms) {
				try {
					Thread.sleep(ms);
				}
				catch (InterruptedException ie) {
					Logger.logln(Logger.LogType.LT_ERR, ie);
				}
			}
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////
		//[Attribute.java]//////////////////////////////////////////////////////////////////////////////
		
		public static class Attribute {
			private int number;
			private String name;
			private String codeName;
			private String type;
			private int beginIndex;
			private int byteLength;
			private byte[] defaultValue;
			private byte[] value;
			
			public Attribute(int number, String name, String codeName, String type, int beginIndex, int byteLength, byte[] defaultValue) {
				this.number = number;
				this.name = name;
				this.codeName = codeName;
				this.type = type;
				this.beginIndex = beginIndex;
				this.byteLength = byteLength;
				this.defaultValue = defaultValue;
				
				if (defaultValue == null) {
					this.value = new byte[byteLength];
				}
				else {
					this.value = defaultValue;
				}
			}
			
			public Attribute(Attribute attribute) {
				this.copyFrom(attribute);
			}
			
			public void copyFrom(Attribute attribute) {
				setNumber(attribute.number);
				setName(new String(attribute.name));
				setCodeName(new String(attribute.codeName));
				setType(new String(attribute.type));
				setBeginIndex(attribute.beginIndex);
				setByteLength(attribute.byteLength);
				
				if (attribute.defaultValue != null) {
					setDefaultValue(Arrays.copyOfRange(attribute.defaultValue, 0, attribute.defaultValue.length));
				}
				else {
					attribute.defaultValue = null;
				}
				
				if (attribute.value != null) {
					setValue(Arrays.copyOfRange(attribute.value, 0, attribute.value.length));
				}
				else {
					setValue(null);
				}
			}
			
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			
			public int getNumber() {
				return number;
			}
			
			public void setNumber(int number) {
				this.number = number;
			}
			
			public String getName() {
				return name;
			}
			
			public void setName(String name) {
				this.name = name;
			}
			
			public String getCodeName() {
				return codeName;
			}
			
			public void setCodeName(String codeName) {
				this.codeName = codeName;
			}
			
			public String getType() {
				return type;
			}
			
			public void setType(String type) {
				this.type = type;
			}
			
			public int getBeginIndex() {
				return beginIndex;
			}
			
			public void setBeginIndex(int beginIndex) {
				this.beginIndex = beginIndex;
			}
			
			public int getByteLength() {
				return byteLength;
			}
			
			public void setByteLength(int byteLength) {
				this.byteLength = byteLength;
			}
			
			public byte[] getDefaultValue() {
				return defaultValue;
			}
			
			public void setDefaultValue(byte[] defaultValue) {
				this.defaultValue = defaultValue;
			}
			
			public byte[] getValue() {
				return value;
			}
			
			public void setValue(final byte[] value) {
				if (value != null) {
					// 길이 체크
					if (value.length > byteLength) {
						// System.out.println("[WARN] : value.length > byteLength 데이터 손실 가능성이 있습니다. (" + value.length + " > " + byteLength + " codeName: [" + codeName + "], value: [" + new String(value) + "])");
					}
				}
					
				// 타입별로 value값 수정
				byte[] valCpy = new byte[byteLength];
				
				if (type.equals("X") || type.equals("C")) { // 공백(' ') 패딩, 좌로 정렬
					Arrays.fill(valCpy, (byte)' ');
					
					if (value != null) {
						for (int i = 0; i < byteLength; ++i) {
							if (i == value.length) break;
							
							valCpy[i] = value[i];
						}
					}
				}
				//else if (type.equals("")) { // 숫자 '0' 패딩, 좌로 정렬
				//	Arrays.fill(valCpy, (byte)'0');
				//	
				//	if (value != null) {
				//		for (int i = 0; i < byteLength; ++i) {
				//			if (i == value.length) break;
				//			
				//			valCpy[i] = value[i];
				//		}
				//	}
				//}
				else if (type.equals("9") || type.equals("N")) { // 숫자 '0' 패딩, 우로 정렬
					Arrays.fill(valCpy, (byte)'0');
					
					if (value != null) {
						int valueI = value.length - 1;
						for (int cpyI = byteLength - 1; cpyI > -1; --cpyI) {
							
							valCpy[cpyI] = value[valueI--];
							
							if (valueI < 0) break;
						}
					}
				}
				else { // 공백(' ') 패딩, 좌로 정렬
					Arrays.fill(valCpy, (byte)' ');
					
					if (value != null) {
						for (int i = 0; i < byteLength; ++i) {
							if (i == value.length) break;
							
							valCpy[i] = value[i];
						}
					}
				}
				
				this.value = valCpy;
			}
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////
		//[AttributeManager.java]///////////////////////////////////////////////////////////////////////
		
		public static class AttributeManager {
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
		
		////////////////////////////////////////////////////////////////////////////////////////////////
		//[KsFileReader.java]///////////////////////////////////////////////////////////////////////////
		
		public static class KsFileReader {
			private String filePath;
			private File file = null;
			private FileReader fileReader = null;
			private BufferedReader bufferedReader = null;
			
			public KsFileReader(String filePath) {
				this.filePath = filePath;
			}
			
			public String readLine() {
				String rtString = null;
				
				try {
					if (file == null) file = new File(filePath);
					if (fileReader == null) fileReader = new FileReader(file);
					if (bufferedReader == null) bufferedReader = new BufferedReader(fileReader);

					if ((rtString = bufferedReader.readLine()) == null) {
						try {				
							if (bufferedReader != null) bufferedReader.close();
							if (fileReader != null) fileReader.close();
						}
						catch (IOException ioe1) {
							Logger.logln(Logger.LogType.LT_ERR, ioe1);
						}
					}
				}
				catch (IOException ioe2) {
					ioe2.printStackTrace();
				}
				finally {
					return rtString;
				}
			}
			
			public ArrayList<String> readLines() {
				ArrayList<String> rtList = new ArrayList<String>();
				
				try {
					if (file == null) file = new File(filePath);
					if (fileReader == null) fileReader = new FileReader(file);
					if (bufferedReader == null) bufferedReader = new BufferedReader(fileReader);
					String lineData = "";
					
					while ((lineData = bufferedReader.readLine()) != null) {
						rtList.add(new String(lineData));
					}
				}
				catch (IOException ioe1) {
					Logger.logln(Logger.LogType.LT_ERR, ioe1);
				}
				finally {
					try {				
						if (bufferedReader != null) bufferedReader.close();
						if (fileReader != null) fileReader.close();
					}
					catch (IOException ioe2) {
						Logger.logln(Logger.LogType.LT_ERR, ioe2);
					}
					
					return rtList;
				}
			}
			
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			
			public String getFilePath() {
				return filePath;
			}
			
			public void setFilePath(String filePath) {
				this.filePath = filePath;
			}
			
			public void close() {
				try {
					if (bufferedReader != null) bufferedReader.close();
					if (fileReader != null) fileReader.close();
				}
				catch (IOException ioe) {
					Logger.logln(Logger.LogType.LT_ERR, ioe);
				}
			}
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////
		//[KsFileWriter.java]///////////////////////////////////////////////////////////////////////////
		
		public static class KsFileWriter {
			private String filePath;
			private int bytePerLine;
			private RandomAccessFile randomAccessFile;
				
			public KsFileWriter(String filePath, int bytePerLine) {
				this.filePath = filePath;
				this.bytePerLine = bytePerLine;
			}
			
			public synchronized void write(byte[] str, long pos) {
				try {
					if (randomAccessFile == null) randomAccessFile = new RandomAccessFile(filePath, "rw");
					
					if (pos != -1) randomAccessFile.seek(pos * bytePerLine);
					
					randomAccessFile.write(str);
				}
				catch (IOException ioe) {
					Logger.logln(Logger.LogType.LT_ERR, "OutputStream 열기 혹은 쓰기 오류.");
					Logger.logln(Logger.LogType.LT_ERR, ioe);
				}
			}
			
			public void close() {
				try {
					randomAccessFile.close();
				}
				catch (IOException ioe) {
					Logger.logln(Logger.LogType.LT_ERR, "OutputStream 닫기 오류.");
					Logger.logln(Logger.LogType.LT_ERR, ioe);
				}
			}
			
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			
			public String getFilePath() {
				return filePath;
			}
			
			public int getBytePerLine() {
				return bytePerLine;
			}
			
			public void setBytePerLine(int bytePerLine) {
				this.bytePerLine = bytePerLine;
			}
			
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			
			public static void copyFromFile(File srcFile, File destFile) throws Exception {
				Files.copy(srcFile.toPath(), destFile.toPath(), StandardCopyOption.REPLACE_EXISTING);
			}
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////
		//[Record.java]/////////////////////////////////////////////////////////////////////////////////
		
		public static class Record {
			private String typeName;
			private String subTypeName;
			private int index;
			private HashMap<String, Attribute> attrMap;
			
			private int sendCnt;		// 전송 횟수
			private long lastSendTime;	// 마지막 전송 시간
			
			public Record (String typeName, int index, byte[] datas) {
				this(typeName, "", index, datas);
			}
			
			public Record(String typeName, String subTypeName, int index, byte[] datas) {
				this.typeName = typeName;
				this.subTypeName = subTypeName;
				this.index = index;
				this.attrMap = null;
				updateRecord(datas);
				
				this.sendCnt = 0;
				this.lastSendTime = 0;
			}
			
			public String getTypeName() {
				return typeName;
			}
			
			public void setTypeName(String typeName) {
				this.typeName = typeName;
			}
			
			public String getSubTypeName() {
				return subTypeName;
			}
			
			public void setSubTypeName(String subTypeName) {
				this.subTypeName = subTypeName;
			}
			
			public int getIndex() {
				return index;
			}
			
			public void setIndex(int index) {
				this.index = index;
			}
			
			public HashMap<String, Attribute> getAttrMap() {
				return attrMap;
			}
			
			public int getSendCnt() {
				return sendCnt;
			}
			
			public void setSendCnt(int sendCnt) {
				this.sendCnt = sendCnt;
			}
			
			public void addSendCnt(int add) {
				this.sendCnt += add;
			}
			
			public long getLastSendTime() {
				return lastSendTime;
			}
			
			public void setLastSendTime(long lastSendTime) {
				this.lastSendTime = lastSendTime;
			}
			
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			
			public void setData(String attributeName, final byte[] data) {
				attrMap.get(attributeName).setValue(data);
			}
			
			public byte[] getData(String attributeName) {
				Attribute attr = attrMap.get(attributeName);
				
				if (attr == null) {
					Logger.logln(Logger.LogType.LT_WARN, "\"" + attributeName + "\"을 키로 갖는 attr이 null입니다.");
					return null;
				}
				
				return attrMap.get(attributeName).getValue();
			}
			
			public void setDataByDefault(String attributeName) {
				Attribute attr = attrMap.get(attributeName);
				attr.setValue(attr.getDefaultValue());
			}
			
			public byte[] toByteAry() {
				int totalByteLength = 0;
				int attrSize = attrMap.size();
				Attribute[] attrAry = new Attribute[attrSize];
				
				for (Map.Entry<String, Attribute> entry : attrMap.entrySet()) {
					Attribute attr = entry.getValue();
					int number = attr.getNumber();
					
					attrAry[number - 1] = attr;
					totalByteLength += attr.getByteLength();
				}
				
				byte[] rtByte = new byte[totalByteLength];
				int rtByteI = 0;
				
				for (Attribute attr : attrAry) {
					byte[] attrValue = attr.getValue();
					
					for (int i = 0; i < attrValue.length; ++i) {
						rtByte[rtByteI++] = attrValue[i];
					}
				}
				
				return rtByte;
			}
			
			public static ArrayList<Record> makeRecordList(String name, ArrayList<String> recordStringList) {
				ArrayList<Record> rtList = new ArrayList<Record>();
				int indexCnt = 0;
				
				for (String recordStr : recordStringList) {
					rtList.add(new Record(name, indexCnt++, recordStr.getBytes()));
				}
				
				return rtList;
			}
			
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			
			private void updateRecord(byte[] datas) {
				if (typeName.equals("HanaRecordClient")) {
					update_hana_record(datas, true);
				}
				else if (typeName.equals("HanaRecordServer")) {
					update_hana_record(datas, false);
				}
			}
			
			private void update_hana_record(byte[] datas, boolean isClientRecord) {
				String indexKeyStr = null;

				// KS Data
				if (isClientRecord) {	
					final String attrName = "HanaAttrClient";
					// 개행 문자 추가
					byte[] copyDatas = null;
					
					if (datas == null) {
						copyDatas = new byte[AttributeManager.getInst().getRecordSizeFromAttributeMap(attrName)];
					}
					else {
						copyDatas = Arrays.copyOfRange(datas, 0, datas.length + 2);
						copyDatas[datas.length] = '\r';
						copyDatas[datas.length + 1] = '\n';
					}
					
					// 표제부, 데이터부, 종료부 구분
					byte idCode = copyDatas[0];

					if (idCode == (byte)'D' || subTypeName.equals("Data")) { // 데이터부
						this.attrMap = AttributeManager.getInst().copyOfAttributeMap(attrName + "_Data");
						this.subTypeName = "Data";
					}
					else if (idCode == (byte)'S' || subTypeName.equals("Head")) { // 표제부
						this.attrMap = AttributeManager.getInst().copyOfAttributeMap(attrName + "_Head");
						this.subTypeName = "Head";
					}
					else if (idCode == (byte)'E' || subTypeName.equals("Tail")) { // 종료부
						this.attrMap = AttributeManager.getInst().copyOfAttributeMap(attrName + "_Tail");
						this.subTypeName = "Tail";
					}
					else { // 오류
						Logger.logln(Logger.LogType.LT_ERR, "알 수 없는 idCode값. (idCode: \"" + idCode + "\")");
						return;
					}
					
					// 속성 맵을 조회하여 바이트 데이터를 필요에 맞게 잘라 저장
					for (Attribute attr : attrMap.values()) {
						int beginIndex = attr.getBeginIndex();
						int byteLength = attr.getByteLength();
						int endIndex = beginIndex + byteLength;

						attr.setValue(Arrays.copyOfRange(copyDatas, beginIndex, endIndex));
					}
					
					indexKeyStr = "d_dataSerialNum"; // 데이터 일련번호 -> 레코드 인덱스
				}
				// Server Data
				else {
					final String attrName = "HanaAttrServer";
					this.attrMap = AttributeManager.getInst().copyOfAttributeMap(attrName);
					
					byte[] copyData = null;
					
					if (datas == null) {
						copyData = new byte[AttributeManager.getInst().getRecordSizeFromAttributeMap(attrName)];
					}
					else {
						copyData = Arrays.copyOfRange(datas, 0, datas.length);
					}
					
					for (Attribute attr : attrMap.values()) {
						int beginIndex = attr.getBeginIndex();
						int byteLength = attr.getByteLength();
						int endIndex = beginIndex + byteLength;
						
						attr.setValue(Arrays.copyOfRange(copyData, beginIndex, endIndex));
					}
					
					indexKeyStr = "h_msgNum"; // 전문번호 -> 레코드 인덱스
				}
				
				// 레코드 인덱스 업데이트 (-1 : 오토 인덱싱)
				if (index == -1) {
					if (indexKeyStr != null) {
						byte[] indexByte = getData(indexKeyStr);
						
						if (indexByte != null) {
							this.index = Integer.parseInt(new String(indexByte));
						}
					}
				}
			}
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////
		//[KsFileWriter.java]///////////////////////////////////////////////////////////////////////////
		
		public static class RecordConverter {
			public byte[] NEW_LINE = { '\r', '\n' };

			private Record inRecord;						// 입력 레코드
			private String outRecordTypeName;				// 출력 레코드 타입명
			private String outRecordSubTypeName;			// 출력 레코드 서브타입명
			private HashMap<String, byte[]> envVarMap;		// 환경변수 해시맵
			private HashMap<String, byte[]> localVarMap;	// 지역변수 해시맵

			public RecordConverter(Record inRecord, String outRecordTypeName, String outRecordSubTypeName, HashMap<String, byte[]> envVarMap) {
				this.inRecord = inRecord;
				this.outRecordTypeName = outRecordTypeName;
				this.outRecordSubTypeName = outRecordSubTypeName;
				this.envVarMap = envVarMap;
				this.localVarMap = new HashMap<String, byte[]>();
			}
			
			public Record convert() {
				String inRecordTypeName = inRecord.getTypeName();
				
				// 하나은행 전문변환
				if (inRecordTypeName.equals("HanaRecordServer") || inRecordTypeName.equals("HanaRecordClient")) {
					return hana_record_convertion(outRecordTypeName);
				}
				
				return null;
			}
			
			public Record convert(Record inRecord) {
				setInRecord(inRecord);
				return convert();
			}
			
			public void close() {
				inRecord = null;
				outRecordSubTypeName = null;
				outRecordSubTypeName = null;
				envVarMap = null;
				localVarMap.clear();
				localVarMap = null;
			}
			
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			
			public Record getInRecord() {
				return inRecord;
			}
			
			public void setInRecord(Record inRecord) {
				this.inRecord = inRecord;
			}
			
			public String getOutRecordTypeName() {
				return outRecordTypeName;
			}
			
			public void setOutRecordTypeName(String outRecordTypeName) {
				this.outRecordTypeName = outRecordTypeName;
			}
			
			public String getOutRecordSubTypeName() {
				return outRecordSubTypeName;
			}
			
			public void setOutRecordSubTypeName(String outRecordSubTypeName) {
				this.outRecordSubTypeName = outRecordSubTypeName;
			}
			
			public HashMap<String, byte[]> getEnvVarMap() {
				return envVarMap;
			}
			
			public void setEnvVarMap(HashMap<String, byte[]> envVarMap) {
				this.envVarMap = envVarMap;
			}
			
			public HashMap<String, byte[]> getLocalVarMap() {
				return localVarMap;
			}
			
			public void setLocalVarMap(HashMap<String, byte[]> localVarMap) {
				this.localVarMap = localVarMap;
			}
				
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			
			private Record hana_record_convertion(String outRecordTypeName) {
				Record rtRecord = null;
				AttributeManager attrMgr = AttributeManager.getInst();
				
				// 시간 포매팅
				SimpleDateFormat dateTime = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
				String today = dateTime.format(new Date(System.currentTimeMillis()));
				String year = today.substring(0, 4),	month = today.substring(5, 7),	date = today.substring(8, 10);
				String hour = today.substring(11, 13),	min = today.substring(14, 16),	sec = today.substring(17, 19);
				
				// Client -> Server
				if (outRecordTypeName.equals("HanaRecordServer")) {
					String inRecordTypeName = inRecord.getTypeName();
					String inRecordSubTypeName = inRecord.getSubTypeName();
					
					if (inRecordTypeName.equals("HanaRecordClient")) {
						if (inRecordSubTypeName.equals("Data")) {
							int recordLength = attrMgr.getRecordSizeFromAttributeMap("HanaAttrServer");
							byte[] dummyDatas = new byte[recordLength];
							rtRecord = new Record("HanaRecordServer", inRecord.getIndex(), dummyDatas);
							
							// [1.공통부 (100Byte)]
							// 식별 코드 (0~8)
							rtRecord.setData("h_idCode", null);
							
							// 업체코드 (9~16)
							rtRecord.setData("h_companyCode", envVarMap.get("FB_PARENT_COMP_CODE"));
							
							// 은행코드2 (17~18)
							final byte[] bankCode3 = envVarMap.get("FB_PARENT_BANK_CODE_3");
							final byte[] bankCode2 = Arrays.copyOfRange(bankCode3, 1, 3);
							rtRecord.setData("h_bankCode2", bankCode2);
							
							// 메시지코드 (19~22)
							final byte[] messageCode = { '0', '1', '0', '0' };
							rtRecord.setData("h_msgCode", messageCode);
							
							// 업무구분코드 (23~25)
							final byte[] workTypeCode = localVarMap.get("loc_hana_svr_h_workTypeCode");
							rtRecord.setData("h_workTypeCode", workTypeCode);
							
							// 송신횟수 (26)
							final byte[] transferCnt = { '1' };
							rtRecord.setData("h_transferCnt", transferCnt);
							
							// 전문번호 (27~32)
							rtRecord.setData("h_msgNum", inRecord.getData("d_dataSerialNum"));
							
							// 전송일자 (33~40)
							final byte[] transferDate = { (byte)year.charAt(0),  (byte)year.charAt(1), (byte)year.charAt(2), (byte)year.charAt(3),
														  (byte)month.charAt(0), (byte)month.charAt(1), 
														  (byte)date.charAt(0),  (byte)date.charAt(1) };
							rtRecord.setData("h_transferDate", transferDate);
							
							// 전송시간 (41~46)
							final byte[] transferTime = { (byte)hour.charAt(0), (byte)hour.charAt(1),
														  (byte)min.charAt(0),  (byte)min.charAt(1),
														  (byte)sec.charAt(0),  (byte)sec.charAt(1) };
							rtRecord.setData("h_transferTime", transferTime);
							
							// 응답코드 (47~50)
							rtRecord.setData("h_responseCode", null);
							
							// 은행응답코드 (51~54)
							rtRecord.setData("h_bankResponseCode", null);
							
							// 조회일자 (55~62)
							rtRecord.setData("h_lookupDate", null);
							
							// 조회번호 (63~68)
							rtRecord.setData("h_lookupNum", null);
							
							// 은행전문번호 (69~83)
							rtRecord.setData("h_bankMsgNum", null);
							
							// 은행코드3 (84~86)
							rtRecord.setData("h_bankCode3", bankCode3);
							
							// 예비 (87~99)
							rtRecord.setData("h_spare", null);
							
							// [2.개별부 (200Byte)]
							// [송급이체/지급이체]
							if (Arrays.equals(messageCode, envVarMap.get("MessageCode_0100"))) {
								if (Arrays.equals(workTypeCode, envVarMap.get("WorkTypeCode_100"))) {
									// 출금계좌번호 (100~114)
									rtRecord.setData("dt_withdrawalAccountNum", envVarMap.get("FB_PARENT_ACCOUNT_NUMB"));
									
									// 통장비밀번호 (115~122)
									rtRecord.setData("dt_bankBookPassword", null);
									
									// 복기부호 (123~128)
									rtRecord.setData("dt_recoveryCode", null);
									
									// 출금금액 (129~141)
									rtRecord.setData("dt_withdrawalAmount", inRecord.getData("d_requestTransferPrice"));
									
									// 출금후잔액부호 (142)
									rtRecord.setData("dt_afterWithdrawalBalanceSign", null);
									
									// 출금후잔액 (143~155)
									rtRecord.setData("dt_afterWithdrawalBalance", null);
									
									// 입금은행코드2 (156~157)
									final byte[] depositBankCode3 = inRecord.getData("d_bankCode");
									final byte[] depositBankCode2 = Arrays.copyOfRange(depositBankCode3, 1, 3);
									rtRecord.setData("dt_depositBankCode2", depositBankCode2);
									
									// 입금계좌번호 (158~172)
									rtRecord.setData("dt_depositAccountNum", inRecord.getData("d_accountNum"));
									
									// 수수료 (173~181)
									rtRecord.setData("dt_fees", null);
									
									// 이체시각 (182~187)
									rtRecord.setData("dt_transferTime", transferTime);
									
									// 입금계좌적요 (188~207)
									rtRecord.setData("dt_depositAccountBriefs", envVarMap.get("FB_PARENT_COMP_NAME"));
									
									// CMS코드 (208~223)
									rtRecord.setData("dt_cmsCode", null);
									
									// 신원확인번호 (224~236)
									rtRecord.setData("dt_identificationNum", null);
									
									// 자동이체구분 (237~238)
									rtRecord.setData("dt_autoTransferClassification", null);
									
									// 출금계좌적요 (239~258)
									rtRecord.setData("dt_withdrawalAccountBriefs", inRecord.getData("d_briefs"));
									
									// 입금은행코드3 (259~261)
									rtRecord.setData("dt_depositBankCode3", depositBankCode3);
									
									// 급여구분 (262)
									rtRecord.setData("dt_salaryClassification", null);
									
									// 예비 (263~299)
									rtRecord.setData("dt_spare", null);
								}
								// [오류]
								else {
									Logger.logln(Logger.LogType.LT_ERR, "알 수 없는 메시지코드. (workTypeCode: " + new String(workTypeCode) + ", messageCode: " + new String(messageCode) + ")");
									return null;
								}
							}
							// [처리결과조회,잔액조회,계좌조회]
							else if(Arrays.equals(messageCode, envVarMap.get("MessageCode_0600"))) {
								// [처리결과조회]
								if (Arrays.equals(workTypeCode, envVarMap.get("WorkTypeCode_101"))) {
									// 추후 기능 추가...
								}
								// [잔액조회]
								else if (Arrays.equals(workTypeCode, envVarMap.get("WorkTypeCode_300"))) {
									// 추후 기능 추가...
								}
								// [계좌조회]
								else if (Arrays.equals(workTypeCode, envVarMap.get("WorkTypeCode_400"))) {
									// 추후 기능 추가...
								}
								// [오류]
								else {
									Logger.logln(Logger.LogType.LT_ERR, "알 수 없는 업무종류코드. (" + new String(workTypeCode) + ")");
									return null;
								}
							}
							// [오류]
							else {
								Logger.logln(Logger.LogType.LT_ERR, "알 수 없는 메시지코드. (workTypeCode: " + new String(workTypeCode) + ", messageCode: " + new String(messageCode) + ")");
								return null;
							}
						}
						else if (inRecordSubTypeName.equals("Head")) {
							// 표제부 - 업무구분코드 로컬 데이터 저장
							byte[] cliWorkTypeCode = inRecord.getData("h_taskComp");
							byte[] svrWorkTypeCode = new byte[cliWorkTypeCode.length + 1];
							
							for (int i = 0; i < cliWorkTypeCode.length; ++i) {
								svrWorkTypeCode[i] = cliWorkTypeCode[i];
							}
							
							svrWorkTypeCode[cliWorkTypeCode.length] = (byte)'0';
							localVarMap.put("loc_hana_svr_h_workTypeCode", svrWorkTypeCode);
						}
						else if (inRecordSubTypeName.equals("Tail")) {
							// 종료부 - 저장할 데이터 없음
						}
					}
					else {
						Logger.logln(Logger.LogType.LT_ERR, "올바르지 않은 inRecordTypeName. (\"" + inRecordTypeName + "\")");
					}
				}
				// Server -> Client
				else if (outRecordTypeName.equals("HanaRecordClient")) {
					if (outRecordSubTypeName.equals("Data")) {
						// [데이터부 (82Byte)]
						int recordLength = attrMgr.getRecordSizeFromAttributeMap("HanaAttrClient_Data");
						byte[] dummyDatas = new byte[recordLength];
						rtRecord = new Record("HanaRecordClient", "Data", inRecord.getIndex(), dummyDatas);

						// 식별코드 (0)
						rtRecord.setDataByDefault("d_idCode");
						
						// 데이터 일련번호 (1~6)
						rtRecord.setData("d_dataSerialNum", inRecord.getData("h_msgNum"));
						
						// 은행코드 (7~10)
						rtRecord.setData("d_bankCode", inRecord.getData("dt_depositBankCode3"));
			
						// 계좌번호 (10~23)
						rtRecord.setData("d_accountNum", inRecord.getData("dt_depositAccountNum"));
						
						// 이체요청금액 (24~34)
						final byte[] withdrawlAmount = inRecord.getData("dt_withdrawalAmount");
						rtRecord.setData("d_requestTransferPrice", withdrawlAmount);
						{
							// 총 의뢰횟수 증가
							final byte[] savedTotalRequestCnt = localVarMap.get("loc_hana_cli_t_totalRequestCnt");
							if (savedTotalRequestCnt != null) { 
								localVarMap.put("loc_hana_cli_t_totalRequestCnt", Long.toString(Long.parseLong(new String(savedTotalRequestCnt)) + 1).getBytes());
							}
							else {
								localVarMap.put("loc_hana_cli_t_totalRequestCnt", "1".getBytes());
							}
							
							// 총 의뢰금액 증가
							final byte[] savedTotalRequestAmount = localVarMap.get("loc_hana_cli_t_totalRequestPrice");
							if (savedTotalRequestAmount != null) { 
								localVarMap.put("loc_hana_cli_t_totalRequestPrice", Long.toString(Long.parseLong(new String(savedTotalRequestAmount)) + Long.parseLong(new String(withdrawlAmount))).getBytes());
							}
							else {
								localVarMap.put("loc_hana_cli_t_totalRequestPrice", withdrawlAmount);
							}
						}

						// 실제이체금액 (35~45)
						rtRecord.setData("d_realTransferPrice", inRecord.getData("dt_withdrawalAmount"));
						
						// 주민/사업자번호 (46~58)
						rtRecord.setData("d_recieverIdNum", null);
						
						// 처리결과 (59)
						final byte[] bankResponseCode = inRecord.getData("h_bankResponseCode");
						if (Arrays.equals(bankResponseCode, envVarMap.get("ProcessingResultOk"))) { // 정상 처리
							final byte[] procY = { 'Y' };
							rtRecord.setData("d_processingResult", procY);
							{
								// 정상처리건수 증가
								final byte[] savedNormalProcCnt = localVarMap.get("loc_hana_cli_t_normalProcessingCnt");
								if (savedNormalProcCnt != null) {
									localVarMap.put("loc_hana_cli_t_normalProcessingCnt", Long.toString(Long.parseLong(new String(savedNormalProcCnt)) + 1).getBytes());
								}
								else {
									localVarMap.put("loc_hana_cli_t_normalProcessingCnt", "1".getBytes());
								}
								
								// 정상처리금액 증가
								final byte[] savedNormalPriceCnt = localVarMap.get("loc_hana_cli_t_normalPriceCnt");
								if (savedNormalPriceCnt != null) { 
									localVarMap.put("loc_hana_cli_t_normalPriceCnt", Long.toString(Long.parseLong(new String(savedNormalPriceCnt)) + Long.parseLong(new String(withdrawlAmount))).getBytes());
								}
								else {
									localVarMap.put("loc_hana_cli_t_normalPriceCnt", withdrawlAmount);
								}
							}
						}
						else { // 불능 처리
							final byte[] procN = { 'N' };
							rtRecord.setData("d_processingResult", procN);
							{
								// 불능처리건수 증가
								final byte[] savedDisableProcCnt = localVarMap.get("loc_hana_cli_t_disableProcessingCnt");
								if (savedDisableProcCnt != null) {
									localVarMap.put("loc_hana_cli_t_disableProcessingCnt", Long.toString(Long.parseLong(new String(savedDisableProcCnt)) + 1).getBytes());
								}
								else {
									localVarMap.put("loc_hana_cli_t_disableProcessingCnt", "1".getBytes());
								}
								
								// 불능처리금액 증가
								final byte[] savedDisablePriceCnt = localVarMap.get("loc_hana_cli_t_disablePriceCnt");
								if (savedDisablePriceCnt != null) { 
									localVarMap.put("loc_hana_cli_t_disablePriceCnt", Long.toString(Long.parseLong(new String(savedDisablePriceCnt)) + Long.parseLong(new String(withdrawlAmount))).getBytes());
								}
								else {
									localVarMap.put("loc_hana_cli_t_disablePriceCnt", withdrawlAmount);
								}
							}
						}
						
						// 불능코드 (60~63)
						rtRecord.setData("d_disableCode", bankResponseCode);
						
						// 적요 (64~75)
						rtRecord.setData("d_briefs", inRecord.getData("dt_withdrawalAccountBriefs"));
						
						// 공란 (76~79)
						rtRecord.setData("d_blank", null);
						
						// 개행문자 (80~81)
						rtRecord.setData("d_newLine", NEW_LINE);
					}
					else if (outRecordSubTypeName.equals("Head")) {
						// [표제부 (82Byte)]
						int recordLength = attrMgr.getRecordSizeFromAttributeMap("HanaAttrClient_Head");
						byte[] dummyDatas = new byte[recordLength];
						rtRecord = new Record("HanaRecordClient", "Head", 0, dummyDatas);
						
						// 식별 코드 (0)
						rtRecord.setDataByDefault("h_idCode");
						
						// 업무 구분 (1~2)
						rtRecord.setDataByDefault("h_taskComp");
						
						// 은행 코드 (3~5)
						rtRecord.setDataByDefault("h_bankCode");
						
						// 업체 코드 (6~13)
						rtRecord.setDataByDefault("h_companyCode");
						
						// 이체의뢰일자 (14~19)
						rtRecord.setDataByDefault("h_comissioningDate");
						
						// 이체처리일자 (20~25)
						final byte[] h_processingDate = { (byte)year.charAt(0),  (byte)year.charAt(1), (byte)year.charAt(2), (byte)year.charAt(3),
														  (byte)month.charAt(0), (byte)month.charAt(1), 
														  (byte)date.charAt(0),  (byte)date.charAt(1) };
						rtRecord.setData("h_processingDate", h_processingDate);
						
						// 모계좌번호 (26~39)
						rtRecord.setDataByDefault("h_motherAccountNum");
						
						// 이체종류 (40~41)
						rtRecord.setDataByDefault("h_transferType");
						
						// 회사번호 (42~47)
						rtRecord.setDataByDefault("h_companyNum");
						
						// 처리결과통보구분 (48)
						rtRecord.setDataByDefault("h_resultNotifyType");
						
						// 전송차수 (49)
						rtRecord.setDataByDefault("h_transferCnt");
						
						// 비밀번호 (50~57)
						rtRecord.setDataByDefault("h_password");
						
						// 공란 (58~76)
						rtRecord.setData("h_blank", null);
						
						// Format (77)
						rtRecord.setDataByDefault("h_format");
						
						// VAN (78~79)
						final byte[] bVan = { 'K', 'C' };
						rtRecord.setData("h_van", bVan);
						
						// 개행문자 (80~81)
						rtRecord.setData("h_newLine", NEW_LINE);
					}
					else if (outRecordSubTypeName.equals("Tail")) {
						// [종료부 (82Byte)]
						int recordLength = attrMgr.getRecordSizeFromAttributeMap("HanaAttrClient_Tail");
						byte[] dummyDatas = new byte[recordLength];
						
						rtRecord = new Record("HanaRecordClient", "Tail", Integer.parseInt(new String(localVarMap.get("loc_hana_cli_t_totalRequestCnt"))), dummyDatas);
						
						// 식별코드 (0)
						rtRecord.setDataByDefault("t_idCode");
						
						// 총의뢰건수 (1~7)
						rtRecord.setData("t_totalRequestCnt", localVarMap.get("loc_hana_cli_t_totalRequestCnt"));
						
						// 총의뢰금액 (8~20)
						rtRecord.setData("t_totalRequestPrice", localVarMap.get("loc_hana_cli_t_totalRequestPrice"));
						
						// 정상처리건수 (21~27)
						rtRecord.setData("t_normalProcessingCnt", localVarMap.get("loc_hana_cli_t_normalProcessingCnt"));
						
						// 정상처리금액 (28~40)
						rtRecord.setData("t_normalProcessingPrice", localVarMap.get("loc_hana_cli_t_normalPriceCnt"));
						
						// 불능처리건수 (41~47)
						rtRecord.setData("t_disableProcessingCnt", localVarMap.get("loc_hana_cli_t_disableProcessingCnt"));
						
						// 불능처리금액 (48~60)
						rtRecord.setData("t_disableProcessingPrice", localVarMap.get("loc_hana_cli_t_disablePriceCnt"));
						
						// 복기부호 (61~68)
						rtRecord.setDataByDefault("t_recoveryCode");
						
						// 공란 (69~79)
						rtRecord.setData("t_blank", null);
						
						// 개행문자 (80~81)
						rtRecord.setData("t_newLine", NEW_LINE);
					}
				}
				
				return rtRecord;
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////
		//[RecordPrinter.java]///////////////////////////////////////////////////////////////////////////
		
		public static class RecordPrinter {
			private Record record;
			
			// 생성자
			public RecordPrinter(Record record) {
				this.record = record;
			}
			
			public void setRecord(Record record) {
				this.record = record;
			}
			
			// 양식별 출력함수
			// 하나은행_지급이체_표제부
			private void print_hana_record() {
				int attrSize = record.getAttrMap().size();
				Attribute[] attrAry = new Attribute[attrSize];
				
				for (Map.Entry<String, Attribute> entry : record.getAttrMap().entrySet()) {
					Attribute attr = entry.getValue();
					int number = attr.getNumber();
					
					attrAry[number - 1] = attr;
				}
			
				System.out.println(String.format("%s\t%-40s\t%s\t%s\t%-20s", "순번", "이름(코드명)", "시작인덱스", "바이트길이", "현재값"));
				for (Attribute attr : attrAry) {
					System.out.println(String.format("%3d\t%-40s\t%4d\t\t%4d\t\t%-20s", attr.getNumber(), (attr.getName() + String.format("(%s)", attr.getCodeName())), attr.getBeginIndex(), attr.getByteLength(), new String(attr.getValue())));
				}		
			}

			public void print(Record record) {
				setRecord(record);
				print();
			}
			
			public void print() {
				String recordType = record.getTypeName();
				
				// 하나은행_지급이체_표제부_데이터부_종료부 양식 출력
				if (recordType.equals("HanaRecordClient")) {
					String recordSubType = record.getSubTypeName();
					
					if (recordSubType.equals("Head")) {
						System.out.println("=[표제부]=================================================================================");
						print_hana_record();
						System.out.println("=================================================================================[표제부]=");
					}
					else if (recordSubType.equals("Data")) {
						System.out.println("=[데이터부]================================================================================");
						print_hana_record();
						System.out.println("================================================================================[데이터부]=");
					}
					else if (recordSubType.equals("Tail")) {
						System.out.println("=[종료부]=================================================================================");
						print_hana_record();
						System.out.println("=================================================================================[종료부]=");
					}			
				}
				// 원화 펌뱅킹 양식 출력
				else if (recordType.equals("HanaRecordServer")) {
					print_hana_record();
				}
				else if (false) {
					// 여기에 새로운 출력타입 추가...
				}
				else {
					System.out.println(this.getClass().getName());
					Logger.logln(Logger.LogType.LT_ERR, "지원하지 않는 전문 종류. (" + recordType + ")");
					return;
				}
			}
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////
		//[RecordTransceiver.java]//////////////////////////////////////////////////////////////////////
		
		public static class RecordTransceiver {
			private boolean reusableSocketMode;				// 재사용 소켓 사용(true), 일회용 소켓 사용(false) 모드

			private String ip;								// 서버 아이피
			private int port;								// 서버 포트
			private int sendTryCnt;							// 전송 시도 개수
			private long sendDelay;							// 전송 대기 시간
			private int sendWaitingRecordListMaxSize;		// 최대 전송대기 리스크 크기
			
			private List<Socket> socketList;					// 송수신 소켓 리스트 (Thread-Safe ArrayList)
			private List<Record> sendWaittingRecordList;		// 보내야 할 데이터 리스트 (Thread-Safe LinkedList)
			private List<Record> fileWriteLeftRecordList;		// 파일 작성작업이 남은 레코드 (Thread-Safe LinkedList)
			private List<Integer> fileWriteDoneRecordIndexList;	// 파일 작성작업이 완료된 레코드 인덱스 (Thread-Safe ArrayList)

			private RecordConverter cliRecordConverter;		// 서버에서 받은 레코드를 클라이언트 형식으로 변경하는 클래스
			private KsFileWriter ksFileWriter;				// .rpy 파일에 최종 결과를 쓰는 클래스
			private TransceiveLogger fileLogger;			// 송수신 로거
			
			private SendSocketThread sendSocketThread;		// 전송용 소켓 스레드 구현부
			private RecvSocketThread recvSocketThread;		// 수신용 소켓 스레드 구현부
			private Thread sendThread;						// 전송용 스레드
			private Thread recvThread;						// 수신용 스레드
			
			private HashMap<String, byte[]> envVarMap;		// 환경변수
			
			public RecordTransceiver (boolean reusableSocketMode, String ip, int port, int socketCnt, int sendWaitingRecordListMaxSize, RecordConverter cliRecordConverter, KsFileWriter ksFileWriter, HashMap<String, byte[]> envVarMap) throws Exception {
				this.reusableSocketMode = reusableSocketMode;
				
				this.ip = ip;
				this.port = port;
				this.sendTryCnt = 0;
				this.sendDelay = 1000;					// 초기 전송 대기시간 1000ms
				this.sendWaitingRecordListMaxSize = 50;	// 최대 50개까지 전송대기 리스트에 저장 가능
				
				this.socketList = Collections.synchronizedList(new ArrayList<Socket>());			// Thread-Safe 소켓 리스트
				
				if (this.reusableSocketMode) {
					addSocket(socketCnt);
				}

				this.sendWaittingRecordList = Collections.synchronizedList(new LinkedList<Record>());	// Thread-Safe 보내야 할 데이터 리스트
				this.fileWriteLeftRecordList = Collections.synchronizedList(new LinkedList<Record>());	// Thread-Safe 파일 작성작업이 남은 레코드 리스트
				this.fileWriteDoneRecordIndexList = Collections.synchronizedList(new ArrayList<Integer>());
				
				this.cliRecordConverter = cliRecordConverter;
				this.ksFileWriter = ksFileWriter;
				this.fileLogger = new TransceiveLogger(new String(envVarMap.get("OUTPUT_LOG_PATH")));
				
				final byte[] bPrefix = { 0x02 }, bSuffix = { 0x03 };
				this.sendSocketThread = new SendSocketThread(this, ip, port, socketList, fileWriteLeftRecordList, fileWriteDoneRecordIndexList, bPrefix, bSuffix, sendWaittingRecordList, fileLogger, envVarMap);
				this.recvSocketThread = new RecvSocketThread(this, ip, port, socketList, fileWriteLeftRecordList, fileWriteDoneRecordIndexList, bPrefix, bSuffix, cliRecordConverter, ksFileWriter, fileLogger, envVarMap);
				
				sendThread = new Thread(sendSocketThread);
				recvThread = new Thread(recvSocketThread);
				sendThread.start();
				recvThread.start();
				
				this.envVarMap = envVarMap;
			}
			
			public long getSendDelay() {
				return sendDelay;
			}
			
			public synchronized void setSendDelay(long sendDelay) {
				this.sendDelay = sendDelay;
			}
			
			public boolean isReusableSocketMode() {
				return reusableSocketMode;
			}
			
			public int getSendWaitingRecordListMaxSize() {
				return sendWaitingRecordListMaxSize;
			}
			
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			
			// 해당 개수만큼 소캣 생성 (소켓은 삭제 불가, 추가만 가능)
			public void addSocket(int addCnt) {
				for (int i = 0; i < addCnt; ++i) {
					boolean hasError = false;
						
					try {
						socketList.add(new Socket(ip, port));
					}
					catch (IOException ioe) {
						// If an error occurs during the connection.
						Logger.logln(Logger.LogType.LT_ERR, ioe);
						Logger.logln(Logger.LogType.LT_ERR, "소켓 접속 오류. (Ip: " + ip + ", Port: " + port + ")");
						hasError = true;
					}
					catch (IllegalBlockingModeException ibme) {
						// If ths socket has an associated channel, and the channel is in non-blocking mode.
						Logger.logln(Logger.LogType.LT_ERR, ibme);
						Logger.logln(Logger.LogType.LT_ERR, "서버 접속 오류. (non-blocking mode)");
						hasError = true;
					}
					catch (IllegalArgumentException iae) {
						// If endpoint is null or a SocketAddress subclass not supported by this socket.
						Logger.logln(Logger.LogType.LT_ERR, iae);
						Logger.logln(Logger.LogType.LT_ERR, "접속 주소 오류. (Ip: " + ip + ", Port: " + port + ")");
						hasError = true;
					}
					finally {
						if (hasError) {
							Logger.logln(Logger.LogType.LT_CRIT, "IP: " + ip + ", Port: " + port + " 서버 접속 실패. 서버 상태 확인바랍니다.");
							System.exit(-1);
						}
					}
				}
			}
			
			private void destroySocketAll() {
				for (Socket socket : socketList) {
					try {
						socket.close();
						socket = null;
					}
					catch (Exception e) {
						Logger.logln(Logger.LogType.LT_ERR, e);
						Logger.logln(Logger.LogType.LT_ERR, "소켓 닫기 실패.");
					}
				}
				
				socketList.clear();
			}
			
			// 송수신기 닫기
			public void close() {
				try {
					// Send 스레드 종료
					sendSocketThread.close();
					sendThread.join();
				}
				catch (Exception e) {
					Logger.logln(Logger.LogType.LT_ERR, e);
				}
				
				try {
					// Recv 스레드 종료
					recvSocketThread.close();
					recvThread.join();
				}
				catch (Exception e) {
					Logger.logln(Logger.LogType.LT_ERR, e);
				}
				
				// 로그파일 작성
				fileLogger.writeToFile();
				
				// 소켓 파괴
				destroySocketAll();
				
				// 정보 출력
				int recvFailedCnt = fileWriteLeftRecordList.size();
				int recvSuccessCnt = sendTryCnt - recvFailedCnt;
				float recvFailedPercent = recvFailedCnt * 100.0f / sendTryCnt;
				float recvSuccessPercent = 100.0f - recvFailedPercent;
				
				System.out.println();
				System.out.println(Global.FC_GREEN + String.format("***** 전송 시도 개수: %d, 수신 성공 개수: %d(%.02f%%), 수신 실패 개수: %d(%.02f%%) ******", sendTryCnt, recvSuccessCnt, recvSuccessPercent, recvFailedCnt, recvFailedPercent) + Global.FC_RESET);
				
				if (recvFailedCnt > 0) {
					System.out.print(Global.FC_GREEN + String.format("***** 실패한 레코드 인덱스: ", sendTryCnt, recvSuccessCnt, recvSuccessPercent, recvFailedCnt, recvFailedPercent) + Global.FC_RESET);
				
					for (Record failedRecord : fileWriteLeftRecordList) {
						System.out.print(Global.FC_GREEN + failedRecord.getIndex() + ", " + Global.FC_RESET);
					}
				}
				System.out.println();
				
				// 초기화
				sendWaittingRecordList.clear();
				fileWriteLeftRecordList.clear();
			}
			
			// 전송 레코드 추가
			public void send(Record sendRecord) {
				sendWaittingRecordList.add(sendRecord);
				++sendTryCnt;
			}
			
			// 송수신 완료 여부 확인
			public boolean checkTransceiverFinished() {
				// 두 스레드 모드 타임아웃인 경우 true
				if ((sendSocketThread.getThreadTimeoutLeft() == 0 && recvSocketThread.getThreadTimeoutLeft() == 0)) {
					return true;
				}
				
				return false;
			}
			
			// 송수신 개수등 정보 출력
			public void printWorkLeft() {
				System.out.println();
				System.out.println(Global.FC_YELLOW + "*** [ Time: " + System.currentTimeMillis() + " ]" + Global.FC_RESET);
				System.out.println(Global.FC_YELLOW + "*** 전송 대기중인 레코드: " + sendWaittingRecordList.size() + "개 / 수신 대기중인 레코드: " + fileWriteLeftRecordList.size() + "개" + Global.FC_RESET);
				System.out.println(Global.FC_YELLOW + "*** 작성 완료 레코드 : " + recvSocketThread.getFileWriteDoneRecordIndexListSize() + "개 / 현재 전송 대기 시간: " + getSendDelay() + "ms" + Global.FC_RESET);
				System.out.println(Global.FC_YELLOW + "*** 초당 전송 속도/제한: " + String.format("%.2f/%d개", (sendThread.isAlive() ? sendSocketThread.getCurSendPerSec() : 0.00f), (sendThread.isAlive() ? sendSocketThread.getTgtSendPerSec() : 0)) + Global.FC_RESET);
				System.out.println(Global.FC_YELLOW + "*** 전송 스레드 타임아웃: " + sendSocketThread.getThreadTimeoutLeft() + "ms / 수신 스레드 타임아웃: " + recvSocketThread.getThreadTimeoutLeft() + "ms" + Global.FC_RESET);
				System.out.println();
			}
			
			// 전송 대기 리스트 크기 제한 확인
			public boolean isSendWaittingListFull() {
				if (sendWaittingRecordList.size() >= sendWaitingRecordListMaxSize) {
					return true;
				}
				
				return false;
			}
			
			public SendSocketThread getSendSocketThread() {
				if (sendThread.isAlive()) return sendSocketThread;
				else return null;
			}
			
			public RecvSocketThread getRecvSocketThread() {
				if (recvThread.isAlive()) return recvSocketThread;
				else return null;
			}
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////
		//[RecordTransceiver.java - SocketThread]///////////////////////////////////////////////////////
		
		public static class SocketThread implements Runnable {
			public long DEFAULT_THREAD_TIMEOUT;	// n초 동안 아무 반응이 없으면 스레드 타임아웃
			public int MAX_RESEND_COUNT;		// 최대 n번 재전송 허용
			public int DEFAULT_RESEND_DELAY;	// n초 이내 수신받지 못하면 재전송 시도
			
			protected RecordTransceiver recordTransceiver;	// 스레드 컨트롤러 RecordTransceiver 클래스
			
			protected String ip;				// 서버 IP
			protected int port;					// 서버 Port
			
			protected boolean running;			// 스레드 제어
			protected long threadTimeoutLeft;	// 남은 스레드 대기시간 (밀리초)
			protected long lastTime;			// 마지막 시간 (밀리초)
			protected long timeDelta;			// 한 틱 경과 시간 (밀리초)
			protected long workDelayLeft;		// 작업 시작전 남은 대기시간
			
			protected List<Socket> socketList;						// 송수신 소켓 리스트 (Thread-Safe ArrayList)
			protected List<Record> fileWriteLeftRecordList;			// 파일 작성작업이 남은 레코드 Thread-Safe LinkedList)
			protected List<Integer> fileWriteDoneRecordIndexList; 	// 파일 작성작업이 완료된 레코드 인덱스 (Thread-Safe ArrayList)
			protected int curWorkSocketIndex;		// 현재 사용해야 할 소켓 인덱스
			protected TransceiveLogger fileLogger;	// 송수신 로거
			
			protected HashMap<String, byte[]> envVarMap;	// 환경변수
			
			public SocketThread(RecordTransceiver recordTransceiver, String ip, int port, List<Socket> socketList, List<Record> fileWriteLeftRecordList, List<Integer> fileWriteDoneRecordIndexList, TransceiveLogger fileLogger, HashMap<String, byte[]> envVarMap) {		
				this.envVarMap = envVarMap;
				
				try {
					DEFAULT_THREAD_TIMEOUT = Long.parseLong(new String(envVarMap.get("SOCKET_THREAD_TIMEOUT")));
					MAX_RESEND_COUNT = Integer.parseInt(new String(envVarMap.get("RECORD_RESEND_MAX_TRY")));
					DEFAULT_RESEND_DELAY = Integer.parseInt(new String(envVarMap.get("RECORD_RESEND_DELAY")));
				}
				catch (NullPointerException npe) {
					Logger.logln(Logger.LogType.LT_ERR, npe.getMessage());
					DEFAULT_THREAD_TIMEOUT = 10000;
					MAX_RESEND_COUNT = 5;
					DEFAULT_RESEND_DELAY = 5000;
				}
				
				this.recordTransceiver = recordTransceiver;
				
				this.ip = ip;
				this.port = port;
				
				this.running = true;
				this.threadTimeoutLeft = DEFAULT_THREAD_TIMEOUT;
				this.lastTime = System.currentTimeMillis();
				this.timeDelta = 0;
				this.workDelayLeft = 0;
				
				this.socketList = socketList;
				this.fileWriteLeftRecordList = fileWriteLeftRecordList;
				this.fileWriteDoneRecordIndexList = fileWriteDoneRecordIndexList;
				
				this.curWorkSocketIndex = 0;
				this.fileLogger = fileLogger;
			}
			
			@Override
			public void run() { }

			public boolean isRunning() {
				return running;
			}
			
			public void setRunning(boolean running) {
				this.running = running;
			}
			
			public long getThreadTimeoutLeft() {
				return threadTimeoutLeft;
			}
			
			public void setThreadTimeoutLeft(long threadTimeoutLeft) {
				this.threadTimeoutLeft = threadTimeoutLeft;
			}
			
			public long getWorkDelayLeft() {
				return workDelayLeft;
			}
			
			public void setWorkDelayLeft(long workDelayLeft) {
				this.workDelayLeft = workDelayLeft;
			}
			
			public int getCurWorkSocketIndex() {
				return curWorkSocketIndex;
			}
			
			public synchronized LinkedList<Record> fileWriteLeftRecordListSyncWork(String work, Record record) {
				work.toLowerCase();
				
				if (work.equals("get")) {
					return new LinkedList<Record>(fileWriteLeftRecordList);
				}
				else if (work.equals("add")) {
					fileWriteLeftRecordList.add(record);
				}
				else if (work.equals("remove")) {
					for (Record removeRecord : fileWriteLeftRecordList) {
						if (removeRecord.getIndex() == record.getIndex()) {
							fileWriteLeftRecordList.remove(removeRecord);
							break;
						}
					}
				}
				else if (work.equals("clear")) {
					fileWriteLeftRecordList.clear();
				}
				
				return null;
			}
			
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			
			// 스레드 시간 업데이트
			protected boolean updateThreadTime() {
				long curTime = System.currentTimeMillis();
				timeDelta = curTime - lastTime;
				
				// 스레드가 타임아웃이면 false
				if ((threadTimeoutLeft -= timeDelta) <= 0) {
					threadTimeoutLeft = 0;
					lastTime = curTime;
					setRunning(false);
					return false;
				}
				
				// 스레드 딜레이가 0 이상인 경우 두 가지 모드로 대기
				if (true) {
					// Thread Sleep Mode
					if (workDelayLeft > 0) {
						try {
							Thread.sleep(workDelayLeft);
						} catch (InterruptedException ie) {
							Logger.logln(Logger.LogType.LT_ERR, ie);
						}
						
						workDelayLeft = 0;
					}
				}
				else {
					// Busy Waiting Mode
					//if ((workDelayLeft -= timeDelta) > 0) {
					//	lastTime = curTime;
					//	return false;
					//}
					//else {
					//	workDelayLeft = 0;
					//}
				}
				
				lastTime = curTime;
				
				return true;
			}
			
			// 바이트 배열로 레코드 생성
			protected Record makeRecord(String recordType, int recordIndex, byte[] prefixAry, byte[] suffixAry, byte[] byteAry) {
				// 반환할 바이트 배열 생성
				int recordByteLength = byteAry.length - prefixAry.length - suffixAry.length;
				byte[] recordByte = new byte[recordByteLength];
				
				// Prefix, Suffix제외하고 복사
				int beginIndex = prefixAry.length;
				
				for (int i = 0; i < recordByteLength; ++i) {
					recordByte[i] = byteAry[i + beginIndex];
				}
				
				// 레코드 생성
				Record rtRecord = new Record(recordType, recordIndex, recordByte);
				
				return rtRecord;
			}
			
			// 배열을 확장하여 새 배열을 반환하는 함수
			protected byte[] makeAppendedByteAry(byte[] leftAry, byte[] rightAry) {
				// 배열 크기 설정
				int leftLength = 0, rightLength = 0, rtLength = 0;
				
				if (leftAry != null) {
					leftLength = leftAry.length;
				}
				
				if (rightAry != null) {
					rightLength = rightAry.length;
				}
				
				if ((rtLength = leftLength + rightLength) <= 0) return null; // left, right배열이 둘 다 null인 경우
					
				// 반환할 배열 생성
				byte[] rtByte = new byte[rtLength];
				int index = 0;
				
				// 좌측 배열 원소 복사
				if (leftAry != null) {
					for (int i = 0; i < leftAry.length; ++i) {
						rtByte[index++] = leftAry[i];
					}
				}
				
				// 우측 배월 원소 복사
				if (rightAry != null) {
					for (int j = 0; j < rightAry.length; ++j) {
						rtByte[index++] = rightAry[j];
					}
				}
				
				return rtByte;
			}
			
			// [beginIndex ~ endIndex) 사이의 배열 원소를 삭제하고 새 배열을 반환하는 함수
			protected byte[] makeRemovedByteAryByIndex(byte[] originAry, int beginIndex, int endIndex) {
				// 인덱스 비교
				if (beginIndex > endIndex) {
					Logger.logln(Logger.LogType.LT_WARN, "beginIndex(" + beginIndex + ") > endIndex(" + endIndex + "두 값을 교체하여 수행합니다.");
					
					int tempIndex = beginIndex;
					beginIndex = endIndex;
					endIndex = tempIndex;
				}
				
				// 길이 비교
				int originLength = originAry.length;
				int removeLength = endIndex - beginIndex;
				int rtLength = originLength - removeLength;
				
				if (rtLength < 0) {
					Logger.logln(Logger.LogType.LT_ERR, "삭제할 배열 범위가 원본 배열 길이를 초과합니다. (originAry.length: " + originLength + ", removeLength: " + removeLength + ")");
					return null;
				}
				else if (rtLength == 0) {
					return null;
				}
				
				// 반환 배열 생성
				byte[] rtByte = new byte[rtLength];
				int rtIndex = 0;

				// 원소 복사
				for (int i = 0; i < originAry.length; ++i) {
					if (i < beginIndex || i >= endIndex) {
						if (rtIndex < rtByte.length) {
							rtByte[rtIndex++] = originAry[i];
						}
						else {
							Logger.logln(Logger.LogType.LT_ERR, "배열 범위 오류. (rtByte.length: " + rtByte.length + ", rtIndex: " + rtIndex + ")");
							break;
						}
					}
				}
				
				return rtByte;
			}
			
			// targetAry에 elementAry가 순서대로 포함되어 있으면 해당 인덱스 반환, 포함되어있지 않은 경우 -1을 반환
			protected int findFromAry(byte[] targetAry, byte[] elementAry) {
				int prefixIndex = -1;
				boolean containPrefix = false;
				
				for (int tgtI = 0; tgtI < targetAry.length; ++tgtI) {
					for (int elemI = 0; elemI < elementAry.length; ++elemI) {
						int recvJ = tgtI + elemI;
						
						if (recvJ >= targetAry.length) {
							containPrefix = false;
							break;
						}
						else if (targetAry[recvJ] != elementAry[elemI]) {
							containPrefix = false;
							break;
						}
						else {
							containPrefix = true;
						}
					}
					
					if (containPrefix == true) {
						prefixIndex = tgtI;
						break;
					}
					else {
						prefixIndex = -1;
					}
				}
				
				return prefixIndex;
			}
				
			// 소켓 재접속을 시도하는 함수
			protected synchronized boolean reconnectSocket(int tryCnt, int tryInterval) {
				boolean isReconnectOk = false;
				int leftTryCnt = tryCnt;
				long curTime = System.currentTimeMillis();
				long nextTryTime = curTime + 1000; // 최초 1초 대기
				
				Logger.logln(Logger.LogType.LT_INFO, "소켓(Index: " + curWorkSocketIndex + ") 연결 끊어짐 감지. 닫기 및 재접속을 시도합니다. (TryingCnt: " + (tryCnt - leftTryCnt) + ")");
				
				// 기존 소켓 닫기
				while (leftTryCnt > 0) {
					curTime = System.currentTimeMillis();
					
					if (nextTryTime < curTime) {
						nextTryTime = curTime + tryInterval;
						--leftTryCnt;
						
						try {
							Logger.logln(Logger.LogType.LT_INFO, "소켓(Index: " + curWorkSocketIndex + ") 닫기 시도중. (TryingCnt: " + (tryCnt - leftTryCnt) + ")");
							
							Socket errSocket = socketList.get(curWorkSocketIndex);
							
							if (errSocket != null) {
								InputStream is = errSocket.getInputStream();
								int isLeftByte = is.available();
							
								if (isLeftByte > 0) {
									Logger.logln(Logger.LogType.LT_INFO, "InputStream의 남은 데이터(+  " + isLeftByte + "Bytes) 읽는 중.");
								}
								else {
									Logger.logln(Logger.LogType.LT_INFO, "소켓에 남은 데이터 길이: " + errSocket.getInputStream().available() + "bytes");
									errSocket.close();
									Logger.logln(Logger.LogType.LT_INFO, "소켓 닫기 성공.");
									break;
								}
							}
						}
						catch (IOException ioe1) {
							Logger.logln(Logger.LogType.LT_ERR, ioe1);
							Logger.logln(Logger.LogType.LT_ERR, "소켓 닫기 실패.");
							break;
						}
					}
				}
				
				leftTryCnt = tryCnt;
				
				// 소켓 재생성 및 연결
				while (leftTryCnt > 0) {
					curTime = System.currentTimeMillis();
					
					if (nextTryTime < curTime) {
						nextTryTime = curTime + tryInterval;
						--leftTryCnt;
						
						// 새로운 소켓 생성 후 재접속 시도
						Logger.logln(Logger.LogType.LT_INFO, "소켓(Index: " + curWorkSocketIndex + ") 재접속 시도중. (TryingCnt: " + (tryCnt - leftTryCnt) + ")");
						
						try {
							socketList.set(curWorkSocketIndex, new Socket(ip, port));
							isReconnectOk = true;
							break;
						}
						catch (IOException ioe) {
							// If an error occurs during the connection.
							Logger.logln(Logger.LogType.LT_ERR, ioe);
							Logger.logln(Logger.LogType.LT_ERR, "소켓 재접속 오류. (Ip: " + ip + ", Port: " + port + ")");
						}
						catch (IllegalBlockingModeException ibme) {
							// If ths socket has an associated channel, and the channel is in non-blocking mode.
							Logger.logln(Logger.LogType.LT_ERR, ibme);
							Logger.logln(Logger.LogType.LT_ERR, "서버 재접속 오류. (non-blocking mode)");
						}
						catch (IllegalArgumentException iae) {
							// If endpoint is null or a SocketAddress subclass not supported by this socket.
							Logger.logln(Logger.LogType.LT_ERR, iae);
							Logger.logln(Logger.LogType.LT_ERR, "재접속 주소 오류. (Ip: " + ip + ", Port: " + port + ")");
						}
					}
				}
				
				if (isReconnectOk) {
					Logger.logln(Logger.LogType.LT_INFO, "소켓(Index: " + curWorkSocketIndex + ") 재접속 성공. (TryingCnt: " + (tryCnt - leftTryCnt) + ")");
				}
				else {
					Logger.logln(Logger.LogType.LT_INFO, "소켓(Index: " + curWorkSocketIndex + ") 재접속 실패. (TryingCnt: " + (tryCnt - leftTryCnt) + ")");
					socketList.remove(curWorkSocketIndex);
					curWorkSocketIndex = 0;
				}
				
				return isReconnectOk;
			}
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////
		//[RecordTransceiver.java - SendSocketThread]///////////////////////////////////////////////////
		
		public static class SendSocketThread extends SocketThread {
			private byte[] sendPrefix;
			private byte[] sendSuffix;
			
			private float curSendPerSec;					// 현재 초당 전송량
			private int tgtSendPerSec;						// 목표 초당 전송량
			
			private List<Record> sendWaittingRecordList;	// 보내야 할 데이터 리스트 (Thread-Safe LinkedList)

			public SendSocketThread(RecordTransceiver recordTransceiver, String ip, int port, List<Socket> socketList, List<Record> fileWriteLeftRecordList, List<Integer> fileWriteDoneRecordIndexList, byte[] recvPrefix, byte[] recvSuffix, List<Record> sendWaittingRecordList, TransceiveLogger fileLogger, HashMap<String, byte[]> envVarMap) {
				super(recordTransceiver, ip, port, socketList, fileWriteLeftRecordList, fileWriteDoneRecordIndexList, fileLogger, envVarMap);
				
				this.sendPrefix = recvPrefix;
				this.sendSuffix = recvSuffix;
				
				this.curSendPerSec = 0.00f;
				this.tgtSendPerSec = Integer.parseInt(new String(envVarMap.get("RECORD_TGT_SEND_PER_SEC")));

				this.sendWaittingRecordList = sendWaittingRecordList;
				this.fileWriteLeftRecordList = fileWriteLeftRecordList;
				this.fileWriteDoneRecordIndexList = fileWriteDoneRecordIndexList;
			}
			
			@Override
			public void run() {
				long sendStartTime = System.currentTimeMillis();
				int sendCntFromStart = 0;
				byte[] sendStream = null;
				
				while (running) {
					// 스레드 시간 업데이트
					if (!updateThreadTime()) continue;
								
					// 초당 전송속도 초과 방지
					if ((curSendPerSec = sendCntFromStart / ((System.currentTimeMillis() - sendStartTime) / 1000.0f)) > tgtSendPerSec) {
						continue;
					}
					
					// 전송 대기 리스트의 레코드 전송 시도
					try {
						if ((sendStream = sendWork(sendStream)) == null) { // 전송 성공
							++sendCntFromStart;
						}
					}
					catch (IOException ioe) {
						Logger.logln(Logger.LogType.LT_ERR, ioe);
						Logger.logln(Logger.LogType.LT_ERR, Global.FC_WHITE + "OutputStream이 닫힌 상태입니다. (SocketIndex: " + curWorkSocketIndex + ")" + Global.FC_RESET);
						
						// [Note] OutputStream.write()에서 IOException이 발생하는 순간, InputStream에 서버에서 보낸 데이터를 수신할 수 없어보임. (따라서, 네트워크 상황에 따라 예외 발생한 레코드 -N개의 레코드가 소실 가능성이 있고, 
						// 재사용 소켓 모드에서 서버 예외 발생 후 2개의 데이터를 .write() 할 때 까지 감지하지 못하므로 +2개의 데이터가 추가로 소실됨)
						
						if (!reconnectSocket(50, 500)) { // 최대 50회,0.5초 간격으로 재접속 시도
							Logger.logln(Logger.LogType.LT_ERR, Global.FC_RED + "소켓 재접속에 실패하였습니다. (SocketIndex: " + curWorkSocketIndex +  ")" + Global.FC_RESET); // OFT
							
							if (socketList.size() == 0) {
								Logger.logln(Logger.LogType.LT_CRIT, Global.FC_RED + "서버와 연결된 소켓이 없습니다. 전송을 강제 종료합니다." + Global.FC_RESET); // OFT
								setThreadTimeoutLeft(0);
								continue;
							}
						}
					}
						
					// 오랫동안 수신하지 못한 데이터를 전송대기 리스트에 재삽입하여 재전송 시도
					resendWork();
					
					// 전송 속도 조절
					controlSendSpeed();
				}
				
				Logger.logln(Logger.LogType.LT_INFO, Global.FC_WHITE + "SendSocketThread 종료." + Global.FC_RESET);
			}
			
			public void close() {
				setRunning(false);
			}
			
			public float getCurSendPerSec() {
				return curSendPerSec;
			}
			
			public int getTgtSendPerSec() {
				return tgtSendPerSec;
			}
			
			public void setTgtSendPerSec(int tgtSendPerSec) {
				this.tgtSendPerSec = tgtSendPerSec;
			}

			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			
			// 전송 시도
			private byte[] sendWork(byte[] sendStream) throws IOException {
				Socket sendSocket = null;
				OutputStream os = null;
				Record sendRecord = null;
				
				// 이전 전송에서 실패한 데이터가 없는 경우 리스트 데이터를 읽어옴
				if (sendStream == null) {
					if (!sendWaittingRecordList.isEmpty()) { // 전송할 레코드가 있음
						// 소켓 선택 및 레코드 바이트 변환
						sendRecord = sendWaittingRecordList.remove(0);
						sendStream = makeSendStream(sendRecord);
					}
				}
				
				if (sendStream != null) {					
					// 전송
					if (recordTransceiver.isReusableSocketMode()) {
						sendSocket = socketList.get(curWorkSocketIndex);
					}
					else {
						recordTransceiver.addSocket(1);
						sendSocket = socketList.get(socketList.size() - 1);
					}
					
					os = sendSocket.getOutputStream();
					os.write(sendStream, 0, sendStream.length); // IOException 발생 가능
					sendRecord.addSendCnt(1);
					sendRecord.setLastSendTime(System.currentTimeMillis());
					fileWriteLeftRecordListSyncWork("add", sendRecord);
					threadTimeoutLeft = DEFAULT_THREAD_TIMEOUT;
					
					// 시간 포맷 및 로깅
					SimpleDateFormat dateTime = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
					String today = dateTime.format(new Date(System.currentTimeMillis()));
					String hour = today.substring(11, 13),	min = today.substring(14, 16),	sec = today.substring(17, 19);
					fileLogger.log(String.format("%s%s%s: snd(%04d)=(", hour, min, sec, sendStream.length), new String(sendStream), ")");
					Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_WHITE + String.format("\n[Soc%d]%s%s%s: snd(%d)=(%s)\n", curWorkSocketIndex, hour, min, sec, sendStream.length, new String(sendStream)) + Global.FC_RESET); // OFT
					
					// 초기화 및 다음 소켓 선택
					if (socketList.size() != 0) curWorkSocketIndex = (++curWorkSocketIndex) % socketList.size();
					
					sendStream = null;
				}
				
				return sendStream;
			}
			
			// 오랫동안 수신하지 못한 데이터를 전송대기 리스트에 재삽입하여 재전송 시도
			private void resendWork() {
				LinkedList<Record> recvWaitingList = fileWriteLeftRecordListSyncWork("get", null);
				int recvWaitingCnt = recvWaitingList.size();
				
				for (Record record : recvWaitingList) {
					long curTime = System.currentTimeMillis();
					long lastSendTime = record.getLastSendTime();
					long lastSendDelta = curTime - lastSendTime;
					int resendCnt = record.getSendCnt();
					
					if (lastSendDelta < DEFAULT_RESEND_DELAY) { // 전송후 재전송 대기
						// ...
					}
					else if (lastSendDelta >= DEFAULT_RESEND_DELAY && resendCnt <= MAX_RESEND_COUNT) { // 전송한지 DEFAULT_RESEND_DELAY초 이후, 재전송 MAX_RESEND_COUNT번 이하	
						if (sendWaittingRecordList.size() < recordTransceiver.getSendWaitingRecordListMaxSize()) { // 대기큐에 여유가 있고
							if (fileWriteDoneRecordIndexList.indexOf(record.getIndex()) == -1) { // 수신완료가 되지 않은 경우
								sendWaittingRecordList.add(record);	// 전송대기에 추가
								Logger.logln(Logger.LogType.LT_INFO, Global.FC_WHITE + record.getIndex() + "번 레코드 재전송 시도. (마지막 전송 후 경과시간: " + lastSendDelta + ", 전송 시도 횟수: " + resendCnt + ")" + Global.FC_RESET);
							}
							
							fileWriteLeftRecordListSyncWork("remove", record); // 수신완료 됐거나, 재전송한 경우 수신대기에서 제거
						}
						else {
							break;
						}
					}
					else {
						--recvWaitingCnt; // 전송 불가 레코드 개수만큼 제외
					}
				}
			}
			
			// 전송 속도 조절
			private void controlSendSpeed() {
				long sendDelay = 1;
						RecvSocketThread recvSocketThread = recordTransceiver.getRecvSocketThread();
						
						if ((recvSocketThread = recordTransceiver.getRecvSocketThread()) != null) {
							long curTime = System.currentTimeMillis();
							long lastRecvTime = recvSocketThread.getLastRecvTime();
							
							sendDelay = Math.min(1000, (curTime - lastRecvTime) / 2);
						}
						
						sendDelay = Math.max(sendDelay, 1);
						recordTransceiver.setSendDelay(sendDelay);
						setWorkDelayLeft(sendDelay); // 전송 속도 제어 및 스레드 과부하 방지
						
						// Old Version
						/*long sendDelay = recordTransceiver.getSendDelay();
						sendDelay = Math.max(sendDelay, Math.max(recvWaitingCnt, 1)); // '수신 지연시간' vs '미수신 데이터 개수' 중 큰 수만큼 대기. (단, 최소 1ms만큼은 대기)
						recordTransceiver.setSendDelay(sendDelay);
						setWorkDelayLeft(sendDelay / 2); // 전송 속도 제어 및 스레드 과부하 방지
						System.out.println("sendDelay: " + sendDelay / 2);*/
			}
			
			// Record를 사용하여 헤더, Prepix, Suffix를 붙인 전송용 byte[]생성
			private byte[] makeSendStream(Record sendRecord) {
				int streamIndex = 0;
				
				// Record 바이트 배열
				final byte[] recordSteram = sendRecord.toByteAry();
				
				// 헤더부 (0000 : 4byte)
				final int streamHeaderLength = 4; 
				
				// 데이터부 (Prefix + 레코드 + Suffix)
				final int streamDataLength = sendPrefix.length + recordSteram.length + sendSuffix.length;
				
				// 전송할 바이트 배열 (헤더 + 데이터부(Prefix+Record+Suffix))
				byte[] sendStream = new byte[streamHeaderLength + streamDataLength];
				
				// Data Length (헤더부 길이 제외)
				final byte[] headerStream = String.format("%04d", streamDataLength).getBytes();
				
				for (int i = 0; i < headerStream.length; ++i) {
					sendStream[streamIndex++] = headerStream[i];
				}
				
				// Prefix Copy (0x02:STX)
				for (int i = 0; i < sendPrefix.length; ++i) {
					sendStream[streamIndex++] = sendPrefix[i];
				}
				
				// Record Copy
				for (int i = 0; i < recordSteram.length; ++i) {
					sendStream[streamIndex++] = recordSteram[i];
				}
				
				// Suffix Copy (0x03:ETX)
				for (int i = 0; i < sendSuffix.length; ++i) {
					sendStream[streamIndex++] = sendSuffix[i];
				}
				
				return sendStream;
			}
		}
		
		
		////////////////////////////////////////////////////////////////////////////////////////////////
		//[RecordTransceiver.java - RecvSocketThread]///////////////////////////////////////////////////
		
		public static class RecvSocketThread extends SocketThread {	
			public static final int streamHeaderLength = 4;	// 헤더부 길이 (0000:4byte)
			
			private byte[] savedByteAry;	// 이전에 읽어들인 바이트 데이터를 담고있는 배열
			
			private long lastRecvTime;		// 마지막으로 데이터를 수신 성공한 시간
			
			private byte[] recvPrefix;
			private byte[] recvSuffix;
			
			private RecordConverter cliRecordConverter;		// 서버에서 받은 레코드를 클라이언트 형식으로 변경하는 클래스
			private KsFileWriter ksFileWriter;				// .rpy 파일에 최종 결과를 쓰는 클래스
			
			public RecvSocketThread(RecordTransceiver recordTransceiver, String ip, int port, List<Socket> socketList, List<Record> fileWriteLeftRecordList, List<Integer> fileWriteDoneRecordIndexList, byte[] recvPrefix, byte[] recvSuffix, RecordConverter cliRecordConverter, KsFileWriter ksFileWriter, TransceiveLogger fileLogger, HashMap<String, byte[]> envVarMap) {
				super(recordTransceiver, ip, port, socketList, fileWriteLeftRecordList, fileWriteDoneRecordIndexList, fileLogger, envVarMap);
				
				this.savedByteAry = null;
				
				this.lastRecvTime = System.currentTimeMillis();
				
				this.recvPrefix = recvPrefix;
				this.recvSuffix = recvSuffix;
				
				this.cliRecordConverter = cliRecordConverter;
				this.ksFileWriter = ksFileWriter;
			}
			
			@Override
			public void run() {
				Socket recvSocket = null;	// 수신 작업을 위한 소켓
				
				while (running) {
					// 스레드 시간 업데이트
					if (!updateThreadTime()) continue;

					try {
						// [수신부]
						if (recordTransceiver.isReusableSocketMode()) {
							recvSocket = socketList.get(curWorkSocketIndex);
						}
						else {
							recvSocket = null;
							
							if (socketList != null) {
								for (int i = 0; i < socketList.size(); ++i) {
									Socket socket = socketList.get(i);
									
									if (socket.getInputStream().available() != 0) {
										recvSocket = socket;
										break;
									}
								}
							}
						}
						
						if (recvSocket == null) continue;
						
						int inputDataLength = recvSocket.getInputStream().available();

						if (inputDataLength > 0) { // 수신할 바이트가 있음
							recvWork(recvSocket, inputDataLength);
						}
						
						if (savedByteAry != null && savedByteAry.length > 0) {
							// [절단부]
							byte[] recordByteAry = cuttingWork(inputDataLength);

							if (recordByteAry == null) continue;
							
							// [가공부]
							cvtAndWriteWork(recordByteAry);
							
							// 다음에 작업할 소켓번호 변경
							if (socketList.size() != 0)	curWorkSocketIndex = (++curWorkSocketIndex) % socketList.size();
						}
					}			
					catch (Exception e) {
						Logger.logln(Logger.LogType.LT_ERR, e);
						
						if (e instanceof IOException) {
							Logger.logln(Logger.LogType.LT_ERR, Global.FC_RED + "InputStream이 닫힌 상태입니다. (SocketIndex: " + curWorkSocketIndex + ", fileWriteLeftRecordList.size(): " + fileWriteLeftRecordList.size() + ")" + Global.FC_RESET);
							setWorkDelayLeft(1000); // 최소 1초간 작업 대기
						}
					}
					
					setWorkDelayLeft(Math.max(getWorkDelayLeft(), 1)); // 스레드 과부하 방지
				}
				
				// 전송 실패한 데이터를 파일에 기록
				writeRecvFailedRecord();
				
				Logger.logln(Logger.LogType.LT_INFO, Global.FC_RED + "RecvSocketThread 종료." + Global.FC_RESET);
			}
				
			public void close() {
				setRunning(false);
			}
			
			public int getFileWriteDoneRecordIndexListSize() {
				return fileWriteDoneRecordIndexList.size();
			}
			
			public long getLastRecvTime() {
				return lastRecvTime;
			}
			
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			
			// 수신부
			private void recvWork(Socket recvSocket, int inputDataLength) throws IOException {
				Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "[남은 처리 개수: " + fileWriteLeftRecordList.size() + "]" + Global.FC_RESET); // OFT
				Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "1-1.available(): " + inputDataLength + Global.FC_RESET); // OFT
				
				InputStream is = recvSocket.getInputStream();
				byte[] readByteAry = new byte[inputDataLength];
				
				if (is.read(readByteAry, 0, inputDataLength) != -1) {
					Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "1-2.readByteLen " + inputDataLength + Global.FC_RESET); // OFT
					
					// 마지막으로 수신한 시간, 타임아웃 갱신
					lastRecvTime = System.currentTimeMillis();
					threadTimeoutLeft = DEFAULT_THREAD_TIMEOUT;

					// 시간 포맷 및 로깅
					SimpleDateFormat dateTime = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
					String today = dateTime.format(new Date(System.currentTimeMillis()));
					String hour = today.substring(11, 13),	min = today.substring(14, 16),	sec = today.substring(17, 19);
					fileLogger.log(String.format("%s%s%s: rcv(%04d)=(", hour, min, sec, readByteAry.length), new String(readByteAry), ")");

					// 임시저장된 Byte와 취합
					this.savedByteAry = makeAppendedByteAry(savedByteAry, readByteAry);
					Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "1-3.savedByteAryLen: " + savedByteAry.length + Global.FC_RESET); // OFT
					
					// 일회용 소켓 모드의 경우 소켓 파괴
					if (!recordTransceiver.isReusableSocketMode()) {
						recvSocket.close();
						socketList.remove(recvSocket);
					}
				}
			}
			
			// 절단부
			private byte[] cuttingWork(int inputDataLength) {
				// Prefix:STX(0x02) 찾기
				int recvPrefixIdx = findFromAry(savedByteAry, recvPrefix);
				
				if (inputDataLength == 0) {
					Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "[남은 처리 개수: " +
								 fileWriteLeftRecordList.size() + ", 완료한 개수: " + fileWriteDoneRecordIndexList.size() + "]" + Global.FC_RESET); // OFT
				}
				
				// Prefix:STX를 아직 수신받지 못함
				if (recvPrefixIdx < 0) {
					Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2.failToFoundPrefix( + " + new String(recvPrefix) + ")"); // OFT
					return null; // 수신부 재수행
				}
				
				Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2-1.prefixIdx: " + recvPrefixIdx + Global.FC_RESET); // OFT
				
				// Suffix:ETX(0x03) 찾기
				int recvSuffixIdx = findFromAry(savedByteAry, recvSuffix);
				
				// Suffix:ETX를 아직 수신받지 못함
				if (recvSuffixIdx < 0) {
					Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2.failToFoundSuffix( + " + new String(recvSuffix) + ")"); // OFT
					return null; // 수신부 재수행
				}
				else {
					recvSuffixIdx += recvSuffix.length; // Suffix:ETX 데이터까지 추출하기 위해 증가
				}
				
				Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2-2.suffixIdx: " + recvSuffixIdx + Global.FC_RESET); // OFT
				
				// STX와 ETX를 모두 수신받은 경우 (의미있는 데이터가 완성된 경우)
				// 레코드 생성을 위한 부분 바이트 배열 생성 (STX/ETX가 포함되고, 헤더 4byte가 포함되지 않는 배열)
				byte[] recordByteAry = Arrays.copyOfRange(savedByteAry, recvPrefixIdx, recvSuffixIdx);
				Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2-3.recordByteAryLen: " + recordByteAry.length + "/302" + Global.FC_RESET); // OFT
				
				// 저장된 배열에서 해당 헤더 + 데이터 제거
				this.savedByteAry = makeRemovedByteAryByIndex(savedByteAry, recvPrefixIdx - streamHeaderLength, recvSuffixIdx);
				
				if (savedByteAry != null) Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2-4.afterRemoveFromSavedByteAry: " + savedByteAry.length + Global.FC_RESET); // OFT
				else Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2-4.afterRemoveFromSavedByteAry: 0" + Global.FC_RESET); // OFT
				
				return recordByteAry;
			}
			
			// 가공부
			private void cvtAndWriteWork(byte[] recordByteAry) {
				// 수신 데이터를 서버 레코드로 가공
				Record recvSvrRecord = makeRecord("HanaRecordServer", -1, recvPrefix, recvSuffix, recordByteAry);
				Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2-5.recvSvrRecordIndex: " + recvSvrRecord.getIndex() + Global.FC_RESET); // OFT
				
				int recvSvrRecordIndex = recvSvrRecord.getIndex();
				
				if (fileWriteDoneRecordIndexList.indexOf(recvSvrRecordIndex) == -1) { // 수신한적이 없는 인덱스 번호를 가진 레코드
					// 해당 레코드 인덱스 번호를 추가
					fileWriteDoneRecordIndexList.add(recvSvrRecordIndex);
					
					// 가공된 서버 레코드를 클라이언트 레코드로 변경
					cliRecordConverter.setOutRecordSubTypeName("Data");
					Record recvCliRecord = cliRecordConverter.convert(recvSvrRecord);
					
					Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2-6.cvtCliRecord (msgNum: " + recvCliRecord.getIndex() + ")" + Global.FC_RESET); // OFT
					
					// 레코드 파일에 쓰기 수행
					ksFileWriter.write(recvCliRecord.toByteAry(), recvSvrRecordIndex);
				}
				
				// 파일 작성대기 리스트에서 해당 인덱스의 레코드 제거
				fileWriteLeftRecordListSyncWork("remove", recvSvrRecord);
				
				LinkedList<Record> fileWriteLeftRecordListCpy = fileWriteLeftRecordListSyncWork("get", null);
				
				if (fileWriteLeftRecordListCpy.size() > 0) {
					Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "수신 대기중인 레코드 인덱스: " + Global.FC_RESET);
					
					for (Record writeLeftRecord : fileWriteLeftRecordListCpy) {
						Logger.log(Logger.LogType.LT_DEBUG, Global.FC_RED + writeLeftRecord.getIndex() + ", " + Global.FC_RESET);
					}
					
					Logger.ln(Logger.LogType.LT_DEBUG); Logger.ln(Logger.LogType.LT_DEBUG);
				}					
			}
			
			// 전송 실패한 데이터를 파일에 기록
			private void writeRecvFailedRecord() {
				for (Record failedRecord : fileWriteLeftRecordListSyncWork("get", null)) {
					cliRecordConverter.setOutRecordSubTypeName("Data");
					failedRecord = cliRecordConverter.convert(failedRecord);
					ksFileWriter.write(failedRecord.toByteAry(), failedRecord.getIndex());
				}
			}
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////
		//[Global.java]/////////////////////////////////////////////////////////////////////////////////
		
		public static class Global {
			/*public static final String FC_RESET		= "\u001B[0m";	// Font Color (Console)
			public static final String FC_BLACK		= "\u001B[30m";
			public static final String FC_RED		= "\u001B[31m";
			public static final String FC_GREEN		= "\u001B[32m";
			public static final String FC_YELLOW	= "\u001B[33m";
			public static final String FC_BLUE		= "\u001B[34m";
			public static final String FC_PURPLE	= "\u001B[35m";
			public static final String FC_CYAN		= "\u001B[36m";
			public static final String FC_WHITE		= "\u001B[37m";
			
			public static final String BC_BLACK		= "\u001B[40m";		// Background Color (Console)
			public static final String BC_RED		= "\u001B[41m";
			public static final String BC_GREEN		= "\u001B[42m";
			public static final String BC_YELLOW	= "\u001B[43m";
			public static final String BC_BLUE		= "\u001B[44m";
			public static final String BC_PURPLE	= "\u001B[45m";
			public static final String BC_CYAN		= "\u001B[46m";
			public static final String BC_WHITE		= "\u001B[47m";*/
			
			public static final String FC_RESET		= "\u001B[0m";		// Font Color (Console)
			public static final String FC_BLACK		= "\u001B[0;90m";
			public static final String FC_RED		= "\u001B[0;91m";
			public static final String FC_GREEN		= "\u001B[0;92m";
			public static final String FC_YELLOW	= "\u001B[0;93m";
			public static final String FC_BLUE		= "\u001B[0;94m";
			public static final String FC_PURPLE	= "\u001B[0;95m";
			public static final String FC_CYAN		= "\u001B[0;96m";
			public static final String FC_WHITE		= "\u001B[0;97m";
			
			public static final String BC_BLACK		= "\u001B[1;90m";	// Background Color (Console)
			public static final String BC_RED		= "\u001B[1;91m";
			public static final String BC_GREEN		= "\u001B[1;92m";
			public static final String BC_YELLOW	= "\u001B[1;93m";
			public static final String BC_BLUE		= "\u001B[1;94m";
			public static final String BC_PURPLE	= "\u001B[1;95m";
			public static final String BC_CYAN		= "\u001B[1;96m";
			public static final String BC_WHITE		= "\u001B[1;97m";
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////
		//[Logger.java]/////////////////////////////////////////////////////////////////////////////////
		
		public static class Logger {
			public enum LogType {
				LT_INFO(0),		// Info
				LT_DEBUG(1),	// Debug
				LT_WARN(2),		// Warning
				LT_ERR(3),		// Error
				LT_CRIT(4),		// Critical
				LT_MAX(5);
				
				private final int value;
				
				private LogType(int value) {
					this.value = value;
				}
				
				public int getValue() {
					return value;
				}
			}
				
			private static boolean[] visibleLogTypeAry = new boolean[LogType.LT_MAX.getValue()];
			
			private static final Logger.LogType[] visibleLogAry_Debug = { Logger.LogType.LT_INFO, Logger.LogType.LT_DEBUG, Logger.LogType.LT_WARN, Logger.LogType.LT_ERR, Logger.LogType.LT_CRIT };	// DEBUG
			private static final Logger.LogType[] visibleLogAry_Release = { Logger.LogType.LT_INFO, Logger.LogType.LT_ERR, Logger.LogType.LT_WARN, Logger.LogType.LT_CRIT };							// RELEASE
			private static final Logger.LogType[] visibleLogAry_Service = { Logger.LogType.LT_INFO, Logger.LogType.LT_CRIT };																			// SERVICE
			
			private static boolean checkVisibleByLogType(LogType logType) {
				return visibleLogTypeAry[logType.getValue()];
			}
			
			private static String getLogPrefix(LogType logType) {
				String logPrefix = null;
				
				switch (logType) {
					case LT_INFO:
						logPrefix = "[INFO] : ";
						break;
					case LT_DEBUG:
						logPrefix = "[DEBUG] : ";
						break;
					case LT_WARN:
						logPrefix = "[WARN] : ";
						break;
					case LT_ERR:
						logPrefix = "[ERR] : ";
						break;
					case LT_CRIT:
						logPrefix = "[CRIT] : ";
						break;
					default:
						logPrefix = "[LOG] : ";
						break;
				}
				
				return logPrefix;
			}
			
			public static void init() {
				for (int i = 0; i < visibleLogTypeAry.length; ++i) {
					visibleLogTypeAry[i] = false;
				}
			}
			
			public static void setVisibleByLogType(LogType[] logTypeAry, boolean isVisible) {
				if (logTypeAry == null || logTypeAry.length == 0) { // 일괄적용
					for (int i = 0; i < visibleLogTypeAry.length; ++i) {
						visibleLogTypeAry[i] = isVisible;
					}
				}
				else { // 개별적용
					for (LogType logType : logTypeAry) {
						visibleLogTypeAry[logType.getValue()] = isVisible;
					}
				}
			}

			public static void setVisibleByLogLevel(String logLevel) {
				Logger.LogType logType[] = null;
				
				logLevel = logLevel.toUpperCase();

				if (logLevel.equals("DEBUG")) {
					logType = visibleLogAry_Debug;
				}
				else if (logLevel.equals("RELEASE")) {
					logType = visibleLogAry_Release;
				}
				else {
					logType = visibleLogAry_Service;
				}
				
				Logger.init();
				Logger.setVisibleByLogType(logType, true);
			}
			
			public static void ln(LogType logType) {
				if (checkVisibleByLogType(logType)) {
					System.out.println();
				}
			}
			
			public static void log(LogType logType, String log) {
				if (checkVisibleByLogType(logType)) {
					System.out.print(log);
				}
			}
			
			public static void logln(LogType logType, byte[] byteAry) {
				logln(logType, new String(byteAry));
			}	
			
			public static void logln(LogType logType, String log) {
				if (checkVisibleByLogType(logType)) {
					String logPrefix = getLogPrefix(logType);
					System.out.println(logPrefix + log);
				}
			}
			
			public static void logln(LogType logType, Exception e) {
				if (checkVisibleByLogType(logType)) {
					System.out.print(getLogPrefix(logType));
					System.out.println(e.getMessage());
					e.printStackTrace();
				}
			}
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////
		//[TransceiveLogger.java]///////////////////////////////////////////////////////////////////////
		
		public static class TransceiveLogger {
			private String logFilePath;
			private StringBuffer logBuffer; // Thread-Safe

			// 생성자
			public TransceiveLogger(String logFilePath) {
				this.logFilePath = logFilePath;
				logBuffer = new StringBuffer();
			}
			
			// 로그 기록
			public synchronized void log(String head, String body, String tail) {
				logBuffer.append(head).append(body).append(tail).append("\r\n");
			}

			// 파일로 출력
			public synchronized void writeToFile() {
				if (logBuffer.length() > 0) {
					FileOutputStream fos = null;
					
					try {
						fos = new FileOutputStream(logFilePath, true);
						fos.write(String.valueOf(logBuffer).getBytes());
					}
					catch (IOException ioe1) {
						Logger.logln(Logger.LogType.LT_ERR, "OutputStream 열기 혹은 쓰기 오류.");
						ioe1.printStackTrace();
					}
					finally {
						try {
							if (fos != null) fos.close();
							fos = null;
						}
						catch (IOException ioe2) {
							Logger.logln(Logger.LogType.LT_ERR, "OutputStream 닫기 오류.");
							ioe2.printStackTrace();
						}
						
						logBuffer.delete(0, logBuffer.length());
					}
				}
			}
		}
    %>
	
    <%
		request.setCharacterEncoding("EUC-KR");
		
		String realPath = application.getRealPath("/");
		String[] args = {
			request.getParameter("RECORD_FILE_PATH"),
			request.getParameter("OUTPUT_FILE_PATH"),
			request.getParameter("ATTRIBUTE_CONFIG_FILE_PATH"),
			request.getParameter("OUTPUT_LOG_PATH"),
			request.getParameter("FB_RELAY_IP"),
			request.getParameter("FB_RELAY_PORT"),
			request.getParameter("FB_PARENT_BANK_CODE_3"),
			request.getParameter("FB_PARENT_COMP_CODE"),
			request.getParameter("FB_PARENT_ACCOUNT_NUMB"),
			request.getParameter("FB_REQ_FILE"),
			request.getParameter("FB_MSG_NUMB_S"),
			request.getParameter("FB_PARENT_COMP_NAME"),
			request.getParameter("REUSABLE_SOCKET_MODE"),
			request.getParameter("SOCKET_CNT"),
			request.getParameter("SOCKET_THREAD_TIMEOUT"),
			request.getParameter("RECORD_RESEND_MAX_TRY"),
			request.getParameter("RECORD_RESEND_DELAY"),
			request.getParameter("RECORD_TGT_SEND_PER_SEC"),
			request.getParameter("RECORD_FILE_PATH"),
			request.getParameter("LOG_LEVEL")
        };

		ClientMain.main(args);
	%>
	
    <h1>ClientWork Done!</h1>
	<p>- Check here: <%=request.getParameter("OUTPUT_FILE_PATH")%></p>
</body>
</html>