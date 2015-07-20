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
#include "RUIThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DWORD WINAPI __Call_RUIThread_WorkThread__(void* pRUIThreadClass)
{
	((RUIThread*) pRUIThreadClass)->WorkThread();

	return 0;
}

RUIThread::RUIThread()
{
	m_pruiCsLocker = NULL;

	SetRun (FALSE);
	SetStop(FALSE);

	m_dwSleepTime = 1;
}

RUIThread::~RUIThread()
{
	if (IsRunning())
		Stop(TRUE);

	if (m_pruiCsLocker != NULL)
	{
		ASSERT(FALSE);
		delete m_pruiCsLocker;
		m_pruiCsLocker = NULL;
	}
}

BOOL RUIThread::GetRun()
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return m_bRun;
}

void RUIThread::SetRun(BOOL bRun)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	m_bRun = bRun;
}

BOOL RUIThread::GetStop()
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return m_bStop;
}

void RUIThread::SetStop(BOOL bStop)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	m_bStop = bStop;
}

void RUIThread::WorkThread()
{
	if (! OnStart())
		return;

	SetRun (TRUE );
	SetStop(FALSE);

	while (! GetStop())
	{
		if (! ThreadProc())
			break;

		Sleep(m_dwSleepTime);
	}

	OnStop();

	SetRun (FALSE);
	SetStop(FALSE);

	if (m_pruiCsLocker != NULL)
	{
		delete m_pruiCsLocker;
		m_pruiCsLocker = NULL;
	}
}

BOOL RUIThread::IsRunning()
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	return m_bRun;
}

DWORD RUIThread::GetSleepTime()
{
	return m_dwSleepTime;
}

void RUIThread::SetSleepTime(DWORD dwSleepTime)
{
	m_dwSleepTime = dwSleepTime;
}

DWORD RUIThread::IncSleepTime()
{
	m_dwSleepTime++;

	return m_dwSleepTime;
}

DWORD RUIThread::DecSleepTime()
{
	if (m_dwSleepTime > 0)
		m_dwSleepTime--;

	return m_dwSleepTime;
}

BOOL RUIThread::Run()
{
	if (m_pruiCsLocker != NULL)
		return FALSE;

	m_pruiCsLocker = new RUICsLocker(m_ruiCsRun);

	if (! GetRun())
	{
		HANDLE	hThread = ::CreateThread(NULL, 0, ::__Call_RUIThread_WorkThread__, this, 0, NULL);
		CloseHandle(hThread);
		m_hThread = hThread;
		return TRUE;
	}

	ASSERT(FALSE);

	return FALSE;
}

BOOL RUIThread::Stop(BOOL bSync)
{
	SetStop(TRUE);

	if (bSync)
	{
		DWORD dwCount = 3000; // 30sec
		while (GetRun() && dwCount-- > 0)
		{
			Sleep(10);
		}

		if( dwCount == 0 )
		{
			::TerminateThread(m_hThread, 1);
		}
	}

	return TRUE;
}

BOOL RUIThread::OnStart()
{
	return TRUE;
}

void RUIThread::OnStop()
{
}
