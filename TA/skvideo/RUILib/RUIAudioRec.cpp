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

UINT RUIAudio::EnumWaveInDevice(ENUM_WAVEIN_DEVICE_CALLBACK pfnCallback, LPVOID pParam)
{
	UINT	nResult = 0;
	UINT	nMaxDev = ::waveInGetNumDevs();

	for (UINT i = 0; i < nMaxDev; i++)
	{
		WAVEINCAPS	wic;

		if (::waveInGetDevCaps(i, &wic, sizeof(wic)) == MMSYSERR_NOERROR)
		{
			if (pfnCallback(i, &wic, pParam))
				nResult++;
		}
	}

	return nResult;
}

MMRESULT RUIAudio::RecordOpen()
{
	RecordClose();

#ifdef _USE_AAC_CODEC
	if (m_pRUIBufferListPCM == NULL)
	{
		ASSERT(FALSE);
		return ERR_AUDIO_RECORD_CANTADD;
	}
#else
	if (m_pRUIBufferListSend == NULL)
	{
		ASSERT(FALSE);
		return ERR_AUDIO_RECORD_CANTADD;
	}
#endif

	if (waveInOpen(&m_hWaveIn, m_nWaveInDeviceID, (LPWAVEFORMATEX) &m_WFEX, (ULONG) WaveInProc, (DWORD_PTR) this, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
		return ERR_AUDIO_RECORD_CANTOPEN;

	RUIBuffer*	pRUIBuffer = m_RecBufferList.GetFirst();
	while (pRUIBuffer != NULL)
	{
		RecAddBuffer(pRUIBuffer);

		pRUIBuffer = m_RecBufferList.GetNext(pRUIBuffer);
	}

	if (waveInStart(m_hWaveIn) != MMSYSERR_NOERROR)
	{
		RecordClose();
		return ERR_AUDIO_RECORD_CANTOPEN;
	}

	if (! Run())
	{
		ASSERT(FALSE);

		RecordClose();
		return ERR_AUDIO_THREAD_DOESNTEXIST;
	}

	return ERR_AUDIO_NOERROR;
}

void RUIAudio::RecordClose()
{
	Stop();
	HWAVEIN hTemp = m_hWaveIn;
	m_hWaveIn = NULL;
	if (hTemp == NULL)
		return;

	RUIBuffer*	pRUIBuffer = m_RecBufferList.GetFirst();
	while (pRUIBuffer != NULL)
	{
		RecRemoveBuffer(pRUIBuffer);

		pRUIBuffer = m_RecBufferList.GetNext(pRUIBuffer);
	}

	if( hTemp == NULL )
		return;

	if (waveInReset(hTemp) != MMSYSERR_NOERROR)
		ASSERT(FALSE);

	if (waveInClose(hTemp) != MMSYSERR_NOERROR)
		ASSERT(FALSE);
}

BOOL RUIAudio::RecAddBuffer(RUIBuffer* pRUIBuffer)
{
	if (pRUIBuffer == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	BYTE*	pBufferRec = pRUIBuffer->GetBuffer();

	WAVEHDR*	pWaveHdr = (WAVEHDR*) pBufferRec;

	pWaveHdr->lpData         = (LPSTR) (pBufferRec + sizeof(WAVEHDR));
	pWaveHdr->dwBufferLength = pRUIBuffer->GetBufferSize() - sizeof(WAVEHDR);
	pWaveHdr->dwFlags        = 0;
	pWaveHdr->dwUser         = 0;

	if (waveInPrepareHeader(m_hWaveIn, pWaveHdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (waveInAddBuffer(m_hWaveIn, pWaveHdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return TRUE;
}

BOOL RUIAudio::RecRemoveBuffer(RUIBuffer* pRUIBuffer)
{
	if (pRUIBuffer == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	BYTE*	pBufferRec = pRUIBuffer->GetBuffer();

	WAVEHDR*	pWaveHdr = (WAVEHDR*) pBufferRec;

	pWaveHdr->lpData         = (LPSTR) (pBufferRec + sizeof(WAVEHDR));
	pWaveHdr->dwBufferLength = pRUIBuffer->GetBufferSize() - sizeof(WAVEHDR);
	pWaveHdr->dwFlags        = 0;
	pWaveHdr->dwUser         = 0;

	if (waveInUnprepareHeader(m_hWaveIn, pWaveHdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return TRUE;
}

void RUIAudio::On_WIM_DATA(WAVEHDR* pWaveHdr)
{
	if (m_hWaveIn != NULL && pWaveHdr != NULL && m_pRUIBufferListPCM != NULL && pWaveHdr->dwBytesRecorded > 0)
	{
		DWORD	dwCurrTick = GetTickCount();

#ifdef _USE_AAC_CODEC
		// [neuromos] !아래 코드는 TCP만 지원함
		RUIBuffer*	pRUIBufferPCM = m_pRUIBufferListPCM->AddBuffer((BYTE*) (pWaveHdr->lpData), pWaveHdr->dwBytesRecorded, dwCurrTick);
#else
		if (m_nCodec != PCM_16)
		{
			DWORD	dwFrameSizeEnc = EncodeVoicePCM(m_nCodec, (void*) (pWaveHdr->lpData), pWaveHdr->dwBytesRecorded, (void*) m_pFrameRecEnc, WAVE_FRAME_SIZE);

			m_pRUIBufferListSend->AddUDPFrame(m_nFrameType, 0, m_dwFrameKey, dwFrameSizeEnc           , dwCurrTick, (BYTE*) m_pFrameRecEnc    , dwFrameSizeEnc           , 0);
		}
		else
			m_pRUIBufferListSend->AddUDPFrame(m_nFrameType, 0, m_dwFrameKey, pWaveHdr->dwBytesRecorded, dwCurrTick, (BYTE*) (pWaveHdr->lpData), pWaveHdr->dwBytesRecorded, 0);
#endif

		m_dwFrameKey++;

		if (waveInPrepareHeader(m_hWaveIn, pWaveHdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			ASSERT(FALSE);
			return;
		}
		
		if( waveInAddBuffer    (m_hWaveIn, pWaveHdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
		{
			ASSERT(FALSE);
			return;
		}
	}
}

void CALLBACK WaveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	switch (uMsg)
	{
		case WIM_OPEN : break;

		case WIM_DATA :
			{
				RUIAudio*	pAudio = (RUIAudio*) dwInstance;
				pAudio->On_WIM_DATA((WAVEHDR*) dwParam1);
			}
			break;

		case WIM_CLOSE : break;
	}
}
