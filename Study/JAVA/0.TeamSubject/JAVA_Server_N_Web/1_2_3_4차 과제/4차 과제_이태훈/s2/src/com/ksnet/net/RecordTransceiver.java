package com.ksnet.net;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.channels.IllegalBlockingModeException;
import java.net.Socket;
import java.net.SocketAddress;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.List;
import java.util.LinkedList;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.text.SimpleDateFormat;

import com.ksnet.util.*;

public class RecordTransceiver {

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
		this.fileLogger = new TransceiveLogger("res/output/Client.out");
		
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
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

class SocketThread implements Runnable {
	
	public static long DEFAULT_THREAD_TIMEOUT;	// n초 동안 아무 반응이 없으면 스레드 타임아웃
	public static int MAX_RESEND_COUNT;			// 최대 n번 재전송 허용
	public static int DEFAULT_RESEND_DELAY;		// n초 이내 수신받지 못하면 재전송 시도
	
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
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////	

class SendSocketThread extends SocketThread {
	
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
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

class RecvSocketThread extends SocketThread {
	
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
