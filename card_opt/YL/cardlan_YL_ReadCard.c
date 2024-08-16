

#include "cardlan_YL_ReadCard.h"

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


int YL_ReadCardInfor_CPU(YL_CPUCARD_Info_t *pt_CardInfo)
{
    int ret = 0;
    unsigned char SW[2];
    
    if(pt_CardInfo == NULL)
    {
        return HANDLE_ERR_ARG_INVAL;
    }
    /*
        应用选择(必备)
        1.目录选择(可选)
        2.直接选择(必备)
     */ 
    /*
        初始化应用(必备)
    */
    
    /*
        读应用记录(必备)
    */
    /*
        脱机数据认证(必备)
        1.标准DDA(必备)
        2.复合DDA/应用密文生成(可选)
    */
    /*
        处理限制(必备)
        1.应用版本号检查(必备)
        2.应用用途控制检查(可选)
        3.生效日期检查(可选)
        4.失效日期检查(必备)
     */
    /*
        持卡人验证(可选)
        1.单独的CVM(可选)
     */
    /*
        终端风险管理(必备)
        1.终端异常文件检查(不适用)
        2.商户强制联机(不适用)
        3.最低限额检查(不适用)
        4.交易日志(不适用)
        5.随机选择(不适用)
        6.频度检查(可选)
        7.新卡检查(可选)
    */
    /*
        终端行为分析(有条件)
        条件:当需要验证IAC时需具备
    */
    /*
        卡片行为分析(必备)
        1.联机/脱机决定(必备)
        2.卡片风险管理(必备)
        3.通知报文(可选)
        4.应用密文(提供算法选择，算法标识见附录E)
    */
    /*
        联机处理(必备)
        1.联机能力(必备)
        2.发卡机构认证(可选)
    */
    /*
        交易结束(必备)
    */
    /*
        发卡机构到卡片脚本处理(可选)
        安全报文(仅具备一种脚本形式)
    */

    unsigned char ar_file_index[] = {GET_YL_RECORD_0x01,GET_YL_RECORD_0x02,GET_YL_BALANCE};
    unsigned char file_index_len = sizeof(ar_file_index)/sizeof(unsigned char);
    ret = Read_YL_CPU_Card_info(ar_file_index,file_index_len,pt_CardInfo);
    if(ret != 0 )
    {
        LOGI("[ %s %d ]ret : %d \n",__FUNCTION__,__LINE__,ret);
        return HANDLE_ERR;
    }

    /* 从记录中查找标签 */
    {
        memset(pt_CardInfo->tag_0x57,0,sizeof(pt_CardInfo->tag_0x57));
        ret = YL_find_tag_0x57(pt_CardInfo->record_sfi_01,sizeof(pt_CardInfo->record_sfi_01),pt_CardInfo->tag_0x57);
        if(ret != 0)
        { 
            LOGI("--- in func %s line %d,warning ret = %d\n",__func__,__LINE__,ret);
        } 

        memset(pt_CardInfo->tag_0x5a,0,sizeof(pt_CardInfo->tag_0x5a));
        ret = YL_find_tag_0x5A(pt_CardInfo->record_sfi_02,sizeof(pt_CardInfo->record_sfi_02),pt_CardInfo->tag_0x5a);
        if(ret != 0)
        { 
            LOGI("--- in func %s line %d,warning ret = %d\n",__func__,__LINE__,ret);
        } 
    }
    return 0;
}

int  Read_YL_CPU_Card_info(unsigned char *ar_file_index,unsigned char file_index_len, YL_CPUCARD_Info_t *p_card_info)
{
    int ret;
    int result;
    static unsigned char receive_buf[130]= {0};
    unsigned char status,len;
    unsigned char binname[256+2];
    
    unsigned char PsamNum[6];
    unsigned char PsamKeyIndex;
    
    unsigned char SW[2];

    if(p_card_info == NULL || ar_file_index == NULL)
    {
        LOGI("Read_CPU_Card_info arg error line=%d\n",__LINE__);    
        return HANDLE_ERR_ARG_INVAL;
    }
    
    memset(p_card_info,0,sizeof(YL_CPUCARD_Info_t));

    unsigned char PPSE[]={"2PAY.SYS.DDF01"};  
    ret = select_and_match_app(PPSE,strlen(PPSE));
    if(ret != 0)
    {
        printf("[ %s %d ]ret : %d \n",__FUNCTION__,__LINE__,ret);
        return HANDLE_ERR;
    }

    int index;
    for(index = 0 ; index < file_index_len; index++)
    {
        switch(ar_file_index[index])
        {
            case GET_YL_RECORD_0x01:     
                {
                    int index = 1;
                    result = apdu_cmd_read_record(index,Lsh(0x01,3)|0x04,receive_buf,SW);
                    if(SW[0] != 0x90 || SW[1] != 0x00)
                    {
                        LOGI("[%s %d ]result error : %d SW[0]=0x%02X SW[1]=0x%02X ----\n",__FUNCTION__,__LINE__,result,SW[0],SW[1]);
                        break;
                    }
                    memcpy(p_card_info->record_sfi_01,receive_buf,result);
                }
                break;
         
            case GET_YL_RECORD_0x02:    
                {
                    int index = 1;
                    result = apdu_cmd_read_record(index,Lsh(0x02,3)|0x04,receive_buf,SW);
                    if(SW[0] != 0x90 || SW[1] != 0x00)
                    {
                        LOGI("[%s %d ]result error : %d SW[0]=0x%02X SW[1]=0x%02X ----\n",__FUNCTION__,__LINE__,result,SW[0],SW[1]);
                        break;
                    }
                    memcpy(p_card_info->record_sfi_02,receive_buf,result);
                }
                break;
          
            case GET_YL_BALANCE:
                {
                    result =  YL_get_balance(p_card_info->balance);
                    if(result <= 0)
                    {
                        break;
                    }
                }
            default:
                {
                }
                break;
        }
    }

    return HANDLE_OK;
}

