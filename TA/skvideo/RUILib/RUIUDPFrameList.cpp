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
#include "RUIUDPFrameList.h"
#include "RUIUDPSock.h"
#include "RUITCPSockPeer.h"
#include "RUILibDef.h"
#include "../Common/RUIDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _USE_CHECK_DROP_FRAME
UINT WM_DROP_FRAME = ::RegisterWindowMessage(_T("WM_DROP_FRAME"));
#endif

RUIUDPFrameList::RUIUDPFrameList()
{
	m_pieParam = NULL;

	SetPacketSize(UDP_FRAME_PACKET_SIZE);

	ResetUDPFrameList();

	m_nCongestionCount = 0;
	m_pUDPSock         = NULL;
	m_pTCPSock         = NULL;

#ifdef _USE_CHECK_DROP_FRAME
	m_pMsgWnd = NULL;
#endif
}

RUIUDPFrameList::~RUIUDPFrameList()
{
}

RUIFrameBuffer* RUIUDPFrameList::_GetFirst(BYTE nFrameType)
{
	RUIFrameBuffer*	pRUIFrameBuffer = (RUIFrameBuffer*) GetFirst();

	while (pRUIFrameBuffer != NULL)
	{
		if (nFrameType == 0 || pRUIFrameBuffer->GetFrameType() == nFrameType)
			return pRUIFrameBuffer;

		pRUIFrameBuffer = (RUIFrameBuffer*) GetNext(pRUIFrameBuffer);
	}

	return NULL;
}

RUIFrameBuffer* RUIUDPFrameList::_GetNext(BYTE nFrameType, RUIFrameBuffer* pRUIFrameBuffer)
{
	if (pRUIFrameBuffer != NULL)
		pRUIFrameBuffer = (RUIFrameBuffer*) GetNext(pRUIFrameBuffer);

	while (pRUIFrameBuffer != NULL)
	{
		if (nFrameType == 0 || pRUIFrameBuffer->GetFrameType() == nFrameType)
			return pRUIFrameBuffer;

		pRUIFrameBuffer = (RUIFrameBuffer*) GetNext(pRUIFrameBuffer);
	}

	return NULL;
}

void RUIUDPFrameList::ResetAllEnableFrameType()
{
	for (int i = 0; i < UDP_FRAME_TYPE_MAX; i++)
//		m_bEnableFrameType[i] = FALSE;
		ResetEnableFrameType(i);
}

void RUIUDPFrameList::SetEnableFrameType(BYTE nFrameType)
{
	m_bEnableFrameType[nFrameType] = TRUE;
}

void RUIUDPFrameList::ResetEnableFrameType(BYTE nFrameType)
{
	m_bEnableFrameType[nFrameType] = FALSE;
	m_bDropOldFrame   [nFrameType] = TRUE;
	m_n64FrameTypeSum [nFrameType] = 0;
	m_dwFrameKeyLast  [nFrameType] = 0;

	if (m_pieParam != NULL)
		m_pieParam->m_dwLastFrameTick[nFrameType] = 0;
}

BOOL RUIUDPFrameList::GetDropOldFrame(BYTE nFrameType)
{
	return m_bDropOldFrame[nFrameType];
}

void RUIUDPFrameList::SetDropOldFrame(BYTE nFrameType, BOOL bDropOldFrame)
{
	m_bDropOldFrame[nFrameType] = bDropOldFrame;
}

__int64 RUIUDPFrameList::GetFrameTypeSum(BYTE nFrameType)
{
	return m_n64FrameTypeSum[nFrameType];
}

__int64 RUIUDPFrameList::AddFrameTypeSum(BYTE nFrameType, DWORD dwSize)
{
	m_n64FrameTypeSum[nFrameType] += (__int64) dwSize;

	return m_n64FrameTypeSum[nFrameType];
}

