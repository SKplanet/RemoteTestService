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


#ifndef _RUISERVERPEER_H
#define _RUISERVERPEER_H

#include "../skvideo/RUILib/RUITCPSockPeer.h"

class RUIUDPSock;

class RUIServerPeer : public RUITCPSockPeer
{
public:
	RUIServerPeer(CWnd* pMsgWnd);
	virtual ~RUIServerPeer();

protected:
	RUIUDPSock*			m_pUDPSock;

public:
	RUIUDPSock*			GetUDPSock()                     { return m_pUDPSock;     }
	void				SetUDPSock(RUIUDPSock* pUDPSock) { m_pUDPSock = pUDPSock; }

	BOOL				GetClientStreaming();
	void				SetClientStreaming(BOOL bClientStreaming);

	void				SendCommandResWaveInDevice(UINT nDeviceID, LPCTSTR szDeviceName);

public:
	virtual void		OnDataReceived(const LPBYTE lpBuffer, DWORD dwCount);
	virtual DWORD		OnDataParsed(const LPBYTE pPacket, DWORD dwPacketSize);
};

#endif
