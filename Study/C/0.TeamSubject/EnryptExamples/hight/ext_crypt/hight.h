/***************************************************************************
 *
 * File : HIGHT_KISA.h
 *
 * Description : header file for HIGHT_KISA.c
 *
 **************************************************************************/

#ifndef _HIGHT_H_
#define _HIGHT_H_

/*************** Header files *********************************************/


/*************** Definitions **********************************************/

#if defined(USER_BIG_ENDIAN)
	#define CT_BIG_ENDIAN
#elif defined(USER_LITTLE_ENDIAN)
	#define CT_LITTLE_ENDIAN
#else
	#if __alpha__	||	__alpha	||	__i386__	||	i386	||	_M_I86	||	_M_IX86	||	\
		__OS2__		||	sun386	||	__TURBOC__	||	vax		||	vms		||	VMS		||	__VMS 
		#define CT_LITTLE_ENDIAN
	#else
		#define CT_BIG_ENDIAN
	#endif
#endif

/*************** Constants ************************************************/

/*************** Macros ***************************************************/
/* // */
#define ROTL_BYTE(x, n) ( (BYTE)((x) << (n)) | (DWORD)((x) >> (8-(n))) )
#define ROTR_BYTE(x, n) ( (BYTE)((x) >> (n)) | (DWORD)((x) << (8-(n))) )

#if defined(_MSC_VER)
    #define ROTL_DWORD(x, n) _lrotl((x), (n))
    #define ROTR_DWORD(x, n) _lrotr((x), (n))
#else
    #define ROTL_DWORD(x, n) ( (DWORD)((x) << (n)) | (DWORD)((x) >> (32-(n))) )
    #define ROTR_DWORD(x, n) ( (DWORD)((x) >> (n)) | (DWORD)((x) << (32-(n))) )
#endif

/* //    reverse the byte order of DWORD(DWORD:4-bytes integer). */
#define ENDIAN_REVERSE_DWORD(dwS)   ( (ROTL_DWORD((dwS),  8) & 0x00ff00ff)  \
                                    | (ROTL_DWORD((dwS), 24) & 0xff00ff00) )

/* // */
#if defined(CT_BIG_ENDIAN)      /* //    Big-Endian machine */
    #define BIG_B2D(B, D)       D = *(DWORD *)(B)
    #define BIG_D2B(D, B)       *(DWORD *)(B) = (DWORD)(D)
    #define LITTLE_B2D(B, D)    D = ENDIAN_REVERSE_DWORD(*(DWORD *)(B))
    #define LITTLE_D2B(D, B)    *(DWORD *)(B) = ENDIAN_REVERSE_DWORD(D)
#elif defined(CT_LITTLE_ENDIAN) /* //    Little-Endian machine */
    #define BIG_B2D(B, D)       D = ENDIAN_REVERSE_DWORD(*(DWORD *)(B))
    #define BIG_D2B(D, B)       *(DWORD *)(B) = ENDIAN_REVERSE_DWORD(D)
    #define LITTLE_B2D(B, D)    D = *(DWORD *)(B)
    #define LITTLE_D2B(D, B)    *(DWORD *)(B) = (DWORD)(D)
#else
    #error ERROR : Invalid DataChangeType
#endif

#if defined(_MSC_VER)
    #define INLINE  _inline
#else
    #define INLINE  inline
#endif

/*************** New Data Types *******************************************/
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
/*************** Prototypes ***********************************************/
void    HIGHT_KeySched(
            BYTE    *UserKey,       
            DWORD   UserKeyLen,     
            BYTE    *RoundKey);     
void    HIGHT_Encrypt(
            BYTE    *RoundKey,      
            BYTE    *Data);         
                                    
void    HIGHT_Decrypt(
            BYTE    *RoundKey,      
            BYTE    *Data);         
                                    


#endif  /* _HIGHT_H_ */