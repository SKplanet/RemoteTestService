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

#ifndef _RUIFRAMEBUFFERLIST_H
#define _RUIFRAMEBUFFERLIST_H

#include "RUIBufferList.h"
#include "RUIFrameBuffer.h"

class RUIFrameBufferList : public RUIBufferList
{
public:
	RUIFrameBufferList();
	virtual ~RUIFrameBufferList();

protected:
	RUIFrameBuffer*			_AddFrameBuffer(BYTE nFrameType, BYTE nFrameSubType, DWORD dwFrameKey, DWORD dwFrameSize, DWORD dwParam);
	RUIFrameBuffer*			_AddFrameBuffer(BYTE nFrameType, BYTE nFrameSubType, DWORD dwFrameKey, DWORD dwFrameSize, DWORD dwParam, BYTE* pBuffer, DWORD dwBufferSize, DWORD dwStart);
	UINT					_RemoveFrameBuffer(BYTE nFrameType);
	int						_GetFrameBufferCount(BYTE nFrameType);
	RUIFrameBuffer*			_FindFrameBuffer(BYTE nFrameType, DWORD dwFrameKey);

public:
	virtual RUIBuffer*		NewBuffer();
	virtual RUIBuffer*		AddBuffer(DWORD dwSize);
	virtual RUIBuffer*		AddBuffer(BYTE* pCopyFrom, DWORD dwSize);
//	virtual BOOL			NewBufferList(DWORD dwSize, UINT nCount);
//	virtual void			DeleteBufferList();
//	virtual RUIBuffer*		PopHead();
//	virtual DWORD			GetBufferSize();
//	virtual DWORD			CopyTo(BYTE* pCopyTo, DWORD dwSizeCopy, DWORD dwFlag);
//	virtual DWORD			MoveTo(BYTE* pMoveTo, DWORD dwSizeMove, DWORD dwFlag);

public:
	RUIFrameBuffer*			AddFrameBuffer(BYTE nFrameType, BYTE nFrameSubType, DWORD dwFrameKey, DWORD dwFrameSize, DWORD dwParam);
	RUIFrameBuffer*			AddFrameBuffer(BYTE nFrameType, BYTE nFrameSubType, DWORD dwFrameKey, DWORD dwFrameSize, DWORD dwParam, BYTE* pBuffer, DWORD dwBufferSize, DWORD dwStart);
	UINT					RemoveFrameBuffer(BYTE nFrameType);
	UINT					GetFrameBufferCount(BYTE nFrameType);

	RUIFrameBuffer*			FindFrameBuffer(BYTE nFrameType, DWORD dwFrameKey);
};

#endif
