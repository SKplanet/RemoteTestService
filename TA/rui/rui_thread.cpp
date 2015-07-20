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
#include "rui.h"
#include "ruitdccmd.h"
#include "../TA/resource.h"
#include "../TA/Progress.h"

DWORD WINAPI RA::ruitdcThread(PVOID pvParam)
{
	RA* p;
	//struct sockaddr_in peer;
	//int len = sizeof(peer);
	fd_set rfds;
	struct timeval tv;
	int n;
	int nCnt = 0;

	p = (RA*)pvParam;

	p->ta_log("TA tdc thread start");
	//TRACE("TA tdc thread start\n");
	//p->tdc_setRotate(0);
	//p->tdc_setRotate(4);
	//p->tdc_getDeviceInfo();
	p->tdc_setLog(0);
	p->tdc_set_prohibit_list();
	int loop = 1;
	int retryMax = 30;
	while( loop )
	{
		while( p->m_tdcsock != SOCKET_ERROR ) {

			FD_ZERO(&rfds);
			FD_SET(p->m_tdcsock, &rfds);

			tv.tv_sec = 0; /* 0 sec */
			tv.tv_usec = 500 * 1000; // 500msec
			n = select(p->m_tdcsock + 1, &rfds, NULL, NULL, &tv);

			if (n == 0) {
				if( p->tdc_send_command(TDC_GETSTATUS) == 0 )
					break;

				if( nCnt++ % 6 == 0 )
				{
					p->tdc_send_command(TDC_GETBATTEMPERATURE);
					p->tdc_send_command(TDC_GETBATTLEVEL);
				}
				continue;
			}

			if (n < 0) {
				//Err("ReadExact: select");
				break;
			}
       
			if (p->m_tdcsock == -1) {
				//Log("client disconnected\n");
				break;
			}

			if (FD_ISSET(p->m_tdcsock, &rfds)) {
			
				if( 0 >= p->ProcessTDCMessage() )
					break;
			
			}

			if (p->m_tdcsock == -1) {
				//Log("client disconnected\n");
				break;
			}
			retryMax = 30;
		}

		if( p->m_tdcsock != INVALID_SOCKET)
		{
			p->ShutdownSocket(p->m_tdcsock);
			p->m_tdcsock = INVALID_SOCKET;
		}
		loop = 0;
		if( p->m_state < TA_STATE_SERVICE_STOP && retryMax-- > 0)
		{
			loop = p->restart_tdc();
			if( loop && (p->m_tdcsock != INVALID_SOCKET))
			{
				p->tdc_setLog(0);
				p->tdc_set_prohibit_list();
			}
		}
	}

	if( p->m_state < TA_STATE_SERVICE_STOP )
	{
		p->set_state(TA_STATE_ERR_START_SERVICE);
		p->DeviceStatusToPOC(8, 1, "Service Error(TDC)");
	}

	p->ta_log("\tTA tdc thread stop");
	//TRACE("TA tdc thread stop\n");
	p->m_htdcThread = 0;
	return 1;
}


DWORD WINAPI RA::ruilistenThread(PVOID pvParam)
{
	RA* p;
	SOCKET client_fd;
	struct sockaddr_in peer;
	int len = sizeof(peer);
	char clientIP[16];

	p = (RA*)pvParam;

	p->ta_log("\tTA listener thread start");
	p->ta_log("[TA_STATE_READY]");
	p->DeviceStatusToPOC(2, 0, "Ready to service");
	p->set_state(TA_STATE_READY);
	p->m_hclientThread = 0;

	//DWORD dwclientThreadID;
	p->InitListener();
	//p->Video_StartServer();
	while( p->m_listenSock != INVALID_SOCKET && (client_fd = accept( p->m_listenSock, (sockaddr*)&peer, &len)) >= 0)
	//while(1)
	{
		//p->Video_StartServer();
		//p->InitListener();
		if (p->m_listenSock == INVALID_SOCKET )
		{
			p->ta_log("\tTA ERR listen socket INVALID");
			break;
		}
		//client_fd = accept( p->m_listenSock, (sockaddr*)&peer, &len);
		//p->ShutdownSocket(p->m_listenSock);
		//p->m_listenSock = INVALID_SOCKET;
		if( client_fd == INVALID_SOCKET )
		{
			p->ta_log("\tTA ERR listen accept socket INVALID - continue listen");
			continue;
		}


		lstrcpy(clientIP, inet_ntoa(peer.sin_addr));
		p->ta_log("[TA_STATE_LISTEN]-(%s):(%d)", clientIP, ::GetCurrentProcessId());

		if( p->m_hclientThread )
		{
			p->ta_log("[Reject] - serve only one client");
			p->ShutdownSocket(client_fd);
			//p->Video_StartServer();

			continue;
		}
	
		DWORD dwclientThreadID;
		p->m_clientTempSock = client_fd;
		lstrcpy(p->m_clientIP, clientIP);
		p->m_hclientThread = CreateThread(NULL, 0, ruiclientThread, p, 0, &dwclientThreadID);
		CloseHandle(p->m_hclientThread);
		//WaitForSingleObject( p->m_hclientThread, INFINITE);
		
	}

	p->ta_log("\tTA listener thread stop");
	if( p->m_state < TA_STATE_SERVICE_STOP )
	{
		p->set_state(TA_STATE_ERR_START_SERVICE);
		p->DeviceStatusToPOC(8, 1, "Service Error(LISTEN)");
	}
	p->m_hlistenThread = 0;
	return 1;
}

