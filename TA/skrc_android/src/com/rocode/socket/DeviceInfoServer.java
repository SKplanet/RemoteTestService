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

package com.rocode.socket;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

import com.rocode.info.DeviceControl;
import com.rocode.log.Log;
import com.rocode.parser.CMDParser;
import com.rocode.skrc.RCService;

public class DeviceInfoServer extends Thread {

	private static final String TAG = "RCService";

	private static final int SIZE_LENGTH = 2;
	private static final int CMD_LENGTH = 2;
	private static final int INDEX_LENGTH = 2;
	private static final int RESULT_LENGTH = 2;

	public static final int HEADER_SIZE = SIZE_LENGTH + CMD_LENGTH + INDEX_LENGTH + RESULT_LENGTH;

	public static final int MAX_PACKET_SIZE = 1024;

	public static final int DATA_LENGTH = MAX_PACKET_SIZE - HEADER_SIZE;

	private ServerSocket serverS;

	private int port;

	private RCService context;

	private DeviceControl deviceControl;

	private byte[] sizeBuffer = new byte[SIZE_LENGTH];
	private byte[] cmdBuffer = new byte[CMD_LENGTH];
	private byte[] indexBuffer = new byte[INDEX_LENGTH];
	private byte[] resultValueBuffer = new byte[RESULT_LENGTH];
	private byte[] bodyBuffer = new byte[DATA_LENGTH];
	private byte[] readBuffer = new byte[DATA_LENGTH];

	private byte[] writeBuffer = new byte[DATA_LENGTH];

	private boolean runRW = true;

	private boolean clientSockConnected = false;

	private BufferedOutputStream bufferedOs;

	public static final int DEFAULT_PORT = 5912;

	private static final int TIME_OUT = 30 * 1000;

	public DeviceInfoServer(RCService context) {
		this(context, DEFAULT_PORT);
	}

	public DeviceInfoServer(RCService context, int port) {
		this.port = port;

		this.runRW = true;

		this.context = context;
		deviceControl = DeviceControl.getDeviceControl(context);
	}

	public void stopServer() {
		this.runRW = false;
		try {
			serverS.close();
		} catch (IOException e) {
		}

		deviceControl.clean();

		context.stopLogcat();
	}

	public boolean isClientConnected() {
		return this.clientSockConnected;
	}

