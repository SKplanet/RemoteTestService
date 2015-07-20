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


int RA::start_log(char* filter) 
{ 

	if( m_hChildStd_OUT_Rd != NULL  ) {
		stop_log();
		//return 1;
	}

	m_hChildStd_OUT_Rd = NULL;
	m_logInit = 50;

   SECURITY_ATTRIBUTES saAttr; 
 

// Set the bInheritHandle flag so pipe handles are inherited. 
 
   saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
   saAttr.bInheritHandle = TRUE; 
   saAttr.lpSecurityDescriptor = NULL; 

// Create a pipe for the child process's STDOUT. 
 
   
   if ( ! CreatePipe(&m_hChildStd_OUT_Rd, &m_hChildStd_OUT_Wr, &saAttr, LOGCAT_BUFSIZE) ) {
		return 0;
	   //ErrorExit(TEXT("StdoutRd CreatePipe")); 
   }


// Ensure the read handle to the pipe for STDOUT is not inherited.

   if ( ! SetHandleInformation(m_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) ) {
      return 0;
	  // ErrorExit(TEXT("Stdout SetHandleInformation")); 
   }
   /*
// Create a pipe for the child process's STDIN. 
 
   if (! CreatePipe(&m_hChildStd_IN_Rd, &m_hChildStd_IN_Wr, &saAttr, 0)) {
	   return 0;
      //ErrorExit(TEXT("Stdin CreatePipe")); 
   }

// Ensure the write handle to the pipe for STDIN is not inherited. 
 
   if ( ! SetHandleInformation(m_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) ) {
	   return 0;
     // ErrorExit(TEXT("Stdin SetHandleInformation")); 
   }
	*/

// Create the child process. 
   
   m_logProcessID = CreateLogcatProcess(filter);

// Get a handle to an input file for the parent. 
// This example assumes a plain text file and uses string output to verify data flow. 
 
   	m_hlogThread = CreateThread(NULL, 0, ruilogcatThread, this, 0, &m_dwlogThreadID);
	CloseHandle(m_hlogThread);
   return 1; 
} 
 

void RA::stop_log()
{
	if( m_hChildStd_OUT_Rd )
		CloseHandle(m_hChildStd_OUT_Rd);
	m_hChildStd_OUT_Rd = 0;

	if( m_hChildStd_OUT_Wr )
		CloseHandle(m_hChildStd_OUT_Wr);
	m_hChildStd_OUT_Wr = 0;

	if( m_logProcessID )
	{
		//CloseHandle(m_logProcessID);
		TerminateProcess(m_logProcessID,0);
		m_logProcessID = 0;
	}

	if( m_hlogThread )
	{
		//CloseHandle(m_hlogThread);
		TerminateThread(m_hlogThread, 0);
		m_hlogThread = 0;
	}
}

void RA::SetIncomingLogCallback(fnIncomingLog pfn, void* pData)
{
	m_IncomingLog = pfn;
	m_IncomingLogData = pData;
}

void RA::incoming_log(RA* pra, BYTE* buffer, int size)
{
	if( pra->m_IncomingLog != NULL) {
		pra->m_IncomingLog(pra, pra->m_IncomingLogData, buffer, size);
	}
}


void RA::SetIncomingTALogCallback(fnIncomingLog pfn, void* pData)
{
	m_IncomingTALog = pfn;
	m_IncomingTALogData = pData;
}

void RA::incoming_TAlog(RA* pra, BYTE* buffer, int size)
{
	if( pra->m_IncomingTALog != NULL) {
		pra->m_IncomingTALog(pra, pra->m_IncomingTALogData, buffer, size);
	}
}

