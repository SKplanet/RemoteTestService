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
#include "RUITCPSockPeer.h"
#include "IPPVideoEncoder.h"
#include "RUILibDef.h"
#include "../Common/RUIDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT WM_START_SERVER      = ::RegisterWindowMessage(_T("WM_START_SERVER"     ));
UINT WM_UPDATE_CONNECTION = ::RegisterWindowMessage(_T("WM_UPDATE_CONNECTION"));

/*
const UINT EVT_CONSUCCESS = 0x0000;	// Connection established
const UINT EVT_CONFAILURE = 0x0001;	// General failure - Wait Connection failed
const UINT EVT_CONDROP	  = 0x0002;	// Connection dropped
const UINT EVT_ZEROLENGTH = 0x0003;	// Zero length message
*/

RUITCPSockPeer::RUITCPSockPeer()
	: m_pMsgWnd(NULL)
{
	m_pieParam         = NULL;
	m_bConnected       = FALSE;
	m_pPrevBuffer      = NULL;
	m_dwPrevBufferSize = 0;

	ResetSyncTick();
}

RUITCPSockPeer::~RUITCPSockPeer()
{
	if (m_pPrevBuffer != NULL)
	{
		delete [] m_pPrevBuffer;
		m_dwPrevBufferSize = 0;
	}
}

BOOL RUITCPSockPeer::GetPeerIP(BYTE& nField0, BYTE& nField1, BYTE& nField2, BYTE& nField3)
{
	nField0 = 0;
	nField1 = 0;
	nField2 = 0;
	nField3 = 0;

	SockAddrIn	saddr_in;

	if (GetPeerName(saddr_in))
	{
		if (! saddr_in.IsNull())
		{
			LONG	uAddr = saddr_in.GetIPAddr();
			BYTE*	sAddr = (BYTE*) &uAddr;
			int		nPort = ntohs(saddr_in.GetPort()); // show port in host format...

//			CString	strAddr;
//			strAddr.Format(_T("%u.%u.%u.%u"), sAddr[0], sAddr[1], sAddr[2], sAddr[3]);

			nField0 = sAddr[0];
			nField1 = sAddr[1];
			nField2 = sAddr[2];
			nField3 = sAddr[3];

			return TRUE;
		}
	}

	return FALSE;
}

DWORD RUITCPSockPeer::ParseData(const LPBYTE pBuffer, DWORD dwBufferSize)
{
	BYTE*	pPacketSize   = pBuffer;
	BYTE*	pPacket;
	DWORD	dwPacketSize;
	DWORD	dwParsedSum   = 0;
	DWORD	dwCountRemain = dwBufferSize;

	do
	{
		dwPacketSize = *((DWORD*) pPacketSize);
		pPacket      = pPacketSize + sizeof(DWORD);

		if (dwPacketSize == 0)
		{
			ASSERT(FALSE);
			return 0;
		}

		if (dwCountRemain < (dwPacketSize + sizeof(DWORD)))
		{
			// [neuromos] 남은 버퍼가 dwPacketSize보다 작다면 다음에 처리한다. 처리된 dwParsed만 리턴한다.
			return dwParsedSum;
		}

		DWORD	dwParsed = OnDataParsed(pPacket, dwPacketSize);

		if (dwParsed == 0)
		{
			ASSERT(FALSE);
			return 0;
		}

		if (dwParsed != dwPacketSize)
		{
			ASSERT(FALSE);
			return 0;
		}

		dwParsed += sizeof(DWORD); // Size

		dwParsedSum   += dwParsed;
		dwCountRemain -= dwParsed;
		pPacketSize   += dwParsed;
	}
	while (dwCountRemain > 0);

	return dwParsedSum;
}

