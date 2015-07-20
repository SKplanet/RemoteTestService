#include "stdafx.h"
#include "RUIBufferReader.h"
#include "../Common/RUIDef.h"

// [neuromos] �б� ���ϴ� ũ�Ⱑ 1024(�Ǵ� 16384)������ ���� ���ۿ� �ִ� ũ�⸸ŭ�� �а� �Ķ���͸� �����Ѵ�.
#define _USE_ASSIGNED_BUFFER_LIST_SIZE

using namespace UMC;

RUIBufferReader::RUIBufferReader()
{
	m_pieParam           = NULL;

	m_pRUIBufferList264  = NULL;
	m_u64Position        = 0;
	m_bStop              = FALSE;

	m_bUseSocket         = FALSE;
	m_nFrameType         = 0;
	m_pRUIBufferListRecv = NULL;
}

RUIBufferReader::~RUIBufferReader()
{
    Close();
}

Status RUIBufferReader::Init(DataReaderParams* pInit)
{
    RUIBufferReaderParams* pParams = DynamicCast<RUIBufferReaderParams, DataReaderParams>(pInit);

	// �ʱ�ȭ
	m_pRUIBufferList264 = pParams->m_pRUIBufferList264;

	if (m_pRUIBufferList264 == NULL)
	{
		ASSERT(FALSE);
		return UMC_ERR_OPEN_FAILED;
	}

    return UMC_OK;
}

Status RUIBufferReader::Close()
{
	m_pRUIBufferList264 = NULL;

    return UMC_OK;
}

Status RUIBufferReader::Reset()
{
	m_u64Position = 0;
	m_bStop       = FALSE;

    return UMC_OK;
}

Status RUIBufferReader::ReadData(void* data, Ipp32u *nsize)
{
	// �������̸� UMC_ERR_END_OF_STREAM�� �����Ѵ�. �� Class������ Stop�� ��쿡�� UMC_ERR_END_OF_STREAM.
	if (m_bStop) // [neuromos] �ܺο��� ������Ŵ
	{
		*nsize = 0; // ���� ���� ����.
		return UMC_ERR_END_OF_STREAM;
	}

// [neuromos] *���ϴ� ũ�Ⱑ ���� ������ ����ϹǷ� ���� ������ ���� �� �ִ�.
// [neuromos] �б� ���ϴ� ũ�Ⱑ 1024(�Ǵ� 16384)������ ���� ���ۿ� �ִ� ũ�⸸ŭ�� �а� �Ķ���͸� �����Ѵ�.
#ifdef _USE_ASSIGNED_BUFFER_LIST_SIZE
	DWORD	dwBufferSize = m_pRUIBufferList264->GetBufferSize();

	if (0 < dwBufferSize && dwBufferSize < *nsize)
		*nsize = (Ipp32u) dwBufferSize;
#endif

    Status	umcRes = CacheData(data, nsize, 0);

    MovePosition(*nsize); // Read�� �ϸ� �ڵ����� pointer(File Offset)�� �����ȴ�.

    return UMC_OK;
}

#define _USE_ADD_VIDEO_AUDIO_DIST

void RUIBufferReader::CheckFrameRecvTick(DWORD dwFrameRecvTick)
{
	m_pieParam->m_dwFrameRecvTickAudioLimit = dwFrameRecvTick - m_pieParam->m_dwAudioDelay;

	DWORD	dwCurrTick      = m_pieParam->m_dwCacheDataTick = GetTickCount();
	DWORD	dwFrameYourTick =
			(
				(m_pieParam->m_nRelMyTickSign > 0) ?
				(dwFrameRecvTick + m_pieParam->m_dwRelMyTick) :
				(dwFrameRecvTick - m_pieParam->m_dwRelMyTick)
			);

	if (dwFrameYourTick > dwCurrTick)
		dwFrameYourTick = dwCurrTick;

	DWORD	dwFrameRecvTickDist;
	if (dwCurrTick > dwFrameYourTick) dwFrameRecvTickDist = dwCurrTick - dwFrameYourTick;
	else                              dwFrameRecvTickDist = 0;

	m_pieParam->m_n64FrameRecvTickSum += (__int64) dwFrameRecvTickDist;

	if (m_pieParam->m_dwFrameRecvTickMax < dwFrameRecvTickDist)
		m_pieParam->m_dwFrameRecvTickMax = dwFrameRecvTickDist;

#ifdef _USE_ADD_VIDEO_AUDIO_DIST
	DWORD	dwLastVideoTick = m_pieParam->m_dwLastFrameTick[RUI_FRAME_TYPE_VIDEO];
	DWORD	dwLastAudioTick = m_pieParam->m_dwLastFrameTick[RUI_FRAME_TYPE_AUDIO];

	// [neuromos] Video�� ũ�Ⱑ Ŀ�� ���߿� �������� ������ Video Tick�� �� ũ��.
	if (dwLastVideoTick != 0 && dwLastAudioTick != 0)
	{
		if (dwLastVideoTick >= dwLastAudioTick) { m_pieParam->m_n64FrameRecvTickSum += (__int64) (dwLastVideoTick - dwLastAudioTick); }
		else                                    { m_pieParam->m_n64FrameRecvTickSum += (__int64) (dwLastAudioTick - dwLastVideoTick); }
	}
#endif

	(m_pieParam->m_dwFrameRecvTickCount)++;

	m_pieParam->m_dwFrameRecvTick = (DWORD) (m_pieParam->m_n64FrameRecvTickSum / (__int64) m_pieParam->m_dwFrameRecvTickCount);
}

