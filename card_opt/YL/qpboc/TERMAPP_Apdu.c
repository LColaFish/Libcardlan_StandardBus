#include "includes.h"
#include "unistd.h"
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


uchar SRCPUCardBuf[1024];
unsigned int SRCPUCardCount;

//---------
uchar buff5A[150];    //00 b2 01 14 00    包含主账号5A的记录
uchar buff57[40];     //00 b2 01 0c 00    包含2磁道等价数据57的记录
ulong BankAmount;

//---------



extern uchar BusAppDiscRec[64];        //Bus application discount information record 实际使用47
extern uchar BusAppDiscRecLen;
extern uchar BankcardFlag;
extern unsigned char OriginalCardType;
//---------extern L9_BusConfig CLconfig;
void delay_1ms()
{
    usleep(1000);
}

int MifarePro_SendCom_ex(char *buffer, int length,unsigned char *respond_buf)
{
    int result;
    result = mifare_read_and_write(buffer,length,respond_buf);
    if(result < 0)
    {
        return 0;
    }
    return result;
}

uchar MifarePro_SendCom(char *buffer, int length)
{
    memset(SRCPUCardBuf, 0, sizeof(SRCPUCardBuf));
    SRCPUCardCount =  MifarePro_SendCom_ex(buffer,length,SRCPUCardBuf);
    return SRCPUCardCount > 2 ? 0 : 1;
}


/***********************************************************************
FUNCTION: 卡应用选择
PARAMETER DESCRIPTION:
    filefalg - P2
    filename - 选择的文件
    len - 长度
RETURN:
    state - 返回的状态值
***********************************************************************/
uchar TERMAPP_SelectApp(const uchar P1,const uchar P2,uchar len,const uchar *filename, uchar * state)
{
    uchar result;
    uchar sbuf[0x50];
    unsigned char SW[2];

    SRCPUCardCount = apdu_cmd_select_file(P1, P2,len,filename,SRCPUCardBuf,SW);

    if(state != NULL)
    {
        state[0] = SW[0];
        state[1] = SW[1];
    }
    
    if(SRCPUCardCount <= 2 || SW[0] != 0x90 || SW[1] != 0x0)
    {
        return err;
    }

    return OK;
}

/***********************************************************************
FUNCTION:  READ BINARY
PARAMETER DESCRIPTION:
    sfi：短文件标识符
    lc ：返回的数据长度
RETURN:
    state - 返回的状态值
***********************************************************************/
uchar TERMAPP_ReadBinary(uchar sfi,uchar lc,uchar *state)
{
    uchar result;
    uchar sbuf[0x50];

    LOGI("TERMAPP_ReadBinary() is called.\n");

    sbuf[0] = 0x00;
    sbuf[1] = 0xb0;
    sbuf[2] = 0x80 | sfi;
    sbuf[3] = 0x00;
    sbuf[4] = lc;

    result = MifarePro_SendCom(sbuf,5);
    memcpy(state,SRCPUCardBuf+SRCPUCardCount -2,2);
    return result;
}

/***************************************************************
FUNCTION:  READ RECORD
PARAMETER DESCRIPTION:
    sfi - p2 短文件标识符
    num - p1 记录号
RETURN:
    state - 返回的状态值
****************************************************************/
int TERMAPP_ReadRecord(uchar sfi,uchar num,unsigned char *p_rev_buffer,uchar *state)
{
    return apdu_cmd_read_record(num, sfi | 0x04,p_rev_buffer, state);
}

/************************************************************************
FUNCTION:  GET GPO
PARAMETER DESCRIPTION:
    *gpolist - 标签数据         
    : B.8.2   emv 5.6
            编码               值
            CLA             0x80
            INS             0xA8
            P1              0x00
            P2              0x00
            Lc              var
            数据域             PDOL  相关数据（如果存在）或 8300
            Le              0x00
    
    len - gpolist len
RETURN:
    state - 返回的状态值
************************************************************************/
int TERMAPP_GetGPO(uchar *gpolist,uchar len,unsigned char *p_rev_buffer,uchar *state)
{
    int result;
  
    uchar dataV[256];
    uchar dataVindex=0;

    if(gpolist == NULL || p_rev_buffer == NULL)
    {
        return -1;
    }

    /* 设置 Lc 和 数据域*/
    {
        memcpy(dataV,"\x83",1) ;
        dataVindex++;
        if(len > 127) 
        {
            memcpy(dataV+dataVindex,"\x81",1);
            dataVindex++;
        }
        memcpy(dataV+dataVindex, (uchar *)&len, 1);
        dataVindex++;
        memcpy(dataV + dataVindex, gpolist, len);
        dataVindex += len;
    }
    
    result = apdu_cmd4(0x80, 0xA8, 0x00, 0x00, dataVindex , dataV, 0x00, p_rev_buffer);
    if(state != NULL)
    {
        state[0] = p_rev_buffer[result - 2];
        state[1] = p_rev_buffer[result - 1];
    }
    
    if(result <= 2 || p_rev_buffer[result - 2] != 0x90 || p_rev_buffer[result -1] != 0x00)
    {
        LOGI("[%s %d ]mifare_read_and_write : result :%d SW1=%02X  SW2=%02X\n",__FUNCTION__,__LINE__,result,p_rev_buffer[result - 2],p_rev_buffer[result -1]);
        return -1;
    }

    return result -2;
}

