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
#include "RUIOffsetBuffer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUIOffsetBuffer::RUIOffsetBuffer()
{
	m_dwOffset = 0;
}

RUIOffsetBuffer::~RUIOffsetBuffer()
{
}

BYTE* RUIOffsetBuffer::GetBuffer()
{
	return (RUIBuffer::GetBuffer() + m_dwOffset);
}

DWORD RUIOffsetBuffer::GetBufferSize()
{
	return (RUIBuffer::GetBufferSize() - m_dwOffset);
}

BYTE* RUIOffsetBuffer::NewBuffer(DWORD dwSize, BOOL bRelease)
{
	m_dwOffset = 0;

	return RUIBuffer::NewBuffer(dwSize, bRelease);
}

BYTE* RUIOffsetBuffer::NewBuffer(BYTE* pCopyFrom, DWORD dwSize, BOOL bRelease)
{
	m_dwOffset = 0;

	return RUIBuffer::NewBuffer(pCopyFrom, dwSize, bRelease);
}

BOOL RUIOffsetBuffer::DeleteBuffer()
{
	m_dwOffset = 0;

	return RUIBuffer::DeleteBuffer();
}

DWORD RUIOffsetBuffer::CopyFrom(BYTE* pCopyFrom, DWORD dwSize)
{
	DWORD	dwSizeBuffer = GetBufferSize();

	if (dwSize > dwSizeBuffer)
	{
		if (dwSize <= (dwSizeBuffer + m_dwOffset))
			ASSERT(FALSE); // m_dwOffset을 줄이면 복사해올 수 있다. 어떻게 처리하나?
	}

	return RUIBuffer::CopyFrom(pCopyFrom, dwSize);
}

DWORD RUIOffsetBuffer::RemoveBufferHead(DWORD dwSize)
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
			m_dwOffset += dwSize;
		}
		else
		{
			dwSize = dwSizeBuffer;
			// [neuromos] !삭제해야 하나? m_dwOffset만 증가하면 되나?
//			DeleteBuffer();
			m_dwOffset += dwSize;
		}
	}

	return dwSize;
}
