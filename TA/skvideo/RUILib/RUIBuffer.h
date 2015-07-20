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

#ifndef _RUIBUFFER_H
#define _RUIBUFFER_H

#include "RUICsLocker.h"

class RUIBuffer
{
public:
	RUIBuffer();
	virtual ~RUIBuffer();

public:
	RUIBuffer*				pPrev;
	RUIBuffer*				pNext;

private:
	BYTE*					m_pBuffer;
	DWORD					m_dwBufferSize;
	DWORD					m_dwParam;
	RUICriticalSection		m_ruiCs; // Lock_XXX 함수에서 사용

public:
	virtual BYTE*			GetBuffer()     { return m_pBuffer;      }
	virtual DWORD			GetBufferSize() { return m_dwBufferSize; }
	virtual BYTE*			NewBuffer(DWORD dwSize, BOOL bRelease);
	virtual BYTE*			NewBuffer(BYTE* pCopyFrom, DWORD dwSize, BOOL bRelease);
	virtual BOOL			DeleteBuffer();
	virtual DWORD			CopyFrom(BYTE* pCopyFrom, DWORD dwSize);
	virtual DWORD			CopyTo(BYTE* pCopyTo, DWORD dwSize);
	virtual DWORD			CopyTo(BYTE* pCopyTo, DWORD dwOffset, DWORD dwSize);
	virtual DWORD			RemoveBufferHead(DWORD dwSize);
	virtual DWORD			GetParam()              { return m_dwParam;    }
	virtual void			SetParam(DWORD dwParam) { m_dwParam = dwParam; }

public:
	DWORD					Lock_CopyFrom(BYTE* pCopyFrom, DWORD dwSize);
	DWORD					Lock_CopyTo(BYTE* pCopyTo, DWORD dwOffset, DWORD dwSize);
};

#endif
