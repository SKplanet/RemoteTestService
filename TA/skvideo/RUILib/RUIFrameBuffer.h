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

#ifndef _RUIFRAMEBUFFER_H
#define _RUIFRAMEBUFFER_H

#include "RUIBuffer.h"
#include "RUIFragmentBufferList.h"

class RUIFrameBuffer : public RUIBuffer
{
public:
	RUIFrameBuffer();
	virtual ~RUIFrameBuffer();

private:
	BYTE					m_nFrameType;
	BYTE					m_nFrameSubType;
	DWORD					m_dwFrameKey;
	RUIFragmentBufferList	m_fragmentBufferList;

public:
	virtual BYTE*			GetBuffer();
	virtual DWORD			GetBufferSize();
	virtual BYTE*			NewBuffer(DWORD dwSize, BOOL bRelease);
	virtual BYTE*			NewBuffer(BYTE* pCopyFrom, DWORD dwSize, BOOL bRelease);
	virtual BOOL			DeleteBuffer();
	virtual DWORD			CopyFrom(BYTE* pCopyFrom, DWORD dwSize);
	virtual DWORD			CopyTo(BYTE* pCopyTo, DWORD dwSize);
	virtual DWORD			CopyTo(BYTE* pCopyTo, DWORD dwOffset, DWORD dwSize);
	virtual DWORD			RemoveBufferHead(DWORD dwSize);
//	virtual DWORD			GetParam();
//	virtual void			SetParam(DWORD dwParam);

public:
	BYTE					GetFrameType()                      { return m_nFrameType;             }
	void					SetFrameType(BYTE nFrameType)       { m_nFrameType = nFrameType;       }
	BYTE					GetFrameSubType()                   { return m_nFrameSubType;          }
	void					SetFrameSubType(BYTE nFrameSubType) { m_nFrameSubType = nFrameSubType; }
	DWORD					GetFrameKey()                       { return m_dwFrameKey;             }
	void					SetFrameKey(DWORD dwFrameKey)       { m_dwFrameKey = dwFrameKey;       }

	DWORD					GetFrameSize();

	RUIFragmentBuffer*		AddFrameBuffer(BYTE* pCopyFrom, DWORD dwSize, DWORD dwStart);
	DWORD					GetFrameBuffer(DWORD dwStart, BYTE* pCopyTo, DWORD dwSize);

	BOOL					IsCompleted();
};

#endif
