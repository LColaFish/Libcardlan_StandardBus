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

uchar TERMAPP_SelectDF(void);
uchar TERMAPP_SelectDDF(uchar indexDF);
uchar TERMAPP_ChooseApp(void);
uchar TERMAPP_SelectFromTerm(void);
static int TERMAPP_MatchTermAID(uchar * aid,uchar aidLen,TERMAPP *termapp_list,uchar termapp_list_Num);
void  TERMAPP_InsertApp(uchar i,uchar j);
uchar TERMAPP_ChooseAppList(void);
uchar TERMAPP_GetAppMatchNo(uchar CandidateNo);


extern char onlineflag;            //连线标记
extern char odaflag;            //走ODA标志

uchar TERMAPP_AppSelect()
{
    uchar PSEName[20];
    uchar ret,i,ret1=0,bResult;
    
    DFNum = 0;
    AppNum = 0;
    memset(AppBlockFlag, 0, 16);
    bRetFromInitApp = 0;

    for(i = 0; i < 16; i++)
    {
        AppPriority[i] = 0;
        AppSeq[i] = i;
    }
    /* 1.决定终端和卡片支持的应用 */
    {
        /* 方法1 : pse选择方法 */
        ret = TERMAPP_SelectDF();               //select DDF
        if(ret == APPSEL_PSEFOUND)              //PSE Select OK;
        {
            if(AppNum <= 0) 
            {
                LOGI("[ %s %d ] err_PSENoMatchApp\n",__FUNCTION__,__LINE__);
                return err_PSENoMatchApp;
            }  
        }
        else if(ret == APPSEL_PSENOTFOUND) //PSE in ICC not found,select from terminal list
        {
            /* 方法2 : 应用列表选择方法 */
            LOGI(" PSE IN ICC not found,select from termnal list \n");
            ret1 = TERMAPP_SelectFromTerm();                
            if(ret1 == OK)
            {
                if(AppNum <=  0)    //list select ok but no app
                {
                    LOGI("[ %s %d ] err_ListNoMatchApp\n",__FUNCTION__,__LINE__);
                    TermInfo.POSEntryMode = MAG_LAST_FAIL_IC;
                    return err_ListNoMatchApp;
                }  
            }
            else 
            {
                bResult = ret1;
            }
        }
        else 
        {
            bResult = ret;
        }

        if(bResult != OK)
        {
            LOGI("[ %s %d ] bResult error : %d \n",__FUNCTION__,__LINE__,bResult);
            return bResult;
        }
    }

    /*
        2.选择交易应用
        从候选应用列表中最终选出执行交易的应用
            无匹配应用 退出IC卡交易
            1个匹配应用 应用被选中
            多个匹配应用 = 提示持卡人选择或选择最高优先权的应用
    */
    {

        
        bResult = TERMAPP_ChooseApp();  
       
        if(bResult != OK)
        {
            LOGI("[ %s %d ] bResult error : %d \n",__FUNCTION__,__LINE__,bResult);
            return bResult;
        }
    }

    
    LOGI("[ %s %d ] Select AID:",__FUNCTION__,__LINE__);    
    menu_print(SelectedApp.AID, SelectedApp.AIDLen);

    LOGI("[ %s %d ] PDOL:\n",__FUNCTION__,__LINE__);      
    menu_print(SelectedApp.PDOL, SelectedApp.PDOLLen);
    
    {
        memcpy(CardInfo.AID, SelectedApp.AID, SelectedApp.AIDLen);
        CardInfo.AIDLen = SelectedApp.AIDLen;
        ICCDataTable[MV_ICC_AID].bExist = 1;
        memcpy(TermInfo.AID, SelectedApp.AID, SelectedApp.AIDLen);
        TermInfo.AIDLen = SelectedApp.AIDLen;
        TermDataTable[MV_TERM_AID - TermDataBase].bExist = 1;

        if(SelectedApp.AppLabelLen != 0)
        {
            memcpy(CardInfo.AppLabel,SelectedApp.AppLabel,SelectedApp.AppLabelLen);
            CardInfo.AppLabelLen = SelectedApp.AppLabelLen;
            ICCDataTable[MV_AppLabel].bExist = 1;
        }

        if(SelectedApp.PreferNameLen != 0)
        {
            memcpy(CardInfo.AppPreferName, SelectedApp.PreferName, SelectedApp.PreferNameLen);
            CardInfo.AppPreferNameLen = SelectedApp.PreferNameLen;
            ICCDataTable[MV_AppPreferName].bExist = 1;
        }

        if(SelectedApp.LangPreferLen != 0)
        {
            memcpy(CardInfo.LangPrefer, SelectedApp.LangPrefer, SelectedApp.LangPreferLen);
            CardInfo.LangPreferLen = SelectedApp.LangPreferLen;
            ICCDataTable[MV_LangPrefer].bExist = 1;
        }
    }
    

    return bResult;
}

uchar TERMAPP_SelectDF()
{
    uchar retCode;
    uchar curDFNum;

    curDFNum=0;
    DFListIcc[0].DFNameLen=14;
    
    if (0)
    {
        memcpy(DFListIcc[0].DFName,"1PAY.SYS.DDF01",14);
    }
    else
    {
        memcpy(DFListIcc[0].DFName, "2PAY.SYS.DDF01", 14);
    }
    
    DFListIcc[0].InsertAppNo=0;
    DFNum=1;

    while(curDFNum < DFNum)
    {
        retCode=TERMAPP_SelectDDF(curDFNum);
        if(retCode != OK)
        {
            return retCode;    
        }
        curDFNum++;
    }

    LOGI("TERMAPP_SelectDF(): AppNum = %d\n", AppNum);

    if(AppNum == 0) 
    {
        return APPSEL_PSENOTFOUND;
    }
    else
    {
        return APPSEL_PSEFOUND;    
    }
}

