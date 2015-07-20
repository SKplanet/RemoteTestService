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
#include "RUIServerPeer.h"
#include "../skvideo/RUILib/RUISockUtil.h"

int EnumWaveInDeviceCallback(UINT nDeviceID, WAVEINCAPS* pwic, void* pParam)
{
	if (pParam != NULL)
	{
		RA*	pra = (RA*) pParam;

		pra->CheckWaveInDevice(nDeviceID, pwic);

		return 1;
	}

	ASSERT(FALSE);

	return 0;
}

void RA::CheckWaveInDevice(UINT nDeviceID, WAVEINCAPS* pwic)
{
	if (pwic != NULL)
	{
		m_waveInDeviceList.AddWaveInDevice(nDeviceID, pwic->szPname);

		//ta_log(_T("WaveIn Device %d"), nDeviceID);
		//ta_log(_T("\tProduct Name\t: %s"      ), pwic->szPname       );
		
		//ta_log(_T("\tManufacturer ID\t: %d"   ), pwic->wMid          );
		//ta_log(_T("\tProduct ID\t: %d"        ), pwic->wPid          );
		//ta_log(_T("\tDriver Version\t: %08X"  ), pwic->vDriverVersion);		
		//ta_log(_T("\tSupported Format\t: %08X"), pwic->dwFormats     );
		//ta_log(_T("\tSupported Channel\t: %d" ), pwic->wChannels     );
	}
}

void RA::Video_StartServer()
{
	//ta_log("Start video server");
	Video_StopServer(FALSE);

	if (m_TCPSockServer.StartServer())
	{
		m_bServerStart = TRUE;
		// 새로운 서버가 필요하다면 m_TCPSockServer.StartServer()를 호출한다.
		// Log는 m_TCPSockServer.StartServer()로 부터 WM_START_SERVER를 받아서 생성한다.
		// RA::Video_StartServer()를 호출하면 모든 Server가 Stop되고 하나가 Start한다.
		// 즉, 여기서 Log를 남기면 m_TCPSockServer.StartServer()를 통해 생성하는 서버의 Log를 남길 수 없다.

		//OnStartServer();

		Video_StartStreamingThread();
		Video_StartEncoder();
	}
}

void RA::Video_StopServer(BOOL bLog)
{
	TRACE("Video_StopServer - Video_StopStreamingThread\n");
	Video_StopStreamingThread();

	TRACE("Video_StopServer - Video_StopEncoder\n");
	Video_StopEncoder();

	TRACE("Video_StopServer - m_TCPSockServer.StopServer\n");
	m_TCPSockServer.StopServer();


	// [neuromos] !m_TCPSockServer - RUIServerPeer::m_pUDPSock이 초기화 안됨. StopStreamingThread()에서 모두 삭제됨. 주의할 것!
	TRACE("Video_StopServer - Done\n");

	//m_bServerStart = FALSE;

	if (bLog)
		ta_log(_T("[H.264 Server Stopped.]"));

	//Video_OnStopServer();
}

BOOL RA::Video_CreateUDPSock(RUIServerPeer* pPeer, LPCTSTR szIP)
{
	if (/*pPeer != NULL &&*/ szIP != NULL)
	{
		RUIUDPSock*	pUDPSock = m_UDPSockServer.AddUDPSock();
		pUDPSock->SetMessageWindow(NULL);

		char	szIPMultibyte[_MAX_PATH];
		IP_WidecharToMultibyte(szIP, szIPMultibyte, _MAX_PATH);

		TCHAR	szLocalAddress[_MAX_PATH];
		GetLocalAddress(szLocalAddress, _MAX_PATH);

		BOOL	bOpen = FALSE;

		if (IsLocalAddressWidechar(szIP))
			bOpen = pUDPSock->OpenSend(0, szIPMultibyte, m_listenPort); //VIDEO_STREAMING_PORT);
		else
			bOpen = pUDPSock->OpenStun(0, szIPMultibyte, m_listenPort, m_listenPort); //VIDEO_STREAMING_PORT, VIDEO_STREAMING_PORT);

		if (bOpen)
		{
			if( pPeer)
				pPeer->SetUDPSock(pUDPSock);
			return TRUE;
		}
		else
			m_UDPSockServer.RemoveUDPSock(pUDPSock);
	}

	ASSERT(FALSE);

	return FALSE;
}

