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
#include "RUIServerPeer.h"

CRITICAL_SECTION CriticalSection;
#define SLEEP( mil ) { int n = mil; while(n> 0 ) { Sleep(100); DummyMessageLoop(m_UIWnd->GetSafeHwnd()); n-= 100; } }
void CALLBACK raTimerProc(void* lpParametar, BOOLEAN TimerOrWaitFired)
{
	// This is used only to call QueueTimerHandler
	// Typically, this function is static member of CTimersDlg
	RA* obj = (RA*) lpParametar;
	obj->QueueTimerHandler();
}

void DummyMessageLoop(HWND hwnd)
{
	MSG msg;
	while(PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
		if( !::IsDialogMessage(hwnd, &msg) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}



RA::RA(CWnd* pWnd)
{
		// Initialise winsock
	WORD wVersionRequested = MAKEWORD(2, 0);
	WSADATA wsaData;
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		PostQuitMessage(1);
	}
	m_UIWnd = pWnd;
	ASSERT(pWnd != NULL);

	m_state = TA_STATE_INIT;
	lstrcpy(m_taVersion, "TEST AGENT 001");
	m_ModulePath[0] = 0;

	for(int i = 0; i < MAX_TD; i++)	
		m_devSerial[i][0] = 0;

	for(int j=0; j<10; j++)
	{
		m_prohibitPackage[j][0] = 0;
		m_prohibitPhoneNum[j][0] = 0;;
	}
	m_bAutoStart = 0;
	m_bForceMonkey = 0;
	m_bAutoReInit = 1;
	m_bCheckEventReturn = 1;
	m_bFirstRun = 0;
	m_devIndex = -1;
	m_audioIndex = -1;
	m_audioName[0] = 0;
	lstrcpy(m_taSerial, "TA00000001");

	m_devExternalIP = 0;
	m_devExternalPort = PORT_EXT_LISTEN; //5910;

	m_devModel[0] = 0;
	m_devManufacture[0] = 0;
	m_devBuild[0] = 0;
	m_devSDKVer[0] = 0;
	m_devBuildSerial[0] = 0;
	m_devIMEI[0] = 0;
	m_devSIMSerial[0] = 0;
	m_devUSIM[0] = 0;
	m_devPhoneNO[0] = 0;
	m_devBattLvl = 0;
	m_devTemperature = 0;
	
	m_listenSock = INVALID_SOCKET;
	m_listenPort = PORT_EXT_LISTEN;
	m_listenInterface.s_addr = 0;
	m_clientSock = INVALID_SOCKET;
	m_clientReserveID[0] = 0;

	m_fbcsock = INVALID_SOCKET;
	m_monkeysock = INVALID_SOCKET;
	m_tdcsock = INVALID_SOCKET;

	m_tickHandle = 0;
	m_monkeyProcess = 0;
	m_fbcProcess = 0;
	//m_devICS = 0;
	m_devOrientation = 0;
	m_devStateChanged = 1;

	m_fbcHandle = 0;
	m_pIStream = NULL;
	m_screenshotname[0] = 0;

	m_hlistenThread = NULL;
	m_hclientThread = NULL;
	m_dwlistenThreadID = 0;

	m_hfbcThread = NULL;

	m_hlogThread = NULL;

	m_htdcThread = NULL;
	m_dwtdcThreadID = 0;

	m_fbUpdateEvent = 0;
	m_fbBufferMutex = 0;
	m_fbOutputMutex = 0;
	m_clientOutputMutex = 0;
	m_fbStartListen = 0;
	
	//m_UpdateCompleteCallback = NULL;
	//m_UpdateCompleteData = NULL;
	m_AdminUpdateCompleteCallback = NULL;
	m_AdminUpdateCompleteData = NULL;
	m_TAStateReportCallback = NULL;
	m_TAStateReportData = NULL;

	m_IncomingLog = NULL;
	m_IncomingLogData = NULL;
	m_hChildStd_OUT_Rd = NULL;
	m_hChildStd_OUT_Wr = NULL;
	m_logProcessID = 0;
	m_IncomingTALog = NULL;
	m_IncomingTALogData = NULL;

	m_taLog[0] = NULL;
	m_temppath[0] = NULL;
	m_AppInstFD = NULL;
	m_AppInst[0] = 0;

	m_bShowProgress = 0;

	//skvideo
	m_bUseUDP = FALSE; // TCP
	m_nBitRate = 150000; //500000;
	m_bStreamingVideo = 0;
	m_bStreamingAudio = 0;
	m_bServerStart = 0;

	// client option
	m_opt_screen = 0;	// 0 - Image, 1 - H.264
	m_opt_h264_quality = 0; // 1-low, 0-normal, 2-high
	m_opt_audio = 0;	// 0 - off, 1- on
	m_ftp = 0;
	m_bFullSizeRequest = 0;
	m_bInitRequired = 0;

	InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x80000400);
}

RA::~RA()
{
	if( m_state < TA_STATE_SERVICE_STOP )
		stop_ta_service();
	DeleteCriticalSection(&CriticalSection);

	// Clean up winsock
	WSACleanup();

}

