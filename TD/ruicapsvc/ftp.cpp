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
#include "util.h"
#include <utime.h>

#define SUCCESS 1
#define FAILURE 0

typedef struct _FileListItemInfo {
    char name[NAME_MAX];
    unsigned int size;
    unsigned int data;
} FileListItemInfo, *FileListItemInfoPtr;

typedef struct _FileListItemSize {
    unsigned int size;
    unsigned int data;
} FileListItemSize, *FileListItemSizePtr;

typedef struct _FileListInfo {
    FileListItemInfoPtr pEntries;
    int numEntries;
} FileListInfo, *FileListInfoPtr;

int AddFileListItemInfo(FileListInfoPtr fileListInfoPtr, char* name, unsigned int size, unsigned int data);
char* GetFileNameAt(FileListInfo fileListInfo, int number);
unsigned int GetFileSizeAt(FileListInfo fileListInfo, int number);
unsigned int GetFileDataAt(FileListInfo fileListInfo, int number);
unsigned int GetSumOfFileNamesLength(FileListInfo fileListInfo);
void FreeFileListInfo(FileListInfo fileListInfo);


void
DisplayFileList(FileListInfo fli)
{
    int i = 0;
    if((fli.pEntries == NULL) || (fli.numEntries == 0)) return;

    Log("DISPLAYING FILE NAMES IN THE LIST ...START\n\n");
    Log("Numer of entries:: %d\n", fli.numEntries);
    for(i = 0; i < fli.numEntries; i++)
		Log("file[%d]\t<%s>\n", i, fli.pEntries[i].name);
    Log("DISPLAYING FILE NAMES IN THE LIST ...END\n\n");
}


char* ConvertPath(svcInfoPtr psvc, char* path)
{
	char p[PATH_MAX];
	memset(p, 0, PATH_MAX);

	if( (path == NULL) ||
		(strlen(path) == 0) ||
		(strlen(path)+strlen(psvc->ftproot) > PATH_MAX - 1) ) {

		return NULL;
	}

	memcpy(p, path, strlen(path));
	memset(path, 0, PATH_MAX);
	sprintf(path, "%s%s", psvc->ftproot, p);

	return path;
}


int
CreateFileListInfo(FileListInfoPtr pFileListInfo, char* path, int flag)
{
	DIR* pDir = NULL;
	struct dirent* pDirent = NULL;


	if((pDir = opendir(path)) == NULL) {
		return FAILURE;
	}

	while((pDirent = readdir(pDir))) {
		if(strcmp(pDirent->d_name, ".") && strcmp(pDirent->d_name, "..")) {
			struct stat stat_buf;
			/*
			int fpLen = sizeof(char)*(strlen(pDirent->d_name)+strlen(path)+2);
			*/
			char fullpath[PATH_MAX];

			memset(fullpath, 0, PATH_MAX);

			strcpy(fullpath, path);
			if(path[strlen(path)-1] != '/')
				strcat(fullpath, "/");
			strcat(fullpath, pDirent->d_name);

			if(stat(fullpath, &stat_buf) < 0) {
				continue;
			}

			if(S_ISDIR(stat_buf.st_mode)) {
				if(AddFileListItemInfo(pFileListInfo, pDirent->d_name, -1, 0) == 0) {
					continue;
				}
			}
			else {
				if(flag) {
					if(AddFileListItemInfo(pFileListInfo, pDirent->d_name,
												stat_buf.st_size,
												stat_buf.st_mtime) == 0) {
						continue;
					}
				}
			}
		}
	}
	if(closedir(pDir) < 0) {

	}

	return SUCCESS;
}