/************************************************************************
FUNCTION:  GetBalance
PARAMETER DESCRIPTION:

RETURN:
    Tag:9F 79 电子现金可用余额
***********************************************************************/
uchar TERMAPP_GetBalance(unsigned int *amount)
{
    uchar result;
    uchar sbuf[0x50]= {0};
    uchar dispbuf[10];
    uchar DDF[]= {"2PAY.SYS.DDF01"};
    uchar AID[]= {0xA0,0x00,0x00,0x03,0x33,0x01,0x01,0x01};
    uchar state[2]= {0};
    uchar tryNo=0;
    uchar Amount[6]= {0};
    ulong uAmount;

    LOGI("TERMAPP_GetBalance() is called.\n");

    *amount = 0;
    sbuf[0] = 0x80;
    sbuf[1] = 0xCA;
    sbuf[2] = 0x9F;
    sbuf[3] = 0x79;
    sbuf[4] = 0x00;

    for(tryNo=0; tryNo<3; tryNo++)
    {
        result = SearchCard();
        if(result != OK)
        {
            continue;
        }
        result = TERMAPP_SelectApp(0x04,0x00,14,DDF,(uchar *)state);
        if (result != OK)
        {
            continue;
        }
        //delay_1ms(100);
        result = TERMAPP_SelectApp(0x04,0x00,8,AID,(uchar *)state);
        if (result != OK)
        {
            continue;
        }
        //delay_1ms(100);    
        result = MifarePro_SendCom(sbuf,5);
        if (result != OK)
        {
            continue;
        }
        //delay_1ms(100);

        memcpy(Amount,SRCPUCardBuf+3,6);
        uAmount=((Amount[3]&0xf0)>>4)*100000+(Amount[3]&0x0f)*10000+((Amount[4]&0xf0)>>4)*1000+(Amount[4]&0x0f)*100+((Amount[5]&0xf0)>>4)*10+(Amount[5]&0x0f);
        sprintf(dispbuf,"   %5d",uAmount);
        //显示余额

        *amount = uAmount;
        LOGI("Balance: %s\n", dispbuf);
        break;
    }    
    if (result != OK)
    {
        LOGI("Status = %d\n", result);
    }

    return result;
}

//取卡号和余额
int TERMAPP_GetCardNum_init(void)
{
    uchar tryNo=0;
    int ret = 0;
    uchar state[2]= {0};

    for(tryNo = 0; tryNo < 1; tryNo++)
    {
        ret = SearchCard();

        AppNum = 0;
        ret = TERMAPP_SelectDF();
        if(ret != APPSEL_PSEFOUND)
        {
            continue;
        }
        else
        {
            if((state[0]!=0x90)&&(state[1]!=0))
            {
                ret = -1;
                break;
            }
        }   
        
        ret = TERMAPP_SelectApp(0x04,0x00,8,AppListCandidate[0].AID,(uchar *)state);
        if (ret != OK)
        {              
            continue;
        }
        else
        {
            if((state[0]!=0x90)&&(state[1]!=0))
            {               
                ret = -2;
                break;                    
            }
        }    
    }
    if(ret != 0)
    {
        return ret;
    }
    return 0;
}

