package com.rblee;

import java.util.Arrays;
import java.util.Set;
import java.util.Iterator;
import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.LinkedList;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.InetAddress;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;

public class IpgRelayServer {
    
    // 클래스 상수 //
    public static int MSG_HEADER_SIZE = 4;          // 전문 헤더 길이
    public static int SERVER_RW_BUFFE_SIZE = 2048;  // 서버 읽기&쓰기 버퍼 크기
    public static int SERVER_PORT = 9595;           // 서버 포트
    public static long CLIENT_TIMEOUT = 10000;      // 클라이언트 타임아웃: 10초

    // 클래스 내부 상수 //
    private static final Logger _logger = Logger.getInstance();

    // 맴버 변수 //
    private String svrName;                 // 서버 이름
    private int svrStateCode;               // 서버 상태코드 (0:초기화, 1:생성완료, 2:기동중, 3:종료중)
    private long svrOpenTime;               // 서버 구동시점 시간
    private Selector selector;              // 서버 소켓 셀렉터
    private ServerSocketChannel svrSoc;     // 서버 소켓 채널
    private List<Client> cliList;           // 클라이언트 리스트

    // 클라이언트 정보 클래스 //
    static class Client {

        private SocketChannel cliSoc;   // 클라이언트 소켓 채널
        private long connectTime;       // 연결한 시점의 시간
        private long timeoutTime;       // 타임아웃될 시점의 시간
        private ByteBuffer recvBuffer;  // 수신받은 바이트를 저장할 버퍼

        public Client(SocketChannel cliSoc) {
            this.cliSoc = cliSoc;
            this.connectTime = System.currentTimeMillis();
            this.timeoutTime = this.connectTime + CLIENT_TIMEOUT;
            this.recvBuffer = ByteBuffer.allocate(SERVER_RW_BUFFE_SIZE);
        }

        // 타임아웃 체크
        public boolean isTimeouted(long curTime) {
            return (timeoutTime - curTime <= 0);
        }

        // 타임아웃 갱신
        public void renewTimeout(long curTime) {
            this.timeoutTime = curTime + CLIENT_TIMEOUT;
        }

        public SocketChannel getSoc() {
            return this.cliSoc;
        }

        public SelectionKey getSelKey(Selector svrSelector) {
            return this.cliSoc.keyFor(svrSelector);
        }

        public ByteBuffer getRecvBuf() {
            return this.recvBuffer;
        }

        public InetAddress getSocAddr() {
            InetAddress socAddr = null;

            try {
                socAddr = this.cliSoc.socket().getInetAddress();
            }
            catch (Exception exception) {
                _logger.log(Logger.ERROR, String.format("클라이언트 IP주소 반환 실패! (예외 메시지: %s)", exception));
            }

            return socAddr;
        }

        public boolean addRecvByte(ByteBuffer recvBuffer) {
            int recvLen = recvBuffer.position();
            int remainLen = this.recvBuffer.remaining();

            if (remainLen - recvLen < 0) { // 버퍼 공간 부족으로 데이터를 버림
                return false;
            }
            
            this.recvBuffer.put(recvBuffer.array(), 0, recvBuffer.capacity() - recvBuffer.remaining());

            return true;
        }
    }

    // 맴버 메서드 //
    public IpgRelayServer(String svrName) {
        this.svrName = svrName;
        this.svrStateCode = 0;
        this.svrOpenTime = System.currentTimeMillis();
    }

    // 서버 생성
    public boolean openServer() {
        _logger.log(Logger.INFO, String.format("[%s] 서버 생성 시작.", this.svrName));

        // 재생성 검사
        if (selector != null || svrSoc != null || cliList != null) {
            _logger.log(Logger.ERROR, String.format("[%s] 서버가 이미 생성된 적이 있습니다. 종료 후 다시 생성하십시오.", this.svrName));
            return false;
        }

        boolean rtResult = false;

        // 소켓 생성 등 맴버 변수 초기화
        try {        
            svrSoc = ServerSocketChannel.open();
            svrSoc.configureBlocking(false);
            svrSoc.socket().bind(new InetSocketAddress(SERVER_PORT));

            this.selector = Selector.open();
            svrSoc.register(this.selector, SelectionKey.OP_ACCEPT);

            cliList = new LinkedList<Client>();
            rtResult = true;
        }
        catch (Exception exception) {
            _logger.log(Logger.WARNING, String.format("[%s] 서버 생성중 오류 발생. (예외 메시지:%s)",
                                                     this.svrName, exception.getMessage()));
            rtResult = false;
        }
            
        // 결과 출력
        if (rtResult) {
            _logger.log(Logger.INFO, String.format("[%s] 서버 생성 성공.", this.svrName));
            this.svrStateCode = 1;
        }
        else {
            _logger.log(Logger.INFO, String.format("[%s] 서버 생성 실패.", this.svrName));
        }

        return true;
    }

