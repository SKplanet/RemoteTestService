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

#ifndef _RUIBUFFERLIST_H
#define _RUIBUFFERLIST_H

#include "TList.h"
#include "RUIBuffer.h"
#include "RUICsLocker.h"

#define RUIBUFFER_MOVE_FLAG_MULTI_BUFFER			0x00000001
#define RUIBUFFER_MOVE_FLAG_EXACT_SIZE				0x00000002

class RUIBufferList : public TList<RUIBuffer>
{
public:
	RUIBufferList();
	virtual ~RUIBufferList();

protected:
	RUICriticalSection		m_ruiCs;

protected:
	RUIBuffer*				_AddBuffer(DWORD dwSize);
	RUIBuffer*				_AddBuffer(BYTE* pCopyFrom, DWORD dwSize);
	RUIBuffer*				_AddBuffer(BYTE* pCopyFrom, DWORD dwSize, DWORD dwParam);
	void					_DeleteBufferList();
	RUIBuffer*				_PopHead();
	DWORD					_GetBufferSize();

	RUIBuffer*				_FindBufferByBuffer    (BYTE* pBuffer);
	RUIBuffer*				_FindBufferForLessParam(DWORD dwParam, int nPrevCount);
	void					_DeleteBufferListBefore(RUIBuffer* pRUIBuffer);

public:
	RUIBuffer*				FindBufferByBuffer    (BYTE* pBuffer);
	RUIBuffer*				FindBufferForLessParam(DWORD dwParam, int nPrevCount);
	void					DeleteBufferListBefore(RUIBuffer* pRUIBuffer);

protected:
	virtual DWORD			_Copy(BYTE* pCopyTo, DWORD dwSizeCopy, BOOL bMove, DWORD dwFlag);

public:
	virtual RUIBuffer*		NewBuffer();
	virtual RUIBuffer*		AddBuffer(DWORD dwSize);
	virtual RUIBuffer*		AddBuffer(BYTE* pCopyFrom, DWORD dwSize);
	virtual RUIBuffer*		AddBuffer(BYTE* pCopyFrom, DWORD dwSize, DWORD dwParam);
	virtual BOOL			NewBufferList(DWORD dwSize, UINT nCount);
	virtual void			DeleteBufferList();
	virtual RUIBuffer*		PopHead();
	virtual DWORD			GetBufferSize();
	virtual DWORD			CopyTo(BYTE* pCopyTo, DWORD dwSizeCopy, DWORD dwFlag);
	virtual DWORD			MoveTo(BYTE* pMoveTo, DWORD dwSizeMove, DWORD dwFlag);
};

#endif
