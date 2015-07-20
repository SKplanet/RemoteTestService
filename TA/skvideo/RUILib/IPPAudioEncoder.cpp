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
#include "IPPAudioEncoder.h"
#include "../h264/common/umc/umc_video_data.h"
#include "../Common/RUIDef.h"

using namespace UMC;

IPPAudioEncoder::IPPAudioEncoder()
{
	m_pieParam          = NULL;
	m_pMsgWnd           = NULL;
	m_nAudioSliceLen    = 0;

	pCodec              = NULL;
#if 0 // [neuromos] To FileVideoEncoder
	srcFile             = NULL;
	dstFile             = NULL;
#endif
	pEncoderParams      = NULL;

	mFramesEncoded      = 0;
	encodedSize         = 0;

	m_bPlaying          = false; // [neuromos] 현재 Encoder Loop에 있는지 확인할 수 있는 플래그.
}

Status IPPAudioEncoder::ReadAudioPlane(AudioData* pAudioData, int nPlane)
{
	Ipp8u*	pAudioDataPtr;
	DWORD	dwOffset = 0;

	pAudioDataPtr = (Ipp8u*) pAudioData->GetDataPointer();

	int	nRead = (Ipp32s) ReadAudioSource((BYTE*) pAudioDataPtr, dwOffset, 1, m_nAudioSliceLen);

	if (nRead != m_nAudioSliceLen)
		return UMC_ERR_FAILED;

	pAudioData->SetDataSize(nRead);

	return UMC_OK;
}

Status IPPAudioEncoder::GetInputData(MediaData* in)
{
	Status		ret        = UMC_OK;
	AudioData*	pAudioData = DynamicCast<AudioData>(in);

	ret = ReadAudioPlane(pAudioData, 0);

	return ret;
}

Status IPPAudioEncoder::PutOutputData(MediaData* out)
{
	size_t	nWrite;

//	nWrite = vm_file_fwrite(out->GetDataPointer(), 1, (Ipp32s) out->GetDataSize(), dstFile);
	nWrite = (size_t) WriteVideoTarget((BYTE*) out->GetDataPointer(), 1, (int) (Ipp32s) out->GetDataSize(), out->GetFrameType());

	if (nWrite != out->GetDataSize())
		return UMC_ERR_FAILED;

	return out->SetDataSize(0);
}

Status IPPAudioEncoder::Run()
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

	encodedSize = 0;

	mFramesEncoded = 0;

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
		encodedSize += len;

		m_dwEncoderTickCurr = GetTickCount();

		if (len > 0)
		{
			if (UMC_OK != PutOutputData(p_dataOut))
			{
				m_bPlaying = false; // [neuromos] 현재 Encoder Loop에 있는지 확인할 수 있는 플래그.
				return UMC_ERR_FAILED;
			}
		}

		if (p_dataIn)
		{
			mFramesEncoded++;
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

int IPPAudioEncoder::WriteVideoTarget(BYTE* pBuffer, int nItemSize, int nItemCount, FrameType frameType)
{
	return 0;
}
