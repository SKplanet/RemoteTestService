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
#include "RUIBufferVideoEncoderThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUIBufferVideoEncoderThread::RUIBufferVideoEncoderThread()
{
	m_pieParam          = NULL;
	m_pMsgWnd           = NULL;

	m_pRUIBufferRGB     = NULL;
	m_pRUIBufferList264 = NULL;
	m_nWidth            = 800;
	m_nHeight           = 480;
	m_dwBitRate         = 150000;
}

RUIBufferVideoEncoderThread::~RUIBufferVideoEncoderThread()
{
}

BOOL RUIBufferVideoEncoderThread::SetBitRateRuntime(DWORD dwBitRate)
{
	BOOL	bResult = FALSE;

	if (m_encoder.pCodec != NULL)
	{
		Status				st;
		VideoEncoderParams	newpar;

		st = m_encoder.pCodec->GetInfo(&newpar);

		if (st == UMC_OK)
		{
			newpar.info.bitrate = dwBitRate;
			st = m_encoder.pCodec->SetParams(&newpar);

			if (st == UMC_OK)
				bResult = TRUE;
		}
	}

	return bResult;
}

void RUIBufferVideoEncoderThread::SetRUIBuffer(RUIBuffer* pRUIBufferRGB, RUIBufferList* pRUIBufferList264)
{
	if (pRUIBufferRGB == NULL || pRUIBufferList264 == NULL)
	{
		ASSERT(FALSE);
		return;
	}

	m_pRUIBufferRGB     = pRUIBufferRGB;
	m_pRUIBufferList264 = pRUIBufferList264;
}

BOOL RUIBufferVideoEncoderThread::Stop(BOOL bSync)
{
	// [neuromos] RUIBufferVideoEncoderThread는 한번 ThreadProc()로 들어가면 종료될 때 까지 나오지 않는다. 
	// [neuromos] 아래 RUIThread::Stop()에서 bSync로 종료될 때 까지 기다리므로 먼저 bStop Flag를 셋팅한다.
	// [neuromos] RUIBufferVideoEncoder에서 인코딩하다가 m_bStop을 확인 후 종료하면 RUIThread::WorkThread()를 빠져나오게 된다.
	m_encoder.SetStop(TRUE);

	// [neuromos] IPP Encoder가 완전히 종료될 때 까지 기다린다.
	while (m_encoder.IsPlaying())
		Sleep(10);

	return RUIThread::Stop(bSync);
}

BOOL RUIBufferVideoEncoderThread::OnStart()
{
	RUIThread::OnStart();

#ifdef WRITE_ENCODED_FILE
	if (m_encoder.GetFile()->Open(_T("C:\\capture.rgb.h264"), CFile::modeCreate | CFile::modeWrite))
	{
		m_encoder.SetFileEnable(TRUE);
	}
#endif

	m_encoder.SetStop(FALSE);

	return TRUE;
}

void RUIBufferVideoEncoderThread::OnStop()
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

BOOL RUIBufferVideoEncoderThread::ThreadProc()
{
	if (m_pRUIBufferRGB == NULL && m_pRUIBufferList264 != NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_encoder.Encode(m_pRUIBufferRGB, m_pRUIBufferList264, m_nWidth, m_nHeight, m_dwBitRate);

	// [neuromos] 위 라인에서 Encoding하는 동안 Block되므로 무조건 Stop한다.
	return FALSE;
}
