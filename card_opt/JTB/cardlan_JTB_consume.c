#include "typea.h"
#include "libcardlan_CardInfo.h"
#include "mcu_opt/psam_opt.h"
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

static int        receive_len[1] = {0};
int DebitChas_Complex_Consume_step1(JTB_CPU_CardInfo *pt_JTB_CardInfo,unsigned int Money , unsigned char PsamNum[6],unsigned char PsamKeyIndex)
{
    unsigned char SW[2];
    unsigned char Recv[256];
    unsigned char apdu_cmd_data[0x0B];
    int ret;
    apdu_cmd_data[0] = PsamKeyIndex;
    apdu_cmd_data[1] = (Money & 0xFF000000) >> 24;
    apdu_cmd_data[2] = (Money & 0xFF0000) >> 16;
    apdu_cmd_data[3] = (Money & 0xFF00) >> 8;
    apdu_cmd_data[4] = Money & 0xFF;      
    memcpy(&apdu_cmd_data[5], PsamNum, 6);             
    
    ret = apdu_cmd_initialize_for_capp_purchase(apdu_cmd_data,Recv,SW);
    if(SW[0] != 0x90 || SW[1] != 0x00)
    {
        LOGI("[%s %d]fail result %02X SW1:%02X SW2:%02X\n",__FUNCTION__,__LINE__,ret,SW[0],SW[1]);
        return HANDLE_ERR_CMD;
    }
    
    memcpy(pt_JTB_CardInfo->offlineSN,&Recv[4],2);
    memcpy(pt_JTB_CardInfo->overdraftAmount,&Recv[6],3);
    pt_JTB_CardInfo->keyVersion = Recv[9];
    pt_JTB_CardInfo->arithmeticLabel = Recv[10];
    memcpy(pt_JTB_CardInfo->PRandom,&Recv[11],4);
    return 0;
}

int DebitChas_Complex_Consume_step2(JTB_CPU_CardInfo *pt_JTB_CardInfo,unsigned int Money,unsigned char Timebuf[7])
{
    unsigned char Send[41];
    unsigned char Recv[256]; 
    int len;
    int ret;
    memset(Recv,0,sizeof(Recv));

    LOGI("初始化后的ATC1:%02x %02x\n",pt_JTB_CardInfo->offlineSN[0],pt_JTB_CardInfo->offlineSN[1]);
    memset(Send,0,sizeof(Send));
    memcpy(Send,"\x80\x70\x00\x00\x24",5);                  //命令头
    memcpy(&Send[5],pt_JTB_CardInfo->PRandom,4);                //用户卡片随机数
    memcpy(&Send[9],pt_JTB_CardInfo->offlineSN,2);              //用户卡脱机交易序号
    Send[11] = (Money&0xFF000000)>>24;
    Send[12] = (Money&0xFF0000)>>16;
    Send[13] = (Money&0xFF00)>>8;
    Send[14] = Money&0xFF;                                  //交易金额
    Send[15] = 0x09;                                        //交易类型
    memcpy(&Send[16],Timebuf,7);                            //交易日期时间
    Send[23] = pt_JTB_CardInfo->keyVersion;                     //密钥版本号
    Send[24] = pt_JTB_CardInfo->arithmeticLabel;                //密钥算法标识
    memcpy(&Send[25],&pt_JTB_CardInfo->appserialnumber[2],8);   //用户卡号
    memcpy(&Send[33],pt_JTB_CardInfo->issuerlabel,8);           //发卡机构编码
    len = 41;
   

    LOGI("生成MAC1发送:");
    menu_print(Send, len);
    ret = Psam_Cmd_Send(0, Send, len, Recv, &len);
    
    LOGI("生成MAC1返回:");
    menu_print(Recv, len);

    if((ret != MI_OK)||(Recv[len-2] != 0x90)||(Recv[len-1] != 0x00))
    {
        LOGI("[%s %d]fail result %02X SW1:%02X SW2:%02X\n",__FUNCTION__,__LINE__,ret,Recv[0],Recv[1]);
        return HANDLE_ERR_CMD;
    }

    memcpy(pt_JTB_CardInfo->PSAMOfflineSN,Recv,4);
    memcpy(pt_JTB_CardInfo->MAC1,&Recv[4],4);      
    return 0;
}

