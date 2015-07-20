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

#ifndef _RUIBUFFERVIDEOENCODER_H
#define _RUIBUFFERVIDEOENCODER_H

#include "../Common/RUIDef.h"
#include "IPPVideoEncoder.h"
#include "../h264/common/vm_plus/umc_sys_info.h"
#include "RUIBuffer.h"
#include "RUIBufferList.h"
#include "RUIUDPFrameList.h"

using namespace UMC;

//#define WRITE_ENCODED_FILE

class RUIBufferVideoEncoder : public IPPVideoEncoder
{
public:
	RUIBufferVideoEncoder();
	~RUIBufferVideoEncoder();

private:
	BOOL					m_bStop;
	Ipp64f					tickDuration;
	DWORD					m_dwLastReadVideoSourceTick;
	RUIBuffer*				m_pRUIBufferRGB;
	RUIBufferList*			m_pRUIBufferList264;
	DWORD					m_dwEncodedSize; // 지금까지 인코딩된 결과의 크기 합

	// Socket
	BOOL					m_bUseSocket;
	BYTE					m_nFrameType;
	DWORD					m_dwFrameKey;
	RUIUDPFrameList*		m_pRUIBufferListSend;

#ifdef WRITE_ENCODED_FILE
	BOOL					m_bFile;
	CFile					m_fileEncoded;
#endif

	BOOL					m_bFirstFrame;

public:
	BOOL					GetStop()           { return m_bStop;         }
	void					SetStop(BOOL bStop) { m_bStop = bStop;        }
	DWORD					GetEncodedSize()    { return m_dwEncodedSize; }

	// Socket
	BOOL					GetUseSocket()                                      { return m_bUseSocket;                     }
	void					SetUseSocket(BOOL bUseSocket)                       { m_bUseSocket = bUseSocket;               }
	BYTE					GetFrameType()                                      { return m_nFrameType;                     }
	void					SetFrameType(BYTE nFrameType)                       { m_nFrameType = nFrameType;               }
	RUIUDPFrameList*		GetRUIBufferSend()                                  { return m_pRUIBufferListSend;             }
	void					SetRUIBufferSend(RUIUDPFrameList* pRUIUDPFrameList) { m_pRUIBufferListSend = pRUIUDPFrameList; }

	Ipp32s					Encode(RUIBuffer* pRUIBufferRGB, RUIBufferList* pRUIBufferList264, int nWidth, int nHeight, DWORD dwBitRate);

#ifdef WRITE_ENCODED_FILE
	BOOL					GetFileEnable()           { return m_bFile;  }
	void					SetFileEnable(BOOL bFile) { m_bFile = bFile; }
	CFile*					GetFile()                 { return &m_fileEncoded; }
#endif

public:
	virtual Status			PutOutputData(MediaData* out);
	virtual Ipp64f			GetTick();

	virtual int				ReadVideoSource (BYTE* pBuffer, DWORD dwOffset, int nItemSize, int nItemCount);
	virtual int				WriteVideoTarget(BYTE* pBuffer, int nItemSize, int nItemCount, FrameType frameType);
};

#endif
