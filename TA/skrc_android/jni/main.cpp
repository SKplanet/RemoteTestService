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

#include "com_rocode_skrc_SKRCServiceJNI.h"

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

svcInfo svc;

static int ProcessClientMessage(svcInfoPtr psvc)
{
    char msg;
    int n = 0;
    int nSize;

    if( (n = ReadExact(psvc, (char*)&msg, 1)) <= 0 ) {
        Err("ProcessClientMessage : read(%d)\n", n);
        return false;
    }


    unsigned int data;
#ifdef PERFORMANCE_REPORT
	struct timeval tS, tE;

	gettimeofday(&tS,NULL);
	psvc->frame_client += ELAPSED(tLast, tS);
#endif
    switch(msg) {

        case 'i':
            //Log("SEND FBC INFO:width(%d),height(%d),bpp(%d)\n",g_fbinfo.width, g_fbinfo.height, g_fbinfo.bpp);
            data = htonl(4*3); /* width,height,bpp */
            WriteExact(psvc, (const char*)&data, sizeof(int));
            data = htonl(g_fbinfo.width); /* width */
            WriteExact(psvc, (const char*)&data, sizeof(int));
            data = htonl(g_fbinfo.height); /* height */
            WriteExact(psvc, (const char*)&data, sizeof(int));
            data = htonl(g_fbinfo.bpp); /* bpp */
            WriteExact(psvc, (const char*)&data, sizeof(int));
            data = htonl((g_fbinfo.version == ICS_SCREENCAP)?1:0); /* ICS */
            WriteExact(psvc, (const char*)&data, sizeof(int));
            break;
        case 'u':


            read_rgb_framebuffer_to_jpeg(psvc);
#ifdef PERFORMANCE_REPORT
	gettimeofday(&tE,NULL);

	psvc->frame_total += ELAPSED(tS, tE);
	tS = tE;
#endif
            break;
        case 'h':

        	g_halfmode = 1;
            read_rgb_framebuffer_to_jpeg(psvc);
            g_halfmode = 0;
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
        default:
            data = htonl(4);
            WriteExact(psvc, (const char*)&data, sizeof(int));
            data = htonl(msg);
            WriteExact(psvc, (const char*)&data, sizeof(int));
    }

    if(( psvc->frame_sent++ % 5) == 0) {
    	g_fbinfo.orientation = get_dev_rotation();
    	//int isScreenOn = get_screen_on();
    	//Log("isScreenOn(%d)\n",isScreenOn);
    }

#ifdef PERFORMANCE_REPORT

            if( (psvc->frame_sent % 10) == 0)
            {
            	double frame_total;
            	double frame_capture;
            	double frame_compress;
            	double frame_colorspace;
            	double frame_tx;
            	double frame_client;
            	double fps;



            	fps = (double)10 * 1000000 / psvc->frame_total;
            	frame_total = psvc->frame_total / 10000;
            	frame_capture = psvc->frame_capture / 10000;
            	frame_compress = psvc->frame_compress / 10000;
            	frame_colorspace = psvc->frame_colorspace / 10000;
            	frame_tx = psvc->frame_tx / 10000;
            	frame_client = psvc->frame_client / 10000;

                if( psvc->frame_sent > 10 )
                {
                	Log("FPS(%5.2f),TOT(%3.0f),CAP(%3.0f),ZIP(%3.0f),CNV(%2.0f),TX(%2.0f),CLI(%2.0f)\n",
                			//psvc->frame_sent-1,
                			fps,
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

    fd_set rfds, efds;
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

    while (1) {

        FD_ZERO(&rfds);
        FD_SET(psvc->sock, &rfds);
        FD_ZERO(&efds);
        FD_SET(psvc->sock, &efds);


        tv.tv_sec = 0; /* 5 sec */
        tv.tv_usec = 200;
        n = select(psvc->sock + 1, &rfds, NULL, &efds, &tv);
        if (n < 0) {
            Err("ReadExact: select");
            break;
        }
        
        if (psvc->sock == -1) {
            Log("client disconnected\n");
            break;
        }

        if (FD_ISSET(psvc->sock, &rfds) || FD_ISSET(psvc->sock, &efds)) {
            if( 0 == ProcessClientMessage(psvc) )
                break;
        }

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


    if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) {
        Err("fcntl failed");
        close(sock);
        return;
    }
    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one)) < 0) {
        Err("setsockopt failed");
        close(sock);
        return;
     }

    psvc->sock = sock;
    if (WriteExact(psvc, psvc->version, strlen(psvc->version)) < 0) {
        Err("NewClient: write");
        ExitClient(psvc);
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

    Log("listener - start(listensock:%d)\n",psvc->listenSock);

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
//    svcInfo svc;
    
    strcpy(svc.version, RUISVC_VERSION);
    svc.listenInterface = htonl(INADDR_ANY);
    svc.sock = -1;
    svc.port=6900;
    svc.socketState=SOCKET_INIT;
    svc.host[0] = 0;
    svc.jpegquality = 90;

    InitSockets(&svc);

    //listenerRun(&svc);

    RunEventLoop(&svc);


}

void ShutdownServer(svcInfoPtr psvc)
{
    ShutdownSockets(psvc);

}

#ifdef CAMEL_REMOVE
int main(int argc, char* argv[])
{
	g_forcefbmode = false;
	if( argc > 1 && strcmp(argv[1], "fb")==0)
		g_forcefbmode = true;

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

    return true;
}

#endif


static JavaVM* g_VM;
static jclass jNativesCls;

#define CB_CLASS "com/rocode/skrc/RCService"
#define CB_CLASS_RC_CB "rotate"
#define CB_CLASS_RC_SIG "(I)V"

jmethodID rotateCallback;
void rotate(int degree);

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	LogEnable(1);

	JNIEnv *env;
	g_VM = vm;
	Log("JNI_OnLoad called");

	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		Log("Failed to get the environment using GetEnv()");
		return -1;
	}

	Log("JNI_OnLoad0: found %s", CB_CLASS);
	jNativesCls = env->FindClass(CB_CLASS);

	Log("JNI_OnLoad: found %s", CB_CLASS);
	if (!jNativesCls) {
		return JNI_ERR;
	}

