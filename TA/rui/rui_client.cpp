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
#include "ruicmd.h"

extern CString GetVersionInfo(HMODULE hLib);
int RA::ProcessClientMessage()
{
	RUICMD* pcmd;
	int n;

	//char buf[RUICMD_HEADER];

	if( (n=ReadExact(m_clientSock, (char*)m_recvBuf, RUICMD_HEADER)) <= 0 ) {
		ta_log("\tTA ERR ProcessClientMessage read header(%d)", n);
		return 0;
	}

	pcmd = (RUICMD*)m_recvBuf;

	//memcpy(m_rBuf, buf, 4);
	pcmd->cmd = Swap32(pcmd->cmd);

	//memcpy(m_rBuf, buf+4, 4);
	pcmd->idx= Swap32(pcmd->idx);
	
	//memcpy(m_rBuf, buf+8, 4);
	pcmd->lsize = Swap32(pcmd->lsize) - RUICMD_HEADER;

	if( pcmd->lsize > 0 && pcmd->lsize < 500 )
	{
		if( (n=ReadExact(m_clientSock, (char*)&pcmd->body, pcmd->lsize)) <= 0 ) {
			ta_log("\tTA ERR ProcessClientMessage read body(%d)", n);
			return 0;
		}
		((char*)&pcmd->body)[pcmd->lsize] = 0;
		//cmd.body = (void*)m_recvBuf;

	}

	switch( pcmd->cmd )
	{
	case REQ_SCREENINFO			:
		//ta_log("TA CLI RUICMD_GETSCREENINFO");
		onGetScreenInfo(pcmd);
		break;

	case REQ_SCREENBUF			:
		//ta_log("TA CLI REQ_SCREENBUF");
		onGetScreenBuf(pcmd);
		break;
	case CLI_MOUSEEVENT:
		onMouseEvent(pcmd);
		break;
	case CLI_KEYEVENT:
		onKeyEvent(pcmd);
		break;
	case REQ_LOGCAT:
		onLogcat(pcmd);
		break;
	case CLI_WAKEUP:
		key_wake();
		break;
	case CLI_ROTATE:
		{
			char r = *((char *)&pcmd->body);
			tdc_setRotate(r);
		}
		break;
	case REQ_FILE_LIST:
		ta_log("\t\tFTP request ftp filelist");
		onReqFileList(pcmd);
		break;
	case REQ_FILE_DOWNLOAD:
		onReqFileDownload(pcmd);
		break;
	case CLI_FILE_UPLOAD:
		onCliFileUpload(pcmd);
		break;
	case CLI_FILE_UPLOAD_DATA:
		onCliFileUploadData(pcmd);
		break;
	case CLI_FILE_ERROR_MSG:
		onCliFileErrorMsg(pcmd);
		break;
	case REQ_APP_INST:
		onAppInstReq(pcmd);
		break;
	case CLI_APP_INST_DATA:
		onAppInstData(pcmd);
		break;
	case CLI_REMOVE_USER_APP:
		ta_log("\t\tclient request remove user application");
		tdc_removemessage();
		tdc_getinstalledpkgname();
		break;
	case REQ_GETBUILD_MANUFACTURE:
	case REQ_GETBUILD_MODEL:
	case REQ_GETBUILD_SERIAL:
	case REQ_GET_BUILD_VERSION_RELEASE:
	case REQ_GET_BUILD_VERSION_SDK_INT:
		onCliGetDevInfo(pcmd);
		break;
	case REQ_RESTART_ENCODER:
		onVideoRestartEncoder(pcmd);
		break;
	case REQ_START_STREAMING:
		onVideoStartStreaming(pcmd);
		break;
	case REQ_SET_STREAMING_MODE:
		onVideoSetStreamingMode(pcmd);
		break;
	case CLI_DISCONNECT:
		onClientDisconnectNotify(pcmd);
		break;
	}

	return 1;
}

