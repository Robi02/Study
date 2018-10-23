@echo off
rem =========================================
rem [ Code ]
set EXE_ARIA=aria.exe
set CODE_ARIA01=aria/aria_test.c
rem ----------------------------------------
set EXE_HIGHT=hight.exe
set CODE_HIGHT01=hight/hight_test.c
rem ----------------------------------------
set EXE_SEED=seed.exe
set CODE_SEED01=seed/seed_test.c
rem ----------------------------------------
set EXE_DES=des.exe
set CODE_DES01=des/des_test.c
rem ----------------------------------------
set EXE_AES=aes.exe
set CODE_AES01=aes/aes_test.c
rem ----------------------------------------
set EXE_BLOWFISH=blowfish.exe
set CODE_BLOWFISH01=blowfish/blowfish_test.c
rem =========================================
rem [ GCC ]
echo "========================================="
echo "< ARIA 빌드 시작... >"
gcc -o %EXE_ARIA% %CODE_ARIA01% ^
-I _include ^
-I _include/ncryptor ^
-I _include/ncoder ^
-L _lib ^
-lncryptor -lncoder -lws2_32
echo "< ARIA 완료. >"
echo "========================================="
echo "< HIGHT 빌드 시작... >"
gcc -o %EXE_HIGHT% %CODE_HIGHT01% ^
-I _include ^
-I _include/ncryptor ^
-I _include/ncoder ^
-L _lib ^
-lncryptor -lncoder -lws2_32
echo "< HIGHT 완료. >"
echo "========================================="
echo "< SEED 빌드 시작... >"
gcc -g -o %EXE_SEED% %CODE_SEED01% ^
-I _include ^
-I _include/ncryptor ^
-I _include/ncoder ^
-L _lib ^
-lncryptor -lncoder -lws2_32
echo "< SEED 완료. >"
echo "========================================="
echo "< DES 빌드 시작... >"
gcc -g -o %EXE_DES% %CODE_DES01% ^
-I _include ^
-I _include/ncryptor ^
-I _include/ncoder ^
-L _lib ^
-lncryptor -lncoder -lws2_32
echo "< DES 완료. >"
echo "========================================="
echo "< AES 빌드 시작... >"
gcc -g -o %EXE_AES% %CODE_AES01% ^
-I _include ^
-I _include/ncryptor ^
-I _include/ncoder ^
-L _lib ^
-lncryptor -lncoder -lws2_32
echo "< AES 완료. >"
echo "========================================="
echo "< BLOWFISH 빌드 시작... >"
gcc -g -o %EXE_BLOWFISH% %CODE_BLOWFISH01% ^
-I _include ^
-I _include/ncryptor ^
-I _include/ncoder ^
-L _lib ^
-lncryptor -lncoder -lws2_32
echo "< BLOWFISH 완료. >"
echo "========================================="
rem =========================================