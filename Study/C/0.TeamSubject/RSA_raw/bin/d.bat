gcc -c ../src/rsarefc/digit.c ../src/rsarefc/nn.c ../src/rsarefc/r_stdlib.c ../src/rsarefc/rsa.c -I../include/rsaref
ar -rus ../lib/rsaref.lib ./*.o
mv *.o ../src/rsarefc
gcc -o modexp_rsaref ../src/modexp_rsaref.c -L../lib -L../lib/rsaref -llibcrypto -lncoder -lrsaref -lws2_32 -I../include -I../include/rsaref