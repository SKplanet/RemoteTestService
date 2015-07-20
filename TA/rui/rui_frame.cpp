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
#include "jpeglib.h"
#include "../rui/RUIServerPeer.h"

using namespace Gdiplus;

int RA::ProcessFBCMessage()
{
	FBCCMD* pcmd;
	int n;

	//char buf[RUICMD_HEADER];
	if( (n=ReadExact(m_fbcsock, (char*)m_FBCBuf, FBCCMD_HEADER)) <= 0 ) {
		ta_log("\t\tTA ERR ProcessFBCMessage read(%d) - Header", n);
		return 0;
	}

	pcmd = (FBCCMD*)m_FBCBuf;
	pcmd->size = Swap32(pcmd->size) - FBCCMD_HEADER;

	pcmd->data = 0;
	if( pcmd->size > 1024 * 1024 )
	{
		ta_log("\t\tTA ERR ProcessFBCMessage packet broken (cmd:%c size:%d", pcmd->cmd, pcmd->size);
		return 0;
	}
	if( pcmd->size > 0 && pcmd->size < 4090)
	{
		if( (n=ReadExact(m_fbcsock, (char*)&pcmd->data, pcmd->size)) <= 0 ) {
			ta_log("\t\tTA ERR ProcessFBCMessage read(%d) - body", n);
			return -1;
		}
		((char*)&pcmd->data)[pcmd->size] = 0;
	}

	switch( pcmd->cmd )
	{
	case 'i':
		onFBC_screen(pcmd);
		break;
	case 'h':
	case 'u':
		onFBC_screendata(pcmd);
		break;
	case 'k':
		onFBC_filelistdata(pcmd);
		break;
	case 'l':
		onFBC_filedownloaddata(pcmd);
		break;
	case 'o':
		onFBC_fileerr(pcmd);
		break;
	default:
		return 0;
	}

	return 1;
}

int RA::init_screen()
{
	if( m_fbcsock == INVALID_SOCKET )
		return 0;
	//ta_log("FBC trying get dev screen information");

	FBCCMD msg;
	msg.cmd = 'i';
	msg.size= Swap32(FBCCMD_HEADER);

	WaitForSingleObject( m_fbOutputMutex, INFINITE);
	WriteExact(m_fbcsock, (char*)&msg, FBCCMD_HEADER);
	ReleaseMutex( m_fbOutputMutex);

	return 1;
}

void RA::onFBC_screen(FBCCMD* pcmd)
{
	FBCCMD_SCRINFO * pscr = (FBCCMD_SCRINFO*)pcmd;
	m_fbcWidth = (USHORT)Swap32(pscr->width);
	m_fbcHeight = (USHORT)Swap32(pscr->height);
	m_fbcBPP = (USHORT)Swap32(pscr->bpp);
	m_fbcTouch = (USHORT)Swap32(pscr->touchfd);
	if( m_bForceMonkey )
		m_fbcTouch = 0;


	if( m_fbcHandle == 0 )
	{
		//ta_log("FBC screen info Width:%d, Height:%d, BPP:%d TOUCH:%d", m_fbcWidth, m_fbcHeight, m_fbcBPP,m_fbcTouch);
		m_fbcHandle = GlobalAlloc(GMEM_FIXED,m_fbcWidth*m_fbcHeight*32/8);
		CreateStreamOnHGlobal(m_fbcHandle, TRUE, (LPSTREAM*)&m_pIStream);
		m_fbcSize = 0;
		m_fbcFullSize = 0;

		// video buffer initialize for H.264
		DWORD	dwBufferSize;
		if( (m_fbcWidth * m_fbcHeight) >= (1080*1920) ) { // HD screen 1/3 scale
			dwBufferSize = m_fbcWidth/3 * m_fbcHeight/3 * 32/8;
		} else {
			dwBufferSize = m_fbcWidth/2 * m_fbcHeight/2 * 32/8;
		}
		// [neuromos] !Encoder Thread가 Capture Thread보다 먼저 실행된다.
		// [neuromos] 아래 NewBuffer()를 호출하면 m_bufRGB의 내부 버퍼가 다시 설정되므로
		// [neuromos] RUIBufferVideoEncoder에서 m_bufRGB를 읽다가 죽을 수 있다.
		// [neuromos] m_bufRGB를 서버가 시작할 때 화면 최대사이즈로 할당해서 계속 사용하도록 수정.
		m_bufRGB.NewBuffer(dwBufferSize, TRUE);
		//m_bCaptureFlag = FALSE;
		ZeroMemory(m_bufRGB.GetBuffer(), dwBufferSize);
	}

	m_fbcFrameCount = 0;
	m_fbcLastFrameCount = -1;

}

