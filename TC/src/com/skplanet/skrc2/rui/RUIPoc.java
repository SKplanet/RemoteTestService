/*
	* The MIT License (MIT)
	* Copyright (c) 2015 SK PLANET. All Rights Reserved.
	*
	* Permission is hereby granted, free of charge, to any person obtaining a copy
	* of this software and associated documentation files (the "Software"), to deal
	* in the Software without restriction, including without limitation the rights
	* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	* copies of the Software, and to permit persons to whom the Software is
	* furnished to do so, subject to the following conditions:
	*
	* The above copyright notice and this permission notice shall be included in
	* all copies or substantial portions of the Software.
	*
	* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	* THE SOFTWARE.
	*/


package com.skplanet.skrc2.rui;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLEncoder;

import com.skplanet.skrc2.utils.AEScrypto;
import com.skplanet.skrc2.utils.Utils;

public class RUIPoc {

	//public static final String POC_ADDRESS = "oic.tstore.co.kr";
	public static final String POC_ADDRESS = "qa.skplanet.co.kr";
	public static final String POC_DEV_STATE_PAGE = "/testlab/front/remoteTest/device/createDeviceLog.action";
	public static final String POC_REGIST_PAGE = "/testlab/front/remoteTest/device/createDevice.action";
	public static final String POC_CLIENT_STATE_PAGE = "/testlab/front/remoteTest/rsrv/createDeviceUseLog.action";
	public static final String POC_RESERVEID_PAGE = "/testlab/front/remoteTest/rsrv/getRtRsrvInfo.action";

	public static String getReservationInfoFromPOC(String reserve_id) {

		if( reserve_id == null || reserve_id.length() == 0 )
			return null;
		
		StringBuffer params = new StringBuffer();

		try {
			params.append("reserve_id=");
			params.append(URLEncoder.encode(reserve_id, "UTF-8"));
		} catch (UnsupportedEncodingException e1) {
			e1.printStackTrace();
		}

		URL sendURL = null;
		OutputStream out;
		try {
			//sendURL = new URL("https", POC_ADDRESS, POC_RESERVEID_PAGE);
			sendURL = new URL("http", POC_ADDRESS, POC_RESERVEID_PAGE);
		} catch (MalformedURLException e) {
			e.printStackTrace();
			return null;
		}

		//HttpsURLConnection connection = null;
		HttpURLConnection connection = null;
		try {
			//connection = (HttpsURLConnection) sendURL.openConnection();
			connection = (HttpURLConnection) sendURL.openConnection();
			connection.setRequestMethod("POST");
			connection.setRequestProperty("Content-length", String.valueOf(params.length()));
			connection.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
			connection.setRequestProperty("charset", "utf8");
			//connection.setRequestProperty("User-Agent", "Mozilla/4.0 (compatible; MSIE 5.0;Windows98;DigExt)"); 

			connection.setDoOutput(true);
			connection.setDoInput(true);
			
			out = connection.getOutputStream();
			out.write(params.toString().getBytes()); 
			out.flush();
			out.close();
		} catch (IOException e) {
			e.printStackTrace();
			return null;
		}

		int nRes;
		String strRes;
		try {
			nRes = connection.getResponseCode();
			strRes = connection.getResponseMessage();
			Utils.LOG("Response(%d:%s)\n", nRes, strRes);
		} catch (IOException e1) {
			e1.printStackTrace();
		}
		
		
		BufferedReader reader = null;
		String sResult = null;
		try {
			reader = new BufferedReader(new InputStreamReader(connection.getInputStream()));
			while((sResult=reader.readLine()) != null){
				sResult = sResult.trim();
				if( sResult.length() > 0 ) {
					Utils.LOG(sResult);
					//Utils.LOG(AEScrypto.DeCrypto(sResult));
					sResult = AEScrypto.deCrypto(sResult);
					break;
				}
			}
			reader.close();
		} catch (IOException e) {
			e.printStackTrace();
			return null;
		}
		
		return sResult;
	}
	
	public static String reportClientStatusToPOC(String reserve_id, int state, int state2, String desc) {

		if( reserve_id == null || reserve_id.length() == 0 )
			return null;
		
		if( reserve_id.length()>8 && reserve_id.substring(8).equalsIgnoreCase(RUIClient.TC_DEVKEY)) {
			return null;
		}
		
		StringBuffer params = new StringBuffer();

		try {
			params.append("reserve_id=");
			params.append(URLEncoder.encode(reserve_id, "UTF-8"));
			params.append("&state=");
			params.append(Integer.toString(state));
			params.append("&state2=");
			params.append(Integer.toString(state2));
			params.append("&description=");
			params.append(URLEncoder.encode(desc, "UTF-8"));
		} catch (UnsupportedEncodingException e1) {
			e1.printStackTrace();
		}

		URL sendURL = null;
		OutputStream out;
		try {
			//sendURL = new URL("https", POC_ADDRESS, POC_CLIENT_STATE_PAGE);
			sendURL = new URL("http", POC_ADDRESS, POC_CLIENT_STATE_PAGE);
		} catch (MalformedURLException e) {
			e.printStackTrace();
			return null;
		}

		//HttpsURLConnection connection = null;
		HttpURLConnection connection = null;
		try {
			//connection = (HttpsURLConnection) sendURL.openConnection();
			connection = (HttpURLConnection) sendURL.openConnection();
			connection.setRequestMethod("POST");
			connection.setRequestProperty("Content-length", String.valueOf(params.length()));
			connection.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
			connection.setRequestProperty("charset", "utf8");
			//connection.setRequestProperty("User-Agent", "Mozilla/4.0 (compatible; MSIE 5.0;Windows98;DigExt)"); 

			connection.setDoOutput(true);
			connection.setDoInput(true);
			
			out = connection.getOutputStream();
			out.write(params.toString().getBytes()); 
			out.flush();
			out.close();
		} catch (IOException e) {
			e.printStackTrace();
			return null;
		}

		int nRes;
		String strRes;
		try {
			nRes = connection.getResponseCode();
			strRes = connection.getResponseMessage();
			Utils.LOG("Response(%d:%s)\n", nRes, strRes);
		} catch (IOException e1) {
			e1.printStackTrace();
		}
		
		
		BufferedReader reader = null;
		String sResult = null;
		try {
			reader = new BufferedReader(new InputStreamReader(connection.getInputStream(), "UTF-8"));
			while((sResult=reader.readLine()) != null){
				sResult = sResult.trim();
				if( sResult.length() > 0 ) {
					Utils.LOG(sResult);
					//break;
				}
			}
			reader.close();
		} catch (IOException e) {
			e.printStackTrace();
			return null;
		}
		
		return sResult;
	}
}
