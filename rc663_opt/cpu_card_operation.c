#include "cpu_card_operation.h"
#include "libcardlan_CardInfo.h"
#include "ISO14443_operation.h"
#include "typea.h"
#include "mcu_opt/psam_opt.h"
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


CardInform CardLan;
CardInformCPU CardLanCPU;
BigUnion NewDriver;
JackRegal Sector;
MonthlyTicket monthylyticket;
SectionFarPar Section;
SectionFarPar Sectionup;
Transfer CardTransfer;
BigUnion BigData;
LongUnon HostValue;
LongUnon HostValueS;
LongUnon HostValueH;
LongUnon HostValueY;
LongUnon DecValue;
DriverRecordFormat SaveData1;
LongUnon DevSID;                        //终端机流水号
ShortUnon Driver;

unsigned char SRCPUCardBuf[1024];
unsigned char SRCPUCardBuf[1024];
static unsigned char receive_buf[128]= {0};
static int  receive_len[1] = {0};
unsigned char PsamNum_bak1[6];            //作为卡联cpu卡流程psamNum的备份，在执行卡联cpu卡流程时使用
unsigned char PsamNum_bak2[6];            //备份交通部的psamnum
struct Permisson CardPermisson;
unsigned long long big_data;
unsigned char g_Fgkongtiaoflag;
unsigned char NandBuf[512];
int  Psamreceive_len[1] = {0};
char Psamreceive_buf[128]= {0};
st_WhiteFile WhiteListFile;
SysTime Time;
time_t utc_time;



static void menu_print(char *buffer, int length)
{
	int i;

	for (i = 0; i < length; i++)
	{
		printf("%02X ", *(buffer + i));
		if ((i + 1) % 8 == 0) printf(" ");
		if ((i + 1) % 16 == 0) printf("\n");
	}
	printf("\n");
}



unsigned char PriceRate(unsigned char FlagV)
{
    unsigned char  status = 2;
    unsigned int i;
    unsigned char pricerate[8],permisson[7];
    int price;
    LOGI("ParseRMB() is called.\n");

    //memcpy(NandBuf,CardLanBuf+8*512,512);    

    memcpy(pricerate,NandBuf,9);
       
    LOGI("参数表", NandBuf, 229);
    if(CardLanCPU.IsLocalCard==2)
    {

        price=pricerate[2]<<8|pricerate[3];
        HostValueS.i = price;
        LOGI("实际价格:%d\n",HostValueS.i);
        if(g_Fgkongtiaoflag)                    
        {
            HostValueS.i = HostValueS.i*pricerate[8]/100;
            LOGI("空调车价格:%d\n",HostValueS.i);
        }
        if(JTB_CardInfo.IsLocalCard==2)
        {
          HostValueS.i = HostValueS.i*pricerate[4]/100;
          LOGI("互通卡价格:%d:折率=%d\n",HostValueS.i,pricerate[4]); 
        }
        status = 0;

    }
    else
    {
    
    for(i = 0 ; i < 22; i++)
    {
    
    //    if(((NandBuf[i*22+9] == 0x55)&&(NandBuf[i*22+9+1] == 0xAA))||
    //        ((NandBuf[i*22+9] == 0x00)&&(NandBuf[i*22+9+1] == 0x00)))
        if((NandBuf[i*22+9] == 0x55)&&(NandBuf[i*22+9+1] == 0xAA))
        {
            DecValue.i = 0;
            HostValue.i = 0;
            status = 2;
            break;
        }

        LOGI("1NandBuf[i*22+9]:%02x:Type=%02x\n",NandBuf[i*22+9],FlagV);    
        if(NandBuf[i*22+9] == FlagV)
        {
                
            LOGI("卡类参数:",&NandBuf[i*22+9],22);
            memcpy(&CardPermisson.walletflag,&NandBuf[i*22+9+1],7);
            LOGI("卡类权限:\n");
            LOGI("钱包:%02x\n",CardPermisson.walletflag);
            LOGI("月票:%02x\n",CardPermisson.yueticketflag);
            LOGI("换乘优惠:%02x\n",CardPermisson.transferflag);
            LOGI("让座语音:%02x\n",CardPermisson.sheetvoiceflag);
            LOGI("时段优惠:%02x\n",CardPermisson.discounttimeflag);
            LOGI("钱包带人:%02x\n",CardPermisson.walletaddflag);
            LOGI("月票带人:%02x\n",CardPermisson.yueticketaddflag);

            price=pricerate[0]<<8|pricerate[1];
            HostValueY.i = price;
            LOGI("月票价格:%d\n",HostValueY.i);

            price=pricerate[2]<<8|pricerate[3];
            HostValueS.i = price;
            LOGI("实际价格:%d\n",HostValueS.i);
            if(g_Fgkongtiaoflag)                    
            {
                HostValueS.i = HostValueS.i*pricerate[8]/100;
                LOGI("空调车价格:%d\n",HostValueS.i);
            }
        //    if(JTB_CardInfo.IsLocalCard==2)
        //        {
        //          HostValueS.i = HostValueS.i*pricerate[4]/100;
        //          LOGI("互通卡价格:%d:折率=%d\n",HostValueS.i,pricerate[4]);    
        //        }
        //    else
                {                          
            HostValueS.i = HostValueS.i*NandBuf[i*22+9+10]/100;
            LOGI("本地卡价格:%d:折率=%d\n",HostValueS.i,NandBuf[i*22+9+10]);
            if(CardPermisson.transferflag)
            {
                HostValueH.i = HostValueS.i*pricerate[4]/100;
                LOGI("本地卡换乘价格:%d:折率=%d\n",HostValueH.i,pricerate[4]);
            }
                  
            }
                        
                
                //有月票权限，本地卡、且在一票制模式下才能用月票
                if((CardPermisson.yueticketflag==0x01)&&(JTB_CardInfo.IsLocalCard==1)&&(Section.Enable!=0x55))
                    {
                        status = 1;
                        
                    }
                else 
                    {
                        status = 0;
                        
                    }
           
            break;
        }
    }
    }
    LOGI("PriceRate = %d\n",status);
    return status;
}



