#ifndef _RUIBUFFERAUDIODECODER_H
#define _RUIBUFFERAUDIODECODER_H

#include "../Common/RUIDef.h"
#include "IPPAudioDecoder.h"
#include "../h264/common/vm_plus/umc_sys_info.h"
#include "RUIBufferList.h"
#include "RUIUDPFrameList.h"

using namespace UMC;

//#define WRITE_ENCODED_FILE

class RUIBufferAudioDecoder : public IPPAudioDecoder
{
public:
	RUIBufferAudioDecoder();
	~RUIBufferAudioDecoder();

private:
	BOOL					m_bStop;
	Ipp64f					tickDuration;
	DWORD					m_dwLastReadVideoSourceTick;
	RUIBufferList*			m_pRUIBufferListPCM;
	DWORD					m_dwDecodedSize; // 지금까지 인코딩된 결과의 크기 합

	// Socket
	BYTE					m_nFrameType;
	DWORD					m_dwFrameKey;
	RUIUDPFrameList*		m_pRUIBufferListRecv;

#ifdef WRITE_ENCODED_FILE
	BOOL					m_bFileSource;
	BOOL					m_bFileTarget;
	CFile					m_fileSource;
	CFile					m_fileTarget;
#endif

public:
	BOOL					GetStop()           { return m_bStop;         }
	void					SetStop(BOOL bStop) { m_bStop = bStop;        }
	DWORD					GetDecodedSize()    { return m_dwDecodedSize; }

	// Socket
	BYTE					GetFrameType()                                      { return m_nFrameType;                     }
	void					SetFrameType(BYTE nFrameType)                       { m_nFrameType = nFrameType;               }
	RUIUDPFrameList*		GetRUIBufferRecv()                                  { return m_pRUIBufferListRecv;             }
	void					SetRUIBufferRecv(RUIUDPFrameList* pRUIUDPFrameList) { m_pRUIBufferListRecv = pRUIUDPFrameList; }

	Ipp32s					Decode(RUIBufferList* pRUIBufferListPCM);

#ifdef WRITE_ENCODED_FILE
	BOOL					GetSourceFileEnable()           { return m_bFileSource;  }
	void					SetSourceFileEnable(BOOL bFile) { m_bFileSource = bFile; }
	BOOL					GetTargetFileEnable()           { return m_bFileTarget;  }
	void					SetTargetFileEnable(BOOL bFile) { m_bFileTarget = bFile; }
	CFile*					GetSourceFile()                 { return &m_fileSource; }
	CFile*					GetTargetFile()                 { return &m_fileTarget; }
#endif

public:
	virtual Status			PutOutputData(MediaData* out);
	virtual Ipp64f			GetTick();

	virtual int				ReadAudioSource (BYTE* pBuffer, DWORD dwOffset, int nItemSize, int nItemCount);
	virtual int				WriteAudioTarget(BYTE* pBuffer, int nItemSize, int nItemCount, FrameType frameType);
};

#endif
