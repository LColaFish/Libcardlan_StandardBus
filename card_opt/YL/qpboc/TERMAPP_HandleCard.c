#include "includes.h"
//---------#include "../include/apparel.h"
//---------#include "../gui/InitSystem.h"
//---------#include "../cpucard_section/pbocsection.h"
//---------#include "../gui/Freesign.h"
//---------#include "../QR/QRcode.h"
//---------#include "../Display/fbtools.h"
//---------#include "../gui/GeneralConfig.h"
//---------#include "../record/record.h"
//---------#include "../gui/GeneralConfig.h"
//---------#include "../gui/OnlineRecharge.h"
//---------#include "../parameter/Blacklist.h"
#include "common/cardlan_StandardBus_tpye.h"
#include "libcardlan_StandardBus_util.h"
#define EMV_META_LEN    7



#if defined(ANDROID_CODE_DEBUG)
#define TARGET_ANDROID
#endif
#if defined(NDK_CODE_DEBUG)
//#define TARGET_DEBUG
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
SysTime Time;
SectionFarPar Section;
LongUnon HostValue;
LongUnon HostValueS;
LongUnon HostValueH;
LongUnon HostValueY;
LongUnon DecValue;
LongUnon DevSID;                        //终端机流水号
JackRegal Sector;
MerchantConfig mchantConf;
unsigned char use_jiaotong_stander;

unsigned char bStolenDis;
unsigned char g_FgCardHandleOrNot;

unsigned char g_FgCardBinlimitnum;//卡柄文件取数据长度标志,如果是0的时候，取值为4字节，为1的时候取7字节    

unsigned char freedomflag;

uchar MLastSelectedSnr[5];
unsigned int StolenAmount;

ulong BankAmount;      //余额
unsigned char g_FgNoDiscount;
LongUnon Fixvalue;
unsigned char g_FgQRCodeRcvAck;
int rechargeflag;    //冲正标志    1要求冲正，0无冲正


//---------





unsigned char PBOC_SwipeCardError;
unsigned char OriginalCardType;
char g_bIblock;
LongUnon HostValuej,HostValuet;





//---------extern MerchantConfig mchantConf;
//---------extern L9_BusConfig   CLconfig;

//---------extern BLACKLIST *BankBlackList ;


extern char onlineflag;
extern char odaflag;
extern char enableodaflag;

uchar BankcardFlag;                //银行卡标识，1代表该卡是银行卡，0表示不确定,2 表示不是银行卡
uchar BusAppDiscRec[64];           //Bus application discount information record 实际使用47
uchar BusAppDiscRecLen;
uchar BusAppInfo[150];             //公交应用信息记录 ，实际使用128
uchar BusAppInfoLen;


//---------extern struct SwipcardInfo swipinfo;
extern uchar BankcardFlag;





uchar TERMAPP_HandleQpbocProcessAfterOfflineDataAuth(int channel);
uchar TERMAPP_SaveTransactionData();
uchar TERMAPP_SaveTransactionData_shij();
uchar TERMAPP_SaveTransactionData_Tao();
uchar TERMAPP_GetOnlineAppDesCode();


    
extern int PBOC_Aboard(unsigned char *stolen);
extern int WriteM1(unsigned int type);
extern int CheckIsStolen(void);
extern unsigned char QpbocCardDiscounts(unsigned char Type);
extern unsigned char ResetM1FormCPU(void);
extern unsigned char WriteQPBOCFlag(unsigned char flg);

static int TERMAPP_HandleQpbocProcess(int channel);
static int TERMAPP_Init_App(     int channel);
static int TERMAPP_Handle_Gpo_Data(        unsigned char *pt_gpo_data,int gpo_data_len,int channel);


uchar SearchCard() 
{
    uchar i,status=0,atq[6];

    LOGI("SearchCard() is called.\n");

    status = CardReset(&SRCPUCardBuf[0], &SRCPUCardCount, 1);
    if (status == 0x20)    
    {
        //串口发出调试信息"卡片复位数据"    
        status = OK;
    //---------    g_bIblock = 0;
    }

    return status;
}

int WENSAH_GET_CARD_INFO(int cmd)
{
    int result;
    unsigned char cpucmd[1024];
    unsigned char psamcmd[1024];
    unsigned char bcdbuff[256];
    int len;
    int i,j;
    
    LOGI("-------------in func [%s] LINE [%d] \n",__func__,__LINE__);

    if(cmd)
    {    
        //复位cpu卡
        result = SearchCard();
        if(result!=0)
            return -1;
        //cpu卡选择mf
        LOGI("in func [%s] LINE [%d] \n",__func__,__LINE__);
        memset(cpucmd,0,sizeof(cpucmd));
        sprintf(cpucmd,"%s","00A4040008A000000333010101");
        len=strlen(cpucmd);
        memset(bcdbuff,0,sizeof(bcdbuff));
        asc_to_bcd(bcdbuff, cpucmd, len);
        len=len/2;
        LOGI("cpu >> len=%d :%s \n",len*2,cpucmd);
        result = MifarePro_SendCom(bcdbuff,len);
        if(result == 0)
        {
            len=SRCPUCardCount ;
            LOGI("cpu<< len =%d :",len);
            for(i=0;i<len;i++)
                LOGI("%02x",SRCPUCardBuf[i]);
            LOGI("\n");
        }
        else
        {
            LOGI("sw1 sw2 =%02x%02x\n",SRCPUCardBuf[SRCPUCardCount-2],SRCPUCardBuf[SRCPUCardCount-1]);
            return -1;
        }
    }


    //读优惠信息
    LOGI("in func [%s] LINE [%d] \n",__func__,__LINE__);
    memset(cpucmd,0,sizeof(cpucmd));
    sprintf(cpucmd,"%s","80B400B0027300");
    len=strlen(cpucmd);
    memset(bcdbuff,0,sizeof(bcdbuff));
    asc_to_bcd(bcdbuff, cpucmd, len);
    len=len/2;
    LOGI("cpu >> len=%d :%s \n",len*2,cpucmd);
    result = MifarePro_SendCom(bcdbuff,len);
    if(result == 0)
    {
        len=SRCPUCardCount ;
        LOGI("cpu<< len =%d :",len);
        for(i=0;i<len;i++)
        {
            LOGI("%02x",SRCPUCardBuf[i]);
        }
        LOGI("\n");
        BusAppDiscRecLen=len-2;
        memcpy(BusAppDiscRec,SRCPUCardBuf,BusAppDiscRecLen);
    }
    else
    {
        LOGI("sw1 sw2 =%02x%02x\n",SRCPUCardBuf[SRCPUCardCount-2],SRCPUCardBuf[SRCPUCardCount-1]);
        return -1;
    }

    return 0;
}


