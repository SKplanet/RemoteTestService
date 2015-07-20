#include "stdafx.h"
#include "FileVideoEncoder.h"

#include "../h264/common/vm/vm_time.h"
#include "../h264/common/vm/vm_strings.h"
#include "../h264/common/vm/vm_sys_info.h"

#include <ippvc.h>

FileVideoEncoder::FileVideoEncoder()
{
	m_nWidth         = 0;
	m_nHeight        = 0;
	maxNumFrames     = 0;
	BitRate          = -1; // (BitRate < 0) means constant quality!
	FrameRate        = 0;
	chroma_format    = 1; // 4:2:0
	SrcFileName[0]   = 0;
	ParFileName      = NULL;
	DstFileName      = NULL;
	numThreads       = 1;

	tickDuration     = 1.0 / (Ipp64f) vm_time_get_frequency();
	FrameSize        = NULL;
	lastFrameNum     = 0;
	lastPTS          = -1.0;
}

FileVideoEncoder::~FileVideoEncoder()
{
	if (pCodec)
	{
		delete pCodec;
		pCodec = NULL;
	}
	if (pEncoderParams)
	{
		delete pEncoderParams;
		pEncoderParams = NULL;
	}
	if (FrameSize)
	{
		delete [] FrameSize;
		FrameSize = NULL;
	}
}

Status FileVideoEncoder::ChangeParams() // check set bitrate/set frame rate
{
	Status				st;
	VideoEncoderParams	newpar;

	st = pCodec->GetInfo(&newpar);
	vm_string_printf(VM_STRING("Q=%d%% "), newpar.qualityMeasure);

	if(st != UMC_OK)
		return st;

	if (mFramesEncoded % 9 == 1)
	{
//		newpar.info.framerate *= 0.9; // random
		newpar.info.bitrate = newpar.info.bitrate * 3 / 4; // random
		if (newpar.qualityMeasure < 40)
			newpar.info.bitrate *= 5; // random

		vm_string_printf(VM_STRING("SetParams Called for bitrate = %d\n"),newpar.info.bitrate);
		st = pCodec->SetParams(&newpar);
//		if (st != UMC_OK) // not supported everywhere
//			return st;
	}
	if (mFramesEncoded % 3 == 1)
	{
		newpar.numThreads = (newpar.numThreads & 3) + 1;
		vm_string_printf(VM_STRING("SetParams Called for threads = %d\n"),newpar.numThreads);
		st = pCodec->SetParams(&newpar);
	}

	if (mFramesEncoded % 25 == 1)
		pCodec->Reset();

	// close caption
	{
		MediaData	cc;
		vm_char		text[30];
		vm_string_sprintf(text, VM_STRING("CCaption %d"), mFramesEncoded);
		cc.SetBufferPointer((Ipp8u*)text, sizeof(text));
		cc.SetDataSize(vm_string_strlen(text)+1);
		pCodec->GetFrame(&cc, 0);
	}

	return UMC_OK;
}

int read_line(vm_char *file_name, int line_num, vm_char *line, int /*max_len*/)
{
	vm_file*	f;
	int			i;

	if (NULL == (f = vm_file_open(file_name, VM_STRING("rt"))))
	{
		vm_file_fprintf(vm_stderr, __VM_STRING("Couldn't open file %s\n"), file_name);
		return 1;
	}

	for (i = 0; i < line_num; i++)
	{
		if (NULL == vm_file_fgets(line, MAX_FILELEN-1, f))
		{
			vm_file_fclose(f);
			return 1;
		}
	}
	vm_file_fclose(f);
	return 0;
}