int RA::raw_touch(int mode, int x, int y)
{
	FBCCMD_TOUCH msg;
	msg.cmd = 't';
	msg.size = Swap32( sizeof(FBCCMD_TOUCH));

	msg.mode = Swap32(mode);
	unsigned short s;
	s = x;
	msg.x = Swap16(s);
	s = y;
	msg.y = Swap16(s);

	WaitForSingleObject( m_fbOutputMutex, INFINITE);

	WriteExact(m_fbcsock, (char*)&msg,  sizeof(FBCCMD_TOUCH));

	ReleaseMutex( m_fbOutputMutex);
	return 1;
}

int RA::raw_key(int mode, unsigned int code)
{
	FBCCMD_KEY msg;
	msg.cmd = 'b';
	msg.size = Swap32( sizeof(FBCCMD_KEY));

	msg.mode = Swap32(mode);
	msg.code = Swap32(code);

	WaitForSingleObject( m_fbOutputMutex, INFINITE);

	WriteExact(m_fbcsock, (char*)&msg,  sizeof(FBCCMD_KEY));

	ReleaseMutex( m_fbOutputMutex);
	return 1;
}
int RA::update_req()
{
	if( m_ftp || m_state < TA_STATE_READY || m_state > TA_STATE_CLIENT_DISCONNECT_ERR)
	{
		//TRACE("skip update_request\n");
		return 1;
	}
	FBCCMD msg;
	msg.cmd = m_bFullSizeRequest ? 'u' : 'h';
	m_bFullSizeRequest = 0;
	msg.size=  Swap32(FBCCMD_HEADER);

	int nRtn = 1;
	DWORD result = WaitForSingleObject( m_fbOutputMutex, 100);
	if( result == WAIT_OBJECT_0 )
	{
		nRtn = WriteExact(m_fbcsock, (char*)&msg, FBCCMD_HEADER);

		ReleaseMutex( m_fbOutputMutex);
	}

	return nRtn;
}

//BYTE* output= 0;
//Bitmap *pbm=0;
DWORD FPS_TIME = 0;
int RA::onFBC_screendata(FBCCMD* pcmd)
{
	//TRACE(".");
	//DWORD ts = clock();
	if( FPS_TIME == 0 )
		FPS_TIME = clock();
	DWORD te;

	m_fbcFrameCount++;
	if( this->m_state ==TA_STATE_CLIENT_CONNECTED )
	{
		update_req();
		OnStateChanged();
	}


	EnterCriticalSection(&CriticalSection);
	try {
		//njpegsize = Read_from_fbc((BYTE*)m_fbcHandle);
		int bFullSize;
		//DWORD dwResult = WaitForSingleObject( m_fbBufferMutex, INFINITE);
		DWORD dwResult = WaitForSingleObject( m_fbBufferMutex, 1000);	 // max 1sec waiting
		if( dwResult != WAIT_OBJECT_0 )
		{
			ta_log("\t\tFBC ERR fbc Buffer mutex wait over 1000 milsec (%d) on onFBC_screendata", m_fbBufferMutex);
			LeaveCriticalSection(&CriticalSection);
			return 0;
		}

		bFullSize = (pcmd->cmd != 'h');
		if( pcmd->size > 0 && pcmd->size < 4090)
		{
			memcpy(m_fbcHandle, &pcmd->data, pcmd->size);
		}
		else {
			if( ReadExact(m_fbcsock, (char*)m_fbcHandle, pcmd->size)< 0 ) {
				ReleaseMutex(m_fbBufferMutex);
				LeaveCriticalSection(&CriticalSection);
				return 0;
			}
		}


		m_fbcSize = pcmd->size;
		m_fbcFullSize = bFullSize;

		if( bFullSize && m_screenshotname[0] )
		{
			FILE* f = NULL;
			fopen_s(&f, m_screenshotname, "wb");
			if( f != NULL ) {
				fwrite(m_fbcHandle, pcmd->size, 1, f);
				fclose(f);
			}
			m_screenshotname[0] = 0;
		}
		if( bFullSize == 0 && m_opt_screen == 1 /*h264*/)
		{
			//update_request();
			JpegToRaw((BYTE*)m_fbcHandle, pcmd->size, m_bufRGB.GetBuffer());
		}

		ReleaseMutex(m_fbBufferMutex);
		
		//TRACE("updatecomplete :%d\n", pcmd->size);

		if( m_opt_screen == 1 && bFullSize )
		{
			OnUpdateComplete();
		}
		update_complete(this, pcmd->size, bFullSize);

	}
	catch (...)
	{
		ta_log("\t\tFBC ERR update fbc exception - onFBC_screendata");
		//TRACE("Read_from_fbc::Exception\n");
		LeaveCriticalSection(&CriticalSection);
		return 0;
	}

	LeaveCriticalSection(&CriticalSection);

	
	te = clock();

	m_frameGetSum ++;
	m_tickRecvSum += (te - FPS_TIME);
	FPS_TIME = te;
	m_byteRecvSum += pcmd->size;

	int stat_interval = 30;
	if( m_frameGetSum %stat_interval == 0 )	// every stat_interval frame
	{
		//m_serverFps = (double)m_tickRecvSum / (double)(CLOCKS_PER_SEC) * (double)stat_interval ;
		//m_serverBps = (double)m_tickRecvSum / (double)(CLOCKS_PER_SEC) * (double)(m_byteRecvSum) / 1000;
		m_serverFps = (double)(CLOCKS_PER_SEC) * (double)stat_interval / (double)m_tickRecvSum;
		m_serverBps = (double)(CLOCKS_PER_SEC) * (double)(m_byteRecvSum) / (double)1000 / (double)m_tickRecvSum;
		//TRACE("server fps (%5.2f) kBps(%5.2f) framesize(%5.2f)\n", m_serverFps, m_serverBps, m_byteRecvSum/(stat_interval * 1000.0f));

		if( m_opt_screen ) // h.264
		{
			DWORD frameSend = m_threadEncoder.GetFramesEncoded();
			DWORD byteSend = m_threadEncoder.GetEncodedSize();
			m_clientFps = (double)(CLOCKS_PER_SEC) * (double)(frameSend - m_frameLastSend) / (double)m_tickRecvSum;
			m_clientBps = (double)(CLOCKS_PER_SEC) * (double)(byteSend - m_byteLastSend) / 1000 / (double)m_tickRecvSum;

			m_frameLastSend = frameSend;
			m_byteLastSend = byteSend;
		}
		else {
			m_clientFps = m_serverFps;
			m_clientBps = m_serverBps;
		}
		m_tickRecvSum = 0;
		m_byteRecvSum = 0;
	}

	return pcmd->size;
	
}


