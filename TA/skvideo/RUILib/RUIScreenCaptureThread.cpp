#include "stdafx.h"
#include "RUIScreenCaptureThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define	WINDOW_MODE					true // Not in fullscreen mode
#define RGB32_BYTES_PER_PIXEL		4

RUIScreenCaptureThread::RUIScreenCaptureThread()
{
	m_hWnd              = NULL;

	m_bD3DInit          = FALSE;
	m_pSurface          = NULL;
	m_pd3dDevice        = NULL;
	m_pD3D              = NULL;

	m_nCaptureLeft      = 0;
	m_nCaptureTop       = 0;
	m_nCaptureRight     = 800 - 1;
	m_nCaptureBottom    = 480 - 1;

//	m_nFrameRate        = 15;
	SetFrameRate(15); // [neuromos] 초기값 셋팅을 위해 직접 할당하지 않고 함수 호출함.

	m_dwLastCaptureTick  = 0;
	m_dwCaptureTickSum   = 0;
	m_dwCaptureTickCount = 0;

	m_pRUIBufferRGB      = NULL;

#ifdef WRITE_CAPTURE_FILE
	m_bFile             = FALSE;
#endif
}

RUIScreenCaptureThread::~RUIScreenCaptureThread()
{
}

HRESULT	RUIScreenCaptureThread::InitD3D(HWND hWnd)
{
	D3DDISPLAYMODE			ddm;
	D3DPRESENT_PARAMETERS	d3dpp;

	if ((m_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
	{
		ASSERT(FALSE);
		return E_FAIL;
	}

	if (FAILED(m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &ddm)))
	{
		ASSERT(FALSE);
		return E_FAIL;
	}

	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));

	d3dpp.Windowed                   = WINDOW_MODE;
	d3dpp.Flags                      = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	d3dpp.BackBufferFormat           = ddm.Format;
	d3dpp.BackBufferHeight           = m_nDisplayHeight = ddm.Height;
	d3dpp.BackBufferWidth            = m_nDisplayWidth  = ddm.Width;
	d3dpp.MultiSampleType            = D3DMULTISAMPLE_NONE;
	d3dpp.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow              = hWnd;
	d3dpp.PresentationInterval       = D3DPRESENT_INTERVAL_DEFAULT;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

	if (FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pd3dDevice)))
	{
		ASSERT(FALSE);
		return E_FAIL;
	}

	if (FAILED(m_pd3dDevice->CreateOffscreenPlainSurface(ddm.Width, ddm.Height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &m_pSurface, NULL)))
	{
		ASSERT(FALSE);
		return E_FAIL;
	}

	m_bD3DInit = TRUE;

	return S_OK;
}

void RUIScreenCaptureThread::Cleanup()
{
	if (m_pSurface != NULL)
	{
		m_pSurface->Release();
		m_pSurface = NULL;
	}

	if (m_pd3dDevice != NULL)
	{
		m_pd3dDevice->Release();
		m_pd3dDevice = NULL;
	}

	if (m_pD3D != NULL)
	{
		m_pD3D->Release();
		m_pD3D = NULL;
	}

	m_bD3DInit = FALSE;
}

BOOL RUIScreenCaptureThread::Capture(int nCopyLeft, int nCopyTop, int nCopyRight, int nCopyBottom, BYTE* pCopyBuffer)
{
	BOOL	bResult = FALSE;

	m_pd3dDevice->GetFrontBufferData(0, m_pSurface);

	LPD3DXBUFFER	pDxBuf;
	D3DXSaveSurfaceToFileInMemory(&pDxBuf, D3DXIFF_DIB, m_pSurface, NULL, NULL); // Save to Memory

	if (pDxBuf != NULL)
	{
		BYTE*	pDIBBuffer      = (BYTE*) pDxBuf->GetBufferPointer();
		DWORD	dwDIBBufferSize =         pDxBuf->GetBufferSize();

		// [neuromos] !아래 숫자를 struct member로 수정해야 함.
		if (dwDIBBufferSize >= 26)
		{
			DWORD*	pdwHdrOffset = (DWORD*) (pDIBBuffer + 10);
			DWORD*	pdwHdrWidth  = (DWORD*) (pDIBBuffer + 18);
			DWORD*	pdwHdrHeight = (DWORD*) (pDIBBuffer + 22);

			BYTE*	pRGBDataBuffer     = pDIBBuffer + *pdwHdrOffset;
			DWORD	dwRGBLineDataSize  = (*pdwHdrWidth ) * RGB32_BYTES_PER_PIXEL;
			DWORD	dwRGBDataSize      = (*pdwHdrHeight) * dwRGBLineDataSize;

			if ((*pdwHdrOffset + 1 + dwRGBDataSize) <= dwDIBBufferSize)
			{
				bResult = TRUE;

				int		nCopyTopOffset            = (*pdwHdrHeight - 1 - nCopyTop) * (int) dwRGBLineDataSize;
				int		nCopyLeftOffset           = nCopyLeft * RGB32_BYTES_PER_PIXEL;
				BYTE*	pRGBDataBufferCopyLeftTop = pRGBDataBuffer + nCopyTopOffset + nCopyLeftOffset;

				int		nCopyWidth  = nCopyRight  - nCopyLeft + 1;
				int		nCopyHeight = nCopyBottom - nCopyTop  + 1;

				DWORD	dwCopyLineDataSize = nCopyWidth * RGB32_BYTES_PER_PIXEL;

				for (int nLine = 0; nLine < nCopyHeight; nLine++)
				{
					CopyMemory(
						(void*) (pCopyBuffer + (nLine * (int) dwCopyLineDataSize)),
//						(const void*) (pRGBDataBuffer + nCopyTopOffset + nCopyLeftOffset - (nLine * (int) dwRGBLineDataSize)),
						(const void*) (pRGBDataBufferCopyLeftTop                         - (nLine * (int) dwRGBLineDataSize)),
						(size_t) dwCopyLineDataSize);
				}
			}
			else
				ASSERT(FALSE); // 버퍼 크기가 Header 정보보다 작다.
		}
		else
			ASSERT(FALSE); // Header에서 정보를 가져올 수 없다.

		pDxBuf->Release();
	}

	return bResult;
}

