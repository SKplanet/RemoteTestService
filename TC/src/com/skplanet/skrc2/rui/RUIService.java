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

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.text.SimpleDateFormat;
import java.util.Iterator;
import java.util.Locale;

import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.filechooser.FileNameExtensionFilter;

import com.skplanet.skrc2.utils.Settings;
import com.skplanet.skrc2.utils.Utils;




public class RUIService extends Thread {
    public static final int MAX_BUFFER = 8192;
	public RUIClient tc;
	private VideoService video;
	private File fCaptureFile;
	
	public int fbcWidth;
	public int fbcHeight;
	public int fbcBPP;
	public String devBuildManufacture ="";
	public String devBuildModel ="";
	public String devBuildSerial ="";
	public String devBuildVersionRelease ="";
	public String devBuildSDKInt ="";
	private byte[] fbcBuffer;
	private long serverValidTime = 0;
	public int svrDisconnectNotify = -1;
	
	private long lFrameSum;
	private long lFrameTimeStamp;
	
	public TAState m_devState;
	
	private boolean soundOn = false;
    /** 
     *  
     * @param abortable 
     * @param host 
     * @param port 
     */  
    public RUIService(RUIClient rc) {  
       tc = rc;
       fCaptureFile = null;
       fbcWidth = 480;
       fbcHeight= 800;
       fbcBPP = 32;
       m_devState = new TAState();
       lFrameSum = 0;
       lFrameTimeStamp = 0;
       svrDisconnectNotify = -1;
       
    }  



    @Override  
    public void run() {  
        super.run();  
        video = new VideoService(tc);
        video.start();
        
        boolean done = false;  
        Selector selector = null;
        fbcBuffer = new byte[1024];
        
        try {
        	this.getScreenInfo();
        	this.getDeviceInfo();
		} catch (TransportException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		} catch (IOException e2) {
			// TODO Auto-generated catch block
			e2.printStackTrace();
		}
        try {  
            tc.onConnected();
            Utils.LOG("RUIService Thread :: started");  
            
            //videoThread.start();
            
            selector = Selector.open();  
            tc.tcSock.register(selector, SelectionKey.OP_READ);  
            
            this.lFrameSum = 0;
            this.lFrameTimeStamp = System.currentTimeMillis();
            ByteBuffer buffer = ByteBuffer.allocate(MAX_BUFFER);
            int bodylength = 0;
            RUICommand rcmd = new RUICommand();
            rcmd.cmd = 0;
            while (!Thread.interrupted() && !tc.abortable.isDone() && !done) {  
                  
                selector.select(100);
                Iterator<SelectionKey> iter = selector.selectedKeys().iterator();
                if( iter.hasNext() == false)
                {
                	//System.out.println("iter.hasnet is false");
                }
                
                while (!Thread.interrupted() && !tc.abortable.isDone() && !done && iter.hasNext()) {  
                    SelectionKey key = iter.next();
                    if( key.isReadable()) {
                        int len = tc.tcSock.read(buffer);  
                        if (len <  0) {  
                        	Utils.LOG("Client :: server closed");  
                            done = true;  
                            break;  
                        }
                        if( rcmd.cmd == 0 && buffer.position() < 12 ) { // at least 12 byte to process
                        	continue;
                        }
                        
                    	buffer.flip();
                        if( rcmd.cmd == 0 ) {
                        	rcmd.cmd = buffer.getInt();
                        	rcmd.idx = buffer.getInt();
                        	rcmd.lsize = buffer.getInt() - 12;
                        	rcmd.body = fbcBuffer;
                        	bodylength = 0;
                        }

                        //Utils.LOG("cmd:%x size:%d\n", rcmd.cmd, rcmd.lsize);
                        int nRead =  (rcmd.lsize - bodylength) < buffer.remaining() ? (rcmd.lsize - bodylength) : buffer.remaining();
                        buffer.get(fbcBuffer, bodylength, nRead );
                        bodylength += nRead;
                        
                        if( bodylength < rcmd.lsize ) {
                        	buffer.clear();
                        	continue;
                        }
                        
                        //buffer.flip();
                        
                        if( processRUICMD(rcmd) == false) {
                        	done = true;
                        	break;
                        }
                        
                        rcmd.cmd = 0;
                        
                        //buffer.position(cmdlength);
                        buffer.compact();  
                    }
                } 
            }  
        }catch (RuntimeException e) {
        	throw e;
        } catch (Exception e) {  
            e.printStackTrace();  
        } finally {  
              
            if (tc.tcSock != null) {  
                try {  
                	tc.tcSock.socket().close();  
                	tc.tcSock.close();  
                	tc.tcSock = null;
                } catch (IOException e) {  
                    e.printStackTrace();  
                }  
            }  
              
            Utils.LOG("RUIService :: done");  
        }  
          
    }  

