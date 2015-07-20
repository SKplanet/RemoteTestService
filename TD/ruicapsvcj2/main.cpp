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
#include <signal.h>


static int ProcessClientMessage(svcInfoPtr psvc);
static void * clientInput(void *data);
void NewClient(svcInfoPtr psvc, int sock);
void ExitClient(svcInfoPtr psvc);
static void* listenerRun(void *data);
void RunEventLoop(svcInfoPtr psvc);
void InitServer();
void ShutdownServer(svcInfoPtr psvc);
#ifdef PERFORMANCE_REPORT
static struct timeval tLast;
#endif

//int last_fbc_output_current = 1;
/*
static void * fbcThread(void *data)
{
    svcInfoPtr psvc = (svcInfoPtr)data;
    Log("fbcThread started\n");
    g_halfmode = 1;
    while(1)
    {
    	LOCK(psvc->fbc_mutex);
    	//Log("wait..\n");
    	WAIT(psvc->updateCond, psvc->fbc_mutex);
    	//Log("update(%d) %d-%d\n",g_halfmode, last_fbc_output_current, fbc_output_current);
    	//if( last_fbc_output_current == fbc_output_current )
    	read_rgb_framebuffer_to_jpeg(psvc);
    	g_halfmode = 1;

			LOCK(psvc->output_mutex);
			fbc_output_current++;
			fbc_output_current %= 2;
			//Log("outbuf current:%d\n", fbc_output_current);
			UNLOCK(psvc->output_mutex);
		//Log("leave\n");
    	UNLOCK(psvc->fbc_mutex);
    }

    Log("fbcThread Exit\n");

    return NULL;
}
*/
/*
 * 	abcdefghijklmnopqrstuvwxyz
 *      e  hi klmno q  t u
 *      |  || ||||| |  | |
 *      |  || ||||| |  | +get screen (full res)
 *      |  || ||||| |  + touch(down,x,y)
 *      |  || ||||| +quit client
 *      |  || ||||+ftp_err
 *      |  || |||+ftp_data
 *      |  || ||+ftp_upload_req
 *      |  || |+ftp_download_req
 *      |  || +ftp_filelist_req
 *      |  |+get screen info
 *      |  +get screen (half req)
 *      +stop service
 */
