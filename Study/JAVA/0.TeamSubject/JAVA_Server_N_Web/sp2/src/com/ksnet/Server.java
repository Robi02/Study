package com.ksnet;

import java.util.regex.Matcher;
import java.util.ArrayList;
import java.util.regex.Pattern;
import java.io.Reader;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.URL;
import java.net.HttpURLConnection;
import java.math.BigDecimal;
import java.util.concurrent.ConcurrentHashMap;
import java.util.Arrays;
import java.nio.ByteBuffer;
import java.nio.channels.SelectableChannel;
import java.nio.channels.SocketChannel;
import java.util.Iterator;
import java.nio.channels.SelectionKey;
import java.io.IOException;
import java.util.logging.Level;
import java.net.SocketAddress;
import java.net.InetSocketAddress;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.util.logging.Logger;

class Server {
    public static final int READ_WRITE_BUFFER_SIZE = 1024;
	
    private Logger logger;
    private ServerSocketChannel svrSocChannel;
    private Selector selector;
    
	// 서버 생성
    public static Server open() {
        final Server server = new Server();
        
		if (!server.init()) {
            return null;
        }
		
        return server;
    }
    
	// 서버 초기화
    public boolean init() {
        try {
            logger = Logger.getLogger(String.format("%s(%d)", Server.class.getName(), this.hashCode()));
			selector = Selector.open();
            svrSocChannel = ServerSocketChannel.open();
			svrSocChannel.configureBlocking(false);
            svrSocChannel.bind(new InetSocketAddress(Integer.parseInt(Global.getEnv(Global.SERVER_SOCKET_BIND_PORT))));
            svrSocChannel.register(selector, SelectionKey.OP_ACCEPT);
        }
        catch (Exception ex) {
            if (logger != null) {
                logger.log(Level.SEVERE, "Server 초기화 오류!", ex);
            }
            return false;
        }
        return true;
    }
    
	// 서버 닫기
    public void close() {
        try {
            if (svrSocChannel != null) {
                svrSocChannel.close();
                svrSocChannel = null;
            }
        }
        catch (Exception ex) {
            logger.log(Level.INFO, "Server svrSocChannel close 오류.", ex);
        }
		
        try {
            if (selector != null) {
                selector.close();
                selector = null;
            }
        }
        catch (Exception ex2) {
            logger.log(Level.INFO, "Server selector close 오류.", ex2);
        }
    }
    
	// 서버 구동
    public boolean run() {
        logger.log(Level.INFO, "Server 구동 시작.");
		
        while (selector != null) {
            try {
                if (selector.select(1) == 0) {
                    continue;
                }
            }
            catch (IOException ex) {
                logger.log(Level.WARNING, "run:selector.select(1) 오류.", ex);
            }
            
			Iterator<SelectionKey> iterator = selector.selectedKeys().iterator();
            SelectionKey selKey = null;
			
			while (iterator.hasNext()) {
                try {
                    selKey = iterator.next();
                    iterator.remove();
					
                    if (!selKey.isValid()) {
                        continue;
                    }
                    if (selKey.isAcceptable()) {
                        acceptClient(selKey);
                    }
                    else if (selKey.isReadable()) {
                        readFromClient(selKey);
                    }
                    else if (selKey.isWritable()) {}
                }
                catch (Exception ex2) {
                    logger.log(Level.WARNING, "run:selKeyWork 오류.", ex2);
                }
            }
        }
		
        logger.log(Level.INFO, "Server 구동 종료.");
        return true;
    }
    
	// 클라이언트 접속
    private void acceptClient(SelectionKey selKey) throws IOException {
        logger.log(Level.INFO, String.format("클라이언트 접속 시도. (%s)", selKey.getClass().getName()));
		
        SocketChannel cliSocChannel = svrSocChannel.accept();
        cliSocChannel.configureBlocking(false);
        cliSocChannel.register(selector, SelectionKey.OP_READ);
		
        logger.log(Level.INFO, "클라이언트 접속 수락.");
    }
    
	// 클라이언트 접속 해제
    private void disconnectClient(SelectionKey selKey) throws IOException {
        SelectableChannel selChannel = selKey.channel();
		
        if (selChannel instanceof SocketChannel) {
            SocketChannel cliChannel = (SocketChannel)selChannel;
			
            selKey.cancel();
            cliChannel.close();
			
            logger.log(Level.INFO, "클라이언트 연결 끊음.");
        }
    }
    
	// 클라이언트 패킷 수신
    private void readFromClient(SelectionKey selKey) throws IOException {
        SelectableChannel selChannel = selKey.channel();
		
        if (selChannel instanceof SocketChannel) {
            byte[] readByte = null;
			
            if ((readByte = readSocChannel((SocketChannel)selChannel, READ_WRITE_BUFFER_SIZE)) == null) {
                return;
            }
			
            translateMsg(selKey, readByte);
        }
    }
    
