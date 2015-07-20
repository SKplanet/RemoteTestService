#ifndef _RUIAVIWRITETHREAD_H
#define _RUIAVIWRITETHREAD_H

#include "RUIThread.h"
#include "AVIGenerator.h"
#include "RUIBufferList.h"

class RUIAviWriteThread : public RUIThread
{
public:
	RUIAviWriteThread();
	virtual ~RUIAviWriteThread();

private:
	int					m_nWidth;
	int					m_nHeight;
	int					m_nBitsPerPixel;
	int					m_nPitch;
	int					m_nImageSize;
	CAVIGenerator*		m_pAvi;
	BYTE*				m_pFlipBuffer;
	RUIBufferList*		m_pRUIBufferListAVI;

protected:
	void				ResetImageSize();
	void				NewFlipBuffer();
	void				DeleteFlipBuffer();

public:
	RUIBufferList*		GetRUIBufferListAVI()                                 { return m_pRUIBufferListAVI;              }
	void				SetRUIBufferListAVI(RUIBufferList* pRUIBufferListAVI) { m_pRUIBufferListAVI = pRUIBufferListAVI; }

	void				FlipImage(BYTE* pSource);

	BOOL				NewAviFile(LPCTSTR szFilePath, int nWidth, int nHeight, int nBitsPerPixel);
	BOOL				DeleteAviFile();
	BOOL				AddFrame(BYTE* pFrame);

public:
	virtual BOOL		OnStart();
	virtual void		OnStop();
	virtual BOOL		ThreadProc();
};

#endif
