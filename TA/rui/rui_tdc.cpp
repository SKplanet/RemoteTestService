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

int RA::ProcessTDCMessage()
{
	TDCCMD cmd;
	int n;
	

	if( (n=ReadExact(m_tdcsock, (char*)&cmd, TDCCMD_HEADER)) <= 0 ) {
		ta_log("\tTDC ERR ProcessTDCMessage read(%d) header", n);
		return 0;
	}

	cmd.size	= Swap16(cmd.size) - 8;
	cmd.cmd		= Swap16(cmd.cmd);
	cmd.idx		= Swap16(cmd.idx);
	cmd.rtn		= Swap16(cmd.rtn);
	
	cmd.body = NULL;

	if( cmd.size > 0 )
	{
		if( (n=ReadExact(m_tdcsock, (char*)m_tdcBuff, cmd.size)) <= 0 ) {
			ta_log("\tTA ERR ProcessTDCMessage read(%d) body", n);
			return 0;
		}
		m_tdcBuff[cmd.size] = 0;
		cmd.body = m_tdcBuff;
	}
	//TRACE("TDC cmd(%d), size(%d), idx(%d), rtn(%d)\n", cmd.cmd, cmd.size, cmd.idx, cmd.rtn);
	switch( cmd.cmd )
	{
		case TDC_GETSTATUS:
			tdc_on_getStatus(&cmd);
			break;
		
		case TDC_GETBATTEMPERATURE:
			tdc_on_getBatTemperature(&cmd);
			break;
		
		case TDC_GETBATTLEVEL:
			tdc_on_getBatLevel(&cmd);
			break;

		case TDC_GET_RC_VERSION:
		case TDC_GETBUILD_BOARD:
		case TDC_GETBUILD_BOOTLOADER:
		case TDC_GETBUILD_BRAND:
		case TDC_GETBUILD_CPUABI:
		case TDC_GETBUILD_CPUABI2:
		case TDC_GETBUILD_DEVICE:
		case TDC_GETBUILD_DISPLAY:
		case TDC_GETBUILD_FINGERPRINT:
		case TDC_GETBUILD_HARDWARE:
		case TDC_GETBUILD_HOST:
		case TDC_GETBUILD_ID:
		case TDC_GETBUILD_MANUFACTURE:
		case TDC_GETBUILD_MODEL:
		case TDC_GETBUILD_PRODUCT:
		case TDC_GETBUILD_RADIO:
		case TDC_GETBUILD_SERIAL:
		case TDC_GETBUILD_TAGS:
		case TDC_GETBUILD_TIME:
		case TDC_GETBUILD_TYPE:
		case TDC_GETBUILD_USER:
		case TDC_GET_BUILD_VERSION_CODENAME:
		case TDC_GET_BUILD_VERSION_INCREMENTAL:
		case TDC_GET_BUILD_VERSION_RELEASE:
		case TDC_GET_BUILD_VERSION_SDK_INT:
		case TDC_IS_SIM_STATE_ABSENT:
		case TDC_GET_LINE1_NUMBER:
		case TDC_GET_DEVICE_ID:
		case TDC_GET_SIM_SERIAL_NUMBER:
			tdc_on_getDeviceInfo(&cmd);
			break;
		case TDC_GETINATALLEDPKGNAME:
			tdc_on_getinstalledpkgname(&cmd);
			break;
	}

	return 1;
}

int RA::tdc_send_command(USHORT command, USHORT idx, USHORT rtn, char * buf, int bufsize)
{
	
	TDCCMD cmd;
	//cmd.size = TDCCMD_HEADER + bufsize;
	cmd.size = Swap16(TDCCMD_HEADER + bufsize);
	cmd.cmd = Swap16(command);
	cmd.idx = Swap16(idx);
	cmd.rtn = Swap16(rtn);
	memcpy(m_tdcBuff, &cmd, TDCCMD_HEADER);
	
	/*
	USHORT s;
	s =TDCCMD_HEADER + bufsize;
	s = Swap16(s);
	memcpy(m_tdcBuff + 0, &s, 2);

	s =command;
	s = Swap16(s);
	memcpy(m_tdcBuff + 2, &s, 2);

	s =idx;
	s = Swap16(s);
	memcpy(m_tdcBuff + 4, &s, 2);

	s =rtn;
	s = Swap16(s);
	memcpy(m_tdcBuff + 6, &s, 2);
	*/

	if(bufsize>0)
		memcpy(m_tdcBuff+TDCCMD_HEADER, buf, bufsize);
	return WriteExact(m_tdcsock, (const char*)m_tdcBuff, TDCCMD_HEADER + bufsize);
}

