#include "bytestream.h"
#include "common_include.h"
#include "common_ncrypt.h"
#include "ncrypt_aes.h"
#include "ncrypt_aria.h"
#include "ncrypt_blowfish.h"
#include "ncrypt_des.h"
#include "ncrypt_hight.h"
#include "ncrypt_seed.h"
#include "ncryptor.h"

int Ncrypt(void *pstNcryptArgs, int szArgs)
{
    NCRYPT_ALGO eNcryptAlgo;

    if (pstNcryptArgs == NULL || szArgs < sizeof(NCRYPT_COMMON_BLOCK_ARGS))
    {
        fprintf(stderr, "pstNcryptArgs == NULL || szArgs < %d (pstNcryptArgs:0x%p, szArgs:%d)\n",
                sizeof(NCRYPT_COMMON_BLOCK_ARGS), pstNcryptArgs, szArgs);
        return -1;
    }

    eNcryptAlgo = ((NCRYPT_COMMON_BLOCK_ARGS *)pstNcryptArgs)->eAlgo;

    if (eNcryptAlgo == NCRYPT_ALGO_SEED)
    {
        if (sizeof(NCRYPT_ALGO_SEED_ARGS) != szArgs)
        {
            fprintf(stderr, "sizeof(NCRYPT_ALGO_SEED_ARGS) != szArgs (%ld != %d)\n", sizeof(NCRYPT_ALGO_SEED_ARGS), szArgs);
            return -1;
        }

        return Ncrypt_SEED((NCRYPT_ALGO_SEED_ARGS *)pstNcryptArgs);
    }
    else if (eNcryptAlgo == NCRYPT_ALGO_HIGHT)
    {
        if (sizeof(NCRYPT_ALGO_HIGHT_ARGS) != szArgs)
        {
            fprintf(stderr, "sizeof(NCRYPT_ALGO_HIGHT_ARGS) != szArgs (%ld != %d)\n", sizeof(NCRYPT_ALGO_HIGHT_ARGS), szArgs);
            return -1;
        }

        return Ncrypt_HIGHT((NCRYPT_ALGO_HIGHT_ARGS *)pstNcryptArgs);
    }
    else if (eNcryptAlgo == NCRYPT_ALGO_ARIA)
    {
        if (sizeof(NCRYPT_ALGO_ARIA_ARGS) != szArgs)
        {
            fprintf(stderr, "sizeof(NCRYPT_ALGO_ARIA_ARGS) != szArgs (%ld != %d)\n", sizeof(NCRYPT_ALGO_ARIA_ARGS), szArgs);
            return -1;
        }

        return Ncrypt_ARIA((NCRYPT_ALGO_ARIA_ARGS *)pstNcryptArgs);
    }
    else if (eNcryptAlgo == NCRYPT_ALGO_DES)
    {
        if (sizeof(NCRYPT_ALGO_DES_ARGS) != szArgs)
        {
            fprintf(stderr, "sizeof(NCRYPT_ALGO_DES_ARGS) != szArgs (%ld != %d)\n", sizeof(NCRYPT_ALGO_DES_ARGS), szArgs);
            return -1;
        }

        return Ncrypt_DES((NCRYPT_ALGO_DES_ARGS *)pstNcryptArgs);
    }
    else if (eNcryptAlgo == NCRYPT_ALGO_AES)
    {
        if (sizeof(NCRYPT_ALGO_AES_ARGS) != szArgs)
        {
            fprintf(stderr, "sizeof(NCRYPT_ALGO_AES_ARGS) != szArgs (%ld != %d)\n", sizeof(NCRYPT_ALGO_AES_ARGS), szArgs);
            return -1;
        }

        return Ncrypt_AES((NCRYPT_ALGO_AES_ARGS *)pstNcryptArgs);
    }
    else if (eNcryptAlgo == NCRYPT_ALGO_BLOWFISH)
    {
        if (sizeof(NCRYPT_ALGO_BLOWFISH_ARGS) != szArgs)
        {
            fprintf(stderr, "sizeof(NCRYPT_ALGO_BLOWFISH_ARGS) != szArgs (%ld != %d)\n", sizeof(NCRYPT_ALGO_BLOWFISH_ARGS), szArgs);
            return -1;
        }

        return Ncrypt_BLOWFISH((NCRYPT_ALGO_BLOWFISH_ARGS *)pstNcryptArgs);
    }
    else if (0)
    {
        // Add new NCODE_TYPE here...
        return -1;
    }
    else
    {
        fprintf(stderr, "Unknown eNcryptAlgo. (eNcryptAlgo:%d)\n", eNcryptAlgo);
        return -1;
    }
}