    /*
     * RUI Command
     */
	public void writeRUICMD(int cmd, int idx, byte[] body) throws TransportException, IOException {
		int length = 12;
		if( body != null )
			length += body.length;

		ByteBuffer buffer = ByteBuffer.allocate(length);
		buffer.putInt(cmd);
		buffer.putInt(idx);
		buffer.putInt(length);
		if( body != null ) {
			buffer.put(body);
		}
		buffer.flip();
		
		write(buffer);
	}
	
	public void setServerValidTime(long ltime) {
		serverValidTime = ltime;
	}
	
	public long getServerValidTime() {
		return serverValidTime;
	}
	
	public boolean getSoundOn() {
		return soundOn;
	}
	public void setAudioStream(boolean on) throws TransportException, IOException
	{
		if( soundOn != on) {
			soundOn = on;
			int idx = 0;
			if( on )
				idx = 0x00000010;
	
			writeRUICMD(RUICommand.REQ_SET_STREAMING_MODE, idx, null);
		}
	}

	
    private boolean processRUICMD(RUICommand rcmd)  {

		switch (rcmd.cmd )
		{
			case RUICommand.RES_SCREENBUF:
			{
				//System.out.println("RUICommand.RES_SCREENBUF:" + rcmd.lsize);
				return onScreenBuf(rcmd);
			}
			case RUICommand.RES_SCREENINFO:
			{
				//System.out.println("RUICommand.RES_SCREENINFO"); return true;
				return onScreenInfo(rcmd);
			}		
			case RUICommand.SVR_STATECHANGED:
			{
				//System.out.println("RUICommand.SVR_STATECHANGED"); return true;
				return onStateChanged(rcmd);
			}
			case RUICommand.RES_GETBUILD_MANUFACTURE:
			case RUICommand.RES_GETBUILD_MODEL:
			case RUICommand.RES_GETBUILD_SERIAL:
			case RUICommand.RES_GET_BUILD_VERSION_RELEASE:
			case RUICommand.RES_GET_BUILD_VERSION_SDK_INT:
			{
				//System.out.println("RUICommand.RES_GETBUILD_MANUFACTURE"); return true;
				return onDeviceInfo(rcmd);
			}
			case RUICommand.RES_LOGCAT:
			{
				return onLogcat(rcmd);
			}
			case RUICommand.RES_APP_INST:
			{
				return onResAppInst(rcmd);
			}
			case RUICommand.RES_FILE_LIST:
			{
				return onResFileList(rcmd);
			}
			case RUICommand.RES_FILE_DOWNLOAD:
			{
				return onResFileDownload(rcmd);
			}
			case RUICommand.SVR_FILE_ERROR_MSG:
			{
				return svrFileErrorMessage(rcmd);
			}
			case RUICommand.SVR_DISCONNECT:
				return onServerDisconnectNotify(rcmd);
		}

    	return true;
    }

