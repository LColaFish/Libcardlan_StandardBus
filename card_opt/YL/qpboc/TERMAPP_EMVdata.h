#ifndef __TERMAPP_EMVDATA_H__
#define __TERMAPP_EMVDATA_H__


#define  JY_OK            0    //交易成功
#define  JY_REFUSE        2    //交易拒绝
#define  JY_END           3    //交易终止
#define  JY_STOPSWIPE     4    //不允许刷卡

#define  QPBOC_PARA_HEAD             0xBA    //参数头
#define  QPBOC_PARA_LISTLEN          24    //IC卡其他参数列表单个长度（包括头,序号，补位，校验位）
#define  QPBOC_PARA_LISTVALUELEN     19    //IC卡其他参数列表单个具体值长度（不包括头,序号，补位，校验位）
#define     QPBOC_PARA_VALUELEN      74    //IC卡其他参数单个具体值长度
#define     QPBOC_CAPK_LISTLEN       28    //CAPK参数列表长度
#define     QPBOC_CAPK_VALUELEN      294    //CAPK参数单个具体值长度


#define  CHINESE          50

extern struct pos_commstrc pos_com;

//return status
#define FALSE        0
#define TRUE         1


#define err                 1
#define OK                  0
#define NOK                 0xff
#define QPBOC_err           1
#define QPBOC_OK            0
#define QPBOC_STOP          2

#define APPSEL_PSEFOUND     2
#define APPSEL_PSENOTFOUND  3
#define RESELECT            4
#define OK_AddOfflineAdvice 5

#define err_NoBatchData     21
#define err_ScriptLen       22
#define err_ScriptReturn    23
#define err_ScriptFormat    24
#define err_TermDataMissing 25
#define err_ComOpen         26
#define err_ComSend         27
#define err_ComRecv         28
#define err_MsgData         29
#define err_IssuerAuthen    30
#define err_FileSys         31
#define err_SendAuthRQ      32
#define err_SendReconci     33

#define err_IccDataFormat   34
#define err_IccCommand      35
#define err_IccReturn       36
#define err_NoAppSel        37
#define err_IccDataMissing  38
#define err_IccDataRedund   39
#define err_AppReselect     40
#define err_GetChallenge    41
#define err_CVRFail         42
#define err_CVRNoSupport    43
#define err_InterAuth       44
#define err_InputCancel     45
#define err_PSENoMatchApp   46
#define err_ListNoMatchApp  47
#define err_NotAccept       48
#define err_EmvDataFormat   49
#define err_CancelTrans     50
#define err_CardBlock       51

#define err_DownCAPK        52
 



//transaction type
#define CASH        0      
#define GOODS       1
#define SERVICE     2
#define CASHBACK    3

#define SCR_NORMAL   0
#define SCR_INVERSE  1


#define ICCDataNum   67
#define TermDataNum  67
#define ICCSystemRelatedDataNum  27

#define MAX_MAINMENU_ITEMS    4
#define MAX_SETUPMENU_ITEMS   7

#define TransCard    0
// 1 user slot;1 merchant slot;4 SAM slots.
#define MAXSLOT    6

#define INSERTED_ICCARD         0x50
#define SWIPED_MAGCARD          0x51
#define PRESSED_KEY             0x52

#define MAG_LAST_FAIL_IC        0x92

//set according to ICCTerminal.pdf of Visa1.3.2

//#define bTermSupportVLP     1
//terminal support VLP transaction,the variable is moved to TermInfo structure.

#if 1
extern uchar bCardConfirmVLP;  //procedure variable,set according to card ret data.
extern uchar bAdvice;          //procedure variable,needn't save.
extern uchar bReversal;        //procedure variable,needn't save.
#endif

//.......AppSel.c
#define PARTIAL_MATCH  0
#define EXACT_MATCH    1

#define MAX_EMV_RECORD_LENGTH     255

#if 1
extern uchar TermAppNum;
#endif

typedef struct {
    uchar ASI;  //Application Selection Indicator.0-needn't match exactly(partial match up to the length);1-match exactly
    uchar AIDLen;
    uchar AID[16];//5-16
    uchar bLocalName;//If display app list using local language considerless of info in card.0-use card info;1-use local language.
    uchar AppLocalNameLen;
    uchar AppLocalName[16];
}TERMAPP;

