package com.ksnet.main.client;

import java.io.File;
import java.util.Scanner;
import java.util.List;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Collections;

import com.ksnet.net.*;
import com.ksnet.util.*;

public class ClientMain {	
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