void RUIUDPFrameList::ResetUDPFrameList()
{
	for (int i = 0; i < UDP_FRAME_TYPE_MAX; i++)
		m_dwLastFrameKey[i] = 0;

	m_dwFrameDropCount = 0;
}

UINT RUIUDPFrameList::AddUDPFrame(BYTE nFrameType, BYTE nFrameSubType, DWORD dwFrameKey, DWORD dwFrameSize, DWORD dwParam, BYTE* pBuffer, DWORD dwBufferSize, DWORD dwStart)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	if (dwFrameSize == 0 || pBuffer == NULL || dwBufferSize == NULL)
	{
		ASSERT(FALSE);
		return 0;
	}

	RUIFrameBuffer*	pRUIFrameBuffer = _FindFrameBuffer(nFrameType, dwFrameKey);
	DWORD			dwSliceSize     = 0;

	if (dwStart == 0)
	{
#ifdef _DEBUG
		if (pRUIFrameBuffer != NULL) // dwStart가 0보다 큰값으로 미리 만드는 경우? 중간부터 만드는 경우는 없다.
		{
			ASSERT(FALSE);
			return 0;
		}
#endif

		dwSliceSize = m_dwPacketSize - sizeof(UDPFrameHeaderFirst);
	}
	else
	{
#ifdef _DEBUG
		if (pRUIFrameBuffer == NULL) // dwStart가 0보다 큰값으로 미리 만드는 경우? 중간부터 만드는 경우는 없다.
		{
			ASSERT(FALSE);
			return 0;
		}

		if (pRUIFrameBuffer->GetFrameSize() != dwFrameSize)
			ASSERT(FALSE);
#endif

		dwSliceSize = m_dwPacketSize - sizeof(UDPFrameHeader);
	}

	UINT	nCount       = 0;
	DWORD	dwRemainSize = dwBufferSize;
	BYTE*	pBufferCurr  = pBuffer;
	DWORD	dwStartCurr  = dwStart;

	while (dwRemainSize > 0)
	{
		if (dwSliceSize > dwRemainSize)
			dwSliceSize = dwRemainSize;

		_AddFrameBuffer(nFrameType, nFrameSubType, dwFrameKey, dwFrameSize, dwParam, pBufferCurr, dwSliceSize, dwStartCurr);
		nCount++;

		pBufferCurr  += dwSliceSize;
		dwStartCurr  += dwSliceSize;
		dwRemainSize -= dwSliceSize;

		dwSliceSize = m_dwPacketSize - sizeof(UDPFrameHeader);
	}

	return nCount;
}

UINT RUIUDPFrameList::RemoveUDPFrame(BYTE nFrameType)
{
	// [neuromos] 이름 때문에 RUIUDPFrameList::RemoveUDPFrame()를 만듬
	return RemoveFrameBuffer(nFrameType);
}

RUIFrameBuffer* RUIUDPFrameList::PopHeadCompleted(BYTE nFrameType)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	RUIFrameBuffer*	pHead = _GetFirst(nFrameType);

	if (pHead != NULL)
	{
		if (pHead->IsCompleted())
		{
			Unlink(pHead);

			return pHead;
		}
	}

	return NULL;
}

RUIFrameBuffer* RUIUDPFrameList::PopLimitHeadCompleted(BYTE nFrameType, DWORD dwFrameRecvTickLimit)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	RUIFrameBuffer*	pHead = _GetFirst(nFrameType);

	if (pHead != NULL)
	{
		if (pHead->IsCompleted())
		{
			if (pHead->GetParam() <= dwFrameRecvTickLimit)
			{
				Unlink(pHead);

				return pHead;
			}
			else
				return NULL; // Audio가 빨라 Play를 대기해야 하는 경우
		}
	}

	return NULL;
}

