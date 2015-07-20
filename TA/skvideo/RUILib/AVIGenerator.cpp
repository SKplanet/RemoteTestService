#include "stdafx.h"
#include "AVIGenerator.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAVIGenerator::CAVIGenerator() :
	m_dwRate(15),
	m_pAVIFile(NULL),
	m_pStream(NULL),
	m_pStreamCompressed(NULL)
{
	memset(&m_bih, 0, sizeof(BITMAPINFOHEADER));
}

CAVIGenerator::CAVIGenerator(LPCTSTR szFilePath, int nWidth, int nHeight, int nBitsPerPixel) :
	m_dwRate(15),
	m_pAVIFile(NULL),
	m_pStream(NULL),
	m_pStreamCompressed(NULL)
{
	_tcscpy(m_szFilePath, szFilePath);
	if (_tcsstr(m_szFilePath, _T("avi")) == NULL)
		_tcscat(m_szFilePath, _T(".avi"));

	m_nWidth        = nWidth;
	m_nHeight       = nHeight;
	m_nBitsPerPixel = nBitsPerPixel;

	InitBitmapHeader(nWidth, nHeight, nBitsPerPixel);
}

CAVIGenerator::~CAVIGenerator()
{
}

void CAVIGenerator::InitBitmapHeader(int nWidth, int nHeight, int nBitsPerPixel)
{
	ZeroMemory(&m_bih, sizeof(BITMAPINFOHEADER));

	m_bih.biSize          = sizeof(BITMAPINFOHEADER);
	m_bih.biWidth         = nWidth;
	m_bih.biHeight        = nHeight;
	m_bih.biPlanes        = 1;
	m_bih.biBitCount      = nBitsPerPixel;
	m_bih.biCompression   = BI_RGB;
	m_bih.biSizeImage     = m_bih.biWidth * m_bih.biHeight * m_bih.biBitCount / 8;
	m_bih.biXPelsPerMeter = 0;
	m_bih.biYPelsPerMeter = 0;
	m_bih.biClrUsed       = 0;
	m_bih.biClrImportant  = 0;
}