static struct jpeg_source_mgr jpegSrcManager;
static JOCTET *jpegBufferPtr;
static size_t jpegBufferLen;
static bool jpegError;
static void JpegInitSource(j_decompress_ptr cinfo);
static boolean JpegFillInputBuffer(j_decompress_ptr cinfo);
static void JpegSkipInputData(j_decompress_ptr cinfo, long num_bytes);
static void JpegTermSource(j_decompress_ptr cinfo);

static void JpegSetSrcManager(j_decompress_ptr cinfo, char *compressedData,
							  int compressedLen);
static void
JpegInitSource(j_decompress_ptr cinfo)
{
  jpegError = false;
}

static boolean
JpegFillInputBuffer(j_decompress_ptr cinfo)
{
  jpegError = true;
  jpegSrcManager.bytes_in_buffer = jpegBufferLen;
  jpegSrcManager.next_input_byte = (JOCTET *)jpegBufferPtr;

  return TRUE;
}

static void
JpegSkipInputData(j_decompress_ptr cinfo, long num_bytes)
{
  if (num_bytes < 0 || (size_t)num_bytes > jpegSrcManager.bytes_in_buffer) {
    jpegError = true;
    jpegSrcManager.bytes_in_buffer = jpegBufferLen;
    jpegSrcManager.next_input_byte = (JOCTET *)jpegBufferPtr;
  } else {
    jpegSrcManager.next_input_byte += (size_t) num_bytes;
    jpegSrcManager.bytes_in_buffer -= (size_t) num_bytes;
  }
}

static void
JpegTermSource(j_decompress_ptr cinfo)
{
  /* No work necessary here. */
}

static void
JpegSetSrcManager(j_decompress_ptr cinfo, char *compressedData, int compressedLen)
{
  jpegBufferPtr = (JOCTET *)compressedData;
  jpegBufferLen = (size_t)compressedLen;

  jpegSrcManager.init_source = JpegInitSource;
  jpegSrcManager.fill_input_buffer = JpegFillInputBuffer;
  jpegSrcManager.skip_input_data = JpegSkipInputData;
  jpegSrcManager.resync_to_restart = jpeg_resync_to_restart;
  jpegSrcManager.term_source = JpegTermSource;
  jpegSrcManager.next_input_byte = jpegBufferPtr;
  jpegSrcManager.bytes_in_buffer = jpegBufferLen;

  cinfo->src = &jpegSrcManager;
}


