package com.ksnet.net;

import java.io.File;
import java.io.FileReader;
import java.io.BufferedReader;
import java.io.IOException;
import java.util.HashMap;
import java.util.ArrayList;

import com.ksnet.util.*;

public class KsFileReader {

	private String filePath;
	private File file = null;
	private FileReader fileReader = null;
	private BufferedReader bufferedReader = null;
	
	public KsFileReader(String filePath) {
		this.filePath = filePath;
	}
	
	public String readLine() {
		String rtString = null;
		
		try {
			if (file == null) file = new File(filePath);
			if (fileReader == null) fileReader = new FileReader(file);
			if (bufferedReader == null) bufferedReader = new BufferedReader(fileReader);

			if ((rtString = bufferedReader.readLine()) == null) {
				try {				
					if (bufferedReader != null) bufferedReader.close();
					if (fileReader != null) fileReader.close();
				}
				catch (IOException ioe1) {
					Logger.logln(Logger.LogType.LT_ERR, ioe1);
				}
			}
		}
		catch (IOException ioe2) {
			ioe2.printStackTrace();
		}
		finally {
			return rtString;
		}
	}
	
	public ArrayList<String> readLines() {
		ArrayList<String> rtList = new ArrayList<String>();
		
		try {
			if (file == null) file = new File(filePath);
			if (fileReader == null) fileReader = new FileReader(file);
			if (bufferedReader == null) bufferedReader = new BufferedReader(fileReader);
			String lineData = "";
			
			while ((lineData = bufferedReader.readLine()) != null) {
				rtList.add(new String(lineData));
			}
		}
		catch (IOException ioe1) {
			Logger.logln(Logger.LogType.LT_ERR, ioe1);
		}
		finally {
			try {				
				if (bufferedReader != null) bufferedReader.close();
				if (fileReader != null) fileReader.close();
			}
			catch (IOException ioe2) {
				Logger.logln(Logger.LogType.LT_ERR, ioe2);
			}
			
			return rtList;
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////
	
	public String getFilePath() {
		return filePath;
	}
	
	public void setFilePath(String filePath) {
		this.filePath = filePath;
	}
	
	public void close() {
		try {
			if (bufferedReader != null) bufferedReader.close();
			if (fileReader != null) fileReader.close();
		}
		catch (IOException ioe) {
			Logger.logln(Logger.LogType.LT_ERR, ioe);
		}
	}
}