int DebitChas_Complex_Consume_step3(JTB_CPU_CardInfo *pt_JTB_CardInfo)
{
    unsigned char SW[2];
    unsigned char Recv[256]; 
    unsigned char apdu_cmd_data[0x80];
    int ret;
    memset(apdu_cmd_data,0,sizeof(apdu_cmd_data));
    memcpy(&apdu_cmd_data[0],"\x27\x02\x7D\x01\x01",5);
    apdu_cmd_data[5] = pt_JTB_CardInfo->applockflag;
    memcpy(&apdu_cmd_data[6],pt_JTB_CardInfo->tradeserialnumber,8);
    apdu_cmd_data[14] = pt_JTB_CardInfo->tradestate;
    memcpy(&apdu_cmd_data[15],pt_JTB_CardInfo->getoncitycode,2);
    memcpy(&apdu_cmd_data[17],pt_JTB_CardInfo->getonissuerlabel,8);
    memcpy(&apdu_cmd_data[25],pt_JTB_CardInfo->getonoperatorcode,2);
    memcpy(&apdu_cmd_data[27],pt_JTB_CardInfo->getonline,2);
    apdu_cmd_data[29] = pt_JTB_CardInfo->getonstation;
    memcpy(&apdu_cmd_data[30],pt_JTB_CardInfo->getonbus,8);
    memcpy(&apdu_cmd_data[38],pt_JTB_CardInfo->getondevice,8);
    memcpy(&apdu_cmd_data[46],pt_JTB_CardInfo->getontime,7);
    memcpy(&apdu_cmd_data[53],pt_JTB_CardInfo->markamount,4);
    apdu_cmd_data[57] = pt_JTB_CardInfo->directionflag;
    memcpy(&apdu_cmd_data[58],pt_JTB_CardInfo->getoffcitycode,2);
    memcpy(&apdu_cmd_data[60],pt_JTB_CardInfo->getoffissuerlabel,8);
    memcpy(&apdu_cmd_data[68],pt_JTB_CardInfo->getoffoperatorcode,2);
    memcpy(&apdu_cmd_data[70],pt_JTB_CardInfo->getoffline,2);
    apdu_cmd_data[72] = pt_JTB_CardInfo->getoffstation;
    memcpy(&apdu_cmd_data[73],pt_JTB_CardInfo->getoffbus,8);
    memcpy(&apdu_cmd_data[81],pt_JTB_CardInfo->getoffdevice,8);
    memcpy(&apdu_cmd_data[89],pt_JTB_CardInfo->getofftime,7);
    memcpy(&apdu_cmd_data[96],pt_JTB_CardInfo->tradeamount,4);
    memcpy(&apdu_cmd_data[100],pt_JTB_CardInfo->ridedistance,2);
    ret = apdu_cmd_update_capp_data_cache2(0x02,0xD0,0x80,apdu_cmd_data,Recv,SW);
    if(SW[0] != 0x90 || SW[1] != 0x00)
    {
        LOGI("[%s %d]fail result %02X SW1:%02X SW2:%02X\n",__FUNCTION__,__LINE__,ret,SW[0],SW[1]);
        return HANDLE_ERR_CMD;
    }
    return HANDLE_OK;
}

