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

#ifndef _RUIBUFFERAUDIOENCODERTHREAD_H
#define _RUIBUFFERAUDIOENCODERTHREAD_H

#include "RUIThread.h"
#include "RUIBufferList.h"
#include "RUIBufferAudioEncoder.h"

#define WRITE_CAPTURE_FILE

class RUIBufferAudioEncoderThread : public RUIThread
{
public:
	RUIBufferAudioEncoderThread();
	virtual ~RUIBufferAudioEncoderThread();

private:
	IppExtensionParam*			m_pieParam;
	CWnd*						m_pMsgWnd;

	RUIBufferList*				m_pRUIBufferListPCM;
	size_t						m_nAudioSliceLen;
	RUIBufferAudioEncoder		m_encoder;

public:
	IppExtensionParam*			GetIppExtensionParam()                            { return m_pieParam;                                               }
	void						SetIppExtensionParam(IppExtensionParam* pieParam) { m_pieParam = pieParam; m_encoder.SetIppExtensionParam(pieParam); }

	CWnd*						GetMessageWindow()               { return m_pMsgWnd;                                         }
	void						SetMessageWindow(CWnd* pMsgWnd)  { m_pMsgWnd = pMsgWnd; m_encoder.SetMessageWindow(pMsgWnd); }

	size_t						GetAudioSliceLen()               { return m_nAudioSliceLen;                                   }
	void						SetAudioSliceLen(size_t nLen)    { m_nAudioSliceLen = nLen; m_encoder.SetAudioSliceLen(nLen); }

	RUIBufferAudioEncoder*		GetEncoder()                  { return &m_encoder;                   }

	DWORD						GetFramesEncoded()            { return m_encoder.GetFramesEncoded(); }
	DWORD						GetEncodedSize()              { return m_encoder.GetEncodedSize();   } // ���ݱ��� ���ڵ��� ����� ũ�� ��

	BYTE						GetFrameType()                { return m_encoder.GetFrameType();     }
	void						SetFrameType(BYTE nFrameType) { m_encoder.SetFrameType(nFrameType);  }
	void						SetRUIBufferList(RUIBufferList* pRUIBufferListPCM);

	// Socket
	RUIUDPFrameList*			GetRUIBufferSend()                                  { return m_encoder.GetRUIBufferSend();          }
	void						SetRUIBufferSend(RUIUDPFrameList* pRUIUDPFrameList) { m_encoder.SetRUIBufferSend(pRUIUDPFrameList); }

public:
	virtual BOOL				Stop(BOOL bSync = TRUE);
	virtual BOOL				OnStart();
	virtual void				OnStop();
	virtual BOOL				ThreadProc();
};

#endif
