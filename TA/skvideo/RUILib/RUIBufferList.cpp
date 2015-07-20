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
#include "RUIBufferList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUIBufferList::RUIBufferList()
{
}

RUIBufferList::~RUIBufferList()
{
	DeleteBufferList(); // �Լ� ������ RUICsLocker ruiCsLocker(m_ruiCs)
}

RUIBuffer* RUIBufferList::_AddBuffer(DWORD dwSize)
{
	if (dwSize == 0)
	{
		ASSERT(FALSE);
		return NULL;
	}

	RUIBuffer*	pRUIBuffer = NewBuffer();

	if (pRUIBuffer->NewBuffer(dwSize, TRUE) == NULL)
	{
		delete pRUIBuffer;
		return NULL;
	}

	InsertAtTail(pRUIBuffer);

	return pRUIBuffer;
}

RUIBuffer* RUIBufferList::_AddBuffer(BYTE* pCopyFrom, DWORD dwSize)
{
	if (pCopyFrom == NULL || dwSize == 0)
	{
		ASSERT(FALSE);
		return NULL;
	}

	RUIBuffer*	pRUIBuffer = _AddBuffer(dwSize);

	if (pRUIBuffer != NULL)
		pRUIBuffer->CopyFrom(pCopyFrom, dwSize);

	return pRUIBuffer;
}

RUIBuffer* RUIBufferList::_AddBuffer(BYTE* pCopyFrom, DWORD dwSize, DWORD dwParam)
{
	RUIBuffer*	pRUIBuffer = _AddBuffer(pCopyFrom, dwSize);

	if (pRUIBuffer != NULL)
		pRUIBuffer->SetParam(dwParam);

	return pRUIBuffer;
}

void RUIBufferList::_DeleteBufferList()
{
	FreeAll();
}

RUIBuffer* RUIBufferList::_PopHead()
{
	RUIBuffer*	pHead = GetFirst();

	if (pHead != NULL)
		Unlink(pHead);

	return pHead;
}

DWORD RUIBufferList::_GetBufferSize()
{
	DWORD		dwSize     = 0;
	RUIBuffer*	pRUIBuffer = GetFirst();

	while (pRUIBuffer != NULL)
	{
		dwSize += pRUIBuffer->GetBufferSize();

		pRUIBuffer = GetNext(pRUIBuffer);
	}

	return dwSize;
}

RUIBuffer* RUIBufferList::_FindBufferByBuffer(BYTE* pBuffer)
{
	RUIBuffer*	pRUIBuffer = GetFirst();

	while (pRUIBuffer != NULL)
	{
		if (pRUIBuffer->GetBuffer() == pBuffer)
			return pRUIBuffer;

		pRUIBuffer = GetNext(pRUIBuffer);
	}

	return NULL;
}

RUIBuffer* RUIBufferList::_FindBufferForLessParam(DWORD dwParam, int nPrevCount)
{
	RUIBuffer*	pRUIBuffer     = GetLast();
	RUIBuffer*	pRUIBufferFind = NULL;

	while (pRUIBuffer != NULL)
	{
		if (pRUIBuffer->GetParam() <= dwParam)
		{
			if (nPrevCount <= 0)
				return pRUIBuffer;

			nPrevCount--;

			pRUIBufferFind = pRUIBuffer;
		}

		pRUIBuffer = GetPrev(pRUIBuffer);
	}

	return pRUIBufferFind;
}

void RUIBufferList::_DeleteBufferListBefore(RUIBuffer* pRUIBuffer)
{
	RUIBuffer*	pRUIBufferSeek;

	while ((pRUIBufferSeek = GetFirst()) != NULL)
	{
		if (pRUIBufferSeek == pRUIBuffer)
			return;

		Remove(pRUIBufferSeek);
	}
}

RUIBuffer* RUIBufferList::FindBufferByBuffer(BYTE* pBuffer)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _FindBufferByBuffer(pBuffer);
}

RUIBuffer* RUIBufferList::FindBufferForLessParam(DWORD dwParam, int nPrevCount)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _FindBufferForLessParam(dwParam, nPrevCount);
}

void RUIBufferList::DeleteBufferListBefore(RUIBuffer* pRUIBuffer)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	_DeleteBufferListBefore(pRUIBuffer);
}

DWORD RUIBufferList::_Copy(BYTE* pCopyTo, DWORD dwSizeCopy, BOOL bMove, DWORD dwFlag) // RUIUseBufferList::_Copy()�� �ڵ尡 ������
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
		dwSizeBufferList = _GetBufferSize();
	}
	else
	{
		RUIBuffer*	pRUIBufferFirst = GetFirst();

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

	RUIBuffer*	pRUIBuffer = GetFirst();

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

			RUIBuffer*	pRUIBufferRemove = pRUIBuffer; // �����ϱ� ���� ����
			pRUIBuffer = GetNext(pRUIBuffer); // ���� ����. ��� ���۸� ����� ����� pRUIBuffer�� NULL�� �� �� �ִ�.

			if (bMove) // ���簡 �ƴ϶�� ������ ���� ����
			{

				Remove(pRUIBufferRemove);

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

RUIBuffer* RUIBufferList::NewBuffer()
{
	return new RUIBuffer;
}

RUIBuffer* RUIBufferList::AddBuffer(DWORD dwSize)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _AddBuffer(dwSize);
}

RUIBuffer* RUIBufferList::AddBuffer(BYTE* pCopyFrom, DWORD dwSize)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _AddBuffer(pCopyFrom, dwSize);
}

RUIBuffer* RUIBufferList::AddBuffer(BYTE* pCopyFrom, DWORD dwSize, DWORD dwParam)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _AddBuffer(pCopyFrom, dwSize, dwParam);
}

BOOL RUIBufferList::NewBufferList(DWORD dwSize, UINT nCount)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	_DeleteBufferList();

	for (UINT i = 0; i < nCount; i++)
	{
		if (_AddBuffer(dwSize) == NULL)
		{
			_DeleteBufferList();
			return FALSE;
		}
	}

	return TRUE;
}

void RUIBufferList::DeleteBufferList()
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	_DeleteBufferList();
}

RUIBuffer* RUIBufferList::PopHead()
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _PopHead();
}

DWORD RUIBufferList::GetBufferSize()
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _GetBufferSize();
}

DWORD RUIBufferList::CopyTo(BYTE* pCopyTo, DWORD dwSizeCopy, DWORD dwFlag)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _Copy(pCopyTo, dwSizeCopy, FALSE, dwFlag);
}

DWORD RUIBufferList::MoveTo(BYTE* pMoveTo, DWORD dwSizeMove, DWORD dwFlag)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _Copy(pMoveTo, dwSizeMove, TRUE, dwFlag);
}
