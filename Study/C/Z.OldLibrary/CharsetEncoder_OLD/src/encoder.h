#ifndef __ENCODER_H__
#define __ENCODER_H__

int CheckCharset(char *pCharset);
int CheckOptions(char *pOptions);
int CheckString(char *pString);
int GetCharsetIdx(char *pCharset);
int GetOptionsIdx(char *pOptions);
int Encode(char *pCharset, char *pOptions, char *pInString, int szInStr, char *pOutString, int szOutStrBuf);

static int (*SelectEncodingFunc(int charsetIdx, int optionsIdx))(char*, int, char*, int);

#endif