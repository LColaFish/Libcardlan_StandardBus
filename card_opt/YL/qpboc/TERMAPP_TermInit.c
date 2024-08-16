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


//---------
LongUnon DevNum;
//---------

/*
extern FATFS fs;            // Work area (file system object) for logical drive
extern FIL fsrc, fdst;      // file objects
extern FRESULT res;         // FatFs function common result code
extern UINT br, bw;         // File R/W count
*/
char onlineflag;            //连线标记
char odaflag;                //0 是存量卡，1是增量卡
char enableodaflag;            //下笔记录开始进行oda功能

char POSSalerNO[20] = {"CARDLAN            "};
char POSTerminaNO[8];
uchar TermAID0[] = {0xA0, 0x00, 0x00, 0x03, 0x33, 0x01, 0x01, 0x01}; // 借记卡AID
uchar TermAID1[] = {0xA0, 0x00, 0x00, 0x03, 0x33, 0x01, 0x01, 0x06}; // 非接电子现金AID
// added aid , taeguk
uchar TermAID2[] = {0xD1, 0x56, 0x00, 0x09, 0x99, 0x01, 0x01, 0x01};
uchar TermAID3[] = {0xA0, 0x00, 0x00, 0x03, 0x33, 0x01, 0x01, 0x02};

/*一下是西安中行的测试APPID*/
uchar TermAID4[] = {0xA0, 0x00, 0x00, 0x03, 0x33, 0x01, 0x01, 0x03};
uchar TermAID5[] = {0xA0, 0x00, 0x00, 0x00, 0x65, 0x10, 0x10};
uchar TermAID6[] = {0xA0, 0x00, 0x00, 0x00, 0x04, 0x30, 0x60};
uchar TermAID7[] = {0xA0, 0x00, 0x00, 0x00, 0x04, 0x10, 0x10};
uchar TermAID8[] = {0xA0, 0x00, 0x00, 0x00, 0x03, 0x30, 0x10};
uchar TermAID9[] = {0xA0, 0x00, 0x00, 0x00, 0x03, 0x20, 0x10};
uchar TermAID10[] = {0xA0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10};

#define        APPID_ITEMS            11

const  unsigned char *pAppId[APPID_ITEMS] = {
    TermAID0, TermAID1, TermAID2, TermAID3, TermAID4, TermAID5,
    TermAID6, TermAID7, TermAID8, TermAID9, TermAID10
};

