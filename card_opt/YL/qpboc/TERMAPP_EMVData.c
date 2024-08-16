#include "includes.h"
#include "common/cardlan_StandardBus_tpye.h"
#include "libcardlan_StandardBus_util.h"


#if defined(ANDROID_CODE_DEBUG)
#define TARGET_ANDROID
#endif
#if defined(NDK_CODE_DEBUG)
#define TARGET_DEBUG
#endif


#if defined(TARGET_ANDROID)
#ifndef LOG_TAG
#define LOG_TAG __FILE__
#endif
#include <android/log.h>
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#elif defined(TARGET_DEBUG)
#include <stdio.h>
#define  LOGI    printf
#define  LOGD    printf
#define  LOGE    printf
#else 
#define  LOGI    
#define  LOGD    
#define  LOGE
#endif


struct pos_commstrc pos_com;

uchar bCardConfirmVLP;  //procedure variable,set according to card ret data.
uchar bAdvice;          //procedure variable,needn't save.
uchar bReversal;        //procedure variable,needn't save.

uchar TermAppNum;


TERMAPP AppListTerm[16];
uchar AppBlockFlag[16];//if the app is block in selecting,set the corresponding uchar 1.
uchar bRetFromInitApp;


APPDATA AppListCandidate[16];

//total app numbers in candidate app list.
uchar AppNum;
//app list sequence after rearranged in final choose.
uchar AppSeq[16];
//corresponding priority of app list after rearranged.
uchar AppPriority[16];

uchar SelectedAppNo;
APPDATA SelectedApp;


DFData DFListIcc[16];
uchar DFNum;

//if there is one mutual support app,the flag indicate if terminal can provide for confirmation by the cardholder
//0: not provide;1: provide
//extern uchar bProvideConfirm;

uchar  MerchantPreferApp;

//.......AppSel
//.......TERMAPP_DataAuth.c
unsigned short DDOLDataLen;
uchar DDOLData[256]; //use for dynamicly store data when processing DDOL,must be initialised in trans init.

uchar bCombineDDA;        //to indicate if Combined DDA/AC is to be performed. 0-not perform,1-to be performed.
uchar bCombineDDAFail;      //to indicate if Combined DDA/AC is fail. 0-not fail,1-fail.

uchar bDDOLHasNoUnpredictNum;
uchar bCDOL1HasNoUnpredictNum;
uchar bCDOL2HasNoUnpredictNum;

uchar CAPK_NUM=0;
CAPK_STRUCT CAPK;

OTHER_ICPARAMETER_STRUCT OtherICPara    ;

uint IPK_REVOKENUM;

uchar ICCPKModul[248];
uchar ICCPKModulLen;//NICC
uchar IPKModul[248];
uchar IPKModulLen;//NI

uchar ICCPIN_EPKModul[248];
uchar ICCPIN_EPKModulLen;//NPE

CVR CVRList[128];
uchar CVRListLen;
unsigned long CVM_X,CVM_Y;
uchar bPrintReceipt;

uchar EncryptPINLen;
uchar EncryptPIN[32];

uchar IssuAuthenDataLen;
uchar IssuAuthenData[16];

uchar IssuScriptLen;
uchar IssuScript[256];
uchar IssuScript71Len;
uchar IssuScript71[256];
uchar IssuScript72Len;
uchar IssuScript72[256];
uchar IssuScriptNum;
uchar IssuScriptResult[32][5];
uchar CurrentScriptID[4];//initialised with all zeros.

uchar TransResult;//initial:0
uchar bAbleOnline;//initial value: 1

unsigned int GenerateACRetDataLen;
uchar GenerateACRetData[128];//Returned data in Generate AC except for SignDynAppData, used for Combined DDA/AC

unsigned short CDOL1DataLen;
uchar CDOL1Data[256];

unsigned short CDOL2DataLen;
uchar CDOL2Data[256];

