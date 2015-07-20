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
#ifndef __UTIL_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "./jpeg/jpeglib.h"
#include "./jpeg/cdjpeg.h"
#ifndef false
#define false 0
#define true -1
#endif
#define RUISVC_VERSION_LEN	13
#ifdef TARGET_AML
	#define RUISVC_VERSION "AMLSCMSVC:1.7"
	#define RUISVC_PORT		55810
#else
	#define RUISVC_VERSION "RUICAPSVC:1.7"
	#define RUISVC_PORT		5910
#endif
#define ICS_SCREENCAP   1
#define BEFORE_ICS      0

#define SOCKET  int

#define LOCK(mutex) pthread_mutex_lock(&(mutex))
#define UNLOCK(mutex) pthread_mutex_unlock(&(mutex))
#define MUTEX(mutex) pthread_mutex_t (mutex)
#define INIT_MUTEX(mutex) pthread_mutex_init(&(mutex),NULL)
#define TINI_MUTEX(mutex) pthread_mutex_destroy(&(mutex))
#define TSIGNAL(cond) pthread_cond_signal(&(cond))
#define WAIT(cond,mutex) pthread_cond_wait(&(cond),&(mutex))
#define COND(cond) pthread_cond_t (cond)
#define INIT_COND(cond) pthread_cond_init(&(cond),NULL)
#define TINI_COND(cond) pthread_cond_destroy(&(cond))
#define IF_PTHREADS(x) x

enum SocketState {
	SOCKET_INIT,
	SOCKET_READY,
	SOCKET_SHUTDOWN
};
#define FILE_FTP_BUFFER_SIZE	8192

#define FILE_ERR_CONVERT		0x0001	// error occur during convert path
#define FILE_ERR_OPENFILE		0x0002	// could not open file
#define FILE_ERR_READ			0x0003	// file read error
#define FILE_ERR_STAT			0x0004	// file stat failed perhaps it is absent or is not a regular file
#define FILE_ERR_CREATE			0x0005  // file creation error
#define FILE_ERR_WRITE			0x0006	// file write error
#define FILE_ERR_CANCELBYUSER	0x0007 // user cancel transfer

//#define PERFORMANCE_REPORT
#define ELAPSED(start,end) ((end.tv_sec-start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec))

struct fbinfo {
    unsigned int version;
    unsigned int bpp;
    unsigned int size;
    unsigned int width;
    unsigned int height;
    unsigned int red_offset;
    unsigned int red_length;
    unsigned int red_mask;
    unsigned int blue_offset;
    unsigned int blue_length;
    unsigned int blue_mask;
    unsigned int green_offset;
    unsigned int green_length;
    unsigned int green_mask;
    unsigned int alpha_offset;
    unsigned int alpha_length;
    unsigned int alpha_mask;
    unsigned int orientation;
} __attribute__((packed));

#define PERFORMANC

typedef unsigned char	BYTE;
typedef unsigned short	USHORT;
typedef unsigned int	UINT;

extern int touchfd;

typedef struct _ruicapsvcInfo {
    char version[64];
    SOCKET listenSock;
    SOCKET sock;
    in_addr_t listenInterface;
    enum SocketState socketState;
    int port;
    char host[64];
    pthread_t client_thread;
    pthread_t fbc_thread;
    int jpegquality;
    char ftproot[PATH_MAX];
    char ftpfilename[PATH_MAX];
    pthread_t ftp_thread;
    int ftp_uploadfd;
    //MUTEX(fbc_mutex);
    //COND(updateCond);
    MUTEX(output_mutex);
	int	   frame_sent;
    #ifdef PERFORMANCE_REPORT
    	double frame_total;
    	double frame_capture;
    	double frame_compress;
    	double frame_colorspace;
    	double frame_tx;
    	double frame_client;
	#endif

} svcInfo, *svcInfoPtr;