int DebitChas_Complex_Consume_step4(JTB_CPU_CardInfo *pt_JTB_CardInfo,unsigned int Money,unsigned char Timebuf[7],unsigned char PsamNum[6])
{
    unsigned char Recv[256]; 
    unsigned char SW[2];
    int ret;
    unsigned char apdu_cmd_data[0x30];
    unsigned int beforeMoney;
    memset(apdu_cmd_data,0,sizeof(apdu_cmd_data));

    //if(Section.Enable!=0x55)                     //交易类型
    if(1)
    {
        apdu_cmd_data[0] = 0x06;
    }
    else
    {
        if(pt_JTB_CardInfo->enterexitflag == 0x55)
        {
            apdu_cmd_data[0] = 0x03;
        }
        else
        {
            apdu_cmd_data[0] = 0x04;
        }
    } 
    apdu_cmd_data[1] = 0;
    apdu_cmd_data[2] = 0;
    memcpy(&apdu_cmd_data[3],PsamNum,6);                  //终端编号
    apdu_cmd_data[9] = 0x02;                             //行业代码
    //memcpy(&Send[15],Yanzhou_Card.LineNO,2);     //线路号
    //if(Section.Enable!=0x55)                     //站点号
    if(1)
    {
        apdu_cmd_data[12] = 0x00;
        apdu_cmd_data[13] = 0x00;
    }
    else
    {
        if(pt_JTB_CardInfo->enterexitflag == 0x55)
        {
            apdu_cmd_data[12] = 0x00;
            apdu_cmd_data[13] = pt_JTB_CardInfo->getonstation;
        }
        else
        {
            apdu_cmd_data[12] = 0x00;
            apdu_cmd_data[13] = pt_JTB_CardInfo->getoffstation;
        }
    }

    //memcpy(&Send[19],Yanzhou_Card.OperatorCode,2);
    apdu_cmd_data[16] = 0;
    apdu_cmd_data[17] = (Money&0xFF000000)>>24;
    apdu_cmd_data[18] = (Money&0xFF0000)>>16;
    apdu_cmd_data[19] = (Money&0xFF00)>>8;
    apdu_cmd_data[20] = Money&0xFF;                            //交易金额
    beforeMoney=pt_JTB_CardInfo->beforemoney[0]<<24|pt_JTB_CardInfo->beforemoney[1]<<16|pt_JTB_CardInfo->beforemoney[2]<<8|pt_JTB_CardInfo->beforemoney[3];                                                    
    //交易后余额    
    apdu_cmd_data[21] = ((beforeMoney - Money)&0xFF000000)>>24;
    apdu_cmd_data[22] = ((beforeMoney - Money)&0xFF0000)>>16;
    apdu_cmd_data[23] = ((beforeMoney - Money)&0xFF00)>>8;
    apdu_cmd_data[24] = ((beforeMoney - Money)&0xFF)>>0;
    memcpy(&apdu_cmd_data[25],Timebuf,7);                    //交易时间
    memcpy(&apdu_cmd_data[32],pt_JTB_CardInfo->citycode,2);   //受理方城市代码
    memcpy(&apdu_cmd_data[34],pt_JTB_CardInfo->issuerlabel,8);  //受理方机构标识                

    ret = apdu_cmd_update_capp_data_cache2( 0x00, 0xF0, 0x30,apdu_cmd_data, Recv,SW);
    if( SW[0] != 0x90 || SW[1] != 0x00)
    {
        LOGI("[%s %d]fail result %02X SW1:%02X SW2:%02X\n",__FUNCTION__,__LINE__,ret,SW[0],SW[1]);
        return HANDLE_ERR_CMD;
    }
    return HANDLE_OK;
}