uchar bTransCapture;//0-trans captured when CID=0x08;1-trans not capture when CID==0x08
//initial value: 0,only set to 1 when testing 2CM.040.00.

//........Analyse.c

//.........ReadData.c
uchar TransTargetPercent,RandNum;
uchar bRandSelected;
uchar bShowRandNum;
//set to 1 for displaying rand num in random trans selection in some EMV test cases,
//other cases needn't display and is set to 0.(default value is 0) 

//.........ReadData.c
//Data types structure
uchar AuthData[1024];//Static data to be authenticated,move it to file in March 4,2002
unsigned int AuthDataLen;
uchar bErrSDATL;//to indicate whether there is error in process SDA tag list. 0-ok,1-error.
uchar bErrAuthData;//to indicate whether there is error in process static authentication data. 0-ok,1-error.             

uchar AFL[16][4];            // Application File Locator
uchar AFL_Num;
unsigned int  PDOLDataLen;
uchar PDOLData[255];

CARDINFO CardInfo;

unsigned short    TransLogMaxNum; //max transLogs stored for check floor limit(default 20)
long    Threshold;                //threshold for random selection.
long    FloorLimit;               //tag'9F1B' terminal floor long    AmtTrans;               //used in online financial or batch capture msglong    AmtNet;                 //total accumulative amount for reconciliation.
unsigned short BatchTransNum;     //number of trans stored in terminal.used for reconciliation
unsigned short TransNum;          //numbers of floorlimit translog for floorlimit check.
unsigned short TransIndex;        //added for new floorlimit translog insert position.
long    AmtAuthBin;               //tag'81' Authorised amount of binary
long    AmtOtherBin;              //tag'9F04' Other(cashback) amount of binary
long    AmtReferCurcy;            //tag'9F3A' Authorised amount in the reference currency

TERMINFO TermInfo;

ISSUER_DATA AidListTerm[16];