int TERMAPP_GetCardNum(unsigned char *csn, unsigned int len, unsigned char *equ2data,unsigned int size)
{
    int ret = 0;

    LOGI("TERMAPP_GetCardNum() is called.\n");
    
    ret = TERMAPP_GetCardNum_init();
    if (ret != OK)
    {
        LOGI("[%s %d]result error = %d\n",__FUNCTION__,__LINE__, ret);
        return ret;
    }
    {
        /*读记录 0x02 ,0x14 =( 0x02    Lsh 3 )+ 4(bit:100)*/
        uchar sbuf[10]= {0};
        sbuf[0] = 0x00;
        sbuf[1] = 0xB2;
        sbuf[2] = 0x01;
        sbuf[3] = 0x14;
        sbuf[4] = 0x00;

        ret = MifarePro_SendCom(sbuf, 5);
        if(ret != 0)
        {
            LOGI("[%s %d]result error = %d\n",__FUNCTION__,__LINE__, ret);
            return -2;
        }

        /* check respond */
        {
            int len = SRCPUCardBuf[1];
            unsigned char SW[2];
            SW[0] = SRCPUCardBuf[SRCPUCardCount - 2];
            SW[1] = SRCPUCardBuf[SRCPUCardCount - 1];
            if(SW[0] != 0x90 | SW[1] != 0x00)
            {
                LOGI("[%s %d] ret:%02X SW1=%02X  SW2=%02X\n",__FUNCTION__,__LINE__,ret,SW[0],SW[1]);
                return -4;
            } 
        }

        if(len < SRCPUCardCount)
        {
            /* error */
            LOGI("[%s %d] error size:%d ,SRCPUCardCount:%d \n",__FUNCTION__,__LINE__,len,SRCPUCardCount);
            return -4;
        }

        if(0)
        {
            DebugPrintChar("sbuf SRCPUCardBuf=", SRCPUCardBuf, SRCPUCardCount);
        }

        memcpy(csn, SRCPUCardBuf, SRCPUCardCount);
        memset(buff5A,0,sizeof(buff5A));
        memcpy(buff5A,SRCPUCardBuf,SRCPUCardCount);
    }
    
    {
        /*读记录 0x01 ,0x0C =( 0x02    Lsh 3 )+ 4(bit:100)*/
        uchar sbuf1[10]= {0};
        sbuf1[0] = 0x00;
        sbuf1[1] = 0xB2;
        sbuf1[2] = 0x01;
        sbuf1[3] = 0x0C;
        sbuf1[4] = 0x00;    

        ret = MifarePro_SendCom(sbuf1, 5);

        if(ret != 0)
        {
            return -4;
        }
        /* 70 + len + record */
        if(0)//SRCPUCardBuf[0] != 0x70)          /*error*/
        {
            DebugPrintChar("sbuf1 SRCPUCardBuf=", SRCPUCardBuf, SRCPUCardCount);
        }
        /* check respond */
        {
            int len = SRCPUCardBuf[1];
            unsigned char SW[2];
            SW[0] = SRCPUCardBuf[SRCPUCardCount - 2];
            SW[1] = SRCPUCardBuf[SRCPUCardCount - 1];
            if(SW[0] != 0x90 | SW[1] != 0x00)
            {
                LOGI("[%s %d] ret:%02X SW1=%02X  SW2=%02X\n",__FUNCTION__,__LINE__,ret,SW[0],SW[1]);
                return -4;
            } 
        }

        if(size < SRCPUCardCount)
        {
            /* error */
            LOGI("[%s %d] error size:%d ,SRCPUCardCount:%d \n",__FUNCTION__,__LINE__,size,SRCPUCardCount);
            return -4;
        }
        {
            /*
                应用主账号（PAN）
                    主账号（ＰＡＮ）由６位数字的发卡机构标识号（ＩＩＮ）、可变长（最大１２
                    位数字）个人账户号和校验位组成
                分隔符（“D”）
                失效日期（YYMM）
                服务码
                PIN 验证域
                自定义数据（由支付系统
                定义）
                补 F（如果不是偶数个
            */
            unsigned char tag = SRCPUCardBuf[2];
            if( tag != 0x57)
            {
                /* error */
                LOGI("[%s %d] error tag:%0x \n",__FUNCTION__,__LINE__,tag);
                return -4;
            }
        }
        
        memcpy(equ2data, SRCPUCardBuf, SRCPUCardCount);
        memset(buff57,0,sizeof(buff57));
        memcpy(buff57,SRCPUCardBuf,SRCPUCardCount);
    }
    
    //获取卡片余额 --- 用于支持双免和ODA 时，取消读电子现金余额,这样才能支持手机PAY
    {
        uchar Amount[6]= {0};
        ulong uAmount;
        uchar dispbuf[10];
        
        uchar sbuf2[10]= {0};
        sbuf2[0] = 0x80;
        sbuf2[1] = 0xCA;
        sbuf2[2] = 0x9F;
        sbuf2[3] = 0x79;
        sbuf2[4] = 0x00;
        ret = MifarePro_SendCom(sbuf2,5);
        if(ret != 0 )
        {
            LOGI("[%s %d ]mifare_read_and_write : ret :%d SW1=%02X  SW2=%02X\n",__FUNCTION__,__LINE__,ret,SRCPUCardBuf[0],SRCPUCardBuf[1]);
            return -5;
        }

        memcpy(Amount, SRCPUCardBuf+3, 6);
        uAmount = ((Amount[3]&0xf0)>>4)*100000+(Amount[3]&0x0f)*10000+((Amount[4]&0xf0)>>4)*1000+(Amount[4]&0x0f)*100+((Amount[5]&0xf0)>>4)*10+(Amount[5]&0x0f);
        sprintf(dispbuf,"   %5d",uAmount);
        BankAmount = uAmount;
        LOGI("Balance: %s\n", dispbuf);
    }
    
    return OK;
}



