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
#include "RUIBufferVideoEncoder.h"
#include "RUILibDef.h"

#include "../h264/common/vm/vm_time.h"
#include "../h264/common/vm/vm_strings.h"
#include "../h264/common/vm/vm_sys_info.h"

#include <ippvc.h>

RUIBufferVideoEncoder::RUIBufferVideoEncoder()
{
	m_bStop                     = FALSE;
	tickDuration                = 1.0 / (Ipp64f) vm_time_get_frequency();
	m_dwLastReadVideoSourceTick = 0;
	m_pRUIBufferRGB             = NULL;
	m_pRUIBufferList264         = NULL;
	m_dwEncodedSize             = 0;

	m_bUseSocket                = FALSE;
	m_nFrameType                = 0;
	m_dwFrameKey                = 0;
	m_pRUIBufferListSend        = NULL;

#ifdef WRITE_ENCODED_FILE
	m_bFile = FALSE;
#endif

	m_bFirstFrame               = FALSE;
}

RUIBufferVideoEncoder::~RUIBufferVideoEncoder()
{
}

BOOL RUIBufferVideoEncoder::Encode(RUIBuffer* pRUIBufferRGB, RUIBufferList* pRUIBufferList264, int nWidth, int nHeight, DWORD dwBitRate)
{
	if (pRUIBufferRGB == NULL || pRUIBufferList264 == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_dwLastReadVideoSourceTick = GetTickCount();
	m_pRUIBufferRGB             = pRUIBufferRGB;
	m_pRUIBufferList264         = pRUIBufferList264;
	m_dwEncodedSize             = 0;

	if (ippStaticInit() < ippStsNoErr)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pCodec         = new H264VideoEncoder();
	pEncoderParams = new VideoEncoderParams;

	numFramesToEncode                     = 0x7fffffff;

	pEncoderParams->info.clip_info.width  = nWidth;
	pEncoderParams->info.clip_info.height = nHeight;
	pEncoderParams->info.framerate        = RUI_FRAMERATE;
	// [neuromos] *원하는 인코딩 비트레이트 지정
	pEncoderParams->info.bitrate          = dwBitRate;
	pEncoderParams->numThreads            = 1;

	m_pieParam->m_dwFrameRate             = (DWORD) (pEncoderParams->info.framerate);

	// [neuromos] 계산된 것의 80%만 쉬도록 한다.
//	m_dwFrameTick = (DWORD) (1000 / pEncoderParams->info.framerate);
//	m_dwFrameTick = (DWORD) ( 800 / pEncoderParams->info.framerate);

	DWORD	dwFrameTick = (DWORD) ( 800 / pEncoderParams->info.framerate);
	SetFrameTimeStart   (dwFrameTick);
	SetFrameTimeModified(dwFrameTick);

	SetBitRateStart   (dwBitRate);
	SetBitRateModified(dwBitRate);

	if (pCodec->Init   (pEncoderParams) == UMC_OK &&
		pCodec->GetInfo(pEncoderParams) == UMC_OK)
	{
		m_bFirstFrame = TRUE;

		Run();
	}

	delete pCodec;
	delete pEncoderParams;

	return 0;
}

Status RUIBufferVideoEncoder::PutOutputData(MediaData* out)
{
	if (! out) // at EOF
		return UMC_ERR_NULL_PTR;

	return IPPVideoEncoder::PutOutputData(out);
}

Ipp64f RUIBufferVideoEncoder::GetTick()
{
	return (Ipp64f) vm_time_get_tick() * tickDuration;
}

int RUIBufferVideoEncoder::ReadVideoSource(BYTE* pBuffer, DWORD dwOffset, int nItemSize, int nItemCount)
{
	if (m_pRUIBufferRGB == NULL)
	{
		ASSERT(FALSE);
		return 0;
	}

	if (m_bStop) // [neuromos] 외부에서 중지시킴
		return 0;

	int	nReadSize = nItemSize * nItemCount;

	// [neuromos] [sync error] Encoder내에서 처음 버퍼를 읽을 때 500ms 쉬도록 수정
	if (m_bFirstFrame)
	{
		Sleep(1000);
		m_bFirstFrame = FALSE;
	}

	// [neuromos] Frame Rate를 조정하는 부분
	if (dwOffset == 0) // Frame의 첫 부분일 때 FrameRate 체크
	{
		while (m_dwLastReadVideoSourceTick + m_dwFrameTimeModified > GetTickCount()) // 아직 인코딩을 하기에 빠르거나
		{
			Sleep(5);

			if (m_bStop) // [neuromos] 외부에서 중지시킴
				return 0;
		}

		m_dwLastReadVideoSourceTick = GetTickCount();
	}

	DWORD	dwCopySize;

	dwCopySize = m_pRUIBufferRGB->CopyTo(pBuffer, dwOffset, nReadSize);

	if (dwCopySize != (DWORD) nReadSize)
		ASSERT(FALSE);

	return nItemCount;
}

int RUIBufferVideoEncoder::WriteVideoTarget(BYTE* pBuffer, int nItemSize, int nItemCount, FrameType frameType)
{
	IPPVideoEncoder::WriteVideoTarget(pBuffer, nItemSize, nItemCount, frameType);

	if (m_pRUIBufferList264 == NULL)
		return 0;

	if (m_bStop) // [neuromos] 외부에서 중지시킴
		return 0;

	int	nWriteSize = nItemSize * nItemCount;

	if (m_bUseSocket)
	{
		if (m_pRUIBufferListSend != NULL)
		{
			m_pRUIBufferListSend->AddUDPFrame(m_nFrameType, (BYTE) frameType, m_dwFrameKey, (DWORD) nWriteSize, m_dwEncoderTickCurr, (BYTE*) pBuffer, (DWORD) nWriteSize, 0);

			m_dwFrameKey++;
		}
	}
	else
	{
		RUIBuffer*	pRUIBuffer264 = m_pRUIBufferList264->AddBuffer(pBuffer, (DWORD) nWriteSize);

		if (pRUIBuffer264 == NULL)
		{
			ASSERT(FALSE); // [neuromos] 버퍼를 못만드는 경우는 없다.
			return 0;
		}
	}

	m_dwEncodedSize += nWriteSize;

#ifdef WRITE_ENCODED_FILE
	if (m_bFile)
	{
		m_fileEncoded.Write((const void*) pBuffer, nWriteSize);
	}
#endif

	return nItemCount;
}
