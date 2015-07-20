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
#include "IPPVideoEncoder.h"
#include "../h264/common/umc/umc_video_data.h"
#include "../Common/RUIDef.h"

using namespace UMC;

IPPVideoEncoder::IPPVideoEncoder()
{
	m_pieParam          = NULL;
	m_pMsgWnd           = NULL;

	pCodec              = NULL;
#if 0 // [neuromos] To FileVideoEncoder
	srcFile             = NULL;
	dstFile             = NULL;
#endif
	pEncoderParams      = NULL;

//	mColorFormat        = YUV420;
//	mBitDepth           = 8; // Can be overwritten according to H.264 parameters.
	// [neuromos] Test
	mColorFormat        = RGB32;
	mBitDepth           = 32;

	mBitDepthAlpha      = 16;
	mSrcFileColorFormat = NONE;
	numFramesToEncode   = 0x7fffffff;

	mFramesEncoded      = 0;
	mFrameSize          = 0;
	encodedSize         = 0;

	m_bPlaying          = false; // [neuromos] 현재 Encoder Loop에 있는지 확인할 수 있는 플래그.

	m_dwEncoderTickMax  = 0;

	m_dwBitRateStart        = 0;
	m_dwBitRateModified     = 0;
	m_nStableBitRateCount   = 0;

	m_dwFrameTimeStart      = 0;
	m_dwFrameTimeModified   = 0;
	m_nStableFrameTimeCount = 0;
}

Status IPPVideoEncoder::ReadVideoPlane(VideoData* pVideoData, int nPlane)
{
	if (pVideoData->GetColorFormat() == YV12)
	{
		// for YV12 change planes to Y,U,V order
		if (nPlane == 1) nPlane = 2;
		if (nPlane == 2) nPlane = 1;
	}

	Ipp8u*					pPlanePtr;
	VideoData::PlaneInfo	planeInfo;

	pPlanePtr = (Ipp8u*) pVideoData->GetPlanePointer(nPlane);
	pVideoData->GetPlaneInfo(&planeInfo, nPlane);

	int	nHeight = planeInfo.m_ippSize.height;
	int	nWidth  = planeInfo.m_ippSize.width;

	// [neuromos] RUIBufferVideoEncoder::m_pRUIBufferListRGB의 Last RUIUseBuffer의 시작 Offset을 지정하도록 수정.
	DWORD	dwOffset = 0;
	for (int i = 0; i < nHeight; i ++)
	{
//		int	nRead = (Ipp32s) vm_file_fread(pPlanePtr, planeInfo.m_iSampleSize, nWidth, pFile);
		int	nRead = (Ipp32s) ReadVideoSource((BYTE*) pPlanePtr, dwOffset, (int) planeInfo.m_iSampleSize, (int) nWidth);

		dwOffset += (DWORD) (planeInfo.m_iSampleSize * nRead);

		pPlanePtr += planeInfo.m_nPitch;

		if (nRead != nWidth)
			return UMC_ERR_FAILED;
	}

	return UMC_OK;
}

DWORD IPPVideoEncoder::GetEncoderTickAvg()
{
	if (m_dwEncoderTickCount > 0)
		return m_dwEncoderTickSum / m_dwEncoderTickCount;

	return 0;
}

DWORD IPPVideoEncoder::GetEncoderTickMax()
{
	return m_dwEncoderTickMax;
}

DWORD IPPVideoEncoder::DecreaseBitRate()
{
	if (pCodec != NULL)
	{
		Status				st;
		VideoEncoderParams	newpar;

		st = pCodec->GetInfo(&newpar);

		if (st == UMC_OK)
		{
			m_dwBitRateModified = newpar.info.bitrate;

			DWORD	dwBitRateNew = m_dwBitRateModified * 8 / 10;

			newpar.info.bitrate = dwBitRateNew;
			st = pCodec->SetParams(&newpar);

			if (st == UMC_OK)
				m_dwBitRateModified = dwBitRateNew;
		}
	}

	return m_dwBitRateModified;
}

