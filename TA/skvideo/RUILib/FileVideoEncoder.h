#ifndef _FILEVIDEOENCODER_H
#define _FILEVIDEOENCODER_H

#include "IPPVideoEncoder.h"
#include "../h264/common/vm_plus/umc_sys_info.h"

using namespace UMC;

#define MAX_FILELEN 1024

class FileVideoEncoder : public IPPVideoEncoder
{
public:
	FileVideoEncoder();
	~FileVideoEncoder();

public:
	// Params
	Ipp32s				m_nWidth;     // Source Video Width
	Ipp32s				m_nHeight;    // Source Video Height
	Ipp64f				FrameRate;    // Framerate
	Ipp32s				maxNumFrames; // Max Frames to Encode
	Ipp32s				BitRate;      // Bitrate / Zero means Encoding without Rate Control
	Ipp32s				chroma_format;
	vm_char				SrcFileName[MAX_FILELEN];
	vm_char*			ParFileName;
	const vm_char*		DstFileName;
	Ipp32s				numThreads;

	// Info
	Ipp64f				tickDuration;
	Ipp32s*				FrameSize;
	Ipp32s				lastFrameNum;
	Ipp64f				lastPTS;

	vm_file*			m_fileSource;
	vm_file*			m_fileTarget;

public:
	Status				ChangeParams();
	Ipp32s				Encode(LPCTSTR szSourceFile, LPCTSTR szTargetFile, int nWidth, int nHeight);

public:
	virtual Status		PutOutputData(MediaData* out);
	virtual Ipp64f		GetTick();

	virtual int			ReadVideoSource (BYTE* pBuffer, DWORD dwOffset, int nItemSize, int nItemCount);
	virtual int			WriteVideoTarget(BYTE* pBuffer, int nItemSize, int nItemCount, FrameType frameType);
};

#endif
