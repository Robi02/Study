/*******************************************************************************
*
* FILE:         KISA_SEED_ECB.h
*
* DESCRIPTION:  header file for KISA_SEED_ECB.c
*
*******************************************************************************/

#ifndef _SEED_H
#define _SEED_H


/******************************* Include files ********************************/
#ifndef u8
#define u8   unsigned char        /*   1-byte data type */
#endif

#ifndef u16
#define u16   unsigned short      /*   2-byte data type */
#endif

#ifndef u32
#ifdef _LP64
#define u32 unsigned int       /*   4-byte data type */
#else
#define u32 unsigned long
#endif
#endif

/******************************* Type Definitions *****************************/
#define BYTE    u8       /*   1-byte data type */
#define WORD    u16      /*   2-byte data type */
#define DWORD   u32      /*   4-byte data type */
/***************************** Endianness Define ******************************/
/*  If endianness is not defined correctly, you must modify here. */
/*  SEED uses the Little endian as a defalut order */

#if defined(USER_BIG_ENDIAN)
	#define BIG_ENDIAN
#elif defined(USER_LITTLE_ENDIAN)
	#define LITTLE_ENDIAN
#else
#if __alpha__	||	__alpha	||	__i386__	||	i386	||	_M_I86	||	_M_IX86	||	\
	__OS2__		||	sun386	||	__TURBOC__	||	vax		||	vms		||	VMS		||	__VMS 
#define LITTLE_ENDIAN
#else
#define BIG_ENDIAN
#endif
#endif

/**************************** Constant Definitions ****************************/

#define NoRounds         16						/*  the number of rounds */
#define NoRoundKeys      (NoRounds*2)			/*  the number of round-keys */
#define SeedBlockSize    16    					/*  block length in bytes */
#define SeedBlockLen     128   					/*  block length in bits */


/******************************** Common Macros *******************************/

/*  macroses for left or right rotations */
#if defined(_MSC_VER)
    #define ROTL(x, n)     (_lrotl((x), (n)))		/*  left rotation */
    #define ROTR(x, n)     (_lrotr((x), (n)))		/*  right rotation */
#else
    #define ROTL(x, n)     (((x) << (n)) | ((x) >> (32-(n))))		/*  left rotation */
    #define ROTR(x, n)     (((x) >> (n)) | ((x) << (32-(n))))		/*  right rotation */
#endif

/*  macroses for converting endianess */
#define EndianChange(dwS)                       \
    ( (ROTL((dwS),  8) & (DWORD)0x00ff00ff) |   \
      (ROTL((dwS), 24) & (DWORD)0xff00ff00) )


/*************************** Function Declarations ****************************/

void SEED_Encrypt(		/* encryption function */
		BYTE *pbData, 				/*  [in,out]	data to be encrypted */
		DWORD *pdwRoundKey			/*  [in]			round keys for encryption */
		);
    
void SEED_Decrypt(		/* decryption function */
		BYTE *pbData, 				/*  [in,out]	data to be decrypted */
		DWORD *pdwRoundKey			/*  [in]			round keys for decryption */
		);
    
void SEED_KeySched(		/* key scheduling function */
		DWORD *pdwRoundKey, 		/*  [out]		round keys for encryption or decryption */
		BYTE *pbUserKey				/*  [in]			secret user key  */
		);


/*************************** END OF FILE **************************************/
#endif