/*
*************************************************************************************************************
- 函数名称 : char * Rd_time (char* buff)
- 函数说明 : 读时间
- 输入参数 : 无
- 输出参数 : 无
*************************************************************************************************************
*/
char * Rd_time (char* buff)
{
    time_t t;
    struct tm * tm;
    time (&t);
    utc_time = t;
    tm = localtime (&t);
    buff[0] = HEX2BCD((unsigned char)tm->tm_year-100);
    buff[1] = HEX2BCD(tm->tm_mon+1);
    buff[2] = HEX2BCD(tm->tm_mday);
    buff[3] = HEX2BCD(tm->tm_hour);
    buff[4] = HEX2BCD(tm->tm_min);
    buff[5] = HEX2BCD(tm->tm_sec);
    return buff;
}

unsigned char Card_JudgeDate_jiaotong(void)
{
    unsigned char status = 1;
    unsigned char t;
    unsigned char buff[7];
    unsigned int start_time,end_time,this_time;


    for(t = 0; t<3; t++)
    {
        memset(buff,0,sizeof(buff));
        Rd_time (buff);
        Time.year = buff[0];
        Time.month = buff[1];
        Time.day = buff[2];
        Time.hour = buff[3];
        Time.min = buff[4];
        Time.sec = buff[5];

        this_time = 0x20<<24|buff[0]<<16|buff[1]<<8|buff[2];
        start_time = JTB_CardInfo.appstarttime[0]<<24|JTB_CardInfo.appstarttime[1]<<16|JTB_CardInfo.appstarttime[2]<<8|JTB_CardInfo.appstarttime[3];
        end_time = JTB_CardInfo.appendtime[0]<<24|JTB_CardInfo.appendtime[1]<<16|JTB_CardInfo.appendtime[2]<<8|JTB_CardInfo.appendtime[3];
        LOGI("this_time:%x \n", this_time);
        LOGI("start_time:%x \n", start_time);
        LOGI("end_time:%x \n", end_time);
        if(this_time>=start_time && this_time<=end_time)
        {
            status = 0;
            break;
        }
        if(start_time> this_time)
        {
            status = 1;
            break;
        }
        else if(end_time<this_time)
        {
            status = 2;
            break;
        }
        
    }
    return status;
}