	// 클라이언트 패킷 전송
    private void writeToClient(SelectionKey selKey, byte[] writeByte) throws IOException {
        SelectableChannel selChannel = selKey.channel();
		
        if (selChannel instanceof SocketChannel) {
            writeSocChannel((SocketChannel)selChannel, writeByte.length, writeByte);
        }
    }
    
	// 소캣 채널 읽기
    private byte[] readSocChannel(SocketChannel socChannel, int readBufLen) throws IOException {
        ByteBuffer readBuf = ByteBuffer.allocate(readBufLen);
        int readLen = -1;
		
        if ((readLen = socChannel.read(readBuf)) == -1) {
            return null;
        }
		
        byte[] readByte = new byte[readLen];
        System.arraycopy(readBuf.array(), 0, readByte, 0, readByte.length);
        
		return readByte;
    }
    
	// 소캣 채널 쓰기
    private void writeSocChannel(final SocketChannel socChannel, int writeBufLen, byte[] writeByte) throws IOException {
        ByteBuffer writeBuf = ByteBuffer.allocate(writeBufLen);
        
		writeBuf.put(writeByte);
        writeBuf.clear();
        socChannel.write(writeBuf);
    }
    
	// msgId 번역 후 수행
    private void translateMsg(final SelectionKey selKey, byte[] readByte) {
        int sohIdx = -1, etxIdx = -1;
        boolean isMsgByte = false;
		
        for (int i = 0; i < readByte.length; ++i) {
            if (readByte[i] == (byte)0x01) {
                sohIdx = i;
                break;
            }
        }
		
        for (int j = sohIdx + 1; j < readByte.length; ++j) {
            if (readByte[j] == (byte)0x03) {
                etxIdx = j;
                break;
            }
        }
		
        if (sohIdx != -1 && etxIdx != -1 && sohIdx < etxIdx) {
            isMsgByte = true;
        }
		
        if (isMsgByte) {
			// [FORMAT  ] [(1)][msgId(5)][dataLen(5)][(1)][dataByte(dataLen)][(1)]
			// [EXAMPLE ] [msg01000100123456789]
			// [ANALYSIS] [SOH][msg01][00010][STX][0123456789][ETX]
			//            [0]  [1-5]  [6-10] [11] [12-21]     [22]  -> total 23 bytes
			//                 <----------(msgByte)---------->
			//                 <--(hdByte)--><---(dataByte)-->
			//                 <msgId><dtLen>
            byte[] msgByte = Arrays.copyOfRange(readByte, sohIdx + 1, etxIdx);
            byte[] hdByte = Arrays.copyOfRange(msgByte, 0, 10);
            String msgId = new String(Arrays.copyOfRange(hdByte, 0, 5));
            int dataLen = Integer.parseInt(new String(Arrays.copyOfRange(hdByte, 5, 10)));
            byte[] dataByte = null;
			
			if (dataLen > 0) {
                dataByte = Arrays.copyOfRange(readByte, 11, 11 + dataLen);
            }
			
            if (msgId.equals("ufcex")) {
                new Thread(() -> { msg_ufcex_updateFcex(selKey); }).start();
            }
            else if (msgId.equals("ufce2")) {
                msg_ufce2_updateFcex2(selKey);
            }
            else {
                logger.log(Level.INFO, String.format("알 수 없는 msgId (%s).", msgId));
            }
        }
    }
    
