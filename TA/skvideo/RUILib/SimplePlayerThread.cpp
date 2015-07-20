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

		if ((++nStatCount) % (1000 / SLEEP_TIME_PLAYFILE) == 0) // [neuromos] 1�ʿ� �ѹ��� ����ڷḦ �����´�.
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
	readContext.m_szFileName[0] = 0; // RUIBufferReader�� �̿��ϹǷ� ���ϸ��� ����.

    r_cc.pReadContext = &readContext;

    UMC::Status		umcRes;
    AVSync::Stat	Stat;
	int				nStatCount = 0;
	umcRes = rAVSync.Init(r_cc);

	// [neuromos] ClientRect�� �ƴ϶� WindowRect��!
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

		if ((++nStatCount) % (1000 / SLEEP_TIME_PLAYBUFFER) == 0) // [neuromos] 1�ʿ� �ѹ��� ����ڷḦ �����´�.
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
	// [neuromos] [AviFile] AviFile�� ���� RGB32�� �߰��Ѵ�.
	// [neuromos] ==> UMC::RGB32�� �����ϸ� ������(400x240)�� �� lPitch�� �̻��� ���̴�. (400 x 4 = 1600 �̾�� �ϴµ� 1644)
	// [neuromos] ==> �ٽ� UMC::RGB565�� �����Ѵ�.
	// [neuromos] !==> AviFile�� Raw Format���� �����ϱ� ������(?) ���߿� BytesPerPixel ���� ��������.
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
	cc.uiPrefVideoRender         = UMC::DX_VIDEO_RENDER; // [neuromos] DX �Ǵ� BLT�� �����Ѵ�.
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
//	cc.bSync                     = true; // [neuromos] Live�̱� ������ Sync�� ���� �ʿ䰡 ������?
	cc.bSync                     = false;
//	cc.szOutputAudio
//	cc.szOutputVideo

	// [neuromos] RUIBufferReader�� �����ϱ� ���� �ڵ�
	{
		RUIBufferReaderParams	bufferParams;
		bufferParams.m_pRUIBufferList264 = m_pRUIBufferList264;
		m_ruiBufferReader.Init(&bufferParams);

		m_externalInfo.m_pDataReader = &m_ruiBufferReader;
		cc.uiPrefDataReader          = RUIBUFFER_DATA_READER;
		cc.pExternalInfo             = &m_externalInfo;
	}

	// [neuromos] ClientRect�� �ƴ϶� WindowRect��!
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
	// [neuromos] SimplePlayerThread�� �ѹ� ThreadProc()�� ���� ����� �� ���� ������ �ʴ´�. 
	// [neuromos] �Ʒ� RUIThread::Stop()���� bSync�� ����� �� ���� ��ٸ��Ƿ� ���� bStop Flag�� �����Ѵ�.
	// [neuromos] RUIBufferReader���� �дٰ� m_bStop�� Ȯ�� �� �����ϸ� RUIThread::WorkThread()�� ���������� �ȴ�.
	m_ruiBufferReader.SetStop(TRUE);

	// [neuromos] IPP Player Thread�� ������ ����� �� ���� ��ٸ���.
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

	// [neuromos] Play�ϴ� ���� Block�ǹǷ� ������ Stop�Ѵ�.
	Play();

	return FALSE;
}