unsigned char Check_CardDate()
{
    unsigned status = 0;
    unsigned int start_time,end_time;
    
    start_time = JTB_CardInfo.appstarttime[0]<<24|JTB_CardInfo.appstarttime[1]<<16|JTB_CardInfo.appstarttime[2]<<8|JTB_CardInfo.appstarttime[3];
    end_time = JTB_CardInfo.appendtime[0]<<24|JTB_CardInfo.appendtime[1]<<16|JTB_CardInfo.appendtime[2]<<8|JTB_CardInfo.appendtime[3];
    if((start_time==0xffffffff)||(end_time==0xffffffff)||(start_time==0x0)||(end_time==0x0))
    {
        status = 1;
    }

    return status;
}

unsigned char Card_White_Cpu_jiaotong(void)
{
    unsigned char  status = 1;
    unsigned char  CJCbuf[12];
    WhiteItem src;
    int find = 0;

    memset(CJCbuf,0,sizeof(CJCbuf));
    memcpy(CJCbuf,JTB_CardInfo.issuerlabel,4);
    memcpy(src.dat, CJCbuf, sizeof(WhiteItem));
    
    half_search_white(src, &find);
    if(find)
        status = 0;

//    LOGI("Card_JudgeCsn() find=%d\n", find);
//    LOGI("Card_JudgeCsn() status=%d\n", status);    

    return status;
}

int half_search_white(WhiteItem dest, int *find)
{
    int low,high,mid,val;
    low = 0;
    *find = 0;
    low = 0;
    mid = 0;

    if((WhiteListFile.count == 0)||(WhiteListFile.buf==NULL))
        return 0;

    high = WhiteListFile.count - 1;

    LOGI("high= %d\n", high);
    LOGI("dest: %02X%02X%02X%02X\n", dest.dat[0],dest.dat[1],dest.dat[2],dest.dat[3]);
    
    //LOGI("正在折半查找黑名单\n");
    while (low <= high)
    {
        mid = (low + high)/ 2;

        LOGI("WhiteListFile.buf[mid].dat: %02X%02X%02X%02X\n", WhiteListFile.buf[mid].dat[0],WhiteListFile.buf[mid].dat[1],\
        WhiteListFile.buf[mid].dat[2],WhiteListFile.buf[mid].dat[3]);

        
        //LOGI("dest %p buf %p \n",dest.dat,BlackListFile.buf[mid].dat);
        val = memcmp(dest.dat, WhiteListFile.buf[mid].dat, 4 );//izeof(BlackListFile.buf[0]));
        //  val= 1;    
        //LOGI("使用memcmp没死掉\n");
        if (val == 0)
        {
            unsigned char src[4];
            memcpy(src, WhiteListFile.buf[mid].dat, sizeof(WhiteListFile.buf[0]));
            LOGI("src: %02X%02X%02X%02X%\n", src[0],src[1],\
                src[2],src[3]);

            *find = 1;
            low = mid;
            break;
        }
        else if (val > 0)
            low = mid + 1;    //dest > src
        else
            high = mid - 1;
    }

    LOGI("low= %d find=%d\n", low, *find);

//    LOGI("half_serach结束\n");
    return low;
}
unsigned char PsamCos(char *Intdata, char *Outdata,unsigned char *len)
{
    int ret;
    unsigned char lens;

    lens = *len;

#if PSAMDIS
    {
        unsigned char i;
        LOGI("\n PsamCos  in:%02d\n",lens);
        for(i = 0; i<lens; i++)
        {
            LOGI("%02X",Intdata[i]);
        }
        LOGI("\n");
    }
#endif
    memset(Psamreceive_buf,0,sizeof(Psamreceive_buf));
    ret = mifare_write(Intdata,lens);//TCOS命令
    if(ret >= MI_OK)
    {
        mifare_ioctl(RECEIVE_LEN, Psamreceive_len);
        mifare_read(Psamreceive_buf, Psamreceive_len[0]);

       // if(Psamreceive_len[0] == 2) {
            memcpy(Outdata,Psamreceive_buf,Psamreceive_len[0]);
            *len = Psamreceive_len[0];
      //  }
       /* else {
            memcpy(Outdata,Psamreceive_buf+1,(Psamreceive_len[0]-1));
            *len = (Psamreceive_len[0]-1);
        }*/
        ret = MI_OK;
    }


#if PSAMDIS
    {
        unsigned char i;
        LOGI(" PsamCos  out:%02d:ret=%d:Psamreceive_len[0]=%d\n",*len,ret,Psamreceive_len[0]);

        for(i = 0; i < Psamreceive_len[0]; i++)
        {
            LOGI("%02X",Psamreceive_buf[i]);
        }
        LOGI("\n");
    }
#endif

    return ret;
}

