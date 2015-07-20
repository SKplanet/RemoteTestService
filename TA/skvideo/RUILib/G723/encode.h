#ifndef _ENCODE_H
#define _ENCODE_H

#define G723_24		1
#define G721_32		2
#define G723_40		3
#define PCM_16		13

typedef struct _tagXBUFFER
{
	unsigned char*	pBuffer;
	int				nSize;
	int				nLen;
	int				nPos;
} XBUFFER;

void SetXBuffer(XBUFFER& xb, unsigned char* pBuffer, int nSize, int nLen, int nPos);
unsigned char* GetXBufferPtr(XBUFFER xb);
int AddXBuffer(XBUFFER& xb, unsigned char data);
int AddXBufferShort(XBUFFER& xb, short data);
int AddXBufferPos(XBUFFER& xb, int nInc);

int EncodeVoicePCM(int codec, void* pBufIn, int nLenIn, void* pBufOut, int nLenOut);

#endif