BOOL RA::Video_DestroyUDPSock(RUIServerPeer* pPeer)
{
	if (pPeer == NULL)
	{
		m_UDPSockServer.CloseFirstUDPSock();
		return TRUE;
	}

	RUIUDPSock*	pUDPSock = pPeer->GetUDPSock();

	if (pUDPSock != NULL)
	{
		pUDPSock->Close();

		m_UDPSockServer.RemoveUDPSock(pUDPSock);

		if (pPeer != NULL)
			pPeer->SetUDPSock(NULL);

		return TRUE;
	}

	return FALSE;
}


void RA::Video_StartEncoder()
{
	Video_InitThreadValue();

	//DWORD	dwBufferSize = m_fbcWidth/2 * m_fbcHeight/2 * m_fbcBPP/8;

	// [neuromos] !Encoder Thread가 Capture Thread보다 먼저 실행된다.
	// [neuromos] 아래 NewBuffer()를 호출하면 m_bufRGB의 내부 버퍼가 다시 설정되므로
	// [neuromos] RUIBufferVideoEncoder에서 m_bufRGB를 읽다가 죽을 수 있다.
	// [neuromos] m_bufRGB를 서버가 시작할 때 화면 최대사이즈로 할당해서 계속 사용하도록 수정.
	//m_bufRGB.NewBuffer(dwBufferSize, TRUE);
	//m_bCaptureFlag = FALSE;
	//ZeroMemory(m_bufRGB.GetBuffer(), dwBufferSize);

	// [neuromos] Player는 Encoder보다 먼저 실행되어야 한다. Encoder는 Capture보다 먼저 실행되어야 한다.
	// [neuromos] 각 Thread가 실행될 때까지 100ms씩 기다린다.
	// [neuromos] !==> Capture가 Encoder보다 먼저 실행되어야 한다.
	// [neuromos] !==> Encoder가 먼저 실행되면 Client에서 Decoding이 안된다.
	// [neuromos] !==> 첫번 째 Capture가 완료될 때까지 Encoder 내부에서 대기하도록 수정.
	Video_StartEncoderThread();		//Sleep(200);
	//Video_StartCaptureThread();		Sleep(200);

	//OnStartEncoding();
}

void RA::Video_StopEncoder()
{
	//Video_StopCaptureThread();
	Video_StopEncoderThread();
}

void RA::Video_InitThreadValue()
{
//	m_threadCapture.SetSleepTime(31); // 초기 Sleep Time. 캡쳐를 진행하면서 Sleep Time을 변경하므로 여기서 꼭 정확하게 지정할 필요가 없다.
//	m_threadCapture.SetSleepTime(10); // ==> Latency를 줄이기 위해 15fps보다 빠르게 캡쳐한다.
//	m_threadCapture.InitD3D(GetSafeHwnd());
//	m_threadCapture.SetCaptureRect(m_nLeft, m_nTop, m_nRight, m_nBottom);
//	m_threadCapture.SetRUIBuffer  (&m_bufRGB      );
//	m_threadCapture.SetCaptureFlag(&m_bCaptureFlag);

	if( (m_fbcWidth * m_fbcHeight) >= (1080*1920) ) { // HD screen 1/3 scale
		m_threadEncoder.SetWidth      (m_fbcWidth/3            );
		m_threadEncoder.SetHeight     (m_fbcHeight/3          );
	} else {
		m_threadEncoder.SetWidth      (m_fbcWidth/2            );
		m_threadEncoder.SetHeight     (m_fbcHeight/2          );
	}
	m_threadEncoder.SetBitRate    (m_nBitRate          );
	m_threadEncoder.SetFrameType  (RUI_FRAME_TYPE_VIDEO);
	m_threadEncoder.SetRUIBuffer  (&m_bufRGB, &m_buf264);
	//m_threadEncoder.SetCaptureFlag(&m_bCaptureFlag     );

	m_threadAudio.SetFrameType     (RUI_FRAME_TYPE_AUDIO);
#ifdef _USE_AAC_CODEC
	m_threadAAC  .SetFrameType     (RUI_FRAME_TYPE_AUDIO);
#endif
	if( m_audioIndex < 0 )
		m_threadAudio.SetWaveInDeviceID(m_audioIndex);
	else {
		CWaveInDevice* pWaveDevice = m_waveInDeviceList.GetAt(m_audioIndex);
		if( pWaveDevice != NULL )
		{
			m_threadAudio.SetWaveInDeviceID(pWaveDevice->GetDeviceID());
		}
		else
		{
			m_threadAudio.SetWaveInDeviceID(-1);
		}

	}

}

