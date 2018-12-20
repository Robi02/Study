package com.ksnet.util;

import java.io.FileOutputStream;
import java.io.IOException;

public class TransceiveLogger {

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