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



#pragma once

#pragma pack(push,4)
#define TDCCMD_HEADER	8
typedef struct {
	unsigned short	size;
	unsigned short	cmd;
	unsigned short	idx;
	unsigned short	rtn;
	void*			body;
} TDCCMD, * PTDCCMD;

//--- TDC DEV STATUS
#define	TDC_STATUS_ROTATE		0x0003
#define	TDC_STATUS_SCREENON		0x0004
#define TDC_STATUS_CALLBLOCKED	0x0008
#define TDC_STATUS_PKGBLOCKED	0x0010
#define TDC_STATUS_SIMABSENT	0x0020
#define TDC_STATUS_ROTATEAUTO	0x0040

//--- TDC Command ID
#define TDC_ISSCREENON		0
#define	TDC_WAKELOCKACQUIRE	1
#define TDC_WAKELOCKRELEASE	2

#define	TDC_GETROTATION		100
#define	TDC_GETWIDTH		101
#define	TDC_GETHEIGHT		102
#define	TDC_GETPIXELFORMAT	103
#define TDC_SETROTATE_BASE	104
	// 0 - 104
	// 90 - 105
	// 180 - 106
	// 270 - 107
	// recovery - 108
#define TDC_GET_DENSITY_DPI	109
#define TDC_GET_RC_VERSION	110

#define TDC_GETBATTEMPERATURE	200
#define	TDC_GETBATTLEVEL		201

#define TDC_GETBUILD_BOARD			300
#define	TDC_GETBUILD_BOOTLOADER		301
#define	TDC_GETBUILD_BRAND			302
#define	TDC_GETBUILD_CPUABI			303
#define	TDC_GETBUILD_CPUABI2		304
#define	TDC_GETBUILD_DEVICE			305
#define	TDC_GETBUILD_DISPLAY		306
#define	TDC_GETBUILD_FINGERPRINT	307
#define	TDC_GETBUILD_HARDWARE		308
#define	TDC_GETBUILD_HOST			309
#define	TDC_GETBUILD_ID				310
#define	TDC_GETBUILD_MANUFACTURE	311
#define	TDC_GETBUILD_MODEL			312
#define	TDC_GETBUILD_PRODUCT		313
#define	TDC_GETBUILD_RADIO			314
#define	TDC_GETBUILD_SERIAL			315
#define	TDC_GETBUILD_TAGS			316
#define	TDC_GETBUILD_TIME			317
#define	TDC_GETBUILD_TYPE			318
#define	TDC_GETBUILD_USER			319
#define	TDC_GET_BUILD_VERSION_CODENAME		320
#define	TDC_GET_BUILD_VERSION_INCREMENTAL	321
#define	TDC_GET_BUILD_VERSION_RELEASE		322
#define	TDC_GET_BUILD_VERSION_SDK_INT		323

#define	TDC_SETBLOCKEDPKGNAMELIST		400
#define	TDC_SETCALLBLOCKLIST			401
#define	TDC_SETCALLALLOWLIST			402
#define TDC_SET_BLOCKED_MIMETYPE_LIST	403

#define	TDC_SETLOGENABLE			500
#define TDC_GETSTATUS				501

#define	TDC_GETINATALLEDPKGNAME		600

#define	TDC_GET_LINE1_NUMBER			700
#define	TDC_GET_NETWORK_OPERATOR		701
#define	TDC_GET_NETWORK_OPERATOR_NAME	702
#define	TDC_IS_SIM_STATE_ABSENT			703
#define	TDC_IS_SIM_STATE_NETWORK_LOCKED	704
#define	TDC_IS_SIM_STATE_PIN_REQUIRED	705
#define	TDC_IS_SIM_STATE_PUK_REQUIRED	706
#define	TDC_IS_SIM_STATE_READY			707
#define	TDC_IS_SIM_STATE_UNKNOWN		708
#define TDC_GET_DEVICE_ID				709
#define TDC_GET_DEVICE_SOFTWARE_VER		710
#define TDC_GET_SIM_SERIAL_NUMBER		711

#define TDC_LOGCAT_MONITOR_START				800
#define TDC_LOGCAT__MONITOR_STOP				801
#define TDC_BLOCK_HANDLER_START					802
#define TDC_BLOCK_HANDLER_STOP					803
#define TDC_BLOCK_HANDLER_DELAY					804


#define TDC_DELETE_MSG_ALL			900
#define TDC_DELETE_MSG_SMS			901
#define TDC_DELETE_MSG_CALL			902

#define TDC_INVALIDCOMMAND			1000

#pragma pack(pop)