/*
void RA::Video_StartCaptureThread()
{
	Video_StopCaptureThread();

	DWORD	dwBufferSize = m_fbcWidth * m_fbcHeight * m_fbcBPP;

//	m_threadCapture.NewRUIBuffer(dwBufferSize);
	// [neuromos] !Encoder Thread가 Capture Thread보다 먼저 실행된다.
	// [neuromos] 아래 NewBuffer()를 호출하면 m_bufRGB의 내부 버퍼가 다시 설정되므로
	// [neuromos] RUIBufferVideoEncoder에서 m_bufRGB를 읽다가 죽을 수 있다.
	// [neuromos] m_bufRGB를 서버가 시작할 때 화면 최대사이즈로 할당해서 계속 사용하도록 수정.
//	m_bufRGB       .NewBuffer   (dwBufferSize, TRUE);

#ifndef _USE_ONLY_AUDIO
	if (m_bStreamingVideo)
		m_threadCapture.Run();
#endif
}

void RA::Video_StopCaptureThread()
{
	m_threadCapture.Stop();
	// [neuromos] !StartCaptureThread()에서 StopCaptureThread()를 호출하는 구조이다.
	// [neuromos] Encoder Thread가 Capture Thread보다 먼저 실행되므로
	// [neuromos] Encoder Thread 생성 전에 m_bufRGB.NewBuffer() 하여도
	// [neuromos] 이곳에서 m_bufRGB.DeleteBuffer()가 호출되면서 Encoder Thread에서 죽을 수 있다.
	// [neuromos] 아래 m_bufRGB.DeleteBuffer()는 수행하지 않도록 수정.
//	m_bufRGB.DeleteBuffer();
}
*/

void RA::Video_StartEncoderThread()
{
//	if (! m_sock.IsRunning()) // Encoder보다 Streaming Socket이 먼저 실행되어야 한다.
//	{
//		ASSERT(FALSE);
//		return;
//	}
	TRACE("Video_StartEncoderThread - Video_StopEncoderThread\n");

	Video_StopEncoderThread();
//	m_buf264.DeleteBufferList(); // StopEncoderThread()에서 처리함.

	// Streaming Socket과 연결한다.
	m_threadEncoder.SetUseSocket(TRUE);
	if (m_bUseUDP) m_threadEncoder.SetRUIBufferSend(m_UDPSockServer.GetSendBufferList());
	else           m_threadEncoder.SetRUIBufferSend(m_TCPSockServer.GetSendBufferList());

#ifndef _USE_ONLY_AUDIO
	if (m_bStreamingVideo)
		m_threadEncoder.Run();
#endif

	//--- Audio ---
#ifdef _USE_AAC_CODEC
	m_threadAudio.SetRUIBufferListPCM(&m_bufPCM);

	m_threadAAC.SetAudioSliceLen(WAVE_FRAME_SIZE);
	m_threadAAC.SetRUIBufferList(&m_bufPCM);

	if (m_bUseUDP) m_threadAAC.SetRUIBufferSend(m_UDPSockServer.GetSendBufferList());
	else           m_threadAAC.SetRUIBufferSend(m_TCPSockServer.GetSendBufferList());
#else
	if (m_bUseUDP) m_threadAudio.SetRUIBufferSend(m_UDPSockServer.GetSendBufferList());
	else           m_threadAudio.SetRUIBufferSend(m_TCPSockServer.GetSendBufferList());
#endif

	if (m_bStreamingAudio)
		m_threadAudio.RecordOpen();

#ifdef _USE_AAC_CODEC
	m_threadAAC.Run();
#endif
	TRACE("Video_StartEncoderThread - Done\n");
}

