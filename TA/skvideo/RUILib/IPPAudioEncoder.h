#ifndef _IPPAUDIOENCODER_H
#define _IPPAUDIOENCODER_H

#include <ipps.h>
#include "../h264/common/umc/umc_defs.h"
#include "../h264/common/codec/umc_aac_encoder_params.h"
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



#include "../h264/encoder/aac_enc/umc_aac_encoder.h"
#include "../h264/encoder/aac_enc_int/umc_aac_encoder_int.h"


namespace UMC
{
	class IPPAudioEncoder
	{
	public:
		IPPAudioEncoder();

	public:
		IppExtensionParam*		m_pieParam;
		CWnd*					m_pMsgWnd;
		size_t					m_nAudioSliceLen;

		// BaseFileTransform
		BaseCodec*				pCodec;

		// Params
		AACEncoderParams*		pEncoderParams;

		// Info
		Ipp32s					mFramesEncoded;
		Ipp32s					encodedSize;

		bool					m_bPlaying;
		DWORD					m_dwEncoderTickCurr;

	public:
		IppExtensionParam*		GetIppExtensionParam()                            { return m_pieParam;     }
		void					SetIppExtensionParam(IppExtensionParam* pieParam) { m_pieParam = pieParam; }

		CWnd*					GetMessageWindow()               { return m_pMsgWnd;    }
		void					SetMessageWindow(CWnd* pMsgWnd)  { m_pMsgWnd = pMsgWnd; }

		size_t					GetAudioSliceLen()               { return m_nAudioSliceLen; }
		void					SetAudioSliceLen(size_t nLen)    { m_nAudioSliceLen = nLen; }

		DWORD					GetFramesEncoded() { return (DWORD) mFramesEncoded; }
		Status					ReadAudioPlane(AudioData* pAudioData, int nPlane);
		bool					IsPlaying()        { return m_bPlaying;             }

	public:
		virtual Status			GetInputData (MediaData* in );
		virtual Status			PutOutputData(MediaData* out);
		virtual Status			Run();
		virtual Ipp64f			GetTick() { return 0; }

		virtual int				ReadAudioSource (BYTE* pBuffer, DWORD dwOffset, int nItemSize, int nItemCount) = 0;
		virtual int				WriteVideoTarget(BYTE* pBuffer, int nItemSize, int nItemCount, FrameType frameType) = 0;
	};
}

#endif