#if 1
extern TERMAPP AppListTerm[16];
extern uchar AppBlockFlag[16];//if the app is block in selecting,set the corresponding uchar 1.
extern uchar bRetFromInitApp;
#endif

//if return from InitialApp(),set this uchar 1. 
//Then select again from candidate list. If only one app left,even bit8 of App Priority Indicatior is 0,still need confirmation.see EMV2000 book1,p98.

typedef struct {
    uchar AIDLen;
    uchar AID[16];//5-16
    uchar AppLabelLen;
    uchar AppLabel[16];
    uchar PreferNameLen;
    uchar PreferName[16];
    uchar Priority; //tag'87'
    uchar LangPreferLen;
    uchar LangPrefer[8];//2-8
    uchar ICTI; //Issuer Code Table Index.lang used for display app list according to ISO8859.but not include Chinese,Korea,etc.
    uchar PDOLLen;
    uchar PDOL[255];
    uchar DirDiscretLen;
    uchar DirDiscret[200];
    uchar IssuerDiscretLen;
    uchar IssuerDiscret[255];
    uchar bLocalName;//If display app list using local language considerless of info in card.0-use card info;1-use local language.
    uchar AppLocalNameLen;
    uchar AppLocalName[16];
}APPDATA;

#if 1
extern APPDATA AppListCandidate[16];

//total app numbers in candidate app list.
extern uchar AppNum;
//app list sequence after rearranged in final choose.
extern uchar AppSeq[16];
//corresponding priority of app list after rearranged.
extern uchar AppPriority[16];

extern uchar SelectedAppNo;
extern APPDATA SelectedApp;
#endif

typedef struct {
        uchar Type; //0:PSE,1:DDF,2:ADF
        uchar DFName[16]; //5-16
        uchar DFNameLen;
        uchar SFI; //1u8
        uchar LangPrefer[8];//2-8
        uchar LangPreferLen;
        uchar ICTI; //Issuer Code Table Index
        uchar IssuerDiscret[64];
        uchar IssuerDiscretLen;
        uchar InsertAppNo;
        uchar DirDiscret[64];
        uchar DirDiscretLen;
    }DFData;
#if 1
extern DFData DFListIcc[16];
extern uchar DFNum;
#endif

//if there is one mutual support app,the flag indicate if terminal can provide for confirmation by the cardholder
//0: not provide;1: provide
//extern uchar bProvideConfirm;
#define bProvideConfirm  1

#if 1
extern uchar  MerchantPreferApp;
#endif

typedef struct {
    uchar Type; //0:PSE,1:DDF,2:ADF
    uchar DFNameExist;//0-non exist;1-exist.
    uchar DFNameLen;
    uchar DFName[16]; //5-16
    uchar FCIPropExist;
    uchar SFIExist;
    uchar SFI; //1byte
    uchar LangPreferExist;
    uchar LangPreferLen;
    uchar LangPrefer[8];//2-8
    uchar ICTIExist;
    uchar ICTI; //Issuer Code Table Index
    uchar AppLabelExist;
    uchar AppLabelLen;
    uchar AppLabel[16];
    uchar PriorityExist;
    uchar Priority; //tag'87'
    uchar PDOLExist;
    uchar PDOLLen;
    uchar PDOL[252];
    uchar PreferNameExist;
    uchar PreferNameLen;
    uchar PreferName[16];
    uchar IssuerDiscretExist;
    uchar IssuerDiscretLen;
    uchar IssuerDiscret[222];
    uchar OdaableExist;
    uchar OdaableLen;
    uchar Odaabledata[20];
}SELECT_RET;

typedef struct {
    uchar Type; //1:DDF,2:ADF
    uchar DFNameExist;//0-non exist;1-exist.
    uchar DFNameLen;
    uchar DFName[16]; //5-16,ADF or DDF name according to Type.
    uchar AppLabelExist;
    uchar AppLabelLen;
    uchar AppLabel[16];
    uchar PreferNameExist;
    uchar PreferNameLen;
    uchar PreferName[16];
    uchar PriorityExist;
    uchar Priority; //tag'87'
    uchar DirDiscretExist;
    uchar DirDiscretLen;
    uchar DirDiscret[222];
}RECORD_PSE;

