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
#include "RUIServerPeer.h"
#include "../skvideo/RUILib/RUIUDPSock.h"
#include "../skvideo/Common/RUIDef.h"
#include "../skvideo/RUILib/RUILibDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUIServerPeer::RUIServerPeer(CWnd* pMsgWnd)
{
	m_pUDPSock = NULL;

	SetMessageWindow(pMsgWnd);
	SetServerState(true); // [neuromos] 서버로 동작
}

RUIServerPeer::~RUIServerPeer()
{
}

BOOL RUIServerPeer::GetClientStreaming()
{
	if (m_pUDPSock != NULL)
		return m_pUDPSock->GetClientStreaming();

	return FALSE;
}

void RUIServerPeer::SetClientStreaming(BOOL bClientStreaming)
{
	if (m_pUDPSock != NULL)
		m_pUDPSock->SetClientStreaming(bClientStreaming);
}

void RUIServerPeer::SendCommandResWaveInDevice(UINT nDeviceID, LPCTSTR szDeviceName)
{
	if (IsOpen())
	{
		CString	strCommand;
		strCommand.Format(_T("%s%10d%s"), RUI_CMD_RES_WAVEIN_DEVICE, nDeviceID, szDeviceName);
//		WriteComm((BYTE*) (LPCTSTR) strCommand, (strCommand.GetLength() + 1) * sizeof(TCHAR), INFINITE);
		SendCommandString((LPCTSTR) strCommand);
	}
}

void RUIServerPeer::OnDataReceived(const LPBYTE lpBuffer, DWORD dwCount)
{
	RUITCPSockPeer::OnDataReceived(lpBuffer, dwCount);
}