int RA::JpegToRaw(BYTE *input, int insize, BYTE * output)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

	if( output == 0 )
		return 0;

  cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

  JpegSetSrcManager(&cinfo, (char*)input, insize);

  jpeg_read_header(&cinfo, TRUE);
  cinfo.out_color_space = JCS_EXT_BGRX;
  //cinfo.out_color_space = JCS_RGB;
  cinfo.dct_method = JDCT_FASTEST;

  jpeg_start_decompress(&cinfo);

  JSAMPROW rowPointer[1];
  BYTE* tempRaw = output;

  //COLORREF *pixelPtr;
  for (int dy = 0; cinfo.output_scanline < cinfo.output_height; dy++) {
    rowPointer[0] = (JSAMPROW)tempRaw;
	 jpeg_read_scanlines(&cinfo, rowPointer, 1);
    if (jpegError) {
      break;
    }
	tempRaw += (cinfo.output_width*4);
  }

  if (!jpegError)
    jpeg_finish_decompress(&cinfo);

  jpeg_destroy_decompress(&cinfo);

  return true;
}


int RA::onFBC_filelistdata(FBCCMD* pcmd)
{
	char* buf;
	if( pcmd->size > 0 && pcmd->size < 4090)
	{
		buf = (char*)&pcmd->data;
	}
	else
	{
		buf = (char*)malloc(pcmd->size);
		int n;
		if( (n=ReadExact(m_fbcsock, (char*)buf, pcmd->size)) <= 0 ) {
			ta_log("\t\tTA ERR fbc read(%d) - onFBC_filelistdata", n);
			return 0;
		}
	}

	//-- relay to client
	file_res_filelist(buf, pcmd->size);

	if( pcmd->size >= 4090)
	{
		free(buf);
	}

	return 1;
}

int RA::onFBC_filedownloaddata(FBCCMD* pcmd)
{
	char* buf;
	if( pcmd->size > 0 && pcmd->size < 4090)
	{
		buf = (char*)&pcmd->data;
	}
	else
	{
		buf = (char*)malloc(pcmd->size);
		int n;
		if( (n=ReadExact(m_fbcsock, (char*)buf, pcmd->size)) <= 0 ) {
			ta_log("\t\tTA ERR fbc read(%d) - onFBC_filedownloaddata", n);
			return 0;
		}
	}

	file_res_filedownload(buf, pcmd->size);

	if( pcmd->size >= 4090)
	{
		free(buf);
	}

	return 1;
}


int RA::onFBC_fileerr(FBCCMD* pcmd)
{
	UINT lerr = Swap32(pcmd->err);

	file_svr_err_msg((USHORT)lerr);
	TRACE("onFBC_fileerr:%x\n", pcmd->err);
	return 1;
}

//#define SLOW_TICK 180
//int RA::update_fbc(bool fastmode)
//{
//	int njpegsize;
//
//	//int fastmode = true;
//	//if( m_serverFps > 10.0f )
//	//	fastmode = false;
//	DWORD ts = clock();
//	DWORD te;
//	if ( fastmode )
//	{
//		try {
//			EnterCriticalSection(&CriticalSection);
//
//			FBCCMD msg;
//			msg.cmd = 'h';
//			msg.size=  Swap16(FBCCMD_HEADER);
//			WriteExact(m_fbcsock, (char*)&msg, FBCCMD_HEADER);
//
//			njpegsize = Read_from_fbc((BYTE*)m_fbcHandle);
//			LeaveCriticalSection(&CriticalSection);
//			if( njpegsize <= 0 )
//				return 0;
//
//			//
//		}
//		catch (...)
//		{
//			ta_log("FBC ERR update fbc exception");
//			//TRACE("Read_from_fbc::Exception\n");
//			return 0;
//		}
//	}
//	else {
//		try {
//			EnterCriticalSection(&CriticalSection);
//			FBCCMD msg;
//			msg.cmd = 'u';
//			msg.size= Swap16(FBCCMD_HEADER);
//			WriteExact(m_fbcsock, (char*)&msg, FBCCMD_HEADER);
//
//
//			njpegsize = Read_from_fbc((BYTE*)m_fbcHandle);
//			LeaveCriticalSection(&CriticalSection);
//			if( njpegsize <= 0 )
//				return 0;
//
//			//
//		}
//		catch (...)
//		{
//			ta_log("FBC ERR update fbc exception");
//			//TRACE("Read_from_fbc::Exception\n");
//			return 0;
//		}
//	}
//
//	te = clock();
//
//	m_frameGetSum ++;
//	m_tickRecvSum += (te - ts);
//	m_byteRecvSum += njpegsize;
//
//	int stat_interval = 30;
//	if( m_frameGetSum %stat_interval == 0 )	// every 10 frame
//	{
//		m_serverFps = (double)(stat_interval * CLOCKS_PER_SEC) / (double)m_tickRecvSum ;
//		m_serverBps = (double)(m_byteRecvSum * CLOCKS_PER_SEC)/ (double)(m_tickRecvSum * 1000);
//		//TRACE("server fps (%5.2f) kBps(%5.2f) framesize(%5.2f)\n", m_serverFps, m_serverBps, m_byteRecvSum/(stat_interval * 1000.0f));
//		m_tickRecvSum = 0;
//		m_byteRecvSum = 0;
//	}
//	return njpegsize;
//}

