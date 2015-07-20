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
	// [neuromos] RUIBufferAudioEncoderThread�� �ѹ� ThreadProc()�� ���� ����� �� ���� ������ �ʴ´�. 
	// [neuromos] �Ʒ� RUIThread::Stop()���� bSync�� ����� �� ���� ��ٸ��Ƿ� ���� bStop Flag�� �����Ѵ�.
	// [neuromos] RUIBufferVideoEncoder���� ���ڵ��ϴٰ� m_bStop�� Ȯ�� �� �����ϸ� RUIThread::WorkThread()�� ���������� �ȴ�.
	m_encoder.SetStop(TRUE);

	// [neuromos] IPP Encoder�� ������ ����� �� ���� ��ٸ���.
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

	// [neuromos] �� ���ο��� Encoding�ϴ� ���� Block�ǹǷ� ������ Stop�Ѵ�.
	return FALSE;
}
