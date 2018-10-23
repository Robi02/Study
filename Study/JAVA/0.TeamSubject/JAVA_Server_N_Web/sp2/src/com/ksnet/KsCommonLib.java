package com.ksnet;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.IOException;
import java.util.Arrays;

public class KsCommonLib {
	
	public static final byte SOH = (byte)0x01;
	public static final byte STX = (byte)0x02;
	public static final byte ETX = (byte)0x03;
	
	public static byte[] makeSendByte(String msgId, byte[] byteData) {
		byte[] rtSendByte = null;
		
		try {
			int dataSize = 0;
			
			if (byteData != null) {
				dataSize = byteData.length;
			}
			
			byte[] headerByte = String.format("%-5s%05d", msgId, dataSize).getBytes("EUC-KR");
			rtSendByte = new byte[headerByte.length + dataSize + 3];

			for (int i = 0; i < headerByte.length; ++i) { // copy header
				rtSendByte[i + 1] = headerByte[i];
			}

			int dataBgnIdx = headerByte.length + 2;
			
			for (int j = 0; j < dataSize; ++j) { // copy data
				rtSendByte[j + dataBgnIdx] = byteData[j];
			}
			
			rtSendByte[0] = SOH; // set identifier tag ()
			rtSendByte[headerByte.length + 1] = STX;
			rtSendByte[rtSendByte.length - 1] = ETX;
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
		return rtSendByte;
	}
	
	public static byte[] makeRecvByte(byte[] byteData) {
		byte[] rtRecvByte = null;
		
		try {
			if (byteData[0] != SOH || byteData[11] != STX) {
				return null;
			}
			
			int dataSize = Integer.parseInt(new String(Arrays.copyOfRange(byteData, 6, 11)));
			
			if (dataSize <= 0) {
				return null;
			}
			
			rtRecvByte = Arrays.copyOfRange(byteData, 12, 12 + dataSize);			
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
		return rtRecvByte;
	}
	
	public static byte[] obj2ByteAry(Object obj) {
		byte[] rtByte = null;
		ByteArrayOutputStream btos = new ByteArrayOutputStream();
		
		try {
			ObjectOutputStream oos = new ObjectOutputStream(btos);
			oos.writeObject(obj);
			oos.flush();
			oos.close();
			btos.close();
			rtByte = btos.toByteArray();
		}
		catch (IOException e) {
			e.printStackTrace();
		}
		
		return rtByte;
	}
	
	public static Object byteAry2Obj(byte[] byteAry) {
		Object rtObj = null;
		
		try {
			ByteArrayInputStream bais = new ByteArrayInputStream(byteAry);
			ObjectInputStream ois = new ObjectInputStream(bais);
			
			rtObj = ois.readObject();
		}
		catch (IOException ioe) {
			ioe.printStackTrace();
		}
		catch (ClassNotFoundException cnfe) {
			cnfe.printStackTrace();
		}
		
		return rtObj;
	}
	
	public static String byteAry2HexString(byte[] byteAry) {
		StringBuilder builder = new StringBuilder();
		
		for (int i = 0; i < byteAry.length; ++i) {
			builder.append(String.format("%02X", byteAry[i]));
		}
		
		return builder.toString();
	}
	
	public static byte[] hexString2ByteAry(String hexString) {
		int strLen = hexString.length();
		byte[] rtByte = new byte[strLen / 2];
		
		for (int i = 0; i < strLen; i += 2) {
			rtByte[i / 2] = (byte)((Character.digit(hexString.charAt(i), 16) << 4) +
								   (Character.digit(hexString.charAt(i + 1), 16)));
		}
		
		return rtByte;
	}
}