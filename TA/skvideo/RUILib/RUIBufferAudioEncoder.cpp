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
#include "RUIBufferAudioEncoder.h"
#include "RUILibDef.h"

#include "../h264/common/vm/vm_time.h"
#include "../h264/common/vm/vm_strings.h"
#include "../h264/common/vm/vm_sys_info.h"

#include <ippvc.h>

RUIBufferAudioEncoder::RUIBufferAudioEncoder()
{
	m_bStop                     = FALSE;
	tickDuration                = 1.0 / (Ipp64f) vm_time_get_frequency();
	m_dwLastReadVideoSourceTick = 0;
	m_pRUIBufferListPCM         = NULL;
	m_dwEncodedSize             = 0;

	m_nFrameType                = 0;
	m_dwFrameKey                = 0;
	m_pRUIBufferListSend        = NULL;

#ifdef WRITE_ENCODED_FILE
	m_bFile = FALSE;
#endif
}

RUIBufferAudioEncoder::~RUIBufferAudioEncoder()
{
}

BOOL RUIBufferAudioEncoder::Encode(RUIBufferList* pRUIBufferListPCM, size_t nAudioSliceLen)
{
	if (pRUIBufferListPCM == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_dwLastReadVideoSourceTick = GetTickCount();
	m_pRUIBufferListPCM         = pRUIBufferListPCM;
	m_dwEncodedSize             = 0;

	if (ippStaticInit() < ippStsNoErr)
	{
		ASSERT(FALSE);
		return FALSE;
	}

//	pCodec         = new AACEncoder();
	pCodec         = new AACEncoderInt();
	pEncoderParams = new AACEncoderParams;

	pEncoderParams->m_info_in.stream_type      = PCM_AUDIO;
	pEncoderParams->m_info_in.channels         = 1;
	pEncoderParams->m_info_in.sample_frequency = 16000;
	pEncoderParams->m_info_in.bitPerSample     = 16;
//	pEncoderParams->m_info_in.bitPerSample     = 8;
	//pEncoderParams->m_info_out.bitrate         = 40000;
	pEncoderParams->m_info_out.bitrate         = 16000; // set AAC codec quality to 16k
	pEncoderParams->m_info_out.stream_type     = AAC_AUDIO;
	pEncoderParams->audioObjectType            = AOT_AAC_LC;
	pEncoderParams->stereo_mode                = UMC_AAC_MONO;
	pEncoderParams->ns_mode                    = 0;
	pEncoderParams->outputFormat               = UMC_AAC_ADTS;

	pEncoderParams->numThreads = 1;

	if (pCodec->Init   (pEncoderParams) == UMC_OK &&
		pCodec->GetInfo(pEncoderParams) == UMC_OK)
	{
		Run();
	}

	delete pCodec;
	delete pEncoderParams;

	return 0;
}

Status RUIBufferAudioEncoder::PutOutputData(MediaData* out)
{
	if (! out) // at EOF
		return UMC_ERR_NULL_PTR;

	return IPPAudioEncoder::PutOutputData(out);
}

Ipp64f RUIBufferAudioEncoder::GetTick()
{
	return (Ipp64f) vm_time_get_tick() * tickDuration;
}

int RUIBufferAudioEncoder::ReadAudioSource(BYTE* pBuffer, DWORD dwOffset, int nItemSize, int nItemCount)
{
	if (m_pRUIBufferListPCM == NULL)
	{
		ASSERT(FALSE);
		return 0;
	}

	if (m_bStop) // [neuromos] 외부에서 중지시킴
		return 0;

	int	nReadSize = nItemSize * nItemCount;

	RUIBuffer*	pRUIBufferPCM;
	
	while ((pRUIBufferPCM = m_pRUIBufferListPCM->PopHead()) == NULL)
	{
		Sleep(10);

		if (m_bStop) // [neuromos] 외부에서 중지시킴
			return 0;
	}

//	m_dwEncoderTickCurr = pRUIBufferPCM->GetParam();

	DWORD	dwBufferSize = pRUIBufferPCM->GetBufferSize();

	if (dwBufferSize != nReadSize)
		ASSERT(FALSE);

	DWORD	dwCopySize;

	dwCopySize = pRUIBufferPCM->CopyTo(pBuffer, 0, dwBufferSize);

	if (dwCopySize != (DWORD) nReadSize)
		ASSERT(FALSE);

	delete pRUIBufferPCM;

	return nItemCount;
}

int RUIBufferAudioEncoder::WriteVideoTarget(BYTE* pBuffer, int nItemSize, int nItemCount, FrameType frameType)
{
	IPPAudioEncoder::WriteVideoTarget(pBuffer, nItemSize, nItemCount, frameType);

	if (m_bStop) // [neuromos] 외부에서 중지시킴
		return 0;

	int	nWriteSize = nItemSize * nItemCount;

	if (m_pRUIBufferListSend != NULL)
	{
		m_pRUIBufferListSend->AddUDPFrame(m_nFrameType, (BYTE) frameType, m_dwFrameKey, (DWORD) nWriteSize, m_dwEncoderTickCurr, (BYTE*) pBuffer, (DWORD) nWriteSize, 0);

		m_dwFrameKey++;
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
