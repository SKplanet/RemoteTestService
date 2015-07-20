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
#include "RUIAudio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUIAudio::RUIAudio()
{
	m_pieParam              = NULL;

#ifdef _USE_AAC_CODEC
#else
	m_nCodec                = G721_32;
#endif

	// 8000Hz, 16bit Sampling
	m_WFEX.wFormatTag		= WAVE_FORMAT_PCM;
	m_WFEX.nChannels		= 1;
	m_WFEX.nSamplesPerSec	= 16000;
	m_WFEX.nAvgBytesPerSec	= 32000;
	m_WFEX.nBlockAlign		= 2;
	m_WFEX.wBitsPerSample	= 16;
	m_WFEX.cbSize			= 0;

	m_nWaveInDeviceID       = WAVE_MAPPER;

	TRACE("set RUIAudio::m_hWaveIn make NULL in Constructor\n");
	m_hWaveIn				= NULL;
	m_nFrameType            = 0;
	m_dwFrameKey            = 0;

	m_hWaveOut				= NULL;

	m_RecBufferList .NewBufferList(sizeof(WAVEHDR) + WAVE_FRAME_SIZE, RUI_BUFFER_COUNT);
	m_PlayBufferList.NewBufferList(sizeof(WAVEHDR) + WAVE_FRAME_SIZE, RUI_BUFFER_COUNT);

#ifdef _USE_AAC_CODEC
	m_pRUIBufferListPCM     = NULL;
#else
	m_pRUIBufferListSend    = NULL;
	m_pRUIBufferListRecv    = NULL;
#endif
}

RUIAudio::~RUIAudio()
{
	RecordClose();
	PlayClose();
}

BOOL RUIAudio::OnStart()
{
	RUIThread::OnStart();

	if (m_bPlayOpen)
	{
#ifdef _USE_AAC_CODEC
		if (m_pRUIBufferListPCM == NULL)
		{
			//ASSERT(FALSE);
			return FALSE;
		}
#else
		if (m_pRUIBufferListRecv == NULL)
		{
			ASSERT(FALSE);
			return FALSE;
		}

		m_pRUIBufferListRecv->SetEnableFrameType(m_nFrameType);
#endif

		SetSleepTime(10); // [neuromos] Play이면 버퍼가 비어있을 때 재생 못했다가 채워지면 다시 재생해야 하기 때문에 10ms 쉼
	}
	else
	{
		SetSleepTime(500); // [neuromos] Record면 Thread에서 할 일이 없으므로 500ms 쉼
	}

	return TRUE;
}

void RUIAudio::OnStop()
{
	RUIThread::OnStop();

	if (m_bPlayOpen)
	{
#ifdef _USE_AAC_CODEC
#else
		if (m_pRUIBufferListRecv != NULL)
		{
			m_pRUIBufferListRecv->ResetEnableFrameType(m_nFrameType);
			m_pRUIBufferListRecv->RemoveUDPFrame      (m_nFrameType);
		}
		else
			ASSERT(FALSE);
#endif
	}
}

BOOL RUIAudio::ThreadProc()
{
	BOOL	bResult = TRUE;

	if (m_hWaveIn != NULL)
	{
		// [neuromos] m_bRecOpen(필요없어 만들지 않음)인 경우.
		// [neuromos] 계속 녹음되고 Callback에서 처리하기 때문에 Thread에서 할 일이 없다.
	}

	else if (m_bPlayOpen)
	{
		if (RecvListToPlayList())
		{
			if (m_nWaitSyncCount > 0)
				m_nWaitSyncCount--;
		}

		if (m_nWaitSyncCount <= 0)
		{
			if (GetPlayCount() < 2)
				PlayListToPlay();
		}
	}

	return bResult;
}
