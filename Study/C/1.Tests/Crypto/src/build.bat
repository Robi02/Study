gcc -o crypto.exe crypto_main.c ^
-lncryptor -lncoder -lws2_32 -L ../lib ^
-I ../include -I ../include/ncoder -I ../include/ncryptor