int RA::onNewClientNegotiate(SOCKET client_fd)
{
	ta_log("[TA_STATE_CLIENT_NEGOTIATE]");
	DeviceStatusToPOC(3, 2, "Client Negotiate");
	set_state(TA_STATE_CLIENT_NEGOTIATE);
	if( WriteExact(client_fd, m_taVersion, TA_VERSION_LEN) < 0 ) {
		ta_log("TA ERR onNewClientNegotiate socket write failed (version)");
		return 0;
	}

	BYTE clen;
	char reserve_id[256];
	BYTE conn_res = 0;
	DWORD conn_res_time = 0;

	m_clientValidStart	=	0;
	m_clientValidEnd	=	0;

	// get tc version
	if( ReadExact(client_fd, (char*)&clen, 1) <= 0 ){
		ta_log("TA ERR onNewClientNegotiate negotiate failed (get tc version len)");
		return 0;
	}

	memset(reserve_id, 0, 256);
	if( ReadExact( client_fd, reserve_id, clen) <= 0 ) {
		ta_log("TA ERR onNewClientNegotiate negotiate failed (get tc version str)");
		return 0;
	}
	TRACE("%d:%s\n", clen, reserve_id);

	lstrcpy(m_clientVersion, reserve_id);
	ta_log("\t\tTA client version [%s]", m_clientVersion);

	// get tc reservation id
	if( ReadExact(client_fd, (char*)&clen, 1) <= 0 ){
		ta_log("TA ERR onNewClientNegotiate negotiate failed (get reservation id len)");
		return 0;
	}

	memset(reserve_id, 0, 256);
	if( ReadExact( client_fd, reserve_id, clen) <= 0 ) {
		ta_log("TA ERR onNewClientNegotiate negotiate failed (get reservation id str)");
		return 0;
	}

	lstrcpy(m_clientReserveID, reserve_id);
	ta_log("\t\tTA client reservation id [%s]", m_clientReserveID);

	if( lstrlen(m_clientReserveID) > 29 &&  strcmp(m_clientReserveID+8, DEVELOPER_CHECK_KEY) == 0 )
	{
		SYSTEMTIME st;
		::GetLocalTime(&st);
		CString validstr, validto;
#ifdef REDECON		// check only 6char (except day for timezone difference)
		validstr.Format("%04d%02d", st.wYear, st.wMonth);
		validto = m_clientReserveID;
		validto = validto.Left(6);
		if( validstr.Compare(validto) != 0 )
		{
			conn_res = CONN_RES_INVALID_RESERVATIONID;
			if( WriteExact(client_fd, (const char*)&conn_res, 1) < 0 ) {
				ta_log("TA ERR client socket write failed (conn_res)");
				return 0;
			}
			return 0;
		}
		lstrcpy(m_clientReserveID, DEVELOPER_CHECK_KEY);
		m_clientValidStart = timedateToDword();
		//m_clientValidEnd = timedateToDword() + 30 * 60 * 2; // 120 min
		m_clientValidEnd = timedateDwordPlus(m_clientValidStart, 0, 2, 0, 0);
#else
		validstr.Format("%04d%02d%02d", st.wYear, st.wMonth, st.wDay);
		validto = m_clientReserveID;
		validto = validto.Left(8);
		if( validstr.Compare(validto) != 0 )
		{
			conn_res = CONN_RES_INVALID_RESERVATIONID;
			if( WriteExact(client_fd, (const char*)&conn_res, 1) < 0 ) {
				ta_log("TA ERR client socket write failed (conn_res)");
				return 0;
			}
			return 0;
		}
		lstrcpy(m_clientReserveID, DEVELOPER_CHECK_KEY);
		m_clientValidStart = timedateToDword();
		//m_clientValidEnd = timedateToDword() + 31 * 10; // 10 min
		m_clientValidEnd = timedateDwordPlus(m_clientValidStart, 0, 0, 10, 0);
#endif
	}

	else {

		if( GetReservInfoFromPOC( m_clientReserveID) <= 0 )
		{
			ClientStatusToPOC(m_clientReserveID, 8, 0, "Client connect negotiate failed");
			ta_log("TA ERR client negotiate failed (GetReservInfoFromPOC failed)");
			
			conn_res = CONN_RES_INVALID_RESERVATIONID;
			if( WriteExact(client_fd, (const char*)&conn_res, 1) < 0 ) {
				ta_log("TA ERR client socket write failed (conn_res)");
				return 0;
			}
			return 0;
		}
	}

	int c1,c2,c3,c4;
	sscanf(m_clientVersion, "%d.%d.%d.%d",  &c1, &c2, &c3, &c4);

	CString s_version = GetVersionInfo(0);
	int s1,s2,s3,s4;
	sscanf(s_version, "%d.%d.%d.%d",  &s1, &s2, &s3, &s4);

	// version check
	if( s1 > c1 || s2 > c2 || s3 > c3)
	{
		ClientStatusToPOC(m_clientReserveID, 8, 0, "Client invalid version");
		ta_log("TA ERR client negotiate failed (Client invalid version)");
			
		conn_res = CONN_RES_INVALID_VERSION;
		if( WriteExact(client_fd, (const char*)&conn_res, 1) < 0 ) {
			ta_log("TA ERR client socket write failed (conn_res)");
			return 0;
		}
		return 0;
	}

	DWORD dwCurrent = timedateToDword();
	if(  m_clientValidStart > 0 && m_clientValidEnd > 0 )
	{
		if( dwCurrent < m_clientValidStart ||
			m_clientValidEnd < dwCurrent )
		{
			ClientStatusToPOC(m_clientReserveID, 8, 0, "Client connect negotiate failed");
			
			conn_res = CONN_RES_INVALID_TIME;
			if( WriteExact(client_fd, (const char*)&conn_res, 1) < 0 ) {
				ta_log("TA ERR client socket write failed (conn_res)");
				return 0;
			}
			ta_log("TA ERR client negotiate failed( reservation time invalid)");
			return 0;
		}
	}

	conn_res = CONN_RES_OK;
	if( WriteExact(client_fd, (const char*)&conn_res, 1) < 0 ) {
		ta_log("TA ERR client socket write failed (conn_res)");
		return 0;
	}
	conn_res_time = timedateDwordMinus(m_clientValidEnd, dwCurrent); //(m_clientValidEnd - dwCurrent);
	//conn_res_time = (m_clientValidEnd - dwCurrent);
	if( WriteExact(client_fd, (const char*)&conn_res_time, 4) < 0 ) {
		ta_log("TA ERR client socket write failed (conn_res)");
		return 0;
	}

	// test 30 sec timeout
	//DWORD dwCurrent = timedateToDword();
	//m_clientValidStart = dwCurrent;
	//m_clientValidEnd = dwCurrent + 15;
	return 1;
}