static int ProcessClientMessage(svcInfoPtr psvc, FBCCMD* pmsg)
{
	//FBCCMD msg;
    //char msg;
    int n = 0;
    int nSize;

    //if( (n = ReadExact(psvc, (char*)&msg, FBCCMD_HEADER)) <= 0 ) {
    //    Err("ProcessClientMessage : read(%d)\n", n);
    //    return false;
    //}
    pmsg->size = ntohl(pmsg->size);
    pmsg->size -= FBCCMD_HEADER;
    //Log("msg received(%c: size - %d)\n", pmsg->cmd, pmsg->size);
    if( pmsg->size > 0) {
    	pmsg->data = (char*)malloc(pmsg->size);
        if( (n = ReadExact(psvc, (char*)pmsg->data, pmsg->size)) <= 0 ) {
            Err("ProcessClientMessage : read(%d)\n", n);
            return false;
        }
    }
    //Log("after alloc %02x %02x %02x %02x\n", msg.data[0],msg.data[1],msg.data[2],msg.data[3]);


    unsigned int data;
#ifdef PERFORMANCE_REPORT
	struct timeval tS, tE;

	gettimeofday(&tS,NULL);
	psvc->frame_client += ELAPSED(tLast, tS);
#endif
    switch(pmsg->cmd) {

        case 'i':
        {
        	FBCCMD_SCRINFO res;
        	res.cmd = 'i';
        	res.size = htonl(sizeof(FBCCMD_SCRINFO));

        	res.width = htonl(g_fbinfo.width);
        	res.height = htonl(g_fbinfo.height);
        	res.bpp = htonl(g_fbinfo.bpp);
        	res.touchfd = htonl( (touchfd != -1 ) ? 1 : 0);
        	LOCK(psvc->output_mutex);
            int rtn = WriteExact(psvc, (const char*)&res, sizeof(FBCCMD_SCRINFO));
            UNLOCK(psvc->output_mutex);
            //Log("response i %d to :%d\n", res.size, rtn);
        }
            break;
        case 'u':
        	//Log("signal-u\n");
        	g_halfmode = 0;
        	//TSIGNAL(psvc->updateCond);
        	read_rgb_framebuffer_to_jpeg(psvc);
#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_total += ELAPSED(tS, tE);
	tS = tE;
#endif
            break;
        case 'h':
        	g_halfmode = 1;
        	//Log("signal-h\n");
        	//TSIGNAL(psvc->updateCond);
            //g_halfmode = 0;
            read_rgb_framebuffer_to_jpeg(psvc);
#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_total += ELAPSED(tS, tE);
	tS = tE;
#endif
            break;
        case 'e':
            return false;
            break;
        case 'q':
        	ExitClient(psvc);
        	ShutdownSockets(psvc);
        	break;
        // ftp
        case 'k':	// ftp filestlist req
        	onFilelistReq(psvc, pmsg);
        	break;
        case 'l':	// ftp filedownload req
        	onFileDownloadReq(psvc, pmsg);
        	break;
        case 'm':	// ftp fileupload req
        	onFileUploadReq(psvc, pmsg);
        	break;
        case 'n':	// ftp fileupload data
            //Log("case n\n");

        	onFileMsg(psvc, pmsg);
        	break;
        case 'o':	// ftp fileerr
        	onFileErr(psvc, pmsg);
        	break;
        case 't':
        	onTouch(psvc, pmsg);
        	break;
    }

    if( pmsg->size > 0) {
    	free(pmsg->data);
    }

    //if(( psvc->frame_sent++ % 5) == 0) {
    //	g_fbinfo.orientation = get_dev_rotation();
    //	//int isScreenOn = get_screen_on();
    //	//Log("isScreenOn(%d)\n",isScreenOn);
    //}

#ifdef PERFORMANCE_REPORT

            if( (psvc->frame_sent % 10) == 0)
            {
            	double frame_total;
            	double frame_capture;
            	double frame_compress;
            	double frame_colorspace;
            	double frame_tx;
            	double frame_client;
            	double fps, fps_svr;



            	fps = (double)10 * 1000000 / (psvc->frame_total + psvc->frame_client);
            	fps_svr = (double)10 * 1000000 / psvc->frame_total;
            	frame_total = psvc->frame_total / 10000;
            	frame_capture = psvc->frame_capture / 10000;
            	frame_compress = psvc->frame_compress / 10000;
            	frame_colorspace = psvc->frame_colorspace / 10000;
            	frame_tx = psvc->frame_tx / 10000;
            	frame_client = psvc->frame_client / 10000;

                if( psvc->frame_sent > 10 )
                {
                	Log("FPS(%5.2f),SVR(%4.1f) TOT(%3.0f),CAP(%3.0f),ZIP(%3.0f),CNV(%2.0f),TX(%2.0f),CLI(%2.0f)\n",
                			//psvc->frame_sent-1,
                			fps, fps_svr,
                			frame_total, frame_capture, frame_compress, frame_colorspace, frame_tx,frame_client);
                }
            	psvc->frame_total		=	0;
            	psvc->frame_capture		=	0;
            	psvc->frame_compress	=	0;
            	psvc->frame_colorspace	=	0;
            	psvc->frame_tx			=	0;
            	psvc->frame_client		= 	0;

            }
            gettimeofday(&tLast,NULL);
#endif /* PERFORMANCE_REPORT */
           // Log("end loop");
    return true;

}