int WENSHAN_UPDATE_0x16()
{
    unsigned char Scatter_factor[20]={0};
    int i,status;
    int result;
    unsigned char mac[4];
    int index=0;
    int len;
    LOGI("%s is called \n",__func__);

    /*组装卡号分散因子*/
    unsigned char ascbuff[256]={0};
    unsigned char cmd[64]={0};
    char bcdbuff[256];
    char outbuff[256];
    
    
    char * pt1=NULL,*pt2=NULL;
    hex_2_ascii(CardInfo.Track2Equ,ascbuff,CardInfo.Track2EquLen);
    pt1=strstr(ascbuff,"D");
    if(pt1!=NULL)
        *pt1='\0';
    LOGI("cardno = %s \n",ascbuff);
    pt1-=14;
    asc_to_bcd(bcdbuff,pt1,14);
    
    LOGI("tag 57,len:%d data:",CardInfo.Track2EquLen);
    for(i=0;i<CardInfo.Track2EquLen;i++)
        LOGI("%02x",CardInfo.Track2Equ[i]);
    LOGI("\n");

    LOGI("tag 5f34 ,len:1 data:%02x \n",CardInfo.PANSeq);
    LOGI("tag 9f36 ,len:2 data:%02x %02x \n",CardInfo.ATC[0],CardInfo.ATC[1]);

//    memcpy(Scatter_factor,CardInfo.Track2Equ+1,7);
    memcpy(Scatter_factor,bcdbuff,7);
    Scatter_factor[7]=CardInfo.PANSeq;
    
    LOGI("分散因子,len:%d data:",8);
    for(i=0;i<8;i++)
        LOGI("%02x",Scatter_factor[i]);
    LOGI("\n");

    
    /*psam计算mac*/
    /*
    
    LOGI("psam 选择 MF \n");
    strcat(cmd,"00A404000E315041592E5359532E4444463031");
    LOGI("psam >>:%s\n",cmd);
    len=strlen(cmd);
    memset(bcdbuff,0,sizeof(bcdbuff));
    asc_to_bcd(bcdbuff, cmd, len);
    len=len/2;
    memset(outbuff,0,sizeof(outbuff));
    status=PsamCos(bcdbuff,outbuff, &len,1);
    if((status == OK)&&(outbuff[len-2]== 0x90)&&(outbuff[len-1]== 0x00))
    {
        LOGI("psam <<:");
        for(i=0;i<len;i++)
        {
            LOGI("%02x",outbuff[i]);
        }
        LOGI("\n");
    }
    else
    {
        LOGI("psam sw1 sw2:%02x %02x\n",outbuff[0],outbuff[1]);
        return -1;
    }
    
    

    LOGI("psam 选择DF01\n ");
    memset(cmd,0,sizeof(cmd));
    strcat(cmd,"00A4040009A00000000386980101");
    LOGI("psam >>:%s\n",cmd);
    len=strlen(cmd);
    memset(bcdbuff,0,sizeof(bcdbuff));
    asc_to_bcd(bcdbuff, cmd, len);
    len=len/2;
    memset(outbuff,0,sizeof(outbuff));
    status=PsamCos(bcdbuff,outbuff, &len,1);
    if((status == OK)&&(outbuff[len-2]== 0x90)&&(outbuff[len-1]== 0x00))
    {
        LOGI("psam <<:");
        for(i=0;i<len;i++)
        {
            LOGI("%02x",outbuff[i]);
        }
        LOGI("\n");
    }
    else
    {
        LOGI("psam sw1 sw2:%02x %02x\n",outbuff[0],outbuff[1]);
        return -1;
    }
    
*/
    LOGI("通用des计算初始化\n");
    memset(bcdbuff,0,sizeof(bcdbuff));
    memcpy(bcdbuff,"\x80\x1A\x28\x00\x08",5);
    memcpy(bcdbuff+5,Scatter_factor,8);
    len=8+5;
    status=PsamCos(bcdbuff,outbuff, &len,1);
    if((status == OK)&&(outbuff[len-2]== 0x90)&&(outbuff[len-1]== 0x00))
    {
        LOGI("psam <<:");
        for(i=0;i<len;i++)
        {
            LOGI("%02x",outbuff[i]);
        }
        LOGI("\n");
    }
    else
    {
        LOGI("psam sw1 sw2:%02x %02x\n",outbuff[0],outbuff[1]);
        return -1;
    }


    /*修改0x16文件优惠记录 记录长度47 ,剩余次数-1，使用次数加1*/ 
    //BusAppDiscRec[BusAppDiscRecLen-1]++;        
    LongUnon LeftNum;
    ShortUnon Used;
    Used.intbuf[0]=BusAppDiscRec[34];
    Used.intbuf[1]=BusAppDiscRec[33];
    LOGI("已使用次数:%d \n",Used.i);
    //memcpy(LeftNum.longbuf,BusAppDiscRec+35,4);
    LeftNum.longbuf[3]=BusAppDiscRec[35];
    LeftNum.longbuf[2]=BusAppDiscRec[36];
    LeftNum.longbuf[1]=BusAppDiscRec[37];
    LeftNum.longbuf[0]=BusAppDiscRec[38];    
    LOGI("剩余次数:%d \n",LeftNum.i);
    Used.i++;
    LeftNum.i--;
    BusAppDiscRec[34]=Used.intbuf[0];
    BusAppDiscRec[33]=Used.intbuf[1];
    BusAppDiscRec[35]=LeftNum.longbuf[3];
    BusAppDiscRec[36]=LeftNum.longbuf[2];
    BusAppDiscRec[37]=LeftNum.longbuf[1];
    BusAppDiscRec[38]=LeftNum.longbuf[0];
    
    index=0;
    LOGI("psam 计算优惠记录 mac \n");
    memset(bcdbuff,0,sizeof(bcdbuff));
    memcpy(bcdbuff+index,"\x80\xFA\x05\x00\x40",5);         // 0x40=8+5+47+4=64
    index+=5;
    memcpy(bcdbuff+index,"\x00\x00\x00\x00\x00\x00",6);
    index+=6;
    bcdbuff[index++]=CardInfo.ATC[0];
    bcdbuff[index++]=CardInfo.ATC[1];
    memcpy(bcdbuff+index,"\x84\xDE\x00\xB0\x33",5);                 //这里要和下面的更新命令的一致
    index+=5;
    memcpy(bcdbuff+index,BusAppDiscRec,BusAppDiscRecLen);            
    index+=BusAppDiscRecLen;
    memcpy(bcdbuff+index,"\x80\x00\x00\x00",4); 
    index+=4;
    len=index;
    memset(outbuff,0,sizeof(outbuff));
    LOGI("psam >>:");
    for(i=0;i<len;i++)
    {
        LOGI("%02x",bcdbuff[i]);
    }
    LOGI("\n");
    status=PsamCos(bcdbuff,outbuff, &len,1);
    if(outbuff[len-2]==0x90 && outbuff[len-1]==0x00)
    {
        memcpy(mac,outbuff,4);
        LOGI("psam 计算优惠mac成功 mac:%02x%02x%02x%02x\n",mac[0],mac[1],mac[2],mac[3]);    
    }
    else
    {
        LOGI("psam 取响应数据 \n");
        memset(bcdbuff,0,sizeof(bcdbuff));
        memcpy(bcdbuff,"\x00\xC0\x00\x00\x04",5);
        len=5;
        PsamCos(bcdbuff,outbuff, &len,1);
        LOGI("psam <<:");
        for(i=0;i<len;i++)
        {
            LOGI("%02x",outbuff[i]);
        }
        LOGI("\n");
        if(outbuff[len-2]==0x90 && outbuff[len-1]==0x00)
        {
            memcpy(mac,outbuff,4);
            LOGI("psam 计算优惠mac成功 mac:%02x%02x%02x%02x\n",mac[0],mac[1],mac[2],mac[3]);    
        }
        else
            return -1;
    }


    
    /*将修改后的记录更新到文件中*/
    LOGI("更新优惠记录\n");
    index=0;
    memset(bcdbuff,0,sizeof(bcdbuff));
    memcpy(bcdbuff+index,"\x84\xDE\x00\xB0\x33",5);         
    index+=5;
    memcpy(bcdbuff+index,BusAppDiscRec,BusAppDiscRecLen);    
    index+=BusAppDiscRecLen;
    memcpy(bcdbuff+index,mac,4);
    index+=4;
    bcdbuff[index++]=0;
    len=index;
    result = MifarePro_SendCom(bcdbuff,len);
    if(result == 0 && SRCPUCardBuf[SRCPUCardCount-2]==0x90 && SRCPUCardBuf[SRCPUCardCount-1]==0x00)
    {
        len=SRCPUCardCount ;
        LOGI("cpu<< len =%d :",len);
        for(i=0;i<len;i++)
            LOGI("%02x",SRCPUCardBuf[i]);
        LOGI("\n");
    }
    else
    {
            LOGI("sw1 sw2 =%02x%02x\n",SRCPUCardBuf[SRCPUCardCount-2],SRCPUCardBuf[SRCPUCardCount-1]);
            return -1;
    }    

    
    return 0;
    
}

