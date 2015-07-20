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
#include <sys/socket.h>

int MaxClientWait = 30000;

#define DEFINE_ZERO 0
#define LISTEN_PARA 32

void InitSockets(svcInfoPtr psvc)
{

	struct sockaddr_in sockaddr;
	int listen_sock;
	int param_one = 1;


    in_addr_t iface = psvc->listenInterface;

    if (psvc->socketState!=SOCKET_INIT)
        return;

    psvc->socketState = SOCKET_READY;

    if(psvc->port > DEFINE_ZERO) {
      Log("Listening for RUICAPSVC connections on TCP port %d\n", psvc->port);

      memset(&sockaddr, 0, sizeof(sockaddr));
      sockaddr.sin_port = htons(psvc->port);
      sockaddr.sin_addr.s_addr = iface;
      sockaddr.sin_family = AF_INET;

      if((listen_sock = socket(AF_INET, SOCK_STREAM, DEFINE_ZERO)) < DEFINE_ZERO) {
    	  Err("ListenOnTCPPort #1");
    	  return;
      }

      if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR,
        		  (char *)&param_one, sizeof(param_one)) < DEFINE_ZERO) {

          Err("ListenOnTCPPort #2");
          close(listen_sock);
          return;
      }

      if (bind(listen_sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < DEFINE_ZERO) {
          Err("ListenOnTCPPort #3");
          close(listen_sock);
          return;
       }

      if (listen(listen_sock, LISTEN_PARA) < DEFINE_ZERO) {
    	  Err("ListenOnTCPPort #4");
    	  close(listen_sock);
    	  return;
       }

      psvc->listenSock = listen_sock;

      /*

        if ((psvc->listenSock = ListenOnTCPPort(psvc->port, iface)) < 0) {
            Err("ListenOnTCPPort");
            return;
        }

        */
    }
}

void ShutdownSockets(svcInfoPtr psvc)
{
    if (psvc->socketState!=SOCKET_READY)
	return;

    psvc->socketState = SOCKET_SHUTDOWN;

    if(psvc->listenSock>-1) {
    	shutdown(psvc->listenSock,SHUT_RDWR);
    	close(psvc->listenSock);
    	psvc->listenSock=-1;
    }

}

void CloseClient(svcInfoPtr psvc)
{
    if( psvc->sock != -1 )
    {
        shutdown(psvc->sock, SHUT_RDWR);
        close(psvc->sock);
        psvc->sock = -1;
    }
}



int ReadExactTimeout(svcInfoPtr psvc, char* buf, int len, int timeout)
{
    int sock = psvc->sock;
    int n;
    fd_set fds;
    struct timeval tv;

    while (len > 0) {
        n = read(sock, buf, len);

        if (n > 0) {

            buf += n;
            len -= n;

        } else if (n == 0) {

            return 0;

        } else {

            if (errno == EINTR)
                continue;

            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                return n;
            }

            FD_ZERO(&fds);
            FD_SET(sock, &fds);
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = (timeout % 1000) * 1000;
            n = select(sock+1, &fds, NULL, &fds, &tv);
            if (n < 0) {
                Err("ReadExact: select");
                return n;
            }
            if (n == 0) {
                errno = ETIMEDOUT;
                return -1;
            }
        }
    }

    return 1;
}

int ReadExact(svcInfoPtr psvc,char* buf,int len)
{
  return(ReadExactTimeout(psvc,buf,len,MaxClientWait));
}

int WriteExact(svcInfoPtr psvc, const char *buf, int len)
{
    int sock = psvc->sock;
    int n;
    fd_set fds;
    struct timeval tv;


    int totalTimeWaited = 0;

    //LOCK(psvc->outputMutex);
    while (len > 0) {


    	n = write(sock, buf, len );

        if (n > 0) {

            buf += n;
            len -= n;

        } else if (n == 0) {
            
            Err("WriteExact: write returned 0?\n");
            return 0;

        } else {
            if (errno == EINTR)
                continue;

            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                //UNLOCK(psvc->outputMutex);
            	Err("WriteExact:%d\n", errno);
                return n;
            }

            /* Retry every 5 seconds until we exceed MaxClientWait.  We
               need to do this because select doesn't necessarily return
               immediately when the other end has gone away */

            FD_ZERO(&fds);
            FD_SET(sock, &fds);
            tv.tv_sec = 5; //timeout / 1000;
            tv.tv_usec = 0; //(timeout % 1000) * 1000;
            n = select(sock+1, NULL, &fds, NULL /* &fds */, &tv);
            if (n < 0) {
       	        if(errno==EINTR)
                    continue;
                Err("WriteExact: select");
                //UNLOCK(psvc->outputMutex);
                return n;
            }
            if (n == 0) {
                totalTimeWaited += 5000;
                //totalTimeWaited += 500;
                Log("network lag %d msec\n",totalTimeWaited);
                if (totalTimeWaited >= MaxClientWait) {
                    errno = ETIMEDOUT;
                    //UNLOCK(psvc->outputMutex);
                    return -1;
                }
            } else {
                totalTimeWaited = 0;
            }
        }
    }
    //UNLOCK(psvc->outputMutex);

    return 1;
}