HBITMAP RA::CreateHBITMAP(int width, int height, void** ppvBits)
{
    BITMAPINFO bi;
    bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth       = width;
    bi.bmiHeader.biHeight      = -height;
    bi.bmiHeader.biPlanes      = 1;
    bi.bmiHeader.biBitCount    = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    HBITMAP hBitmap = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, ppvBits, NULL, 0);
    return hBitmap;
}


//void RA::SetUpdateCompleteCallback(fnUpdateComplete pfn, void* pData)
//{
//	m_UpdateCompleteCallback = pfn;
//	m_UpdateCompleteData = pData;
//}


void RA::SetAdminUpdateCompleteCallback(fnUpdateComplete pfn, void* pData)
{
	m_AdminUpdateCompleteCallback = pfn;
	m_AdminUpdateCompleteData = pData;
}

void RA::SetTAStateReportCallback(fnTAStateReport pfn, void* pData)
{
	m_TAStateReportCallback = pfn;
	m_TAStateReportData = pData;
}


void RA::update_complete(RA* pra, int size, int bFullsize)
{
	if( pra->m_AdminUpdateCompleteCallback != NULL) {
		pra->m_AdminUpdateCompleteCallback(pra, pra->m_AdminUpdateCompleteData, size, bFullsize);
	}

	//if( pra->m_UpdateCompleteCallback != NULL) {
	//	pra->m_UpdateCompleteCallback(pra, pra->m_UpdateCompleteData, size, bFullsize);
	//}
}

void RA::TAStateReport(RA* pra, int bdev)
{
	if( pra->m_TAStateReportCallback != NULL) {
		pra->m_TAStateReportCallback(pra, pra->m_TAStateReportData, bdev);
	}
}


/*
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   ImageCodecInfo* pImageCodecInfo = NULL;

   GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}
*/

int RA::screenshot(char* fname)
{
	//USES_CONVERSION;

	char buf[MAX_PATH];
	if( fname[0] == 0 )
	{
		lstrcpy(buf, m_ModulePath);
		lstrcat(buf,"\\screen");
		if(GetFileAttributes(buf) == INVALID_FILE_ATTRIBUTES)
		{
			CreateDirectory(buf, 0);
		}
		time_t timer;
		struct tm t;

		timer = time(NULL);
		localtime_s(&t, &timer);
		sprintf_s(buf, "%s\\screen\\%04d%02d%02d%02d%02d%02d.jpg",
			m_ModulePath,
			t.tm_year+1900, t.tm_mon+1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
		lstrcpy(fname, buf);
	} else {
		lstrcpy(buf, fname);
	}
	
	lstrcpy(m_screenshotname, buf);

	m_bFullSizeRequest = 1;
	//update_request(false);

	/*
	DWORD dwResult = WaitForSingleObject(m_fbBufferMutex, 500);
	Image * img = Image::FromStream(m_pIStream);
	ReleaseMutex(m_fbBufferMutex);

	CLSID clsid;
	GetEncoderClsid(L"image/jpeg", &clsid);

	img->Save(A2W(fname), &clsid);
	delete img;
	*/

	/*
	DWORD dwResult = WaitForSingleObject(m_fbBufferMutex, 500);
	FILE* f = NULL;
	fopen_s(&f, fname, "wb");
	if( f == NULL ) {
		ReleaseMutex(m_fbBufferMutex);
		return 0;
	}

	fwrite(m_fbcHandle, nSize, 1, f);
	ReleaseMutex(m_fbBufferMutex);
	fclose(f);
	
	return nSize;
	*/
	return 1;
}