/***********************************************************
FUNCTION:  设置终端交易参数，systeminit时调用
PARAMETER DESCRIPTION:
channel: 0 是电子现金 ， 1是双免，2是oda
RETURN:

************************************************************/
void TERMAPP_QPBOCTermInit(int channel)
{
    //检查卡黑名单文件
    //检查终端AID列表，如果不存在，使用默认AIDs
    //初始化终端CAPK信息
    //检查公钥回收列表
    uchar Temp[16];
    uchar ret;
    uchar AIDListTemp[18][16];
    uchar AIDNum;
    uchar i, j;
    uchar strBuf[9];


    LOGI("TERMAPP_QPBOCTermInit() is called.\n");

    // UNUSED
    //收单机构
    memcpy(TermInfo.AcquireID, "\x00\x00\x00\x00\x00\x01", 6);            
    //商户编号
    memcpy(TermInfo.MerchID, POSSalerNO, 15);                            //注意替换
    //终端机号
    memset(POSTerminaNO, 0, sizeof(POSTerminaNO));                        //注意替换
    bin_to_bcd(POSTerminaNO + 4, DevNum.i, 4);                            //注意替换

    // 终端能力参数     
    /*
        终端能力:由终端设置,tag为'9F33',3个字节(在EMV Book4中可找到)
        • Byte 1: Card Data Input Capability
        • Byte 2: CVM Capability
        • Byte 3: Security Capability
    */
    if(1)
    {
        memcpy(TermInfo.posTermCapab, "\xE0\xF9\xC8", 3);
        memcpy(TermInfo.TermCapab, "\xE0\xF9\xC8", 3);
    }
    else
    {
        memcpy(TermInfo.posTermCapab, "\x00\x08\xc0", 3);
        memcpy(TermInfo.TermCapab, "\x00\x08\xc0", 3); //
    }
    
    // 终端能力附加参数
    memcpy(TermInfo.TermAddCapab, "\x20\x00\x10\x10\x00", 5);

    sprintf(strBuf, "%08d", DevNum.i);
    memcpy(TermInfo.IFD_SN, strBuf, 8);                //注意替换
    memcpy(TermInfo.TermID, strBuf, 8);                //注意替换
    // 商户类型码
    memcpy(TermInfo.MerchCateCode, "\x00\x00", 2);

    // MCHIP专用的交易类型码
    TermInfo.MCHIPTransCateCode = 'R';

    // 国家代码、终端类型、货币代码特性参数
    memcpy(TermInfo.CountryCode, "\x01\x56", 2);    //注意替换

    //if(onlineflag)
    if(channel)                        //双免+ODA
    {
        // TermInfo.TermType=0x22;//0x22;                   //之前是25
        TermInfo.TermType=0x25;
    }
    else
    {
        TermInfo.TermType = 0x26;
    }
    //交易货币代码       Trading currency code
    memcpy(TermInfo.TransCurcyCode, "\x01\x56", 2);
    TermInfo.TransCurcyExp = 0x02;
    memcpy(TermInfo.TransReferCurcyCode, "\x01\x56", 2);
    TermInfo.TransReferCurcyExp = 0x02;
    TermInfo.TransType = SERVICE;
    TermInfo.TransTypeValue = transTypeValue[TermInfo.TransType];
    /**********************************************************************
    //    Byte1：                                  支持:1;       不支持:0
    //          bit8：是否支持非接触磁条
    //          bit7：是否支持非接触PBOC
    //          bit6：是否支持非接触qPBOC
    //          bit5：是否支持接触PBOC
    //          bit4：终端是否脱机
    //          bit3：是否支持联机PIN
    //          bit2：是否支持签名
    //          bit1：预留
    //    Byte2：
    //          bit8：是否要求联机密文
    //          bit7：是否要求CVM
    //          其它：预留
    //    其它：
    //          预留
    ************************************************************************/
    
    //if(enableodaflag)
    if(channel==2)
    {
        if(odaflag)
        {
            memcpy(TermInfo.TermTransProp,"\x21\x80\x00\x00",4);                    //注意替换s
        }
        else
        {
            memcpy(TermInfo.TermTransProp,"\x46\x80\x00\x00",4);        
        }
    }
    else
    {
        if(channel)                    //双免                                
        {
            memcpy(TermInfo.TermTransProp,"\x26\x80\x00\x00",4); 
        }
        else       
        {
            memcpy(TermInfo.TermTransProp, "\x28\x00\x00\x00", 4);
        }
    }
    //VLP VISA Low-Value Payment Visa小额支付
    TermInfo.bTermSupportVLP = 0x01;
    memcpy(TermInfo.VLPTransLimit, "\x00\x00\x00\x00\x40\x00", 6);
    memcpy(TermInfo.VLPTACDenial, "\x7C\x70\xB8\x08\x00", 5); //initial VLP TACs set according to VIS1.4.0.
    memcpy(TermInfo.VLPTACOnline, "\x00\x00\x00\x00\x00", 5);
    memcpy(TermInfo.VLPTACDefault, "\x7C\x70\xB8\x08\x00", 5);

    // 默认DDOL   动态数据认证数据对象列表（Dynamic Data Authentication Data Object List）
    TermInfo.TermDDOLLen = 0x03;
    memcpy(TermInfo.TermDDOL, "\x9F\x37\x04", 3);
    // 默认TDOL 交易证书数据对象列表（Transaction Certificate Data Object List）
    TermInfo.TermTDOLLen = 15;
    memcpy(TermInfo.TermTDOL, "\x9F\x02\x06\x5F\x2A\x02\x9A\x03\x9C\x01\x95\x05\x9F\x37\x04", 15);
    // 终端其他参数初始化
    TermInfo.Language = CHINESE;
    TermInfo.bTermDDOL = 1;
    TermInfo.bForceAccept = 0;
    TermInfo.bForceOnline = 0;
    TermInfo.bBatchCapture = 0;
    TermInfo.TargetPercent = 20;
    TermInfo.MaxTargetPercent = 30;
    TermInfo.MerchNameLocateLen = 0;
    memcpy(TermInfo.AppVer, "\x00\x20", 2);

    TransLogMaxNum = 20;
    IntToByteArray(TransLogMaxNum, TermInfo.TransLogMaxNum, 2);
    Threshold = 20;        // long
    IntToByteArray(Threshold, TermInfo.Threshold, 4);
    FloorLimit = 30000;        // long
    //终端交易金额上限
    IntToByteArray(FloorLimit, TermInfo.FloorLimit, 4);
    BatchTransNum = 0;
    TransNum = 0;
    TransIndex = 0;
    AmtAuthBin = 0;
    AmtOtherBin = 0;
    AmtReferCurcy = 0;

    memset(TermInfo.AmtAuthNum, 0, sizeof(TermInfo.AmtAuthNum));
    //TermInfo.AmtAuthNum[5] = 0x01;
    memcpy(TermInfo.AmtAuthBin, TermInfo.AmtAuthNum + 2, 4);

    /*
    ret=ReadFile("para.t",Temp,1*16,6);            //其他金额
    if(ret==OK) memcpy(TermInfo.AmtOtherNum,Temp,6);
    else    memset(TermInfo.AmtOtherNum,0,6);

    ret=ReadFile("para.t",Temp,4*16,1);  //随机目标百分比
    if(ret==OK) TermInfo.TargetPercent=Temp[0];
    else    TermInfo.TargetPercent=99;

    ret=ReadFile("para.t",Temp,5*16,4);  //偏置随机选择阈值
    if(ret==OK) memcpy(TermInfo.Threshold,Temp,4);
    else    memcpy(TermInfo.Threshold,"\xFF\xFF\xFF\xFF",4);

    ret=ReadFile("para.t",Temp,6*16,1);     //偏置随机选择的最大目标百分比
    if(ret==OK) TermInfo.MaxTargetPercent=Temp[0];
    else    TermInfo.MaxTargetPercent=99;

    ret=ReadFile("para.t",Temp,7*16,2);     //终端国家代码
    if(ret==OK) memcpy(TermInfo.CountryCode,Temp,2);
    else    memcpy(TermInfo.CountryCode,"\x01\x56",2);

    ret=ReadFile("para.t",Temp,8*16,6);     //非接触读写器脱机最低限额
    if(ret==OK) {memcpy(TermInfoEx.OfflineLowestLimit,Temp,6);TermInfoEx.OfflineLowestLimitbExist=1;}
    else    memcpy(TermInfoEx.OfflineLowestLimit,"\x0\x0\x0\x0\x0\x0",6);

    ret=ReadFile("para.t",Temp,9*16,6);  //非接触读写器交易限额
    if(ret==OK) {memcpy(TermInfoEx.TransLimit,Temp,6);TermInfoEx.TransLimitbExist=1;}
    else    memcpy(TermInfoEx.TransLimit,"\x0\x0\x0\x0\x0\x0",6);

    ret=ReadFile("para.t",Temp,10*16,4); //终端最低限额
    if(ret==OK) memcpy(TermInfo.FloorLimit,Temp,4);
    else    memcpy(TermInfo.FloorLimit,"\x0\x0\x0\x0",4);

    ret=ReadFile("para.t",Temp,11*16,15);//终端行为码缺省+联机+拒绝
    if(ret==OK)
    {
        memcpy(TermInfo.TACDefault,Temp,5);
        memcpy(TermInfo.TACOnline,Temp+5,5);
        memcpy(TermInfo.TACDenial,Temp+10,5);
    }
    else
    {
        memset(TermInfo.TACDefault,0,5);
        memset(TermInfo.TACOnline,0,5);
        memset(TermInfo.TACDenial,0,5);
    }
    */


    //其他金额
    memset(TermInfo.AmtOtherNum, 0, 6);

    //随机目标百分比
    TermInfo.TargetPercent = 99;

    //偏置随机选择阈值
    memcpy(TermInfo.Threshold, "\xFF\xFF\xFF\xFF", 4);

    //偏置随机选择的最大目标百分比
    TermInfo.MaxTargetPercent = 99;

    //终端国家代码
    memcpy(TermInfo.CountryCode, "\x01\x56", 2);

    //非接触读写器脱机最低限额
    memcpy(TermInfoEx.OfflineLowestLimit, "\x0\x0\x0\x0\x0\x0", 6);

    //非接触读写器交易限额
    memcpy(TermInfoEx.TransLimit, "\x0\x0\x0\x0\x0\x0", 6);

    //终端最低限额
    memcpy(TermInfo.FloorLimit, "\x0\x0\x0\x0", 4);

    //终端行为码缺省+联机+拒绝
    memset(TermInfo.TACDefault, 0, 5);
    memset(TermInfo.TACOnline, 0, 5);
    memset(TermInfo.TACDenial, 0, 5);

    // added by taeguk 增加终端电子现金AID
    memcpy(TermInfo.AID, TermAID1, sizeof(TermAID1));

    /*
    res = f_mount(0, &fs);
    res = f_open (&fsrc,"AIDLIST",FA_READ);
    if(res==OK)
    {
        AIDNum=(fsrc.fsize-32)/16;
        TermAppNum=AIDNum;
        res=f_read(&fsrc, AIDListTemp,fsrc.fsize, &br);
        for(i=0;i<AIDNum;i++)
        {
            AppListTerm[i].ASI=AIDListTemp[0][i];
            AppListTerm[i].AIDLen=AIDListTemp[1][i];
            memcpy(AppListTerm[i].AID,&AIDListTemp[i+2][0],AppListTerm[i].AIDLen);
        }
        f_close (&fsrc);
        f_mount(0,NULL);
    }
    f_close (&fsrc);
    f_mount(0,NULL);
    */

    if(1)                        //支持手机pay，手机pay的AID比较特殊
    {
        TermAppNum = APPID_ITEMS;
        memset(AppListTerm, 0, sizeof(AppListTerm));

        for(i = 0; i < APPID_ITEMS; i++) {
            AppListTerm[i].ASI = 0;
            AppListTerm[i].AIDLen = sizeof(pAppId[i]);
            memcpy(AppListTerm[i].AID, pAppId[i], AppListTerm[i].AIDLen);
           }
    }
    else
    {
        TermAppNum = 1;
        // Add AID0
        AppListTerm[0].ASI = 0; // partial match
        AppListTerm[0].AIDLen = sizeof(TermAID0);
        memcpy(AppListTerm[0].AID, TermAID0, AppListTerm[0].AIDLen);
    /*
        // Add AID1
        AppListTerm[1].ASI = 0; // partial match
        AppListTerm[1].AIDLen = sizeof(TermAID1);
        memcpy(AppListTerm[1].AID, TermAID1, AppListTerm[1].AIDLen);

        // Add AID2
        AppListTerm[2].ASI = 0; // partial match
        AppListTerm[2].AIDLen = sizeof(TermAID2);
        memcpy(AppListTerm[2].AID, TermAID2, AppListTerm[2].AIDLen);

        // Add AID2
        AppListTerm[3].ASI = 0; // partial match
        AppListTerm[3].AIDLen = sizeof(TermAID3);
        memcpy(AppListTerm[3].AID, TermAID3, AppListTerm[3].AIDLen);
    */
    }
    /*
    LOGI("TERMAPP_QPBOCTermInit(): TermAppNum = %d.\n", TermAppNum);
    for (j = 0; j < TermAppNum; j++)
    {
        LOGI("TERMAPP_QPBOCTermInit(): AppListTerm[%d]:\n", j);
        menu_print((char *)&AppListTerm[j], sizeof(TERMAPP));
    }
    */
}