    // 서버 종료
    public void closeServer() {
        _logger.log(Logger.INFO, String.format("[%s] 서버 종료 시작.", this.svrName));

        this.svrStateCode = 3;

        // 모든 클라이언트 연결 해제
        Set<SelectionKey> selKeySet = this.selector.keys();
        Iterator<SelectionKey> selKeyIter = selKeySet.iterator();
        SelectionKey selKey = null;

        while (selKeyIter.hasNext()) {
            selKey = selKeyIter.next();

            if (selKey.channel() instanceof SocketChannel) {
                closeClient(selKey);
            }
        }

        // 서버 자원 해제
        try {
            this.svrSoc.close();
            this.selector.close();
        }
        catch (Exception exception) {
            _logger.log(Logger.WARNING, String.format("[%s] 서버 종료중 오류 발생. 자원 누수 위험! (예외 메시지: %s)",
                                                  this.svrName, exception.getMessage()));
        }

        this.svrStateCode = 0;
        _logger.log(Logger.INFO, String.format("[%s] 서버 종료 완료.", this.svrName));
    }

    // 서버 가동
    public boolean runServer() {
        _logger.log(Logger.INFO, String.format("[%s] 서버 가동 시작.", this.svrName));

        if (this.svrStateCode != 1) {
            _logger.log(Logger.ERROR, String.format("[%s] 서버를 가동할 수 없음. 서버가 '생성완료(1)' 상태가 아닙니다.", this.svrName));
            return false;
        }
        
        this.svrStateCode = 2;
        long lastIdleTime = 0;              // 마지막으로 Idle이 수행된 시간
        long nextTimeoutUpdateTime = 0;     // 다음 Timeout 업데이트 시간
        long nextLifeLoggingTime = 0;       // 다음 생존 로그 출력한 시간
        
        while (true) {
            if (this.svrStateCode != 2) {
                break;
            }

            try {
                // 셀렉터 수행
                if (this.selector.select(1) == 0) {
                    // 처리할 소켓이 없는 경우 Idle로직 수행
                    lastIdleTime = System.currentTimeMillis();

                    // 클라이언트 타임아웃 감지 (1초에 한 번)
                    if (nextTimeoutUpdateTime <= lastIdleTime) {
                        updateClientsTimeout();
                        nextTimeoutUpdateTime = System.currentTimeMillis() + 1000; // 클라이언트 타임아웃 처리가 오래 걸릴 수 있으므로 현재 시간을 매번 다시 구함
                    }

                    // 생존 로깅 (60초에 한 번)
                    if (nextLifeLoggingTime <= lastIdleTime) {
                        _logger.log(Logger.INFO, String.format("[%s] 서버 가동중... (현재 클라이언트:%d / 경과 시간:%dms)",
                                                              this.svrName, this.cliList.size(), lastIdleTime - this.svrOpenTime));
                        nextLifeLoggingTime = lastIdleTime + 60000;
                    }

                    continue;
                }

                // 셀렉터로 받아온 SelectorKey들 수행
                Set<SelectionKey> selKeySet = this.selector.selectedKeys();
                Iterator<SelectionKey> selKeyIter = selKeySet.iterator();

                while (selKeyIter.hasNext()) {
                    SelectionKey selKey = selKeyIter.next();

                    selKeyIter.remove();
                    
                    if (!selKey.isValid()) { // 키 유효성 검사
                        _logger.log(Logger.WARNING, String.format("[%s] InValid SelectionKey! (%s)", this.svrName, selKey.toString()));
                        continue;
                    }
                    
                    if (selKey.isAcceptable()) { // 클라이언트 접속
                        acceptClient(selKey);
                        continue;
                    }
                    else if (selKey.isReadable()) { // 클라이언트 데이터수신 및 업데이트
                        updateClient(readFromClient(selKey));
                        closeClient(selKey); // 성공실패 상관없이 업데이트 후 연결 종료 (1회성 연결)
                        continue;
                    }
                    else if (selKey.isWritable()) {
                        // ...
                        continue;
                    }
                }
            }
            catch (Exception exception) {
                _logger.log(Logger.ERROR, String.format("[%s] 서버 가동중 오류 발생. (예외 메시지: %s)",
                                                       this.svrName, exception.getMessage()));
            }
        }

        _logger.log(Logger.INFO, String.format("[%s] 서버 가동 종료.", this.svrName));

        return true;
    }