DWORD IPPVideoEncoder::RecoverBitRate()
{
	if (m_dwBitRateModified < m_dwBitRateStart)
	{
		m_nStableBitRateCount++;

		if (m_nStableBitRateCount >= 150)
		{
			DWORD				dwBitRateIncrease = (m_dwBitRateStart - m_dwBitRateModified) / 1000;
			if (dwBitRateIncrease == 0)
				dwBitRateIncrease = 1;

			Status				st;
			VideoEncoderParams	newpar;

			st = pCodec->GetInfo(&newpar);

			if (st == UMC_OK)
			{
				m_dwBitRateModified = newpar.info.bitrate;

				DWORD	dwBitRateNew = m_dwBitRateModified + dwBitRateIncrease;

				newpar.info.bitrate = dwBitRateNew;
				st = pCodec->SetParams(&newpar);

				if (st == UMC_OK)
					m_dwBitRateModified = dwBitRateNew;
			}

			m_nStableBitRateCount = 0;
		}
	}

	return m_dwBitRateModified;
}

DWORD IPPVideoEncoder::IncreaseFrameTime()
{
	DWORD	dwFrameTimeNew = m_dwFrameTimeModified * 12 / 10;
	m_dwFrameTimeModified  = dwFrameTimeNew;

	return m_dwFrameTimeModified;
}

DWORD IPPVideoEncoder::RecoverFrameTime()
{
	if (m_dwFrameTimeModified > m_dwFrameTimeStart)
	{
		m_nStableFrameTimeCount++;

		if (m_nStableFrameTimeCount >= 150)
		{
			DWORD				dwFrameTimeDecrease = (m_dwFrameTimeModified - m_dwFrameTimeStart) / 20;
			if (dwFrameTimeDecrease == 0)
				dwFrameTimeDecrease = 1;

			DWORD	dwFrameTimeNew = m_dwFrameTimeModified - dwFrameTimeDecrease;
			m_dwFrameTimeModified  = dwFrameTimeNew;

			m_nStableFrameTimeCount = 0;
		}
	}

	return m_dwFrameTimeModified;
}

Status IPPVideoEncoder::GetInputData(MediaData* in)
{
	Status		ret        = UMC_OK;
	VideoData*	pVideoData = DynamicCast<VideoData>(in);
	int			nNumPlanes = pVideoData->GetNumPlanes();
	int			bAlphaFlag = 0;

	if (mColorFormat == YUV420A || mColorFormat == YUV422A || mColorFormat == YUV444A)
	{
		nNumPlanes = 3;
		bAlphaFlag = 1;
	}

	for (int k = 0; k < nNumPlanes; k++)
	{
		int kk = k;

		if (mSrcFileColorFormat == YV12 && pVideoData->GetColorFormat() == YUV420)
		{
			if      (k == 1) kk = 2;
			else if (k == 2) kk = 1;
		}

		ret = ReadVideoPlane(pVideoData, kk);

		if (UMC_OK != ret)
		{
//			ASSERT(FALSE);
			// [neuromos] 다 읽었을 때 이곳에 들어온다.
			break;
		}
	}

	return ret;
}

Status IPPVideoEncoder::PutOutputData(MediaData* out)
{
	size_t	nWrite;

//	nWrite = vm_file_fwrite(out->GetDataPointer(), 1, (Ipp32s) out->GetDataSize(), dstFile);
	nWrite = (size_t) WriteVideoTarget((BYTE*) out->GetDataPointer(), 1, (int) (Ipp32s) out->GetDataSize(), out->GetFrameType());

	if (nWrite != out->GetDataSize())
		return UMC_ERR_FAILED;

	return out->SetDataSize(0);
}

