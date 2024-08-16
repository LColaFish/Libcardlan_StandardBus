#include "includes.h"
//---------#include "apparel.h"
#include "TERMAPP_record.h"
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
LongUnon HostValue;
CardInform CardLan;
//---------

/*
extern FATFS fs;            // Work area (file system object) for logical drive
extern FIL fsrc, fdst;      // file objects
extern FRESULT res;         // FatFs function common result code
extern UINT br, bw;         // File R/W count
*/


extern uchar buff5A[150];    //00 b2 01 14 00    包含主账号5A的记录
extern uchar buff57[40];    //00 b2 01 0c 00    包含2磁道等价数据57的记录
extern ulong BankAmount;    //银行卡余额
extern unsigned char PBOC_SwipeCardError;
extern unsigned char OriginalCardType;

void  TERMAPP_CheckFloorLimit(void);
void  TERMAPP_RandTransSelect(void);
void  TERMAPP_CheckExceptionFile(void);
void  TERMAPP_CheckVer(void);
void  TERMAPP_CheckAUC(void);
uchar TERMAPP_CheckExpDate(void);
uchar TERMAPP_CheckDateFormat(uchar* date);

uchar TERMAPP_ReadTag(int type, char *RecvData, int len)
{
    char SendData[16];
    uchar result = 0;
    memset(SendData, 0, sizeof(SendData));

    LOGI("TERMAPP_ReadTag() is called.\n");
    
    switch(type)
    {
        case ATC_TAG:
            SendData[0] = 0x80;
            SendData[1] = 0xCA;
            SendData[2] = 0x9F;
            SendData[3] = 0x36;
            SendData[4] = 0x00;
            break;
        default:
            printf("TERMAPP_ReadTag not fount type[%d]\n", type);
            break;
    }

    if(MifarePro_SendCom(SendData,5) == 0)
    {
        if(len >= SRCPUCardCount)
            memcpy(RecvData, SRCPUCardBuf, SRCPUCardCount);
        else
            printf("TERMAPP_ReadTag Buffer is to little\n");
    }

    return 0;
}


