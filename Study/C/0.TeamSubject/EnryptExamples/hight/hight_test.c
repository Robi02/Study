#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>

#include "bytestream.h"
#include "lib_ncoder.h"
#include "lib_ncryptor.h"

#define _MAX_MSG_SIZE     1024
#define _CRYPT_BLOCK_SIZE 8

int encrypt_hight_cbc_pkcs7(char *tdata ,char *sdata, int slen, unsigned char *key16, unsigned char *iv);
int decrypt_hight_cbc_pkcs7(char *tdata ,char *sdata, int slen, unsigned char *key16, unsigned char *iv);
int encrypt_hight_ecb(char *tdata ,char *sdata, int slen, char pchar ,unsigned char *key16);
int decrypt_hight_ecb(char *tdata ,char *sdata, int slen, char pchar ,unsigned char *key16);

static void str2hex(unsigned char *s, char *dest, int n ,int add_null);
static void hex2str(char *h, char *s, int n ,int add_null);
static void printUsage(int argno, char **argv);

static char *P_CHARSET = "-base64";

void main(int argc, char **argv)
{
    unsigned char	iv[_CRYPT_BLOCK_SIZE ] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

    char pchar;
    char ekey[64] ,smsg[1024] ,tmsg[1024];
    int  is_pkcs7 = 0 ,is_enc = 0 ,klen = 0 ,slen = 0 ,tlen = 0;

    if (argc != 5) { printUsage(0, argv);}
    	
    memset(ekey ,0x00 ,sizeof(ekey));
    memset(smsg ,0x00 ,sizeof(smsg));
    memset(tmsg ,0x00 ,sizeof(tmsg));

    /* 01. encoding or decoding                                                             */
    if       (0 == strcmp(argv[1] ,"-e")) { is_enc    = 1   ; }
    else if  (0 == strcmp(argv[1] ,"-d")) { is_enc    = 0   ; }
    else                                  { printUsage(1, argv);}

    /* 02. padding type                                                                     */
    if      (0 == strcmp(argv[2],"1")) { pchar = 0x00; }
    else if (0 == strcmp(argv[2],"2")) { pchar = 0x20; }
    else if (0 == strcmp(argv[2],"3")) { is_pkcs7 = 1; pchar = 0xff;}
    else                               { printUsage(2, argv);}
   
    /* 03. key                                                                              */
    if (0 == memcmp(argv[3] ,"H/" ,2))
    {
		klen = (strlen(argv[3]) - 2);
		if (32 != klen) {printUsage(3, argv);}
				
		hex2str(&argv[3][2], ekey, klen ,0);
		
		klen = klen / 2;
	}else
	{
		klen = strlen(argv[3]);
		if (16 != klen) {printUsage(3, argv);}
			
		memcpy(ekey ,argv[3], klen);
	}

    /* 04. msg                                                                              */
    if (0 == memcmp(argv[4] ,"H/" ,2))
    {
		slen = (strlen(argv[4]) - 2);
		if (!is_enc && 0 != slen % 16) {printUsage(4, argv);}
				
		hex2str(&argv[4][2], smsg, slen ,0);
		
		slen = slen / 2;
	}else
	{
		slen = strlen(argv[4]);			
		memcpy(smsg ,argv[4], slen);
	}
	
	if (is_pkcs7)
	{
		if (is_enc) { tlen = encrypt_hight_cbc_pkcs7(tmsg ,smsg, slen, (unsigned char *)ekey, iv); }
		else        { tlen = decrypt_hight_cbc_pkcs7(tmsg ,smsg, slen, (unsigned char *)ekey, iv); }
	}else
	{
		if (is_enc) { tlen = encrypt_hight_ecb(tmsg ,smsg, slen, pchar, (unsigned char *)ekey); }
		else        { tlen = decrypt_hight_ecb(tmsg ,smsg, slen, pchar, (unsigned char *)ekey); }
	}

	// test
	char tmsgd[_MAX_MSG_SIZE];
	memset(tmsgd, 0x00, sizeof(tmsgd));
	memcpy(tmsgd, tmsg, tlen);
	fprintf(stdout, "1.STRING:\n"); PrintStreamToString(tmsgd, tlen);
	fprintf(stdout, "2.HEXA:\n");   PrintStreamToHexa(tmsgd, tlen);
	// test

    printf("3.RESULT!!\nenc(%s) padding(%s) key(%s) s(%d:[%s])->t(%d:[%s])\n",argv[1],argv[2],argv[3],slen,argv[4],tlen,tmsg);
}