//	jNativesCls = env->NewGlobalRef(cls);
//	if (!jNativesCls) {
//		return JNI_ERR;
//	}

	rotateCallback = env->GetStaticMethodID(jNativesCls, CB_CLASS_RC_CB, CB_CLASS_RC_SIG);

	Log("JNI_OnLoad: rotateCallback created");
	if (!rotateCallback) {
		Log("Unable to find method %s in %s.", CB_CLASS_RC_CB, CB_CLASS);
		return JNI_ERR;
	}

	Log("JNI_OnLoad success");

	return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved) {
	JNIEnv *env;
	if (jvm->GetEnv((void **)&env, JNI_VERSION_1_4)) {
		return;
	}
	env->DeleteGlobalRef(jNativesCls);


	Log("JNI_OnUnload success");
	return;
}


JNIEXPORT void JNICALL Java_com_rocode_skrc_SKRCServiceJNI_startRuiCapSvc
  (JNIEnv *env, jclass obj) {
	g_forcefbmode = false;
//	if( argc > 1 && strcmp(argv[1], "fb")==0)
//		g_forcefbmode = true;

    //--- fb initialize
    if( init_framebuffer() == 0 ) {
        Err("init framebuffer failed\n");
        return;
    }
    Log("Surfaceflinger initialized.\n");

    InitServer(); // endless loop


    return;
}

JNIEXPORT void JNICALL Java_com_rocode_skrc_SKRCServiceJNI_stopRuiCapSvc
  (JNIEnv *env, jclass obj) {

	return;
}


void rotate(int degree) {
	JNIEnv *env;

	Log("rotate%d", degree);
	if (!g_VM) {
		Log("No JNI VM avaliable.");
		return;
	}

	bool attached = false;
	switch(g_VM->GetEnv((void**)&env, JNI_VERSION_1_6)) {
	case JNI_OK:
		env->CallStaticVoidMethod(jNativesCls, rotateCallback, degree);
		break;
	case JNI_EDETACHED:
		if (g_VM->AttachCurrentThread((JNIEnv **)&env, NULL) != 0) {
			Log("Could not attach current thread");
			break;
		}
		attached = true;
		env->CallStaticVoidMethod(jNativesCls, rotateCallback, degree);
		break;
	case JNI_EVERSION:
		Log("Invalid java version");
		break;
	default:
		Log("Could not GetEnv");
		break;
	}

	if (attached) {
		g_VM->DetachCurrentThread();
	}
	return;
}