void RUITCPSockPeer::SendCommandString(LPCTSTR szCommand)
{
	size_t	nCommandLen      = _tcslen(szCommand) + 1; // NULL Terminal 포함
	DWORD	dwPacketBodySize = (DWORD) (nCommandLen * sizeof(TCHAR));
	DWORD	dwPacketSize     = sizeof(DWORD) + dwPacketBodySize;
	BYTE*	pPacket          = new BYTE[dwPacketSize];
	BYTE*	pPacketBody      = pPacket + sizeof(DWORD);

	*((DWORD*) pPacket) = dwPacketBodySize;
	CopyMemory(pPacketBody, szCommand, dwPacketBodySize);

	WriteComm(pPacket, dwPacketSize, INFINITE);

	delete [] pPacket;
}


DWORD RUITCPSockPeer::GetSendLatency()
{
	if (m_dwSendLatencyCount > 0)
		return m_dwSendLatencySum / m_dwSendLatencyCount;

	return 0xFFFFFFFF;
}

void RUITCPSockPeer::ResetSyncTick()
{
	m_dwSendLatency      = 0xFFFFFFFF;
	m_dwSendLatencySum   = 0;
	m_dwSendLatencyCount = 0;
	m_dwErrorMin         = 0xFFFFFFFF;
}

void RUITCPSockPeer::SendCommandSyncTick(DWORD dwMyTick, DWORD dwYourTick, int nRelMyTickSign, DWORD dwRelMyTick, int nCount)
{
	if (IsOpen())
	{
		CString	strCommand;
		strCommand.Format(_T("%s%10u%10u%01d%10u%04d"), RUI_CMD_SYNC_TICK, dwMyTick, dwYourTick, nRelMyTickSign, dwRelMyTick, nCount);
//		WriteComm((BYTE*) (LPCTSTR) strCommand, (strCommand.GetLength() + 1) * sizeof(TCHAR), INFINITE);
		SendCommandString((LPCTSTR) strCommand);
	}
}

// [neuromos] [checkpoint]
BOOL RUITCPSockPeer::ParseSyncTick(CString& strParam)
{
	DWORD	dwCurrMyTick     = GetTickCount();
	DWORD	dwYourTick       = (DWORD) _ttoi64((LPCTSTR) (strParam.Mid( 0, 10))); // 상대방이 패킷을 보낼 때 Tick
	DWORD	dwMyTick         = (DWORD) _ttoi64((LPCTSTR) (strParam.Mid(10, 10))); // 이전 패킷에 보낸 나의 Tick
	int		nRelYourTickSign =         _ttoi64((LPCTSTR) (strParam.Mid(20,  1))); // 상대방이 판단한 Tick 차이 부호. [[[자신을 기준으로 자신의 Tick이 크다면 +
	DWORD	dwRelYourTick    = (DWORD) _ttoi64((LPCTSTR) (strParam.Mid(21, 10))); // 상대방이 판단한 Tick 차이.         즉 이 값이 +라면 상대방 Tick이 큰 것임.]]]
	int		nSyncTickCount   =         _ttoi64((LPCTSTR) (strParam.Mid(31,  4))); // SyncTick을 해야 하는 남은 Count
	int		nRelMyTickSign   = 1;
	DWORD	dwRelMyTick      = 0;

#ifdef _DEBUG
	if (nSyncTickCount > 300) // strParam이 잘못 들어오는 경우를 체크
		ASSERT(FALSE);
#endif

	if (dwMyTick != 0)
	{
		DWORD	dwSendLatency  = (dwCurrMyTick - dwMyTick) / 2; // 이전 패킷에 실어 보내 Tick와 현재 Tick을 반으로 나누면(가고 오고) 네트워크 Latency를 추정할 수 있음
//		DWORD	dwSendLatency  = (dwCurrMyTick - dwMyTick) / 2; // 이전 패킷에 실어 보내 Tick와 현재 Tick으로 네트워크 Latency를 추정할 수 있음

		m_dwSendLatencySum += dwSendLatency;
		m_dwSendLatencyCount++;
		dwSendLatency = GetSendLatency();

		DWORD	dwCurrYourTick = dwYourTick - dwSendLatency;    // 상대방이 보낸 시점의 Tick에 네트워크 Latency를 반영하면 현재 상대방의 Tick을 추정할 수 있음

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
			DWORD	dwCurrMyTickByRel;
			DWORD	dwError;

			if (nRelYourTickSign > 0) dwCurrMyTickByRel = dwCurrYourTick - dwRelYourTick;
			else                      dwCurrMyTickByRel = dwCurrYourTick + dwRelYourTick;

			if (dwCurrMyTickByRel > dwCurrMyTick) dwError = dwCurrMyTickByRel - dwCurrMyTick;
			else                                  dwError = dwCurrMyTick      - dwCurrMyTickByRel;

#ifdef _DEBUG
			TRACE(_T("dwCurrMyTick(%u) dwMyTick(%u) dwYourTick(%u) dwSendLatency(%u) dwCurrMyTickByRel(%u) Error(%u)\n"),
				dwCurrMyTick, dwMyTick, dwYourTick, dwSendLatency, dwCurrMyTickByRel, dwError);
#endif

			if (dwError < m_dwErrorMin)
			{
				m_pieParam->m_nRelMyTickSign = nRelMyTickSign;
				m_pieParam->m_dwRelMyTick    = dwRelMyTick;
				m_dwErrorMin                 = dwError;
			}
		}
	}

	if ((--nSyncTickCount) > 0)
		SendCommandSyncTick(dwCurrMyTick, dwYourTick, nRelMyTickSign, dwRelMyTick, nSyncTickCount);

	// nSyncTickCount가 0이라면 위 SendCommandSyncTick를 호출하지 않았고,
	//                  1이라면 상대방의 응답이 오지 않는다.
	// SyncTick Process가 종료된 것으로 간주한다.
	if (nSyncTickCount < 2)
	{
		if (m_pMsgWnd != NULL && ::IsWindow(m_pMsgWnd->GetSafeHwnd()))
			m_pMsgWnd->PostMessage(WM_END_SYNCTICK);

		return FALSE;
	}

	return TRUE;
}

