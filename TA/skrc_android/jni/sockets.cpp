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

int MaxClientWait = 20000;

void InitSockets(svcInfoPtr psvc)
{
    in_addr_t iface = psvc->listenInterface;

    if (psvc->socketState!=SOCKET_INIT)
        return;

    psvc->socketState = SOCKET_READY;

    if(psvc->port>0) {
      Log("Listening for RUICAPSVC connections on TCP port %d\n", psvc->port);

        if ((psvc->listenSock = ListenOnTCPPort(psvc->port, iface)) < 0) {
            Err("ListenOnTCPPort");
            return;
        }
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



int Connect(svcInfoPtr psvc, char *host, int port)
{
    int sock;
    int one = 1;

    Log("Making connection to client on host %s port %d\n", host,port);

    if ((sock = ConnectToTcpAddr(host, port)) < 0) {
        Err("connection failed");
        return -1;
    }

    if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0) {
        Err("fcntl failed");
        close(sock);
        return -1;
    }

    if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one)) < 0) {
        Err("setsockopt failed");
        close(sock);
        return -1;
    }
	
#ifdef CAMEL_TEMP
	{
		int sndbuf = 8192;    /* send 버퍼의 크기 설정 */
		
		setsockopt(sock, IPPROTO_TCP, SO_SNDBUF, &sndbuf, sizeof(sndbuf));
	}
#endif

    return sock;
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

int StringToAddr(char *str, in_addr_t *addr)  {
    if (str == NULL || *str == '\0' || strcmp(str, "any") == 0) {
        *addr = htonl(INADDR_ANY);
    } else if (strcmp(str, "localhost") == 0) {
        *addr = htonl(INADDR_LOOPBACK);
    } else {
        struct hostent *hp;
        if ((*addr = inet_addr(str)) == htonl(INADDR_NONE)) {
            if (!(hp = gethostbyname(str))) {
                return 0;
            }
            *addr = *(unsigned long *)hp->h_addr;
        }
    }
    return 1;
}

int ListenOnTCPPort(int port, in_addr_t iface)
{
    struct sockaddr_in addr;
    int sock;
    int one = 1;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = iface;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one)) < 0) {
        close(sock);
        return -1;
    }
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    if (listen(sock, 32) < 0) {
        close(sock);
        return -1;
    }

    return sock;
}

int ConnectToTcpAddr(char *host, int port)
{
    struct hostent *hp;
    int sock;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if ((addr.sin_addr.s_addr = inet_addr(host)) == htonl(INADDR_NONE))
    {
        if (!(hp = gethostbyname(host))) {
            errno = EINVAL;
            return -1;
        }
        addr.sin_addr.s_addr = *(unsigned long *)hp->h_addr;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&addr, (sizeof(addr))) < 0) {
        close(sock);
        return -1;
    }

    return sock;
}