DWORD RUIUDPFrameList::GetUDPPacket(RUIFrameBuffer* pRUIFrameBuffer, USHORT nSliceIndex, BYTE* pBuffer, DWORD dwBufferSize)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	if (pRUIFrameBuffer == NULL || pBuffer == NULL || dwBufferSize != m_dwPacketSize)
	{
		ASSERT(FALSE);
		return 0;
	}

	DWORD	dwStart;
	BYTE*	pBufferBody;
	DWORD	dwBodySize;

	if (nSliceIndex == 0)
	{
		UDPFrameHeaderFirst		headerFirst;

		// [neuromos] [nFrameSubType] UDP Packet으로 보내기 전 FrameType(0xF0)과 FrameSubType(0x0F)을 합쳐 FrameType(0xFF)으로 한다.
		headerFirst.nFrameType  = pRUIFrameBuffer->GetFrameType() | pRUIFrameBuffer->GetFrameSubType();
		headerFirst.dwFrameKey  = pRUIFrameBuffer->GetFrameKey ();
		headerFirst.nSliceIndex = 0;
		headerFirst.dwFrameSize = pRUIFrameBuffer->GetFrameSize();
		headerFirst.dwFrameTick = GetTickCount(); // [neuromos] [checkpoint]

		CopyMemory((void*) pBuffer, (const void*) &headerFirst, sizeof(UDPFrameHeaderFirst));

		dwStart     = 0;
		pBufferBody = pBuffer        + sizeof(UDPFrameHeaderFirst);
		dwBodySize  = m_dwPacketSize - sizeof(UDPFrameHeaderFirst);
	}
	else
	{
		UDPFrameHeader			header;

		// [neuromos] [nFrameSubType] UDP Packet으로 보내기 전 FrameType(0xF0)과 FrameSubType(0x0F)을 합쳐 FrameType(0xFF)으로 한다.
		header.nFrameType  = pRUIFrameBuffer->GetFrameType() | pRUIFrameBuffer->GetFrameSubType();
		header.dwFrameKey  = pRUIFrameBuffer->GetFrameKey ();
		header.nSliceIndex = nSliceIndex;

		CopyMemory((void*) pBuffer, (const void*) &header, sizeof(UDPFrameHeader));

		dwStart     = nSliceIndex * (m_dwPacketSize - sizeof(UDPFrameHeader)) - (sizeof(UDPFrameHeaderFirst) - sizeof(UDPFrameHeader));
		pBufferBody = pBuffer        + sizeof(UDPFrameHeader);
		dwBodySize  = m_dwPacketSize - sizeof(UDPFrameHeader);
	}

	return pRUIFrameBuffer->GetFrameBuffer(dwStart, pBufferBody, dwBodySize);
}

