package com.ksnet.main.server;

import java.util.Arrays;
import java.util.Set;
import java.util.Iterator;
import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.LinkedList;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;

import com.ksnet.util.*;

public class RelayServerMain {

	public static HashMap<String, String> envHash;	// 환경해시
	public static RelayServer relayServer;			// 중개서버 객체

	// 메인
	public static void main(String[] args) {
		try {
			init(args);
			run();
		}
		catch (Exception e) {
			Logger.logln(Logger.LogType.LT_CRIT, e);
			Logger.logln(Logger.LogType.LT_CRIT, "오류 발생. 2분후 종료됩니다.");
		}
		finally {
			try { Thread.sleep(120000); } catch (Exception e) {}
		}
	}
		
	// 프로그램 초기화
	private static void init(String[] args) {		
		// 환경 해시 초기화
		envHash = new HashMap<String, String>();
		envHash.put("MY_PORT", args[0]);
		envHash.put("DEST_IP", args[1]);
		envHash.put("DEST_PORT", args[2]);
		envHash.put("DEST_SOCKET_CNT", args[3]);
		envHash.put("LOG_LEVEL", args[4]);
		
		// 로거 초기화
		String logLevel = new String(getEnv("LOG_LEVEL"));
		Logger.setVisibleByLogLevel(logLevel);
		
		// 서버 생성
		relayServer = new RelayServer();
	}
	
	// 서버 가동
	private static void run() {
		if (relayServer != null && relayServer.checkRunnable()) {
			relayServer.run();
		}
	}
	
	// 키로 환경값 가져오기
	public static String getEnv(String key) {
		String envVal = null;
		
		try {
			envVal = envHash.get(key);
		}
		catch (Exception e) {
			Logger.logln(Logger.LogType.LT_ERR, e);
		}
		finally {
			return envVal;
		}
	}
}

class RelayServer {
	
	private Selector selector;	// 중개서버 Selector
	
	private ServerSocketChannel serverSocketChannel;		// 중개서버 ServerSocketChannel
	private List<SocketChannel> clientList;					// 중개서버와 연결된 클라이언트 SocketChannel
	private long clientTimeout;								// 중개서버와 클라이언트와 타임아웃 시간
	private Map<SelectionKey, Long> clientLastReadTimeMap;	// 중개서버와 연결된 클라이언트로부터의 마지막 수신시간을 저장
	
	private String bankSvrIp;				// 은행서버 Ip
	private int bankSvrPort;				// 은행서버 Port
	private List<SocketChannel> serverList;	// 중개서버와 연결된 은행서버 SocketChannel
	private int curChannelIndex;			// 은행서버로 전송하기위한 현재 채널 인덱스;
	
	private Map<Integer, SocketChannel> cliSvrMapping;	// 클라이언트와 은행서버를 매핑하는 맵
	
	private TransceiveLogger transceiveLogger;	// 채널 수신발신에 사용되는 로거
	
	// 생성자
	public RelayServer() {
		initServer();
	}
	
	// 서버 구동가능여부 점검
	public boolean checkRunnable() {
		if (selector == null || !selector.isOpen()) return false;
		if (serverSocketChannel == null || !serverSocketChannel.isOpen()) return false;
		if (clientList == null) return false;
		if (serverList == null && serverList.size() == 0) return false;
		
		return true;
	}
	
