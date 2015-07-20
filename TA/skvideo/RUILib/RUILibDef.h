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


#ifndef _RUILIBDEF_H
#define _RUILIBDEF_H

// [neuromos] codec_pipeline.h에 Reader Type이 다음과 같이 선언되어 있음.
#if 0
enum
{
    DEF_DATA_READER = 0,
    FILE_DATA_READER,
    VOB_DATA_READER,
    SOCKET_DATA_READER,
    HTTP_DATA_READER
};
#endif

#define RUI_FRAMERATE				15

#define RUIBUFFER_DATA_READER		100

#define RUI_FRAMESUBTYPE_NONE		0
#define RUI_FRAMESUBTYPE_I			1
#define RUI_FRAMESUBTYPE_P			2
#define RUI_FRAMESUBTYPE_B			3

#define RUI_CMD_STUN				_T("#s")
#define RUI_CMD_STUN_ACK			_T("#a")
#define RUI_CMD_SYNC_TICK			_T("$t")
#define RUI_CMD_PACKET_CONGESTION	_T("$c")
#define RUI_CMD_STREAM				_T("$s")

extern UINT WM_END_SYNCTICK;

#endif
