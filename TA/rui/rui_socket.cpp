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
#include "ruicmd.h"

int RA::ReadExactTimeout(SOCKET sock, char* buf, int len, int timeout)
{

    int n;
    fd_set fds;
    struct timeval tv;

    while (len > 0) {
		//TRACE("Read socket b - ");
        n = recv(sock, buf, len, 0);
		//TRACE("e\n");
        if (n > 0) {

            buf += n;
            len -= n;

        } else if (n == 0) {

            return 0;

        } else {

			errno = WSAGetLastError();

            if (errno != WSAEWOULDBLOCK ) {

                return n;
            }

            FD_ZERO(&fds);
            FD_SET(sock, &fds);
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = (timeout % 1000) * 1000;
            n = select(sock+1, &fds, NULL, &fds, &tv);
            if (n < 0) {
                //TRACE("ReadExact: select");
                return n;
            }
            if (n == 0) {
                errno = ETIMEDOUT;
                return -1;
            }
        }
    }

	//TRACE("Read:%d\n",n);
    return 1;
}

int RA::ReadExact(SOCKET sock,char* buf,int len)
{
  return(ReadExactTimeout(sock,buf,len,MAXCLIENTWAIT));
}

int RA::WriteExact(SOCKET sock, const char *buf, int len)
{
    int n;
    fd_set fds;
    struct timeval tv;


    int totalTimeWaited = 0;

	//if( sock == this->m_clientSock )
		//TRACE("Write(%04d):%01x %01x %01x %01x\n", len, buf[0], buf[1], buf[2], buf[3]);
    //LOCK(psvc->outputMutex);
    while (len > 0) {
		
		//TRACE("Write socket (%d)[",len);
        n = send(sock, buf, len, 0);
		//TRACE("](%d)\n",n);

        if (n > 0) {

            buf += n;
            len -= n;

        } else if (n == 0) {
            ta_log("SOCKET ERR write returned 0 - WriteExact");
            return 0;

        } else {
			errno = WSAGetLastError();

            if (errno != WSAEWOULDBLOCK ) {

                //UNLOCK(psvc->outputMutex);
                return n;
            }

            /* Retry every 1 seconds until we exceed MAXCLIENTWAIT.  We
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
               // TRACE("WriteExact: select");
                //UNLOCK(psvc->outputMutex);
                return n;
            }
            if (n == 0) {
                totalTimeWaited += 5000;
                //totalTimeWaited += 500;
                ta_log("TA SOCK network lag %d msec -WriteExact",totalTimeWaited);
                if (totalTimeWaited >= MAXCLIENTWAIT) {
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

// WSAStringToAddress

int RA::StringToAddr(char *str, IN_ADDR *addr)  {
    if (str == NULL || *str == '\0' || strcmp(str, "any") == 0) {
        addr->s_addr = htonl(INADDR_ANY);
    } else if (strcmp(str, "localhost") == 0) {
        addr->s_addr = htonl(INADDR_LOOPBACK);
    } else {
        struct hostent *hp;
        if ((addr->s_addr = inet_addr(str)) == htonl(INADDR_NONE)) {
            if (!(hp = gethostbyname(str))) {
                return 0;
            }
            addr->s_addr = *(unsigned long *)hp->h_addr;
        }
    }
    return 1;
}

int RA::ListenOnTCPPort(int port, IN_ADDR iface)
{
    struct sockaddr_in addr;
    int sock;
    int one = 1;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = iface.s_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one)) < 0) {
        closesocket(sock);
        return -1;
    }
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        closesocket(sock);
        return -1;
    }
    if (listen(sock, 32) < 0) {
        closesocket(sock);
        return -1;
    }

    return sock;
}


int RA::ConnectToTcpAddr(char *host, int port)
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
        closesocket(sock);
        return -1;
    }

    return sock;
}

int RA::InitListener()
{
	if( m_listenSock != INVALID_SOCKET )
		return 0;

	m_listenInterface.s_addr = htonl(INADDR_ANY);

	//ta_log("Listen on %d", m_listenPort);
    if ((m_listenSock = ListenOnTCPPort(m_listenPort, m_listenInterface)) < 0) {
        ta_log("ListenOnTCPPort failed");
        return 0;
    }

	return 1;
}

void RA::ShutdownSocket(SOCKET sock)
{
    if(sock>=0) {
    	shutdown(sock,SD_BOTH);
    	closesocket(sock);
    }
}

void RA::add_header_sendbuffer(UINT command, UINT idx, UINT size)
{
	RUICMD cmd;

	cmd.cmd = Swap32(command);
	cmd.idx = Swap32(idx);
	cmd.lsize = Swap32(size + RUICMD_HEADER);
	memcpy(m_sendBuf, &cmd, RUICMD_HEADER);
	m_sendBufIdx += RUICMD_HEADER;
/*
	toU32(command);
	add_sendbuffer(m_sBuf,4);

	toU32(idx);
	add_sendbuffer(m_sBuf,4);

	toU32(size + RUICMD_HEADER);
	add_sendbuffer(m_sBuf,4);
*/
}

int RA::add_sendbuffer(BYTE* buf, int size)
{
	if( m_sendBufIdx + size < 500 )
	{
		memcpy(m_sendBuf+m_sendBufIdx, buf, size);
		m_sendBufIdx += size;
		return m_sendBufIdx;
	}
	else
		return 0;
}


int	RA::flush_sendbuffer(SOCKET sock)
{
	if( WriteExact(sock, (const char*)m_sendBuf, m_sendBufIdx) <= 0 )
		return 0;
	//ta_log("flush(%d)",m_sendBufIdx);

	reset_sendbuffer();
	return 1;
}

int	RA::write_sendbuffer(SOCKET sock, unsigned short command, unsigned short idx, BYTE* buffer, int size)
{
	add_header_sendbuffer(command, idx, size);

	flush_sendbuffer(sock);

	if( WriteExact(sock, (const char*)buffer, size) <= 0 )
		return 0;
	//ta_log("write_sendbuf (%d)bytes", size);
	return 1;
}


void RA::reset_sendbuffer()
{
	m_sendBufIdx = 0;
}