void RA::set_state(UINT st)
{
	m_state = st;
}
BOOL RA::DirectoryExists(LPCTSTR szPath)
{
  DWORD dwAttrib = GetFileAttributes(szPath);

  return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
         (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
int RA::init_ra()
{

	getModulePath(m_ModulePath,MAX_PATH);

	get_log_filename(m_taLog);
	// -- move to get log file name
	//lstrcpy(m_taLog, m_ModulePath);
	//lstrcat(m_taLog, "\\log\\ta.log");

	//if( m_taLog[0] != 0 ) {
	//	FILE* f = NULL;
	//	fopen_s(&f, m_taLog, "a");
	//	if( f == NULL )
	//	{
	//		char dir[MAX_PATH];
	//		lstrcpy(dir, m_ModulePath);
	//		lstrcat(dir, "\\log");
	//		CreateDirectoryA(dir, NULL);
	//		fopen_s(&f, m_taLog, "a");
	//		if( f )
	//			fclose(f);
	//	} else {
	//		fclose(f);
	//	}
	//}

	m_temppath;
	lstrcpy(m_temppath, m_ModulePath);
	lstrcat(m_temppath, "\\temp");

	if( false == DirectoryExists(m_temppath) ) {
			CreateDirectoryA(m_temppath, NULL);
	}

	ta_log("===========================================================================");
	ta_log("[TA_STATE_INIT]");
	set_state(TA_STATE_INIT);
	ta_log("TA Module Path:%s", m_ModulePath);
	ta_log("TA Log file: %s",m_taLog);

	ta_log("[TA_STATE_INIT_PHASE1]"); //TA Initializing adb channel");
	set_state(TA_STATE_INIT_PHASE1);

	// wait for devices
	{
		ShowProgress(1, "TA : Wait for device", 60);
		//ta_log("Wait for device : 60 sec");

		//HANDLE hAdb = adb_command("wait-for-device");
		//DWORD dwResult = WaitForSingleObject(hAdb,1000 * 120);
		//if( dwResult != WAIT_OBJECT_0 )
		//{
		//	ta_log("failed to wait for device");
		//	ShowProgress(0);
		//	return 0;
		//}
		char buf[64];
		memset(buf,0,64);
		cmd_adb("wait-for-device", 60,buf, 64);
		ShowProgress(0);

		if( lstrlen(buf) == 0 )
		{
			ta_log("failed to wait for device on init_ra but ignore it");
			//return 0;
		}
	}

	//if( init_adb() <= 0 )
	//{
	//	ta_log("ERR Init adb failed.");
	//	return 0;
	//}

	ta_log("[TA_STATE_INIT_PHASE2]"); //TA starting adb server");
	set_state(TA_STATE_INIT_PHASE2);
	if( cmd_adb("start-server") <= 0 )
	{
		ta_log("ERR adb start-server failed on init_ra");
		return 0;
	}

	ta_log("[TA_STATE_INIT_PHASE3]"); //TA get devices");
	set_state(TA_STATE_INIT_PHASE3);

	ShowProgress(1, "TA : Scan Devices", 20);
	int nDev = ScanDevices();
	if( nDev == 0 )
	{
		ta_log("ScanDevices failed on init_ra: wait more 5 sec");
		//Sleep(5000);
		SLEEP(5000);
		nDev = ScanDevices();
	}
	if( nDev == 0 )
	{
		ta_log("ScanDevices failed on init_ra: wait more 10 sec");
		SLEEP(10000);
		nDev = ScanDevices();
	}
	ta_log("Found %d devices : on init_ra", nDev);
	ShowProgress(0);

	load_ini();
	save_ini();
	ta_log("[TA_STATE_INIT_DONE]");
	set_state(TA_STATE_INIT_DONE);

	::CreateTimerQueueTimer(&m_tickHandle, NULL, raTimerProc, this, 0, 1000, WT_EXECUTEINTIMERTHREAD);

	m_state_tick = 0;
	return 1;

}

void RA::QueueTimerHandler() // called every 1000 milliseconds
{
	m_state_tick++;
	//update_req();
	ScheduledDeviceStatusToPOC();

	DWORD dwCurrent = timedateToDword();
	// check reservation id validity
	if( m_state == TA_STATE_CLIENT_CONNECTED )
	{
		if(  m_clientValidStart > 0 && m_clientValidEnd > 0 )
		{
			if( dwCurrent < m_clientValidStart ||
				m_clientValidEnd < dwCurrent )
			{
				ta_log("[TA_STATE_CLIENT_TIMEOUT]");
				set_state(TA_STATE_CLIENT_TIMEOUT);
				svr_disconnectnotify(2); // timeout
				DeviceStatusToPOC(4, 2, "Client Timeout Disconnect");
				ClientStatusToPOC(m_clientReserveID, 4, 2, "Client Timeout Disconnect");
				ShutdownSocket(m_clientSock);
				m_clientSock = INVALID_SOCKET;


				//Video_StopServer();
				//Video_StartServer();
				//Video_StopStreamingThread();
				//m_TCPSockServer.StopServer();
				//return;

			}
			//else if ( m_clientValidEnd - dwCurrent == 10) // 2min before timeout
			//{
			//	svr_disconnectnotify(1); // timeout
			//}
		}
	}
	else
	{
		if( m_bInitRequired && m_bInitRequiredTime > 0)
		{
			if( m_bInitRequiredTime < dwCurrent )
			{
				ReInit();
			}
		}
#ifndef REDECON		// only for remote test : reinit idle device 
		else if( m_state >= TA_STATE_READY && m_state < TA_STATE_AFTER_CLIENT_REINIT && m_bAutoReInit )
		{
			SYSTEMTIME st;
			::GetLocalTime(&st);

			// every odd hour 50 minute do reinit ex)1:50, 3:50, 5:50
			if( (st.wMinute %= 5) == 0 && st.wSecond < 2)
			{
				TRACE("wake up event\n");
				this->key_wake();
			}
			//if( st.wHour % 2 == 1  && st.wMinute == 50 )
			//{
			//	m_bInitRequired = 1;
			//	ReInit();
			//}
		}
#endif
	}

	TAStateReport(this, 0);
}


int RA::ScanDevices()
{

	char buf[2048];
	int nDev;

	for(int i = 0; i < MAX_TD; i++)	
		m_devSerial[i][0] = 0;

	CWaveInDevice* pWaveDev;

	while( pWaveDev = m_waveInDeviceList.GetFirst() )
	{
		m_waveInDeviceList.Remove(pWaveDev);
	}
	m_devIndex = -1;
	m_audioIndex = -1;

	m_threadAudio.EnumWaveInDevice(EnumWaveInDeviceCallback, (LPVOID) this);
	ta_log("ScanDevices:Found %d WaveIn devices", m_waveInDeviceList.GetCount());

	if( (nDev = getdevices_adb(buf)) <= 0 )
	{
		ta_log("ScanDevices:WARN get devices return(%d)", nDev);
	}
	char * d = buf; char *name;
	
	// remove by camel 2012-11-19 for service safety
	//if( nDev == 1 )
	//	m_devIndex = 0;

	nDev = 0;
	while ( ( name = strtok_s(d, "|", &d)) != NULL )
	{
		ta_log("\tScanDevices:found devices:%s",name);
		lstrcpy(m_devSerial[nDev], name);
		nDev++;
		if(nDev > MAX_TD) break;	// check only 4 devices max
	}
	return nDev;
}

void RA::load_ini()
{
	char inifile[MAX_PATH];
	sprintf_s(inifile, "%s\\%s", m_ModulePath, RUIINI);

	ta_log("\tTA load ini (%s)", inifile);
	char name[256];
	GetPrivateProfileString("TA", "taserial", "TA00000001", name, 256, inifile);
	lstrcpy(m_taSerial,name);

	GetPrivateProfileString("TA", "devserial", "", name, 256, inifile);
	m_bAutoStart = GetPrivateProfileInt("TA", "autostart", 0, inifile);

	for( int i = 0; i < MAX_TD; i++) {
		if( m_devSerial[i][0] && lstrcmpi(m_devSerial[i], name) == 0)
		{
			m_devIndex = i;
			break;
		}
	}

	if( m_devIndex < 0 ) // device not found
		m_bAutoStart = 0;
	
	m_bFirstRun = GetPrivateProfileInt("TA", "firstrun", 1, inifile);

	GetPrivateProfileString("TA", "audioIndex", "", name, 128, inifile);
	int nWaveIndex = 0;
	m_audioIndex = -1;
	m_audioName[0] = 0;
	CWaveInDevice* pDev = m_waveInDeviceList.GetFirst();
	while( pDev )
	{
		if( lstrcmpi(pDev->GetDeviceName(), name) == 0 )
		{
			m_audioIndex = nWaveIndex;
			lstrcpy(m_audioName, pDev->GetDeviceName());
			break;
		}
		pDev = m_waveInDeviceList.GetNext(pDev);
		nWaveIndex++;
	}

	m_bForceMonkey = GetPrivateProfileInt("TA", "forcemonkey", 0, inifile);
	m_bAutoReInit = GetPrivateProfileInt("TA", "AutoReInit", 1, inifile);
	m_bCheckEventReturn = GetPrivateProfileInt("TA", "CheckEventReturn", 1, inifile);
	m_devExternalIP = (DWORD)GetPrivateProfileInt("TA", "externalip", 0, inifile);
	m_devExternalPort = GetPrivateProfileInt("TA", "externalport", PORT_EXT_LISTEN, inifile);

	for(int j=0; j<10; j++)
	{
		sprintf(name, "prohibitpkg%d", j);
		GetPrivateProfileString("TA", name,"",	m_prohibitPackage[j], 128, inifile);
		sprintf(name, "prohibitphonenum%d", j);
		GetPrivateProfileString("TA", name,"",	m_prohibitPhoneNum[j], 128, inifile);
	}
}

void RA::save_ini()
{
	char inifile[MAX_PATH];
	char intbuf[64];
	sprintf_s(inifile, "%s\\%s", m_ModulePath, RUIINI);

	ta_log("\tTA save ini (%s)", inifile);
	WritePrivateProfileString("TA", "taserial", m_taSerial, inifile);
	if( m_devIndex >= 0)
		WritePrivateProfileString("TA", "devserial", m_devSerial[m_devIndex], inifile);

	sprintf_s(intbuf, "%d", m_bFirstRun);
	WritePrivateProfileString("TA", "firstrun", intbuf, inifile);

	if( m_audioIndex >= 0 )
	{
		WritePrivateProfileString("TA", "audioIndex", m_audioName, inifile);
	}

	sprintf_s(intbuf, "%d", m_bAutoStart);
	WritePrivateProfileString("TA", "autostart", intbuf, inifile);
	
	sprintf_s(intbuf, "%d", m_bForceMonkey);
	WritePrivateProfileString("TA", "forcemonkey", intbuf, inifile);

	sprintf_s(intbuf, "%d", m_bAutoReInit);
	WritePrivateProfileString("TA", "AutoReInit", intbuf, inifile);

	sprintf_s(intbuf, "%d", m_bCheckEventReturn);
	WritePrivateProfileString("TA", "CheckEventReturn", intbuf, inifile);

	sprintf_s(intbuf, "%d", m_devExternalIP);
	WritePrivateProfileString("TA", "externalip", intbuf, inifile);

	sprintf_s(intbuf, "%d", m_devExternalPort);
	WritePrivateProfileString("TA", "externalport", intbuf, inifile);

	for(int j=0; j<10; j++)
	{
		sprintf(intbuf, "prohibitpkg%d", j);
		WritePrivateProfileString("TA", intbuf,	m_prohibitPackage[j], inifile);
		sprintf(intbuf, "prohibitphonenum%d", j);
		WritePrivateProfileString("TA", intbuf,	m_prohibitPhoneNum[j], inifile);
	}
}


void RA::get_log_filename(char* logfilename)
{
	//char logfilename[MAX_PATH];
	if( logfilename == 0 ) return;

	time_t timer;
	struct tm t;
	timer = time(NULL);
	localtime_s(&t, &timer);
	char buf[MAX_PATH];
	sprintf_s(buf, "%s\\log\\%04d%02d%02d.ta.log",
	m_ModulePath,
	t.tm_year+1900, t.tm_mon+1, t.tm_mday);

	// check exist
	if( buf[0] != 0 ) {
		FILE* f = NULL;
		fopen_s(&f, buf, "a");
		if( f == NULL )
		{
			char dir[MAX_PATH];
			lstrcpy(dir, m_ModulePath);
			lstrcat(dir, "\\log");
			CreateDirectoryA(dir, NULL);
			fopen_s(&f, buf, "a");
			if( f )
				fclose(f);
		} else {
			fclose(f);
		}
	}
	strcpy(logfilename, buf);
}

void RA::check_log_filename()
{
	char temp_filename[MAX_PATH];
	get_log_filename(temp_filename);

	if( strcmp(m_taLog, temp_filename) != 0 )
	{
		//ta_log("change log filename(%s to %s)", m_taLog, temp_filename);
		strcpy(m_taLog, temp_filename);
	}

}

void RA::check_log_size()
{
	if( m_taLog[0] != 0 ) {
		FILE* f = NULL;
		fopen_s(&f, m_taLog, "a");
		if( f == NULL )
		{
			char dir[MAX_PATH];
			lstrcpy(dir, m_ModulePath);
			lstrcat(dir, "\\log");
			CreateDirectoryA(dir, NULL);
			fopen_s(&f, m_taLog, "a");
			if( f )
				fclose(f);
		} else {
			fclose(f);
		}
	}

	// check ta log file size
	if( m_taLog[0] != 0 ) {
		FILE* f = NULL;
		fopen_s(&f, m_taLog, "a");
		if( f == NULL )
			return;

		fseek(f, 0, SEEK_END);
		long size = ftell(f);

		if( size > 4*1024*1024 ) // 4M byte
		{
			time_t timer;
			struct tm t;
			char buf[MAX_PATH];
			timer = time(NULL);
			localtime_s(&t, &timer);
			sprintf_s(buf, "%s\\log\\%04d%02d%02d%02d%02d%02d_ta.log",
				m_ModulePath,
				t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
			
			fclose(f);
			ta_log("TA log file size exceed 4M bytes. backup to (%s)", buf);
			
			rename(m_taLog, buf);

			fopen_s(&f, m_taLog, "a");
		}

		fclose(f);
	}
}

void RA::exit_ra()
{
	m_IncomingLog = NULL;
	m_IncomingTALog = NULL;

	if( m_tickHandle )
	{
		DeleteTimerQueueTimer(NULL, m_tickHandle, NULL); 
		m_tickHandle = 0;
	}

	CWaveInDevice* pWaveDev;
	while( pWaveDev = m_waveInDeviceList.GetFirst() )
	{
		m_waveInDeviceList.Remove(pWaveDev);
	}

	if( m_state < TA_STATE_SERVICE_STOP )
		stop_ta_service();

	//stop_adb();
	set_state(TA_STATE_EXIT);

	ta_log("TA Exit -------------------------------------------------------------------");
}
/*
int RA::reconnect_fbc_svc()
{
	stop_ta_service();
	Sleep(2000);
	connect_fbc_svc();
	return 1;
}
*/

int RA::start_ta_service(bool bReinit)
{	
	// check ta log file size
	//check_log_size();

	if( !bReinit )
	{
		//RegisterToPOC();
		DeviceStatusToPOC(0, 0, "Initial state");	// TA_STATE_INIT
		DeviceStatusToPOC(0, 4, "Init Done");		// TA_STATE_INIT_DONW
		DeviceStatusToPOC(1, 0, "Start Service");	// TA_STATE_START_SERVICE
	}
	ta_log("[TA_STATE_START_SERVICE]");

	//stop_adb();
	//init_adb();

	set_state(TA_STATE_START_SERVICE);
	if( m_devIndex < 0 ) {
		ta_log("FBC ERR device serial is null - start_ta_service");

		DeviceStatusToPOC(8, 0, "Device not found error");	// TA_STATE_ERR_DEVNOTFOUND

		stop_ta_service();
		return 0;
	}

	m_fbcsock = INVALID_SOCKET;
	m_monkeysock = INVALID_SOCKET;
	m_tdcsock = INVALID_SOCKET;
	//m_bServerStart = 0;

	ta_log("[TA_STATE_START_SERVICE_PHASE1]");
	if( !bReinit )
	{
		DeviceStatusToPOC(1, 1, "Start Service Phase 1");
	}

	set_state(TA_STATE_START_SERVICE_PHASE1);
	ta_log("[WAIT FOR DEVICE] - start_ta_service");

	ShowProgress(1, "TA : Wait for device", 60);
	//HANDLE hAdb = adb_command("wait-for-device");
	//DWORD dwResult = WaitForSingleObject(hAdb,1000 * 120);
	char buf[64];
	memset(buf,0,64);
	cmd_adb("wait-for-device", 60,buf, 64);
	ShowProgress(0);

	if( lstrlen(buf) == 0 )
	{
		ta_log("[failed to wait for device] - start_ta_service");
		DeviceStatusToPOC(8, 1, "Device not found Error");	// TA_STATE_ERR_SERVICE
		set_state(TA_STATE_ERR_DEVNOTFOUND);
		stop_ta_service();
		return 0;
	}
	//if( dwResult != WAIT_OBJECT_0 )
	//{
	//	ta_log("[failed to wait for device]");
	//	DeviceStatusToPOC(8, 1, "Device not found Error");	// TA_STATE_ERR_SERVICE
	//	set_state(TA_STATE_ERR_DEVNOTFOUND);
	//	stop_ta_service();
	//	return 0;
	//}
	ShowProgress(1, "TA : Starting Service", 60);
	//ta_log("Initilize wait 10 seconds");
	SLEEP(2000);
	m_sdkVersion = getSDKVersion();
	ta_log("Device SDK Version:%d", m_sdkVersion);
	// --- init FBC
	if( init_fbc() <= 0 )
	{
		ta_log("FBC ERR init failed");
		DeviceStatusToPOC(8, 1, "FBC Service Error");	// TA_STATE_ERR_SERVICE

		stop_ta_service();
		return 0;
	}
	SLEEP(1);
	// --- init EVT
	if( init_evt() <= 0 )
	{
		ta_log("EVT ERR init failed");
		DeviceStatusToPOC(8, 1, "EVT Service Error");	// TA_STATE_ERR_SERVICE

		stop_ta_service();
		return 0;
	}
	SLEEP(1);
	// --- init TDC
	if( init_tdc() <= 0 )
	{
		ta_log("TDC ERR init failed");
		DeviceStatusToPOC(8, 1, "TDC Service Error");	// TA_STATE_ERR_SERVICE

		stop_ta_service();
		return 0;
	}
	SLEEP(1);

	//ta_log("FBC wait for 2000 milsec");
	//Sleep(2000);

	ta_log("[TA_STATE_START_SERVICE_PHASE2]");
	if( !bReinit )
	{
		DeviceStatusToPOC(1, 2, "Start Service Phase 2");
	}

	set_state(TA_STATE_START_SERVICE_PHASE2);
	// --- Connect FBC
	if( connect_fbc() <= 0 )
	{
		if( restart_fbc() <= 0 ) // second try
		{
			if( restart_fbc() <= 0 ) // third try
			{
				ta_log("FBC ERR connection failed");
				DeviceStatusToPOC(8, 1, "FBC Connect Service Error");	// TA_STATE_ERR_SERVICE

				stop_ta_service();
				return 0;
			}
		}
	}
	SLEEP(1);
	// --- Connect EVT
	if( connect_evt() <= 0 )
	{
		if( restart_evt() <= 0 )
		{
			if( restart_evt() <= 0 )
			{
				ta_log("EVT ERR connection failed");
				DeviceStatusToPOC(8, 1, "EVT Connect Service Error");	// TA_STATE_ERR_SERVICE

				stop_ta_service();
				return 0;
			}
		}
	}
	SLEEP(1);

	// --- Connect TDC
	if( connect_tdc() <= 0 )
	{
		if( restart_tdc() <= 0 )
		{
			if( restart_tdc() <= 0 )
			{
				ta_log("TDC ERR connection failed");
				DeviceStatusToPOC(8, 1, "TDC Connect Service Error");	// TA_STATE_ERR_SERVICE

				stop_ta_service();
				return 0;
			}
		}
	}
	SLEEP(1);

	ta_log("[TA_STATE_START_SERVICE_PHASE3]");
	if( !bReinit )
	{
		DeviceStatusToPOC(1, 3, "Start Service Phase 3");
	}

	set_state(TA_STATE_START_SERVICE_PHASE3);

	m_tickRecvSum = 0;	// tick for framebuffer get (server capability)
	m_frameGetSum = 0;	// count frameget
	m_frameLastSend = 0;
	m_byteLastSend = 0;
	m_byteRecvSum = 0;	// BPS for framebuffer get
	m_tickSendSum = 0;	// tick for framebuffer update request interval (maybe client fps)
	m_serverFps = 0;
	m_serverBps = 0;
	m_clientFps = 0;
	m_clientBps = 0;
	m_sendBufIdx = 0;
	m_devStatus = 0;

	m_ftp = 0;

	fnIncomingLog tempIncomingLog = m_IncomingTALog;
	m_IncomingTALog = NULL;
	// fbc thread
	m_fbUpdateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_fbBufferMutex = CreateMutex(NULL, FALSE, NULL);
	m_fbOutputMutex = CreateMutex(NULL, FALSE, NULL);
	m_clientOutputMutex =CreateMutex(NULL, FALSE, NULL);
	m_fbStartListen = CreateEvent(NULL, TRUE, FALSE, NULL);


	m_hfbcThread = CreateThread(NULL, 0, ruifbcThread, this, 0, &m_dwfbcThreadID);
	CloseHandle(m_hfbcThread);
	init_screen();

	m_htdcThread = CreateThread(NULL, 0, ruitdcThread, this, 0, &m_dwtdcThreadID);
	CloseHandle(m_htdcThread);


	//ta_log("FBC wait for tdc getDeviceInforComplete 2000msec");
	//Sleep(2000);


	//ta_log("FBC wait for 1000 milsec");
	//Sleep(2000);
	// tdc thread


	m_IncomingTALog = tempIncomingLog;

	// initialize listener socket
	m_listenPort =  m_devExternalPort; //PORT_LISTEN + m_devIndex*10;
	//InitListener();


	//Video start server
	if( m_bServerStart == 0 )
	{
		ta_log("[H.264 Server Initialize]");
		InitIppExtensionParam_Connect(&m_ieParam);
		m_ieParam.m_pEncoder = (void*) m_threadEncoder.GetEncoder();

		m_TCPSockServer.SetIppExtensionParam(&(m_ieParam));
		m_TCPSockServer.SetPort(m_listenPort+1);

		for (int i = 0; i < MAX_CLIENT_CONNECTION; i++)
			m_TCPSockServer.AddPeer(new RUIServerPeer(m_UIWnd));

		m_UDPSockServer.SetIppExtensionParam(&m_ieParam);

		m_threadEncoder.SetIppExtensionParam(&(m_ieParam));
		m_threadEncoder.SetMessageWindow(m_UIWnd);

	}
	//ta_log("\t\tvideo server start");
	//Video_StartServer();
	//ta_log("\t\tvideo server start - done");

	tdc_getDeviceInfo();
	ta_log("\tFBC wait for tdc getDeviceInfoComplete");
	int nWait = 30;
	DWORD dwResult = WaitForSingleObject(m_fbStartListen, 1000);
	while ( dwResult != WAIT_OBJECT_0 && nWait-- > 0)
	{
		SLEEP(100);
		ta_log("FBC wait for tdc getDeviceInfoComplete(%d)", nWait);
		dwResult = WaitForSingleObject(m_fbStartListen, 1000);
	}
	ResetEvent(m_fbStartListen);

	if( nWait <= 0 )
	{
		ta_log("TDC ERR tdc service - failed to wait for tdc getDeviceInfoComplete");
		DeviceStatusToPOC(8, 1, "Service Error"); // TA_STATE_ERR_SERVICE
		stop_ta_service();
		return 0;
	}

	// listener thread
	m_hlistenThread = CreateThread(NULL, 0, ruilistenThread, this, 0, &m_dwlistenThreadID);
	CloseHandle(m_hlistenThread);

	//ta_log("start video encoder");
	RegisterToPOC();
	ta_log("success start_ta_service");
	//set_state(TA_STATE_READY);

	ShowProgress(0);

	return 1;
}


int RA::stop_ta_service()
{
	ShowProgress(0);
	DWORD dwResult;

	if( m_state > TA_STATE_SERVICE_STOP)
		return 1;

	ta_log("[TA_STATE_SERVICE_STOP]");
	DeviceStatusToPOC(6, 0, "Service stop");

	set_state(TA_STATE_SERVICE_STOP);

	ta_log("[TA_STATE_SERVICE_STOP_PHASE1]");
	set_state(TA_STATE_SERVICE_STOP_PHASE1);

	if( m_bServerStart )
	{
		ta_log("\tH.264 server stop - stop_ta_service");
		Video_StopServer();
		ta_log("\tH.264 server stop - done");
	}

	if( m_clientSock != INVALID_SOCKET)
	{
		ta_log("\tTA client socket closing");
		ShutdownSocket(m_clientSock);
		m_clientSock = INVALID_SOCKET;
	}

	if( m_listenSock != INVALID_SOCKET)
	{
		ta_log("\tTA listen socket closing");
		ShutdownSocket(m_listenSock);
		m_listenSock = INVALID_SOCKET;
	}

	if( m_tdcsock != INVALID_SOCKET)
	{
		ta_log("\tTA tdc socket closing");
		ShutdownSocket(m_tdcsock);
		m_tdcsock = INVALID_SOCKET;
	}

	if( m_monkeysock != INVALID_SOCKET) {
		ta_log("\tFBC eventinjector socket closing");
		ShutdownSocket(m_monkeysock);
		m_monkeysock = INVALID_SOCKET;
	}

	if (m_fbcsock != INVALID_SOCKET) {	
		ta_log("\tFBC fbc svc closing");
		WriteExact(m_fbcsock, "q", 1);

		ShutdownSocket(m_fbcsock);
		m_fbcsock = INVALID_SOCKET;
	}
	
	if( m_monkeyProcess ) {
		ta_log("\tdelete monkey");
		delete m_monkeyProcess;
		m_monkeyProcess = 0;
	}

	if( m_fbcProcess > 0 ) {
		ta_log("\tFBC fbc process closing");
		TerminateProcess(m_fbcProcess, 0);
		m_fbcProcess = 0;
	}

	if( m_pIStream != 0 ) {
		m_pIStream->Release();
		m_pIStream = 0;
	}

	m_fbcHandle = 0;


	ta_log("[TA_STATE_SERVICE_STOP_PHASE2]");
	set_state(TA_STATE_SERVICE_STOP_PHASE2);	
	if( m_hclientThread ) {
		dwResult = WaitForSingleObject(m_hclientThread, 3000);
		if( dwResult != WAIT_OBJECT_0 )
		{
		//Sleep(1000);
			ta_log("\tFBC client thread terminating");
			TerminateThread(m_hclientThread,0);
		}
		m_hclientThread = 0;
	}
	if( m_hlistenThread ) {
		dwResult = WaitForSingleObject(m_hlistenThread, 3000);
		if( dwResult != WAIT_OBJECT_0 )
		{
			//Sleep(1000);
			ta_log("\tFBC listen thread terminating");
			//CloseHandle(m_hlistenThread);
			TerminateThread(m_hlistenThread,0);
		}
		m_hlistenThread = 0;
	}

	if( m_fbStartListen ) {
		CloseHandle(m_fbStartListen);
		m_fbStartListen = 0;
	}

	if( m_htdcThread ) {
		dwResult = WaitForSingleObject(m_htdcThread, 3000);
		if( dwResult != WAIT_OBJECT_0 )
		{
			ta_log("\tFBC tdc thread terminating");
			TerminateThread(m_htdcThread,0);
		}
		m_htdcThread = 0;
	}
	if( m_hfbcThread ) {
		dwResult = WaitForSingleObject(m_hfbcThread, 3000);
		if( dwResult != WAIT_OBJECT_0 )
		{
			ta_log("\tFBC fbc thread terminating");
			TerminateThread(m_hfbcThread,0);
		}
		m_hfbcThread = 0;
	}
	ta_log("\tFBC logcat closing");
	stop_log();

	ta_log("[TA_STATE_SERVICE_STOP_DONE]");
	set_state(TA_STATE_SERVICE_STOP_DONE);
	return 1;
}


int RA::init_fbc()
{
	char buf[1024];


	ta_log("\tinit_fbc start");
	// FBC
	//ta_log("FBC clear dummy fbcsvc on device(%s)", m_devSerial[m_devIndex]);
	lstrcpy(buf, "shell \"ps -x\" 1> ps.txt && FOR /F \"usebackq tokens=1-8 delims= \" %m IN (`findstr \"ruicapsvc\" ps.txt`) do ( @");

	lstrcat(buf, RUIADB);
	if( m_devIndex >= 0 ) {
		lstrcat(buf," -s \"");
		lstrcat(buf, m_devSerial[m_devIndex]);
		lstrcat(buf, "\" ");
	}
	lstrcat(buf, " shell kill -9 %n )");
	if( cmd_adb(buf) < 0 )
	{
		return 0;
	}


	//ta_log("FBC %d port forwarding", PORT_FBC+ m_devIndex*10);
	sprintf_s(buf, "forward tcp:%d tcp:%d", PORT_FBC+ m_devIndex*10, PORT_FBC);
	if( cmd_adb(buf) < 0 )
	{
		return 0;
	}

	//ta_log("FBC copy fbcsvc on device");
	if( m_sdkVersion > 16 ) { // jelly 4.2
		sprintf_s(buf, "push ruicapsvcj2 /data/local/tmp/ruicapsvc");
	}
	else if( m_sdkVersion > 15 ) { // jelly 4.1
		sprintf_s(buf, "push ruicapsvcj1 /data/local/tmp/ruicapsvc");
	}
	else {
		sprintf_s(buf, "push ruicapsvc /data/local/tmp/ruicapsvc");
	}
	if( cmd_adb(buf) < 0 )
	{
		return 0;
	}

	//ta_log("FBC chmod fbcsvc on device");
	if( cmd_adb("shell chmod 755 /data/local/tmp/ruicapsvc") < 0 )
	{
		return 0;
	}

	//if (m_fbcsock != INVALID_SOCKET) {	
	//	ta_log("\tFBC fbc svc closing");
	//	WriteExact(m_fbcsock, "q", 1);

	//	ShutdownSocket(m_fbcsock);
	//	m_fbcsock = INVALID_SOCKET;
	//}

	if( m_fbcProcess > 0 ) {
		ta_log("FBC fbc process closing - init_fbc");
		TerminateProcess(m_fbcProcess, 0);
		m_fbcProcess = 0;
	}


	//ta_log("FBC run fbcsvc");
	//if( (m_fbcProcess = adb_command("shell /data/local/tmp/ruicapsvc"   /*,1*/ )) < 0 )
	//{
	//	stop_ta_service(); return 0;
	//}

	//SLEEP(1000);

	//TerminateProcess(m_fbcProcess, 0);

	if( cmd_adb("shell /data/local/tmp/ruicapsvc") <= 0 )
	{
		stop_ta_service(); return 0;
	}
	m_fbcProcess = 0;
	ta_log("\tinit_fbc done");
	return 1;
}

int RA::init_evt()
{

	ta_log("\tinit_evt start");
	// EVENTINJECTOR
	char buf[1024];

	if( m_monkeysock != INVALID_SOCKET) {
		//ta_log("\tmonkey socket closing - init_evt");
		ShutdownSocket(m_monkeysock);
		m_monkeysock = INVALID_SOCKET;
	}

	if( m_monkeyProcess ) {
		//ta_log("\tdelete monkey");
		delete m_monkeyProcess;
		m_monkeyProcess = 0;
	}

	//ta_log("FBC clear dummy eventinjector on device");
	lstrcpy(buf, "shell \"ps -x\" 1> ps.txt && FOR /F \"usebackq tokens=1-8 delims= \" %m IN (`findstr \"com.android.commands.monkey\" ps.txt`) do ( @");
	lstrcat(buf, RUIADB);
	if( m_devIndex >= 0 ) {
		lstrcat(buf," -s \"");
		lstrcat(buf, m_devSerial[m_devIndex]);
		lstrcat(buf, "\" ");
	}
	lstrcat(buf, " shell kill -9 %n )");

	
	if( cmd_adb(buf) < 0 )
	{
		return 0;
	}

	//ta_log("EVT %d port forwarding ", PORT_EVENTINJECT+ m_devIndex*10);
	sprintf_s(buf, "forward tcp:%d tcp:%d", PORT_EVENTINJECT+ m_devIndex*10, PORT_EVENTINJECT);
	if( cmd_adb(buf) < 0 )
	{
		return 0;
	}
	
	//ta_log("FBC run eventinjector server to listen port(%d)", PORT_EVENTINJECT);
	sprintf_s(buf, "shell monkey --port %d &", PORT_EVENTINJECT);

	ra_adb * padb = 0;
	
	padb = new ra_adb(this);

	int nrtn = padb->cmd_adb(buf, 3, 0, 0, 1);
	if( nrtn <= 0 )
	{
		delete padb;
		stop_ta_service();
		return 0;
	}
	
	m_monkeyProcess = padb;


	//if( cmd_adb(buf) <= 0 )
	//{
	//	stop_ta_service(); return 0;
	//}


	ta_log("\tinit_evt done");
	return 1;
}

int RA::restart_fbc()
{
	ta_log("\trestart_fbc start");
	
	if( !init_fbc() )
		return 0;

	if( !connect_fbc() )
		return 0;

	ta_log("\trestart_fbc done");
	return 1;
}

int RA::restart_evt()
{
	ta_log("\trestart_evt start");

	if( !init_evt() )
		return 0;

	if( !connect_evt() )
		return 0;


	ta_log("\trestart_evt done");
	return 1;
}
int RA::restart_tdc()
{
	ta_log("\trestart_tdc start");

	if( !init_tdc() )
		return 0;

	if( !connect_tdc() )
		return 0;

	ta_log("\trestart_tdc done");
	tdc_getDeviceInfo();
	return 1;
}

int RA::init_tdc()
{
	// TDC
	ta_log("\tinit_tdc start");
	char buf[1024];

	//ta_log("FBC clear dummy logcat on device(%s)", m_devSerial[m_devIndex]);

	// no more need  CAMEL-20120810
	//lstrcpy(buf, "shell \"ps -x\" 1> ps.txt && FOR /F \"usebackq tokens=1-8 delims= \" %m IN (`findstr \"logcat\" ps.txt`) do ( @");
	//lstrcat(buf, RUIADB);
	//lstrcat(buf, " shell kill -9 %n )");
	//if( cmd_adb(buf) < 0 )
	//{
	//	return 0;
	//}
	//ta_log("FBC copy tdcsvc on device");

	//sprintf_s(buf, "uninstall com.rocode.skrc");
	//if( cmd_adb(buf) < 0 )
	//{
	//	return 0;
	//}

	sprintf_s(buf, "install -r com.rocode.skrc");
	if( cmd_adb(buf) < 0 )
	{
		return 0;
	}


	//ta_log("FBC start tdcsvc on device");
	if( m_sdkVersion > 16 ) { // jelly 4.2
		if( cmd_adb("shell am startservice --user 0 -W -a android.intent.action.RCService") < 0 )
		{
			return 0;
		}
	}
	else {
	if( cmd_adb("shell am startservice -W -a android.intent.action.RCService") < 0 )
	{
		return 0;
	}
	}

	//ta_log("TDC %d port forwarding", PORT_TDC+ m_devIndex*10);
	sprintf_s(buf, "forward tcp:%d tcp:%d", PORT_TDC+ m_devIndex*10, PORT_TDC);
	if( cmd_adb(buf) < 0 )
	{
		return 0;
	}

	SLEEP(1000);
	ta_log("\tinit_tdc done");
	return 1;
}

int RA::connect_fbc()
{
	ta_log("\tconnect_fbc start");
	struct sockaddr_in thataddr;
	int res;

	//ta_log("FBC fbc socket creation");
	m_fbcsock = socket(PF_INET, SOCK_STREAM, 0);

	if (m_fbcsock == INVALID_SOCKET) {
		ta_log("FBC ERR socket creation failed");
		return 0;
	}

	thataddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	thataddr.sin_family = AF_INET;
	thataddr.sin_port = htons(PORT_FBC+ m_devIndex*10);

	res = connect(m_fbcsock, (LPSOCKADDR) &thataddr, sizeof(thataddr));
	if (res == SOCKET_ERROR)
	{
		ta_log("FBC ERR socket connect failed -1");
		SLEEP(1000);
		res = connect(m_fbcsock, (LPSOCKADDR) &thataddr, sizeof(thataddr));
		if (res == SOCKET_ERROR)
		{
			ta_log("FBC ERR socket connect failed -2");
			SLEEP(1000);
			res = connect(m_fbcsock, (LPSOCKADDR) &thataddr, sizeof(thataddr));
			if (res == SOCKET_ERROR)
			{
				int a=WSAGetLastError();
				ta_log("FBC ERR connecion failed (%i) -3", a);
				closesocket(m_fbcsock);
				m_fbcsock = INVALID_SOCKET;
				return 0;
			}
		}
	}

	BOOL nodelayval = TRUE;
	if (setsockopt(m_fbcsock, IPPROTO_TCP, TCP_NODELAY, (const char *) &nodelayval, sizeof(BOOL)))
	{
		closesocket(m_fbcsock);
		m_fbcsock = INVALID_SOCKET;
		ta_log("FBC ERR socket option set failed 1");
		return 0;
	}
	int one = 1;
    if (setsockopt(m_fbcsock, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one)) < 0) {
		closesocket(m_fbcsock);
		m_fbcsock = INVALID_SOCKET;
		ta_log("FBC ERR socket option set failed 2");
        return 0;
    }

	try {
		char version[14];
		recv(m_fbcsock, version, 13, 0);
		version[13] = 0;
		ta_log("FBC Connected to fbc svc(%s)", version);
		//ta_log("FBC Connected to fbc svc server[%s] port %d", version, PORT_FBC+ m_devIndex*10);
	}
	catch (...)
	{
		ta_log("FBC ERR Connect fbc svc::Exception\n");
		closesocket(m_fbcsock);
		m_fbcsock = INVALID_SOCKET;
		return 0;
	}
	//int nonblocking =1;
	//res = ioctlsocket(m_fbcsock, FIONBIO, (unsigned long*) &nonblocking);

	ta_log("\tconnect_fbc done");
	return 1;
}

int RA::connect_evt()
{
	ta_log("\tconnect_evt start");
	//--- monkey
	struct sockaddr_in thataddr;
	int res;

	//ta_log("FBC eventinjector creation");
	m_monkeysock = socket(PF_INET, SOCK_STREAM, 0);

	if (m_monkeysock == INVALID_SOCKET) {
		int a=WSAGetLastError();
		ta_log("EVT ERR socket creation error %i",a);
		return 0;
	}

	thataddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	thataddr.sin_family = AF_INET;
	thataddr.sin_port = htons(PORT_EVENTINJECT+ m_devIndex*10);

	//ta_log("FBC eventinjector connected on port(%d)", PORT_EVENTINJECT+ m_devIndex*10);
	res = connect(m_monkeysock, (LPSOCKADDR) &thataddr, sizeof(thataddr));
	if (res == SOCKET_ERROR)
	{
		ta_log("EVT ERR connecion failed -1");
		SLEEP(1000);
		res = connect(m_monkeysock, (LPSOCKADDR) &thataddr, sizeof(thataddr));
		if (res == SOCKET_ERROR)
		{
			ta_log("EVT ERR connecion failed -2");
			SLEEP(1000);
			res = connect(m_monkeysock, (LPSOCKADDR) &thataddr, sizeof(thataddr));
			if (res == SOCKET_ERROR)
			{
				int a=WSAGetLastError();
				ta_log("EVT ERR connecion failed (%i) -3", a);
				closesocket(m_monkeysock);
				m_monkeysock = INVALID_SOCKET;
				return 0;
			}
		}
	}
	
	BOOL nodelayval = TRUE;
	if (setsockopt(m_monkeysock, IPPROTO_TCP, TCP_NODELAY, (const char *) &nodelayval, sizeof(BOOL)))
	{
		ta_log("EVT ERR socket option set failed 1");
		closesocket(m_monkeysock);
		m_monkeysock = INVALID_SOCKET;
		return 0;
	}
	int one = 1;
    if (setsockopt(m_monkeysock, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one)) < 0) {
		ta_log("EVT ERR socket option set failed 2");
		closesocket(m_monkeysock);
		m_monkeysock = INVALID_SOCKET;
        return 0;
    }
	int nonblocking =1;
	res = ioctlsocket(m_monkeysock, FIONBIO, (unsigned long*) &nonblocking);

	ta_log("\tconnect_evt done");
	return 1;
}

int RA::connect_tdc()
{
	ta_log("\tconnect_tdc start");
	//--- tdc
	struct sockaddr_in thataddr;
	int res;

	//ta_log("FBC tdc creation");
	m_tdcsock = socket(PF_INET, SOCK_STREAM, 0);

	if (m_tdcsock == INVALID_SOCKET) {
		int a=WSAGetLastError();
		ta_log("TDC ERR socket creation error %i",a);
		return 0;
	}

	thataddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	thataddr.sin_family = AF_INET;
	thataddr.sin_port = htons(PORT_TDC + m_devIndex*10);

	//ta_log("FBC tdc connected on port(%d)", PORT_TDC+ m_devIndex*10);
	res = connect(m_tdcsock, (LPSOCKADDR) &thataddr, sizeof(thataddr));
	if (res == SOCKET_ERROR)
	{
		ta_log("TDC ERR connecion failed -1");
		SLEEP(1000);
		res = connect(m_tdcsock, (LPSOCKADDR) &thataddr, sizeof(thataddr));
		if (res == SOCKET_ERROR)
		{
			ta_log("TDC ERR connecion failed -2");
			SLEEP(1000);
			res = connect(m_tdcsock, (LPSOCKADDR) &thataddr, sizeof(thataddr));
			if (res == SOCKET_ERROR)
			{
				int a=WSAGetLastError();
				ta_log("TDC ERR connecion failed %i -3", a);
				closesocket(m_tdcsock);
				m_tdcsock = INVALID_SOCKET;
				return 0;
			}
		}
	}	
	//int nonblocking =1;
	//res = ioctlsocket(m_monkeysock, FIONBIO, (unsigned long*) &nonblocking);

	BOOL nodelayval = TRUE;
	if (setsockopt(m_tdcsock, IPPROTO_TCP, TCP_NODELAY, (const char *) &nodelayval, sizeof(BOOL)))
	{
		ta_log("TDC ERR socket option set failed 1");
        closesocket(m_tdcsock);
		m_tdcsock = INVALID_SOCKET;
		return 0;
	}
	int one = 1;
    if (setsockopt(m_tdcsock, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one)) < 0) {
		ta_log("TDC ERR socket option set failed 2");
        closesocket(m_tdcsock);
		m_tdcsock = INVALID_SOCKET;
        return 0;
    }

	ta_log("\tconnect_tdc done");
	return 1;
}
/*

int RA::Read_from_fbc(BYTE* buff)
{
	if (m_fbcsock == INVALID_SOCKET)
		return 0;

	UINT s;
	if( ReadExact(m_fbcsock, (char*)&s, 4) < 0 )
		return 0;
	USHORT sRotate = (USHORT)Swap32(s);
	if( sRotate != m_devOrientation)
		tdc_send_command(TDC_GETSTATUS);
	m_devOrientation = sRotate;

	unsigned int size;
	if( ReadExact(m_fbcsock, (char*)&size, 4) < 0 )
		return 0;
	size = Swap32(size);
	//TRACE(0, _T("fbc cmd len:%d\n"), size);

	DWORD dwResult = WaitForSingleObject( m_fbBufferMutex, 500);
	if( dwResult != WAIT_OBJECT_0 )
	{
		//ta_log("FBC WARN fbc Buffer mutex wait over 5000 milsec (%d) on Read_from_fbc", m_fbBufferMutex);
	}
	if( ReadExact(m_fbcsock, (char*)buff, size)< 0 ) {
		ReleaseMutex(m_fbBufferMutex);
		return 0;
	}
	ReleaseMutex(m_fbBufferMutex);

	//static int once = 1;
	//if( once )
	//{
	//	FILE* out = fopen("c:\\temp\\test.jpg", "wb");
	//	fwrite(buff, 1, size, out);
	//	fclose(out);
	//	once = 0;
	//}


	return size;
}
*/
void RA::getModulePath(char* buffer, int size)
{
	if (GetModuleFileName(NULL, buffer, MAX_PATH))
	{
		char* p = strrchr(buffer, '\\');
		if (p != NULL)
		{
			*p = '\0';
		}
	}
}