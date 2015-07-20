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

#ifndef _RUIUDPFRAMELIST_H
#define _RUIUDPFRAMELIST_H

#include "RUIFrameBufferList.h"
#include "../h264/common/extension/ipp_extension.h"

#define _USE_CHECK_DROP_FRAME

#define UDP_FRAME_PACKET_SIZE		508
//#define UDP_FRAME_PACKET_SIZE		1500 // [neuromos] !이게 더 효율이 좋음. 혹시 문제 생길 수 있으니 계속 확인할 것.

#pragma pack(1)
	typedef struct tagUDPFrameHeaderFirst
	{
		BYTE	nFrameType;
		DWORD	dwFrameKey;
		USHORT	nSliceIndex;
		DWORD	dwFrameSize;
		DWORD	dwFrameTick;
	} UDPFrameHeaderFirst;

	typedef struct tagUDPFrameHeader
	{
		BYTE	nFrameType;
		DWORD	dwFrameKey;
		USHORT	nSliceIndex;
	} UDPFrameHeader;
#pragma pack()

#ifdef _USE_CHECK_DROP_FRAME
extern UINT WM_DROP_FRAME;
#endif

class RUIUDPSock;
class RUITCPSockPeer;

class RUIUDPFrameList : public RUIFrameBufferList
{
public:
	RUIUDPFrameList();
	virtual ~RUIUDPFrameList();

private:
	IppExtensionParam*		m_pieParam;

	DWORD					m_dwPacketSize;
	BOOL					m_bEnableFrameType[UDP_FRAME_TYPE_MAX + 1];
	BOOL					m_bDropOldFrame   [UDP_FRAME_TYPE_MAX + 1];
	DWORD					m_dwLastFrameKey  [UDP_FRAME_TYPE_MAX + 1]; // [neuromos] FrameType 수 만큼
	DWORD					m_dwFrameDropCount;

	__int64					m_n64FrameTypeSum [UDP_FRAME_TYPE_MAX + 1];
	DWORD					m_dwFrameKeyLast  [UDP_FRAME_TYPE_MAX + 1];

	int						m_nCongestionCount;
	RUIUDPSock*				m_pUDPSock;
	RUITCPSockPeer*			m_pTCPSock;

private:
	RUIFrameBuffer*			_GetFirst(BYTE nFrameType);
	RUIFrameBuffer*			_GetNext(BYTE nFrameType, RUIFrameBuffer* pRUIFrameBuffer);

public:
	IppExtensionParam*		GetIppExtensionParam()                            { return m_pieParam;     }
	void					SetIppExtensionParam(IppExtensionParam* pieParam) { m_pieParam = pieParam; }

	DWORD					GetPacketSize()                         { return m_dwPacketSize;           }
	void					SetPacketSize(DWORD dwPacketSize)       { m_dwPacketSize = dwPacketSize;   }

	RUIUDPSock*				GetUDPSock()                            { return m_pUDPSock;     }
	void					SetUDPSock(RUIUDPSock* pUDPSock)        { m_pUDPSock = pUDPSock; }

	RUITCPSockPeer*			GetTCPSock()                            { return m_pTCPSock;     }
	void					SetTCPSock(RUITCPSockPeer* pTCPSock)    { m_pTCPSock = pTCPSock; }

	BOOL					GetEnableFrameType  (BYTE nFrameType) { return m_bEnableFrameType[nFrameType]; }
	void					ResetAllEnableFrameType();
	void					SetEnableFrameType  (BYTE nFrameType);
	void					ResetEnableFrameType(BYTE nFrameType);
	BOOL					GetDropOldFrame     (BYTE nFrameType);
	void					SetDropOldFrame     (BYTE nFrameType, BOOL bDropOldFrame);
	__int64					GetFrameTypeSum     (BYTE nFrameType);
	__int64					AddFrameTypeSum     (BYTE nFrameType, DWORD dwSize);

	void					ResetUDPFrameList();

	DWORD					GetFrameDropCount()               { return m_dwFrameDropCount;     }

	UINT					AddUDPFrame(BYTE nFrameType, BYTE nFrameSubType, DWORD dwFrameKey, DWORD dwFrameSize, DWORD dwParam, BYTE* pBuffer, DWORD dwBufferSize, DWORD dwStart);
	UINT					RemoveUDPFrame(BYTE nFrameType);
	RUIFrameBuffer*			PopHeadCompleted(BYTE nFrameType);
	RUIFrameBuffer*			PopLimitHeadCompleted(BYTE nFrameType, DWORD dwFrameRecvTickLimit); // Audio에서 사용. dwFrameRecvTickLimit보다 큰 Tick은 처리하지 않는다.
	DWORD					GetUDPPacket(RUIFrameBuffer* pRUIFrameBuffer, USHORT nSliceIndex, BYTE* pBuffer, DWORD dwBufferSize); 
	RUIFrameBuffer*			OnReceiveUDPFrame(BYTE* pRecv, DWORD dwRecvSize);

#ifdef _USE_CHECK_DROP_FRAME
private:
	CWnd*					m_pMsgWnd;

public:
	CWnd*					GetMessageWindow()               { return m_pMsgWnd;    }
	void					SetMessageWindow(CWnd* pMsgWnd)  { m_pMsgWnd = pMsgWnd; }
#endif
};

#endif
