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

HANDLE RA::adb_command(char* command, int show)
{
	//ta_log("adb_command(show:%d)-%s", show, command);

	char cmd[2048];

	lstrcpy(cmd, m_ModulePath);
	lstrcat(cmd, "\\");
	lstrcat(cmd,RUIADB);
	lstrcat(cmd, " ");
	if( m_devIndex >= 0) {
		lstrcat(cmd,"-s \"");
		lstrcat(cmd, m_devSerial[m_devIndex]);
		lstrcat(cmd, "\" ");
	}
	lstrcat(cmd,command);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si,sizeof(si));
	si.cb=sizeof(si);
	si.dwFlags |= STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	
	//show = 1;
	if( show )
		si.wShowWindow = SW_SHOW;

	ZeroMemory(&pi,sizeof(pi));

	if(!CreateProcess(NULL,cmd,0,0,TRUE,NORMAL_PRIORITY_CLASS,0,0,&si,&pi))
	{
		return (HANDLE)-1;
	}
   {
	   CloseHandle(pi.hThread);
	   //CloseHandle(pi.hProcess);
   }
	////Now 'pi.hProcess' contains the process HANDLE, which you can use to wait for it like this:
	//if( wait )
	//{
	//	WaitForSingleObject(pi.hProcess,INFINITE);
	//}
	//else {
	//	m_fbcProcess = pi.hProcess;
	//}



	return pi.hProcess;
}

#define BUFSIZE 4096

BOOL CALLBACK EnumKillWindows(HWND hwnd, LPARAM lParam)
{
  DWORD wndPid;
  //CString Title;
  // lParam = procInfo.dwProcessId;

  // This gets the windows handle and pid of enumerated window.
  GetWindowThreadProcessId(hwnd, &wndPid);

  // This gets the windows title text
  // from the window, using the window handle
  
  //CWnd::FromHandle( hwnd )->GetWindowText(Title);

  //  this makes sure that the PID matches that PID we started, and window
  // text exists, before we kill it . I don't think this is really needed, 
  // I included it because some apps have more than one window.
  if ( wndPid == (DWORD)lParam /*&& Title.GetLength() != 0*/)
  {
    //  Please kindly close this process
    ::PostMessage(hwnd, WM_CLOSE, 0, 0);
    return true;
  }
  else
  {
    // Keep enumerating
    return true;
  }
}

void g_killProcess(DWORD dwPID)
{
  HANDLE ps = OpenProcess( SYNCHRONIZE|PROCESS_TERMINATE, 
                                        FALSE, dwPID);

  if( ps == NULL )
	  return;

  EnumWindows(EnumKillWindows, dwPID);

  if(WaitForSingleObject(ps, 5000)!=WAIT_OBJECT_0)
         TerminateProcess(ps,0);

  CloseHandle(ps) ;
}

ra_adb::ra_adb(RA* pra)
{
	m_hADBStd_OUT_Rd = 0;
	m_hADBStd_OUT_Wr = 0;
	m_hADBStd_IN_Rd = 0;
	m_hADBStd_IN_Wr = 0;
	m_adbProcessID = 0;
	m_dwadbProcessID = 0;

	m_pra = pra;
}

ra_adb::~ra_adb()
{
	if( m_hADBStd_OUT_Rd )
		CloseHandle(m_hADBStd_OUT_Rd);
	m_hADBStd_OUT_Rd = 0;
	
	if( m_hADBStd_OUT_Wr )
		CloseHandle(m_hADBStd_OUT_Wr);
	m_hADBStd_OUT_Wr = 0;

	if( m_hADBStd_IN_Rd )
		CloseHandle(m_hADBStd_IN_Rd);
	m_hADBStd_IN_Rd = 0;

	if( m_hADBStd_IN_Wr )
		CloseHandle(m_hADBStd_IN_Wr);
	m_hADBStd_IN_Wr = 0;

	if( m_adbProcessID )
	{
		//CloseHandle(m_adbProcessID);
		//TerminateProcess(m_adbProcessID,0);
		g_killProcess(m_dwadbProcessID);
		m_adbProcessID = 0;
	}
}