uchar TERMAPP_SwipeCardErrorHandler(void)
{    
    uchar RecvData[64];
    uchar ATC_buf[4];
    uchar TempBuf[64];
    uchar pBuffer[128];
    uchar DataLen = 0, ret = 0;
    
    struct SaveSwipeCardErrorData ErrInfo;

    memset(ATC_buf, 0, sizeof(ATC_buf));
    memset(RecvData,0, sizeof(RecvData));
    memset(TempBuf,0, sizeof(TempBuf));
    memset(&ErrInfo, 0, sizeof(struct SaveSwipeCardErrorData));

    
    
    memcpy(ErrInfo.Buf5A, buff5A, sizeof(buff5A));
    memcpy(ErrInfo.Buf57, buff57, sizeof(buff57));
    memcpy(ErrInfo.ATC_buf, CardInfo.ATC, sizeof(CardInfo.ATC));
    memcpy(ErrInfo.AFL_last, AFL[AFL_Num-1], sizeof(AFL[AFL_Num-1]));
    memcpy(ErrInfo.CardCsnB, CardLan.CardCsnB, 4);
    memcpy(ErrInfo.CardCsnB_blk, CardLan.CardCsnB_blk, 8);
    
    ErrInfo.BankAmount = BankAmount;

    memset(buff5A, 0, sizeof(buff5A));
    memset(buff57, 0, sizeof(buff57));
    BankAmount = 0;
    
     //---------ShowSwipeCardStatus(1);
    
    if(0) //---------if(WaitForSwipeCardOrTimeOut(&PBOC_SwipeCardError)  != MI_OK)
    {
        return SWIPE_CARD_ERROR;
    }

    if(CardReset(pBuffer,&DataLen, 0) != 0x20)
    {
        return SWIPE_CARD_ERROR;
    }

    //取卡类型
     //---------ret = GetPBOCCardType(&OriginalCardType);
    if (ret != OK)
    {
        if(ret == 0xA0)
        {
            Err_display(1);
            return JY_REFUSE;
        }
        else
        {
            OriginalCardType = 1;
            ret = OK;
        }
    }    
        
    TERMAPP_GetCardNum(RecvData, sizeof(RecvData),TempBuf,sizeof(TempBuf));

    //---------PBOC_GetCardNum();
    
    /* 这里是比较是否为同一张卡 */
    DebugPrintChar("CardLan.CardCsnB", CardLan.CardCsnB, 4);
    DebugPrintChar("CardLan.CardCsnB_blk", CardLan.CardCsnB_blk, 8);
    
    DebugPrintChar("ErrInfo.CardCsnB", ErrInfo.CardCsnB, 4);
    DebugPrintChar("ErrInfo.CardCsnB_blk", ErrInfo.CardCsnB_blk, 8);

    
    
    if(strncmp(CardLan.CardCsnB, ErrInfo.CardCsnB, sizeof(ErrInfo.CardCsnB)))
    {
        if(strncmp(CardLan.CardCsnB_blk, ErrInfo.CardCsnB_blk, sizeof(ErrInfo.CardCsnB_blk)))
        {
            return SWIPE_CARD_ERROR;
        }
    }

    /* 读取ATC标签验证是否相同 */
        
    
    if(TERMAPP_ReadTag(ATC_TAG, RecvData, sizeof(RecvData)) == 0)
    {
        if((RecvData[0] == 0x9F) && (RecvData[1] == 0x36) && (RecvData[2] == 0x02))
        {
            memcpy(ATC_buf, RecvData+3, 2);
            DebugPrintChar("ErrInfo.ATC_buf", ErrInfo.ATC_buf, 2);
            DebugPrintChar("ATC_buf", ATC_buf, 2);
            
            if(strncmp(ErrInfo.ATC_buf, ATC_buf, 2))
                return SWIPE_CARD_ERROR;
        }else
            return SWIPE_CARD_ERROR;
    }else
        return SWIPE_CARD_ERROR;
        
    /* 比较两次的金额是否一致 */
    DebugPrintf("BankAmount = %u\n", BankAmount);
    DebugPrintf("ErrInfo.BankAmount = %u\n", ErrInfo.BankAmount);

    if(ErrInfo.BankAmount == BankAmount)
    {
        return SWIPE_CARD_ERROR;
    }

    ret = CardReset(&SRCPUCardBuf[0], &SRCPUCardCount, 1);
    if (ret != MI_OK)   
    {
        ret = JY_END;
    }


    return OK;
}