/*

    channel: 0 电子现金
             1 双免 
             2 ODA

*/
uchar TERMAPP_HandleCard(int channel, unsigned int  money) 
{
    struct timeval first, second;
    struct  timeval    tv;
    struct  timezone   tz;
    int ret = OK;
    LongUnon tmp;
    //unsigned char pbocCardTypeLUT[] = {1, 7, 2, 3, 4, 5, 6, 1, 8, 9};  
    //unsigned char cardtype;
    use_jiaotong_stander = 0;
    OriginalCardType = 0;
    char channelbak = channel;

    {
        char Timebuf[7];
        Rd_time (Timebuf + 1);
        Timebuf[0] = 0x20;
        Time.year = Timebuf[1];
        Time.month = Timebuf[2];
        Time.day = Timebuf[3];
        Time.hour = Timebuf[4];
        Time.min = Timebuf[5];
        Time.sec = Timebuf[6];

        LOGI("--- in func %s ,channl= %d money :%d \n",__func__,channel,money);        
        HostValue.i = money;
        TERMAPP_QPBOCTransInit(DevSID.i + 1, HostValue.i,channelbak);       
    }
   
    int i;   
    unsigned char ar_step[10];
    unsigned char step_len;

    switch(channel)
    {
        case 0:  //电子现金
            {
                step_len = 1;
                memcpy(ar_step,"\x01",step_len);
            }
            break;
        case 1: //双免
            {
                step_len = 1;
                memcpy(ar_step,"\x01",step_len);
            }
            break;
        case 2: //ODA
            {
                step_len = 2;
                memcpy(ar_step,"\x01\x07",step_len);
            }
            break;
        default:
            {
                return JY_END;
            }
            break;
    }

    for(i = 0 ;i < step_len; i++)
    {
        switch(ar_step[i])
        {
            case 1: //启动(必备)
                {
                    {
                        ret = TERMAPP_HandleQpbocProcess(channelbak);        
                        if (ret != OK)
                        { 
                            LOGI("--- in func %s line %d,error ret = %d\n",__func__,__LINE__,ret);
                            return JY_END;
                        }
                        
                        /* debug */
                        {
                            unsigned int amount = 0;
                            amount = bcd_to_bin(CardInfo.OfflineAmount, 6);
                            LOGI("TERMAPP_HandleCard(): Transaction succeed! MV_OfflineAmount = %d\n", amount);        
                        }
    
                    }
                }
                break;
            case 2: //脱机数据认证    (可选)
                {
                    //静态数据认证
                    {
                    
                    }
                    //动态数据认证
                    {
                    
                    }
                    //复合动态数据认证
                    {
                    
                    }
                }
                break;
            case 3: //处理限制(必备)
                {
                
                }
                break;
            case 4://持卡人验证(必备)
                {
                
                }
                break;
            case 5://终端风险管理(必备)
                {
                
                }
                break;
            case 6://终端行为分析(必备)
                {
                
                }
                break;
            case 7://卡片行为分析(必备)
                {
                    /*取应用密文*/  
                    ret = TERMAPP_GetOnlineAppDesCode();
                    LOGI("TERMAPP_HandleCard(): TERMAPP_GetOnlineAppDesCode() ret = %d.\n", ret);
                    short temp;
                    temp = CardInfo.ATC[0] << 8 | CardInfo.ATC[1];
                    if((temp == 0)||(ret != OK))
                    {
                        LOGI("--- in func %s line %d,error ret = %d\n",__func__,__LINE__,ret);
                        return JY_REFUSE;
                    }
                    /* ??? */
                    if (ret != OK)
                    {
                        LOGI("--- in func %s line %d,error ret = %d\n",__func__,__LINE__,ret);
                        if ((ret == JY_REFUSE)&&((BankAmount==0)||(BankAmount < HostValue.i)))
                        {
                            /*
                                "交易失败"   "您的卡片金额不够""支付本次消费金额"
                            */
                            return JY_REFUSE;
                        }    
                        else if (ret == JY_REFUSE)
                        {
                            /*
                                "交易失败""电子现金交易被拒绝"
                            */
                            return JY_REFUSE;
                        }
                        
                        return JY_END;
                    }
                }
                break;

            case 8://联机处理(可选)
                {

                }
                break;
            case 9://发行卡脚本处理(可选)
                {
                
                }
                break;
            case 10://交易结束
                {
                }
                break;
            default:
                {
                }
                break;
        }

    }

    //OfflineDataAuth、ProcessRestrict、TermRiskManage、TermActAnaly        
    {
        ret = TERMAPP_HandleQpbocProcessAfterOfflineDataAuth(channelbak);    
    }
    //gettimeofday(&tv,&tz);
    //LOGI("---------离线认证结束时间:%d s,%d us  \n",tv.tv_sec,tv.tv_usec);

    if( channelbak && ret == 0 )
    {
        if(channelbak==1)
        {
            LOGI("--- in func %s line %d ,hostvalue = %d \n",__func__,__LINE__,HostValue.i);
            {
                 
                extern unsigned char emvrecord[256];
                extern int emvrecordlen;
                
                memset(emvrecord,0,sizeof(emvrecord));
                TERMAPP_PackEMVData(emvrecord,&emvrecordlen);
                menu_print(emvrecord, emvrecordlen);
            }
            /*
            //status=YinLian_FreeSign_Swipcard_Proccess(channelbak,CardLan.CardType);
            if(status < 0)
            {
                LOGI("--- in func %s line %d,error status = %d\n",__func__,__LINE__,status);
                return JY_REFUSE;
            }
            else if(status == 0)
            {
                return 0;
            }
            */
            return 0;
        }
        else if(channelbak == 2)
        {
            int status;
            // status=YinLian_FreeSign_Swipcard_Proccess(channel);
            //status = YinLian_ODA_Proccess(channelbak,CardLan.CardType);
            if(status<0)
            {
                return JY_REFUSE;
            }
            else if(!status)
            {
                //支付成功
                return 0;
            }
        }
    }
    
    //tmp.i = amount + HostValue.i;
    tmp.i=BankAmount;

//---------memcpy(CardLan.QCash , tmp.longbuf ,4);
//---------LOGI("交易前余额余额:%d  ,hex %02x%02x%02x%02x  \n",tmp.i,CardLan.QCash[0],CardLan.QCash[1],CardLan.QCash[2],CardLan.QCash[3]);

    if (ret == OK)
    {
        /* 消费成功 */                   
    }
    else
    {
        // 脱机数据验证失败
        //---------Err_display(61);
    }

    //gettimeofday(&first,0);
    TERMAPP_SaveTransactionData();
    //gettimeofday(&second,0);
    //LOGI("\n保存记录耗时=%d\n",(second.tv_sec-first.tv_sec)*1000000+second.tv_usec-first.tv_usec);

    return OK;
}
/* 
    应用选择
*/
static int TERMAPP_Select_App(void)
{
    int i;
    int ret;
    
    memset((uchar *)&AppListCandidate,0,16*sizeof(APPDATA));
    ret = TERMAPP_AppSelect();                                                    
    if( ret != OK )
    {
        if(ret == err_CardBlock || ret == err_IccDataFormat) 
        {
            TermInfo.TVR[0] |= 0x20;
            LOGI("ICC data MIssing\n");
            return JY_END;
        }//ICC Data Missing
        else if(ret == err_ListNoMatchApp)                                            
        {
            for(i = 0;i < 16;i++)
            {
                if(AppBlockFlag[i] == 1) 
                {
                    return (JY_END);
                }
            }
        }
        else
        {
            LOGI("[ %s %d ] TERMAPP_AppSelect error ret : %d n",__FUNCTION__,__LINE__,ret);
            return JY_END;
        }
    }    
    
    for(i = 0 ; i < TermAppNum; i++)                                                    
    {
        if(memcmp(CardInfo.AID,AidListTerm[i].AID,AidListTerm[i].AIDLen) == 0)
        {
            break;
        }
    }
    
    if(i == TermAppNum)   //NO matching AID
    {
        LOGI("[ %s %d ] no matching AID\n",__FUNCTION__,__LINE__);
        return (JY_END);
    }
    
    i = 0;
    TermInfo.TermDDOLLen = AidListTerm[i].DDOLlen;
    memcpy(TermInfo.TermDDOL,AidListTerm[i].DDOL,AidListTerm[i].DDOLlen);

    return 0;
}
/*
    初始化应用
    获取交易应用信息，即卡片支持的功能和交易数据存放的文件记录
    根据中断要求的卡片作初步的判断。
*/
static int TERMAPP_Init_App(     int channel)
{
    int ret;
    int gpo_data_len;
    unsigned char gpo_data[256];
    uchar pdolData[255];
    unsigned int pdolDataLen;
    /* ？？？ */
    {    
       TERMAPP_QPBOCTermInit(channel);            
       LOGI("[ %s %d ] termAppNum=%d\n",__func__,__LINE__,TermAppNum);

        if(0)//if( (CLconfig.Customer == 1) && (CardLan.CardType==4 || CardLan.CardType==0x03) )
        {
            AmtAuthBin = 0;
        }
        else
        {
            AmtAuthBin = HostValue.i;
        }

        IntToByteArray(AmtAuthBin, TermInfo.AmtAuthBin, 4);
        memset(TermInfo.AmtAuthNum, 0, sizeof(TermInfo.AmtAuthNum));
        bin_to_bcd(TermInfo.AmtAuthNum + 2, AmtAuthBin, 4);
        
        /* get current time */
        {
            uchar transTime[8];
            GetTime(transTime);
            memcpy(TermInfo.TransDate, transTime, 3);
            memcpy(TermInfo.TransTime, transTime + 3, 3);
        }
        
        memset(TermInfo.TVR,0,5);
        memset(TermInfo.TSI,0,2);
        
    }   
    /*
        PDOL (Processing Options Data Object List)
        处理选项数据对象列表
        来源: 在应用选择时卡片返回的应用数据
        用途：用于发送在应用初始化时需要给卡片的终端数据
        Tag 9F38
    */

    {
        memset(pdolData, 0, 255);
        
        if(ICCDataTable[MV_PDOL].bExist == 0)
        {
            LOGI("[ %s %d ] :PDOL don't exist\n",__func__,__LINE__);
            return JY_END;                       //PDOL don't exist
        }
   
        if(TERMAPP_PDOLProcess(pdolData , &pdolDataLen) != OK)
        {
            return JY_END;           //Process PDOL end here
        }
        //PDOLDataLen = pdolDataLen;
        //memcpy(PDOLData, pdolData, pdolDataLen);
    }

    /*
        获取处理选项 GPO
        给卡片发送处理选项 GPO(GET Processing Options)命令
        需要送给卡片的终端数据(PDOL '9F38')
        卡片返回AIP和应用文件定位器AFL   
    */
    {
        uchar rev[256];
        uchar state[2];
        int result = 0;
        result = TERMAPP_GetGPO(pdolData, pdolDataLen,rev, state);                       //卡片与终端交互次数
        
        if(state[0] !=0x90 || state[1] != 0x00)     //Get Processing Options OK
        {
            LOGI("[ %s %d ] gpo state: %02x %02x \n",__func__,__LINE__,state[0],state[1]);
            if(state[0] == 0x69 && state[1] == 0x85)
            {
                //提示终端将当前应用从候选列表中删除并返回应用选择步骤选择另一个应用；
                LOGI("[ %s %d ]使用条件不满足",__FUNCTION__,__LINE__);
#if 0          
                bRetFromInitApp = 1;
                if(AppNum > 1)
                {
                    for(i = SelectedAppNo; i < AppNum - 1; i++)
                    {
                        TERMAPP_AppCopy(i,i+1);
                    }
                    AppNum--;
                    if(TERMAPP_ChooseApp() != OK)
                    {
                        return JY_END;
                    }
                    else 
                    {
                        /*
                            TERMAPP_InitialApp();  ->切换到电子现金交易？
                        */                    
                        //提示终端将当前应用从候选列表中删除并返回应用选择步骤选择另一个应用；
                        //->调到重新选择应用
                        return JY_END;
                    }
                 }
#endif
            }
            return JY_END;
        }
        
        gpo_data_len  = result;
        memcpy(gpo_data, rev, result);
    }
    /*
        AIP(应用交互特征)和AFL（应用文件定位器）分析处理
        决定要读取的文件记录，文件位置，建立 AFL。针对交易的不同情况可以返回不同 AFL 和 AIP；
    */
    ret = TERMAPP_Handle_Gpo_Data(gpo_data,gpo_data_len,channel);
    if(ret != OK)
    {
        return JY_END;
    }
    return OK;
}

