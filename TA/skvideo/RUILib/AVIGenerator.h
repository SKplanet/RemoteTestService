#ifndef _AVIGENERATOR_H
#define _AVIGENERATOR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable:4996)

#include <comdef.h>
#include <memory.h>
#include <tchar.h>
#include <string.h>
#include <vfw.h>

#pragma comment(lib, "vfw32.lib")

// Step 1 : Declare an CAVIGenerator object
// Step 2 : Set Bitmap by calling SetBitmapHeader functions + other parameters
// Step 3 : Initialize engine by calling InitEngine
// Step 4 : Send each frames to engine with function AddFrame
// Step 5 : Close engine by calling ReleaseEngine

class CAVIGenerator
{
public:
	CAVIGenerator();
	CAVIGenerator(LPCTSTR szFilePath, int nWidth, int nHeight, int nBitsPerPixel);
	~CAVIGenerator();

private:
	TCHAR				m_szFilePath[_MAX_PATH];
	DWORD				m_dwRate;	
	BITMAPINFOHEADER	m_bih;	
	long				m_lFrame;
	PAVIFILE			m_pAVIFile;
	PAVISTREAM			m_pStream;		
	PAVISTREAM			m_pStreamCompressed; 

	int					m_nWidth;
	int					m_nHeight;
	int					m_nBitsPerPixel;

public:
	HRESULT				InitEngine();
	void				InitBitmapHeader(int nWidth, int nHeight, int nBitsPerPixel);
	HRESULT				AddFrame(BYTE* bmBits);
	void				ReleaseEngine();
};

#endif