int RA::tdc_setRotate(int rotate)
{
	return tdc_send_command(TDC_SETROTATE_BASE+rotate);
}

//--------------------
#ifdef _DEBUG
char *USHORTToBinary(USHORT i) {
  static char s[16+ 1] = { '0', };
  int count = 16;

  do { s[--count] = '0' + (char) (i & 1);
       i = i >> 1;
  } while (count);

  return s;
}
#endif
void RA::tdc_on_getStatus(TDCCMD* pcmd)
{
	//USHORT status = Swap16(pcmd->rtn);
	USHORT status = pcmd->rtn;
	//TRACE("TDCSTATUS(%s)\n", USHORTToBinary(status));
	if( m_devStatus != status )
	{
		m_devStatus = status;
		m_devStateChanged = 1;

		if( m_devStatus & TDC_STATUS_CALLBLOCKED )
		{
			ClientStatusToPOC(m_clientReserveID, 7, 0, "Client call try blocked");
			if( pcmd->body )
				ta_log("\t\tTDC callblocked(%s)", pcmd->body);
			else
				ta_log("\t\tTDC callblocked");
		}
		if( m_devStatus & TDC_STATUS_PKGBLOCKED )
		{
			ClientStatusToPOC(m_clientReserveID, 7, 1, "Client execute package blocked");
			if( pcmd->body)
				ta_log("\t\tTDC package execute blocked(%s)", pcmd->body);
			else
				ta_log("\t\tTDC package execute blocked");
		}
		TAStateReport(this);

	}
	m_devStatus = status;

}

int RA::tdc_set_prohibit_list()
{
	CString blockPkg;
	CString blockPhoneNum;

	for(int j=0; j< 10; j++)
	{
		if( m_prohibitPackage[j][0] != 0 )
		{
			if( j==0)
				blockPkg = m_prohibitPackage[j];
			else
			{
				blockPkg += ",";
				blockPkg += m_prohibitPackage[j];
			}
		}

		if( m_prohibitPhoneNum[j][0] != 0 )
		{
			if( j==0)
				blockPhoneNum = m_prohibitPhoneNum[j];
			else
			{
				blockPhoneNum += ",";
				blockPhoneNum += m_prohibitPhoneNum[j];
			}
		}
	}
	if( blockPkg.GetLength() > 0 )
	{
		tdc_send_command(TDC_SETBLOCKEDPKGNAMELIST, 0, 0, (LPSTR)(LPCSTR)blockPkg, blockPkg.GetLength());
	}

	if( blockPhoneNum.GetLength() > 0 )
	{
		tdc_send_command(TDC_SETCALLBLOCKLIST, 0, 0,(LPSTR)(LPCSTR)blockPhoneNum, blockPhoneNum.GetLength());
	}

	tdc_send_command(TDC_BLOCK_HANDLER_START);
	
	return 1;
}

void RA::tdc_getDeviceInfo()
{
	//TRACE("tdc_getDeviceInfo - start");
	if( 0 >= tdc_send_command(TDC_GETBUILD_BOARD) )
	{
		ta_log("\t\tTDC send command failed");
	}
	tdc_send_command(TDC_GET_RC_VERSION);
	tdc_send_command(TDC_GETBUILD_BOOTLOADER);
	tdc_send_command(TDC_GETBUILD_BRAND);
	tdc_send_command(TDC_GETBUILD_CPUABI);
	tdc_send_command(TDC_GETBUILD_CPUABI2);
	tdc_send_command(TDC_GETBUILD_DEVICE);
	tdc_send_command(TDC_GETBUILD_DISPLAY);
	tdc_send_command(TDC_GETBUILD_FINGERPRINT);
	tdc_send_command(TDC_GETBUILD_HARDWARE);
	tdc_send_command(TDC_GETBUILD_HOST);
	tdc_send_command(TDC_GETBUILD_ID);
	tdc_send_command(TDC_GETBUILD_MANUFACTURE);
	tdc_send_command(TDC_GETBUILD_MODEL);
	tdc_send_command(TDC_GETBUILD_PRODUCT);
	tdc_send_command(TDC_GETBUILD_RADIO);
	tdc_send_command(TDC_GETBUILD_SERIAL);
	tdc_send_command(TDC_GETBUILD_TAGS);
	tdc_send_command(TDC_GETBUILD_TIME);
	tdc_send_command(TDC_GETBUILD_TYPE);
	tdc_send_command(TDC_GETBUILD_USER);
	tdc_send_command(TDC_GET_BUILD_VERSION_CODENAME);
	tdc_send_command(TDC_GET_BUILD_VERSION_INCREMENTAL);
	tdc_send_command(TDC_GET_BUILD_VERSION_RELEASE);
	tdc_send_command(TDC_GET_BUILD_VERSION_SDK_INT);
	tdc_send_command(TDC_IS_SIM_STATE_ABSENT);
	tdc_send_command(TDC_GET_LINE1_NUMBER);
	tdc_send_command(TDC_GET_DEVICE_ID);
	tdc_send_command(TDC_GET_SIM_SERIAL_NUMBER);
	
	//TRACE("tdc_getDeviceInfo - end\n");
}

