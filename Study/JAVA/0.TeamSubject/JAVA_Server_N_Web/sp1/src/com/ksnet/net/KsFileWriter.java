package com.ksnet.net;

import java.io.File;
import java.io.RandomAccessFile;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;

import com.ksnet.util.*;

public class KsFileWriter {

	private String filePath;
	private int bytePerLine;
	private RandomAccessFile randomAccessFile;
		
	public KsFileWriter(String filePath, int bytePerLine) {
		this.filePath = filePath;
		this.bytePerLine = bytePerLine;
	}
	
	public synchronized void write(byte[] str, long pos) {
		try {
			if (randomAccessFile == null) randomAccessFile = new RandomAccessFile(filePath, "rw");
			
			if (pos != -1) randomAccessFile.seek(pos * bytePerLine);
			
			randomAccessFile.write(str);
		}
		catch (IOException ioe) {
			Logger.logln(Logger.LogType.LT_ERR, "OutputStream 열기 혹은 쓰기 오류.");
			Logger.logln(Logger.LogType.LT_ERR, ioe);
		}
	}
	
	public void close() {
		try {
			randomAccessFile.close();
		}
		catch (IOException ioe) {
			Logger.logln(Logger.LogType.LT_ERR, "OutputStream 닫기 오류.");
			Logger.logln(Logger.LogType.LT_ERR, ioe);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	public String getFilePath() {
		return filePath;
	}
	
	public int getBytePerLine() {
		return bytePerLine;
	}
	
	public void setBytePerLine(int bytePerLine) {
		this.bytePerLine = bytePerLine;
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	public static void copyFromFile(File srcFile, File destFile) throws Exception {
		Files.copy(srcFile.toPath(), destFile.toPath(), StandardCopyOption.REPLACE_EXISTING);
	}
}