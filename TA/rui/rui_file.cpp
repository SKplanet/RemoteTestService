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

void RA::onReqFileList(RUICMD *pcmd)
{
	TRACE("onReqFileList - ");
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
			ta_log("\t\tTA ERR tdc read(%d) - onReqFileList", n);
			return;
		}
	}

	FBCCMD msg;
	msg.cmd = 'k';
	msg.size = Swap32(pcmd->lsize + FBCCMD_HEADER);

	/*
	FBCCMD_FILELISTREQ msg;

	USHORT pathlen;
	memcpy( &pathlen, &buf[2],2);
	pathlen = Swap16(pathlen);

	char* path= &buf[4];
	path[pathlen] = 0;

	msg.cmd = 'k';
	msg.size = Swap32(sizeof(FBCCMD_FILELISTREQ) +  pathlen);
	msg.flags =   buf[0];
	msg.pathsize = Swap16(pathlen);

	WriteExact(m_fbcsock, (char*)&msg, sizeof(FBCCMD_FILELISTREQ));
	WriteExact(m_fbcsock, path, pathlen);
	*/
	WaitForSingleObject( m_fbOutputMutex, INFINITE);
	WriteExact(m_fbcsock, (char*)&msg, FBCCMD_HEADER);
	WriteExact(m_fbcsock, (char*)buf, pcmd->lsize);
	ReleaseMutex(m_fbOutputMutex);

	char * fname = buf + sizeof(USHORT) + sizeof(USHORT);
	USHORT fnamesize;
	memcpy(&fnamesize, buf + sizeof(USHORT), sizeof(USHORT));
	fnamesize = Swap16(fnamesize);
	WCHAR Unicode[2048];
	char UTF8Code[4096];
	int nUnicodeSize = MultiByteToWideChar(CP_UTF8, 0, fname, fnamesize, Unicode, 2048);
	int nUTF8CodeSize = WideCharToMultiByte(CP_ACP, 0, Unicode, nUnicodeSize, UTF8Code, 4096, NULL, NULL);
	UTF8Code[nUTF8CodeSize] = 0;
	ta_log("\t\tFTP ftp filelist request(%s)", UTF8Code);

	if( pcmd->lsize >= 500)
	{
		free(buf);
	}
	TRACE("end\n");
	return;

	////------------ read
	//int onFBC_filelistdata(FBCCMD* pcmd);
}


void RA::onReqFileDownload(RUICMD *pcmd)
{
	TRACE("onReqFileDownload - ");
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
			ta_log("\t\tTA ERR tdc read(%d) - onReqFileDownload", n);
			return;
		}
	}

	m_ftp = 1;
	//ta_log("ftp start on file download req");
	FBCCMD msg;
	msg.cmd = 'l';
	msg.size = Swap32(pcmd->lsize + FBCCMD_HEADER);
	WaitForSingleObject( m_fbOutputMutex, INFINITE);
	WriteExact(m_fbcsock, (char*)&msg, FBCCMD_HEADER);
	WriteExact(m_fbcsock, (char*)buf, pcmd->lsize);
	ReleaseMutex(m_fbOutputMutex);

	WCHAR Unicode[2048];
	char UTF8Code[4096];
	int nUnicodeSize = MultiByteToWideChar(CP_UTF8, 0, buf, pcmd->lsize, Unicode, 2048);
	int nUTF8CodeSize = WideCharToMultiByte(CP_ACP, 0, Unicode, nUnicodeSize, UTF8Code, 4096, NULL, NULL);
	UTF8Code[nUTF8CodeSize] = 0;
	ta_log("\t\tFTP ftp file download request(%s)", UTF8Code);


	if( pcmd->lsize >= 500)
	{
		free(buf);
	}
	TRACE("end\n");
	return;
}

void RA::onCliFileUpload(RUICMD *pcmd)
{
	TRACE("onCliFileUpload - ");
	char* buf;
	//ta_log("onCliFileUpload:%d", pcmd->lsize);
	if( pcmd->lsize > 0 && pcmd->lsize < 500)
	{
		buf = (char*)&pcmd->body;
	}
	else
	{
		buf = (char*)malloc(pcmd->lsize);
		int n;
		if( (n=ReadExact(m_clientSock, (char*)buf, pcmd->lsize)) <= 0 ) {
			ta_log("\t\tTA ERR tdc read(%d) - onCliFileUpload", n);
			return;
		}
	}

	m_ftp = 1;
	//ta_log("ftp start on client file upload");
	FBCCMD msg;
	msg.cmd = 'm';
	msg.size = Swap32(pcmd->lsize + FBCCMD_HEADER);

	WaitForSingleObject( m_fbOutputMutex, INFINITE);
	WriteExact(m_fbcsock, (char*)&msg, FBCCMD_HEADER);
	WriteExact(m_fbcsock, (char*)buf, pcmd->lsize);
	ReleaseMutex( m_fbOutputMutex);

	char * fname = buf + sizeof(USHORT) + sizeof(USHORT) + sizeof(ULONG);
	USHORT fnamesize;
	memcpy(&fnamesize, buf, sizeof(USHORT));
	fnamesize = Swap16(fnamesize);
	WCHAR Unicode[2048];
	char UTF8Code[4096];
	int nUnicodeSize = MultiByteToWideChar(CP_UTF8, 0, fname, fnamesize, Unicode, 2048);
	int nUTF8CodeSize = WideCharToMultiByte(CP_ACP, 0, Unicode, nUnicodeSize, UTF8Code, 4096, NULL, NULL);
	UTF8Code[nUTF8CodeSize] = 0;
	ta_log("\t\tFTP ftp upload request(%s)", UTF8Code);


	if( pcmd->lsize >= 500)
	{
		free(buf);
	}
	TRACE("end\n");
}