int encrypt_hight_cbc_pkcs7(char *tdata ,char *sdata, int slen, unsigned char *key16, unsigned char *iv)
{
	unsigned char pBuf[_MAX_MSG_SIZE];
	NCRYPT_ALGO_HIGHT_ARGS stHIGHT;
	NCODE_TYPE_BASE64_ARGS stBase64;
	int rtVal;

	memset(&stHIGHT, 0x00, sizeof(stHIGHT));
	stHIGHT.stArgs.isEncrypt = 1;
	stHIGHT.stArgs.eAlgo = NCRYPT_ALGO_HIGHT;
	stHIGHT.stArgs.ePadd = NCRYPT_PADD_PKCS;
	stHIGHT.stArgs.eMode = NCRYPT_MODE_CBC;
	stHIGHT.stArgs.pbIV  = iv;
	stHIGHT.stArgs.szIV  = _CRYPT_BLOCK_SIZE;
	stHIGHT.stArgs.pbKey = key16;
	stHIGHT.stArgs.szKey = strlen(key16);
	stHIGHT.stArgs.pbInStream = sdata;
	stHIGHT.stArgs.szInStream = slen;
	stHIGHT.stArgs.pbOutBuf   = pBuf;
	stHIGHT.stArgs.szOutBuf   = _MAX_MSG_SIZE;

	if ((rtVal = Ncrypt(&stHIGHT, sizeof(stHIGHT))) < 0)
	{
		fprintf(stderr, "Ncrypt(encrypt_HIGHT_cbc_pkcs7) error. (return:%d)\n", rtVal);
		return rtVal;
	}

	memset(&stBase64, 0x00, sizeof(stBase64));
	stBase64.isEncode   = 1;
	stBase64.pbInStream = pBuf;
	stBase64.szInStream = rtVal;
	stBase64.pbOutBuf   = tdata;
	stBase64.szOutBuf   = _MAX_MSG_SIZE;

	if ((rtVal = Ncode(NCODE_TYPE_BASE64, &stBase64, sizeof(stBase64))) < 0)
	{
		fprintf(stderr, "Encode(encrypt_HIGHT_cbc_pkcs7) error. (return:%d)\n", rtVal);
		return rtVal;
	}

	return rtVal;
}

int decrypt_hight_cbc_pkcs7(char *tdata ,char *sdata, int slen, unsigned char *key16, unsigned char *iv)
{
	unsigned char pBuf[_MAX_MSG_SIZE];
	NCRYPT_ALGO_HIGHT_ARGS stHIGHT;
	NCODE_TYPE_BASE64_ARGS stBase64;
	int rtVal;

	memset(&stBase64, 0x00, sizeof(stBase64));
	stBase64.isEncode   = 0;
	stBase64.pbInStream = sdata;
	stBase64.szInStream = slen;
	stBase64.pbOutBuf   = pBuf;
	stBase64.szOutBuf   = _MAX_MSG_SIZE;

	if ((rtVal = Ncode(NCODE_TYPE_BASE64, &stBase64, sizeof(stBase64))) < 0)
	{
		fprintf(stderr, "Encode(encrypt_HIGHT_cbc_pkcs7) error. (return:%d)\n", rtVal);
		return rtVal;
	}

	memset(&stHIGHT, 0x00, sizeof(stHIGHT));
	stHIGHT.stArgs.isEncrypt = 0;
	stHIGHT.stArgs.eAlgo = NCRYPT_ALGO_HIGHT;
	stHIGHT.stArgs.ePadd = NCRYPT_PADD_PKCS;
	stHIGHT.stArgs.eMode = NCRYPT_MODE_CBC;
	stHIGHT.stArgs.pbIV  = iv;
	stHIGHT.stArgs.szIV  = _CRYPT_BLOCK_SIZE;
	stHIGHT.stArgs.pbKey = key16;
	stHIGHT.stArgs.szKey = strlen(key16);
	stHIGHT.stArgs.pbInStream = pBuf;
	stHIGHT.stArgs.szInStream = rtVal;
	stHIGHT.stArgs.pbOutBuf   = tdata;
	stHIGHT.stArgs.szOutBuf   = _MAX_MSG_SIZE;

	if ((rtVal = Ncrypt(&stHIGHT, sizeof(stHIGHT))) < 0)
	{
		fprintf(stderr, "Ncrypt(encrypt_HIGHT_cbc_pkcs7) error. (return:%d)\n", rtVal);
		return rtVal;
	}

	return rtVal;
}

int encrypt_hight_ecb(char *tdata ,char *sdata, int slen, char pchar ,unsigned char *key16)
{
	unsigned char pBuf[_MAX_MSG_SIZE];
	NCRYPT_ALGO_HIGHT_ARGS stHIGHT;
	NCODE_TYPE_BASE64_ARGS stBase64;
	int rtVal;

	memset(&stHIGHT, 0x00, sizeof(stHIGHT));
	stHIGHT.stArgs.isEncrypt = 1;
	stHIGHT.stArgs.eAlgo = NCRYPT_ALGO_HIGHT;
	stHIGHT.stArgs.ePadd = NCRYPT_PADD_NULL;
	stHIGHT.stArgs.eMode = NCRYPT_MODE_ECB;
	stHIGHT.stArgs.pbKey = key16;
	stHIGHT.stArgs.szKey = strlen(key16);
	stHIGHT.stArgs.pbInStream = sdata;
	stHIGHT.stArgs.szInStream = slen;
	stHIGHT.stArgs.pbOutBuf   = pBuf;
	stHIGHT.stArgs.szOutBuf   = _MAX_MSG_SIZE;

	if ((rtVal = Ncrypt(&stHIGHT, sizeof(stHIGHT))) < 0)
	{
		fprintf(stderr, "Ncrypt(encrypt_HIGHT_cbc_pkcs7) error. (return:%d)\n", rtVal);
		return rtVal;
	}

	memset(&stBase64, 0x00, sizeof(stBase64));
	stBase64.isEncode   = 1;
	stBase64.pbInStream = pBuf;
	stBase64.szInStream = rtVal;
	stBase64.pbOutBuf   = tdata;
	stBase64.szOutBuf   = _MAX_MSG_SIZE;

	if ((rtVal = Ncode(NCODE_TYPE_BASE64, &stBase64, sizeof(stBase64))) < 0)
	{
		fprintf(stderr, "Encode(encrypt_HIGHT_cbc_pkcs7) error. (return:%d)\n", rtVal);
		return rtVal;
	}

	return rtVal;
}

