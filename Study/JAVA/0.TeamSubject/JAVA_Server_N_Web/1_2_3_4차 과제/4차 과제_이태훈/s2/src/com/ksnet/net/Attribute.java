package com.ksnet.net;

import java.util.Arrays;

public class Attribute {

	private int number;
	private String name;
	private String codeName;
	private String type;
	private int beginIndex;
	private int byteLength;
	private byte[] defaultValue;
	private byte[] value;
	
	public Attribute(int number, String name, String codeName, String type, int beginIndex, int byteLength, byte[] defaultValue) {
		this.number = number;
		this.name = name;
		this.codeName = codeName;
		this.type = type;
		this.beginIndex = beginIndex;
		this.byteLength = byteLength;
		this.defaultValue = defaultValue;
		
		if (defaultValue == null) {
			this.value = new byte[byteLength];
		}
		else {
			this.value = defaultValue;
		}
	}
	
	public Attribute(Attribute attribute) {
		this.copyFrom(attribute);
	}
	
	public void copyFrom(Attribute attribute) {
		setNumber(attribute.number);
		setName(new String(attribute.name));
		setCodeName(new String(attribute.codeName));
		setType(new String(attribute.type));
		setBeginIndex(attribute.beginIndex);
		setByteLength(attribute.byteLength);
		
		if (attribute.defaultValue != null) {
			setDefaultValue(Arrays.copyOfRange(attribute.defaultValue, 0, attribute.defaultValue.length));
		}
		else {
			attribute.defaultValue = null;
		}
		
		if (attribute.value != null) {
			setValue(Arrays.copyOfRange(attribute.value, 0, attribute.value.length));
		}
		else {
			setValue(null);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	public int getNumber() {
		return number;
	}
	
	public void setNumber(int number) {
		this.number = number;
	}
	
	public String getName() {
		return name;
	}
	
	public void setName(String name) {
		this.name = name;
	}
	
	public String getCodeName() {
		return codeName;
	}
	
	public void setCodeName(String codeName) {
		this.codeName = codeName;
	}
	
	public String getType() {
		return type;
	}
	
	public void setType(String type) {
		this.type = type;
	}
	
	public int getBeginIndex() {
		return beginIndex;
	}
	
	public void setBeginIndex(int beginIndex) {
		this.beginIndex = beginIndex;
	}
	
	public int getByteLength() {
		return byteLength;
	}
	
	public void setByteLength(int byteLength) {
		this.byteLength = byteLength;
	}
	
	public byte[] getDefaultValue() {
		return defaultValue;
	}
	
	public void setDefaultValue(byte[] defaultValue) {
		this.defaultValue = defaultValue;
	}
	
	public byte[] getValue() {
		return value;
	}
	
	public void setValue(final byte[] value) {
		if (value != null) {
			// 길이 체크
			if (value.length > byteLength) {
				// System.out.println("[WARN] : value.length > byteLength 데이터 손실 가능성이 있습니다. (" + value.length + " > " + byteLength + " codeName: [" + codeName + "], value: [" + new String(value) + "])");
			}
		}
			
		// 타입별로 value값 수정
		byte[] valCpy = new byte[byteLength];
		
		if (type.equals("X") || type.equals("C")) { // 공백(' ') 패딩, 좌로 정렬
			Arrays.fill(valCpy, (byte)' ');
			
			if (value != null) {
				for (int i = 0; i < byteLength; ++i) {
					if (i == value.length) break;
					
					valCpy[i] = value[i];
				}
			}
		}
		//else if (type.equals("")) { // 숫자 '0' 패딩, 좌로 정렬
		//	Arrays.fill(valCpy, (byte)'0');
		//	
		//	if (value != null) {
		//		for (int i = 0; i < byteLength; ++i) {
		//			if (i == value.length) break;
		//			
		//			valCpy[i] = value[i];
		//		}
		//	}
		//}
		else if (type.equals("9") || type.equals("N")) { // 숫자 '0' 패딩, 우로 정렬
			Arrays.fill(valCpy, (byte)'0');
			
			if (value != null) {
				int valueI = value.length - 1;
				for (int cpyI = byteLength - 1; cpyI > -1; --cpyI) {
					
					valCpy[cpyI] = value[valueI--];
					
					if (valueI < 0) break;
				}
			}
		}
		else { // 공백(' ') 패딩, 좌로 정렬
			Arrays.fill(valCpy, (byte)' ');
			
			if (value != null) {
				for (int i = 0; i < byteLength; ++i) {
					if (i == value.length) break;
					
					valCpy[i] = value[i];
				}
			}
		}
		
		this.value = valCpy;
	}
}