void RA::onCliFileUploadData(RUICMD *pcmd)
{
	TRACE("onCliFileUploadData - ");
	char* buf;

	//ta_log("onCliFileUploadData:%d", pcmd->lsize);
	if( pcmd->lsize > 0 && pcmd->lsize < 500)
	{
		buf = (char*)&pcmd->body;
	}
	else
	{
		buf = (char*)malloc(pcmd->lsize);
		int n;
		if( (n=ReadExact(m_clientSock, (char*)buf, pcmd->lsize)) <= 0 ) {
			ta_log("\t\tTA ERR tdc read(%d) - onCliFileUploadData", n);
			return;
		}
	}
	USHORT realsize;
	memcpy(&realsize, buf, 2);

	realsize = Swap16(realsize);


	FBCCMD msg;
	msg.cmd = 'n';
	msg.size = Swap32(pcmd->lsize + FBCCMD_HEADER);

	WaitForSingleObject( m_fbOutputMutex, INFINITE);
	WriteExact(m_fbcsock, (char*)&msg, FBCCMD_HEADER);
	WriteExact(m_fbcsock, (char*)buf, pcmd->lsize);
	ReleaseMutex( m_fbOutputMutex);

	if( realsize == 0 )
	{
		m_ftp = 0;
		//ta_log("ftp stop on file upload end");
	}

	if( pcmd->lsize >= 500)
	{
		free(buf);
	}
	TRACE("end\n");
}

void RA::onCliFileErrorMsg(RUICMD *pcmd)
{
	ta_log("\t\tonCliFileErrorMsg - ");
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
			ta_log("\t\tTA ERR tdc read(%d) - onCliFileErrorMsg", n);
			return;
		}
	}

	FBCCMD msg;
	msg.cmd = 'o';
	USHORT err = (USHORT)pcmd->idx;
	msg.err = Swap16(err);
	msg.size = Swap32(FBCCMD_HEADER);

	WaitForSingleObject( m_fbOutputMutex, INFINITE);
	WriteExact(m_fbcsock, (char*)&msg, FBCCMD_HEADER);
	ReleaseMutex( m_fbOutputMutex);

	m_ftp = 0;
	//ta_log("ftp stop on ErrorMsg");

	if( pcmd->lsize >= 500)
	{
		free(buf);
	}
	//TRACE("end\n");
}
int RA::file_res_filelist(char* buf, int size)
{
	TRACE("file_res_filelist - ");
	//TRACE("filelist:%d\n", size);
	RUICMD cmd;
	cmd.cmd = Swap32(RES_FILE_LIST);
	cmd.lsize = Swap32( size + RUICMD_HEADER);

	WaitForSingleObject( m_clientOutputMutex, INFINITE);
	WriteExact( m_clientSock, (char*)&cmd, RUICMD_HEADER);
	WriteExact( m_clientSock, buf, size);
	ReleaseMutex(m_clientOutputMutex);
	TRACE("end\n");
	return 1;
}

int RA::file_res_filedownload(char* buf, int size)
{
	TRACE("file_res_filedownload - ");
	RUICMD cmd;
	cmd.cmd = Swap32(RES_FILE_DOWNLOAD);
	cmd.lsize = Swap32( size + RUICMD_HEADER);

	USHORT realsize;
	memcpy(&realsize, buf, 2);

	realsize = Swap16(realsize);

	WaitForSingleObject( m_clientOutputMutex, INFINITE);
	WriteExact( m_clientSock, (char*)&cmd, RUICMD_HEADER);
	WriteExact( m_clientSock, buf, size);
	ReleaseMutex(m_clientOutputMutex);

	if( realsize == 0)
	{
		m_ftp = 0;
		//ta_log("ftp stop on download end");
	}
	TRACE("end\n");
	return 1;
}

int RA::file_svr_err_msg(USHORT err)
{
	ta_log("file_svr_err_msg - ");
	RUICMD cmd;
	cmd.cmd = Swap32(SVR_FILE_ERROR_MSG);
	UINT lerr = err;
	cmd.idx = Swap32(lerr);
	cmd.lsize = Swap32(RUICMD_HEADER);

	WaitForSingleObject( m_clientOutputMutex, INFINITE);
	WriteExact( m_clientSock, (char*)&cmd, RUICMD_HEADER);
	ReleaseMutex(m_clientOutputMutex);

	m_ftp = 0;
	//ta_log("ftp stop on server error msg");
	//TRACE("end\n");
	return 1;
}