    // 클라이언트 리스트에서 클라이언트 반환
    public Client getClientFromList(SelectionKey selKey) {
        Client cli = null;
        int szCliList = this.cliList.size();
        SocketChannel cliSoc = (SocketChannel)selKey.channel();

        for (int i = 0; i < szCliList; ++i) {
            cli = this.cliList.get(i);

            if (cliSoc == cli.getSoc()) {
                return cli;
            }
        }

        return null;
    }

    // 클라이언트 연결
    public void acceptClient(SelectionKey selKey) {
        try {
            SocketChannel cliSoc = ((ServerSocketChannel)selKey.channel()).accept();
            cliSoc.configureBlocking(false);
            cliSoc.register(this.selector, SelectionKey.OP_READ);

            Client client = new Client(cliSoc);
            this.cliList.add(client);

            InetAddress cliAddr = client.getSocAddr();
            _logger.log(Logger.INFO, String.format("[%s] 클라이언트 '%s' Accept성공. (현재 클라이언트: %d)",
                                                  this.svrName, cliAddr.toString(), cliList.size()));
        }
        catch (Exception exception) {
            _logger.log(Logger.ERROR, String.format("[%s] 클라이언트 Accept 실패! (예외 메시지:%s)",
                                                   this.svrName, exception.getMessage()));
        }
    }

    // 클라이언트 연결 해제
    public void closeClient(SelectionKey selKey) {
        SocketChannel cliSoc = null;
        InetAddress cliAddr = null;

        try {
            Client cli = this.getClientFromList(selKey);

            cliSoc = cli.getSoc();
            cliAddr = cli.getSocAddr();

            // 클라이언트 리스트에서 제외
            if (cli != null) {
                this.cliList.remove(cli);
            }
            else {
                _logger.log(Logger.WARNING, String.format("[%s] Close된 클라이언트 '%s'가 cliList에 존재하지 않음! (연결된 클라이언트: %d)",
                                                         this.svrName, cliAddr.toString(), cliList.size()));
            }

            // 셀렉터 해제
            selKey.cancel();

            // 소켓 닫기
            cliSoc.close();
            _logger.log(Logger.INFO, String.format("[%s] 클라이언트 '%s' 소켓 닫기 성공. (연결된 클라이언트: %d)",
                                                  this.svrName, cliAddr.toString(), cliList.size()));
        }
        catch (Exception exception) {
            _logger.log(Logger.ERROR, String.format("[%s] 클라이언트 '%s' 소켓 닫기 실패! (연결된 클라이언트: %d)",
                                                   this.svrName, cliAddr.toString(), cliList.size()));
        }
    }

    // 클라이언트 요청 읽기
    public Client readFromClient(SelectionKey selKey) {
        boolean closeCliFlag = false;
        InetAddress cliAddr = null;
        Client cli = null;

        try {
            SocketChannel cliSoc = (SocketChannel)selKey.channel();
            ByteBuffer readBuffer = ByteBuffer.allocate(SERVER_RW_BUFFE_SIZE);
            int readLen = 0;

            cli = this.getClientFromList(selKey);

            if (cli == null) {
                // 클라이언트 리스트에서 찾을 수 없으면 오류로 간주
                throw new Exception("클라이언트를 클라이언트 리스트에서 찾을 수 없음!");
            }

            cliAddr = cli.getSocAddr();

            if ((readLen = cliSoc.read(readBuffer)) > 0) { // 소켓에서 데이터를 정상적으로 읽음                
                // 클라이언트의 수신버퍼에 추가
                if (!cli.addRecvByte(readBuffer)) {
                    _logger.log(Logger.WARNING, String.format("[%s] 데이터 손실 발생! 클라이언트 '%s'의 데이터 [%s]가 들어갈 클라이언트 수신 버퍼가 부족함.",
                                                             this.svrName, cliAddr.toString(), new String(readBuffer.array())));
                }
            }
            else { // 클라이언트 소켓이 닫혔거나 오류
                closeCliFlag = true;
            }
        
            _logger.log(Logger.INFO, String.format("[%s] Recv from client(%s) Msg:[%s]",
                                                  this.svrName, cliAddr.toString(), new String(readBuffer.array())));
        }
        catch (Exception exception) {
            exception.printStackTrace();
            closeCliFlag = true;
            _logger.log(Logger.ERROR, String.format("[%s] Recv from client(%s) 실패!. (예외 메시지:%s)",
                                                   this.svrName, cliAddr.toString(), exception.getMessage()));
        }
        finally {
            if (closeCliFlag) {
                closeClient(selKey);
            }
        }

        return cli;
    }