//.......AppSel
//.......TERMAPP_DataAuth.c
#if 1
extern unsigned short DDOLDataLen;
extern uchar DDOLData[256]; //use for dynamicly store data when processing DDOL,must be initialised in trans init.

extern uchar bCombineDDA;        //to indicate if Combined DDA/AC is to be performed. 0-not perform,1-to be performed.
extern uchar bCombineDDAFail;      //to indicate if Combined DDA/AC is fail. 0-not fail,1-fail.

extern uchar bDDOLHasNoUnpredictNum;
extern uchar bCDOL1HasNoUnpredictNum;
extern uchar bCDOL2HasNoUnpredictNum;
#endif

typedef struct {
    uchar RID[5];
    uchar CAPKI;
    uchar HashInd;
    uchar ArithInd;
    uchar ModulLen;
    //unsigned int ModulLen;
    uchar Modul[248];
    uchar ExponentLen;
    uchar Exponent[3];
    uchar CheckSum[20];
    uchar Expiry[8];
}CAPK_STRUCT;

typedef struct{                                                                            
    uchar AIDLen;
    uchar AID[16];                    //9F06
    uchar ASI;                        //DF01
    uchar AppVer[2];                //9F08
    uchar TACDefault[5];            //DF11
    uchar TACOnline[5];                //DF12
    uchar TACDenial[5];                //DF13
    uchar FloorLimit[4];            //tag'9F1B' terminal floor limit
    uchar Threshold[4];                //threshold for random selection.    DF15
    uchar MaxTargetPercent;            //DF16
    uchar TargetPercent;              //Preset by terminal. range 0-99, and MTP>=TP      DF17
    uchar OnlinePIN;                //DF18
    uchar VLPTransLimit[6];            //9F7B
    uchar OfflineLowestLimit[6];    //DF19    
    uchar TransLimit[6];            //DF20
    uchar CVMLimit[6];              //DF21
}OTHER_ICPARAMETER_STRUCT;

#if 1                                                                                           
extern OTHER_ICPARAMETER_STRUCT OtherICPara    ;
#endif

typedef struct{
    uchar RID[5];
    uchar CAPKI;
    uchar ModulLen;
    uchar Modul[248];
    uchar ExponentLen;
    uchar Exponent[3];
}CAPK_TestSTRUCT;

#if 1
extern uchar CAPK_NUM;
extern CAPK_STRUCT CAPK;
#endif

typedef struct {
    uchar RID[5];
    uchar CAPKI;
    uchar CERTSerial[3];
}IPK_REVOKE;

#if 1
extern uint IPK_REVOKENUM;
#endif

#define IPKRevokeListStoredNum 1

typedef struct  {
    uchar DataHead;//'6A'
    uchar CertFormat;//'02'
    uchar IssuID[4];
    uchar ExpireDate[2];
    uchar CertSerial[3];
    uchar HashInd;
    uchar IPKAlgoInd;
    uchar IPKLen;
    uchar IPKExpLen;
    uchar IPKLeft[212];//NCA-36
    uchar HashResult[20];
    uchar DataTrail;//'BC'
}IPK_RECOVER;

struct ICCPK_RECOVER{
    uchar DataHead;//'6A'
    uchar CertFormat;//'04'
    uchar AppPAN[10];
    uchar ExpireDate[2];
    uchar CertSerial[3];
    uchar HashInd;
    uchar ICCPKAlgoInd;
    uchar ICCPKLen;
    uchar ICCPKExpLen;    
    uchar ICCPKLeft[206];//NI-42
    uchar HashResult[20];
    uchar DataTrail;//'BC'
};

struct SIGN_STAT_APPDATA_RECOVER{
    uchar DataHead;//'6A'
    uchar DataFormat;//'03'
    uchar HashInd;
    uchar DataAuthCode[2];
    uchar PadPattern[222];//NI-26
    uchar HashResult[20];
    uchar DataTrail;//'BC'
};    

struct SIGN_DYN_APPDATA_RECOVER{
    uchar DataHead;//'6A'
    uchar DataFormat;//'05'
    uchar HashInd;//'01'
    uchar ICCDynDataLen;
    uchar ICCDynData[223];//LDD    <NIC-25
    uchar PadPattern[223];//NIC-LDD-25,padded with 'BB'
    uchar HashResult[20];        
    uchar DataTrail;//'BC'
};