void RUITCPSockPeer::SendCommandPacketCongestion()
{
	CString	strCommand;
	strCommand.Format(_T("%s"), RUI_CMD_PACKET_CONGESTION);

	SendCommandString((LPCTSTR) strCommand);
}

void RUITCPSockPeer::OnDataReceived(const LPBYTE pBuffer, DWORD dwBufferSize)
{
#if 1
	BYTE*	pCurrBuffer;
	DWORD	dwCurrBufferSize;

	if (m_pPrevBuffer != NULL && m_dwPrevBufferSize > 0)
	{
		dwCurrBufferSize  = m_dwPrevBufferSize;
		dwCurrBufferSize += dwBufferSize;

		pCurrBuffer = new BYTE[dwCurrBufferSize];

		CopyMemory(pCurrBuffer                     , m_pPrevBuffer, m_dwPrevBufferSize);
		CopyMemory(pCurrBuffer + m_dwPrevBufferSize, pBuffer      , dwBufferSize      );

		delete [] m_pPrevBuffer;
		m_pPrevBuffer      = NULL;
		m_dwPrevBufferSize = 0;
	}
	else
	{
		dwCurrBufferSize  = dwBufferSize;

		pCurrBuffer = new BYTE[dwCurrBufferSize];

		CopyMemory(pCurrBuffer                     , pBuffer      , dwBufferSize      );
	}

	DWORD	dwParsed = ParseData(pCurrBuffer, dwCurrBufferSize);

	if (dwParsed <= dwCurrBufferSize)
	{
		m_dwPrevBufferSize = dwCurrBufferSize - dwParsed;

		if (m_dwPrevBufferSize > 0)
		{
			m_pPrevBuffer = new BYTE[m_dwPrevBufferSize];

			CopyMemory(m_pPrevBuffer, pCurrBuffer + dwParsed, m_dwPrevBufferSize);
		}
	}
	else
	{
		ASSERT(FALSE);
	}

	if (pCurrBuffer != NULL)
	{
		delete [] pCurrBuffer;
		pCurrBuffer      = NULL;
		dwCurrBufferSize = 0;
	}
#else
	LPBYTE			lpData = lpBuffer;
	SockAddrIn		origAddr;
	stMessageProxy	msgProxy;

	if (IsSmartAddressing())
	{
		dwBufferSize = __min(sizeof(msgProxy), dwBufferSize);
		memcpy(&msgProxy, lpBuffer, dwBufferSize);
		origAddr = msgProxy.address;

		if (IsServer())
		{
			// broadcast message to all
			msgProxy.address.sin_addr.s_addr = htonl(INADDR_BROADCAST);
			WriteComm((const LPBYTE) &msgProxy, dwBufferSize, 0L);
		}

		dwBufferSize -= sizeof(msgProxy.address);
		lpData = msgProxy.byData;
	}

	if (! origAddr.IsNull())
	{
		LONG	uAddr = origAddr.GetIPAddr();
		BYTE*	sAddr = (BYTE*) &uAddr;
		int		nPort = ntohs(origAddr.GetPort()); // show port in host format...
		CString	strAddr;

		// Address is stored in network format...
		strAddr.Format(_T("%u.%u.%u.%u"), sAddr[0], sAddr[1], sAddr[2], sAddr[3]);
	}
#endif
}

