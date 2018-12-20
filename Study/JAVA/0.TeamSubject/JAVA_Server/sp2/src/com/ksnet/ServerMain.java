package com.ksnet;

import java.util.logging.Level;
import java.util.logging.Logger;

public class ServerMain {
	private static Logger logger = Logger.getLogger(ServerMain.class.getName());
	private static Server server = null;
	
	public static void main(String[] args) {
		int exitCode = -2;
		
		if (init(args)) {
			exitCode = (run() ? 0 : -1);
		}
		
		System.exit(exitCode);
	}
	
	public static boolean init(String[] args) {
		try {
			Global.init(args);
			server = Server.open();
		}
		catch (Exception e) {
			logger.log(Level.SEVERE, "서버 초기화 오류!", e);
			return false;
		}
		
		return true;
	}
	
	public static boolean run() {
		try {
			return server.run();
		}
		catch (Exception e) {
			logger.log(Level.SEVERE, "서버 구동 오류!", e);
			return false;
		}
	}
}