	// 서버 초기화
	public void initServer() {
		Logger.logln(Logger.LogType.LT_DEBUG, "서버 초기화 시작.");
		
		try {
			selector = Selector.open();
			serverSocketChannel = ServerSocketChannel.open();
			serverSocketChannel.configureBlocking(false);
			serverSocketChannel.bind(new InetSocketAddress(Integer.parseInt(RelayServerMain.getEnv("MY_PORT"))));
			serverSocketChannel.register(selector, SelectionKey.OP_ACCEPT);
			
			clientList = new LinkedList<SocketChannel>();
			serverList = new LinkedList<SocketChannel>();
			
			clientTimeout = 60000; // 클라이언트 최대 1분간 연결 유지
			clientLastReadTimeMap = new HashMap<SelectionKey, Long>();
			
			bankSvrIp = RelayServerMain.getEnv("DEST_IP");
			bankSvrPort = Integer.parseInt(RelayServerMain.getEnv("DEST_PORT"));
			
			connectSocketChannel(bankSvrIp, bankSvrPort);
			
			curChannelIndex = 0;
			cliSvrMapping = new HashMap<Integer, SocketChannel>();
			
			transceiveLogger = new TransceiveLogger("res/output/RelayServer.out");
			
			Logger.logln(Logger.LogType.LT_DEBUG, "서버 초기화 완료.");
		}
		catch (Exception e) {
			Logger.logln(Logger.LogType.LT_DEBUG, "서버 초기화 실패.");
			Logger.logln(Logger.LogType.LT_ERR, e);
			closeServer();
		}
	}
	
	// 서버 닫기
	public void closeServer() {
		Logger.logln(Logger.LogType.LT_INFO, "서버 닫기 시작.");
		
		try {
			if (clientList != null && clientList.size() > 0) {
				disconnectClientAll();
			}
			
			
			if (serverSocketChannel != null && serverSocketChannel.isOpen()) {
				serverSocketChannel.close();
				serverSocketChannel = null;
			}
			
			if (selector != null && selector.isOpen()) {
				selector.close();
				selector = null;
			}
			
			if (transceiveLogger != null) {
				transceiveLogger.writeToFile();
			}
			
			Logger.logln(Logger.LogType.LT_INFO, "서버 닫기 완료.");
		}
		catch (Exception e) {
			Logger.logln(Logger.LogType.LT_INFO, "서버 닫기 실패.");
			Logger.logln(Logger.LogType.LT_ERR, e);
		}
	}
	
	// 일정 시간동안(Timeout) 송수신이 없는 SocketChannel 닫고, 클라-은행서버 매핑 초기화 관리
	public void closeTimeoutedSocketChannelAll() {
		Set<SelectionKey> selectorKeys = selector.keys();
		
		for (SelectionKey key : selectorKeys) {
			try {
				Long lastReadTime = null;
				
				if ((lastReadTime = clientLastReadTimeMap.get(key)) == null) {
					continue; // 클라이언트 소켓채널이 아닌 경우 다음 키 검사
				}
				
				long curTime = System.currentTimeMillis();
				long elapsedTime = curTime - lastReadTime;
				
				if (elapsedTime > clientTimeout) {
					Logger.logln(Logger.LogType.LT_DEBUG, ("타임아웃으로 클라이언트(" + getSocketChannelAddr((SocketChannel)key.channel()) + ")와의 연결을 종료합니다. (elapsedTime: " + elapsedTime + ", 남은 연결: " + (clientList.size() - 1) + ")"));
					closeSocketChannel(key);
				}
			}
			catch (Exception e) {
				Logger.logln(Logger.LogType.LT_ERR, e);
			}
		}
		
		if (clientList.size() == 0 && cliSvrMapping.size() > 0) {
			cliSvrMapping.clear();
			Logger.logln(Logger.LogType.LT_INFO, ("연결된 클라이언트가 존재하지 않아 클라-은행서버 매핑을 초기화합니다."));
		}
	}
	
