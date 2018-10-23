#include <stdio.h>
#include <string.h>
#include "rbmath.h"
#include "enc_base64.h"

/* [ BASE64 Encoding ]
 *
 * 1. Synopsis
 *
 * < 3Char >           < Binary 24bit >                  < Base64 6bit binary >		   < Base64 >
 *   "Man"      [01001101][01100001][01101110]		[010011][010110][000101][101110]     "TWFu"
 *
 *  > Make group each 6bits for 4 base64 char and padding '=' for empty space.
 *  > BaseSymbol: ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/
 *
 * 2. Example
 *
 *  < Input Str > "KOREA"
 *
 *  < Binary > 01001011(K) 01001111(O) 01010010(R) 01000101(E) 01000001(A)
 *
 *  < Base64 6bit binary > 010010(S)110100(0)111101(9)010010(S)010001(R)010100(U)0001[00](E)(=)
 *
 *  < Result > "S09SRUE="
 *
 */

static const char *const BASE64_SYMBOL = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const int BASE64_REVERSE_TABLE[128] = 
{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, 
  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, 
  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1 };

int EncBase64(char *pInStr, int szInStr, char *pOutStr, int szOutStr)
{
	unsigned char buf3Byte[3], buf6Bit[4];	/* InStr: buf3Byte[4], OutStr: buf6Bit[3] */
	int inStrIdx, outStrIdx, outStrLen, accumOutStrLen, bufStrLen, szRemainStr, paddingCnt;
	
	memset(pOutStr, 0, szOutStr);
	szRemainStr = szInStr;
	paddingCnt = 0;

	if ( szOutStr < 4 )
	{
		fprintf(stderr, "%s() Error: Output buffer size must be over 4byte! (CurSize:%d)\n", __FUNCTION__, szOutStr);
		return -1;
	}
	
	for ( inStrIdx = outStrIdx = accumOutStrLen = 0; inStrIdx < szRemainStr; inStrIdx += bufStrLen )
	{
		bufStrLen = min(szRemainStr - inStrIdx, 3);
		outStrLen =  bufStrLen + 1;
		accumOutStrLen += outStrLen;
		memset(buf6Bit, 0, 4);
		memset(buf3Byte, 0, 3);
		memcpy(buf3Byte, &pInStr[inStrIdx], bufStrLen);

		if ( accumOutStrLen > szOutStr )
		{
			break;
		}

		buf6Bit[0] =  (buf3Byte[0] & 0xfc) >> 2;		 						/* wwwwww00 00000000 00000000 */
		buf6Bit[1] = ((buf3Byte[0] & 0x03) << 4) | ((buf3Byte[1] & 0xf0) >> 4);	/* 000000xx xxxx0000 00000000 */
		buf6Bit[2] = ((buf3Byte[1] & 0x0f) << 2) | ((buf3Byte[2] & 0xc0) >> 6);	/* 00000000 0000yyyy yy000000 */
		buf6Bit[3] =  (buf3Byte[2] & 0x3f);										/* 00000000 00000000 00zzzzzz */

		paddingCnt = 3 - bufStrLen;
		memset(&pOutStr[outStrIdx + outStrLen], '=', paddingCnt);
		
		for ( ; outStrIdx < accumOutStrLen; ++outStrIdx )
		{
			pOutStr[outStrIdx] = BASE64_SYMBOL[buf6Bit[outStrIdx % 4]];
		}
	}
	
	return outStrIdx + paddingCnt;
}

int DecBase64(char *pInStr, int szInStr, char *pOutStr, int szOutStr)
{
	unsigned char base64Str[4], tmpByte;
	unsigned int buf4Byte;
	int i, inStrIdx, outStrIdx, accumOutStrLen, szRemainStr;

	memset(pOutStr, 0, szOutStr);
	szRemainStr = szInStr;
	
	if ( szOutStr < 3 )
	{
		fprintf(stderr, "%s() Error: Output buffer size must be over 3byte! (CurSize:%d)\n", __FUNCTION__, szOutStr);
		return -1;
	}

	if ( szRemainStr % 4 != 0 )
	{
		fprintf(stderr, "%s() Error: Base64 encoded string size is must be multiple of 4! (InStrSize:%d)\n",
				__FUNCTION__, szRemainStr);
		return -3;
	}

	for ( inStrIdx = outStrIdx = accumOutStrLen = 0; inStrIdx < szRemainStr; inStrIdx += 4 )
	{
		accumOutStrLen += 3;
		buf4Byte = 0;
		memset(base64Str, 0, 4);
		memcpy(base64Str, &pInStr[inStrIdx], 4);

		if ( accumOutStrLen > szOutStr )
		{
			break;
		}

		for ( i = 0; i < 4; ++i )
		{
            buf4Byte <<= 6;
			tmpByte = base64Str[i];
            
			if ( tmpByte < 0 || tmpByte > 127 )
			{
				/* Byte range error */
				fprintf(stderr, "%s() Error: Byte range error! (base64Str[%d]: %d)\n", __FUNCTION__, i, base64Str[i]);
				return -4;
			}
			
			if ( tmpByte == '=' )
			{
				/* Char '=' is padding symbol. Decoding logic will ignore it */
				base64Str[i] = 255;
				accumOutStrLen -= 1;
				continue;
			}

			if ( (base64Str[i] = BASE64_REVERSE_TABLE[tmpByte]) == (unsigned char)-1 )
			{
				/* Undefined symbol error */
				fprintf(stderr, "%s() Error: Undefined base64 symbol '%c' in input string!\n", __FUNCTION__, tmpByte);
				return -2;
			}

			buf4Byte |= base64Str[i]; /* 00000000 wwwwwwxx xxxxyyyy yyzzzzzz */
		}

		for ( ; outStrIdx < accumOutStrLen; ++outStrIdx )
		{
			buf4Byte <<= 8;							/* wwwwwwxx xxxxyyyy yyzzzzzz 00000000 */
			pOutStr[outStrIdx] = buf4Byte >> 24;	/* 00000000 00000000 00000000 00wwwwww */
		}
	}

	return outStrIdx;
}