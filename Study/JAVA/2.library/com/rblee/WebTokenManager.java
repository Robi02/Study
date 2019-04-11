package com.rblee;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;
import java.nio.channels.OverlappingFileLockException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Formatter;
import java.util.HashMap;

public class WebTokenManager {

    public static String DEFAULT_WEB_TOKEN_DIR = "rb_web_tokens/";  // 웹토큰 저장 위치(디폴트)

    // - 웹토큰 포멧 : [ {svrCode}{svcCode}{systimesec(Base62)}{TodayWtIdCounter(Base62)} ]
    // - 토큰 길이 : [ 서버코드(2) + 서비스코드(2) + 초단위시간정보(6) + 난수(1) + 금일시퀀스(4) + 난수(1) ] => 16자리
    private static final long WtIdCounterOffset = 1000;  // 토큰 번호 시작 오프셋
    private static long TodayWtIdCounter = 0;            // 금일 웹 토큰 카운터
    private static int TodayDate = -1;                   // 금일 일자
    private static long LastTokenSecond = -1;            // 마지막으로 출력된 시점의 초
    private static String LastTokenTimeCacheStr = null;  // 마지막으로 출력된 토큰 시간정보 문자열

    // 저장경로 변경
    public static void setDefaultWebTokenDir(String newWebTokenDir) {
        DEFAULT_WEB_TOKEN_DIR = newWebTokenDir;
    }

    // 고유 웹 토큰을 반환
    public static synchronized String genWebTokenStr(String svrCode, String svcCode, Logger logger) {
        Calendar cal = Calendar.getInstance();
        int day = cal.get(Calendar.DAY_OF_MONTH);
        long sysTimeSec = cal.getTimeInMillis() / 1000L; // 초단위로 측정

        if (day != TodayDate) { // 일자가 변경됨
            TodayWtIdCounter = WtIdCounterOffset;
            TodayDate = day;
        }
        
        ++TodayWtIdCounter;

        StringBuilder sb = new StringBuilder();

        if (sysTimeSec != LastTokenSecond) {
            // 1초이상 경과된 호출: 시간정보 재생성, 캐시 교체
            LastTokenSecond = sysTimeSec;
            LastTokenTimeCacheStr = ("000000" + Base62.Encode(sysTimeSec)); // 캐시된 시간정보 문자열 갱신
            int cacheLen = LastTokenTimeCacheStr.length();
            LastTokenTimeCacheStr = LastTokenTimeCacheStr.substring(cacheLen - 6, cacheLen);
        }

        String b62WtIdCount  = ("0000" + Base62.Encode(TodayWtIdCounter));
        int countLen = b62WtIdCount.length();
        b62WtIdCount = b62WtIdCount.substring(countLen - 4, countLen);

        String randomPadding = RandomManager.genRandomStr(2, "ALPHA_NUMBER");

        sb.append(svrCode).append(svcCode).append(LastTokenTimeCacheStr)
          .append(randomPadding.charAt(0)).append(b62WtIdCount).append(randomPadding.charAt(1));

        return sb.toString();
    }

    // 토큰 암호 확인
    public static boolean checkWebTokenPw(String wtDirStr, String wtId, String wtPw, Logger logger) {
        HashMap<String, String> readMap = new HashMap<String, String>();

        if (!readWebToken(wtDirStr, wtId, wtPw, readMap, logger)) { // 토큰을 성공적으로 읽었으면 패스워드체크 통과
            return false;
        }
        
        return true;
    }

