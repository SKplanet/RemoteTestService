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
#include "RUISockUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning(disable:4996)

LPCSTR IP_ByteToMultibyte(BYTE nField0, BYTE nField1, BYTE nField2, BYTE nField3, LPSTR szIPMultibyte)
{
	if (szIPMultibyte != NULL)
	{
		char	szField0[20];
		char	szField1[20];
		char	szField2[20];
		char	szField3[20];

		itoa(nField0, szField0, 10);
		itoa(nField1, szField1, 10);
		itoa(nField2, szField2, 10);
		itoa(nField3, szField3, 10);

		strcpy(szIPMultibyte, szField0); strcat(szIPMultibyte, ".");
		strcat(szIPMultibyte, szField1); strcat(szIPMultibyte, ".");
		strcat(szIPMultibyte, szField2); strcat(szIPMultibyte, ".");
		strcat(szIPMultibyte, szField3);

		return (LPCSTR) szIPMultibyte;
	}

	ASSERT(FALSE);

	return NULL;
}

LPCTSTR	IP_ByteToWidechar(BYTE nField0, BYTE nField1, BYTE nField2, BYTE nField3, LPTSTR szIPWidechar, int nWidecharLen)
{
	char	szIPMultibyte[_MAX_PATH];

	if (IP_ByteToMultibyte(nField0, nField1, nField2, nField3, szIPMultibyte) != NULL)
	{
		return IP_MultibyteToWidechar(szIPMultibyte, szIPWidechar, nWidecharLen);
	}

	ASSERT(FALSE);

	return NULL;
}

LPCSTR IP_LongToMultibyte(ULONG S_addr, LPSTR szIPMultibyte)
{
	if (szIPMultibyte != NULL)
	{
		struct in_addr	in;
		in.s_addr = S_addr;

		strcpy(szIPMultibyte, inet_ntoa(in));

		return (LPCSTR) szIPMultibyte;
	}

	ASSERT(FALSE);

	return NULL;
}

LPCTSTR IP_LongToWidechar(ULONG S_addr, LPTSTR szIPWidechar, int nWidecharLen)
{
	char	szIPMultibyte[_MAX_PATH];

	if (IP_LongToMultibyte(S_addr, szIPMultibyte) != NULL)
	{
		return IP_MultibyteToWidechar(szIPMultibyte, szIPWidechar, nWidecharLen);
	}

	ASSERT(FALSE);

	return NULL;
}

LPCTSTR IP_MultibyteToWidechar(LPCSTR szIPMultibyte, LPTSTR szIPWidechar, int nWidecharLen)
{
	if (szIPMultibyte != NULL && szIPWidechar != NULL)
	{
#ifdef _UNICODE
		MultiByteToWideChar(CP_ACP, 0, szIPMultibyte, -1, szIPWidechar, nWidecharLen);
#else
		_tcscpy(szIPWidechar, szIPMultibyte);
#endif

		return (LPCTSTR) szIPWidechar;
	}

	ASSERT(FALSE);

	return NULL;
}

LPCSTR IP_WidecharToMultibyte(LPCTSTR szIPWidechar, LPSTR szIPMultibyte, int nMultibyteLen)
{
	if (szIPWidechar != NULL && szIPMultibyte != NULL)
	{
#ifdef _UNICODE
        WideCharToMultiByte(CP_ACP, 0, szIPWidechar, -1, szIPMultibyte, nMultibyteLen, NULL, NULL);
#else
		_tcscpy(szIPMultibyte, szIPWidechar);
#endif

		return (LPCSTR) szIPMultibyte;
	}

	ASSERT(FALSE);

	return NULL;
}

BOOL GetLocalAddress(LPTSTR szAddress, UINT nSize)
{
    // Get computer local address
    if (szAddress != NULL && nSize > 0)
    {
        char strHost[_MAX_PATH] = { 0 };

        // get host name, if fail, SetLastError is called
        if (SOCKET_ERROR != gethostname(strHost, sizeof(strHost)))
        {
            struct hostent* hp;
            hp = gethostbyname(strHost);
            if (hp != NULL && hp->h_addr_list[0] != NULL)
            {
                // IPv4: Address is four bytes (32-bit)
                if ( hp->h_length < 4)
                    return false;

                // Convert address to . format
                strHost[0] = 0;

                // IPv4: Create Address string
                sprintf(strHost, "%u.%u.%u.%u",
                    (UINT)(((PBYTE) hp->h_addr_list[0])[0]),
                    (UINT)(((PBYTE) hp->h_addr_list[0])[1]),
                    (UINT)(((PBYTE) hp->h_addr_list[0])[2]),
                    (UINT)(((PBYTE) hp->h_addr_list[0])[3]));

                // check if user provide enough buffer
                if (strlen(strHost) > nSize)
                {
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return false;
                }

            // Unicode conversion
#ifdef _UNICODE
                return (0 != MultiByteToWideChar(CP_ACP, 0, strHost, -1, szAddress, nSize));
#else
                _tcscpy(szAddress, strHost);
                return true;
#endif
            }
        }
    }
    else
        SetLastError(ERROR_INVALID_PARAMETER);
    return false;
}

BOOL IsLocalAddressWidechar(LPCTSTR szIPWidechar)
{
	if (_tcscmp(szIPWidechar, _T("127.0.0.1")) == 0)
		return TRUE;

	TCHAR	szLocalAddress[_MAX_PATH];
	GetLocalAddress(szLocalAddress, _MAX_PATH);

	if (_tcscmp(szIPWidechar, szLocalAddress) == 0)
		return TRUE;

	return FALSE;
}

BOOL IsLocalAddressMultibyte(LPCSTR szIPMultibyte)
{
	TCHAR	szIPWidechar[_MAX_PATH];

	if (IP_MultibyteToWidechar(szIPMultibyte, szIPWidechar, _MAX_PATH))
		return IsLocalAddressWidechar(szIPWidechar);

	return FALSE;
}
