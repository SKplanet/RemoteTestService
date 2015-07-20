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
#include "RUIBuffer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUIBuffer::RUIBuffer()
{
	m_pBuffer      = NULL;
	m_dwBufferSize = 0;
	m_dwParam      = 0;
}

RUIBuffer::~RUIBuffer()
{
	DeleteBuffer();
}

BYTE* RUIBuffer::NewBuffer(DWORD dwSize, BOOL bRelease)
{
	if (bRelease)
		DeleteBuffer();

	if (dwSize > 0)
	{
		m_pBuffer      = new BYTE[dwSize];
		m_dwBufferSize = dwSize;
	}

	return m_pBuffer;
}

BYTE* RUIBuffer::NewBuffer(BYTE* pCopyFrom, DWORD dwSize, BOOL bRelease)
{
	if (NewBuffer(dwSize, bRelease) != NULL)
	{
		if (m_pBuffer != NULL && pCopyFrom != NULL && dwSize > 0)
		{
			CopyMemory(
				(void*) m_pBuffer,
				(const void*) pCopyFrom,
				dwSize);
		}
		else
			ASSERT(FALSE);
	}

	return m_pBuffer;
}

BOOL RUIBuffer::DeleteBuffer()
{
	if (m_pBuffer != NULL)
	{
		delete [] m_pBuffer;

		m_pBuffer      = NULL;
		m_dwBufferSize = 0;

		return TRUE;
	}

	return FALSE;
}

DWORD RUIBuffer::CopyFrom(BYTE* pCopyFrom, DWORD dwSize)
{
	if (pCopyFrom != NULL && dwSize > 0)
	{
		BYTE*	pBuffer      = GetBuffer();
		DWORD	dwSizeBuffer = GetBufferSize();

		if (pBuffer != NULL && dwSizeBuffer > 0)
		{
			if (dwSize > dwSizeBuffer)
				dwSize = dwSizeBuffer;

			CopyMemory(
				(void*) pBuffer,
				(const void*) pCopyFrom,
				dwSize);

			return dwSize;
		}
	}

	return 0;
}

DWORD RUIBuffer::CopyTo(BYTE* pCopyTo, DWORD dwSize)
{
	if (pCopyTo != NULL && dwSize > 0)
	{
		BYTE*	pBuffer      = GetBuffer();
		DWORD	dwSizeBuffer = GetBufferSize();

		if (pBuffer != NULL && dwSizeBuffer > 0)
		{
			if (dwSize > dwSizeBuffer)
				dwSize = dwSizeBuffer;

			CopyMemory(
				(void*) pCopyTo,
				(const void*) pBuffer,
				dwSize);

			return dwSize;
		}
	}

	return 0;
}

DWORD RUIBuffer::CopyTo(BYTE* pCopyTo, DWORD dwOffset, DWORD dwSize)
{
	if (pCopyTo != NULL && dwSize > 0)
	{
		BYTE*	pBuffer      = GetBuffer();
		DWORD	dwSizeBuffer = GetBufferSize();

		if (pBuffer != NULL && dwSizeBuffer > 0)
		{
			if ((dwOffset + dwSize) > dwSizeBuffer)
			{
				ASSERT(FALSE); // 현재는 Encoder에서만 사용하기 때문에 크기가 안맞는 경우 오류를 발생.
				dwSize = dwSizeBuffer - dwOffset;
			}

			CopyMemory(
				(void*) pCopyTo,
				(const void*) (pBuffer + dwOffset),
				dwSize);

			return dwSize;
		}
	}

	return 0;
}

DWORD RUIBuffer::RemoveBufferHead(DWORD dwSize)
{
	if (dwSize > 0)
	{
		BYTE*	pBuffer      = GetBuffer();
		DWORD	dwSizeBuffer = GetBufferSize();

		if (pBuffer == NULL || dwSizeBuffer == 0)
		{
			dwSize = 0;
		}
		else if (dwSize < dwSizeBuffer)
		{
			NewBuffer(pBuffer + dwSize, dwSizeBuffer - dwSize, FALSE);
			delete [] pBuffer;
		}
		else
		{
			dwSize = dwSizeBuffer;
			DeleteBuffer();
		}
	}

	return dwSize;
}

DWORD RUIBuffer::Lock_CopyFrom(BYTE* pCopyFrom, DWORD dwSize)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return CopyFrom(pCopyFrom, dwSize);
}

DWORD RUIBuffer::Lock_CopyTo(BYTE* pCopyTo, DWORD dwOffset, DWORD dwSize)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return CopyTo(pCopyTo, dwOffset, dwSize);
}
