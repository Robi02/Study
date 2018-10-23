#ifndef __CLIRECORD_H__
#define __CLIRECORD_H__

#include "stdheader.h"

#pragma pack(1)

/*
 * Record Attribute
 */
typedef struct _Attribute {
	char *pAttrKey;
	char *pAttrName;
	char *pAttrType;
	int   beginIdx;
	int   szAttr;
	char *pDefaultVal;
} Attribute;

/* 
 * Client Record
 */
typedef struct _CliHeadRecord {
	char idCode[1];
	char taskComp[2];
	char bankCode[3];
	char companyCode[8];
	char comissioningDate[6];
	char processingDate[6];
	char motherAccountNum[14];
	char transferType[2];
	char companyNum[6];
	char resultNotifyType[1];
	char transferCnt[1];
	char password[8];
	char blank[19];
	char format[1];
	char van[2];
	char newLine[2];
} CliHeadRecord;

typedef struct _CliDataRecord {
	char idCode[1];
	char dataSerialNum[6];
	char bankCode[3];
	char accountNum[14];
	char requestTransferPrice[11];
	char realTransferPrice[11];
	char recieverIdNum[13];
	char processingResult[1];
	char disableCode[4];
	char briefs[12];
	char blank[4];
	char newLine[2];
} CliDataRecord;

typedef struct _CliTailRecord {
	char idCode[1];
	char totalRequestCnt[7];	
	char totalRequestPrice[13];
	char normalProcessingCnt[7];	
	char normalProcessingPrice[13];
	char disableProcessingCnt[7];
	char disableProcessingPrice[13];
	char recoveryCode[8];
	char blank[11];
	char newLine[2];
} CliTailRecord;

typedef struct _CliRecord {
	CliHeadRecord stHead;
	CliDataRecord stData;
	CliTailRecord stTail;
} CliRecord;

/*
 * Server Record
 */
typedef struct _SvrHeadRecord { /* Head - 100bytes */
	char idCode[9];
    char companyCode[8];
    char bankCode2[2];
    char msgCode[4];
    char workTypeCode[3];
    char transferCnt[1];
    char msgNum[6];
    char transferDate[8];
    char transferTime[6];
    char responseCode[4];
    char bankResponseCode[4];
    char lookupDate[8];
    char lookupNum[6];
    char bankMsgNum[15];
    char bankCode3[3];
    char spare[13];
} SvrHeadRecord;

typedef struct _SvrDataRecord { /* Data - 200bytes */
	char withdrawalAccountNum[15];
    char bankBookPassword[8];
    char recoveryCode[6];
    char withdrawalAmount[13];
    char afterWithdrawalBalanceSign[1];
    char afterWithdrawalBalance[13];
    char depositBankCode2[2];
    char depositAccountNum[15];
    char fees[9];
    char transferTime[6];
    char depositAccountBriefs[20];
    char cmsCode[16];
    char identificationNum[13];
    char autoTransferClassification[2];
    char withdrawalAccountBriefs[20];
    char depositBankCode3[3];
    char salaryClassification[1];
	char spare[37];
} SvrDataRecord;

typedef struct _SvrRecord { /* Server Record - 300bytes */
	SvrHeadRecord stHead;
	SvrDataRecord stData;
} SvrRecord;

#pragma unpack

/*
 * record.h Global variables
 */
extern Attribute g_astCliHeadAttrs[];
extern Attribute g_astCliDataAttrs[];
extern Attribute g_astCliTailAttrs[];
extern Attribute g_astSvrHeadAttrs[];
extern Attribute g_astSvrDataAttrs[];

extern int g_arAttrSize[];
extern int g_arAttrLen[];

/*
 * Common Record Func
 */
void initRecord(void *pRecord, int szRecord);
int makeRecord(char *pData, int szData, void *pstOutRecord, int szRecord);
Attribute* getAttrFromAttrKey(char *pAttrKey, int *outAttrId, Attribute astAttributes[]);
Attribute* getDataFromAttrKey(void *pRecord, char *pAttrKey, char *pOutData, Attribute astAttributes[]);
void setDataByAttrKey(void *pRecord, char *pAttrKey, char *pData, Attribute astAttributes[]);

/*
 * Client Record Func
 */
int cvtCli2SvrRecord(CliRecord *pstCliRecord, SvrRecord *pstOutSvrRecord);
void printCliRecord(CliRecord *pstCliRecord);

/*
 * Server Record Func
 */
int cvtSvr2CliRecord(SvrRecord *pstSvrRecord, CliRecord *pstOutCliRecord, int option);
void printSvrRecord(SvrRecord *pstSvrRecord);

#endif