static int TERMAPP_Handle_Gpo_Data(        unsigned char *pt_gpo_data,int gpo_data_len,int channel)
{
    int index = 0;
    int result = 0;
    int i,j,t,k;
    int len;
    
    if(pt_gpo_data == NULL)
    {
        return JY_END;
    }
    
    switch(pt_gpo_data[0])
    {
        case 0x80:
            {
                if(gpo_data_len < 2)
                {
                    return JY_END;
                }
                index++;
                if(pt_gpo_data[index] <= 127)
                {
                    len = pt_gpo_data[index];
                    index++;
                }
                else
                {
                    len = 0;
                    t = pt_gpo_data[index] & 0x7F;
                    for(j = 1;j <= t; j++)
                    {
                        len = len * 256 + pt_gpo_data[index + j];
                    }
                    index += t + 1;
                }

                if(channel)
                {
                    if(index + len != ( gpo_data_len - 2 ))     
                    {
                        return JY_END;
                    }
                    
                    memcpy(CardInfo.AIP,pt_gpo_data + index, 2);
                    ICCDataTable[MV_AIP].bExist = 1;
                    index += 2;
                    if(pt_gpo_data[index] == 0x00)
                    {
                        return JY_END;
                    }
                    k = (len - 2) / 4;
                    memcpy((uchar *)&AFL, pt_gpo_data + index, len - 2);
                    AFL_Num = k;
                 
                    if( k > 0 );
                    {
                        ICCDataTable[MV_AFL].bExist = 1;
                    }
                }
                else         
                { 
                    if(index + len != gpo_data_len)    //这里要注意和联机的不一样
                    {
                        return JY_END;
                    }
                    
                    memcpy(CardInfo.AIP, pt_gpo_data + index, 2);
                    ICCDataTable[MV_AIP].bExist = 1;
                    index += 2;
                    if(pt_gpo_data[index] == 0x00) 
                    {
                        return JY_END;
                    }
                    k = (len - 2) / 4;
                    memcpy((uchar *)&AFL, pt_gpo_data + index, len - 2);
                    AFL_Num = k;                                                        
                }
            }
            break;
        case 0x77: //TLV coded data
            {
                int ret = 0;
                ret = DecodeTLVLen(pt_gpo_data,gpo_data_len);    
                if(ret != OK)
                {
                    return ret;
                }
                ret = TERMAPP_DecodeTLV(pt_gpo_data,gpo_data_len);
                if(ret != OK) 
                {
                    return JY_END;            
                }
            }
            break;
        default:
            {
                return JY_END;
            }
            break;
   }
       
   LOGI("TERMAPP_InitialApp(): ICCDataTable existing elements:\n");
   for(i = 0; i < ICCDataNum; i++)
   {
       if (ICCDataTable[i].bExist) 
       {
           LOGI("%d, ", i);
       }
   }
   LOGI("\n");
   
   if(channel == 0)
   {
       LOGI("CardInfo.IssuAppData[4] = 0x%02X\n", CardInfo.IssuAppData[4]);    
       
       if(ICCDataTable[MV_AIP].bExist!=1) return JY_END;
       if(ICCDataTable[MV_ATC].bExist!=1) return JY_END;
       if(ICCDataTable[MV_AppCrypt].bExist!=1) return JY_END;
       if(ICCDataTable[MV_IssuAppData].bExist!=1) return JY_END;
   
       if((CardInfo.IssuAppData[4] & 0x30) ==0x00)              //拒绝
       {
           if(ICCDataTable[MV_Track2Equivalent].bExist!=1)
           {
               return JY_END;      
           }
           return JY_REFUSE;
       }
       else if((CardInfo.IssuAppData[4] & 0x30) ==0x20)      //联机
       {
           if(ICCDataTable[MV_CardholderName].bExist!=1)       
           {
               return JY_REFUSE;    
           }
           if(ICCDataTable[MV_Track2Equivalent].bExist!=1)       
           {
                 return JY_END;
           }
           return JY_REFUSE;                                                      
       }
       else if((CardInfo.IssuAppData[4] & 0x30) ==0x10)      //脱机
       {
           if(ICCDataTable[MV_AFL].bExist != 1 || CardInfo.AFLLen==0x0) 
           {
               return JY_END;
           }
       }        
   }
   return 0;
}