static void printusage_test()
{
    fprintf(stdout, "\n");
    fprintf(stdout, "> 'ncrypt' Usage : ncrypt [-worktype] [-algorithm] [-mode] [-padding] [key] [string]\n");
    fprintf(stdout, " \n[worktype]\n");
    fprintf(stdout, "  1. -e : Encrypt.\n");
    fprintf(stdout, "  2. -d : Decrypt.\n");
    fprintf(stdout, " \n[algorithm]\n");
    fprintf(stdout, "  1. -seed           : KISA-SEED algorithm.\n");
    fprintf(stdout, "  2. -hight          : KISA-HIGHT algorithm.\n");
    fprintf(stdout, "  3. -aria(12/14/16) : KISA-ARIA(12/14/16round) algorithm.\n");
	fprintf(stdout, "  4. -des            : DES algorithm.\n");
	fprintf(stdout, "  5. -aes            : AES algorithm.\n");
	fprintf(stdout, "  6. -blowfish       : Blowfish algorithm.\n");
    fprintf(stdout, " \n[mode]\n");
    fprintf(stdout, "  1. -ecb : ECB(eletronic code book) mode.\n");
    fprintf(stdout, "  2. -cbc : CBC(cipher block chaning) mode.\n");
    fprintf(stdout, " \n[padding]\n");
    fprintf(stdout, "  1. -null : null padding.\n");
    fprintf(stdout, "  2. -pkcs : pkcs padding.\n");
    fprintf(stdout, "\n");
}

static void copystructure_test(NCRYPT_COMMON_BLOCK_ARGS *pstComm, int nArgs, ...)
{
    int szComm = sizeof(NCRYPT_COMMON_BLOCK_ARGS);
    void *pstArg = NULL;
    va_list ap;

    va_start(ap, nArgs);

    // 1. SEED
    pstArg = (void *)va_arg(ap, void*);
    memcpy(&((NCRYPT_ALGO_SEED_ARGS *)pstArg)->stArgs, pstComm, szComm);

    // 2. HIGHT
    pstArg = (void *)va_arg(ap, void*);
    memcpy(&((NCRYPT_ALGO_HIGHT_ARGS *)pstArg)->stArgs, pstComm, szComm);

    // 3. ARIA
    pstArg = (void *)va_arg(ap, void*);
    memcpy(&((NCRYPT_ALGO_ARIA_ARGS *)pstArg)->stArgs, pstComm, szComm);

    // 4. DES
    pstArg = (void *)va_arg(ap, void*);
    memcpy(&((NCRYPT_ALGO_DES_ARGS *)pstArg)->stArgs, pstComm, szComm);

    // 5. AES
    pstArg = (void *)va_arg(ap, void*);
    memcpy(&((NCRYPT_ALGO_AES_ARGS *)pstArg)->stArgs, pstComm, szComm);

    // 6. BLOWFISH
    pstArg = (void *)va_arg(ap, void*);
    memcpy(&((NCRYPT_ALGO_BLOWFISH_ARGS *)pstArg)->stArgs, pstComm, szComm);

    // Add new algorithm struecture here...
    // ...

    va_end(ap);
}

