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
#include "RUIUseBufferList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUIUseBufferList::RUIUseBufferList()
{
#ifdef _DEBUG // m_bAddLock�� �̿��Ͽ� �߰��� �߰��Ǵ� ���� üũ
	m_bAddLock = TRUE;
#endif
}

RUIUseBufferList::~RUIUseBufferList()
{
}

int RUIUseBufferList::_GetUseBufferCount(BOOL bUse)
{
	int				nCount        = 0;
	RUIUseBuffer*	pRUIUseBuffer = (RUIUseBuffer*) GetFirst();

	while (pRUIUseBuffer != NULL)
	{
		if (pRUIUseBuffer->GetUse() == bUse)
			nCount++;

		pRUIUseBuffer = (RUIUseBuffer*) GetNext(pRUIUseBuffer);
	}

	return nCount;
}

DWORD RUIUseBufferList::_GetUseBufferSize(BOOL bUse)
{
	DWORD			dwSize        = 0;
	RUIUseBuffer*	pRUIUseBuffer = (RUIUseBuffer*) GetFirst();

	while (pRUIUseBuffer != NULL)
	{
		if (pRUIUseBuffer->GetUse() == bUse)
			dwSize += pRUIUseBuffer->GetBufferSize();

		pRUIUseBuffer = (RUIUseBuffer*) GetNext(pRUIUseBuffer);
	}

	return dwSize;
}

RUIUseBuffer* RUIUseBufferList::_FindFirstUseBuffer(RUIUseBuffer* pRUIUseBufferAfter, BOOL bUse)
{
	RUIUseBuffer*	pRUIUseBuffer;
	
	if (pRUIUseBufferAfter != NULL) pRUIUseBuffer = (RUIUseBuffer*) GetNext(pRUIUseBufferAfter);
	else                            pRUIUseBuffer = (RUIUseBuffer*) GetFirst();

	while (pRUIUseBuffer != NULL)
	{
		if (pRUIUseBuffer->GetUse() == bUse)
			return pRUIUseBuffer;

		pRUIUseBuffer = (RUIUseBuffer*) GetNext(pRUIUseBuffer);
	}

	return NULL;
}

RUIUseBuffer* RUIUseBufferList::_FindLastUseBuffer(RUIUseBuffer* pRUIUseBufferBefore, BOOL bUse)
{
	RUIUseBuffer*	pRUIUseBuffer;
	
	if (pRUIUseBufferBefore != NULL) pRUIUseBuffer = (RUIUseBuffer*) GetPrev(pRUIUseBufferBefore);
	else                             pRUIUseBuffer = (RUIUseBuffer*) GetLast();

	while (pRUIUseBuffer != NULL)
	{
		if (pRUIUseBuffer->GetUse() == bUse)
			return pRUIUseBuffer;

		pRUIUseBuffer = (RUIUseBuffer*) GetPrev(pRUIUseBuffer);
	}

	return NULL;
}

void RUIUseBufferList::_SetUseBufferAll(BOOL bUse)
{
	RUIUseBuffer*	pRUIUseBuffer = (RUIUseBuffer*) GetFirst();

	while (pRUIUseBuffer != NULL)
	{
		pRUIUseBuffer->SetUse(bUse);

		pRUIUseBuffer = (RUIUseBuffer*) GetNext(pRUIUseBuffer);
	}
}