void RA::tdc_on_getDeviceInfo(TDCCMD* pcmd)
{
	char buf[2048];

	switch( pcmd->cmd )
	{
		case TDC_GET_RC_VERSION:
			lstrcpy(buf, "TDC_GET_RC_VERSION:");
			if( pcmd->body )
				ta_log("skrc ver:%s", pcmd->body);
			else
				ta_log("skrc ver:none");
			break;

		case TDC_GETBUILD_BOARD:
			lstrcpy(buf, "TDC_GETBUILD_BOARD:");
			break;
		case TDC_GETBUILD_BOOTLOADER:
			lstrcpy(buf, "TDC_GETBUILD_BOOTLOADER:");
			break;
		case TDC_GETBUILD_BRAND:
			lstrcpy(buf, "TDC_GETBUILD_BRAND:");
			break;
		case TDC_GETBUILD_CPUABI:
			lstrcpy(buf, "TDC_GETBUILD_CPUABI:");
			break;
		case TDC_GETBUILD_CPUABI2:
			lstrcpy(buf, "TDC_GETBUILD_CPUABI2:");
			break;
		case TDC_GETBUILD_DEVICE:
			lstrcpy(buf, "TDC_GETBUILD_DEVICE:");
			break;
		case TDC_GETBUILD_DISPLAY:
			lstrcpy(buf, "TDC_GETBUILD_DISPLAY:");
			break;
		case TDC_GETBUILD_FINGERPRINT:
			lstrcpy(buf, "TDC_GETBUILD_FINGERPRINT:");
			break;
		case TDC_GETBUILD_HARDWARE:
			lstrcpy(buf, "TDC_GETBUILD_HARDWARE:");
			break;
		case TDC_GETBUILD_HOST:
			lstrcpy(buf, "TDC_GETBUILD_HOST:");
			break;
		case TDC_GETBUILD_ID:
			lstrcpy(buf, "TDC_GETBUILD_ID:");
			break;
		case TDC_GETBUILD_MANUFACTURE:							// use
			if( pcmd->body )
				lstrcpy(m_devManufacture, (LPCSTR)pcmd->body);
			else
				lstrcpy(m_devManufacture, "");
			lstrcpy(buf, "TDC_GETBUILD_MANUFACTURE:");
			break;
		case TDC_GETBUILD_MODEL:							// use
			if( pcmd->body )
				lstrcpy(m_devModel, (LPCSTR)pcmd->body);
			else
				lstrcpy(m_devModel, "");
			lstrcpy(buf, "TDC_GETBUILD_MODEL:");
			break;
		case TDC_GETBUILD_PRODUCT:
			lstrcpy(buf, "TDC_GETBUILD_PRODUCT:");
			break;
		case TDC_GETBUILD_RADIO:
			lstrcpy(buf, "TDC_GETBUILD_RADIO:");
			break;
		case TDC_GETBUILD_SERIAL:							// use
			if( pcmd->body )
				lstrcpy(m_devBuildSerial, (LPCSTR)pcmd->body);
			else
				lstrcpy(m_devBuildSerial, "");
			lstrcpy(buf, "TDC_GETBUILD_SERIAL:");
			break;
		case TDC_GETBUILD_TAGS:
			lstrcpy(buf, "TDC_GETBUILD_TAGS:");
			break;
		case TDC_GETBUILD_TIME:
			lstrcpy(buf, "TDC_GETBUILD_TIME:");
			break;
		case TDC_GETBUILD_TYPE:
			lstrcpy(buf, "TDC_GETBUILD_TYPE:");
			break;
		case TDC_GETBUILD_USER:
			lstrcpy(buf, "TDC_GETBUILD_USER:");
			break;
		case TDC_GET_BUILD_VERSION_CODENAME:
			lstrcpy(buf, "TDC_GET_BUILD_VERSION_CODENAME:");
			break;
		case TDC_GET_BUILD_VERSION_INCREMENTAL:
			lstrcpy(buf, "TDC_GET_BUILD_VERSION_INCREMENTAL:");
			break;
		case TDC_GET_BUILD_VERSION_RELEASE:							// use
			if( pcmd->body )
				lstrcpy(m_devBuild, (LPCSTR)pcmd->body);
			else
				lstrcpy(m_devBuild, "");
			lstrcpy(buf, "TDC_GET_BUILD_VERSION_RELEASE:");
			break;
		case TDC_GET_BUILD_VERSION_SDK_INT:							// use
			if( pcmd->body )
				lstrcpy(m_devSDKVer, (LPCSTR)pcmd->body);
			else
				lstrcpy(m_devSDKVer, "");
			lstrcpy(buf, "TDC_GET_BUILD_VERSION_SDK_INT:");
			break;
		case TDC_IS_SIM_STATE_ABSENT: // check return value
			lstrcpy(m_devUSIM, pcmd->rtn ? "false" : "true");
			lstrcpy(buf, "TDC_IS_SIM_STATE_ABSENT:");
			lstrcat(buf, pcmd->rtn ? "FALSE" : "TRUE");
			break;
		case TDC_GET_LINE1_NUMBER:
			if( pcmd->body )
				lstrcpy(m_devPhoneNO, (LPCSTR)pcmd->body);
			else
				lstrcpy(m_devPhoneNO, "");
			lstrcpy(buf, "TDC_GET_LINE1_NUMBER:");
			break;
		case TDC_GET_DEVICE_ID:
			if( pcmd->body )
				lstrcpy(m_devIMEI, (LPCSTR)pcmd->body);
			else
				lstrcpy(m_devIMEI, "");
			lstrcpy(buf, "TDC_GET_DEVICE_ID:");
			SetEvent(m_fbStartListen);	// fire event
			break;
		case TDC_GET_SIM_SERIAL_NUMBER:
			if( pcmd->body )
				lstrcpy(m_devSIMSerial, (LPCSTR)pcmd->body);
			else
				lstrcpy(m_devSIMSerial, "n/a");
			lstrcpy(buf, "TDC_GET_SIM_SERIAL_NUMBER:");
			break;
	}

	if( pcmd->cmd != TDC_IS_SIM_STATE_ABSENT)
	{
		if( pcmd->body )
			lstrcat(buf, (char*)pcmd->body);
	}
	//ta_log(buf);
	TAStateReport(this, 1);

}

