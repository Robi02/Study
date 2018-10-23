#include <stdio.h>
#include <string.h>
#include "enc_hexa.h"

/* [ HEXA Encoding ]
 *
 * 1. Synopsis
 *
 *  < Binary 8bit >            < Char >   < Decimal >   < Hexa >
 *  [0][0][1][1][1][0][1][0] =    :     =     58      =    3A
 *
 *  > 1byte = 8bit / 8bit = 2hex base / 1byte = 2hex base
 *  > BaseSymbol : 0123456789ABCDEF
 *
 * 2. Example
 *
 *  < Input Str > "KOREA"
 *
 *  < Binary > 01001011(K) 01001111(O) 01010010(R) 01000101(E) 01000001(A)
 *
 *  < Hexa > 4(0100)B(1011) 4(0100)F(1111) 5(0101)2(0010) 4(0100)5(0101) 4(0100)1(0001)
 *
 *  < Result > "4B4F524541"
 *
 */

static const char *const HEXA_SYMBOL = "0123456789ABCDEF";
static const int HEXA_REVERSE_TABLE[128] = 
{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1, 
  -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

int EncHexa(char *pInStr, int szInStr, char *pOutStr, int szOutStr)
{
	unsigned char oneByte, left4Bit, right4Bit;
	int inStrIdx, outStrIdx, szRemainStr;
	
	memset(pOutStr, 0, szOutStr);
	szRemainStr = szInStr;

	if ( szOutStr < 2 )
	{
		fprintf(stderr, "%s() Error: Output buffer size must be over 2byte! (CurSize:%d)\n", __FUNCTION__, szOutStr);
		return -1;
	}
	
	for ( inStrIdx = outStrIdx = 0; inStrIdx < szRemainStr; ++inStrIdx )
	{
		if ( outStrIdx + 2 > szOutStr )
		{
			break;
		}
		
		oneByte   = pInStr[inStrIdx];
		left4Bit  = oneByte >> 4;
		right4Bit = oneByte & 0x0f;
		
		pOutStr[outStrIdx++] = HEXA_SYMBOL[left4Bit];
		pOutStr[outStrIdx++] = HEXA_SYMBOL[right4Bit];
	}

	return outStrIdx;
}

int DecHexa(char *pInStr, int szInStr, char *pOutStr, int szOutStr)
{
	unsigned char left4Bit, right4Bit, combinedByte;
	int inStrIdx, outStrIdx, szRemainStr;

	memset(pOutStr, 0, szOutStr);
	szRemainStr = szInStr;

	if ( szOutStr < 1 )
	{
		fprintf(stderr, "%s() Error: Output buffer size must be over 1byte! (CurSize:%d)\n", __FUNCTION__, szOutStr);
		return -1;
	}
	
	if ( szRemainStr % 2 != 0 )
	{
		fprintf(stderr, "%s() Error: Hexa encoded string size is must be multiple of 2! (InStrSize:%d)\n", __FUNCTION__, szRemainStr);
		return -3;
	}
	
	for ( inStrIdx = outStrIdx = 0; inStrIdx + 1 < szRemainStr; )
	{
		if ( outStrIdx + 1 > szOutStr )
		{
			break;
		}
		
		left4Bit  = pInStr[inStrIdx++];
		right4Bit = pInStr[inStrIdx++];
		
		if ( (combinedByte = CombineHalfByte(left4Bit, right4Bit)) < 0 )
		{
			return combinedByte;
		}

		pOutStr[outStrIdx++] = (char)combinedByte;
	}
	
	return outStrIdx;
}

static unsigned char CombineHalfByte(unsigned char inLeft4Bit, unsigned char inRight4Bit)
{
	unsigned char *pInByte, left4Bit, right4Bit, tmpByte, rtByte;
	int i;
	
	left4Bit  = inLeft4Bit;
	right4Bit = inRight4Bit;
	
	for ( i = 0 ; i < 2; ++i )
	{
		/* 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 65, 66, 67, 68, 69, 70  < ASCII >
		    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  A,  B,  C,  D,  E,  F  < HEXA  > */

		pInByte = (i == 0 ? &left4Bit : &right4Bit);
		tmpByte = *pInByte;

		if ( tmpByte < 0 || tmpByte > 127 )
		{
			/* Byte range error */
			fprintf(stderr, "%s() Error: Byte range error! (tmpByte: %d)\n", __FUNCTION__, i, tmpByte);
			return -4;
		}

		if ( (*pInByte = HEXA_REVERSE_TABLE[tmpByte]) == (unsigned char)-1 )
		{
			/* Undefined symbol error */
			fprintf(stderr, "%s() Error: Undefined symbol '%c' in input string!\n", __FUNCTION__, tmpByte);
			return -2;
		}
	}

	rtByte = 0x00;
	rtByte |= left4Bit << 4;
	rtByte |= right4Bit;
	
	return rtByte;
}