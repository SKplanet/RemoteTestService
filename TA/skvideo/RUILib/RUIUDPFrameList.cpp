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
		if (pRUIFrameBuffer != NULL) // dwStart�� 0���� ū������ �̸� ����� ���? �߰����� ����� ���� ����.
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
		if (pRUIFrameBuffer == NULL) // dwStart�� 0���� ū������ �̸� ����� ���? �߰����� ����� ���� ����.
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
	// [neuromos] �̸� ������ RUIUDPFrameList::RemoveUDPFrame()�� ����
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
				return NULL; // Audio�� ���� Play�� ����ؾ� �ϴ� ���
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

		// [neuromos] [nFrameSubType] UDP Packet���� ������ �� FrameType(0xF0)�� FrameSubType(0x0F)�� ���� FrameType(0xFF)���� �Ѵ�.
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

		// [neuromos] [nFrameSubType] UDP Packet���� ������ �� FrameType(0xF0)�� FrameSubType(0x0F)�� ���� FrameType(0xFF)���� �Ѵ�.
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

	// [neuromos] [nFrameSubType] ������ FrameType(0xFF)�� FrameType(0xF0)�� FrameSubType(0x0F)���� �и��Ѵ�.
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
		if (pRUIFrameBuffer != NULL) // nSliceIndex�� 0 (ó��) �� FrameBuffer�� �̹� �ִ�.
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
		if (pRUIFrameBuffer == NULL) // nSliceIndex�� 0 (ó��) �� �ƴѵ� FrameBuffer�� �̹� ����.
		{
			// [neuromos] �߰� ��Ŷ�� ���� �� �־� ASSERT(FALSE)�� �ڸ�Ʈ ó���Ѵ�.
//			ASSERT(FALSE);
			return NULL;
		}
#endif

		dwFrameSize  = pRUIFrameBuffer->GetFrameSize();
		pBuffer      = pRecv      + sizeof(UDPFrameHeader);
		dwBufferSize = dwRecvSize - sizeof(UDPFrameHeader);
		dwStart      = pHeader->nSliceIndex * (m_dwPacketSize - sizeof(UDPFrameHeader)) - (sizeof(UDPFrameHeaderFirst) - sizeof(UDPFrameHeader));
	}

	if (dwFrameSize < (dwStart + dwBufferSize)) // ���� �߰��Ϸ��� �ϴ� FrameBuffer�� FrameSize�� �Ѵ� ���̶��? ������ Slice�� ���� �� ����.
	{
#ifdef _DEBUG
		if (dwFrameSize <= dwStart) // dwFrameSize�� ��� Slice
		{
			ASSERT(FALSE);
			return NULL;
		}
#endif

		dwBufferSize = dwFrameSize - dwStart; // ��Ȯ�� ũ��� ����
	}

	if (m_dwFrameKeyLast[nFrameType] > dwFrameKey)
		return NULL;

	pRUIFrameBuffer = _AddFrameBuffer(nFrameType, nFrameSubType, dwFrameKey, dwFrameSize, dwFrameTick, pBuffer, dwBufferSize, dwStart);

	// [neuromos] Command Frame�̶�� �� �̻� Slice�� ����. �Ʒ� ������ ó�� ���Ѵ�. (�ð� ���. Old Frame ����)
	if (nFrameType == RUI_FRAME_TYPE_COMMAND)
		return pRUIFrameBuffer;

	m_n64FrameTypeSum[nFrameType] += dwBufferSize;

	// HeaderFirst���� FrameTick�� �Ƿ��´�. �� Tick�� FrameBuffer Param�� �����Ѵ�.
	// [neuromos] [checkpoint]
	if (bFirst)
	{
		pRUIFrameBuffer->SetParam(dwFrameTick);

		if (m_pieParam->m_dwLastFrameTick[nFrameType] < dwFrameTick)
			m_pieParam->m_dwLastFrameTick[nFrameType] = dwFrameTick;
	}

	// �߰��� Frame�� dwFrameKey�� m_dwLastFrameKey[nFrameType]���� ũ�ٸ�
	if (m_dwLastFrameKey == 0 || dwFrameKey > m_dwLastFrameKey[nFrameType])
		m_dwLastFrameKey[nFrameType] = dwFrameKey;

	//---------------------------------------------------------------------------------------------
	// [neuromos] �� �Ʒ��� I-Frame�� �ϼ��� ��� ���� Frame�� ��� �����ϴ� �ڵ���.
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
				// �� �� �ϳ��� ������.
				if (m_pUDPSock != NULL) m_pUDPSock->SendCommandPacketCongestion();
				if (m_pTCPSock != NULL) m_pTCPSock->SendCommandPacketCongestion();
			}
		}
	}

#if 0
	// [neuromos] ��Ʈ��ũ Band Width ������ Packet�� ��ü�ǰ� �ִ��� üũ�ϰ�
	// [neuromos] ��ü�ǰ� �ִٸ� Bitrate�� ���ߵ��� ������ �޼����� ������.
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

				// [neuromos] �ϼ� �ȵ� �͵��� �ȼ��� �� �������� ��� �����Ѵ�.
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