struct ICC_DYN_DATA{
    uchar ICCDynNumLen;
    uchar ICCDynNum[8];
    uchar CryptInfo;
    uchar AppCrypt[8];
    uchar HashResult[20];
};

#if 1
extern uchar ICCPKModul[248];
extern uchar ICCPKModulLen;//NICC
extern uchar IPKModul[248];
extern uchar IPKModulLen;//NI
#endif

typedef struct{
    uchar method;
    uchar condition;
}CVR;

struct ICCPIN_EPK_RECOVER{
    uchar DataHead;//'6A'
    uchar CertFormat;//'04'
    uchar AppPAN[10];
    uchar ExpireDate[2];
    uchar CertSerial[3];
    uchar HashInd;
    uchar ICCPIN_EPKAlgoInd;
    uchar ICCPIN_EPKLen;
    uchar ICCPIN_EPKExpLen;    
    uchar ICCPIN_EPKLeft[206];//NI-42
    uchar HashResult[20];
    uchar DataTrail;//'BC'
};

#if 1
extern uchar ICCPIN_EPKModul[248];
extern uchar ICCPIN_EPKModulLen;//NPE

extern CVR CVRList[128];
extern uchar CVRListLen;
extern unsigned long CVM_X,CVM_Y;
extern uchar bPrintReceipt;
#endif


typedef struct 
{
    uchar PAN[10];
    uchar PANSeq;
}EXCEPTION_PAN;

#if 1
extern uchar EncryptPINLen;
extern uchar EncryptPIN[32];
#endif

#define OFFLINE 10
#define ONLINE  11
#define DENIAL  13

//transaction result
#define OFFLINE_APPROVE        10
#define OFFLINE_DECLINE        11
#define ONLINE_APPROVE        12
#define ONLINE_DECLINE        13

#define CryptType_TC      10
#define CryptType_ARQC    11
#define CryptType_AAR     12
#define CryptType_AAC     13

//define referral initialized type: by ICC or issuer
#define ICC_REFER         0
#define ISSUER_REFER      1

//Data object list type
#define typePDOL    0
#define typeCDOL1    1
#define typeCDOL2    2
#define typeDDOL    3
#define typeTDOL    4
#define typePackData        5

#define GenerateAC1  1
#define GenerateAC2  2

//all must be initialized with all zeros.

#if 1
extern uchar IssuAuthenDataLen;
extern uchar IssuAuthenData[16];

extern uchar IssuScriptLen;
extern uchar IssuScript[256];
extern uchar IssuScript71Len;
extern uchar IssuScript71[256];
extern uchar IssuScript72Len;
extern uchar IssuScript72[256];
extern uchar IssuScriptNum;
extern uchar IssuScriptResult[32][5];
extern uchar CurrentScriptID[4];//initialised with all zeros.

extern uchar TransResult;//initial:0
extern uchar bAbleOnline;//initial value: 1

extern unsigned int GenerateACRetDataLen;
extern uchar GenerateACRetData[128];//Returned data in Generate AC except for SignDynAppData, used for Combined DDA/AC

extern unsigned short CDOL1DataLen;
extern uchar CDOL1Data[256];

extern unsigned short CDOL2DataLen;
extern uchar CDOL2Data[256];

extern uchar bTransCapture;//0-trans captured when CID=0x08;1-trans not capture when CID==0x08
//initial value: 0,only set to 1 when testing 2CM.040.00.

//........Analyse.c

//.........ReadData.c
extern uchar TransTargetPercent;
extern uchar RandNum;
extern uchar bRandSelected;
extern uchar bShowRandNum;
//set to 1 for displaying rand num in random trans selection in some EMV test cases,
//other cases needn't display and is set to 0.(default value is 0) 

//.........ReadData.c
//Data types structure
extern uchar AuthData[1024];//Static data to be authenticated,move it to file in March 4,2002
extern unsigned int AuthDataLen;
extern uchar bErrSDATL;//to indicate whether there is error in process SDA tag list. 0-ok,1-error.
extern uchar bErrAuthData;//to indicate whether there is error in process static authentication data. 0-ok,1-error.             

extern uchar AFL[16][4];            // Application File Locator
extern uchar AFL_Num;
extern unsigned int  PDOLDataLen;
extern uchar PDOLData[255];
#endif