int DebitChas_Complex_Consume_step5(JTB_CPU_CardInfo *pt_JTB_CardInfo,unsigned int Money,unsigned char Timebuf[7])
{
    unsigned char Recv[256]; 
    unsigned char SW[2];
    unsigned char apdu_cmd_data[0x0F];
    int ret;
    memset(apdu_cmd_data,0,sizeof(apdu_cmd_data));
    memcpy(&apdu_cmd_data[0],pt_JTB_CardInfo->PSAMOfflineSN,4);   
    memcpy(&apdu_cmd_data[4],Timebuf,7);                       
    memcpy(&apdu_cmd_data[11],pt_JTB_CardInfo->MAC1,4);       
    ret = apdu_cmd_debit_for_purchase(apdu_cmd_data,Recv,SW);
    if(SW[0] != 0x90 || SW[1] != 0x00)
    {
        if(ret == 2)
        {
            LOGI("[%s %d]fail result %02X SW1:%02X SW2:%02X\n",__FUNCTION__,__LINE__,ret,SW[0],SW[1]);
            return HANDLE_ERR_CMD;
        }
        return HANDLE_ERR;
    }
    memcpy(pt_JTB_CardInfo->TAC, Recv,4);
    memcpy(pt_JTB_CardInfo->MAC2, &Recv[4],4);
    return HANDLE_OK;
}
int DebitChas_Complex_Consume_step6(JTB_CPU_CardInfo *pt_JTB_CardInfo)
{
    unsigned char Recv[256]; 
    unsigned char Send[9];
    int len;
    int ret;
    int i;
    memset(Send,0,sizeof(Send));
    memcpy(Send,"\x80\x72\x00\x00\x04",5);          //命令头
    memcpy(&Send[5],pt_JTB_CardInfo->MAC2,4);       //MAC2
    len = 9;
    
    ret = Psam_Cmd_Send(0, Send, len, Recv, &len);
    if((ret != MI_OK)||(Recv[len-2] != 0x90)||(Recv[len-1] != 0x00))
    {                         
        return HANDLE_ERR_CMD;
    }

    return HANDLE_OK;
}