static int setargs_test(int argc, char **argv, NCRYPT_COMMON_BLOCK_ARGS *pstArgs)
{
	const char *pcIsEncrypt = argv[1];
	const char *pcAlgo = argv[2];
	const char *pcMode = argv[3];
	const char *pcPadd = argv[4];
	const char *pcKey  = argv[5];
	const char *pcStr  = argv[6];
	int rtVal;

    if (argc != 7)
    {
        printusage_test();
        return -1;
    }

    if (pstArgs == NULL)
    {
        fprintf(stderr, "pstArgs == NULL. (pstArgs:0x%p)\n", pstArgs);
		printusage_test();
        return -1;
    }

    if (pcIsEncrypt != NULL)
    {
        if (strcmp(pcIsEncrypt, "-e") == 0)
        {
            pstArgs->isEncrypt = 1;
        }
        else if (strcmp(pcIsEncrypt, "-d") == 0)
        {
            pstArgs->isEncrypt = 0;
        }
        else
        {
            fprintf(stderr, "Unkown worktype '%s'\n", pcIsEncrypt);
			printusage_test();
            return -1;
        }
    }

    if (pcAlgo != NULL)
    {
        if (strcmp(pcAlgo, "-seed") == 0)
        {
            pstArgs->eAlgo = NCRYPT_ALGO_SEED;
        }
        else if (strcmp(pcAlgo, "-hight") == 0)
        {
            pstArgs->eAlgo = NCRYPT_ALGO_HIGHT;
        }
        else if (strncmp(pcAlgo, "-aria", 5) == 0)
        {
            pstArgs->eAlgo = NCRYPT_ALGO_ARIA;
        }
        else if (strncmp(pcAlgo, "-des", 4) == 0)
        {
            pstArgs->eAlgo = NCRYPT_ALGO_DES;
        }
        else if (strncmp(pcAlgo, "-aes", 4) == 0)
        {
            pstArgs->eAlgo = NCRYPT_ALGO_AES;
        }
        else if (strcmp(pcAlgo, "-blowfish") == 0)
        {
            pstArgs->eAlgo = NCRYPT_ALGO_BLOWFISH;
        }
        else
        {
            fprintf(stderr, "Unkown algorithm '%s'\n", pcAlgo);
			printusage_test();
            return -1;
        }
    }

    if (pcMode != NULL)
    {
        if (strcmp(pcMode, "-ecb") == 0)
        {
            pstArgs->eMode = NCRYPT_MODE_ECB;
        }
        else if (strcmp(pcMode, "-cbc") == 0)
        {
            pstArgs->eMode = NCRYPT_MODE_CBC;
        }
        else
        {
            fprintf(stderr, "Unkown mode '%s'\n", pcMode);
			printusage_test();
            return -1;
        }
    }

    if (pcPadd != NULL)
    {
        if (strcmp(pcPadd, "-null") == 0)
        {
            pstArgs->ePadd = NCRYPT_PADD_NULL;
        }
        else if (strcmp(pcPadd, "-pkcs") == 0)
        {
            pstArgs->ePadd = NCRYPT_PADD_PKCS;
        }
        else
        {
            fprintf(stderr, "Unkown padding '%s'\n", pcPadd);
			printusage_test();
            return -1;
        }
    }

    if (pcKey == NULL || strlen(pcKey) < 1)
    {
		fprintf(stderr, "pcKey == NULL || strlen(pcKey) < 1 (pcKey:0x%p, szKey:%d)\n", pcKey, strlen(pcKey));
        printusage_test();
		return -1;
    }

    if (pcStr == NULL || strlen(pcStr) < 1)
    {
		fprintf(stderr, "pcStr == NULL || strlen(pcStr) < 1 (pcStr:0x%p, szStr:%d)\n", pcStr, strlen(pcStr));
        printusage_test();
		return -1;
    }

    return 0;
}

#define COPY_STRUCT() copystructure_test(&stArgs, 6, \
                                         &stSEED_Args, &stHIGHT_Args, &stARIA_Args, \
                                         &stDES_Args, &stAES_Args, &stBLOWFISH_Args)

