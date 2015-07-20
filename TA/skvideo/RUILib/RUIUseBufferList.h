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

#ifndef _RUIUSEBUFFERLIST_H
#define _RUIUSEBUFFERLIST_H

#include "RUIBufferList.h"
#include "RUIUseBuffer.h"

class RUIUseBufferList : public RUIBufferList
{
public:
	RUIUseBufferList();
	virtual ~RUIUseBufferList();

private:
#ifdef _DEBUG // m_bAddLock을 이용하여 중간에 추가되는 것을 체크
	BOOL					m_bAddLock;
#endif

protected:
	int						_GetUseBufferCount(BOOL bUse);
	DWORD					_GetUseBufferSize(BOOL bUse);
	RUIUseBuffer*			_FindFirstUseBuffer(RUIUseBuffer* pRUIUseBufferAfter, BOOL bUse);
	RUIUseBuffer*			_FindLastUseBuffer(RUIUseBuffer* pRUIUseBufferBefore, BOOL bUse);
	void					_SetUseBufferAll(BOOL bUse);
	void					_UnlinkAndInsertTail(RUIUseBuffer* pRUIUseBuffer);

protected:
	virtual DWORD			_Copy(BYTE* pCopyTo, DWORD dwSizeCopy, BOOL bMove, DWORD dwFlag);

public:
	virtual RUIBuffer*		NewBuffer();
#ifdef _DEBUG // m_bAddLock을 이용하여 중간에 추가되는 것을 체크
	virtual RUIBuffer*		AddBuffer(DWORD dwSize);
	virtual RUIBuffer*		AddBuffer(BYTE* pCopyFrom, DWORD dwSize);
	virtual BOOL			NewBufferList(DWORD dwSize, UINT nCount);
#endif
//	virtual void			DeleteBufferList();
//	virtual RUIBuffer*		PopHead();
//	virtual DWORD			GetBufferSize();
//	virtual DWORD			CopyTo(BYTE* pCopyTo, DWORD dwSizeCopy, DWORD dwFlag);
//	virtual DWORD			MoveTo(BYTE* pMoveTo, DWORD dwSizeMove, DWORD dwFlag);

public:
	int						GetUseBufferCount();
	DWORD					GetUseBufferSize();
	RUIUseBuffer*			FindFirstUnuseBuffer();
	RUIUseBuffer*			FindFirstUseBuffer();
	RUIUseBuffer*			FindLastUseBuffer();

	void					SetUnuseBufferAll();
	BOOL					SetUnuseBufferAndMoveTail(RUIUseBuffer* pRUIUseBuffer);
};

#endif