static int TERMAPP_HandleQpbocProcess(int channel)
{
    uchar atr[64],len;
    int ret ;
    unsigned char step = 0;
    int i;

    unsigned char ar_step[10];
    unsigned char step_len;
    
    if(channel != 1)
    {
        step_len = 3;
        memcpy(ar_step,"\x01\x02\x03",step_len);
    }
    else
    {
        step_len = 2;
        memcpy(ar_step,"\x01\x02",step_len);
    }
    for(i = 0 ;i < step_len; i++)
    {
        LOGI("[ %s %d ] ----step :%d ----- \n",__FUNCTION__,__LINE__,ar_step[i]);
        switch(ar_step[i])
        {
            case 1:
                {
                    ret = TERMAPP_Select_App();
                    if(ret != OK)
                    {
                        LOGI("[ %s %d ] error ret = %d.\n",__FUNCTION__,__LINE__,ret);
                        return i;
                    }
                }
                break;

            case 2:
                {
                    ret = TERMAPP_Init_App(channel);
                    if(ret != OK)
                    {
                        LOGI("[ %s %d ] error ret = %d.\n",__FUNCTION__,__LINE__,ret);
                        return i;
                    }
                }
                break;

            case 3:
                {
                    ret = TERMAPP_ReadAppData();
                    if(ret != OK) 
                    {
                        LOGI("[ %s %d ] error ret = %d.\n",__FUNCTION__,__LINE__,ret);
                        return i;
                    }
                    
                    len = CardInfo.Track2EquLen * 2;
                     
                    if((CardInfo.Track2Equ[len / 2 - 1] & 0x0F) == 0x0F)
                    {
                        len = len - 1;    
                    }
                    
                    if(len > 37) 
                    {
                        len = 37;    
                    }
                    
                    BCDToASC(pos_com.SwipeData2, CardInfo.Track2Equ, len);

                    for(i = 0; i < len; i++)
                    {
                        if(pos_com.SwipeData2[i] == 'D' || pos_com.SwipeData2[i] == 'd')
                        {
                            pos_com.SwipeData2[i]= '=';
                            break;
                        }
                    }
                    
                    pos_com.card_seq = CardInfo.PANSeq;  
                }
                break;
            default :
                {
                    /*unknow cmd*/
                
                }
                break;
        }
    }
    
    return JY_OK;
}


uchar TERMAPP_GetGAC1(uchar *GAC,uchar len, uchar * state)
{
    uchar result;
    uchar sbuf[0x50];
    LOGI("TERMAPP_GetData() is called.\n");
    sbuf[0] = 0x80;
    sbuf[1] = 0xAE;
    sbuf[2] = 0x80;
    
    sbuf[3] = 0x00;
    sbuf[4] = len;
    memcpy(&sbuf[5],GAC,len);
    sbuf[5+len] = 0;
    result = MifarePro_SendCom(sbuf,len+5+1);    
    memcpy(state,SRCPUCardBuf+SRCPUCardCount -2,2);
    if (OK == result)
    {
        if((state[0]==0x90)&&(state[1]==0x0))
        {
            return OK;
        }
    }
    return NOK;
}