	// msg_ufcex 로직 수행
    private synchronized void msg_ufcex_updateFcex(final SelectionKey selKey) {
        final String[] array = { "USD", "JPY", "EUR", "CNY" };
        final ConcurrentHashMap<String, BigDecimal> concurrentHashMap = new ConcurrentHashMap<String, BigDecimal>();
        final StringBuilder sb = new StringBuilder();
        try {
            final HttpURLConnection httpURLConnection = (HttpURLConnection)new URL(Global.getEnv(Global.HANA_FCEXT_URL)).openConnection();
            httpURLConnection.setRequestMethod("POST");
            httpURLConnection.setDoInput(true);
            httpURLConnection.setDoOutput(true);
            httpURLConnection.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
            final BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(httpURLConnection.getInputStream(), "EUC-KR"), httpURLConnection.getContentLength());
            String line;
            while ((line = bufferedReader.readLine()) != null) {
                sb.append(line);
            }
            bufferedReader.close();
            String s = sb.toString();
            final Matcher matcher = Pattern.compile("(<tbody>)(.+?)(</tbody>)").matcher(s);
            if (matcher.find()) {
                s = matcher.group();
            }
            final Matcher matcher2 = Pattern.compile("(?<=(alt='' />))(.+?)(?=(</td>))").matcher(s);
            final ArrayList<String> list = new ArrayList<String>();
            while (matcher2.find()) {
                list.add(matcher2.group().replaceAll("[^0-9]", ""));
            }
            final Matcher matcher3 = Pattern.compile("(?<=(<td class='sell'>))(.+?)(?=(</td>))").matcher(s);
            int n = 0;
            while (matcher3.find()) {
                concurrentHashMap.put(array[n], new BigDecimal(matcher3.group()).divide(new BigDecimal((String)list.get(n)), 10, 1));
                ++n;
            }
            final byte[] obj2ByteAry = KsCommonLib.obj2ByteAry((Object)concurrentHashMap);
            if (selKey == null || !selKey.channel().isOpen()) {
                return;
            }
            final byte[] sendByte = KsCommonLib.makeSendByte("ufcex", KsCommonLib.byteAry2HexString(obj2ByteAry).getBytes("EUC-KR"));
            final SocketChannel socketChannel = (SocketChannel)selKey.channel();
            final ByteBuffer allocate = ByteBuffer.allocate(sendByte.length);
            allocate.put(sendByte);
            allocate.clear();
            Thread.sleep(5000);
            socketChannel.write(allocate);
            disconnectClient(selKey);
            logger.log(Level.INFO, String.format("전송:\n%s", new String(sendByte)));
        }
        catch (Exception ex) {
            logger.log(Level.WARNING, "msg_ufcex_updateFcex 오류.", ex);
        }
    }
    
	// msg_ufce2 로직 수행
    public void msg_ufce2_updateFcex2(final SelectionKey selKey) {
        final String[] CurrencyCodename = { "USD", "JPY", "EUR", "CNY" };
        final ConcurrentHashMap<String, Long> concurrentHashMap = new ConcurrentHashMap<String, Long>();
        final StringBuilder sb = new StringBuilder();

        try {
            final HttpURLConnection httpURLConnection = (HttpURLConnection)new URL(Global.getEnv(Global.HANA_FCEXT_URL)).openConnection();
            httpURLConnection.setRequestMethod("POST");
            httpURLConnection.setDoInput(true);
            httpURLConnection.setDoOutput(true);
            httpURLConnection.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");

            final BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(httpURLConnection.getInputStream(), "EUC-KR"), httpURLConnection.getContentLength());
            String line;

            while ((line = bufferedReader.readLine()) != null) {
                sb.append(line);
            }

            bufferedReader.close();
            String recvHtml = sb.toString();

            final Matcher matcher = Pattern.compile("(<tbody>)(.+?)(</tbody>)").matcher(recvHtml);

            if (matcher.find()) {
                recvHtml = matcher.group();
            }

            final Matcher matcher2 = Pattern.compile("(?<=(alt='' />))(.+?)(?=(</td>))").matcher(recvHtml);
            final ArrayList<String> unitList = new ArrayList<String>();

            while (matcher2.find()) {
                unitList.add(matcher2.group().replaceAll("[^0-9]", ""));
            }

            final Matcher matcher3 = Pattern.compile("(?<=(<td class='sell'>))(.+?)(?=(</td>))").matcher(recvHtml);
            int n = 0;

            while (matcher3.find()) {
                String unit = unitList.get(n);
                String number = matcher3.group();

                if (!unit.equals("1")) {
                    number = new BigDecimal(number).divide(new BigDecimal(unit), 10, 1).toString();
                }

                int ptIdx = number.indexOf(".");
                int numLen = number.length();
                int n2;

                if (ptIdx == -1) { // 소수점 없음
                    n2 = numLen;
                }
                else { // 소수점 둘째자리
                    ptIdx += 2;
                    n2 = ((ptIdx > numLen) ? numLen : ptIdx);
                }

                concurrentHashMap.put(CurrencyCodename[n], Long.parseLong(number.substring(0, n2 + 1).replaceAll("\\.", "")));
                ++n;
            }

            final byte[] obj2ByteAry = KsCommonLib.obj2ByteAry((Object)concurrentHashMap);

            if (selKey == null || !selKey.channel().isOpen()) {
                return;
            }

            final byte[] sendByte = KsCommonLib.makeSendByte("ufcex", KsCommonLib.byteAry2HexString(obj2ByteAry).getBytes("EUC-KR"));
            final SocketChannel socketChannel = (SocketChannel)selKey.channel();
            final ByteBuffer sendBuf = ByteBuffer.allocate(sendByte.length);

            sendBuf.put(sendByte);
            sendBuf.clear();

            Thread.sleep(5000);

            socketChannel.write(sendBuf);
            disconnectClient(selKey);

            logger.log(Level.INFO, String.format("해시맵: %s", concurrentHashMap.toString()));
            logger.log(Level.INFO, String.format("전송:\n%s", new String(sendByte)));
        }
        catch (Exception ex) {
            logger.log(Level.WARNING, "msg_ufcex_updateFcex 오류.", ex);
        }
    }
}
