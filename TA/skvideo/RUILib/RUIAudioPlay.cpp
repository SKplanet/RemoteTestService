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
#include "G723/encode.h"
#include "G723/decode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DWORD RUIAudio::PlayOpen()
{
	PlayClose();

#ifdef _USE_AAC_CODEC
	if (m_pRUIBufferListPCM == NULL)
	{
		ASSERT(FALSE);
		return ERR_AUDIO_PLAY_CANTADD;
	}
#else
	if (m_pRUIBufferListRecv == NULL)
	{
		ASSERT(FALSE);
		return ERR_AUDIO_PLAY_CANTADD;
	}
#endif

	if (waveOutOpen(&m_hWaveOut, WAVE_MAPPER, (LPWAVEFORMATEX) &m_WFEX, (ULONG) WaveOutProc, (DWORD_PTR) this, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
		return ERR_AUDIO_PLAY_CANTOPEN;

	RUIBuffer*	pRUIBuffer = m_PlayBufferList.GetFirst();
	while (pRUIBuffer != NULL)
	{
		PlayAddBuffer(pRUIBuffer);

		pRUIBuffer = m_PlayBufferList.GetNext(pRUIBuffer);
	}

	m_bPlayOpen = TRUE;
	m_bPlaySync = FALSE;

	m_nWaitSyncCount = 10;

	ResetPlayCount();

	if (! Run())
	{
		ASSERT(FALSE);

		PlayClose();
		return ERR_AUDIO_THREAD_DOESNTEXIST;
	}

	return ERR_AUDIO_NOERROR;
}

void RUIAudio::PlayClose()
{
	Stop();

	if (m_hWaveOut == NULL)
		return;

	if (waveOutPause(m_hWaveOut) != MMSYSERR_NOERROR)
		ASSERT(FALSE);

	if (waveOutReset(m_hWaveOut) != MMSYSERR_NOERROR)
		ASSERT(FALSE);

	RUIBuffer*	pRUIBuffer = m_PlayBufferList.GetFirst();
	while (pRUIBuffer != NULL)
	{
		PlayRemoveBuffer(pRUIBuffer);

		pRUIBuffer = m_PlayBufferList.GetNext(pRUIBuffer);
	}

	if (waveOutClose(m_hWaveOut) != MMSYSERR_NOERROR)
		ASSERT(FALSE);

	m_hWaveOut  = NULL;
	m_bPlayOpen = FALSE;
	m_bPlaySync = FALSE;

	m_nWaitSyncCount = 10;

	ResetPlayCount();

	m_RecBufferList .SetUnuseBufferAll();
	m_PlayBufferList.SetUnuseBufferAll();
	m_WaitBufferList.DeleteBufferList();
}

BOOL RUIAudio::PlayAddBuffer(RUIBuffer* pRUIBuffer)
{
	if (pRUIBuffer == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	BYTE*	pBufferPlay = pRUIBuffer->GetBuffer();

	WAVEHDR*	pWaveHdr = (WAVEHDR*) pBufferPlay;

	pWaveHdr->lpData         = (LPSTR) (pBufferPlay + sizeof(WAVEHDR));
	pWaveHdr->dwBufferLength = pRUIBuffer->GetBufferSize() - sizeof(WAVEHDR);
	pWaveHdr->dwFlags        = 0;
	pWaveHdr->dwUser         = 0;

	if (waveOutPrepareHeader(m_hWaveOut, pWaveHdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return TRUE;
}

BOOL RUIAudio::PlayRemoveBuffer(RUIBuffer* pRUIBuffer)
{
	if (pRUIBuffer == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	BYTE*	pBufferPlay = pRUIBuffer->GetBuffer();
	WAVEHDR*	pWaveHdr = (WAVEHDR*) pBufferPlay;

	if (waveOutUnprepareHeader(m_hWaveOut, pWaveHdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return TRUE;
}

BOOL RUIAudio::RecvListToPlayList()
{
#ifdef _USE_AAC_CODEC
	RUIBuffer*	pRUIBufferRecv = m_pRUIBufferListPCM->PopHead();
#else
//	RUIBuffer*	pRUIBufferRecv = m_pRUIBufferListRecv->PopHeadCompleted(m_nFrameType);
	RUIBuffer*	pRUIBufferRecv = m_pRUIBufferListRecv->PopLimitHeadCompleted(m_nFrameType, m_pieParam->m_dwFrameRecvTickAudioLimit);
#endif

	if (pRUIBufferRecv != NULL)
	{
			DWORD	dwBufferSize = pRUIBufferRecv->GetBufferSize();
		RUIBuffer*	pRUIBufferWait = m_WaitBufferList.AddBuffer(dwBufferSize);
		BYTE*		pWaveData      = pRUIBufferWait->GetBuffer();

#ifdef _USE_AAC_CODEC
#else
			if (m_nCodec != PCM_16)
			{
				DWORD	dwPcmSize = 0;

				if (pRUIBufferRecv->CopyTo(m_pFramePlayEnc, dwBufferSize) == dwBufferSize)
				{
					dwPcmSize = (DWORD) DecodeVoicePCM(m_nCodec, (void*) m_pFramePlayEnc, dwBufferSize, (void*) pWaveData, WAVE_FRAME_SIZE);
				}
				else
					ASSERT(FALSE);

				if (dwPcmSize != WAVE_FRAME_SIZE)
					ASSERT(FALSE);
			}
			else
#endif
			{
			if (dwBufferSize                                            != WAVE_FRAME_SIZE ||
				pRUIBufferRecv->CopyTo((BYTE*) pWaveData, dwBufferSize) != WAVE_FRAME_SIZE)
					ASSERT(FALSE);
			}

		pRUIBufferWait->SetParam(pRUIBufferRecv->GetParam());

		delete pRUIBufferRecv;

		return TRUE;
	}

	// [neuromos] 아직 Play할 패킷이 도착하지 않았음

	return FALSE;
}

void RUIAudio::PlayListToPlay()
{
	RUIUseBuffer*	pRUIUseBufferPlay = m_PlayBufferList.FindFirstUnuseBuffer();

	if (pRUIUseBufferPlay == NULL)
		return; // [neuromos] Wave Out 할 버퍼가 없음. (이런 경우는 아마도 없다)

	BYTE*		pBufferPlay = pRUIUseBufferPlay->GetBuffer();
	WAVEHDR*	pWaveHdr    = (WAVEHDR*) pBufferPlay;

	if ((pWaveHdr->dwFlags & WHDR_PREPARED) != WHDR_PREPARED &&
		(pWaveHdr->dwFlags & WHDR_DONE    ) != WHDR_DONE)
	{
		ASSERT(FALSE); // [neuromos] 분명 Unuse인데 아직 Play중 이다.
		return;
	}

	RUIBuffer*	pRUIBufferWait;

	if (! m_bPlaySync)
	{
		if (m_pieParam->m_bVideoAudioSync)
		{
			pRUIBufferWait = m_WaitBufferList.FindBufferForLessParam(m_pieParam->m_dwAudioTickCurr, 8);
		}
		else
		{
			pRUIBufferWait = m_WaitBufferList.GetLast();
		}

		if (pRUIBufferWait != NULL)
		{
			m_WaitBufferList.DeleteBufferListBefore(pRUIBufferWait);

			m_bPlaySync = TRUE;
		}
		else
			return; // [neuromos] Sync하지 못했음. 시간이 큰 것만 있음. Play할 필요없음.
	}

	pRUIBufferWait = m_WaitBufferList.PopHead();

	if (pRUIBufferWait != NULL)
	{
		CopyMemory(pWaveHdr->lpData, pRUIBufferWait->GetBuffer(), WAVE_FRAME_SIZE);
		pRUIUseBufferPlay->SetUse(TRUE);

		MMRESULT	mmResult;

		if ((mmResult = waveOutWrite(m_hWaveOut, pWaveHdr, sizeof(WAVEHDR))) == MMSYSERR_NOERROR)
		{
			IncPlayCount();
		}
		else
		{
			pRUIUseBufferPlay->SetUse(FALSE);
			ASSERT(FALSE); // Wave Out 실패
		}

		if (m_pieParam->m_bVideoAudioSync)
		{
			DWORD	dwParamWait = pRUIBufferWait->GetParam();
			if (m_pieParam->m_dwAudioTickCurr > dwParamWait)
			{
				if ((m_pieParam->m_dwAudioTickCurr - dwParamWait) > 200)
					m_bPlaySync = FALSE;
			}
			else
			{
				if ((dwParamWait - m_pieParam->m_dwAudioTickCurr) > 200)
					m_bPlaySync = FALSE;
			}
		}
		else
		{
			if (m_WaitBufferList.GetCount() > 2)
				m_bPlaySync = FALSE;
		}

		delete pRUIBufferWait;
	}
	else
	{
		// Play할 버퍼가 없음.
	}
}

void RUIAudio::On_WOM_DONE(WAVEHDR* pWaveHdr)
{
	DecPlayCount();

	if (m_hWaveOut != NULL && pWaveHdr != NULL)
	{
		RUIUseBuffer*	pRUIUseBufferPlay = (RUIUseBuffer*) m_PlayBufferList.FindBufferByBuffer((BYTE*) pWaveHdr);

		if (pRUIUseBufferPlay != NULL)
		{
			m_PlayBufferList.SetUnuseBufferAndMoveTail(pRUIUseBufferPlay);
		}
		else
		{
			ASSERT(FALSE); // Wave Out 한 버퍼를 찾지 못함.
		}
	}
}

void CALLBACK WaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	switch (uMsg)
	{
		case WOM_OPEN : break;

		case WOM_DONE :
			{
				RUIAudio*	pAudio = (RUIAudio*) dwInstance;
				pAudio->On_WOM_DONE((WAVEHDR*) dwParam1);
			}
			break;

		case WOM_CLOSE : break;
	}
}

void RUIAudio::ResetPlayCount()
{
	RUICsLocker	ruiCsLocker(m_playCs);
	m_nPlayCount = 0;
}

int RUIAudio::IncPlayCount()
{
	RUICsLocker	ruiCsLocker(m_playCs);
	return ++m_nPlayCount;
}

int RUIAudio::DecPlayCount()
{
	RUICsLocker	ruiCsLocker(m_playCs);
	return --m_nPlayCount;
}

int RUIAudio::GetPlayCount()
{
	RUICsLocker	ruiCsLocker(m_playCs);
	return m_nPlayCount;
}