/* debitchas Card Consume */
unsigned char MonthlyCardConsume(JTB_CPU_CardInfo *pt_JTB_CardInfo,unsigned int Money)
{
    
    int result;
    unsigned char Timebuf[8];
    unsigned char buff1[20];
    unsigned char Send[128],Recv[128];
    unsigned char flag,t,status,len,i;


    Timebuf[0] = 0x20;
    Rd_time (Timebuf + 1);

    flag = 1;
    t = 1;
    LOGI("in card_debitchas_jiaotong\n");
    
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
    
    while(flag)
    {
        switch(t)
        {
            case 1://消费初始化
                {
                    
                    memset(Send,0,sizeof(Send));
                    memcpy(Send,"\x80\x50\x01\x02\x0B",5);        //命令头
                    Send[5] = PsamKeyIndex;                        //密钥索引
                    Send[6] = (Money&0xFF000000)>>24;
                    Send[7] = (Money&0xFF0000)>>16;
                    Send[8] = (Money&0xFF00)>>8;
                    Send[9] = Money&0xFF;                        //交易金额
                    memcpy(&Send[10],PsamNum,6);                //终端机号
                    Send[16] = 0x0f;                            //Le
                    len = 17;
                    
                    result = mifare_read_and_write(Send, len, Recv);
                    if(result <= 2)
                    {
                        LOGI("[%s %d] mifare_read_and_write fail result %02X SW1:%02X SW2:%02X\n",__FUNCTION__,__LINE__,result,Recv[0],Recv[1]);
                        flag = 0 ;
                        break;
                    }

                    t++;
                    memcpy(pt_JTB_CardInfo->offlineSN,&Recv[4],2);
                    memcpy(pt_JTB_CardInfo->overdraftAmount,&Recv[6],3);
                    pt_JTB_CardInfo->keyVersion = Recv[9];
                    pt_JTB_CardInfo->arithmeticLabel = Recv[10];
                    memcpy(pt_JTB_CardInfo->PRandom,&Recv[11],4);
                }
                break;

            case 2://PSAM卡生成MAC1
                {
                    memset(Send,0,sizeof(Send));
                    memcpy(Send,"\x80\x70\x00\x00\x24",5);                  //命令头
                    memcpy(&Send[5],pt_JTB_CardInfo->PRandom,4);                //用户卡片随机数
                    memcpy(&Send[9],pt_JTB_CardInfo->offlineSN,2);              //用户卡脱机交易序号
                    Send[11] = (Money&0xFF000000)>>24;
                    Send[12] = (Money&0xFF0000)>>16;
                    Send[13] = (Money&0xFF00)>>8;
                    Send[14] = Money&0xFF;                                  //交易金额
                    Send[15] = 0x06;                                        //交易类型
                    memcpy(&Send[16],Timebuf,7);                            //交易日期时间
                    Send[23] = pt_JTB_CardInfo->keyVersion;                     //密钥版本号
                    Send[24] = pt_JTB_CardInfo->arithmeticLabel;                //密钥算法标识
                    memcpy(&Send[25],&pt_JTB_CardInfo->appserialnumber[2],8);   //用户卡号
                    memcpy(&Send[33],pt_JTB_CardInfo->issuerlabel,8);           //发卡机构编码
                    len = 41;
                    memset(Recv,0,sizeof(Recv));
                    LOGI("计算MAC发送:");
                    menu_print(Send, len);
                    status = Psam_Cmd_Send(0, Send, len, Recv, &len);
                    if((status == MI_OK)&&(Recv[len-2] == 0x90)&&(Recv[len-1] == 0x00))
                    {
                        t++;
                        //LOGI("PSAM卡计算MAC1返回:");
                        menu_print(Recv, len);
                        memcpy(pt_JTB_CardInfo->PSAMOfflineSN,Recv,4);
                        memcpy(pt_JTB_CardInfo->MAC1,&Recv[4],4);
                    }
                    else 
                    {
                        flag = 0;
                    }
                }
                break;
            
            case 3://执行扣款
                {
                    memset(Send,0,sizeof(Send));
                    memcpy(Send,"\x80\x54\x01\x00\x0F",5);        //命令头
                    memcpy(&Send[5],pt_JTB_CardInfo->PSAMOfflineSN,4);    //终端交易序号
                    memcpy(&Send[9],Timebuf,7);                    //终端交易日期时间
                    memcpy(&Send[16],pt_JTB_CardInfo->MAC1,4);        //MAC1
                    Send[20] = 0x08;                            //Le
                    len = 21;
                    result = mifare_write(Send,len);
                    LOGI("消费扣款发送:");
                    menu_print(Send, len);
                    if(result == MI_OK)
                    {
                        mifare_ioctl(FIFO_RCV_LEN, receive_len);
                        result = mifare_read(Recv, receive_len[0]);
                        if(receive_len[0] > 2)
                        {
                            t++;
                            //LOGI("执行扣款返回:");
                            menu_print(Recv, receive_len[0]);
                            memcpy(pt_JTB_CardInfo->TAC,Recv,4);
                            memcpy(pt_JTB_CardInfo->MAC2,&Recv[4],4);
                        }
                        else if(receive_len[0] == 2)
                        {
                            LOGI("执行扣款SW1=%02X   SW2=%02X\n",Recv[0],Recv[1]);
                            flag = 0;
                        }
                        else
                        {
                            //防插拔处理
                            //status = SecondDebitChas_jiaotong(Money,Key,Keylen);
                            //if(status == 0)
                            //    t ++;
                            //else if(status == 0xFF)
                            //    t = 1;
                            //else
                            t = 0xAA;
                        }
                    }
                    else flag = 0;
                }
                break;

            case 4://PSAM卡验证MAC2
                {
                    memset(Send,0,sizeof(Send));
                    memcpy(Send,"\x80\x72\x00\x00\x04",5);        //命令头
                    memcpy(&Send[5],pt_JTB_CardInfo->MAC2,4);        //MAC2
                    len = 9;
                    for(i=0;i<2;i++)
                    {
                        status = Psam_Cmd_Send(0, Send, len, Recv, &len);
                        if((status == MI_OK)&&(Recv[len-2] == 0x90)&&(Recv[len-1] == 0x00))
                        {
                        //    LOGI("PSAM卡验证MAC2  SW1=%02X  SW2=%02X\n",Recv[len-2],Recv[len-1]);
                            break;
                        }
                    }
                    t++;
                }
                break;
            case 5:
                t = 0;
                flag =0 ;
                break;

            default :
            flag =0 ;
            break;
        }
    }
    return t;
}