static void * clientInput(void *data)
{
    svcInfoPtr psvc = (svcInfoPtr)data;
    Log("clientInput Thread started(%d)\n", psvc->sock);

    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    getpeername(psvc->sock, (struct sockaddr *)&addr, &addrlen);
    strcpy(psvc->host, inet_ntoa(addr.sin_addr));
    Log("Client(%s) connected\n",psvc->host);

    fd_set rfds, sfds;
    struct timeval tv;
    int n;

    psvc->frame_sent 		= 0;
#ifdef PERFORMANCE_REPORT
	psvc->frame_total		=	0;
	psvc->frame_capture		=	0;
	psvc->frame_compress	=	0;
	psvc->frame_colorspace	=	0;
	psvc->frame_tx			=	0;
    psvc->frame_client		= 	0;
    gettimeofday(&tLast,NULL);
#endif /* PERFORMANCE_REPORT */

    //LOCK(psvc->output_mutex);
    //last_fbc_output_current = fbc_output_current;
    //UNLOCK(psvc->output_mutex);
    FBCCMD msgt, msg;
    while (1) {

        FD_ZERO(&rfds);
        FD_SET(psvc->sock, &rfds);
        //FD_ZERO(&sfds);
        //FD_SET(psvc->sock, &sfds);


        tv.tv_sec = 30;
        tv.tv_usec = 0;	// 50 msec
        n = select(psvc->sock + 1, &rfds,  /*&sfds*/ NULL, NULL, &tv);
        if (n < 0) {
            Err("ReadExact: select");
            break;
        }
        
        if (psvc->sock == -1) {
            Log("client disconnected\n");
            break;
        }

        if (FD_ISSET(psvc->sock, &rfds) ) {
            if( ReadExact(psvc, (char*)&msg, FBCCMD_HEADER) <= 0 ) {
                Err("Read Err");
                break;
            }

            if( msg.cmd == 'h')
            {
				while( 1 )
				{
					FD_ZERO(&rfds);
					FD_SET(psvc->sock, &rfds);
					tv.tv_sec = 0;
					tv.tv_usec = 1000;
					n = select(psvc->sock + 1, &rfds,  /*&sfds*/ NULL, NULL, &tv);
					if( n== 0) // no more packet
						break;
					if (FD_ISSET(psvc->sock, &rfds) ) {
						if( ReadExact(psvc, (char*)&msgt, FBCCMD_HEADER) <= 0 ) {
							Err("Read Err2\n");
							break;
						}

						if( msgt.cmd == msg.cmd ) {// same update packet
							//Log("skip same update cmd\n");
							continue;
						}
						ProcessClientMessage(psvc, &msg);
						msg = msgt;
					}
					break;
				}
            }
            if( 0 == ProcessClientMessage(psvc, &msg) )
                break;
        }
        /*
        if (FD_ISSET(psvc->sock, &sfds) ) {
            LOCK(psvc->output_mutex);

            if( last_fbc_output_current != fbc_output_current )
            {
            	//Log("output-start %d-%d\n",last_fbc_output_current,fbc_output_current);
            	//Log("client  outbuf  last:%d current:%d\n", last_fbc_output_current, fbc_output_current);
            	unsigned int size = fbc_output[last_fbc_output_current].size;
            	fbc_output[last_fbc_output_current].size = htonl(size);
            	//int result1, result2;
            	WriteExact(psvc, (const char*)&fbc_output[last_fbc_output_current], FBCCMD_HEADER);
            	WriteExact(psvc, (const char*)fbc_output[last_fbc_output_current].jpegBuf, size - FBCCMD_HEADER);
            	last_fbc_output_current = fbc_output_current;
            	//Log("output result(%d,%d:%d)\n", result1, result2,size);
            }
            UNLOCK(psvc->output_mutex);
        }
        */

        if (psvc->sock == -1) {
            Log("client disconnected\n");
            break;
        }
    }

    ExitClient(psvc);

    Log("clientInput Thread Exit\n");

    return NULL;
}