void RA::Video_StopEncoderThread()
{
	TRACE("Video_StopEncoderThread - m_threadEncoder.Stop\n");
	m_threadEncoder.Stop();
	TRACE("Video_StopEncoderThread - m_threadAudio  .Stop\n");
	m_threadAudio  .Stop();
#ifdef _USE_AAC_CODEC
	TRACE("Video_StopEncoderThread - m_threadAAC    .Stop\n");
	m_threadAAC    .Stop();
#endif

	m_buf264.DeleteBufferList();

	// Streaming Socket과 연결을 해제한다.
	m_threadEncoder.SetUseSocket(FALSE);
	m_threadEncoder.SetRUIBufferSend(NULL);

	//--- Audio ---
	TRACE("Video_StopEncoderThread - m_threadAudio.RecordClose\n");
	m_threadAudio.RecordClose();

#ifdef _USE_AAC_CODEC
	m_threadAudio.SetRUIBufferListPCM(NULL);

	m_threadAAC.SetRUIBufferList(NULL);
	m_threadAAC.SetRUIBufferSend(NULL);
#else
	m_threadAudio.SetRUIBufferSend(NULL);
#endif
	TRACE("Video_StopEncoderThread - Done\n");
}

void RA::Video_StartStreamingThread()
{
	Video_StopStreamingThread();
	if (m_bUseUDP) m_UDPSockServer.Run();
	else           m_TCPSockServer.Run();
}

void RA::Video_StopStreamingThread()
{
	TRACE("Video_StopStreamingThread\n");
	if (m_bUseUDP) m_UDPSockServer.Stop();
	else           m_TCPSockServer.Stop();
	TRACE("Done\n");
}

void RA::onVideoRestartEncoder(RUICMD* pcmd)
{
	//ta_log(_T("[Video Restart Encoder]"));

	Video_StopEncoder();
	Video_StartEncoder();
}

void RA::onVideoStartStreaming(RUICMD* pcmd)
{
	RUIUDPSock*	pUDPSock = m_UDPSockServer.GetFirstUDPSock();
	if( pUDPSock)
		pUDPSock->SetClientStreaming(TRUE);
}

void RA::onVideoSetStreamingMode(RUICMD* pcmd)
{
	BOOL bAudio = 0;
	if( pcmd->idx & 0x00000010 )
	{
		bAudio = 1;
	}

	m_bStreamingAudio = bAudio;

	ta_log("\t\tclient request audio:%s", m_bStreamingAudio ? "ON" : "OFF");

	if (bAudio)
	{
#ifdef _USE_AAC_CODEC
		m_threadAAC.Stop();
#endif

		if (! m_threadAudio.IsRunning())
		{
#ifdef _USE_AAC_CODEC
			m_threadAudio.SetRUIBufferListPCM(&m_bufPCM);

			m_threadAAC.SetAudioSliceLen(WAVE_FRAME_SIZE);
			m_threadAAC.SetRUIBufferList(&m_bufPCM);

			if (m_bUseUDP) m_threadAAC.SetRUIBufferSend(m_UDPSockServer.GetSendBufferList());
			else           m_threadAAC.SetRUIBufferSend(m_TCPSockServer.GetSendBufferList());
#else
			if (m_bUseUDP) m_threadAudio.SetRUIBufferSend(m_UDPSockServer.GetSendBufferList());
			else           m_threadAudio.SetRUIBufferSend(m_TCPSockServer.GetSendBufferList());
#endif

			m_threadAudio.RecordOpen();

#ifdef _USE_AAC_CODEC
			m_threadAAC.Run();
#endif
		}
	}
	else
	{
		m_threadAudio.RecordClose();

#ifdef _USE_AAC_CODEC
		m_threadAudio.Stop();

		m_threadAudio.SetRUIBufferListPCM(NULL);

		m_threadAAC.SetRUIBufferList(NULL);
		m_threadAAC.SetRUIBufferSend(NULL);
#else
		m_threadAudio.SetRUIBufferSend(NULL);
#endif
	}
}
