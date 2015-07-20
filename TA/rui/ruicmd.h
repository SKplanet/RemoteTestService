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

#define FILE_ERR_CONVERT		0x0001	// error occur during convert path
#define FILE_ERR_OPENFILE		0x0002	// could not open file
#define FILE_ERR_READ			0x0003	// file read error
#define FILE_ERR_STAT			0x0004	// file stat failed perhaps it is absent or is not a regular file
#define FILE_ERR_CREATE			0x0005  // file creation error
#define FILE_ERR_WRITE			0x0006	// file write error
#define FILE_ERR_CANCELBYUSER	0x0007 // user cancel transfer

#define APP_INST_RES_SUCCESS			0x0001
#define APP_INST_RES_ERR_FILECREATE		0x0002
#define APP_INST_RES_ERR_INSTALL		0x0003
#define APP_INST_RES_USER_CANCEL		0x0004

#define CONN_RES_OK						0x00
#define CONN_RES_INVALID_RESERVATIONID	0x01
#define CONN_RES_INVALID_TIME			0x02
#define CONN_RES_INVALID_VERSION		0x03

#define	SCRI_SCREEN_H264			0x00000001	// use video format (not jpeg frame)
#define SCRI_SCREEN_JPEG			0x00000000	// use frame format (not video h.264)
#define	SCRI_AUDIO					0x00000010	// capture audio
#define SCRI_VIDEO_QUALITY_LOW		0x00000100	// video quality low
#define SCRI_VIDEO_QUALITY_NORMAL	0x00000000	// video quality normal
#define SCRI_VIDEO_QUALITY_HIGH		0x00001000	// video quality high

#ifdef REDECON
	#define DEVELOPER_CHECK_KEY	"0119633537901033417798"
#else
	#define DEVELOPER_CHECK_KEY	"0108995225901052700256"
#endif

#define RUICMD_HEADER	12
typedef struct {
	unsigned int	cmd;
	unsigned int	idx;
	unsigned int	lsize;
	char*			body;
} RUICMD, * PRUICMD;

#define _RUICMD	\
	unsigned int	cmd; \
	unsigned int	idx; \
	unsigned int	lsize;

typedef struct {
	_RUICMD
	USHORT	width;
	USHORT	height;
	USHORT	BPP;
} RUICMD_SCREENINFO;

typedef struct {
	_RUICMD
	USHORT	state;
} RUICMD_STATECHANGED;

typedef struct {
	_RUICMD
	USHORT	x;
	USHORT	y;
	USHORT	mode;
} RUICMD_MOUSEEVENT;

typedef struct {
	_RUICMD
	UINT	key;
	USHORT	mode;
} RUICMD_KEYEVENT;

//--- file transfer
typedef struct {
	_RUICMD
	BYTE	flags;
	BYTE	dummy1;
	USHORT	dirnamesize;
	// followed by dirName[dirnamesize]
} RUICMD_FILELISTREQ;					// svr <-- client

typedef struct {
	_RUICMD
	BYTE	flags;
	BYTE	dummy3;
	USHORT dummy4;
	USHORT	numFiles;
	USHORT	datasize;
	// followed by SizeData(ULONG)[numFiles]
	// followed by Filenames[datasize]
} RUICMD_FILELISTDATA;					// svr --> client


typedef struct {
	_RUICMD
	USHORT	fnamesize;
	USHORT dummy3;
	ULONG	position;
	// followed by fname[fnamesize]
} RUICMD_FILEUPLOADREQ;					// svr <-- client

typedef struct {
	_RUICMD
	USHORT	size;
	USHORT dummy3;
	// followed by filedata[size]
} RUICMD_FILEDATA;						// svr <-> client

typedef struct {
	_RUICMD
	USHORT	reasonlen;
	//followed by reason[reasonlen]
} RUICMD_FILE_ERRMSG;					// svr <-> client


#define	REQ_SCREENINFO				0x0001	// req screen information
#define		RES_SCREENINFO					0x1001	// res screen information

#define REQ_SCREENBUF				0x0002  // req screen buffer
#define		RES_SCREENBUF					0x1002	// res screen buffer

#define		SVR_STATECHANGED				0x1003	// server state changed

#define	CLI_MOUSEEVENT				0x0004		// client mouse event

#define	CLI_KEYEVENT				0x0005		// client key event

#define	REQ_LOGCAT					0x0006	// req logcat
#define		RES_LOGCAT						0x1006	// res logcat

#define	CLI_WAKEUP					0x0007
#define CLI_ROTATE					0x0008

#define	REQ_FILE_LIST				0x0009
#define		RES_FILE_LIST					0x1009
#define REQ_FILE_DOWNLOAD			0x000a
#define		RES_FILE_DOWNLOAD				0x100a
#define	CLI_FILE_UPLOAD				0x000b
#define	CLI_FILE_UPLOAD_DATA		0x000c
#define	SVR_FILE_ERROR_MSG			0x000d
#define		CLI_FILE_ERROR_MSG				0x100d

#define REQ_APP_INST				0x000e
#define	CLI_APP_INST_DATA			0x000f
#define		RES_APP_INST					0x100e
#define CLI_REMOVE_USER_APP			0x0010

#define REQ_GETBUILD_MANUFACTURE	  0x0011
#define		RES_GETBUILD_MANUFACTURE		0x1011
#define REQ_GETBUILD_MODEL			  0x0012
#define		RES_GETBUILD_MODEL		        0x1012
#define	REQ_GETBUILD_SERIAL           0x0013
#define		RES_GETBUILD_SERIAL             0x1013
#define REQ_GET_BUILD_VERSION_RELEASE 0x0014
#define     RES_GET_BUILD_VERSION_RELEASE   0x1014
#define REQ_GET_BUILD_VERSION_SDK_INT 0x0015
#define		RES_GET_BUILD_VERSION_SDK_INT   0x1015
#define REQ_RESTART_ENCODER			  0x0016
#define REQ_START_STREAMING			  0x0017
#define REQ_SET_STREAMING_MODE		  0x0018

#define SVR_DISCONNECT				0xFFF1				// idx 0 : invalid access, 1: timeout, 2: unknown
#define		CLI_DISCONNECT					0xFFF2

#pragma pack(pop)