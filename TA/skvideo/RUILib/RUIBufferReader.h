#ifndef _RUIBUFFERREADER_H
#define _RUIBUFFERREADER_H

#include <ippdefs.h>
#include "../h264/common/umc/umc_data_reader.h"
#include "RUIBufferList.h"
#include "RUIUDPFrameList.h"
#include "../h264/common/extension/ipp_extension.h"

using namespace UMC;

class RUIBufferReaderParams : public DataReaderParams
{
    DYNAMIC_CAST_DECL(RUIBufferReaderParams, DataReaderParams)

public:
    RUIBufferReaderParams()
    {
		m_pRUIBufferList264 = NULL;
	}

	RUIBufferList*	m_pRUIBufferList264;
};

class RUIBufferReader : public DataReader
{
    DYNAMIC_CAST_DECL(RUIBufferReader, DataReader)

public:
    RUIBufferReader();
    virtual ~RUIBufferReader();

public:
    Status				Init(DataReaderParams* InitParams);
    Status				Close();
    Status				Reset();
    Status				ReadData(void* data, Ipp32u* nsize);
    Status				MovePosition(Ipp64u npos);
    Status				CacheData(void* data, Ipp32u* nsize, Ipp32s how_far);
    Status				SetPosition(Ipp64f pos);
    Ipp64u				GetPosition();
    Ipp64u				GetSize();

private:
	IppExtensionParam*	m_pieParam;

	RUIBufferList*		m_pRUIBufferList264;
	Ipp64u				m_u64Position;
	BOOL				m_bStop;

	// Socket
	BOOL				m_bUseSocket;
	BYTE				m_nFrameType;
	RUIUDPFrameList*	m_pRUIBufferListRecv;

public:
	IppExtensionParam*	GetIppExtensionParam()                            { return m_pieParam;     }
	void				SetIppExtensionParam(IppExtensionParam* pieParam) { m_pieParam = pieParam; }

	BOOL				GetStop()           { return m_bStop;  }
	void				SetStop(BOOL bStop) { m_bStop = bStop; }

	void				CheckFrameRecvTick(DWORD dwFrameRecvTick);

	// Socket
	BOOL				GetUseSocket()                                          { return m_bUseSocket;                     }
	void				SetUseSocket(BOOL bUseSocket)                           { m_bUseSocket = bUseSocket;               }
	BYTE				GetFrameType()                                          { return m_nFrameType;                     }
	void				SetFrameType(BYTE nFrameType)                           { m_nFrameType = nFrameType;               }
	RUIUDPFrameList*	GetRUIBufferListRecv()                                  { return m_pRUIBufferListRecv;             }
	void				SetRUIBufferListRecv(RUIUDPFrameList* pRUIUDPFrameList) { m_pRUIBufferListRecv = pRUIUDPFrameList; }
};

#endif