BOOL RUIScreenCaptureThread::Capture2(int nCopyLeft, int nCopyTop, int nCopyRight, int nCopyBottom, BYTE* pCopyBuffer)
{
	BOOL	bResult = FALSE;

	m_pd3dDevice->GetFrontBufferData(0, m_pSurface);

	RECT	rcSrc;
	rcSrc.left   = nCopyLeft;
	rcSrc.top    = nCopyTop;
	rcSrc.right  = nCopyRight  + 1;
	rcSrc.bottom = nCopyBottom + 1;

	LPD3DXBUFFER	pDxBuf;
	D3DXSaveSurfaceToFileInMemory(&pDxBuf, D3DXIFF_DIB, m_pSurface, NULL, &rcSrc); // Save to Memory

	if (pDxBuf != NULL)
	{
		BYTE*	pDIBBuffer      = (BYTE*) pDxBuf->GetBufferPointer();
		DWORD	dwDIBBufferSize =         pDxBuf->GetBufferSize();

		// [neuromos] !아래 숫자를 struct member로 수정해야 함.
		if (dwDIBBufferSize >= 26)
		{
			DWORD*	pdwHdrOffset = (DWORD*) (pDIBBuffer + 10);
			DWORD*	pdwHdrWidth  = (DWORD*) (pDIBBuffer + 18);
			DWORD*	pdwHdrHeight = (DWORD*) (pDIBBuffer + 22);

			BYTE*	pRGBDataBuffer    = pDIBBuffer + *pdwHdrOffset;
			DWORD	dwRGBLineDataSize = (*pdwHdrWidth ) * RGB32_BYTES_PER_PIXEL;
			DWORD	dwRGBDataSize     = (*pdwHdrHeight) * dwRGBLineDataSize;

			if ((*pdwHdrOffset + 1 + dwRGBDataSize) <= dwDIBBufferSize)
			{
				bResult = TRUE;

				int		nCopyTopOffset            = (*pdwHdrHeight - 1 - nCopyTop) * (int) dwRGBLineDataSize;
				int		nCopyLeftOffset           = nCopyLeft * RGB32_BYTES_PER_PIXEL;
				BYTE*	pRGBDataBufferCopyLeftTop = pRGBDataBuffer + nCopyTopOffset + nCopyLeftOffset;

				int		nCopyWidth  = nCopyRight  - nCopyLeft + 1;
				int		nCopyHeight = nCopyBottom - nCopyTop  + 1;

				DWORD	dwCopyLineDataSize = nCopyWidth * RGB32_BYTES_PER_PIXEL;

				for (int nLine = 0; nLine < nCopyHeight; nLine++)
				{
					CopyMemory(
						(void*) (pCopyBuffer + (nLine * (int) dwCopyLineDataSize)),
//						(const void*) (pRGBDataBuffer + nCopyTopOffset + nCopyLeftOffset - (nLine * (int) dwRGBLineDataSize)),
						(const void*) (pRGBDataBufferCopyLeftTop                         - (nLine * (int) dwRGBLineDataSize)),
						(size_t) dwCopyLineDataSize);
				}
			}
			else
				ASSERT(FALSE); // 버퍼 크기가 Header 정보보다 작다.
		}
		else
			ASSERT(FALSE); // Header에서 정보를 가져올 수 없다.

		pDxBuf->Release();
	}

	return bResult;
}

BOOL RUIScreenCaptureThread::SetCaptureRect(int nCaptureLeft, int nCaptureTop, int nCaptureRight, int nCaptureBottom)
{
	m_nCaptureLeft   = nCaptureLeft;
	m_nCaptureTop    = nCaptureTop;
	m_nCaptureRight  = nCaptureRight;
	m_nCaptureBottom = nCaptureBottom;

	return TRUE;
}