uchar TERMAPP_SelectDDF(uchar indexDF)
{
    uchar DFName[16],DFNameLen;
    uchar state[2]={0};
    uchar j,t,k;
    int index,indexADF,len,lenAEF,lenFCI,matchNum;
    int indexRecord,indexEntry,lenRecord,lenEntry,indexFCI;
    int indexIssuerDiscret,lenIssuerDiscret;
    int indexPSEAID,lenPSEAID;
    SELECT_RET selectRet;
    RECORD_PSE recordPSE;
    int i, length;
    
    DFNameLen=DFListIcc[indexDF].DFNameLen;
    memcpy(DFName,DFListIcc[indexDF].DFName,DFNameLen);

    
    if(TERMAPP_SelectApp(0x04, 0x00, DFNameLen, DFName,(uchar *)state)!=OK)                
    {
        return err_IccCommand;
    }

    if(state[0] !=0x90 || state[1] !=0x00)   //DF select found ok.
    {
        if(state[0]==0x6A && state[1]==0x81)   
        {
            LOGI("[ %s %d ] 卡片锁定或者选择（SELECT）命令不支持",__FUNCTION__,__LINE__);
            return err_CardBlock;
        }
        else if(state[0]==0x6A && state[1]==0x82)   
        {
            LOGI("[ %s %d ] 卡片中没有 PSE，卡片响应选择（SELECT）命令指出文件不存在",__FUNCTION__,__LINE__);
            return err_CardBlock;
        }
        else if(state[0]==0x6A && state[1]==0x83)   
        {
            LOGI("[ %s %d ] 卡片锁定或者选择（SELECT）命令不支持",__FUNCTION__,__LINE__);
            return err_CardBlock;
        }
        else
        {
            LOGI("[ %s %d ] error : state = %02X%02X\n",__FUNCTION__,__LINE__, state[0], state[1]);
            return err_CardBlock;
        }
    }

    /*选择 PSE 的响应报文（FCI）*/
    memset((uchar *)&recordPSE,0,sizeof(RECORD_PSE));
    index=0;
    if(SRCPUCardBuf[index] != 0x6F) /*FCI 模板*/
    {
        return err_IccDataFormat;
    }
    index++;

    if(SRCPUCardBuf[index] <= 127)
    {
        lenRecord=SRCPUCardBuf[index];
        index++;
    }
    else
    {
        lenRecord=0;
        t=SRCPUCardBuf[index]&0x7F;
        for(j=1;j<=t;j++)
        {
            lenRecord=lenRecord*256+SRCPUCardBuf[index+j];
        }
        index+=t+1;
    }
    indexRecord = index;

    while( index < indexRecord + lenRecord)
    {
        if(index>=SRCPUCardCount)
        {
            return err_IccDataFormat;
        }
        if(SRCPUCardBuf[index]==0xff || SRCPUCardBuf[index]==0x00) 
        {
            index++;
            continue;
        }
        else if(SRCPUCardBuf[index]==0x84)  /*DF 名*/
        {
            LOGI("TERMAPP_SelectDDF(): found tag: %02X\n", SRCPUCardBuf[index]);
    
            if(recordPSE.DFNameExist==1) return err_IccDataFormat;
            index++;
            recordPSE.DFNameLen=SRCPUCardBuf[index];
            index++;
            memcpy(recordPSE.DFName,SRCPUCardBuf+index,recordPSE.DFNameLen);
            index+=recordPSE.DFNameLen;
            recordPSE.DFNameExist=1;
            recordPSE.Type=2;
        }
        else if(SRCPUCardBuf[index] == 0xA5) /* FCI 数据专用模板 */
        {
            LOGI("TERMAPP_SelectDDF(): found tag: %02X\n", SRCPUCardBuf[index]);
        
            index++;
            if(SRCPUCardBuf[index]<=127)
            {
                lenFCI = SRCPUCardBuf[index];
                index++;
            }
            else
            {
                lenFCI = 0;
                t = SRCPUCardBuf[index]&0x7F;
                for(j = 1; j <= t; j++)
                {
                    lenFCI = lenFCI * 256 + SRCPUCardBuf[index + j];
                }
                index += t + 1;
            }
            indexFCI = index;
    
            while(index < indexFCI + lenFCI)
            {
                if(index >= SRCPUCardCount)
                {
                    return err_IccDataFormat;
                }
                if(SRCPUCardBuf[index] == 0xff || SRCPUCardBuf[index] == 0x00)
                {
                    index++;
                    continue;
                }
                else if(!memcmp(SRCPUCardBuf + index,"\xBF\x0C",2)) /*自发行自定义数据*/
                {
                    LOGI("TERMAPP_ReadSelectRetData(): found tag: %02X%02X\n", SRCPUCardBuf[index], SRCPUCardBuf[index + 1]);
                
                    if(recordPSE.DirDiscretExist == 1)
                    {
                        return err_IccDataFormat;
                    }
                    index += 2;
                    if(SRCPUCardBuf[index] <= 127)
                    {
                        lenIssuerDiscret=SRCPUCardBuf[index];
                        index++;
                    }
                    else
                    {
                        lenIssuerDiscret=0;
                        t=SRCPUCardBuf[index]&0x7F;
                        for(j = 1;j <= t; j++)
                        {
                            lenIssuerDiscret=lenIssuerDiscret*256+SRCPUCardBuf[index+j];
                        }
                        index+=t+1;
                    }
                    recordPSE.DirDiscretLen=lenIssuerDiscret;
                    memcpy(recordPSE.DirDiscret,SRCPUCardBuf+index,lenIssuerDiscret);
                    index+=lenIssuerDiscret;
                    recordPSE.DirDiscretExist=1;
                }
#if 0
                else if(SRCPUCardBuf[index]==0x88) /*目录基本文件的SFI*/
                {
                    /*skip next*/
                }
                else if(!memcmp(SRCPUCardBuf + index,"\x5F\x2D",2))/*语言选择*/
                {
                    /*skip next*/
                }
                else if(!memcmp(SRCPUCardBuf + index,"\x9F\x11",2))/*发卡行代码表索引*/
                {
                    /*skip next*/
                }
#endif
                else  //other unknown TLV data
                {
                    k=SRCPUCardBuf[index];
                    if((k & 0x1F) == 0x1F)
                    {
                        index++;
                    }
                    index++;
                    if(SRCPUCardBuf[index] <= 127)
                    {
                        len = SRCPUCardBuf[index];
                        index++;
                    }
                    else
                    {
                        len=0;
                        t=SRCPUCardBuf[index]&0x7F;
                        for(j = 1; j <= t; j++)
                        {
                            len = len * 256 + SRCPUCardBuf[index + j];
                        }
                        index += t + 1;
                    }
                    index += len;
                }
            }
            if(index != indexFCI + lenFCI)
            {
                return err_IccDataFormat;
            }
        }
        else 
        {
            k=SRCPUCardBuf[index];
            if((k&0x1F)==0x1F) index++;
            index++;
            if(SRCPUCardBuf[index]<=127)
            {
                len=SRCPUCardBuf[index];
                index++;
            }
            else
            {
                len=0;
                t=SRCPUCardBuf[index]&0x7F;
                for(j=1;j<=t;j++)
                {
                    len=len*256+SRCPUCardBuf[index+j];
                }
                index+=t+1;
            }
            index+=len;
        }
    }
    
    
    if(index!=indexRecord+lenRecord)
    {
         return err_IccDataFormat;
    }

    LOGI("TERMAPP_SelectDDF(): recordPSE:\n");
    length = (int)&recordPSE.DirDiscret[0] - (int)&recordPSE;
    menu_print((char *)&recordPSE, length);
    
    index=0;

    while(index<recordPSE.DirDiscretLen)
    {
        if(recordPSE.DirDiscretExist != 1)
        {
            return err_IccDataFormat;        
        }
        
        if(recordPSE.DirDiscret[index] == 0xFF || recordPSE.DirDiscret[index] == 0x00) 
        {
            index++;
            continue;
        }
        else if(recordPSE.DirDiscret[index] == 0x61) /*应用模板*/
        {
            LOGI("TERMAPP_SelectDDF(): found tag: %02X\n", recordPSE.DirDiscret[index]);
        
             index++;
             if(recordPSE.DirDiscret[index] <= 127)
             {
                   lenPSEAID=recordPSE.DirDiscret[index];
                  index++;
             }
             else
             {
                 lenPSEAID=0;
                 t=recordPSE.DirDiscret[index]&0x7F;
                 for(j=1;j<=t;j++)
                 {
                     lenPSEAID=lenPSEAID*256+recordPSE.DirDiscret[index+j];
                 }
                 index+=t+1;
             }
             indexPSEAID=index;

             while(index<indexPSEAID + lenPSEAID)
             {
                 if(index>=recordPSE.DirDiscretLen) return err_IccDataFormat;
                 if(recordPSE.DirDiscret[index]==0xFF ||recordPSE.DirDiscret[index]==0x00) {index++;continue;}
                 else if(recordPSE.DirDiscret[index]==0x4f) /*应用标识符（AID）-卡片*/
                 {
                    LOGI("TERMAPP_SelectDDF(): found tag: %02X\n", recordPSE.DirDiscret[index]);
                 
                    index++;
                    recordPSE.DFNameLen=recordPSE.DirDiscret[index];
                    index++;
                    memcpy(recordPSE.DFName,recordPSE.DirDiscret+index,recordPSE.DFNameLen);
                    index+=recordPSE.DFNameLen;
                    recordPSE.DFNameExist=1;
                    recordPSE.Type=2;
                 }
                 else if(recordPSE.DirDiscret[index]==0x50)  /*应用标签*/
                 {
                    LOGI("TERMAPP_SelectDDF(): found tag: %02X\n", recordPSE.DirDiscret[index]);
                 
                    index++;
                    recordPSE.AppLabelLen=recordPSE.DirDiscret[index];
                    index++;
                    memcpy(recordPSE.AppLabel,recordPSE.DirDiscret+index,recordPSE.AppLabelLen);
                    index+=recordPSE.AppLabelLen;
                    recordPSE.AppLabelExist = 1;
                 } 
                 else if(!memcmp(recordPSE.DirDiscret+index,"\x9F\x12",2)) /*应用优先名称*/
                 {
                    LOGI("TERMAPP_ReadSelectRetData(): found tag: %02X%02X\n", recordPSE.DirDiscret[index], recordPSE.DirDiscret[index + 1]);
                 
                    index+=2;
                    recordPSE.PreferNameLen = recordPSE.DirDiscret[index];
                    index++;
                    memcpy(recordPSE.PreferName,recordPSE.DirDiscret+index,recordPSE.PreferNameLen);
                    index+=recordPSE.PreferNameLen;
                    recordPSE.PreferNameExist=1;
                 }
                 else if(recordPSE.DirDiscret[index] == 0x87)   /*应用优先指示符*/
                 {
                    LOGI("TERMAPP_SelectDDF(): found tag: %02X\n", recordPSE.DirDiscret[index]);

                    index+=2;
                    recordPSE.Priority=recordPSE.DirDiscret[index];
                    index++;
                    recordPSE.PriorityExist=1;
                 }
#if 0
                 else if(recordPSE.DirDiscret[index] == 0x9D)   /*目录定义文件（DDF）名称*/
                 else if(recordPSE.DirDiscret[index] == 0x73)   /*命令自定义模板*/
#endif
                 else
                 {
                    k = recordPSE.DirDiscret[index];
                    if((k & 0x1F) == 0x1F)
                    {
                        index++;
                    }
                    index++;
                    k = recordPSE.DirDiscret[index];
                    index += k + 1;
                 }
            }
             
            if(index != indexPSEAID + lenPSEAID)
            {
                return err_IccDataFormat;
            }
            
            if(recordPSE.Type==2 && recordPSE.DFNameExist==1)
            {
                matchNum = MatchTermAID(recordPSE.DFName,recordPSE.DFNameLen,AppListTerm,TermAppNum);    
                if(matchNum < 0)
                {
                    continue;
                }
                
                memcpy(AppListCandidate[AppNum].AID,recordPSE.DFName,recordPSE.DFNameLen);                
                AppListCandidate[AppNum].AIDLen=recordPSE.DFNameLen;
                if(recordPSE.AppLabelExist==1)
                {    
                    memcpy(AppListCandidate[AppNum].AppLabel,recordPSE.AppLabel,recordPSE.AppLabelLen);
                    AppListCandidate[AppNum].AppLabelLen=recordPSE.AppLabelLen;
                }
                
                if(recordPSE.PreferNameExist==1)
                {
                    memcpy(AppListCandidate[AppNum].PreferName,recordPSE.PreferName,recordPSE.PreferNameLen);
                    AppListCandidate[AppNum].PreferNameLen = recordPSE.PreferNameLen;
                }
                if(recordPSE.PriorityExist==1)
                {
                    AppListCandidate[AppNum].Priority = recordPSE.Priority;
                }
                AppListCandidate[AppNum].bLocalName=AppListTerm[matchNum].bLocalName;
                AppListCandidate[AppNum].AppLocalNameLen=AppListTerm[matchNum].AppLocalNameLen;
                memcpy(AppListCandidate[AppNum].AppLocalName,AppListTerm[matchNum].AppLocalName,AppListTerm[matchNum].AppLocalNameLen);

                AppNum++;
                TERMAPP_InsertApp(DFListIcc[indexDF].InsertAppNo,AppNum - 1);
                DFListIcc[indexDF].InsertAppNo++;
            }
            else
            {
                return err_IccDataFormat;
            }
        }
#if 0
        else if(recordPSE.DirDiscret[index] == 0x73) /*目录自定义模板*/
        {

        }
#endif 
        else
        {
            k = recordPSE.DirDiscret[index];
            if(( k & 0x1F) == 0x1F ) 
            {
                index++;
            }
            index++;
            k = recordPSE.DirDiscret[index];
            index += k + 1;
        }
    }

    if(index != recordPSE.DirDiscretLen)
    {
        return err_IccDataFormat;
    }  
    LOGI("[%s %d]建立候选列表完成\n",__FUNCTION__,__LINE__);
    return OK;
}

