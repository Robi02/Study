#ifndef __KISA_AIRA_H__
#define __KISA_ARIA_H__

#include "KISA/KISA_typedef.h"

static void printBlockOfLength(BYTE *b, int len);
static void printBlock(BYTE *b);
static int EncKeySetup(const BYTE *mk, BYTE *rk, int keyBits);
static int DecKeySetup(const BYTE *mk, BYTE *rk, int keyBits);
static void Crypt(const BYTE *i, int Nr, const BYTE *rk, BYTE *o);
static void ARIA_test();

void    ARIA_KeySched(
            BYTE    *UserKey,       
            DWORD   UserKeyLen,     
            BYTE    *RoundKey,
            int     isDecKey);

void    ARIA_Encrypt(
            BYTE    *RoundKey,      
            BYTE    *Data,
            int     szData);         
                                    
void    ARIA_Decrypt(
            BYTE    *RoundKey,      
            BYTE    *Data,
            int     szData);         

#endif