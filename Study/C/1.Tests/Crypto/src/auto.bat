@echo off
crypto -des -ecb -null "12345678" "����ȭ ���� �Ǿ����ϴ�."
crypto -des -ecb -pkcs "12345678" "����ȭ ���� �Ǿ����ϴ�."
crypto -des -cbc -null "12345678" "����ȭ ���� �Ǿ����ϴ�." "00000000"
crypto -des -cbc -pkcs "12345678" "����ȭ ���� �Ǿ����ϴ�." "00000000"
crypto -des -ecb -null "12345678abcdefgh" "����ȭ ���� �Ǿ����ϴ�."
crypto -des -ecb -pkcs "12345678abcdefgh" "����ȭ ���� �Ǿ����ϴ�."
crypto -des -cbc -null "12345678abcdefgh" "����ȭ ���� �Ǿ����ϴ�." "00000000"
crypto -des -cbc -pkcs "12345678abcdefgh" "����ȭ ���� �Ǿ����ϴ�." "00000000"
crypto -des -ecb -null "12345678abcdefghABCDEFGH" "����ȭ ���� �Ǿ����ϴ�."
crypto -des -ecb -pkcs "12345678abcdefghABCDEFGH" "����ȭ ���� �Ǿ����ϴ�."
crypto -des -cbc -null "12345678abcdefghABCDEFGH" "����ȭ ���� �Ǿ����ϴ�." "00000000"
crypto -des -cbc -pkcs "12345678abcdefghABCDEFGH" "����ȭ ���� �Ǿ����ϴ�." "00000000"

crypto -aes -ecb -null "12345678abcdefgh" "����ȭ ���� �Ǿ����ϴ�."
crypto -aes -ecb -pkcs "12345678abcdefgh" "����ȭ ���� �Ǿ����ϴ�."
crypto -aes -cbc -null "12345678abcdefgh" "����ȭ ���� �Ǿ����ϴ�." "0000000000000000"
crypto -aes -cbc -pkcs "12345678abcdefgh" "����ȭ ���� �Ǿ����ϴ�." "0000000000000000"
crypto -aes -ecb -null "12345678abcdefghABCDEFGH" "����ȭ ���� �Ǿ����ϴ�."
crypto -aes -ecb -pkcs "12345678abcdefghABCDEFGH" "����ȭ ���� �Ǿ����ϴ�."
crypto -aes -cbc -null "12345678abcdefghABCDEFGH" "����ȭ ���� �Ǿ����ϴ�." "0000000000000000"
crypto -aes -cbc -pkcs "12345678abcdefghABCDEFGH" "����ȭ ���� �Ǿ����ϴ�." "0000000000000000"
crypto -aes -ecb -null "12345678abcdefghABCDEFGH12345678" "����ȭ ���� �Ǿ����ϴ�."
crypto -aes -ecb -pkcs "12345678abcdefghABCDEFGH12345678" "����ȭ ���� �Ǿ����ϴ�."
crypto -aes -cbc -null "12345678abcdefghABCDEFGH12345678" "����ȭ ���� �Ǿ����ϴ�." "0000000000000000"
crypto -aes -cbc -pkcs "12345678abcdefghABCDEFGH12345678" "����ȭ ���� �Ǿ����ϴ�." "0000000000000000"

crypto -blowfish -ecb -null "12345678" "����ȭ ���� �Ǿ����ϴ�."
crypto -blowfish -ecb -pkcs "12345678" "����ȭ ���� �Ǿ����ϴ�."
crypto -blowfish -cbc -null "12345678" "����ȭ ���� �Ǿ����ϴ�." "00000000"
crypto -blowfish -cbc -pkcs "12345678" "����ȭ ���� �Ǿ����ϴ�." "00000000"
crypto -blowfish -ecb -null "1234ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" "����ȭ ���� �Ǿ����ϴ�."
crypto -blowfish -ecb -pkcs "1234ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" "����ȭ ���� �Ǿ����ϴ�."
crypto -blowfish -cbc -null "1234ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" "����ȭ ���� �Ǿ����ϴ�." "00000000"
crypto -blowfish -cbc -pkcs "1234ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" "����ȭ ���� �Ǿ����ϴ�." "00000000"