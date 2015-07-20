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
#include "RUIUDPSock.h"
#include <process.h>
#include "IPPVideoEncoder.h"
#include "RUISockUtil.h"
#include "RUILibDef.h"
#include "../Common/RUIDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning(disable:4996)

RUIUDPSock::RUIUDPSock()
{
	m_RecvBufferList.SetUDPSock(this);

	m_pieParam         = NULL;
	m_pMsgWnd          = NULL;

	m_bStunRestriction = FALSE;
	m_bClientStreaming = FALSE;

	m_hSocket          = INVALID_SOCKET;

	m_szDestIP[0]      = 0;
	m_uiSendPort       = 0;
	m_uiDestPort       = 0;
	m_uiRecvPort       = 0;

	m_nPacketSize      = 0;
	m_pBufferSend      = NULL;
	m_pBufferRecv      = NULL;

	SetPacketSize(UDP_FRAME_PACKET_SIZE);

	ResetSyncTick();
	m_RecvBufferList.ResetAllEnableFrameType();

	InitWinsock();

	SetSleepTime(10);
//	SetSleepTime(0);
}

RUIUDPSock::~RUIUDPSock()
{
	Close();

	ExitWinsock();

	RemoveBufferSend();
	RemoveBufferRecv();
}

BOOL RUIUDPSock::InitWinsock()
{
	WORD		wVersionRequested = MAKEWORD(1, 1);
	WSADATA		wsaData;
	int			nRet;

	nRet = WSAStartup(wVersionRequested, &wsaData);

	if (nRet)
	{
		ASSERT(FALSE);
		WSACleanup();

		return FALSE;
	}

	if (wsaData.wVersion != wVersionRequested)
	{
		ASSERT(FALSE);
		WSACleanup();

		return FALSE;
	}

	return TRUE;
}

void RUIUDPSock::ExitWinsock()
{
	WSACleanup();
}

UINT RUIUDPSock::GetBufferSize(int nOptName, SOCKET hSocket)
{
	UINT	nCurrSize;
	int		nSizeSize = sizeof(nCurrSize);

	if (getsockopt(hSocket, SOL_SOCKET, nOptName, (char*) &nCurrSize, &nSizeSize) < 0)
	{
		ASSERT(FALSE);
		return 0;
	}

	return nCurrSize;
}

UINT RUIUDPSock::GetSendBufferSize(SOCKET hSocket)
{
	return GetBufferSize(SO_SNDBUF, hSocket);
}

UINT RUIUDPSock::GetRecvBufferSize(SOCKET hSocket)
{
	return GetBufferSize(SO_RCVBUF, hSocket);
}

UINT RUIUDPSock::IncreaseBuffer(int nOptName, SOCKET hSocket, unsigned nReqSize)
{
	// First, get the current buffer size.  If it's already at least as big as what we're requesting, do nothing.
	UINT	nCurrSize = GetBufferSize(nOptName, hSocket);

	// Next, try to increase the buffer to the requested size, or to some smaller size, if that's not possible:
	while (nReqSize > nCurrSize)
	{
		int	nSizeSize = sizeof(nReqSize);

		if (setsockopt(hSocket, SOL_SOCKET, nOptName, (char*) &nReqSize, nSizeSize) >= 0)
		{
			// success
			return nReqSize;
		}

		nReqSize = (nReqSize + nCurrSize) / 2;
	}

	return GetBufferSize(nOptName, hSocket);
}

UINT RUIUDPSock::IncreaseSendBuffer(SOCKET hSocket, unsigned nReqSize)
{
	return IncreaseBuffer(SO_SNDBUF, hSocket, nReqSize);
}

UINT RUIUDPSock::IncreaseRecvBuffer(SOCKET hSocket, unsigned nReqSize)
{
	return IncreaseBuffer(SO_RCVBUF, hSocket, nReqSize);
}

int RUIUDPSock::GetPacketSize()
{
	return m_nPacketSize;
}

void RUIUDPSock::SetPacketSize(int nPacketSize)
{
	m_nPacketSize = nPacketSize;

	AllocBufferSend(m_nPacketSize);
	AllocBufferRecv(m_nPacketSize);
}