typedef struct{
    uchar    AppCrypt[8];            //tag'9F26'
    uchar    AppCurcyCode[2];        //App currency code
    uchar    AppCurcyExp;            //App currency exponent
    uchar    AppDiscretDataLen;        //
    uchar    AppDiscretData[32];        //tag'9F05'
    uchar    AppEffectDate[3];        //application effective date. YYMMDD, BCD3
    uchar    AppExpireDate[3];        //application expired date. BCD3
    uchar    AFLLen;
    uchar    AFL[252];               //added in P70.
    uchar    AIDLen;
    uchar    AID[16];                //Application Identifier(in ICC)
    uchar    AIP[2];                    //Application Interchange Profile
    uchar    AppLabelLen;
    uchar    AppLabel[16];
    uchar    AppPreferNameLen;
    uchar    AppPreferName[16];
    uchar    PANLen;
    uchar    PAN[10];
    uchar    PANSeq;                    //identify card with same pan.
    uchar    AppPriority;
    uchar    AppReferCurcyLen;       
    uchar    AppReferCurcy[8];        //Application reference currency
    uchar    AppReferCurcyExpLen;       
    uchar    AppReferCurcyExp[4];    //Application reference currency exponent
    uchar    ATC[2];                    //Appication transaction counter
    uchar    AUC[2];                    //Application Usage Control
    uchar    AppVer[2];                //VIS1.3.1--0x0083;VIS1.3.2--0x0084;VIS140--0x008c;
    uchar    CDOL1Len;
    uchar    CDOL1[252];
    uchar    CDOL2Len;
    uchar    CDOL2[252];
    uchar    CardHoldNameLen;
    uchar    CardHoldName[26];
    uchar    CardHoldNameExtLen;
    uchar    CardHoldNameExt[45];
    uchar    CVMListLen;
    uchar    CVMList[252];          //Cardholder verification method list
    uchar    CAPKI;                   //Certification Authority Public Key Index
    uchar    CryptInfo;             //tag'9F27'
    uchar    DataAuthCode[2];       //tag"9F45"
    uchar    DFNameLen;
    uchar    DFName[16];
    uchar    DDFNameLen;
    uchar    DDFName[16];
    uchar    DDOLLen;
    uchar    DDOL[252];
    uchar    ICCDynNumLen;
    uchar    ICCDynNum[8];           //ICC dynamic number. len 2-8
    uchar    ICCPIN_EPKCertLen;      //NI
    uchar    ICCPIN_EPKCert[248];
    uchar    ICCPIN_EPKExpLen;       //1 or 3
    uchar    ICCPIN_EPKExp[3];
    uchar    ICCPIN_EPKRemLen;       //NPE-NI+42 < 42
    uchar    ICCPIN_EPKRem[42];
    uchar    ICCPKCertLen;           //NI 
    uchar    ICCPKCert[248];         //ICC Public Key certification
    uchar    ICCPKExpLen;
    uchar    ICCPKExp[3];            //ICC Public Key exponent
    uchar    ICCPKRemLen;            //NIC-NI+42 < 42
    uchar    ICCPKRem[42];            //ICC Public Key remainder
    uchar     IACDenial[5];
    uchar    IACOnline[5];
    uchar    IACDefault[5];
    uchar    IssuAppDataLen;
    uchar    IssuAppData[32];
    uchar    ICTI;                   //Issuer code table index
    uchar    IssuCountryCode[2];
    //uchar    IPKCertLen;          //NCA
    unsigned int IPKCertLen;
    uchar    IPKCert[248];
    uchar    IPKExpLen;           //1-3
    uchar    IPKExp[3];
    uchar    IPKRemLen;           //NI-NCA+36<36
    uchar    IPKRem[36];
    uchar    LangPreferLen;          //2-8
    uchar    LangPrefer[8];
    uchar    LOATC[2];                //Last Online ATC Register
    uchar    LCOL;                    //Lower Consecutive Offline Limit 
    uchar    PINTryCount;            //PIN try counter
    uchar    PDOLLen;
    uchar    PDOL[252];              //added in P70.
    uchar    ServiceCode[2];
    uchar    SignDynAppDataLen;      //NIC
    uchar    SignDynAppData[248];    //128->248
    uchar    SignStatAppDataLen;      //NI
    uchar    SignStatAppData[248];
    uchar    SDATagListLen;
    uchar    SDATagList[128];       //in EMV2000 only tag'82' appears if exist.
    uchar    Track1DiscretLen;
    uchar    Track1Discret[128];
    uchar    Track2DiscretLen;
    uchar    Track2Discret[128];
    uchar    Track2EquLen;
    uchar    Track2Equ[19];          //Track2 equaivalent data
    uchar    TDOLLen;
    uchar    TDOL[252];
    uchar    UCOL;                    //Upper Consecutive Offline Limit
    uchar    IssuerURLLen;
    uchar    IssuerURL[128];    //new added data in EMV2000
    uchar    VLPAvailableFund[6];   //tag'9F79',n12,new added for VLP in VIS1.4.0.
    uchar    VLPIssuAuthorCode[6];   //tag'9F74',asc,"VLP***",new added for VLP in VIS1.4.0.
    uchar    PERSONIDLen;
    uchar    PERSONID[40];            //'9F61'
    uchar    PERSONIDTYPE;            //'9F62'
    uchar    OfflineAmount[6];      //'9F5D'
    uchar    CardTradePro[2];       //'9F6C'
    uchar    Odalen;      
    uchar    Odadata[10];           // 'DF61'
}CARDINFO;

