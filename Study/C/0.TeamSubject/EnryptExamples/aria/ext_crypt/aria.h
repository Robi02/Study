#include <stdio.h>
 
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

#define ARIA_rounds(kbytes)\
		((kbytes+32)/4)

#define ARIA_bits(kbytes)\
		((kbytes*8)

#define ARIA_bytes(kbits)\
		((kbits/8)

#define ARIA_set_encrypt_key(userKey, bits, roundKey) \
		EncKeySetup(userKey, roundKey ,bits)
#define ARIA_set_decrypt_key(userKey, bits, roundKey) \
		DecKeySetup(userKey, roundKey ,bits)
#define ARIA_encrypt(in, out, roundKey, rounds) \
		Crypt(in, rounds, roundKey, out)
#define ARIA_decrypt(in, out, roundKey, rounds) \
		Crypt(in, rounds, roundKey, out)