/************************************************************
FUNCTION:  QPBOC卡交易参数初始化，每笔交易前调用
PARAMETER DESCRIPTION:

RETURN:
*************************************************************/
void TERMAPP_QPBOCTransInit(unsigned int transSeqId, unsigned int amount,int channel)
{
    uchar i;

    LOGI("TERMAPP_QPBOCTransInit() is called.\n");

    bPrintReceipt = 0;
    bErrSDATL = 0;
    bErrAuthData = 0;
    bCombineDDA = 0;
    bCombineDDAFail = 0;
    bDDOLHasNoUnpredictNum = 0;
    bCDOL1HasNoUnpredictNum = 0;
    bCDOL2HasNoUnpredictNum = 0;

    memset((uchar *)&CAPK, 0, sizeof(CAPK_STRUCT));
    memset(IPKModul, 0, 248);
    memset(ICCPKModul, 0, 248);

    ICCPKModulLen = 0;
    IPKModulLen = 0;

    memset((uchar *)&AFL, 0, 16 * 4);
    memset(AuthData, 0, 1024);
    AuthDataLen = 0;

    memset((uchar *)&CardInfo, 0, sizeof(CARDINFO));
    memset(CardInfo.PAN, 0xFF, 10);
    memset(CardInfo.Track2Discret, 0xFF, 128);
    CardInfo.PANSeq = 0xFF;

    memset((uchar *)&SelectedApp, 0, sizeof(APPDATA));

    memset((uchar *)&AppListCandidate[0], 0, sizeof(APPDATA));
    memset((uchar *)&AppListCandidate[1], 0, sizeof(APPDATA));
    memset((uchar *)&AppListCandidate[2], 0, sizeof(APPDATA));
    memset((uchar *)&AppListCandidate[3], 0, sizeof(APPDATA));
    memset((uchar *)&AppListCandidate[4], 0, sizeof(APPDATA));
    memset((uchar *)&AppListCandidate[5], 0, sizeof(APPDATA));
    memset((uchar *)&AppListCandidate[6], 0, sizeof(APPDATA));
    memset((uchar *)&AppListCandidate[7], 0, sizeof(APPDATA));
    memset((uchar *)&AppListCandidate[8], 0, sizeof(APPDATA));
    memset((uchar *)&AppListCandidate[9], 0, sizeof(APPDATA));
    memset((uchar *)&AppListCandidate[10], 0, sizeof(APPDATA));
    memset((uchar *)&AppListCandidate[11], 0, sizeof(APPDATA));
    memset((uchar *)&AppListCandidate[12], 0, sizeof(APPDATA));
    memset((uchar *)&AppListCandidate[13], 0, sizeof(APPDATA));
    memset((uchar *)&AppListCandidate[14], 0, sizeof(APPDATA));
    memset((uchar *)&AppListCandidate[15], 0, sizeof(APPDATA));

    for(i = 0; i < ICCDataNum; i++)
    {
        ICCDataTable[i].bExist = 0;
    }
    
    bin_to_bcd(TermInfo.TransSeqCount, transSeqId, 4);

    AmtAuthBin = amount;
    IntToByteArray(AmtAuthBin, TermInfo.AmtAuthBin, 4);
    memset(TermInfo.AmtAuthNum, 0, sizeof(TermInfo.AmtAuthNum));
    bin_to_bcd(TermInfo.AmtAuthNum + 2, AmtAuthBin, 4);

    AmtOtherBin = 0;
    IntToByteArray(AmtOtherBin, TermInfo.AmtOtherBin, 4);
    memset(TermInfo.TVR, 0, 5);
    memset(TermInfo.TSI, 0, 2);
    TermInfo.AIDLen = 0;
    memset(TermInfo.AID, 0, 16);
    TermDataTable[MV_TERM_AID - TermDataBase].bExist = 0;

    memcpy(TermInfo.TermCapab, TermInfo.posTermCapab, 3);

    memset(TermInfo.AuthRespCode, 0, 2);
    TermDataTable[MV_AuthorRespCode - TermDataBase].bExist = 0;

    TermInfo.VLPIndicator = 0; //initialised to 0 before a trans.
    bCardConfirmVLP = 0;
    //if(onlineflag)
    if(channel)
    {
        TermInfo.CVMResult[0] = 0x00;
    }
    else
    {
        TermInfo.CVMResult[0] = 0x3F;
    }
    
    TermInfo.POSEntryMode = 0x07;

    memset(TermInfo.UnpredictNum, 0, 4);
    TermDataTable[MV_UnpredictNum - TermDataBase].bExist = 0; //no Unpredictable number at first.

    memset(TermInfo.IssuerAuthenData, 0, 16);
    TermInfo.IssuerAuthenDataLen = 0;
    memset(IssuAuthenData, 0, 16);
    IssuAuthenDataLen = 0;

    memset(EncryptPIN, 0, 32);
    EncryptPINLen = 0;

    memset(PDOLData, 0, 255);
    PDOLDataLen = 0;

    memset(IssuScript, 0, 256);
    IssuScriptLen = 0;
    memset(IssuScript71, 0, 256);
    IssuScript71Len = 0;
    memset(IssuScript72, 0, 256);
    IssuScript72Len = 0;
    memset((uchar *)&IssuScriptResult, 0, 32 * 5);
    IssuScriptNum = 0;
    memset(CurrentScriptID, 0, 4); //initialised with all zeros.

    TransResult = 0; //initial:0

    //if(onlineflag)
    if(channel)
        bAbleOnline = 1;
    else
        bAbleOnline = 0; //initial value: 1
    bReversal = 0; //used for sending reversal in completion.
    bAdvice = 0; //used for send online advice in completion.

    SelectedAppNo = 0xff;
    TERMAPP_CreateUnpredictNum();
    TermDataTable[MV_TermTransProp].bExist = 0;
}

