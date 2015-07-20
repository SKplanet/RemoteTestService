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
#include "RUIUDPSockServer.h"
#include <process.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUIUDPSockServer::RUIUDPSockServer()
{
	m_pieParam    = NULL;

	m_nPacketSize = 0;
	m_pBufferSend = NULL;

	SetPacketSize(UDP_FRAME_PACKET_SIZE);

	InitWinsock();

	SetSleepTime(2);
}

RUIUDPSockServer::~RUIUDPSockServer()
{
	CloseUDPSock();

	ExitWinsock();

	RemoveBufferSend();
}

BOOL RUIUDPSockServer::InitWinsock()
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

void RUIUDPSockServer::ExitWinsock()
{
	WSACleanup();
}

int RUIUDPSockServer::GetPacketSize()
{
	return m_nPacketSize;
}

void RUIUDPSockServer::SetPacketSize(int nPacketSize)
{
	m_nPacketSize = nPacketSize;

	AllocBufferSend(m_nPacketSize);
}

BYTE* RUIUDPSockServer::AllocBufferSend(int nSize)
{
	RemoveBufferSend();

	if (nSize > 0)
		m_pBufferSend = new BYTE[nSize];

	return m_pBufferSend;
}

void RUIUDPSockServer::RemoveBufferSend()
{
	if (m_pBufferSend != NULL)
	{
		delete [] m_pBufferSend;
		m_pBufferSend = NULL;
	}
}

void RUIUDPSockServer::SendRecvThreadProc()
{
}

RUIUDPSock* RUIUDPSockServer::AddUDPSock()
{
	return m_udpSockList.AddUDPSock();
}

void RUIUDPSockServer::RemoveUDPSock(RUIUDPSock* pUDPSock)
{
	m_udpSockList.RemoveUDPSock(pUDPSock);
}

void RUIUDPSockServer::StreamTo(BYTE* pBuffer, int nPacketSize)
{
	m_udpSockList.StreamTo(pBuffer, nPacketSize);
}

void RUIUDPSockServer::CloseUDPSock()
{
	m_udpSockList.CloseUDPSock();
}

BOOL RUIUDPSockServer::OnStart()
{
	RUIThread::OnStart();

	if (m_nPacketSize == 0 || m_pBufferSend == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return TRUE;
}

void RUIUDPSockServer::OnStop()
{
	RUIThread::OnStop();

	CloseUDPSock();
}

BOOL RUIUDPSockServer::ThreadProc()
{
	RUIFrameBuffer*	pRUIFrameBufferSend = m_SendBufferList.PopHeadCompleted(0);

	if (pRUIFrameBufferSend != NULL) // 보낼 수 있는 완성된 FrameBuffer가 있다면
	{
		BOOL	bSend       = TRUE;
		USHORT	nSliceIndex = 0;

		DWORD	dwFrameSize = pRUIFrameBufferSend->GetFrameSize();
		DWORD	dwSendSize  = 0;

//		while (bSend)
		while (bSend && dwSendSize < dwFrameSize)
		{
			DWORD	dwSend = m_SendBufferList.GetUDPPacket(pRUIFrameBufferSend, nSliceIndex, m_pBufferSend, m_nPacketSize);

			if (dwSend != 0)
			{
//				int	nSent = sendto(m_hSocket, (const char*) m_pBufferSend, m_nPacketSize, 0, (LPSOCKADDR) &sa, sizeof(SOCKADDR_IN));
				StreamTo(m_pBufferSend, m_nPacketSize);

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

#ifdef _DEBUG // [neuromos] 큰 Frame(I)만 찍는다.
//		if (dwSendSize > 10000)
//			TRACE(_T("%d(%d)\n"), pRUIFrameBufferSend->GetFrameKey(), dwSendSize);
#endif

		delete pRUIFrameBufferSend;
	}

	return TRUE;
}
