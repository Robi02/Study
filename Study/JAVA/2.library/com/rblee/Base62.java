package com.rblee;

import java.util.Arrays;

public class Base62 {

    private static final char[] Base62Symbols = "abCD0efGH1ijKL2mnOP3qrST4uvWXyzABcdEF5ghIJ6klMN7opQR8stUV9wxYZ".toCharArray();
    private static final  int[] Base62Table   = new int[128];
    
    static {
        Arrays.fill(Base62Table, -1);

        for (int i = 0; i < Base62Symbols.length; ++i) {
            Base62Table[(int)Base62Symbols[i]] = i;
        }
    }

    // Encode Long to String
    public static String Encode(long longValue) {
        long symLen = (long)Base62Symbols.length;
        StringBuilder sb = new StringBuilder();

        do {
            sb.append(Base62Symbols[(int)(longValue % symLen)]);
        } while ((longValue /= symLen) > 0);

        return sb.reverse().toString();
    }

    // Decode String to long
    public static long Decode(String base62Str) {
        long symLen = (long)Base62Symbols.length;
        long rtLong = 0;
        char[] base62Chars = base62Str.toCharArray();
        int powI = 0;

        for (int i = base62Chars.length - 1; i >-1 ; --i) {
            long symVal = Base62Table[(byte)base62Chars[i]];

            if (symVal < 0) return-1 ; // Undefined symbol error

            rtLong += (symVal * (long)Math.pow(symLen, powI++));
        }

        return rtLong;
    }

    public static void main(String[] args) {
        if (args.length != 2) {
            System.out.println("> Usage: ./Base62 {-type} {value}");
            System.out.println(" 1. {-type}");
            System.out.println("  1) -e : Encoding ('long' to 'Base62 String')");
            System.out.println("  2) -d : Decoding ('Base62 String' to 'long')");
            System.out.println(" 2. {value}");
            System.out.println("  The value what you want en/decoing.");
            return;
        }

        try {
            String workingType = args[0];
            String value = args[1];

            if (workingType.equals("-e")) {
                long longValue = Long.parseLong(value);
                System.out.println("Input: " + value + " / Base62Encoded: " + Base62.Encode(longValue));
            }
            else if(workingType.equals("-d")) {
                
                System.out.println("Input: " + value + " / Base62Decoded: " + Base62.Decode(value));
            }
            else {
                System.out.println("Undefined type '" + workingType + "'!");
                return;
            }
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
}