	// 서버 구동
	public void run() {
		long stateLogTermMs = 5000;
		long lastStateLogTime = 0;
		
		Logger.logln(Logger.LogType.LT_DEBUG, "서버 구동 시작.");

		while (true) {
			try {
				if (selector == null) break;
				
				// 최소 1ms동안 select결과가 있을때까지 블로킹 (클라이언트가 무수히 많이 연결되면 if안으로 도달하지 않을 수 있음에 유의)
				if (selector.select(1) == 0) {
					closeTimeoutedSocketChannelAll();
					
					long curTime = System.currentTimeMillis();
					
					if (curTime - lastStateLogTime > stateLogTermMs) {
						Logger.logln(Logger.LogType.LT_INFO, "[" + curTime + "] 연결된 클라이언트: " + clientList.size() + ", 연결된 서버: " + serverList.size() + ", 클라/서버 매핑: " + cliSvrMapping.size() + ", 클라/타임아웃 매핑: " + clientLastReadTimeMap.size());
						lastStateLogTime = System.currentTimeMillis();
					}
					
					transceiveLogger.writeToFile();
					continue;
				}
				
				// selector로 받은 key들에 해당하는 기능 수행
				Set<SelectionKey> selectedKeySet = selector.selectedKeys();
				Iterator<SelectionKey> iter = selectedKeySet.iterator();
				
				while (iter.hasNext()) {
					SelectionKey selectionKey = iter.next();
					
					iter.remove();
					
					if (!selectionKey.isValid()) {
						continue;
					}
					else if (selectionKey.isAcceptable()) {
						clientAccept(selectionKey);
					}
					else if (selectionKey.isReadable()) {
						read(selectionKey);
					}
					else if (selectionKey.isWritable()) {
						// write(selectionKey);
					}
				}
			}
			catch (Exception e) {
				Logger.logln(Logger.LogType.LT_ERR, "서버 구동중 오류 발생.");
				Logger.logln(Logger.LogType.LT_ERR, e);
			}
		}
		
		Logger.logln(Logger.LogType.LT_INFO, "서버 구동 정지.");
	}
	
	// 클라이언트 접속 승인
	private void clientAccept(SelectionKey selectionKey) {
		try {			
			ServerSocketChannel serverSocketChannel = (ServerSocketChannel)selectionKey.channel();
			SocketChannel socketChannel = serverSocketChannel.accept();
			
			socketChannel.configureBlocking(false);
			socketChannel.register(selector, SelectionKey.OP_READ);
			clientList.add(socketChannel);
		}
		catch (Exception e) {
			Logger.logln(Logger.LogType.LT_ERR, e);
		}
	}
	
	// 모든 클라이언트 연결 종료
	private void disconnectClientAll() {
		Logger.logln(Logger.LogType.LT_DEBUG, "모든 클라이언트(" + clientList.size() + ")의 연결 종료를 시작합니다.");
		
		Set<SelectionKey> keySet = selector.keys();
		Iterator<SelectionKey> iter = keySet.iterator();
		SelectionKey key = null;
		SocketChannel socket = null;
		
		while (iter.hasNext()) {
			key = iter.next();
			
			if (key.channel() instanceof SocketChannel) { // ServerSocketChannel 제외
				socket = (SocketChannel)key.channel();
				
				if (serverList.indexOf(socket) == -1) { // 클라이언트 SocketChannel인 경우 true
					closeSocketChannel(key);
				}
			}
		}
		
		Logger.logln(Logger.LogType.LT_DEBUG, "모든 클라이언트의 연결 종료를 완료했습니다.");
	}
	
	// 중개서버로 들어온 모든 입력 처리
	private void read(SelectionKey selectionKey) {
		SocketChannel socketChannel = (SocketChannel)selectionKey.channel();
		int svrSocketIndex = serverList.indexOf(socketChannel);
		
		if (svrSocketIndex == -1) {
			read_client(selectionKey);
		}
		else {
			read_bankServer(selectionKey);
		}
	}
	
	// 클라이언트 입력 처리
	private void read_client(SelectionKey selectionKey) {
		SocketChannel clientSocket = (SocketChannel)selectionKey.channel();
		byte[] readByte = null;
		
		if ((readByte = readSocketChannel(clientSocket, 306)) == null) {
			Logger.logln(Logger.LogType.LT_DEBUG, "클라이언트(" + getSocketChannelAddr(clientSocket) + ")가 연결을 종료했습니다.");
			closeSocketChannel(selectionKey);
			return;
		}
		
		// 클라이언트로부터 수신한 시간 저장
		clientLastReadTimeMap.put(selectionKey, System.currentTimeMillis());
		
		// 전문 번호를 키로 클라이언트 SocketChannel 저장
		Integer hashKey = Integer.parseInt(new String(Arrays.copyOfRange(readByte, 32, 38)));
		cliSvrMapping.put(hashKey, clientSocket);
		
		// 은행 서버로 전송
		Logger.logln(Logger.LogType.LT_DEBUG, "hashKey: " + hashKey + ", svrSocIdx: " + curChannelIndex);
		
		if (serverList.size() > 0) {
			SocketChannel bankServerSocket = serverList.get(updateCurChannelIndex());
			writeSocketChannel(bankServerSocket, readByte);
		}
		else {
			connectSocketChannel(bankSvrIp, bankSvrPort);
		}
	}
	