void RA::onNewClient()
{
	ta_log("[TA_STATE_CLIENT_ACCEPTED]");
	Video_StartServer();
	DeviceStatusToPOC(3, 3, "Client Connected");
	ClientStatusToPOC(m_clientReserveID, 3, 0, "Client Connedted");
	set_state(TA_STATE_CLIENT_CONNECTED);
	key_wake();

	ta_log("\t\tclient version [%s]", m_clientVersion);
	ta_log("\t\tclient reservation id [%s]", m_clientReserveID);


	//Video_StartServer();
	m_bInitRequired = 1;
#ifndef REDECON	
	if( strcmp(m_clientReserveID, DEVELOPER_CHECK_KEY) == 0 )
	{
		m_bInitRequired = 0;
	}
#endif
	m_bInitRequiredTime =  m_clientValidEnd;

	m_opt_screen = 0;	// 0 - Image, 1 - H.264
	m_opt_h264_quality = 0; // 1-low, 0-normal, 2-high
	m_opt_audio = 0;	// 0 - off, 1- on
	m_bFullSizeRequest = 0;

	m_sendBufIdx = 0;
	m_clientFps = 0;
	m_clientBps = 0;

	//SetUpdateCompleteCallback( RA::OnUpdateComplete, (void*)this);
	tdc_setRotate(0);
	tdc_setRotate(4);
	m_devStateChanged = 1;
	m_frameLastSend = 0;
	m_byteLastSend = 0;

//	Video_CreateUDPSock(NULL, m_clientIP);
//	InitIppExtensionParam_Connect(&m_ieParam);
//	m_ieParam.m_pEncoder = (void*) m_threadEncoder.GetEncoder();

	tdc_wake_lock_aquire();
	m_fbcLastFrameCount = -1;
	update_req();
}

void RA::onClientDisconnectNotify(RUICMD* cmd)
{
	ta_log("[TA_STATE_CLIENT_DISCONNECT_NOTIFY]");
	DeviceStatusToPOC(4, 1, "Client User Disconnect");
	ClientStatusToPOC(m_clientReserveID, 4, 1, "Client User Disconnect");

	set_state(TA_STATE_CLIENT_USER_DISCONNECT);

	if( m_clientSock != INVALID_SOCKET)
	{
		ShutdownSocket(m_clientSock);
		m_clientSock = INVALID_SOCKET;
	}
}

