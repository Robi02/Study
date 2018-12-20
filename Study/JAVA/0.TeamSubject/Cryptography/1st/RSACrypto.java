import java.io.*;
import java.security.*;
import java.security.spec.*;
import javax.crypto.*;

import java.util.Base64;
import java.util.Base64.Encoder;
import java.util.Base64.Decoder;

public class RSACrypto {

    public static String Encrypt(int bit, String plainStr) {
        // 암호화 키 파일로 생성
        PublicKey pubKey = null;
        PrivateKey priKey = null;

        SecureRandom secureRandom = new SecureRandom();
        KeyPairGenerator keyPairGenerator;

        try {
            keyPairGenerator = KeyPairGenerator.getInstance("RSA");
            keyPairGenerator.initialize(bit, secureRandom);

            KeyPair keyPair = keyPairGenerator.genKeyPair();
            pubKey = keyPair.getPublic();
            priKey = keyPair.getPrivate();

            KeyFactory keyFactory = KeyFactory.getInstance("RSA");
            RSAPublicKeySpec rsaPubKeySpec = keyFactory.getKeySpec(pubKey, RSAPublicKeySpec.class);
            RSAPrivateKeySpec rsaPriKeySpec = keyFactory.getKeySpec(priKey, RSAPrivateKeySpec.class);

            System.out.println("\n두 소수의 곱(n=pq) : " + rsaPubKeySpec.getModulus());
            System.out.println("\n공개키 : " + rsaPubKeySpec.getPublicExponent());
            System.out.println("\n두 소수의 곱(n=pq) : " + rsaPriKeySpec.getModulus());
            System.out.println("\n개인키 : " + rsaPriKeySpec.getPrivateExponent());
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        } catch (InvalidKeySpecException e) {
            e.printStackTrace();
        }

        Encoder b64Encoder = null;
        byte[] pubKeyByte = null, priKeyByte = null;
        String pubKeyStr = null, priKeyStr = null;

        try {
            b64Encoder = Base64.getEncoder();
            pubKeyByte = pubKey.getEncoded();
            pubKeyStr = new String(b64Encoder.encode(pubKeyByte), "EUC-KR");
            priKeyByte = priKey.getEncoded();
            priKeyStr = new String(b64Encoder.encode(priKeyByte), "EUC-KR");
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }

        try {
            BufferedWriter pubBw = new BufferedWriter(new FileWriter("PublicKey.txt"));
            pubBw.write(pubKeyStr);
            pubBw.newLine();
            pubBw.close();

            BufferedWriter priBw = new BufferedWriter(new FileWriter("PrivateKey.txt"));
            priBw.write(priKeyStr);
            priBw.newLine();
            priBw.close();
        } catch(IOException e) {
            e.printStackTrace();
        }

        // 암호화 시작
        String cipherBase64 = null;

        try {
            Cipher cipher = Cipher.getInstance("RSA");
            cipher.init(Cipher.ENCRYPT_MODE, pubKey);
            byte[] cipherByte = cipher.doFinal(plainStr.getBytes("EUC-KR"));
            cipherBase64 = new String(b64Encoder.encode(cipherByte), "EUC-KR");
        } catch (NoSuchAlgorithmException | NoSuchPaddingException | InvalidKeyException |
                 IllegalBlockSizeException | BadPaddingException e) {
            e.printStackTrace();
        } finally {
            return cipherBase64;
        }
    }

    public static String Decrypt(String cipherStr) {
        // 암호화 키 로드
        String pubKeyStr = null, priKeyStr = null;
        BufferedReader pubBr = null, priBr = null;

        try {            
            pubBr = new BufferedReader(new FileReader("PublicKey.txt"));
            pubKeyStr = pubBr.readLine();
            priBr = new BufferedReader(new FileReader("PrivateKey.txt"));
            priKeyStr = priBr.readLine();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                if (pubBr != null) { pubBr.close(); }
                if (priBr != null) { priBr.close(); }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        Decoder b64Decoder = null;
        byte[] pubKeyByte = null, priKeyByte = null;
        PublicKey pubKey = null;
        PrivateKey priKey = null;        

        try {
            b64Decoder = Base64.getDecoder();
            pubKeyByte = b64Decoder.decode(pubKeyStr.getBytes("EUC-KR"));
            priKeyByte = b64Decoder.decode(priKeyStr.getBytes("EUC-KR"));
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }

        try {
            KeyFactory keyFactory = KeyFactory.getInstance("RSA");
            X509EncodedKeySpec pubKeySpec = new X509EncodedKeySpec(pubKeyByte);
            pubKey = keyFactory.generatePublic(pubKeySpec);

            PKCS8EncodedKeySpec priKeySpec = new PKCS8EncodedKeySpec(priKeyByte);
            priKey = keyFactory.generatePrivate(priKeySpec);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            e.printStackTrace();
        }

        // 복호화 시작
        String plainStr = null;

        try {
            byte[] cipherByte = b64Decoder.decode(cipherStr.getBytes("EUC-KR"));
            Cipher cipher = Cipher.getInstance("RSA");
            cipher.init(Cipher.DECRYPT_MODE, priKey);
            byte[] plainByte = cipher.doFinal(cipherByte);
            plainStr = new String(plainByte, "EUC-KR"); 
        } catch (NoSuchAlgorithmException | NoSuchPaddingException | InvalidKeyException |
                 IllegalBlockSizeException | BadPaddingException e) {
            e.printStackTrace();
        } finally {
            return plainStr;
        }
    }

    public static void printUsage() {
        System.out.println("\n> RSACrypto Usage: java RSACrypto [Op] [Bit] [Str]");
        System.out.println(" 1. [Op] -e, -d : Encrypt or Decrypt.");
        System.out.println(" 2. [Bit] 1024, 2048 : RSA-1024/2048.");
        System.out.println(" 3. [Str] : \"PlainText or Base64CipherText.\"");
    }

    public static void main(String[] args) {
        if (args.length != 3) {
            System.out.println("> args.lenght Error!");
            printUsage();
            return;
        }

        if (args[0].equals("-e")) {
            System.out.println("\n[ RSA Encrypt ]");
            System.out.println(Encrypt(Integer.parseInt(args[1]), args[2]));
        }
        else if (args[0].equals("-d")) {
            System.out.println("\n[ RSA Decrypt ]");
            System.out.println(Decrypt(args[2]));
        }
        else {
            System.out.println("> Unknown args[0] Error!");
            printUsage();
            return;
        }
    }
}