void onFilelistReq(svcInfoPtr psvc, PFBCCMD msg)
{
	// msg parsing
	Log("onFilelistReq\n");
	FileListInfo fileListInfo;
	int status = -1;


	BYTE flags = (BYTE)msg->data[0];
	unsigned short pathsize;
	memcpy(&pathsize, msg->data+2, 2);
	pathsize = ntohs(pathsize);
	//Log("path size:%d\n", pathsize);
	char* npath = msg->data+4;

	char path[PATH_MAX];
	memcpy(path, npath, pathsize);

	path[pathsize] = 0;
	//Log("path:%s\n", path);
	if( ConvertPath(psvc, path) == 0 )
	{
		return;
	}
	//Log("path:%s\n", path);
	//Log("onFilelistReq:flags(%c), pathsize(%d:%s)\n", flags, pathsize, path);
	// get filelist
	memset(&fileListInfo, 0, sizeof(FileListInfo));

	status = CreateFileListInfo(&fileListInfo, path, !(flags  & 0x10));

	// send filelistreq response
	if(status != FAILURE) {

		char *data = NULL, *pFileNames = NULL;
		unsigned int length = 0, dsSize = 0;
		int i = 0;
		FileListItemSizePtr pFileListItemSize = NULL;

		dsSize = fileListInfo.numEntries * 8;
		length = dsSize +	// numFiles * ULONG(size)
				GetSumOfFileNamesLength(fileListInfo) +	// filenames
				fileListInfo.numEntries;	// delimeter (1byte)

		data = (char*) calloc(length, sizeof(char));
		if(data == NULL) {
			return;
		}
		pFileListItemSize = (FileListItemSizePtr) &data[0];
		pFileNames = &data[dsSize];

		flags				= flags & 0xF0;
		USHORT numFiles		= htons(fileListInfo.numEntries);
		USHORT dataSize 	= htons(GetSumOfFileNamesLength(fileListInfo) +
											fileListInfo.numEntries);
		for(i =0; i <fileListInfo.numEntries; i++) {
			pFileListItemSize[i].size = htonl(GetFileSizeAt(fileListInfo, i));
			pFileListItemSize[i].data = htonl(GetFileDataAt(fileListInfo, i));
			strcpy(pFileNames, GetFileNameAt(fileListInfo, i));

			if(i+1 < fileListInfo.numEntries)
				pFileNames += strlen(pFileNames) + 1;
		}

		//DisplayFileList(fileListInfo);
		FBCCMD_FILELISTDATA res;
		res.cmd = 'k';
		res.size = htonl(sizeof(FBCCMD_FILELISTDATA) + length);
		res.flags = flags;
		res.numFiles = numFiles;
		res.datasize = dataSize;
		//Log("response:size(%d), numFiles(%d), datasize(%d)\n", FBCCMD_HEADER+length, ntohs(numFiles), ntohs(dataSize));
		LOCK(psvc->output_mutex);
		WriteExact(psvc, (const char*)&res, sizeof(FBCCMD_FILELISTDATA));
		WriteExact(psvc, (const char*)data, length);
		UNLOCK(psvc->output_mutex);

		//fileListMsg.data 	= data;
		//fileListMsg.length 	= length;

		free(data);
		FreeFileListInfo(fileListInfo);
		//Log("-done\n");
	}
}
void onFileDownloadReq(svcInfoPtr psvc, PFBCCMD msg)
{

	char* npath = msg->data;
	char path[PATH_MAX];
	memcpy(path, npath, msg->size);

	path[msg->size] = 0;
	Log("onFileDownloadReq:%s\n", path);

	if( ConvertPath(psvc, path) == 0 )
	{
		SendFileErrMsg(psvc, FILE_ERR_CONVERT);
		return;
	}
	Log("path:%s\n", path);

	strcpy(psvc->ftpfilename, path);

	pthread_create(&psvc->ftp_thread, NULL, HandleFileDownload, (void *)psvc);

}