/*
    ICCdataTable:关于银行卡所有tag的信息索引
    具体的数据是存放在Terminfo中
*/
 DATAELEMENT ICCDataTable[]={ //data with source from ICC
    {/*"AppCrypt",0,*/"\x9F\x26",255,8,0,0,0,0},/* 0 */
    {/*"AppCurcyCode",0,*/"\x9F\x42",255,2,1,8,0,0},
    {/*"AppCurcyExp",0,*/"\x9F\x44",255,1,1,10,0,0},
    {/*"AppDiscretData",0,*/"\x9F\x05",1,32,0,12,0,0},
    {/*"AppEffectDate",0,*/"\x5F\x25",255,3,1,44,0,0},
    {/*"AppExpireDate",0,*/"\x5F\x24",255,3,1,47,0,0},//mandatory /* 5 */
    {/*"AFL",0,*/"\x94\x00",0,252,0,51,1,0},
    {/*"AID",0,*/"\x4F\x00",5,16,0,304,0,0},
    {/*"AIP",0,*/"\x82\x00",255,2,0,320,0,0},
    {/*"AppLabel",0,*/"\x50\x00",1,16,0,323,0,0},

    {/*"AppPreferName",0,*/"\x9F\x12",1,16,0,340,0,0},/* 10 */
    {/*"PAN",0,*/"\x5A\x00",0,10,2,357,1,0},//mandatory
    {/*"PANSeqNum",0,*/"\x5F\x34",255,1,1,367,0,0},
    {/*"AppPriority",0,*/"\x87\x00",255,1,0,368,0,0},
    {/*"AppReferCurcy",0,*/"\x9F\x3B",2,8,1,370,0,0},
    {/*"AppReferCurcyExp",0,*/"\x9F\x43",1,4,1,379,0,0},/* 15 */
    {/*"ATC",0,*/"\x9F\x36",255,2,0,383,0,0},
    {/*"AUC",0,*/"\x9F\x07",255,2,0,385,0,0},
    {/*"AppVerNum",0,*/"\x9F\x08",255,2,0,387,0,0},
    {/*"CDOL1",0,*/"\x8C\x00",0,252,0,390,0,0},//mandatory

    {/*"CDOL2",0,*/"\x8D\x00",0,252,0,643,0,0},//mandatory/* 20 */
    {/*"CardholderName",0,*/"\x5F\x20",2,26,0,896,0,0},
    {/*"CardholderNameExt",0,*/"\x9F\x0B",27,45,0,923,0,0},
    {/*"CVMList",0,*/"\x8E\x00",0,252,0,969,0,0},
    {/*"CAPKI",0,*/"\x8F\x00",255,1,0,1221,6,0},//mandatory,SDA,DDA
    {/*"CryptInfoData",0,*/"\x9F\x27",255,1,0,1222,0,0},/* 25 */
    {/*"DataAuthenCode",0,*/"\x9F\x45",255,2,0,1223,0,0},
    {/*"DFName",0,*/"\x84\x00",5,16,0,1226,0,0},
    {/*"DDFName",0,*/"\x9D\x00",5,16,0,1243,0,0},
    {/*"DDOL",0,*/"\x9F\x49",0,252,0,1260,0,0},//mandatory,DDA,clear in P70 and not joining IccDataMissing check.

    {/*"ICCDynNum",0,*/"\x9F\x4C",2,8,0,1513,0,0},/* 30 */
    {/*"ICCPIN_EPKCert",0,*/"\x9F\x2D",0,248,0,1522,0,0},//NI
    {/*"ICCPIN_EPKExp",0,*/"\x9F\x2E",1,3,0,1771,0,0},
    {/*"ICCPIN_EPKRem",0,*/"\x9F\x2F",0,42,0,1775,0,0},//NPE-NI+42
    {/*"ICCPKCert",0,*/"\x9F\x46",0,248,0,1818,4,0},//NI, mandatory,DDA
    {/*"ICCPKExp",0,*/"\x9F\x47",1,3,0,2067,4,0},//mandatory,DDA /* 35 */
    {/*"ICCPKRem",0,*/"\x9F\x48",0,42,0,2071,0,0},//NIC-NI+42. mandatory,DDA,judge in running
    {/*"IACDenial",0,*/"\x9F\x0E",255,5,0,2113,0,0},
    {/*"IACOnline",0,*/"\x9F\x0F",255,5,0,2118,0,0},
    {/*"IACDefault",0,*/"\x9F\x0D",255,5,0,2123,0,0},

    {/*"IssuAppData",0,*/"\x9F\x10",0,32,0,2129,0,0},/* 40 */
    {/*"IssuCodeTableIndex",0,*/"\x9F\x11",255,1,1,2161,0,0},
    {/*"IssuCountryCode",0,*/"\x5F\x28",255,2,1,2162,0,0},
    {/*"IPKCert",0,*/"\x90\x00",0,248,0,2165,6,0},//NCA. mandatory,SDA,DDA
    {/*"IPKExp",0,*/"\x9F\x32",1,3,0,2414,6,0}, //mandatory,SDA,DDA
    {/*"IPKRem",0,*/"\x92\x00",0,36,0,2418,0,0},//NI-NCA+36. mandatory,SDA,DDA,judge in running
    {/*"LangPrefer",0,*/"\x5F\x2D",2,8,0,2455,0,0},
    {/*"LOATCReg",0,*/"\x9F\x13",255,2,0,2463,0,0},
    {/*"LCOL",0,*/"\x9F\x14",255,1,0,2465,0,0},
    {/*"PINTryCount",0,*/"\x9F\x17",255,1,0,2466,0,0},

    {/*"PDOL",0,*/"\x9F\x38",0,252,0,2468,0,0}, /* 50 */
    {/*"ServiceCode",0,*/"\x5F\x30",255,2,1,2720,0,0},
    {/*"SignDynAppData",0,*/"\x9F\x4B",0,248,0,2723,0,0},//NIC
    {/*"SignStatAppData",0,*/"\x93\x00",0,248,0,2972,2,0},//NI. mandatory,SDA
    {/*"SDATagList",0,*/"\x9F\x4A",0,128,0,3221,0,0},
    {/*"Track1Discret",0,*/"\x9F\x1F",0,128,0,3350,0,0},/* 55 */
    {/*"Track2Discret",0,*/"\x9F\x20",0,128,2,3479,0,0},
    {/*"Track2Equivalent",0,*/"\x57\x00",0,19,0,3608,0,0},
    {/*"TDOL",0,*/"\x97\x00",0,252,0,3628,0,0},
    {/*"UCOL",0,*/"\x9F\x23",255,1,0,3880,0,0},

    {/*"IssuerURL",0,*/"\x5F\x50",0,128,0,3882,0,0},/* 60 */
    {/*"VLPAvailableFund",0,*/"\x9F\x79",255,6,1,4010,0,0},
    {/*"VLPIssuAuthorCode",0,*/"\x9F\x74",255,6,0,4016,0,0},
    {/*"PersonID",0,*/"\x9F\x61",1,40,0,4023,0,0},
    {/*"PersonIDType",0,*/"\x9F\x62",255,1,0,4063,0,0},
    {/*"OfflineAmount",0,*/"\x9F\x5D",255,6,2,4064,0,0},
    {/*"CardTradePro",0,*/ "\x9F\x6C",255,2,1,4070,0,0}
  };

 DATAELEMENT TermDataTable[]={ //data with source from Terminal
    {/*"AcquireID",1,*/"\x9F\x01",255,6,1,0,0,0},/* 0 */
    {/*"TermCapab",1,*/"\x9F\x33",255,3,0,6,0,0},
    {/*"TermAddCapab",1,*/"\x9F\x40",255,5,0,9,0,0},
    {/*"IFDSerNum",1,*/"\x9F\x1E",255,8,0,14,0,0},
    {/*"TermID",1,*/"\x9F\x1C",255,8,0,22,0,0},
    {/*"MerchCateCode",1,*/"\x9F\x15",255,2,1,30,0,0},/* 5 */
    {/*"MerchID",1,*/"\x9F\x16",255,15,0,32,0,0},
    {/*"TermCountryCode",1,*/"\x9F\x1A",255,2,1,47,0,0},
    {/*"TRMData",1,*/"\x9F\x1D",1,8,0,50,0,0},
    {/*"TermType",1,*/"\x9F\x35",255,1,1,58,0,0},

    {/*"AppVerNum",1,*/"\x9F\x09",255,2,0,59,0,0},/* 10 */
    {/*"TransCurrencyCode",1,*/"\x5F\x2A",255,2,1,61,0,0},
    {/*"TransCurrencyExp",1,*/"\x5F\x36",255,1,1,63,0,0},
    {/*"TransReferCurrencyCode",1,*/"\x9F\x3C",255,2,1,64,0,0},
    {/*"TransReferCurrencyExp",1,*/"\x9F\x3D",255,1,1,66,0,0},
    {/*"TACDenial",1,*/"\x00\x00",255,5,0,67,0,0},/* 15 */
    {/*"TACOnline",1,*/"\x00\x00",255,5,0,72,0,0},
    {/*"TACDefault",1,*/"\x00\x00",255,5,0,77,0,0},
    {/*"TransType",1,*/"\x00\x00",255,1,0,82,0,0},
    {/*"TransTypeValue",1,*/"\x9C\x00",255,1,1,83,0,0},

    {/*"VLPTransLimit",1,*/"\x9F\x7B",255,6,1,84,0,0},/* 20 */
    {/*"VLPTACDenial",1,*/"\x00\x00",255,5,0,90,0,0},
    {/*"VLPTACOnline",1,*/"\x00\x00",255,5,0,95,0,0},
    {/*"VLPTACDefault",1,*/"\x00\x00",255,5,0,100,0,0},
    {/*"Language",1,*/"\x00\x00",255,1,0,105,0,0},
    {/*"bTermDDOL",1,*/"\x00\x00",255,1,0,106,0,0},/* 25 */
    {/*"bForceAccept",1,*/"\x00\x00",255,1,0,107,0,0},
    {/*"bForceOnline",1,*/"\x00\x00",255,1,0,108,0,0},
    {/*"bBatchCapture",1,*/"\x00\x00",255,1,0,109,0,0},
    {/*"bTermSupportVLP",1,*/"\x00\x00",255,1,0,110,0,0},

    {/*"MaxTargetPercent",1,*/"\x00\x00",255,1,0,111,0,0},/* 30 */
    {/*"TargetPercent",1,*/"\x00\x00",255,1,0,112,0,0},
    {/*"TermDDOL",1,*/"\x00\x00",0,128,0,114,0,0},
    {/*"TermTDOL",1,*/"\x00\x00",0,128,0,243,0,0},
    {/*"MerchNameLocate",1,*/"\x00\x00",0,128,0,372,0,0},
    {/*"TransLogMaxNum",1,*/"\x00\x00",255,2,0,500,0,0},/* 35 */
    {/*"Threshold",1,*/"\x00\x00",255,4,1,502,0,0},
    {/*"FloorLimit",1,*/"\x9F\x1B",255,4,1,506,0,0},
    {/*"AmtTrans",1,*/"\x00\x00",255,4,1,510,0,0},
    {/*"AmtNet",1,*/"\x00\x00",255,4,1,514,0,0},

    {/*"BatchTransNum",1,*/"\x00\x00",255,2,1,518,0,0},/* 40 */
    {/*"TransNum",1,*/"\x00\x00",255,2,1,520,0,0},
    {/*"TransIndex",1,*/"\x00\x00",255,2,1,522,0,0},
    {/*"TransSeqCount",1,*/"\x9F\x41",255,4,1,524,0,0},
    {/*"AmtAuthorBin",1,*/"\x81\x00",255,4,1,528,0,0},
    {/*"AmtAuthorNum",1,*/"\x9F\x02",255,6,1,532,0,0},/* 45 */
    {/*"AmtOtherBin",1,*/"\x9F\x04",255,4,1,538,0,0},
    {/*"AmtOtherNum",1,*/"\x9F\x03",255,6,1,542,0,0},
    {/*"AmtReferCurrency",1,*/"\x9F\x3A",255,4,1,548,0,0},
    {/*"AID",1,*/"\x9F\x06",5,16,0,553,0,0},

    {/*"AuthorCode",1,*/"\x89\x00",255,6,0,569,0,0},/* 50 */
    {/*"AuthorRespCode",1,*/"\x8A\x00",255,2,0,575,0,0},
    {/*"CVR",1,*/"\x9F\x34",255,3,0,577,0,0},
    {/*"POSEntryMode",1,*/"\x9F\x39",255,1,1,580,0,0},
    {/*"PIN",1,*/"\x99\x00",0,12,2,582,0,0},
    {/*"TVR",1,*/"\x95\x00",255,5,0,594,0,0},/* 55 */
    {/*"TSI",1,*/"\x9B\x00",255,2,0,599,0,0},
    {/*"VLPIndicator",1,*/"\x9F\x7A",255,1,1,601,0,0},
    {/*"TransDate",1,*/"\x9A\x00",255,3,1,602,0,0},
    {/*"TransTime",1,*/"\x9F\x21",255,3,1,605,0,0},

    {/*"TCHashValue",1,*/"\x98\x00",255,20,0,608,0,0},/* 60 */
    {/*"UnpredictNum",1,*/"\x9F\x37",255,4,0,628,0,0},
    {/*"IssuerAuthenData",1,*/"\x91\x00",8,16,0,633,0,0},
    {/*"MCHIPTransCategoryCode",1,*/"\x9F\x53",255,1,0,649,0,0},
    {/*"TermTransProp"*/"\x9F\x66",255,4,0,653,1,0},
    {/*"MerchName"*/"\x9F\x4E",255,20,0,657,0,0},
    {/*Extended application support*/"\xDF\x60",255,1,1,677,0,0}
  };

