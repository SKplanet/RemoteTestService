#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "g72x.h"
#include "encode.h"

int UnpackInput(unsigned int& nSavePackData, int& nSavePackBit, XBUFFER& xbin, unsigned char& nUnpackData, int nPackBitSize)
{
    if (nSavePackBit < nPackBitSize)
    {
		if (xbin.nPos >= xbin.nLen)
		{
			nUnpackData = 0;
			return -1;
		}

	    unsigned char	nPackData;

		nPackData = *((unsigned char*) GetXBufferPtr(xbin));
		if (AddXBufferPos(xbin, sizeof(unsigned char)) == 0)
		{
			xbin.nPos = xbin.nLen;
		}

        nSavePackData |= (nPackData << nSavePackBit);
        nSavePackBit += 8;
    }

    nUnpackData = nSavePackData & ((1 << nPackBitSize) - 1);

    nSavePackData >>= nPackBitSize;
    nSavePackBit -= nPackBitSize;

    return (nSavePackBit > 0);
}

int DecodeVoicePCM(int codec, void* pBufIn, int nLenIn, void* pBufOut, int nLenOut)
{
    struct g72x_state	state;
    int					dec_bits;

	XBUFFER				xbin;
	XBUFFER				xbout;

	unsigned int		nSavePackData = 0;
	int					nSavePackBit  = 0;
    unsigned char		nUnpackEncData;

	switch (codec)
	{
		case G723_24 : dec_bits = 3; break;
		case G721_32 : dec_bits = 4; break;
		case G723_40 : dec_bits = 5; break;
		default : return 0;
	}

    g72x_init_state(&state);

	SetXBuffer(xbin , (unsigned char*) pBufIn , nLenIn , nLenIn, 0);
	SetXBuffer(xbout, (unsigned char*) pBufOut, nLenOut, 0     , 0);

    while (UnpackInput(nSavePackData, nSavePackBit, xbin, nUnpackEncData, dec_bits) >= 0)
    {
		int nUnpackSourceData;

		switch (codec)
		{
			case G723_24 : nUnpackSourceData = g723_24_decoder(nUnpackEncData, AUDIO_ENCODING_LINEAR, &state); break;
			case G721_32 : nUnpackSourceData = g721_decoder   (nUnpackEncData, AUDIO_ENCODING_LINEAR, &state); break;
			case G723_40 : nUnpackSourceData = g723_40_decoder(nUnpackEncData, AUDIO_ENCODING_LINEAR, &state); break;
		}

		AddXBufferShort(xbout, nUnpackSourceData);
    }

    return xbout.nLen;
}
