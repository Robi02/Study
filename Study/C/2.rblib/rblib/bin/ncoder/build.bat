gcc -c ../bytestream.c ncode_base64.c ncode_hexa.c ncoder.c -I../../include -I../../include/ncoder
ar -urcs ncoder.lib ncode_base64.o ncode_hexa.o ncoder.o bytestream.o