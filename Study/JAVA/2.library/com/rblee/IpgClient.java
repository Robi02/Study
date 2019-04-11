package com.rblee;

import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.net.InetSocketAddress;
import java.util.Arrays;

public class IpgClient {

    // 맴버 변수 //
    private String ipgAddr; // 대상 서버 주소
    private int ipgPort;    // 대상 서버 포트
    
    private String traceId; // 추적 ID
    private Logger logger;  // 로거

    // 맴버 메서드 //
    public IpgClient(String ipgAddr, int ipgPort, String traceId, Logger logger) {
        this.ipgAddr = ipgAddr;
        this.ipgPort = ipgPort;
        this.traceId = traceId;
        this.logger = logger;
    }

    // IPG로 전문 발신 후 수신 (블로킹)
    public boolean sendAndRecvMsg(IpgMsg sendIpgMsg, IpgMsg recvIpgMsg, boolean localTest) {
        if (sendIpgMsg == null || recvIpgMsg == null) {
            log(Logger.ERROR, traceId, "One of sendIpg/recvIpgMsg is null! (sendIpgMsg:" + sendIpgMsg + ", recvIpgMsg:" + recvIpgMsg + ")");
            return false;
        }

        byte[] sendByteStream = sendIpgMsg.getMsgStream();

        // 내부망 스위치 암호화 전문헤더 붙이기
        if (!localTest) {
            int originSendMsgLen = sendByteStream.length; // 원래 전문 길이

            // {암호화여부 포함한 원래전문 길이(6) + 암호화여부(1)}{원래전문(...)} (0:비암호화)
            // 원래전문을 앞에 7바이트를 비운 위치로 카피
            byte[] headedSendByteStream = new byte[originSendMsgLen + 7];
            System.arraycopy(sendByteStream, 0, headedSendByteStream, 7, originSendMsgLen);
            
            // {암호화 전문 헤더(길이 + 암호화여부)}
            // 앞의 7바이트에 암호화 헤더 붙임 (originSendMsgLen + 1 : 암호화여부도 길이로 포함)
            byte[] headByteStream = String.format("%06d0", Math.min(originSendMsgLen + 1, 999999)).getBytes();
            System.arraycopy(headByteStream, 0, headedSendByteStream, 0, 7);
            
            sendByteStream = headedSendByteStream;
        }

        long sendMsgLen = sendByteStream.length;
        long recvMsgLen = recvIpgMsg.getMsgStream().length + (localTest ? 0 : 6);   // 내부망 스위치 전문헤더 6 : 원래전문길이정보(6)
        long remainSendLen = sendMsgLen;                                            // 발신해야 할 전문의 남은 길이
        long remainReadLen = recvMsgLen;                                            // 수신해야 할 전문의 남은 길이
        SocketChannel cliSoc = null;
        boolean rtResult = true;

        try {
            cliSoc = SocketChannel.open((SocketAddress)new InetSocketAddress(ipgAddr, ipgPort)); // Open and Connect

            ByteBuffer writeBuffer = ByteBuffer.wrap(sendByteStream);
            
            while (remainSendLen > 0) {
                // 기본모드 : ByteBuffer.remaining()크기만큼 다 전송할 때까지 블로킹
                remainSendLen -= cliSoc.write(writeBuffer);
                Thread.sleep(1);
            }

            ByteBuffer readBuffer = ByteBuffer.allocate((int)remainReadLen);
            
            while (remainReadLen > 0) {
                // 기본모드 : 수신버퍼 내에서 ByteBuffer.remaining() 크기만큼 읽음 읽는동안 블로킹
                long readLen = -1;

                if ((readLen = cliSoc.read(readBuffer)) > 0) {
                    if ((remainReadLen -= readLen) == 0) {
                        break;
                    }
                }
                else {
                    if (remainReadLen > 0) {
                        // 수신받을 데이터가 남음: 오류로 소켓 닫기
                        log(Logger.ERROR, traceId, "Error! The socket will be closed. (readLen:" + readLen +
                                                   ", remainReadLen:" + remainReadLen + ")");
                        
                        if (cliSoc != null && cliSoc.isConnected()) {
                            cliSoc.close();
                        }

                        rtResult = false;
                    }
                }
            }

            if (!localTest) { // 내부망 스위치 전문헤더 제거
                recvIpgMsg.setMsgStream(Arrays.copyOfRange(readBuffer.array(), 6, (int)recvMsgLen));
            }
            else {
                recvIpgMsg.setMsgStream(readBuffer.array());
            }
        }
        catch (Exception exception1) {
            log(Logger.ERROR, traceId, "Exception!", exception1);
            rtResult = false;
        }
        finally {
            try {
                if (cliSoc != null && cliSoc.isConnected()) {
                    cliSoc.close();
                }
            }
            catch (Exception exception2) {
                // 소켓 닫기 실패 예외는 경고로 처리하고 실패로 처리하지 않는다...
                log(Logger.WARNING, traceId, "Exception has occurred while closing the socket!", exception2);
            }

            return rtResult;
        }
    }

    private void log(int logType, String traceId, Object... logObjs) {
        if (logger != null) {
            logger.traceLog(logType, traceId, logObjs);
        }
    }
}
