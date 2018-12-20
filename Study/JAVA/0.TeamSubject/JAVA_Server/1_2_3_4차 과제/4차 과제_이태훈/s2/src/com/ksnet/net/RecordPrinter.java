package com.ksnet.net;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;

import com.ksnet.util.*;

public class RecordPrinter {
	
	private Record record;
	
	// 생성자
	public RecordPrinter(Record record) {
		this.record = record;
	}
	
	public void setRecord(Record record) {
		this.record = record;
	}
	
	// 양식별 출력함수
	// 하나은행_지급이체_표제부
	private void print_hana_record() {
		int attrSize = record.getAttrMap().size();
		Attribute[] attrAry = new Attribute[attrSize];
		
		for (Map.Entry<String, Attribute> entry : record.getAttrMap().entrySet()) {
			Attribute attr = entry.getValue();
			int number = attr.getNumber();
			
			attrAry[number - 1] = attr;
		}
	
		System.out.println(String.format("%s\t%-40s\t%s\t%s\t%-20s", "순번", "이름(코드명)", "시작인덱스", "바이트길이", "현재값"));
		for (Attribute attr : attrAry) {
			System.out.println(String.format("%3d\t%-40s\t%4d\t\t%4d\t\t%-20s", attr.getNumber(), (attr.getName() + String.format("(%s)", attr.getCodeName())), attr.getBeginIndex(), attr.getByteLength(), new String(attr.getValue())));
		}		
	}

	public void print(Record record) {
		setRecord(record);
		print();
	}
	
	public void print() {
		String recordType = record.getTypeName();
		
		// 하나은행_지급이체_표제부_데이터부_종료부 양식 출력
		if (recordType.equals("HanaRecordClient")) {
			String recordSubType = record.getSubTypeName();
			
			if (recordSubType.equals("Head")) {
				System.out.println("=[표제부]=================================================================================");
				print_hana_record();
				System.out.println("=================================================================================[표제부]=");
			}
			else if (recordSubType.equals("Data")) {
				System.out.println("=[데이터부]================================================================================");
				print_hana_record();
				System.out.println("================================================================================[데이터부]=");
			}
			else if (recordSubType.equals("Tail")) {
				System.out.println("=[종료부]=================================================================================");
				print_hana_record();
				System.out.println("=================================================================================[종료부]=");
			}			
		}
		// 원화 펌뱅킹 양식 출력
		else if (recordType.equals("HanaRecordServer")) {
			print_hana_record();
		}
		else if (false) {
			// 여기에 새로운 출력타입 추가...
		}
		else {
			System.out.println(this.getClass().getName());
			Logger.logln(Logger.LogType.LT_ERR, "지원하지 않는 전문 종류. (" + recordType + ")");
			return;
		}
	}
}