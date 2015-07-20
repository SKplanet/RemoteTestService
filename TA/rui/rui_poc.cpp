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
#include <afxinet.h>
#include "stdafx.h"
#include "WinAES.h"


const TCHAR HEX2DEC[256] = 
{
/* 0 1 2 3 4 5 6 7 8 9 A B C D E F */
/* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
/* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
/* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
/* 3 */ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1, -1,-1,-1,-1,
/* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
/* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
/* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
/* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
/* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
/* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
/* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
/* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
/* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
/* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
/* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
/* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
 };

const TCHAR SAFE[256] =
{
/* 0 1 2 3 4 5 6 7 8 9 A B C D E F */
/* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,
/* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
/* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
/* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
/* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
/* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
/* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};

CString KNUrlDecode(CString aSrc)
{
        // Note from RFC1630: "Sequences which start with a percent sign
        // but are not followed by two hexadecimal characters (0-9, A-F) are reserved
        // for future extension"
        TCHAR *pSrc = aSrc.GetBuffer();
        const int SRC_LEN = aSrc.GetLength();
        TCHAR * SRC_END = pSrc + SRC_LEN;
        TCHAR * SRC_LAST_DEC = SRC_END - 2; // last decode able '%' 
        TCHAR *pStart = new TCHAR[SRC_LEN];
        TCHAR *pEnd = pStart;
        TCHAR dec1, dec2;

        while (pSrc < SRC_LAST_DEC)
        {
                if (*pSrc == '%')
                {
                        if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
                                && -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
                        {
                                *pEnd++ = (dec1 << 4) + dec2;
                                pSrc += 3;
                                continue;
                        }
                }
                *pEnd++ = *pSrc++;
        }

        // the last 2- chars
        while (pSrc < SRC_END)
        {
                *pEnd++ = *pSrc++;
        }

        CString result(_T(""));
        size_t length = pEnd - pStart;
        size_t index;

        for (index = 0; index < length; index++)
        {
                result += pStart[index];
        }
        delete [] pStart;
        return result;
}

CString UrlEncode(CString aSrc)
{
        const TCHAR DEC2HEX[16 + 1] = _T("0123456789ABCDEF");
        TCHAR *pSrc = aSrc.GetBuffer();
        const int SRC_LEN = aSrc.GetLength();
        TCHAR *pStart = new TCHAR[SRC_LEN * 3];
        TCHAR *pEnd = pStart;
        TCHAR *SRC_END = pSrc + SRC_LEN;

        for (; pSrc < SRC_END; ++pSrc)
        {
                if (SAFE[*pSrc])
                {
                        *pEnd++ = *pSrc;
                }
                else if (*pSrc >= 256)
                {
                        *pEnd++ = *pSrc;
                }
                else
                {
                        // escape this char
                        *pEnd++ = '%';
                        *pEnd++ = DEC2HEX[*pSrc >> 4];
                        *pEnd++ = DEC2HEX[*pSrc & 0x0F];
                }
        }
        CString result(_T(""));
        size_t length = pEnd - pStart;
        size_t index;
        
        for (index = 0; index < length; index++)
        {
                result += pStart[index];
        }
        delete [] pStart;
        return result;
}


int toHexString(byte* b, int off, int len, byte* out) {

	int olen = 0;
	for (int i = 0; i < len; i++) {
		char c1, c2;
		c1 = (char)((b[off+i]>>4)&0xf);
		c2 = (char)(b[off+i]&0xf);
		c1 = (char)((c1 > 9) ? 'a'+(c1-10) : '0'+c1);
		c2 = (char)((c2 > 9) ? 'a'+(c2-10) : '0'+c2);
		out[olen] = c1; olen++;
		out[olen] = c2; olen++;
	}
	out[olen] = 0;
	return olen;
}


byte fromDigitChar(char ch) {
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;

	return ch;
}

int fromHexaStr(byte* hexaStr, int len, byte* out){

	int i = 0, j = 0;
	if ((len % 2) == 1)
		out[j++] = fromDigitChar(hexaStr[i++]);
	while (i < len) {
		out[j++] = (byte)(
			(fromDigitChar(hexaStr[i]) << 4) | fromDigitChar(hexaStr[i+1]));
		i+=2;
	}
	out[j] = 0;
	return j;
}

DWORD timedateToDword(int yr, int month, int day, int hour, int minute, int sec)
{
	if( yr == 0 )
	{
		SYSTEMTIME st;
		::GetLocalTime(&st);
		yr = st.wYear;
		month = st.wMonth;
		day = st.wDay;
		hour = st.wHour;
		minute = st.wMinute;
		sec = st.wSecond;
	}
	DWORD dValue = ((yr - 1980) << 25) | ((month & 0xF) << 21) | ((day & 0x1F) << 16) | ((hour & 0x1F) << 11) | ((minute & 0x3F) << 5) | (sec / 2);
	return dValue;
}

DWORD timedateDwordPlus(DWORD dValue, int d, int H, int M, int S)
{
	int yr		= ((dValue >> 25) & 0x7F) + 1980;
	int month	= ((dValue >> 21) & 0x0F);
	int day		= ((dValue >> 16) & 0x1F);
	int hour	= ((dValue >> 11) & 0x1F);
	int minute	= ((dValue >> 5)  & 0x3F);
	int sec		= ((dValue )      & 0x1F) * 2;
	
	CTime dTime(yr, month, day, hour, minute, sec);

	CTimeSpan sTime(d, H, M, S);

	CTime rTime = dTime + sTime;

	return timedateToDword(rTime.GetYear(), rTime.GetMonth(), rTime.GetDay(), rTime.GetHour(), rTime.GetMinute(), rTime.GetSecond());
	
}

DWORD timedateDwordMinus(DWORD dValue, DWORD mValue) // dValue - mValue
{
	int yr		= ((dValue >> 25) & 0x7F) + 1980;
	int month	= ((dValue >> 21) & 0x0F);
	int day		= ((dValue >> 16) & 0x1F);
	int hour	= ((dValue >> 11) & 0x1F);
	int minute	= ((dValue >> 5)  & 0x3F);
	int sec		= ((dValue )      & 0x1F) * 2;

	CTime dTime(yr, month, day, hour, minute, sec);

	yr		= ((mValue >> 25) & 0x7F) + 1980;
	month	= ((mValue >> 21) & 0x0F);
	day		= ((mValue >> 16) & 0x1F);
	hour	= ((mValue >> 11) & 0x1F);
	minute	= ((mValue >> 5)  & 0x3F);
	sec		= ((mValue )      & 0x1F) * 2;

	CTime mTime(yr, month, day, hour, minute, sec);

	CTimeSpan ts = dTime - mTime;

	CTime oTime(1980, 1, 1, 0, 0, 0);
	oTime = oTime + ts;

	return timedateToDword(oTime.GetYear(), oTime.GetMonth(), oTime.GetDay(), oTime.GetHour(), oTime.GetMinute(), oTime.GetSecond());
}

CString DwordToTimedate(DWORD dValue)
{
	CString result;

	int yr		= ((dValue >> 25) & 0x7F) + 1980;
	int month	= ((dValue >> 21) & 0x0F);
	int day		= ((dValue >> 16) & 0x1F);
	int hour	= ((dValue >> 11) & 0x1F);
	int minute	= ((dValue >> 5)  & 0x3F);
	int sec		= ((dValue )      & 0x1F) * 2;

	result.Format("%d-%d-%d %d:%d:%d", yr, month, day, hour, minute, sec);
	return result;
}

int RA::RegisterToPOC()
{
#ifdef REDECON
	return 1;
#endif

	if( ! POC_ENABLE )
	{
		ta_log("[Register TA to POC] skipped");
		return 1;
	}
	ta_log("[Register TA to POC]");
	CString strHeaders = _T("Content-Type: application/x-www-form-urlencoded");
	// URL-encoded form variables -
	// name = "John Doe", userid = "hithere", other = "P&Q"
	CString strFormData;
	CString temp;
	strFormData = "id";
	strFormData += "=";
	strFormData += UrlEncode(m_taSerial);
	strFormData += "&";

	strFormData += "model";
	strFormData += "=";
	strFormData += UrlEncode(m_devModel);
	strFormData += "&";

	strFormData += "manufacture";
	strFormData += "=";
	strFormData += UrlEncode(m_devManufacture);
	strFormData += "&";

	strFormData += "build_version";
	strFormData += "=";
	strFormData += UrlEncode(m_devBuild);
	strFormData += "&";

	strFormData += "os";
	strFormData += "=";
	strFormData += "01";
	strFormData += "&";

	strFormData += "sdk_version";
	strFormData += "=";
	strFormData += UrlEncode(m_devSDKVer);
	strFormData += "&";

	strFormData += "display_width";
	strFormData += "=";
	temp.Format("%d", m_fbcWidth);
	strFormData += temp;
	strFormData += "&";

	strFormData += "display_height";
	strFormData += "=";
	temp.Format("%d", m_fbcHeight);
	strFormData += temp;
	strFormData += "&";

	strFormData += "display_dpi";
	strFormData += "=";
	temp.Format("%d", m_fbcBPP);
	strFormData += temp;
	strFormData += "&";

	strFormData += "ip";
	strFormData += "=";
	IN_ADDR ip;
	ip.S_un.S_addr = m_devExternalIP;
	temp.Format("%d.%d.%d.%d", ip.S_un.S_un_b.s_b4, ip.S_un.S_un_b.s_b3, ip.S_un.S_un_b.s_b2, ip.S_un.S_un_b.s_b1);
	strFormData += temp;
	strFormData += "&";

	strFormData += "port";
	strFormData += "=";
	temp.Format("%d", m_devExternalPort);
	strFormData += temp;
	strFormData += "&";

	strFormData += "usim";
	strFormData += "=";
	strFormData += m_devUSIM;
	strFormData += "&";

	strFormData += "phonenumber";
	strFormData += "=";
	strFormData += UrlEncode(m_devPhoneNO);
	strFormData += "&";

	strFormData += "serial";
	strFormData += "=";
	strFormData += UrlEncode(m_devBuildSerial);
	strFormData += "&";

	strFormData += "IMEI";
	strFormData += "=";
	strFormData += UrlEncode(m_devIMEI);

	strFormData += "IMSI";
	strFormData += "=";
	strFormData += UrlEncode(m_devSIMSerial);


	CInternetSession session;
	CHttpConnection *pConnection = NULL;
	CHttpFile *pFile = NULL;

	try {
		if( USE_HTTPS ) {
			pConnection = session.GetHttpConnection(_T(POC_ADDRESS), INTERNET_DEFAULT_HTTPS_PORT, _T(""), _T(""));
			if( pConnection == NULL )
			{
					throw CString("POC connection failed");
			}
			pFile = pConnection->OpenRequest(
				CHttpConnection::HTTP_VERB_POST,
				_T(POC_REGIST_PAGE),
				NULL, 1, NULL, NULL, INTERNET_FLAG_SECURE |
				INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
				INTERNET_FLAG_IGNORE_CERT_DATE_INVALID);
		}
		else {
			pConnection = session.GetHttpConnection(_T(POC_ADDRESS));
			pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST,
				_T(POC_REGIST_PAGE));
		}
		if( pFile == NULL ) {
			throw CString("POC open failed");
		}
	}
	catch (CInternetException *pEX) {
		if(pFile ) {
			pFile->Close();
			delete pFile;
		}

		if( pConnection ) {
			pConnection->Close();
			delete pConnection;
		}

		TCHAR lpszErrorMessage[255];
		pEX->GetErrorMessage(lpszErrorMessage, 255);
		pEX->Delete();
		ta_log("POC Regist Exception:%s", lpszErrorMessage);
		return 0;
	}
	catch (CString e) {
		ta_log("POC Regist Error:%s", e);
		return 0;
	}

	WCHAR Unicode[2048];
	char UTF8Code[4096];
	int nUnicodeSize = MultiByteToWideChar(CP_ACP, 0, strFormData, strFormData.GetLength(), Unicode, 2048);
	int nUTF8CodeSize = WideCharToMultiByte(CP_UTF8, 0, Unicode, nUnicodeSize, UTF8Code, 4096, NULL, NULL);
	

	BOOL result;
	
	try {
		result = pFile->SendRequest(strHeaders, (LPVOID) (LPCTSTR) UTF8Code,
			nUTF8CodeSize);
	}
	catch (...)
	{
		ta_log("POC Regist Error : send request failed");
		result = false;
	}

	if( result != TRUE )
	{
		ta_log("POC Regist Error : send request return false");
		pFile->Close();
		delete pFile;
		pConnection->Close();
		delete pConnection;
		return 0;
	}

	CString line;
	while ( pFile->ReadString(line) != NULL ) {
		// line += "\r\n";
		nUnicodeSize = MultiByteToWideChar(CP_UTF8, 0, line, line.GetLength(), Unicode, 2048);
		nUTF8CodeSize = WideCharToMultiByte(CP_ACP, 0, Unicode, nUnicodeSize, UTF8Code, 4096, NULL, NULL);
		UTF8Code[nUTF8CodeSize] = 0;
		if( nUTF8CodeSize == 0 ) continue;

		if( UTF8Code[0] == '0' )
		{
			//ta_log("[Register TA to POC Success:%s]", UTF8Code); 
			pFile->Close();
			delete pFile;
			pConnection->Close();
			delete pConnection;
			return 1;
		}
		else {
			ta_log("[Register TA to POC Failed:%s]", UTF8Code);
			pFile->Close();
			delete pFile;
			pConnection->Close();
			delete pConnection;
			return 0;
		}
	}

	pFile->Close();
	delete pFile;
	pConnection->Close();
	delete pConnection;
	return 1;
}

void RA::ScheduledDeviceStatusToPOC()
{
	if( m_state < TA_STATE_READY )
		return;

	if( m_state >= TA_STATE_ERR_DEVNOTFOUND && m_UIWnd->GetSafeHwnd() )
	{
		m_UIWnd->FlashWindow(TRUE);
		MessageBeep(-1);
		Beep( 750, 300 );
	}

	if( m_state_tick % 180 == 0 ) // every 3min sec
	{
		if( m_state <= TA_STATE_INIT_DONE )
		{
			DeviceStatusToPOC(0, 0, "Initial state");
		}
		else if( m_state <= TA_STATE_START_SERVICE_PHASE3 )
		{
			DeviceStatusToPOC(1, 0, "Start Service");
		}
		else if( m_state <= TA_STATE_READY )
		{
			DeviceStatusToPOC(2, 0, "Ready to service");
		}
		else if( m_state <= TA_STATE_CLIENT_INCOMING )
		{
			DeviceStatusToPOC(3, 1, "Client Incoming");
		}
		else if( m_state <= TA_STATE_CLIENT_NEGOTIATE )
		{
			DeviceStatusToPOC(3, 2, "Client Negotiate");
		}
		else if( m_state <= TA_STATE_CLIENT_CONNECTED )
		{
			DeviceStatusToPOC(3, 3, "Client Connected");
		}
		else if( m_state <= TA_STATE_CLIENT_DISCONNECT )
		{
			DeviceStatusToPOC(4, 0, "Client Disconnected");
		}
		else if( m_state <= TA_STATE_CLIENT_USER_DISCONNECT )
		{
			DeviceStatusToPOC(4, 1, "Client User Disconnect");
		}
		else if( m_state <= TA_STATE_CLIENT_TIMEOUT )
		{
			DeviceStatusToPOC(4, 2, "Client Timeout Disconnect");
		}
		else if( m_state <= TA_STATE_CLIENT_DISCONNECT_ERR )
		{
			DeviceStatusToPOC(4, 3, "Client Disconnected on Error");
		}
		else if( m_state <= TA_STATE_AFTER_CLIENT_REINIT )
		{
			DeviceStatusToPOC(5, 0, "Reinit after client session ");
		}
		else if( m_state <= TA_STATE_SERVICE_STOP_DONE )
		{
			DeviceStatusToPOC(6, 0, "Service Stop");
		}
		else if ( m_state == TA_STATE_ERR_INIT_RA )
		{
			DeviceStatusToPOC(8, 1, "Service Init Failed");
		}
		else if ( m_state == TA_STATE_ERR_DEVNOTFOUND )
		{
			DeviceStatusToPOC(8, 0, "Device not found error");
		}
		else if ( m_state == TA_STATE_ERR_START_SERVICE )
		{
			DeviceStatusToPOC(8, 1, "Service Error");
		}

		if( m_devBattLvl < 30 )
		{
			CString msg;
			msg.Format("Low battery warning(%d %%)", m_devBattLvl);
			DeviceStatusToPOC(7, 0, msg);
		}

		if( m_devTemperature > 500 )
		{
			CString msg;
			msg.Format("Device temperature high warning(%4.1f)", m_devTemperature / 10.0f);
			DeviceStatusToPOC(7, 1, msg);
		}
	}
}

int RA::DeviceStatusToPOC(int state, int state2, const char* description)
{
	if( ! POC_ENABLE )
	{
		//ta_log("[Device Status to POC] skipped (%d,%d,%s)", state, state2, description);
		return 1;
	}

	//ta_log("\t\t-Device Status to POC : (%d,%d,%s)", state, state2, description);
	CString strHeaders = _T("Content-Type: application/x-www-form-urlencoded");
	// URL-encoded form variables -
	// name = "John Doe", userid = "hithere", other = "P&Q"
	CString strFormData;
	CString temp;

#ifdef REDECON
	strFormData = "devid";
	strFormData += "=";
	strFormData += UrlEncode(m_taSerial);
	strFormData += "&";

	strFormData += "ip";
	strFormData += "=";
	IN_ADDR ip;
	ip.S_un.S_addr = m_devExternalIP;
	temp.Format("%d.%d.%d.%d", ip.S_un.S_un_b.s_b4, ip.S_un.S_un_b.s_b3, ip.S_un.S_un_b.s_b2, ip.S_un.S_un_b.s_b1);
	strFormData += temp;
	strFormData += "&";

	strFormData += "port";
	strFormData += "=";
	temp.Format("%d", m_devExternalPort);
	strFormData += temp;
	strFormData += "&";

	strFormData += "modelno";
	strFormData += "=";
	strFormData += UrlEncode(m_devModel);
	strFormData += "&";

	strFormData += "manufacture";
	strFormData += "=";
	strFormData += UrlEncode(m_devManufacture);
	strFormData += "&";

	strFormData += "build";
	strFormData += "=";
	strFormData += UrlEncode(m_devBuild);
	strFormData += "&";

	strFormData += "sdkver";
	strFormData += "=";
	strFormData += UrlEncode(m_devSDKVer);
	strFormData += "&";

	strFormData += "serial";
	strFormData += "=";
	strFormData += UrlEncode(m_devSerial[m_devIndex]);
	strFormData += "&";

	strFormData += "state";
	strFormData += "=";
	
	if( state < 2 )
	{
		strFormData += UrlEncode("Init");
	}
	else if ( state == 2 )
	{
		strFormData += UrlEncode("Ready");
	}
	else if ( state == 3 )
	{
		strFormData += UrlEncode("Connected");
	}
	else if ( state <= 5 )
	{
		strFormData += UrlEncode("ReInit");
	}
	else 
	{
		strFormData += UrlEncode("Exit");
	}

#else
	strFormData = "id";
	strFormData += "=";
	strFormData += UrlEncode(m_taSerial);
	strFormData += "&";

	strFormData += "model";
	strFormData += "=";
	strFormData += UrlEncode(m_devModel);
	strFormData += "&";

	strFormData += "build_version";
	strFormData += "=";
	strFormData += UrlEncode(m_devBuild);
	strFormData += "&";

	strFormData += "state";
	strFormData += "=";
	temp.Format("%d", state);
	strFormData += temp;
	strFormData += "&";

	strFormData += "state2";
	strFormData += "=";
	temp.Format("%d", state2);
	strFormData += temp;
	strFormData += "&";

	strFormData += "description";
	strFormData += "=";
	if( description && *description)
		temp = description;
	else
		temp = "";
	strFormData += UrlEncode(temp);
#endif

	CInternetSession session;
	CHttpConnection *pConnection = NULL;
	CHttpFile *pFile = NULL;

	try {
#ifdef REDECON
		pConnection = session.GetHttpConnection(_T(POC_ADDRESS));
#else
	if( USE_HTTPS ) {
		pConnection = session.GetHttpConnection(_T(POC_ADDRESS), INTERNET_DEFAULT_HTTPS_PORT, _T(""), _T(""));
	} else {
		pConnection = session.GetHttpConnection(_T(POC_ADDRESS));
	}
#endif
		if( pConnection == NULL )
		{
				throw CString("POC dev state connection failed");
		}
	
#ifdef REDECON
		pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST,
			_T(POC_DEV_STATE_PAGE));
#else
		if( USE_HTTPS ) {
			pFile = pConnection->OpenRequest(
				CHttpConnection::HTTP_VERB_POST,
				_T(POC_DEV_STATE_PAGE),
				NULL, 1, NULL, NULL, INTERNET_FLAG_SECURE |
				INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
				INTERNET_FLAG_IGNORE_CERT_DATE_INVALID);
		} else {
			pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST,
				_T(POC_DEV_STATE_PAGE));
		}
#endif
		if( pFile == NULL ) {
			throw CString("POC dev state open failed");
		}
	}
	catch (CInternetException *pEX) {
		if(pFile ) {
			pFile->Close();
			delete pFile;
		}

		if( pConnection ) {
			pConnection->Close();
			delete pConnection;
		}

		TCHAR lpszErrorMessage[255];
		pEX->GetErrorMessage(lpszErrorMessage, 255);
		pEX->Delete();
		//ta_log("POC dev state Exception:%s", lpszErrorMessage);
		return 0;
	}
	catch (CString e) {
		//ta_log("POC dev state Error:%s", e);
		return 0;
	}

	WCHAR Unicode[2048];
	char UTF8Code[4096];
	int nUnicodeSize = MultiByteToWideChar(CP_ACP, 0, strFormData, strFormData.GetLength(), Unicode, 2048);
	int nUTF8CodeSize = WideCharToMultiByte(CP_UTF8, 0, Unicode, nUnicodeSize, UTF8Code, 4096, NULL, NULL);
	

	BOOL result;
	try {
		result = pFile->SendRequest(strHeaders, (LPVOID) (LPCTSTR) UTF8Code,
			nUTF8CodeSize);
	}
	catch (CInternetException *pEX ) 
	{
		TCHAR lpszErrorMessage[255];
		pEX->GetErrorMessage(lpszErrorMessage, 255);
		pEX->Delete();

		//ta_log("POC dev state Error : send request failed (%s)", lpszErrorMessage);
		result = false;
	}

	if( result != TRUE )
	{
		pFile->Close();
		delete pFile;
		pConnection->Close();
		delete pConnection;
		//ta_log("POC dev state Error : send request return false");
		return 0;
	}

	CString line;
	while ( pFile->ReadString(line) != NULL ) {
		// line += "\r\n";
		if( line.GetLength() > 2047 )
			line = line.Left(2047);

		nUnicodeSize = MultiByteToWideChar(CP_UTF8, 0, line, line.GetLength(), Unicode, 2048);
		nUTF8CodeSize = WideCharToMultiByte(CP_ACP, 0, Unicode, nUnicodeSize, UTF8Code, 4096, NULL, NULL);
		UTF8Code[nUTF8CodeSize] = 0;
		if( nUTF8CodeSize == 0 ) continue;

		if( UTF8Code[0] == '0' )
		{
			//ta_log("[Report Dev State to POC Success:%s]", UTF8Code); 
			pFile->Close();
			delete pFile;
			pConnection->Close();
			delete pConnection;
			return 1;
		}
		else {
#ifndef REDECON
			//ta_log("[Report Dev State to POC Failed:%s]", UTF8Code);
#endif
			pFile->Close();
			delete pFile;
			pConnection->Close();
			delete pConnection;
			return 0;
		}
	}

	pFile->Close();
	delete pFile;
	pConnection->Close();
	delete pConnection;
	return 1;
}
int RA::ClientStatusToPOC(const char* reserve_id, int state, int state2, const char* description)
{
#ifdef REDECON
	return 1;
#endif

	if( ! POC_ENABLE )
	{
		//ta_log("[Client Status to POC] skipped (%s, %d, %d, %s)", reserve_id, state, state2, description);
		return 1;
	}

	if( strcmp(reserve_id, DEVELOPER_CHECK_KEY) == 0 )
	{
		return 1;
	}

	if( m_state <= TA_STATE_READY || m_state >= TA_STATE_AFTER_CLIENT_REINIT )
	{
		return 1;
	}

	ta_log("\t\t=Client Status to POC : (%s, %d, %d, %s)", reserve_id, state, state2, description);
	CString strHeaders = _T("Content-Type: application/x-www-form-urlencoded");
	// URL-encoded form variables -
	// name = "John Doe", userid = "hithere", other = "P&Q"
	CString strFormData;
	CString temp;
	strFormData = "reserve_id";
	strFormData += "=";
	temp = reserve_id;
	strFormData += UrlEncode(temp);
	strFormData += "&";

	strFormData += "state";
	strFormData += "=";
	temp.Format("%d", state);
	strFormData += temp;
	strFormData += "&";

	strFormData += "state2";
	strFormData += "=";
	temp.Format("%d", state2);
	strFormData += temp;
	strFormData += "&";

	strFormData += "description";
	strFormData += "=";
	if( description && *description)
		temp = description;
	else
		temp = "";
	strFormData += UrlEncode(temp);

	CInternetSession session;
	CHttpConnection *pConnection = NULL;
	CHttpFile *pFile = NULL;

	try {
		if( USE_HTTPS ) {
			pConnection = session.GetHttpConnection(_T(POC_ADDRESS), INTERNET_DEFAULT_HTTPS_PORT, _T(""), _T(""));
		} else {
			pConnection = session.GetHttpConnection(_T(POC_ADDRESS));
		}

		if( pConnection == NULL )
		{
				throw CString("POC client state connection failed");
		}
	
		if( USE_HTTPS ) {
			pFile = pConnection->OpenRequest(
				CHttpConnection::HTTP_VERB_POST,
				_T(POC_CLIENT_STATE_PAGE),
				NULL, 1, NULL, NULL, INTERNET_FLAG_SECURE |
				INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
				INTERNET_FLAG_IGNORE_CERT_DATE_INVALID);
		} else {
		pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST,
			_T(POC_CLIENT_STATE_PAGE));
		}

		if( pFile == NULL ) {
			throw CString("POC client state open failed");
		}
	}
	catch (CInternetException *pEX) {
		if(pFile ) {
			pFile->Close();
			delete pFile;
		}

		if( pConnection ) {
			pConnection->Close();
			delete pConnection;
		}

		TCHAR lpszErrorMessage[255];
		pEX->GetErrorMessage(lpszErrorMessage, 255);
		pEX->Delete();
		ta_log("POC client state Exception:%s", lpszErrorMessage);
		return 0;
	}
	catch (CString e) {
		ta_log("POC client state Error:%s", e);
		return 0;
	}

	WCHAR Unicode[2048];
	char UTF8Code[4096];
	int nUnicodeSize = MultiByteToWideChar(CP_ACP, 0, strFormData, strFormData.GetLength(), Unicode, 2048);
	int nUTF8CodeSize = WideCharToMultiByte(CP_UTF8, 0, Unicode, nUnicodeSize, UTF8Code, 4096, NULL, NULL);
	

	BOOL result;
	
	try {
		result = pFile->SendRequest(strHeaders, (LPVOID) (LPCTSTR) UTF8Code,
			nUTF8CodeSize);
	}
	catch (... ) 
	{
		ta_log("POC client state Error : send request failed");
		result = false;
	}

	if( result != TRUE )
	{
		ta_log("POC client state Error : send request return false");
		pFile->Close();
		delete pFile;
		pConnection->Close();
		delete pConnection;
		return 0;
	}

	CString line;
	while ( pFile->ReadString(line) != NULL ) {
		// line += "\r\n";
		if( line.GetLength() > 2047 )
			line = line.Left(2047);

		nUnicodeSize = MultiByteToWideChar(CP_UTF8, 0, line, line.GetLength(), Unicode, 2048);
		nUTF8CodeSize = WideCharToMultiByte(CP_ACP, 0, Unicode, nUnicodeSize, UTF8Code, 4096, NULL, NULL);
		UTF8Code[nUTF8CodeSize] = 0;
		if( nUTF8CodeSize == 0 ) continue;

		if( UTF8Code[0] == '0' )
		{
			//ta_log("[Report Client State to POC Success:%s]", UTF8Code); 
			pFile->Close();
			delete pFile;
			pConnection->Close();
			delete pConnection;
			return 1;
		}
		else {
			ta_log("[Report Client State to POC Failed:%s]", UTF8Code);
			pFile->Close();
			delete pFile;
			pConnection->Close();
			delete pConnection;
			return 0;
		}
	}
	pFile->Close();
	delete pFile;
	pConnection->Close();
	delete pConnection;


	return 1;
}

int RA::GetReservInfoFromPOC(const char* reserve_id)
{
#ifdef REDECON
	return 0;
#endif

	if( ! POC_ENABLE )
	{
		ta_log("[Get Reservation Info From POC] skipped");
		return 1;
	}

	ta_log("[Get Reservation Info From POC]");
	CString strHeaders = _T("Content-Type: application/x-www-form-urlencoded");
	// URL-encoded form variables -
	// name = "John Doe", userid = "hithere", other = "P&Q"
	CString strFormData;
	CString temp;
	strFormData = "reserve_id";
	strFormData += "=";
	temp = reserve_id;
	strFormData += UrlEncode(temp);

	CInternetSession session;
	CHttpConnection *pConnection = NULL;
	CHttpFile *pFile = NULL;

	try {
		if( USE_HTTPS ) {
			pConnection = session.GetHttpConnection(_T(POC_ADDRESS), INTERNET_DEFAULT_HTTPS_PORT, _T(""), _T(""));
			if( pConnection == NULL )
			{
					throw CString("POC get reservation info connection failed");
			}
			pFile = pConnection->OpenRequest(
				CHttpConnection::HTTP_VERB_POST,
				_T(POC_RESERVEID_PAGE),
				NULL, 1, NULL, NULL, INTERNET_FLAG_SECURE |
				INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
				INTERNET_FLAG_IGNORE_CERT_DATE_INVALID);
		}
		else {
			pConnection = session.GetHttpConnection(_T(POC_ADDRESS));
			pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST,
				_T(POC_RESERVEID_PAGE));
		}
	
		if( pFile == NULL ) {
			throw CString("POC get reservation info open failed");
		}
	}
	catch (CInternetException *pEX) {
		if(pFile ) {
			pFile->Close();
			delete pFile;
		}

		if( pConnection ) {
			pConnection->Close();
			delete pConnection;
		}

		TCHAR lpszErrorMessage[255];
		pEX->GetErrorMessage(lpszErrorMessage, 255);
		pEX->Delete();
		ta_log("POC get reservation info Exception:%s", lpszErrorMessage);
		return 0;
	}
	catch (CString e) {
		ta_log("POC get reservation info Error:%s", e);
		return 0;
	}

	WCHAR Unicode[2048];
	char UTF8Code[4096];
	int nUnicodeSize = MultiByteToWideChar(CP_ACP, 0, strFormData, strFormData.GetLength(), Unicode, 2048);
	int nUTF8CodeSize = WideCharToMultiByte(CP_UTF8, 0, Unicode, nUnicodeSize, UTF8Code, 4096, NULL, NULL);
	

	BOOL result;
	
	try {
		result = pFile->SendRequest(strHeaders, (LPVOID) (LPCTSTR) UTF8Code,
			nUTF8CodeSize);
	}
	catch (... )
	{
		ta_log("POC get reservation info Error : send request failed");
		result = false;
	}

	if( result != TRUE )
	{
		ta_log("POC get reservation info Error : send request return false");
		pFile->Close();
		delete pFile;
		pConnection->Close();
		delete pConnection;
		return 0;
	}

	CString line;
	while ( pFile->ReadString(line) != NULL ) {
		// line += "\r\n";
		if( line.GetLength() > 2047 )
			line = line.Left(2047);

		nUnicodeSize = MultiByteToWideChar(CP_UTF8, 0, line, line.GetLength(), Unicode, 2048);
		nUTF8CodeSize = WideCharToMultiByte(CP_ACP, 0, Unicode, nUnicodeSize, UTF8Code, 4096, NULL, NULL);
		UTF8Code[nUTF8CodeSize] = 0;
		if( nUTF8CodeSize == 0 ) continue;

		if( strlen(UTF8Code) > 80)
		{
			char out[256];
			if( Decrypt(UTF8Code, out) > 0 )
			{
				ta_log("[POC reserve_id(%s) : %s]", reserve_id, out);
				char * line = out;
				char* token;
				//while( line != NULL) {
					token = strtok_s(line, "|", &line);
					if( !token) return 0;
					int b1,b2,b3,b4;
					sscanf(token,"%d.%d.%d.%d", &b1,&b2,&b3,&b4);
					if( b1 < 0 || b1 > 255 ) return 0;
					if( b2 < 0 || b2 > 255 ) return 0;
					if( b3 < 0 || b3 > 255 ) return 0;
					if( b4 < 0 || b4 > 255 ) return 0;

					token = strtok_s(line, "|", &line);
					if( !token) return 0;
					int port;
					sscanf(token,"%d", &port);

					token = strtok_s(line, "|", &line);
					if( !token) return 0;
					int sY,sM,sD, sh, sm, ss;
					sscanf(token,"%d-%d-%d %d:%d:%d", &sY,&sM,&sD, &sh, &sm, &ss);
					m_clientValidStart = timedateToDword(sY, sM, sD, sh, sm, ss);

					token = strtok_s(line, "|", &line);
					if( !token) return 0;
					int eY,eM,eD, eh, em, es;
					sscanf(token,"%d-%d-%d %d:%d:%d", &eY,&eM,&eD, &eh, &em, &es);
					m_clientValidEnd = timedateToDword(eY, eM, eD, eh, em, es);

					//SYSTEMTIME st;
					//::GetLocalTime(&st);
					pFile->Close();
					delete pFile;
					pConnection->Close();
					delete pConnection;

					return 1;

				//}
			}
		}
	}
	pFile->Close();
	delete pFile;
	pConnection->Close();
	delete pConnection;

	return 0;
}


int RA::Decrypt(char* enc, char*out)
{
	WinAES aes;

	//char* rawKey = "57c3937cad712da9";
	char* rawKey = "57c3038cad712da9"; // season2
	byte key[ WinAES::KEYSIZE_128 ];
    byte iv[ WinAES::BLOCKSIZE ];

	memcpy( key, rawKey, 16);
	memcpy( iv, rawKey, 16);

	char ciphertext[256];
	char result[256];
	size_t rsize;

	try
    {
		aes.SetKeyWithIv( key, sizeof(key), iv, sizeof(iv) );
		
		int clen = fromHexaStr((byte*)enc, strlen(enc), (byte*)ciphertext);
		rsize = 256;

		ciphertext[clen] = 0;

        if( !aes.Decrypt( (byte*)ciphertext, clen, (byte*)result, rsize ) ) {
            printf("Failed to decrypt cipher text\n");
			return 0;
        }
	}
    catch( const WinAESException& e )
    {
        printf("Exception: %s\n", e.what());
		return 0;
    }

	result[rsize] = 0;
	lstrcpy(out, result);

	printf("result:%s\n",result);

	return 1;
}
