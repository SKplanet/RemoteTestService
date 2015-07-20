#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "g72x.h"
#include "encode.h"

void SetXBuffer(XBUFFER& xb, unsigned char* pBuffer, int nSize, int nLen, int nPos)
{
	xb.pBuffer = pBuffer;
	xb.nSize   = nSize;
	xb.nLen    = nLen;
	xb.nPos    = nPos;
}

unsigned char* GetXBufferPtr(XBUFFER xb)
{
	return (xb.pBuffer + xb.nPos);
}

int AddXBuffer(XBUFFER& xb, unsigned char data)
{
	if (xb.pBuffer != NULL)
	{
		if (xb.nLen < xb.nSize - 1)
		{
			*(xb.pBuffer + xb.nLen) = data;
			(xb.nLen)++;

			return xb.nLen;
		}
	}

	return 0;
}

int AddXBufferShort(XBUFFER& xb, short data)
{
	if (xb.pBuffer != NULL)
	{
		if (xb.nLen < xb.nSize - 2)
		{
			*((short*) ((unsigned char*) xb.pBuffer + xb.nLen)) = data;
			(xb.nLen) += sizeof(short);

			return xb.nLen;
		}
	}

	return 0;
}

int AddXBufferPos(XBUFFER& xb, int nInc)
{
	int	nPosNew = xb.nPos + nInc;

	if (nPosNew < 0 || nPosNew > xb.nLen - 1)
		return 0;

	xb.nPos = nPosNew;

	return nInc;
}

int PackOutput(unsigned int& nSavePackData, int& nSavePackBit, unsigned int nUnpackData, int nPackBitSize, XBUFFER& xbout)
{
    nSavePackData |= (nUnpackData << nSavePackBit);
    nSavePackBit += nPackBitSize;

    if (nSavePackBit >= 8)
    {
	    unsigned char	nPackData;

        nPackData = nSavePackData & 0xFF;

        nSavePackBit -= 8;
        nSavePackData >>= 8;

		AddXBuffer(xbout, nPackData);
    }

    return (nSavePackBit > 0);
}

int EncodeVoicePCM(int codec, void* pBufIn, int nLenIn, void* pBufOut, int nLenOut)
{
	XBUFFER	xbin;
	XBUFFER	xbout;

	SetXBuffer(xbin , (unsigned char*) pBufIn , nLenIn , nLenIn, 0);
	SetXBuffer(xbout, (unsigned char*) pBufOut, nLenOut, 0     , 0);

	if (nLenIn > 0)
	{
		int					enc_bits;
		struct g72x_state	state;
		int					bLoop = 1;
		unsigned char		nUnpackEncData;
		unsigned int		nSavePackData = 0;
		int					nSavePackBit  = 0;
		int					bAlignment = 0;

		switch (codec)
		{
			case G723_24 : enc_bits = 3; break;
			case G721_32 : enc_bits = 4; break;
			case G723_40 : enc_bits = 5; break;
			default : return 0;
		}

		g72x_init_state(&state);

		while (bLoop)
		{
			int nUnpackSourceData = *((short*) GetXBufferPtr(xbin));

			switch (codec)
			{
				case G723_24 : nUnpackEncData = g723_24_encoder(nUnpackSourceData, AUDIO_ENCODING_LINEAR, &state); break;
				case G721_32 : nUnpackEncData = g721_encoder   (nUnpackSourceData, AUDIO_ENCODING_LINEAR, &state); break;
				case G723_40 : nUnpackEncData = g723_40_encoder(nUnpackSourceData, AUDIO_ENCODING_LINEAR, &state); break;
			}

			bAlignment = PackOutput(nSavePackData, nSavePackBit, nUnpackEncData, enc_bits, xbout);

			bLoop = AddXBufferPos(xbin, sizeof(short));
		}

		while (bAlignment != 0)
		{
			bAlignment = PackOutput(nSavePackData, nSavePackBit, 0, enc_bits, xbout);
		}
	}

    return xbout.nLen;
}
