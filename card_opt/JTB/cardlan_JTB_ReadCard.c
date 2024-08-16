
#include "cardlan_JTB_ReadCard.h"
#include "libcardlan_StandardBus_util.h"
#include "mcu_opt/psam_opt.h"
#include "card_opt/apdu_cmd.h"

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

extern MonthlyTicket monthylyticket;
extern unsigned char PsamNum_bak1[6];            //作为卡联cpu卡流程psamNum的备份，在执行卡联cpu卡流程时使用
extern unsigned char PsamNum_bak2[6];            //备份交通部的psamnum
extern struct Permisson CardPermisson;
extern CardInform CardLan;
extern CardInformCPU CardLanCPU;
extern LongUnon HostValueS;
extern LongUnon HostValueH;
extern LongUnon HostValueY;
extern LongUnon DecValue;
extern JackRegal Sector;
extern MonthlyTicket monthylyticket;
extern SectionFarPar Section;
extern SectionFarPar Sectionup;
extern Transfer CardTransfer;
extern BigUnion BigData;
extern LongUnon HostValue;
extern BigUnion NewDriver;


static unsigned char receive_buf[128]= {0};
static int  receive_len[1] = {0};

extern unsigned long long big_data;
extern unsigned char g_Fgkongtiaoflag;
extern unsigned char NandBuf[512];
extern int  Psamreceive_len[1];
extern char Psamreceive_buf[128];
extern st_WhiteFile WhiteListFile;
extern SysTime Time;
extern time_t utc_time;
int select_and_match_app_by_AID_list(    APPDATA_T *p_match_app)
{
    int ret = 0;
    TERMAPP_T *pt_aid_list = NULL;
    int num = -1;
    unsigned char app_list_num = 0;
    
    if(p_match_app == NULL)
    {
        return -1;
    }
    
    ret = get_terminal_support_AID(&pt_aid_list,&num);
    if(ret != 0)
    {
        return -2;
    }
    
    memcpy(p_match_app->AID,pt_aid_list[0].AID,pt_aid_list[0].AIDLen);
    p_match_app->AIDLen = pt_aid_list[0].AIDLen;
    p_match_app->AIDExist = 1;
    return 0;

}

int select_and_match_app_by_PSE(       const unsigned char *PPSE,unsigned char PPSE_len,APPDATA_T *p_match_app)
{
    int match_App_index = 0;
    int match_App_Num = 0;
    int ret = 0;
    
    RECORD_PPSE recordPSE;
    unsigned char SW[2];

    if(p_match_app == NULL)
    {
        return -1;
    }

    ret = apdu_cmd_select_PPSE(PPSE,PPSE_len, &recordPSE, SW);
    if(ret != 0)
    {
       return -2;
    }
    
    /*匹配应用*/
    TERMAPP_T *pt_aid_list = NULL;
    int num = -1;
    unsigned char app_list_num = 0;
    ret = get_terminal_support_AID(&pt_aid_list,&num);
    if(ret != 0)
    {
        /*not found*/
        LOGI("[ %s %d ]: terminal support AID not found !.\n",__FUNCTION__,__LINE__);
        return -3;
    }

    app_list_num = num; 
    int index = 0;
    for(index = 0 ; index < recordPSE.Appnum ; index++)
    {
        match_App_index = MatchTermAID( recordPSE.AppList[index].AID , recordPSE.AppList[index].AIDLen, pt_aid_list,app_list_num);    
        if(match_App_index < 0)
        {
            continue;
        }
        /*加入候选列表*/ 
        memcpy(p_match_app, &recordPSE.AppList[index] , sizeof(APPDATA_T));                
        //match_app.bLocalName = pt_aid_list[match_App_index].bLocalName;
        //match_app.AppLocalNameLen = pt_aid_list[match_App_index].AppLocalNameLen;
        //match_app.AppLocalName,pt_aid_list[match_App_index].AppLocalName,pt_aid_list[match_App_index].AppLocalNameLen;
        p_match_app->AIDExist = 1;
        match_App_Num++;
        //memcpy(&match_app_list,&pt_aid_list[match_App_Num],sizeof(TERMAPP_T));
    }
    
    if(match_App_Num <= 0)
    {
        LOGI("[ %s %d ]: AID not matched !.\n",__FUNCTION__,__LINE__);
        return -4;
    }
    LOGI("[%s %d]建立候选列表完成\n",__FUNCTION__,__LINE__);
    return 0;
}


int select_and_match_app(unsigned char *PPSE,unsigned char PPSE_len)
{
    APPDATA_T match_app;
    int ret = 0 ;
    
    /* 方法1 : pse选择方法 */
    do
    {
        ret = select_and_match_app_by_PSE(PPSE,PPSE_len,&match_app);
        if(ret == 0 && match_app.AIDExist != 0)
        {
           break;
        }
        /* 方法2 : 应用列表选择方法 */
        ret = select_and_match_app_by_AID_list(&match_app); 
        if(ret == 0 && match_app.AIDExist != 0)
        {
           break;
        }
    }
    while (0);
    
  
    if(ret != 0 || match_app.AIDExist == 0)
    {
        LOGI("[ %s %d ]: can not match app !.\n",__FUNCTION__,__LINE__);
        return -2;
    } 

    //for match num
    {
        unsigned char SW[2];
        unsigned char Recv[256] = {0};

        ret = apdu_cmd_select_APP(match_app.AID,match_app.AIDLen,Recv,SW); 
        if(ret < 0 || SW[0] != 0x90 || SW[1] != 0x00)
        {
            if((SW[0]==0x62)&&(SW[1]==0x83))
            {
                return -3;
            }
            else if((SW[0]==0x6A)&&(SW[1]==0x82))
            {
                return -4;
            }
            return -5;  //continue loop
        }
    }
    return 0;
}



