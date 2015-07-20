#include "stdafx.h"
#include "RUIBufferAudioDecoderThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RUIBufferAudioDecoderThread::RUIBufferAudioDecoderThread()
{
	m_pieParam          = NULL;
	m_pMsgWnd           = NULL;

	m_pRUIBufferListPCM = NULL;
}

RUIBufferAudioDecoderThread::~RUIBufferAudioDecoderThread()
{
}

void RUIBufferAudioDecoderThread::SetRUIBufferList(RUIBufferList* pRUIBufferListPCM)
{
	m_pRUIBufferListPCM = pRUIBufferListPCM;
}

BOOL RUIBufferAudioDecoderThread::Stop(BOOL bSync)
{
	// [neuromos] RUIBufferAudioDecoderThread는 한번 ThreadProc()로 들어가면 종료될 때 까지 나오지 않는다. 
	// [neuromos] 아래 RUIThread::Stop()에서 bSync로 종료될 때 까지 기다리므로 먼저 bStop Flag를 셋팅한다.
	// [neuromos] RUIBufferVideoEncoder에서 인코딩하다가 m_bStop을 확인 후 종료하면 RUIThread::WorkThread()를 빠져나오게 된다.
	m_decoder.SetStop(TRUE);

	// [neuromos] IPP Encoder가 완전히 종료될 때 까지 기다린다.
	while (m_decoder.IsPlaying())
		Sleep(10);

	return RUIThread::Stop(bSync);
}

BOOL RUIBufferAudioDecoderThread::OnStart()
{
	RUIThread::OnStart();

#ifdef WRITE_ENCODED_FILE
	if (m_decoder.GetSourceFile()->Open(_T("C:\\test.aac"), CFile::modeCreate | CFile::modeWrite))
	{
		m_decoder.SetSourceFileEnable(TRUE);
	}
	if (m_decoder.GetTargetFile()->Open(_T("C:\\test.pcm"), CFile::modeCreate | CFile::modeWrite))
	{
		m_decoder.SetTargetFileEnable(TRUE);
	}
#endif

	m_decoder.SetStop(FALSE);

	return TRUE;
}

void RUIBufferAudioDecoderThread::OnStop()
{
	RUIThread::OnStop();

#ifdef WRITE_ENCODED_FILE
	if (m_decoder.GetSourceFileEnable())
	{
		m_decoder.GetSourceFile()->Close();
		m_decoder.SetSourceFileEnable(FALSE);
	}
	if (m_decoder.GetTargetFileEnable())
	{
		m_decoder.GetTargetFile()->Close();
		m_decoder.SetTargetFileEnable(FALSE);
	}
#endif
}

BOOL RUIBufferAudioDecoderThread::ThreadProc()
{
	if (m_pRUIBufferListPCM == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_decoder.Decode(m_pRUIBufferListPCM);

	// [neuromos] 위 라인에서 Encoding하는 동안 Block되므로 무조건 Stop한다.
	return FALSE;
}