Status IPPVideoEncoder::Run()
{
	m_bPlaying = true; // [neuromos] 현재 Encoder Loop에 있는지 확인할 수 있는 플래그.

	Ipp32s		mWidth  = pEncoderParams->info.clip_info.width;
	Ipp32s		mHeight = pEncoderParams->info.clip_info.height;
	VideoData	in;
	MediaData	out(4 * mWidth * mHeight);
	VideoData*	p_dataIn  = &in;
	MediaData*	p_dataOut = &out;
	Ipp32s		maxNumFrames = numFramesToEncode;
	Status		ret = UMC_OK;
	Ipp32s		len;
	Ipp64f		t_start;

	p_dataIn->Init(mWidth, mHeight, mColorFormat, mBitDepth);
	if (mColorFormat == YUV420A || mColorFormat == YUV422A || mColorFormat == YUV444A)
		p_dataIn->SetPlaneBitDepth(mBitDepthAlpha, 3);
	mFrameSize = (Ipp32s) p_dataIn->GetMappingSize();

	p_dataIn->Alloc(); // if mpeg2 use internal buffer of encoder

	encodedSize = 0;

	mFramesEncoded = 0;

	m_dwEncoderTickSum   = 0;
	m_dwEncoderTickCount = 0;
	m_dwEncoderTickMax   = 0;
	DWORD	dwEncoderTickStart;

	for (;;)
	{
		m_dwEncoderTickCurr = GetTickCount();

		if (mFramesEncoded >= maxNumFrames)
			p_dataIn = NULL; // get frames buffered inside encoder
		if (p_dataIn)
		{
			if (UMC_OK == GetInputData(p_dataIn))
				p_dataIn->SetTime((mFramesEncoded+1)/pEncoderParams->info.framerate);
			else
				p_dataIn = NULL;
		}

		// [neuromos] [checkpoint]
		dwEncoderTickStart = GetTickCount();

		t_start = GetTick();
		ret = pCodec->GetFrame(p_dataIn, p_dataOut);

		if (ret != UMC_OK && ret != UMC_ERR_NOT_ENOUGH_DATA && ret != UMC_ERR_END_OF_STREAM)
		{
			vm_file_fprintf(vm_stderr, VM_STRING("Error: encoding failed at %d source frame (exit with %d)!\n"), mFramesEncoded, ret);
			m_bPlaying = false; // [neuromos] 현재 Encoder Loop에 있는지 확인할 수 있는 플래그.
			return UMC_ERR_FAILED;
		}

		len = (Ipp32s)p_dataOut->GetDataSize();
		encodedSize += len;

		if (UMC_OK != PutOutputData(p_dataOut))
		{
			vm_file_fprintf(vm_stderr, VM_STRING("Error: Couldn't write output\n"));
			m_bPlaying = false; // [neuromos] 현재 Encoder Loop에 있는지 확인할 수 있는 플래그.
			return UMC_ERR_FAILED;
		}

		if (p_dataIn)
		{
			mFramesEncoded++;
		}
		else
		{
			if (! len || ret == UMC_ERR_END_OF_STREAM) break; // EOF
		}

		// [neuromos] [checkpoint]
		DWORD	dwCurrTick = GetTickCount();
		DWORD	dwEncoderTick;

		if (dwCurrTick > dwEncoderTickStart) dwEncoderTick = dwCurrTick - dwEncoderTickStart;
		else                                 dwEncoderTick = 0;

		m_dwEncoderTickSum += dwEncoderTick;
		m_dwEncoderTickCount++;

		if (m_dwEncoderTickMax < dwEncoderTick)
			m_dwEncoderTickMax = dwEncoderTick;
	}

	PutOutputData(NULL); // means EOF

	pCodec->Close();

	m_bPlaying = false; // [neuromos] 현재 Encoder Loop에 있는지 확인할 수 있는 플래그.

	return UMC_OK;
}

int IPPVideoEncoder::WriteVideoTarget(BYTE* pBuffer, int nItemSize, int nItemCount, FrameType frameType)
{
	DWORD	dwBitRateModified   = m_dwBitRateModified;
	DWORD	dwFrameTimeModified = m_dwFrameTimeModified;

	RecoverBitRate();
	RecoverFrameTime();

	if (m_dwBitRateModified   != dwBitRateModified ||
		m_dwFrameTimeModified != dwFrameTimeModified)
	{
		if (m_pMsgWnd != NULL && ::IsWindow(m_pMsgWnd->GetSafeHwnd()))
		{
			CString	strMessage;
			strMessage.Format(_T("Inc Bitrate : %d / Dec Frametime : %d"), m_dwBitRateModified, m_dwFrameTimeModified);
			m_pMsgWnd->SendMessage(WM_LOG_MESSAGE, (WPARAM) (LPCTSTR) strMessage);
		}
	}

	return 0;
}