BYTE* RUIUDPSock::AllocBufferSend(int nSize)
{
	RemoveBufferSend();

	if (nSize > 0)
		m_pBufferSend = new BYTE[nSize];

	return m_pBufferSend;
}

BYTE* RUIUDPSock::AllocBufferRecv(int nSize)
{
	RemoveBufferRecv();

	if (nSize > 0)
		m_pBufferRecv = new BYTE[nSize];

	return m_pBufferRecv;
}

void RUIUDPSock::RemoveBufferSend()
{
	if (m_pBufferSend != NULL)
	{
		delete [] m_pBufferSend;
		m_pBufferSend = NULL;
	}
}

void RUIUDPSock::RemoveBufferRecv()
{
	if (m_pBufferRecv != NULL)
	{
		delete [] m_pBufferRecv;
		m_pBufferRecv = NULL;
	}
}

BOOL RUIUDPSock::OpenSocket(USHORT uiSendPort, LPCSTR szDestIP, USHORT uiDestPort, USHORT uiRecvPort, UINT nSendBufferSize, UINT nRecvBufferSize)
{
	CloseSocket();

	m_hSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_hSocket != INVALID_SOCKET)
	{
		if (nSendBufferSize > 0)
			IncreaseSendBuffer(m_hSocket, nSendBufferSize);

		if (nRecvBufferSize > 0)
			IncreaseRecvBuffer(m_hSocket, nRecvBufferSize);

		SOCKADDR_IN		sa;

		sa.sin_family      = AF_INET;
		sa.sin_port        = htons(uiRecvPort);
		sa.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(m_hSocket, (LPSOCKADDR) &sa, sizeof(SOCKADDR_IN)) != SOCKET_ERROR)
		{
			strcpy(m_szDestIP, szDestIP);
			m_uiSendPort = uiSendPort;
			m_uiDestPort = uiDestPort;
			m_uiRecvPort = uiRecvPort;

			return TRUE;
		}

		CloseSocket();
	}

	ASSERT(FALSE);

	return FALSE;
}

BOOL RUIUDPSock::CloseSocket()
{
	m_bStunRestriction = FALSE;
	m_bClientStreaming = FALSE;

	m_szDestIP[0]      = 0;
	m_uiSendPort       = 0;
	m_uiDestPort       = 0;
	m_uiRecvPort       = 0;

	if (m_hSocket != INVALID_SOCKET)
	{
		closesocket(m_hSocket);
		m_hSocket = INVALID_SOCKET;

		return TRUE;
	}

	return FALSE;
}

