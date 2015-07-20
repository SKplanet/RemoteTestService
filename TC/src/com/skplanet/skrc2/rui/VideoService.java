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

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;
import java.util.Iterator;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.SourceDataLine;

import net.sourceforge.jaad.aac.AACException;
import net.sourceforge.jaad.aac.Decoder;
import net.sourceforge.jaad.aac.SampleBuffer;
import net.sourceforge.jaad.adts.ADTSDemultiplexer;

import com.skplanet.skrc2.utils.Utils;


public class VideoService extends Thread {

	public static final int RUI_FRAMERATE					= 15;

	public static final int RUIBUFFER_DATA_READER	= 100;

	public static final int RUI_FRAMESUBTYPE_NONE	= 0;
	public static final int RUI_FRAMESUBTYPE_I			= 1;
	public static final int RUI_FRAMESUBTYPE_P			= 2;
	public static final int RUI_FRAMESUBTYPE_B			= 3;

	public static final String RUI_CMD_STUN							= "#s";
	public static final String RUI_CMD_STUN_ACK					= "#a";
	public static final String RUI_CMD_SYNC_TICK					= "$t";
	public static final String RUI_CMD_PACKET_CONGESTION	= "$c";
	public static final String RUI_CMD_STREAM						= "$s";
	
	public static final long MAX_AUDIO_DIFF	= 400;
	public RUIClient tc;
	public boolean connected;
	SocketChannel tcVChannel;
	//long m_dwSendLatency      = (long)0xFFFFFFFFL;
	long m_dwSendLatencySum   = 0;
	long m_dwSendLatencyCount = 0;
	long m_dwErrorMin         = (long)0xFFFFFFFFL;	
	
	long m_nRelMyTickSign = 0;
	long m_dwRelMyTick  = 0;
	
    ByteBuffer m_buffer;
    Decoder aacdec;
	SampleBuffer pcmbuf;
	ADTSDemultiplexer adts;
	InputStream aacframestream;
	byte[] aacframebuffer;
	SourceDataLine line = null;
	
	public VideoService(RUIClient c) {
		tc = c;
		connected = false;
		aacframebuffer = null;
		line = null;
	}

