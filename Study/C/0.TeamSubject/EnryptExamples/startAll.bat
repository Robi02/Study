@echo =============================================================================================
@echo *
@echo [ SEED - ECB - NULL ]
seed -e 1 "12345678abcdefgh" "무궁화 꽃이 피었습니다."
seed -d 1 "12345678abcdefgh" "pcNCwNjjnACa+t/bcPLCJ3/bAD47f/gi+uDPYaAmxEM="
@echo *
@echo [ SEED - CBC - PKCS ]
seed -e 3 "12345678abcdefgh" "무궁화 꽃이 피었습니다."
seed -d 3 "12345678abcdefgh" "pcNCwNjjnACa+t/bcPLCJ7HqfnjWeyw86sSPxhXJxU4="
@echo *　
@echo =============================================================================================
@echo *　
@echo [ HIGHT - ECB - NULL ]
hight -e 1 "12345678abcdefgh" "무궁화 꽃이 피었습니다."
hight -d 1 "12345678abcdefgh" "lhqmaYCo2b57An+QyNomYlX3FxtXQXSE"
@echo *
@echo [ HIGHT - CBC - PKCS ]
hight -e 3 "12345678abcdefgh" "무궁화 꽃이 피었습니다."
hight -d 3 "12345678abcdefgh" "lhqmaYCo2b5O8O4vYQes+2BfdlvgXOkD"
@echo *　
@echo =============================================================================================
@echo *　
@echo [ ARIA12 - ECB - NULL ]
aria -e 1 "12345678abcdefgh" "무궁화 꽃이 피었습니다."
aria -d 1 "12345678abcdefgh" "wt22PEm0G5xfs9+qv7E52VMQG3pEqNQlbh4m9nx3VgU="
@echo *
@echo [ ARIA14 - CBC - PKCS ]
aria -e 3 "12345678abcdefghABCDEFGH" "무궁화 꽃이 피었습니다."
aria -d 3 "12345678abcdefghABCDEFGH" "bDSeXOiJtKbd+CjlpRUqXVHMH2Sg+JeO8Y6i2X79yOU="
@echo *　
@echo =============================================================================================
@echo *　
@echo [ DES - ECB - NULL ]
des -e 1 "12345678" "무궁화 꽃이 피었습니다."
des -d 1 "12345678" "ED/TeiEDoIUxe6SZB4eMoMJmdlcpWAH+"
@echo *
@echo [ DES3 - CBC - PKCS ]
des -e 3 "12345678abcdefghABCDEFGH" "무궁화 꽃이 피었습니다."
des -d 3 "12345678abcdefghABCDEFGH" "TQCxEE3nszhB1NMSqryQai8+srn6xRPt"
@echo *　
@echo =============================================================================================
@echo *　
@echo [ AES128 - ECB - NULL ]
aes -e 1 "12345678abcdefgh" "무궁화 꽃이 피었습니다."
aes -d 1 "12345678abcdefgh" "XOaJ4f6clmlreFvkx6H+9y1W8bk42Q1zhIUtdFDtcGk="
@echo *
@echo [ AES265 - CBC - PKCS ]
aes -e 3 "12345678abcdefghABCDEFGH12345678" "무궁화 꽃이 피었습니다."
aes -d 3 "12345678abcdefghABCDEFGH12345678" "7/u+x9gdBd/oYkf/3Zqcnm1oDO1+8SYB+iuEAOsmmDw="
@echo *　
@echo =============================================================================================
@echo *　
@echo [ BLOWFISH(8) - ECB - NULL ]
blowfish -e 1 "12345678" "무궁화 꽃이 피었습니다."
blowfish -d 1 "12345678" "WXo1RHBDNeWjmHIysrwpZ/NpvXLeaEOm"
@echo *
@echo [ BLOWFISH(32) - CBC - PKCS ]
blowfish -e 3 "12345678abcdefghABCDEFGH12345678" "무궁화 꽃이 피었습니다."
blowfish -d 3 "12345678abcdefghABCDEFGH12345678" "0pqtRmZmTT8R0Ld6T7MmJHFnFkjqUpxe"
@echo *　
@echo =============================================================================================