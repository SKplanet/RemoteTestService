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

#ifndef _RUIBUFFERVIDEOENCODERTHREAD_H
#define _RUIBUFFERVIDEOENCODERTHREAD_H

#include "RUIThread.h"
#include "RUIBuffer.h"
#include "RUIBufferList.h"
#include "RUIBufferVideoEncoder.h"

#define WRITE_CAPTURE_FILE

class RUIBufferVideoEncoderThread : public RUIThread
{
public:
	RUIBufferVideoEncoderThread();
	virtual ~RUIBufferVideoEncoderThread();

private:
	IppExtensionParam*			m_pieParam;
	CWnd*						m_pMsgWnd;

	RUIBuffer*					m_pRUIBufferRGB;
	RUIBufferList*				m_pRUIBufferList264;
	int							m_nWidth;
	int							m_nHeight;
	DWORD						m_dwBitRate;
	RUIBufferVideoEncoder		m_encoder;

public:
	IppExtensionParam*			GetIppExtensionParam()                            { return m_pieParam;                                               }
	void						SetIppExtensionParam(IppExtensionParam* pieParam) { m_pieParam = pieParam; m_encoder.SetIppExtensionParam(pieParam); }

	CWnd*						GetMessageWindow()               { return m_pMsgWnd;                                         }
	void						SetMessageWindow(CWnd* pMsgWnd)  { m_pMsgWnd = pMsgWnd; m_encoder.SetMessageWindow(pMsgWnd); }

	int							GetWidth()                    { return m_nWidth;                     }
	void						SetWidth(int nWidth)          { m_nWidth  = nWidth;                  }
	int							GetHeight()                   { return m_nHeight;                    }
	void						SetHeight(int nHeight)        { m_nHeight = nHeight;                 }
	DWORD						GetBitRate()                  { return m_dwBitRate;                  }
	void						SetBitRate(DWORD dwBitRate)   { m_dwBitRate = dwBitRate;             }
	BOOL						SetBitRateRuntime(DWORD dwBitRate);
	RUIBufferVideoEncoder*		GetEncoder()                  { return &m_encoder;                   }

	DWORD						GetFramesEncoded()            { return m_encoder.GetFramesEncoded(); }
	DWORD						GetEncodedSize()              { return m_encoder.GetEncodedSize();   } // 지금까지 인코딩된 결과의 크기 합

	BYTE						GetFrameType()                { return m_encoder.GetFrameType();     }
	void						SetFrameType(BYTE nFrameType) { m_encoder.SetFrameType(nFrameType);  }
	void						SetRUIBuffer(RUIBuffer* pRUIBufferRGB, RUIBufferList* pRUIBufferList264);

	// [neuromos] [checkpoint]
	DWORD						GetEncoderTickAvg()           { return m_encoder.GetEncoderTickAvg(); }
	DWORD						GetEncoderTickMax()           { return m_encoder.GetEncoderTickMax(); }

	// Socket
	BOOL						GetUseSocket()                                      { return m_encoder.GetUseSocket();              }
	void						SetUseSocket(BOOL bUseSocket)                       { m_encoder.SetUseSocket(bUseSocket);           }
	RUIUDPFrameList*			GetRUIBufferSend()                                  { return m_encoder.GetRUIBufferSend();          }
	void						SetRUIBufferSend(RUIUDPFrameList* pRUIUDPFrameList) { m_encoder.SetRUIBufferSend(pRUIUDPFrameList); }

public:
	virtual BOOL				Stop(BOOL bSync = TRUE);
	virtual BOOL				OnStart();
	virtual void				OnStop();
	virtual BOOL				ThreadProc();
};

#endif
