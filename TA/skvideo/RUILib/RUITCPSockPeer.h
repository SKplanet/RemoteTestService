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

#ifndef _RUITCPSOCKPEER_H
#define _RUITCPSOCKPEER_H

#include "RUITCPSock.h"
#include "../h264/common/extension/ipp_extension.h"

extern UINT WM_START_SERVER;
extern UINT WM_UPDATE_CONNECTION;

class RUITCPSockPeer : public RUITCPSock
{
public:
	RUITCPSockPeer();
	virtual ~RUITCPSockPeer();

public:
	RUITCPSockPeer*		pPrev;
	RUITCPSockPeer*		pNext;

protected:
	IppExtensionParam*	m_pieParam;
	CWnd*				m_pMsgWnd;

	BOOL				m_bConnected;

	BYTE*				m_pPrevBuffer;
	DWORD				m_dwPrevBufferSize;

	// Sync Tick
	DWORD				m_dwSendLatency;
	DWORD				m_dwSendLatencySum;
	DWORD				m_dwSendLatencyCount;
	DWORD				m_dwErrorMin;

public:
	IppExtensionParam*	GetIppExtensionParam()                            { return m_pieParam;     }
	void				SetIppExtensionParam(IppExtensionParam* pieParam) { m_pieParam = pieParam; }

	CWnd*				GetMessageWindow()               { return m_pMsgWnd;    }
	void				SetMessageWindow(CWnd* pMsgWnd)  { m_pMsgWnd = pMsgWnd; }

	BOOL				GetConnected()                   { return m_bConnected; }

	BOOL				GetPeerIP(BYTE& nField0, BYTE& nField1, BYTE& nField2, BYTE& nField3);

	DWORD				ParseData(const LPBYTE pBuffer, DWORD dwBufferSize);

	void				SendCommandString(LPCTSTR szCommand);

	// Sync Tick
	DWORD				GetSendLatency();
	DWORD				GetErrorMin()                    { return m_dwErrorMin; }

	void				ResetSyncTick();
	void				SendCommandSyncTick(DWORD dwMyTick, DWORD dwYourTick, int nRelMyTickSign, DWORD dwRelMyTick, int nCount);
	BOOL				ParseSyncTick(CString& strParam);

	void				SendCommandPacketCongestion();

public:
	virtual void		OnDataReceived(const LPBYTE pBuffer, DWORD dwBufferSize);
	virtual DWORD		OnDataParsed(const LPBYTE pPacket, DWORD dwPacketSize);
	virtual void		OnEvent(UINT uEvent, LPVOID lpvData);
};

#endif
