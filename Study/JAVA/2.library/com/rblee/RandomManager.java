package com.rblee;

import java.util.Random;

public class RandomManager {
	
	private static final Random RANDOM				= new Random();	// 난수 시드값 유지를 위해 최초 1회만 생성
	private static final String RANDOM_ALPHA		= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	private static final String RANDOM_NUMBER		= "0123456789";
	private static final String RANDOM_ALPHA_NUMBER = RANDOM_ALPHA + RANDOM_NUMBER;
	
	// 옵션에 따라(영문, 숫자, 영문 + 숫자) len길이의 무작위 문자열을 반환
	public static String genRandomStr(int len, String option) {
		StringBuilder sb = new StringBuilder();
		String op = null;
		
		if (option.equals("ALPHA")) {
			op = RANDOM_ALPHA;
		}
		else if (option.equals("NUMBER")) {
			op = RANDOM_NUMBER;
		}
		else if (option.equals("ALPHA_NUMBER")) {
			op = RANDOM_ALPHA_NUMBER;
		}

		// [ https://docs.oracle.com/javase/8/docs/api/index.html?java/util/Random.html ]
		// java.util.Random are thread safe. However, the concurrent use of the same java.util.Random instance
		// across threads may encounter contention and consequent poor performance.
		// -> Random 클래스는 기본적으로 Thread-Safe지만, 여러 쓰레드에서 병행 사용시 낮은 효율성을 보일 수 있음.
		
		int opLen = op.length();
		
		for (int i = 0; i < len; ++i) {
			sb.append(op.charAt(RANDOM.nextInt(opLen)));
		}
		
		return sb.toString();
	}
}