/* 
    读应用数据
        根据AFL 发送read record 命令
        读取交易以需的应用数据
    短文标识符SFI:
        1-10：EMV数据文件
        11-20：支付系统数据文件
        21-30：发卡行数据文件
    TLV编码格式 TLV(type length value)
*/
uchar TERMAPP_ReadAppData()
{
    int ret = 0;
    uchar i,j,k,m,n,t,bInTable;
    unsigned int index,indexOut,len;
    uchar state[2];
    ulong TempAmount=0;
    uchar OfflineAmount[6]={0};
    uchar dispbuf[15]={0};
    uchar SwipeCardError = 0;

    LOGI("TERMAPP_ReadAppData() is called.\n");

    indexOut=0;
    bErrAuthData=0;
    bErrSDATL=0;

    LOGI("TERMAPP_ReadAppData(): AFL_Num = %d.\n", AFL_Num);

    for(m = 0; m < AFL_Num; m++)
    {
        t = AFL[m][0];
        t = t >> 3;
        if((t <= 0) || (t >= 31))                   return JY_END;
        if(AFL[m][1] < 1)                           return JY_END;
        if(AFL[m][2] < AFL[m][1])                   return JY_END;
        if(AFL[m][3] > (AFL[m][2] - AFL[m][1] + 1)) return JY_END;
    }    

    /* AFL_Num = 5 */
    for(m = 0; m < AFL_Num; m++)
    {
        t = AFL[m][0];
        t = t >> 3;    
        //printf("---m:%d,AFL[%d][1]:%d, AFL[%d][2]:%d---\n",m,m,AFL[m][1],m,AFL[m][2]);
        for(n = AFL[m][1]; n <= AFL[m][2]; n++)     /* from first record to last record for each AFL */
        {    
            if(SwipeCardError == 1)
            {
                n = AFL[m][2];
            }            
            unsigned char record[256];
            int result = 0;

            result = TERMAPP_ReadRecord(AFL[m][0] , n ,record,state);
            if(state[0] != 0x90 || state[1] != 0x00)
            {
                return JY_END;
            }

            SwipeCardError = 0; 
            index = 0;    
                
            if((0 < t) && (t <= 10))
            {
                ret = DecodeTLVLen(record, result);                                      
                if(ret != OK)
                {
                    LOGI("[ %s %d ] error ret : %d \n",__FUNCTION__,__LINE__,ret);
                    return JY_END;                                                      
                }
                
                if( n < AFL[m][1] + AFL[m][3] )                                                                  
                {
                    if(record[index] == 0x70)                                                          
                    {                                                                                      
                        index++;                                                                      
                        if(record[index] == 0x81) 
                        {
                            index++;                                      
                        }
                        len = record[index];                                                      
                        index++;                                                                      
                        memcpy((uchar *)&AuthData[indexOut], record + index, len);                  
                        indexOut += len;                                                                  
                    }
                    else
                    {
                        bErrAuthData=1;
                        TermInfo.TVR[0] |= 0x08;        
                        TermInfo.TSI[0] |= 0x80;        
                        return JY_END;                
                    }
                }                
            }
            
            if((t >= 11) && (t <= 30))
            {
                if( n < AFL[m][1] + AFL[m][3])
                {
                    if(record[0] == 0x70)
                    {
                        memcpy((uchar *)&AuthData[indexOut], record, result);
                        indexOut += result;
                    }
                }    
            }
            
            ret = TERMAPP_DecodeTLV(record,result);  
            if(ICCDataTable[MV_AppExpireDate].bExist == 1)                   
            {
                ret = TERMAPP_ExpireDate(CardInfo.AppExpireDate);
                if( ret != OK )
                {
                    LOGI("[ %s %d ] error ret : %d \n",__FUNCTION__,__LINE__,ret);
                    return JY_REFUSE;     
                }
            }    
            
            if(ret != OK)
            {
                LOGI("[ %s %d ] error ret : %d \n",__FUNCTION__,__LINE__,ret);
                return ret;
            }
        }    
    }

    for(i = 0; i < ICCDataNum; i++)   //check if mandatory data are missing
    {
        if(((ICCDataTable[i].flagM & 0x01) == 0x01) && (ICCDataTable[i].bExist == 0))
        {
            LOGI("TERMAPP_ReadAppData(): ICCDataTable[%d].flagM = 0x%02X.\n", i, ICCDataTable[i].flagM);
            return JY_REFUSE;
        }
    }

    if(ICCDataTable[MV_OfflineAmount].bExist == 1)
    {
        memcpy(OfflineAmount, CardInfo.OfflineAmount, 6);
        TempAmount = ((OfflineAmount[3] & 0xf0) >> 4) * 100000 + (OfflineAmount[3] & 0x0f) * 10000 + (( OfflineAmount[4] & 0xf0) >> 4) * 1000 + (OfflineAmount[4] & 0x0f) * 100+((OfflineAmount[5] & 0xf0) >> 4) * 10 + (OfflineAmount[5] & 0x0f);
        sprintf(dispbuf, "   %5d", TempAmount);
        //屏幕显示扣值后的卡内余额dispbuf
        LOGI("TERMAPP_ReadAppData(): MV_OfflineAmount = %s.\n", dispbuf);
    }

    //语音提示：可以移开卡片    
    if(ICCDataTable[MV_SDATagList].bExist == 1)  //if SDA_TL exist
    {
        if((CardInfo.SDATagListLen == 1) && (CardInfo.SDATagList[0] == 0x82))
        {    
            memcpy((uchar *) & AuthData[indexOut], CardInfo.AIP, 2);
            indexOut += 2;
        }
        else 
        {
            bErrSDATL = 1;
        }
    }
    
    AuthDataLen = indexOut;
    if(ICCDataTable[MV_VLPIssuAuthorCode].bExist == 1)                    
    {
        bCardConfirmVLP = 1;
    }
    
    return OK;
}

