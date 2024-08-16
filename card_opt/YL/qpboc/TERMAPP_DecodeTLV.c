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

uchar TERMAPP_DecodeTLV(uchar *TLVBuf,u32 Length)
{
    u32 i=0,tagIndex=0;
    u32 datalen=0;
    u32 tlvbuf_len,lenTemp;

    LOGI("TERMAPP_DecodeTLV() is called.\n");

    tlvbuf_len = Length;
    while(tlvbuf_len - 2 > i)
    {
        if(memcmp(TLVBuf+i,"\x90\x00",2)==0 ) return OK;
        if(TLVBuf[i]==0xFF ||TLVBuf[i]==0x00) {i++;continue;}
        
        if((TLVBuf[i] & 0x20) != 0x20) 
        {
            if((TLVBuf[i] & 0x1F) == 0x1F)
            {
                tagIndex=i;
                if(TLVBuf[i+2]<0x80)  {datalen=TLVBuf[i+2];i=i+3;}
                else if(TLVBuf[i+2]==0x81) {datalen=TLVBuf[i+3];i=i+4;}
                else if(TLVBuf[i+2]==0x82) {datalen=256*TLVBuf[i+3]+TLVBuf[i+4];i=i+5;}
                else return JY_END;                 

                switch(TLVBuf[tagIndex])
                {
                    case 0x9f:
                    {
                        LOGI("TERMAPP_DecodeTLV(): found tag: %02X%02X\n", TLVBuf[tagIndex], TLVBuf[tagIndex + 1]);

                        switch(TLVBuf[tagIndex+1])
                        {
                            case 0x07 :memcpy(CardInfo.AUC,TLVBuf+i,datalen);if(ICCDataTable[MV_AUC].bExist==1) return JY_END;ICCDataTable[MV_AUC].bExist=1;break;                          //应用用途控制，2        
                            case 0x08 :memcpy(CardInfo.AppVer,TLVBuf+i,datalen);if(ICCDataTable[MV_ICC_AppVer].bExist==1) return JY_END;ICCDataTable[MV_ICC_AppVer].bExist=1;break;                  //应用版本号，2
                            case 0x0b :memcpy(CardInfo.CardHoldNameExt,TLVBuf+i,datalen);if(ICCDataTable[MV_CardholderNameExt].bExist==1)return JY_END;CardInfo.CardHoldNameExtLen=datalen;ICCDataTable[MV_CardholderNameExt].bExist=1;break;
                            case 0x0d :memcpy(CardInfo.IACDefault,TLVBuf+i,datalen);if(ICCDataTable[MV_IACDefault].bExist==1) return JY_END;ICCDataTable[MV_IACDefault].bExist=1;break;                   //IAC同意，5            
                            case 0x0e :memcpy(CardInfo.IACDenial,TLVBuf+i,datalen);if(ICCDataTable[MV_IACDenial].bExist==1) return JY_END;ICCDataTable[MV_IACDenial].bExist=1;break;                  //IAC拒绝，5                
                            case 0x0f :memcpy(CardInfo.IACOnline,TLVBuf+i,datalen);if(ICCDataTable[MV_IACOnline].bExist==1) return JY_END;ICCDataTable[MV_IACOnline].bExist=1;break;                  //IAC联机，5                    
                            
                            case 0x10 :memcpy(CardInfo.IssuAppData,TLVBuf+i,datalen);CardInfo.IssuAppDataLen=datalen;if(ICCDataTable[MV_IssuAppData].bExist==1) return JY_END;ICCDataTable[MV_IssuAppData].bExist=1;break;                            
                            case 0x11 :memcpy(&CardInfo.ICTI,TLVBuf+i,datalen);if(ICCDataTable[MV_ICTI].bExist==1) return JY_END;ICCDataTable[MV_ICTI].bExist=1;break;          //发卡行代码索引表，1
                            case 0x13 :memcpy(CardInfo.LOATC,TLVBuf+i,datalen);if(ICCDataTable[MV_LOATCReg].bExist==1) return JY_END;ICCDataTable[MV_LOATCReg].bExist=1;break; //上次联机应用交易计数器，2            
                            case 0x14 :memcpy(&CardInfo.LCOL,TLVBuf+i,datalen);if(ICCDataTable[MV_LCOL].bExist==1) return JY_END;ICCDataTable[MV_LCOL].bExist=1;break;          //连续脱机消费下限，1                        
                            
                            case 0x23 :memcpy(&CardInfo.UCOL,TLVBuf+i,datalen);if(ICCDataTable[MV_UCOL].bExist==1) return JY_END;ICCDataTable[MV_UCOL].bExist=1;break;          //连续脱机消费上限，1                        
                            case 0x26 :memcpy(CardInfo.AppCrypt,TLVBuf+i,datalen);if(ICCDataTable[MV_AppCrypt].bExist==1) return JY_END;ICCDataTable[MV_AppCrypt].bExist=1;break;      //应用密文，8                                    
                            
                            case 0x32 :CardInfo.IPKExpLen=datalen;if(datalen==0x01) CardInfo.IPKExp[2]=TLVBuf[i];if(datalen==0x03) memcpy(CardInfo.IPKExp,TLVBuf+i,datalen);if(ICCDataTable[MV_IPKExp].bExist==1) return JY_END;ICCDataTable[MV_IPKExp].bExist=1;break; 
                            case 0x36 :memcpy(CardInfo.ATC,TLVBuf+i,datalen);if(ICCDataTable[MV_ATC].bExist==1) return JY_END;ICCDataTable[MV_ATC].bExist=1;break;                          //应用交易计数器，2        
                            
                            case 0x42 :memcpy(CardInfo.AppCurcyCode,TLVBuf+i,datalen);if(ICCDataTable[MV_AppCurcyCode].bExist==1) return JY_END;ICCDataTable[MV_AppCurcyCode].bExist=1;break;            //应用货币代码，2
                            case 0x46 :memcpy(CardInfo.ICCPKCert,TLVBuf+i,datalen);CardInfo.ICCPKCertLen=datalen;if(ICCDataTable[MV_ICCPKCert].bExist==1) return JY_END;ICCDataTable[MV_ICCPKCert].bExist=1;break;            
                            case 0x47 :CardInfo.ICCPKExpLen=datalen;if(datalen==0x01) CardInfo.ICCPKExp[2]=TLVBuf[i];if(datalen==0x03) memcpy(CardInfo.ICCPKExp,TLVBuf+i,datalen);if(ICCDataTable[MV_ICCPKExp].bExist==1) return JY_END;ICCDataTable[MV_ICCPKExp].bExist=1;break; 
                            case 0x48 :memcpy(CardInfo.ICCPKRem,TLVBuf+i,datalen);CardInfo.ICCPKRemLen=datalen;if(ICCDataTable[MV_ICCPKRem].bExist==1) return JY_END;ICCDataTable[MV_ICCPKRem].bExist=1;break;                                
                            case 0x49 :memcpy(CardInfo.DDOL,TLVBuf+i,datalen);CardInfo.DDOLLen=datalen;if(ICCDataTable[MV_DDOL].bExist==1) return JY_END;ICCDataTable[MV_DDOL].bExist=1;break;                                
                            case 0x4a :memcpy(CardInfo.SDATagList,TLVBuf+i,datalen);CardInfo.SDATagListLen=datalen;if(ICCDataTable[MV_SDATagList].bExist==1) return JY_END;ICCDataTable[MV_SDATagList].bExist=1;break;            
                            case 0x4b :
                                memcpy(CardInfo.SignDynAppData,TLVBuf+i,datalen);
                                CardInfo.SignDynAppDataLen=datalen;
                                if(ICCDataTable[MV_SignDynAppData].bExist==1) return JY_END;
                                ICCDataTable[MV_SignDynAppData].bExist=1;
                                break;                                
                            
                            case 0x5d :
                                memcpy(CardInfo.OfflineAmount,TLVBuf+i,datalen);                                
                                if(ICCDataTable[MV_OfflineAmount].bExist==1) return JY_END;                            
                                ICCDataTable[MV_OfflineAmount].bExist=1;break;              //可用脱机消费余额，6                            
                            
                            case 0x74 :memcpy(CardInfo.VLPIssuAuthorCode,TLVBuf+i,datalen);if(ICCDataTable[MV_VLPIssuAuthorCode].bExist==1) return JY_END;ICCDataTable[MV_VLPIssuAuthorCode].bExist=1;break;                //电子现金发卡行授权码，6
                            default:break;
                            }
                    }
                    break;
                    case 0x5f  :
                    {
                        LOGI("TERMAPP_DecodeTLV(): found tag: %02X%02X\n", TLVBuf[tagIndex], TLVBuf[tagIndex + 1]);
                        
                        switch(TLVBuf[tagIndex+1])
                        {
                            case 0x20  :memcpy(CardInfo.CardHoldName,TLVBuf+i,datalen);CardInfo.CardHoldNameLen=datalen;if(ICCDataTable[MV_CardholderName].bExist==1) return JY_END;ICCDataTable[MV_CardholderName].bExist=1;break;                    //持卡人姓名，26
                            case 0x24  :memcpy(CardInfo.AppExpireDate,TLVBuf+i,datalen);if(ICCDataTable[MV_AppExpireDate].bExist==1) return JY_END;ICCDataTable[MV_AppExpireDate].bExist=1;break;                //应用失效日期,3                
                            case 0x25  :memcpy(CardInfo.AppEffectDate,TLVBuf+i,datalen);if(ICCDataTable[MV_AppEffectDate].bExist==1) return JY_END;ICCDataTable[MV_AppEffectDate].bExist=1;break;            
                            case 0x28  :memcpy(CardInfo.IssuCountryCode,TLVBuf+i,datalen);if(ICCDataTable[MV_IssuCountryCode].bExist==1) return JY_END;ICCDataTable[MV_IssuCountryCode].bExist=1;break;                //发卡行国家代码,2                                            
                            case 0x2d  :memcpy(CardInfo.LangPrefer,TLVBuf+i,datalen);CardInfo.LangPreferLen=datalen;if(ICCDataTable[MV_LangPrefer].bExist==1) return JY_END;ICCDataTable[MV_LangPrefer].bExist=1;break;    //首选语言，2-8                
                            case 0x30  :memcpy(CardInfo.ServiceCode,TLVBuf+i,datalen);if(ICCDataTable[MV_ServiceCode].bExist==1) return JY_END;ICCDataTable[MV_ServiceCode].bExist=1;break;                    //服务代码，2
                            case 0x34  :memcpy(&CardInfo.PANSeq,TLVBuf+i,datalen);ICCDataTable[MV_PANSeqNum].bExist=1;break;                        //应用主账号序列号，1
                            default:break;
                        }
                    }    
                    break;        
                }
                i=i+datalen;                                //point the next tag
            }
            else                                            //tag has one byte
            {
                //analyse tlvbuf_len field
                tagIndex=i;
                if(TLVBuf[i+1]<0x80)  {datalen=TLVBuf[i+1];i=i+2;}                    //len<128,len=TLVBuf[i+1]                
                else if(TLVBuf[i+1]==0x81) {datalen=TLVBuf[i+2];i=i+3;}                    //128<=len<256,TLVBuf[i+1]=0x81，len=TLVBuf[i+2]            
                else if(TLVBuf[i+1]==0x82) {datalen=256*TLVBuf[i+2]+TLVBuf[i+3];i=i+4;}      //256<=len<65536,TLVBuf[i+1]=0x82,len=256*TLVBuf[i+2]+TLVBuf[i+3]
                else return NOK;

                LOGI("TERMAPP_DecodeTLV(): found tag: %02X\n", TLVBuf[tagIndex]);
                
                //analyse value field
                switch(TLVBuf[tagIndex])
                {
                    case 0x57  :memcpy(CardInfo.Track2Equ,TLVBuf+i,datalen);CardInfo.Track2EquLen=datalen;if(ICCDataTable[MV_Track2Equivalent].bExist==1) return JY_END;ICCDataTable[MV_Track2Equivalent].bExist=1;break;                   //2磁道等价数据，19
                    case 0x5a  :memcpy(CardInfo.PAN,TLVBuf+i,datalen);if(ICCDataTable[MV_PAN].bExist==1) return JY_END;ICCDataTable[MV_PAN].bExist=1;CardInfo.PANLen=datalen;break;
                    case 0x82  :memcpy(CardInfo.AIP,TLVBuf+i,datalen);if(ICCDataTable[MV_AIP].bExist==1) return JY_END;ICCDataTable[MV_AIP].bExist=1;break;                       //应用交互特征，2
                    case 0x8e  :memcpy(CardInfo.CVMList,TLVBuf+i,datalen);CardInfo.CVMListLen=datalen;if(ICCDataTable[MV_CVMList].bExist==1) return JY_END;ICCDataTable[MV_CVMList].bExist=1;break;
                    case 0x8F  :memcpy(&CardInfo.CAPKI,TLVBuf+i,datalen);if(ICCDataTable[MV_ICCAPKI].bExist==1) return JY_END;ICCDataTable[MV_ICCAPKI].bExist=1;break;               //CA公钥索引，1
                    case 0x90  :memcpy(CardInfo.IPKCert,TLVBuf+i,datalen);CardInfo.IPKCertLen=datalen;if(ICCDataTable[MV_IPKCert].bExist==1) return JY_END;ICCDataTable[MV_IPKCert].bExist=1;break;
                    case 0x92  :memcpy(CardInfo.IPKRem,TLVBuf+i,datalen);CardInfo.IPKRemLen=datalen;if(ICCDataTable[MV_IPKRem].bExist==1) return JY_END;ICCDataTable[MV_IPKRem].bExist=1;break;
                    case 0x93  :memcpy(CardInfo.SignStatAppData,TLVBuf+i,datalen);CardInfo.SignStatAppDataLen=datalen;if(ICCDataTable[MV_SignStatAppData].bExist==1) return JY_END;ICCDataTable[MV_SignStatAppData].bExist=1;break;                    
                    case 0x94  :memcpy(CardInfo.AFL,TLVBuf+i,datalen);CardInfo.AFLLen=datalen;if(ICCDataTable[MV_AFL].bExist==1) return JY_END;ICCDataTable[MV_AFL].bExist=1;AFL_Num=(CardInfo.AFLLen)/4;memcpy((uchar *)AFL,CardInfo.AFL,datalen);break;                                    
                    case 0x8c  :memcpy(CardInfo.CDOL1,TLVBuf+i,datalen);CardInfo.CDOL1Len = datalen;if(ICCDataTable[MV_CDOL1].bExist==1) return JY_END;ICCDataTable[MV_CDOL1].bExist=1;break;           //CDOL  wal 20160630
                    default:break;
                }
                i=i+datalen;
            }
        }
        else                                                                  
        {            
             if((TLVBuf[i] & 0x1F) == 0x1F)                    //tag has two bytes
             {    
                 tagIndex=i;
                if(TLVBuf[i+2]<0x80)  {datalen=TLVBuf[i+2];i=i+3;}                          //len<128,len=TLVBuf[i+1]                
                else if(TLVBuf[i+2]==0x81) {datalen=TLVBuf[i+3];i=i+4;}                     //128<=len<256,TLVBuf[i+1]=0x81，len=TLVBuf[i+2]                
                else if(TLVBuf[i+2]==0x82) {datalen=256*TLVBuf[i+3]+TLVBuf[i+4];i=i+5;}     //256<=len<65536,TLVBuf[i+1]=0x82,len=256*TLVBuf[i+2]+TLVBuf[i+3]
                else return JY_END;                        

                if(TERMAPP_DecodeTLV(TLVBuf+i,datalen) == OK)
                {
                    i=i+datalen;
                }
                else    return JY_END;    
             }
             else
             {
                tagIndex=i;
                if(TLVBuf[i+1]<0x80)  {datalen=TLVBuf[i+1];i=i+2;}                          //len<128,len=TLVBuf[i+1]                
                else if(TLVBuf[i+1]==0x81) {datalen=TLVBuf[i+2];i=i+3;}                     //128<=len<256,TLVBuf[i+1]=0x81，len=TLVBuf[i+2]                
                else if(TLVBuf[i+1]==0x82) {datalen=256*TLVBuf[i+2]+TLVBuf[i+3];i=i+4;}     //256<=len<65536,TLVBuf[i+1]=0x82,len=256*TLVBuf[i+2]+TLVBuf[i+3]                        
                else return JY_END;
                if(TERMAPP_DecodeTLV(TLVBuf+i,datalen) == OK)
                {
                    i=i+datalen;
                }
                else
                    return JY_END;
             }
        }
    }
    return OK;
}

