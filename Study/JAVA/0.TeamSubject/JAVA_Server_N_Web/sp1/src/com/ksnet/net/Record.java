package com.ksnet.net;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;

import com.ksnet.util.*;

public class Record {

	private String typeName;
	private String subTypeName;
	private int index;
	private HashMap<String, Attribute> attrMap;
	
	private int sendCnt;		// 전송 횟수
	private long lastSendTime;	// 마지막 전송 시간
	
	public Record (String typeName, int index, byte[] datas) {
		this(typeName, "", index, datas);
	}
	
	public Record(String typeName, String subTypeName, int index, byte[] datas) {
		this.typeName = typeName;
		this.subTypeName = subTypeName;
		this.index = index;
		this.attrMap = null;
		updateRecord(datas);
		
		this.sendCnt = 0;
		this.lastSendTime = 0;
	}
	
	public String getTypeName() {
		return typeName;
	}
	
	public void setTypeName(String typeName) {
		this.typeName = typeName;
	}
	
	public String getSubTypeName() {
		return subTypeName;
	}
	
	public void setSubTypeName(String subTypeName) {
		this.subTypeName = subTypeName;
	}
	
	public int getIndex() {
		return index;
	}
	
	public void setIndex(int index) {
		this.index = index;
	}
	
	public HashMap<String, Attribute> getAttrMap() {
		return attrMap;
	}
	
	public int getSendCnt() {
		return sendCnt;
	}
	
	public void setSendCnt(int sendCnt) {
		this.sendCnt = sendCnt;
	}
	
	public void addSendCnt(int add) {
		this.sendCnt += add;
	}
	
	public long getLastSendTime() {
		return lastSendTime;
	}
	
	public void setLastSendTime(long lastSendTime) {
		this.lastSendTime = lastSendTime;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	public void setData(String attributeName, final byte[] data) {
		attrMap.get(attributeName).setValue(data);
	}
	
	public byte[] getData(String attributeName) {
		Attribute attr = attrMap.get(attributeName);
		
		if (attr == null) {
			Logger.logln(Logger.LogType.LT_WARN, "\"" + attributeName + "\"을 키로 갖는 attr이 null입니다.");
			return null;
		}
		
		return attrMap.get(attributeName).getValue();
	}
	
	public void setDataByDefault(String attributeName) {
		Attribute attr = attrMap.get(attributeName);
		attr.setValue(attr.getDefaultValue());
	}
	
	public byte[] toByteAry() {
		int totalByteLength = 0;
		int attrSize = attrMap.size();
		Attribute[] attrAry = new Attribute[attrSize];
		
		for (Map.Entry<String, Attribute> entry : attrMap.entrySet()) {
			Attribute attr = entry.getValue();
			int number = attr.getNumber();
			
			attrAry[number - 1] = attr;
			totalByteLength += attr.getByteLength();
		}
		
		byte[] rtByte = new byte[totalByteLength];
		int rtByteI = 0;
		
		for (Attribute attr : attrAry) {
			byte[] attrValue = attr.getValue();
			
			for (int i = 0; i < attrValue.length; ++i) {
				rtByte[rtByteI++] = attrValue[i];
			}
		}
		
		return rtByte;
	}
	
	public static ArrayList<Record> makeRecordList(String name, ArrayList<String> recordStringList) {
		ArrayList<Record> rtList = new ArrayList<Record>();
		int indexCnt = 0;
		
		for (String recordStr : recordStringList) {
			rtList.add(new Record(name, indexCnt++, recordStr.getBytes()));
		}
		
		return rtList;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	private void updateRecord(byte[] datas) {
		if (typeName.equals("HanaRecordClient")) {
			update_hana_record(datas, true);
		}
		else if (typeName.equals("HanaRecordServer")) {
			update_hana_record(datas, false);
		}
	}
	
	private void update_hana_record(byte[] datas, boolean isClientRecord) {
		String indexKeyStr = null;

		// KS Data
		if (isClientRecord) {	
			final String attrName = "HanaAttrClient";
			// 개행 문자 추가
			byte[] copyDatas = null;
			
			if (datas == null) {
				copyDatas = new byte[AttributeManager.getInst().getRecordSizeFromAttributeMap(attrName)];
			}
			else {
				copyDatas = Arrays.copyOfRange(datas, 0, datas.length + 2);
				copyDatas[datas.length] = '\r';
				copyDatas[datas.length + 1] = '\n';
			}
			
			// 표제부, 데이터부, 종료부 구분
			byte idCode = copyDatas[0];

			if (idCode == (byte)'D' || subTypeName.equals("Data")) { // 데이터부
				this.attrMap = AttributeManager.getInst().copyOfAttributeMap(attrName + "_Data");
				this.subTypeName = "Data";
			}
			else if (idCode == (byte)'S' || subTypeName.equals("Head")) { // 표제부
				this.attrMap = AttributeManager.getInst().copyOfAttributeMap(attrName + "_Head");
				this.subTypeName = "Head";
			}
			else if (idCode == (byte)'E' || subTypeName.equals("Tail")) { // 종료부
				this.attrMap = AttributeManager.getInst().copyOfAttributeMap(attrName + "_Tail");
				this.subTypeName = "Tail";
			}
			else { // 오류
				Logger.logln(Logger.LogType.LT_ERR, "알 수 없는 idCode값. (idCode: \"" + idCode + "\")");
				return;
			}
			
			// 속성 맵을 조회하여 바이트 데이터를 필요에 맞게 잘라 저장
			for (Attribute attr : attrMap.values()) {
				int beginIndex = attr.getBeginIndex();
				int byteLength = attr.getByteLength();
				int endIndex = beginIndex + byteLength;

				attr.setValue(Arrays.copyOfRange(copyDatas, beginIndex, endIndex));
			}
			
			indexKeyStr = "d_dataSerialNum"; // 데이터 일련번호 -> 레코드 인덱스
		}
		// Server Data
		else {
			final String attrName = "HanaAttrServer";
			this.attrMap = AttributeManager.getInst().copyOfAttributeMap(attrName);
			
			byte[] copyData = null;
			
			if (datas == null) {
				copyData = new byte[AttributeManager.getInst().getRecordSizeFromAttributeMap(attrName)];
			}
			else {
				copyData = Arrays.copyOfRange(datas, 0, datas.length);
			}
			
			for (Attribute attr : attrMap.values()) {
				int beginIndex = attr.getBeginIndex();
				int byteLength = attr.getByteLength();
				int endIndex = beginIndex + byteLength;
				
				attr.setValue(Arrays.copyOfRange(copyData, beginIndex, endIndex));
			}
			
			indexKeyStr = "h_msgNum"; // 전문번호 -> 레코드 인덱스
		}
		
		// 레코드 인덱스 업데이트 (-1 : 오토 인덱싱)
		if (index == -1) {
			if (indexKeyStr != null) {
				byte[] indexByte = getData(indexKeyStr);
				
				if (indexByte != null) {
					this.index = Integer.parseInt(new String(indexByte));
				}
			}
		}
	}
}