void RA::tdc_setLog(int bEnable)
{
	char buf[1];
	buf[0] = 'a';
	if( bEnable )
	{
		tdc_send_command(TDC_SETLOGENABLE, 0, 0, buf, 1);
	}
	else
	{
		tdc_send_command(TDC_SETLOGENABLE);
	}
}

void RA::tdc_on_getBatTemperature(TDCCMD* pcmd)
{
	m_devTemperature = pcmd->rtn;
	//ta_log("Device Bat temperature :%04.1f", m_devTemperature/10.0f);
}

void RA::tdc_on_getBatLevel(TDCCMD* pcmd)
{
	m_devBattLvl = pcmd->rtn;
	//ta_log("Device Bat Level :%d percent", m_devBattLvl);
	TAStateReport(this, 1);
}
int RA::tdc_removemessage()
{
	return tdc_send_command(TDC_DELETE_MSG_ALL,0,0);
}

int RA::tdc_getinstalledpkgname()
{
	return tdc_send_command(TDC_GETINATALLEDPKGNAME,0,0);
}

void RA::tdc_on_getinstalledpkgname(TDCCMD* pcmd)
{
	//TRACE("tdc_on_getinstalledpkgname:%s\n", (char*)pcmd->body);
	char* token;
	char* type;
	char cmd_buf[1024];	// 2012-11-05 array size change from 100 to 1024 by camel

	//ta_log("tdc_on_getinstalledpkgname:%d:%d", pcmd->cmd, pcmd->size);
	if(pcmd->size == 0 ) return;
	//ta_log("pcmd:%x", pcmd);
	//ta_log("pcmd body:%x", pcmd->body);
	if(pcmd->body == 0 ) return;

	token = strtok_s((char*)pcmd->body, ",", &type);
	//ta_log("token:%s", token);
	while (token != NULL) 
	{
		sprintf_s(cmd_buf, sizeof(cmd_buf), "uninstall %s", token);
		ta_log("tdc_on_getinstalledpkgname: %s", cmd_buf);
		cmd_adb(cmd_buf);
		token = strtok_s(NULL, ",", &type);
	}

}