    // wtId이름과 wtPw암호를 가진 토큰파일을 생성 후 결과 반환
    public static boolean genWebTokenFile(String wtDirStr, String wtId, String wtPw, Logger logger) {
        if (wtId == null || wtPw == null) {
            return false;
        }

        wtDirStr = checkWebTokenDirStr(wtDirStr);

        File wtDir = new File(wtDirStr);
        File wtFile = null;

        try {
            // 토큰경로 검사 or 생성
            if (!wtDir.exists()) {
                if(!wtDir.mkdirs()) {
                    log(logger, Logger.ERROR, wtId, "mkdirs() return false!");
                    return false; // 새 경로 생성 실패
                }
            }

            // 토큰파일 생성
            wtFile = new File(wtDirStr + wtId);

            if (!wtFile.exists()) {
                if (!wtFile.createNewFile()) {
                    log(logger, Logger.ERROR, "createNewFile() return false!");
                    return false; // 새 토큰 파일 생성 실패
                }
            }
            else {
                return false; // 토큰 파일이 이미 존재함
            }
        }
        catch (Exception e0) {
            log(logger, Logger.ERROR, wtId, "Exception!", e0);
        }

        // 토큰에 wtPw(토큰 패스워드)와 생성 시스템시간 기록
        RandomAccessFile wtStream = null;
        FileChannel wtChannel = null;
        FileLock wtLock = null;
        String keyValStr = String.format("{\n\"_wtPw\":\"%s\",\n\"_wtCreatedTime\":\"%d\"\n}", wtPw, System.currentTimeMillis());

        try {
            wtStream = new RandomAccessFile(wtFile, "rw");
            wtChannel = wtStream.getChannel();

            // // FILE LOCKED // //
            wtLock = wtChannel.tryLock(); // Already Locked : throws OverlappingFileLockException
            // // FILE LOCKED // //

            wtChannel.write(ByteBuffer.wrap(keyValStr.getBytes()));
        }
        catch (Exception e3) {
            log(logger, Logger.ERROR, wtId, "Exception!", e3);
        }
        finally {
            try {
                // FILE UNLOCKED //
                if (wtLock != null) {
                    wtLock.release();   // OverlappingFileLockException 예외를 제외하고는
                                        // 항상 이 코드에 도착시켜서 Locking 해제
                }
                // FILE UNLOCKED //

                if (wtStream != null) wtStream.close();
                if (wtChannel != null) wtChannel.close();
            }
            catch (Exception e3_2) {
                log(logger, Logger.ERROR, wtId, "Exception!", e3_2);
            }
        }

        return true;
    }

    // 웹 토큰파일 삭제
    public static boolean removeWebTokenFile(String wtDirStr, String wtId, Logger logger) {
        if (wtId == null) {
            return false;
        }

        wtDirStr = checkWebTokenDirStr(wtDirStr);
        
        File wtDir = new File(wtDirStr);
        File wtFile = null;
        
        try {
            // 토큰파일 경로 검사
            if (!wtDir.exists()) {
                return false;
            }

            wtFile = new File(wtDirStr + wtId);

            // 토큰파일 열기
            if (!wtFile.exists()) {
                return false;
            }

            // 파일 삭제
            wtFile.delete();
            wtFile = null;
        }
        catch (Exception e0) {
            log(logger, Logger.ERROR, wtId, "Exception!", e0);
            return false;
        }

        return true;
    }

    // wtId이름을 가진 파일을 연 후 tkMap의 데이터를 JSON 형식으로 저장
    public static boolean writeWebToken(String wtDirStr, String wtId, String wtPw, HashMap<String, String> tkMap, Logger logger) {
        if (wtId == null || tkMap == null || tkMap.size() < 1) {
            return false;
        }

        wtDirStr = checkWebTokenDirStr(wtDirStr);
        
        File wtDir = new File(wtDirStr);
        File wtFile = null;
        
        try {
            // 토큰파일 경로 검사
            if (!wtDir.exists()) {
                return false;
            }

            wtFile = new File(wtDirStr + wtId);

            // 토큰파일 열기
            if (!wtFile.exists()) {
                return false;
            }
        }
        catch (Exception e0) {
            log(logger, Logger.ERROR, wtId, "Exception!", e0);
        }

        // 파일에 쓰기
        int retryLimitCnt = 10;
        int retryDelayMillis = (int)(System.currentTimeMillis() % 101L) + 50; // 50~150ms

        if (!fileLockWithReadWrite(wtFile, wtId, wtPw, null, tkMap, retryLimitCnt, retryDelayMillis, logger)) {
            return false;
        }

        return true;
    }

    // wtId이름을 가진 파일을 열기 후 tkMap에 JSON데이터를 채움
    public static boolean readWebToken(String wtDirStr, String wtId, String wtPw, HashMap<String, String> tkMap, Logger logger) {
        if (wtId == null || wtPw == null || tkMap == null) {
            return false;
        }

        wtDirStr = checkWebTokenDirStr(wtDirStr);

        // 토큰파일 열기
        boolean wtPwPassed = false;
        File wtFile = new File(wtDirStr + wtId);

        try {
            if (!wtFile.exists()) {
                return false;
            }
        }
        catch (Exception e0) {
            log(logger, Logger.ERROR, wtId, "Exception!", e0);
            return false;
        }

        // 파일에서 읽기
        int retryLimitCnt = 10;
        int retryDelayMillis = (int)(System.currentTimeMillis() % 101L) + 50; // 50~150ms

        if (!fileLockWithReadWrite(wtFile, wtId, wtPw, tkMap, null, retryLimitCnt, retryDelayMillis, logger)) {
            return false;
        }

        return true;
    }

