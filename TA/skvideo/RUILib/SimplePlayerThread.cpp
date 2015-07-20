#include "stdafx.h"
#include "SimplePlayerThread.h"
#include "RUILibDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SLEEP_TIME_PLAYFILE			300
#define SLEEP_TIME_PLAYBUFFER		100

SimplePlayerThread::SimplePlayerThread()
{
	m_hWndOwner         = NULL;

	m_nDisplayLeft      = 0;
	m_nDisplayTop       = 0;
	m_nDisplayWidth     = 0;
	m_nDisplayHeight    = 0;

	m_pieParam          = NULL;
	m_pfnRenderFrameBeforeCallback      = NULL;
	m_pfnRenderFrameBeforeCallbackParam = NULL;
	m_pfnRenderFrameAfterCallback       = NULL;
	m_pfnRenderFrameAfterCallbackParam  = NULL;

	m_nFramesDecoded    = 0;
	m_nFramesRendered   = 0;

	m_pRUIBufferList264 = NULL;

	m_bStoped           = TRUE;
}

SimplePlayerThread::~SimplePlayerThread()
{
}

UMC::Status SimplePlayerThread::PlaySingleFile(AVSync& rAVSync, AVSync::CommonCtl& r_cc, vm_char* szFileName)
{
    UMC::LocalReaderContext		readContext;
    vm_string_strcpy(readContext.m_szFileName, szFileName);

    r_cc.pReadContext = &readContext;

    UMC::Status		umcRes;
    AVSync::Stat	Stat;
	int				nStatCount = 0;
	umcRes = rAVSync.Init(r_cc);

//	r_cc.rectDisp.bottom = (Ipp16s) rAVSync.GetDstFrmHeight();
//	r_cc.rectDisp.right  = (Ipp16s) rAVSync.GetDstFrmWidth();

//	r_cc.rectRange.bottom = (Ipp16s) rAVSync.GetDstFrmHeight();
//	r_cc.rectRange.right  = (Ipp16s) rAVSync.GetDstFrmWidth();

//    rAVSync.ResizeDisplay(r_cc.rectDisp, r_cc.rectRange);

    if (UMC::UMC_OK == umcRes)
		umcRes = rAVSync.Start();

    if (UMC::UMC_OK == umcRes)
		m_bStoped = FALSE;

    while (UMC::UMC_OK == umcRes && rAVSync.IsPlaying())
    {
        vm_time_sleep(SLEEP_TIME_PLAYFILE);

		if ((++nStatCount) % (1000 / SLEEP_TIME_PLAYFILE) == 0) // [neuromos] 1초에 한번씩 통계자료를 가져온다.
		{
			umcRes = rAVSync.GetStat(Stat);

			m_nFramesDecoded  = Stat.uiFramesDecoded;
			m_nFramesRendered = Stat.uiFramesRendered;
		}
    }

    rAVSync.Stop();

	m_bStoped = TRUE;

    return umcRes;
}

