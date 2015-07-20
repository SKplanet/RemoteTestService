#include "stdafx.h"
#include "RUIBufferReader.h"
#include "../Common/RUIDef.h"

// [neuromos] 읽기 원하는 크기가 1024(또는 16384)이지만 현재 버퍼에 있는 크기만큼만 읽고 파라메터를 수정한다.
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

	// 초기화
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
	// 마지막이면 UMC_ERR_END_OF_STREAM를 리턴한다. 이 Class에서는 Stop일 경우에만 UMC_ERR_END_OF_STREAM.
	if (m_bStop) // [neuromos] 외부에서 중지시킴
	{
		*nsize = 0; // 읽은 것이 없다.
		return UMC_ERR_END_OF_STREAM;
	}

// [neuromos] *원하는 크기가 쌓일 때까지 대기하므로 끊김 현상이 생길 수 있다.
// [neuromos] 읽기 원하는 크기가 1024(또는 16384)이지만 현재 버퍼에 있는 크기만큼만 읽고 파라메터를 수정한다.
#ifdef _USE_ASSIGNED_BUFFER_LIST_SIZE
	DWORD	dwBufferSize = m_pRUIBufferList264->GetBufferSize();

	if (0 < dwBufferSize && dwBufferSize < *nsize)
		*nsize = (Ipp32u) dwBufferSize;
#endif

    Status	umcRes = CacheData(data, nsize, 0);

    MovePosition(*nsize); // Read를 하면 자동으로 pointer(File Offset)이 증가된다.

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

	// [neuromos] Video가 크기가 커서 나중에 보내지기 때문에 Video Tick이 더 크다.
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
	// how_far는 Offset(건너뛰기)

	BYTE*	pTarget    = (BYTE*) data;
	DWORD	dwReadSize = (DWORD) (*nsize + how_far);

	if (how_far > 0) // 건너뛰기가 지정되면 따로 버퍼를 준비한다. GetAssignedBufferData2 내에서 건너뛰기 구현이 어렵다.
	{
		pTarget = new BYTE[*nsize + how_far];
	}

	while (! m_pRUIBufferList264->CopyTo(pTarget, dwReadSize, RUIBUFFER_MOVE_FLAG_MULTI_BUFFER | RUIBUFFER_MOVE_FLAG_EXACT_SIZE))
	{
		// 소켓을 사용하는 경우 인코더가 m_pRUIBufferList264를 채우지 않는다.
		// 주기적으로 m_pRUIBufferListRecv에 추가된 FrameBuffer를 확인하여 m_pRUIBufferList264로 옮긴다.
		if (m_bUseSocket && m_pRUIBufferListRecv != NULL)
		{
			// [neuromos] !이곳에서 m_dwFrameRecvTickVideoLimit를 갱신한다.
			DWORD	dwCurrTick = GetTickCount();

			m_pieParam->m_dwFrameRecvTickVideoLimit = 
				(
					(m_pieParam->m_nRelMyTickSign > 0) ?
					(dwCurrTick - m_pieParam->m_dwRelMyTick) :
					(dwCurrTick + m_pieParam->m_dwRelMyTick)
				)
				- m_pieParam->m_dwRenderingDelay;

//			RUIFrameBuffer*	pRUIBufferRecv = m_pRUIBufferListRecv->PopHeadCompleted(m_nFrameType); // m_pRUIBufferListRecv에 완성된 FrameBuffer가 있는가?
			RUIFrameBuffer*	pRUIBufferRecv = m_pRUIBufferListRecv->PopLimitHeadCompleted(m_nFrameType, m_pieParam->m_dwFrameRecvTickVideoLimit);

			if (pRUIBufferRecv != NULL)
			{
				// [neuromos] [checkpoint]
				DWORD	dwFrameYourTick = pRUIBufferRecv->GetParam();
				m_pieParam->m_dwVideoTickCurr = dwFrameYourTick;

				CheckFrameRecvTick(dwFrameYourTick);

				DWORD		dwFrameSize    = pRUIBufferRecv->GetFrameSize();
				// [neuromos] pRUIBufferRecv->GetBuffer()는 사용할 수 없음. RUIFrameBuffer는 GetBuffer() 지원 안함.
//				RUIBuffer*	pRUIBufferCopy = m_pRUIBufferList264->AddBuffer(pRUIBufferRecv->GetBuffer(), pRUIBufferRecv->GetFrameSize());
				RUIBuffer*	pRUIBufferCopy = m_pRUIBufferList264->AddBuffer(dwFrameSize);

				if (pRUIBufferCopy != NULL)
					pRUIBufferRecv->CopyTo(pRUIBufferCopy->GetBuffer(), dwFrameSize);

				delete pRUIBufferRecv;
			}
			else
			{
//				Sleep(5); // [neuromos] 버퍼가 준비가 안되었으니 5ms동안 쉰다.
//				DWORD	dwWaitStart = GetTickCount();
				::ResetEvent(m_pieParam->m_hEventRecv);
				::WaitForSingleObject(m_pieParam->m_hEventRecv, EVENT_RECV_TIMEOUT);
//				TRACE(_T("WaitForSingleObject Recv %d\n"), GetTickCount() - dwWaitStart);
			}
		}

		if (m_bStop) // [neuromos] 외부에서 중지시킴
		{
			*nsize = 0; // 읽은 것이 없다.
			return UMC_ERR_END_OF_STREAM;
		}
	}

	if (how_far > 0) // 따로 버퍼를 준비했다면 복사하고 지운다.
	{
		CopyMemory((void*) data, (const void*) (pTarget + how_far), *nsize);
		delete [] pTarget;
	}

    return UMC_OK;
}

Status RUIBufferReader::MovePosition(Ipp64u npos)
{
	// 상대적인 위치 이동.
	m_u64Position += npos;

	if (npos > 0)
	{
		if (! m_pRUIBufferList264->MoveTo(NULL, (DWORD) npos, RUIBUFFER_MOVE_FLAG_MULTI_BUFFER | RUIBUFFER_MOVE_FLAG_EXACT_SIZE)) // 삭제
		{
			// [neuromos] CacheData()에서 들어온 것이라면 무조건 읽고 들어온 것이다. 실패할 리가 없다.
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
	// position을 0.0 ~ 1.0까지 지정해서 이동시키는 함수.
	// 이 Class는 스트림의 끝을 모르기 때문에 지원하지 않는다.

	ASSERT(FALSE);

//	return UMC_OK;
	return UMC_ERR_FAILED;
}

Ipp64u RUIBufferReader::GetSize()
{
	// 0 셋팅 시도해 본다. 아래처럼 코멘트되어 있음
	// 0 means transcoded, network or DVD streams

//	return (Ipp64u) 0x7FFFFFFFFFFFFFFF;
	return (Ipp64u) 0;
}