uchar  DecodeTLVLen(uchar *TLVBuf,u32 Length)
{
    u32 tlvbuf_len;

    LOGI("DecodeTLVLen() is called, TLVBuf[1] = %d, Length = %d.\n", TLVBuf[1], Length);

    tlvbuf_len = Length;

    if(TLVBuf[1] < 0x80) 
    {
        if((tlvbuf_len - 2) != TLVBuf[1]) 
        {
            return JY_END;
        }
    }
    
    if(TLVBuf[1] == 0x81) 
    {
        if((tlvbuf_len - 3) != TLVBuf[2]) 
        {
            return JY_END;
        }
    }
    
    if(TLVBuf[1] == 0x82) 
    {
        if((tlvbuf_len - 4) != TLVBuf[2] * 256 + TLVBuf[3]) 
        {
            return JY_END;
        }
    }
    
    return OK;
}

uchar TERMAPP_ExpireDate(uchar* ExpireDate)
{
    uchar buf[8],CurDate[3],ExpDate[3];

    LOGI("TERMAPP_ExpireDate() is called.\n");

    GetTime(buf);
    if(buf[0] > 0x49)
    {
        CurDate[0]=0x19;
    }
    else
    {
        CurDate[0]=0x20;
    }
    
    memcpy((uchar*)&CurDate[1], buf, 2);

    if(ExpireDate[0] > 0x49)
    {
        ExpDate[0] = 0x19;         
    }
    else
    {
        ExpDate[0] = 0x20;                        
    }
    ExpDate[1] = ExpireDate[0];                      
    ExpDate[2] = ExpireDate[1];                      

    menu_print(ExpireDate, 6);

    if(memcmp(CurDate,ExpDate,3) > 0) 
    {
        return JY_REFUSE;
    }
    else
    {
        return OK;
    }
}

