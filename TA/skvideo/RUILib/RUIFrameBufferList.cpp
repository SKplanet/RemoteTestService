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
#include "RUIFrameBufferList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUIFrameBufferList::RUIFrameBufferList()
{
}

RUIFrameBufferList::~RUIFrameBufferList()
{
}

RUIFrameBuffer* RUIFrameBufferList::_AddFrameBuffer(BYTE nFrameType, BYTE nFrameSubType, DWORD dwFrameKey, DWORD dwFrameSize, DWORD dwParam)
{
	// dwFrameKey가 중복되어 추가될 수 있다. 주의해야 한다.

	RUIFrameBuffer*	pRUIFrameBuffer = (RUIFrameBuffer*) NewBuffer();

	// [neuromos] pRUIFrameBuffer->NewBuffer()의 리턴값은 항상 NULL이다.
//	if (pRUIFrameBuffer->NewBuffer(dwFrameSize, TRUE) == NULL)
//	{
//		delete pRUIFrameBuffer;
//		return NULL;
//	}

	pRUIFrameBuffer->NewBuffer(dwFrameSize, TRUE);

	pRUIFrameBuffer->SetFrameType   (nFrameType   );
	pRUIFrameBuffer->SetFrameSubType(nFrameSubType);
	pRUIFrameBuffer->SetFrameKey    (dwFrameKey   );
	pRUIFrameBuffer->SetParam       (dwParam      );

	BOOL	bInsert = FALSE;

	RUIFrameBuffer*	pRUIFrameBufferSeek = (RUIFrameBuffer*) GetFirst();

	while (pRUIFrameBufferSeek != NULL)
	{
		if (pRUIFrameBufferSeek->GetFrameType() == nFrameType && pRUIFrameBufferSeek->GetFrameKey() > dwFrameKey)
		{
			InsertBefore(pRUIFrameBufferSeek, pRUIFrameBuffer);
			bInsert = TRUE;
			break;
		}

		pRUIFrameBufferSeek = (RUIFrameBuffer*) GetNext(pRUIFrameBufferSeek);
	}

	if (! bInsert)
		InsertAtTail(pRUIFrameBuffer);

	return pRUIFrameBuffer;
}

RUIFrameBuffer* RUIFrameBufferList::_AddFrameBuffer(BYTE nFrameType, BYTE nFrameSubType, DWORD dwFrameKey, DWORD dwFrameSize, DWORD dwParam, BYTE* pBuffer, DWORD dwBufferSize, DWORD dwStart)
{
	RUIFrameBuffer*	pRUIFrameBuffer = _FindFrameBuffer(nFrameType, dwFrameKey);

	if (pRUIFrameBuffer == NULL)
		pRUIFrameBuffer = _AddFrameBuffer(nFrameType, nFrameSubType, dwFrameKey, dwFrameSize, dwParam);

#ifdef _DEBUG // 이미 있는 FrameBuffer라면 크기가 같아야 한다.
	else if (pRUIFrameBuffer->GetFrameSize() != dwFrameSize)
		ASSERT(FALSE);
#endif

	pRUIFrameBuffer->AddFrameBuffer(pBuffer, dwBufferSize, dwStart);

	if (dwParam != 0)
	{
#ifdef _DEBUG // 이미 있는 FrameBuffer의 Param과 같아야 한다.
		DWORD	dwParamOld = pRUIFrameBuffer->GetParam();
		if (dwParamOld != 0 && dwParamOld != dwParam)
			ASSERT(FALSE);
#endif

		pRUIFrameBuffer->SetParam(dwParam);
	}

	return pRUIFrameBuffer;
}

