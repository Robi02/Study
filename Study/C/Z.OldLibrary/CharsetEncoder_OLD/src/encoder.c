#include <stdio.h>
#include <string.h>
#include "enc_hexa.h"
#include "enc_base64.h"
#include "encoder.h"

typedef enum _ENC_CHARSET /* Encoding Charset */
{
	CS_HEXA = 0,
	CS_BASE64

} ENC_CHARSET;

typedef enum _ENC_OPTIONS /* Encoding Options */
{
	OP_ENCODE = 0,
	OP_DECODE

} ENC_OPTIONS;

static const char *G_CHARSET[] = { "-hexa", "-base64" };
static const char *G_OPTIONS[] = { "-e", "-d" };

int CheckCharset(char *pCharset)
{
	int rtVal;
	
	rtVal = GetCharsetIdx(pCharset);
	
	return ( (rtVal > -1) ? 0 : -1 );
}

int CheckOptions(char *pOptions)
{
	int rtVal;
	
	rtVal = GetOptionsIdx(pOptions);
	
	return ( (rtVal > -1) ? 0 : -1 );
}

int CheckString(char *pInString)
{
	if ( pInString == NULL )
	{
		return -1;
	}
	
	return 0;
}

int GetCharsetIdx(char *pCharset)
{
	int i, charsetCnt;
	
	charsetCnt = sizeof(G_CHARSET) / sizeof(char*);
	
	if ( pCharset == NULL )
	{
		return -1;
	}
	
	for (i = 0; i < charsetCnt; ++i)
	{
		if ( strcmp(pCharset, G_CHARSET[i]) == 0 )
		{
			return i;
		}
	}
	
	return -1;
}

int GetOptionsIdx(char *pOptions)
{
	int i, optionsCnt;
	
	optionsCnt = sizeof(G_OPTIONS) / sizeof(char*);
	
	if ( pOptions == NULL )
	{
		return -1;
	}
	
	for (i = 0; i < optionsCnt; ++i)
	{
		if ( strcmp(pOptions, G_OPTIONS[i]) == 0 )
		{
			return i;
		}
	}
	
	return -1;
}

int Encode(char *pCharset, char *pOptions, char *pInStr, int szInStr, char *pOutStr, int szOutStrBuf)
{
	int charsetIdx, optionsIdx, rtVal;
	int (*pEncoderFunc)(char*, int, char*, int);
	
	if ( (CheckCharset(pCharset) != 0) || (CheckOptions(pOptions) != 0) || (CheckString(pInStr) != 0) )
	{
		fprintf(stderr, "%s() Error: Check pCharset|Options|String. \
						(pCharset=%s, pOptions=%s, pInStr=%s)\n", __FUNCTION__, pCharset, pOptions, pInStr);
		return -1;
	}
	
	if ( (charsetIdx = GetCharsetIdx(pCharset)) < 0 )
	{
		fprintf(stderr, "%s() Error: Check pCharset|G_CHARSET. (pCharset=%s)\n", __FUNCTION__, pCharset);
		return -1;
	}
	
	if ( (optionsIdx = GetOptionsIdx(pOptions)) < 0 )
	{
		fprintf(stderr, "%s() Error: Check pOptions|G_OPTIONS. (pOptions=%s)\n", __FUNCTION__, pOptions);
		return -1;
	}
	
	if ( (pEncoderFunc = SelectEncodingFunc(charsetIdx, optionsIdx)) == NULL )
	{
		fprintf(stderr, "%s() Error: Cannot found correct En/Decoder function. \
						(pCharset=%s, pOptions=%s)\n", pCharset, pOptions);
		return -1;
	}
	
	if ( (rtVal = pEncoderFunc(pInStr, szInStr, pOutStr, szOutStrBuf)) < 0 )
	{
		fprintf(stderr, "%s() Error: pEncoderFunc() error. (returns:%d)\n", __FUNCTION__, rtVal);
		return -1;
	}
	
	return rtVal;
}

static int (*SelectEncodingFunc(int charsetIdx, int optionsIdx))(char*, int, char*, int)
{
	if ( charsetIdx == CS_HEXA )
	{
		return ( (optionsIdx == OP_ENCODE) ? EncHexa : DecHexa );
	}
	else if ( charsetIdx == CS_BASE64 )
	{
		return ( (optionsIdx == OP_ENCODE) ? EncBase64 : DecBase64 );
	}
	else if ( 0 )
	{
		/* Add new charset en/decoding function here... */
	}
	
	return NULL;
}