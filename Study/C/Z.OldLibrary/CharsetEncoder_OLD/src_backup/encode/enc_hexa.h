#ifndef __ENC_HEXA_H__
#define __ENC_HEXA_H__

int EncHexa(char *pInStr, int szInStr, char *pOutStr, int szOutStr);
int DecHexa(char *pInStr, int szInStr, char *pOutStr, int szOutStr);

static unsigned char CombineHalfByte(unsigned char inLeft4Bit, unsigned char inRight4Bit);

#endif