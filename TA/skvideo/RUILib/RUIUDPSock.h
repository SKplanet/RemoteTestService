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

#ifndef _RUIUDPSOCK_H
#define _RUIUDPSOCK_H

#include "RUIThread.h"
#include "RUIUDPFrameList.h"

#define SEND_BUFFER_SIZE		( 512 * 1024)
#define RECV_BUFFER_SIZE		(1024 * 1024)

#define SEND_SOCKET				1
#define RECV_SOCKET				2

typedef struct tagDelaySendPacketContext
{
	char	szDestIP[_MAX_PATH];
	UINT	uiDestPort;
	DWORD	dwTickToSend;
} DelaySendPacketContext;

class RUIUDPSock : public RUIThread
{
public:
	RUIUDPSock();
	virtual ~RUIUDPSock();

public:
	RUIUDPSock*			pPrev;
	RUIUDPSock*			pNext;

private:
	IppExtensionParam*	m_pieParam;
	CWnd*				m_pMsgWnd;

	BOOL				m_bStunRestriction;
	BOOL				m_bClientStreaming; // Client Command·Î ¼ÂÆÃµÊ

	SOCKET				m_hSocket;

	char				m_szDestIP[_MAX_PATH];
	USHORT				m_uiSendPort;
	USHORT				m_uiDestPort;
	USHORT				m_uiRecvPort;

	int					m_nPacketSize;
	BYTE*				m_pBufferSend;
	BYTE*				m_pBufferRecv;

	RUIUDPFrameList		m_SendBufferList;
	RUIUDPFrameList		m_RecvBufferList;

	RUIBufferList		m_DelaySendBufferList;

	// Sync Tick
	DWORD				m_dwSendLatency;
	DWORD				m_dwSendLatencySum;
	DWORD				m_dwSendLatencyCount;
	DWORD				m_dwErrorMin;

public:
	IppExtensionParam*	GetIppExtensionParam()                            { return m_pieParam;                                                      }
	void				SetIppExtensionParam(IppExtensionParam* pieParam) { m_pieParam = pieParam; m_RecvBufferList.SetIppExtensionParam(pieParam); }

	CWnd*				GetMessageWindow()               { return m_pMsgWnd;                                                }
	void				SetMessageWindow(CWnd* pMsgWnd)  { m_pMsgWnd = pMsgWnd; m_RecvBufferList.SetMessageWindow(pMsgWnd); }

	BOOL				GetStunRestriction()                      { return m_bStunRestriction;             }
	void				SetStunRestriction(BOOL bStunRestriction) { m_bStunRestriction = bStunRestriction; }
	BOOL				GetClientStreaming()                      { return m_bClientStreaming;             }
	void				SetClientStreaming(BOOL bClientStreaming) { m_bClientStreaming = bClientStreaming; }

	BOOL				InitWinsock();
	void				ExitWinsock();

	UINT				GetBufferSize(int nOptName, SOCKET hSocket);
	UINT				GetSendBufferSize(SOCKET hSocket);
	UINT				GetRecvBufferSize(SOCKET hSocket);
	UINT				IncreaseBuffer(int nOptName, SOCKET hSocket, unsigned nReqSize);
	UINT				IncreaseSendBuffer(SOCKET hSocket, unsigned nReqSize);
	UINT				IncreaseRecvBuffer(SOCKET hSocket, unsigned nReqSize);

	int					GetPacketSize();
	void				SetPacketSize(int nPacketSize);
	RUIUDPFrameList*	GetSendBufferList() { return &m_SendBufferList; }
	RUIUDPFrameList*	GetRecvBufferList() { return &m_RecvBufferList; }

	BYTE*				AllocBufferSend(int nSize);
	BYTE*				AllocBufferRecv(int nSize);
	void				RemoveBufferSend();
	void				RemoveBufferRecv();

	BOOL				OpenSocket(USHORT uiSendPort, LPCSTR szDestIP, USHORT uiDestPort, USHORT uiRecvPort,
									UINT nSendBufferSize, UINT nRecvBufferSize);
	BOOL				CloseSocket();

	BOOL				OpenSend(USHORT uiSendPort, LPCSTR szDestIP, USHORT uiDestPort);
	BOOL				OpenRecv(USHORT uiSendPort, LPCSTR szDestIP, USHORT uiDestPort, USHORT uiRecvPort);
	BOOL				OpenStun(USHORT uiSendPort, LPCSTR szDestIP, USHORT uiDestPort, USHORT uiRecvPort);

	void				Close();

	BOOL				SendStunPacket(LPCSTR szDestIP, UINT uiDestPort);
	RUIBuffer*			SendDelayPacket(LPCSTR szDestIP, UINT uiDestPort, DWORD dwTickAfter, BYTE* pBuffer, DWORD dwSize);

	int					SendPacket(BYTE* pBuffer, int nSize, LPCSTR szDestIP = NULL, USHORT uiDestPort = 0);
	BOOL				RecvPacket(BYTE* pBuffer, int nSize, int& nRecv, LPSTR szFromIP, USHORT& uiFromPort);

	BOOL				Proc_Send();
	BOOL				Proc_DelaySend();
	BOOL				Proc_Command();
	BOOL				Proc_Recv();

	BOOL				ParseData(LPBYTE lpBuffer, DWORD dwCount);

	BOOL				SendCommand(LPCTSTR szCommand, BOOL bDelay, DWORD dwTickAfter);
	void				SendCommandStun();
	void				SendCommandStunAck();
	void				SendCommandSyncTick(DWORD dwMyTick, DWORD dwYourTick, int nRelMyTickSign, DWORD dwRelMyTick, int nCount);
	void				SendCommandPacketCongestion();

	DWORD				GetSendLatency();
	DWORD				GetErrorMin() { return m_dwErrorMin;  }
	void				ResetSyncTick();
	BOOL				ParseSyncTick(CString& strParam);

public:
	virtual BOOL		OnStart();
	virtual void		OnStop();
	virtual BOOL		ThreadProc();

	virtual void		OnDataParsed(LPCTSTR szData, int nLen);
};

#endif
