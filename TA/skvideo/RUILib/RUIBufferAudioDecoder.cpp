#include "stdafx.h"
#include "RUIBufferAudioDecoder.h"
#include "RUILibDef.h"

#include "../h264/common/vm/vm_time.h"
#include "../h264/common/vm/vm_strings.h"
#include "../h264/common/vm/vm_sys_info.h"

#include <ippvc.h>

RUIBufferAudioDecoder::RUIBufferAudioDecoder()
{
	m_bStop                     = FALSE;
	tickDuration                = 1.0 / (Ipp64f) vm_time_get_frequency();
	m_dwLastReadVideoSourceTick = 0;
	m_pRUIBufferListPCM         = NULL;
	m_dwDecodedSize             = 0;

	m_nFrameType                = 0;
	m_dwFrameKey                = 0;
	m_pRUIBufferListRecv        = NULL;

#ifdef WRITE_ENCODED_FILE
	m_bFileSource = FALSE;
	m_bFileTarget = FALSE;
#endif
}

RUIBufferAudioDecoder::~RUIBufferAudioDecoder()
{
}

BOOL RUIBufferAudioDecoder::Decode(RUIBufferList* pRUIBufferListPCM)
{
	if (pRUIBufferListPCM == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_dwLastReadVideoSourceTick = GetTickCount();
	m_pRUIBufferListPCM         = pRUIBufferListPCM;
	m_dwDecodedSize             = 0;

	if (ippStaticInit() < ippStsNoErr)
	{
		ASSERT(FALSE);
		return FALSE;
	}

//	pCodec         = new AACDecoder();
	pCodec         = new AACDecoderInt();
//	pDecoderParams = new AACDecoderParams;
	pDecoderParams = new AudioCodecParams;

	pDecoderParams->m_info_in.stream_type = UNDEF_AUDIO;

	pDecoderParams->numThreads = 1;

	m_pRUIBufferListRecv->SetEnableFrameType(m_nFrameType);

	if (pCodec->Init(pDecoderParams) == UMC_OK)
	{
		Run();
	}

	m_pRUIBufferListRecv->ResetEnableFrameType(m_nFrameType);
	m_pRUIBufferListRecv->RemoveUDPFrame      (m_nFrameType);

	delete pCodec;
	delete pDecoderParams;

	return 0;
}

Status RUIBufferAudioDecoder::PutOutputData(MediaData* out)
{
	if (! out) // at EOF
		return UMC_ERR_NULL_PTR;

	return IPPAudioDecoder::PutOutputData(out);
}

Ipp64f RUIBufferAudioDecoder::GetTick()
{
	return (Ipp64f) vm_time_get_tick() * tickDuration;
}

int RUIBufferAudioDecoder::ReadAudioSource(BYTE* pBuffer, DWORD dwOffset, int nItemSize, int nItemCount)
{
	if (m_pRUIBufferListPCM == NULL)
	{
		ASSERT(FALSE);
		return 0;
	}

	if (m_bStop) // [neuromos] 외부에서 중지시킴
		return 0;

	int	nReadSize = nItemSize * nItemCount;

	RUIFrameBuffer*	pRUIFrameBufferAAC;
	
	// [neuromos] 여기서 거르지 않고 RUIAudio에서 waveOutWrite()할 때 서버에서 보내준 Encoder Tick을 비교하여 처리함.
	while ((pRUIFrameBufferAAC = m_pRUIBufferListRecv->PopHeadCompleted(m_nFrameType)) == NULL)
//	while ((pRUIFrameBufferAAC = m_pRUIBufferListRecv->PopLimitHeadCompleted(m_nFrameType, m_pieParam->m_dwFrameRecvTickAudioLimit)) == NULL)
	{
		Sleep(10);

		if (m_bStop) // [neuromos] 외부에서 중지시킴
			return 0;
	}

	m_dwEncoderTickCurr = pRUIFrameBufferAAC->GetParam(); // RUITCPSockClient::OnDataParsed()를 참고하면 dwFrameTick을 dwParam에 보관함.

	DWORD	dwBufferSize = pRUIFrameBufferAAC->GetFrameSize();

//	if (dwBufferSize != nReadSize)
//		ASSERT(FALSE);

	DWORD	dwCopySize;

	dwCopySize = pRUIFrameBufferAAC->CopyTo(pBuffer, dwBufferSize);

#ifdef WRITE_ENCODED_FILE
	if (m_bFileSource)
	{
		m_fileSource.Write((const void*) pBuffer, dwCopySize);
	}
#endif

//	if (dwCopySize != (DWORD) nReadSize)
//		ASSERT(FALSE);

	delete pRUIFrameBufferAAC;

	return (int) dwCopySize;
}

int RUIBufferAudioDecoder::WriteAudioTarget(BYTE* pBuffer, int nItemSize, int nItemCount, FrameType frameType)
{
	IPPAudioDecoder::WriteAudioTarget(pBuffer, nItemSize, nItemCount, frameType);

	if (m_bStop) // [neuromos] 외부에서 중지시킴
		return 0;

	int	nWriteSize = nItemSize * nItemCount;

	if (m_pRUIBufferListPCM != NULL)
	{
		// [neuromos] 서버에서 넘어온 Audio Encoder Tick
		RUIBuffer*	pRUIBufferPCM = m_pRUIBufferListPCM->AddBuffer(pBuffer, (DWORD) nWriteSize, m_dwEncoderTickCurr);
	}

	m_dwDecodedSize += nWriteSize;

#ifdef WRITE_ENCODED_FILE
	if (m_bFileTarget)
	{
		m_fileTarget.Write((const void*) pBuffer, nWriteSize);
	}
#endif

	return nItemCount;
}