BOOL FileVideoEncoder::Encode(LPCTSTR szSourceFile, LPCTSTR szTargetFile, int nWidth, int nHeight)
{
    vm_string_strcpy(SrcFileName, szSourceFile);
    DstFileName = szTargetFile;
	m_nWidth    = nWidth;
	m_nHeight   = nHeight;

//	Ipp64f	size_ratio = 0;
	vm_char	line[MAX_FILELEN] = {0};

	if (ippStaticInit() < ippStsNoErr)
	{
		vm_file_fprintf(vm_stderr, VM_STRING("Error: Can't initialize ipp libs!\n"));
		return 5;
	}

	pCodec = new H264VideoEncoder();
	if (ParFileName)
		pEncoderParams = new H264EncoderParams;

	if (ParFileName)
	{
		Status res;
		res = pEncoderParams->ReadParamFile(ParFileName);
		if (res < UMC_OK) // < means error
		{
			if (res == UMC_ERR_OPEN_FAILED)
				vm_file_fprintf(vm_stderr, VM_STRING("Error: Can't open par file %s\n"), ParFileName);
			else
				vm_file_fprintf(vm_stderr, VM_STRING("Error: Can't parse par file %s\n"), ParFileName);
			return 3;
		}

		H264EncoderParams*	params = (H264EncoderParams*)pEncoderParams;
		chroma_format              = params->chroma_format_idc;
		mBitDepth                  = (IPP_MAX(params->bit_depth_luma, params->bit_depth_chroma) > 8)? 16 : 8;
		mBitDepthAlpha             = (params->bit_depth_aux > 8)? 16: 8;
		numFramesToEncode          = params->numFramesToEncode;
		mColorFormat               =  (chroma_format == 0) ? GRAY
									: (chroma_format == 2) ? YUV422
									: (chroma_format == 3) ? YUV444
									: YUV420; // Accept monochromes with UV components filled with dummy values.
	}

	if ((! SrcFileName[0] || SrcFileName[vm_string_strlen(SrcFileName) - 1] == '\\') && ParFileName)
	{
		// read stream filename from second line of ParFile
		vm_char SrcName[MAX_FILELEN];

		if (read_line(ParFileName, 2, line, MAX_FILELEN))
			return 3;
		vm_string_sscanf(line, VM_STRING("%s"), SrcName);
		vm_string_strcat(SrcFileName, SrcName); // concatenate
	}

	if (!ParFileName)
	{
		pEncoderParams = new VideoEncoderParams;
		if (! maxNumFrames) maxNumFrames = 0x7fffffff;
		if (! FrameRate   ) FrameRate    = 30;
		if (BitRate < 0   ) BitRate      = 5000000;
	}

	// override param's if non-zero
	if (m_nWidth && m_nHeight)
	{
		pEncoderParams->info.clip_info.width  = m_nWidth;
		pEncoderParams->info.clip_info.height = m_nHeight;
	}
	else
	{
		m_nWidth  = pEncoderParams->info.clip_info.width;
		m_nHeight = pEncoderParams->info.clip_info.height;
	}

	if (maxNumFrames) numFramesToEncode              = maxNumFrames; else maxNumFrames = numFramesToEncode;
	if (FrameRate   ) pEncoderParams->info.framerate = FrameRate;    else FrameRate    = pEncoderParams->info.framerate;
	if (BitRate >= 0) pEncoderParams->info.bitrate   = BitRate;      else BitRate      = pEncoderParams->info.bitrate;
	pEncoderParams->numThreads = numThreads;

	m_fileSource = vm_file_open(SrcFileName, VM_STRING("rb"));

	if (m_fileSource != NULL)
	{
		m_fileTarget = vm_file_open(DstFileName, VM_STRING("wb"));

		if (m_fileTarget != NULL)
		{
			if (pCodec->Init   (pEncoderParams) == UMC_OK &&
				pCodec->GetInfo(pEncoderParams) == UMC_OK)
			{
				Run();
			}

			vm_file_fclose(m_fileTarget);
			m_fileTarget = NULL;
		}

		vm_file_fclose(m_fileSource);
		m_fileSource = NULL;
	}

//	if (pEncoderParams->info.framerate > 0 && pEncoderParams->info.bitrate > 0)
//		size_ratio = encodedSize / (((Ipp64f) mFramesEncoded / pEncoderParams->info.framerate) * (pEncoderParams->info.bitrate / 8));

	return 0;
}

Status FileVideoEncoder::PutOutputData(MediaData* out)
{
	if (! out) // at EOF
		return UMC_ERR_NULL_PTR;

	Ipp32s	len = (Ipp32s) out->GetDataSize();

	Ipp64f	PTS = out->GetTime();
	if (! FrameSize && numFramesToEncode < (1 << 16))
		FrameSize = new Ipp32s[numFramesToEncode];

	if (FrameSize && len && lastFrameNum < numFramesToEncode)
	{
		if (lastFrameNum == 0 || PTS > lastPTS)
		{
			FrameSize[lastFrameNum] = len;
			lastPTS = PTS;
		}
		else
		{
			FrameSize[lastFrameNum] = FrameSize[lastFrameNum-1];
			FrameSize[lastFrameNum-1] = len;
		}
		lastFrameNum++;
	}

	if (! (mFramesEncoded % 10))
		vm_string_printf(VM_STRING("%d."), mFramesEncoded);

#ifdef CHANGE_PARAMS
	ChangeParams();
#endif

	return IPPVideoEncoder::PutOutputData(out);
}

Ipp64f FileVideoEncoder::GetTick()
{
	return (Ipp64f) vm_time_get_tick() * tickDuration;
}

int FileVideoEncoder::ReadVideoSource(BYTE* pBuffer, DWORD dwOffset, int nItemSize, int nItemCount)
{
	return (int) vm_file_fread((void*) pBuffer, (Ipp32u) nItemSize, (Ipp32s) nItemCount, m_fileSource);
}

int FileVideoEncoder::WriteVideoTarget(BYTE* pBuffer, int nItemSize, int nItemCount, FrameType frameType)
{
	return (int) vm_file_fwrite((void*) pBuffer, (Ipp32u) nItemSize, (Ipp32s) nItemCount, m_fileTarget);
}
