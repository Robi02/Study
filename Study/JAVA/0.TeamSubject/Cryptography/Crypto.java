import java.io.IOException;
import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.util.Arrays;
import java.util.Base64;
import java.util.Base64.Encoder;
import java.util.Base64.Decoder;

public class Crypto {

    private static String Charset = "EUC-KR";

    public static String PrintStreamToHexa(byte[] inByte) {
        StringBuilder sb = new StringBuilder();

        for (byte b : inByte) {
            sb.append(String.format("%02X ", b));
        }

        return sb.toString();
    }

    public static String checkAlgo(String algo) throws Exception {
        if (algo.equals("-des")) {
            return new String("DES");
        }
        else if (algo.equals("-aes")) {
            return new String("AES");
        }
        else if (algo.equals("-blowfish")) {
            return new String("BLOWFISH");
        }
        else {
            throw new Exception("Undefined algo '" + algo + "'.");
        }
    }

    public static String checkMode(String mode) throws Exception {
        if (mode.equals("-ecb")) {
            return new String("ECB");
        }
        else if (mode.equals("-cbc")) {
            return new String("CBC");
        }
        else {
            throw new Exception("Undefined mode '" + mode + "'.");
        }
    }

    public static String checkPadd(String padd) throws Exception {
        if (padd.equals("-null")) {
            return new String("NoPadding");
        }
        else if (padd.equals("-pkcs")) {
            return new String("PKCS5Padding");
        }
        else {
            throw new Exception("Undefined padd '" + padd + "'.");
        }
	}

	public static boolean checkKey(String algo, String key) throws Exception {
		int keyLen = key.length();

		if (algo.equals("DES")) {
			if (keyLen != 8 && (keyLen != 16 && keyLen != 24)) { // DES,DESede
				System.out.println(algo + " key size error. (keyLen=" + keyLen + " / (Req:8,16,24)");
				return false;
            }
		}
		else if (algo.equals("AES")) {
			if (keyLen != 16 && keyLen != 24 && keyLen != 32) { // 128,196,256bit AES
				System.out.println(algo + " key size error. (keyLen=" + keyLen + " / (Req:16,24,32)");
				return false;
			}
		}
		else if (algo.equals("BLOWFISH")) {
			if (keyLen < 8 || keyLen > 56) { // 8~56byte key Blowfish
				System.out.println(algo + " key size error. (keyLen=" + keyLen + " / (Req:8~56)");
				return false;
			}
		}
		else {
			throw new Exception("Undefined algo '" + algo + "'.");
        }
        
        return true;
    }

    public static byte[] makeNullPadd(String algo, byte[] planeByte) {
        String nullStr = "\0";
        int msgLen = planeByte.length;
        int padLen = 0;

        if (algo.substring(0, 3).equals("DES")) { // 8
            padLen = 8 - (msgLen % 8);
            padLen %= 8;
        }
        else if (algo.equals("AES")) { // 16
            padLen = 16 - (msgLen % 16);
            padLen %= 16;
        }
        else if (algo.equals("BLOWFISH")) { // 8
            padLen = 8 - (msgLen % 8);
            padLen %= 8;
        }

        if (padLen == 0) {
            return planeByte;
        }

        byte[] rtByte = new byte[msgLen + padLen];
        Arrays.fill(rtByte, (byte)0x00);
        System.arraycopy(planeByte, 0, rtByte, 0, msgLen);

        return rtByte;
    }
    
    public static String encrypt(String algo, String mode, String padd, String key, String iv, String planeMsg) {
        try {
            // DES check
            int keyLen = key.length();

            if (algo.equals("DES") && (keyLen == 16 || keyLen == 24)) {
                algo = algo.concat("ede"); // DES -> DESede

                if (keyLen == 16) {
                    key = key.concat(key.substring(0, 8));
                }
            }

            // Get cipher instance
            String cipherMod = String.format("%s/%s/%s", algo, mode, padd);
            Cipher cipher = Cipher.getInstance(cipherMod);
            
            // Set key and iv into cipher
            SecretKeySpec skey = new SecretKeySpec(key.getBytes(Charset), algo);

            if (mode.equals("ECB")) {
                cipher.init(Cipher.ENCRYPT_MODE, skey);
            }
            else {
                IvParameterSpec siv = new IvParameterSpec(iv.getBytes(Charset));
                cipher.init(Cipher.ENCRYPT_MODE, skey, siv);
            }

            // NullPadd(NoPadding) check
            byte[] planeByte = planeMsg.getBytes(Charset);
            
            if (padd.equals("NoPadding")) {
                planeByte = makeNullPadd(algo, planeByte);
            }

            // Encrypt and convert byte stream to Base64 symbol
            byte[] cipherByte = cipher.doFinal(planeByte);
            Encoder   encoder = Base64.getEncoder();

            return new String(encoder.encode(cipherByte), Charset);
        }
        catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }

    public static String decrypt(String algo, String mode, String padd, String key, String iv, String cipherMsg) {
        try {
            // DES check
            int keyLen = key.length();

            if (algo.equals("DES") && (keyLen == 16 || keyLen == 24)) {
                algo = algo.concat("ede"); // DES -> DESede

                if (keyLen == 16) {
                    key = key.concat(key.substring(0, 8));
                }
            }

            // Get cipher instance
            String cipherMod = String.format("%s/%s/%s", algo, mode, padd);
            Cipher cipher = Cipher.getInstance(cipherMod);

            // Set key and iv into cipher
            SecretKeySpec skey = new SecretKeySpec(key.getBytes(Charset), algo);

            if (mode.equals("ECB")) {
                cipher.init(Cipher.DECRYPT_MODE, skey);
            }
            else {
                IvParameterSpec siv = new IvParameterSpec(iv.getBytes(Charset));
                cipher.init(Cipher.DECRYPT_MODE, skey, siv);
            }

            // Decode Base64 symbol to byte stream and decrypt it
            Decoder   decoder = Base64.getDecoder();
            byte[] cipherByte = cipherMsg.getBytes(Charset);
            byte[]  planeByte = cipher.doFinal(decoder.decode(cipherByte));

            return new String(planeByte, Charset);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
        
        return null;
    }

    public static void printUsage() {
        System.out.println("[Crypto Usage]");
        System.out.println("java crypto [-algo] [-mode] [-padd] [key] [string] (iv)");
        System.out.println(" 1. [-algo] : -des, -aes, -blowfish");
        System.out.println(" 2. [-mode] : -ecb, -cbc");
        System.out.println(" 3. [-padd] : -null, -pkcs");
    }

    public static void main(String[] args) {
        try {
			// Check arguments
            if (args.length < 5) {
                System.out.println("Input args error! (args.length:" + args.length + " < 6)");
                printUsage();
                return;
            }

            String algo     = checkAlgo(args[0]);
            String mode     = checkMode(args[1]);
            String padd     = checkPadd(args[2]);
            String key      = args[3];
            String planeMsg = args[4];
            String iv       = !(mode.equals("ECB")) ? args[5] : null;

            // Key length check
			if (checkKey(algo, key) == false) {
				return;
			}

            // Encryption
            StringBuilder esb = new StringBuilder();
            esb.append(String.format("\n>> Encrypting... (%s/%s/%s)\n", algo, mode, padd));
            esb.append(String.format(">> Input : [%s]\n", planeMsg));
            esb.append(String.format(">> InHex : %s\n", PrintStreamToHexa(planeMsg.getBytes(Charset))));
            esb.append(String.format(">>  Key  : [%s]\n", key));
            esb.append(String.format(">>  IV   : [%s]\n", iv));
            String cipherMsg = encrypt(algo, mode, padd, key, iv, planeMsg);
            esb.append(String.format(">> Result: [%s]\n", cipherMsg));
            esb.append(String.format(">> ResHex: %s\n", PrintStreamToHexa(cipherMsg.getBytes(Charset))));
            System.out.println(esb.toString());

            // Decryption
            esb.setLength(0);
            esb.append(String.format(">> Decrypting... (%s/%s/%s)\n", algo, mode, padd));
            esb.append(String.format(">> Input : [%s]\n", cipherMsg));
            esb.append(String.format(">> InHex : %s\n", PrintStreamToHexa(cipherMsg.getBytes(Charset))));
            esb.append(String.format(">>  Key  : [%s]\n", key));
            esb.append(String.format(">>  IV   : [%s]\n", iv));
            String rePlaneMsg = decrypt(algo, mode, padd, key, iv, cipherMsg);
            esb.append(String.format(">> Result: [%s]\n", rePlaneMsg));
            esb.append(String.format(">> ResHex: %s\n", PrintStreamToHexa(rePlaneMsg.getBytes(Charset))));
            System.out.println(esb.toString());
        }
        catch (Exception e) {
            System.out.println("> Error! : " + e.getMessage() + "\n");
            System.out.println("> Stack : ");
            e.printStackTrace();
        }

        // System.out.println("=¨Ö¨Í¨â¨Í=======================================================");
    }
}