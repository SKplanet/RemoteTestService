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
#define RUISVC_VERSION "RUICAPSVC:1.0"
#define ICS_SCREENCAP   1
#define BEFORE_ICS      0

#define SOCKET  int

#define LOCK(mutex) pthread_mutex_lock(&(mutex));
#define UNLOCK(mutex) pthread_mutex_unlock(&(mutex));
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

#define PERFORMANCE_REPORT
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
typedef struct _ruicapsvcInfo {
    char version[64];
    SOCKET listenSock;
    SOCKET sock;
    in_addr_t listenInterface;
    enum SocketState socketState;
    int port;
    char host[64];
    pthread_t client_thread;
    int jpegquality;

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


typedef void (*LogProc)(const char *format, ...);


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

// sockets.c
void InitSockets(svcInfoPtr psvc);
void ShutdownSockets(svcInfoPtr psvc);
void CloseClient(svcInfoPtr psvc);
int Connect(svcInfoPtr psvc, char *host, int port);
int ReadExactTimeout(svcInfoPtr psvc, char* buf, int len, int timeout);
int ReadExact(svcInfoPtr psvc,char* buf,int len);
int WriteExact(svcInfoPtr psvc, const char *buf, int len);
int StringToAddr(char *str, in_addr_t *addr);
int ListenOnTCPPort(int port, in_addr_t iface);
int ConnectToTcpAddr(char *host, int port);

#endif /* __UTIL_H__ */
