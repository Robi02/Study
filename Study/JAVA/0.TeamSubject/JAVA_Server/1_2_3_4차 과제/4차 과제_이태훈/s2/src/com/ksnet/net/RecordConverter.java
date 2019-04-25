package com.ksnet.net;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Date;
import java.text.SimpleDateFormat;

import com.ksnet.util.*;

public class RecordConverter {

	public static final byte[] NEW_LINE = { '\r', '\n' };

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