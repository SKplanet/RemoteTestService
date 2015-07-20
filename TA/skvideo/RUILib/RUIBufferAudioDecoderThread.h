#ifndef _RUIBUFFERAUDIODECODERTHREAD_H
#define _RUIBUFFERAUDIODECODERTHREAD_H

#include "RUIThread.h"
#include "RUIBufferList.h"
#include "RUIBufferAudioDecoder.h"

#define WRITE_CAPTURE_FILE

class RUIBufferAudioDecoderThread : public RUIThread
{
public:
	RUIBufferAudioDecoderThread();
	virtual ~RUIBufferAudioDecoderThread();

private:
	IppExtensionParam*			m_pieParam;
	CWnd*						m_pMsgWnd;

	RUIBufferList*				m_pRUIBufferListPCM;
	size_t						m_nAudioSliceLen;
	RUIBufferAudioDecoder		m_decoder;

public:
	IppExtensionParam*			GetIppExtensionParam()                            { return m_pieParam;                                               }
	void						SetIppExtensionParam(IppExtensionParam* pieParam) { m_pieParam = pieParam; m_decoder.SetIppExtensionParam(pieParam); }

	CWnd*						GetMessageWindow()               { return m_pMsgWnd;                                         }
	void						SetMessageWindow(CWnd* pMsgWnd)  { m_pMsgWnd = pMsgWnd; m_decoder.SetMessageWindow(pMsgWnd); }

	size_t						GetAudioSliceLen()               { return m_nAudioSliceLen;                                   }
	void						SetAudioSliceLen(size_t nLen)    { m_nAudioSliceLen = nLen; m_decoder.SetAudioSliceLen(nLen); }

	RUIBufferAudioDecoder*		GetDecoder()                  { return &m_decoder;                   }

	DWORD						GetFramesDecoded()            { return m_decoder.GetFramesDecoded(); }
	DWORD						GetDecodedSize()              { return m_decoder.GetDecodedSize();   } // 지금까지 인코딩된 결과의 크기 합

	BYTE						GetFrameType()                { return m_decoder.GetFrameType();     }
	void						SetFrameType(BYTE nFrameType) { m_decoder.SetFrameType(nFrameType);  }
	void						SetRUIBufferList(RUIBufferList* pRUIBufferListPCM);

	// Socket
	RUIUDPFrameList*			GetRUIBufferRecv()                                  { return m_decoder.GetRUIBufferRecv();          }
	void						SetRUIBufferRecv(RUIUDPFrameList* pRUIUDPFrameList) { m_decoder.SetRUIBufferRecv(pRUIUDPFrameList); }

public:
	virtual BOOL				Stop(BOOL bSync = TRUE);
	virtual BOOL				OnStart();
	virtual void				OnStop();
	virtual BOOL				ThreadProc();
};

#endif