void RUIUseBufferList::_UnlinkAndInsertTail(RUIUseBuffer* pRUIUseBuffer)
{
	if (pRUIUseBuffer == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	Unlink(pRUIUseBuffer);
	InsertAtTail(pRUIUseBuffer);
}

DWORD RUIUseBufferList::_Copy(BYTE* pCopyTo, DWORD dwSizeCopy, BOOL bMove, DWORD dwFlag) // RUIBufferList::_Copy()�� �ڵ尡 ������
{
	if ((pCopyTo == NULL && ! bMove) || // �����ε� pCopyTo�� ������ �ȵǾ��ٸ�
		dwSizeCopy == 0)
	{
		ASSERT(FALSE);
		return 0;
	}

	DWORD	dwSizeBufferList = 0;

	if (dwFlag & RUIBUFFER_MOVE_FLAG_MULTI_BUFFER)
	{
		dwSizeBufferList = _GetUseBufferSize(TRUE);
	}
	else
	{
		RUIUseBuffer*	pRUIBufferFirst = _FindFirstUseBuffer(NULL, TRUE);

		if (pRUIBufferFirst != NULL)
			dwSizeBufferList = pRUIBufferFirst->GetBufferSize();
	}

	if (dwSizeBufferList == 0) // ������ ���۰� ����.
		return 0;

	if (dwSizeCopy > dwSizeBufferList)
	{
		if (dwFlag & RUIBUFFER_MOVE_FLAG_EXACT_SIZE)
			return 0;
		else
			dwSizeCopy = dwSizeBufferList;
	}

	BYTE*	pCopyToCurr  = pCopyTo;
	DWORD	dwSizeResult = 0;
	DWORD	dwSizeRemain = dwSizeCopy;

	RUIUseBuffer*	pRUIBuffer = _FindFirstUseBuffer(NULL, TRUE);

	while (dwSizeRemain > 0)
	{
		DWORD	dwSizeCopied = 0;

#ifdef _DEBUG
		if (pRUIBuffer == NULL) // [neuromos] ������ Size�� üũ�߱� ������ �̰��� ���� �� ����.
		{
			ASSERT(FALSE);
			return 0;
		}
#endif

		DWORD	dwSizeBuffer = pRUIBuffer->GetBufferSize();

		if (dwSizeBuffer > dwSizeRemain)
		{
			if (pCopyToCurr != NULL) // [neuromos] pRUIBuffer�� �� ū ���. �� ���� ����.
				pRUIBuffer->CopyTo(pCopyToCurr, dwSizeRemain);

			dwSizeCopied = dwSizeRemain;

			if (bMove) // ���簡 �ƴ϶�� �պκ��� �����Ѵ�.
				pRUIBuffer->RemoveBufferHead(dwSizeCopied);
		}
		else
		{
			if (pCopyToCurr != NULL) // [neuromos] pRUIBuffer�� ���ڸ��ų� �� �´� ���
				pRUIBuffer->CopyTo(pCopyToCurr, dwSizeBuffer);

			dwSizeCopied = dwSizeBuffer;

			RUIUseBuffer*	pRUIBufferRemove = pRUIBuffer; // �����ϱ� ���� ����
			pRUIBuffer = _FindFirstUseBuffer(pRUIBuffer, TRUE); // ���� ����. ��� ���۸� ����� ����� pRUIBuffer�� NULL�� �� �� �ִ�.

			if (bMove) // ���簡 �ƴ϶�� ������ ���� ����
			{
				pRUIBufferRemove->SetOffset(0);
				pRUIBufferRemove->SetUse(FALSE);
				_UnlinkAndInsertTail(pRUIBufferRemove);
			}
		}

		if (pCopyToCurr != NULL)
			pCopyToCurr  += dwSizeCopied;

		dwSizeResult += dwSizeCopied;
		dwSizeRemain -= dwSizeCopied;

		if (dwSizeRemain > 0 && pRUIBuffer == NULL) // ���� ������ ���� ���Ҵµ� ���۰� ���� ���
		{
			// [neuromos] ������ Size�� üũ�߱� ������ �̰��� ���� �� ����.
			ASSERT(FALSE);
			return 0;
		}
	}

	return dwSizeResult;
}

RUIBuffer* RUIUseBufferList::NewBuffer()
{
	return (RUIBuffer*) new RUIUseBuffer;
}

#ifdef _DEBUG // m_bAddLock�� �̿��Ͽ� �߰��� �߰��Ǵ� ���� üũ
RUIBuffer* RUIUseBufferList::AddBuffer(DWORD dwSize)
{
	if (m_bAddLock)
	{
		ASSERT(FALSE);
		return NULL;
	}

	return RUIBufferList::AddBuffer(dwSize);
}

RUIBuffer* RUIUseBufferList::AddBuffer(BYTE* pCopyFrom, DWORD dwSize)
{
	if (m_bAddLock)
	{
		ASSERT(FALSE);
		return NULL;
	}

	return RUIBufferList::AddBuffer(pCopyFrom, dwSize);
}

BOOL RUIUseBufferList::NewBufferList(DWORD dwSize, UINT nCount)
{
	BOOL	bResult;

	m_bAddLock = FALSE;
	{
		bResult = RUIBufferList::NewBufferList(dwSize, nCount);
	}
	m_bAddLock = TRUE;

	return bResult;
}
#endif

int RUIUseBufferList::GetUseBufferCount()
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _GetUseBufferCount(TRUE);
}

DWORD RUIUseBufferList::GetUseBufferSize()
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _GetUseBufferSize(TRUE);
}

RUIUseBuffer* RUIUseBufferList::FindFirstUnuseBuffer()
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _FindFirstUseBuffer(NULL, FALSE);
}

RUIUseBuffer* RUIUseBufferList::FindFirstUseBuffer()
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _FindFirstUseBuffer(NULL, TRUE);
}

RUIUseBuffer* RUIUseBufferList::FindLastUseBuffer()
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _FindLastUseBuffer(NULL, TRUE);
}

void RUIUseBufferList::SetUnuseBufferAll()
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	_SetUseBufferAll(FALSE);
}

BOOL RUIUseBufferList::SetUnuseBufferAndMoveTail(RUIUseBuffer* pRUIUseBuffer)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	if (pRUIUseBuffer != NULL && pRUIUseBuffer->GetUse())
	{
		pRUIUseBuffer->SetUse(FALSE);
		_UnlinkAndInsertTail(pRUIUseBuffer);

		return TRUE;
	}

	ASSERT(FALSE);

	return FALSE;
}
