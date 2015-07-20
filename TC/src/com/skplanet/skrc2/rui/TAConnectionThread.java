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
import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.CharBuffer;
import java.nio.channels.SocketChannel;
import java.nio.charset.Charset;

import com.skplanet.skrc2.utils.Utils;


public class TAConnectionThread extends Thread {
	
	public RUIClient tc;
	public boolean connected;
	SocketChannel tcChannel;
	int conn_result = RUIClient.CONN_RES_OK;
	
	public TAConnectionThread(RUIClient c) {
		tc = c;
		connected = false;
	}

	public void run()  {
		super.run();
		Utils.LOG("Client :: connection start:"+tc.tcAddress+"("+tc.tcPort+")");
		
		// set watch timeout 
		WatchThread wt = new WatchThread(this, tc.abortable) {
			@Override
			public void onResult(boolean btimeout, boolean aborted, boolean interrupted) {

				tc.onConnectionResult(btimeout, aborted, connected);
				if(  !aborted && connected == true )
				{
					Utils.LOG("connect validation passed");
				} else {
					RUIPoc.reportClientStatusToPOC(tc.tcReservationID, 9, 1, "Connection failed");
					onStop();
					return;
				}
			}
		};
		
		wt.watch(RUIClient.CONNECTION_TIMEOUT * 1000);

		// 1. do tc connect
		try {
			tcChannel = SocketChannel.open();
		} catch (IOException e) {
			e.printStackTrace();
			tc.tcErr = "Socket Open Failed. (1)";
			tc.onConnectionResult(false, false, false);
			return;
		}
		
		try {
			tcChannel.configureBlocking(false);
			tcChannel.connect(new InetSocketAddress(tc.tcAddress, tc.tcPort));
			tcChannel.socket().setSoTimeout(30000);
		} catch (IOException e) {
			e.printStackTrace();
			tc.tcErr = "Socket Open Failed. (2)";
			tc.onConnectionResult(false, false, false);
			return;
		}
		
        try {
			while (!Thread.interrupted()  && !tc.abortable.isDone() &&  !tcChannel.finishConnect()) {  
			    try {
					Thread.sleep(10);
				} catch (InterruptedException e) {
					e.printStackTrace();
					tc.tcErr = "Socket Open Failed. (3)";
					tc.onConnectionResult(false, false, false);
					return;
				}  
			}
		} catch (IOException e1) {
			e1.printStackTrace();
			tc.tcErr = "Socket Connection failed.";
			tc.onConnectionResult(false, false, false);
			return;
		}
        
        if( Thread.interrupted() || tc.abortable.isDone()) {
        	return;
        }
        
		try {
			tcChannel.configureBlocking(true);
		} catch (IOException e2) {
			e2.printStackTrace();
			
			tc.tcErr = "Set Socket option failed.";
			tc.onConnectionResult(false, false, false);
		}        


        ByteBuffer buffer = ByteBuffer.allocate(1024);  

        //--- blocking I/O
        
        // 2. get server ID
        int nread = 0;
        Charset cs = Charset.forName("UTF-8"); 
		try {
			nread = tcChannel.socket().getInputStream().read(buffer.array(), 0, RUIClient.TA_VERSION_LEN);
		} catch (IOException e1) {
			e1.printStackTrace();
			tc.tcErr = "Illegal Server connection.";
			tc.onConnectionResult(false, false, false);
			return;
		}
        if( nread > 0 )
        {
        	buffer.limit(nread);
            CharBuffer cb = cs. decode(buffer);  
            
            Utils.LOG("TA server (%d) :", nread);  
            while (cb.hasRemaining()) {  
            	Utils.LOG("%c", cb.get());  
            }  
            Utils.LOG("");  
        }
        else {
        	Utils.LOG("no response from server ID (" + nread + ")");
			tc.tcErr = "Server response not found.";
			tc.onConnectionResult(false, false, false);
        	return;
        }
        buffer.clear();
        
        // send client version
        byte vlen = (byte)RUIClient.TC_VERSION.length();
        buffer.put(vlen);
        
        buffer.put(RUIClient.TC_VERSION.getBytes());
        
        vlen = (byte)tc.tcReservationID.length();
        buffer.put(vlen);
        
        buffer.put(tc.tcReservationID.getBytes());
        buffer.flip();
        
        try {
        	//tcChannel.socket().getOutputStream().write(buffer.array());
        	tcChannel.write(buffer);
		} catch (IOException e1) {
			Utils.LOG("send client version failed(" + RUIClient.TC_VERSION + ":" + tc.tcReservationID + ")");
			tc.tcErr = "Server version not match.";
			tc.onConnectionResult(false, false, false);
        	return;
		}
        
        buffer.clear();
        
        // get server grant result
		try {
			nread  = tcChannel.socket().getInputStream().read(buffer.array(), 0, 1);
		} catch (IOException e1) {
			e1.printStackTrace();
			tc.tcErr = "Server grant code not received.";
			tc.onConnectionResult(false, false, false);
			return;
		}

		conn_result = RUIClient.CONN_RES_INVALID_UNKNOWN;
		if( nread > 0 )
        {
        		byte bResult = buffer.get();
        	
        		conn_result = (int)bResult;
        		Utils.LOG("TA server response(" + conn_result +")");  
        }
		
		if( conn_result != RUIClient.CONN_RES_OK) {
			tc.onConnResFromServer(conn_result);
	 	   return;
		}
		
        buffer.clear();

        // get server valid time
		try {
			nread = tcChannel.socket().getInputStream().read(buffer.array(), 0, 4);
		} catch (IOException e1) {
			e1.printStackTrace();
			tc.tcErr = "Server grant timestamp not received. (1)";
			tc.onConnectionResult(false, false, false);
			return;
		}
        if( nread > 0 )
        {
        	buffer.order(ByteOrder.LITTLE_ENDIAN);
        	int gI = buffer.getInt();
        	long ltime = gI & 0xffffffffL;
        	Utils.LOG("getTime(%x), to long(%x)\n", gI, ltime);
        	int hour = (int) ((ltime >>> 11) & 0x1f);
        	int min = (int) ((ltime >>> 5) & 0x3f);
        	int sec = (int) ((ltime) & 0x3f) * 2;
        	tc.tcService.setServerValidTime(sec + min * 60 + hour *3600 -1);
        	Utils.LOG("TA server response " + hour + ":" + min + ":" + sec);  
            
        }
        else {
        	Utils.LOG("TA server response  valid time failed (" + nread + ")");
			tc.tcErr = "Server grant timestamp not received.(2)";
			tc.onConnectionResult(false, false, false);
			return;
        }

        //--- non-blocking I/O
        try {
        	tcChannel.configureBlocking(false);
		} catch (IOException e) {
			e.printStackTrace();
			tc.tcErr = "Set socket option failed (2)";
			tc.onConnectionResult(false, false, false);
			return;
		}

        Utils.LOG("Client :: connected");  
        connected = true;
        
        tc.tcSock = tcChannel;
        
        // 
        tc.tcService.start();

		WatchThread rt = new WatchThread(tc.tcService, tc.abortable) {
			@Override
			public void onResult(boolean btimeout, boolean aborted, boolean interrupted) {
				if( !interrupted && tc.tcService.svrDisconnectNotify >= 0)
				{
					aborted = true;
				}
				tc.onDisconnectResult(btimeout, aborted, interrupted);
				onStop();
			}
		};
		
		rt.watch(0);	// wait for end of thread
		Utils.LOG("TAConnectionThread done");
	}
	
	public void onStop() {
		tc.abortable.done = true;
	}
}
