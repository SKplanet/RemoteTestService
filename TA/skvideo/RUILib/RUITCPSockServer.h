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

#ifndef _RUITCPSOCKSERVER_H
#define _RUITCPSOCKSERVER_H

#include "RUIThread.h"
#include "../h264/common/extension/ipp_extension.h"
#include "RUITCPSockPeerList.h"
#include "RUIUDPFrameList.h"

class RUITCPSockServer : public RUIThread
{
public:
	RUITCPSockServer();
	virtual ~RUITCPSockServer();

private:
	IppExtensionParam*		m_pieParam;

	UINT					m_nPort;
	RUITCPSockPeerList		m_peerList;

	RUIUDPFrameList			m_SendBufferList;

public:
	IppExtensionParam*		GetIppExtensionParam()                            { return m_pieParam;     }
	void					SetIppExtensionParam(IppExtensionParam* pieParam) { m_pieParam = pieParam; }

	UINT					GetPort()           { return m_nPort;  }
	void					SetPort(UINT nPort) { m_nPort = nPort; }

	RUIUDPFrameList*		GetSendBufferList() { return &m_SendBufferList; }

	RUITCPSockPeer*			AddPeer();
	RUITCPSockPeer*			AddPeer(RUITCPSockPeer* pPeer);
	RUITCPSockPeer*			FindPeerAvailable();
	RUITCPSockPeer*			StartServer();
	void					StopServer();
	void					SendMessageToClient(BYTE* pMessage, DWORD dwSize);

public:
	virtual BOOL			OnStart();
	virtual void			OnStop();
	virtual BOOL			ThreadProc();
};

#endif