uchar TERMAPP_TermRiskManage()
{
    uchar ret=1;

    LOGI("TERMAPP_TermRiskManage() is called.\n");

    if((CardInfo.AIP[0] & 0x08))    //AIP of card support terminal risk management.
    {
        TERMAPP_CheckFloorLimit();
        //TERMAPP_RandTransSelect();
        TERMAPP_CheckExceptionFile();                       //查黑名单
        TermInfo.TSI[0] |= 0x08; //set 'Terminal risk management was performed' bit 1
    }
    return OK;
}

void TERMAPP_CheckFloorLimit()
{
    uchar i;
    long amt=0;
    int fid;
    TRANS_LOG transLog;

    LOGI("TERMAPP_CheckFloorLimit() is called.\n");

    amt=AmtAuthBin;

//    if(FloorLimit==0) return;//added according to test script V2CJ131.00(only in EMV96,deleted in EMV2000)Gu Bohua,July 15,2002
    if(amt >= FloorLimit)                     
    {
        TermInfo.TVR[3] |= 0x80;//set 'transaction exceeds floor limit' bit 1.
    }
}

void TERMAPP_RandTransSelect()
{
    unsigned int seed;
    uchar time[8];

    LOGI("TERMAPP_RandTransSelect() is called.\n");

    bRandSelected=0;

    GetTime(time);
    seed=time[3]+time[4]+time[5];
    srand(seed);
    RandNum=(uchar)(rand()%99+1);//rand(): 0-65535;RandNum:1-99
    
    if(AmtAuthBin<Threshold)
    {
        if(RandNum<=TermInfo.TargetPercent) bRandSelected=1;
    }
    else
    {
        if(FloorLimit<Threshold+1) return;
        TransTargetPercent=(uchar)((TermInfo.MaxTargetPercent-TermInfo.TargetPercent)*(AmtAuthBin-Threshold)/(FloorLimit-Threshold)+TermInfo.TargetPercent);
        if(RandNum<=TransTargetPercent) bRandSelected=1;
    }
    if(bRandSelected==1)
        TermInfo.TVR[3]|=0x10;//set 'Transaction selected randomly for online processing' bit 1.
}

void TERMAPP_CheckExceptionFile()
{
    /*
    long i,len,ExceptionFileNum;
    uchar FileTemp[_MAX_SS];
    u32 filesize;
    EXCEPTION_PAN exceptionPan;

    res    = f_open (&fsrc,"UNUSUAL",FA_READ);
    if(res!=OK) return;
    filesize = fsrc.fsize;
    res=f_read(&fsrc, FileTemp,filesize, &br);
    if(res!=OK) return;
    for(i=0;i<filesize;i++)
    {
        if(!memcmp(CardInfo.PAN,FileTemp,CardInfo.PANLen))
        {
            TermInfo.TVR[0]|=0x10;//set "Card appears on exception file" bit 1
            break;
        }
    }
    f_close (&fsrc);
    f_mount(0,NULL);
    */

    LOGI("TERMAPP_CheckExceptionFile() is called.\n");
    
}

uchar TERMAPP_ProcessRestrict()
{
    LOGI("TERMAPP_ProcessRestrict() is called.\n");

    TERMAPP_CheckVer();
    TERMAPP_CheckAUC();
    if(TERMAPP_CheckExpDate()!=OK) return NOK;
    return OK;
}

void TERMAPP_CheckVer()
{
    LOGI("TERMAPP_CheckVer() is called.\n");
    uchar Ver[2];
    memcpy(Ver,"\x00\x30",2);
    if(ICCDataTable[MV_ICC_AppVer].bExist==0)//App version num is not exist,don't check it and continue following  process
        return;
    if(memcmp(CardInfo.AppVer,TermInfo.AppVer,2)){
        if(memcmp(CardInfo.AppVer,Ver,2))
            TermInfo.TVR[1]|=0x80;//set ICC and Term have diffenrent ver bit 1.
    }
}

