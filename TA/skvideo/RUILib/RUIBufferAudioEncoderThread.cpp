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
#include "RUIBufferAudioEncoderThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUIBufferAudioEncoderThread::RUIBufferAudioEncoderThread()
{
	m_pieParam          = NULL;
	m_pMsgWnd           = NULL;

	m_pRUIBufferListPCM = NULL;
}

RUIBufferAudioEncoderThread::~RUIBufferAudioEncoderThread()
{
}

void RUIBufferAudioEncoderThread::SetRUIBufferList(RUIBufferList* pRUIBufferListPCM)
{
	m_pRUIBufferListPCM = pRUIBufferListPCM;
}

BOOL RUIBufferAudioEncoderThread::Stop(BOOL bSync)
{
	// [neuromos] RUIBufferAudioEncoderThread는 한번 ThreadProc()로 들어가면 종료될 때 까지 나오지 않는다. 
	// [neuromos] 아래 RUIThread::Stop()에서 bSync로 종료될 때 까지 기다리므로 먼저 bStop Flag를 셋팅한다.
	// [neuromos] RUIBufferVideoEncoder에서 인코딩하다가 m_bStop을 확인 후 종료하면 RUIThread::WorkThread()를 빠져나오게 된다.
	m_encoder.SetStop(TRUE);

	// [neuromos] IPP Encoder가 완전히 종료될 때 까지 기다린다.
	while (m_encoder.IsPlaying())
		Sleep(10);

	return RUIThread::Stop(bSync);
}

BOOL RUIBufferAudioEncoderThread::OnStart()
{
	RUIThread::OnStart();

#ifdef WRITE_ENCODED_FILE
	if (m_encoder.GetFile()->Open(_T("C:\\test.aac"), CFile::modeCreate | CFile::modeWrite))
	{
		m_encoder.SetFileEnable(TRUE);
	}
#endif

	m_encoder.SetStop(FALSE);

	return TRUE;
}

void RUIBufferAudioEncoderThread::OnStop()
{
	RUIThread::OnStop();

#ifdef WRITE_ENCODED_FILE
	if (m_encoder.GetFileEnable())
	{
		m_encoder.GetFile()->Close();
		m_encoder.SetFileEnable(FALSE);
	}
#endif
}

BOOL RUIBufferAudioEncoderThread::ThreadProc()
{
	if (m_pRUIBufferListPCM == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_encoder.Encode(m_pRUIBufferListPCM, m_nAudioSliceLen);

	// [neuromos] 위 라인에서 Encoding하는 동안 Block되므로 무조건 Stop한다.
	return FALSE;
}