UINT RUIFrameBufferList::_RemoveFrameBuffer(BYTE nFrameType)
{
	UINT			nCount          = 0;
	RUIFrameBuffer*	pRUIFrameBuffer = (RUIFrameBuffer*) GetFirst();

	while (pRUIFrameBuffer != NULL)
	{
		RUIFrameBuffer*	pRUIFrameBufferNext = (RUIFrameBuffer*) GetNext(pRUIFrameBuffer);

		if (pRUIFrameBuffer->GetFrameType() == nFrameType)
		{
			Remove(pRUIFrameBuffer);
			nCount++;
		}

		pRUIFrameBuffer = pRUIFrameBufferNext;
	}

	return nCount;
}

int RUIFrameBufferList::_GetFrameBufferCount(BYTE nFrameType)
{
	int				nCount          = 0;
	RUIFrameBuffer*	pRUIFrameBuffer = (RUIFrameBuffer*) GetFirst();

	while (pRUIFrameBuffer != NULL)
	{
		if (pRUIFrameBuffer->GetFrameType() == nFrameType)
			nCount++;

		pRUIFrameBuffer = (RUIFrameBuffer*) GetNext(pRUIFrameBuffer);
	}

	return nCount;
}

RUIFrameBuffer* RUIFrameBufferList::_FindFrameBuffer(BYTE nFrameType, DWORD dwFrameKey)
{
	RUIFrameBuffer*	pRUIFrameBuffer = (RUIFrameBuffer*) GetFirst();

	while (pRUIFrameBuffer != NULL)
	{
		if (pRUIFrameBuffer->GetFrameType() == nFrameType &&
			pRUIFrameBuffer->GetFrameKey () == dwFrameKey)
			return pRUIFrameBuffer;

		pRUIFrameBuffer = (RUIFrameBuffer*) GetNext(pRUIFrameBuffer);
	}

	return NULL;
}

RUIBuffer* RUIFrameBufferList::NewBuffer()
{
	return (RUIBuffer*) new RUIFrameBuffer;
}

RUIBuffer* RUIFrameBufferList::AddBuffer(DWORD dwSize)
{
	// [neuromos] RUIFrameBufferList는 이 함수를 지원하지 않음.
	ASSERT(FALSE);
	return NULL;
}

RUIBuffer* RUIFrameBufferList::AddBuffer(BYTE* pCopyFrom, DWORD dwSize)
{
	// [neuromos] RUIFrameBufferList는 이 함수를 지원하지 않음.
	ASSERT(FALSE);
	return NULL;
}

RUIFrameBuffer* RUIFrameBufferList::AddFrameBuffer(BYTE nFrameType, BYTE nFrameSubType, DWORD dwFrameKey, DWORD dwFrameSize, DWORD dwParam) // 새로 추가
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	if (_FindFrameBuffer(nFrameType, dwFrameKey) != NULL) // 이미 FrameBuffer가 있다면 추가할 수 없음
	{
		ASSERT(FALSE);
		return NULL;
	}

	return _AddFrameBuffer(nFrameType, nFrameSubType, dwFrameKey, dwFrameSize, dwParam);
}

RUIFrameBuffer* RUIFrameBufferList::AddFrameBuffer(BYTE nFrameType, BYTE nFrameSubType, DWORD dwFrameKey, DWORD dwFrameSize, DWORD dwParam, BYTE* pBuffer, DWORD dwBufferSize, DWORD dwStart) // 이미 있는 FrameBuffer에 추가 가능
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _AddFrameBuffer(nFrameType, nFrameSubType, dwFrameKey, dwFrameSize, dwParam, pBuffer, dwBufferSize, dwStart);
}

UINT RUIFrameBufferList::RemoveFrameBuffer(BYTE nFrameType)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _RemoveFrameBuffer(nFrameType);
}

UINT RUIFrameBufferList::GetFrameBufferCount(BYTE nFrameType)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _GetFrameBufferCount(nFrameType);
}

RUIFrameBuffer* RUIFrameBufferList::FindFrameBuffer(BYTE nFrameType, DWORD dwFrameKey)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _FindFrameBuffer(nFrameType, dwFrameKey);
}