#if 1
extern CARDINFO CardInfo;
#endif


typedef struct{
    long  TransAmt;        /* Transaction amount */
    long TransCount;     /* Transaction Sequence Counter. inc by 1 for each trans*/
    uchar  TransType;        /* Transaction type BCD */
    uchar  TSI[2];           /* Transaction Status Information */
    uchar  PANLen;
    uchar  PAN[10];          /* Primary account No. */
    uchar  PANSeq;           /* EMVTest: sequence num with same PAN */
    uchar  TransDate[3];     /* Trancaction Date(Form:"YY/MM/DD") */
    uchar  TransTime[3];     /* Trancaction Time(Form:"HH/MM/SS") */
} TRANS_LOG;


#if 1
//The following variables correspond to variables in TermInfo which are expressed in uchar array to 
//make it uchar order independent in different platform.(modify in June 6,2002)
extern unsigned short    TransLogMaxNum; //max transLogs stored for check floor limit(default 20)
extern long    Threshold;           //threshold for random selection.
extern long    FloorLimit;          //tag'9F1B' terminal floor long    AmtTrans;               //used in online financial or batch capture msglong    AmtNet;                 //total accumulative amount for reconciliation.
extern unsigned short BatchTransNum;//number of trans stored in terminal.used for reconciliation
extern unsigned short TransNum;     //numbers of floorlimit translog for floorlimit check.
extern unsigned short TransIndex;   //added for new floorlimit translog insert position.
extern long    AmtAuthBin;          //tag'81' Authorised amount of binary
extern long    AmtOtherBin;         //tag'9F04' Other(cashback) amount of binary
extern long    AmtReferCurcy;       //tag'9F3A' Authorised amount in the reference currency
#endif
    
