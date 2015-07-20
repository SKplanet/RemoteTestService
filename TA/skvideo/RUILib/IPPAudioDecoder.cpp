#include "stdafx.h"
#include "IPPAudioDecoder.h"
#include "../h264/common/umc/umc_video_data.h"
#include "../Common/RUIDef.h"

using namespace UMC;

IPPAudioDecoder::IPPAudioDecoder()
{
	m_pieParam          = NULL;
	m_pMsgWnd           = NULL;
	m_nAudioSliceLen    = 0;

	pCodec              = NULL;
#if 0 // [neuromos] To FileVideoEncoder
	srcFile             = NULL;
	dstFile             = NULL;
#endif
	pDecoderParams      = NULL;

	mFramesDecoded      = 0;
	decodedSize         = 0;

	m_bPlaying          = false; // [neuromos] 현재 Encoder Loop에 있는지 확인할 수 있는 플래그.
}

Status IPPAudioDecoder::ReadAudioPlane(AudioData* pAudioData, int nPlane)
{
	Ipp8u*	pAudioDataPtr;
	DWORD	dwOffset = 0;

	pAudioDataPtr = (Ipp8u*) pAudioData->GetDataPointer();

	int	nRead = (Ipp32s) ReadAudioSource((BYTE*) pAudioDataPtr, dwOffset, 1, m_nAudioSliceLen);

	if (nRead == 0)
		return UMC_ERR_FAILED;

	pAudioData->SetDataSize(nRead);

	return UMC_OK;
}

Status IPPAudioDecoder::GetInputData(MediaData* in)
{
	Status		ret        = UMC_OK;
	AudioData*	pAudioData = DynamicCast<AudioData>(in);

	ret = ReadAudioPlane(pAudioData, 0);

	return ret;
}

Status IPPAudioDecoder::PutOutputData(MediaData* out)
{
	size_t	nWrite;

//	nWrite = vm_file_fwrite(out->GetDataPointer(), 1, (Ipp32s) out->GetDataSize(), dstFile);
	nWrite = (size_t) WriteAudioTarget((BYTE*) out->GetDataPointer(), 1, (int) (Ipp32s) out->GetDataSize(), out->GetFrameType());

	if (nWrite != out->GetDataSize())
		return UMC_ERR_FAILED;

	return out->SetDataSize(0);
}

Status IPPAudioDecoder::Run()
{
	m_bPlaying = true; // [neuromos] 현재 Encoder Loop에 있는지 확인할 수 있는 플래그.

	if (m_nAudioSliceLen == 0)
		ASSERT(FALSE);

	AudioData	in;
	AudioData	out; out.Alloc(m_nAudioSliceLen);
	AudioData*	p_dataIn  = &in;
	AudioData*	p_dataOut = &out;
	Status		ret = UMC_OK;
	Ipp32s		len;

//	p_dataIn->Init();

	p_dataIn->Alloc(m_nAudioSliceLen);

	decodedSize = 0;

	mFramesDecoded = 0;

	for (;;)
	{
		if (p_dataIn)
		{
			if (UMC_OK != GetInputData(p_dataIn))
				break;
//			if (UMC_OK == GetInputData(p_dataIn))
//				p_dataIn->SetTime((mFramesEncoded+1) / pEncoderParams->info.framerate);
//			else
//				p_dataIn = NULL;
		}

		int	nDataInSize = p_dataIn->GetDataSize();
		ret = pCodec->GetFrame(p_dataIn, p_dataOut);
		p_dataIn->MoveDataPointer(-nDataInSize);

		if (ret != UMC_OK && ret != UMC_ERR_NOT_ENOUGH_DATA && ret != UMC_ERR_END_OF_STREAM)
		{
			m_bPlaying = false; // [neuromos] 현재 Encoder Loop에 있는지 확인할 수 있는 플래그.
			return UMC_ERR_FAILED;
		}

		len = (Ipp32s) p_dataOut->GetDataSize();
		decodedSize += len;

		if (UMC_OK != PutOutputData(p_dataOut))
		{
			m_bPlaying = false; // [neuromos] 현재 Encoder Loop에 있는지 확인할 수 있는 플래그.
			return UMC_ERR_FAILED;
		}

		if (p_dataIn)
		{
			mFramesDecoded++;
		}
		else
		{
			if (! len || ret == UMC_ERR_END_OF_STREAM) break; // EOF
		}
	}

	PutOutputData(NULL); // means EOF

	pCodec->Close();

	m_bPlaying = false; // [neuromos] 현재 Encoder Loop에 있는지 확인할 수 있는 플래그.

	return UMC_OK;
}

int IPPAudioDecoder::WriteAudioTarget(BYTE* pBuffer, int nItemSize, int nItemCount, FrameType frameType)
{
	return 0;
}