/* DebitChas_complex Card Consume */
unsigned char NormalCardConsume(JTB_CPU_CardInfo *pt_JTB_CardInfo,unsigned int Money)
{
    unsigned char PsamNum[6];
    unsigned char PsamKeyIndex;
    
    int result;
    unsigned char Timebuf[7];
    unsigned char Send[256];
    unsigned char flag,t,len,i;
    unsigned int beforeMoney;

    Timebuf[0] = 0x20;
    Rd_time (Timebuf + 1);
    flag = 1;
    t = 1;
    
    result = InitPsam(0,9600);
    if(result != MI_OK)
    {
        LOGI("[%s %d] get psam id error result: %d\n",__FUNCTION__,__LINE__,result);
        return MI_FAIL;
    }
    result = GetPsamID_ex(0,PsamNum,&PsamKeyIndex);
    if(result != MI_OK)
    {
        LOGI("[%s %d] get psam id error result: %d\n",__FUNCTION__,__LINE__,result);
        return MI_FAIL;
    }

    while(flag)
    {

        switch(t)
        {
            case 1://复合应用消费初始化
                {
                    result = DebitChas_Complex_Consume_step1(pt_JTB_CardInfo,Money,PsamNum,PsamKeyIndex);
                    if(result != HANDLE_OK)
                    {
                        flag = 0;
                        break;
                    }
                    t++;
                }
                break;
            
            case 2://PSAM卡生成MAC1
                {
                    result = DebitChas_Complex_Consume_step2(pt_JTB_CardInfo,Money,Timebuf);
                    if(result != HANDLE_OK)
                    {
                        flag = 0;
                        break;
                    }
                    t++;
                }
                break;          
            
            case 3:         //用户卡更新复合应用记录文件
                {
                    if(0)
                    {
                        result = DebitChas_Complex_Consume_step3(pt_JTB_CardInfo);
                        if(result != HANDLE_OK)
                        {
                            flag = 0;
                            break;
                        }
                    }
                    t++;
                }
                break;

            case 4:     //更新交易信息记录
                {
                    result = DebitChas_Complex_Consume_step4(pt_JTB_CardInfo,Money,Timebuf,PsamNum);
                    if(result != HANDLE_OK)
                    {
                        flag = 0;
                        break;
                    }
                    t++;
                }
                break;

            case 5://执行扣款
                {
                    result = DebitChas_Complex_Consume_step5(pt_JTB_CardInfo,Money,Timebuf);
                    if(result != HANDLE_OK)
                    {
                        result = SecondDebitChas(pt_JTB_CardInfo,Money);
                        if(result != HANDLE_OK)
                        {
                            flag = 0;
                            break;
                        }
                    }
                    t++;
                }
                break;
            
            case 6://PSAM卡验证MAC2
                {
                    int i;
                    for(i = 0 ; i < 2 ; i++)
                    {
                        result = DebitChas_Complex_Consume_step6(pt_JTB_CardInfo);
                        if(result != HANDLE_OK)
                        {
                            break;
                        }
                    }
                    if(i == 2)
                    {
                        flag = 0;
                        break;
                    }
                    t++;
                }
                break;
                   
        default:
            flag = 0;
            t = 0;
            break;
        }
    }
    return t;
}
//防插拔处理
int SecondDebitChas(JTB_CPU_CardInfo *pt_JTB_CardInfo,unsigned int Money)
{
    int result;
    unsigned char Recv[256];
    unsigned char Timebuf[8];
    unsigned char flag,t,status,len,Qflag;
    unsigned char PsamNum[6];
    unsigned char PsamKeyIndex;
    unsigned short ic;
    CardInformCPU tempbuf;

    
    LOGI("初始化后的ATC3:%02x %02x\n",pt_JTB_CardInfo->offlineSN[0],pt_JTB_CardInfo->offlineSN[1]);
    memcpy(&tempbuf.IsLocalCard,&pt_JTB_CardInfo->IsLocalCard,sizeof(CardInformCPU));             //备份交通部卡信息
    flag = 1;
    t = 1;
    Qflag = 1;
    ic = 0;   
    while(Qflag)
    {
        Timebuf[0] = 0x20;
        Rd_time (Timebuf + 1);

        ic++;
        if(ic > 10)
        {
            Qflag =0;
            break;
        }

        flag = 1;
        t = 1;
        
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
        while(flag)
        {
            LOGI("[%s %d] break point t : %d\n",__FUNCTION__,__LINE__, t);
            switch(t)
            {
            case 1://卡复位并判定是不是同一张卡
                {
                    result =  CardReset(Recv,&len,1);
                    if(status != MI_OK)
                    {
                        flag = 0;
                        break;
                    }
                    t++; 
                }
                break;

            case 2://选应用目录
                {
                    unsigned char SW[2];
                    unsigned char AID[] =  {0xA0,0x00,0x00,0x06,0x32,0x01,0x01,0x05};
                    result = apdu_cmd_select_APP(AID, sizeof(AID),Recv,SW);
                    if(SW[0] != 0x90 || SW[1] != 0x00)
                    {
                         flag = 0;
                         break;
                    }
                    t++;
                }
                break;
            case 3: //取交易证明
                {
                    unsigned char SW[2];
                    unsigned char data[2];
                    LOGI("初始化后的ATC:%02X %02X\n",pt_JTB_CardInfo->offlineSN[0],pt_JTB_CardInfo->offlineSN[1]);
                    memcpy(&pt_JTB_CardInfo->IsLocalCard,&tempbuf.IsLocalCard,sizeof(CardInformCPU)); 
                    data[0] = ((pt_JTB_CardInfo->offlineSN[0]<<8|pt_JTB_CardInfo->offlineSN[1] + 1)&0xFF00)>>8;
                    data[1] = ((pt_JTB_CardInfo->offlineSN[0]<<8|pt_JTB_CardInfo->offlineSN[1] + 1)&0xFF)>>0;
                    result = apdu_cmd_get_transaction_prove(0x09,data,Recv,SW);
                    if(result > 0 && SW[0] == 0x90 && SW[1] == 0x00)
                    {
                        memcpy(pt_JTB_CardInfo->MAC2,Recv,4);
                        memcpy(pt_JTB_CardInfo->TAC,&Recv[4],4);
                        return HANDLE_OK;
                    }
                    else if(result < 0)
                    {
                        flag = 0;
                        break;
                    }
                    LOGI("取TAC返回:%02x %02x\n",SW[0],SW[1]);                        //交易未成功，需要重新消费
                    t++;
                }
                break;

            case 4://复合应用消费初始化
                {
                    result = DebitChas_Complex_Consume_step1(pt_JTB_CardInfo,Money,PsamNum,PsamKeyIndex);
                    if(result != HANDLE_OK)
                    {
                        flag = 0;
                        break;
                    }
                    t++;
                }
                break;
            
            case 5://PSAM卡生成MAC1
                {                
                    LOGI("重刷初始化后的ATC1:%02x %02x\n",pt_JTB_CardInfo->offlineSN[0],pt_JTB_CardInfo->offlineSN[1]);
                    result = DebitChas_Complex_Consume_step2(pt_JTB_CardInfo,Money,Timebuf);
                    if(result != HANDLE_OK)
                    {
                        flag = 0;
                        break;
                    }
                    t++;
                }
                break;          
            
            case 6://用户卡更新复合应用记录文件
                {
                    if(0) //Section.Enable != 0x55)
                    {
                        result = DebitChas_Complex_Consume_step3(pt_JTB_CardInfo);
                        if(result != HANDLE_OK)
                        {
                            flag = 0;
                            break;
                        }
                    }
                    t++;
                }
                break;

            case 7:
                {
                    result = DebitChas_Complex_Consume_step4(pt_JTB_CardInfo,Money,Timebuf,PsamNum);
                    if(result != HANDLE_OK)
                    {
                        flag = 0;
                        break;
                    }
                    t++;
                }
                break;

            case 8://执行扣款
                {
                    result = DebitChas_Complex_Consume_step5(pt_JTB_CardInfo,Money,Timebuf);
                    if(result != HANDLE_OK)
                    {
                        flag = 0;
                        break;
                    }
                    t++;
                }
                break;
            case 9:
                flag = 0;
                t=0;
                Qflag = 0;
                break;
            }
        }
    }
    return t;
}