	// 은행서버 입력 처리
	private void read_bankServer(SelectionKey selectionKey) {
		SocketChannel bankServerSocket = (SocketChannel)selectionKey.channel();
		byte[] readByte = null;
		
		if ((readByte = readSocketChannel(bankServerSocket, 306)) == null) {
			Logger.logln(Logger.LogType.LT_INFO, "은행서버(" + getSocketChannelAddr(bankServerSocket) + ")가 연결을 종료시켰습니다.");
			
			if (clientList.size() > 0) {
				disconnectClientAll();
				reconnectSocketChannel(selectionKey);
			}
			else {
				closeSocketChannel(selectionKey);
			}
			
			return;
		}
		
		// 전문 번호를 키로 클라이언트 SocketChannel 불러옴
		Integer hashKey = Integer.parseInt(new String(Arrays.copyOfRange(readByte, 32, 38)));
		SocketChannel clientSocket = cliSvrMapping.remove(hashKey);
		
		if (clientSocket == null) {
			Logger.logln(Logger.LogType.LT_ERR, "clientSocket == null (hashKey: " + hashKey + ")");
			return;
		}
		
		// 클라이언트에게 전송
		writeSocketChannel(clientSocket, readByte);
	}
	
	// 소켓채널 연결
	private void connectSocketChannel(String destIp, int destPort) {
		int socketIndex = -1;
		
		try {
			String bankServerIp = RelayServerMain.getEnv("DEST_IP");
			int bankServerPort = Integer.parseInt(RelayServerMain.getEnv("DEST_PORT"));
			
			if (destIp.equals(bankServerIp) && destPort == bankServerPort) { // 은행서버 연결
				int socketCnt = Integer.parseInt(RelayServerMain.getEnv("DEST_SOCKET_CNT"));
				
				for (int i = serverList.size(); i < socketCnt; ++i) {
					socketIndex = serverList.size();
					
					Logger.logln(Logger.LogType.LT_INFO, ("중개서버 -> 은행서버(" + destIp + ":" + destPort + ") 연결 시도. (" + socketIndex + ")"));
					
					SocketChannel toSvrChannel = SocketChannel.open();
					
					toSvrChannel.connect(new InetSocketAddress(destIp, destPort));
					toSvrChannel.configureBlocking(false);
					toSvrChannel.register(selector, SelectionKey.OP_READ);
					serverList.add(toSvrChannel);
					curChannelIndex = serverList.size() - 1;
					
					Logger.logln(Logger.LogType.LT_INFO, ("중개서버 -> 은행서버(" + destIp + ":" + destPort + ") 연결 완료. (" + socketIndex + ")"));
				}
			}
			else {
				Logger.logln(Logger.LogType.LT_INFO, ("SocketChannel 연결 거부. 정의되지 않은 Ip:Port(" + destIp + ":" + destPort + ")"));
			}
		}
		catch (Exception e) {
			Logger.logln(Logger.LogType.LT_ERR, e);
			Logger.logln(Logger.LogType.LT_INFO, ("중개서버 -> 은행서버(" + destIp + ":" + destPort + ") 연결 실패. (" + socketIndex + ")"));
		}
	}
	
