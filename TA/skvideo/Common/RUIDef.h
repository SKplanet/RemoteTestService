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



#ifndef _RUIDEF_H
#define _RUIDEF_H

#define _USE_ENHANCED_CAPTURE

#define SERVER_PORT					9095
#define VIDEO_STREAMING_PORT		9096

#define MAX_CLIENT_CONNECTION		1

#define RUI_FRAME_TYPE_COMMAND		0x10
#define RUI_FRAME_TYPE_VIDEO		0x20
#define RUI_FRAME_TYPE_AUDIO		0x30

// Client --> Server
#define RUI_CMD_REQ_WAVEIN_DEVICE	_T("qw")
#define RUI_CMD_SELECT_VIDEO		_T("lv")
#define RUI_CMD_SELECT_AUDIO		_T("la")
#define RUI_CMD_SET_RECT			_T("sr")
#define RUI_CMD_SET_FRAMERATE		_T("sf")
#define RUI_CMD_SET_BITRATE			_T("sb")
#define RUI_CMD_RESTART_ENCODER		_T("ec")
#define RUI_CMD_SET_STREAMING_MODE	_T("mo")
#define RUI_CMD_START_STREAMING		_T("ss")
#define RUI_CMD_STOP_STREAMING		_T("ts")
#define RUI_CMD_MOUSE_STATUS		_T("ms")
#define RUI_CMD_KEYBOARD_STATUS		_T("kb")

// Server --> Client
#define RUI_CMD_RES_WAVEIN_DEVICE	_T("rw")

#define RUI_CMD_STREAMING_MODE_VIDEO	0x00000001
#define RUI_CMD_STREAMING_MODE_AUDIO	0x00000002

extern UINT WM_LOG_MESSAGE;

extern UINT WM_RUIC_SELECT_VIDEO;
extern UINT WM_RUIC_SELECT_AUDIO;
extern UINT WM_RUIC_SET_RECT;
extern UINT WM_RUIC_SET_FRAMERATE;
extern UINT WM_RUIC_SET_BITRATE;
extern UINT WM_RUIC_RESTART_ENCODER;
extern UINT WM_RUIC_SET_STREAMING_MODE;
extern UINT WM_RUIC_START_STREAMING;
extern UINT WM_RUIC_STOP_STREAMING;
extern UINT WM_RUIC_MOUSE_STATUS;
extern UINT WM_RUIC_KEYBOARD_STATUS;

extern UINT WM_RUIC_REQ_WAVEIN_DEVICE;
extern UINT WM_RUIC_RES_WAVEIN_DEVICE;

#endif