    public boolean onScreenBuf(RUICommand rcmd) {
    	this.lFrameSum++;
    	if( this.lFrameSum % 100 == 0) {
    		long milSum = System.currentTimeMillis() - this.lFrameTimeStamp;
    		
    		float fps = 100000f / (float)milSum;
    		Utils.LOG("FPS:"+fps);
    		
    		
    		this.lFrameTimeStamp = System.currentTimeMillis();
    	}
    	if( fCaptureFile != null && rcmd.idx != 0) {
    		FileOutputStream fos;
			try {
	    		tc.tcStatus.setText("Screen captured : " + fCaptureFile.getCanonicalPath());
				fos = new FileOutputStream(fCaptureFile);
	    		//fos.write(rcmd.body);
	    		fos.write(rcmd.body, 0, rcmd.lsize);
	    		fos.close();
	    		
			} catch (FileNotFoundException e) {
	    		tc.tcStatus.setText("Screen capture failed : file not found");
				e.printStackTrace();
			} catch (IOException e) {
	    		tc.tcStatus.setText("Screen capture failed : write failed");
				e.printStackTrace();
			}
    		fCaptureFile = null;
    	}
		tc.onScreenBuf(rcmd.body);
		try {
			reqScreenBuffer(false);
		} catch (TransportException e) {
			e.printStackTrace();
			return false;
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}
		return true;
    }
    