const  uchar transTypeValue[]={0x01,0x00,0x00,0x09};
  
const  TERMAPP AppListStored[]={
    {0,7,"\xA0\x00\x00\x00\x03\x10\x10",0,0,},//Visa - VSDC,also used in EMV test.
    {0,7,"\xA0\x00\x00\x00\x99\x90\x90",0,0,},//EMV - test
    //{0,7,"\xA0\x00\x00\x00\x03\x20\x10",0,0,},//Visa - Electron
    
    //{0,7,"\xA0\x00\x00\x00\x04\x10\x10",0,0,},//Mastercard - M/Chip
    //{0,7,"\xA0\x00\x00\x00\x04\x30\x60",0,0,},//Mastercard - Maestro
    //{0,7,"\xA0\x00\x00\x00\x04\x60\x00",0,0,},//Mastercard - Cirrus
    //{0,7,"\xA0\x00\x00\x00\x04\x60\x10",0,0,},//Mastercard - app
    //{0,7,"\xA0\x00\x00\x00\x10\x10\x30",0,0,},//Mastercard - eurocheque

    //{0,7,"\xF1\x23\x45\x67\x89\x01\x23",0,0,},//JCB - test app.
    //{0,7,"\xA0\x00\x00\x00\x65\x10\x10",0,0,},//JCB - J/Smart AID.
    
    //next three AIDs are used for Korean Moneta cards.
    //{0,7,"\xA0\x00\x00\x00\x03\x10\x10",1,4,"脚侩"},
    //{0,9,"\xA0\x00\x00\x00\x04\x60\x10\x84\x02",1,8,"厚磊某矫"},
    //{0,12,"SKT_MEMBER01",1,6,"糕滚奖"}
  };

