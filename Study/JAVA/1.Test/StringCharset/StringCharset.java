public class StringCharset {
    // Charset (CS)
    public static final String CS_EUC_KR = "EUC-KR";
    public static final String CS_8859_1 = "ISO-8859-1";
    public static final String CS_UTF8   = "UTF-8";
    public static final String CS_UTF16  = "UTF-16";
    // Test String
    public static final String TEST_CASE_EN   = "10ȯ���� ���/�ް���";
    public static final String TEST_CASE_KR   = "�ȳ� ����!";
    public static final String TEST_CASE_ENKR = "Hello ����!";
    // System Charset (Encoding)
    public static final String SYSTEM_CS = System.getProperty("sun.jnu.encoding");

    // Byte to Hex
    public static String cvtByteToHexaStr(byte[] inByte) {
        StringBuilder sb = new StringBuilder();

        for (byte b : inByte) {
            sb.append(String.format("%02X ", b));
        }

        return sb.deleteCharAt(sb.length() - 1).toString();
    }
    // Print result
    public static void printInOu(String inStr, String inCharset, byte[] inByte, String ouCharset) {
        String inCs  = (inCharset != null ? inCharset : SYSTEM_CS);
        String ouCs  = (ouCharset != null ? ouCharset : SYSTEM_CS);
        String ouStr = cvtByteToHexaStr(inByte);

        System.out.println(String.format("[InStr] : %s (%s)", inStr, inCs));
        System.out.println(String.format("[OuHex] : %s (%s)", ouStr, ouCs));
        System.out.println();
    }
    // Main
    public static void main(String[] args) {
        try {
            String str_java    = new String(TEST_CASE_EN);

            byte[] byte_java   = str_java.getBytes();           // getBytes()�� �⺻������ �ý��� ����Ʈ ü������� ����(�� �Ǿ�: MS949)
            byte[] byte_euckr  = str_java.getBytes(CS_EUC_KR);  // Charset������ byte�迭�� �̾Ƴ�
            byte[] byte_utf8   = str_java.getBytes(CS_UTF8  );
            byte[] byte_utf16  = str_java.getBytes(CS_UTF16 );
            byte[] byte_8859_1 = str_java.getBytes(CS_8859_1);

            String str_java2  = new String(byte_java);
            String str_euckr  = new String(byte_euckr,  CS_EUC_KR); // ��ǲ byte�迭�� Charset�� �˷���
            String str_utf8   = new String(byte_utf8,   CS_UTF8  );
            String str_utf16  = new String(byte_utf16,  CS_UTF16 );
            String str_8859_1 = new String(byte_8859_1, CS_8859_1);

            printInOu(str_java2,  null,      byte_java,   null     );
            printInOu(str_euckr,  CS_EUC_KR, byte_euckr,  CS_EUC_KR);
            printInOu(str_utf8,   CS_UTF8,   byte_utf8,   CS_UTF8  );
            printInOu(str_utf16,  CS_UTF16,  byte_utf16,  CS_UTF16 );
            printInOu(str_8859_1, CS_8859_1, byte_8859_1, CS_8859_1);

            // TEST
            System.out.println("\nTEST BEGIN\n");
            String testStr = new String(str_java.getBytes(CS_8859_1), CS_UTF8);
            printInOu(testStr, CS_UTF8, byte_8859_1, CS_8859_1);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
}