RUIFrameBuffer* RUIUDPFrameList::OnReceiveUDPFrame(BYTE* pRecv, DWORD dwRecvSize)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	if (pRecv == NULL || dwRecvSize != m_dwPacketSize)
	{
		ASSERT(FALSE);
		return NULL;
	}

	UDPFrameHeader*	pHeader    = (UDPFrameHeader*) pRecv;
	BYTE			nFrameType = pHeader->nFrameType;
	BYTE			nFrameSubType;
	DWORD			dwFrameKey = pHeader->dwFrameKey;

	// [neuromos] [nFrameSubType] 수신한 FrameType(0xFF)을 FrameType(0xF0)과 FrameSubType(0x0F)으로 분리한다.
	nFrameSubType = nFrameType & 0x0F;
	nFrameType    = nFrameType & 0xF0;

	if (! m_bEnableFrameType[nFrameType])
		return NULL;

	RUIFrameBuffer*	pRUIFrameBuffer = _FindFrameBuffer(nFrameType, dwFrameKey);

	BOOL			bFirst       = FALSE;
	DWORD			dwFrameSize  = 0;
	BYTE*			pBuffer      = NULL;
	DWORD			dwBufferSize = 0;
	DWORD			dwStart      = 0;
	DWORD			dwFrameTick  = 0;

	if (pHeader->nSliceIndex == 0)
	{
#ifdef _DEBUG
		if (pRUIFrameBuffer != NULL) // nSliceIndex가 0 (처음) 인 FrameBuffer가 이미 있다.
		{
			ASSERT(FALSE);
			return NULL;
		}
#endif

		UDPFrameHeaderFirst*	pHeaderFirst = (UDPFrameHeaderFirst*) pRecv;

		bFirst       = TRUE;
		dwFrameSize  = pHeaderFirst->dwFrameSize;
		pBuffer      = pRecv      + sizeof(UDPFrameHeaderFirst);
		dwBufferSize = dwRecvSize - sizeof(UDPFrameHeaderFirst);
		dwStart      = 0;
		dwFrameTick  = pHeaderFirst->dwFrameTick; // [neuromos] [checkpoint]
	}
	else
	{
#ifdef _DEBUG
		if (pRUIFrameBuffer == NULL) // nSliceIndex가 0 (처음) 이 아닌데 FrameBuffer가 이미 없다.
		{
			// [neuromos] 중간 패킷이 들어올 수 있어 ASSERT(FALSE)는 코멘트 처리한다.
//			ASSERT(FALSE);
			return NULL;
		}
#endif

		dwFrameSize  = pRUIFrameBuffer->GetFrameSize();
		pBuffer      = pRecv      + sizeof(UDPFrameHeader);
		dwBufferSize = dwRecvSize - sizeof(UDPFrameHeader);
		dwStart      = pHeader->nSliceIndex * (m_dwPacketSize - sizeof(UDPFrameHeader)) - (sizeof(UDPFrameHeaderFirst) - sizeof(UDPFrameHeader));
	}

	if (dwFrameSize < (dwStart + dwBufferSize)) // 지금 추가하려고 하는 FrameBuffer가 FrameSize를 넘는 것이라면? 마지막 Slice면 넘을 수 있음.
	{
#ifdef _DEBUG
		if (dwFrameSize <= dwStart) // dwFrameSize를 벗어난 Slice
		{
			ASSERT(FALSE);
			return NULL;
		}
#endif

		dwBufferSize = dwFrameSize - dwStart; // 정확한 크기로 수정
	}

	if (m_dwFrameKeyLast[nFrameType] > dwFrameKey)
		return NULL;

	pRUIFrameBuffer = _AddFrameBuffer(nFrameType, nFrameSubType, dwFrameKey, dwFrameSize, dwFrameTick, pBuffer, dwBufferSize, dwStart);

	// [neuromos] Command Frame이라면 더 이상 Slice는 없다. 아래 로직도 처리 안한다. (시간 계산. Old Frame 정리)
	if (nFrameType == RUI_FRAME_TYPE_COMMAND)
		return pRUIFrameBuffer;

	m_n64FrameTypeSum[nFrameType] += dwBufferSize;

	// HeaderFirst에는 FrameTick이 실려온다. 이 Tick을 FrameBuffer Param에 저장한다.
	// [neuromos] [checkpoint]
	if (bFirst)
	{
		pRUIFrameBuffer->SetParam(dwFrameTick);

		if (m_pieParam->m_dwLastFrameTick[nFrameType] < dwFrameTick)
			m_pieParam->m_dwLastFrameTick[nFrameType] = dwFrameTick;
	}

	// 추가한 Frame의 dwFrameKey가 m_dwLastFrameKey[nFrameType]보다 크다면
	if (m_dwLastFrameKey == 0 || dwFrameKey > m_dwLastFrameKey[nFrameType])
		m_dwLastFrameKey[nFrameType] = dwFrameKey;

	//---------------------------------------------------------------------------------------------
	// [neuromos] 이 아래는 I-Frame이 완성될 경우 이전 Frame을 모두 삭제하는 코드임.
	//---------------------------------------------------------------------------------------------
	if (pRUIFrameBuffer->IsCompleted())
	{
		::SetEvent(m_pieParam->m_hEventRecv);

		if (nFrameType == RUI_FRAME_TYPE_VIDEO)
		{
			if (nFrameSubType == RUI_FRAMESUBTYPE_I)
			{
				DWORD	dwFrameKey = pRUIFrameBuffer->GetFrameKey();

				m_dwFrameKeyLast[nFrameType] = dwFrameKey;

				RUIFrameBuffer*	pRUIFrameBufferCheck = _GetFirst(nFrameType);

				while (pRUIFrameBufferCheck != NULL)
				{
					RUIFrameBuffer*	pRUIFrameBufferCheckNext = _GetNext(nFrameType, pRUIFrameBufferCheck);

					if (pRUIFrameBufferCheck->GetFrameKey() < dwFrameKey &&
						pRUIFrameBufferCheck->GetParam()    < m_pieParam->m_dwFrameRecvTickVideoLimit)
					{
#ifdef _USE_CHECK_DROP_FRAME
//						if (m_pMsgWnd != NULL)
//							m_pMsgWnd->SendMessage(WM_DROP_FRAME, (WPARAM) pRUIFrameBuffer, (LPARAM) pRUIFrameBufferCheck);
#endif

						Remove(pRUIFrameBufferCheck);
						m_dwFrameDropCount++;
					}

					pRUIFrameBufferCheck = pRUIFrameBufferCheckNext;
				}
			}

			DWORD	dwCurrTick           = GetTickCount();
			DWORD	dwFrameTickCompleted = pRUIFrameBuffer->GetParam();
			DWORD	dwFrameYourTick      =
					(
						(m_pieParam->m_nRelMyTickSign > 0) ?
						(dwFrameTickCompleted + m_pieParam->m_dwRelMyTick) :
						(dwFrameTickCompleted - m_pieParam->m_dwRelMyTick)
					);

			if (dwCurrTick > dwFrameYourTick &&
				dwCurrTick - dwFrameYourTick > 200)
			{
				// 둘 중 하나만 보낸다.
				if (m_pUDPSock != NULL) m_pUDPSock->SendCommandPacketCongestion();
				if (m_pTCPSock != NULL) m_pTCPSock->SendCommandPacketCongestion();
			}
		}
	}

