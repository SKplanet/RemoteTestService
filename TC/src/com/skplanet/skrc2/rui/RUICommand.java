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

import com.skplanet.skrc2.utils.Utils;


public class RUICommand {
	
	// rui command id
	public static final int	REQ_SCREENINFO				= 0x0001;	// req screen information
	public static final int	RES_SCREENINFO						= 0x1001;	// res screen information

	public static final int	REQ_SCREENBUF				= 	0x0002;  // req screen buffer
	public static final int	RES_SCREENBUF							= 	0x1002;	// res screen buffer

	public static final int	SVR_STATECHANGED					= 	0x1003;	// server state changed

	public static final int	CLI_MOUSEEVENT				= 	0x0004;		// client mouse event

	public static final int	CLI_KEYEVENT					= 	0x0005;		// client key event

	public static final int	REQ_LOGCAT					= 	0x0006;	// req logcat
	public static final int	RES_LOGCAT								= 	0x1006;	// res logcat

	public static final int	CLI_WAKEUP					= 	0x0007;
	public static final int	CLI_ROTATE							= 	0x0008;

	public static final int	REQ_FILE_LIST				= 	0x0009;
	public static final int	RES_FILE_LIST								= 	0x1009;
	public static final int	REQ_FILE_DOWNLOAD		= 	0x000a;
	public static final int	RES_FILE_DOWNLOAD					= 	0x100a;
	public static final int	CLI_FILE_UPLOAD				= 	0x000b;
	public static final int	CLI_FILE_UPLOAD_DATA	= 	0x000c;
	
	public static final int	SVR_FILE_ERROR_MSG		= 	0x000d;
	public static final int	CLI_FILE_ERROR_MSG					= 	0x100d;

	public static final int	REQ_APP_INST					= 	0x000e;
	public static final int	CLI_APP_INST_DATA			= 	0x000f;
	public static final int	RES_APP_INST							= 	0x100e;
	public static final int	CLI_REMOVE_USER_APP		= 	0x0010;

	public static final int	REQ_GETBUILD_MANUFACTURE= 	0x0011;
	public static final int	RES_GETBUILD_MANUFACTURE	= 	0x1011;
	public static final int	REQ_GETBUILD_MODEL		= 	 0x0012;
	public static final int	RES_GETBUILD_MODEL		    	= 	0x1012;
	public static final int	REQ_GETBUILD_SERIAL		= 	0x0013;
	public static final int	RES_GETBUILD_SERIAL        		= 	0x1013;
	public static final int	REQ_GET_BUILD_VERSION_RELEASE	= 	0x0014;
	public static final int	RES_GET_BUILD_VERSION_RELEASE	= 0x1014;
	public static final int	REQ_GET_BUILD_VERSION_SDK_INT	= 0x0015;
	public static final int	RES_GET_BUILD_VERSION_SDK_INT	= 0x1015;
	public static final int	REQ_RESTART_ENCODER		= 0x0016;
	public static final int	REQ_START_STREAMING		= 0x0017;
	public static final int	REQ_SET_STREAMING_MODE	= 0x0018;

	public static final int	SVR_DISCONNECT			= 0xFFF1;				// idx 0 : invalid access, 1: timeout, 2: unknown
	public static final int	CLI_DISCONNECT				= 0xFFF2;
		
	public int		cmd = 0;
	public int		idx = 0;
	public int		lsize = 0;
	public byte[]	body = new byte[0];
	
	public void dump() {
		Utils.LOG("=== RUICommand ===========================");
		Utils.LOG(" cmd(%04x), idx(%04x), size(%d)\r", cmd, idx, lsize);

		if( body.length > 0) {
			String s = String.format("%02x %02x %02x %02x", // %02x %02x %02x %02x",
					body[0], body[1], body[2], body[3]); //, body[4], body[5], body[6], body[7]);
			
			Utils.LOG("body:" + s);
		}
		Utils.LOG("======================================");
	}
}