unsigned char GET_MAC()
{
    int result;
    unsigned char Timebuf[8];
    unsigned char Send[256],Recv[256];
    unsigned char flag,t,status,len,i;    
    
    flag = 1;
    t = 0;
    while(flag)
    {
        switch(t)
        {

            case 0:
                {
                    result = mifare_read_and_write("\x80\x5c\x03\x02\x04",sizeof("\x80\x5c\x03\x02\x04"),receive_buf);
                    if(result <= 2)
                    {
                        LOGI("mifare_read_and_write fail result:%02X SW1:%02X SW2:%02X\n",result,receive_buf[0],receive_buf[1]);
                        return MI_FAIL;
                    }
                    t++;
                    memcpy(JTB_CardInfo.beforemoney,receive_buf,4);
                    LOGI("\n交易前余额%d分\n",receive_buf[0]<<24|receive_buf[1]<<16|receive_buf[2]<<8|receive_buf[3]);
                }
                break;

        
            case 1://进入PSAM卡片目录
                {
                    result = mifare_read_and_write("\x00\x84\x00\x00\x08",5,receive_buf);
                    if(result <= 2)
                    {
                        LOGI("mifare_read_and_write fail result:%02X SW1:%02X SW2:%02X\n",result,receive_buf[0],receive_buf[1]);
                        return MI_FAIL;
                    }

                    t=3;
                    LOGI("取卡随机数返回");
                    menu_print(receive_buf, receive_len[0]);
                    memcpy(JTB_CardInfo.PSAMRandom,&receive_buf[0],8);               
                }
                break;
            
            case 2://PSAM卡生随机数              
                {
                    memset(Send,0,sizeof(Send));
                    memcpy(Send,"\x00\x84\x00\x00\x08",5);            //脙眉脕卯脥路            
                    len = 5;
                    memset(Recv,0,sizeof(Recv));

                    LOGI("生成随机数发送");
                    menu_print(Send, len);
                    status = PsamCos(Send,Recv,&len);
                    
                    LOGI("生成随机数返回");
                    menu_print(Recv, len);
            
                    if((status == MI_OK)&&(Recv[len-2] == 0x90)&&(Recv[len-1] == 0x00))
                    {
                        t++;                
                        memcpy(JTB_CardInfo.PSAMRandom,Recv,8);
                        
                    }
                    else
                    {
                        flag = 0;
                    }
                }
                break;          
            
            case 3://准备发送
                {
                    memset(Send,0,sizeof(Send));
                    memcpy(Send,"\x80\x1A\x45\x02\x10",5);    //脙眉脕卯脥路    
                    memcpy(Send+5,JTB_CardInfo.appserialnumber+2,8);
                    memcpy(Send+13,JTB_CardInfo.issuerlabel,8);
                    len = 21;
                    
                    LOGI("准备发送");
                    menu_print(Send, len);
                    status = PsamCos(Send,Recv,&len);
                    
                    LOGI("准备发送返回");
                    menu_print(Recv, len);
            
                    if((status == MI_OK)&&(Recv[len-2] == 0x90)&&(Recv[len-1] == 0x00))
                    {
                        t++;                
                    }
                    else
                    {
                        flag = 0;
                    }
                }
                break;

            case 4://初始化计算MAC
                {
                    memset(Send,0,sizeof(Send));
                    memcpy(Send,"\x80\xFA\x05\x00\x10",5);
                    memcpy(Send+5,JTB_CardInfo.PSAMRandom,8);
                    memcpy(Send+13,"\x84\x1E\x00\x00\x04\x80\x00\x00",8);
                    len = 21;
                    LOGI("初始化计算MAC发送");
                    menu_print(Send, len);
                    status = PsamCos(Send,Recv,&len);
                    
                    LOGI("初始化计算MAC返回");
                    menu_print(Recv, len);    
                    if((status == MI_OK)&&(Recv[len-2] == 0x90)&&(Recv[len-1] == 0x00))
                    {
                        t++;
                        memcpy(JTB_CardInfo.DESCRY,Recv,4);
                    }
                    else
                    {
                        flag = 0;
                    }
                }
                break;

            case 5://应用锁定                      
                {
                    break;
                    memset(Send,0,sizeof(Send));
                    memcpy(Send,"\x84\x1E\x00\x00\x04",5);        //脙眉脕卯脥路
                    memcpy(&Send[5],JTB_CardInfo.DESCRY,4);    //脰脮露脣陆禄脪脳脨貌潞脜                                        
                    len = 9;
                    LOGI("应用锁定发送");
                    menu_print(Send, len);
                    result = mifare_write(Send,len);
                    if(result == MI_OK)
                    {
                        mifare_ioctl( FIFO_RCV_LEN, receive_len);
                        result = mifare_read( receive_buf, receive_len[0]);
                        if(receive_len[0] > 2)
                        {
                            t++;
                            LOGI("应用锁定发送");
                            menu_print(receive_buf, receive_len[0]);                    
                        }
                        else if(receive_len[0] == 2)
                        {
                            LOGI("应用锁定返回:SW1=%02X   SW2=%02X\n",receive_buf[0],receive_buf[1]);
                            flag = 0;
                        }
                    }
                    else 
                    {
                        flag = 0;
                    }
                }
                break;                
        default:
            flag=0;
            t=0;
            break;
        }
    }
    return t;
}


