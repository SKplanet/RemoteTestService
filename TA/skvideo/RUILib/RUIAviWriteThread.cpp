#include "stdafx.h"
#include "RUIAviWriteThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUIAviWriteThread::RUIAviWriteThread()
{
	ResetImageSize();

	m_pAvi              = NULL;
	m_pFlipBuffer       = NULL;
	m_pRUIBufferListAVI = NULL;
}

RUIAviWriteThread::~RUIAviWriteThread()
{
	DeleteAviFile();
}

void RUIAviWriteThread::ResetImageSize()
{
	m_nWidth        = 0;
	m_nHeight       = 0;
	m_nBitsPerPixel = 0;
	m_nPitch        = 0;
	m_nImageSize    = 0;
}

void RUIAviWriteThread::NewFlipBuffer()
{
	if (m_nImageSize > 0)
		m_pFlipBuffer = new BYTE[m_nImageSize];
}

void RUIAviWriteThread::DeleteFlipBuffer()
{
	if (m_pFlipBuffer != NULL)
	{
		delete [] m_pFlipBuffer;
		m_pFlipBuffer = NULL;
	}
}

void RUIAviWriteThread::FlipImage(BYTE* pSource)
{
	if (m_pFlipBuffer != NULL && pSource != NULL)
	{
		for (int i = 0; i < m_nHeight; i++)
		{
			CopyMemory(m_pFlipBuffer + m_nPitch * i, pSource + m_nImageSize - m_nPitch - (m_nPitch * i), m_nPitch);
		}
	}
}

BOOL RUIAviWriteThread::NewAviFile(LPCTSTR szFilePath, int nWidth, int nHeight, int nBitsPerPixel)
{
	DeleteAviFile();

	m_pAvi = new CAVIGenerator(szFilePath, nWidth, nHeight, nBitsPerPixel);
	//if (FAILED(m_pAvi->InitEngine()))
	if (0 != m_pAvi->InitEngine())	// camel modify BUGFIX 20130405
	{
		DeleteAviFile();
		return FALSE;
	}

	m_nWidth        = nWidth;
	m_nHeight       = nHeight;
	m_nBitsPerPixel = nBitsPerPixel;
	m_nPitch        = m_nWidth  * m_nBitsPerPixel / 8;
	m_nImageSize    = m_nHeight * m_nPitch;

	NewFlipBuffer();

	if (Run())
	{
		return TRUE;
	}

	DeleteAviFile(); // [neuromos] 이곳에서 m_pRUIBufferListAVI를 FreeAll() 해도 RenderFrame에서 추가하여 찌꺼기가 남을 수 있음.

	return FALSE;
}

BOOL RUIAviWriteThread::DeleteAviFile()
{
	Stop();

	if (m_pAvi != NULL)
	{
		m_pAvi->ReleaseEngine();

		delete m_pAvi;
		m_pAvi = NULL;
	}

	DeleteFlipBuffer();
	ResetImageSize();

	return TRUE;
}

BOOL RUIAviWriteThread::AddFrame(BYTE* pFrame)
{
	if (m_pFlipBuffer != NULL && pFrame != NULL)
	{
		FlipImage(pFrame);

		if (SUCCEEDED(m_pAvi->AddFrame(m_pFlipBuffer)))
			return TRUE;
	}

	return FALSE;
}

BOOL RUIAviWriteThread::OnStart()
{
	RUIThread::OnStart();

	if (m_pAvi == NULL || m_pRUIBufferListAVI == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	return TRUE;
}

void RUIAviWriteThread::OnStop()
{
	RUIThread::OnStop();
}

BOOL RUIAviWriteThread::ThreadProc()
{
	RUIBuffer*	pRUIBuffer;

	while ((pRUIBuffer = m_pRUIBufferListAVI->PopHead()) != NULL)
	{
		if (m_pFlipBuffer != NULL)
		{
			FlipImage(pRUIBuffer->GetBuffer());
			m_pAvi->AddFrame(m_pFlipBuffer);
		}

		delete pRUIBuffer;
	}

	return TRUE;
}