int ra_adb::init_adb() 
{ 
   SECURITY_ATTRIBUTES saAttr; 
 

// Set the bInheritHandle flag so pipe handles are inherited. 
 
   saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
   saAttr.bInheritHandle = TRUE; 
   saAttr.lpSecurityDescriptor = NULL; 

// Create a pipe for the child process's STDOUT. 
   if ( ! CreatePipe(&m_hADBStd_OUT_Rd, &m_hADBStd_OUT_Wr, &saAttr, BUFSIZE) ) {
		return 0;
	   //ErrorExit(TEXT("StdoutRd CreatePipe")); 
   }
// Create a pipe for the child process's STDIN. 
 
   if (! CreatePipe(&m_hADBStd_IN_Rd, &m_hADBStd_IN_Wr, &saAttr, 0)) {
	   return 0;
      //ErrorExit(TEXT("Stdin CreatePipe")); 
   }

// Create the ADB process. 
   char szCmdline[256];
   //getModulePath(szCmdline, 256);
   //lstrcat(szCmdline,"\\");
   //lstrcat(szCmdline, RUIADB);
   lstrcpy(szCmdline,"cmd.exe /q /k prompt $g");

   PROCESS_INFORMATION piProcInfo; 
   STARTUPINFO siStartInfo;
   BOOL bSuccess = FALSE; 
 
   ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
 
   ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
   siStartInfo.cb = sizeof(STARTUPINFO); 
   siStartInfo.hStdError = m_hADBStd_OUT_Wr;
   siStartInfo.hStdOutput = m_hADBStd_OUT_Wr;
   siStartInfo.hStdInput = m_hADBStd_IN_Rd;
   siStartInfo.dwFlags |= (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW );
   siStartInfo.wShowWindow = SW_HIDE;
   //siStartInfo.wShowWindow = SW_SHOW;
   bSuccess = CreateProcess(NULL, 
      szCmdline,     // command line 
      NULL,          // process security attributes 
      NULL,          // primary thread security attributes 
      TRUE,          // handles are inherited 
      0, //CREATE_NEW_PROCESS_GROUP,             // creation flags 
      NULL,          // use parent's environment 
      m_pra->m_ModulePath,          // use parent's current directory 
      &siStartInfo,  // STARTUPINFO pointer 
      &piProcInfo);  // receives PROCESS_INFORMATION 
   
   // If an error occurs, exit the application. 
   if ( ! bSuccess ) 
   {
   		//ta_log("init_adb -failed");
	   return 0;
   }
   {
	   CloseHandle(piProcInfo.hThread);
	   CloseHandle(piProcInfo.hProcess);
   }
   
   m_adbProcessID = piProcInfo.hProcess;
   m_dwadbProcessID = piProcInfo.dwProcessId;

   purge_adb();
	//ta_log("init_adb -done");

   return 1; 
} 
 


void ra_adb::stop_adb()
{
	//ta_log("stop_adb -start");
	char buf[12];

	//::GenerateConsoleCtrlEvent(CTRL_C_EVENT, m_dwadbProcessID);
	//::GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, m_dwadbProcessID);
	//::GenerateConsoleCtrlEvent(CTRL_CLOSE_EVENT, m_dwadbProcessID);

	lstrcpy(buf,"exit\n");
	WriteToADB(buf, strlen(buf));
	if( m_hADBStd_OUT_Rd )
		CloseHandle(m_hADBStd_OUT_Rd);
	m_hADBStd_OUT_Rd = 0;
	
	if( m_hADBStd_OUT_Wr )
		CloseHandle(m_hADBStd_OUT_Wr);
	m_hADBStd_OUT_Wr = 0;

	if( m_hADBStd_IN_Rd )
		CloseHandle(m_hADBStd_IN_Rd);
	m_hADBStd_IN_Rd = 0;

	if( m_hADBStd_IN_Wr )
		CloseHandle(m_hADBStd_IN_Wr);
	m_hADBStd_IN_Wr = 0;

	if( m_adbProcessID )
	{
		//CloseHandle(m_adbProcessID);
		//TerminateProcess(m_adbProcessID,0);
		g_killProcess(m_dwadbProcessID);
		m_adbProcessID = 0;
	}
	//ta_log("stop_adb -done");

}

int ra_adb::ReadFromADB(char* buffer, int size) 

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data. 
{ 
   DWORD dwRead; 
   BOOL bSuccess = FALSE;

  if( PeekNamedPipe(m_hADBStd_OUT_Rd, NULL, 0, NULL, &dwRead, NULL) == 0 )
	  return 0;

   if( size <= 0)
	   return dwRead;

   if( dwRead > 0 )
   {
		if(!ReadFile(
			m_hADBStd_OUT_Rd,	 // handle to pipe to copy from 
			buffer,	 // address of buffer that receives data
			size,	 // number of bytes to read
			&dwRead,	 // address of number of bytes read
			NULL	 // address of structure for data for overlapped I/O
			))
		{
			//if( dwRead < 0 )
			//{
			//	if( GetLastError() != ERROR_IO_PENDING)
			//		  return 0;
			//} else {
				return -1;
			//}
		}
	}

/*
	if( dwRead > 0 )
	{	char *tb = (char*)malloc(dwRead+1);
		memcpy(tb, buffer, dwRead);
		tb[dwRead] = 0;
		TRACE("RFADB(%d):%s\n", dwRead, tb);
		free(tb);
	}
*/

	return dwRead;
} 
 
