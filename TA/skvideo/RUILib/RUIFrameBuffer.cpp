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
#include "RUIFrameBuffer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUIFrameBuffer::RUIFrameBuffer()
{
	m_nFrameType = 0;
	m_dwFrameKey = 0;
}

RUIFrameBuffer::~RUIFrameBuffer()
{
}

BYTE* RUIFrameBuffer::GetBuffer()
{
	// [neuromos] RUIFrameBuffer�� m_fragmentBufferList�� �����Ǿ� �� �Լ��� ����� �� ����.
	ASSERT(FALSE);
	return NULL;
}

DWORD RUIFrameBuffer::GetBufferSize()
{
	// [neuromos] !GetFrameSize()�� RUIFragmentBufferList::m_dwListSize�� �ǹ�������
	// [neuromos] GetBufferSize()�� RUIFragmentBufferList���� �����ϴ� ���� ������ ũ�� ���� �ǹ��Ѵ�.
	// [neuromos] ���� �� �Լ��� ASSERT(FALSE) ó���� �Ͽ�����
	// [neuromos] RUIUDPFrameList::OnReceiveUDPFrame()���� �ޱ���� FrameBuffer�� ũ��� ���� ���� ũ�� �񱳸� �ϱ� ���� �Ʒ��� ���� ������.
	// [neuromos] �̷��� ����ϴ� ���� ������?

	return m_fragmentBufferList.GetBufferSize();
}

BYTE* RUIFrameBuffer::NewBuffer(DWORD dwSize, BOOL bRelease)
{
	// [neuromos] ���ϰ��� �׻� NULL�̴�.
	// [neuromos] bRelease(���� ���� delete)�� ������� �ʴ´�.

	m_fragmentBufferList.DeleteBufferList();
	m_fragmentBufferList.SetListSize(dwSize);

	return NULL;
}

BYTE* RUIFrameBuffer::NewBuffer(BYTE* pCopyFrom, DWORD dwSize, BOOL bRelease)
{
	ASSERT(FALSE);

	return NewBuffer(dwSize, bRelease);
}

BOOL RUIFrameBuffer::DeleteBuffer()
{
	m_fragmentBufferList.DeleteBufferList();
	m_fragmentBufferList.SetListSize(0);

	return TRUE;
}

DWORD RUIFrameBuffer::CopyFrom(BYTE* pCopyFrom, DWORD dwSize)
{
	// [neuromos] RUIFrameBuffer�� m_fragmentBufferList�� �����Ǿ� �� �Լ��� ����� �� ����.
	ASSERT(FALSE);
	return 0;
}

DWORD RUIFrameBuffer::CopyTo(BYTE* pCopyTo, DWORD dwSize)
{
	return m_fragmentBufferList.CopyTo(pCopyTo, dwSize, RUIBUFFER_MOVE_FLAG_MULTI_BUFFER | RUIBUFFER_MOVE_FLAG_EXACT_SIZE);
}

DWORD RUIFrameBuffer::CopyTo(BYTE* pCopyTo, DWORD dwOffset, DWORD dwSize)
{
	ASSERT(FALSE);
	return 0;
}

DWORD RUIFrameBuffer::RemoveBufferHead(DWORD dwSize)
{
	// [neuromos] RUIFrameBuffer�� m_fragmentBufferList�� �����Ǿ� �� �Լ��� ����� �� ����.
	ASSERT(FALSE);
	return 0;
}

DWORD RUIFrameBuffer::GetFrameSize()
{
	return m_fragmentBufferList.GetListSize();
}

RUIFragmentBuffer* RUIFrameBuffer::AddFrameBuffer(BYTE* pCopyFrom, DWORD dwSize, DWORD dwStart)
{
	return m_fragmentBufferList.AddFragmentBuffer(pCopyFrom, dwSize, dwStart);
}

DWORD RUIFrameBuffer::GetFrameBuffer(DWORD dwStart, BYTE* pCopyTo, DWORD dwSize)
{
	return m_fragmentBufferList.GetFragmentBuffer(dwStart, pCopyTo, dwSize);
}

BOOL RUIFrameBuffer::IsCompleted()
{
	return m_fragmentBufferList.IsCompleted();
}