typedef struct{
    uchar   AcquireID[6];           //tag'9F01' Acquirer ID
    uchar    TermCapab[3];          //tag'9F33' terminal capability
    uchar    TermAddCapab[5];       //tag'9F40' terminal additional capability
    uchar    IFD_SN[8];             //tag'9F1E' IFD(POS device) serial no. asc8
    uchar   TermID[8];              //tag'9F1C' Terminal ID
    uchar   MerchCateCode[2];       //tag'9F15' Merchant Category715291 Code
    uchar   MerchID[15];            //tag'9F16' Merchant ID
    uchar    CountryCode[2];        //tag'9F1A' Terminal country code BCD
    uchar    TRMDataLen;
    uchar    TRMData[8];            //tag'9F1D' Terminal Risk Management Data
    uchar    TermType;              //tag'9F35' Terminal type
    uchar    AppVer[2];             //tag'9F09' Application Version Number in terminal//VIS1.3.1--0x0083;VIS1.3.2--0x0084;VIS140--0x008c;
    uchar   TransCurcyCode[2];      //tag'5F2A'
    uchar   TransCurcyExp;          //tag'5F36'
    uchar   TransReferCurcyCode[2]; //tag'9F3C'
    uchar   TransReferCurcyExp;     //tag'9F3D'
    uchar    TACDenial[5];          //Terminal action code-denial
    uchar    TACOnline[5];          //Terminal action code-online
    uchar    TACDefault[5];         //Terminal action code-default
    uchar   TransType;              //for distinguish different trans types such as goods and service.
    uchar   TransTypeValue;         //tag'9C',transtype value(first two digits of processing code) as stated in EMV2000. goods and service are both 0x00.
    uchar    VLPTransLimit[6];      //tag'9F7B',n12, new added in VIS1.4.0.
    uchar    VLPTACDenial[5];       //Terminal action code-denial for VLP
    uchar    VLPTACOnline[5];       //Terminal action code-online for VLP
    uchar    VLPTACDefault[5];      //Terminal action code-default for VLP
    uchar    Language;              //CHINESE or ENGLISH for display and print language.
    uchar    bTermDDOL;             //0-no default DDOl in terminal;1- has default DDOL in terminal  
    uchar    bForceAccept;          //this two set according test script V2CM080.00,V2CM081.00
    uchar    bForceOnline;          //also see emvterm.pdf p32
    uchar    bBatchCapture;         //private set for send different msg to host-AuthRQ and FinaRQ.
    uchar   bTermSupportVLP;        //0-not support;1-support. configurable terminal parameter to indicate if VLP is supported.
    uchar    MaxTargetPercent;
    uchar    TargetPercent;         //Preset by terminal. range 0-99, and MTP>=TP
    uchar    TermDDOLLen;
    uchar    TermDDOL[128];         //term hold of default DDOL,must be initialised in init.
    uchar    TermTDOLLen;
    uchar    TermTDOL[128];         //terminal stored default TDOL.
    uchar   MerchNameLocateLen;
    uchar   MerchNameLocate[128];   //EMV2000 new added
    uchar    TransLogMaxNum[2];     //max transLogs stored for check floor limit(default 20)
    uchar    Threshold[4];          //threshold for random selection.
    uchar    FloorLimit[4];         //tag'9F1B' terminal floor limit
    
    //sencond part are transaction various data and need to be saved.
    uchar    AmtTrans[4];           //used in online financial or batch capture msg.
    uchar    AmtNet[4];             //total accumulative amount for reconciliation.
    uchar    BatchTransNum[2];      //number of trans stored in terminal.used for reconciliation
    uchar    TransNum[2];           //numbers of floorlimit translog for floorlimit check.
    uchar    TransIndex[2];         //added for new floorlimit translog insert position.
    uchar    TransSeqCount[4];      //increment by 1 for each trans. BCD numeric.

    //third part are transaction dependent data and needn't be save.
    //But they may be used in processing DOL. 
    uchar    AmtAuthBin[4];         //tag'81' Authorised amount of binary
    uchar   AmtAuthNum[6];          //tag'9F02' Authorised amount of BCD numeric
    uchar    AmtOtherBin[4];        //tag'9F04' Other(cashback) amount of binary
    uchar   AmtOtherNum[6];         //tag'9F03' Other(cashback) amount of BCD numeric
    uchar    AmtReferCurcy[4];      //tag'9F3A' Authorised amount in the reference currency
    uchar    AIDLen;
    uchar    AID[16];               //tag'9F06' Application Identifier for selected application,5-16
    uchar   AuthorCode[6];          //tag'89'   ret from issuer.move to TermInfo from global variable in P70.
    uchar    AuthRespCode[2];       //tag'8A'   Authorised respose code received from host.
    uchar    CVMResult[3];          //tag'9F34' cardholder verification methods perform result
    uchar    POSEntryMode;          //tag'9F39' POS entry mode,BCD
    uchar   PINLen;
    uchar   PIN[12];                //tag'99'
    uchar    TVR[5];                //tag'95'   Terminal Verification Results
    uchar   TSI[2];                 //tag'9B' Transaction Status Information 
    uchar    VLPIndicator;          //tag'9F7A' //0-not support; 1-support; 2-VLP only. variable parameter to indicate if this trans is VLP supported.
    uchar   TransDate[3];           //tag'9A'   YYMMDD
    uchar   TransTime[3];           //tag'9F21',HHMMSS,BCD
    uchar   TCHashValue[20];        //tag'98'
    uchar   UnpredictNum[4];        //tag'9F37' Terminal created for each transaction.
    uchar   IssuerAuthenDataLen;
    uchar   IssuerAuthenData[16];   //tag'91'   Issuer Authentication Data.
    uchar   MCHIPTransCateCode;     //tag '9F53' Transaction Category Code, Mastercard M/Chip private data.
    uchar    posTermCapab[3];
    uchar   TermTransProp[4];         //tag '9F66'    
    uchar     MerchName[20];             //tag  '9F4E'
    uchar     ExternApp;                 //tag   'DF60"
}TERMINFO;