	public void initAAC() {
		try {
			adts = new ADTSDemultiplexer(aacframestream);
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		
		byte[] decinf = adts.getDecoderSpecificInfo();

		
		try {
			aacdec = new Decoder(decinf);
		} catch (AACException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		pcmbuf = new SampleBuffer();
	}
	public boolean VideoServerConnect() {
		Utils.LOG("VideoService :: connection start:"+tc.tcAddress+"("+tc.tcVPort+")");
		
		// 1. do tc connect
		try {
			tcVChannel = SocketChannel.open();
		} catch (IOException e) {
			e.printStackTrace();
			tc.tcErr = "Video Socket Open Failed. (1)";
			tc.onConnectionResult(false, false, false);
			return false;
		}
		
		try {
			tcVChannel.configureBlocking(false);
			tcVChannel.connect(new InetSocketAddress(tc.tcAddress, tc.tcVPort));
			tcVChannel.socket().setSoTimeout(30000);
		} catch (IOException e) {
			e.printStackTrace();
			tc.tcErr = "Video Socket Open Failed. (2)";
			tc.onConnectionResult(false, false, false);
			return false;
		}
		
        try {
			while (!Thread.interrupted()  && !tc.abortable.isDone() &&  !tcVChannel.finishConnect()) {  
			    try {
					Thread.sleep(10);
				} catch (InterruptedException e) {
					e.printStackTrace();
					tc.tcErr = "Socket Open Failed. (3)";
					tc.onConnectionResult(false, false, false);
					return false;
				}  
			}
		} catch (IOException e1) {
			e1.printStackTrace();
			tc.tcErr = "Socket Connection failed.";
			tc.onConnectionResult(false, false, false);
			return false;
		}
        
        if( Thread.interrupted() || tc.abortable.isDone()) {
        	return false;
        }
        
		
		
		try {
			Thread.sleep(100);
	        if( Thread.interrupted() || tc.abortable.isDone()) {
	        	return false;
	        }
			tc.tcService.writeRUICMD(RUICommand.REQ_RESTART_ENCODER, 0, null);
			tc.tcService.writeRUICMD(RUICommand.REQ_START_STREAMING, 0, null);
		} catch (TransportException e) {
			e.printStackTrace();
			return false;
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		} catch (InterruptedException e) {
			e.printStackTrace();
			return false;
		}
		
		return true;
	}
	
	public static long getTickCount() {
		//return System.currentTimeMillis() & 0x24FFFFFFFL;
		return System.currentTimeMillis() & 0x00000000ffffffffL;
	}
	 @Override  
	    public void run() {  
	        super.run();  

	        // connect video server
	        if( false == VideoServerConnect() )
	        {
	        	if( !tc.abortable.isDone()) {
					Utils.LOG("Media sync init failed. disconnect and connect again.");
					tc.tcErr = "Media sync init failed. disconnect and connect again.";
					tc.onConnectionResult(false, false, false);
	        	} else {
	        		tc.disconnect();
	        	}
	        	return;
	        }
	        
	        // calc sync tick
	        resetSyncTick();
	        try {
				SendCommandSyncTick(getTickCount(), 0, 1, 0, 20);
			} catch (IOException e1) {
				e1.printStackTrace();
	        	if( !tc.abortable.isDone()) {
					Utils.LOG("Media sync failed. disconnect and connect again.");
					tc.tcErr = "Media sync failed.\nPlease try again.";
					tc.onConnectionResult(false, false, false);
	        	} else {
					tc.disconnect();
	        	}
				return;
			}
	        
	        boolean done = false;  
	        Selector selector = null;
	          
	        try {  
	              
	        	Utils.LOG("VideoService Thread :: started");  
	            
	            //videoThread.start();
	            
	            selector = Selector.open();  
	            this.tcVChannel.register(selector, SelectionKey.OP_READ);  
	            
	    	    m_buffer = ByteBuffer.allocate(4096);
	    	    m_buffer.order(ByteOrder.LITTLE_ENDIAN);

	            while (!Thread.interrupted() && !tc.abortable.isDone() && !done) {  
	                  
	            	if( m_dwSendLatencyCount == 0) {
	        	        try {
	        				SendCommandSyncTick(getTickCount(), 0, 1, 0, 20);
	        			} catch (IOException e1) {
	        				e1.printStackTrace();
	        				tc.tcErr = "Media sync failed.\nPlease try again.";
	        				tc.disconnect();
	        				tc.onConnectionResult(false, false, false);
	        				break;
	        			}
	            	}
	                selector.select(100);
	                Iterator<SelectionKey> iter = selector.selectedKeys().iterator();
	                if( iter.hasNext() == false)
	                {
	                	//System.out.println("iter.hasnet is false");
	                }
	                
	                while (!Thread.interrupted() && !tc.abortable.isDone() && !done && iter.hasNext()) {  
	                    SelectionKey key = iter.next();
	                    if( key.isReadable()) {
	                        int len = tcVChannel.read(m_buffer);  
	                        if (len <  0) {  
	                        	Utils.LOG("Video Server server closed");  
	                            done = true;  
	                            break;  
	                        } else if (len == 0) {  
	                            continue;  
	                        }
	                        
	                        long nPacketSize = m_buffer.getInt(0)  & 0xffffffffL + 4;
	                        if( m_buffer.position() < nPacketSize) {
	                        	continue;
	                        }
	                   
	                        m_buffer.flip();
	                        
	                        //Thread.sleep(200);
	                        if( processVideoCMD() == false) {
	                        	done = true;
	                        	break;
	                        }
	                          
	                        m_buffer.clear();  
	                    }
	                } 
	            }  
	        }catch (RuntimeException e) {
	        	throw e;
	        } catch (Exception e) {  
	            e.printStackTrace();  
	        } finally {  
	              
	            if (tcVChannel != null) {  
	                try {  
	                	tcVChannel.socket().close();  
	                	tcVChannel.close();  
	                	tcVChannel = null;
	                } catch (IOException e) {  
	                    e.printStackTrace();  
	                }  
	            }  
	              
	            Utils.LOG("Video Service :: done");  
	        }  
	    } 
	 
	 public boolean processVideoCMD() {
		 //long cmdlen = m_buffer.getInt()  & 0xffffffffL;
		 m_buffer.getInt();
		 byte[] buffer = new byte[10];
		 m_buffer.get(buffer, 0, 10);
		 String cmd =  new String(buffer);
		 m_buffer.rewind();
		 
		 if( cmd.startsWith(RUI_CMD_SYNC_TICK, 0) ) {
			return ParseSyncTick();
		 }
		 else if ( cmd.startsWith( RUI_CMD_PACKET_CONGESTION,0)) {
			 
		 }
		 else	if ( cmd.startsWith( RUI_CMD_STREAM,0)) {
			 return ParseStream();
		 }
		 return true;
	 }
	 public long GetSendLatency()
	 {
	 	if (m_dwSendLatencyCount > 0)
	 		return m_dwSendLatencySum / m_dwSendLatencyCount;

	 	return (long)0xFFFFFFFF;
	 }
		
	 //static boolean bSkip = false;
	 
	public boolean ParseStream() {
		 int cmdlen = m_buffer.getInt();
		 m_buffer.position(m_buffer.position() + RUI_CMD_STREAM.length());

		 //byte nFrameType = m_buffer.get();
		 m_buffer.get();
		 //long dwFrameKey= m_buffer.getInt()  & 0xffffffffL;
		 m_buffer.getInt();
		 long dwFrameTick= m_buffer.getInt() & 0xFFFFFFFFL;
		 
		 if(m_nRelMyTickSign>0 )
			 dwFrameTick += m_dwRelMyTick;
		 else
			 dwFrameTick -= m_dwRelMyTick;
		 
		 
		 int dwFrameSize = cmdlen - RUI_CMD_STREAM.length() - 1 - 4 - 4;


		long CurrentTick = getTickCount();
		long diff;
		if( CurrentTick > dwFrameTick)
			diff = CurrentTick - dwFrameTick;
		else
			diff = dwFrameTick - CurrentTick;
		
		
		if( diff > MAX_AUDIO_DIFF )
		{
			Utils.LOG("Audio Latency exceed error level:(%d)\n", diff);
			m_buffer.position(m_buffer.position() + dwFrameSize);
			return true;
		}
		
		if( aacframebuffer == null ) {
			aacframebuffer = new byte[8192];
			m_buffer.get(aacframebuffer, 0, dwFrameSize);
			aacframestream = new ByteArrayInputStream(aacframebuffer);
			aacframestream.mark(dwFrameSize);
			this.initAAC();
		} else {
			try {
				aacframestream.reset();
			} catch (IOException e) {
				e.printStackTrace();
			}
			m_buffer.get(aacframebuffer, 0, dwFrameSize);
			aacframestream.mark(dwFrameSize);
		}
		//byte[] aacFrame = m_buffer.array();
		
		byte[] aacFrame;
		try {
			aacFrame = adts.readNextFrame();
			//while((aacFrame = adts.readNextFrame())!=null) {
			if( aacFrame != null ) {
				try {
					aacdec.decodeFrame(aacFrame, pcmbuf);
				} catch (AACException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				//the aacFrame array contains the AAC frame to decode
				byte[] audio = pcmbuf.getData(); //this array contains the raw PCM audio data
				
				if(line==null) {
					final AudioFormat aufmt = new AudioFormat(pcmbuf.getSampleRate(), pcmbuf.getBitsPerSample(), pcmbuf.getChannels(), true, true);
					try {
						line = AudioSystem.getSourceDataLine(aufmt);
					} catch (LineUnavailableException e) {
						e.printStackTrace();
					}
					if( line != null ) {
						try {
							line.open();
						} catch (LineUnavailableException e) {
							e.printStackTrace();
						}
						line.start();
					}
				}
				
				if( line != null ) {
					line.write(audio, 0, audio.length);
				}
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		


			return true;
	 }
	 
	 public boolean ParseSyncTick()
	 {
		 int cmdlen = m_buffer.getInt();
		 byte[] buffer = new byte[cmdlen];
		 m_buffer.position(m_buffer.position() + RUI_CMD_SYNC_TICK.length());
		 m_buffer.get(buffer, 0, cmdlen - RUI_CMD_SYNC_TICK.length());
		 String strParam =  new String(buffer);
		 
	 	long	dwCurrMyTick		= getTickCount();
	 	long	dwYourTick       	= Long.valueOf(strParam.substring(0, 10).trim());
	 	long	dwMyTick         	= Long.valueOf(strParam.substring(10, 20).trim());
	 	int		nRelYourTickSign	= Integer.valueOf(strParam.substring(20, 21).trim());
	 	long	dwRelYourTick		= Long.valueOf(strParam.substring(21, 31).trim());
	 	int		nSyncTickCount   = Integer.valueOf(strParam.substring(31, 35).trim()); // SyncTick을 해야 하는 남은 Count
	 	int		nRelMyTickSign   = 1;
	 	long	dwRelMyTick      = 0;

	 	if (dwMyTick != 0)
	 	{
	 		long	dwSendLatency  = (dwCurrMyTick - dwMyTick) / 2; // 이전 패킷에 실어 보내 Tick와 현재 Tick을 반으로 나누면(가고 오고) 네트워크 Latency를 추정할 수 있음

	 		m_dwSendLatencySum += dwSendLatency;
	 		m_dwSendLatencyCount++;
	 		dwSendLatency = GetSendLatency();

	 		long	dwCurrYourTick = dwYourTick - dwSendLatency;    // 상대방이 보낸 시점의 Tick에 네트워크 Latency를 반영하면 현재 상대방의 Tick을 추정할 수 있음

	 		if (dwCurrMyTick > dwCurrYourTick)
	 		{
	 			nRelMyTickSign = 1;
	 			dwRelMyTick = dwCurrMyTick - dwCurrYourTick;
	 		}
	 		else
	 		{
	 			nRelMyTickSign = 0;
	 			dwRelMyTick = dwCurrYourTick - dwCurrMyTick;
	 		}

	 		if (dwRelYourTick != 0)
	 		{
	 			long	dwCurrMyTickByRel;
	 			long	dwError;

	 			if (nRelYourTickSign > 0) dwCurrMyTickByRel = dwCurrYourTick - dwRelYourTick;
	 			else                      dwCurrMyTickByRel = dwCurrYourTick + dwRelYourTick;

	 			if (dwCurrMyTickByRel > dwCurrMyTick) dwError = dwCurrMyTickByRel - dwCurrMyTick;
	 			else                                  dwError = dwCurrMyTick      - dwCurrMyTickByRel;


	 			if (dwError < m_dwErrorMin)
	 			{
	 				m_nRelMyTickSign = nRelMyTickSign;
	 				m_dwRelMyTick    = dwRelMyTick;
	 				m_dwErrorMin      = dwError;
	 			}
	 		}
	 	}

	 	if ((--nSyncTickCount) > 0)
			try {
				SendCommandSyncTick(dwCurrMyTick, dwYourTick, nRelMyTickSign, dwRelMyTick, nSyncTickCount);
			} catch (IOException e) {
				e.printStackTrace();
				return false;
			}


 		Utils.LOG("m_nRelMyTickSign(%d) m_dwRelMyTick(%d) m_dwErrorMin(%d)\n",m_nRelMyTickSign, m_dwRelMyTick,m_dwErrorMin);

	 	return true;
	 }
	 private void resetSyncTick() {
			 //m_dwSendLatency      = 0xFFFFFFFFL;
			 m_dwSendLatencySum   = 0;
			 m_dwSendLatencyCount = 0;
			 m_dwErrorMin         = 0xFFFFFFFFL;	
	}

	public void SendCommandString(String szCommand) throws IOException
	 {
		 	int 	nCommandLen      = szCommand.length() + 1; // NULL Terminal 포함
		 	int		dwPacketBodySize= nCommandLen;
		 	int		dwPacketSize      = 4 + dwPacketBodySize;
	 	
		 	ByteBuffer buffer = ByteBuffer.allocate(dwPacketSize);
		 	buffer.order(ByteOrder.LITTLE_ENDIAN);
		 	
		 	buffer.putInt(dwPacketBodySize);
		 	buffer.put(szCommand.getBytes());
		 	buffer.put((byte)0);
		 	
		 	//byte[] t = buffer.array();
		 	//System.out.println(String.format("%02x %02x %02x %02x", t[0], t[1], t[2], t[3]));
		 	buffer.flip();
		 	tcVChannel.write(buffer);

	 }
	 
	 public void SendCommandSyncTick(long dwMyTick, long dwYourTick, int nRelMyTickSign, long dwRelMyTick, int nCount) throws IOException
	 {
 		String strCommand;
 		strCommand = String.format("%s%10d%10d%01d%10d%04d", RUI_CMD_SYNC_TICK, dwMyTick, dwYourTick, nRelMyTickSign, dwRelMyTick, nCount);
 		//Utils.LOG(strCommand);
 		Utils.LOG("SendAudioSyncTick");
 		SendCommandString(strCommand);
	 }
}
