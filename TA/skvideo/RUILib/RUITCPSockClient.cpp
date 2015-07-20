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
#include "RUITCPSockClient.h"
#include "RUILibDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUITCPSockClient::RUITCPSockClient()
{
	m_RecvBufferList.SetTCPSock(this);

	SetServerState(false);
	SetSmartAddressing(false); // always send to server
}

RUITCPSockClient::~RUITCPSockClient()
{
}

BOOL RUITCPSockClient::Connect(BYTE nField0, BYTE nField1, BYTE nField2, BYTE nField3, UINT nPort)
{
	CString	strIP;
	strIP.Format(_T("%d.%d.%d.%d"), nField0, nField1, nField2, nField3);

	CString	strPort;
	strPort.Format(_T("%d"), nPort);

	bool	bSuccess = ConnectTo((LPCTSTR) strIP, (LPCTSTR) strPort, AF_INET, SOCK_STREAM); // TCP

	if (bSuccess && WatchComm())
	{
		SetReuseAddr();

		return TRUE;
	}

	return FALSE;
}

void RUITCPSockClient::OnDataReceived(const LPBYTE pBuffer, DWORD dwBufferSize)
{
	RUITCPSockPeer::OnDataReceived(pBuffer, dwBufferSize);
}

DWORD RUITCPSockClient::OnDataParsed(const LPBYTE pPacket, DWORD dwPacketSize)
{
	DWORD	dwDataParsed = 0;

	if ((dwDataParsed = RUITCPSockPeer::OnDataParsed(pPacket, dwPacketSize)) > 0)
		return dwDataParsed;

	static const size_t	nCmdStream = _tcslen(RUI_CMD_STREAM);

	if (_tcsncmp((LPCTSTR) pPacket, RUI_CMD_STREAM, nCmdStream) == 0)
	{
		DWORD	dwFrameSize  = dwPacketSize;
		BYTE*	pPacketField = pPacket + nCmdStream * sizeof(TCHAR);                    dwFrameSize -= nCmdStream * sizeof(TCHAR);
		BYTE	nFrameType   = *((BYTE* ) pPacketField); pPacketField += sizeof(BYTE);  dwFrameSize -= sizeof(BYTE);
		DWORD	dwFrameKey   = *((DWORD*) pPacketField); pPacketField += sizeof(DWORD); dwFrameSize -= sizeof(DWORD);
		DWORD	dwFrameTick  = *((DWORD*) pPacketField); pPacketField += sizeof(DWORD); dwFrameSize -= sizeof(DWORD);

		BYTE	nFrameSubType;

		nFrameSubType = nFrameType & 0x0F;
		nFrameType    = nFrameType & 0xF0;

		if (m_RecvBufferList.GetEnableFrameType(nFrameType))
		{
			m_RecvBufferList.AddFrameTypeSum(nFrameType, dwFrameSize);

			RUIFrameBuffer*	pRUIFrameBuffer =
				m_RecvBufferList.AddFrameBuffer(nFrameType, nFrameSubType, dwFrameKey, dwFrameSize, dwFrameTick, pPacketField, dwFrameSize, 0);

			::SetEvent(m_pieParam->m_hEventRecv);
		}

		dwDataParsed = dwPacketSize;
	}

	return dwDataParsed;
}