static int TERMAPP_MatchTermAID(uchar * aid,uchar aidLen,TERMAPP *termapp_list,uchar termapp_list_Num)
{
    int i;

    LOGI("TERMAPP_MatchTermAID() is called. AID:\n");
    menu_print(aid, aidLen);
    
    LOGI("in func %s , TerAppnum=%d\n",__func__,termapp_list_Num);
    for(i = 0; i < termapp_list_Num; i++)
    {
        if(termapp_list[i].ASI==PARTIAL_MATCH)
        {
            if(!memcmp(aid,termapp_list[i].AID,termapp_list[i].AIDLen))
            {
                LOGI("TERMAPP_MatchTermAID(): AID matched!.\n");
                return i;
            }
        }
        else//exact match
        {
            if(!memcmp(aid,termapp_list[i].AID,aidLen))
            {
                LOGI("TERMAPP_MatchTermAID(): EXACT_MATCH AID matched!.\n");
                return i;
            }
        }
    }
    LOGI("TERMAPP_MatchTermAID(): i=%d\n",i);
    return -1;
}

uchar TERMAPP_ReadSelectRetData(SELECT_RET *selectRet,uchar *DataOut,int LenOut)
{
    uchar j,t,k;
    int index,indexFCI,indexFCIProp,len,lenFCI,lenFCIProp=0,lenFCIPropDisc,indexFCIPropDisc;
    int indexEntry,lenEntry;

    LOGI("TERMAPP_ReadSelectRetData() is called.\n");

    //串口发出调试信息"开始读TERMAPP_ReadSelectRetData"
    index = 0;
    if(DataOut[index] != 0x6F)
    {
        return NOK; //FCI template
    }
    index++;
    if(DataOut[index] <= 127)
    {
        lenFCI = DataOut[index];
        index++;
    }
    else
    {
       lenFCI = 0;
       t = DataOut[index] & 0x7F;
       for(j = 1; j <= t ; j++)
       {
               lenFCI = lenFCI * 256 + DataOut[index + j];
       }
       index += t + 1;
    }
    indexFCI = index;

    while(index < indexFCI + lenFCI)
    {
        if(index >= LenOut)
        {
            return err_IccDataFormat;
        }
        if(DataOut[index] == 0xff || DataOut[index] == 0x00) 
        {
            index++;
            continue;
        }
        else if(DataOut[index] == 0x84)
        {
            LOGI("TERMAPP_ReadSelectRetData(): found tag: %02X\n", DataOut[index]);

            index++;
            selectRet->DFNameLen = DataOut[index];
            memcpy(selectRet->DFName, DataOut + index + 1, DataOut[index]);
            memcpy(CardInfo.DFName, DataOut + index + 1, DataOut[index]);         
            CardInfo.DFNameLen = DataOut[index];                            
            index += selectRet->DFNameLen + 1;
            selectRet->DFNameExist = 1;
        }
        else if(DataOut[index] == 0xA5)
        {
            LOGI("TERMAPP_ReadSelectRetData(): found tag: %02X\n", DataOut[index]);
        
            selectRet->FCIPropExist = 1;
            index++;
            if(DataOut[index] <= 127)
            {
                lenFCIProp = DataOut[index];
                index++;
            }
            else
            {
                len = 0;
                t = DataOut[index] & 0x7F;
                for(j = 1;j <= t; j++)
                {
                    lenFCIProp = lenFCIProp * 256 + DataOut[index + j];
                }
                index += t + 1;
            }
            indexFCIProp = index;

            while(index < indexFCIProp + lenFCIProp)
            {
                if(index >= LenOut)
                {
                    return err_IccDataFormat;
                }
                if(DataOut[index] == 0xFF || DataOut[index] == 0x00) 
                {
                    index++;
                    continue;
                }
                else if(!memcmp(DataOut + index,"\x5F\x2D",2))
                {
                    LOGI("TERMAPP_ReadSelectRetData(): found tag: %02X%02X\n", DataOut[index], DataOut[index + 1]);

                    if(selectRet->LangPreferExist == 1)
                    {
                        return err_IccDataFormat;
                    }
                    index += 2;
                    selectRet->LangPreferLen = DataOut[index];
                    index++;
                    memcpy(selectRet->LangPrefer, DataOut + index, selectRet->LangPreferLen);
                    index += selectRet->LangPreferLen;
                    selectRet->LangPreferExist = 1;
                }
                else if(!memcmp(DataOut + index,"\x9F\x11",2))
                {
                    LOGI("TERMAPP_ReadSelectRetData(): found tag: %02X%02X\n", DataOut[index], DataOut[index + 1]);

                    if(selectRet->ICTIExist==1)
                    {
                        return err_IccDataFormat;
                    }
                    if(DataOut[index+2]!=1)
                    {
                        return err_EmvDataFormat;
                    }
                    index += 3;
                    selectRet->ICTI = DataOut[index];
                    index++;
                    selectRet->ICTIExist = 1;
                    CardInfo.ICTI = selectRet->ICTI;
                }
                else if(DataOut[index] == 0x50) //App Label
                {
                    LOGI("TERMAPP_ReadSelectRetData(): found tag: %02X\n", DataOut[index]);

                    if(selectRet->AppLabelExist==1)
                    {
                        return err_IccDataFormat;
                    }
                    index++;
                    selectRet->AppLabelLen=DataOut[index];
                    index++;
                    memcpy(selectRet->AppLabel, DataOut + index, selectRet->AppLabelLen);
                    index += selectRet->AppLabelLen;
                    selectRet->AppLabelExist = 1;
                }
                else if(DataOut[index] == 0x87)//App Priority Indicator
                {
                    LOGI("TERMAPP_ReadSelectRetData(): found tag: %02X\n", DataOut[index]);

                    if(selectRet->PriorityExist == 1)
                    {
                        return err_IccDataFormat;
                    }
                    if(DataOut[index+1] != 1)
                    {
                        return err_EmvDataFormat;//with wrong len,select next app.
                    }
                    index += 2;
                    selectRet->Priority = DataOut[index];
                    index++;
                    selectRet->PriorityExist = 1;
                }
                else if(!memcmp(DataOut + index,"\x9F\x12",2)) //App Prefer Name
                {
                    LOGI("TERMAPP_ReadSelectRetData(): found tag: %02X%02X\n", DataOut[index], DataOut[index + 1]);

                    if(selectRet->PreferNameExist == 1)
                    {
                        return err_IccDataFormat;
                    }
                    index += 2;
                    selectRet->PreferNameLen = DataOut[index];
                    index++;
                    memcpy(selectRet->PreferName, DataOut + index, selectRet->PreferNameLen);
                    index+=selectRet->PreferNameLen;
                    selectRet->PreferNameExist = 1;
                }
                else if(!memcmp(DataOut + index,"\x9F\x38",2)) //PDOL
                {
                    LOGI("TERMAPP_ReadSelectRetData(): found tag: %02X%02X\n", DataOut[index], DataOut[index + 1]);    
                    if(selectRet->PDOLExist == 1)
                    {
                        return NOK;
                    }
                    index += 2;
                    if(DataOut[index] <= 127) 
                    {
                        len=DataOut[index];
                        index++;
                    }
                    else
                    {
                        len = 0;
                        t = DataOut[index]&0x7F;
                        for(j = 1; j <= t; j++)
                        {
                            len = len * 256 + DataOut[index + j];
                        }
                        index += t + 1;
                    }
                    selectRet->PDOLLen = len;
                    memcpy(selectRet->PDOL,DataOut + index, selectRet->PDOLLen);
                    index+=selectRet->PDOLLen;
                    selectRet->PDOLExist = 1;

                    LOGI("selectRet->PDOLExist = %d \n", selectRet->PDOLExist);

                    
                }
                else if(!memcmp(DataOut+index,"\xBF\x0C",2))
                {
                    LOGI("TERMAPP_ReadSelectRetData(): found tag: %02X%02X\n", DataOut[index], DataOut[index + 1]);

                    index += 2;
                    if(DataOut[index] <= 127) 
                    {
                        len = DataOut[index];
                        index++;
                    }
                    else
                    {
                        len = 0;
                        t = DataOut[index] & 0x7F;
                        for(j = 1; j <= t; j++)
                        {
                            len = len * 256 + DataOut[index + j];
                        }
                        index += t + 1;
                    }
                    selectRet->IssuerDiscretLen = len;
                    memcpy(selectRet->IssuerDiscret, DataOut + index, selectRet->IssuerDiscretLen);
                    index += selectRet->IssuerDiscretLen;
                    selectRet->IssuerDiscretExist = 1;
                }
                else if(!memcmp(DataOut + index,"\xDF\x61",2))
                {
                    LOGI("TERMAPP_ReadSelectRetData(): found tag: %02X%02X\n", DataOut[index], DataOut[index + 1]);

                    index += 2;
                    if(DataOut[index] <= 127) 
                    {
                        len = DataOut[index];
                        index++;
                    }
                    else
                    {
                        len = 0;
                        t = DataOut[index] & 0x7F;
                        for(j = 1; j <= t; j++)
                        {
                            len = len * 256 + DataOut[index + j];
                        }
                        index += t + 1;
                    }
                    selectRet->OdaableLen = len;
                    memcpy(selectRet->Odaabledata, DataOut + index, selectRet->OdaableLen);
                    index+=selectRet->OdaableLen;
                    selectRet->OdaableExist = 1;
                }
                else
                {
                    k = DataOut[index];
                    if((k & 0x0F) == 0x1F)
                    {
                        index++;
                    }
                    index++;
                    k = DataOut[index];
                    index += k + 1;
                }
            }
            if(index != indexFCIProp+lenFCIProp)
            {
                return err_IccDataFormat;
            }
        }
        else 
        {
            k = DataOut[index];
            if((k & 0x0F) == 0x0F)
            {
                index++;
            }
            index++;
            k = DataOut[index];
            index += k + 1;
        }
    }
    if((selectRet->DFNameExist != 1) ||(selectRet->FCIPropExist != 1))
    {
        return NOK;
    }
    if(index != indexFCI+lenFCI)
    {
        return err_IccDataFormat;
    }
    
    return OK;
}

