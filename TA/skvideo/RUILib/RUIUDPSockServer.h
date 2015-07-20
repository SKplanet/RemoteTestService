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

#ifndef _RUIUDPSOCKSERVER_H
#define _RUIUDPSOCKSERVER_H

#include "RUIThread.h"
#include "RUIUDPSockList.h"

class RUIUDPSockServer : public RUIThread
{
public:
	RUIUDPSockServer();
	virtual ~RUIUDPSockServer();

private:
	IppExtensionParam*	m_pieParam;

	RUIUDPSockList		m_udpSockList;

	int					m_nPacketSize;
	BYTE*				m_pBufferSend;

	RUIUDPFrameList		m_SendBufferList;

public:
	IppExtensionParam*	GetIppExtensionParam()                            { return m_pieParam;                                                   }
	void				SetIppExtensionParam(IppExtensionParam* pieParam) { m_pieParam = pieParam; m_udpSockList.SetIppExtensionParam(pieParam); }

	BOOL				InitWinsock();
	void				ExitWinsock();

	int					GetPacketSize();
	void				SetPacketSize(int nPacketSize);
	RUIUDPFrameList*	GetSendBufferList() { return &m_SendBufferList; }

	BYTE*				AllocBufferSend(int nSize);
	void				RemoveBufferSend();

	void				SendRecvThreadProc();

	// for RUIUDPSockServer
public:
	RUIUDPSock*			AddUDPSock();
	void				RemoveUDPSock(RUIUDPSock* pUDPSock);
	RUIUDPSock*			GetFirstUDPSock()   { return m_udpSockList.GetFirst();          }
	BOOL				CloseFirstUDPSock() { return m_udpSockList.CloseFirstUDPSock(); }
	void				StreamTo(BYTE* pBuffer, int nPacketSize);
	void				CloseUDPSock();

public:
	virtual BOOL		OnStart();
	virtual void		OnStop();
	virtual BOOL		ThreadProc();
};

#endif