void RA::onExitClient()
{
	if( m_ftp ) {
		FBCCMD msg;
		msg.cmd = 'o';
		USHORT err = (USHORT)7;
		msg.err = Swap16(err);
		msg.size = Swap32(FBCCMD_HEADER);

		WaitForSingleObject( m_fbOutputMutex, INFINITE);
		WriteExact(m_fbcsock, (char*)&msg, FBCCMD_HEADER);
		ReleaseMutex( m_fbOutputMutex);
		m_ftp = 0;
	}

	stop_log();
	tdc_wake_lock_release();

	if( m_state < TA_STATE_CLIENT_DISCONNECT)
	{
		ta_log("[TA_STATE_CLIENT_ACCIDENTAL_DISCONNECT]");
		DeviceStatusToPOC(4, 3, "Client Disconnected on Error");
		ClientStatusToPOC(m_clientReserveID, 4, 3, "Client Disconnected on Error");
		if( m_state < TA_STATE_AFTER_CLIENT_REINIT)
			set_state(TA_STATE_CLIENT_DISCONNECT);
	} else {
		//ClientStatusToPOC(m_clientReserveID, 4, 0, "Client Disconnected");
		ta_log("[TA_STATE_CLIENT_DISCONNECT]");
	}

	if( m_clientSock != INVALID_SOCKET )
	{
		ShutdownSocket(m_clientSock);
		m_clientSock = INVALID_SOCKET;
	}
	
	//if( m_state < TA_STATE_AFTER_CLIENT_REINIT )
	//{
	//	Video_StartServer();
	//}

	m_opt_screen = 0;	// 0 - Image, 1 - H.264
	m_opt_h264_quality = 0; // 1-low, 0-normal, 2-high
	m_opt_audio = 0;	// 0 - off, 1- on
	m_frameLastSend = 0;
	m_byteLastSend = 0;
	m_clientReserveID[0] = 0;


}

int RA::ReInit()
{
	DeviceStatusToPOC(5, 0, "Reinit after client session");
	set_state(TA_STATE_AFTER_CLIENT_REINIT);
	if( m_bInitRequired )
	{
		//if( m_hlistenThread ) {
		//	//Sleep(1000);
		//	ta_log("FBC listen thread terminating");
		//	CloseHandle(m_hlistenThread);
		//	TerminateThread(m_hlistenThread,0);
		//	m_hlistenThread = 0;
		//}

		key_wake();
		// remove installed package
		ShowProgress(1, "TA : Remove installed package (30 sec)", 30);
		tdc_removemessage();
		tdc_getinstalledpkgname();
		Sleep(30000);
		ShowProgress(0);

		key_wake();
		stop_ta_service();
		ShowProgress(1, "TA : Reboot Device (60 sec)", 60);
		cmd_adb("reboot");

		//Sleep(2000);
		ta_log("\t\tReboot wait 60 seconds");
		Sleep(60000);
		ShowProgress(0);


		if( start_ta_service(1) <= 0 )
		{
			ta_log("\t\tstart ta service failed : retry 1 more");

			stop_ta_service();
			Sleep(10000);
			if( start_ta_service(1) <= 0 )
			{
				ta_log("\t\tstart ta service retry failed");
			}
		}
		DummyMessageLoop(m_UIWnd->GetSafeHwnd());
		
		//m_hlistenThread = CreateThread(NULL, 0, ruilistenThread, this, 0, &m_dwlistenThreadID);

		////ta_log("start video encoder");
		//RegisterToPOC();
		//set_state(TA_STATE_READY);

	}

	m_bInitRequired = 0;

	return 1;
}


