#ifndef _IPPAUDIODECODER_H
#define _IPPAUDIODECODER_H

#include <ipps.h>
#include "../h264/common/umc/umc_defs.h"
#include "../h264/common/codec/umc_aac_decoder_params.h"
//#include "../h264/decoder/aac_dec/umc_aac_decoder.h"
#include "../h264/decoder/aac_dec/umc_aac_decoder.h"
#include "../h264/decoder/aac_dec_int/umc_aac_decoder_int.h"

namespace UMC
{
	class IPPAudioDecoder
	{
	public:
		IPPAudioDecoder();

	public:
		IppExtensionParam*		m_pieParam;
		CWnd*					m_pMsgWnd;
		size_t					m_nAudioSliceLen;

		// BaseFileTransform
		BaseCodec*				pCodec;

		// Params
//		AACDecoderParams*		pDecoderParams;
		AudioCodecParams*		pDecoderParams;

		// Info
		Ipp32s					mFramesDecoded;
		Ipp32s					decodedSize;

		bool					m_bPlaying;
		DWORD					m_dwEncoderTickCurr; // [neuromos] 서버에서 넘어온 Audio Encoder Tick

	public:
		IppExtensionParam*		GetIppExtensionParam()                            { return m_pieParam;     }
		void					SetIppExtensionParam(IppExtensionParam* pieParam) { m_pieParam = pieParam; }

		CWnd*					GetMessageWindow()               { return m_pMsgWnd;    }
		void					SetMessageWindow(CWnd* pMsgWnd)  { m_pMsgWnd = pMsgWnd; }

		size_t					GetAudioSliceLen()               { return m_nAudioSliceLen; }
		void					SetAudioSliceLen(size_t nLen)    { m_nAudioSliceLen = nLen; }

		DWORD					GetFramesDecoded() { return (DWORD) mFramesDecoded; }
		Status					ReadAudioPlane(AudioData* pAudioData, int nPlane);
		bool					IsPlaying()        { return m_bPlaying;             }

	public:
		virtual Status			GetInputData (MediaData* in );
		virtual Status			PutOutputData(MediaData* out);
		virtual Status			Run();
		virtual Ipp64f			GetTick() { return 0; }

		virtual int				ReadAudioSource (BYTE* pBuffer, DWORD dwOffset, int nItemSize, int nItemCount) = 0;
		virtual int				WriteAudioTarget(BYTE* pBuffer, int nItemSize, int nItemCount, FrameType frameType) = 0;
	};
}

#endif
