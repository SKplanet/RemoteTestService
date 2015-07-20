#ifndef _SIMPLEPLAYERTHREAD_H
#define _SIMPLEPLAYERTHREAD_H

#include "RUIThread.h"
#include "RUIBufferList.h"
#include "RUIBufferReader.h"
#include "../h264/decoder/umc_pipeline/avsync.h"

class SimplePlayerThread : public RUIThread
{
public:
	SimplePlayerThread();
	virtual ~SimplePlayerThread();

private:
	HWND					m_hWndOwner;
	int						m_nDisplayLeft;
	int						m_nDisplayTop;
	int						m_nDisplayWidth;
	int						m_nDisplayHeight;

	IppExtensionParam*		m_pieParam;
	RENDER_FRAME_CALLBACK	m_pfnRenderFrameBeforeCallback;
	LPVOID					m_pfnRenderFrameBeforeCallbackParam;
	RENDER_FRAME_CALLBACK	m_pfnRenderFrameAfterCallback;
	LPVOID					m_pfnRenderFrameAfterCallbackParam;

	UINT					m_nFramesDecoded;
	UINT					m_nFramesRendered;

	RUIBufferList*			m_pRUIBufferList264;

	RUIBufferReader			m_ruiBufferReader;
	AVSync::ExternalInfo	m_externalInfo;

	BOOL					m_bStoped;

private:
	UMC::Status				PlaySingleFile  (AVSync& rAVSync, AVSync::CommonCtl& r_cc, vm_char* szFileName);
	UMC::Status				PlaySingleBuffer(AVSync& rAVSync, AVSync::CommonCtl& r_cc);

public:
	HWND					GetOwnerHWND()                       { return m_hWndOwner;                         }
	void					SetOwnerHWND(HWND hWndOwner)         { m_hWndOwner = hWndOwner;                    }
	int						GetDisplayLeft()                     { return m_nDisplayLeft;                      }
	void					SetDisplayLeft(int nDisplayLeft)     { m_nDisplayLeft = nDisplayLeft;              }
	int						GetDisplayTop()                      { return m_nDisplayTop;                       }
	void					SetDisplayTop(int nDisplayTop)       { m_nDisplayTop = nDisplayTop;                }
	int						GetDisplayWidth()                    { return m_nDisplayWidth;                     }
	void					SetDisplayWidth(int nDisplayWidth)   { m_nDisplayWidth = nDisplayWidth;            }
	int						GetDisplayHeight()                   { return m_nDisplayHeight;                    }
	void					SetDisplayHeight(int nDisplayHeight) { m_nDisplayHeight = nDisplayHeight;          }

	IppExtensionParam*		GetIppExtensionParam()               { return m_pieParam;                          }
	void					SetIppExtensionParam(IppExtensionParam* pieParam) { m_pieParam = pieParam; m_ruiBufferReader.SetIppExtensionParam(pieParam); }

	UINT					GetFramesDecoded()                   { return m_nFramesDecoded;                    } 
	UINT					GetFramesRendered()                  { return m_nFramesRendered;                   } 

	BYTE					GetFrameType()                       { return m_ruiBufferReader.GetFrameType();    }
	void					SetFrameType(BYTE nFrameType)        { m_ruiBufferReader.SetFrameType(nFrameType); }
	void					SetRUIBufferList(RUIBufferList* pRUIBufferList264) { m_pRUIBufferList264 = pRUIBufferList264; }

	BOOL					Play();

	// Socket
	BOOL					GetUseSocket()                                          { return m_ruiBufferReader.GetUseSocket();                  }
	void					SetUseSocket(BOOL bUseSocket)                           { m_ruiBufferReader.SetUseSocket(bUseSocket);               }
	RUIUDPFrameList*		GetRUIBufferListRecv()                                  { return m_ruiBufferReader.GetRUIBufferListRecv();          }
	void					SetRUIBufferListRecv(RUIUDPFrameList* pRUIUDPFrameList) { m_ruiBufferReader.SetRUIBufferListRecv(pRUIUDPFrameList); }

public:
	virtual BOOL			Stop(BOOL bSync = TRUE);
	virtual BOOL			OnStart();
	virtual void			OnStop();
	virtual BOOL			ThreadProc();
};

#endif