int RA::onGetScreenInfo(RUICMD* cmd)
{
	m_opt_screen = cmd->idx & SCRI_SCREEN_H264;
	m_opt_audio = (cmd->idx & SCRI_AUDIO) != 0;
	UINT bH264Quality = cmd->idx & 0x0000FF00;
	bH264Quality = bH264Quality >> 8;
	if( bH264Quality > 1 ) 
		bH264Quality = 2;
	else if( bH264Quality > 0 )
		bH264Quality = 1;
	
	ta_log("\t\tclient mode : screen[%s], sound[%s]", m_opt_screen ? "H.264" : "Image", m_opt_audio ? "ON":"OFF");


	RUICMD_SCREENINFO scmd;

	scmd.cmd = Swap32(RES_SCREENINFO);
	scmd.idx = Swap32(0);
	scmd.lsize = Swap32(sizeof(RUICMD_SCREENINFO));
	scmd.width = Swap16(m_fbcWidth);
	scmd.height = Swap16(m_fbcHeight);
	scmd.BPP = Swap16(m_fbcBPP);

	WaitForSingleObject( m_clientOutputMutex, INFINITE);
	if( WriteExact(m_clientSock, (const char*)&scmd, sizeof(RUICMD_SCREENINFO)) <= 0 )
	{
		ReleaseMutex(m_clientOutputMutex);
		return 0;
	}
	ReleaseMutex(m_clientOutputMutex);

	//if( m_bStreamingAudio != m_opt_audio ||
	//	m_bStreamingVideo != m_opt_screen ||
	//	m_opt_h264_quality != (int)bH264Quality )
	{
		m_bStreamingAudio = m_opt_audio;
		m_bStreamingVideo = m_opt_screen;
		m_opt_h264_quality = (int)bH264Quality;
		if( m_opt_h264_quality == 1 ) // low
			m_nBitRate = 75000; // 75kbps
		else if( m_opt_h264_quality == 2 ) // high
			m_nBitRate = 500000; // 500kbps
		else
			m_nBitRate = 150000; // normal 150k bps
	
		//Video_StopEncoder();
		//Video_StartEncoder();
		//m_bCaptureFlag = TRUE;
	}
	if( m_opt_screen )
	{
		ta_log("\t\tH.264 bit rate : %d", m_nBitRate);
	}

	return 1;
}

int RA::onGetScreenBuf(RUICMD* cmd)
{
	//ta_log("update request");
	//int bFastmode = (cmd->idx != 1);

	if( m_bFullSizeRequest == 0)
		m_bFullSizeRequest = cmd->idx;
	//update_request(bFastmode);
	OnUpdateComplete();
	return 1;
}

// CPreview
//void RA::OnUpdateComplete(RA* pra, void* pData, int size, int bFullsize)
void RA::OnUpdateComplete()
{
	if( m_clientSock == INVALID_SOCKET)
		return;

	//OnStateChanged();
	int cnt = 100;
	// max 1000msec wait
	while ( cnt-- > 0 && m_fbcLastFrameCount == m_fbcFrameCount )
	{
		Sleep(10);
	}
	m_fbcLastFrameCount = m_fbcFrameCount;


	DWORD dwResult = WaitForSingleObject( m_fbBufferMutex, INFINITE);
	if( dwResult != WAIT_OBJECT_0 )
	{
		ta_log("\t\tFBC ERR fbc Buffer mutex wait over 500 milsec (%d) on onFBC_screendata(OnUpdateComplete)", m_fbBufferMutex);
	}
	if( m_fbcFullSize == 1 || m_opt_screen == 0 /*! h264*/)
	{
		//pra->ta_log("FBC buffer send(%d)bytes start", size);
		WaitForSingleObject( m_clientOutputMutex, INFINITE);
		if( m_devStatus & TDC_STATUS_SCREENON)
		{
			write_sendbuffer(m_clientSock, RES_SCREENBUF, m_fbcFullSize, (BYTE*)m_fbcHandle, m_fbcSize);
		}
		else {
			write_sendbuffer(m_clientSock, RES_SCREENBUF, 0, 0, 0);
		}
		ReleaseMutex(m_clientOutputMutex);
	}
	ReleaseMutex(m_fbBufferMutex);
	//pra->ta_log("FBC buffer send(%d)bytes end", size);
}

int RA::OnStateChanged()
{
	if( m_devStateChanged)
	{
		if( m_clientSock != INVALID_SOCKET)
		{
			m_devStateChanged = 0;

			 RUICMD_STATECHANGED cmd;
			 cmd.cmd = Swap32(SVR_STATECHANGED);
			 cmd.idx = Swap32(0);
			 cmd.lsize = Swap32(sizeof(RUICMD_STATECHANGED));
			 cmd.state = Swap16(m_devStatus);
			//add_header_sendbuffer(SVR_STATECHANGED,0,2);
		
			//USHORT s = m_devOrientation;
			//s = Swap16(s);
			//add_sendbuffer((BYTE*)&s, 2);

			//return flush_sendbuffer(m_clientSock);
			WaitForSingleObject( m_clientOutputMutex, INFINITE);
			if( WriteExact(m_clientSock, (const char*)&cmd, sizeof(RUICMD_STATECHANGED)) <= 0 )
			{
				ReleaseMutex(m_clientOutputMutex);
				return 0;
			}
			ReleaseMutex(m_clientOutputMutex);

		}
	}
	return 1;
}