HRESULT CAVIGenerator::InitEngine()
{
	HRESULT					hr;
	AVISTREAMINFO			strHdr; // information for a single stream
	AVICOMPRESSOPTIONS		opts;
	AVICOMPRESSOPTIONS FAR*	aopts[1] = { &opts };

	// Step 0 : Let's make sure we are running on 1.1
	DWORD	wVer = HIWORD(VideoForWindowsVersion());
	if (wVer < 0x010a)
	{
		 // oops, we are too old, blow out of here 
//		_tcscpy(m_szError, _T("Version of Video for Windows too old. Come on, join the 21th century!"));
		return S_FALSE;
	}

	// Step 1 : initialize AVI engine
	AVIFileInit();

	// Step 2 : Open the movie file for writing....
	hr = AVIFileOpen(&m_pAVIFile,    // Address to contain the new file interface pointer
		       (LPCTSTR) m_szFilePath,    // Null-terminated string containing the name of the file to open
		       OF_WRITE | OF_CREATE, // Access mode to use when opening the file. 
		       NULL);                // use handler determined from file extension.
                                     // Name your file .avi -> very important

	if (hr != AVIERR_OK)
	{
//		_tprintf(szBuffer, _T("AVI Engine failed to initialize. Check filename %s."), m_szFilePath);
//		_tcscpy(m_szError, szBuffer);

		// Check it succeded.
		switch (hr)
		{
			case AVIERR_BADFORMAT :
//				_tcscat(m_szError, _T("The file couldn't be read, indicating a corrupt file or an unrecognized format."));
				break;
			case AVIERR_MEMORY :
//				_tcscat(m_szError, _T("The file could not be opened because of insufficient memory."));
				break;
			case AVIERR_FILEREAD :
//				_tcscat(m_szError, _T("A disk error occurred while reading the file."));
				break;
			case AVIERR_FILEOPEN :
//				_tcscat(m_szError, _T("A disk error occurred while opening the file."));
				break;
			case REGDB_E_CLASSNOTREG :
//				_tcscat(m_szError, _T("According to the registry, the type of file specified in AVIFileOpen does not have a handler to process it"));
				break;
		}

		return hr;
	}

	// Fill in the header for the video stream....
	memset(&strHdr, 0, sizeof(strHdr));
	strHdr.fccType               = streamtypeVIDEO;   // video stream type
	strHdr.fccHandler            = 0;
	strHdr.dwScale               = 1;                 // should be one for video
	strHdr.dwRate                = m_dwRate;          // fps
	strHdr.dwSuggestedBufferSize = m_bih.biSizeImage; // Recommended buffer size, in bytes, for the stream.
	SetRect(&strHdr.rcFrame, 0, 0,                    // rectangle for stream
	    (int) m_bih.biWidth,
	    (int) m_bih.biHeight);

	// Step 3 : Create the stream;
	hr = AVIFileCreateStream(m_pAVIFile, // file pointer
			         &m_pStream,         // returned stream pointer
			         &strHdr);           // stream header

	// Check it succeded.
	if (hr != AVIERR_OK)
	{
//		_tcscpy(m_szError, _T("AVI Stream creation failed. Check Bitmap info."));

//		if (hr == AVIERR_READONLY)
//			_tcscat(m_szError, _T(" Read only file."));

		return hr;
	}

	// Step 4: Get codec and infos about codec
	memset(&opts, 0, sizeof(opts));

	// Poping codec dialog
	//bool	bPopupCodec = false;
	bool	bPopupCodec = true;

	if (bPopupCodec)
	{
		if (! AVISaveOptions(NULL, 0, 1, &m_pStream, (LPAVICOMPRESSOPTIONS FAR*) &aopts))
		{
			AVISaveOptionsFree(1, (LPAVICOMPRESSOPTIONS FAR*) &aopts);
			return S_FALSE;
		}
	}
	else
	{
		opts.fccType           = 0;
		opts.fccHandler        = mmioFOURCC('m','s','v','c');
		opts.dwKeyFrameEvery   = 0;
		opts.dwQuality         = 7500;
		opts.dwBytesPerSecond  = 0;
		opts.lpFormat          = 0;
		opts.cbFormat          = 0;
		opts.cbParms           = 4;
		opts.lpParms           = malloc(opts.cbParms);
		((BYTE*) opts.lpParms)[0] = 0x4B; 
		((BYTE*) opts.lpParms)[1] = 0x00; 
		((BYTE*) opts.lpParms)[2] = 0x00; 
		((BYTE*) opts.lpParms)[3] = 0x00; 
		opts.dwInterleaveEvery = 0;
		opts.dwFlags           = AVICOMPRESSF_VALID;
	}

	// Step 5:  Create a compressed stream using codec options.
	hr = AVIMakeCompressedStream(&m_pStreamCompressed, 
			m_pStream, 
			&opts, 
			NULL);

	if (hr != AVIERR_OK)
	{
//		_tcscpy(m_szError, _T("AVI Compressed Stream creation failed."));
		
		switch (hr)
		{
			case AVIERR_NOCOMPRESSOR :
//				_tcscat(m_szError, _T("A suitable compressor cannot be found."));
				break;
			case AVIERR_MEMORY :
//				_tcscat(m_szError, _T("There is not enough memory to complete the operation."));
				break; 
			case AVIERR_UNSUPPORTED :
//				_tcscat(m_szError, _T("Compression is not supported for this type of data. This error might be returned if you try to compress data that is not audio or video."));
				break;
		}

		if (bPopupCodec) AVISaveOptionsFree(1, (LPAVICOMPRESSOPTIONS FAR*) &aopts); else free(opts.lpParms);
		return hr;
	}

	// Step 6 : sets the format of a stream at the specified position
	hr = AVIStreamSetFormat(m_pStreamCompressed, 
					0,             // position
					&m_bih,        // stream format
					m_bih.biSize + // format size
					m_bih.biClrUsed * sizeof(RGBQUAD));

	if (hr != AVIERR_OK)
	{
//		_tcscpy(m_szError, _T("AVI Compressed Stream format setting failed."));

		if (bPopupCodec) AVISaveOptionsFree(1, (LPAVICOMPRESSOPTIONS FAR*) &aopts); else free(opts.lpParms);
		return hr;
	}

	// Step 6 : Initialize step counter
	m_lFrame = 0;

	if (bPopupCodec) AVISaveOptionsFree(1, (LPAVICOMPRESSOPTIONS FAR*) &aopts); else free(opts.lpParms);
	return hr;
}

void CAVIGenerator::ReleaseEngine()
{
	if (m_pStream)
	{
		AVIStreamRelease(m_pStream);
		m_pStream = NULL;
	}

	if (m_pStreamCompressed)
	{
		AVIStreamRelease(m_pStreamCompressed);
		m_pStreamCompressed = NULL;
	}

	if (m_pAVIFile)
	{
		AVIFileRelease(m_pAVIFile);
		m_pAVIFile = NULL;
	}

	// Close engine
	AVIFileExit();
}

HRESULT CAVIGenerator::AddFrame(BYTE* bmBits)
{
	HRESULT	hr;

	// compress bitmap
	hr = AVIStreamWrite(m_pStreamCompressed, // stream pointer
		m_lFrame,                            // time of this frame
		1,                                   // number to write
		bmBits,                              // image buffer
		m_bih.biSizeImage,                   // size of this frame
		AVIIF_KEYFRAME,                      // flags....
		NULL,
		NULL);

	// updating frame counter
	m_lFrame++;

	return hr;
}