    // 클라이언트 요청 수행
    public boolean updateClient(Client cli) {
        InetAddress cliAddr = null;

        // 헤더와 바디를 분리하여 전문 생성
        ByteBuffer cliRecvBuf = cli.getRecvBuf();
        byte[] headBytes = new byte[MSG_HEADER_SIZE];
        byte[] bodyBytes = null;
        byte[] msgBytes = null; // head + body Bytes

        try {
            cliAddr = cli.getSocAddr();
            cliRecvBuf.rewind();
            cliRecvBuf.get(headBytes);
            
            int bodyLen = Integer.valueOf(new String(headBytes));
            bodyBytes = new byte[bodyLen];

            cliRecvBuf.get(bodyBytes);
            cliRecvBuf.compact();

            msgBytes = new byte[headBytes.length + bodyBytes.length];
            System.arraycopy(headBytes, 0, msgBytes, 0, headBytes.length);
            System.arraycopy(bodyBytes, 0, msgBytes, headBytes.length, bodyBytes.length);
        }
        catch (Exception exception) {
            exception.printStackTrace();
            _logger.log(Logger.ERROR, String.format("[%s] 클라이언트(%s) 전문 생성 실패! (예외 메시지:%s)",
                                                   this.svrName, cliAddr.toString(), exception.getMessage()));
            return false;
        }
        
        // 생성된 전문 분류 및 서비스 수행
        byte tb1 = msgBytes[72], tb2 = msgBytes[73];
        IpgMsg cliRecvMsg = null;
        IpgMsg cliSendMsg = null;
        boolean serviceResult = false;

        if (tb1 == 'N' && tb2 == 'I') { // 사용통보 전문
            cliRecvMsg = new IpgMsg(msgBytes, IpgMsg.ReqPayNotifyAttr, null, null);
            cliSendMsg = new IpgMsg(null,     IpgMsg.RpyPayNotifyAttr, null, null);
            serviceResult = IpgService.payNotifyService(cli, cliRecvMsg, cliSendMsg);
        }
        else if (tb1 == 'M' && tb2 == 'I') { // 가맹점 정보조회 전문
            cliRecvMsg = new IpgMsg(msgBytes, IpgMsg.ReqShopInfoMsgAttr, null, null);
            cliSendMsg = new IpgMsg(null,     IpgMsg.RpyShopInfoMsgAttr, null, null);
            serviceResult = IpgService.shopInfoService(cli, cliRecvMsg, cliSendMsg);
        }
        else if (false) {
            // ... 새로운 서비스 추가
        }
        else { // 오류 전문
            _logger.log(Logger.ERROR, String.format("[%s] 클라이언트(%s) 전문 오류! (알 수 없는 전문 분류: '%c%c')",
                                                   this.svrName, cliAddr.toString(), tb1, tb2));
            return false;
        }

        // 클라이언트에게 결과 응답
        if (cliRecvMsg != null) {
            this.writeToClient(cli.getSoc(), cliSendMsg.getMsgStream());
        }

        return serviceResult;
    }

    // 클라이언트 요청 응답
    public void writeToClient(SocketChannel cliSoc, byte[] writeBytes) {
        InetAddress cliAddr = null;

        try {
            cliAddr = cliSoc.socket().getInetAddress();

            ByteBuffer writeBuf = ByteBuffer.wrap(writeBytes);
            int writeRemainLen = writeBytes.length;

            while (writeRemainLen > 0) {
                writeRemainLen -= cliSoc.write(writeBuf);
            }

            _logger.log(Logger.INFO, String.format("[%s] Send to client(%s). MSG:[%s]",
                                                  this.svrName, cliAddr.toString(), new String(writeBytes)));
        }
        catch (Exception exception) {
            exception.printStackTrace();
            _logger.log(Logger.ERROR, String.format("[%s] Send to client(%s) 실패! (예외 메시지:%s)",
                                                   this.svrName, cliAddr.toString(), exception.getMessage()));
        }
    }

    // 클라이언트 타임아웃 확인
    public void updateClientsTimeout() {
        long curTime = System.currentTimeMillis();

        for (Client cli : cliList) {
            if (cli.isTimeouted(curTime)) {
                closeClient(cli.getSelKey(this.selector));
            }
        }
    }

    public static void main(String[] args) {
        // 서버 생성
        IpgRelayServer svr = new IpgRelayServer("TestIpgSvr");

        if (!svr.openServer()) {
            _logger.log(Logger.ERROR, "서버 Open 실패. 다시 시도하십시오.");
            return;
        }

        // 서버 가동
        if (!svr.runServer()) {
            _logger.log(Logger.ERROR, "서버 Run 실패. 다시 시도하십시오.");
            return;
        }

        // 서버 닫고 프로그램 종료
        svr.closeServer();
        _logger.log(Logger.INFO, "서버 프로세스를 종료합니다.");
    }
}