int MatchTermAID(unsigned char * aid,unsigned char aidLen,TERMAPP_T *pt_app_list,unsigned char app_list_num)
{
    int i = -1;

    if(aid == NULL || pt_app_list == NULL || aidLen <= 0 || app_list_num <= 0)
    {
        return -1;
    }

    for(i = 0; i < app_list_num; i++)
    {
        if(pt_app_list[i].ASI == 0) //PARTIAL_MATCH
        {
            if(!memcmp(aid,pt_app_list[i].AID,pt_app_list[i].AIDLen))
            {
                break;
            }
        }
        else//exact match
        {
            if(!memcmp(aid,pt_app_list[i].AID,aidLen))
            {
                break;
            }
        }
    }


    if(i == app_list_num)
    {
        return -1;
    }
    
    LOGI("[ %s %d ]: AID  matched !.\n",__FUNCTION__,__LINE__);
    menu_print(aid,aidLen);
    return i;    
}

unsigned char SelectAppDF(char *DFname,char *Recvdata,unsigned  char *namelen)
{
    int result;
    unsigned char receive_buf[128]= {0};
    unsigned char Send[256] = {0};
    unsigned char len;
    
    unsigned char selfileDF[]= {0x00,0xa4,0x04,0x00}; //,0x09,0xA0,0x00,0x00,0x00,0x03,0x86,0x98,0x07,0x01,0x00};
    unsigned char selfileDFFci[]= {0x00,0xa4,0x00,0x00}; //,0x09,0xA0,0x00,0x00,0x00,0x03,0x86,0x98,0x07,0x01,0x00};

    if(*namelen == 0x02)
    {
        memcpy(Send,selfileDFFci,sizeof(selfileDFFci));
    }
    else
    {
        memcpy(Send,selfileDF,sizeof(selfileDF));
    }

    Send[4] = *namelen;
    len = *namelen;
    memcpy(Send + 5,DFname,len);
    len = len + 6;
    memset(receive_buf,0,sizeof(receive_buf));

    result = mifare_read_and_write(Send, len,receive_buf);

    *namelen  = result;
    memcpy(Recvdata,receive_buf,result);

    if(result <= 2)
    {
        LOGI("mifare_read_and_write fail result:%02X SW1:%02X SW2:%02X\n",result,receive_buf[0],receive_buf[1]);
        return MI_FAIL;
    }

    return MI_OK;
}



