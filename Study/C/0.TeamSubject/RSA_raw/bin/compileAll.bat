gcc -o modexp ../src/modexp.c -L../lib -llibcrypto -lws2_32
gcc -o modexp_openssl ../src/modexp_openssl.c -L../lib -llibcrypto -lncoder -lws2_32 -I../include
gcc -o modexp_rsaref ../src/modexp_rsaref.c -L../lib -L../lib/rsaref -llibcrypto -lncoder -lrsaref -lws2_32 -I../include -I../include/rsaref