#if 1
extern TERMINFO TermInfo;
#endif

typedef struct{
    //uchar* Name;
    //uchar  Source;//0-ICC;1-Terminal
    uchar  Tag[3];//Second uchar '00' means only first one uchar tag.add third uchar to make int variable oven aligned.
    uchar  Len1; //Len1>Len2: fix len with Len2;Len1<Len2 and Len2!=255: Len1<= len <=Len2; Len2=255: len=Len1+Len2
    uchar  Len2;
    uchar  bAmt;//0- non numeric;1-numeric;2-compact numric
    u16 address;//var address in struct TermInfo(TERMINFO)
    uchar  flagM;//'1'--bit0: mandatory;bit1: SDA mandatory; bit2: DDA mandatory;
    uchar  bExist;//0-not presented,1-have been existed.    
}DATAELEMENT;

typedef struct{
    uchar TACDenial[5];
    uchar TACOnline[5];
    uchar TACDefault[5];
}ISSUER_TAC;

typedef struct{
    uchar AID[16];
    uchar AIDLen;
    uchar TACDenial[6];
    uchar TACOnline[6];
    uchar TACDefault[6];
    long FloorLimit;
    uchar TargetPercent;
    uchar MaxTargetPercent;
//    unsigned short    TransLogMaxNum;    // unsigned short.
    long Threshold;        // long
    uchar DDOL[128];
    uchar DDOLlen;
    uchar TermOnlinePin;
    uchar AppVer[3];
}ISSUER_DATA;

#if 1
extern ISSUER_DATA AidListTerm[16];
#endif

enum
{
    BATCHUP,
    PRINTRECEIPT,
    UPDATEPARA,
    CHECKTVR
};

enum
{
    SETSYSTIME,
    SETTIDMID,
    SELECTLANG,
    SETTAC,
    SETTRMPARA,
    SETTRANSTYPE,
    SETVLPPARA
};

#if 1
extern DATAELEMENT ICCDataTable[] ;
   

extern DATAELEMENT TermDataTable[];

extern const uchar transTypeValue[];

#endif

#define AppListStoredNum  2

#if 1
extern const TERMAPP AppListStored[];
  
extern  const IPK_REVOKE IPKRevokeListStored[];

extern  const EXCEPTION_PAN ExceptionFileStored[];

extern  const ISSUER_TAC Visa132TAC;
extern  const ISSUER_TAC Visa140TAC;
 extern  const ISSUER_TAC VisaVLPTAC;
        
extern  const ISSUER_TAC Jcb12L2TAC;
extern  const ISSUER_TAC Jcb12L3TAC;
extern  const ISSUER_TAC Jcb20L2TAC;
extern  const ISSUER_TAC Jcb20L3TAC;

extern  const ISSUER_TAC MasterUOBTAC;
#endif

typedef struct{ 
  uchar OfflineLowestLimit[6];        //DF19
  uchar OfflineLowestLimitbExist;    
  uchar TransLimit[6];                //DF20
  uchar TransLimitbExist;
}TERMINFOEX;

#if 1
extern TERMINFOEX TermInfoEx;
#endif

extern uchar TERMAPP_HandleCard(int channel, unsigned int  HostValue);
extern uchar TERMAPP_GetBalance(unsigned int *amount);
extern void  TERMAPP_QPBOCTermInit(int channel);

//extern uchar ReadFile(uchar *File_Name,uchar *Cout_Data,ulong startaddr,uint len);
//extern uchar ReadWholeFile(uchar *File_Name,uchar *Cin_Data);
extern uchar TERMAPP_testGetCAPK(void);
extern void  TERMAPP_CreateUnpredictNum(void);
extern uchar TERMAPP_PackEMVData(uchar *buffer, uchar *DataLen);

//extern uchar UnlinkFile(void);

#endif