/*
    联机去应用密文9f26;
*/
uchar TERMAPP_GetOnlineAppDesCode()
{
    uchar buf[256];
    uchar state[2];
    uchar len;
    uchar index,i,j,t;
    
    LOGI("%s:GET AC.\n",__func__);
    if(ICCDataTable[MV_CDOL1].bExist != 1)
    {
        LOGI("[ %s %d ] error ICCDataTable[MV_CDOL1].bExist: %d \n",__FUNCTION__,__LINE__,ICCDataTable[MV_CDOL1].bExist );    
        return JY_REFUSE;
    }
    
    {
       memset(buf,0,sizeof(buf));
       len = 0;
       memcpy(buf,PDOLData+4,29);
       menu_print(buf,29);
       len = 29;
       memcpy(buf+29,TermInfo.TransTime,3);
    //          memcpy(buf+32,TermInfo.MerchID,14);
       len+=3;
       len+=20;
       
       menu_print(buf, len);
       TERMAPP_GetGAC1(buf,len,state);
       if(state[0] != 0x90 || state[1] != 0x00)    
       {
           LOGI("[ %s %d ] TERMAPP_GetGAC1 state error : 0x%2X 0x%2X \n",__FUNCTION__,__LINE__,state[0],state[1]);  
           return JY_REFUSE;
       }

       index=0;
       if(SRCPUCardBuf[0] == 0x80)
       {
           if(SRCPUCardCount<2)
           {
               return JY_END;
           }
           index++;
           if(SRCPUCardBuf[index] <= 127)
           {
               len = SRCPUCardBuf[index];
               index++;
           }
           else
           {
               len = 0;
               t=SRCPUCardBuf[index]&0x7F;
               for(j = 1; j <= t; j++)
               {
                   len = len * 256 + SRCPUCardBuf[index+j];
               }
               index += t + 1;
           }

           if(index+len != (SRCPUCardCount-2))     
           {
               return JY_END;                      
           }   
       }

       CardInfo.CryptInfo=*(SRCPUCardBuf+index);        //应用密文版本信息 9F27
       ICCDataTable[MV_CryptInfoData].bExist= 1;                     
       index+=1;

       memcpy(CardInfo.ATC,SRCPUCardBuf+index,2);                //应用交易计数器    9F36
       index+=2;
       ICCDataTable[MV_ATC].bExist = 1;

       memcpy(CardInfo.AppCrypt,SRCPUCardBuf+index,8);         //应用密文        9F26
       index+=8;
       ICCDataTable[MV_AppCrypt].bExist = 1;

       memcpy(CardInfo.IssuAppData, SRCPUCardBuf + index, SRCPUCardCount - 2 - index); //发卡行应用数据  9F10
       CardInfo.IssuAppDataLen = SRCPUCardCount-2-index;
       ICCDataTable[MV_IssuAppData].bExist = 1;

       LOGI("GET AC:\n"); 
       LOGI("CardInfo.CryptInfo = ");    
       LOGI("%02x\n",CardInfo.CryptInfo);
       LOGI("CardInfo.ATC = ");  
       menu_print(CardInfo.ATC, 2);
       LOGI("CardInfo.AppCrypt = ");  
       menu_print(CardInfo.AppCrypt, 8);
       LOGI("CardInfo.IssuAppData = ");  
       menu_print(CardInfo.IssuAppData, CardInfo.IssuAppDataLen);                    
    }

    return 0;
}


uchar TERMAPP_HandleQpbocProcessAfterOfflineDataAuth(int channel)    
{
    uchar ret;
    uchar i;

    LOGI("TERMAPP_HandleQpbocProcessAfterOfflineDataAuth() is called.\n");

    // 授权响应码预设"Z1"脱机交易失败
    memcpy(TermInfo.AuthRespCode, "Z1", 2);
    TermDataTable[MV_AuthorRespCode-TermDataBase].bExist=1;

    if(channel != 1)
    {
        if((ret = TERMAPP_DataAuth(channel)) != OK)
        {
            return ret;
        }
    }
    
    if((ret = TERMAPP_ProcessRestrict()) != OK)
    {
        return ret;
    }
    if((ret = TERMAPP_TermRiskManage()) != OK)
    {
        return ret;
    }
    if((ret = TERMAPP_TermActAnaly()) != OK)                  
    {
        return ret;
    }
    // 授权响应码设为"Y1"脱机交易成功
    memcpy(TermInfo.AuthRespCode, "Y1", 2);

    return OK;
}




uchar TERMAPP_SaveTransactionData_Tao()
{
    uchar ret = OK;
    unsigned int amount = 0;
    int len, i,j;
    LongUnon tmp;
    uchar emvdata[256];
    uchar datalen, savelen, index;
    RecordFormat record;
    
    // copy card info
    // DevSID.longbuf
    //---------   IncSerId();
    //remove
    // CardLan.CardCsnB 卡号
    if(CardInfo.Track2EquLen)
    {
        len = CardInfo.Track2EquLen;
        for (i = 4; i < len; i++)
        {
            if ((CardInfo.Track2Equ[i] & 0xF0) == 0xD0)
            {
 //---------                memcpy(CardLan.CardCsnB, &CardInfo.Track2Equ[i - 4], 4); 
                break;
            }
            else if ((CardInfo.Track2Equ[i] & 0x0F) == 0x0D)
            {
//---------                 CardLan.CardCsnB[0] = (CardInfo.Track2Equ[i - 4] << 4) | (CardInfo.Track2Equ[i - 3] >> 4);
//---------                 CardLan.CardCsnB[1] = (CardInfo.Track2Equ[i - 3] << 4) | (CardInfo.Track2Equ[i - 2] >> 4);
//---------                 CardLan.CardCsnB[2] = (CardInfo.Track2Equ[i - 2] << 4) | (CardInfo.Track2Equ[i - 1] >> 4);
//---------                 CardLan.CardCsnB[3] = (CardInfo.Track2Equ[i - 1] << 4) | (CardInfo.Track2Equ[i] >> 4);
                break;
            }
        }
    }
    else
        // remove
    {
        len = CardInfo.PANLen;
        if(len <= 4)
        {
//---------             memcpy(CardLan.CardCsnB, CardInfo.PAN, 4);
        }
        else
        {
            if(CardInfo.PAN[len-1]&0x0F == 0x0F)
            {
                for(i = 0; i < 4; i++)
                {
//---------                     CardLan.CardCsnB[i] = ((CardInfo.PAN[len - 5+i]&0x0F)<<4 | (CardInfo.PAN[len - 4+i]&0xF0)>>4);
                    //LOGI("CardLan.CardCsnB[%d]: %x \n",i,CardLan.CardCsnB[i]);
                }                
            }else
            {
                for(i = 0; i < 4; i++)
                {
//---------                     CardLan.CardCsnB[i] = CardInfo.PAN[len - 4+i];
                    //LOGI("CardLan.CardCsnB[%d]: %x \n",i,CardLan.CardCsnB[i]);
                }
            }
        }        
    }
    // CardLan.QCash 上次余额
    amount = bcd_to_bin(CardInfo.OfflineAmount, 6);
    tmp.i = amount + HostValue.i;
//---------     memcpy(CardLan.QCash, tmp.longbuf, 4);
    
    // save record
    // Mode    
    // 保存数据  定额

    if (ret == OK)
    {
        memset(emvdata, 0, sizeof(emvdata));
        TERMAPP_PackEMVData(emvdata, &datalen);
        if (datalen > 256 - 4 * EMV_META_LEN)
        {
            LOGI("TERMAPP_HandleCard(): EMV data overflow!!! datalen = %d\n", datalen);
        }

        i = 0;
        index = 0;
        datalen = 256 - 4 * EMV_META_LEN;         // save up to 228 bytes of emv data
        while ((ret == OK) && datalen)
        {
            savelen = datalen;
            
            if (savelen > (sizeof(record) - EMV_META_LEN)) 
                savelen = sizeof(record) - EMV_META_LEN;

            memset(&record, 0, sizeof(record));
            memcpy(record.RFIccsn, DevSID.longbuf, 4);
            memcpy(record.RFDtac, &emvdata[index], savelen);
//---------             record.RFflag = CARD_SPEC_EMV_DATA;
            record.RFspare = i;
//---------             record.RFXor = Save_Data_Xor((unsigned char *)(&record));

            //if (Savedatasql(record,0,0))
            if(0)
            {
                ret = JY_REFUSE;
            }
            index += savelen;
            datalen -= savelen;
            i++;
        }

        if (datalen) ret = JY_REFUSE;
    }

    return ret;
}