int decrypt_hight_ecb(char *tdata ,char *sdata, int slen, char pchar ,unsigned char *key16)
{
	unsigned char pBuf[_MAX_MSG_SIZE];
	NCRYPT_ALGO_HIGHT_ARGS stHIGHT;
	NCODE_TYPE_BASE64_ARGS stBase64;
	int rtVal;

	memset(&stBase64, 0x00, sizeof(stBase64));
	stBase64.isEncode   = 0;
	stBase64.pbInStream = sdata;
	stBase64.szInStream = slen;
	stBase64.pbOutBuf   = pBuf;
	stBase64.szOutBuf   = _MAX_MSG_SIZE;

	if ((rtVal = Ncode(NCODE_TYPE_BASE64, &stBase64, sizeof(stBase64))) < 0)
	{
		fprintf(stderr, "Encode(encrypt_HIGHT_cbc_pkcs7) error. (return:%d)\n", rtVal);
		return rtVal;
	}

	memset(&stHIGHT, 0x00, sizeof(stHIGHT));
	stHIGHT.stArgs.isEncrypt = 0;
	stHIGHT.stArgs.eAlgo = NCRYPT_ALGO_HIGHT;
	stHIGHT.stArgs.ePadd = NCRYPT_PADD_NULL;
	stHIGHT.stArgs.eMode = NCRYPT_MODE_ECB;
	stHIGHT.stArgs.pbKey = key16;
	stHIGHT.stArgs.szKey = strlen(key16);
	stHIGHT.stArgs.pbInStream = pBuf;
	stHIGHT.stArgs.szInStream = rtVal;
	stHIGHT.stArgs.pbOutBuf   = tdata;
	stHIGHT.stArgs.szOutBuf   = _MAX_MSG_SIZE;

	if ((rtVal = Ncrypt(&stHIGHT, sizeof(stHIGHT))) < 0)
	{
		fprintf(stderr, "Ncrypt(encrypt_HIGHT_cbc_pkcs7) error. (return:%d)\n", rtVal);
		return rtVal;
	}

	return rtVal;
}

static void str2hex(unsigned char *s, char *dest, int n ,int add_null)
{
	int i;
	char buf[3];
	for(i = 0;n-->0;s++, i += 2)
	{
		if( !(*s) && n > 0 )
		{
		    sprintf(buf, "00" );
		}else
		{
			sprintf(buf, "%0.2X", (unsigned char)*s);
		}
		memcpy(dest+i,buf,2);
	}
	if (add_null) *(dest+i) = 0x00;
}

static void hex2str(char *h, char *s, int n ,int add_null)
{
	int i,j;
	for(j=0; (!n || j<n) && (sscanf(h+j, "%02X", &i) == 1 ); j += 2)
	{
		*s++ = i;
	}
	if (add_null) *s = 0;
}

static void printUsage(int argno, char **argv)
{
    printf("ERROR!! invalid arg(%d)(%s)\n", argno,argv[argno]                           );
    printf("-------------------------------------------------------------------------\n");
    printf("usage : %s -d/e 1/2/3 key(8~32) msg\n", argv[0]);
    printf("           d/e : decoding/encoding                                       \n");
    printf("           1/2/3 : padding(1-0x00, 2-0x20 ,3-pkcs5,7)                    \n");
    printf("           key : ascii or H/16hex                                        \n");
    printf("-------------------------------------------------------------------------\n");
    printf("example1 : %s -e 1 12345678abcdefgh '무궁화 꽃이 피었습니다.'         \n", argv[0]);
    printf("example2 : %s -d 1 12345678abcdefgh '0ABC862F0035348B0ABC862F0035348B'\n", argv[0]);
    printf("-------------------------------------------------------------------------\n");
    printf("comment  : 타언어와 호환성이 필요할 경우 padding은 0x20으로...           \n");
    printf("           암호화결과/복화화입력값은 base64 encoding 적용                \n");
    printf("           3key 방식의 triple des(168 DESede)는 사용빈도가 낮음          \n");
    printf("           ksnet의 working key 형태는 16hex로 MASTER KEY 확인            \n");
    printf("-------------------------------------------------------------------------\n");
    
    exit(0);
}