void _encode_tlv(uchar *DOL, uchar *dest, uchar *source, uchar *length, uchar condition)
{
    if (condition && *length)
    {
        if ((DOL[0] & 0x1F) == 0x1F)
        {
            // two bytes tag
            dest[0] = DOL[0];
            dest[1] = DOL[1];
            dest[2] = *length;
            memcpy(&dest[3], source, *length);
            *length += 3;
        }
        else
        {
            // 1 byte tag
            dest[0] = DOL[0];
            dest[1] = *length;
            memcpy(&dest[2], source, *length);            
            *length += 2;
        }
    }
    else
    {
        if ((DOL[0] & 0x1F) == 0x1F)
        {
            printf("_encode_tlv(): no data for tag: %02X%02X\n", DOL[0], DOL[1]);
        }
        else
        {
            printf("_encode_tlv(): no data for tag: %02X\n", DOL[0]);
        }
        *length = 0;
    }
}


/*
    根据dol列表构建tag信息集合
    
*/
void TERMAPP_EncodeTLV(uchar *DOL, uchar DOLLen, uchar *TLVBuf, uchar *TLVLen)
{
    
    uchar index,indexOut;
    uchar datalen;
    uchar tagFound;

    LOGI("TERMAPP_EncodeTLV() is called.\n");

    *TLVLen = 0;
    index=0;
    indexOut=0;

    // Tag
    TLVBuf[0] = 0xA5;
    //TLVBuf[1] = *TLVLen;
    indexOut += 2;

    while(index<DOLLen)  //Process DOL
    {
        tagFound = 1;
        if((DOL[index] & 0x1F) == 0x1F)
        {
            // two bytes tag
            datalen = DOL[index+2];
            switch(DOL[index])
            {
                case 0x9f:
                {
                    switch(DOL[index+1])
                    {
                        case 0x26: 
                            {
                                // {/*"AppCrypt",0,*/"\x9F\x26",255,8,0,0,0,0},/* 0 */
                                _encode_tlv(&DOL[index], &TLVBuf[indexOut], CardInfo.AppCrypt, &datalen, ICCDataTable[MV_AppCrypt].bExist);
                            
                                indexOut += datalen;
                            }
                            break;

                        case 0x27:
                            // {/*"CryptInfoData",0,*/"\x9F\x27",255,1,0,1222,0,0},/* 25 */
                            if (ICCDataTable[MV_CryptInfoData].bExist)
                            {
                                _encode_tlv(&DOL[index], &TLVBuf[indexOut], &CardInfo.CryptInfo, &datalen, ICCDataTable[MV_CryptInfoData].bExist);
                                indexOut += datalen;
                            }
                            else
                            {
                                TLVBuf[indexOut] = 0x9F;
                                TLVBuf[indexOut + 1] = 0x27;
                                TLVBuf[indexOut + 2] = 0x01;
                                TLVBuf[indexOut + 3] = 0x80; // default crypt info
                                indexOut += 4;
                            }
                            break;

                        case 0x10:
                            //     {/*"IssuAppData",0,*/"\x9F\x10",0,32,0,2129,0,0},/* 40 */
                            datalen = CardInfo.IssuAppDataLen;
                            _encode_tlv(&DOL[index], &TLVBuf[indexOut], CardInfo.IssuAppData, &datalen, ICCDataTable[MV_IssuAppData].bExist);
                            indexOut += datalen;
                            break;

                        case 0x37:
                            //     {/*"UnpredictNum",1,*/"\x9F\x37",255,4,0,628,0,0},
                            _encode_tlv(&DOL[index], &TLVBuf[indexOut], TermInfo.UnpredictNum, &datalen, 1);
                            indexOut += datalen;
                            break;
                            
                        case 0x36:
                            // {/*"ATC",0,*/"\x9F\x36",255,2,0,383,0,0},
                            _encode_tlv(&DOL[index], &TLVBuf[indexOut], CardInfo.ATC, &datalen, ICCDataTable[MV_ATC].bExist);
                            indexOut += datalen;
                            break;

                            
                        case 0x02:
                            // {/*"AmtAuthorNum",1,*/"\x9F\x02",255,6,1,532,0,0},/* 45 */
                            _encode_tlv(&DOL[index], &TLVBuf[indexOut], TermInfo.AmtAuthNum, &datalen, 1);
                            indexOut += datalen;
                            break;

                        case 0x1A:
                            // {/*"TermCountryCode",1,*/"\x9F\x1A",255,2,1,47,0,0},
                            _encode_tlv(&DOL[index], &TLVBuf[indexOut], TermInfo.CountryCode, &datalen, 1);
                            indexOut += datalen;
                            break;
                            
                        case 0x03:
                            // {/*"AmtOtherNum",1,*/"\x9F\x03",255,6,1,542,0,0},
                            _encode_tlv(&DOL[index], &TLVBuf[indexOut], TermInfo.AmtOtherNum, &datalen, 1);
                            indexOut += datalen;
                            break;
                            
                        case 0x33:
                            // {/*"TermCapab",1,*/"\x9F\x33",255,3,0,6,0,0},
                            _encode_tlv(&DOL[index], &TLVBuf[indexOut], TermInfo.TermCapab, &datalen, 1);
                            indexOut += datalen;
                            break;
                            
                        case 0x34:
                            //{/*"CVR",1,*/"\x9F\x34",255,3,0,577,0,0},
                            _encode_tlv(&DOL[index], &TLVBuf[indexOut], TermInfo.CVMResult, &datalen, 1);
                            indexOut += datalen;
                            break;
                            
                        case 0x35:
                            // {/*"TermType",1,*/"\x9F\x35",255,1,1,58,0,0},
                            _encode_tlv(&DOL[index], &TLVBuf[indexOut], &TermInfo.TermType, &datalen, 1);
                            indexOut += datalen;
                            break;
                            
                        case 0x1E:
                            // {/*"IFDSerNum",1,*/"\x9F\x1E",255,8,0,14,0,0},
                            _encode_tlv(&DOL[index], &TLVBuf[indexOut], TermInfo.IFD_SN, &datalen, 1);
                            indexOut += datalen;
                            break;
                            
                        case 0x09:
                            // {/*"AppVerNum",1,*/"\x9F\x09",255,2,0,59,0,0},/* 10 */
                            _encode_tlv(&DOL[index], &TLVBuf[indexOut], TermInfo.AppVer, &datalen, 1);
                            indexOut += datalen;
                            break;
                            
                        case 0x41:
                            // {/*"TransSeqCount",1,*/"\x9F\x41",255,4,1,524,0,0},
                            _encode_tlv(&DOL[index], &TLVBuf[indexOut], TermInfo.TransSeqCount, &datalen, 1);
                            indexOut += datalen;
                            break;
                            
                        case 0x74:
                            // {/*"VLPIssuAuthorCode",0,*/"\x9F\x74",255,6,0,4016,0,0},
                            _encode_tlv(&DOL[index], &TLVBuf[indexOut], CardInfo.VLPIssuAuthorCode, &datalen, ICCDataTable[MV_VLPIssuAuthorCode].bExist);
                            indexOut += datalen;
                            break;
                            
                        case 0x63:
                            break;
                            
                        default:
                            tagFound = 0;
                            printf("TERMAPP_EncodeTLV(): Unknown tag: %02X%02X\n", DOL[index], DOL[index + 1]);
                            break;
                    }
                }
                break;

                case 0x5f:
                {
                    switch(DOL[index+1])
                    {
                        case 0x2A:
                            // {/*"TransCurrencyCode",1,*/"\x5F\x2A",255,2,1,61,0,0},
                            _encode_tlv(&DOL[index], &TLVBuf[indexOut], TermInfo.TransCurcyCode, &datalen, 1);
                            indexOut += datalen;
                            break;

                        case 0x24:
                            // {/*"AppExpireDate",0,*/"\x5F\x24",255,3,1,47,0,0},//mandatory /* 5 */
                            _encode_tlv(&DOL[index], &TLVBuf[indexOut], CardInfo.AppExpireDate, &datalen, ICCDataTable[MV_AppExpireDate].bExist);
                            indexOut += datalen;
                            break;

                        case 0x34:
                            //     {/*"PANSeqNum",0,*/"\x5F\x34",255,1,1,367,0,0},
                            _encode_tlv(&DOL[index], &TLVBuf[indexOut], &CardInfo.PANSeq, &datalen, ICCDataTable[MV_PANSeqNum].bExist);
                            indexOut += datalen;
                            break;

                         default:
                            tagFound = 0;
                            printf("TERMAPP_EncodeTLV(): Unknown tag: %02X%02X\n", DOL[index], DOL[index + 1]);
                            break;
                    }
                }    
                break;    

                default:
                    tagFound = 0;
                    printf("TERMAPP_EncodeTLV(): Unknown tag: %02X%02X\n", DOL[index], DOL[index + 1]);
                    break;
            }

            if (tagFound)
            {
                index += 3;
            }
        }
        else                                            //tag has one byte
        {
            //analyse value field
            datalen = DOL[index+1];            
            switch(DOL[index])
            {
                case 0x57:
                    // {/*"Track2Equivalent",0,*/"\x57\x00",0,19,0,3608,0,0},
                    datalen = CardInfo.Track2EquLen;
                    _encode_tlv(&DOL[index], &TLVBuf[indexOut], CardInfo.Track2Equ, &datalen, ICCDataTable[MV_Track2Equivalent].bExist);
                    indexOut += datalen;
                    break;
                // added by taeguk added tag 5A    
                case 0x5A:
                    datalen = CardInfo.PANLen;
                    _encode_tlv(&DOL[index], &TLVBuf[indexOut], CardInfo.PAN, &datalen, ICCDataTable[MV_PAN].bExist);
                    indexOut += datalen;
                    break;

                case 0x95:
                    // {/*"TVR",1,*/"\x95\x00",255,5,0,594,0,0},/* 55 */
                    _encode_tlv(&DOL[index], &TLVBuf[indexOut], TermInfo.TVR, &datalen, 1);
                    indexOut += datalen;
                    break;

                case 0x9A:
                    // {/*"TransDate",1,*/"\x9A\x00",255,3,1,602,0,0},
                    _encode_tlv(&DOL[index], &TLVBuf[indexOut], TermInfo.TransDate, &datalen, 1);
                    indexOut += datalen;
                    break;

                case 0x9C:
                    // {/*"TransTypeValue",1,*/"\x9C\x00",255,1,1,83,0,0},
                    _encode_tlv(&DOL[index], &TLVBuf[indexOut], &TermInfo.TransTypeValue, &datalen, 1);
                    indexOut += datalen;
                    break;


                case 0x82:
                    // {/*"AIP",0,*/"\x82\x00",255,2,0,320,0,0},
                    _encode_tlv(&DOL[index], &TLVBuf[indexOut], CardInfo.AIP, &datalen, ICCDataTable[MV_AIP].bExist);
                    indexOut += datalen;
                    break;

                case 0x84:
                    // {/*"DFName",0,*/"\x84\x00",5,16,0,1226,0,0},
                    datalen = CardInfo.DFNameLen;
                    _encode_tlv(&DOL[index], &TLVBuf[indexOut], CardInfo.DFName, &datalen, CardInfo.DFNameLen);
                    indexOut += datalen;
                    break;
                    
                case 0x91:
                    // {/*"IssuerAuthenData",1,*/"\x91\x00",8,16,0,633,0,0},
                    datalen = TermInfo.IssuerAuthenDataLen;
                    _encode_tlv(&DOL[index], &TLVBuf[indexOut], TermInfo.IssuerAuthenData, &datalen, TermInfo.IssuerAuthenDataLen);
                    indexOut += datalen;
                    break;
                    
                case 0x8A:
                    // {/*"AuthorRespCode",1,*/"\x8A\x00",255,2,0,575,0,0},
                    _encode_tlv(&DOL[index], &TLVBuf[indexOut], TermInfo.AuthRespCode, &datalen, TermDataTable[MV_AuthorRespCode-TermDataBase].bExist);
                    indexOut += datalen;
                    break;            

                default:
                    tagFound = 0;
                    printf("TERMAPP_EncodeTLV(): Unknown tag: %02X\n", DOL[index]);
                    break;
            }

            if (tagFound)
            {
                index += 2;
            }
        }
    }

    *TLVLen = indexOut;
    TLVBuf[1] = *TLVLen - 2;
}

