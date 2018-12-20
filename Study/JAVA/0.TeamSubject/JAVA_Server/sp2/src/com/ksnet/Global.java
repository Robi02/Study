package com.ksnet;

import java.util.HashMap;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.Map;

public class Global {
	private static Logger logger = Logger.getLogger(Global.class.getName());
	private static Map<String, String> envMap = new HashMap<String, String>();
	
	public static String SERVER_SOCKET_BIND_PORT = "SERVER_SOCKET_BIND_PORT";
	public static String HANA_FCEXT_URL = "HANA_FCEXT_URL";
	
	public static void init(String[] args) {
		for (int i = 0; i < args.length; i += 2) {
			envMap.put(args[i], args[i + 1]);
		}
	}
	
	public static String getEnv(String key) {
		return envMap.get(key);
	}
}