const IPK_REVOKE IPKRevokeListStored[]={
    {"\xA0\x00\x00\x00\x03",0x96,"\x12\x34\x56"}
  };//This is set according to EMV96 test script V2cc023.00

const EXCEPTION_PAN ExceptionFileStored[]={
    {"\x47\x61\x73\x90\x21\x21\x00\x12\xFF\xFF",1}
  };

const   ISSUER_TAC Visa132TAC={ "\x00\x10\x00\x00\x00","\xD8\x40\x00\xF8\x00","\xD8\x40\x00\xA8\x00"};
const   ISSUER_TAC Visa140TAC={ "\x00\x10\x00\x00\x00","\xD8\x40\x04\xF8\x00","\xD8\x40\x00\xA8\x00"};
const   ISSUER_TAC VisaVLPTAC={ "\x7C\x70\xB8\x08\x00","\x00\x00\x00\x00\x00","\x7C\x70\xB8\x08\x00"};
        
const   ISSUER_TAC Jcb12L2TAC= {"\x48\x10\x80\x00\x00","\xB0\x60\x2C\xF8\x00","\xB0\x60\x2C\xA8\x00"};
const   ISSUER_TAC Jcb12L3TAC= {"\x00\x10\x00\x00\x00","\xF8\x60\xAC\xF8\x00","\xF8\x60\xAC\xA8\x00"};
const   ISSUER_TAC Jcb20L2TAC= {"\x4C\x10\x80\x00\x00","\xB0\x60\x2C\xF8\x00","\xB0\x60\x2C\xA8\x00"};
const   ISSUER_TAC Jcb20L3TAC= {"\x00\x10\x00\x00\x00","\xFC\x60\xAC\xF8\x00","\xFC\x60\xAC\xA8\x00"};

