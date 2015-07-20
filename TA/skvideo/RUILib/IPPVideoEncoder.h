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

#ifndef _IPPVIDEOENCODER_H
#define _IPPVIDEOENCODER_H

#include <ipps.h>
#include "../h264/common/umc/umc_defs.h"
#include "../h264/decoder/umc_pipeline/umc_video_encoders.h"

namespace UMC
{
	class IPPVideoEncoder
	{
	public:
		IPPVideoEncoder();

	public:
		IppExtensionParam*		m_pieParam;
		CWnd*					m_pMsgWnd;

		// BaseFileTransform
		BaseCodec*				pCodec;

		// Params
		VideoEncoderParams*		pEncoderParams;
		ColorFormat				mColorFormat;
		Ipp32s					mBitDepth;
		Ipp32s					mBitDepthAlpha;
		ColorFormat				mSrcFileColorFormat;
		Ipp32s					numFramesToEncode;

		// Info
		Ipp32s					mFramesEncoded;
		Ipp32s					mFrameSize;
		Ipp32s					encodedSize;

		bool					m_bPlaying;
		DWORD					m_dwEncoderTickSum;
		DWORD					m_dwEncoderTickCount;
		DWORD					m_dwEncoderTickMax;
		DWORD					m_dwEncoderTickCurr;

		DWORD					m_dwBitRateStart;
		DWORD					m_dwBitRateModified;
		int						m_nStableBitRateCount;

		DWORD					m_dwFrameTimeStart;
		DWORD					m_dwFrameTimeModified;
		int						m_nStableFrameTimeCount;

	public:
		IppExtensionParam*		GetIppExtensionParam()                            { return m_pieParam;     }
		void					SetIppExtensionParam(IppExtensionParam* pieParam) { m_pieParam = pieParam; }

		CWnd*					GetMessageWindow()               { return m_pMsgWnd;    }
		void					SetMessageWindow(CWnd* pMsgWnd)  { m_pMsgWnd = pMsgWnd; }

		DWORD					GetFramesEncoded() { return (DWORD) mFramesEncoded; }
		Status					ReadVideoPlane(VideoData* pVideoData, int nPlane);
		bool					IsPlaying()        { return m_bPlaying;             }
		DWORD					GetEncoderTickAvg();
		DWORD					GetEncoderTickMax();

		DWORD					GetBitRateStart()                       { return m_dwBitRateStart;             }
		void					SetBitRateStart(DWORD dwBitRate)        { m_dwBitRateStart = dwBitRate;        }
		DWORD					GetBitRateModified()                    { return m_dwBitRateModified;          }
		void					SetBitRateModified(DWORD dwBitRate)     { m_dwBitRateModified = dwBitRate;     }
		DWORD					DecreaseBitRate();
		DWORD					RecoverBitRate();

		DWORD					GetFrameTimeStart()                     { return m_dwFrameTimeStart;           }
		void					SetFrameTimeStart(DWORD dwFrameTime)    { m_dwFrameTimeStart = dwFrameTime;    }
		DWORD					GetFrameTimeModified()                  { return m_dwFrameTimeModified;        }
		void					SetFrameTimeModified(DWORD dwFrameTime) { m_dwFrameTimeModified = dwFrameTime; }
		DWORD					IncreaseFrameTime();
		DWORD					RecoverFrameTime();

	public:
		virtual Status			GetInputData (MediaData* in );
		virtual Status			PutOutputData(MediaData* out);
		virtual Status			Run();
		virtual Ipp64f			GetTick() { return 0; }

		virtual int				ReadVideoSource (BYTE* pBuffer, DWORD dwOffset, int nItemSize, int nItemCount) = 0;
		virtual int				WriteVideoTarget(BYTE* pBuffer, int nItemSize, int nItemCount, FrameType frameType) = 0;
	};
}

#endif
