package com.rblee;

import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import javax.xml.bind.DatatypeConverter;

public class CipherManager {
    // [Class private constants]
    private static final Logger logger = Logger.getInstance();

    // [Class public constants]
	public static final int AES = 0;
	
	// [Class private variables]
	private static boolean ManagerInitialized	= false;	// Manager 초기화 여부
	private static Cipher AesCipher				= null;     // 128bit-AES암호화
	private static IvParameterSpec IvParamSpec	= null;     // Initial Vector
	
	// [Methods]
	// 매니저 클래스 초기화
	public static void init() throws Exception {
		// AesCipher
		// (NoSuchAlgorithmException | NoSuchPaddingException)
		AesCipher = Cipher.getInstance("AES/CBC/PKCS5Padding"); // 128bit-AES
		
		// IvParamSpec
		byte[] ivBytes = new byte[16]; // 128bit-AES -> 16byte key
		System.arraycopy("0123456789ABCDEF0123456789abcdef".getBytes(), 0, ivBytes, 0, ivBytes.length);
		IvParamSpec = new IvParameterSpec(ivBytes);
		
		ManagerInitialized = true;
    }
    
    // 키 생성
    public static SecretKeySpec makeSecretKeySpec(int cipherType, byte[] keyBytes) throws Exception {
        if (keyBytes == null) {
            logger.error("'keyBytes' is null!");
            return null;
        }

        if (cipherType == AES) {
			byte[] newKeyBytes = new byte[16];
			System.arraycopy(keyBytes, 0, newKeyBytes, 0, newKeyBytes.length);
			return new SecretKeySpec(newKeyBytes, "AES");
		}
		else {
			logger.error("'cipherType' is undefined! (cipherType:'" + cipherType + "')");
		}

        return null;
    }
	
	// 대칭키 암호화 수행
	public static String encrypt(int cipherType, String plainStr, SecretKeySpec secretKeySpec) throws Exception {
		if (!ManagerInitialized) {
			init();
		}
		
		if (plainStr == null || secretKeySpec == null) {
			logger.error("'plainStr' or 'secretKeySpec' is null! (plainStr:" + plainStr + ")");
			return null;
		}
		
		byte[] cipherBytes = null;
		
		if (cipherType == AES) {
			cipherBytes = cipherAES(Cipher.ENCRYPT_MODE, plainStr.getBytes(), secretKeySpec);
		}
		
		if (cipherBytes == null) {
			logger.error("The encode reseult 'cipherBytes' is null. encoding failed!");
			return null;
		}

		return DatatypeConverter.printBase64Binary(cipherBytes);
	}
	
	// 대칭키 복호화 수행
	public static String decrypt(int cipherType, String cipherStr, SecretKeySpec secretKeySpec) throws Exception {
		if (!ManagerInitialized) {
			init();
		}
		
		if (cipherStr == null || secretKeySpec == null) {
			logger.error("'cipherStr' or 'secretKeySpec' is null! (cipherStr:" + cipherStr + ")");
			return null;
		}
		
		byte[] cipherBytes = null;
		byte[] plainBytes = null;
		cipherBytes = DatatypeConverter.parseBase64Binary(cipherStr);
		
		if (cipherType == AES) {
			plainBytes = cipherAES(Cipher.DECRYPT_MODE, cipherBytes, secretKeySpec);
		}
		
		if (plainBytes == null) {
			logger.error("The decode reseult 'plainBytes' is null. decoding failed!");
			return null;
		}
		
		return new String(plainBytes);
	}
	
	// AES 암호화
	private static byte[] cipherAES(int opMode, byte[] inBytes, SecretKeySpec secretKeySpec) throws Exception {
		byte[] outBytes = null;
		
		// (InvalidKeyException | InvalidAlgorithmParameterException | IllegalBlockSizeException | BadPaddingException)
		AesCipher.init(opMode, secretKeySpec, IvParamSpec);
		outBytes = AesCipher.doFinal(inBytes);
		
		return outBytes;
	}
}