uchar TERMAPP_SelectFromTerm()
{
    uchar bSecond,i,j,t,ret,P2;
    uchar state[2];
    unsigned int index,len;

    SELECT_RET selectRet;

    LOGI("TERMAPP_SelectFromTerm() is called.\n");

    bSecond = 0;
    i = 0;

    while(i < TermAppNum)
    {
        if(bSecond)
        {
            P2 = 0x02;
        }
        else
        {
            P2 = 0x00;
        }

        ret = TERMAPP_SelectApp(0x04,P2,AppListTerm[i].AIDLen,AppListTerm[i].AID,state);
        if(ret != OK)
        {
            LOGI("[ %s %d ] error ret : %d \n",__FUNCTION__,__LINE__,ret);
            return err_IccCommand;
        }
        
        if(state[0] == 0x6A && state[1] == 0x81)
        {
            return err_CardBlock;
        }
        else if(state[0] == 0x62 && state[1] == 0x83)
        {
            AppBlockFlag[i] = 1;
            if(SRCPUCardCount == 0x0)
            {
                bSecond=0;
                i++;
                continue;
            }
            index=0;
            if(SRCPUCardBuf[index] != 0x6F)
            {
                return err_IccDataFormat;
            }
            index++;
            if(SRCPUCardBuf[index] <= 127)
            {
                index++;
            }
            else
            {
                index += (SRCPUCardBuf[index] & 0x7F) + 1;
            }
            
            if(SRCPUCardBuf[index] != 0x84)
            {
                return err_IccDataFormat;
            }
            index++;
            //compare AID with DF name in ICC
            if(!memcmp(AppListTerm[i].AID, SRCPUCardBuf + index, AppListTerm[i].AIDLen))
            {
                if(SRCPUCardBuf[index] == AppListTerm[i].AIDLen)
                {
                    i++;
                    bSecond=0;
                }
                else if(SRCPUCardBuf[index]>AppListTerm[i].AIDLen)
                {
                    bSecond=1;
                }
                else return err_IccDataFormat;
            }
            else return err_IccDataFormat;
        }
        else if(state[0]==0x90 && state[1]==0x00)
        {                                              
            memset((uchar *)&selectRet,0,sizeof(SELECT_RET));
            ret=TERMAPP_ReadSelectRetData(&selectRet,SRCPUCardBuf,SRCPUCardCount);
            if(ret != 0)
            {
                if(ret == err_EmvDataFormat)
                {
                    bSecond = 0;
                    i++;
                    continue;
                }
                else
                {
                    return err_IccDataFormat;
                }
             }
            
             if(selectRet.DFNameExist == 0 || selectRet.FCIPropExist == 0)
             {
                return err_IccDataFormat;
             }
             
             if(!memcmp(AppListTerm[i].AID, selectRet.DFName, AppListTerm[i].AIDLen))
             {
                 if(AppListTerm[i].AIDLen == selectRet.DFNameLen)
                {
                    bSecond = 0;
                    i++;
                }
                else
                {
                    bSecond = 1;
                    if(AppListTerm[i].ASI == EXACT_MATCH)
                    {
                        continue;
                    }
                }
             }
             else
             {
                 bSecond=0;
                i++;
                continue;
             }
             memcpy(AppListCandidate[AppNum].AID,selectRet.DFName,selectRet.DFNameLen);
             AppListCandidate[AppNum].AIDLen=selectRet.DFNameLen;

             memcpy(AppListCandidate[AppNum].AppLabel,selectRet.AppLabel,selectRet.AppLabelLen);
             AppListCandidate[AppNum].AppLabelLen=selectRet.AppLabelLen;

             if(selectRet.PriorityExist==1)
             {
                AppListCandidate[AppNum].Priority=selectRet.Priority;
             }

             if(selectRet.PDOLExist==1)
             {
                 memcpy(AppListCandidate[AppNum].PDOL,selectRet.PDOL,selectRet.PDOLLen);
                 AppListCandidate[AppNum].PDOLLen=selectRet.PDOLLen;
             }    

             if(selectRet.LangPreferExist==1)
             {
                 memcpy(AppListCandidate[AppNum].LangPrefer,selectRet.LangPrefer,selectRet.LangPreferLen);
                 AppListCandidate[AppNum].LangPreferLen=selectRet.LangPreferLen;
             }

             if(selectRet.ICTIExist==1)
             {
                 AppListCandidate[AppNum].ICTI=selectRet.ICTI;
                 if(AppListCandidate[AppNum].ICTI>0x16) return err_IccDataFormat;
                //added for test script 2CL.038.00,icti must be 1-10(BCD)
             }

             if(selectRet.PreferNameExist==1)
             {
                 memcpy(AppListCandidate[AppNum].PreferName,selectRet.PreferName,selectRet.PreferNameLen);
                 AppListCandidate[AppNum].PreferNameLen=selectRet.PreferNameLen;
             }
            
             if(selectRet.IssuerDiscretExist==1)
             {
                 memcpy(AppListCandidate[AppNum].IssuerDiscret,selectRet.IssuerDiscret,selectRet.IssuerDiscretLen);
                 AppListCandidate[AppNum].IssuerDiscretLen=selectRet.IssuerDiscretLen;
             }

             AppNum++;
        }
        else
        {
            i++;
            bSecond=0;
        }        
    }
    return OK;
}