int  Read_JTB_CPU_Card_info(unsigned char *ar_file_index,unsigned char file_index_len, JTB_CPUCARD_info_t *p_card_info)
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
    
    memset(p_card_info,0,sizeof(JTB_CPUCARD_info_t));

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
            case GET_JTB_BIN_0x15:      /* 0015 公共应用基本文件 */
                {
                    memset(receive_buf,0, sizeof(receive_buf));
                    result = apdu_cmd_read_binary1(Lsh(0x01,7)|0x15, 0x00, receive_buf,SW); 
                    if(result != 30 || SW[0] != 0x90 || SW[1] != 0x00)
                    {
                        LOGI("[%s %d ]result error : %d SW[0]=0x%02X SW[1]=0x%02X ----\n",__FUNCTION__,__LINE__,result,SW[0],SW[1]);
                        break;
                    }

                    memcpy(p_card_info->issuerlabel_0015,      &receive_buf[0],8);    
                    memcpy(&p_card_info->apptypelabel_0015,    &receive_buf[8],1);
                    memcpy(&p_card_info->issuerappversion_0015,&receive_buf[9],1);
                    memcpy(p_card_info->appserialnumber_0015,  &receive_buf[10],10);
                    memcpy(p_card_info->appstarttime_0015,     &receive_buf[20],4);
                    memcpy(p_card_info->appendtime_0015,       &receive_buf[24],4);
                    memcpy(p_card_info->FCI_0015,              &receive_buf[28],2);
                    
                }
                break;
         
            case GET_JTB_BIN_0x16:     /* 0016 持卡人基本信息 */
                {
                    memset(receive_buf,0, sizeof(receive_buf));
                    result = apdu_cmd_read_binary1(Lsh(0x01,7)|0x16,0x00, receive_buf,SW);
                    if(result != 55 || SW[0] != 0x90 || SW[1] != 0x00)
                    {
                        LOGI("[%s %d ]result error : %d SW[0]=0x%02X SW[1]=0x%02X ----\n",__FUNCTION__,__LINE__,result,SW[0],SW[1]);
                        break;
                    }

                    memcpy(&p_card_info->CardFlag_0016,             &receive_buf[0],1);    
                    memcpy(&p_card_info->Bank_Employees_0016,       &receive_buf[1],1);   
                    memcpy(p_card_info->Cardholder_Name_0016,       &receive_buf[2],20);    
                    memcpy(p_card_info->Cardholder_ID_Number_0016,  &receive_buf[22],32);   
                    memcpy(&p_card_info->Cardholder_ID_type_0016,   &receive_buf[54],1);    
                }
                break;
            case GET_JTB_RECORD_0x17:  /* 0017 管理信息文件 */
                {
                    memset(receive_buf,0, sizeof(receive_buf));
                    result = apdu_cmd_read_binary1(Lsh(0x01,7)|0x17,0x00, receive_buf,SW);
                    if(result != 60 || SW[0] != 0x90 || SW[1] != 0x00)
                    {
                        LOGI("[%s %d ]result error : %d SW[0]=0x%02X SW[1]=0x%02X ----\n",__FUNCTION__,__LINE__,result,SW[0],SW[1]);
                        break;
                    }

                    memcpy(p_card_info->countrycode_0017,    &receive_buf[0],4);    
                    memcpy(p_card_info->provincecode_0017,   &receive_buf[4],2);
                    memcpy(p_card_info->citycode_0017,       &receive_buf[6],2);
                    memcpy(p_card_info->unioncardtype_0017,  &receive_buf[8],2);
                    memcpy(&p_card_info->cardtype_0017,      &receive_buf[10],1);
                    memcpy(p_card_info->reserve_0017,        &receive_buf[11],49);
                }
                break;
            case GET_JTB_RECORD_0x18: /* 0018 交易记录文件      */
                {  
                    int index = 1;
                    result = apdu_cmd_read_record(index,Lsh(0x18,3)|0x04, receive_buf,SW);
                    if(SW[0] != 0x90 || SW[1] != 0x00)
                    {
                        LOGI("[%s %d ]result error : %d SW[0]=0x%02X SW[1]=0x%02X ----\n",__FUNCTION__,__LINE__,result,SW[0],SW[1]);
                        break;
                    }
                    memcpy(&p_card_info->record_0018[index - 1],receive_buf,sizeof(t_record_0018));
                }
                break;
            case GET_JTB_RECORD_0x1A:  /* 001A  互联互通变长记录文件 */
                {
                    int index = 2;
                    result = apdu_cmd_read_record(0x02,Lsh(0x1A,3)|0x04, receive_buf,SW);
                    if(SW[0] != 0x90 || SW[1] != 0x00)
                    {
                        LOGI("[%s %d ]result error : %d SW[0]=0x%02X SW[1]=0x%02X ----\n",__FUNCTION__,__LINE__,result,SW[0],SW[1]);
                        break;
                    }
                    memcpy(&p_card_info->record_001A_2,receive_buf,sizeof(t_record_0018));
                }
                break;
            case GET_JTB_RECORD_0x1E:  /* 001E  互联互通循环记录文件 */ 
                {
                    int index = 1;
                    result = apdu_cmd_read_record(index,Lsh(0x1E,3)|0x04, receive_buf,SW);
                    if(SW[0] != 0x90 || SW[1] != 0x00)
                    {
                        LOGI("[%s %d ]result error : %d SW[0]=0x%02X SW[1]=0x%02X ----\n",__FUNCTION__,__LINE__,result,SW[0],SW[1]);
                        break;
                    }
                    memcpy(&p_card_info->record_001E[index -1],receive_buf,sizeof(t_record_001E));
                }
                break;
            case GET_JTB_BIN_0x05:  /* 0005 应用控制信息文件 */
                {
                    result = apdu_cmd_read_binary1(Lsh(0x01,7)|0x05, 0x00 , receive_buf,SW);
                    if(SW[0] != 0x90 || SW[1] != 0x00)
                    {
                        printf("[%s %d]result error : %d SW1:%02X SW2:%02X\n",__FUNCTION__,__LINE__,result,SW[0],SW[1]);
                        break;
                    }
                     
                    memcpy(p_card_info->RFU_1_0005,                 receive_buf + 0,2);    
                    memcpy(p_card_info->RFU_2_0005,                 receive_buf + 2,2);
                    memcpy(p_card_info->industry_code_0005,         receive_buf + 4,2);
                    memcpy(p_card_info->Card_Version_0005,          receive_buf + 6,2);
                    memcpy(p_card_info->appflag_0005,               receive_buf + 8,1);
                    memcpy(p_card_info->Card_flag_0005,             receive_buf + 9,1);
                    memcpy(p_card_info->Connectivity_falg_0005,     receive_buf + 10,2);    
                    memcpy(p_card_info->reserve_0005_1,             receive_buf + 12,8);
                    memcpy(p_card_info->reserve_0005_2,             receive_buf + 20,4);
                    memcpy(p_card_info->reserve_0005_3,             receive_buf + 24,4);
                    memcpy(p_card_info->main_card_type_0005,        receive_buf + 28,1);
                    memcpy(p_card_info->sub_card_type_0005,         receive_buf + 29,1);
                    memcpy(p_card_info->deposit_0005,               receive_buf + 30,1);    
                    memcpy(p_card_info->Date_Of_Annual_Survey_0005, receive_buf + 31,4);
                    memcpy(p_card_info->Business_bitmap_0005,       receive_buf + 35,4);

                }
                break;  
            case 0x19:   /* 0019  本地复合交易记录文件 */
                {
                
                }
                break;  
            case GET_JTB_BALANCE:
                {
                    result =  apdu_cmd_get_balance1(p_card_info->balance);
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

unsigned char ReadCardInfor_CPU(JTB_CPU_CardInfo *pt_JTB_CardInfo)
{
    int result;
    char Recv[256];
    char binname[20];
    unsigned char flag,t,status,len;
    static unsigned char receive_buf[128]= {0};
    static int        receive_len[1] = {0};

    char tempbuf[8],tempbuf1[8];   
    char timebuff[7];
    unsigned char i;    
    unsigned short temp;

    LOGI("ReadCardInfor_CPU() is called.\n");

    flag = 1;
    t = 1;
    while(flag)
    {
        LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
        switch(t)
        {
            case 1:    /* 1.决定终端和卡片支持的应用 */
                {
                    int ret = 0 ;
                    {
                        unsigned char ar_file_index[] = {GET_JTB_BIN_0x05,GET_JTB_BIN_0x15,GET_JTB_BIN_0x16,GET_JTB_RECORD_0x17,GET_JTB_RECORD_0x18,GET_JTB_RECORD_0x1A,GET_JTB_RECORD_0x1E,GET_JTB_BALANCE};
                        unsigned char file_index_len = sizeof(ar_file_index)/sizeof(unsigned char);
                        JTB_CPUCARD_info_t card_info;
                        ret = Read_JTB_CPU_Card_info(ar_file_index,file_index_len,&card_info);
                        if(ret != 0 )
                        {
                            printf("[ %s %d ]ret : %d \n",__FUNCTION__,__LINE__,ret);
                            return -2;
                        }
                        /*0x05*/
                        memcpy(&pt_JTB_CardInfo->appflag,card_info.appflag_0005, 1);
                        memcpy(&pt_JTB_CardInfo->mastercardtype,card_info.main_card_type_0005,1);               
                    
                        /*0x15*/
                        memcpy(pt_JTB_CardInfo->issuerlabel,      card_info.issuerlabel_0015,8);    
                        memcpy(&pt_JTB_CardInfo->apptypelabel,    card_info.apptypelabel_0015,1);
                        memcpy(&pt_JTB_CardInfo->issuerappversion,card_info.issuerappversion_0015,1);
                        memcpy(pt_JTB_CardInfo->appserialnumber,  card_info.appserialnumber_0015,10);
                        memcpy(pt_JTB_CardInfo->appstarttime,     card_info.appstarttime_0015,4);
                        memcpy(pt_JTB_CardInfo->appendtime,       card_info.appendtime_0015,4);
                        
                        /*0x17*/
                        memcpy(pt_JTB_CardInfo->countrycode,     card_info.countrycode_0017,4);    
                        memcpy(pt_JTB_CardInfo->provincecode,    card_info.provincecode_0017,2);
                        memcpy(pt_JTB_CardInfo->citycode,        card_info.citycode_0017,2);
                        memcpy(pt_JTB_CardInfo->unioncardtype,   card_info.unioncardtype_0017,2);
                        memcpy(&pt_JTB_CardInfo->cardtype,       card_info.cardtype_0017,1);
                        //memcpy(pt_JTB_CardInfo->settlenumber,&receive_buf[11],4);

                        /*0x18*/
                        memcpy(pt_JTB_CardInfo->tradenumber,     card_info.record_0018[0].tradenumber,2);
                        memcpy(pt_JTB_CardInfo->overdraftlimit,  card_info.record_0018[0].overdraftlimit,3);
                        memcpy(pt_JTB_CardInfo->trademoney,      card_info.record_0018[0].trademoney,4);
                        pt_JTB_CardInfo->tradetype = card_info.record_0018[0].tradetype;
                        memcpy(pt_JTB_CardInfo->deviceNO,        card_info.record_0018[0].deviceNO,6);
                        memcpy(pt_JTB_CardInfo->tradedate,       card_info.record_0018[0].tradedate,4);
                        memcpy(pt_JTB_CardInfo->tradetime,       card_info.record_0018[0].tradetime,3);

                        /*0x1A*/
                        #if 0
                        pt_JTB_CardInfo->applockflag = card_info.record_001A_2.app_flag;
                        memcpy(pt_JTB_CardInfo->tradeserialnumber, card_info.record_001A_2.tradeserial_number,8);
                        pt_JTB_CardInfo->tradestate = card_info.record_001A_2.tradeserial_status;
                        memcpy(pt_JTB_CardInfo->getoncitycode,     card_info.record_001A_2.in_city_code,2);
                        memcpy(pt_JTB_CardInfo->getonissuerlabel,  card_info.record_001A_2.in_issuer_label,8);
                        //memcpy(pt_JTB_CardInfo->getonoperatorcode, card_info.record_001A_2.in_issuer_label,2);
                        memcpy(pt_JTB_CardInfo->getonline,&receive_buf[27],2);
                        pt_JTB_CardInfo->getonstation = receive_buf[29];
                        memcpy(pt_JTB_CardInfo->getonbus,&receive_buf[30],8);
                        memcpy(pt_JTB_CardInfo->getondevice,&receive_buf[38],8);
                        memcpy(pt_JTB_CardInfo->getontime,&receive_buf[46],7);
                        memcpy(pt_JTB_CardInfo->markamount,&receive_buf[53],4);
                        pt_JTB_CardInfo->directionflag = receive_buf[57];
                        memcpy(pt_JTB_CardInfo->getoffcitycode,&receive_buf[58],2);
                        memcpy(pt_JTB_CardInfo->getoffissuerlabel,&receive_buf[60],8);
                        memcpy(pt_JTB_CardInfo->getoffoperatorcode,&receive_buf[68],2);
                        memcpy(pt_JTB_CardInfo->getoffline,&receive_buf[70],2);
                        pt_JTB_CardInfo->getoffstation = receive_buf[72];
                        memcpy(pt_JTB_CardInfo->getoffbus,&receive_buf[73],8);
                        memcpy(pt_JTB_CardInfo->getoffdevice,&receive_buf[81],8);
                        memcpy(pt_JTB_CardInfo->getofftime,&receive_buf[89],7);
                        memcpy(pt_JTB_CardInfo->tradeamount,&receive_buf[96],4);
                        memcpy(pt_JTB_CardInfo->ridedistance,&receive_buf[100],2);
                        #endif                        

                        /*余额*/  
                        memcpy(pt_JTB_CardInfo->beforemoney,card_info.balance,4);
                        LOGI("\n交易前余额%d分\n",card_info.balance[0]<<24|card_info.balance[1]<<16|card_info.balance[2]<<8|card_info.balance[3]);     
                    }
   
                    return MI_OK;
                }
                break;  

            //月票卡处理过稿          
        case 24:
            {
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
                memset(binname,0,sizeof(binname));
                memcpy(binname,"\xB0\x00\x00\x06\x32\x01\x09\x01",8);
                len=8;
                status=SelectAppDF(binname,Recv,&len);
                if(status == MI_OK)
                {
                    if((receive_buf[len-2]==0x90)&&(receive_buf[len-1]==0x00))
                    {                    
                        t++;
                    }
                     else if((receive_buf[len-2]==0x62)&&(receive_buf[len-1]==0x83))
                    {
                        LOGI("出错6283\n");
                         //use_jiaotong_stander=0x0;
                        flag=0; 
                        break;
                    }
                    else if((receive_buf[len-2]==0x6A)&&(receive_buf[len-1]==0x82))
                    {
                        LOGI("出错6A82\n");
                         //use_jiaotong_stander=0x0;
                        flag=0;
                        break;
                    }

                }    
                else
                {
                    flag = 0;
                    //use_jiaotong_stander=0;
                }
                
        
            }
            break;

        case 25:                
            {
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
                memset(binname,0,sizeof(binname));
                binname[0] = 0x00;
                binname[1] = 0xb0;
                binname[2] = 0x95;  // 00015
                binname[3] = 0x00;
                binname[4] = 0x00;
                result = mifare_write(binname,5);
                //LOGI("脭脷露脕0015脦脛录镁脙禄脣脌碌么\n");
                if(result == MI_OK)
                {
                    mifare_ioctl(FIFO_RCV_LEN, receive_len);
                    result = mifare_read(receive_buf, receive_len[0]);
                    if(receive_len[0] > 2)
                    {
                        LOGI("在读0015文件没死掿");
                        menu_print(receive_buf, 30);
                        t++;
                        pt_JTB_CardInfo->yappflag = receive_buf[1];
                        memcpy(pt_JTB_CardInfo->ystarttime,receive_buf+4,4);
                        memcpy(pt_JTB_CardInfo->yendtime,receive_buf+8,4);
                        memcpy(pt_JTB_CardInfo->basicnum,receive_buf+12,2);
                        
                        }                
                    else
                    {
                        LOGI("月票0015 SW1=%02X  SW2=%02X\n",receive_buf[0],receive_buf[1]);
                        //use_jiaotong_stander=0;
                        flag = 0;
                    }
                }
                else 
                {
                    //use_jiaotong_stander=0;
                    flag = 0;
                }
            }
            break;

        case 26:
            {
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
                //      if(pt_JTB_CardInfo->yappflag!=1)
                if(0)                //test
                {
                    //use_jiaotong_stander=0x0;
                    flag=0;
                }
                else
                    t++;
            }
            break;
            
        case 27:
            { 
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
                memset(binname,0,sizeof(binname));
                binname[0] = 0x00;
                binname[1] = 0xb2; 
                binname[2] = 0x01; 
                binname[3] = 0xC4; // 0018
                binname[4] = 0x00;
                result = mifare_write(binname,5);
                if(result == MI_OK)
                {
                    mifare_ioctl(FIFO_RCV_LEN, receive_len);
                    result = mifare_read(receive_buf, receive_len[0]);
                    if(receive_len[0] > 2)
                    {
                        LOGI("月票0018记录文件返回:");
                        menu_print(receive_buf, 23);    
                        t++;
                        memcpy(pt_JTB_CardInfo->ytranssn,receive_buf,2);
                        memcpy(pt_JTB_CardInfo->ytransq,&receive_buf[2],3);
                        memcpy(pt_JTB_CardInfo->ytrasnum,&receive_buf[5],4);
                        
                        memcpy(pt_JTB_CardInfo->ytransdev,&receive_buf[10],6);
                        memcpy(pt_JTB_CardInfo->ytransdate,&receive_buf[16],4);
                        memcpy(pt_JTB_CardInfo->ytranstime,&receive_buf[20],3);
                    }
                    else
                    {
                        LOGI("0018 SW1=%02X  SW2=%02X\n",receive_buf[0],receive_buf[1]);
                        //use_jiaotong_stander=0;
                        //flag = 0;
                        t++;
                    }
                }
                else 
                {
                    //use_jiaotong_stander=0;
                    //flag = 0;
                    t++;
                }
            }
            break;
            
        case 28:  
            {
                //用于月票测试   test
                //   t++;
                //   break;

                //在同一终端第二次刷月票，则跳到现金消费
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);

                result = InitPsam(0,9600);
                if(result != MI_OK)
                {
                    return MI_FAIL;
                }
                result = GetPsamID_ex(0,PsamNum,&PsamKeyIndex);
                if(result != MI_OK)
                {
                    return MI_FAIL;
                }
                
                if((!memcmp(pt_JTB_CardInfo->ytransdev,PsamNum,6)))
                {
                    memset(binname,0,sizeof(binname));
                    memcpy(binname,"\xA0\x00\x00\x06\x32\x01\x01\x05",8);
                    len=8;
                    status=SelectAppDF(binname,Recv,&len);

                    if(status==MI_OK)      
                    {
                        if((receive_buf[len-2]==0x90)&&(receive_buf[len-1]==0x00))
                        {
                            memcpy(PsamNum,PsamNum_bak2,sizeof(PsamNum_bak2));                    
                            LOGI("使用交通部流程02:line=%d\n",__LINE__);
                        }
                        else if((receive_buf[len-2]==0x62)&&(receive_buf[len-1]==0x83))
                        {
                            LOGI("出错6283\n");
                            //use_jiaotong_stander=0x0;
                            flag=0; 
                            break;
                        }
                        else if((receive_buf[len-2]==0x6A)&&(receive_buf[len-1]==0x82))
                        {
                            LOGI("出错6A82\n");
                            //use_jiaotong_stander=0x0;
                            flag=0;
                            break;
                        }
                    }              



                    if(status!=MI_OK)
                    {
                    flag=0;
                    }

                    t=13;                       
                }
                else
                {
                    t++;
                }
            }
            break;

        case 29:
            {
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
                //status = Card_JudgeDate_YuePiao();
                if(CardPermisson.walletflag==0)
                {
                    if(status == 2)
                        {
                            //use_jiaotong_stander=0;
                            flag = 0;
                        }
                    else if(status == 1)
                    {
                        //use_jiaotong_stander=0;
                        flag = 0;
                        }
                    else
                        t++;
                }
                else
                {
                    if(status!=0)
                    {
                        memset(binname,0,sizeof(binname));
                        memcpy(binname,"\xA0\x00\x00\x06\x32\x01\x01\x05",8);
                        len=8;
                        status=SelectAppDF(binname,Recv,&len);
                        if(status == MI_OK)
                        {
                            if((receive_buf[len-2]==0x90)&&(receive_buf[len-1]==0x00))
                            {                     
                                t++;
                            }
                            else if((receive_buf[len-2]==0x62)&&(receive_buf[len-1]==0x83))
                            {
                                LOGI("出错6283\n");
                                 //use_jiaotong_stander=0x0;
                                flag=0; 
                                break;
                            }
                            else if((receive_buf[len-2]==0x6A)&&(receive_buf[len-1]==0x82))
                            {
                                LOGI("出错6A82\n");
                                 //use_jiaotong_stander=0x0;
                                flag=0;
                                break;
                            }
                          
                        }      
                        else
                        {
                            flag = 0;
                            //use_jiaotong_stander=0;
                        }    
                        t=13;
                    }
                    else
                    {
                        t++;
                    }
                }
            }
            break;

        case 30:// 查询余额
                //LOGI("查询余额\n");
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
                result = mifare_write("\x80\x5c\x00\x02\x04",5);
                if(result == MI_OK)
                {
                    mifare_ioctl(FIFO_RCV_LEN, receive_len);
                    result = mifare_read(receive_buf, receive_len[0]);
                    if(receive_len[0] > 2)
                    {
                        
                        t++;
                        //Rd_time(timebuff);
                        
                        memset(tempbuf,0,8);
                        memcpy(tempbuf,receive_buf,4);
                        memcpy(pt_JTB_CardInfo->beforemoney,receive_buf,4);
                        memset(&monthylyticket,0,sizeof(MonthlyTicket));
                        if((!memcmp(receive_buf,"\x00\xff\xff\xff",4))||(!memcmp(receive_buf,"\x00\x00\x00\x00",4)))
                        {
                            monthylyticket.tickettype = 0;
                            monthylyticket.blance.i = 0;
                          //  memset(pt_JTB_CardInfo->beforemoney,0,4);
                            }
                        else
                        {
                        temp = 0;
                        //获取月票的时间和类型，即某年某月月票、季票或年票

                                             
                        temp = (tempbuf[1]<<8|(tempbuf[2]&0xf0))>>4;
                        LOGI("取出的倿=%04x\n",temp);
                        temp =(~temp)&0x0fff;
                        LOGI("取出的倿=%04x\n",temp); 
                        
                        monthylyticket.ticketyear = temp/18;
                        monthylyticket.ticketdate = temp%18;
                        if(monthylyticket.ticketdate==13)
                            monthylyticket.tickettype = 1;
                        if(monthylyticket.ticketdate==14)
                            monthylyticket.tickettype = 2;
                        if(monthylyticket.ticketdate==15)
                            monthylyticket.tickettype = 3;
                        if(monthylyticket.ticketdate==16)
                            monthylyticket.tickettype = 4;
                        if(monthylyticket.ticketdate==17)
                            monthylyticket.tickettype = 5;
                        else
                            monthylyticket.tickettype= 0;

                        //获取余额次数
                        temp = 0;
                        memcpy(&temp,tempbuf+2,2);
                        temp =temp&0x0fff;
                    
                       monthylyticket.blance.intbuf[1]=tempbuf[2]&0x0f;
                       monthylyticket.blance.intbuf[0]=tempbuf[3]; 
                        
                        LOGI("\n交易前月票余颿:%02x %02x麓脦\n",monthylyticket.blance.intbuf[0],monthylyticket.blance.intbuf[1]);
                        //monthylyticket.blance.i = temp;
                        LOGI("交易前月票余颿\n",monthylyticket.ticketyear,monthylyticket.ticketdate);

                        
                            }
                     //   memcpy(pt_JTB_CardInfo->beforemoney, monthylyticket.blance.intbuf,2);
                        LOGI("\n交易前月票余颿%02x %02x %02x %02x次\n",receive_buf[0],receive_buf[1],receive_buf[2],receive_buf[3]);
                        LOGI("\n交易前月票余颿%d次\n",monthylyticket.blance.i);
                        
                            
                    }
                    else
                    {
                        LOGI("查询余额 SW1=%02X  SW2=%02X\n",receive_buf[0],receive_buf[1]);
                        flag = 0;
                    }
                }
                else {
                    flag = 0;
                    //use_jiaotong_stander=0;
                    }
                break;

         case 31:
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
             monthylyticket.rechargflag = 0;
            switch(monthylyticket.tickettype)
            {
                case 5:
                    LOGI("年卡\n");
                    if(monthylyticket.ticketyear==BCDToDec(&timebuff[0],1))
                            monthylyticket.blanceuse.i = monthylyticket.blance.i;
                    //else if(monthylyticket.ticketyear<BCDToDec(Time.year,1))
                    //        memcpy(monthylyticket.blanceuse.intbuf,pt_JTB_CardInfo->basicnum,2);

                    break;

                case 1:
                    LOGI("碌脷1录戮驴篓\n");
                     //a.i = 0;
                   //  memcpy(a.intbuf,pt_JTB_CardInfo->basicnum,2);
                   //ChangChcek_2(pt_JTB_CardInfo->basicnum, a.intbuf);
                     if((1==BCDToDec(&timebuff[1],1))||(2==BCDToDec(&timebuff[1],1))||(3==BCDToDec(&timebuff[1],1)))
                     {                        
                            monthylyticket.blanceuse.i = monthylyticket.blance.i;                    
                        }
                     else
                     {
                         //monthylyticket.blanceuse.i = a.i; 
                         monthylyticket.rechargflag = 1;
                        }

                    break;

                case 2:
                     LOGI("碌脷2录戮驴篓\n");
                     //a.i = 0;
                   //  memcpy(a.intbuf,pt_JTB_CardInfo->basicnum,2);
                   //ChangChcek_2(pt_JTB_CardInfo->basicnum, a.intbuf);
                     if((4==BCDToDec(&timebuff[1],1))||(5==BCDToDec(&timebuff[1],1))||(6==BCDToDec(&timebuff[1],1)))
                     {                        
                            monthylyticket.blanceuse.i = monthylyticket.blance.i;                    
                        }
                     else
                     {
                         //monthylyticket.blanceuse.i = a.i;  
                         monthylyticket.rechargflag = 1;
                        }
                     
                    break;

                case 3:
                     LOGI("碌脷3录戮驴篓\n");
                     //a.i = 0;
                   //  memcpy(a.intbuf,pt_JTB_CardInfo->basicnum,2);
                    //ChangChcek_2(pt_JTB_CardInfo->basicnum, a.intbuf);
                     if((7==BCDToDec(&timebuff[1],1))||(8==BCDToDec(&timebuff[1],1))||(9==BCDToDec(&timebuff[1],1)))
                     {                        
                            monthylyticket.blanceuse.i = monthylyticket.blance.i;                    
                        }
                     else
                     {
                         //monthylyticket.blanceuse.i = a.i;  
                         monthylyticket.rechargflag = 1;
                        }                    
                     break;   

            
                case 4:
                     LOGI("碌脷4录戮驴篓\n");
                     //a.i = 0;
                     //ChangChcek_2(pt_JTB_CardInfo->basicnum, a.intbuf);
                   //  memcpy(a.intbuf,pt_JTB_CardInfo->basicnum,2);
                     if((10==BCDToDec(&timebuff[1],1))||(11==BCDToDec(&timebuff[1],1))||(12==BCDToDec(&timebuff[1],1)))
                     {                        
                            monthylyticket.blanceuse.i = monthylyticket.blance.i;                    
                        }
                     else
                     {
                         //monthylyticket.blanceuse.i = a.i;  
                         monthylyticket.rechargflag = 1;
                        }                    
                     break;

                default:
                     LOGI("脭脗驴篓\n");
                     //a.i = 0;
                   //  memcpy(a.intbuf,pt_JTB_CardInfo->basicnum,2);
                   //ChangChcek_2(pt_JTB_CardInfo->basicnum, a.intbuf);
                     //LOGI("禄霉卤戮麓脦脢媒拢陆%02x %02x\n",a.intbuf[0],a.intbuf[1]);
                     LOGI("脭脗脝卤脢鹿脫脙脢卤录盲:=%d 脭脗\n",monthylyticket.ticketdate);
                     LOGI("碌卤脟掳脢卤录盲:=%d 脭脗\n",BCDToDec(&timebuff[1],1));
                    if(monthylyticket.ticketdate==BCDToDec(&timebuff[1],1))
                    {
                        LOGI("1111111\n");
                        monthylyticket.blanceuse.i = monthylyticket.blance.i;
                        }
                    else if(monthylyticket.ticketdate<BCDToDec(&timebuff[1],1))
                    {
                        LOGI("2222222\n");
                        //monthylyticket.blanceuse.i = a.i; 
                        monthylyticket.rechargflag = 1;
                        }                        

                    break;
                
                }                         
                //脭脗脝卤驴脡脫脙脫脿露卯
                memcpy(CardLan.Views,monthylyticket.blanceuse.intbuf,2);
                LOGI("脭脗脝卤碌卤脟掳脭脗驴脡脫脙脫脿露卯:%d\n",monthylyticket.blanceuse.i);
                //脫脨脟庐掳眉脠篓脧脼拢卢脭脷碌卤脭脗脛脷脫脿露卯虏禄脳茫
                if((CardPermisson.walletflag==1)&&(monthylyticket.blanceuse.i<HostValueS.i)&&(monthylyticket.rechargflag==0))            
                {

                   memset(binname,0,sizeof(binname));
                   memcpy(binname,"\xA0\x00\x00\x06\x32\x01\x01\x05",8);
                   len=8;
                   status=SelectAppDF(binname,Recv,&len);
                   if(status == MI_OK)
                     {
                       if((receive_buf[len-2]==0x90)&&(receive_buf[len-1]==0x00))
                      {                       
                       t++;
                       }
                        else if((receive_buf[len-2]==0x62)&&(receive_buf[len-1]==0x83))
                       {
                           LOGI("鲁枚麓铆6283\n");
                            //use_jiaotong_stander=0x0;
                           flag=0; 
                           break;
                           }
                       else if((receive_buf[len-2]==0x6A)&&(receive_buf[len-1]==0x82))
                       {
                               LOGI("鲁枚麓铆6A82\n");
                                //use_jiaotong_stander=0x0;
                               flag=0;
                               break;
                           }
                       
                   }    
                   else {
                       flag = 0;
                       //use_jiaotong_stander=0;
                       }                    
                    t=13;
                }
                else  if((monthylyticket.blanceuse.i<HostValueS.i)&&( monthylyticket.rechargflag==0))
                    {
                    //use_jiaotong_stander=0;
                    flag = 0;
                }
                else
                    t++;
            break;

            case 32:
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
                flag = 0;                 
                pt_JTB_CardInfo->yflag = 1;
                //use_jiaotong_stander==0x55;
                break;
            

            case 33:
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
                memset(binname,0,sizeof(binname));
                binname[0] = 0x00;
                binname[1] = 0xb0;
                binname[2] = 0x96;    // 00016
                binname[3] = 0x00;
                binname[4] = 0x00;
                result = mifare_write(binname,5);
                if(result == MI_OK)
                {
                    mifare_ioctl(FIFO_RCV_LEN, receive_len);
                    result = mifare_read(receive_buf, receive_len[0]);
                    if(receive_len[0] > 2)
                    {
                    LOGI("0016脦脛录镁路碌禄脴:");
                        menu_print(receive_buf, receive_len[0]);
                        memcpy(pt_JTB_CardInfo->drivername,receive_buf+2,20);
                        memcpy(pt_JTB_CardInfo->CertiID,receive_buf+22,18);
                        //t++;                    
                        
                    }
                    else
                    {
                        LOGI("0016 SW1=%02X  SW2=%02X\n",receive_buf[0],receive_buf[1]);
                        //use_jiaotong_stander=0;
                        flag = 0;
                    }
                }
                else 
                    {
                    //use_jiaotong_stander=0;
                    flag = 0;
                    }

                //脭枚录脫露脕0x17脦脛录镁拢卢脠隆鲁脟脢脨麓煤脗毛 
                memset(binname,0,sizeof(binname));
                binname[0] = 0x00;
                binname[1] = 0xb0;
                binname[2] = 0x97;  // 00017
                binname[3] = 0x00;
                binname[4] = 0x00;
                result = mifare_write(binname,5);
                if(result == MI_OK)
                {
                    mifare_ioctl(FIFO_RCV_LEN, receive_len);
                    result = mifare_read(receive_buf, receive_len[0]);
                    if(receive_len[0] > 2)
                    {
                    LOGI("0017脦脛录镁路碌禄脴:");
                        menu_print(receive_buf, 15);    
                        t++;
                        memcpy(pt_JTB_CardInfo->countrycode,receive_buf,4);    
                        memcpy(pt_JTB_CardInfo->provincecode,&receive_buf[4],2);
                        memcpy(pt_JTB_CardInfo->citycode,&receive_buf[6],2);
                        memcpy(pt_JTB_CardInfo->unioncardtype,&receive_buf[8],2);
                        pt_JTB_CardInfo->cardtype = receive_buf[10];
                        memcpy(pt_JTB_CardInfo->settlenumber,&receive_buf[11],4);
                        
                    }
                    else
                    {
                        LOGI("0017 SW1=%02X  SW2=%02X\n",receive_buf[0],receive_buf[1]);
                        //use_jiaotong_stander=0;
                        flag = 0;
                    }
                }
                else 
                    {
                    //use_jiaotong_stander=0;
                    flag = 0;
                    }

                break;
                
                case 34:
                    LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
                    //脜脨露脧驴篓脝卢脢脟路帽脢脟潞脷脙没碌楼驴篓
                    //LOGI("脜脨露脧潞脷脙没碌楼\n");
                    //status    = Card_JudgeCsn_Cpu_jiaotong();
                    if(status == MI_OK)t++;
                    else
                    {
                        //脢脟潞脷脙没碌楼,脭貌脕脵脢卤脣酶露篓脫娄脫脙
                            GET_MAC();
                        //脨猫脪陋虏煤脡煤脣酶驴篓录脟脗录
                        //IncSerId();
                        //SaveCardData_jiaotong(CARD_SPEC_CPU_PBCO20, CONSUME_MODE_LOCK, GET_RECORD | SAVE_RECORD); //卤拢麓忙脢媒戮脻  露篓露卯
                        //use_jiaotong_stander=0;
                        flag = 0;
                    }
                    break;
            case 35:
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
                //if(Check_CardDate()!=0)
                //{
                //    //use_jiaotong_stander=0;
                //    flag = 0;                    
                //    }
                //else
                //    {
                //    t++;
                //    }
                //break;

            case 36:
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
                //Rd_time(timebuff);
                //bcd_to_bin(pt_JTB_CardInfo->appserialnumber, 10);
                BigData.i = 0;
                BigData.i = big_data;                
    
                if(NewDriver.i==0)
                {
                    memcpy(NewDriver.buff,BigData.buff,8);
                    //ReadOrWriteFileB(DRIVER);
                    pt_JTB_CardInfo->enterexitflag = 0;
                    t++;
                }
                else if(memcmp(NewDriver.buff,BigData.buff,8)==0)
                    {
                        NewDriver.i = 0;
                        //ReadOrWriteFileB(DRIVER);
                        pt_JTB_CardInfo->enterexitflag = 1;
                        t++;
                    }
                else
                    {
                //    use_jiaotong_stander=0;
                //    flag = 0;                    
                    memcpy(NewDriver.buff,BigData.buff,8);
                    //ReadOrWriteFileB(DRIVER);
                    pt_JTB_CardInfo->enterexitflag = 0;
                    t++;
                    }

                break;


            case 37:
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
                flag = 0;  
                CardLan.CardType = 0xcc;
                //use_jiaotong_stander==0x55;
                break;                

            //脧脽脗路虏脦脢媒驴篓
            case 38:
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
               memset(binname,0,sizeof(binname));
               memcpy(binname,"\xB0\x00\x00\x06\x32\x01\x09\x03",8);
                   len=8;
               status=SelectAppDF(binname,Recv,&len);
               if(status == MI_OK)
                 {
                   if((receive_buf[len-2]==0x90)&&(receive_buf[len-1]==0x00))
                  {                       
                   t++;
                   }
                    else if((receive_buf[len-2]==0x62)&&(receive_buf[len-1]==0x83))
                   {
                       LOGI("鲁枚麓铆6283\n");
                        //use_jiaotong_stander=0x0;
                       flag=0; 
                       break;
                       }
                   else if((receive_buf[len-2]==0x6A)&&(receive_buf[len-1]==0x82))
                   {
                           LOGI("鲁枚麓铆6A82\n");
                            //use_jiaotong_stander=0x0;
                           flag=0;
                           break;
                       }
                   
               }    
               else {
                   flag = 0;
                   //use_jiaotong_stander=0;
                   }
                   
           
               break;

           case 39:                   
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
               memset(binname,0,sizeof(binname));
               binname[0] = 0x00;
               binname[1] = 0xb0;
               binname[2] = 0x95;  // 00015
               binname[3] = 0x00;
               binname[4] = 0x00;
               result = mifare_write(binname,5);
               //LOGI("脭脷露脕0015脦脛录镁脙禄脣脌碌么\n");
               if(result == MI_OK)
               {
                   mifare_ioctl(FIFO_RCV_LEN, receive_len);
                   result = mifare_read(receive_buf, receive_len[0]);
                   if(receive_len[0] > 2)
                   {
                       LOGI("脧脽脗路0015脦脛录镁路碌禄脴:");
                       menu_print(receive_buf, 40);
                       t++;
                       pt_JTB_CardInfo->lcardtype = receive_buf[0];
                       pt_JTB_CardInfo->lcmethod= receive_buf[25];
                       memcpy(pt_JTB_CardInfo->lstarttime,receive_buf+11,4);
                       memcpy(pt_JTB_CardInfo->lendtime,receive_buf+15,4);
                       memcpy(pt_JTB_CardInfo->lnum,&receive_buf[19],6);
                       pt_JTB_CardInfo->lupnums = receive_buf[35];
                       pt_JTB_CardInfo->ldownnums = receive_buf[36];
                       pt_JTB_CardInfo->lcardtypenums = receive_buf[39];

                       memset(&Section.SationNum[0],0,sizeof(SectionFarPar));
                       if(pt_JTB_CardInfo->lcmethod==0)
                        Section.Enable = 0;
                       else
                        {
                          Section.Enable = 0x55;
                          Section.SationNum[0] = pt_JTB_CardInfo->lupnums;
                          Sectionup.SationNum[0]=pt_JTB_CardInfo->ldownnums;
                        }
                     //  for(i=0;i<6;i++)
                     //        Section.Linenum[i]=(pt_JTB_CardInfo->lnum[i]&0xf0>>4)*10+pt_JTB_CardInfo->lnum[i]&0x0f;
                        memcpy(Section.Linenum,pt_JTB_CardInfo->lnum,6);
                       
                       //ReadOrWriteFile(SETSECTIONLINE);
                       //ReadOrWriteFile(SETSECTION);
                       //ReadOrWriteFile(SETSECTIONUP);
                       }                
                   else
                   {
                       LOGI("脧脽脗路0015 SW1=%02X  SW2=%02X\n",receive_buf[0],receive_buf[1]);
                       //use_jiaotong_stander=0;
                       flag = 0;
                   }
                }
               else 
                   {
                       //use_jiaotong_stander=0;
                       flag = 0;
                }
               break;

        case 40:
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
            if(pt_JTB_CardInfo->lcardtype!=0x80)
                {
                   //use_jiaotong_stander=0;
                   flag = 0;
                
                }
            else
                {
                t++;                    
            }
            
            break;

        case 41:
            LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
            t++;
            break;

        case 42:    
            LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
              //露脕0018脦脛录镁
              memset(binname,0,sizeof(binname));
              binname[0] = 0x00;
              binname[1] = 0xb0;
              binname[2] = 0x98;  // 00018
              binname[3] = 0x00;
              binname[4] = 0x23;
              result = mifare_write(binname,5);
             
              if(result == MI_OK)
              {
                  mifare_ioctl(FIFO_RCV_LEN, receive_len);
                  result = mifare_read(receive_buf, receive_len[0]);
                  if(receive_len[0] > 2)
                  {
                      menu_print(receive_buf, 35);
                      t++;
                      memcpy(pt_JTB_CardInfo->lticketlimit,&receive_buf[13],4);
                      memcpy(pt_JTB_CardInfo->linsbalance,&receive_buf[23],2);
                      g_Fgkongtiaoflag = receive_buf[25];
                      memcpy(pt_JTB_CardInfo->lyupiaoprice,&receive_buf[26],2);
                      memcpy(pt_JTB_CardInfo->lputongprice,&receive_buf[28],2);
                      pt_JTB_CardInfo->luniondisrate = receive_buf[30];
                      pt_JTB_CardInfo->ldistimeaddrate = receive_buf[31];
                      pt_JTB_CardInfo->lwallettransfer1st = receive_buf[32];
                      pt_JTB_CardInfo->laddrate = receive_buf[33];
                      pt_JTB_CardInfo->lkongtiaorate = receive_buf[34];
                      //status = Para_cardlan1(&receive_buf[26],4096,9,0);
                      //Read_Parameter();
                      //ReadOrWriteFile(KONGTIAO);
                      }                   
                  else
                  {
                      LOGI("脧脽脗路0018 SW1=%02X  SW2=%02X\n",receive_buf[0],receive_buf[1]);
                      //use_jiaotong_stander=0;
                      flag = 0;
                  }
               }
              else 
                  {
                      //use_jiaotong_stander=0;
                      flag = 0;
               }
              break;

        case 43:    
              //露脕0019脦脛录镁
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
              memset(binname,0,sizeof(binname));
              binname[0] = 0x00;
              binname[1] = 0xb0;
              binname[2] = 0x99;  // 00018
              binname[3] = 0x00;
              binname[4] = 0x18;
              result = mifare_write(binname,5);
        
              if(result == MI_OK)
              {
                  mifare_ioctl(FIFO_RCV_LEN, receive_len);
                  result = mifare_read(receive_buf, receive_len[0]);
                  if(receive_len[0] > 2)
                  {
                      LOGI("脧脽脗路0019脦脛录镁路碌禄脴:");
                      menu_print(receive_buf, 24);
                      t++;
                      memcpy(pt_JTB_CardInfo->lnewdiscountstarttime,&receive_buf[0],4);
                      memcpy(pt_JTB_CardInfo->lnewdiscountendtime,&receive_buf[4],4);
                      pt_JTB_CardInfo->lnewdiscountflag = receive_buf[8];
                      pt_JTB_CardInfo->lchecktimeflag = receive_buf[10];
                      memcpy(&CardTransfer.ltransferstarttime[0],&receive_buf[11],sizeof(Transfer));
                      //ReadOrWriteFile(TRANSFER);
                      
                  }                   
                  else
                  {
                      LOGI("脧脽脗路0018 SW1=%02X  SW2=%02X\n",receive_buf[0],receive_buf[1]);
                      //use_jiaotong_stander=0;
                      flag = 0;
                  }
               }
              else 
              {
                      //use_jiaotong_stander=0;
                      flag = 0;
               }
              break;

              case 44:
                LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
                //露脕脩颅禄路录脟脗录脦脛录镁    
                    //status = Para_cardlan1(tempbuf,4105,sizeof(tempbuf),0);
                for(i=1;i<pt_JTB_CardInfo->lcardtypenums+1;i++)
                {
                    memset(binname,0,sizeof(binname));
                    binname[0] = 0x00;
                    binname[1] = 0xb2; 
                    binname[2] = i; 
                    binname[3] = 0xd4; // 001A
                    binname[4] = 0x00;
                    result = mifare_write(binname,5);
                    if(result == MI_OK)
                    {
                      mifare_ioctl(FIFO_RCV_LEN, receive_len);
                      result = mifare_read(receive_buf, receive_len[0]);
                      if(receive_len[0] > 2)
                      {
                          LOGI("001A录脟脗录脦脛录镁路碌禄脴:");
                          menu_print(receive_buf, receive_len[0]);      
                          //status = Para_cardlan1(receive_buf,4105+(i-1)*22,receive_len[0],0);
                      }
                      else
                      {
                          LOGI("001A SW1=%02X  SW2=%02X\n",receive_buf[0],receive_buf[1]);
                          //use_jiaotong_stander=0;
                          flag = 0;
                      }
                    }
                    else 
                      {
                          //use_jiaotong_stander=0;
                          flag = 0;
                      }
                }
                LOGI("i=%d\n",i);
                if(i==pt_JTB_CardInfo->lcardtypenums+1)
                    {
                        tempbuf[0]=0x55;
                        tempbuf[1] = 0xAA;
                        //status = Para_cardlan1(tempbuf,4105+(i-1)*22,2,0);
                        //Read_Parameter();
                        t++;
                        CardLan.CardType=0xcd;
                    }
                else
                    {
                     //use_jiaotong_stander=0;
                      flag = 0;
                    }
        break;
        default:
            flag = 0;
            break;
        }
    }

    LOGI("ReadCardInfor_CPU     End == %d \n",t);

    return t;
}



