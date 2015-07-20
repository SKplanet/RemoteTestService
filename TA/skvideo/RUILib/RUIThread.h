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

#ifndef _RUITHREAD_H
#define _RUITHREAD_H

#include "RUICsLocker.h"

class RUIThread
{
public:
	RUIThread();
	virtual ~RUIThread();

private:
	RUICriticalSection		m_ruiCs;
	RUICriticalSection		m_ruiCsRun;
	HANDLE				m_hThread;
        RUICsLocker*			m_pruiCsLocker;
	BOOL					m_bRun;
	BOOL					m_bStop;
	DWORD					m_dwSleepTime;

private:
	BOOL					GetRun();
	void					SetRun(BOOL bRun);
	BOOL					GetStop();
	void					SetStop(BOOL bStop);

public:
	void					WorkThread();

	BOOL					IsRunning();
	DWORD					GetSleepTime();
	void					SetSleepTime(DWORD dwSleepTime);
	DWORD					IncSleepTime();
	DWORD					DecSleepTime();

	BOOL					Run();

public:
	virtual BOOL			Stop(BOOL bSync = TRUE);
	virtual BOOL			OnStart();
	virtual void			OnStop();
	virtual BOOL			ThreadProc() = 0;
};

#endif