void TERMAPP_AppCopy(uchar i,uchar j)
{
    memcpy((uchar *)&AppListCandidate[i],(uchar *)&AppListCandidate[j],sizeof(APPDATA));
}

//insert application of number j to the position of i.
void TERMAPP_InsertApp(uchar i,uchar j)
{
    uchar m;
    APPDATA tempApp;

    LOGI("TERMAPP_InsertApp() is called, i = %d, j = %d.\n", i, j);

    if(j<=i) return;
    
    memcpy((uchar *)&tempApp,(uchar *)&AppListCandidate[j],sizeof(APPDATA));
    for(m=j-1;m>=i;m++)
        TERMAPP_AppCopy(m+1,m);
    memcpy((uchar *)&AppListCandidate[i],(uchar *)&tempApp,sizeof(APPDATA));
}

uchar TERMAPP_ChooseApp()
{
    uchar retCode = OK,i;

    if(AppNum <= 0) 
    {
        retCode = err_NoAppSel;
        LOGI("[ %s %d ] error : %d .\n",err_NoAppSel);
        return retCode;
    }

    while((retCode = TERMAPP_ChooseAppList()) == err_AppReselect)
    {
        if(AppNum <= 0)
        {
            return err_NoAppSel;
        }

        for(i = SelectedAppNo; i < AppNum; i++)
        {
            TERMAPP_AppCopy(i, i + 1);
        }
        AppNum--;
    }
    return retCode;
}


