#ifndef __COMMON_INCLUDE_H__
#define __COMMON_INCLUDE_H__

// [ System(x86/64) define. ]
#ifndef __X64_SYS__
    #ifndef __X86_SYS__
        #define __X86_SYS__		// >Default : __X86_SYS__
    #endif
#endif

// [ Archtecture(LITTLE/BIG endian)  define. ]
#ifndef __BIG_END_CPU__
    #ifndef __LIT_END_CPU__
        #define __LIT_END_CPU__	// >Default : __LIT_END_CPU__
    #endif
#endif

// [ Build mode(DEBUG/RELEASE) define. ]
#ifndef __DEBUG__
    #ifndef __RELEASE__
        #define __RELEASE__		// >Default : __RELEASE__
    #endif
#endif

#endif