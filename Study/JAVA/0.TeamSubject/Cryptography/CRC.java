import java.util.zip.CRC32;
import java.util.Arrays;

public class CRC {

    public static String streamToHexa(byte[] inByte) {
        StringBuilder sb = new StringBuilder();

        for (byte b : inByte) {
            sb.append(String.format("%02X ", b));
        }

        return sb.toString();
    }

    public static String longToBinary(long in) {
        StringBuilder sb = new StringBuilder();
        long mod = 0;

        while (in != 0) {
            mod = in % 2;
            in /= 2;
            sb.append(mod);
        }

        sb.reverse();
        return sb.toString();
    }

    public static byte[] longToByteArray(long value) {
        return new byte[] {
            (byte) (value >> 56),
            (byte) (value >> 48),
            (byte) (value >> 40),
            (byte) (value >> 32),
            (byte) (value >> 24),
            (byte) (value >> 16),
            (byte) (value >> 8),
            (byte) value
        };
    }

    public static void main(String[] args) {
        if (args.length != 1) {
            System.out.println("Input String!\n");
            return;
        }

        try {
            // CRC generate
            System.out.println("\n[CRC32-Generate]");
            String inStr = args[0];
            byte[] inByte = inStr.getBytes("EUC-KR");
            System.out.println("- Input(String): " + inStr);
            System.out.println("- Input(Stream): " + streamToHexa(inByte));

            CRC32 crc32 = new CRC32();
            crc32.update(inByte);
            long result = crc32.getValue();
            byte[] rstByte = longToByteArray(result);
            System.out.println("> Result(CRC32-Bin ): " + longToBinary(result));
            System.out.println("> Result(CRC32-Hexa): " + streamToHexa(rstByte));
            System.out.println("> Result(CRC32-Long): " + result);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
}
