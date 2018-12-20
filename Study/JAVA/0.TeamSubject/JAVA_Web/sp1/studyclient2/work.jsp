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
			// ����
			public static void main(String[] args) {
				// ����
				long beginTime = 0, endTime = 0, runTime = 0;
				HashMap<String, byte[]> envVarMap = new HashMap<String, byte[]>();
				KsFileReader ksFileReader = null;
				KsFileWriter ksFileWriter = null;
				RecordConverter svrRecordConverter = null, cliRecordConverter = null;
				RecordTransceiver recordTransceiver = null;
				RecordPrinter recordPrinter = null;
				
				// �ʱ�ȭ
				try {
					System.out.println("========================================================================");

					beginTime = System.currentTimeMillis();
					
					init(envVarMap, args);
					
					endTime = System.currentTimeMillis();
					runTime = endTime - beginTime;
					
					System.out.println("> �ʱ�ȭ �Ϸ� : " + (runTime / 1000.0) + "��");
					beginTime = System.currentTimeMillis();
					
					////////////////////////////////////////////////////////////////////////////////////////////////////
					
					// ���� �б� �۾��� �񵿱� ���� �۾��� ���� Ŭ���� (�񵿱� ���� �۾���, ����->Ŭ�� ���ڵ� ��ȯ ����)
					String inFilePath = new String(envVarMap.get("INPUT_FILE_PATH"));
					String outFilePath = new String(envVarMap.get("OUTPUT_FILE_PATH"));
					
					KsFileWriter.copyFromFile(new File(inFilePath), new File(outFilePath)); // ���� ����
					
					ksFileReader = new KsFileReader(inFilePath);
					ksFileWriter = new KsFileWriter(outFilePath, AttributeManager.getInst().getRecordSizeFromAttributeMap("HanaAttrClient_Data"));
					
					// Ŭ��->����->Ŭ�� ���ڵ� ��ȯ�� ���� Ŭ����
					svrRecordConverter = new RecordConverter(null, "HanaRecordServer", "", envVarMap); // ������ ��ȯ
					cliRecordConverter = new RecordConverter(null, "HanaRecordClient", "", envVarMap); // Ŭ��� ��ȯ
					
					// ������ �����͸� ������ Ŭ����
					String svrIp = new String(envVarMap.get("FB_IP"));
					int svrPort = Integer.parseInt(new String(envVarMap.get("FB_PORT")));
					

					boolean reusableSocketMode = new String(envVarMap.get("REUSABLE_SOCKET_MODE")).toUpperCase().equals("TRUE") ? true : false;
					int socketCnt = Integer.parseInt(new String(envVarMap.get("SOCKET_CNT")));
					
					recordTransceiver = new RecordTransceiver(reusableSocketMode, svrIp, svrPort, socketCnt, 50,
															  cliRecordConverter, ksFileWriter, envVarMap);
					
					// ���ڵ� ����� ���� Ŭ����
					recordPrinter = new RecordPrinter(null);
					
					// ��� ���� ǥ���� ����
					cliRecordConverter.setOutRecordSubTypeName("Head");
					ksFileWriter.write(cliRecordConverter.convert(new Record("HanaRecordServer", "", 0, null)).toByteAry(), 0);
				}
				catch (Exception e) {
					Logger.logln(Logger.LogType.LT_CRIT, e);
					finishProgram(recordTransceiver, cliRecordConverter, svrRecordConverter, ksFileReader, ksFileWriter, 0);
				}
				
				// ���� ������ ���κ��� �����鼭 ��ȯ �� ����, ���Ϸ� ���� ����
				int lineCnt = 0;
				
				try {
					for (; ; ++lineCnt) {
						// ���� ���� ���κ� �б�
						String recordStr = ksFileReader.readLine();
						
						// ���̻� ���� ������ ������ Ż��
						if (recordStr == null) break;

						// ���� �������� ���ڵ� ����
						Record cliRecord = new Record("HanaRecordClient", lineCnt, recordStr.getBytes());
						
						// ���� ���ڵ�� ��ȯ
						Record toSvrRecord = svrRecordConverter.convert(cliRecord);
						
						// �����ͺθ� ������ ����
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
					
					// ��� ���ڵ��� ���۰� ���� ���Ⱑ �Ϸ�� ������ ���
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
					
					// ����
					endTime = System.currentTimeMillis();
					runTime = endTime - beginTime;
					System.out.println("> ���� �ۼ��� �Ϸ� : " + (runTime / 1000.0) + "��");
					System.out.println("========================================================================");
				}
			}
			
			// �ʱ�ȭ
			public static void init(HashMap<String, byte[]> envVarMap, String[] args) {
				// ȯ�溯�� �ؽ� �ʱ�ȭ
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
				
				// �Ŵ��� �ʱ�ȭ
				try {
					AttributeManager.InitManager(new String(envVarMap.get("ATTR_CONFIG_FILE_PATH")));
				}
				catch (Exception e) {
					Logger.logln(Logger.LogType.LT_ERR, e);
				}
				
				// �ΰ� �ʱ�ȭ
				String logLevel = new String(envVarMap.get("LOG_LEVEL"));
				
				Logger.setVisibleByLogLevel(logLevel);
			}
			
			// ���α׷� ������
			public static void finishProgram(RecordTransceiver recordTransceiver, RecordConverter cliRecordConverter, RecordConverter svrRecordConverter, KsFileReader ksFileReader, KsFileWriter ksFileWriter, int lineCnt) {
				try {
					// ���� ���� �ݱ�
					if (ksFileReader != null) ksFileReader.close();
					
					// ���۱� ����
					if (recordTransceiver != null) recordTransceiver.close();
					
					// ��� ���� ����� ����
					if (cliRecordConverter != null) cliRecordConverter.setOutRecordSubTypeName("Tail");
					if (ksFileWriter != null) ksFileWriter.write(cliRecordConverter.convert(new Record("HanaRecordServer", "", lineCnt, null)).toByteAry(), lineCnt - 1);
					
					// ���ڵ� ��ȯ�� �� ���� �ݱ�
					if (cliRecordConverter != null) cliRecordConverter.close();
					if (ksFileWriter != null) ksFileWriter.close();
				}
				catch (Exception e) {
					System.out.println("========================================================================");
					System.exit(-1);
				}
			}
			
			// ������ ���
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
					// ���� üũ
					if (value.length > byteLength) {
						// System.out.println("[WARN] : value.length > byteLength ������ �ս� ���ɼ��� �ֽ��ϴ�. (" + value.length + " > " + byteLength + " codeName: [" + codeName + "], value: [" + new String(value) + "])");
					}
				}
					
				// Ÿ�Ժ��� value�� ����
				byte[] valCpy = new byte[byteLength];
				
				if (type.equals("X") || type.equals("C")) { // ����(' ') �е�, �·� ����
					Arrays.fill(valCpy, (byte)' ');
					
					if (value != null) {
						for (int i = 0; i < byteLength; ++i) {
							if (i == value.length) break;
							
							valCpy[i] = value[i];
						}
					}
				}
				//else if (type.equals("")) { // ���� '0' �е�, �·� ����
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
				else if (type.equals("9") || type.equals("N")) { // ���� '0' �е�, ��� ����
					Arrays.fill(valCpy, (byte)'0');
					
					if (value != null) {
						int valueI = value.length - 1;
						for (int cpyI = byteLength - 1; cpyI > -1; --cpyI) {
							
							valCpy[cpyI] = value[valueI--];
							
							if (valueI < 0) break;
						}
					}
				}
				else { // ����(' ') �е�, �·� ����
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
			
			private static AttributeManager attributeManager; // �̱��� Ŭ����
			private static HashMap<String, HashMap<String, Attribute>> attributeMapMap;	// Attribute�������� ���� ���� ���� ��
			private static HashMap<String, Integer> attributeSizeMap; // Attribute���� ũ�⸦ ���� ��
			
			// ������
			private AttributeManager() {}
			
			// �ʱ�ȭ
			public static void InitManager(String configFilePath) throws Exception {
				// �̱��� ��ü ����
				attributeManager = new AttributeManager();
				
				// �ؽø� �ʱ�ȭ
				attributeMapMap = new HashMap<String, HashMap<String, Attribute>>();
				attributeSizeMap = new HashMap<String, Integer>();
				
				// �������� ���� ���ڿ� ����Ʈ�� ��ȯ
				KsFileReader ksFileReader = new KsFileReader(configFilePath);
				ArrayList<String> configStrList = ksFileReader.readLines();
				
				// �������� KEYWORD_ATTRFILES Ű���� �˻�
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

				// �Ӽ����� �о AttributeMapMap ä������
				//for (String attrFilePath : configFilePathList) {
				//	updateAttributeMapMap(attrFilePath);
				//}
				
				// �ӽ÷� �ϵ��ڵ�
				HashMap<String, Attribute> attrMap = new HashMap<String, Attribute>();
				attrMap.put("h_idCode",					new Attribute(1,	"�ĺ��ڵ�",		"h_idCode",					"X",	0,	1,	"S".getBytes()			));
				attrMap.put("h_taskComp",				new Attribute(2,	"��������",		"h_taskComp",				"X",	1,	2,	"10".getBytes()			));
				attrMap.put("h_bankCode",				new Attribute(3,	"�����ڵ�",		"h_bankCode",				"9",	3,	3,	"081".getBytes()		));
				attrMap.put("h_companyCode",			new Attribute(4,	"��ü�ڵ�",		"h_companyCode",			"X",	6,	8,	"KSANP001".getBytes()	));
				attrMap.put("h_comissioningDate",		new Attribute(5,	"��ü�Ƿ�����",		"h_comissioningDate",		"9",	14,	6,	"180404".getBytes()		));
				attrMap.put("h_processingDate",			new Attribute(6,	"��üó������",		"h_processingDate",			"9",	20,	6,	null					));
				attrMap.put("h_motherAccountNum",		new Attribute(7,	"����¹�ȣ",		"h_motherAccountNum",		"9",	26,	14,	"25791005094404".getBytes()	));
				attrMap.put("h_transferType",			new Attribute(8,	"��ü����",		"h_transferType",			"9",	40,	2,	"51".getBytes()			));
				attrMap.put("h_companyNum",				new Attribute(9,	"ȸ���ȣ",		"h_companyNum",				"9",	42,	6,	"000000".getBytes()		));
				attrMap.put("h_resultNotifyType",		new Attribute(10,	"ó������뺸����",	"h_resultNotifyType",		"X",	48,	1,	"1".getBytes()			));
				attrMap.put("h_transferCnt",			new Attribute(11,	"��������",		"h_transferCnt",			"X",	49,	1,	"1".getBytes()			));
				attrMap.put("h_password",				new Attribute(12,	"��й�ȣ",		"h_password",				"X",	50,	8,	"4380".getBytes()		));
				attrMap.put("h_blank",					new Attribute(13,	"����",			"h_blank",					"X",	58,	19,	null					));
				attrMap.put("h_format",					new Attribute(14,	"Format",		"h_format",					"X",	77,	1,	"1".getBytes()			));
				attrMap.put("h_van",					new Attribute(15,	"VAN",			"h_van",					"X",	78,	2,	null					));
				attrMap.put("h_newLine",				new Attribute(16,	"���๮��",		"h_newLine",				"X",	80,	2,	null					));
				attributeMapMap.put("HanaAttrClient_Head", attrMap);
				
				attrMap = new HashMap<String, Attribute>();
				attrMap.put("d_idCode",					new Attribute(1,	"�ĺ��ڵ�",		"d_idCode",					"X",	0,	1,	"D".getBytes()			));
				attrMap.put("d_dataSerialNum",			new Attribute(2,	"������ �Ϸù�ȣ",	"d_dataSerialNum",			"9",	1,	6,	null					));
				attrMap.put("d_bankCode",				new Attribute(3,	"�����ڵ�",		"d_bankCode",				"9",	7,	3,	null					));
				attrMap.put("d_accountNum",				new Attribute(4,	"���¹�ȣ",		"d_accountNum",				"X",	10,	14,	null					));
				attrMap.put("d_requestTransferPrice",	new Attribute(5,	"��ü��û�ݾ�",		"d_requestTransferPrice",	"9",	24,	11,	null					));
				attrMap.put("d_realTransferPrice",		new Attribute(6,	"������ü�ݾ�",		"d_realTransferPrice",		"9",	35,	11,	null					));
				attrMap.put("d_recieverIdNum",			new Attribute(7,	"�ֹ�/����ڹ�ȣ",	"d_recieverIdNum",			"X",	46,	13,	null					));
				attrMap.put("d_processingResult",		new Attribute(8,	"ó�����",		"d_processingResult",		"X",	59,	1,	null					));
				attrMap.put("d_disableCode",			new Attribute(9,	"�Ҵ��ڵ�",		"d_disableCode",			"X",	60,	4,	null					));
				attrMap.put("d_briefs",					new Attribute(10,	"����",			"d_briefs",					"X",	64,	12,	null					));
				attrMap.put("d_blank",					new Attribute(11,	"����",			"d_blank",					"X",	76,	4,	null					));
				attrMap.put("d_newLine",				new Attribute(12,	"���๮��",		"d_newLine",				"X",	80,	2,	null					));
				attributeMapMap.put("HanaAttrClient_Data", attrMap);
				
				attrMap = new HashMap<String, Attribute>();
				attrMap.put("t_idCode",					new Attribute(1,	"�ĺ��ڵ�",		"t_idCode",					"X",	0,	1,	"E".getBytes()			));
				attrMap.put("t_totalRequestCnt",		new Attribute(2,	"���ǷڰǼ�",		"t_totalRequestCnt",		"9",	1,	7,	null					));
				attrMap.put("t_totalRequestPrice",		new Attribute(3,	"���Ƿڱݾ�",		"t_totalRequestPrice",		"9",	8,	13,	null					));
				attrMap.put("t_normalProcessingCnt",	new Attribute(4,	"����ó���Ǽ�",		"t_normalProcessingCnt",	"9",	21,	7,	null					));
				attrMap.put("t_normalProcessingPrice",	new Attribute(5,	"����ó���ݾ�",		"t_normalProcessingPrice",	"9",	28,	13,	null					));
				attrMap.put("t_disableProcessingCnt",	new Attribute(6,	"�Ҵ�ó���Ǽ�",		"t_disableProcessingCnt",	"9",	41,	7,	null					));
				attrMap.put("t_disableProcessingPrice",	new Attribute(7,	"�Ҵ�ó���ݾ�",		"t_disableProcessingPrice",	"9",	48,	13,	null					));
				attrMap.put("t_recoveryCode",			new Attribute(8,	"�����ȣ",		"t_recoveryCode",			"X",	61,	8,	"3706".getBytes()		));
				attrMap.put("t_blank",					new Attribute(9,	"����",			"t_blank",					"X",	69,	11,	null					));
				attrMap.put("t_newLine",				new Attribute(10,	"���๮��",		"t_newLine",				"X",	80,	2,	null					));
				attributeMapMap.put("HanaAttrClient_Tail", attrMap);
				
				attrMap = new HashMap<String, Attribute>();
				attrMap.put("h_idCode",							new Attribute(1,	"�ĺ��ڵ�",		"h_idCode",							"C",	0,		9,	null));
				attrMap.put("h_companyCode",					new Attribute(2,	"��ü�ڵ�",		"h_companyCode",					"C",	9,		8,	null));
				attrMap.put("h_bankCode2",						new Attribute(3,	"�����ڵ�2",		"h_bankCode2",						"C",	17,		2,	null));
				attrMap.put("h_msgCode",						new Attribute(4,	"�޽����ڵ�",		"h_msgCode",						"C",	19,		4,	null));
				attrMap.put("h_workTypeCode",					new Attribute(5,	"���������ڵ�",		"h_workTypeCode",					"C",	23,		3,	null));
				attrMap.put("h_transferCnt",					new Attribute(6,	"�۽�Ƚ��",		"h_transferCnt",					"C",	26,		1,	null));
				attrMap.put("h_msgNum",							new Attribute(7,	"������ȣ",		"h_msgNum",							"N",	27,		6,	null));
				attrMap.put("h_transferDate",					new Attribute(8,	"��������",		"h_transferDate",					"D",	33,		8,	null));
				attrMap.put("h_transferTime",					new Attribute(9,	"���۽ð�",		"h_transferTime",					"T",	41,		6,	null));
				attrMap.put("h_responseCode",					new Attribute(10,	"�����ڵ�",		"h_responseCode",					"C",	47,		4,	null));
				attrMap.put("h_bankResponseCode",				new Attribute(11,	"���� �����ڵ�",		"h_bankResponseCode",				"C",	51,		4,	null));
				attrMap.put("h_lookupDate",						new Attribute(12,	"��ȸ����",		"h_lookupDate",						"D",	55,		8,	null));
				attrMap.put("h_lookupNum",						new Attribute(13,	"��ȸ��ȣ",		"h_lookupNum",						"N",	63,		6,	null));
				attrMap.put("h_bankMsgNum",						new Attribute(14,	"����������ȣ",		"h_bankMsgNum",						"C",	69,		15,	null));
				attrMap.put("h_bankCode3",						new Attribute(15,	"�����ڵ�3",		"h_bankCode3",						"C",	84,		3,	null));
				attrMap.put("h_spare",							new Attribute(16,	"����",			"h_spare",							"C",	87,		13,	null));
				attrMap.put("dt_withdrawalAccountNum",			new Attribute(17,	"��� ���¹�ȣ",		"dt_withdrawalAccountNum",			"C",	100,	15,	null));
				attrMap.put("dt_bankBookPassword",				new Attribute(18,	"���� ��й�ȣ",		"dt_bankBookPassword",				"C",	115,	8,	null));
				attrMap.put("dt_recoveryCode",					new Attribute(19,	"�����ȣ",		"dt_recoveryCode",					"C",	123,	6,	null));
				attrMap.put("dt_withdrawalAmount",				new Attribute(20,	"��� �ݾ�",		"dt_withdrawalAmount",				"N",	129,	13,	null));
				attrMap.put("dt_afterWithdrawalBalanceSign",	new Attribute(21,	"��� �� �ܾ׺�ȣ",	"dt_afterWithdrawalBalanceSign",	"C",	142,	1,	null));
				attrMap.put("dt_afterWithdrawalBalance",		new Attribute(22,	"��� �� �ܾ�",		"dt_afterWithdrawalBalance",		"N",	143,	13,	null));
				attrMap.put("dt_depositBankCode2",				new Attribute(23,	"�Ա� �����ڵ�2",	"dt_depositBankCode2",				"C",	156,	2,	null));
				attrMap.put("dt_depositAccountNum",				new Attribute(24,	"�Ա� ���¹�ȣ",		"dt_depositAccountNum",				"C",	158,	15,	null));
				attrMap.put("dt_fees",							new Attribute(25,	"������",			"dt_fees",							"N",	173,	9,	null));
				attrMap.put("dt_transferTime",					new Attribute(26,	"��ü �ð�",		"dt_transferTime",					"T",	182,	6,	null));
				attrMap.put("dt_depositAccountBriefs",			new Attribute(27,	"�Ա� ���� ����",	"dt_depositAccountBriefs",			"C",	188,	20,	null));
				attrMap.put("dt_cmsCode",						new Attribute(28,	"CMS�ڵ�",		"dt_cmsCode",						"C",	208,	16,	null));
				attrMap.put("dt_identificationNum",				new Attribute(29,	"�ſ�Ȯ�ι�ȣ",		"dt_identificationNum",				"C",	224,	13,	null));
				attrMap.put("dt_autoTransferClassification",	new Attribute(30,	"�ڵ���ü ����",		"dt_autoTransferClassification",	"C",	237,	2,	null));
				attrMap.put("dt_withdrawalAccountBriefs",		new Attribute(31,	"��� ���� ����",	"dt_withdrawalAccountBriefs",		"C",	239,	20,	null));
				attrMap.put("dt_depositBankCode3",				new Attribute(32,	"�Ա� �����ڵ�3",	"dt_depositBankCode3",				"C",	259,	3,	null));
				attrMap.put("dt_salaryClassification",			new Attribute(33,	"�޿� ����",		"dt_salaryClassification",			"C",	262,	1,	null));
				attrMap.put("dt_spare",							new Attribute(34,	"����",			"dt_spare",							"C",	263,	37,	null));
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
					
					for (Map.Entry<String, Attribute> entry : attributeMap.entrySet()) { // ��ī�� ����
						rtMap.put(entry.getKey(), new Attribute(entry.getValue()));
					}
				}
				else {
					Logger.logln(Logger.LogType.LT_WARN, "\"" + attributeMapName + "\"���� Ű�� ���� attributeMapMap�� �����ϴ�.");
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
						Logger.logln(Logger.LogType.LT_WARN, "\"" + attributeMapName + "\"���� Ű�� ���� attributeMapMap�� �����ϴ�.");
					}
				}
				
				return rtInt;
			}
			
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////
			
			// .attr ���� �Ľ�
			private void updateAttributeMapMap(String attrFilePath) throws Exception {
				final int META_ROW_CNT = 2;
				final String KEY_NEWLINE = "\r\n";
				final String KEY_COMMENT = "COMMENT";
				final String KEY_STRING = "STRING";
				final String KEY_INT = "INT";
				final String KEY_BYTE = "BYTE";
				
				// ���ڿ� ��ķ� ���� �б�
				final String[][] strAry2D = cvtAttrFile2StringAry2D(attrFilePath);
				final int rowCnt = strAry2D.length - META_ROW_CNT;
				final int colCnt = strAry2D[0].length;

				// ��� �������� ����� ���� �� �ڷ��� ����Ʈ ����
				// (������ AttrRecord�� �ڷ����� �������ִ� '����'�� Ŭ��������, ���� .attr���� ���� ����� �ڷ����� ����
				//  '����'���� �Ӽ����� ������ �� �ֵ��� �ϱ� ���� �� �ڷ����� �������� �ľ��Ͽ� ����Ʈ�� ������ ��.)
				ArrayList<Integer> colIndexList = new ArrayList<Integer>();
				ArrayList<String> colTypeList = new ArrayList<String>();
				
				for (int col = 0; col < colCnt; ++col) {
					String keyWord = strAry2D[0][col];

					// �ּ� �÷�
					if (keyWord.equals(KEY_COMMENT)) {}
					// ���ڿ� �÷�
					else if (keyWord.equals(KEY_STRING)) {
						colIndexList.add(col);
						colTypeList.add(KEY_STRING);
					}
					// ���� �÷�
					else if (keyWord.equals(KEY_INT)) {
						colIndexList.add(col);
						colTypeList.add(KEY_INT);
					}
					// ����Ʈ �÷�
					else if (keyWord.equals(KEY_BYTE)) {
						colIndexList.add(col);
						colTypeList.add(KEY_BYTE);
					}
					// ���� (������ Ű����)
					else {
						throw new Exception("[����: �� �� ���� Ű���� (File: " + attrFilePath + "\"" + keyWord + "\", row: " + 0 + ", col: " + col + ")]");
					}
				}
				
				// attrMapMap�� attrMap�߰� (HanaAttrClient, HanaAttrServer ����. ���� ���� Attribute Ŭ������ ���� �ʿ�...)
				final String attrFileName = attrFilePath.substring(attrFilePath.lastIndexOf("/") + 1, attrFilePath.lastIndexOf("."));
				int orderCnt = 0;
				HashMap<String, Attribute> attributeMap = new HashMap<String, Attribute>();

				//for (int i = 0; i < colIndexList.size(); ++i) { // (���� �̷�������...)
					//String varType = colTypeList.get(i);
					//int col = colIndexList.get(i);
					// ......
				//}	
				
				try {
					if (attrFileName.equals("HanaAttrClient") || attrFileName.equals("HanaAttrServer")) { // �ϳ����� ���� (Ŭ���/������)
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
						throw new Exception("[����: (" + attrFileName + ")�� ������ ���� �����Դϴ�.]");
					}
				}
				catch (Exception e) {
					Logger.logln(Logger.LogType.LT_ERR, e);
				}
				
				attributeMapMap.put(attrFileName, attributeMap);
			}
			
			private String[][] cvtAttrFile2StringAry2D(String attrFilePath) {
				// .attr���� ���ڿ��� ����
				KsFileReader ksFileReader = new KsFileReader(attrFilePath);
				ArrayList<String> attrFileStrList = ksFileReader.readLines();
				
				StringBuilder strBuilder = new StringBuilder();
				for (String lineStr : attrFileStrList) {
					System.out.print(">" + lineStr + "\r\n");
					strBuilder.append(lineStr).append("\r\n");
				}
				System.out.println();
				
				// ����� ũ�� ���ϱ�
				String fileStr = strBuilder.toString();
				String[] rowStrAry = fileStr.split("\r\n");			// �� ������ �迭
				final int rowCnt = rowStrAry.length - 1;			// ���� ���� (������ �� �� ����)
				final int colCnt = rowStrAry[0].split("\t").length;	// ���� ����
				String[][] strAry2D = new String[rowCnt][colCnt];	// ��� ������ �迭
				
				// ���� �����͸� ���ȭ
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
					Logger.logln(Logger.LogType.LT_ERR, "OutputStream ���� Ȥ�� ���� ����.");
					Logger.logln(Logger.LogType.LT_ERR, ioe);
				}
			}
			
			public void close() {
				try {
					randomAccessFile.close();
				}
				catch (IOException ioe) {
					Logger.logln(Logger.LogType.LT_ERR, "OutputStream �ݱ� ����.");
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
			
			private int sendCnt;		// ���� Ƚ��
			private long lastSendTime;	// ������ ���� �ð�
			
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
					Logger.logln(Logger.LogType.LT_WARN, "\"" + attributeName + "\"�� Ű�� ���� attr�� null�Դϴ�.");
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
					// ���� ���� �߰�
					byte[] copyDatas = null;
					
					if (datas == null) {
						copyDatas = new byte[AttributeManager.getInst().getRecordSizeFromAttributeMap(attrName)];
					}
					else {
						copyDatas = Arrays.copyOfRange(datas, 0, datas.length + 2);
						copyDatas[datas.length] = '\r';
						copyDatas[datas.length + 1] = '\n';
					}
					
					// ǥ����, �����ͺ�, ����� ����
					byte idCode = copyDatas[0];

					if (idCode == (byte)'D' || subTypeName.equals("Data")) { // �����ͺ�
						this.attrMap = AttributeManager.getInst().copyOfAttributeMap(attrName + "_Data");
						this.subTypeName = "Data";
					}
					else if (idCode == (byte)'S' || subTypeName.equals("Head")) { // ǥ����
						this.attrMap = AttributeManager.getInst().copyOfAttributeMap(attrName + "_Head");
						this.subTypeName = "Head";
					}
					else if (idCode == (byte)'E' || subTypeName.equals("Tail")) { // �����
						this.attrMap = AttributeManager.getInst().copyOfAttributeMap(attrName + "_Tail");
						this.subTypeName = "Tail";
					}
					else { // ����
						Logger.logln(Logger.LogType.LT_ERR, "�� �� ���� idCode��. (idCode: \"" + idCode + "\")");
						return;
					}
					
					// �Ӽ� ���� ��ȸ�Ͽ� ����Ʈ �����͸� �ʿ信 �°� �߶� ����
					for (Attribute attr : attrMap.values()) {
						int beginIndex = attr.getBeginIndex();
						int byteLength = attr.getByteLength();
						int endIndex = beginIndex + byteLength;

						attr.setValue(Arrays.copyOfRange(copyDatas, beginIndex, endIndex));
					}
					
					indexKeyStr = "d_dataSerialNum"; // ������ �Ϸù�ȣ -> ���ڵ� �ε���
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
					
					indexKeyStr = "h_msgNum"; // ������ȣ -> ���ڵ� �ε���
				}
				
				// ���ڵ� �ε��� ������Ʈ (-1 : ���� �ε���)
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

			private Record inRecord;						// �Է� ���ڵ�
			private String outRecordTypeName;				// ��� ���ڵ� Ÿ�Ը�
			private String outRecordSubTypeName;			// ��� ���ڵ� ����Ÿ�Ը�
			private HashMap<String, byte[]> envVarMap;		// ȯ�溯�� �ؽø�
			private HashMap<String, byte[]> localVarMap;	// �������� �ؽø�

			public RecordConverter(Record inRecord, String outRecordTypeName, String outRecordSubTypeName, HashMap<String, byte[]> envVarMap) {
				this.inRecord = inRecord;
				this.outRecordTypeName = outRecordTypeName;
				this.outRecordSubTypeName = outRecordSubTypeName;
				this.envVarMap = envVarMap;
				this.localVarMap = new HashMap<String, byte[]>();
			}
			
			public Record convert() {
				String inRecordTypeName = inRecord.getTypeName();
				
				// �ϳ����� ������ȯ
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
				
				// �ð� ������
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
							
							// [1.����� (100Byte)]
							// �ĺ� �ڵ� (0~8)
							rtRecord.setData("h_idCode", null);
							
							// ��ü�ڵ� (9~16)
							rtRecord.setData("h_companyCode", envVarMap.get("FB_PARENT_COMP_CODE"));
							
							// �����ڵ�2 (17~18)
							final byte[] bankCode3 = envVarMap.get("FB_PARENT_BANK_CODE_3");
							final byte[] bankCode2 = Arrays.copyOfRange(bankCode3, 1, 3);
							rtRecord.setData("h_bankCode2", bankCode2);
							
							// �޽����ڵ� (19~22)
							final byte[] messageCode = { '0', '1', '0', '0' };
							rtRecord.setData("h_msgCode", messageCode);
							
							// ���������ڵ� (23~25)
							final byte[] workTypeCode = localVarMap.get("loc_hana_svr_h_workTypeCode");
							rtRecord.setData("h_workTypeCode", workTypeCode);
							
							// �۽�Ƚ�� (26)
							final byte[] transferCnt = { '1' };
							rtRecord.setData("h_transferCnt", transferCnt);
							
							// ������ȣ (27~32)
							rtRecord.setData("h_msgNum", inRecord.getData("d_dataSerialNum"));
							
							// �������� (33~40)
							final byte[] transferDate = { (byte)year.charAt(0),  (byte)year.charAt(1), (byte)year.charAt(2), (byte)year.charAt(3),
														  (byte)month.charAt(0), (byte)month.charAt(1), 
														  (byte)date.charAt(0),  (byte)date.charAt(1) };
							rtRecord.setData("h_transferDate", transferDate);
							
							// ���۽ð� (41~46)
							final byte[] transferTime = { (byte)hour.charAt(0), (byte)hour.charAt(1),
														  (byte)min.charAt(0),  (byte)min.charAt(1),
														  (byte)sec.charAt(0),  (byte)sec.charAt(1) };
							rtRecord.setData("h_transferTime", transferTime);
							
							// �����ڵ� (47~50)
							rtRecord.setData("h_responseCode", null);
							
							// ���������ڵ� (51~54)
							rtRecord.setData("h_bankResponseCode", null);
							
							// ��ȸ���� (55~62)
							rtRecord.setData("h_lookupDate", null);
							
							// ��ȸ��ȣ (63~68)
							rtRecord.setData("h_lookupNum", null);
							
							// ����������ȣ (69~83)
							rtRecord.setData("h_bankMsgNum", null);
							
							// �����ڵ�3 (84~86)
							rtRecord.setData("h_bankCode3", bankCode3);
							
							// ���� (87~99)
							rtRecord.setData("h_spare", null);
							
							// [2.������ (200Byte)]
							// [�۱���ü/������ü]
							if (Arrays.equals(messageCode, envVarMap.get("MessageCode_0100"))) {
								if (Arrays.equals(workTypeCode, envVarMap.get("WorkTypeCode_100"))) {
									// ��ݰ��¹�ȣ (100~114)
									rtRecord.setData("dt_withdrawalAccountNum", envVarMap.get("FB_PARENT_ACCOUNT_NUMB"));
									
									// �����й�ȣ (115~122)
									rtRecord.setData("dt_bankBookPassword", null);
									
									// �����ȣ (123~128)
									rtRecord.setData("dt_recoveryCode", null);
									
									// ��ݱݾ� (129~141)
									rtRecord.setData("dt_withdrawalAmount", inRecord.getData("d_requestTransferPrice"));
									
									// ������ܾ׺�ȣ (142)
									rtRecord.setData("dt_afterWithdrawalBalanceSign", null);
									
									// ������ܾ� (143~155)
									rtRecord.setData("dt_afterWithdrawalBalance", null);
									
									// �Ա������ڵ�2 (156~157)
									final byte[] depositBankCode3 = inRecord.getData("d_bankCode");
									final byte[] depositBankCode2 = Arrays.copyOfRange(depositBankCode3, 1, 3);
									rtRecord.setData("dt_depositBankCode2", depositBankCode2);
									
									// �Աݰ��¹�ȣ (158~172)
									rtRecord.setData("dt_depositAccountNum", inRecord.getData("d_accountNum"));
									
									// ������ (173~181)
									rtRecord.setData("dt_fees", null);
									
									// ��ü�ð� (182~187)
									rtRecord.setData("dt_transferTime", transferTime);
									
									// �Աݰ������� (188~207)
									rtRecord.setData("dt_depositAccountBriefs", envVarMap.get("FB_PARENT_COMP_NAME"));
									
									// CMS�ڵ� (208~223)
									rtRecord.setData("dt_cmsCode", null);
									
									// �ſ�Ȯ�ι�ȣ (224~236)
									rtRecord.setData("dt_identificationNum", null);
									
									// �ڵ���ü���� (237~238)
									rtRecord.setData("dt_autoTransferClassification", null);
									
									// ��ݰ������� (239~258)
									rtRecord.setData("dt_withdrawalAccountBriefs", inRecord.getData("d_briefs"));
									
									// �Ա������ڵ�3 (259~261)
									rtRecord.setData("dt_depositBankCode3", depositBankCode3);
									
									// �޿����� (262)
									rtRecord.setData("dt_salaryClassification", null);
									
									// ���� (263~299)
									rtRecord.setData("dt_spare", null);
								}
								// [����]
								else {
									Logger.logln(Logger.LogType.LT_ERR, "�� �� ���� �޽����ڵ�. (workTypeCode: " + new String(workTypeCode) + ", messageCode: " + new String(messageCode) + ")");
									return null;
								}
							}
							// [ó�������ȸ,�ܾ���ȸ,������ȸ]
							else if(Arrays.equals(messageCode, envVarMap.get("MessageCode_0600"))) {
								// [ó�������ȸ]
								if (Arrays.equals(workTypeCode, envVarMap.get("WorkTypeCode_101"))) {
									// ���� ��� �߰�...
								}
								// [�ܾ���ȸ]
								else if (Arrays.equals(workTypeCode, envVarMap.get("WorkTypeCode_300"))) {
									// ���� ��� �߰�...
								}
								// [������ȸ]
								else if (Arrays.equals(workTypeCode, envVarMap.get("WorkTypeCode_400"))) {
									// ���� ��� �߰�...
								}
								// [����]
								else {
									Logger.logln(Logger.LogType.LT_ERR, "�� �� ���� ���������ڵ�. (" + new String(workTypeCode) + ")");
									return null;
								}
							}
							// [����]
							else {
								Logger.logln(Logger.LogType.LT_ERR, "�� �� ���� �޽����ڵ�. (workTypeCode: " + new String(workTypeCode) + ", messageCode: " + new String(messageCode) + ")");
								return null;
							}
						}
						else if (inRecordSubTypeName.equals("Head")) {
							// ǥ���� - ���������ڵ� ���� ������ ����
							byte[] cliWorkTypeCode = inRecord.getData("h_taskComp");
							byte[] svrWorkTypeCode = new byte[cliWorkTypeCode.length + 1];
							
							for (int i = 0; i < cliWorkTypeCode.length; ++i) {
								svrWorkTypeCode[i] = cliWorkTypeCode[i];
							}
							
							svrWorkTypeCode[cliWorkTypeCode.length] = (byte)'0';
							localVarMap.put("loc_hana_svr_h_workTypeCode", svrWorkTypeCode);
						}
						else if (inRecordSubTypeName.equals("Tail")) {
							// ����� - ������ ������ ����
						}
					}
					else {
						Logger.logln(Logger.LogType.LT_ERR, "�ùٸ��� ���� inRecordTypeName. (\"" + inRecordTypeName + "\")");
					}
				}
				// Server -> Client
				else if (outRecordTypeName.equals("HanaRecordClient")) {
					if (outRecordSubTypeName.equals("Data")) {
						// [�����ͺ� (82Byte)]
						int recordLength = attrMgr.getRecordSizeFromAttributeMap("HanaAttrClient_Data");
						byte[] dummyDatas = new byte[recordLength];
						rtRecord = new Record("HanaRecordClient", "Data", inRecord.getIndex(), dummyDatas);

						// �ĺ��ڵ� (0)
						rtRecord.setDataByDefault("d_idCode");
						
						// ������ �Ϸù�ȣ (1~6)
						rtRecord.setData("d_dataSerialNum", inRecord.getData("h_msgNum"));
						
						// �����ڵ� (7~10)
						rtRecord.setData("d_bankCode", inRecord.getData("dt_depositBankCode3"));
			
						// ���¹�ȣ (10~23)
						rtRecord.setData("d_accountNum", inRecord.getData("dt_depositAccountNum"));
						
						// ��ü��û�ݾ� (24~34)
						final byte[] withdrawlAmount = inRecord.getData("dt_withdrawalAmount");
						rtRecord.setData("d_requestTransferPrice", withdrawlAmount);
						{
							// �� �Ƿ�Ƚ�� ����
							final byte[] savedTotalRequestCnt = localVarMap.get("loc_hana_cli_t_totalRequestCnt");
							if (savedTotalRequestCnt != null) { 
								localVarMap.put("loc_hana_cli_t_totalRequestCnt", Long.toString(Long.parseLong(new String(savedTotalRequestCnt)) + 1).getBytes());
							}
							else {
								localVarMap.put("loc_hana_cli_t_totalRequestCnt", "1".getBytes());
							}
							
							// �� �Ƿڱݾ� ����
							final byte[] savedTotalRequestAmount = localVarMap.get("loc_hana_cli_t_totalRequestPrice");
							if (savedTotalRequestAmount != null) { 
								localVarMap.put("loc_hana_cli_t_totalRequestPrice", Long.toString(Long.parseLong(new String(savedTotalRequestAmount)) + Long.parseLong(new String(withdrawlAmount))).getBytes());
							}
							else {
								localVarMap.put("loc_hana_cli_t_totalRequestPrice", withdrawlAmount);
							}
						}

						// ������ü�ݾ� (35~45)
						rtRecord.setData("d_realTransferPrice", inRecord.getData("dt_withdrawalAmount"));
						
						// �ֹ�/����ڹ�ȣ (46~58)
						rtRecord.setData("d_recieverIdNum", null);
						
						// ó����� (59)
						final byte[] bankResponseCode = inRecord.getData("h_bankResponseCode");
						if (Arrays.equals(bankResponseCode, envVarMap.get("ProcessingResultOk"))) { // ���� ó��
							final byte[] procY = { 'Y' };
							rtRecord.setData("d_processingResult", procY);
							{
								// ����ó���Ǽ� ����
								final byte[] savedNormalProcCnt = localVarMap.get("loc_hana_cli_t_normalProcessingCnt");
								if (savedNormalProcCnt != null) {
									localVarMap.put("loc_hana_cli_t_normalProcessingCnt", Long.toString(Long.parseLong(new String(savedNormalProcCnt)) + 1).getBytes());
								}
								else {
									localVarMap.put("loc_hana_cli_t_normalProcessingCnt", "1".getBytes());
								}
								
								// ����ó���ݾ� ����
								final byte[] savedNormalPriceCnt = localVarMap.get("loc_hana_cli_t_normalPriceCnt");
								if (savedNormalPriceCnt != null) { 
									localVarMap.put("loc_hana_cli_t_normalPriceCnt", Long.toString(Long.parseLong(new String(savedNormalPriceCnt)) + Long.parseLong(new String(withdrawlAmount))).getBytes());
								}
								else {
									localVarMap.put("loc_hana_cli_t_normalPriceCnt", withdrawlAmount);
								}
							}
						}
						else { // �Ҵ� ó��
							final byte[] procN = { 'N' };
							rtRecord.setData("d_processingResult", procN);
							{
								// �Ҵ�ó���Ǽ� ����
								final byte[] savedDisableProcCnt = localVarMap.get("loc_hana_cli_t_disableProcessingCnt");
								if (savedDisableProcCnt != null) {
									localVarMap.put("loc_hana_cli_t_disableProcessingCnt", Long.toString(Long.parseLong(new String(savedDisableProcCnt)) + 1).getBytes());
								}
								else {
									localVarMap.put("loc_hana_cli_t_disableProcessingCnt", "1".getBytes());
								}
								
								// �Ҵ�ó���ݾ� ����
								final byte[] savedDisablePriceCnt = localVarMap.get("loc_hana_cli_t_disablePriceCnt");
								if (savedDisablePriceCnt != null) { 
									localVarMap.put("loc_hana_cli_t_disablePriceCnt", Long.toString(Long.parseLong(new String(savedDisablePriceCnt)) + Long.parseLong(new String(withdrawlAmount))).getBytes());
								}
								else {
									localVarMap.put("loc_hana_cli_t_disablePriceCnt", withdrawlAmount);
								}
							}
						}
						
						// �Ҵ��ڵ� (60~63)
						rtRecord.setData("d_disableCode", bankResponseCode);
						
						// ���� (64~75)
						rtRecord.setData("d_briefs", inRecord.getData("dt_withdrawalAccountBriefs"));
						
						// ���� (76~79)
						rtRecord.setData("d_blank", null);
						
						// ���๮�� (80~81)
						rtRecord.setData("d_newLine", NEW_LINE);
					}
					else if (outRecordSubTypeName.equals("Head")) {
						// [ǥ���� (82Byte)]
						int recordLength = attrMgr.getRecordSizeFromAttributeMap("HanaAttrClient_Head");
						byte[] dummyDatas = new byte[recordLength];
						rtRecord = new Record("HanaRecordClient", "Head", 0, dummyDatas);
						
						// �ĺ� �ڵ� (0)
						rtRecord.setDataByDefault("h_idCode");
						
						// ���� ���� (1~2)
						rtRecord.setDataByDefault("h_taskComp");
						
						// ���� �ڵ� (3~5)
						rtRecord.setDataByDefault("h_bankCode");
						
						// ��ü �ڵ� (6~13)
						rtRecord.setDataByDefault("h_companyCode");
						
						// ��ü�Ƿ����� (14~19)
						rtRecord.setDataByDefault("h_comissioningDate");
						
						// ��üó������ (20~25)
						final byte[] h_processingDate = { (byte)year.charAt(0),  (byte)year.charAt(1), (byte)year.charAt(2), (byte)year.charAt(3),
														  (byte)month.charAt(0), (byte)month.charAt(1), 
														  (byte)date.charAt(0),  (byte)date.charAt(1) };
						rtRecord.setData("h_processingDate", h_processingDate);
						
						// ����¹�ȣ (26~39)
						rtRecord.setDataByDefault("h_motherAccountNum");
						
						// ��ü���� (40~41)
						rtRecord.setDataByDefault("h_transferType");
						
						// ȸ���ȣ (42~47)
						rtRecord.setDataByDefault("h_companyNum");
						
						// ó������뺸���� (48)
						rtRecord.setDataByDefault("h_resultNotifyType");
						
						// �������� (49)
						rtRecord.setDataByDefault("h_transferCnt");
						
						// ��й�ȣ (50~57)
						rtRecord.setDataByDefault("h_password");
						
						// ���� (58~76)
						rtRecord.setData("h_blank", null);
						
						// Format (77)
						rtRecord.setDataByDefault("h_format");
						
						// VAN (78~79)
						final byte[] bVan = { 'K', 'C' };
						rtRecord.setData("h_van", bVan);
						
						// ���๮�� (80~81)
						rtRecord.setData("h_newLine", NEW_LINE);
					}
					else if (outRecordSubTypeName.equals("Tail")) {
						// [����� (82Byte)]
						int recordLength = attrMgr.getRecordSizeFromAttributeMap("HanaAttrClient_Tail");
						byte[] dummyDatas = new byte[recordLength];
						
						rtRecord = new Record("HanaRecordClient", "Tail", Integer.parseInt(new String(localVarMap.get("loc_hana_cli_t_totalRequestCnt"))), dummyDatas);
						
						// �ĺ��ڵ� (0)
						rtRecord.setDataByDefault("t_idCode");
						
						// ���ǷڰǼ� (1~7)
						rtRecord.setData("t_totalRequestCnt", localVarMap.get("loc_hana_cli_t_totalRequestCnt"));
						
						// ���Ƿڱݾ� (8~20)
						rtRecord.setData("t_totalRequestPrice", localVarMap.get("loc_hana_cli_t_totalRequestPrice"));
						
						// ����ó���Ǽ� (21~27)
						rtRecord.setData("t_normalProcessingCnt", localVarMap.get("loc_hana_cli_t_normalProcessingCnt"));
						
						// ����ó���ݾ� (28~40)
						rtRecord.setData("t_normalProcessingPrice", localVarMap.get("loc_hana_cli_t_normalPriceCnt"));
						
						// �Ҵ�ó���Ǽ� (41~47)
						rtRecord.setData("t_disableProcessingCnt", localVarMap.get("loc_hana_cli_t_disableProcessingCnt"));
						
						// �Ҵ�ó���ݾ� (48~60)
						rtRecord.setData("t_disableProcessingPrice", localVarMap.get("loc_hana_cli_t_disablePriceCnt"));
						
						// �����ȣ (61~68)
						rtRecord.setDataByDefault("t_recoveryCode");
						
						// ���� (69~79)
						rtRecord.setData("t_blank", null);
						
						// ���๮�� (80~81)
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
			
			// ������
			public RecordPrinter(Record record) {
				this.record = record;
			}
			
			public void setRecord(Record record) {
				this.record = record;
			}
			
			// ��ĺ� ����Լ�
			// �ϳ�����_������ü_ǥ����
			private void print_hana_record() {
				int attrSize = record.getAttrMap().size();
				Attribute[] attrAry = new Attribute[attrSize];
				
				for (Map.Entry<String, Attribute> entry : record.getAttrMap().entrySet()) {
					Attribute attr = entry.getValue();
					int number = attr.getNumber();
					
					attrAry[number - 1] = attr;
				}
			
				System.out.println(String.format("%s\t%-40s\t%s\t%s\t%-20s", "����", "�̸�(�ڵ��)", "�����ε���", "����Ʈ����", "���簪"));
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
				
				// �ϳ�����_������ü_ǥ����_�����ͺ�_����� ��� ���
				if (recordType.equals("HanaRecordClient")) {
					String recordSubType = record.getSubTypeName();
					
					if (recordSubType.equals("Head")) {
						System.out.println("=[ǥ����]=================================================================================");
						print_hana_record();
						System.out.println("=================================================================================[ǥ����]=");
					}
					else if (recordSubType.equals("Data")) {
						System.out.println("=[�����ͺ�]================================================================================");
						print_hana_record();
						System.out.println("================================================================================[�����ͺ�]=");
					}
					else if (recordSubType.equals("Tail")) {
						System.out.println("=[�����]=================================================================================");
						print_hana_record();
						System.out.println("=================================================================================[�����]=");
					}			
				}
				// ��ȭ �߹�ŷ ��� ���
				else if (recordType.equals("HanaRecordServer")) {
					print_hana_record();
				}
				else if (false) {
					// ���⿡ ���ο� ���Ÿ�� �߰�...
				}
				else {
					System.out.println(this.getClass().getName());
					Logger.logln(Logger.LogType.LT_ERR, "�������� �ʴ� ���� ����. (" + recordType + ")");
					return;
				}
			}
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////
		//[RecordTransceiver.java]//////////////////////////////////////////////////////////////////////
		
		public static class RecordTransceiver {
			private boolean reusableSocketMode;				// ���� ���� ���(true), ��ȸ�� ���� ���(false) ���

			private String ip;								// ���� ������
			private int port;								// ���� ��Ʈ
			private int sendTryCnt;							// ���� �õ� ����
			private long sendDelay;							// ���� ��� �ð�
			private int sendWaitingRecordListMaxSize;		// �ִ� ���۴�� ����ũ ũ��
			
			private List<Socket> socketList;					// �ۼ��� ���� ����Ʈ (Thread-Safe ArrayList)
			private List<Record> sendWaittingRecordList;		// ������ �� ������ ����Ʈ (Thread-Safe LinkedList)
			private List<Record> fileWriteLeftRecordList;		// ���� �ۼ��۾��� ���� ���ڵ� (Thread-Safe LinkedList)
			private List<Integer> fileWriteDoneRecordIndexList;	// ���� �ۼ��۾��� �Ϸ�� ���ڵ� �ε��� (Thread-Safe ArrayList)

			private RecordConverter cliRecordConverter;		// �������� ���� ���ڵ带 Ŭ���̾�Ʈ �������� �����ϴ� Ŭ����
			private KsFileWriter ksFileWriter;				// .rpy ���Ͽ� ���� ����� ���� Ŭ����
			private TransceiveLogger fileLogger;			// �ۼ��� �ΰ�
			
			private SendSocketThread sendSocketThread;		// ���ۿ� ���� ������ ������
			private RecvSocketThread recvSocketThread;		// ���ſ� ���� ������ ������
			private Thread sendThread;						// ���ۿ� ������
			private Thread recvThread;						// ���ſ� ������
			
			private HashMap<String, byte[]> envVarMap;		// ȯ�溯��
			
			public RecordTransceiver (boolean reusableSocketMode, String ip, int port, int socketCnt, int sendWaitingRecordListMaxSize, RecordConverter cliRecordConverter, KsFileWriter ksFileWriter, HashMap<String, byte[]> envVarMap) throws Exception {
				this.reusableSocketMode = reusableSocketMode;
				
				this.ip = ip;
				this.port = port;
				this.sendTryCnt = 0;
				this.sendDelay = 1000;					// �ʱ� ���� ���ð� 1000ms
				this.sendWaitingRecordListMaxSize = 50;	// �ִ� 50������ ���۴�� ����Ʈ�� ���� ����
				
				this.socketList = Collections.synchronizedList(new ArrayList<Socket>());			// Thread-Safe ���� ����Ʈ
				
				if (this.reusableSocketMode) {
					addSocket(socketCnt);
				}

				this.sendWaittingRecordList = Collections.synchronizedList(new LinkedList<Record>());	// Thread-Safe ������ �� ������ ����Ʈ
				this.fileWriteLeftRecordList = Collections.synchronizedList(new LinkedList<Record>());	// Thread-Safe ���� �ۼ��۾��� ���� ���ڵ� ����Ʈ
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
			
			// �ش� ������ŭ ��Ĺ ���� (������ ���� �Ұ�, �߰��� ����)
			public void addSocket(int addCnt) {
				for (int i = 0; i < addCnt; ++i) {
					boolean hasError = false;
						
					try {
						socketList.add(new Socket(ip, port));
					}
					catch (IOException ioe) {
						// If an error occurs during the connection.
						Logger.logln(Logger.LogType.LT_ERR, ioe);
						Logger.logln(Logger.LogType.LT_ERR, "���� ���� ����. (Ip: " + ip + ", Port: " + port + ")");
						hasError = true;
					}
					catch (IllegalBlockingModeException ibme) {
						// If ths socket has an associated channel, and the channel is in non-blocking mode.
						Logger.logln(Logger.LogType.LT_ERR, ibme);
						Logger.logln(Logger.LogType.LT_ERR, "���� ���� ����. (non-blocking mode)");
						hasError = true;
					}
					catch (IllegalArgumentException iae) {
						// If endpoint is null or a SocketAddress subclass not supported by this socket.
						Logger.logln(Logger.LogType.LT_ERR, iae);
						Logger.logln(Logger.LogType.LT_ERR, "���� �ּ� ����. (Ip: " + ip + ", Port: " + port + ")");
						hasError = true;
					}
					finally {
						if (hasError) {
							Logger.logln(Logger.LogType.LT_CRIT, "IP: " + ip + ", Port: " + port + " ���� ���� ����. ���� ���� Ȯ�ιٶ��ϴ�.");
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
						Logger.logln(Logger.LogType.LT_ERR, "���� �ݱ� ����.");
					}
				}
				
				socketList.clear();
			}
			
			// �ۼ��ű� �ݱ�
			public void close() {
				try {
					// Send ������ ����
					sendSocketThread.close();
					sendThread.join();
				}
				catch (Exception e) {
					Logger.logln(Logger.LogType.LT_ERR, e);
				}
				
				try {
					// Recv ������ ����
					recvSocketThread.close();
					recvThread.join();
				}
				catch (Exception e) {
					Logger.logln(Logger.LogType.LT_ERR, e);
				}
				
				// �α����� �ۼ�
				fileLogger.writeToFile();
				
				// ���� �ı�
				destroySocketAll();
				
				// ���� ���
				int recvFailedCnt = fileWriteLeftRecordList.size();
				int recvSuccessCnt = sendTryCnt - recvFailedCnt;
				float recvFailedPercent = recvFailedCnt * 100.0f / sendTryCnt;
				float recvSuccessPercent = 100.0f - recvFailedPercent;
				
				System.out.println();
				System.out.println(Global.FC_GREEN + String.format("***** ���� �õ� ����: %d, ���� ���� ����: %d(%.02f%%), ���� ���� ����: %d(%.02f%%) ******", sendTryCnt, recvSuccessCnt, recvSuccessPercent, recvFailedCnt, recvFailedPercent) + Global.FC_RESET);
				
				if (recvFailedCnt > 0) {
					System.out.print(Global.FC_GREEN + String.format("***** ������ ���ڵ� �ε���: ", sendTryCnt, recvSuccessCnt, recvSuccessPercent, recvFailedCnt, recvFailedPercent) + Global.FC_RESET);
				
					for (Record failedRecord : fileWriteLeftRecordList) {
						System.out.print(Global.FC_GREEN + failedRecord.getIndex() + ", " + Global.FC_RESET);
					}
				}
				System.out.println();
				
				// �ʱ�ȭ
				sendWaittingRecordList.clear();
				fileWriteLeftRecordList.clear();
			}
			
			// ���� ���ڵ� �߰�
			public void send(Record sendRecord) {
				sendWaittingRecordList.add(sendRecord);
				++sendTryCnt;
			}
			
			// �ۼ��� �Ϸ� ���� Ȯ��
			public boolean checkTransceiverFinished() {
				// �� ������ ��� Ÿ�Ӿƿ��� ��� true
				if ((sendSocketThread.getThreadTimeoutLeft() == 0 && recvSocketThread.getThreadTimeoutLeft() == 0)) {
					return true;
				}
				
				return false;
			}
			
			// �ۼ��� ������ ���� ���
			public void printWorkLeft() {
				System.out.println();
				System.out.println(Global.FC_YELLOW + "*** [ Time: " + System.currentTimeMillis() + " ]" + Global.FC_RESET);
				System.out.println(Global.FC_YELLOW + "*** ���� ������� ���ڵ�: " + sendWaittingRecordList.size() + "�� / ���� ������� ���ڵ�: " + fileWriteLeftRecordList.size() + "��" + Global.FC_RESET);
				System.out.println(Global.FC_YELLOW + "*** �ۼ� �Ϸ� ���ڵ� : " + recvSocketThread.getFileWriteDoneRecordIndexListSize() + "�� / ���� ���� ��� �ð�: " + getSendDelay() + "ms" + Global.FC_RESET);
				System.out.println(Global.FC_YELLOW + "*** �ʴ� ���� �ӵ�/����: " + String.format("%.2f/%d��", (sendThread.isAlive() ? sendSocketThread.getCurSendPerSec() : 0.00f), (sendThread.isAlive() ? sendSocketThread.getTgtSendPerSec() : 0)) + Global.FC_RESET);
				System.out.println(Global.FC_YELLOW + "*** ���� ������ Ÿ�Ӿƿ�: " + sendSocketThread.getThreadTimeoutLeft() + "ms / ���� ������ Ÿ�Ӿƿ�: " + recvSocketThread.getThreadTimeoutLeft() + "ms" + Global.FC_RESET);
				System.out.println();
			}
			
			// ���� ��� ����Ʈ ũ�� ���� Ȯ��
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
			public long DEFAULT_THREAD_TIMEOUT;	// n�� ���� �ƹ� ������ ������ ������ Ÿ�Ӿƿ�
			public int MAX_RESEND_COUNT;		// �ִ� n�� ������ ���
			public int DEFAULT_RESEND_DELAY;	// n�� �̳� ���Ź��� ���ϸ� ������ �õ�
			
			protected RecordTransceiver recordTransceiver;	// ������ ��Ʈ�ѷ� RecordTransceiver Ŭ����
			
			protected String ip;				// ���� IP
			protected int port;					// ���� Port
			
			protected boolean running;			// ������ ����
			protected long threadTimeoutLeft;	// ���� ������ ���ð� (�и���)
			protected long lastTime;			// ������ �ð� (�и���)
			protected long timeDelta;			// �� ƽ ��� �ð� (�и���)
			protected long workDelayLeft;		// �۾� ������ ���� ���ð�
			
			protected List<Socket> socketList;						// �ۼ��� ���� ����Ʈ (Thread-Safe ArrayList)
			protected List<Record> fileWriteLeftRecordList;			// ���� �ۼ��۾��� ���� ���ڵ� Thread-Safe LinkedList)
			protected List<Integer> fileWriteDoneRecordIndexList; 	// ���� �ۼ��۾��� �Ϸ�� ���ڵ� �ε��� (Thread-Safe ArrayList)
			protected int curWorkSocketIndex;		// ���� ����ؾ� �� ���� �ε���
			protected TransceiveLogger fileLogger;	// �ۼ��� �ΰ�
			
			protected HashMap<String, byte[]> envVarMap;	// ȯ�溯��
			
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
			
			// ������ �ð� ������Ʈ
			protected boolean updateThreadTime() {
				long curTime = System.currentTimeMillis();
				timeDelta = curTime - lastTime;
				
				// �����尡 Ÿ�Ӿƿ��̸� false
				if ((threadTimeoutLeft -= timeDelta) <= 0) {
					threadTimeoutLeft = 0;
					lastTime = curTime;
					setRunning(false);
					return false;
				}
				
				// ������ �����̰� 0 �̻��� ��� �� ���� ���� ���
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
			
			// ����Ʈ �迭�� ���ڵ� ����
			protected Record makeRecord(String recordType, int recordIndex, byte[] prefixAry, byte[] suffixAry, byte[] byteAry) {
				// ��ȯ�� ����Ʈ �迭 ����
				int recordByteLength = byteAry.length - prefixAry.length - suffixAry.length;
				byte[] recordByte = new byte[recordByteLength];
				
				// Prefix, Suffix�����ϰ� ����
				int beginIndex = prefixAry.length;
				
				for (int i = 0; i < recordByteLength; ++i) {
					recordByte[i] = byteAry[i + beginIndex];
				}
				
				// ���ڵ� ����
				Record rtRecord = new Record(recordType, recordIndex, recordByte);
				
				return rtRecord;
			}
			
			// �迭�� Ȯ���Ͽ� �� �迭�� ��ȯ�ϴ� �Լ�
			protected byte[] makeAppendedByteAry(byte[] leftAry, byte[] rightAry) {
				// �迭 ũ�� ����
				int leftLength = 0, rightLength = 0, rtLength = 0;
				
				if (leftAry != null) {
					leftLength = leftAry.length;
				}
				
				if (rightAry != null) {
					rightLength = rightAry.length;
				}
				
				if ((rtLength = leftLength + rightLength) <= 0) return null; // left, right�迭�� �� �� null�� ���
					
				// ��ȯ�� �迭 ����
				byte[] rtByte = new byte[rtLength];
				int index = 0;
				
				// ���� �迭 ���� ����
				if (leftAry != null) {
					for (int i = 0; i < leftAry.length; ++i) {
						rtByte[index++] = leftAry[i];
					}
				}
				
				// ���� ��� ���� ����
				if (rightAry != null) {
					for (int j = 0; j < rightAry.length; ++j) {
						rtByte[index++] = rightAry[j];
					}
				}
				
				return rtByte;
			}
			
			// [beginIndex ~ endIndex) ������ �迭 ���Ҹ� �����ϰ� �� �迭�� ��ȯ�ϴ� �Լ�
			protected byte[] makeRemovedByteAryByIndex(byte[] originAry, int beginIndex, int endIndex) {
				// �ε��� ��
				if (beginIndex > endIndex) {
					Logger.logln(Logger.LogType.LT_WARN, "beginIndex(" + beginIndex + ") > endIndex(" + endIndex + "�� ���� ��ü�Ͽ� �����մϴ�.");
					
					int tempIndex = beginIndex;
					beginIndex = endIndex;
					endIndex = tempIndex;
				}
				
				// ���� ��
				int originLength = originAry.length;
				int removeLength = endIndex - beginIndex;
				int rtLength = originLength - removeLength;
				
				if (rtLength < 0) {
					Logger.logln(Logger.LogType.LT_ERR, "������ �迭 ������ ���� �迭 ���̸� �ʰ��մϴ�. (originAry.length: " + originLength + ", removeLength: " + removeLength + ")");
					return null;
				}
				else if (rtLength == 0) {
					return null;
				}
				
				// ��ȯ �迭 ����
				byte[] rtByte = new byte[rtLength];
				int rtIndex = 0;

				// ���� ����
				for (int i = 0; i < originAry.length; ++i) {
					if (i < beginIndex || i >= endIndex) {
						if (rtIndex < rtByte.length) {
							rtByte[rtIndex++] = originAry[i];
						}
						else {
							Logger.logln(Logger.LogType.LT_ERR, "�迭 ���� ����. (rtByte.length: " + rtByte.length + ", rtIndex: " + rtIndex + ")");
							break;
						}
					}
				}
				
				return rtByte;
			}
			
			// targetAry�� elementAry�� ������� ���ԵǾ� ������ �ش� �ε��� ��ȯ, ���ԵǾ����� ���� ��� -1�� ��ȯ
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
				
			// ���� �������� �õ��ϴ� �Լ�
			protected synchronized boolean reconnectSocket(int tryCnt, int tryInterval) {
				boolean isReconnectOk = false;
				int leftTryCnt = tryCnt;
				long curTime = System.currentTimeMillis();
				long nextTryTime = curTime + 1000; // ���� 1�� ���
				
				Logger.logln(Logger.LogType.LT_INFO, "����(Index: " + curWorkSocketIndex + ") ���� ������ ����. �ݱ� �� �������� �õ��մϴ�. (TryingCnt: " + (tryCnt - leftTryCnt) + ")");
				
				// ���� ���� �ݱ�
				while (leftTryCnt > 0) {
					curTime = System.currentTimeMillis();
					
					if (nextTryTime < curTime) {
						nextTryTime = curTime + tryInterval;
						--leftTryCnt;
						
						try {
							Logger.logln(Logger.LogType.LT_INFO, "����(Index: " + curWorkSocketIndex + ") �ݱ� �õ���. (TryingCnt: " + (tryCnt - leftTryCnt) + ")");
							
							Socket errSocket = socketList.get(curWorkSocketIndex);
							
							if (errSocket != null) {
								InputStream is = errSocket.getInputStream();
								int isLeftByte = is.available();
							
								if (isLeftByte > 0) {
									Logger.logln(Logger.LogType.LT_INFO, "InputStream�� ���� ������(+  " + isLeftByte + "Bytes) �д� ��.");
								}
								else {
									Logger.logln(Logger.LogType.LT_INFO, "���Ͽ� ���� ������ ����: " + errSocket.getInputStream().available() + "bytes");
									errSocket.close();
									Logger.logln(Logger.LogType.LT_INFO, "���� �ݱ� ����.");
									break;
								}
							}
						}
						catch (IOException ioe1) {
							Logger.logln(Logger.LogType.LT_ERR, ioe1);
							Logger.logln(Logger.LogType.LT_ERR, "���� �ݱ� ����.");
							break;
						}
					}
				}
				
				leftTryCnt = tryCnt;
				
				// ���� ����� �� ����
				while (leftTryCnt > 0) {
					curTime = System.currentTimeMillis();
					
					if (nextTryTime < curTime) {
						nextTryTime = curTime + tryInterval;
						--leftTryCnt;
						
						// ���ο� ���� ���� �� ������ �õ�
						Logger.logln(Logger.LogType.LT_INFO, "����(Index: " + curWorkSocketIndex + ") ������ �õ���. (TryingCnt: " + (tryCnt - leftTryCnt) + ")");
						
						try {
							socketList.set(curWorkSocketIndex, new Socket(ip, port));
							isReconnectOk = true;
							break;
						}
						catch (IOException ioe) {
							// If an error occurs during the connection.
							Logger.logln(Logger.LogType.LT_ERR, ioe);
							Logger.logln(Logger.LogType.LT_ERR, "���� ������ ����. (Ip: " + ip + ", Port: " + port + ")");
						}
						catch (IllegalBlockingModeException ibme) {
							// If ths socket has an associated channel, and the channel is in non-blocking mode.
							Logger.logln(Logger.LogType.LT_ERR, ibme);
							Logger.logln(Logger.LogType.LT_ERR, "���� ������ ����. (non-blocking mode)");
						}
						catch (IllegalArgumentException iae) {
							// If endpoint is null or a SocketAddress subclass not supported by this socket.
							Logger.logln(Logger.LogType.LT_ERR, iae);
							Logger.logln(Logger.LogType.LT_ERR, "������ �ּ� ����. (Ip: " + ip + ", Port: " + port + ")");
						}
					}
				}
				
				if (isReconnectOk) {
					Logger.logln(Logger.LogType.LT_INFO, "����(Index: " + curWorkSocketIndex + ") ������ ����. (TryingCnt: " + (tryCnt - leftTryCnt) + ")");
				}
				else {
					Logger.logln(Logger.LogType.LT_INFO, "����(Index: " + curWorkSocketIndex + ") ������ ����. (TryingCnt: " + (tryCnt - leftTryCnt) + ")");
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
			
			private float curSendPerSec;					// ���� �ʴ� ���۷�
			private int tgtSendPerSec;						// ��ǥ �ʴ� ���۷�
			
			private List<Record> sendWaittingRecordList;	// ������ �� ������ ����Ʈ (Thread-Safe LinkedList)

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
					// ������ �ð� ������Ʈ
					if (!updateThreadTime()) continue;
								
					// �ʴ� ���ۼӵ� �ʰ� ����
					if ((curSendPerSec = sendCntFromStart / ((System.currentTimeMillis() - sendStartTime) / 1000.0f)) > tgtSendPerSec) {
						continue;
					}
					
					// ���� ��� ����Ʈ�� ���ڵ� ���� �õ�
					try {
						if ((sendStream = sendWork(sendStream)) == null) { // ���� ����
							++sendCntFromStart;
						}
					}
					catch (IOException ioe) {
						Logger.logln(Logger.LogType.LT_ERR, ioe);
						Logger.logln(Logger.LogType.LT_ERR, Global.FC_WHITE + "OutputStream�� ���� �����Դϴ�. (SocketIndex: " + curWorkSocketIndex + ")" + Global.FC_RESET);
						
						// [Note] OutputStream.write()���� IOException�� �߻��ϴ� ����, InputStream�� �������� ���� �����͸� ������ �� �����. (����, ��Ʈ��ũ ��Ȳ�� ���� ���� �߻��� ���ڵ� -N���� ���ڵ尡 �ҽ� ���ɼ��� �ְ�, 
						// ���� ���� ��忡�� ���� ���� �߻� �� 2���� �����͸� .write() �� �� ���� �������� ���ϹǷ� +2���� �����Ͱ� �߰��� �ҽǵ�)
						
						if (!reconnectSocket(50, 500)) { // �ִ� 50ȸ,0.5�� �������� ������ �õ�
							Logger.logln(Logger.LogType.LT_ERR, Global.FC_RED + "���� �����ӿ� �����Ͽ����ϴ�. (SocketIndex: " + curWorkSocketIndex +  ")" + Global.FC_RESET); // OFT
							
							if (socketList.size() == 0) {
								Logger.logln(Logger.LogType.LT_CRIT, Global.FC_RED + "������ ����� ������ �����ϴ�. ������ ���� �����մϴ�." + Global.FC_RESET); // OFT
								setThreadTimeoutLeft(0);
								continue;
							}
						}
					}
						
					// �������� �������� ���� �����͸� ���۴�� ����Ʈ�� ������Ͽ� ������ �õ�
					resendWork();
					
					// ���� �ӵ� ����
					controlSendSpeed();
				}
				
				Logger.logln(Logger.LogType.LT_INFO, Global.FC_WHITE + "SendSocketThread ����." + Global.FC_RESET);
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
			
			// ���� �õ�
			private byte[] sendWork(byte[] sendStream) throws IOException {
				Socket sendSocket = null;
				OutputStream os = null;
				Record sendRecord = null;
				
				// ���� ���ۿ��� ������ �����Ͱ� ���� ��� ����Ʈ �����͸� �о��
				if (sendStream == null) {
					if (!sendWaittingRecordList.isEmpty()) { // ������ ���ڵ尡 ����
						// ���� ���� �� ���ڵ� ����Ʈ ��ȯ
						sendRecord = sendWaittingRecordList.remove(0);
						sendStream = makeSendStream(sendRecord);
					}
				}
				
				if (sendStream != null) {					
					// ����
					if (recordTransceiver.isReusableSocketMode()) {
						sendSocket = socketList.get(curWorkSocketIndex);
					}
					else {
						recordTransceiver.addSocket(1);
						sendSocket = socketList.get(socketList.size() - 1);
					}
					
					os = sendSocket.getOutputStream();
					os.write(sendStream, 0, sendStream.length); // IOException �߻� ����
					sendRecord.addSendCnt(1);
					sendRecord.setLastSendTime(System.currentTimeMillis());
					fileWriteLeftRecordListSyncWork("add", sendRecord);
					threadTimeoutLeft = DEFAULT_THREAD_TIMEOUT;
					
					// �ð� ���� �� �α�
					SimpleDateFormat dateTime = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
					String today = dateTime.format(new Date(System.currentTimeMillis()));
					String hour = today.substring(11, 13),	min = today.substring(14, 16),	sec = today.substring(17, 19);
					fileLogger.log(String.format("%s%s%s: snd(%04d)=(", hour, min, sec, sendStream.length), new String(sendStream), ")");
					Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_WHITE + String.format("\n[Soc%d]%s%s%s: snd(%d)=(%s)\n", curWorkSocketIndex, hour, min, sec, sendStream.length, new String(sendStream)) + Global.FC_RESET); // OFT
					
					// �ʱ�ȭ �� ���� ���� ����
					if (socketList.size() != 0) curWorkSocketIndex = (++curWorkSocketIndex) % socketList.size();
					
					sendStream = null;
				}
				
				return sendStream;
			}
			
			// �������� �������� ���� �����͸� ���۴�� ����Ʈ�� ������Ͽ� ������ �õ�
			private void resendWork() {
				LinkedList<Record> recvWaitingList = fileWriteLeftRecordListSyncWork("get", null);
				int recvWaitingCnt = recvWaitingList.size();
				
				for (Record record : recvWaitingList) {
					long curTime = System.currentTimeMillis();
					long lastSendTime = record.getLastSendTime();
					long lastSendDelta = curTime - lastSendTime;
					int resendCnt = record.getSendCnt();
					
					if (lastSendDelta < DEFAULT_RESEND_DELAY) { // ������ ������ ���
						// ...
					}
					else if (lastSendDelta >= DEFAULT_RESEND_DELAY && resendCnt <= MAX_RESEND_COUNT) { // �������� DEFAULT_RESEND_DELAY�� ����, ������ MAX_RESEND_COUNT�� ����	
						if (sendWaittingRecordList.size() < recordTransceiver.getSendWaitingRecordListMaxSize()) { // ���ť�� ������ �ְ�
							if (fileWriteDoneRecordIndexList.indexOf(record.getIndex()) == -1) { // ���ſϷᰡ ���� ���� ���
								sendWaittingRecordList.add(record);	// ���۴�⿡ �߰�
								Logger.logln(Logger.LogType.LT_INFO, Global.FC_WHITE + record.getIndex() + "�� ���ڵ� ������ �õ�. (������ ���� �� ����ð�: " + lastSendDelta + ", ���� �õ� Ƚ��: " + resendCnt + ")" + Global.FC_RESET);
							}
							
							fileWriteLeftRecordListSyncWork("remove", record); // ���ſϷ� �ưų�, �������� ��� ���Ŵ�⿡�� ����
						}
						else {
							break;
						}
					}
					else {
						--recvWaitingCnt; // ���� �Ұ� ���ڵ� ������ŭ ����
					}
				}
			}
			
			// ���� �ӵ� ����
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
						setWorkDelayLeft(sendDelay); // ���� �ӵ� ���� �� ������ ������ ����
						
						// Old Version
						/*long sendDelay = recordTransceiver.getSendDelay();
						sendDelay = Math.max(sendDelay, Math.max(recvWaitingCnt, 1)); // '���� �����ð�' vs '�̼��� ������ ����' �� ū ����ŭ ���. (��, �ּ� 1ms��ŭ�� ���)
						recordTransceiver.setSendDelay(sendDelay);
						setWorkDelayLeft(sendDelay / 2); // ���� �ӵ� ���� �� ������ ������ ����
						System.out.println("sendDelay: " + sendDelay / 2);*/
			}
			
			// Record�� ����Ͽ� ���, Prepix, Suffix�� ���� ���ۿ� byte[]����
			private byte[] makeSendStream(Record sendRecord) {
				int streamIndex = 0;
				
				// Record ����Ʈ �迭
				final byte[] recordSteram = sendRecord.toByteAry();
				
				// ����� (0000 : 4byte)
				final int streamHeaderLength = 4; 
				
				// �����ͺ� (Prefix + ���ڵ� + Suffix)
				final int streamDataLength = sendPrefix.length + recordSteram.length + sendSuffix.length;
				
				// ������ ����Ʈ �迭 (��� + �����ͺ�(Prefix+Record+Suffix))
				byte[] sendStream = new byte[streamHeaderLength + streamDataLength];
				
				// Data Length (����� ���� ����)
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
			public static final int streamHeaderLength = 4;	// ����� ���� (0000:4byte)
			
			private byte[] savedByteAry;	// ������ �о���� ����Ʈ �����͸� ����ִ� �迭
			
			private long lastRecvTime;		// ���������� �����͸� ���� ������ �ð�
			
			private byte[] recvPrefix;
			private byte[] recvSuffix;
			
			private RecordConverter cliRecordConverter;		// �������� ���� ���ڵ带 Ŭ���̾�Ʈ �������� �����ϴ� Ŭ����
			private KsFileWriter ksFileWriter;				// .rpy ���Ͽ� ���� ����� ���� Ŭ����
			
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
				Socket recvSocket = null;	// ���� �۾��� ���� ����
				
				while (running) {
					// ������ �ð� ������Ʈ
					if (!updateThreadTime()) continue;

					try {
						// [���ź�]
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

						if (inputDataLength > 0) { // ������ ����Ʈ�� ����
							recvWork(recvSocket, inputDataLength);
						}
						
						if (savedByteAry != null && savedByteAry.length > 0) {
							// [���ܺ�]
							byte[] recordByteAry = cuttingWork(inputDataLength);

							if (recordByteAry == null) continue;
							
							// [������]
							cvtAndWriteWork(recordByteAry);
							
							// ������ �۾��� ���Ϲ�ȣ ����
							if (socketList.size() != 0)	curWorkSocketIndex = (++curWorkSocketIndex) % socketList.size();
						}
					}			
					catch (Exception e) {
						Logger.logln(Logger.LogType.LT_ERR, e);
						
						if (e instanceof IOException) {
							Logger.logln(Logger.LogType.LT_ERR, Global.FC_RED + "InputStream�� ���� �����Դϴ�. (SocketIndex: " + curWorkSocketIndex + ", fileWriteLeftRecordList.size(): " + fileWriteLeftRecordList.size() + ")" + Global.FC_RESET);
							setWorkDelayLeft(1000); // �ּ� 1�ʰ� �۾� ���
						}
					}
					
					setWorkDelayLeft(Math.max(getWorkDelayLeft(), 1)); // ������ ������ ����
				}
				
				// ���� ������ �����͸� ���Ͽ� ���
				writeRecvFailedRecord();
				
				Logger.logln(Logger.LogType.LT_INFO, Global.FC_RED + "RecvSocketThread ����." + Global.FC_RESET);
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
			
			// ���ź�
			private void recvWork(Socket recvSocket, int inputDataLength) throws IOException {
				Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "[���� ó�� ����: " + fileWriteLeftRecordList.size() + "]" + Global.FC_RESET); // OFT
				Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "1-1.available(): " + inputDataLength + Global.FC_RESET); // OFT
				
				InputStream is = recvSocket.getInputStream();
				byte[] readByteAry = new byte[inputDataLength];
				
				if (is.read(readByteAry, 0, inputDataLength) != -1) {
					Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "1-2.readByteLen " + inputDataLength + Global.FC_RESET); // OFT
					
					// ���������� ������ �ð�, Ÿ�Ӿƿ� ����
					lastRecvTime = System.currentTimeMillis();
					threadTimeoutLeft = DEFAULT_THREAD_TIMEOUT;

					// �ð� ���� �� �α�
					SimpleDateFormat dateTime = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
					String today = dateTime.format(new Date(System.currentTimeMillis()));
					String hour = today.substring(11, 13),	min = today.substring(14, 16),	sec = today.substring(17, 19);
					fileLogger.log(String.format("%s%s%s: rcv(%04d)=(", hour, min, sec, readByteAry.length), new String(readByteAry), ")");

					// �ӽ������ Byte�� ����
					this.savedByteAry = makeAppendedByteAry(savedByteAry, readByteAry);
					Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "1-3.savedByteAryLen: " + savedByteAry.length + Global.FC_RESET); // OFT
					
					// ��ȸ�� ���� ����� ��� ���� �ı�
					if (!recordTransceiver.isReusableSocketMode()) {
						recvSocket.close();
						socketList.remove(recvSocket);
					}
				}
			}
			
			// ���ܺ�
			private byte[] cuttingWork(int inputDataLength) {
				// Prefix:STX(0x02) ã��
				int recvPrefixIdx = findFromAry(savedByteAry, recvPrefix);
				
				if (inputDataLength == 0) {
					Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "[���� ó�� ����: " +
								 fileWriteLeftRecordList.size() + ", �Ϸ��� ����: " + fileWriteDoneRecordIndexList.size() + "]" + Global.FC_RESET); // OFT
				}
				
				// Prefix:STX�� ���� ���Ź��� ����
				if (recvPrefixIdx < 0) {
					Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2.failToFoundPrefix( + " + new String(recvPrefix) + ")"); // OFT
					return null; // ���ź� �����
				}
				
				Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2-1.prefixIdx: " + recvPrefixIdx + Global.FC_RESET); // OFT
				
				// Suffix:ETX(0x03) ã��
				int recvSuffixIdx = findFromAry(savedByteAry, recvSuffix);
				
				// Suffix:ETX�� ���� ���Ź��� ����
				if (recvSuffixIdx < 0) {
					Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2.failToFoundSuffix( + " + new String(recvSuffix) + ")"); // OFT
					return null; // ���ź� �����
				}
				else {
					recvSuffixIdx += recvSuffix.length; // Suffix:ETX �����ͱ��� �����ϱ� ���� ����
				}
				
				Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2-2.suffixIdx: " + recvSuffixIdx + Global.FC_RESET); // OFT
				
				// STX�� ETX�� ��� ���Ź��� ��� (�ǹ��ִ� �����Ͱ� �ϼ��� ���)
				// ���ڵ� ������ ���� �κ� ����Ʈ �迭 ���� (STX/ETX�� ���Եǰ�, ��� 4byte�� ���Ե��� �ʴ� �迭)
				byte[] recordByteAry = Arrays.copyOfRange(savedByteAry, recvPrefixIdx, recvSuffixIdx);
				Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2-3.recordByteAryLen: " + recordByteAry.length + "/302" + Global.FC_RESET); // OFT
				
				// ����� �迭���� �ش� ��� + ������ ����
				this.savedByteAry = makeRemovedByteAryByIndex(savedByteAry, recvPrefixIdx - streamHeaderLength, recvSuffixIdx);
				
				if (savedByteAry != null) Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2-4.afterRemoveFromSavedByteAry: " + savedByteAry.length + Global.FC_RESET); // OFT
				else Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2-4.afterRemoveFromSavedByteAry: 0" + Global.FC_RESET); // OFT
				
				return recordByteAry;
			}
			
			// ������
			private void cvtAndWriteWork(byte[] recordByteAry) {
				// ���� �����͸� ���� ���ڵ�� ����
				Record recvSvrRecord = makeRecord("HanaRecordServer", -1, recvPrefix, recvSuffix, recordByteAry);
				Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2-5.recvSvrRecordIndex: " + recvSvrRecord.getIndex() + Global.FC_RESET); // OFT
				
				int recvSvrRecordIndex = recvSvrRecord.getIndex();
				
				if (fileWriteDoneRecordIndexList.indexOf(recvSvrRecordIndex) == -1) { // ���������� ���� �ε��� ��ȣ�� ���� ���ڵ�
					// �ش� ���ڵ� �ε��� ��ȣ�� �߰�
					fileWriteDoneRecordIndexList.add(recvSvrRecordIndex);
					
					// ������ ���� ���ڵ带 Ŭ���̾�Ʈ ���ڵ�� ����
					cliRecordConverter.setOutRecordSubTypeName("Data");
					Record recvCliRecord = cliRecordConverter.convert(recvSvrRecord);
					
					Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "2-6.cvtCliRecord (msgNum: " + recvCliRecord.getIndex() + ")" + Global.FC_RESET); // OFT
					
					// ���ڵ� ���Ͽ� ���� ����
					ksFileWriter.write(recvCliRecord.toByteAry(), recvSvrRecordIndex);
				}
				
				// ���� �ۼ���� ����Ʈ���� �ش� �ε����� ���ڵ� ����
				fileWriteLeftRecordListSyncWork("remove", recvSvrRecord);
				
				LinkedList<Record> fileWriteLeftRecordListCpy = fileWriteLeftRecordListSyncWork("get", null);
				
				if (fileWriteLeftRecordListCpy.size() > 0) {
					Logger.logln(Logger.LogType.LT_DEBUG, Global.FC_RED + "���� ������� ���ڵ� �ε���: " + Global.FC_RESET);
					
					for (Record writeLeftRecord : fileWriteLeftRecordListCpy) {
						Logger.log(Logger.LogType.LT_DEBUG, Global.FC_RED + writeLeftRecord.getIndex() + ", " + Global.FC_RESET);
					}
					
					Logger.ln(Logger.LogType.LT_DEBUG); Logger.ln(Logger.LogType.LT_DEBUG);
				}					
			}
			
			// ���� ������ �����͸� ���Ͽ� ���
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
				if (logTypeAry == null || logTypeAry.length == 0) { // �ϰ�����
					for (int i = 0; i < visibleLogTypeAry.length; ++i) {
						visibleLogTypeAry[i] = isVisible;
					}
				}
				else { // ��������
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

			// ������
			public TransceiveLogger(String logFilePath) {
				this.logFilePath = logFilePath;
				logBuffer = new StringBuffer();
			}
			
			// �α� ���
			public synchronized void log(String head, String body, String tail) {
				logBuffer.append(head).append(body).append(tail).append("\r\n");
			}

			// ���Ϸ� ���
			public synchronized void writeToFile() {
				if (logBuffer.length() > 0) {
					FileOutputStream fos = null;
					
					try {
						fos = new FileOutputStream(logFilePath, true);
						fos.write(String.valueOf(logBuffer).getBytes());
					}
					catch (IOException ioe1) {
						Logger.logln(Logger.LogType.LT_ERR, "OutputStream ���� Ȥ�� ���� ����.");
						ioe1.printStackTrace();
					}
					finally {
						try {
							if (fos != null) fos.close();
							fos = null;
						}
						catch (IOException ioe2) {
							Logger.logln(Logger.LogType.LT_ERR, "OutputStream �ݱ� ����.");
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