#if 0
	// [neuromos] 네트워크 Band Width 때문에 Packet이 적체되고 있는지 체크하고
	// [neuromos] 적체되고 있다면 Bitrate를 낮추도록 서버에 메세지를 보낸다.
	if (nFrameType == RUI_FRAME_TYPE_VIDEO)
	{
		UINT	nVideoFrameBufferCount = GetFrameBufferCount(RUI_FRAME_TYPE_VIDEO);

		if (nVideoFrameBufferCount > m_pieParam->m_dwVideoOverFrameNumber)
		{
			m_nCongestionCount++;

			if (m_nCongestionCount > 1)
			{
				if (m_pUDPSock != NULL)
					m_pUDPSock->SendCommandPacketCongestion();

				// [neuromos] 완성 안된 것들은 안성된 것 이전까지 모두 삭제한다.
				RUIFrameBuffer*	pRUIFrameBufferCheck = _GetFirst(nFrameType);
				while (pRUIFrameBufferCheck != NULL)
				{
					if (pRUIFrameBufferCheck == pRUIFrameBuffer)
						break;

					RUIFrameBuffer*	pRUIFrameBufferCheckNext = _GetNext(nFrameType, pRUIFrameBufferCheck);
					Remove(pRUIFrameBufferCheck);
					pRUIFrameBufferCheck = pRUIFrameBufferCheckNext;
				}

				m_nCongestionCount = 0;
			}
		}

#ifdef _RUICLIENT
		CRUIClientView*	pView = (CRUIClientView*) m_pMsgWnd;
		if (pView != NULL)
			pView->UpdateRecvBufferListCount();
#endif
	}
#endif

	return pRUIFrameBuffer;
}
