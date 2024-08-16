
#include "YL_card_util.h"
#include "card_opt/apdu_cmd.h"
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
static int YL_find_tag(unsigned char *p_tag,int tag_len,unsigned char *p_src,int length);

static int YL_find_tag(unsigned char *p_tag,int tag_len,unsigned char *p_src,int length)
{
    if(tag_len < 0 || tag_len > 3 || p_tag == NULL || p_src == NULL)
    {
        return -1;
    }
    
    int i,j;
    for(i = 0; i < length; i++)
    { 
       if( p_src[i] == p_tag[0] && i + tag_len < length )
       {
            for(j = 0 ; j < tag_len ; j++)
            {
                if(p_src[i + j] != p_tag[j])
                {
                   break;
                }               
            }

            if(j != tag_len)
            {
                i = i + tag_len; 
                continue;
            }

            break;
       }
    }
    if(i == length)
    {
        /* not found */
        LOGI("--- in func %s line %d,cat not find tag[0] 0x%2X tag_len %d in buffer :\n",__func__,__LINE__,p_tag[0],tag_len);
        menu_print(p_src,length);
        return -2;
    }
    
    LOGI("--- in func %s line %d,tag[0] 0x%2X tag_len %d in index %d\n",__func__,__LINE__,p_tag[0],tag_len,i);
    return i;
}
//持卡人有效账号
int YL_find_tag_0x5A(unsigned char *p_src,int length,unsigned char dst[12])
{
    unsigned char tag[1] = {0x5A};

    int index = YL_find_tag(tag,sizeof(tag),p_src,length);
    if(index < 0)
    {
        return -1;
    }
    int len = p_src[ index + 1 ];
    
    if(len + index > length || len > 10)
    {
        /* data format error*/
        return -3;
    }

    memcpy(dst, p_src + index  ,len + 2);

    return 0;
}
/*
    按 GB/T 17552 的规定，磁条 2 的数据。不包括起始位、结束位和 LRC（验证码），包括：
    应用主账号（PAN）
    分隔符（“D”）
    失效日期（YYMM）
    服务码
    PIN 验证域
    自定义数据（由支付系统
    定义）
    补 F（如果不是偶数个）
*/
int YL_find_tag_0x57(unsigned char *p_src,int length,unsigned char dst[21])
{
    unsigned char tag[1] = {0x57};
    
    int index = YL_find_tag(tag,sizeof(tag),p_src,length);
    if(index < 0)
    {
        return -1;
    }
    int len = p_src[ index + 1 ];

    if(len + index > length || len > 19)
    {
        /* data format error*/
        return -3;
    }

    memcpy(dst, p_src + index ,len + 2);

    return 0;
}
/*
    JR/T 0025.15 5.4.1
    GET DATA 命令格式
    字节      值               注释
    CLA     ‘80’ 
    INS     ‘CA’ 
    P1      ‘9F’或‘DF’     ‘9F79’为电子现金余额数据元标签
                          ‘DF79’为第二币种电子现金余额数据元标签  
    P2      ‘79’
    LC      不存在 
    Data    不存在 
    Le      ‘00’ 
    
    电子现金余额查询响应
    字节          值                  注释
    标签（T）       ‘9F79 / DF79’ 
    长度（L）       ‘06’              6 字节长
    数据（V）                         电子现金余额  以应用定义货币表示
    SW1/SW2     ---               状态信息
*/
int YL_get_balance(unsigned long *p_balance)
{
    if(p_balance == NULL)
    {
        return -1;
    }
    int ret = 0;
    unsigned char P1 = 0x9F;
    unsigned char P2 = 0x79;
    unsigned char respond_buffer[2 + 1 + 6 + 2];
    
    ret = YL_get_tag_data(P1,P2,respond_buffer,sizeof(respond_buffer));
    if(ret != 0)
    {
        return -2;
    }
    /*
         tag: 0x9F 0x79  record_buffer[ 0 - 1 ]
         len: record_buffer[ 2 ]
         data: record_buffer[ 3- len ]
     */
    
    {
        unsigned char Amount[6]= {0};
        unsigned long uAmount;
        
        memcpy(Amount, respond_buffer+3, 6);
        uAmount = ((Amount[3]&0xf0)>>4)*100000+(Amount[3]&0x0f)*10000+((Amount[4]&0xf0)>>4)*1000+(Amount[4]&0x0f)*100+((Amount[5]&0xf0)>>4)*10+(Amount[5]&0x0f);
        *p_balance = uAmount;
   
        /* debug */
        {
            unsigned char dispbuf[10];
            sprintf(dispbuf,"   %5d",*p_balance);
            LOGI("Balance: %s\n", dispbuf);
        }
    }
 
    return 0;
}
/*
    JR/T 0025.5 B.7.2
    取数据（GET DATA）命令报文
    编码      值
    CLA     80
    INS     CA
    P1 P2   要访问数据的标签
    Lc      不存在
    数据域     不存在
    Le      00
*/
int YL_get_tag_data(unsigned char P1,unsigned char P2,unsigned char *p_respond_buff,int length)
{
    if(p_respond_buff == NULL || length < 0)
    {
        return -1;
    }
    int len;
    unsigned char CLA = 0x80;
    unsigned char INS = 0xCA;
    unsigned char Le = 0x00;
    
    len = apdu_cmd2(CLA,INS,P1,P2,Le,p_respond_buff);
    if(len < 2 )
    {
        LOGI("[%s %d ] error : len :%d \n",__FUNCTION__,__LINE__,len);
        return -1;
    }

    if(p_respond_buff[len - 2] != 0x90 && p_respond_buff[len - 1] != 0x00)
    {
        LOGI("[%s %d ] respond: sw1:0x%02x sw2:0x%02x \n",__FUNCTION__,__LINE__,p_respond_buff[len - 2], p_respond_buff[len - 1]);
        return -2;
    }

#if defined (TARGET_DEBUG)    
    LOGI("------tag: 0x%2x%2x buff:\n",P1,P2);
    menu_print(p_respond_buff,length);
#endif    
    return 0;
}