int ra_adb::WriteToADB(char* buffer, int size) 
{
	BOOL bSuccess;
	DWORD dwWritten;

    bSuccess = WriteFile(m_hADBStd_IN_Wr, buffer, size, &dwWritten, NULL);
/*
	if( size > 0 )
	{	char *tb = (char*)malloc(size+1);
		memcpy(tb, buffer, size);
		tb[size] = 0;
		TRACE("WTADB(%d):%s\n", size, tb);
		free(tb);
	}
*/
 
	if ( ! bSuccess ) 
		return -1;

	return dwWritten;
} 
 
void ra_adb::purge_adb()
{
	//ta_log("purge_adb -start");
	char buf[1024];

	//Sleep(200);
	int nRead;
	int nMax = 10;
	while( (nRead = ReadFromADB(0,0)) <= 0 && nMax-- > 0)
	{
		Sleep(50);
	}

	if( nRead <= 0  )
		return;
	
	nMax = 100;
	while( (nRead = ReadFromADB(buf,1023)) > 0 && nMax-- > 0)
	{
		buf[nRead] = 0;
		//TRACE(buf);
		Sleep(50);
	}
	//ta_log("purge_adb -done");
}

int ra_adb::cmd_adb(char* command, int waitsecond, char* output, int outsize, int bBackground)
{
	//m_pra->ta_log("cmd_adb(wait:%d)-%s", waitsecond, command);
	init_adb();
	char buf[4097];
	int waitmilsecond = waitsecond * 10;
	sprintf_s(buf,"%s ", RUIADB);
	if( m_pra->m_devIndex >= 0 ) {
		lstrcat(buf,"-s \"");
		lstrcat(buf, m_pra->m_devSerial[m_pra->m_devIndex]);
		lstrcat(buf, "\" ");
	}
	lstrcat(buf, command);
	lstrcat(buf, "\n");

	if( WriteToADB(buf, lstrlen(buf)) < 0 )
	{
		stop_adb();
		return -1;
	}

	int nRead;
	
	int nMax = 100;
	while( (nRead = ReadFromADB(0,0)) <= 0 && nMax-- > 0)
	{
		Sleep(waitmilsecond);
	}

	if( bBackground )
		return 1;
	//if( nRead <= 0 )
	//{
	//	WriteToADB("\x03",1);
	//}
	
	int nPos = 0;
	while( nRead > 0 )
	{
		nRead = ReadFromADB(buf,4096);
		if( nRead > 0) {
			buf[nRead] = 0;
			//TRACE(buf);
			//m_pra->ta_log("result:%s",buf);
			if( output )
			{
				if( outsize > nPos+nRead )
				{
					memcpy(output+nPos, buf, nRead);
					nPos += nRead;
					output[nPos] = 0;
				}
				else
				{
					if( nPos < outsize )
					{
						memcpy(output + nPos, buf, outsize-nPos);
						nPos = outsize;
						output[outsize-1] = 0;
					}
				}
			}
			if( buf[nRead-1] == '>')
				break;
		}
		nMax = 100;
		while( (nRead = ReadFromADB(0,0)) <= 0 && nMax-- > 0)
		{
			Sleep(waitmilsecond);
		}
	}

	//ta_log("cmd_adb - exit - purge");
	purge_adb();
	//ta_log("cmd_adb - exit - stop");


	stop_adb();
	//ta_log("cmd_adb - done");
	return 1;
}


int RA::cmd_adb(char* command, int waitsecond, char* output, int outsize)
{
	ra_adb * padb = new ra_adb(this);

	int nrtn = padb->cmd_adb(command, waitsecond, output, outsize);

	//if( !bBackground)
		delete padb;

	return nrtn;
}

int RA::getdevices_adb(char* buf)
{
	int nDevices = 0;

	char output[2048];
	buf[0] = 0;
	int index = m_devIndex;
	m_devIndex = -1;
	cmd_adb("devices", 3, output, 2048);
	m_devIndex = index;

	char * line = strchr(output,'\n');	// skip first line
	while( line != NULL) {
		line++;
		char* serial = strtok_s(line, "\t", &line);
		char* status = strtok_s(NULL, "\t\r", &line);
		
		if( serial == 0 || status == 0 )	break;

		if( lstrcmpi(status, "device") == 0)
		{
			//TRACE("serial:%s(%s)\n", serial, status);
			nDevices++;
			lstrcat(buf, serial);
			lstrcat(buf,"|");
		}
	}
	return nDevices;
}

int RA::getSDKVersion(void)
{
	int version = 0;
	char output[2048];
	
	cmd_adb("shell getprop ro.build.version.sdk", 3, output, 1024);
	sscanf(output, "%d", &version);
	TRACE("SDK version %d\n",version);
	return version;
}