Status RUIBufferReader::CacheData(void* data, Ipp32u* nsize, Ipp32s how_far)
{
	// how_far�� Offset(�ǳʶٱ�)

	BYTE*	pTarget    = (BYTE*) data;
	DWORD	dwReadSize = (DWORD) (*nsize + how_far);

	if (how_far > 0) // �ǳʶٱⰡ �����Ǹ� ���� ���۸� �غ��Ѵ�. GetAssignedBufferData2 ������ �ǳʶٱ� ������ ��ƴ�.
	{
		pTarget = new BYTE[*nsize + how_far];
	}

	while (! m_pRUIBufferList264->CopyTo(pTarget, dwReadSize, RUIBUFFER_MOVE_FLAG_MULTI_BUFFER | RUIBUFFER_MOVE_FLAG_EXACT_SIZE))
	{
		// ������ ����ϴ� ��� ���ڴ��� m_pRUIBufferList264�� ä���� �ʴ´�.
		// �ֱ������� m_pRUIBufferListRecv�� �߰��� FrameBuffer�� Ȯ���Ͽ� m_pRUIBufferList264�� �ű��.
		if (m_bUseSocket && m_pRUIBufferListRecv != NULL)
		{
			// [neuromos] !�̰����� m_dwFrameRecvTickVideoLimit�� �����Ѵ�.
			DWORD	dwCurrTick = GetTickCount();

			m_pieParam->m_dwFrameRecvTickVideoLimit = 
				(
					(m_pieParam->m_nRelMyTickSign > 0) ?
					(dwCurrTick - m_pieParam->m_dwRelMyTick) :
					(dwCurrTick + m_pieParam->m_dwRelMyTick)
				)
				- m_pieParam->m_dwRenderingDelay;

//			RUIFrameBuffer*	pRUIBufferRecv = m_pRUIBufferListRecv->PopHeadCompleted(m_nFrameType); // m_pRUIBufferListRecv�� �ϼ��� FrameBuffer�� �ִ°�?
			RUIFrameBuffer*	pRUIBufferRecv = m_pRUIBufferListRecv->PopLimitHeadCompleted(m_nFrameType, m_pieParam->m_dwFrameRecvTickVideoLimit);

			if (pRUIBufferRecv != NULL)
			{
				// [neuromos] [checkpoint]
				DWORD	dwFrameYourTick = pRUIBufferRecv->GetParam();
				m_pieParam->m_dwVideoTickCurr = dwFrameYourTick;

				CheckFrameRecvTick(dwFrameYourTick);

				DWORD		dwFrameSize    = pRUIBufferRecv->GetFrameSize();
				// [neuromos] pRUIBufferRecv->GetBuffer()�� ����� �� ����. RUIFrameBuffer�� GetBuffer() ���� ����.
//				RUIBuffer*	pRUIBufferCopy = m_pRUIBufferList264->AddBuffer(pRUIBufferRecv->GetBuffer(), pRUIBufferRecv->GetFrameSize());
				RUIBuffer*	pRUIBufferCopy = m_pRUIBufferList264->AddBuffer(dwFrameSize);

				if (pRUIBufferCopy != NULL)
					pRUIBufferRecv->CopyTo(pRUIBufferCopy->GetBuffer(), dwFrameSize);

				delete pRUIBufferRecv;
			}
			else
			{
//				Sleep(5); // [neuromos] ���۰� �غ� �ȵǾ����� 5ms���� ����.
//				DWORD	dwWaitStart = GetTickCount();
				::ResetEvent(m_pieParam->m_hEventRecv);
				::WaitForSingleObject(m_pieParam->m_hEventRecv, EVENT_RECV_TIMEOUT);
//				TRACE(_T("WaitForSingleObject Recv %d\n"), GetTickCount() - dwWaitStart);
			}
		}

		if (m_bStop) // [neuromos] �ܺο��� ������Ŵ
		{
			*nsize = 0; // ���� ���� ����.
			return UMC_ERR_END_OF_STREAM;
		}
	}

	if (how_far > 0) // ���� ���۸� �غ��ߴٸ� �����ϰ� �����.
	{
		CopyMemory((void*) data, (const void*) (pTarget + how_far), *nsize);
		delete [] pTarget;
	}

    return UMC_OK;
}

Status RUIBufferReader::MovePosition(Ipp64u npos)
{
	// ������� ��ġ �̵�.
	m_u64Position += npos;

	if (npos > 0)
	{
		if (! m_pRUIBufferList264->MoveTo(NULL, (DWORD) npos, RUIBUFFER_MOVE_FLAG_MULTI_BUFFER | RUIBUFFER_MOVE_FLAG_EXACT_SIZE)) // ����
		{
			// [neuromos] CacheData()���� ���� ���̶�� ������ �а� ���� ���̴�. ������ ���� ����.
			ASSERT(FALSE);
			return UMC_ERR_FAILED;
		}
	}

    return UMC_OK;
}

Ipp64u RUIBufferReader::GetPosition()
{
    return m_u64Position;
}

Status RUIBufferReader::SetPosition(Ipp64f position)
{
	// position�� 0.0 ~ 1.0���� �����ؼ� �̵���Ű�� �Լ�.
	// �� Class�� ��Ʈ���� ���� �𸣱� ������ �������� �ʴ´�.

	ASSERT(FALSE);

//	return UMC_OK;
	return UMC_ERR_FAILED;
}

Ipp64u RUIBufferReader::GetSize()
{
	// 0 ���� �õ��� ����. �Ʒ�ó�� �ڸ�Ʈ�Ǿ� ����
	// 0 means transcoded, network or DVD streams

//	return (Ipp64u) 0x7FFFFFFFFFFFFFFF;
	return (Ipp64u) 0;
}
