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
	// [neuromos] RUIBufferAudioDecoderThread�� �ѹ� ThreadProc()�� ���� ����� �� ���� ������ �ʴ´�. 
	// [neuromos] �Ʒ� RUIThread::Stop()���� bSync�� ����� �� ���� ��ٸ��Ƿ� ���� bStop Flag�� �����Ѵ�.
	// [neuromos] RUIBufferVideoEncoder���� ���ڵ��ϴٰ� m_bStop�� Ȯ�� �� �����ϸ� RUIThread::WorkThread()�� ���������� �ȴ�.
	m_decoder.SetStop(TRUE);

	// [neuromos] IPP Encoder�� ������ ����� �� ���� ��ٸ���.
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

	// [neuromos] �� ���ο��� Encoding�ϴ� ���� Block�ǹǷ� ������ Stop�Ѵ�.
	return FALSE;
}