void TERMAPP_CheckAUC()
{
    uchar bTestFail;

    LOGI("TERMAPP_CheckAUC() is called.\n");

    bTestFail=0;
    if(ICCDataTable[MV_AUC].bExist)//AUC exist.
    {
        if((TermInfo.TermType==0x14 || TermInfo.TermType==0x15 || TermInfo.TermType==0x16 )
            && (TermInfo.TermAddCapab[0]&0x80)){//The termianl is ATM
            if(!(CardInfo.AUC[0]&0x02))// if‘Valid at ATMs’bit not on.
                bTestFail=1;
        }
        else{//The terminal is not ATM
            if(!(CardInfo.AUC[0]&0x01))// if‘Valid at terminals other than ATMs’bit not on.
                bTestFail=1;
        }
        if(ICCDataTable[MV_IssuCountryCode].bExist){//Issuer country code exist
            if(!memcmp(CardInfo.IssuCountryCode,TermInfo.CountryCode,2)){//domestic
                if(TermInfo.TransType==CASH){
                    if(!(CardInfo.AUC[0]&0x80))// if‘Valid for domestic cash transactions’bit not on.
                        bTestFail=1;
                }
                if(TermInfo.TransType==GOODS){
                    if(!(CardInfo.AUC[0]&0x20))// if‘Valid for domestic goods’bit not on.
                        bTestFail=1;
                }
                if(TermInfo.TransType==SERVICE){
                    if(!(CardInfo.AUC[0]&0x08))// if‘Valid for domestic services’bit not on.
                        bTestFail=1;
                }
                //if(TermInfo.TransType==CASHBACK){
                if(AmtOtherBin!=0 || TermInfo.TransType==CASHBACK){
                    if(!(CardInfo.AUC[1]&0x80))// if‘domestic cashback allowed’bit not on.
                        bTestFail=1;
                }
            }
            else{//international,terminal country code differ from issuer country code
                if(TermInfo.TransType==CASH){
                    if(!(CardInfo.AUC[0]&0x40))// if‘Valid for international cash transactions’bit not on.
                        bTestFail=1;
                }
                if(TermInfo.TransType==GOODS){
                    if(!(CardInfo.AUC[0]&0x10))// if‘Valid for international goods’bit not on.
                        bTestFail=1;
                }
                if(TermInfo.TransType==SERVICE){
                    if(!(CardInfo.AUC[0]&0x04))// if‘Valid for international goods’bit not on.
                        bTestFail=1;
                }
                //if(TermInfo.TransType==CASHBACK){
                if(AmtOtherBin!=0 || TermInfo.TransType==CASHBACK){
                    if(!(CardInfo.AUC[1]&0x40))// if‘international cashback allowed’bit not on.
                        bTestFail=1;
                }
            }
        }
    }
    if(bTestFail)
    {
        TermInfo.TVR[1]|=0x10;//set‘Requested service not allowed for card product’bit 1
    }
}

uchar TERMAPP_CheckExpDate()
{
    uchar buf[10],currentDate[4],EffectDate[4],ExpireDate[4];

    LOGI("TERMAPP_CheckExpDate() is called.\n");

    GetTime(buf);
    if(buf[0]>0x49) currentDate[0]=0x19;
    else currentDate[0]=0x20;
    memcpy((uchar *)&currentDate[1],buf,3);
    if(ICCDataTable[MV_AppEffectDate].bExist)//App effective date exist
    {
        if(TERMAPP_CheckDateFormat(CardInfo.AppEffectDate)==NOK) return NOK;
        if(CardInfo.AppEffectDate[0]>0x49) EffectDate[0]=0x19;
        else EffectDate[0]=0x20;
        memcpy((uchar *)&EffectDate[1],CardInfo.AppEffectDate,3);
        if(memcmp(currentDate,EffectDate,4)<0)
            TermInfo.TVR[1]|=0x20;//set‘Application not yet effective’bit 1
    }
    if(ICCDataTable[MV_AppExpireDate].bExist){//App expiration date exist
        if(TERMAPP_CheckDateFormat(CardInfo.AppExpireDate)==NOK) return NOK;
        if(CardInfo.AppExpireDate[0]>0x49) ExpireDate[0]=0x19;
        else ExpireDate[0]=0x20;
        memcpy((uchar *)&ExpireDate[1],CardInfo.AppExpireDate,3);
        if(memcmp(currentDate,ExpireDate,4)>0)
            TermInfo.TVR[1]|=0x40;//set‘Expired application’bit 1
    }
    return OK;
}