uchar TERMAPP_SaveTransactionData_shij()
{
    uchar ret = OK;
    unsigned int amount = 0;
    int len, i,j;
    LongUnon tmp;
    uchar emvdata[256];
    uchar datalen, savelen, index;
    RecordFormat record;
    
    // copy card info
    // DevSID.longbuf
    //---------IncSerId();

    // CardLan.CardCsnB 卡号
    if(CardInfo.Track2EquLen)
    {
        len = CardInfo.Track2EquLen;
        for (i = 4; i < len; i++)
        {
            if ((CardInfo.Track2Equ[i] & 0xF0) == 0xD0)
            {
//---------                 memcpy(CardLan.CardCsnB, &CardInfo.Track2Equ[i - 4], 4); 
                break;
            }
            else if ((CardInfo.Track2Equ[i] & 0x0F) == 0x0D)
            {
//---------                 CardLan.CardCsnB[0] = (CardInfo.Track2Equ[i - 4] << 4) | (CardInfo.Track2Equ[i - 3] >> 4);
//---------                 CardLan.CardCsnB[1] = (CardInfo.Track2Equ[i - 3] << 4) | (CardInfo.Track2Equ[i - 2] >> 4);
//---------                 CardLan.CardCsnB[2] = (CardInfo.Track2Equ[i - 2] << 4) | (CardInfo.Track2Equ[i - 1] >> 4);
//---------                 CardLan.CardCsnB[3] = (CardInfo.Track2Equ[i - 1] << 4) | (CardInfo.Track2Equ[i] >> 4);
                break;
            }
        }
    }
    else
    {
        len = CardInfo.PANLen;
        if(len <= 4)
        {
  //---------           memcpy(CardLan.CardCsnB, CardInfo.PAN, 4);
        }
        else
        {
            if(CardInfo.PAN[len-1]&0x0F == 0x0F)
            {
                for(i = 0; i < 4; i++)
                {
//---------                     CardLan.CardCsnB[i] = ((CardInfo.PAN[len - 5+i]&0x0F)<<4 | (CardInfo.PAN[len - 4+i]&0xF0)>>4);
                    //LOGI("CardLan.CardCsnB[%d]: %x \n",i,CardLan.CardCsnB[i]);
                }                
            }else
            {
                for(i = 0; i < 4; i++)
                {
//---------                     CardLan.CardCsnB[i] = CardInfo.PAN[len - 4+i];
                    //LOGI("CardLan.CardCsnB[%d]: %x \n",i,CardLan.CardCsnB[i]);
                }
            }
        }        
    }
    // CardLan.QCash 上次余额
    amount = bcd_to_bin(CardInfo.OfflineAmount, 6);
    tmp.i = amount + HostValue.i;
    //tmp.i = HostValue.i;
//---------     memcpy(CardLan.QCash, tmp.longbuf, 4);
    
    // save record
    // Mode    
    // 保存数据  定额

    if (ret == OK)
    {
        memset(emvdata, 0, sizeof(emvdata));
        TERMAPP_PackEMVData(emvdata, &datalen);
        if (datalen > 256 - 4 * EMV_META_LEN)
        {
            LOGI("TERMAPP_HandleCard(): EMV data overflow!!! datalen = %d\n", datalen);
        }

        i = 0;
        index = 0;
        datalen = 256 - 4 * EMV_META_LEN; // save up to 228 bytes of emv data
        while ((ret == OK) && datalen)
        {
            savelen = datalen;
            if (savelen > (sizeof(record) - EMV_META_LEN)) savelen = sizeof(record) - EMV_META_LEN;

            // reserve these 3 fields
            //unsigned char RFflag;
            //unsigned char RFspare;
            //unsigned char RFXor;

#ifdef    TEST_USER
/*
            for(j= 0; j < 57; j++)
            {
                if(j%20 == 0)
                    LOGI("\n");            
                LOGI("0x%x ", emvdata[index+j]);
            }
            LOGI("\n");
*/            
#endif            
            memset(&record, 0, sizeof(record));
            memcpy(record.RFIccsn, DevSID.longbuf, 4);
            memcpy(record.RFDtac, &emvdata[index], savelen);
//---------             record.RFflag = CARD_SPEC_EMV_DATA;
            record.RFspare = i;
//---------             record.RFXor = Save_Data_Xor((unsigned char *)(&record));
                
            

            //if (Savedatasql(record,0,0))
            if(0)
            {
                ret = JY_REFUSE;
            }

            index += savelen;
            datalen -= savelen;
            i++;
        }

        if (datalen) ret = JY_REFUSE;
    }

    return ret;
}


uchar TERMAPP_SaveTransactionData()
{
    uchar ret = OK;
    unsigned int amount = 0;
    int len, i,j;
    LongUnon tmp;
    char tmp1[6];
    uchar emvdata[256];
    uchar savedata[288];//四条待保存的emv数据，72X4=288
    uchar datalen, savelen, index , index_save;

    YLmain_Record record;

    LOGI("\nTERMAPP_SaveTransactionData() is called !\n");
    
    //---------IncSerId();    
    
#if ZHANGJIAKOU_BANK
    if(OriginalCardType==0x05)
//---------         memcpy(CardLan.CardCsnB,CardLan.CardCsnBbak,4);
#endif    

    // CardLan.QCash 上次余额
    amount = bcd_to_bin(CardInfo.OfflineAmount, 6);
    tmp.i = amount + HostValue.i;
    //tmp.i = HostValue.i;
//---------     memcpy(CardLan.QCash, tmp.longbuf, 4);
    
    // save record
    // Mode    
    // 保存数据  定额
    /*组建消费记录*/
     memset((char *)&record,0,sizeof(record));
    record.CardType=1;
//---------     record.LogiCardType=CardLan.CardType;
    if(Section.Enable!=0x55)
        record.TransType=1;
    else
        {
            record.TransType=3;
//---------             memcpy(record.ATC,CardLan.ViewMoney,4);
        }
    record.Channel=3;
    memcpy(record.SerialNum,DevSID.longbuf,4);
    Rd_time(tmp1);
    memcpy(record.TransDate,tmp1,3);
    memcpy(record.TransTime,tmp1+3,3);
    memcpy(record.ShouldValue,DecValue.longbuf,4);
    memcpy(record.TransValue,HostValue.longbuf,4);
//---------     memcpy(record.OriginalBalance,CardLan.QCash,4);
//---------     memcpy(tmp.longbuf,CardLan.QCash,4);
    tmp.i-=HostValue.i;
    memcpy(record.Balance,tmp.longbuf,4);
    memcpy(record.MchantUnm,mchantConf.MERCHANTNO,15);
    memcpy(record.TerminalNum,mchantConf.TERMINALNO,8);

    strcat(record.LineNum,mchantConf.LineNo);
    strcat(record.VehicleNum,mchantConf.LicensePlate);
    strcat(record.DriverID,mchantConf.DriverId);

     memset(record.AcountNum,0,sizeof(record.AcountNum));
//---------      memcpy(record.AcountNum,pos_com.SwipeData2,CardLan.bankcardnum); 

     record.PursType = Sector.FlagValue;
     

    if(Section.Enable==0x55)
    {
        record.roundTrip = Section.Updown;
//---------         record.InOutFlag = CardLan.EnterExitFlag;
        record.StationNo[0] = Section.SationNow;
        }    

    
    memset(emvdata, 0, sizeof(emvdata));
    TERMAPP_PackEMVData(emvdata, &datalen);
    memcpy(record.domain55,emvdata+1,datalen);    

    return ret;
}