void RUIScreenCaptureThread::SetFrameRate(UINT nFrameRate)
{
	if (nFrameRate == 0)
		nFrameRate = 1;

	m_nFrameRate  = nFrameRate;
	m_dwFrameTime = 1000 / m_nFrameRate;

	// 초기값 셋팅
	m_nCaptureTimeIndex = 0;
	for (int i = 0; i < CAPTURE_TICK_HISTORY_COUNT; i++)
		m_dwCaptureInterval[i] = 1000 / m_nFrameRate;
}

float RUIScreenCaptureThread::GetCaptureIntervalAvg()
{
	float	fCaptureIntervalSum = 0;

	for (int i = 0; i < CAPTURE_TICK_HISTORY_COUNT; i++)
		fCaptureIntervalSum += (float) m_dwCaptureInterval[i];

	return fCaptureIntervalSum / (float) CAPTURE_TICK_HISTORY_COUNT;
}

DWORD RUIScreenCaptureThread::GetCaptureTickAvg()
{
	if (m_dwCaptureTickCount > 0)
		return m_dwCaptureTickSum / m_dwCaptureTickCount;

	return 0;
}

void RUIScreenCaptureThread::NewRUIBuffer(DWORD dwSize)
{
	m_RUIBufferRGB.NewBuffer(dwSize, TRUE);
}

BOOL RUIScreenCaptureThread::OnStart()
{
	RUIThread::OnStart();

	m_dwLastCaptureTick  = 0;
	m_dwCaptureTickSum   = 0;
	m_dwCaptureTickCount = 0;

	if (m_hWnd != NULL && ::IsWindow(m_hWnd))
	{
		if (SUCCEEDED(InitD3D(m_hWnd)))
		{
#ifdef WRITE_CAPTURE_FILE
			if (m_fileCapture.Open(_T("C:\\capture.rgb"), CFile::modeCreate | CFile::modeWrite))
			{
				m_bFile = TRUE;
			}
#endif

			return TRUE;
		}
	}

	return FALSE;
}

void RUIScreenCaptureThread::OnStop()
{
	RUIThread::OnStop();

	Cleanup();

#ifdef WRITE_CAPTURE_FILE
	if (m_bFile)
	{
		m_fileCapture.Close();
		m_bFile = FALSE;
	}
#endif
}

BOOL RUIScreenCaptureThread::ThreadProc()
{
	if (m_pRUIBufferRGB == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	BOOL	bResult    = FALSE;
	DWORD	dwCurrTick = GetTickCount();

	// [neuromos] Latency를 줄이기 위해 SleepTime을 조정하지 않고 15fps보다 빠르게 캡쳐한다.
	// [neuromos] 중간에 처리하지 않은 Frame은 그냥 삭제된다.
	// [neuromos] ==> 해상도에 비해 Bitrate가 큰 경우 Encoder가 이상 동작을 하기 때문에 FrameRate에 맞춰 캡쳐한다.

#if 0
	// m_nFrameRate를 맞추기 위해 m_dwLastCaptureTick을 이용하여 Thread의 SleepTime을 조정한다.
	if (m_dwLastCaptureTick == 0)
	{
		m_dwLastCaptureTick = dwCurrTick;
	}
	else
	{
		m_dwCaptureInterval[m_nCaptureTimeIndex] = dwCurrTick - m_dwLastCaptureTick;

		float	fCaptureIntervalAvg = GetCaptureIntervalAvg();
		if      ((DWORD) fCaptureIntervalAvg > m_dwFrameTime) DecSleepTime();
		else if ((DWORD) fCaptureIntervalAvg < m_dwFrameTime) IncSleepTime();

		// m_nCaptureTimeIndex 증가
		m_nCaptureTimeIndex = (m_nCaptureTimeIndex + 1) % CAPTURE_TICK_HISTORY_COUNT;

		m_dwLastCaptureTick = dwCurrTick;
	}
#endif

	BYTE*	pBufferRGB = m_RUIBufferRGB.GetBuffer();

	// [neuromos] 아래 Capture2() 함수는 DX 모드에서 실행시간 0ms. BLT 모드는 많은 실행시간 임.
	BOOL	bCaptureResult = Capture2(m_nCaptureLeft, m_nCaptureTop, m_nCaptureRight, m_nCaptureBottom, pBufferRGB);

	if (bCaptureResult)
	{
		m_pRUIBufferRGB->Lock_CopyFrom(pBufferRGB, m_RUIBufferRGB.GetBufferSize());

#ifdef WRITE_CAPTURE_FILE
		if (m_bFile)
		{
			int nCaptureWidth  = m_nCaptureRight  - m_nCaptureLeft + 1;
			int nCaptureHeight = m_nCaptureBottom - m_nCaptureTop  + 1;

			m_fileCapture.Write((const void*) pBufferRGB, nCaptureWidth * nCaptureHeight * RGB32_BYTES_PER_PIXEL);
		}
#endif

		// [neuromos] [checkpoint]
		m_dwCaptureTickSum += (GetTickCount() - dwCurrTick);
		m_dwCaptureTickCount++;

		bResult = TRUE;
	}

	return bResult;
}
