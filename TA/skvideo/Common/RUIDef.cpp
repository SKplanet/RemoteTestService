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



#include "stdafx.h"
#include "RUIDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT WM_LOG_MESSAGE = ::RegisterWindowMessage(_T("WM_LOG_MESSAGE"));

UINT WM_RUIC_SELECT_VIDEO       = ::RegisterWindowMessage(_T("WM_RUIC_SELECT_VIDEO"      ));
UINT WM_RUIC_SELECT_AUDIO       = ::RegisterWindowMessage(_T("WM_RUIC_SELECT_AUDIO"      ));
UINT WM_RUIC_SET_RECT           = ::RegisterWindowMessage(_T("WM_RUIC_SET_RECT"          ));
UINT WM_RUIC_SET_FRAMERATE      = ::RegisterWindowMessage(_T("WM_RUIC_SET_FRAMERATE"     ));
UINT WM_RUIC_SET_BITRATE        = ::RegisterWindowMessage(_T("WM_RUIC_SET_BITRATE"       ));
UINT WM_RUIC_RESTART_ENCODER    = ::RegisterWindowMessage(_T("WM_RUIC_RESTART_ENCODER"   ));
UINT WM_RUIC_SET_STREAMING_MODE = ::RegisterWindowMessage(_T("WM_RUIC_SET_STREAMING_MODE"));
UINT WM_RUIC_START_STREAMING    = ::RegisterWindowMessage(_T("WM_RUIC_START_STREAMING"   ));
UINT WM_RUIC_STOP_STREAMING     = ::RegisterWindowMessage(_T("WM_RUIC_STOP_STREAMING"    ));
UINT WM_RUIC_MOUSE_STATUS       = ::RegisterWindowMessage(_T("WM_RUIC_MOUSE_STATUS"      ));
UINT WM_RUIC_KEYBOARD_STATUS    = ::RegisterWindowMessage(_T("WM_RUIC_KEYBOARD_STATUS"   ));

UINT WM_RUIC_REQ_WAVEIN_DEVICE  = ::RegisterWindowMessage(_T("WM_RUIC_REQ_WAVEIN_DEVICE" ));
UINT WM_RUIC_RES_WAVEIN_DEVICE  = ::RegisterWindowMessage(_T("WM_RUIC_RES_WAVEIN_DEVICE" ));