HANDLE RA::CreateLogcatProcess(char* filter)
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{ 
	char szCmdline[2048];

	lstrcpy(szCmdline, m_ModulePath);
	lstrcat(szCmdline,"\\");
	lstrcat(szCmdline,RUIADB);
	lstrcat(szCmdline," ");
	if( m_devIndex >= 0) {
		lstrcat(szCmdline,"-s \"");
		lstrcat(szCmdline, m_devSerial[m_devIndex]);
		lstrcat(szCmdline, "\" ");
	}
	lstrcat(szCmdline," logcat -c");

	PROCESS_INFORMATION piProcInfo; 
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE; 
 
// Set up members of the PROCESS_INFORMATION structure. 
 
   ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
 
// Set up members of the STARTUPINFO structure. 
// This structure specifies the STDIN and STDOUT handles for redirection.
 
   ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
   siStartInfo.cb = sizeof(STARTUPINFO); 
   siStartInfo.dwFlags |= (STARTF_USESHOWWINDOW );
   siStartInfo.wShowWindow = SW_HIDE;
 
// Create the child process. 
    
   bSuccess = CreateProcess(NULL, 
      szCmdline,     // command line 
      NULL,          // process security attributes 
      NULL,          // primary thread security attributes 
      TRUE,          // handles are inherited 
      0,             // creation flags 
      NULL,          // use parent's environment 
      NULL,          // use parent's current directory 
      &siStartInfo,  // STARTUPINFO pointer 
      &piProcInfo);  // receives PROCESS_INFORMATION 
   
   // If an error occurs, exit the application. 
   if ( ! bSuccess ) 
      return 0;
   if( bSuccess )
   {
	   CloseHandle(piProcInfo.hThread);
	   //CloseHandle(piProcInfo.hProcess);
   }

	WaitForSingleObject(piProcInfo.hProcess,10000);


   	lstrcpy(szCmdline, m_ModulePath);
	lstrcat(szCmdline,"\\");
	lstrcat(szCmdline,RUIADB);
	lstrcat(szCmdline," ");
	if( m_devIndex >= 0) {
		lstrcat(szCmdline,"-s \"");
		lstrcat(szCmdline, m_devSerial[m_devIndex]);
		lstrcat(szCmdline, "\" ");
	}
	lstrcat(szCmdline,"logcat ");
	lstrcat(szCmdline,filter);

   
	ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
 
// Set up members of the STARTUPINFO structure. 
// This structure specifies the STDIN and STDOUT handles for redirection.
 
   ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
   siStartInfo.cb = sizeof(STARTUPINFO); 
   siStartInfo.hStdError = m_hChildStd_OUT_Wr;
   siStartInfo.hStdOutput = m_hChildStd_OUT_Wr;
   siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
   siStartInfo.dwFlags |= (STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES);
   siStartInfo.wShowWindow = SW_HIDE;

   bSuccess = CreateProcess(NULL, 
      szCmdline,     // command line 
      NULL,          // process security attributes 
      NULL,          // primary thread security attributes 
      TRUE,          // handles are inherited 
      0,             // creation flags 
      NULL,          // use parent's environment 
      NULL,          // use parent's current directory 
      &siStartInfo,  // STARTUPINFO pointer 
      &piProcInfo);  // receives PROCESS_INFORMATION 
   if( bSuccess )
   {
	   CloseHandle(piProcInfo.hThread);
	   //CloseHandle(piProcInfo.hProcess);
   }
   
   //CloseHandle(m_hChildStd_IN_Wr);
   //m_hChildStd_IN_Wr = NULL;

   return piProcInfo.hProcess;
}
 
int RA::ReadFromPipe(RA* pra) 

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data. 
{ 
   DWORD dwRead; 
   BOOL bSuccess = FALSE;
   static int i = 0;
   while( m_logInit-- > 0)
   {

		   //TRACE("initial log clean (%d)", i++);
		   PeekNamedPipe(m_hChildStd_OUT_Rd, NULL, 0, NULL, &dwRead, NULL);
		   if( dwRead != 0) {
		   ReadFile(
					m_hChildStd_OUT_Rd,	 // handle to pipe to copy from 
					m_chBuf,	 // address of buffer that receives data
					LOGCAT_BUFSIZE,	 // number of bytes to read
					&dwRead,	 // address of number of bytes read
					NULL	 // address of structure for data for overlapped I/O
					);
		   }
		   Sleep(10);

   }

   PeekNamedPipe(m_hChildStd_OUT_Rd, NULL, 0, NULL, &dwRead, NULL);
   if( dwRead != 0 ) {
		if(!ReadFile(
			m_hChildStd_OUT_Rd,	 // handle to pipe to copy from 
			m_chBuf,	 // address of buffer that receives data
			LOGCAT_BUFSIZE,	 // number of bytes to read
			&dwRead,	 // address of number of bytes read
			NULL	 // address of structure for data for overlapped I/O
			))
		  {
			  if( GetLastError() != ERROR_IO_PENDING)
				  return 0;
		  } else {
			  //TRACE("initial log clean (%d)", i++);
			  if( pra->m_IncomingLog != NULL ) {
				  pra->m_IncomingLog(pra, pra->m_IncomingLogData, (BYTE*)m_chBuf, dwRead);
			  }
		  }
	}
	  return 1;
 
} 
 
void RA::ta_log(const char* log, ...)
{
	 va_list args;
    char buf[2048];
    time_t timer;
	struct tm t;

	FILE* f = NULL;
	//EnterCriticalSection(&CriticalSection);
	check_log_filename();
	if( m_taLog[0] != 0 ) {
		fopen_s(&f, m_taLog, "a");
	}

    va_start(args, log);

    timer = time(NULL);
    localtime_s(&t, &timer);
	sprintf_s(buf, "%04d%02d%02d %02d:%02d:%02d:", t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	int len = lstrlen(buf);
    
	if( f != NULL )
		fwrite(buf, 1, len, f);
	
	DeviceStatusToPOC(9, 0, buf);

#ifdef DEBUG
	//TRACE(buf);
	OutputDebugString(buf);
#endif
	incoming_TAlog(this, (BYTE*)buf, len);

	vsprintf_s(buf, log, args);
	lstrcat(buf,"\r\n");
	len = lstrlen(buf);
	
	if( f != NULL )
		fwrite(buf, 1, len, f);

#ifdef DEBUG
	//TRACE(buf);
	OutputDebugString(buf);
#endif

	incoming_TAlog(this, (BYTE*)buf, len);
	
    //fflush(f);
	if( f != NULL )
		fclose(f);

    va_end(args);
	//LeaveCriticalSection(&CriticalSection);
}