void NewClient(svcInfoPtr psvc, int sock)
{
    int one=1;


    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one)) < 0) {
        Err("setsockopt failed");
        close(sock);
        return;
     }

    psvc->sock = sock;
    if (WriteExact(psvc, psvc->version, RUISVC_VERSION_LEN) < 0) {
        Err("NewClient: write");
        ExitClient(psvc);
        return;
    }

    if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) {
        Err("fcntl failed");
        close(sock);
        return;
    }

    //pthread_attr_t myattr;
    //pthread_attr_init(&myattr);
    pthread_create(&psvc->client_thread, NULL, clientInput, (void *)psvc);
    void* rtn = 0;
    pthread_join(psvc->client_thread, &rtn);
}

void ExitClient(svcInfoPtr psvc)
{
    Log("ExitClient\n");
    CloseClient(psvc);
}

static void* listenerRun(void *data)
{
    int client_fd;
    struct sockaddr_in peer;
    socklen_t len;

    svcInfoPtr psvc = (svcInfoPtr)data;
    len = sizeof(peer);

    Log("listener - start\n");
    //pthread_create(&psvc->fbc_thread, NULL, fbcThread, (void *)psvc);
    while (psvc->listenSock != -1 && (client_fd = accept(psvc->listenSock,
                               (struct sockaddr*)&peer, &len)) >= 0) {
        NewClient(psvc, client_fd);
        len = sizeof(peer);

    }

    Log("Listener - stop \n");
    return(NULL);
}

void RunEventLoop(svcInfoPtr psvc)
{
       pthread_t listener_thread;

       pthread_create(&listener_thread, NULL, listenerRun, psvc);
}


//void InitServer(svcInfoPtr psvc)
void InitServer()
{
    svcInfo svc;
    
    strcpy(svc.version, RUISVC_VERSION);
    svc.listenInterface = htonl(INADDR_ANY);
    svc.sock = -1;
    svc.port=RUISVC_PORT;
    svc.socketState=SOCKET_INIT;
    svc.host[0] = 0;
    svc.jpegquality = 90;
    strcpy(svc.ftproot, "/sdcard");

    //INIT_MUTEX(svc.fbc_mutex);
    //INIT_COND(svc.updateCond);
    INIT_MUTEX(svc.output_mutex);

    InitSockets(&svc);

    listenerRun(&svc);

   // RunEventLoop(&svc);


}

void ShutdownServer(svcInfoPtr psvc)
{
    ShutdownSockets(psvc);

}

#ifndef CAMEL_REMOVE
int main(int argc, char* argv[])
{
	/*
	pid_t childPID;
	g_forcefbmode = false;
	if( argc > 1 && strcmp(argv[1], "fb")==0)
		g_forcefbmode = true;

	childPID = fork();

	if( childPID >= 0 )
	{
		if( childPID == 0 ) // child process
		{
			Log("\n Child Activated\n");
		    //--- fb initialize
		    if( init_framebuffer() == 0 ) {
		        Err("init framebuffer failed\n");
		        return 0;
		    }
		    Log("Surfaceflinger initialized.\n");

			Log("\n Child Activated - init server\n");
		    InitServer(); // endless loop
		}
		else
		{
			sleep(3);
			Log("\n Launch Process succeeded\n");
		}
	}
	*/

	fprintf(stderr,"%s\n", RUISVC_VERSION);


	g_forcefbmode = false;
	if( argc > 1 && strcmp(argv[1], "fb")==0)
		g_forcefbmode = true;

	if( argc > 1 && strcmp(argv[1], "verbose")==0)
		LogEnable(1);

	signal (SIGHUP, SIG_IGN);

    //--- fb initialize
    if( init_framebuffer() == 0 ) {
        Err("init framebuffer failed\n");
        return 0;
    }
    Log("Surfaceflinger initialized.\n");

    InitServer(); // endless loop


    // --- never come here
    /*
    int cnt = 0;
    while(1) {
        sleep(1);
        Log("%04d:Idle\n",cnt++);
    }

    //--- exit
    if( exit_framebuffer() == 0 ) {
        Err("exit framebuffer failed\n");
        return 0;
    }

    Log("Framebuffer Exit.\n");
	*/

    fprintf(stderr,"BYE\n");
    return true;
}


#endif