void RA::tdc_wake_lock_aquire()
{
	tdc_send_command(TDC_WAKELOCKACQUIRE,0,0);
}

void RA::tdc_wake_lock_release()
{
	tdc_send_command(TDC_WAKELOCKRELEASE,0,0);
}


void RA::onAppInstReq(RUICMD *pcmd)
{
	char* buf;

	if( pcmd->lsize > 0 && pcmd->lsize < 500)
	{
		buf = (char*)&pcmd->body;
	}
	else
	{
		buf = (char*)malloc(pcmd->lsize);
		int n;
		if( (n=ReadExact(m_clientSock, (char*)buf, pcmd->lsize)) <= 0 ) {
			ta_log("TA ERR tdc read(%d) -onAppInstReq", n);
			return;
		}
	}

	if( pcmd->idx == 1 ) // start
	{
		//TRACE("App inst start request\n");
		char buf[MAX_PATH];
		{
			lstrcpy(buf, m_ModulePath);
			lstrcat(buf,"\\AppInst");
			if(GetFileAttributes(buf) == INVALID_FILE_ATTRIBUTES)
			{
				CreateDirectory(buf, 0);
			}
			time_t timer;
			struct tm t;

			timer = time(NULL);
			localtime_s(&t, &timer);
			sprintf_s(buf, "%s\\AppInst\\%04d%02d%02d%02d%02d%02d.apk",
				m_ModulePath,
				t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
		}
	
		FILE* f = NULL;
		fopen_s(&f, buf, "wb");
		ta_log("\t\tAPK Install request:(%s)", buf);
		if( f == NULL ) {
			res_AppInst(APP_INST_RES_ERR_FILECREATE);
			m_AppInstFD = NULL;
			m_AppInst[0] = 0;
		}
		else
		{
			lstrcpy(m_AppInst, buf);
			m_AppInstFD = f;
		}
	}
	else {			// stop
		//TRACE("App inst stop request\n");
		if( m_AppInstFD != NULL )
		{
			fclose(m_AppInstFD);
			::DeleteFile(m_AppInst);
		}
		m_AppInst[0] = 0;
		m_AppInstFD = NULL;
	}

	if( pcmd->lsize >= 500)
	{
		free(buf);
	}
	return;
}


DWORD WINAPI ruiappinstallthread(PVOID pvParam)
{
	RA* p;

	p = (RA*)pvParam;

	if( p->m_clientSock == INVALID_SOCKET)	// bugfix CAMEL 20130405
		return 0;

	if( p->m_AppInst[0] == 0 ) // bugfix CAMEL 20130405
		return 0;

	char command[1024];
	sprintf_s(command, "install -r \"%s\"", p->m_AppInst);

	char out[4096];
	char appname[MAX_PATH];
	char * ps = strrchr(p->m_AppInst, '\\');
	if( ps )
	{
		ps++;
		lstrcpy(appname,ps); 
	}
	else
	{
		lstrcpy(appname, p->m_AppInst);
	}
	p->m_AppInst[0] = 0;
	p->m_AppInstFD = NULL;

	if( p->cmd_adb(command, 60, out, 4096) < 0 ) {
		if( p->m_clientSock != INVALID_SOCKET) // bugfix CAMEL 20130405
			p->res_AppInst(APP_INST_RES_ERR_INSTALL);

		sprintf_s(command, " shell rm /data/local/tmp/%s", appname);
		p->cmd_adb(command);
		
		if( p->m_clientSock == INVALID_SOCKET) // bugfix CAMEL 20130405
			return 0;
	}
	else {
		char buffer[4096];
		char* f = out;
		int pos;
		int bFind = 0;
		while( *f != 0 && f-out < 4096)
		{
			pos = 0;
			do{
				buffer[pos++] = *f;
				f++;
			}while(*f != 0 && *f != '\n' && f-out < 4096);

			buffer[pos] = 0;
				
			if( p->m_clientSock == INVALID_SOCKET) // bugfix CAMEL 20130405
				return 0;

			// line is now in buffer
			if(	strstr(buffer, "Success" ) )
			{
				p->res_AppInst(APP_INST_RES_SUCCESS);
				bFind = 1;
				break;
			}
			if(	strstr(buffer, "Failure" ) ||strstr(buffer, "EOCD" ) )
			{
				p->res_AppInst(APP_INST_RES_ERR_INSTALL, buffer);
				bFind = 1;
				sprintf_s(command, " shell rm /data/local/tmp/%s", appname);
				p->cmd_adb(command);
				break;
			}
		}
		if( p->m_clientSock == INVALID_SOCKET)	// bugfix CAMEL 20130405
			return 0;

		if( !bFind )
		{
			p->res_AppInst(APP_INST_RES_ERR_INSTALL, "Unknown reason");
			sprintf_s(command, " shell rm /data/local/tmp/%s", appname);
			p->cmd_adb(command);
		}
	}

	return 1;
}

void RA::onAppInstData(RUICMD *pcmd)
{
	char* buf;

	if( pcmd->lsize > 0 && pcmd->lsize < 500)
	{
		buf = (char*)&pcmd->body;
	}
	else
	{
		buf = (char*)malloc(pcmd->lsize);
		int n;
		if( (n=ReadExact(m_clientSock, (char*)buf, pcmd->lsize)) <= 0 ) {
			ta_log("TA ERR tdc read(%d) -onAppInstData", n);
			return;
		}
	}

	USHORT realsize;
	memcpy(&realsize, buf, 2);
	realsize = Swap16(realsize);

	char* pdata = buf + 4;

	if( m_AppInst[0] && m_AppInstFD != NULL )
	{
		if( realsize > 0 )
		{
			fwrite(pdata, realsize, 1, m_AppInstFD);
		}
		else {
			fclose(m_AppInstFD);
			DWORD dw;
			HANDLE happinstall;
			happinstall = CreateThread(NULL, 0, ruiappinstallthread, this, 0, &dw);
			CloseHandle(happinstall);
			//------------------------------------------
			/*
			char command[1024];
			sprintf_s(command, "install -r %s", m_AppInst);

			char out[4096];
			char appname[MAX_PATH];
			char * ps = strrchr(m_AppInst, '\\');
			if( ps )
			{
				ps++;
				lstrcpy(appname,ps); 
			}
			else
			{
				lstrcpy(appname, m_AppInst);
			}

			if( cmd_adb(command, 0, 30, out, 4096) < 0 ) {
				res_AppInst(APP_INST_RES_ERR_INSTALL);

				sprintf_s(command, " shell rm /data/local/tmp/%s", appname);
				cmd_adb(command);
			}
			else {
				char buffer[4096];
				char* f = out;
				int pos;
				int bFind = 0;
				while( *f != 0 && f-out < 4096)
				{
					pos = 0;
					do{
					  buffer[pos++] = *f;
					  f++;
					}while(*f != 0 && *f != '\n' && f-out < 4096);

					buffer[pos] = 0;
				
					// line is now in buffer
					if(	strstr(buffer, "Success" ) )
					{
						res_AppInst(APP_INST_RES_SUCCESS);
						bFind = 1;
						break;
					}
					if(	strstr(buffer, "Failure" ) ||strstr(buffer, "EOCD" ) )
					{
						res_AppInst(APP_INST_RES_ERR_INSTALL, buffer);
						bFind = 1;
						sprintf_s(command, " shell rm /data/local/tmp/%s", appname);
						cmd_adb(command);
						break;
					}
				}
				if( !bFind )
				{
					res_AppInst(APP_INST_RES_ERR_INSTALL, "Unknown reason");
					sprintf_s(command, " shell rm /data/local/tmp/%s", appname);
					cmd_adb(command);
				}
			}
			//-------------------------------
			//::DeleteFile(m_AppInst);
			m_AppInst[0] = 0;
			m_AppInstFD = NULL;
			*/
		}
	}

	//TRACE("App inst Data size(%d), data:%02x %02x %02x %02x ... \n", realsize, pdata[0], pdata[1], pdata[2], pdata[3]);


	if( pcmd->lsize >= 500)
	{
		free(buf);
	}
	return;
}