uchar TERMAPP_ChooseAppList()
{
    uchar i,j,t,bPriority,bConfirm,temp,ret,k,missPriorityNo=0;
    unsigned int len;
    uchar state[2];
    SELECT_RET selectRet;

    LOGI("TERMAPP_ChooseAppList() is called.\n");

    SelectedAppNo=0xff;
    if(SelectedAppNo > AppNum)
    {
        bPriority=1;
        bConfirm=1;
        for(i=0;i<AppNum;i++)
        {
            AppPriority[i]=AppListCandidate[i].Priority & 0x0F;
            AppSeq[i]=i;
        }
        if(AppNum==1)
        {
            SelectedAppNo=0;
        }
        else
        {
            for(j=0;j<AppNum-1;j++)
            {
                for(i=0;i<AppNum-1-j;i++)
                {
                    if(AppPriority[i]>AppPriority[i+1])
                    {
                        temp=AppPriority[i];
                        AppPriority[i]=AppPriority[i+1];
                        AppPriority[i+1]=temp;
                        temp=AppSeq[i];
                        AppSeq[i]=AppSeq[i+1];
                        AppSeq[i+1]=temp;
                    }                    
                }
            }
            for(i=0;i<AppNum;i++)
            {
                if(AppPriority[i]==0) {bPriority=0;missPriorityNo++;} //priority missing
                if(i<AppNum-1)
                {
                    if(AppPriority[i]==AppPriority[i+1]) bPriority=0; //the same priority    (had sorted)
                }        
            }
            if(bPriority==1)                                          //all priority exist
            {
                SelectedAppNo=AppSeq[0];
            }
            else
            {
                if(missPriorityNo==AppNum) SelectedAppNo=AppSeq[0];   //all Priority missing
                if(missPriorityNo<AppNum)  SelectedAppNo=AppSeq[missPriorityNo]; //partial Priority missing,choose the one which has a priority
            }
        }
    }

    if(SelectedAppNo > AppNum)
    {
        return JY_END;
    }
    else //An App is selected,send Select Command to ICC again.
    {
        TERMAPP_SelectApp(0x04,0x00,AppListCandidate[SelectedAppNo].AIDLen,AppListCandidate[SelectedAppNo].AID,state);
        if(state[0]==0x90 && state[1]==0x00)    
        {
            memset((uchar *)&selectRet,0,sizeof(SELECT_RET));
            if(TERMAPP_ReadSelectRetData(&selectRet,SRCPUCardBuf,SRCPUCardCount)!=OK)        
            {    
                if(AppNum>1)
                {
                    for(i=SelectedAppNo;i<AppNum-1;i++)
                        TERMAPP_AppCopy(i,i+1);
                    AppNum--;
                    if(AppNum>0)
                    {
                        TERMAPP_ChooseAppList();
                    }
                    else 
                    {
                        return JY_END;
                    }
                 }
                 else 
                 {
                    return JY_END;
                 }
            }

            memcpy(SelectedApp.AID,AppListCandidate[SelectedAppNo].AID,AppListCandidate[SelectedAppNo].AIDLen);
            SelectedApp.AIDLen=AppListCandidate[SelectedAppNo].AIDLen;

            memcpy((uchar *)(SelectedApp.AppLabel),(uchar *)(selectRet.AppLabel),selectRet.AppLabelLen);
            SelectedApp.AppLabelLen=selectRet.AppLabelLen;

            if(selectRet.PriorityExist==1)
                SelectedApp.Priority=selectRet.Priority;        
            if(selectRet.PDOLExist==1)
            {    
                memcpy(SelectedApp.PDOL,selectRet.PDOL,selectRet.PDOLLen);
                SelectedApp.PDOLLen=selectRet.PDOLLen;

                memcpy(CardInfo.PDOL,selectRet.PDOL,selectRet.PDOLLen);
                CardInfo.PDOLLen=selectRet.PDOLLen;
                ICCDataTable[MV_PDOL].bExist=1;
            }            
            if(selectRet.LangPreferExist==1)
            {
                memcpy(SelectedApp.LangPrefer,selectRet.LangPrefer,selectRet.LangPreferLen);
                SelectedApp.LangPreferLen=selectRet.LangPreferLen;
            }
            if(selectRet.ICTIExist==1)
            {
                SelectedApp.ICTI=selectRet.ICTI;
            }
            if(selectRet.PreferNameExist==1)
            {
                memcpy(SelectedApp.PreferName,selectRet.PreferName,selectRet.PreferNameLen);
                SelectedApp.PreferNameLen=selectRet.PreferNameLen;
            }            
            if(selectRet.IssuerDiscretExist==1)
            {
                memcpy(SelectedApp.IssuerDiscret,selectRet.IssuerDiscret,selectRet.IssuerDiscretLen);
                SelectedApp.IssuerDiscretLen=selectRet.IssuerDiscretLen;


                odaflag = 0;
                ////////////新增用于查询是否支持ODA///////
                /*
                memset((uchar *)&selectRet,0,sizeof(SELECT_RET));
                 TERMAPP_ReadSelectRetData(&selectRet,SelectedApp.IssuerDiscret,SelectedApp.IssuerDiscretLen);
                 if(selectRet.OdaableExist==1)
                {
                    memcpy(CardInfo.Odadata,selectRet.Odaabledata,selectRet.OdaableLen);
                    if((CardInfo.Odadata[0]&0x40)==0x40)
                        odaflag = 1;
                     }
                     */

                DebugPrintChar("BF OC:", SelectedApp.IssuerDiscret, SelectedApp.IssuerDiscretLen);
                
                for(i=0;i<SelectedApp.IssuerDiscretLen;i++)
                {
                    if((SelectedApp.IssuerDiscret[i]==0xdf)&&(SelectedApp.IssuerDiscret[i+1]==0x61))
                            break;
                    }
                CardInfo.Odadata[0] = SelectedApp.IssuerDiscret[i+3];
                if((CardInfo.Odadata[0]&0x40)==0x40)
                        odaflag = 1;
                        
                
                LOGI("odaflag=%d:CardInfo.Odadata[0]=%02x\n",odaflag,CardInfo.Odadata[0]);
                
                //////////////////////////////
                
            }
        }
        else if(state[0]==0x62 && state[1]==0x81)  //card block or command not support
        {
            return err_CardBlock;
        }
        else
        {
            if(state[0]==0x62 && state[1]==0x83)   //App is block
            {
                k=TERMAPP_GetAppMatchNo(SelectedAppNo);
                if(k>15) k=15;
                AppBlockFlag[k]=1;
            }
            bRetFromInitApp=1;
            return err_AppReselect;
        }
    }
    return OK;
}

uchar TERMAPP_GetAppMatchNo(uchar CandidateNo)
{
    uchar i;

    LOGI("TERMAPP_GetAppMatchNo() is called.\n");

    for(i=0;i<TermAppNum;i++)
    {
        if(!memcmp(AppListCandidate[CandidateNo].AID,AppListTerm[i].AID,AppListTerm[i].AIDLen))
            return i;
    }
    return 0xff;
}