DWORD WINAPI RA::ruiclientThread(PVOID pvParam)
{
	RA* p;
	//SOCKET client_fd;

	p = (RA*)pvParam;
	p->ta_log("\tTA client thread start");

	int nonblocking =0;
	int res = ioctlsocket(p->m_clientTempSock, FIONBIO, (unsigned long*) &nonblocking);

	BOOL nodelayval = TRUE;
	if (setsockopt(p->m_clientTempSock, IPPROTO_TCP, TCP_NODELAY, (const char *) &nodelayval, sizeof(BOOL)))
	{
		p->ta_log("\tTA ERR client socket option 1 set failed");
		p->ShutdownSocket(p->m_clientTempSock);
		p->DeviceStatusToPOC(4, 0, "Client Disconnected");
		p->set_state(TA_STATE_READY);
		p->m_hclientThread = 0;
		//p->Video_StartServer();
		return 0;
	}

	p->ta_log("[TA_STATE_CLIENT_INCOMING]-(%s)", p->m_clientIP);
	p->DeviceStatusToPOC(3, 1, "Client Incoming");
	p->set_state(TA_STATE_CLIENT_INCOMING);

	if( p->onNewClientNegotiate(p->m_clientTempSock) == 0 )
	{
		p->svr_disconnectnotify(0); // invalid access
		p->ShutdownSocket(p->m_clientTempSock);
		p->DeviceStatusToPOC(4, 0, "Client Disconnected");
		p->set_state(TA_STATE_READY);
		p->m_hclientThread = 0;
		//p->Video_StartServer();
		return 0;
	}

	nonblocking =1;
	res = ioctlsocket(p->m_clientTempSock, FIONBIO, (unsigned long*) &nonblocking);
		

	p->m_clientSock = p->m_clientTempSock;

	// client serving loop
	fd_set rfds;
	struct timeval tv;
	int n;

	p->ta_log("Client Session start <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
	p->onNewClient();

	while (p->m_clientSock != INVALID_SOCKET) {

		FD_ZERO(&rfds);
		FD_SET(p->m_clientSock, &rfds);

		//if( p->m_opt_screen ) // h264
		//{
		//	tv.tv_sec = 1; // 1sec
		//	tv.tv_usec = 0;
		//}
		//else 
		{
			tv.tv_sec = 5; /* 5 sec */
			tv.tv_usec = 0;
		}
		n = select(p->m_clientSock + 1, &rfds, NULL, &rfds, &tv);

		if (n < 0) {
			//Err("ReadExact: select");
			break;
		}
        
		if (p->m_clientSock == -1) {
			//Log("client disconnected\n");
			break;
		}
		//if( n == 0 )
		//{
		//		p->update_request();
		//}
		//else 
		if (FD_ISSET(p->m_clientSock, &rfds)) {
			if( 0 == p->ProcessClientMessage() )
					break;
		}

		if (p->m_clientSock == -1) {
			//Log("client disconnected\n");
			break;
		}
	}

	//p->SetUpdateCompleteCallback( 0,0);
	p->onExitClient();
	p->ta_log("Client Session end  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	//p->ReInit();
	if( p->m_state < TA_STATE_AFTER_CLIENT_REINIT )
	{
		p->ta_log("[TA_STATE_READY]");

		p->DeviceStatusToPOC(2, 0, "Ready to service");
		p->set_state(TA_STATE_READY);
	}

	//p->ta_log("TA client thread stop");
	p->m_hclientThread = 0;
	return 1;
}
DWORD WINAPI RA::ruifbcThread(PVOID pvParam)
{
	RA* p;
	fd_set rfds;
	struct timeval tv;
	int n;
	//SOCKET client_fd;

	p = (RA*)pvParam;

	int loop = 1;
	int retryMax = 30;
	while(loop)
	{
		p->m_fbcsock;
		while(p->m_fbcsock != INVALID_SOCKET) {
				FD_ZERO(&rfds);
				FD_SET(p->m_fbcsock, &rfds);

				tv.tv_sec = 1; /* 5 sec */
				tv.tv_usec = 0;
				n = select(p->m_fbcsock + 1, &rfds, NULL, NULL, &tv);

				if( n == 0 ) // timeout
				{
					p->update_req();
					continue;
				}

				if (n < 0) {
					//Err("ReadExact: select");
					break;
				}
        
				if (p->m_fbcsock == -1) {
					//Log("client disconnected\n");
					break;
				}

				if (FD_ISSET(p->m_fbcsock, &rfds)) {
					if( 0 == p->ProcessFBCMessage() )
						break;
				}

				if (p->m_fbcsock == -1) {
					//Log("client disconnected\n");
					break;
				}
				retryMax = 30;
		}

		if( p->m_fbcsock != INVALID_SOCKET)
		{
			p->WriteExact(p->m_fbcsock, "q", 1);
			p->ShutdownSocket(p->m_fbcsock);
			p->m_fbcsock = INVALID_SOCKET;
		}
		loop = 0;
		if( p->m_state < TA_STATE_SERVICE_STOP && retryMax-- > 0)
		{
			loop = p->restart_fbc();
		}
	}
	
	if( p->m_state < TA_STATE_SERVICE_STOP )
	{
		p->set_state(TA_STATE_ERR_START_SERVICE);
		p->DeviceStatusToPOC(8, 1, "Service Error(FBC)");
	}
	//p->stop_log();
	p->m_hfbcThread = 0;
	return 1;
}

DWORD WINAPI RA::ruilogcatThread(PVOID pvParam)
{
	RA* p;
	int rtn;

	p = (RA*)pvParam;

	while(1) {
		rtn = p->ReadFromPipe(p);
		if( rtn == 0 )
			break;
		Sleep(300);
	}

	CloseHandle(p->m_hlogThread);
	p->m_hlogThread = 0;
	return 1;
}

DWORD WINAPI ruiwaitThread(PVOID pvParam)
{
	CProgress * pDlg;
	RA* p;
	//int rtn;

	pDlg = (CProgress*)pvParam;
	if( pDlg == 0 )
		return 0;
	p = pDlg->pra;
	if( p == 0 )
		return 0;
	int max = pDlg->max;

	pDlg->Create(IDD_DIALOG_PROGRESS); //, p->m_UIWnd);
	pDlg->ShowWindow(SW_SHOW);
	
	HWND hDlg = pDlg->GetSafeHwnd();
	pDlg->m_Progress.SetRange(0,max);
	int n = 0;
	while(::IsWindow(hDlg) && p->m_bShowProgress)
	{
		pDlg->m_Progress.SetPos(n++);

		Sleep(50);
		if( !p->m_bShowProgress )
			break;

		DummyMessageLoop(hDlg);
		if( !p->m_bShowProgress )
			break;

		Sleep(50);
		if( !p->m_bShowProgress )
			break;

		DummyMessageLoop(p->m_UIWnd->GetSafeHwnd());

		n = n % max;
	}
	//p->m_bShowProgress = 0;

	if( ::IsWindow(hDlg) )
	{
		pDlg->PostMessage(WM_CLOSE,0,0);
		//pDlg->DestroyWindow();
	}

	return 1;
}

void RA::ShowProgress(bool bShow, const char* title, int second)
{
	DWORD dw;
	//TRACE("ShowProgress(%d)\n",bShow);
	if( bShow ) 
	{
		if( m_bShowProgress )
			return;
		m_bShowProgress = 1;

		CProgress * pDlg = new CProgress(m_UIWnd);
		pDlg->title = title;
		pDlg->max = second * 10;
		pDlg->pra = this;
		//pDlg->Create(IDD_DIALOG_PROGRESS); //, m_UIWnd);

		HANDLE hThread = CreateThread(NULL, 0, ruiwaitThread, pDlg, 0, &dw);
		CloseHandle(hThread);
	}
	else
	{
		m_bShowProgress = 0;
		Sleep(100);
	}
}

