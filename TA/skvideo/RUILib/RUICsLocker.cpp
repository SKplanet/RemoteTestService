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
#include "RUICsLocker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUICriticalSection::RUICriticalSection()
{
	::InitializeCriticalSection(&m_cs);
}

RUICriticalSection::~RUICriticalSection()
{
	::DeleteCriticalSection(&m_cs);
}

void RUICriticalSection::Enter()
{
	::EnterCriticalSection(&m_cs);
}

void RUICriticalSection::Leave()
{
	::LeaveCriticalSection(&m_cs);
}

#if(_WIN32_WINNT >= 0x0400)
BOOL RUICriticalSection::Try()
{
	return TryEnterCriticalSection(&m_cs);
}
#endif

RUICsLocker::RUICsLocker(RUICriticalSection& ruiCs) :
	m_ruiCs(ruiCs)
{
	m_ruiCs.Enter();
}

RUICsLocker::~RUICsLocker()
{
	m_ruiCs.Leave();
}