DWORD RUITCPSockPeer::OnDataParsed(const LPBYTE pPacket, DWORD dwPacketSize)
{
	static const size_t	nCmdSyncTickLen      = _tcslen(RUI_CMD_SYNC_TICK        );
	static const size_t	nCmdPacketCongestion = _tcslen(RUI_CMD_PACKET_CONGESTION);

	if (_tcsncmp((LPCTSTR) pPacket, RUI_CMD_SYNC_TICK, nCmdSyncTickLen) == 0)
	{
		CString	strParam = (LPCTSTR) (pPacket + nCmdSyncTickLen * sizeof(TCHAR));

		// [neuromos] [checkpoint]
		ParseSyncTick(strParam);

		return (DWORD) ((nCmdSyncTickLen + strParam.GetLength() + 1) * sizeof(TCHAR));
	}

	else if (_tcsncmp((LPCTSTR) pPacket, RUI_CMD_PACKET_CONGESTION, nCmdPacketCongestion) == 0)
	{
		UMC::IPPVideoEncoder*	pEncoder = (UMC::IPPVideoEncoder*) m_pieParam->m_pEncoder;
		if (pEncoder != NULL)
		{
			DWORD	dwBitRateModified   = pEncoder->DecreaseBitRate();
			DWORD	dwFrameTimeModified = pEncoder->IncreaseFrameTime();

			if (m_pMsgWnd != NULL && ::IsWindow(m_pMsgWnd->GetSafeHwnd()))
			{
				CString	strMessage;
				strMessage.Format(_T("Dec Bitrate : %d / Inc Frametime : %d"), dwBitRateModified, dwFrameTimeModified);
				m_pMsgWnd->SendMessage(WM_LOG_MESSAGE, (WPARAM) (LPCTSTR) strMessage);
			}
		}
		else
			ASSERT(FALSE);
	}

	return 0;
}

// Send message to parent window to indicate connection status
void RUITCPSockPeer::OnEvent(UINT uEvent, LPVOID lpvData)
{
	if (m_pMsgWnd == NULL || ! ::IsWindow(m_pMsgWnd->GetSafeHwnd()))
		return;

	switch (uEvent)
	{
		case EVT_CONSUCCESS : // Connection Established
			m_bConnected = TRUE;
			break;

		case EVT_CONFAILURE : // Connection Failed
			m_bConnected = FALSE;
			break;

		case EVT_CONDROP : // Connection Abandonned
			m_bConnected = FALSE;
			break;

		case EVT_ZEROLENGTH : // Zero Length Message
			break;

		default : // Unknown Socket event
			break;
	}

	m_pMsgWnd->PostMessage(WM_UPDATE_CONNECTION, uEvent, (LPARAM) this);
}
