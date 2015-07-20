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
#include "RUITCPSockServer.h"
#include "RUILibDef.h"
#include "../Common/RUIDef.h"

#pragma warning(disable:4996)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUITCPSockServer::RUITCPSockServer()
{
	m_pieParam = NULL;
	m_nPort    = 9095;
}

RUITCPSockServer::~RUITCPSockServer()
{
}

RUITCPSockPeer* RUITCPSockServer::AddPeer()
{
	RUITCPSockPeer*	pPeer = new RUITCPSockPeer;

	return AddPeer(pPeer);
}

RUITCPSockPeer* RUITCPSockServer::AddPeer(RUITCPSockPeer* pPeer)
{
	if (pPeer != NULL)
	{
		pPeer->SetIppExtensionParam(m_pieParam);

		m_peerList.InsertAtTail(pPeer);
	}

	return pPeer;
}

RUITCPSockPeer* RUITCPSockServer::FindPeerAvailable()
{
	RUITCPSockPeer*	pPeer = m_peerList.GetFirst();

	while (pPeer != NULL)
	{
		if (! pPeer->IsOpen())
			return pPeer;

		pPeer = m_peerList.GetNext(pPeer);
	}

	return NULL;
}

RUITCPSockPeer* RUITCPSockServer::StartServer()
{
	RUITCPSockPeer*	pNewPeer = FindPeerAvailable();

	if (pNewPeer != NULL)
	{
		CString	strPort;
		strPort.Format(_T("%d"), m_nPort);

		bool	bResult = false;

//		if (m_nSockType == SOCK_TCP)
		{
			pNewPeer->SetSmartAddressing(false); // no smart addressing - we use connection oriented
			bResult = pNewPeer->CreateSocket((LPCTSTR) strPort, AF_INET, SOCK_STREAM, SO_REUSEADDR); // TCP
		}
//		else
//		{
//			m_pNextNewServer->SetSmartAddressing(true);
//			bResult = m_pNextNewServer->CreateSocket((LPCTSTR) strPort, AF_INET, SOCK_DGRAM, SO_BROADCAST); // UDP
//		}

		if (bResult && pNewPeer->WatchComm())
		{
			pNewPeer->SetReuseAddr();

			CString	strServer,
					strAddr;

			pNewPeer->GetLocalName   (strServer.GetBuffer(_MAX_PATH), _MAX_PATH); strServer.ReleaseBuffer();
			pNewPeer->GetLocalAddress(strAddr  .GetBuffer(_MAX_PATH), _MAX_PATH); strAddr  .ReleaseBuffer();

			// [neuromos] 서버 시작이 성공함.
			CWnd*	pMsgWnd = pNewPeer->GetMessageWindow();

			if (pMsgWnd != NULL && ::IsWindow(pMsgWnd->GetSafeHwnd()))
				pMsgWnd->PostMessage(WM_START_SERVER, 0, (LPARAM) pNewPeer);

			return pNewPeer;
		}
	}

	return NULL;
}

void RUITCPSockServer::StopServer()
{
	Stop();

	RUITCPSockPeer*	pPeer = m_peerList.GetFirst();

	while (pPeer != NULL)
	{
		pPeer->StopComm();

		pPeer = m_peerList.GetNext(pPeer);
	}
}

void RUITCPSockServer::SendMessageToClient(BYTE* pMessage, DWORD dwSize)
{
	RUITCPSockPeer*	pPeer = m_peerList.GetFirst();

	while (pPeer != NULL)
	{
		if (pPeer->GetConnected())
			pPeer->WriteComm(pMessage, dwSize, INFINITE);

		pPeer = m_peerList.GetNext(pPeer);
	}
}

BOOL RUITCPSockServer::OnStart()
{
	RUIThread::OnStart();

	return TRUE;
}

void RUITCPSockServer::OnStop()
{
	TRACE("RUITCPSockServer::OnStop()\n");
	RUIThread::OnStop();
}

BOOL RUITCPSockServer::ThreadProc()
{
	static const size_t	nCmdStream = _tcslen(RUI_CMD_STREAM);

	RUIFrameBuffer*	pRUIFrameBufferSend = m_SendBufferList.PopHeadCompleted(0);

	if (pRUIFrameBufferSend != NULL)
	{
		if (pRUIFrameBufferSend->GetFrameType() == RUI_FRAME_TYPE_VIDEO && pRUIFrameBufferSend->GetFrameSubType() == RUI_FRAMESUBTYPE_I)
		{
			if (m_SendBufferList.GetFrameBufferCount(RUI_FRAME_TYPE_VIDEO) >= (m_pieParam->m_dwFrameRate * 2))
	{
		do
		{
			delete pRUIFrameBufferSend;

					pRUIFrameBufferSend = m_SendBufferList.PopHeadCompleted(RUI_FRAME_TYPE_VIDEO);
		}
		while (pRUIFrameBufferSend != NULL && pRUIFrameBufferSend->GetFrameSubType() != RUI_FRAMESUBTYPE_I);
	}
		}
	}

	if (pRUIFrameBufferSend != NULL) // 보낼 수 있는 완성된 FrameBuffer가 있다면
	{
		DWORD	dwFrameSize      = pRUIFrameBufferSend->GetFrameSize();
		DWORD	dwPacketBodySize = nCmdStream * sizeof(TCHAR) + sizeof(BYTE) + sizeof(DWORD) + sizeof(DWORD) + dwFrameSize;
		DWORD	dwPacketSize     = sizeof(DWORD) + dwPacketBodySize;

		BYTE*	pPacket      = new BYTE[dwPacketSize];
		BYTE*	pPacketField = pPacket;

		*((DWORD*) pPacketField) = dwPacketBodySize;                                                             pPacketField += sizeof(DWORD);
		_tcscpy((LPTSTR) pPacketField, RUI_CMD_STREAM);                                                          pPacketField += nCmdStream * sizeof(TCHAR);
		*((BYTE* ) pPacketField) = pRUIFrameBufferSend->GetFrameType() | pRUIFrameBufferSend->GetFrameSubType(); pPacketField += sizeof(BYTE);
		*((DWORD*) pPacketField) = pRUIFrameBufferSend->GetFrameKey();                                           pPacketField += sizeof(DWORD);
//		*((DWORD*) pPacketField) = GetTickCount();                                                               pPacketField += sizeof(DWORD);
		*((DWORD*) pPacketField) = pRUIFrameBufferSend->GetParam();                                              pPacketField += sizeof(DWORD);
		pRUIFrameBufferSend->CopyTo(pPacketField, dwFrameSize);

		SendMessageToClient(pPacket, dwPacketSize);

		delete [] pPacket;

		delete pRUIFrameBufferSend;
	}

	return TRUE;
}
