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

#include "stdafx.h"
#include "RUIUDPSockList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUIUDPSockList::RUIUDPSockList()
{
	m_pieParam = NULL;
}

RUIUDPSockList::~RUIUDPSockList()
{
}

RUIUDPSock* RUIUDPSockList::AddUDPSock()
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	RUIUDPSock*	pUDPSock = new RUIUDPSock;

	pUDPSock->SetIppExtensionParam(m_pieParam);

	InsertAtTail(pUDPSock);

	return pUDPSock;
}

void RUIUDPSockList::RemoveUDPSock(RUIUDPSock* pUDPSock)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	if (pUDPSock != NULL)
		Remove(pUDPSock);
}

BOOL RUIUDPSockList::CloseFirstUDPSock()
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	RUIUDPSock*	pUDPSock = GetFirst();

	if (pUDPSock != NULL)
	{
		pUDPSock->Close();
		Remove(pUDPSock);

		return TRUE;
	}

	return FALSE;
}

void RUIUDPSockList::StreamTo(BYTE* pBuffer, int nPacketSize)
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	RUIUDPSock*	pUDPSock = GetFirst();

	while (pUDPSock != NULL)
	{
		if (pUDPSock->GetClientStreaming())
			pUDPSock->SendPacket(pBuffer, nPacketSize);

		pUDPSock = GetNext(pUDPSock);
	}
}

void RUIUDPSockList::CloseUDPSock()
{
	RUICsLocker	ruiCsLocker(m_ruiCs);

	RUIUDPSock*	pUDPSock = GetFirst();

	while (pUDPSock != NULL)
	{
		pUDPSock->Close();

		RUIUDPSock*	pUDPSockRemove = pUDPSock;

		pUDPSock = GetNext(pUDPSock);

		Remove(pUDPSockRemove);
	}
}
