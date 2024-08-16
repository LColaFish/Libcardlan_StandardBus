#ifndef _CARDLAN_YL_READCARD_H_
#define _CARDLAN_YL_READCARD_H_


#include "common/cardlan_StandardBus_tpye.h"
#include "card_opt/apdu_cmd.h"

typedef enum{
    GET_YL_RECORD_0x01,
    GET_YL_RECORD_0x02,
    GET_YL_BALANCE,
}E_YL_CMD;
typedef struct {

    /*record sfi = 1*/
    unsigned char record_sfi_01[256];

    /*record sfi = 2*/
    unsigned char record_sfi_02[256];

    /* tag data */
    unsigned char tag_0x57[19 + 2];      //磁道2等效数据
    unsigned char tag_0x5a[10 + 2];      //持卡人有效账号 
    unsigned char tag_0x9F26 [8 + 2];    //应用密文（AC）  
    unsigned char tag_0x9F27 [1 + 2];    //密文信息数据  
    unsigned char tag_0x9F10 [32 + 2];   //发卡行应用数据        
    //unsigned char tag_0x9F37 [ ];    //不可预知数据的标签  
    unsigned char tag_0x9F36 [2 + 2];    //应用交易计数器  
    //unsigned char tag_0x95 [ ];      
    //unsigned char tag_0x9A [ ];      
    //unsigned char tag_0x9C [ ];      
    //unsigned char tag_0x9F02 [ ];      
    //unsigned char tag_0x5F2A [ ];      
    //unsigned char tag_0x82 [ ];      
    //unsigned char tag_0x9F1A [ ];      
    //unsigned char tag_0x9F03 [ ];      
    //unsigned char tag_0x9F33 [ ];      
    //unsigned char tag_0x9F34 [ ];      
    //unsigned char tag_0x9F35 [ ];      
    //unsigned char tag_0x9F1E [ ];      
    unsigned char tag_0x9F79 [11 + 2];   //电子现金余额
    //unsigned char tag_0x [ ];  

    /*custom data*/
    unsigned char balance[4];           //电子现金余额
  
}YL_CPUCARD_Info_t;



int  Read_YL_CPU_Card_info(unsigned char *ar_file_index,unsigned char file_index_len, YL_CPUCARD_Info_t *p_card_info);
int YL_ReadCardInfor_CPU(YL_CPUCARD_Info_t *pt_CardInfo);

#endif
