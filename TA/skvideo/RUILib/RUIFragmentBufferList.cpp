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
#include "RUIFragmentBufferList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUIFragmentBufferList::RUIFragmentBufferList()
{
	m_dwListSize = 0;
}

RUIFragmentBufferList::~RUIFragmentBufferList()
{
}

RUIFragmentBuffer* RUIFragmentBufferList::_AddFragmentBuffer(BYTE* pCopyFrom, DWORD dwSize, DWORD dwStart)
{
	if (pCopyFrom == NULL || dwSize == 0)
	{
		ASSERT(FALSE);
		return NULL;
	}

	RUIFragmentBuffer*	pRUIFragmentBuffer = (RUIFragmentBuffer*) NewBuffer();

	if (pRUIFragmentBuffer->NewBuffer(dwSize, TRUE) == NULL)
	{
		delete pRUIFragmentBuffer;
		return NULL;
	}

	pRUIFragmentBuffer->CopyFrom(pCopyFrom, dwSize);
	pRUIFragmentBuffer->SetStart(dwStart);

	InsertAtTail(pRUIFragmentBuffer);

	return pRUIFragmentBuffer;
}

RUIFragmentBuffer* RUIFragmentBufferList::_FindFragmentBufferByStart(DWORD dwStart)
{
	RUIFragmentBuffer*	pRUIFragmentBuffer = (RUIFragmentBuffer*) GetFirst();

	while (pRUIFragmentBuffer != NULL)
	{
		if (pRUIFragmentBuffer->GetStart() == dwStart)
			return pRUIFragmentBuffer;

		pRUIFragmentBuffer = (RUIFragmentBuffer*) GetNext(pRUIFragmentBuffer);
	}

	return NULL;
}

DWORD RUIFragmentBufferList::_GetFragmentBuffer(DWORD dwStart, BYTE* pCopyTo, DWORD dwSize)
{
	if (pCopyTo == NULL || dwSize == 0)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	RUIFragmentBuffer*	pRUIFragmentBuffer = _FindFragmentBufferByStart(dwStart);

	if (pRUIFragmentBuffer != NULL)
	{
		DWORD	dwFragmentBufferSize = pRUIFragmentBuffer->GetBufferSize();

		if (dwFragmentBufferSize < dwSize)
			dwSize = dwFragmentBufferSize;

		return pRUIFragmentBuffer->CopyTo(pCopyTo, dwSize);
	}

	return FALSE;
}

BOOL RUIFragmentBufferList::_IsCompleted()
{
//	if (m_dwListSize == 0)
//		return TRUE;

	DWORD	dwStart = 0;
	DWORD	dwEnd   = m_dwListSize;

	while (dwStart < dwEnd)
	{
		RUIFragmentBuffer*	pRUIFragmentBuffer = _FindFragmentBufferByStart(dwStart);

		if (pRUIFragmentBuffer != NULL)
		{
			DWORD	dwBufferSize = pRUIFragmentBuffer->GetBufferSize();

			if (dwBufferSize == 0)
				ASSERT(FALSE);

			dwStart += dwBufferSize;
		}
		else
			return FALSE;
	}

	return TRUE;
}

DWORD RUIFragmentBufferList::_Copy(BYTE* pCopyTo, DWORD dwSizeCopy, BOOL bMove, DWORD dwFlag)
{
	if (pCopyTo == NULL || dwSizeCopy == 0 || bMove) // Move�� �������� �ʴ´�.
	{
		ASSERT(FALSE);
		return 0;
	}

	if (! (dwFlag & RUIBUFFER_MOVE_FLAG_MULTI_BUFFER) || // RUIFragmentBufferList�� �ݵ�� RUIBUFFER_MOVE_FLAG_MULTI_BUFFER�� ����ؾ� �Ѵ�.
		! (dwFlag & RUIBUFFER_MOVE_FLAG_EXACT_SIZE  ))   // RUIFragmentBufferList�� �ݵ�� RUIBUFFER_MOVE_FLAG_EXACT_SIZE  �� ����ؾ� �Ѵ�.
	{
		ASSERT(FALSE);
		return 0;
	}

	DWORD	dwStart      = 0;

	BYTE*	pCopyToCurr  = pCopyTo;
	DWORD	dwSizeResult = 0;
	DWORD	dwSizeRemain = dwSizeCopy;

	while (dwSizeRemain > 0)
	{
		RUIFragmentBuffer*	pRUIFragmentBuffer = _FindFragmentBufferByStart(dwStart);

		// [neuromos] �ٸ� Class�� while Loop ���� ���۰� ������� üũ�Ѵ�.
		// [neuromos] RUIFragmentBufferList�� Ư���� ���۸� ã�ٰ� ���� ��쿡 return 0; �Ѵ�.
		if (pRUIFragmentBuffer == NULL) // ���� �����ϱ⿡�� ���۰� �����ϴ�.
			return 0;

		DWORD	dwSizeCopied = 0;
		DWORD	dwSizeBuffer = pRUIFragmentBuffer->GetBufferSize();

		if (dwSizeBuffer > dwSizeRemain) dwSizeCopied = dwSizeRemain;
		else                             dwSizeCopied = dwSizeBuffer;

		pRUIFragmentBuffer->CopyTo(pCopyToCurr, dwSizeCopied);

		dwStart      += dwSizeCopied;
		pCopyToCurr  += dwSizeCopied;
		dwSizeResult += dwSizeCopied;
		dwSizeRemain -= dwSizeCopied;
	}

	return dwSizeResult;
}

RUIBuffer* RUIFragmentBufferList::NewBuffer()
{
	return (RUIBuffer*) new RUIFragmentBuffer;
}

RUIBuffer* RUIFragmentBufferList::AddBuffer(BYTE* pCopyFrom, DWORD dwSize)
{
	// [neuromos] RUIFragmentBufferList�� �� �Լ��� �������� ����.
	ASSERT(FALSE);
	return NULL;
}

RUIBuffer* RUIFragmentBufferList::PopHead()
{
	// [neuromos] RUIFragmentBufferList�� �� �Լ��� �������� ����.
	ASSERT(FALSE);
	return NULL;
}

RUIFragmentBuffer* RUIFragmentBufferList::AddFragmentBuffer(BYTE* pCopyFrom, DWORD dwSize, DWORD dwStart)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	// [neuromos] !dwStart�� ��ġ�� �Ϳ� ���ؼ��� ���� üũ���� �ʰ� �ִ�. ��� �ؾ� �ϳ�?

	if ((dwStart + dwSize) > m_dwListSize) // ������ m_dwListSize���� ũ�� �߰��� �� ����.
	{
		ASSERT(FALSE);
		return NULL;
	}

	return _AddFragmentBuffer(pCopyFrom, dwSize, dwStart);
}

DWORD RUIFragmentBufferList::GetFragmentBuffer(DWORD dwStart, BYTE* pCopyTo, DWORD dwSize)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _GetFragmentBuffer(dwStart, pCopyTo, dwSize);
}

BOOL RUIFragmentBufferList::IsCompleted()
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return _IsCompleted();
}