int
ChkFileUploadErr(svcInfoPtr psvc, char* path)
{

	int uploadFD;
	if(  (uploadFD = creat(path,	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) == -1) {
			SendFileErrMsg(psvc, FILE_ERR_CREATE);
	}

	return uploadFD;
}


void onFileUploadReq(svcInfoPtr psvc,  PFBCCMD msg)
{
	USHORT pathsize;
	memcpy(&pathsize, msg->data, sizeof(USHORT));
	pathsize = ntohs(pathsize);

	unsigned int position;
	memcpy(&position, msg->data+4, sizeof(unsigned int));
	position = ntohl(position);

	char* npath = msg->data+8;
	char path[PATH_MAX];
	memcpy(path, npath, pathsize);

	path[pathsize] = 0;
	Log("onFileUploadReq:%s\n", path);

	if( ConvertPath(psvc, path) == 0 )
	{
		SendFileErrMsg(psvc, FILE_ERR_CONVERT);
		return;
	}
	Log("path:%s\n", path);

	strcpy(psvc->ftpfilename, path);

	psvc->ftp_uploadfd = ChkFileUploadErr(psvc, path);

}


void
CloseUndoneFileTransfer(svcInfoPtr psvc)
{

	if(psvc == NULL)
		return;


	if(psvc->ftp_uploadfd != -1) {
		close(psvc->ftp_uploadfd);
		psvc->ftp_uploadfd = -1;
	}

	if( psvc->ftpfilename[0] != 0) {
		unlink(psvc->ftpfilename);
		psvc->ftpfilename[0] = 0;
	}

}


void
HandleFileUploadWrite(svcInfoPtr psvc, USHORT realsize, char* pBuf)
{

	unsigned long numOfBytesWritten = 0;

	numOfBytesWritten = write(psvc->ftp_uploadfd, pBuf, realsize);

	if(numOfBytesWritten != realsize) {
		SendFileErrMsg(psvc, FILE_ERR_WRITE);
		CloseUndoneFileTransfer(psvc);
	}
}

void
FileUpdateComplete(svcInfoPtr psvc, unsigned long mtime)
{
	/* Here we are settimg the modification and access time of the file */
	/* Windows code stes mod/access/creation time of the file */
	struct utimbuf utb;

	if(psvc->ftp_uploadfd != -1) {
		close(psvc->ftp_uploadfd);
		psvc->ftp_uploadfd = -1;
	}

	utb.actime = utb.modtime = mtime;
	if(utime(psvc->ftpfilename, &utb) == -1) {
		//Log("Setting the modification/access"
		//		" time for the file <%s> failed\n",psvc->ftpfilename);
	}

}

void onFileMsg(svcInfoPtr psvc, PFBCCMD msg)
{
	USHORT realsize;
	memcpy(&realsize, msg->data, sizeof(USHORT));
	realsize = ntohs(realsize);

	//Log("onFileMsg:%d\n", realsize);
	char* data = msg->data+4;

	if(realsize == 0) {
		unsigned long mtime;
		memcpy(&mtime, data, sizeof(unsigned long));
		mtime = htonl(mtime);

		FileUpdateComplete(psvc, mtime);
		return;
	}

	HandleFileUploadWrite(psvc, realsize, data);
}

void onFileErr(svcInfoPtr psvc, PFBCCMD msg)
{
	USHORT lerr = ntohs(msg->err);
	Log("OnFileTransfer Error:%04x\n",lerr);
	if( psvc->ftp_uploadfd != -1) {
		CloseUndoneFileTransfer(psvc);
	}

	if( psvc->ftp_thread != 0) {
		psvc->ftp_thread = 0;
	}
}


int
AddFileListItemInfo(FileListInfoPtr fileListInfoPtr, char* name,
					unsigned int size, unsigned int data)
{
	FileListItemInfoPtr fileListItemInfoPtr = (FileListItemInfoPtr)
												calloc((fileListInfoPtr->numEntries + 1),
														sizeof(FileListItemInfo));
	if(fileListItemInfoPtr == NULL) {
		return FAILURE;
	}

	if(fileListInfoPtr->numEntries != 0) {
	    memcpy(fileListItemInfoPtr, fileListInfoPtr->pEntries,
	    		fileListInfoPtr->numEntries * sizeof(FileListItemInfo));
	}

	strcpy(fileListItemInfoPtr[fileListInfoPtr->numEntries].name, name);
	fileListItemInfoPtr[fileListInfoPtr->numEntries].size = size;
	fileListItemInfoPtr[fileListInfoPtr->numEntries].data = data;

	if(fileListInfoPtr->pEntries != NULL) {
	    free(fileListInfoPtr->pEntries);
	    fileListInfoPtr->pEntries = NULL;
	}

	fileListInfoPtr->pEntries = fileListItemInfoPtr;
	fileListItemInfoPtr = NULL;
	fileListInfoPtr->numEntries++;

	return SUCCESS;
}


char*
GetFileNameAt(FileListInfo fileListInfo, int number)
{
	char* name = NULL;
	if(number >= 0 && number < fileListInfo.numEntries)
		name = fileListInfo.pEntries[number].name;
	return name;
}


unsigned int
GetFileSizeAt(FileListInfo fileListInfo, int number)
{
	unsigned int size = 0;
	if(number >= 0 && number < fileListInfo.numEntries)
		size = fileListInfo.pEntries[number].size;
	return size;
}


unsigned int
GetFileDataAt(FileListInfo fileListInfo, int number)
{
	unsigned int data = 0;
	if(number >= 0 && number < fileListInfo.numEntries)
		data = fileListInfo.pEntries[number].data;
	return data;
}


unsigned int
GetSumOfFileNamesLength(FileListInfo fileListInfo)
{
	int i = 0, sumLen = 0;
	for(i = 0; i < fileListInfo.numEntries; i++)
		sumLen += strlen(fileListInfo.pEntries[i].name);
	return sumLen;
}


void
FreeFileListInfo(FileListInfo fileListInfo)
{
	if(fileListInfo.pEntries != NULL) {
		free(fileListInfo.pEntries);
		fileListInfo.pEntries = NULL;
	}
	fileListInfo.numEntries = 0;
}

void
SendFileErrMsg(svcInfoPtr psvc, USHORT err)
{
	FBCCMD res;
	res.cmd = 'o';
	res.size = htonl(FBCCMD_HEADER);
	res.err = htons(err);

	LOCK(psvc->output_mutex);
	WriteExact(psvc, (const char*)&res, FBCCMD_HEADER);
	UNLOCK(psvc->output_mutex);
}

unsigned long ChkFileDownloadErr(svcInfoPtr psvc, char* path)
{
	struct stat stat_buf;
	if( (path == NULL) || (strlen(path) == 0) ||
		(stat(path, &stat_buf) < 0) || (!(S_ISREG(stat_buf.st_mode))) ) {

		SendFileErrMsg(psvc, FILE_ERR_STAT);
		return 0;
	}

	return stat_buf.st_mtime;
}

static void*
HandleFileDownload(void* data)
{
	Log("ftp thread start\n");
	svcInfoPtr psvc = (svcInfoPtr)data;
	char* path = psvc->ftpfilename;

	char pBuf[FILE_FTP_BUFFER_SIZE];

	memset(pBuf, 0, FILE_FTP_BUFFER_SIZE);

	int downloadFD;
	int numOfBytesRead;
	unsigned long mtime;
	if( (mtime = ChkFileDownloadErr(psvc, path)) == 0 )
	{
		Log("ftp thread exit stat failed\n");
		return NULL;
	}
	if((downloadFD = open(path, O_RDONLY)) == -1) {
			Log("ERR Couldn't open file\n");

			SendFileErrMsg(psvc, FILE_ERR_OPENFILE);

			Log("ftp thread exit open failed\n");
			return NULL;
	}

	FBCCMD_FILEDATA res;
	res.cmd = 'l';
	USHORT realsize;
	while(psvc->ftp_thread != 0) {
		if( (numOfBytesRead = read(downloadFD, pBuf, FILE_FTP_BUFFER_SIZE)) <= 0) {
			close(downloadFD);

			if(numOfBytesRead == 0) {
				LOCK(psvc->output_mutex);
				res.size = htonl(sizeof(FBCCMD_FILEDATA) + sizeof(unsigned long));
				realsize = (USHORT)numOfBytesRead;
				res.realsize = htons(realsize);
				WriteExact(psvc, (const char*)&res, sizeof(FBCCMD_FILEDATA));
				WriteExact(psvc, (const char*)&mtime, sizeof(unsigned long));
				UNLOCK(psvc->output_mutex);
			}
			else {
				SendFileErrMsg(psvc, FILE_ERR_READ);
			}
			break;
		}
		else {
			LOCK(psvc->output_mutex);
			res.size = htonl(sizeof(FBCCMD_FILEDATA) + numOfBytesRead );
			realsize = (USHORT)numOfBytesRead;
			res.realsize = htons(realsize);
			WriteExact(psvc, (const char*)&res, sizeof(FBCCMD_FILEDATA));
			WriteExact(psvc, (const char*)pBuf, numOfBytesRead);
			UNLOCK(psvc->output_mutex);
		}
	}

	psvc->ftp_thread = 0;
	Log("ftp thread exit\n");
	return NULL;
}