//uchar TERMAPP_GetOnlineAppDesCode();
//void TERMAPP_QPBOCTransInit(unsigned int transSeqId, unsigned int amount,int channel);
//uchar TERMAPP_HandleQpbocProcess(int channel);
//uchar TERMAPP_HandleQpbocProcessAfterOfflineDataAuth(int channel);

//---------extern int Save_Comsue_CZ(char *src,int len);

extern unsigned short Mcolor;
//--------- extern CardInformCPU CardLanCPU;

/*
    形参：原交易流水
                      通道
                      冲正报文数据
                      报文长度
                     芯片序列号
*/

#if 1
//---------extern int Save_Main_Record(char * src, int len);
extern RECORDFILE * YLMainRecord;        //交易主记录

/*
    重刷产生的冲刷记录为oda记录，注意oda记录的前4字节表示的是对应主记录的索引,
    orgsid:原交易的流水号
    orgcsn:原交易卡片的物理卡号

*/
int Re_SwipCard_Proccess(LongUnon orgsid,int channel,char * czdata,int czlen,char * orgcsn)
{
    char datbuf[512/*YLODA_SINGEL_LEN*/]={0};
    YLmain_Record record;
    int len=0;
    time_t now,start;
    int ret;        
    int step,loop;
    char buff[256];
    unsigned char  buflen;
    int status;
    LongUnon lfiguer;
    time(&now);
    start=now;

    LOGI("---------------------[%s]-------------------\n",__func__);
    char text[3][32]={
            {"联机超时"},
            {"请重刷(%d)"},
    };
    int count;
    char str[64];
    
    ////--------- PlayMusic(int index, int LoopFlag);        //请冲刷
    //--------- SetColor(Mcolor);
    //--------- SetTextColor(Color_white);
    //--------- SetTextSize(32);
    //--------- beepopen(2);
    //--------- PlayMusic(31, 0);        //提示请重刷
    count=mchantConf.ReSwipTimout;
    sprintf(str,text[1],count);
    //--------- TextOut(0, 60, text[0]);
    //--------- TextOut(0, 100, text[1]);
    
    
    /*寻卡*/
    while(1)
    {
        time(&now);
        if(now-start>mchantConf.ReSwipTimout)
        {
            /*建立冲正*/
            //---------Save_Comsue_CZ(czdata,czlen);
            /*提示请投币*/
            //--------- PlayMusic(30, 0);
            return -1;
        }
        count=mchantConf.ReSwipTimout-(now-start);
        sprintf(str,text[1],count);
        //--------- TextOut(0, 100, str);
        
        status=CardReset(buff, &buflen,0);
        if(status==0x20)
            break;
    }

    LOGI("寻到卡片\n");
    loop=1;
    step=1;
    while(loop)
    {
        LOGI("---- in func %s , step = %d \n",__func__,step);
        switch(step)
        {
            case 1:
                TERMAPP_QPBOCTransInit(DevSID.i + 1, HostValue.i,channel);
                step++;
                break;
            case 2:
                ret = TERMAPP_HandleQpbocProcess(channel);
                if(ret!=0)
                {
                    //---------Save_Comsue_CZ(czdata,czlen);
                    //--------- YL_Err_Display(255);
                    //--------- PlayMusic(30 ,0);
                    return -1;
                }
                else
                    step++;
                break;
            case 3:
                if(channel==2)
                {
                    ret=TERMAPP_GetOnlineAppDesCode();
                    LOGI("TERMAPP_HandleCard(): TERMAPP_GetOnlineAppDesCode() ret = %d.\n", ret);
                    short temp;
                    temp = CardInfo.ATC[0]<<8|CardInfo.ATC[1];
                    if((temp==0)||(ret!=OK))
                        {
                            //---------Save_Comsue_CZ(czdata,czlen);
                            //--------- YL_Err_Display(255);
                            //--------- PlayMusic(30 ,0);
                            return -1;
                        }
                }
                else
                    return JY_REFUSE;
                step++;
                break;
            case 4:
                ret=TERMAPP_HandleQpbocProcessAfterOfflineDataAuth(channel);
                if(ret!=0)
                    {
                        //---------Save_Comsue_CZ(czdata,czlen);
                        //--------- YL_Err_Display(10);
                        //--------- PlayMusic(30 ,0);
                        return JY_REFUSE;
                    }
                else
                    step++;
                    break;
            case 5:
                /*判断是否为同一张卡*/
                if(0) //(memcmp(CardLanCPU.CSN,orgcsn,4)!=0)
                {
                    /*不是同一张卡，保存冲正记录*/
                    //---------Save_Comsue_CZ(czdata,czlen);
                    /*提示重新刷卡*/
                    return -1;
                }
                else
                {
                    /*产生再扣款请求*/
                    LOGI("产生再请款记录\n");
                    //---------IncSerId();
                    LOGI("在请款记录自身的流水号 %d",DevSID.i);
                    memset(&record,0,sizeof(record));
                    //---------build_Recomsue( channel,1 ,datbuf+4,&len,orgsid,&record);

                    /*提示刷卡成功*/


                    /*保存主记录*/
                    char tmp[16]={0};
                    record.CardType=1;
                    record.LogiCardType=0;
                    if(Section.Enable == 0x55)
                    {
                        record.TransType=3;
                        lfiguer.i=1;
                        memcpy(record.ATC,lfiguer.longbuf,4);    
                    }
                    else
                    {
                        record.TransType=1;                                    //分段还是一票制
                    }
                    record.Channel=2;
                    memcpy(record.SerialNum,DevSID.longbuf,4);
                    Rd_time(tmp);
                    memcpy(record.TransDate,tmp,3);
                    memcpy(record.TransTime,tmp,3);
                    memcpy(record.ShouldValue,DecValue.longbuf,4);
                    memcpy(record.TransValue,HostValue.longbuf,4);
                    memcpy(record.MchantUnm,mchantConf.MERCHANTNO,15);
                    memcpy(record.TerminalNum,mchantConf.TERMINALNO,8);

                    strcat(record.LineNum,mchantConf.LineNo);
                    strcat(record.VehicleNum,mchantConf.LicensePlate);
                    strcat(record.DriverID,mchantConf.DriverId);


                    //---------Save_Main_Record((char *)&record, sizeof(record));

                    //---------status=Save_ODA_Comsue(YLMainRecord->nSaveNum,datbuf,len+4);     //保存在请款记录，即oda记录
                }
                step++;
                break;
                default:
                    loop=0;
                    step=0;
                    break;
        }

    }


    return 0;
}


#endif

