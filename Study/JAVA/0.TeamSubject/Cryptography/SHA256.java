import java.security.MessageDigest;

public class SHA256 {

    public static String streamToHexa(byte[] inByte) {
        StringBuilder sb = new StringBuilder();

        for (byte b : inByte) {
            sb.append(String.format("%02X ", b));
        }

        return sb.toString();
    }

    public static void main(String[] args) {
        try {
            String inStr = args[0];
            byte[] inByte = inStr.getBytes("EUC-KR");
            MessageDigest sha256 = MessageDigest.getInstance("SHA-256");
            byte[] sha256Rst = sha256.digest(inByte);
            String rstStr = streamToHexa(sha256Rst);
            
            System.out.println("> In : " + inStr);
            System.out.println("> Ou : " + rstStr);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
}