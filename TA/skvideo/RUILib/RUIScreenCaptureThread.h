#ifndef _RUISCREENCAPTURE_H
#define _RUISCREENCAPTURE_H

#include "../Common/RUIDef.h"
#include "RUIThread.h"
#include <d3d9.h>
#include <d3dx9.h>
#include "RUIBuffer.h"

// [neuromos] *ĸ�� �۾� ������ �ð� �������� Capture Rate�� �Ǵ��ϰ� Sleep Time�� ����
// [neuromos] �ð� ���� �ϳ��� ������ Sleep Time�� �����ϴ� ���� �ƴ϶� �Ʒ��� ������ ���� ��ŭ�� ����(���)�� ���� �����Ѵ�.
#define CAPTURE_TICK_HISTORY_COUNT		10

//#define WRITE_CAPTURE_FILE

class RUIScreenCaptureThread : public RUIThread
{
public:
	RUIScreenCaptureThread();
	virtual ~RUIScreenCaptureThread();

private:
	HWND					m_hWnd;
	BOOL					m_bD3DInit;
	IDirect3D9*				m_pD3D;
	IDirect3DDevice9*		m_pd3dDevice;
	IDirect3DSurface9*		m_pSurface;

	UINT					m_nDisplayWidth;
	UINT					m_nDisplayHeight;

	int						m_nCaptureLeft;
	int						m_nCaptureTop;
	int						m_nCaptureRight;
	int						m_nCaptureBottom;

	UINT					m_nFrameRate;
	DWORD					m_dwFrameTime;
	DWORD					m_dwLastCaptureTick;
	int						m_nCaptureTimeIndex;
	DWORD					m_dwCaptureInterval[CAPTURE_TICK_HISTORY_COUNT];
	DWORD					m_dwCaptureTickSum;
	DWORD					m_dwCaptureTickCount;

	RUIBuffer				m_RUIBufferRGB;
	RUIBuffer*				m_pRUIBufferRGB;

#ifdef WRITE_CAPTURE_FILE
	BOOL					m_bFile;
	CFile					m_fileCapture;
#endif

public:
	HWND					GetHWND()          { return m_hWnd; }
	void					SetHWND(HWND hWnd) { m_hWnd = hWnd; }

	HRESULT					InitD3D(HWND hWnd);
	void					Cleanup();

	BOOL					Capture(int nCopyLeft, int nCopyTop, int nCopyRight, int nCopyBottom, BYTE* pCopyBuffer);
	BOOL					Capture2(int nCopyLeft, int nCopyTop, int nCopyRight, int nCopyBottom, BYTE* pCopyBuffer);

	BOOL					SetCaptureRect(int nCaptureLeft, int nCaptureTop, int nCaptureRight, int nCaptureBottom);
	UINT					GetFrameRate()                         { return m_nFrameRate;             }
	void					SetFrameRate(UINT nFrameRate);
	DWORD					GetFramesCaptured()                    { return m_dwCaptureTickCount;     }
	float					GetCaptureIntervalAvg();
	DWORD					GetCaptureTickAvg();

	void					NewRUIBuffer(DWORD dwSize);
	void					SetRUIBuffer(RUIBuffer* pRUIBufferRGB) { m_pRUIBufferRGB = pRUIBufferRGB; }

public:
	virtual BOOL			OnStart();
	virtual void			OnStop();
	virtual BOOL			ThreadProc();
};

#endif