unsigned char mystrncmp(const unsigned char *istra,const unsigned char *istrb,unsigned char len)
{
    unsigned char i;

    for(i=0; i<len; i++)
    {
        if(istra[i] != istrb[i])
        {
            break;
        }
    }

    if(i==len) i=0;
    else i=1;

    return i;
}

unsigned char ResetM1FormCPU(void)
{
    mifare_ioctl(DO_TYPEA_M1, 0);
    unsigned char status = 0;
    int ret = 0;
    int i ;
    unsigned char reset_type = 0;

    for( i = 0 ; i < 5 ; i++)
    {
        ret = ISO14443_CardReset();
        if(ret == MI_OK)
        {
            break;
        }
        usleep(1000);
    }

    if(i == 5)
    {
        printf("[%s %d ] ret = %d\n",__FUNCTION__,__LINE__,ret);
        return -1;
    }

    mifare_ioctl(WRITE_TYPE, W_CPU);
    ret = mifare_ioctl(TYPEA_CPU_REST, 0);
    if(ret != 0)
    {
        printf("[%s %d ] ret = %d\n",__FUNCTION__,__LINE__,ret);
        return -3;
    }

    return 0;
}

int CardHalt(void)
{
    int ret = 0;
    if(1)
    {
        ret = mifare_ioctl(RC531_HALT, 0);
    }
    else if(0)
    {
       // ret = iso15693_halt();
    }
    else
    {
        LOGI("[%s %d] error status\n",__FUNCTION__,__LINE__);
        return MI_FAIL;
    }
        
    return ret;
}
int CardReset_0x20(char *data,unsigned char *plen)
{
    int rcv_len,read_len;
    unsigned char recv[128];

    int ret = 0;
    mifare_ioctl(WRITE_TYPE, W_CPU);
    ret = mifare_ioctl(TYPEA_CPU_REST, 0);
    if(ret != MI_OK)
    {
        return -1;
    }   

    mifare_ioctl(FIFO_RCV_LEN, &rcv_len);
    read_len = mifare_read(recv, rcv_len);
    memcpy(data,recv,read_len);  
    *plen = read_len;
    return 0;
}

int CardReset_bak(char *data,unsigned char *plen,unsigned char type)
{
    unsigned char status = 0;
    int ret = 0;
    int i ;
    unsigned char reset_type = 0;
    int rcv_len,read_len;
    unsigned char recv[256];
	
	
    for( i = 0 ; i < 5 ; i++)
    {
        ret = ISO14443_CardReset();
        if(ret == MI_OK)
        {
            break;
        }
        usleep(1000);
    }

    if(i == 5)
    {
        printf("[%s %d ] ret = %d\n",__FUNCTION__,__LINE__,ret);
        return -1;
    }

    mifare_ioctl(FIFO_RCV_LEN, &rcv_len);
    read_len = mifare_read(recv, rcv_len);
    reset_type = recv[read_len - 1];
    
    memcpy(data,recv,read_len);
    *plen = read_len;
	
    printf("[%s %d ] reclen %d\n",__FUNCTION__,__LINE__,read_len);
    menu_print(recv, read_len);
    if(reset_type == 0x20)
    {
        mifare_ioctl(WRITE_TYPE, W_CPU);
        ret = mifare_ioctl(TYPEA_CPU_REST, 0);
        if(ret != 0)
        {
            printf("[%s %d ] ret = %d\n",__FUNCTION__,__LINE__,ret);
            return -3;
        }
    }
    else if(reset_type == 0x08 || reset_type == 0x18)
    {
    
    }
    else if((reset_type & 0x08) == 0x08)
    {
		reset_type = 0x08;
	}
	else
    {
    	
        printf("[%s %d ] not support %d\n",__FUNCTION__,__LINE__,reset_type);
        return -3;
    }

    return reset_type;
}