uchar TERMAPP_CheckDateFormat(uchar* date)
{
    uchar i,k,n[3];

    LOGI("TERMAPP_CheckDateFormat() is called.\n");

    for(i=0;i<3;i++)
    {
        k=date[i];
        if((k&0x0F) > 9) return NOK;
        if(((k&0xF0)>>4) > 9) return NOK;
        n[i]=((k&0xF0)>>4)*10 + (k&0x0F);
    }
    if(n[1]<1 || n[1]>12) return NOK;
    if(n[2]<1 || n[2]>31) return NOK;
    return OK;
}

uchar TERMAPP_PreTreatQpboc()
    /**********************************************************************
    //      Byte1：                                  支持:1;       不支持:0
    //          bit8：是否支持非接触磁条           
    //          bit7：是否支持非接触PBOC
    //            bit6：是否支持非接触qPBOC
    //            bit5：是否支持接触PBOC
    //            bit4：终端是否脱机
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

{
    LOGI("TERMAPP_PreTreatQpboc() is called.\n");

    TermInfo.TermTransProp[0] |=0x28; //支持非接触qPBOC，脱机
    TermInfo.TermTransProp[1] &=0x0;  
    if(memcmp(TermInfo.AmtAuthNum,TermInfoEx.TransLimit,6)>=0)
    {
        return     JY_STOPSWIPE;
    }
    if(memcmp(TermInfo.AmtAuthNum,"\x0\x0\x0\x0\x0\x0",6)==0)    
    {
        return     JY_END;
    }
    if(memcmp(TermInfo.AmtAuthNum,TermInfoEx.OfflineLowestLimit,6)>=0)      
    {
        return  JY_END;
    }
    if(!memcmp(TermInfoEx.OfflineLowestLimit,"\x0\x0\x0\x0\x0\x0",6))
    {
         if(memcmp(TermInfo.AmtAuthBin,TermInfo.FloorLimit,4)>0)
         {
             TermInfo.TermTransProp[0] &=0xF7; 
         }
         else
         {
             if(!memcmp(TermInfo.AmtAuthNum,"\x0\x0\x0\x0\x0\x0",6))
            {
                TermInfo.TermTransProp[0] &=0xF7;
            }
            else
            {
                TermInfo.TermTransProp[0] |=0x08;

            }
         }
    }
    else
    {
        if(memcmp(TermInfo.AmtAuthNum,TermInfoEx.OfflineLowestLimit,6)>=0)
         {
             TermInfo.TermTransProp[0] &=0xF7; 
         }
         else
         {
             if(!memcmp(TermInfo.AmtAuthNum,"\x0\x0\x0\x0\x0\x0",6))
            {
                TermInfo.TermTransProp[0] &=0xF7;
            }
            else
            {
                TermInfo.TermTransProp[0] |=0x08;

            }
         }
    }

    if(memcmp(TermInfo.AmtAuthBin,TermInfo.FloorLimit,4)>0)        
    {                                                            
         TermInfo.TermTransProp[0] &=0xF7;                         
        TermInfo.TermTransProp[1] |=0x80;                     
    }                                                            
    return OK;
}
uchar TERMAPP_TermActAnaly()                    //终端行为分析
{
    uchar i,j,k;//TermAnaResult,CardAnaResult;//0-Denial,1-Online,2-Offline
    uchar bFitIAC,bFitTAC;

    LOGI("TERMAPP_TermActAnaly() is called.\n");

    if(ICCDataTable[MV_IACDenial].bExist==0)//IAC-denial not exist
    {
        memset(CardInfo.IACDenial,0,5);
    }
    if(ICCDataTable[MV_IACOnline].bExist==0)//IAC-online not exist
    {
        memset(CardInfo.IACOnline,0xFF,5);
    }
    if(ICCDataTable[MV_IACDefault].bExist==0)//IAC-default not exist
    {
        memset(CardInfo.IACDefault,0xFF,5);
    }

    if(TermDataTable[MV_TACDenial-TermDataBase].bExist==0)
    {
        memset(TermInfo.TACDenial,0,5);
    }
    if(TermDataTable[MV_TACOnline-TermDataBase].bExist==0)
    {
        memset(TermInfo.TACOnline,0,5);
        TermInfo.TACOnline[0]|=0xC8;//EMVapp,p35
    }
    if(TermDataTable[MV_TACDefault-TermDataBase].bExist==0)
    {
        memset(TermInfo.TACDefault,0,5);
        TermInfo.TACDefault[0]|=0xC8;
    }

    LOGI("TermInfo.VLPIndicator=0x%02X, bCardConfirmVLP=0x%02X\n", TermInfo.VLPIndicator, bCardConfirmVLP);

    LOGI("TermInfo.TVR[]:");
    menu_print(TermInfo.TVR, sizeof(TermInfo.TVR));

    LOGI("CardInfo.IACDenial[]:");
    menu_print(CardInfo.IACDenial, sizeof(CardInfo.IACDenial));

    LOGI("TermInfo.VLPTACDenial[]:");
    menu_print(TermInfo.VLPTACDenial, sizeof(TermInfo.VLPTACDenial));

    LOGI("TermInfo.TACDenial[]:");
    menu_print(TermInfo.TACDenial, sizeof(TermInfo.TACDenial));

    if(!memcmp(TermInfo.TVR,"\x00\x00\x00\x00\x00",5)) return OK;      
    
    bFitIAC=0;
    bFitTAC=0;
    for(i=0;i<5;i++)
    {
        k=TermInfo.TVR[i];
        if((k&CardInfo.IACDenial[i])!=0) bFitIAC=1;
        if(TermInfo.VLPIndicator==1 && bCardConfirmVLP==1)
        {
            if((k&TermInfo.VLPTACDenial[i])!=0) bFitTAC=1;
        }
        else
        {
            if((k&TermInfo.TACDenial[i])!=0) bFitTAC=1;
        }
    }

    LOGI("TERMAPP_TermActAnaly(): L1: bFitIAC=%d, bFitTAC=%d.\n", bFitIAC, bFitTAC);
    
    if(bFitIAC||bFitTAC) {return JY_REFUSE;}     


    LOGI("TermInfo.TermType=0x%02X, bAbleOnline=0x%02X\n", TermInfo.TermType, bAbleOnline);

    LOGI("CardInfo.IACOnline[]:");
    menu_print(CardInfo.IACOnline, sizeof(CardInfo.IACOnline));

    LOGI("TermInfo.TACOnline[]:");
    menu_print(TermInfo.TACOnline, sizeof(TermInfo.TACOnline));

    k=TermInfo.TermType&0x0F;
    if((k==1||k==2||k==4||k==5)&&(bAbleOnline))              //Terminal has Online capability
    {
        bFitIAC=0;
        bFitTAC=0;
        for(i=0;i<5;i++)
        {
            k=TermInfo.TVR[i];
            if((k & CardInfo.IACOnline[i])!=0) bFitIAC=1;
            if((k & TermInfo.TACOnline[i])!=0) bFitTAC=1;
        }
        if(bFitIAC||bFitTAC) return JY_REFUSE;              
        else return OK;                                     
    }

    LOGI("TERMAPP_TermActAnaly(): L2: bFitIAC=%d, bFitTAC=%d.\n", bFitIAC, bFitTAC);

    LOGI("TermInfo.VLPIndicator=0x%02X, bCardConfirmVLP=0x%02X\n", TermInfo.VLPIndicator, bCardConfirmVLP);

    LOGI("CardInfo.IACDefault[]:");
    menu_print(CardInfo.IACDefault, sizeof(CardInfo.IACDefault));

    LOGI("TermInfo.VLPTACDefault[]:");
    menu_print(TermInfo.VLPTACDefault, sizeof(TermInfo.VLPTACDefault));

    LOGI("TermInfo.TACDefault[]:");
    menu_print(TermInfo.TACDefault, sizeof(TermInfo.TACDefault));

    bFitIAC=0;
    bFitTAC=0;

    for(i=0;i<5;i++)
    {
        k=TermInfo.TVR[i];
        if((k&CardInfo.IACDefault[i])!=0) bFitIAC=1;
        if(TermInfo.VLPIndicator==1 && bCardConfirmVLP==1)
        {
            if((k&TermInfo.VLPTACDefault[i])!=0) bFitTAC=1;
        }
        else
        {
            if((k&TermInfo.TACDefault[i])!=0) bFitTAC=1;
        }
    }

    LOGI("TERMAPP_TermActAnaly(): L3: bFitIAC=%d, bFitTAC=%d.\n", bFitIAC, bFitTAC);

    if(bFitIAC||bFitTAC) return JY_REFUSE;
    else return OK;
}