const   ISSUER_TAC MasterUOBTAC={ "\x00\x00\x00\x00\x00","\xF8\x50\xAC\xF8\x00","\xF8\x50\xAC\xA0\x00"};

TERMINFOEX TermInfoEx;

// 中国银联银行卡联网联合技术规范V2.1
// 第2 部分 报文接口规范
// 2012-11-02
// 域55 基于PBOC 借贷记标准的IC 卡数据域
// Integrated Circuit Card（ICC）System Related Data
// 具体的数据是存放在Terminfo中

DATAELEMENT ICCSystemRelatedData[] =
{
    {"\x57\x00",255,19,0,0,0,0}, /* 0 */
    {"\x5F\x24",255,3,0,0,0,0},
    {"\x9F\x26",255,8,0,0,0,0},
    {"\x9F\x27",255,1,0,0,0,0},
    {"\x9F\x10",255,32,0,0,0,0},
    {"\x9F\x37",255,4,0,0,0,0},
    {"\x9F\x36",255,2,0,0,0,0},
    {"\x95\x00",255,5,0,0,0,0},
    {"\x9A\x00",255,3,0,0,0,0},
    {"\x9C\x00",255,1,0,0,0,0},
    {"\x9F\x02",255,6,0,0,0,0}, /* 10 */
    {"\x5F\x2A",255,2,0,0,0,0},
    {"\x82\x00",255,2,0,0,0,0}, 
    {"\x9F\x1A",255,2,0,0,0,0},
    {"\x9F\x03",255,6,0,0,0,0},
    {"\x9F\x33",255,3,0,0,0,0},
    
    {"\x9F\x34",255,3,0,0,0,0}, /* 16 */
    {"\x9F\x35",255,1,0,0,0,0},
    {"\x9F\x1E",255,8,0,0,0,0},
    {"\x84\x00",255,16,0,0,0,0},
    {"\x9F\x09",255,2,0,0,0,0}, /* 20 */
    {"\x9F\x41",255,4,0,0,0,0},
    //{"\x91\x00",255,16,0,0,0,0},
    //{"\x71\x00",255,128,0,0,0,0},
    //{"\x72\x00",255,128,0,0,0,0},
    //{"\xDF\x31",255,21,0,0,0,0},
    {"\x9F\x74",255,6,0,0,0,0},
    {"\x9F\x63",255,16,0,0,0,0},
    {"\x8A\x00",255,2,0,0,0,0}, /* 24 */
    {"\x5F\x34",255,1,0,0,0,0},
    {"\x5A\x00",255,10,0,0,0,0}
};