int CardReset(char *data,unsigned char *plen,unsigned char type)
{
    unsigned char status = 0;
    int ret = 0;
    int i ;
    unsigned char reset_type = 0;
    int rcv_len,read_len;
    unsigned char recv[256];
	
	
    for( i = 0 ; i < 5 ; i++)
    {
        ret = ISO14443_CardReset();
        if(ret == MI_OK)
        {
            break;
        }
        usleep(1000);
    }

    if(i == 5)
    {
   //     printf("[%s %d ] ret = %d\n",__FUNCTION__,__LINE__,ret);
        return -1;
    }

    mifare_ioctl(FIFO_RCV_LEN, &rcv_len);
    read_len = mifare_read(recv, rcv_len);
    reset_type = recv[read_len - 1];
    
    memcpy(data,recv,read_len-1);
    *plen = read_len-1;
	
   // printf("[%s %d ] reclen %d\n",__FUNCTION__,__LINE__,read_len);
   // menu_print(recv, read_len);    

    return reset_type;
}




void InitWhiteListBuff()
{
    FILE *fp;
    int i,j;
    WhiteListFile.buf = NULL;
    WhiteListFile.count = 0;
    WhiteListFile.buf = (WhiteItem *)malloc(MAX_BLACK_CONNT * sizeof(WhiteItem));
    if(WhiteListFile.buf == NULL) return;
    memset(WhiteListFile.buf, 0x00, (MAX_BLACK_CONNT * sizeof(WhiteListFile.buf[0])));

    fp = fopen(CITYUNION_BL_FILEWHI, "rb");
    if(fp == NULL)
    {    
        LOGI("Can't open the Whitelist.sys\n");
        return;
    }

    fseek(fp,0,SEEK_END);
    WhiteListFile.count = ftell(fp) / sizeof(WhiteListFile.buf[0]);
    fseek(fp,0,SEEK_SET);
    fread(WhiteListFile.buf, sizeof(WhiteItem), WhiteListFile.count, fp);
    fclose(fp);
       

}

size_t mifare_read_and_write(const unsigned char *data, size_t data_len,unsigned char *result_buff)
{
    ssize_t result;

    if(1)
    {
        int i;
        for(i = 0 ; i < 1 ;i++)
        {
            result = ISO14443_read_and_write(data,data_len,result_buff);
            if(result <= 0)
            {
                continue;
            }
            break;
        }
    }
    else if(0)
    {
        result = ISO15693_read_and_write(data,data_len,result_buff);
    }
    else
    {
        /*unknow*/
        return -1;
    }
    return result;
}

static TERMAPP_T terminal_support_app_list[10];
static int terminal_support_app_num = 0;

int get_terminal_support_AID(TERMAPP_T **pt_app_list,int *pt_app_num)
{
   *pt_app_list = terminal_support_app_list;
   *pt_app_num = terminal_support_app_num;

    return 0;
}

int set_terminal_support_AID(TERMAPP_T *pt_app_list,int app_num)
{
    if(pt_app_list == NULL)
    {
        return -1;
    }
    
    memcpy(terminal_support_app_list, pt_app_list, app_num * sizeof(TERMAPP_T));
    terminal_support_app_num = app_num;
    return 0;
}

int add_terminal_support_AID(TERMAPP_T pt_app)
{
    return 0;
}