BOOL RUIUDPSock::OpenSend(USHORT uiSendPort, LPCSTR szDestIP, USHORT uiDestPort)
{
	Stop();

	if (m_nPacketSize == 0 || m_pBufferSend == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (OpenSocket(uiSendPort, szDestIP, uiDestPort, 0, SEND_BUFFER_SIZE, RECV_BUFFER_SIZE))
	{
		if (Run())
			return TRUE;

		CloseSocket();
	}

	ASSERT(FALSE);

	return FALSE;
}

BOOL RUIUDPSock::OpenRecv(USHORT uiSendPort, LPCSTR szDestIP, USHORT uiDestPort, USHORT uiRecvPort)
{
	Stop();

	if (m_nPacketSize == 0 || m_pBufferRecv == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (OpenSocket(uiSendPort, szDestIP, uiDestPort, uiRecvPort, SEND_BUFFER_SIZE, RECV_BUFFER_SIZE))
	{
		if (Run())
			return TRUE;

		CloseSocket();
	}

	ASSERT(FALSE);

	return FALSE;
}

BOOL RUIUDPSock::OpenStun(USHORT uiSendPort, LPCSTR szDestIP, USHORT uiDestPort, USHORT uiRecvPort)
{
	Stop();

	if (m_nPacketSize == 0 || m_pBufferSend == NULL || m_pBufferRecv == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (OpenSocket(uiSendPort, szDestIP, uiDestPort, uiRecvPort, SEND_BUFFER_SIZE, RECV_BUFFER_SIZE))
	{
		m_bStunRestriction = TRUE;

		if (Run())
			return TRUE;

		CloseSocket();
	}

	ASSERT(FALSE);

	return FALSE;
}

void RUIUDPSock::Close()
{
	Stop();

	CloseSocket();
}

BOOL RUIUDPSock::SendStunPacket(LPCSTR szDestIP, UINT uiDestPort)
{
	if (m_hSocket == INVALID_SOCKET)
		return FALSE;

#if 1
	SendCommandStun();
#else
	BYTE*	szSTUN = new BYTE[m_nPacketSize];
	ZeroMemory(szSTUN, m_nPacketSize);
	_tcscpy((LPTSTR) szSTUN, RUI_CMD_STUN);

	SendDelayPacket(szDestIP, uiDestPort, 100, (BYTE*) szSTUN, m_nPacketSize);

	delete [] szSTUN;
#endif

	return TRUE;
}

RUIBuffer* RUIUDPSock::SendDelayPacket(LPCSTR szDestIP, UINT uiDestPort, DWORD dwTickAfter, BYTE* pBuffer, DWORD dwSize)
{
	if (szDestIP == NULL || pBuffer == NULL || dwSize == 0)
	{
		ASSERT(FALSE);
		return NULL;
	}

	if (szDestIP[0] == 0 || uiDestPort == 0)
		ASSERT(FALSE);

	RUIBuffer*	pRUIBuffer = m_DelaySendBufferList.AddBuffer(pBuffer, dwSize);

	if (pRUIBuffer != NULL)
	{
		DelaySendPacketContext*	pdspc = new DelaySendPacketContext;

		strcpy(pdspc->szDestIP, szDestIP);
		pdspc->uiDestPort   = uiDestPort;
		pdspc->dwTickToSend = GetTickCount() + dwTickAfter;

		pRUIBuffer->SetParam((DWORD) pdspc);

		return pRUIBuffer;
	}

	return NULL;
}

int RUIUDPSock::SendPacket(BYTE* pBuffer, int nSize, LPCSTR szDestIP, USHORT uiDestPort)
{
	if (m_bStunRestriction) // [neuromos] [STUN] Client로 부터 Command가 바로 와서 m_bClientStreaming이 TRUE일 수 있음.
		return 0;

	if (m_hSocket == INVALID_SOCKET)
	{
		ASSERT(FALSE);
		return 0;
	}

	if (szDestIP   == NULL) szDestIP   = m_szDestIP;
	if (uiDestPort == 0   ) uiDestPort = m_uiDestPort;

	if (szDestIP[0] == 0 || uiDestPort == 0)
		ASSERT(FALSE);

	SOCKADDR_IN		sa;

	sa.sin_family      = AF_INET;
	sa.sin_port        = htons    (uiDestPort);
	sa.sin_addr.s_addr = inet_addr(szDestIP  );

	int	nSent = sendto(m_hSocket, (const char*) pBuffer, nSize, 0, (LPSOCKADDR) &sa, sizeof(SOCKADDR_IN));

	return nSent;
}

BOOL RUIUDPSock::RecvPacket(BYTE* pBuffer, int nSize, int& nRecv, LPSTR szFromIP, USHORT& uiFromPort)
{
	nRecv = 0;

	if (m_hSocket == INVALID_SOCKET)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	ULONG	ulRecv = 0L;

	if (ioctlsocket(m_hSocket, FIONREAD, &ulRecv) == 0)
	{
		if (ulRecv <= 0)
			return FALSE;
	}
	else
	{
		ASSERT(FALSE);	
		return FALSE;
	}

	ZeroMemory(pBuffer, nSize);

	SOCKADDR_IN		sa;
	INT				iAddrSize = sizeof(SOCKADDR_IN);

	nRecv = recvfrom(m_hSocket, (char*) pBuffer, nSize, 0, (LPSOCKADDR) &sa, &iAddrSize);

	if (nRecv == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK) // WOULDBLOCK can be ignored
			return FALSE;
	}

	// [neuromos] 받은 크기가 비정상적인지 체크하기 위한 코드. 패킷 사이즈는 항상 m_nPacketSize이다.
#ifdef _DEBUG
	if (nRecv != nSize)
		ASSERT(FALSE);
#endif

	strcpy(szFromIP, inet_ntoa(sa.sin_addr));
	uiFromPort = ntohs(sa.sin_port);

	return TRUE;
}

BOOL RUIUDPSock::OnStart()
{
	RUIThread::OnStart();

	if (m_nPacketSize == 0 || m_pBufferSend == NULL || m_pBufferRecv == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	// [neuromos] Command는 무조건 받는다.
	m_RecvBufferList.SetEnableFrameType(RUI_FRAME_TYPE_COMMAND);

	return TRUE;
}

void RUIUDPSock::OnStop()
{
	RUIThread::OnStop();
}

BOOL RUIUDPSock::Proc_Send()
{
	BOOL	bResult = TRUE;

	RUIFrameBuffer*	pRUIFrameBufferSend = m_SendBufferList.PopHeadCompleted(0);

	while (pRUIFrameBufferSend != NULL) // 보낼 수 있는 완성된 FrameBuffer가 있다면
	{
		DWORD	dwFrameSize = pRUIFrameBufferSend->GetFrameSize();
		BOOL	bSend       = TRUE;
		DWORD	dwSendSize  = 0;
		USHORT	nSliceIndex = 0;

		while (bSend && dwSendSize < dwFrameSize)
		{
			DWORD	dwSend = m_SendBufferList.GetUDPPacket(pRUIFrameBufferSend, nSliceIndex, m_pBufferSend, m_nPacketSize);

			if (dwSend > 0)
			{
				int	nSent = SendPacket(m_pBufferSend, m_nPacketSize);

				if (nSent == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK) // WOULDBLOCK can be ignored
					{
						ASSERT(FALSE);
						bSend   = FALSE;
						bResult = FALSE;
						break;
					}
				}

				if (nSent != (int) m_nPacketSize)
					ASSERT(FALSE);

#ifdef _DEBUG
//				static int	s_Progress = 0;
//				TRACE(_T("s"));
//				if ((++s_Progress) % 30 == 0)
//					TRACE(_T("\n"));
#endif

				nSliceIndex++;
				dwSendSize += dwSend;
			}
			else
			{
				ASSERT(FALSE);
				bSend = FALSE;
			}
		}

		if (dwSendSize != dwFrameSize) // FrameSize를 다 보내지 못했다면
			ASSERT(FALSE);

		delete pRUIFrameBufferSend;

		pRUIFrameBufferSend = m_SendBufferList.PopHeadCompleted(0);
	}

	return bResult;
}

BOOL RUIUDPSock::Proc_DelaySend()
{
	DWORD		dwCurrTick = GetTickCount();

	RUIBuffer*	pRUIBuffer = m_DelaySendBufferList.GetFirst();

	while (pRUIBuffer != NULL)
	{
		RUIBuffer*	pRUIBufferNext = m_DelaySendBufferList.GetNext(pRUIBuffer);

		DelaySendPacketContext*	pdspc = (DelaySendPacketContext*) pRUIBuffer->GetParam();

		if (pdspc->dwTickToSend < dwCurrTick)
		{
			SendPacket(pRUIBuffer->GetBuffer(), pRUIBuffer->GetBufferSize(), pdspc->szDestIP, pdspc->uiDestPort);

			delete pdspc;
			m_DelaySendBufferList.Remove(pRUIBuffer);
		}

		pRUIBuffer = pRUIBufferNext;
	}

	return TRUE;
}

BOOL RUIUDPSock::Proc_Command()
{
	// Command 처리
	RUIFrameBuffer*	pRUIFrameBuffer = m_RecvBufferList.PopHeadCompleted(RUI_FRAME_TYPE_COMMAND);

	if (pRUIFrameBuffer != NULL)
	{
		DWORD	dwCommandSize = pRUIFrameBuffer->GetFrameSize();

		if (dwCommandSize > 0)
		{
			BYTE*	pCommand = new BYTE[dwCommandSize];

			pRUIFrameBuffer->CopyTo(pCommand, dwCommandSize);
			ParseData(pCommand, dwCommandSize);

			delete [] pCommand;
		}

		delete pRUIFrameBuffer;

		return TRUE;
	}

	return FALSE;
}

BOOL RUIUDPSock::Proc_Recv()
{
	int		nRecv;
	char	szFromIP[_MAX_PATH];
	USHORT	uiFromPort;

	while (RecvPacket(m_pBufferRecv, m_nPacketSize, nRecv, szFromIP, uiFromPort))
	{
		if (nRecv == m_nPacketSize)
		{
			if (m_bStunRestriction)
			{
				UDPFrameHeaderFirst*	pHeaderFirst = (UDPFrameHeaderFirst*) m_pBufferRecv;

				if(pHeaderFirst->nFrameType == RUI_FRAME_TYPE_COMMAND)
				{
					ParseData((m_pBufferRecv + sizeof(UDPFrameHeaderFirst)), pHeaderFirst->dwFrameSize);

					if (m_pMsgWnd != NULL && ::IsWindow(m_pMsgWnd->GetSafeHwnd()))
					{
						TCHAR	szIPWidechar[_MAX_PATH];
						IP_MultibyteToWidechar(szFromIP, szIPWidechar, _MAX_PATH);

						CString	strMessage;
						strMessage.Format(_T("[STN] IP:%s PORT:%d MSG:%s"),
							szIPWidechar, uiFromPort, (LPCTSTR) (m_pBufferRecv + sizeof(UDPFrameHeaderFirst)));
						m_pMsgWnd->SendMessage(WM_LOG_MESSAGE, (WPARAM) (LPCTSTR) strMessage);
					}
				}
				else
					ASSERT(FALSE); // m_bStunRestriction 인 상태에서 Command 말고 들어오면 안된다.
			}
			else
			{
				RUIFrameBuffer*	pRUIFrameBuffer = m_RecvBufferList.OnReceiveUDPFrame(m_pBufferRecv, m_nPacketSize);

				Proc_Command();
			}
		}
		else
			ASSERT(FALSE);
	}

	return TRUE;
}

BOOL RUIUDPSock::ThreadProc()
{
	BOOL	bResultSend = TRUE;
	BOOL	bResultRecv = TRUE;

	bResultSend = Proc_Send();
	bResultRecv = Proc_Recv();

	Proc_DelaySend();

	return (bResultSend && bResultRecv);
}

BOOL RUIUDPSock::ParseData(LPBYTE lpBuffer, DWORD dwCount)
{
	LPCTSTR	szData = (LPCTSTR) lpBuffer;
	int		nLen   = (int) (dwCount / sizeof(TCHAR)); // NULL 포함 길이

	// [neuromos] !lpBuffer가 Parsing 가능한 것이려면 데이터의 마지막은 NULL이어야 한다.
	if (szData == NULL || nLen < 2 || szData[nLen - 1] != 0)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	while (nLen > 0)
	{
		CString	strToken  = szData;
		int		nLenToken = strToken.GetLength();

		OnDataParsed((LPCTSTR) strToken, nLenToken);

		szData += nLenToken + 1;
		nLen   -= nLenToken + 1;
	}

	return TRUE;
}

void RUIUDPSock::OnDataParsed(LPCTSTR szData, int nLen)
{
	CString	strCommand = szData;
	CString	strType    = strCommand.Left(2);
	CString	strParam   = strCommand.Mid (2);

	if (strType.Compare(RUI_CMD_STUN) == 0)
	{
		m_bStunRestriction = FALSE;
		SendCommandStunAck();
	}
	else if (strType.Compare(RUI_CMD_STUN_ACK) == 0)
	{
		// [neuromos] UDP의 경우 Client에서 RUI_CMD_STUN를 보내고 RUI_CMD_STUN_ACK를 받으면 SyncTick Process를 시작한다.
		SendCommandSyncTick(GetTickCount(), 0, 1, 0, 20);
	}
	else if (strType.Compare(RUI_CMD_SYNC_TICK) == 0)
	{
		// [neuromos] [checkpoint]
		if (! ParseSyncTick(strParam))
		{
			// SyncTick Process가 정상적으로 종료되었다.
		}
	}
	else if (strType.Compare(RUI_CMD_PACKET_CONGESTION) == 0)
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
}

BOOL RUIUDPSock::SendCommand(LPCTSTR szCommand, BOOL bDelay, DWORD dwTickAfter)
{
	if (szCommand != NULL)
	{
		int	nHeaderLen  = (int) sizeof(UDPFrameHeaderFirst);
		int	nCommandLen = (int) ((_tcslen(szCommand) + 1) * sizeof(TCHAR));

		if ((nHeaderLen + nCommandLen) <= m_nPacketSize)
		{
			BYTE*	pPacket = new BYTE[m_nPacketSize];

			UDPFrameHeaderFirst*	pHeaderFirst = (UDPFrameHeaderFirst*) pPacket;
			LPTSTR					szCommand2   = (LPTSTR) (pPacket + sizeof(UDPFrameHeaderFirst));

			pHeaderFirst->nFrameType  = RUI_FRAME_TYPE_COMMAND;
			pHeaderFirst->dwFrameKey  = 0;
			pHeaderFirst->nSliceIndex = 0;
			pHeaderFirst->dwFrameSize = nCommandLen;
			pHeaderFirst->dwFrameTick = 0;

			_tcscpy(szCommand2, szCommand);

			if (bDelay)
				SendDelayPacket(m_szDestIP, m_uiDestPort, dwTickAfter, pPacket, m_nPacketSize);
			else
				SendPacket                                            (pPacket, m_nPacketSize);

			delete [] pPacket;

			return TRUE;
		}
	}

	ASSERT(FALSE);

	return FALSE;
}

void RUIUDPSock::SendCommandStun()
{
	CString	strCommand;
	strCommand.Format(_T("%s"), RUI_CMD_STUN);

	SendCommand((LPCTSTR) strCommand, TRUE, 100);
}

void RUIUDPSock::SendCommandStunAck()
{
	CString	strCommand;
	strCommand.Format(_T("%s"), RUI_CMD_STUN_ACK);

	SendCommand((LPCTSTR) strCommand, FALSE, 0);
}

void RUIUDPSock::SendCommandSyncTick(DWORD dwMyTick, DWORD dwYourTick, int nRelMyTickSign, DWORD dwRelMyTick, int nCount)
{
	CString	strCommand;
	strCommand.Format(_T("%s%10u%10u%01d%10u%04d"), RUI_CMD_SYNC_TICK, dwMyTick, dwYourTick, nRelMyTickSign, dwRelMyTick, nCount);

	SendCommand((LPCTSTR) strCommand, FALSE, 0);
}

void RUIUDPSock::SendCommandPacketCongestion()
{
	CString	strCommand;
	strCommand.Format(_T("%s"), RUI_CMD_PACKET_CONGESTION);

	SendCommand((LPCTSTR) strCommand, FALSE, 0);
}

//--- Sync Tick -----------------------------------------------------------------------------------

DWORD RUIUDPSock::GetSendLatency()
{
	if (m_dwSendLatencyCount > 0)
		return m_dwSendLatencySum / m_dwSendLatencyCount;

	return 0xFFFFFFFF;
}

void RUIUDPSock::ResetSyncTick()
{
	m_dwSendLatency      = 0xFFFFFFFF;
	m_dwSendLatencySum   = 0;
	m_dwSendLatencyCount = 0;
	m_dwErrorMin         = 0xFFFFFFFF;
}

// [neuromos] [checkpoint]
BOOL RUIUDPSock::ParseSyncTick(CString& strParam)
{
	DWORD	dwCurrMyTick     = GetTickCount();
	DWORD	dwYourTick       = (DWORD) _ttol((LPCTSTR) (strParam.Mid( 0, 10))); // 상대방이 패킷을 보낼 때 Tick
	DWORD	dwMyTick         = (DWORD) _ttol((LPCTSTR) (strParam.Mid(10, 10))); // 이전 패킷에 보낸 나의 Tick
	int		nRelYourTickSign =         _ttoi((LPCTSTR) (strParam.Mid(20,  1))); // 상대방이 판단한 Tick 차이 부호. [[[자신을 기준으로 자신의 Tick이 크다면 +
	DWORD	dwRelYourTick    = (DWORD) _ttol((LPCTSTR) (strParam.Mid(21, 10))); // 상대방이 판단한 Tick 차이.         즉 이 값이 +라면 상대방 Tick이 큰 것임.]]]
	int		nSyncTickCount   =         _ttoi((LPCTSTR) (strParam.Mid(31,  4))); // SyncTick을 해야 하는 남은 Count
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
		if (m_pMsgWnd != NULL)
			m_pMsgWnd->PostMessage(WM_END_SYNCTICK);

		return FALSE;
	}

	return TRUE;
}