static int ncryptor_test(int argc, char **argv)
{
    const char *pcIsEncrypt = argv[1];
	const char *pcAlgo      = argv[2];
	const char *pcMode      = argv[3];
	const char *pcPadd      = argv[4];
	const char *pcKey       = argv[5];
	const char *pcStr       = argv[6];

	NCRYPT_COMMON_BLOCK_ARGS    stArgs;
    NCRYPT_ALGO_SEED_ARGS       stSEED_Args;
    NCRYPT_ALGO_HIGHT_ARGS      stHIGHT_Args;
    NCRYPT_ALGO_ARIA_ARGS       stARIA_Args;
    NCRYPT_ALGO_DES_ARGS        stDES_Args;
    NCRYPT_ALGO_AES_ARGS        stAES_Args;
    NCRYPT_ALGO_BLOWFISH_ARGS   stBLOWFISH_Args;
    void                        *pstArgs = NULL;
    int                         szArgs = 0;
	int                         rtVal;

	memset(&stArgs,             0x00, sizeof(NCRYPT_COMMON_BLOCK_ARGS));
    memset(&stSEED_Args,        0x00, sizeof(NCRYPT_ALGO_SEED_ARGS));
    memset(&stHIGHT_Args,       0x00, sizeof(NCRYPT_ALGO_HIGHT_ARGS));
    memset(&stARIA_Args,        0x00, sizeof(NCRYPT_ALGO_ARIA_ARGS));
    memset(&stDES_Args,         0x00, sizeof(NCRYPT_ALGO_DES_ARGS));
    memset(&stAES_Args,         0x00, sizeof(NCRYPT_ALGO_AES_ARGS));
    memset(&stBLOWFISH_Args,    0x00, sizeof(NCRYPT_ALGO_BLOWFISH_ARGS));

    // Check input value, set basic options in 'stArgs'.
	if ((rtVal = setargs_test(argc, argv, &stArgs)) < 0)
	{
		fprintf(stderr, "setargs_test() error. (return:%d)\n", rtVal);
		return -1;
	}

    // Set detailed value in structure that selected from eAlgo value.
    if (stArgs.eAlgo == NCRYPT_ALGO_SEED)
    {
        // SEED
        pstArgs = &stSEED_Args;
        szArgs = sizeof(NCRYPT_ALGO_SEED_ARGS);
    }
    else if (stArgs.eAlgo == NCRYPT_ALGO_HIGHT)
    {
        // HIGHT
        pstArgs = &stHIGHT_Args;
        szArgs = sizeof(NCRYPT_ALGO_HIGHT_ARGS);
    }
    else if (stArgs.eAlgo == NCRYPT_ALGO_ARIA)
    {
        // ARIA12, ARIA14, ARIA16
        if ((strncmp(pcAlgo, "-aria", 5) == 0 && strlen(pcAlgo) == 5) || strncmp(&pcAlgo[5], "12", 2) == 0)
        {
            stARIA_Args.eRounds = NCRYPT_ARIA_ROUND12;   
        }
        else if (strncmp(&pcAlgo[5], "14", 2) == 0)
        {
            stARIA_Args.eRounds = NCRYPT_ARIA_ROUND14;
        }
        else if (strncmp(&pcAlgo[5], "16", 2) == 0)
        {
            stARIA_Args.eRounds = NCRYPT_ARIA_ROUND16;
        }
        else
        {
            fprintf(stderr, "Undefined ARIA Round type.\n");
            printusage_test();
            return -1;
        }

        pstArgs = &stARIA_Args;
        szArgs = sizeof(NCRYPT_ALGO_ARIA_ARGS);
    }
    else if (stArgs.eAlgo == NCRYPT_ALGO_DES)
    {
        // DES, DES3
        if (strlen(pcAlgo) == 4 && strncmp(pcAlgo, "-des", 4) == 0)
        {
            stDES_Args.isTripleDES = 0;
        }
        else if (pcAlgo[4] == '3')
        {
            stDES_Args.isTripleDES = 1;
        }
        else
        {
            fprintf(stderr, "Undefined DES algo. (algo:%s)\n", pcAlgo);
            return -1;
        }

        pstArgs = &stDES_Args;
        szArgs = sizeof(NCRYPT_ALGO_DES_ARGS);
    }
    else if (stArgs.eAlgo == NCRYPT_ALGO_AES)
    {
        // AES128, AES196, AES256
        if ((strncmp(pcAlgo, "-aes", 4) == 0 && strlen(pcAlgo) == 4) || strncmp(&pcAlgo[4], "128", 3) == 0)
        {
            stAES_Args.eKeyBit = NCRYPT_AES_KEYBIT_128;
        }
        else if (strncmp(&pcAlgo[4], "196", 3) == 0)
        {
            stAES_Args.eKeyBit = NCRYPT_AES_KEYBIT_196;
        }
        else if (strncmp(&pcAlgo[4], "256", 3) == 0)
        {
            stAES_Args.eKeyBit = NCRYPT_AES_KEYBIT_256;
        }
        else
        {
            fprintf(stderr, "Undefined AES keybit type.\n");
            printusage_test();
            return -1;
        }

        pstArgs = &stAES_Args;
        szArgs = sizeof(NCRYPT_ALGO_AES_ARGS);
    }
    else if (stArgs.eAlgo == NCRYPT_ALGO_BLOWFISH)
    {
        // BLOWFISH
        pstArgs = &stBLOWFISH_Args;
        szArgs = sizeof(NCRYPT_ALGO_BLOWFISH_ARGS);
    }
    else if (0)
    {
        // Add new algorithm and args structure here...
    }
    else
    {
        fprintf(stderr, "Unknown algorithm id. (eAlgo:%d)\n", stArgs.eAlgo);
        return -1;
    }

    // Set input stream, output buffer, key and IV.
	int  szInSteram = strlen(pcStr);
	int  szInBuf = szInSteram * 2;
	BYTE abInBuf[szInBuf];
	memcpy(abInBuf, pcStr, szInBuf);

	int  szOutBuf = szInBuf;
	BYTE abOutBuf[szOutBuf];
	memset(abOutBuf, 0x00, szOutBuf);

	int  szKeyBuf = strlen(pcKey);
	BYTE abKeyBuf[szKeyBuf];
	memcpy(abKeyBuf, pcKey, szKeyBuf);

	int  szIVBuf = szKeyBuf;
	BYTE abIVBuf[szIVBuf];
	memset(abIVBuf, 0x11, szIVBuf);

    stArgs.pbKey = abKeyBuf;
	stArgs.szKey = szKeyBuf;
	stArgs.pbIV  = abIVBuf;
	stArgs.szIV  = szIVBuf;
    COPY_STRUCT();

    // Print info, Encrypt, Decrypt and Print results.
	// Info //
	fprintf(stdout, "\n================================================================\n");
	fprintf(stdout, "\n [ Info (%s / %s / %s) ] \n", pcAlgo, pcMode, pcPadd);

	fprintf(stdout, "\n- Key[String](%dByte) : \n", szKeyBuf);
	PrintStreamToString(abKeyBuf, szKeyBuf);
	fprintf(stdout, "\n- Key[Hexa](%dByte) : \n", szKeyBuf);
	PrintStreamToHexa(abKeyBuf, szKeyBuf);
	fprintf(stdout, "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

	if (stArgs.eMode != NCRYPT_MODE_ECB)
	{
		fprintf(stdout, "\n- IV[String](%dByte) : \n", szIVBuf);
		PrintStreamToString(abIVBuf, szIVBuf);
		fprintf(stdout, "\n- IV[Hexa](%dByte) : \n", szIVBuf);
		PrintStreamToHexa(abIVBuf, szIVBuf);
		fprintf(stdout, "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	}

	fprintf(stdout, "\n- InBuf[String](%dByte) : \n", szInSteram);
	PrintStreamToString(abInBuf, szInSteram);
	fprintf(stdout, "\n- InBuf[Hexa](%dByte) : \n", szInSteram);
	PrintStreamToHexa(abInBuf, szInSteram);

	// Encryption //
	fprintf(stdout, "\n----------------------------------------------------------------\n");
	fprintf(stdout, "\n [ Encrypting...... ] \n");

	stArgs.isEncrypt = 1;
	stArgs.pbInStream = abInBuf;
	stArgs.szInStream = szInSteram;
	stArgs.pbOutBuf = abOutBuf;
	stArgs.szOutBuf = szOutBuf;
    COPY_STRUCT();

	if ((rtVal = Ncrypt(pstArgs, szArgs)) < 1)
	{
		fprintf(stderr, "Ncrypt(Encryption) error. (return:%d)\n", rtVal);
		return rtVal;
	}

	fprintf(stdout, "\n- OutBuf[String](%dByte) : \n", rtVal);
	PrintStreamToString(abOutBuf, rtVal);
	fprintf(stdout, "\n- OutBuf[Hexa](%dByte) : \n", rtVal);
	PrintStreamToHexa(abOutBuf, rtVal);

	// Decryption //
	fprintf(stdout, "\n----------------------------------------------------------------\n");
	fprintf(stdout, "\n [ Decrypting...... ] \n");

	stArgs.isEncrypt = 0;
	stArgs.pbInStream = abOutBuf;
	stArgs.szInStream = rtVal;
	stArgs.pbOutBuf = abInBuf;
	stArgs.szOutBuf = szOutBuf;
    COPY_STRUCT();

	if ((rtVal = Ncrypt(pstArgs, szArgs)) < 1)
	{
		fprintf(stderr, "Ncrypt(Decryption) error. (return:%d)\n", rtVal);
		return rtVal;
	}

	fprintf(stdout, "\n- OutBuf[String](%dByte) : \n", rtVal);
	PrintStreamToString(abInBuf, rtVal);
	fprintf(stdout, "\n- OutBuf[Hexa](%dByte) : \n", rtVal);
	PrintStreamToHexa(abInBuf, rtVal);

	return 0;
}

//int main(int argc, char **argv)
//{
//  return ncryptor_test(argc, argv);
//}