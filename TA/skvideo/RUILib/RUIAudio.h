/*
	* The MIT License (MIT)
	* Copyright (c) 2015 SK PLANET. All Rights Reserved.
	*
	* Permission is hereby granted, free of charge, to any person obtaining a copy
	* of this software and associated documentation files (the "Software"), to deal
	* in the Software without restriction, including without limitation the rights
	* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	* copies of the Software, and to permit persons to whom the Software is
	* furnished to do so, subject to the following conditions:
	*
	* The above copyright notice and this permission notice shall be included in
	* all copies or substantial portions of the Software.
	*
	* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	* THE SOFTWARE.
*/

#ifndef _AUDIO_H
#define _AUDIO_H

#define _USE_AAC_CODEC

#include "RUIThread.h"
#include "../h264/common/extension/ipp_extension.h"
#include <mmsystem.h>

#include "../RUILib/RUIUseBufferList.h"
#include "../RUILib/RUIUDPFrameList.h"
#include "../RUILib/RUICsLocker.h"

#define RUI_BUFFER_COUNT						100
#ifdef _USE_AAC_CODEC
#define WAVE_FRAME_SIZE							2048
#else
#define WAVE_FRAME_SIZE							1024
#endif

#define ERR_AUDIO_NOERROR						0x00000000
#define ERR_AUDIO_RECORD_DOESNTOPENED			0x00000001
#define ERR_AUDIO_RECORD_ALREADYOPENED			0x00000002
#define ERR_AUDIO_RECORD_CANTOPEN				0x00000003
#define ERR_AUDIO_RECORD_CANTADD				0x00000004
#define ERR_AUDIO_PLAY_ALREADYOPENED			0x00000005
#define ERR_AUDIO_PLAY_DOESNTOPENED				0x00000006
#define ERR_AUDIO_PLAY_CANTOPEN					0x00000007
#define ERR_AUDIO_PLAY_CANTADD					0x00000008
#define ERR_AUDIO_RECORDING_ALREADYSTARTED		0x00000009
#define ERR_AUDIO_RECORDING_DOESNTSTARTED		0x0000000A
#define ERR_AUDIO_PLAYING_ALREADYSTARTED		0x0000000B
#define ERR_AUDIO_PLAYING_DOESNTSTARTED			0x0000000C
#define ERR_AUDIO_QUEUE_OVERFLOW				0x0000000D
#define ERR_AUDIO_QUEUE_UNDERFLOW				0x0000000E
#define ERR_AUDIO_FILE_CANTCREATE				0x0000000F
#define ERR_AUDIO_FILE_CANTOPEN					0x00000010
#define ERR_AUDIO_FILE_CANTREAD					0x00000011
#define ERR_AUDIO_FAILMEMORY					0x00000012
#define ERR_AUDIO_THREAD_DOESNTEXIST			0x00000013

typedef int (*ENUM_WAVEIN_DEVICE_CALLBACK) (UINT nDeviceID, WAVEINCAPS* pwic, void* pParam);

void CALLBACK	WaveInProc (HWAVEIN  hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
void CALLBACK	WaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

class RUIAudio : public RUIThread
{
public:
	RUIAudio();
	virtual ~RUIAudio();

private:
	IppExtensionParam*		m_pieParam;

#ifdef _USE_AAC_CODEC
#else
	int						m_nCodec;
#endif

	WAVEFORMATEX			m_WFEX;

	// Record
	UINT					m_nWaveInDeviceID;
	HWAVEIN					m_hWaveIn;
	RUIUseBufferList		m_RecBufferList;
	BYTE					m_nFrameType;
	DWORD					m_dwFrameKey;
	BYTE					m_pFrameRecEnc[WAVE_FRAME_SIZE];

	// Play
	HWAVEOUT				m_hWaveOut;
	BOOL					m_bPlayOpen;
	BOOL					m_bPlaySync;
	int						m_nWaitSyncCount;
	RUIBufferList			m_WaitBufferList;
	RUIUseBufferList		m_PlayBufferList;
	BYTE					m_pFramePlayEnc[WAVE_FRAME_SIZE];

	RUICriticalSection		m_playCs;
	int						m_nPlayCount;

#ifdef _USE_AAC_CODEC
	RUIBufferList*			m_pRUIBufferListPCM;
#else
	RUIUDPFrameList*		m_pRUIBufferListSend;
	RUIUDPFrameList*		m_pRUIBufferListRecv;
#endif

public:
	IppExtensionParam*		GetIppExtensionParam()                            { return m_pieParam;     }
	void					SetIppExtensionParam(IppExtensionParam* pieParam) { m_pieParam = pieParam; }

#ifdef _USE_AAC_CODEC
#else
	int						GetCodec()                  { return m_nCodec;          }
	void					SetCodec(int nCodec)        { m_nCodec = nCodec;        }
#endif

	UINT					GetWaveInDeviceID()         { return m_nWaveInDeviceID; }
	void					SetWaveInDeviceID(UINT nID) { m_nWaveInDeviceID = nID;  }

	// Record
	UINT					EnumWaveInDevice(ENUM_WAVEIN_DEVICE_CALLBACK pfnCallback, LPVOID pParam);
	MMRESULT				RecordOpen();
	void					RecordClose();
	BOOL					RecAddBuffer   (RUIBuffer* pRUIBuffer);
	BOOL					RecRemoveBuffer(RUIBuffer* pRUIBuffer);
	void					On_WIM_DATA(WAVEHDR* pWaveHdr);

	// Play
	DWORD					PlayOpen();
	void					PlayClose();
	BOOL					PlayAddBuffer   (RUIBuffer* pRUIBuffer);
	BOOL					PlayRemoveBuffer(RUIBuffer* pRUIBuffer);
	BOOL					RecvListToPlayList();
	void					PlayListToPlay();
	void					On_WOM_DONE(WAVEHDR* pWaveHdr);

	void					ResetPlayCount();
	int						IncPlayCount();
	int						DecPlayCount();
	int						GetPlayCount();

	BYTE					GetFrameType()                                        { return m_nFrameType;                     }
	void					SetFrameType(BYTE nFrameType)                         { m_nFrameType = nFrameType;               }

#ifdef _USE_AAC_CODEC
	RUIBufferList*			GetRUIBufferListPCM()                                 { return m_pRUIBufferListPCM;              }
	void					SetRUIBufferListPCM(RUIBufferList* pRUIBufferListPCM) { m_pRUIBufferListPCM = pRUIBufferListPCM; }
#else
	RUIBufferList*			GetRUIBufferSend()                                    { return m_pRUIBufferListSend;             }
	void					SetRUIBufferSend(RUIUDPFrameList* pRUIUDPFrameList)   { m_pRUIBufferListSend = pRUIUDPFrameList; }

	RUIBufferList*			GetRUIBufferRecv()                                    { return m_pRUIBufferListRecv;             }
	void					SetRUIBufferRecv(RUIUDPFrameList* pRUIUDPFrameList)   { m_pRUIBufferListRecv = pRUIUDPFrameList; }
#endif

public:
	virtual BOOL			OnStart();
	virtual void			OnStop();
	virtual BOOL			ThreadProc();
};

#endif