#define FBCCMD_HEADER	8
typedef struct tagFBCCMD {
	unsigned char  cmd;
	unsigned char  dummy1;
	unsigned short err;
	unsigned int size;
	char* 		   data;
} FBCCMD, *PFBCCMD;

#define _FBCCMD \
	unsigned char cmd; \
	unsigned char dummy1; \
	unsigned short dummy2; \
	unsigned int size;

typedef struct {
	_FBCCMD
	unsigned int	width;
	unsigned int	height;
	unsigned int	bpp;
	unsigned int	touchfd;
} FBCCMD_SCRINFO;

typedef struct {
	_FBCCMD
	unsigned char	flags;
	unsigned char	dummy3;
	unsigned short	pathsize;
	// followed by path
} FBCCMD_FILELISTREQ;

typedef struct {
	_FBCCMD
	BYTE	flags;
	BYTE	dummy3;
	USHORT  dummy4;
	USHORT	numFiles;
	USHORT	datasize;
	// followed by SizeData(ULONG)[numFiles]
	// followed by Filenames[datasize]
} FBCCMD_FILELISTDATA;

typedef struct {
	_FBCCMD
	USHORT	realsize;
	USHORT dummy3;
	// followed by filedata[size]
} FBCCMD_FILEDATA;

typedef void (*LogProc)(const char *format, ...);
typedef struct tagfbc_outbuf {
	_FBCCMD
	char* jpegBuf;
}FBC_OUTPUT;
extern int fbc_output_current;
extern FBC_OUTPUT fbc_output[2];

extern int g_forcefbmode;
extern int g_halfmode;
extern struct fbinfo g_fbinfo;
extern void LogEnable(int enabled);
extern LogProc Log, Err;

// main.c
void NewClient(svcInfoPtr psvc, int sock);
void ExitClient(svcInfoPtr psvc);
static void* listenerRun(void *data);
void RunEventLoop(svcInfoPtr psvc);
void InitServer();
void ShutdownServer(svcInfoPtr psvc);
void InitServer();

// framebuffer.c
int get_screen_on();
int get_dev_rotation();
int check_screencap(struct fbinfo * pfbinfo);
void framebuffer_service(int fd, void *cookie);
int exit_framebuffer();
int init_framebuffer();
int read_rgb_framebuffer();
int read_rgb_framebuffer_to_jpeg(svcInfoPtr psvc);
int read_rgb_framebuffer_screencap();
int read_rgb_framebuffer_screencap_to_jpeg(svcInfoPtr psvc);
int read_rgb_framebuffer_fb0();
int read_rgb_framebuffer_fb0_to_jpeg(svcInfoPtr psvc);
int convert_to_rgb(unsigned char * x, int xsize, unsigned char * target);
int convert_to_jpeg(int quality);
void init_jpeg(int quality);
int exit_jpeg();
void injectTouchEvent(int mode, int x, int y);
void onTouch(svcInfoPtr psvc, PFBCCMD msg);

// sockets.c
void InitSockets(svcInfoPtr psvc);
void ShutdownSockets(svcInfoPtr psvc);
void CloseClient(svcInfoPtr psvc);
int ReadExactTimeout(svcInfoPtr psvc, char* buf, int len, int timeout);
int ReadExact(svcInfoPtr psvc,char* buf,int len);
int WriteExact(svcInfoPtr psvc, const char *buf, int len);

// ftp.cpp
void onFilelistReq(svcInfoPtr psvc, PFBCCMD msg);
void onFileDownloadReq(svcInfoPtr psvc, PFBCCMD msg);
void onFileUploadReq(svcInfoPtr psvc,  PFBCCMD msg);
void onFileMsg(svcInfoPtr psvc, PFBCCMD msg);
void onFileErr(svcInfoPtr psvc, PFBCCMD msg);
void SendFileErrMsg(svcInfoPtr psvc, USHORT err);
unsigned long ChkFileDownloadErr(svcInfoPtr psvc, char* path);
static void* HandleFileDownload(void* data);
#endif /* __UTIL_H__ */