	// 소켓채널 닫기
	private void closeSocketChannel(SelectionKey selectionKey) {
		try {
			SocketChannel socketChannel = (SocketChannel)selectionKey.channel();
			
			if (selectionKey != null) selectionKey.cancel();
			
			if (socketChannel != null && socketChannel.isOpen()) {
				InetSocketAddress remoteAddr = getSocketChannelAddr(socketChannel);
				
				socketChannel.close();
				
				if (clientList.remove(socketChannel)) {
					clientLastReadTimeMap.remove(selectionKey);
					Logger.logln(Logger.LogType.LT_DEBUG, "클라이언트(" + remoteAddr + ")와의 연결 하나가 종료되었습니다. (남은 연결: " + clientList.size() + ")");
				}
				else if (serverList.remove(socketChannel)) {
					Logger.logln(Logger.LogType.LT_WARN, "은행서버(" + remoteAddr + ")와의 연결 하나가 종료되었습니다. (남은 연결: " + serverList.size() + ")");
				}
				else {
					Logger.logln(Logger.LogType.LT_WARN, "알 수 없는 연결(" + remoteAddr + ") 하나가 종료되었습니다.");
				}
			}
		}
		catch (IOException ioe) {
			Logger.logln(Logger.LogType.LT_ERR, ioe);
		}
	}
	
	// 소켓서버 재연결
	private void reconnectSocketChannel(SelectionKey selectionKey) {
		closeSocketChannel(selectionKey);
		connectSocketChannel(bankSvrIp, bankSvrPort);
	}
	
	// SocketChannel의 내용을 read하여 반환하는 함수
	private byte[] readSocketChannel(SocketChannel socketChannel, int bufferLength) {
		ByteBuffer readBuffer = ByteBuffer.allocate(bufferLength);
		byte[] readByte = null;
		int readLength = -1;
		
		try {
			if ((readLength = socketChannel.read(readBuffer)) == -1) {
				return null;
			}
		}
		catch (IOException ioe) {
			Logger.logln(Logger.LogType.LT_ERR, ioe);
			return null;
		}
		
		readByte = new byte[readLength];
		System.arraycopy(readBuffer.array(), 0, readByte, 0, readByte.length);
		
		String recvAddr = getSocketChannelAddr(socketChannel).toString();
		Logger.logln(Logger.LogType.LT_DEBUG, ("[RecvFrom:" + recvAddr + "](" + new String(readByte) + ")"));
		transceiveLogger.log(String.format("[%d]rcv(%s:%04d)=(", System.currentTimeMillis(), recvAddr, readByte.length), new String(readByte), ")");
		
		return readByte;
	}
	
	// SocketChannel의 목적지로 write하는 함수
	private boolean writeSocketChannel(SocketChannel socketChannel, byte[] writeByte) {
		ByteBuffer writeBuffer = ByteBuffer.allocate(writeByte.length);
		
		writeBuffer.put(writeByte);
		writeBuffer.clear();
		
		try {
			socketChannel.write(writeBuffer);
		}
		catch (IOException ioe) {
			Logger.logln(Logger.LogType.LT_ERR, ioe);
			return false;
		}
		
		String recvAddr = getSocketChannelAddr(socketChannel).toString();
		Logger.logln(Logger.LogType.LT_DEBUG, ("[SendTo:" + getSocketChannelAddr(socketChannel) + "](" + new String(writeByte) + ")"));
		transceiveLogger.log(String.format("[%d]snd(%s:%04d)=(", System.currentTimeMillis(), recvAddr, writeByte.length), new String(writeByte), ")");
		
		return true;
	}
	
	// 현재 서버채널 인덱스 반환 및, 채널 인덱스 증가
	private int updateCurChannelIndex() {
		int curChannelIndexCopy = 0;
		
		if (serverList == null || serverList.size() == 0) {
			curChannelIndex = 0;
		}
		else {
			curChannelIndexCopy = curChannelIndex;
			curChannelIndex = ++curChannelIndex % serverList.size();
		}
		
		return curChannelIndexCopy;
	}
	
	// 해당 소켓의 InetSocketAddress 반환
	private InetSocketAddress getSocketChannelAddr(SocketChannel socketChannel) {
		InetSocketAddress rtAddr = null;
		
		try {
			rtAddr = (InetSocketAddress)socketChannel.getRemoteAddress();
		}
		catch (IOException ioe) {
			Logger.logln(Logger.LogType.LT_ERR, ioe);
		}
		finally {
			return rtAddr;
		}
	}
}