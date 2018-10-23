package com.ksnet.util;

public class Logger {
	
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
		if (logTypeAry == null || logTypeAry.length == 0) { // 일괄적용
			for (int i = 0; i < visibleLogTypeAry.length; ++i) {
				visibleLogTypeAry[i] = isVisible;
			}
		}
		else { // 개별적용
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