UMC::Status SimplePlayerThread::PlaySingleBuffer(AVSync& rAVSync, AVSync::CommonCtl& r_cc)
{
    UMC::LocalReaderContext		readContext;
	readContext.m_szFileName[0] = 0; // RUIBufferReader를 이용하므로 파일명이 없다.

    r_cc.pReadContext = &readContext;

    UMC::Status		umcRes;
    AVSync::Stat	Stat;
	int				nStatCount = 0;
	umcRes = rAVSync.Init(r_cc);

	// [neuromos] ClientRect가 아니라 WindowRect임!
	HWND	hWndOwner = ((HWNDModuleContext*) r_cc.pRenContext)->m_hWnd;
	POINT	ptScreen  = { 0, 0 };
	::ClientToScreen(hWndOwner, &ptScreen);

	r_cc.rectDisp.left   = (Ipp16s) (ptScreen.x);
	r_cc.rectDisp.top    = (Ipp16s) (ptScreen.y);
	r_cc.rectDisp.right  = (Ipp16s) (r_cc.rectDisp.left + m_nDisplayWidth  - 1);
	r_cc.rectDisp.bottom = (Ipp16s) (r_cc.rectDisp.top  + m_nDisplayHeight - 1);

    rAVSync.ResizeDisplay(r_cc.rectDisp, r_cc.rectRange);

    if (UMC::UMC_OK == umcRes)
		umcRes = rAVSync.Start();

    if (UMC::UMC_OK == umcRes)
		m_bStoped = FALSE;

    while (UMC::UMC_OK == umcRes && rAVSync.IsPlaying())
    {
        vm_time_sleep(SLEEP_TIME_PLAYBUFFER);

		if ((++nStatCount) % (1000 / SLEEP_TIME_PLAYBUFFER) == 0) // [neuromos] 1초에 한번씩 통계자료를 가져온다.
		{
			umcRes = rAVSync.GetStat(Stat);

			m_nFramesDecoded  = Stat.uiFramesDecoded;
			m_nFramesRendered = Stat.uiFramesRendered;
		}
    }

    rAVSync.Stop();

	m_bStoped = TRUE;

    return umcRes;
}

BOOL SimplePlayerThread::Play()
{
	if (m_hWndOwner == NULL || ! ::IsWindow(m_hWndOwner) || m_nDisplayWidth == 0 || m_nDisplayHeight == 0)
	{
//		ASSERT(FALSE);
//		return FALSE;
	}

    UMC::Status	umcRes = UMC::UMC_OK;

    AVSync				AVSync;
    AVSync::CommonCtl	cc;

    ippStaticInit();

    UMC::HWNDModuleContext	Context;
    Context.m_hWnd     = m_hWndOwner;
    Context.m_ColorKey = RGB(0, 0, 0);

//	cc.file_list
//  cc.cformat                   = UMC::YV12;
//  cc.cformat                   = UMC::RGB565;
	// [neuromos] [AviFile] AviFile을 위해 RGB32를 추가한다.
	// [neuromos] ==> UMC::RGB32로 셋팅하면 렌더링(400x240)할 때 lPitch가 이상한 값이다. (400 x 4 = 1600 이어야 하는데 1644)
	// [neuromos] ==> 다시 UMC::RGB565로 복귀한다.
	// [neuromos] !==> AviFile은 Raw Format으로 저장하기 때문에(?) 나중에 BytesPerPixel 값을 조정하자.
    cc.cformat                   = UMC::RGB32;
//	cc.iBitDepth
    cc.ulReduceCoeff             = 0;
    cc.ulSplitterFlags           = UMC::VIDEO_SPLITTER; // | UMC::FLAG_VSPL_VIDEO_HEADER_REQ
    cc.ulVideoDecoderFlags       = UMC::FLAG_VDEC_REORDER;
#if 0 // [neuromos] Audio
    cc.ulAudioDecoderFlags       = 0;
#endif
    cc.ulVideoRenderFlags        = 0; // UMC::FLAG_VREN_REORDER
#if 0 // [neuromos] Audio
    cc.ulAudioRenderFlags        = 0;
#endif
    cc.pRenContext               = &Context;
//	cc.pReadContext
    cc.rectDisp.left             = 0;
    cc.rectDisp.top              = 0;
    cc.rectDisp.right            = m_nDisplayWidth;
    cc.rectDisp.bottom           = m_nDisplayHeight;
//	cc.rectRange
    cc.lInterpolation            = IPPI_INTER_NN;
#ifdef UMC_ENABLE_BLT_VIDEO_RENDER
    cc.uiPrefVideoRender         = UMC::BLT_VIDEO_RENDER;
#else
	cc.uiPrefVideoRender         = UMC::DX_VIDEO_RENDER; // [neuromos] DX 또는 BLT를 선택한다.
#endif
#if 0 // [neuromos] Audio
    cc.uiPrefAudioRender         = UMC::DEF_AUDIO_RENDER;
#endif
	cc.uiPrefDataReader          = 0;
//	cc.lDeinterlacing
    cc.uiLimitVideoDecodeThreads = 0; // Auto Threading
//	cc.uiTrickMode
//	cc.uiSelectedVideoID
//	cc.uiSelectedAudioID
    cc.pExternalInfo             = NULL;
    cc.terminate                 = false;
    cc.performance               = false;
    cc.repeat                    = false;
    cc.fullscr                   = false;
    cc.stick                     = true;
    cc.debug                     = true;
    cc.step                      = false;
//	cc.bSync                     = true; // [neuromos] Live이기 때문에 Sync를 맞출 필요가 있을까?
	cc.bSync                     = false;
//	cc.szOutputAudio
//	cc.szOutputVideo

	// [neuromos] RUIBufferReader를 지정하기 위한 코드
	{
		RUIBufferReaderParams	bufferParams;
		bufferParams.m_pRUIBufferList264 = m_pRUIBufferList264;
		m_ruiBufferReader.Init(&bufferParams);

		m_externalInfo.m_pDataReader = &m_ruiBufferReader;
		cc.uiPrefDataReader          = RUIBUFFER_DATA_READER;
		cc.pExternalInfo             = &m_externalInfo;
	}

	// [neuromos] ClientRect가 아니라 WindowRect임!
	cc.rectDisp.left   = m_nDisplayLeft;
	cc.rectDisp.top    = m_nDisplayTop;
	cc.rectDisp.right  = m_nDisplayLeft + m_nDisplayWidth  - 1;
	cc.rectDisp.bottom = m_nDisplayTop  + m_nDisplayHeight - 1;

    ::RECT	rect;
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
    UMC::Rect2UMCRect(rect, cc.rectRange);

	cc.m_pieParam = m_pieParam;

//	umcRes = PlaySingleFile(AVSync, cc, _T("c:\\capture.rgb.h264"));
	umcRes = PlaySingleBuffer(AVSync, cc);

	return TRUE;
}