	/**
	 * 서버의 생명주기는 외부 프로그램에서 관리함.
	 * RCService와 소켓 연결이 종료되면 외부 프로그램에서 RCService를 다시 설치하고 실행시킴.
	 */
	@Override
	public void run() {
		try {
			serverS = new ServerSocket(port);
		} catch (IOException ioe) {
			ioe.printStackTrace();
			return;
		}

		Socket tcpSocket = null;

		int sendSize = 0;

		int size = 0;
		int cmd = 0;
		int index = -1;
		int resultValue = -1;
		int bodySize = 0;

		String body = null;

		int receivByteCount = 0;
		int count = 0;

		BufferedInputStream bufferedIs = null;
		bufferedOs = null;
		try {
			Log.d(TAG, "listen...");
			clientSockConnected = false;
			tcpSocket = serverS.accept();
			tcpSocket.setTcpNoDelay(true);
			tcpSocket.setKeepAlive(true);
			tcpSocket.setSoTimeout(TIME_OUT);

			// 클라이언트는 한 번 연결해 두고 계속 사용함.
			// 소켓 연결이 끊어진 상태에선 블러킹이 안되는게 요청사항.
			clientSockConnected = true;

			Log.d(TAG, "client IP : " + tcpSocket.getInetAddress().getHostAddress());

			bufferedIs = new BufferedInputStream(tcpSocket.getInputStream());
			bufferedOs = new BufferedOutputStream(tcpSocket.getOutputStream());

			while (this.runRW) {
				// init
				size = 0;
				cmd = -1;
				index = -1;
				resultValue = -1;
				bodySize = 0;
				body = null;

				sendSize = 0;

				receivByteCount = 0;

				// read size
				count = 0;

				count = bufferedIs.read(sizeBuffer);

				Log.d(TAG, "read size: " + count);
				if (count == SIZE_LENGTH) {
					receivByteCount += count;

					size = ((sizeBuffer[0] & 0xFF) << 8) + (sizeBuffer[1] & 0xFF);

				} else {

					tcpSocket.close();
					break;
				}

				Log.d(TAG, "size: " + size);

				bodySize = size - HEADER_SIZE;

				// read cmd
				count = 0;
				count = bufferedIs.read(cmdBuffer);

				Log.d(TAG, "cmd size: " + count);
				if (count == CMD_LENGTH) {
					receivByteCount += count;

					cmd = ((cmdBuffer[0] & 0xFF) << 8) + (cmdBuffer[1] & 0xFF);

				} else {

					tcpSocket.close();
					break;
				}

				Log.d(TAG, "cmd: " + cmd);

				// read index
				count = 0;
				count = bufferedIs.read(indexBuffer);

				Log.d(TAG, "index size: " + count);
				if (count == INDEX_LENGTH) {
					receivByteCount += count;

					index = ((indexBuffer[0] & 0xFF) << 8) + (indexBuffer[1] & 0xFF);

				} else {

					tcpSocket.close();
					break;
				}

				Log.d(TAG, "index: " + index);

				// read resultValue
				count = 0;
				count = bufferedIs.read(resultValueBuffer);

				Log.d(TAG, "resultValue size: " + count);
				if (count == RESULT_LENGTH) {
					receivByteCount += count;

					resultValue = ((resultValueBuffer[0] & 0xFF) << 8) + (resultValueBuffer[1] & 0xFF);

				} else {

					tcpSocket.close();
					break;
				}

				Log.d(TAG, "resultValue: " + resultValue);

				if (bodySize > 0) {
					// init body Buffer
					for (int i = bodySize; i < bodyBuffer.length; ++i) {
						bodyBuffer[i] = 0;
					}

					// read body
					count = 0;
					count = bufferedIs.read(bodyBuffer, 0, bodySize);

					Log.d(TAG, "body size: " + count);
					if (count == bodySize) {
						receivByteCount += count;

						body = new String(bodyBuffer).trim();

					} else {

						tcpSocket.close();
						break;
					}

				}

				if (receivByteCount != size) {

					Log.e(TAG, "received byte count != size");
					tcpSocket.close();
					break;
				}

				// process

				sendSize = CMDParser.execute(context, deviceControl, cmd, index, body, readBuffer);

				if (sendSize > 0) {
					bufferedOs.write(readBuffer, 0, sendSize);

					bufferedOs.flush();
				}

			}
		} catch (Exception ioe) {
			Log.d(TAG, "Service is down.");
			Log.e(TAG, ioe.toString());
			Log.e(TAG, ioe.getStackTrace().toString());
		} finally {

			if (bufferedIs != null) {
				try {
					bufferedIs.close();
				} catch (IOException e) {
				}
			}
			if (bufferedOs != null) {
				try {
					bufferedOs.close();
				} catch (IOException e) {
				}
			}

			if (tcpSocket != null) {
				try {
					tcpSocket.close();
				} catch (IOException e) {
				}
			}

		}
		
		// 소켓 연결이 종료되면 서비스 자체를 죽인다.
		context.stopRCService();
	}

	public void sendMsg(int cmd, int index, int resultValue, String body) throws IOException {

		byte[] byteValue = body.getBytes();

		int size = HEADER_SIZE + byteValue.length;

		writeBuffer[0] = (byte) (size >>> 8);
		writeBuffer[1] = (byte) (size);
		writeBuffer[2] = (byte) (cmd >>> 8);
		writeBuffer[3] = (byte) (cmd);
		writeBuffer[4] = (byte) (index >>> 8);
		writeBuffer[5] = (byte) (index);
		writeBuffer[6] = (byte) (resultValue >>> 8);
		writeBuffer[7] = (byte) (resultValue);

		for (int i = HEADER_SIZE; i < size; ++i) {
			writeBuffer[i] = byteValue[i - HEADER_SIZE];
		}

		bufferedOs.write(writeBuffer, 0, size);
		bufferedOs.flush();
		return;
	}
}