/*
    从EMV数据中获取中国银联55域相关tag信息
    ICCSystemRelatedData是只和55域相关的数据信息索引
    
*/
uchar TERMAPP_PackEMVData(uchar *buffer, uchar *DataLen)
{
    uchar DOLLen = 0, DOLDataLen = 0;
    uchar DOL[256];
    uchar i, index;

    LOGI("TERMAPP_PackEMVData() is called.\n");

    *DataLen = 0;    
    memset(buffer, 0, MAX_EMV_RECORD_LENGTH);
    memset(DOL, 0, 256);

    index = 0;
    for (i = 0; i < ICCSystemRelatedDataNum; i++)
    {
        if ((ICCSystemRelatedData[i].Tag[0] & 0x1F) == 0x1F)
        {
            memcpy(&DOL[index], ICCSystemRelatedData[i].Tag, 2);
            index += 2;
        }
        else
        {
            DOL[index] = ICCSystemRelatedData[i].Tag[0];
            index++;
        }
        DOL[index] = ICCSystemRelatedData[i].Len2;
        index++;    
    }
    DOLLen = index;

    LOGI(" in func %s:55域相关DOL 信息 : \n",__func__);
    menu_print(DOL,index);
    
    TERMAPP_EncodeTLV(DOL, DOLLen, buffer, &DOLDataLen);
    *DataLen = DOLDataLen;

    return OK;
}