    // 파일 락 수행 -> 파일 '읽기(Read) / 읽기+쓰기(Write)' -> 파일 락 해제
    private static boolean fileLockWithReadWrite(File wtFile, String wtId, String wtPw, HashMap<String, String> wtReadMap,
                                                 HashMap<String, String> wtWriteMap, int retryLimitCnt, int retryDelay, Logger logger) {
        // [Note]
        // 1) 웹토큰 읽기 작업은 파일락 수행 후 읽은 JSON을 파싱하여 wtReadMap에 담아보내고 락 해제후 성공여부 반환
        // 2) 웹토큰 쓰기 작업은 읽기 작업 이후, 추가된 파라미터값을 wtReadMap과 더하여 파일에 새로 기록하고 락 해제후 성공여부 반환
        
        if (wtReadMap == null && wtWriteMap != null) {
            wtReadMap = new HashMap<String, String>();
        }
        else if (wtReadMap == null && wtWriteMap == null) {
            return false;
        }

        boolean rtResult = false;
        boolean fileReadComplete = false;
        RandomAccessFile wtStream = null;
        FileChannel wtChannel = null;
        FileLock wtLock = null;

        // [Note]
        // - wtChannel.tryLock() 으로 다른 프로세스/스레드에서의 파일 읽기/쓰기를 방지할 수 있다.
        // - 특이한 점은 tryLock()을 수행한 try-catch-finally 구문 안에서 읽기/쓰기는 가능하지만,
        //   해당 구문을 벗어난 곳에서 접근을 시도하면 IOException이 발생한다. (스레드 단위의 Locking이 아닌 듯)
        //   그래서 복잡한 로직을 분리하지 못하고 하나의 구문에 다 작성했다.
        
        while ((retryLimitCnt--) > 0) {
            try {
                wtStream = new RandomAccessFile(wtFile, "rw");
                wtChannel = wtStream.getChannel();

                // // FILE LOCKED // //
                wtLock = wtChannel.tryLock(); // 파일이 이미 잠겨있음 : throws OverlappingFileLockException
                // // FILE LOCKED // //

                // 읽기 작업 수행
                // 파일을 바이트 단위로 읽어서 wtStr에 저장
                ByteBuffer readBuf = ByteBuffer.allocate((int)wtStream.length());
                wtChannel.read(readBuf);
                String wtStr = new String(readBuf.array());
                fileReadComplete = true;

                // JSON 문자열 파싱
                int keyBgnIdx = -1, keyEndIdx = -1, valBgnIdx = -1, valEndIdx = -1;
                String key = null, val = null;

                if ((keyBgnIdx = wtStr.indexOf("{")) == -1) {
                    log(logger, Logger.ERROR, wtId, String.format("keyBgnIdx error! (wtFileName:%s, keyBgnIdx:%d)", wtFile.getName(), keyBgnIdx));
                    return false;
                }

                while ((keyBgnIdx = wtStr.indexOf("\"", keyBgnIdx)) != -1) {
                    // Key
                    ++keyBgnIdx;

                    if ((keyEndIdx = wtStr.indexOf("\":", keyBgnIdx)) == -1) {
                        log(logger, Logger.ERROR, wtId, String.format("keyEndIdx error! (wtFileName:%s, keyBgnIdx:%d, keyEndIdx:%d)", wtFile.getName(), keyBgnIdx, keyEndIdx));
                        break;
                    }

                    if (keyBgnIdx < keyEndIdx) {
                        // Found key
                        key = wtStr.substring(keyBgnIdx, keyEndIdx);
                    }
                    else {
                        log(logger, Logger.ERROR, wtId, String.format("null key error! (wtFileName:%s, keyBgnIdx:%d, keyEndIdx:%d)", wtFile.getName(), keyBgnIdx, keyEndIdx));
                        break;
                    }

                    keyEndIdx += 2;

                    // Value
                    if ((valBgnIdx = wtStr.indexOf("\"", keyEndIdx)) == -1) {
                        log(logger, Logger.ERROR, wtId, String.format("valBgnIdx error! (wtFileName:%s, keyEndIdx:%d, valBgnIdx:%d)", wtFile.getName(), keyEndIdx, valBgnIdx));
                        break;
                    }

                    ++valBgnIdx;

                    if ((valEndIdx = wtStr.indexOf("\"", valBgnIdx)) == -1) {
                        log(logger, Logger.ERROR, wtId, String.format("valEndIdx error! (wtFileName:%s, valBgnIdx:%d, valEndIdx:%d)", wtFile.getName(), valBgnIdx, valEndIdx));
                        break;
                    }

                    if (valBgnIdx < valEndIdx) {
                        // Found value
                        val = wtStr.substring(valBgnIdx, valEndIdx);
                    }
                    else if (valBgnIdx == valEndIdx) {
                        // Value is null
                        val = null;
                    }
                    else {
                        log(logger, Logger.ERROR, wtId, String.format("valBgn/EndIdx error! (wtFileName:%s, valBgnIdx:%d, valEndIdx:%d)", wtFile.getName(), valBgnIdx, valEndIdx));
                        break;
                    }

                    keyBgnIdx = Math.min(valEndIdx + 1, wtStr.length());

                    // Put key and val to wtReadMap
                    wtReadMap.put(key, val);
                }

                // 토큰 패스워드 체크
                String readWtPw = wtReadMap.get("_wtPw");

                if (readWtPw == null || (!readWtPw.equals(wtPw))) {
                    // 패스워드 틀릴 시 읽은 데이터 파기
                    log(logger, Logger.WARNING, wtId, String.format("Wrong password! (wtFileName:%s, inputWtPw:%s)", wtFile.getName(), wtPw));
                    wtReadMap.clear();
                    rtResult = false;
                    break;
                }
                else {
                    // wtWriteMap이 전달된 경우 : 쓰기 작업 수행
                    if (wtWriteMap != null && wtWriteMap.size() > 0) {
                        // 마지막으로 수정한 시간을 추가
                        wtWriteMap.put("_wtLastModifiedTime", String.valueOf(System.currentTimeMillis()));
                        
                        // 읽은 데이터에 추가할 데이터를 합침
                        wtReadMap.putAll(wtWriteMap);

                        StringBuffer keyValSb = new StringBuffer();
                        keyValSb.append("{\n");

                        for (String writeKey : wtReadMap.keySet()) {
                            keyValSb.append("\"").append(writeKey).append("\":\"")
                                    .append(wtReadMap.get(writeKey)).append("\",\n"); // "writeKey":"writeVal",\n
                        }

                        keyValSb.setLength(keyValSb.length() - 2); // 마지막 ",\n" 문자 제거
                        keyValSb.append("\n}");

                        wtStream.setLength(0); // 파일 내용 다 지우고 새로운 내용으로 씀
                        wtChannel.write(ByteBuffer.wrap(keyValSb.toString().getBytes()));
                    }

                    rtResult = true;
                }

                break; // 로직 끝, 반복문 탈출
            }
            catch (OverlappingFileLockException e1) {
                // 파일이 락된 경우는 {retryDelay}ms 후에 재시도
                try { Thread.sleep(retryDelay); } catch (Exception e1_1) { 
                    log(logger, Logger.ERROR, wtId, String.format("Exception! (wtFileName:%s)", wtFile.getName()), e1_1);
                }
                continue;
            }
            catch (Exception e2) {
                log(logger, Logger.ERROR, wtId, String.format("Exception! (wtFileName:%s)", wtFile.getName()), e2);
            }
        }

        try {
            // FILE UNLOCKED //
            if (wtLock != null) {   // OverlappingFileLockException 예외를 제외하고는
                wtLock.release();   // 항상 이 코드에 도달하여 Locking 해제
            }
            // FILE UNLOCKED //

            if (wtStream != null) wtStream.close();
            if (wtChannel != null) wtChannel.close();
        }
        catch (Exception e3) {
            // 주의 : 로직이 이곳에 도달한 경우에는 파일 락이 안풀렸거나 채널이 정상적으로 닫히지 않았을 수 있음!
            log(logger, Logger.ERROR, wtId, String.format("Exception! (wtFileName:%s)", wtFile.getName()), e3);
            return false;
        }

        if (!fileReadComplete) {
            // 파일 읽기 실패
            log(logger, Logger.ERROR, wtId, String.format("File read failed! (wtFileName:%s)", wtFile.getName()));
            return false;
        }

        return rtResult;
    }

    // 토큰 경로포멧 검사 (가장 마지막 문자열이 항상 '/' 가 되도록 함)
    private static String checkWebTokenDirStr(String wtDirStr) {
        if (wtDirStr == null) {
            wtDirStr = DEFAULT_WEB_TOKEN_DIR;
        }

        int dirStrLen = wtDirStr.length();

        if (wtDirStr.charAt(dirStrLen - 1) != '/') {
            wtDirStr += "/";
        }

        return wtDirStr;
    }

    // WebTokenManager로거
    private static void log(Logger logger, int logType, String traceId, Object... logObjs) {
        if (logger != null) {
            logger.traceLog(logType, traceId, logObjs);
        }
	}
}