BOOL SimplePlayerThread::Stop(BOOL bSync)
{
	// [neuromos] SimplePlayerThread는 한번 ThreadProc()로 들어가면 종료될 때 까지 나오지 않는다. 
	// [neuromos] 아래 RUIThread::Stop()에서 bSync로 종료될 때 까지 기다리므로 먼저 bStop Flag를 셋팅한다.
	// [neuromos] RUIBufferReader에서 읽다가 m_bStop을 확인 후 종료하면 RUIThread::WorkThread()를 빠져나오게 된다.
	m_ruiBufferReader.SetStop(TRUE);

	// [neuromos] IPP Player Thread가 완전히 종료될 때 까지 기다린다.
	while (! m_bStoped)
		Sleep(10);

	return RUIThread::Stop(bSync);
}

BOOL SimplePlayerThread::OnStart()
{
	RUIThread::OnStart();

	BYTE				nFrameType       = m_ruiBufferReader.GetFrameType();
	RUIUDPFrameList*	pRUIUDPFrameList = m_ruiBufferReader.GetRUIBufferListRecv();

	if (pRUIUDPFrameList == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pRUIUDPFrameList->SetEnableFrameType(nFrameType);

	m_pieParam->m_dwCacheDataTick = 0;

	return TRUE;
}

void SimplePlayerThread::OnStop()
{
	RUIThread::OnStop();

	BYTE				nFrameType       = m_ruiBufferReader.GetFrameType();
	RUIUDPFrameList*	pRUIUDPFrameList = m_ruiBufferReader.GetRUIBufferListRecv();

	if (pRUIUDPFrameList != NULL)
	{
		pRUIUDPFrameList->ResetEnableFrameType(nFrameType);
		pRUIUDPFrameList->RemoveUDPFrame      (nFrameType);
	}
	else
		ASSERT(FALSE);
}

BOOL SimplePlayerThread::ThreadProc()
{
	if (m_pRUIBufferList264 == NULL)
		return FALSE;

	// [neuromos] Play하는 동안 Block되므로 무조건 Stop한다.
	Play();

	return FALSE;
}