DWORD RUIServerPeer::OnDataParsed(const LPBYTE pPacket, DWORD dwPacketSize)
{
	DWORD	dwDataParsed = 0;

	if ((dwDataParsed = RUITCPSockPeer::OnDataParsed(pPacket, dwPacketSize)) > 0)
		return dwDataParsed;

	if (m_pMsgWnd == NULL || ! ::IsWindow(m_pMsgWnd->GetSafeHwnd()))
		return 0;

	static const size_t	nCmdReqWaveinDevice  = _tcslen(RUI_CMD_REQ_WAVEIN_DEVICE );
	static const size_t	nCmdSelectVideo      = _tcslen(RUI_CMD_SELECT_VIDEO      );
	static const size_t	nCmdSelectAudio      = _tcslen(RUI_CMD_SELECT_AUDIO      );
	static const size_t	nCmdSetRect          = _tcslen(RUI_CMD_SET_RECT          );
	static const size_t	nCmdSetFramerate     = _tcslen(RUI_CMD_SET_FRAMERATE     );
	static const size_t	nCmdSetBitrate       = _tcslen(RUI_CMD_SET_BITRATE       );
	static const size_t	nCmdRestartEncoder   = _tcslen(RUI_CMD_RESTART_ENCODER   );
	static const size_t	nCmdSetStreamingMode = _tcslen(RUI_CMD_SET_STREAMING_MODE);
	static const size_t	nCmdStartStreaming   = _tcslen(RUI_CMD_START_STREAMING   );
	static const size_t	nCmdStopStreaming    = _tcslen(RUI_CMD_STOP_STREAMING    );
	static const size_t	nCmdMouseStatus      = _tcslen(RUI_CMD_MOUSE_STATUS      );
	static const size_t	nCmdKeyboardStatus   = _tcslen(RUI_CMD_KEYBOARD_STATUS   );

	CString	strParam;

	if (_tcsncmp((LPCTSTR) pPacket, RUI_CMD_REQ_WAVEIN_DEVICE, nCmdReqWaveinDevice) == 0)
	{
		m_pMsgWnd->PostMessage(WM_RUIC_REQ_WAVEIN_DEVICE, 0, (LPARAM) this);

		dwDataParsed = (DWORD) ((nCmdReqWaveinDevice + strParam.GetLength() + 1) * sizeof(TCHAR));
	}
	else if (_tcsncmp((LPCTSTR) pPacket, RUI_CMD_SELECT_VIDEO, nCmdSelectVideo) == 0)
	{
		strParam = (LPCTSTR) (pPacket + nCmdSelectVideo * sizeof(TCHAR));

		m_pMsgWnd->PostMessage(WM_RUIC_SELECT_VIDEO, (WPARAM) _ttoi((LPCTSTR) strParam));

		dwDataParsed = (DWORD) ((nCmdSelectVideo + strParam.GetLength() + 1) * sizeof(TCHAR));
	}
	else if (_tcsncmp((LPCTSTR) pPacket, RUI_CMD_SELECT_AUDIO, nCmdSelectAudio) == 0)
	{
		strParam = (LPCTSTR) (pPacket + nCmdSelectAudio * sizeof(TCHAR));

		m_pMsgWnd->PostMessage(WM_RUIC_SELECT_AUDIO, (WPARAM) _ttoi((LPCTSTR) strParam));

		dwDataParsed = (DWORD) ((nCmdSelectAudio + strParam.GetLength() + 1) * sizeof(TCHAR));
	}
	else if (_tcsncmp((LPCTSTR) pPacket, RUI_CMD_SET_RECT, nCmdSetRect) == 0)
	{
		strParam = (LPCTSTR) (pPacket + nCmdSetRect * sizeof(TCHAR));

		m_pMsgWnd->PostMessage(WM_RUIC_SET_RECT,
			(WPARAM) MAKELONG(_ttoi((LPCTSTR) (strParam.Mid(0, 4))), _ttoi((LPCTSTR) (strParam.Mid( 4, 4)))),
			(LPARAM) MAKELONG(_ttoi((LPCTSTR) (strParam.Mid(8, 4))), _ttoi((LPCTSTR) (strParam.Mid(12, 4)))));

		dwDataParsed = (DWORD) ((nCmdSetRect + strParam.GetLength() + 1) * sizeof(TCHAR));
	}
	else if (_tcsncmp((LPCTSTR) pPacket, RUI_CMD_SET_FRAMERATE, nCmdSetFramerate) == 0)
	{
		strParam = (LPCTSTR) (pPacket + nCmdSetFramerate * sizeof(TCHAR));

		m_pMsgWnd->PostMessage(WM_RUIC_SET_FRAMERATE, (WPARAM) _ttoi((LPCTSTR) strParam));

		dwDataParsed = (DWORD) ((nCmdSetFramerate + strParam.GetLength() + 1) * sizeof(TCHAR));
	}
	else if (_tcsncmp((LPCTSTR) pPacket, RUI_CMD_SET_BITRATE, nCmdSetBitrate) == 0)
	{
		strParam = (LPCTSTR) (pPacket + nCmdSetBitrate * sizeof(TCHAR));

		m_pMsgWnd->PostMessage(WM_RUIC_SET_BITRATE, (WPARAM) _ttoi((LPCTSTR) strParam));

		dwDataParsed = (DWORD) ((nCmdSetBitrate + strParam.GetLength() + 1) * sizeof(TCHAR));
	}
	else if (_tcsncmp((LPCTSTR) pPacket, RUI_CMD_RESTART_ENCODER, nCmdRestartEncoder) == 0)
	{
		m_pMsgWnd->PostMessage(WM_RUIC_RESTART_ENCODER);

		dwDataParsed = (DWORD) ((nCmdRestartEncoder + strParam.GetLength() + 1) * sizeof(TCHAR));
	}
	else if (_tcsncmp((LPCTSTR) pPacket, RUI_CMD_SET_STREAMING_MODE, nCmdSetStreamingMode) == 0)
	{
		strParam = (LPCTSTR) (pPacket + nCmdSetStreamingMode * sizeof(TCHAR));

		DWORD	dwMode = 0;

		if      (strParam.CompareNoCase(_T("A" )) == 0) dwMode = RUI_CMD_STREAMING_MODE_AUDIO;
		else if (strParam.CompareNoCase(_T("V" )) == 0) dwMode = RUI_CMD_STREAMING_MODE_VIDEO;
		else if (strParam.CompareNoCase(_T("AV")) == 0) dwMode = RUI_CMD_STREAMING_MODE_VIDEO | RUI_CMD_STREAMING_MODE_AUDIO;

		m_pMsgWnd->PostMessage(WM_RUIC_SET_STREAMING_MODE, (WPARAM) dwMode, (LPARAM) this);

		dwDataParsed = (DWORD) ((nCmdSetStreamingMode + strParam.GetLength() + 1) * sizeof(TCHAR));
	}
	else if (_tcsncmp((LPCTSTR) pPacket, RUI_CMD_START_STREAMING, nCmdStartStreaming) == 0)
	{
		m_pMsgWnd->PostMessage(WM_RUIC_START_STREAMING, 0, (LPARAM) this);

		dwDataParsed = (DWORD) ((nCmdStartStreaming + strParam.GetLength() + 1) * sizeof(TCHAR));
	}
	else if (_tcsncmp((LPCTSTR) pPacket, RUI_CMD_STOP_STREAMING, nCmdStopStreaming) == 0)
	{
		m_pMsgWnd->PostMessage(WM_RUIC_STOP_STREAMING, 0, (LPARAM) this);

		dwDataParsed = (DWORD) ((nCmdStopStreaming + strParam.GetLength() + 1) * sizeof(TCHAR));
	}
	else if (_tcsncmp((LPCTSTR) pPacket, RUI_CMD_MOUSE_STATUS, nCmdMouseStatus) == 0)
	{
		strParam = (LPCTSTR) (pPacket + nCmdMouseStatus * sizeof(TCHAR));

		DWORD	dwStatus = 0;

		CString	strType2 = strParam.Left(2);

		if      (strType2.CompareNoCase(_T("MM")) == 0) dwStatus = WM_MOUSEMOVE;
		else if (strType2.CompareNoCase(_T("LD")) == 0) dwStatus = WM_LBUTTONDOWN;
		else if (strType2.CompareNoCase(_T("LU")) == 0) dwStatus = WM_LBUTTONUP;

		if (dwStatus != 0)
		{
			m_pMsgWnd->PostMessage(WM_RUIC_MOUSE_STATUS,
				(WPARAM) MAKELONG(_ttoi((LPCTSTR) (strParam.Mid(2, 4))), _ttoi((LPCTSTR) (strParam.Mid(6, 4)))),
				(LPARAM) dwStatus);
		}

		dwDataParsed = (DWORD) ((nCmdMouseStatus + strParam.GetLength() + 1) * sizeof(TCHAR));
	}
	else if (_tcsncmp((LPCTSTR) pPacket, RUI_CMD_KEYBOARD_STATUS, nCmdKeyboardStatus) == 0)
	{
		strParam = (LPCTSTR) (pPacket + nCmdKeyboardStatus * sizeof(TCHAR));

		DWORD	dwStatus = 0;

		CString	strType2 = strParam.Left(2);

		if      (strType2.CompareNoCase(_T("KD")) == 0) dwStatus = WM_KEYDOWN;
		else if (strType2.CompareNoCase(_T("KU")) == 0) dwStatus = WM_KEYUP;

		if (dwStatus != 0)
		{
			m_pMsgWnd->PostMessage(WM_RUIC_KEYBOARD_STATUS, (WPARAM) _ttoi((LPCTSTR) strParam.Mid(2, 4)),
				(LPARAM) dwStatus);
		}

		dwDataParsed = (DWORD) ((nCmdKeyboardStatus + strParam.GetLength() + 1) * sizeof(TCHAR));
	}

	if (dwDataParsed == 0)
		ASSERT(FALSE);

	return dwDataParsed;
}
