import java.security.MessageDigest;

public class MD5 {

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
            MessageDigest md5 = MessageDigest.getInstance("MD5");
            byte[] md5Rst = md5.digest(inByte);
            String rstStr = streamToHexa(md5Rst);
            
            System.out.println("> In : " + inStr);
            System.out.println("> Ou : " + rstStr);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
}