int RA::onMouseEvent(RUICMD* cmd)
{
	RUICMD_MOUSEEVENT* pcmd = (RUICMD_MOUSEEVENT*)cmd;

	USHORT x = Swap16(pcmd->x);
	USHORT y = Swap16(pcmd->y);
	USHORT mode = Swap16(pcmd->mode);

	mouse_event(x, y, mode);
	return 1;
}

int RA::onKeyEvent(RUICMD* cmd)
{
	RUICMD_KEYEVENT* pcmd = (RUICMD_KEYEVENT*)cmd;
	
	UINT key = Swap32(pcmd->key);
	USHORT mode = Swap16(pcmd->mode);

	key_event(key, mode);

	return 1;
}

void RA::logreceiver(RA* pra, void* pData, BYTE* buf, int len)
{
	if( !pra || pra->m_clientSock == INVALID_SOCKET)
		return;

	WaitForSingleObject( pra->m_clientOutputMutex, INFINITE);
	pra->write_sendbuffer(pra->m_clientSock, RES_LOGCAT, 0, buf, len);
	ReleaseMutex(pra->m_clientOutputMutex);
}

int RA::onLogcat(RUICMD* cmd)
{
	if( cmd->lsize == 0 )
	{
		ta_log("\t\tstop logcat");
		stop_log();
		SetIncomingLogCallback(0,0);
	}
	else {
		ta_log("\t\tstart logcat");
		start_log((char*)&cmd->body);
		SetIncomingLogCallback(logreceiver, 0);
	}

	return 1;
}

int RA::svr_disconnectnotify(UINT result)
{
	RUICMD cmd;
	cmd.cmd = Swap32(SVR_DISCONNECT);
	cmd.idx = Swap32(result);
	cmd.lsize = Swap32(RUICMD_HEADER);

	int nRtn;

	WaitForSingleObject( m_clientOutputMutex, INFINITE);
	nRtn = WriteExact(m_clientSock, (const char*)&cmd, RUICMD_HEADER);
	ReleaseMutex(m_clientOutputMutex);

	return nRtn;
}
int RA::res_AppInst(UINT result, char * str)
{
	RUICMD cmd;
	int len = 0;
	if( str && *str)
		len = strlen(str);

	cmd.cmd = Swap32(RES_APP_INST);
	cmd.idx = Swap32(result);
	cmd.lsize = Swap32(RUICMD_HEADER + len);

	int nRtn = WriteExact(m_clientSock, (const char*)&cmd, RUICMD_HEADER);
	if( len )
		WriteExact(m_clientSock, (const char*)str, len);
	ReleaseMutex(m_clientOutputMutex);
	return nRtn;
}

int RA::onCliGetDevInfo(RUICMD* pcmd)
{
	char result[1024];
	UINT resCmd;
	switch( pcmd->cmd )
	{
	case REQ_GETBUILD_MANUFACTURE:
		resCmd = RES_GETBUILD_MANUFACTURE;
		lstrcpy(result, m_devManufacture);
		break;
	case REQ_GETBUILD_MODEL:
		resCmd = RES_GETBUILD_MODEL;
		lstrcpy(result, m_devModel);
		break;
	case REQ_GETBUILD_SERIAL:
		resCmd = RES_GETBUILD_SERIAL;
		lstrcpy(result, m_devBuildSerial);
		break;
	case REQ_GET_BUILD_VERSION_RELEASE:
		resCmd = RES_GET_BUILD_VERSION_RELEASE;
		lstrcpy(result, m_devBuild);
		break;
	case REQ_GET_BUILD_VERSION_SDK_INT:
		resCmd = RES_GET_BUILD_VERSION_SDK_INT;
		lstrcpy(result, m_devSDKVer);
		break;
	}

	RUICMD cmd;

	int len = strlen(result);
	cmd.cmd = Swap32(resCmd);
	cmd.lsize = Swap32(RUICMD_HEADER + len);

	WaitForSingleObject( m_clientOutputMutex, INFINITE);
	WriteExact(m_clientSock, (const char*)&cmd, RUICMD_HEADER);
	WriteExact(m_clientSock, (const char*)result, len);
	ReleaseMutex(m_clientOutputMutex);

	return 1;

}