    public boolean onScreenInfo(RUICommand rcmd) {
    	
    	ByteBuffer buffer = ByteBuffer.wrap(rcmd.body);
    	
    	fbcWidth = (int)buffer.getShort() & 0xffff;
    	fbcHeight = (int)buffer.getShort() & 0xffff;
    	fbcBPP = (int)buffer.getShort() & 0xffff;
    	
    	fbcBuffer = new byte[fbcWidth * fbcHeight * 4];

    	Utils.LOG(String.format("ScreenInfo %dx%d, %dBPP", fbcWidth, fbcHeight, fbcBPP));
		try {
			reqScreenBuffer(false);
		} catch (TransportException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return true;
    }
    
    public boolean onStateChanged(RUICommand rcmd) {
    	
    	ByteBuffer buffer = ByteBuffer.wrap(rcmd.body);

    	m_devState.setState( buffer.getShort() );
    	
    	tc.onTAStateChanged(m_devState);
    	return true;
    }
    
    public boolean onDeviceInfo(RUICommand rcmd) {
    	
			switch( rcmd.cmd ) {
				case RUICommand.RES_GETBUILD_MANUFACTURE:
						devBuildManufacture = new String(rcmd.body, 0, rcmd.lsize);
					break;
				case RUICommand.RES_GETBUILD_MODEL:
					devBuildModel = new String(rcmd.body, 0, rcmd.lsize);
					break;
				case RUICommand.RES_GETBUILD_SERIAL:
					devBuildSerial = new String(rcmd.body, 0, rcmd.lsize);
					break;
				case RUICommand.RES_GET_BUILD_VERSION_RELEASE:
					devBuildVersionRelease = new String(rcmd.body, 0, rcmd.lsize);
					break;
				case RUICommand.RES_GET_BUILD_VERSION_SDK_INT:
					devBuildSDKInt = new String(rcmd.body, 0, rcmd.lsize);
					tc.onDeviceInfoUpdate();
					break;
				default:
						break;
	    	}
		return true;
    }

    // Server Command
    public void getScreenInfo() throws TransportException, IOException {
    	writeRUICMD(RUICommand.REQ_SCREENINFO,0,null);
    }
    
    public void getDeviceInfo() throws TransportException, IOException {
    	writeRUICMD(RUICommand.REQ_GETBUILD_MANUFACTURE, 0, null);
    	writeRUICMD(RUICommand.REQ_GETBUILD_MODEL, 0, null);
    	writeRUICMD(RUICommand.REQ_GETBUILD_SERIAL, 0, null);
    	writeRUICMD(RUICommand.REQ_GET_BUILD_VERSION_RELEASE, 0, null);
    	writeRUICMD(RUICommand.REQ_GET_BUILD_VERSION_SDK_INT, 0, null);
    }
    
    public void reqScreenBuffer(boolean bFull) throws TransportException, IOException {
    	//System.out.println("reqScreenBuffer:"+bFull);
    	writeRUICMD(RUICommand.REQ_SCREENBUF, bFull ? 1 : 0, null);
    }

	public void mouseEvent(int x, int y, int mode) throws IOException {

		ByteBuffer buffer = ByteBuffer.allocate(12+8);
		buffer.putInt(RUICommand.CLI_MOUSEEVENT);
		buffer.putInt(0); // indx
		buffer.putInt(12 + 6);
		buffer.putShort((short)(x & 0xffff));
		buffer.putShort((short)(y & 0xffff));
		buffer.putShort((short)mode);
		buffer.flip();
		
		write(buffer);
	}

	public synchronized int  write(ByteBuffer buffer) throws IOException {
		
		int w = 0;
		int length = buffer.limit();
		long timeOut = System.currentTimeMillis();
		if( length == 0 ) 
			return 0;
		int nDelay = 0;
		if( tc.tcSock == null || !tc.tcSock.isConnected() || tc.abortable.isDone()) {
			return 0;
		}
		while( w < length && tc.tcSock.isConnected()) {
			int n = tc.tcSock.write(buffer);
			if( n == -1 || (System.currentTimeMillis() - timeOut) > Settings.connectionTimeout) {
				break;
			}
			
			w += n;
			//if( w < length ) {
				nDelay++;
		}

		if( w < length )
			throw new IOException();
		
		if( nDelay > 1 )  {
			Utils.LOG("Write Delay : %dmsec\n", nDelay);
		}
		return w;
	}
	public void keyEvent(int code, int mode) throws IOException {

		ByteBuffer buffer = ByteBuffer.allocate(12+8);
		buffer.putInt(RUICommand.CLI_KEYEVENT);
		buffer.putInt(0); // indx
		buffer.putInt(12 + 6);
		buffer.putInt(code);
		buffer.putShort((short)mode);
		buffer.flip();
		
		write(buffer);
	}
	
	public void wakeUP() throws TransportException, IOException {
		reqScreenBuffer(false);
		writeRUICMD(RUICommand.CLI_WAKEUP,0,null);
	}
	
	public void screenCapture() {
        JFileChooser fc = new JFileChooser();
		FileNameExtensionFilter filter = new FileNameExtensionFilter(
                "JPG Images", "jpg");  //description,......확장자
        fc.setFileFilter(filter);    //필터 셋팅
        fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
		try{  
			String homePath = Settings.getWorkingDirectory(); //System.getProperty("user.home");
			//SimpleDateFormat formatter = new SimpleDateFormat ( "yyyyMMddkkmmssSSSS", Locale.KOREA );
			SimpleDateFormat formatter = new SimpleDateFormat ( "yyyyMMddkkmmss", Locale.KOREA );
			java.util.Date currentTime = new java.util.Date();
			String dTime = formatter.format ( currentTime );
			
			String name = String.format("%s/%s-%s.jpg", homePath, devBuildModel, dTime);
					
            //create a file object containing the cannonical path of the desired file   
            //File f = new File(new File("x.html").getCanonicalPath());  
            File f = new File(new File(name).getCanonicalPath());  
            //set the selected file  
            
            fc.setSelectedFile(f);  
        }catch (IOException ex3){  
        }
		int retVal = fc.showSaveDialog(tc.uiWindow);
		if (retVal == JFileChooser.APPROVE_OPTION) {  
            //get the currently selected file  
            //File thefile = fc.getSelectedFile();  
            fCaptureFile= fc.getSelectedFile();
            String nameOfFile = "";  
            nameOfFile = fCaptureFile.getPath();
            Utils.LOG("ToSave:"+nameOfFile);
            try {
				Settings.setWorkingDirectory(fc.getCurrentDirectory().getCanonicalPath());
			} catch (IOException e1) {
				e1.printStackTrace();
			}
            try {
				reqScreenBuffer(true);
			} catch (TransportException e) {
				Utils.LOG("Scareen buffer request (full size) failed.");
				e.printStackTrace();
			} catch (IOException e) {
				Utils.LOG("Scareen buffer request (full size) failed.");
				e.printStackTrace();
			}
		}
	}
	
	/*
	 * String tag : tag string
	 * priority : 0 - Verbose
	 * 				1 - Debug
	 * 				2 - Info
	 * 				3 - Warning
	 * 				4 - Error
	 * 				5 - Fatal
	 * 			6 - stop
	 */
	public void getLogcat(String tag, int priority) {
		String buf;
		buf = "";
		if( priority < 6 ) {
			if( tag.isEmpty() || tag.equals("")) {
				buf = "*:";
			} else {
				buf = "\"" + tag + "\":";
			}
			switch(priority) {
				case 1: buf = buf.concat("D"); break;
				case 2: buf = buf.concat("I"); break;
				case 3: buf = buf.concat("W"); break;
				case 4: buf = buf.concat("E"); break;
				case 5: buf = buf.concat("F"); break;
				default: buf = buf.concat("V"); break;
			}
			
			if( !tag.isEmpty() && !tag.equals("")) {
				buf = buf.concat(" *:S");
			}
		}
		
		try {
			writeRUICMD(RUICommand.REQ_LOGCAT,0, buf.getBytes());
		} catch (TransportException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
    public boolean onLogcat(RUICommand rcmd) {
    	
    	String log = new String(rcmd.body, 0, rcmd.lsize);

    	tc.tcLogcat.onLogcat(log);
		return true;
    }
    
    public void setRotate(int rotate) {
    	byte[] buf = new byte[1];
    	
    	buf[0] = (byte)rotate;
    	try {
			writeRUICMD(RUICommand.CLI_ROTATE, 0, buf);
		} catch (TransportException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
    }
    
    public void removeUserPackage() {
    	try {
			writeRUICMD(RUICommand.CLI_REMOVE_USER_APP,0, null);
		} catch (TransportException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
    }
    
    public void reqAppInstall(String apkname, int bStart) {
    	try {
			writeRUICMD(RUICommand.REQ_APP_INST, bStart, null);
		} catch (TransportException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
    }
    
    public void sendAppInstallData(byte[] data, int size) {
		ByteBuffer buffer = ByteBuffer.allocate(12+ 4 +size);
		buffer.putInt(RUICommand.CLI_APP_INST_DATA);
		buffer.putInt(0); // indx
		buffer.putInt(12 + 4 + size);
		buffer.putShort((short)size);
		buffer.putShort((short)0);
		if( size != 0 ) {
			buffer.put(data, 0, size);
		}
		buffer.flip();
		
		try {
			write(buffer);
		} catch (IOException e) {
			e.printStackTrace();
		}
    }
    
    public boolean onResAppInst(RUICommand rcmd) {
    	
    	int nResult = rcmd.idx;
    	String msg = "";
    	int msgType = JOptionPane.ERROR_MESSAGE;
    	if( rcmd.lsize > 0) {
    		msg  = new String(rcmd.body, 0, rcmd.lsize);
    	}
    	
    	boolean bShow = false;
		if( nResult  == 1 ) {
			bShow = true;
			msgType = JOptionPane.PLAIN_MESSAGE;
			if( rcmd.lsize == 0 ) {
				msg = "App Install Success.";
			}
		}
		else if ( nResult == 2 ) {
			bShow = true;
			if( rcmd.lsize == 0 ) {
				msg = "App Install Failed (2:File creation failed.)";
			}
		}
		else if ( nResult == 3 ) {
			bShow = true;
			if( rcmd.lsize == 0 ) {
				msg = "App Install Failed (3)";
			}
		}
		else if ( nResult == 4 ) {
			if( rcmd.lsize == 0 ) {
				msg = "App Install Failed (4:User Cancelled)";
			}
		}
		else {
			bShow = true;
			msg = "Unknown response from server. (App Install)";
		}

		if( bShow )
			JOptionPane.showMessageDialog(null, msg, RUIClient.TC_PRODUCT_NAME, msgType);

		return true;
    }
    
    public void reqFileUpload(String filename) {
    	filename = filename.replace(File.separatorChar, '/');
    	byte[] bfilename;
    	try {
    		bfilename = filename.getBytes("UTF-8");
		} catch (UnsupportedEncodingException e1) {
			e1.printStackTrace();
			return;
		}
		int length = bfilename.length; //filename.length();

		ByteBuffer buffer = ByteBuffer.allocate(12 + 8 + length);
		buffer.putInt(RUICommand.CLI_FILE_UPLOAD);
		buffer.putInt(0);
		buffer.putInt(12+ 8 + length);
		
		buffer.putShort((short)length);
		buffer.putShort((short)0);
		buffer.putInt(0);
		//buffer.put(filename.getBytes());
		buffer.put(bfilename);
		
		buffer.flip();
		
		try {
			write(buffer);
		} catch (IOException e) {
			e.printStackTrace();
		}
    }
    
    public void reqFileUploadData(int size, byte[] data, int nTime) {

		ByteBuffer buffer = ByteBuffer.allocate(12 + 4 + size);
		buffer.putInt(RUICommand.CLI_FILE_UPLOAD_DATA);
		buffer.putInt(0);
		buffer.putInt(12+ 4 + size);
		
		if( nTime > 0) {
			buffer.putShort((short)0);
			buffer.putShort((short)0);
			buffer.putInt((int)(nTime&0xffffffffL));
		}
		else {
			buffer.putShort((short)size);
			buffer.putShort((short)0);
			buffer.put(data, 0, size);
		}
		
		buffer.flip();
		
		try {
			write(buffer);
		} catch (IOException e) {
			e.printStackTrace();
		}
    }
    
    public void reqFileList(String pathname) {
    	pathname = pathname.replace(File.separatorChar, '/');
        if( pathname.length() > 1 && pathname.endsWith("/") ) {
        	pathname = pathname.substring(0, pathname.length() - 1);
        }
        byte[] bpathname;
        try {
        	bpathname = pathname.getBytes("UTF-8");
		} catch (UnsupportedEncodingException e1) {
			e1.printStackTrace();
			return;
		}
		int length = bpathname.length; //pathname.length();

		ByteBuffer buffer = ByteBuffer.allocate(12 + 4 + length);
		buffer.putInt(RUICommand.REQ_FILE_LIST);
		buffer.putInt(0);
		buffer.putInt(12+ 4 + length);
		
		buffer.put((byte)0);	// flag - always 0
		buffer.put((byte)0); // dummy
		buffer.putShort((short)length);
		//buffer.put(pathname.getBytes());
		buffer.put(bpathname);
		
		buffer.flip();
		
		try {
			write(buffer);
		} catch (IOException e) {
			e.printStackTrace();
		}

    }
    
	public boolean onResFileList(RUICommand rcmd) {
    	ByteBuffer buffer = ByteBuffer.wrap(rcmd.body);

    	//byte flag = buffer.get();
    	buffer.get();
    	buffer.get();		// skip dummy
    	buffer.getShort();// skip dummy
    	
    	int numFiles = (int)buffer.getShort() & 0xffff;
    	//int dataSize = (int)buffer.getShort() & 0xffff;
    	buffer.getShort();

    	int[] sizearray = new int[numFiles];
    	int[] datearray = new int[numFiles];
    	for(int i =0; i < numFiles; i++) {
    		sizearray[i] = (int) (buffer.getInt() & 0xffffffffL);
    		datearray[i] = (int) (buffer.getInt() & 0xffffffffL);
    	}
    	String[] filenames = new String[numFiles];
    	byte[] strbuffer = new byte[1024];
    	
    	int len = 0;
    	int strcnt = 0;
    	while(strcnt < numFiles) {
    		byte b = buffer.get();
    		strbuffer[len++] = b;
    		if( b == 0) {
    			try {
					filenames[strcnt] = new String(strbuffer, 0, len-1, "UTF-8");
				} catch (UnsupportedEncodingException e) {
					e.printStackTrace();
				}
    			Utils.LOG(strcnt+ ":" + filenames[strcnt] + ":" + sizearray[strcnt] + " : " + datearray[strcnt]);
    			strcnt++;
    			len = 0;
    		}
    	}
    	
    	if( tc.ftpdlg != null /*&& tc.ftpdlg.isValid() && tc.ftpdlg.isActive()*/) {
    		tc.ftpdlg.onFileList(numFiles, filenames, sizearray, datearray);
    	}
    	
    	return true;
    }
    
    public boolean svrFileErrorMessage(RUICommand rcmd) {
    	if( tc.ftpdlg != null /*&& tc.ftpdlg.isValid() && tc.ftpdlg.isActive()*/) {
    		tc.ftpdlg.onSvrFileError(rcmd.idx);
    	}
    	return true;
    }
    
    public void cliFileErrorMessage(int errCode) {
    	Utils.LOG("cliFileErrorMessage:"+errCode);
    	try {
			this.writeRUICMD(RUICommand.CLI_FILE_ERROR_MSG, errCode, null);
		} catch (TransportException e) {
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    }
    
    public void reqFileDownload(String filename) {
    	filename = filename.replace(File.separatorChar, '/');
    	byte[] bfilename;
    	try {
    		bfilename = filename.getBytes("UTF-8");
		} catch (UnsupportedEncodingException e1) {
			e1.printStackTrace();
			return;
		}
    	
		int length = bfilename.length; // filename.length();

		ByteBuffer buffer = ByteBuffer.allocate(12 + length);
		buffer.putInt(RUICommand.REQ_FILE_DOWNLOAD);
		buffer.putInt(0);
		buffer.putInt(12+ length);
		
		//buffer.put(filename.getBytes());
		buffer.put(bfilename);
		
		buffer.flip();
		
		try {
			write(buffer);
		} catch (IOException e) {
			e.printStackTrace();
		}
    }
    
    public boolean onResFileDownload(RUICommand rcmd) {
    	
    	ByteBuffer buffer = ByteBuffer.wrap(rcmd.body);
    	if( tc.ftpdlg != null /*&& tc.ftpdlg.isValid() && tc.ftpdlg.isActive()*/) {
    		tc.ftpdlg.onFileDownload(buffer, rcmd.lsize);
    	}
    	return true;
    }

    public void notifyDisconnectToServer() {
    	try {
			writeRUICMD(RUICommand.CLI_DISCONNECT, 0, null);
		} catch (TransportException e) {
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    }

    public boolean onServerDisconnectNotify(RUICommand rcmd) {
    	
    	String msg = "";
    	svrDisconnectNotify = rcmd.idx;
    	switch(rcmd.idx) {
    	case 0:
    		msg = "Invalid Access";
    		break;
    	case 1:
    		msg = "not enough time";
    		break;
    	case 2:
    		msg = "Server Access Timeout";
    		break;
    	case 3:
    		msg = "Server do not support this client version";
    		break;
    	default:
    			msg = String.format("Server disconnect connection (%d)", rcmd.idx);
    	}
	   JOptionPane.showMessageDialog(null, msg, RUIClient.TC_PRODUCT_NAME, JOptionPane.ERROR_MESSAGE);

    	return true;
    }
}  