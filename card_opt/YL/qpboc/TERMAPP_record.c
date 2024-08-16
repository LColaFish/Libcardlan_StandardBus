#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip.h>   
#include <netinet/ip_icmp.h>  
#include <sys/wait.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <netinet/ip.h>   
#include <netinet/ip_icmp.h>

#include "TERMAPP_record.h"
#include "encrypt_lib/stades.h"
//---------#include "../gui/RC500.h"
//---------#include "apparel.h"
//---------#include "../gui/InitSystem.h"
#include "includes.h"
//---------#include "../gui/GeneralConfig.h"
//---------#include "../QR/QRcode.h"
//---------#include "../gui/Freesign.h"

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

struct PbocTagContainer TagData;
struct SignContainerReply SignData;
struct ConsumeContainerReply ConsumeData;


unsigned char G_SignInfo_PBOC = 0;        /* 银行卡签到的命令标志 */
unsigned char G_Consume_PBOC = 0;        /* 上传消费信息 */
unsigned char G_BatchSett_PBOC = 0;        /* 银行卡结算的命令标志 */


//---------
//---------#if SUPPORT_QR_CODE
UART_QRCODE G_QRuartInfo;
QRMANAGER G_QRcodeManage;
//---------#endif

MerchantConfig mchantConf;
CardInform CardLan;
//Card_Driver card_driver;
LongUnon DevNum;
Base_Info basinfo;
LongUnon DecValue;
unsigned char DevSerialID[4];
LongUnon DevSID;
CardInform CardLan;
SysTime Time;
LongUnon HostValue;

unsigned char enableodaflag;
int issign;            //签到标记，1已签到 ， 0未签到
int rechargeflag;    //冲正标志    1要求冲正，0无冲正

//---------

unsigned char emvrecord[256];
unsigned char consumecz[1024];
int emvrecordlen;

struct UploadRecodContainer UploadData = {
    "\x00\x00\x00\x00\x00\x90",
    "\x11\x10\x94",
    "\x25\x03",
    "\x32\x30\x31\x35\x32\x30\x38\x31",
    "\x33\x30\x35\x35\x31\x30\x36\x34\x31\x31\x31\x30\x30\x30\x31",
    "\x31\x35\x36",    
    "\x00\x00\x24",    
    "\x43\x55\x50",    
    "\x00\x00\x00\x00\x00\x00\x00\x00",
};


struct SettleContainer SettleData = {
    "\x00\x00\x09",
    "\x32\x30\x31\x30\x30\x33\x34\x33",
    "\x39\x35\x32\x35\x31\x30\x36\x34\x31\x31\x31\x30\x30\x30\x31",
    "\x00",
    "\x31\x35\x36",
    "\x00",
    "\x00\x11",
    "\x20\x10",
    "\x30\x30\x31",
};

struct strPBOCKey PbocKey ={
    "\xD9\x38\xB3\x0D\x15\x3E\x8F\x5B\x49\x2C\x16\xA8\xA8\x4F\xE6\x29",
    "\x0A\x0B\x0C\x0D\x0E\x0F\x01\x02",
};



void DebugPrintChar(char *name, unsigned char *data, int len)
{
    int i = 0;
    LOGI("%s :\n", name);
    for(i=0; i<len; i++)
    {
        if((i%8 == 0) && (i))
            LOGI("  ");
        
        if((i%16 == 0) && (i))
            LOGI("\n");
        
        LOGI("%02X ", *(data++));
    }
    LOGI("\n");
}

int ParseIntToHex(int value, char *data, int index, int lenght)
{
    int i = 0;
    
      if (data != NULL && index >= 0 && lenght >= 0 && strlen(data) >= index + lenght)
    {
        for (i = 0; i < lenght; i++)
        {
            data[index + lenght - 1 - i] = (char)((value >> (8 * i)) & 0xFF);
        }
    }

    return 0;
}


/*
    设置位域，传入的参数范围1-64,低位靠左
*/
void SetBitMap(char* bitmap, int bit)
{
    int index, offset;
    bit -= 1;
    index = bit / 8 ;
    offset = bit % 8;
    bitmap[index] |= (char)(1 << (7 - offset));
}

/*
    查看该位是否被设置，传入参数范围1-64,di低位靠左
*/
int IsBitSet(char* bitmap, int bit)
{
    int index, offset;
    bit -= 1;
    index = bit / 8 ;    
    offset = bit % 8;
    char mask=0;
    mask = (char)(1 << (7 - offset));

    //printf(" bit=%d , index=%d , offset=%d ,mask=%02x bitmap[%d]=%02x ,bitmap[]&mask=%02x \n",bit,index,offset,mask,index,bitmap[index]&mask);
    
    if(bitmap[index]&mask)
        return 1;
    else
        return 0;
}

/* 这里是进行单倍的des加密 */

int PbocEncrypt_DES(char *dataIn, int datalen, char* dataOut, int *Outlen, char *key)
{
    unsigned char TempBuf[8];
    unsigned char OutTempBuf[16];
    unsigned char OutTempBuf_Hex[256];
    int CurPost = 0, i = 0, j = 0;
    des_context ctx;

    memset(OutTempBuf, 0, sizeof(OutTempBuf));

    DebugPrintChar("key", key, 8);

    
    while(datalen > 0)
    {
        memcpy(TempBuf, dataIn+CurPost, 8);
        des_set_key(&ctx, key);
        des_crypt(ctx.esk, TempBuf, OutTempBuf);
        memcpy(dataOut+CurPost, OutTempBuf, 8);
        datalen -= 8;
        CurPost += 8;
    }
    
    DebugPrintf("#################\n");
    
    for (i = 0; i < CurPost; i++)
    {
        sprintf(OutTempBuf_Hex + 2*i, "%02X", (unsigned char)dataOut[i]);
    }
    DebugPrintf("#################\n");
    //printf("OutTempBuf_Hex = %s\n", OutTempBuf_Hex);
    //*Outlen = CurPost;    
    strcpy(dataOut, OutTempBuf_Hex);
    //*Outlen = 2 * CurPost;    //一个字符加密后用两个字符来表示

    return 0;
}


/* 这里是进行单倍的des解密 */

int PbocDecrypt_DES(char *dataIn, int datalen, char* dataOut, int *Outlen, char *key)
{

    unsigned char TempBuf[8];
    unsigned char OutTempBuf[16];
    unsigned char OutTempBuf_Hex[128];
    int CurPost = 0, i = 0, j = 0;
    des_context ctx;

    memset(OutTempBuf, 0, sizeof(OutTempBuf));
    //DebugPrintChar("key = \n", key,8);
    /*
    while(datalen > 0)
    {
        memcpy(TempBuf, dataIn+CurPost, 8);
        des_set_key(&ctx, key);
        des_encrypt(ctx.dsk, TempBuf, OutTempBuf);
        memcpy(dataOut+CurPost, OutTempBuf, 8);
        DebugPrintChar("OutTempBuf = \n", OutTempBuf,8);
        datalen -= 8;
        CurPost += 8;
    }
    
    for (i = 0; i < CurPost; i++)
    {
        sprintf(OutTempBuf_Hex + 2*i, "%02X", (unsigned char)dataOut[i]);
    }
    
    printf("OutTempBuf_Hex = %s\n", OutTempBuf_Hex);
    strcpy(dataOut, OutTempBuf_Hex);
    *Outlen = 2*CurPost;    //一个字符加密后用两个字符来表示
    //memcpy(dataOut,OutTempBuf_Hex,8);
    */
    memcpy(TempBuf, dataIn, 8);
    des_set_key(&ctx, key);
    des_encrypt(&ctx, TempBuf, OutTempBuf);
    memcpy(dataOut, OutTempBuf, 8);
    
	return 0;
}


/*银联双免签到请求
  银联签到请求的长度为60(不包括长度字段),其他银行的不一定

*/
int BuildSignInInfoByte(unsigned char *data)
{
    char bitmap[8], tempBuf[32];
    //unsigned char data[256];
    int index=0, BitMapPos = 0;
    char tpdubuf[5];
    
    
    memset(data, 0, sizeof(data));
    memset(bitmap, 0, sizeof(bitmap));
    memset(tempBuf, 0, sizeof(tempBuf));
    memset(tpdubuf,0,sizeof(tpdubuf));
    
    data[index] = 0x00;
    data[index+1] = 0x3C;
    index += 2;

    //TPDU共10位 5个字节，前两位为TPDU ID 一般为"60",中间4位为TPDU 目的地址,最后4位为TPDU 源地址为"0000",
    //默认值为"60 00 03 00 00"

    memcpy(data+index,basinfo.TPDU,5);    
    index += 5;

    memcpy(data+index,basinfo.HEAD,6);
    index+=6;


    //消息类型0 1
    data[index] = 0x08;
    data[index+1] = 0x00;    
    index += 2;
    DebugPrintf("index = %d\n", index);

    //这里是预留给位元表的
    BitMapPos = index;
    printf("位元起始下标:%d \n",BitMapPos);
    index += 8;

    //pos机流水号
    LongUnon tmp;
    unsigned char BcdBuf[16];
    memset(BcdBuf,0,sizeof(BcdBuf));
    tmp.i = 0;
    
    memcpy(tmp.longbuf,DevSID.longbuf,4);
//    tmp.i = 100310;    
    Convert_IntTOBcd_2(tmp.i,BcdBuf);
    memcpy(data+index,BcdBuf+1,3);
    SetBitMap(bitmap, 11);
    index += 3;



    //终端代码    41域   由银行提供
    SetBitMap(bitmap, 41);
    //测试终端号
    memset(tempBuf,0,sizeof(tempBuf));
//    memcpy(tempBuf,"GA000802",strlen("GA000802"));                    //测试终端号
//    memcpy(tempBuf,"GA005901",strlen("GA005901"));
//    memcpy(tempBuf,card_driver.TermNum,8);
    memcpy(tempBuf,basinfo.TermNum,8);
    memcpy(&data[index], tempBuf, 8);
    index += 8;
    DebugPrintf("index = %d\n", index);

    //商户代码    42域   由银行提供
    SetBitMap(bitmap, 42);
    //测试商户号
    memset(tempBuf, 0x0, sizeof(tempBuf));
//    memcpy(tempBuf,"306530158120008",strlen("306530158120008"));    //测试商户号
//    memcpy(tempBuf,"306530156910059",strlen("306530156910059"));
//    memcpy(tempBuf,card_driver.MerchantNum,15);
    memcpy(tempBuf,basinfo.MerchantNum,15);
    memcpy(&data[index], tempBuf, 15);
    index += 15;
    DebugPrintf("index = %d\n", index);

    //自定义域    60域
    SetBitMap(bitmap, 60);
    data[index]=0x00,
    data[index+1] = 0x11;        //60域的长度
    index += 2;

    //交易码类型     60.1域
    data[index] = 0x00;
    index += 1;
    
    //批次号 60.2域
    data[index] = 0x00;
    data[index+1] = 0x00;
    data[index+2] = 0x00;   //需要使用双倍长密钥机制，网控器将返回60字节的密钥
    index += 3;
    DebugPrintf("index = %d\n", index);
    
    //网络管理信息码 60.3域
    data[index] = 0x00;
    data[index + 1] = 0x30;
    //data[index + 1] = 0x10;
    index += 2;
    
    //63 自定义域    ans...003    LLLVAR    ASCII 2    M(44,2)    
    SetBitMap(bitmap, 63);
    data[index] = 0x00;
    data[index + 1] = 0x03;
    index += 2;
    DebugPrintf("index = %d\n", index);

    //操作员代码
    data[index] = 0x30;
    data[index+1] = 0x30;
    data[index+2] = 0x31;
    index += 3;
    data[index] = '\0';

    memcpy(data+BitMapPos, bitmap, 8);
    
    printf("index = %d\n", index);
    
    DebugPrintChar("签到信息", data, index);

    return 0;
}

/*
    农商行的双免签到请求，长度为
*/

int BuildSignIn_NongYe(unsigned char *data,unsigned char * tpdu,unsigned char* head,unsigned char *terminal,unsigned char* mchant)
{
    char bitmap[8], tempBuf[32];
    //unsigned char data[256];
    int index=0, BitMapPos = 0;
    char tpdubuf[5];
    
    
    memset(data, 0, sizeof(data));
    memset(bitmap, 0, sizeof(bitmap));
    memset(tempBuf, 0, sizeof(tempBuf));
    
    data[index] = 0x00;
    data[index+1] = 0x3F;                    //报文长度 ,60.5 60.6 60.7没写
    index += 2;

    memcpy(data+index,tpdu,5);
    index+=5;
    memcpy(data+index,head,6);
    index+=6;

    data[index++]=0x08;
    data[index++]=0x00;

    BitMapPos=index;
    index+=8;
    
    SetBitMap(bitmap,11);
    LongUnon tmp;
    unsigned char BcdBuf[16];
    memset(BcdBuf,0,sizeof(BcdBuf));
    tmp.i = 0;
    memcpy(tmp.longbuf,DevSID.longbuf,4);
    Convert_IntTOBcd_2(tmp.i,BcdBuf);
    memcpy(data+index,BcdBuf+1,3);
    SetBitMap(bitmap, 11);
    index += 3;

    SetBitMap(bitmap,21);            //这里与银联的有差异
    data[index++]='1';                //下发邋TDKEY

    SetBitMap(bitmap,41);
    memcpy(data+index,terminal,8);
    index+=8;

    SetBitMap(bitmap,42);
    memcpy(data+index,mchant,15);
    index+=15;

    SetBitMap(bitmap,60);
    data[index]=0x00,
    data[index+1] = 0x11;        //60域的长度 ,60.4 60.5 60.6 60.7没写
    index += 2;
    //交易码类型     60.1域
    data[index] = 0x00;
    index += 1;
    //批次号 60.2域
    data[index] = 0x00;
    data[index+1] = 0x00;
    data[index+2] = 0x01;   //需要使用双倍长密钥机制，网控器将返回60字节的密钥
    index += 3;
    DebugPrintf("index = %d\n", index);
    //网络管理信息码 60.3域 与 60.4
    data[index] = 0x00;
    data[index + 1] = 0x30;        //有一位数是60.4
    index += 2;
#if 0    
    //60.5
    data[index++]=;
    //60.6
    data[index++]=;
    //60.7
    data[index++]=;
#endif
    SetBitMap(bitmap,63);
    data[index]=0;
    data[index+1]=0x05;
    index+=2;
    //操作员代码
    data[index] = 0x30;
    data[index+1] = 0x30;
    data[index+2] = 0x31;
    index += 3;
    data[index]=0x30;
    data[index]=0x31;
    index+=2;

    data[index] = '\0';
    memcpy(data+BitMapPos, bitmap, 8);
    
    printf("index = %d\n", index);
    
    DebugPrintChar("签到信息", data, index);
    
    return 0;
}



/*
构造回响测试报文，保持链接
*/
int BuildFeedbackByte(unsigned char *data)
{
    char bitmap[8], tempBuf[32];
    //unsigned char data[256];
    int index=0, BitMapPos = 0;
    char tpdubuf[5];
    
    
    memset(data, 0, sizeof(data));
    memset(bitmap, 0, sizeof(bitmap));
    memset(tempBuf, 0, sizeof(tempBuf));
    memset(tpdubuf,0,sizeof(tpdubuf));
    
    
    index += 2;

    //TPDU共10位 5个字节，前两位为TPDU ID 一般为"60",中间4位为TPDU 目的地址,最后4位为TPDU 源地址为"0000",
    //默认值为"60 00 03 00 00"

    memcpy(data+index,basinfo.TPDU,5);    
         index += 5;

    memcpy(data+index,basinfo.HEAD,6);
    index+=6;


    //消息类型0 1
        data[index] = 0x08;
        data[index+1] = 0x20;    
    index += 2;
    DebugPrintf("index = %d\n", index);

    //这里是预留给位元表的
    BitMapPos = index;
    printf("位元起始下标:%d \n",BitMapPos);
    index += 8;

    



    //终端代码    41域   由银行提供
    SetBitMap(bitmap, 41);
    //测试终端号
    memset(tempBuf,0,sizeof(tempBuf));

    memcpy(tempBuf,basinfo.TermNum,8);
    memcpy(&data[index], tempBuf, 8);
    index += 8;
    DebugPrintf("index = %d\n", index);

    //商户代码    42域   由银行提供
    SetBitMap(bitmap, 42);
    //测试商户号
    memset(tempBuf, 0x0, sizeof(tempBuf));
//    memcpy(tempBuf,"306530158120008",strlen("306530158120008"));    //测试商户号
//    memcpy(tempBuf,"306530156910059",strlen("306530156910059"));
//    memcpy(tempBuf,card_driver.MerchantNum,15);
    memcpy(tempBuf,basinfo.MerchantNum,15);
    memcpy(&data[index], tempBuf, 15);
    index += 15;
    DebugPrintf("index = %d\n", index);

    //自定义域    60域
    SetBitMap(bitmap, 60);
    data[index]=0x00,
    data[index+1] = 0x11;        //60域的长度
    index += 2;

    //交易码类型     60.1域
    data[index] = 0x00;
    index += 1;
    
    //批次号 60.2域
    data[index] = 0x00;
    data[index+1] = 0x00;
    data[index+2] = 0x01;   //需要使用双倍长密钥机制，网控器将返回60字节的密钥
    index += 3;
    DebugPrintf("index = %d\n", index);
    
    //网络管理信息码 60.3域
    data[index] = 0x30;
    data[index + 1] = 0x10;
    index += 2;
    
    data[0] = 0;
    data[1] = index-2;
    

    memcpy(data+BitMapPos, bitmap, 8);
    
    printf("index = %d\n", index);
    
    DebugPrintChar("回响信息", data, index);
    
    return 0;
}





#if 0 //没有被调用过
int BuildGetCodeInfo(unsigned char *data)
{
    char bitmap[8], tempBuf[32];
    //unsigned char data[256];
    int index=0, BitMapPos = 0;
    char tpdubuf[5];
    
    memset(data, 0, sizeof(data));
    memset(bitmap, 0, sizeof(bitmap));
    memset(tempBuf, 0, sizeof(tempBuf));
    memset(tpdubuf,0,sizeof(tpdubuf));
    
//    data[index] = 0x00;
//    data[index+1] = 0x5f;
    
    index += 2;

    //TPDU共10位 5个字节，前两位为TPDU ID 一般为"60",中间4位为TPDU 目的地址,最后4位为TPDU 源地址为"0000",
    //默认值为"60 00 03 00 00"
    
#ifdef YUNNAN
        //memcpy(data+index,card_driver.TPDU2,5);    
        ASCToBCD(tpdubuf,card_driver.TPDU2,10);
        memcpy(data+index,tpdubuf,5);
#else
        data[index]=0x60;
        data[index + 1]=0x00;//测试
        data[index + 2]=0x04;//测试
      //  data[index + 1] = 0x00;//正式
      //  data[index + 2]=0x09;
        data[index + 3]=0x00;
        data[index + 4]=0x00;
#endif    

    index += 5;
    DebugPrintf("index = %d\n", index);
    //报文头：应用类别 1
    data[index] = 0x60;
    index += 1;
    DebugPrintf("index = %d\n", index);
    //报文头：软件版本号 1
    data[index ] = 0x31;
    index += 1;
    DebugPrintf("index = %d\n", index);
    //报文头：终端状态和处理要求 00
    data[index] = 0x00;
    index += 1;
    DebugPrintf("index = %d\n", index);
    //报文头：保留 3 
    data[index] = 0x00;
    data[index + 1] = 0x00;
    data[index + 2] = 0x00;
    index += 3;
    DebugPrintf("index = %d\n", index);
    
    //消息类型0 1
    data[index] = 0x08;
    data[index+1] = 0x00;    
    index += 2;
    DebugPrintf("index = %d\n", index);

    //这里是预留给位元表的
    BitMapPos = index;
    index += 8;    
    DebugPrintf("index = %d\n", index);
    //之后需要加进去

//终端代码    41域   由银行提供
    SetBitMap(bitmap, 41);
    //测试终端号
    memset(tempBuf,0,sizeof(tempBuf));
//    memcpy(tempBuf,"GA000802",strlen("GA000802"));                    //测试终端号
//    memcpy(tempBuf,"GA005901",strlen("GA005901"));
    memcpy(tempBuf,card_driver.TermNum,8);
    memcpy(&data[index], tempBuf, 8);
    index += 8;
    DebugPrintf("index = %d\n", index);

    //商户代码    42域   由银行提供
    SetBitMap(bitmap, 42);
    //测试商户号
    memset(tempBuf, 0x0, sizeof(tempBuf));
//    memcpy(tempBuf,"306530158120008",strlen("306530158120008"));    //测试商户号
//    memcpy(tempBuf,"306530156910059",strlen("306530156910059"));
    memcpy(tempBuf,card_driver.MerchantNum,15);
    memcpy(&data[index], tempBuf, 15);
    index += 15;
    DebugPrintf("index = %d\n", index);



    //自定义域    60域
    SetBitMap(bitmap, 60);
    data[index] = 0;
    data[index+1] = 0x20;
    index += 2;
    DebugPrintf("index = %d\n", index);

    //交易码类型     60.1域
    
    sprintf(tempBuf,"%8d",DevNum.i);
    memcpy(data+index,tempBuf,8);
    index += 8;
    DebugPrintf("index = %d\n", index);
    //批次号 60.2域
    data[index] = 0x00;
    data[index+1] = 0x04;
    index += 2;
    
    DebugPrintf("index = %d\n", index);
    
    //62 终端信息/终端密钥    b...084    LLLVAR    ASCII 2    M(44,2)    
    SetBitMap(bitmap, 62);
    data[index] = 0x00;
    data[index + 1] = 0x3c;
    index += 2;
    DebugPrintf("index = %d\n", index);

    //操作员代码
    memcpy(data+index,SignData.DeviceKeyBuf,60);
    index += 60;
    data[index] = '\0';

    memcpy(data+BitMapPos, bitmap, 8);
    
    data[0] = 0x00;
    data[1] = index-2;
    DebugPrintf("index = %d\n", index);
    
    DebugPrintChar("签到信息", data, index);

    return 0;
}
#endif


int Convert_HexTOInt(char *HexBuf, int len)
{
    char *p = HexBuf;
    int i = 0, value = 0;
    
    while(len--)
    {
        if((*p >= '0') && (*p <= '9'))
            i = (*p - '0');
        else if ((*p >= 'A') && (*p <= 'F'))
            i = (*p - 'A' + 10);
        else if ((*p >= 'a') && (*p <= 'f'))
            i = (*p - 'a' + 10);
        else
            i = -1;
        
        if (i >= 0)
            value = value * 16 + i;
        
        p++;        
    }
    //printf("value = %d\n", value);
    return value;
}

int Convert_StrToInt(char *HexBuf, int len)
{
    char *p = HexBuf;
    int i = 0, value = 0;
    while(len--)
    {
        if((*p >= '0') && (*p <= '9'))
            i = (*p - '0');
        else
            i = -1;
        
        if (i >= 0)
            value = value * 10 + i;
        
        p++;        
    }
    //printf("value = %d\n", value);
    return value;
}

int Convert_HexTOStr(unsigned char *InBuf, int InLen,unsigned char *OutBuf, int *OutLen)
{
    int i = 0, j = 0;

    for(i=0; i< InLen; i += 2)
    {
        OutBuf[j] = Convert_HexTOInt(InBuf+i, 2);
        
        //printf("OutBuf[%d] = %02X ", j,OutBuf[j]);
        j++;
     }
    printf("\n");

    *OutLen = InLen/2;
    
    return 0;
}


int Convert_IntTOBcd(int num,unsigned char *OutBuf)
{
    int i = 0, j = 0;
    unsigned char NumBuf[8];

    memset(NumBuf, 0, sizeof(NumBuf));
    sprintf(NumBuf, "%04d", num);

    for(i=0; i<4; i+=2)
    {
        OutBuf[j] = (NumBuf[i] - '0') << 4 | (NumBuf[i+1] - '0');
        j++;
    }

    return 0;
}

int Convert_IntTOBcd_2(int num,unsigned char *OutBuf)
{
    int i = 0, j = 0;
    unsigned char NumBuf[16];

    memset(NumBuf, 0, sizeof(NumBuf));
    sprintf(NumBuf, "%08d", num);
    

    for(i=0; i<8; i+=2)
    {
        OutBuf[j] = (NumBuf[i] - '0') << 4 | (NumBuf[i+1] - '0');
        j++;
    }
//    printf("输入BUF:%s\n",NumBuf);
//    printf("输出BUF:%02x%02x%02x%02x\n",OutBuf[0],OutBuf[1],OutBuf[2],OutBuf[3]);

    return 0;
}

int Convert_BcdTOInt(char *InBuf,int len)
{
    int i = 0, j = 0;
    int date[]={10,100,1000,10000,100000};
    //0x12;
   
     if(len==1)
    {
        j = (InBuf[0]>>4)*date[len-1]+(InBuf[0]&0x0f);
        return j;
        }
    //0x12 0x34
    
    if(len==2)
    {
        j = (InBuf[0]>>4)*date[len]+(InBuf[0]&0x0f)*date[len-1]+(InBuf[1]>>4)*date[len-2]+(InBuf[1]&0x0f);
        return j;
        }  

    return 0;
}



int ChangBigToEd(char *inbuf,char *outbuf)
{
    char temp[4];
    int i;
    memcpy(temp,inbuf,4);
    for(i=0;i<4;i++)
        {
         outbuf[i] = temp[3-i];
        }
    printf("输入BUF:%02x%02x%02x%02x\n",temp[0],temp[1],temp[2],temp[3]);
    printf("输出BUF:%02x%02x%02x%02x\n",outbuf[0],outbuf[1],outbuf[2],outbuf[3]);
    return 0;
}

//根据得到的位元表信息，将数据逐一取出
int GetBitMap(char *pBitDataIn ,char *pBitMapOut)
{
    int i, j;
    unsigned char HexChar, TempBuf[8];

    for(i=0; i<8; i++)
    {
        memcpy(TempBuf, pBitDataIn+2*i, 2);
        HexChar = Convert_HexTOInt(TempBuf, 2);
        //printf("HexChar = 0x%02X\n", HexChar);
        
        for(j=0; j<8; j++)
        {
            pBitMapOut[i*8+ (7-j)] = (HexChar >> j) & (0x01);
            //printf("%X\n", pBitMapOut[i*8+j]);
        }
    }
    //printf("\n");
    
    return 0;
}

int GetBitMap1(char *pBitDataIn ,char *pBitMapOut)
{
    int i, j;
    unsigned char HexChar, TempBuf[8];

    for(i=0; i<8; i++)
    {
        //memcpy(TempBuf, pBitDataIn+2*i, 2);
        //HexChar = Convert_HexTOInt(TempBuf, 2);
        //printf("HexChar = 0x%02X\n", HexChar);
        
        for(j=0; j<8; j++)
        {
            pBitMapOut[i*8+ (7-j)] = (pBitDataIn[i] >> j) & (0x01);
            //printf("%X\n", pBitMapOut[i*8+j]);
        }
    }
    //printf("\n");
    
    return 0;
}



int GetDecodeBsaeKey(char *sour, int len)
{
    unsigned char TempBuf[16], TempBuf_HEX[16];
    int TempLen = 0, Outlen;

    DebugPrintf("sour = %s[%d]\n", sour, len);

    memset(TempBuf_HEX, 0, sizeof(TempBuf_HEX));
    memset(TempBuf, 0, sizeof(TempBuf));

    memcpy(TempBuf_HEX, sour + len/2, 16);

    Convert_HexTOStr(TempBuf_HEX, 16, TempBuf, &TempLen);
    DebugPrintf("TempLen = %d\n", TempLen);

    memset(PbocKey.DecryptKey, 0, sizeof(PbocKey.DecryptKey));
    
    PbocDecrypt_DES(TempBuf, TempLen, PbocKey.DecryptKey, &Outlen, PbocKey.BaseKey);
    DebugPrintChar("DecryptKey", PbocKey.DecryptKey, 8);

    return 0;
}




#if 0

/*签到返回处理解析*/

int HandleSignReplay(char *str,int len)
{
    int RecvLen = 0, i=0, j=0;
    int index = 0, TempInt = 0,DeviceKeyBuf = 0 ;
    unsigned char pTempBuf[128], pBitMap[64];
    ShortUnon datalen;


    char *pStr = str;
    memset(&SignData, 0, sizeof(SignData));
    memset(pBitMap, 0, sizeof(pBitMap));

    datalen.i = 0;
    datalen.intbuf[0] = pStr[1];
    datalen.intbuf[1] = pStr[0];
    if(datalen.i != (len-2))
    {
        printf("签到返回数据错误：长度不对！返回数据 : %d\n", RecvLen);
    }

    memcpy(pTempBuf,pStr+15,8);    //从数组中取位元表
    GetBitMap1(pTempBuf, pBitMap);
    DebugPrintChar("位元表内容:",pTempBuf,8);
    DebugPrintChar("转换后的内容:",pBitMap,64);

    index=15+8;

    for(i=0;i<64;i++)
    {
        if(pBitMap[i])
        {
            printf("--- 检查到域:%d \n",i+1);
            switch(i+1)
            {
                case 11:        //受卡方系统跟踪号
                    memcpy(SignData.strSernialNo,pStr+index,3);
                    index+=3;
                    break;
                case 12:        //受卡方所在地时间
                    memcpy(&SignData.time.hour,pStr+index,3);
                    index+=3;
                    break;
                case 13:
                    memcpy(&SignData.time.month,pStr+index,2);
                    index+=2;
                    break;
                case 32:        //受理方标识码
                    TempInt=Convert_BcdTOInt(pStr+index,1);
                    DebugPrintf("长度:%d\n",TempInt);
                    index++;
                    memcpy(&SignData.AcceptorFlag,pStr+index,TempInt);
                    pStr+=TempInt;
                    break;

                case 37:        //检索参考号
                    memcpy(&SignData.RetrievalNo,pStr+index,12);
                    index+=12;
                    break;
                case 39:
                    memcpy(SignData.ReplyCode, pStr+index, 2);
                    DebugPrintChar("应答码:",SignData.ReplyCode,2);
                    printf("应答码:%02x%02x \n",SignData.ReplyCode[0],SignData.ReplyCode[1]);
                    if((SignData.ReplyCode[0]!=0x30)|| (SignData.ReplyCode[1]!=0x30))
                        return 1;
                    index+=2;
                    break;
                case 40:
                    TempInt=Convert_BcdTOInt(pStr+index,1);
                    printf("长度:%d\n",TempInt);
                    index+=1;
                    memcpy(SignData.RetrievalNo, pStr+index, TempInt);
                    DebugPrintChar("应答描述:",SignData.ReplyCode,TempInt);
                    printf("应答描述:%s\n",SignData.ReplayDescri);
                    index+=TempInt;
                    break;
                case 41:
                    memcpy(SignData.DeviceNo, pStr+index, 8);
                    DebugPrintChar("终端机标识码:",SignData.DeviceNo,8);
                    index+=8;
                    break;
                case 42:
                    memcpy(SignData.strTCode, pStr+index, 15);
                    DebugPrintChar("受卡方标识码:",SignData.strTCode,15);
                    index+=15;
                    break;
                case 60:
                    datalen.i = 0;
                    datalen.intbuf[0] = *(pStr+index);
                    datalen.intbuf[1] = *(pStr+index+1);
                    index+=2;

                    TempInt=BCDToDec(datalen.intbuf,2);        //指的是位数，所以要除以2
                    TempInt = TempInt/2;
                    if(TempInt%2)
                        TempInt+=1;

                    memcpy(SignData.userdefine,pStr+index,TempInt);
                    index+=TempInt;
                    DebugPrintChar("自定义域:",SignData.userdefine,TempInt);
                    memcpy(UploadData.BatchCode,SignData.userdefine+1,3);

                    /*获取信息管理码*/

                    //memcpy(SignData.netmange,SignData.userdefine+4,2);

                    printf("网络管理信息码:%02x%02x \n",SignData.userdefine[4],SignData.userdefine[5]);
                case 62:
                    datalen.i = 0;
                    datalen.intbuf[0] = *(pStr+index);
                    datalen.intbuf[1] = *(pStr+index+1);
                    index+=2;
                    TempInt=BCDToDec(datalen.intbuf,2);                    //24 , 40 ,60 三种情况
                    memcpy(SignData.DeviceKeyBuf,pStr+index,TempInt);
                    DebugPrintChar("工作密钥密文",SignData.DeviceKeyBuf , TempInt);
                 default:
                    printf("未解析该域\n");
                    break;
            }
        }

    }

    return 0;

}
#else
//返回：004F 6000000003612200000000 0810 003800010AC00010
//000001 180147 1002 0848024210 303030323033333033303136 303332303130303334343935323531303634313131303030310011001410020040
//判断长度是否正确
int HandleSignReplay(char *str,int len)
{
    int RecvLen = 0, i=0, j=0;
    int tmplen;
    int index = 0, TempInt = 0,DeviceKeyBuf = 0 ;
    unsigned char pTempBuf[128], pBitMap[64];
    ShortUnon datalen;
    int offset=0;

//    struct SignContainerReply SignData;
    
    memset(&SignData, 0, sizeof(SignData));
    memset(pBitMap, 0, sizeof(pBitMap));
    memset(pTempBuf, 0, sizeof(pTempBuf));

    char *pText = "\x00\x00\x00\x00\x00\x00\x00\x00";
    //char *pText = "0000000000000000";
    //char * pStr =   "004F60000000096021000000000810003800010AC0001000000116444710090800096500303030303030303030303030393732303130303334343935323531303634313131303030310011001410090010";
//    char * pStr =     "006960000000096021000000000810003800010AC00014000002145850111908000965003134353835303333313531303030323031353230383133303535313036343131313030303100110000002400100024B2BE021366619F2785CE901640D96E17766C3F2784001126";
    char *pStr = str;

    datalen.i = 0;
    datalen.intbuf[0] = pStr[1];
    datalen.intbuf[1] = pStr[0];
    DebugPrintChar("接收到平台签到返回值:",pStr,len);
    if(datalen.i != (len-2))
    {
        printf("签到返回数据错误：长度不对！返回数据 : %d\n", RecvLen);
    }

    memcpy(pTempBuf,pStr+15,8);    //从数组中取位元表
    GetBitMap1(pTempBuf, pBitMap);
    DebugPrintChar("位元表内容:",pTempBuf,8);
    DebugPrintChar("转换后的内容:",pBitMap,64);

    offset=15+8;
    
    //11 受卡方系统跟踪号(POS终端交易流水) 46
    if (pBitMap[10] == 1)
    {
         memcpy(SignData.strSernialNo, pStr+offset, 3);            // 23
         DebugPrintChar("交易流水号:",SignData.strSernialNo,3);
         offset+=3;
    }

    
    //12 和 13 受卡方所在地日期    52     
    if(pBitMap[11] == 1 && pBitMap[12]==1)
    {
    
        SignData.time.hour= *(pStr+offset);        // 26
        SignData.time.min= *(pStr+offset+1);
        SignData.time.sec= *(pStr+offset+2);
        SignData.time.month= *(pStr+offset+3);
        SignData.time.day= *(pStr+offset+4);
        DebugPrintf("SignData.time.hour = %02x\n", SignData.time.hour);
        DebugPrintf("SignData.time.min = %02x\n", SignData.time.min);
        DebugPrintf("SignData.time.sec = %02x\n", SignData.time.sec);
        DebugPrintf("SignData.time.month = %02x\n", SignData.time.month);
        DebugPrintf("SignData.time.day = %02x\n", SignData.time.day);
        offset+=5;
    }

    //32    受理方标识码    62 
    if(pBitMap[31] == 1)
    {

        TempInt = *(pStr+offset);        
        offset++;
        memcpy(SignData.AcceptorFlag, pStr+offset, TempInt/2);
        DebugPrintChar("受卡方标识:",SignData.AcceptorFlag,4);        
        offset+=TempInt/2;
        
    }

    //37    检索参考号         72
    if(pBitMap[36] == 1)
    {
        memcpy(SignData.RetrievalNo, pStr+offset, 12);
        DebugPrintChar("检索参考号:",SignData.RetrievalNo,12);    
        offset+=12;
    }

    //39    应答码            96
    if(pBitMap[38] == 1)
    {
        memcpy(SignData.ReplyCode, pStr+offset, 2);
        DebugPrintChar("应答码:",SignData.ReplyCode,2);    
        offset+=2;
      if((SignData.ReplyCode[0]!=0x30)&&(SignData.ReplyCode[1]!=0x30))
            return 1;        
    }

    //40域，应答码描述
    if(pBitMap[39]==1)
    {
        
        tmplen=((pStr[offset]>>4)&0x0f)*10 + pStr[offset]&0x0f;
        printf("40域长度: %02x %d",pStr[offset],tmplen);
        offset++;
        memcpy(SignData.ReplayDescri,pStr+offset,tmplen);
        DebugPrintChar("应答码描述:",SignData.ReplayDescri,tmplen);    
        pStr+=tmplen;
    }



    //41    受卡机终端标识码    100
    if(pBitMap[40] == 1)
    {
        memcpy(SignData.DeviceNo, pStr+offset, 8);
        DebugPrintChar("终端机标识码:",SignData.DeviceNo,8);
        offset+=8;
    }

    //42    受卡方标识码    116  商户代码
    if(pBitMap[41] == 1)
    {
        memcpy(SignData.strTCode, pStr+offset, 15);
        DebugPrintChar("受卡方标识码:",SignData.strTCode,15);
        offset+=15;
    }

    //60 自定义域        146
    if(pBitMap[59] == 1)
    {
        /*
        memcpy(pTempBuf, pStr+73, 4);

        //自定义域的长度
        TempInt = Convert_StrToInt(pTempBuf, 4);
        DebugPrintf("TempInt = %d\n", TempInt);

        //交易码类型
        memcpy(SignData.TradeType, RecvData+150, 2);
        DebugPrintf("SignData.TradeType = %s\n", SignData.TradeType);

        //批次号
        memcpy(SignData.strBatchNum, RecvData+152, 6);
        DebugPrintf("SignData.TradeType = %s\n", SignData.strBatchNum);

        //网络管理信息码
        memcpy(pTempBuf, RecvData+158, 4);
        SignData.NetManageCode = Convert_StrToInt(pTempBuf, 3);
        DebugPrintf("SignData.NetManageCode = %d\n", SignData.NetManageCode);
        */
                    
        printf("60 域，offset = %d \n",offset);
        datalen.i = 0;
        datalen.intbuf[0] = *(pStr+offset);
        datalen.intbuf[1] = *(pStr+offset+1);
        offset+=2;
        
        TempInt=BCDToDec(datalen.intbuf,2);        //指的是位数，所以要除以2
        TempInt = TempInt/2;
        if(TempInt%2)
            TempInt+=1;
        
        printf("60 域自定义长度 %d \n",TempInt);
        memcpy(SignData.userdefine,pStr+offset,TempInt);
        offset+=TempInt;
        DebugPrintChar("自定义域:",SignData.userdefine,TempInt);
        memcpy(UploadData.BatchCode,SignData.userdefine+1,3);
        
        /*获取信息管理码*/
        
        //memcpy(SignData.netmange,SignData.userdefine+4,2);
        
        printf("网络管理信息码:%02x%02x \n",SignData.userdefine[4],SignData.userdefine[5]);
    }

    //62域主控密钥密文
    /*
    memcpy(pTempBuf, RecvData+162, 4);
    TempInt = Convert_StrToInt(pTempBuf, 4);
    DebugPrintf("TempInt = %d\n", TempInt);
    
    memcpy(SignData.DeviceKeyBuf, RecvData+166, TempInt*2);
    DebugPrintf("SignData.TradeType = %s\n", SignData.DeviceKeyBuf);
    memcpy(SignData.DeviceKeyBuf, "9A2726F370A04D430E34C61C03ACE7EEF011454E9EEBA768", 48);
    DeviceKeyBuf = strlen(SignData.DeviceKeyBuf);
        
    GetDecodeBsaeKey(SignData.DeviceKeyBuf, DeviceKeyBuf);

    DebugPrintChar("pText", pText, 8);
    PbocEncrypt_DES(pText, 8, pTempBuf, TempInt, PbocKey.DecryptKey);

    DebugPrintChar("pTempBuf", pTempBuf , 8);
    
    if(!strncmp(pTempBuf, SignData.DeviceKeyBuf+DeviceKeyBuf-8, 8))
    {
        printf("It is success to calculate key\n");
    }
*/
    //62域
    if((pBitMap[61] == 1)    )
    {
        //TempInt 是60域的数据长度，基于上下文
        datalen.i = 0;
        datalen.intbuf[0] = *(pStr+offset);
        datalen.intbuf[1] = *(pStr+offset+1);        
        offset+=2;
        TempInt=BCDToDec(datalen.intbuf,2);                    //24 , 40 ,60 三种情况
        printf("62 域长度:%d \n",TempInt);
        memcpy(SignData.DeviceKeyBuf,pStr+offset,TempInt);
        DebugPrintChar("工作密钥密文",SignData.DeviceKeyBuf , TempInt);
    }
        //测试时给出的是明文密钥，
        //生产环境还需发送请求到解密软件去获得真正明文密钥
    
        


    /*       此步还需要处理的数据信息
     * 1. 用母pos机下发下来的秘钥来解密最后的面个des秘钥，得到明文秘钥;
     * 2. 读出文件对比终端代码和商户代码 ;
     * 3. 向文件里面写记录信息;
     */
     
    return 0;
}
#endif

#if 0
int HandleGetReplay(char *str,int len)
{
    int RecvLen = 0, i=0, j=0;
    int index = 0, TempInt = 0,DeviceKeyBuf = 0 ;
    unsigned char pTempBuf[128], pBitMap[64];
    ShortUnon datalen;

//    struct SignContainerReply SignData;
    
    memset(&SignData, 0, sizeof(SignData));
    memset(pBitMap, 0, sizeof(pBitMap));

    memset(pTempBuf, 0, sizeof(pTempBuf));

    char *pText = "\x00\x00\x00\x00\x00\x00\x00\x00";
    //char *pText = "0000000000000000";
    //char * pStr =   "004F60000000096021000000000810003800010AC0001000000116444710090800096500303030303030303030303030393732303130303334343935323531303634313131303030310011001410090010";
//    char * pStr =     "006960000000096021000000000810003800010AC00014000002145850111908000965003134353835303333313531303030323031353230383133303535313036343131313030303100110000002400100024B2BE021366619F2785CE901640D96E17766C3F2784001126";
    char *pStr = str;
    DebugPrintChar("接收到平台解密返回值:",pStr,len);
    datalen.i = 0;
    datalen.intbuf[0] = pStr[1];
    datalen.intbuf[1] = pStr[0];    
    if(datalen.i != (len-2))
    {
        printf("签到返回数据错误：长度不对！返回数据 : %d\n", RecvLen);
    }

    memcpy(pTempBuf,pStr+15,8);    //从数组中取位元表
    GetBitMap1(pTempBuf, pBitMap);
    DebugPrintChar("位元表内容:",pTempBuf,8);
    DebugPrintChar("转换后的内容:",pBitMap,64);    
    
    index = 23;
    //39    应答码            96
    if(pBitMap[38] == 1)
    {
        memcpy(SignData.ReplyCode, pStr+index, 2);
        DebugPrintChar("应答码:",SignData.ReplyCode,2);
        if((SignData.ReplyCode[0]!=0x30)&&(SignData.ReplyCode[1]!=0x30))
            return 1;
        index+=2;
        }    


    //41    受卡机终端标识码    100
    if(pBitMap[40] == 1)
    {
        memcpy(SignData.DeviceNo, pStr+index, 8);
        DebugPrintChar("终端机标识码:",SignData.DeviceNo,8);
        index+=8;
    }

    //42    受卡方标识码    116
    if(pBitMap[41] == 1)
    {
        memcpy(SignData.strTCode, pStr+index, 15);
        DebugPrintChar("受卡方标识码:",SignData.strTCode,15);
        index+=15;
    }    

    //60 自定义域        146
    if(pBitMap[59] == 1)
    {
        /*
        memcpy(pTempBuf, pStr+73, 4);

        //自定义域的长度
        TempInt = Convert_StrToInt(pTempBuf, 4);
        DebugPrintf("TempInt = %d\n", TempInt);

        //交易码类型
        memcpy(SignData.TradeType, RecvData+150, 2);
        DebugPrintf("SignData.TradeType = %s\n", SignData.TradeType);

        //批次号
        memcpy(SignData.strBatchNum, RecvData+152, 6);
        DebugPrintf("SignData.TradeType = %s\n", SignData.strBatchNum);

        //网络管理信息码
        memcpy(pTempBuf, RecvData+158, 4);
        SignData.NetManageCode = Convert_StrToInt(pTempBuf, 3);
        DebugPrintf("SignData.NetManageCode = %d\n", SignData.NetManageCode);
        */
        datalen.i = 0;
        datalen.intbuf[0] = *(pStr+index);
        datalen.intbuf[1] = *(pStr+index+1);
        TempInt=BCDToDec(datalen.intbuf,2);
        TempInt = TempInt/2;
        if(TempInt%2)
            TempInt+=1;
        index+=2;
        memcpy(SignData.userdefine,pStr+index,TempInt);
        DebugPrintChar("自定义域:",SignData.userdefine,TempInt);
        index+=TempInt;
    }

    //62域主控密钥密文
    /*
    memcpy(pTempBuf, RecvData+162, 4);
    TempInt = Convert_StrToInt(pTempBuf, 4);
    DebugPrintf("TempInt = %d\n", TempInt);
    
    memcpy(SignData.DeviceKeyBuf, RecvData+166, TempInt*2);
    DebugPrintf("SignData.TradeType = %s\n", SignData.DeviceKeyBuf);
    memcpy(SignData.DeviceKeyBuf, "9A2726F370A04D430E34C61C03ACE7EEF011454E9EEBA768", 48);
    DeviceKeyBuf = strlen(SignData.DeviceKeyBuf);
        
    GetDecodeBsaeKey(SignData.DeviceKeyBuf, DeviceKeyBuf);

    DebugPrintChar("pText", pText, 8);
    PbocEncrypt_DES(pText, 8, pTempBuf, TempInt, PbocKey.DecryptKey);

    DebugPrintChar("pTempBuf", pTempBuf , 8);
    
    if(!strncmp(pTempBuf, SignData.DeviceKeyBuf+DeviceKeyBuf-8, 8))
    {
        printf("It is success to calculate key\n");
    }
*/
        
        datalen.i = 0;
        datalen.intbuf[0] = *(pStr+index);
        datalen.intbuf[1] = *(pStr+index+1);        
        index += 2;
        TempInt=BCDToDec(datalen.intbuf,2);        
        memcpy(SignData.DeviceKeyBuf,pStr+index,TempInt);
        DebugPrintChar("主控密钥密文", SignData.DeviceKeyBuf , TempInt);

        //测试时给出的是明文密钥，
        //生产环境还需发送请求到解密软件去获得真正明文密钥
    
        memcpy(PbocKey.DecryptKey,SignData.DeviceKeyBuf,TempInt);            



    return 0;
}
#endif

/*
    消费返回报文解析
*/
int HandelSendReplay(char *RevBuf,int len)
{
    int errcode=-1;
    int RecvLen = 0, i=0, j=0;
    int index = 0, TempInt = 0;
    unsigned char pTempBuf[128], pBitMap[64];
    unsigned char pStr[1024], TempChar;
    ShortUnon datalen;
    char temp;
    struct UploadRecodContainerReply ReplyData;
    
    memset(&ReplyData, 0, sizeof(ReplyData));
    memset(pBitMap, 0, sizeof(pBitMap));
    

    memset(pTempBuf, 0, sizeof(pTempBuf));

    //pStr = RevBuf;
    memcpy(pStr,RevBuf,len);
    
    DebugPrintChar("接收到平台返回值:",pStr,len);    
    datalen.i = 0;
    datalen.intbuf[0] = pStr[1];
    datalen.intbuf[1] = pStr[0];    
    if(datalen.i != (len-2))
    {
        DebugPrintf("数据错误：长度不对！返回数据 : %d\n", RecvLen);
    }

    memcpy(pTempBuf,pStr+15,8);    //从数组中取位元表
    GetBitMap1(pTempBuf, pBitMap);
    DebugPrintChar("位元表内容:",pTempBuf,8);
    DebugPrintChar("转换后的内容:",pBitMap,64);
    index=15+8;
    memset(&ConsumeData,0,sizeof(struct ConsumeContainerReply));
    for(i=0;i<64;i++)
    {
    if(pBitMap[i])
    {
        DebugPrintf("--- 检查到域:%d \n",i+1);
        switch(i+1)
        {
            case 2:
                TempInt=Convert_BcdTOInt(pStr+23,1);
                DebugPrintf("长度:%d\n",TempInt);
                index+=1;
                if(TempInt%2>0)
                temp = (TempInt/2)+1;
                else
                temp = (TempInt/2);
                memcpy(ConsumeData.MainNo, pStr+index, temp);
                DebugPrintChar("主帐号:",ConsumeData.MainNo,temp);
                index+=temp;
                break;
            case 3:
                memcpy(ConsumeData.ProcCode, pStr+index, 3);
                DebugPrintChar("交易处理码:",ConsumeData.ProcCode,3);
                index+=3;
                break;
            case 4:
                memcpy(ConsumeData.MaxMoney, pStr+index, 6);
                DebugPrintChar("最大输入金额:",ConsumeData.MaxMoney,6);
                      index+=6;
                break;
            case 11:
                memcpy(ConsumeData.FlowNo, pStr+index, 3);
                DebugPrintChar("受卡方系统跟踪号:",ConsumeData.FlowNo,3);
                      index+=3;
                break;
            case 12:
                memcpy(ConsumeData.Time, pStr+index, 3);
                DebugPrintChar("受卡方时间:",ConsumeData.Time,3);
                index+=3;
                break;
                
            case 13:
                memcpy(ConsumeData.Date, pStr+index, 2);
                DebugPrintChar("受卡方日期:",ConsumeData.Date,2);
                index+=2;
                break;
            case 14:
                 memcpy(ConsumeData.EffeTime, pStr+index, 2);
                   DebugPrintChar("卡有效期:",ConsumeData.EffeTime,2);
                     index+=2;
                 break;
            case 15:
                memcpy(ConsumeData.BetTime, pStr+index, 2);
                DebugPrintChar("清算日期:",ConsumeData.BetTime,2);
                index+=2;
                break;
            case 23:
                 DebugPrintf("序列号：%02x %02x\n",pStr[index],pStr[index+1]);
                 index+=2;
                 break;
            case 25:
                ConsumeData.ConCode = *(pStr+index);    
                DebugPrintf("服务点条件码:%02x\n",*(pStr+index));
                index+=1;
                break;
            case 32:
                 // TempInt = *(pStr+index);
                TempInt=Convert_BcdTOInt(pStr+index,1);
                DebugPrintf("长度:%d\n",TempInt);
                index+=1;
                memcpy(ConsumeData.strTCode, pStr+index, TempInt/2);      
                DebugPrintChar("受卡标识码:",ConsumeData.strTCode,TempInt/2);
                index+=(TempInt/2);
                break;
            case 37:
                memcpy(ConsumeData.CheckNo, pStr+index, 12);
                DebugPrintChar("检索参考码:",ConsumeData.CheckNo,12);
                index+=12;
                break;
                
            case 38:
                memset(pTempBuf,0,sizeof(pTempBuf));
                memcpy(pTempBuf, pStr+index, 6);
                DebugPrintChar("授权码:",pTempBuf,6);
                index+=6;

                break;
            case 39:
                memcpy(ConsumeData.ReplyCode, pStr+index, 2);
                DebugPrintChar("应答码:",ConsumeData.ReplyCode,2);
                DebugPrintf(" in func %s , 应答码: %02x %02x\n",__func__,ConsumeData.ReplyCode[0],ConsumeData.ReplyCode[1]);
                if((ConsumeData.ReplyCode[0]==0x30)&&(ConsumeData.ReplyCode[1]==0x30))
                    errcode=0;
                if((ConsumeData.ReplyCode[0]==0x41)&&(ConsumeData.ReplyCode[1]==0x30))    
                    issign = 0;
                    
                index+=2;
                break;
            case 40:
                TempInt=Convert_BcdTOInt(pStr+index,1);
                DebugPrintf("长度:%d\n",TempInt);
                index+=1;
                memcpy(ConsumeData.Answerdes, pStr+index, TempInt);      
                DebugPrintChar("应答描述:",ConsumeData.Answerdes,TempInt);
                index+=TempInt;
                break;
            case 41:
                memcpy(ConsumeData.DeviceNo, pStr+index, 8);
                DebugPrintChar("终端代码:",ConsumeData.DeviceNo,8);
                index+=8;
                break;
            case 42:
                memcpy(ConsumeData.CostCode, pStr+index, 15);
                DebugPrintChar("商户代码:",ConsumeData.CostCode,15);
                index+=15;
                break;
            case 44:
                temp = Convert_BcdTOInt(pStr+index, 1);
                index+=1;
                memcpy(ConsumeData.Reparea, pStr+index, temp);
                DebugPrintChar("收单机构:",ConsumeData.Reparea,temp);
                index+=temp;
                break;
            case 48:
                TempInt = Convert_BcdTOInt(pStr+index,2);                
                index+=2;
                DebugPrintChar("行业业务数据:",pStr+index,TempInt);
                index+=TempInt;
                break;
            case 49:
                memcpy(ConsumeData.Currencycode, pStr+index, 3);
                DebugPrintChar("交易货币代码:",ConsumeData.Currencycode,3);
                index+=3;
                break;

            case 55:
                //
                TempInt = Convert_BcdTOInt(pStr+index,2);
                DebugPrintf("长度:%d \n",TempInt);
                index+=2;
                DebugPrintChar("IC 卡数据:",pStr+index,TempInt);
                index+=TempInt;
                break;


            case 59:
                TempInt = Convert_BcdTOInt(pStr+index,2);
                DebugPrintf("长度:%d \n",TempInt);
                index+=2;
                if((pStr[index]==0x41)&&(pStr[index+1]==0x34))
                    {
                    index+=2;
                    TempInt = (pStr[index]-'0')*100+(pStr[index+1]-'0')*10+(pStr[index+2]-'0');
                    DebugPrintf("支付凭证长度:%d \n",TempInt);
                    index+=3;
                    memcpy(ConsumeData.QRvocher,pStr+index,TempInt);
                    DebugPrintChar("支付凭证:",ConsumeData.QRvocher,TempInt);
                }
                index+=TempInt;
                break;
                
            case 60:
                //printf(" bcd len:%02x %02x\n",pStr[index],pStr[index+1]);
                //TempInt=(pStr[index]>>4)*10000+(pStr[index]&0x0f)*100+(pStr[index+1]>>4)*10+pStr[index+1]&0x0f;
                TempInt = Convert_BcdTOInt(pStr+index,2);
                DebugPrintf("长度:%d \n",TempInt/2);
                index +=2;
                memcpy(ConsumeData.Userdefine60, pStr+index, TempInt/2);
                DebugPrintChar("自定义域60:",ConsumeData.Userdefine60,TempInt/2);
                index+=TempInt/2;
                break;
            
            case 64:
                memcpy(ConsumeData.Mac,pStr+index,8);
                index+=8;
                break;
            default:
                DebugPrintf("-- err : 域%d 未处理\n",i+1);
            //    return -1;
                break;
        }

    }

    }

    return errcode;
}

#if 1
int HandelSendReplay_QR(char *RevBuf,int len)
{
    int errcode=-1;
    int RecvLen = 0, i=0, j=0;
    int index = 0, TempInt = 0;
    unsigned char pTempBuf[128], pBitMap[64];
    unsigned char RecvData[1024], TempChar;
    ShortUnon datalen;
    char temp;
    struct UploadRecodContainerReply ReplyData;

    memset(&ReplyData, 0, sizeof(ReplyData));
    memset(pBitMap, 0, sizeof(pBitMap));

    memset(RecvData, 0, sizeof(RecvData));
    memset(pTempBuf, 0, sizeof(pTempBuf));

    char *pStr = RevBuf;

    DebugPrintChar("接收到平台返回值:",pStr,len);
    datalen.i = 0;
    datalen.intbuf[0] = pStr[1];
    datalen.intbuf[1] = pStr[0];
    if(datalen.i != (len-2))
    {
        printf("数据错误：长度不对！返回数据 : %d\n", RecvLen);
    }

    memcpy(pTempBuf,pStr+15,8);    //从数组中取位元表
    GetBitMap1(pTempBuf, pBitMap);
    DebugPrintChar("位元表内容:",pTempBuf,8);
    DebugPrintChar("转换后的内容:",pBitMap,64);
    index=15+8;

    for(i=0;i<64;i++)
    {
        if(pBitMap[i])
        {
            DebugPrintf("--- 检查到域:%d \n",i+1);
            switch(i+1)
            {
              case 2:
                TempInt=Convert_BcdTOInt(pStr+23,1);
                DebugPrintf("长度:%d\n",TempInt);
                index+=1;
                if(TempInt%2>0)
                temp = (TempInt/2)+1;
                else
                temp = (TempInt/2);
                memcpy(ConsumeData.MainNo, pStr+index, temp);
                DebugPrintChar("主帐号:",ConsumeData.MainNo,temp);
                index+=temp;
                break;

              case 3:
                memcpy(ConsumeData.ProcCode, pStr+index, 3);
                DebugPrintChar("交易处理码:",ConsumeData.ProcCode,3);
                index+=3;
                break;
              case 4:
                memcpy(ConsumeData.MaxMoney, pStr+index, 6);
                DebugPrintChar("最大输入金额:",ConsumeData.MaxMoney,6);
                index+=6;
                break;

              case 11:
                memcpy(ConsumeData.FlowNo, pStr+index, 3);
                DebugPrintChar("受卡方系统跟踪号:",ConsumeData.FlowNo,3);
                index+=3;
                break;
            case 12:
                memcpy(ConsumeData.Time, pStr+index, 3);
                DebugPrintChar("受卡方时间:",ConsumeData.Time,3);
                index+=3;
                break;

            case 13:
                memcpy(ConsumeData.Date, pStr+index, 2);
                DebugPrintChar("受卡方日期:",ConsumeData.Date,2);
                index+=2;
                break;
            case 14:
                 memcpy(ConsumeData.EffeTime, pStr+index, 2);
                 DebugPrintChar("卡有效期:",ConsumeData.EffeTime,2);
                     index+=2;
                 break;
            case 15:
                memcpy(ConsumeData.BetTime, pStr+index, 2);
                DebugPrintChar("清算日期:",ConsumeData.BetTime,2);
                index+=2;
                break;
            case 23:
                 printf("序列号：%02x %02x\n",pStr[index],pStr[index+1]);
                 index+=2;
                 break;
            case 25:
                ConsumeData.ConCode = *(pStr+index);
                printf("服务点条件码:%02x\n",*(pStr+index));
                index+=1;
                break;
            case 32:
                 // TempInt = *(pStr+index);
                TempInt=Convert_BcdTOInt(pStr+index,1);
                DebugPrintf("长度:%d\n",TempInt);
                index+=1;
                memcpy(ConsumeData.strTCode, pStr+index, TempInt/2);
                DebugPrintChar("受卡标识码:",ConsumeData.strTCode,TempInt/2);
                index+=(TempInt/2);
                break;
            case 37:
                memcpy(ConsumeData.CheckNo, pStr+index, 12);
                DebugPrintChar("检索参考码:",ConsumeData.CheckNo,12);
                index+=12;
                break;

            case 39:
                memcpy(ConsumeData.ReplyCode, pStr+index, 2);
                DebugPrintChar("应答码:",ConsumeData.ReplyCode,2);
                printf(" in func %s , 应答码: %02x %02x\n",__func__,ConsumeData.ReplyCode[0],ConsumeData.ReplyCode[1]);
                if((ConsumeData.ReplyCode[0]==0x30)&&(ConsumeData.ReplyCode[1]==0x30))
                    errcode=0;
                if((ConsumeData.ReplyCode[0]==0x41)&&(ConsumeData.ReplyCode[1]==0x30))
                    issign = 0;

                index+=2;
                break;
            case 40:
                TempInt=Convert_BcdTOInt(pStr+index,1);
                DebugPrintf("长度:%d\n",TempInt);
                index+=1;
                memcpy(ConsumeData.Answerdes, pStr+index, TempInt);
                DebugPrintChar("应答描述:",ConsumeData.Answerdes,TempInt);
                index+=TempInt;
                break;
            case 41:
                memcpy(ConsumeData.DeviceNo, pStr+index, 8);
                DebugPrintChar("终端代码:",ConsumeData.DeviceNo,8);
                index+=8;
                break;
            case 42:
                memcpy(ConsumeData.CostCode, pStr+index, 15);
                DebugPrintChar("商户代码:",ConsumeData.CostCode,15);
                index+=15;
                break;
            case 44:
                temp = Convert_BcdTOInt(pStr+index, 1);
                index+=1;
                memcpy(ConsumeData.Reparea, pStr+index, temp);
                DebugPrintChar("收单机构:",ConsumeData.Reparea,temp);
                index+=temp;
                break;
            case 48:
                TempInt = Convert_BcdTOInt(pStr+index,2);
                printf("");
                index+=2;
                DebugPrintChar("行业业务数据:",pStr+index,TempInt);
                index+=TempInt;
                break;
            case 49:
                memcpy(ConsumeData.Currencycode, pStr+index, 3);
                DebugPrintChar("交易货币代码:",ConsumeData.Currencycode,3);
                index+=3;
                break;
            case 55:
                //
                TempInt = Convert_BcdTOInt(pStr+index,2);
                printf("长度:%d \n",TempInt);
                index+=2;
                DebugPrintChar("IC 卡数据:",pStr+index,TempInt);
                index+=TempInt;
                break;

            case 60:
                //printf(" bcd len:%02x %02x\n",pStr[index],pStr[index+1]);
                //TempInt=(pStr[index]>>4)*10000+(pStr[index]&0x0f)*100+(pStr[index+1]>>4)*10+pStr[index+1]&0x0f;
                TempInt = Convert_BcdTOInt(pStr+index,2);
                printf("长度:%d \n",TempInt/2);
                index +=2;
                memcpy(ConsumeData.Userdefine60, pStr+index, TempInt/2);
                DebugPrintChar("自定义域60:",ConsumeData.Userdefine60,TempInt/2);
                index+=TempInt/2;
                break;

            case 64:
                memcpy(ConsumeData.Mac,pStr+index,8);
                index+=8;
                break;
            default:
                printf("-- err : 域%d 未处理\n",i+1);
            //    return -1;
                break;
            }
        }
    }

    return errcode;
}
#else
int HandelSendReplay_QR(char *RevBuf,int len)
{
    int RecvLen = 0, i=0, j=0;
    int index = 0, TempInt = 0;
    unsigned char pTempBuf[128], pBitMap[64];
    unsigned char RecvData[1024], TempChar;
    ShortUnon datalen;
    char temp;
    char type[2];
    
    struct UploadRecodContainerReply ReplyData;
    
    memset(&ReplyData, 0, sizeof(ReplyData));
    memset(pBitMap, 0, sizeof(pBitMap));
    memset(RecvData, 0, sizeof(RecvData));
    memset(pTempBuf, 0, sizeof(pTempBuf));


     
    char *pStr = RevBuf;
/*
    char pStr[] = {
    0x00 ,0x8A ,0x60 ,0x00 ,0x00 ,0x00 ,0x04 ,0x60   ,0x31 ,0x00 ,0x00 ,0x00 ,0x00 ,0x02 ,0x10 ,0x70   
    ,0x3E ,0x02 ,0x81 ,0x0A ,0xD0 ,0x80 ,0x12 ,0x19   ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00   
    ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x10   ,0x00 ,0x00 ,0x00 ,0x72 ,0x00 ,0x00 ,0x18 ,0x26   
    ,0x02 ,0x06 ,0x03 ,0x22 ,0x05 ,0x06 ,0x03 ,0x00   ,0x01 ,0x00 ,0x08 ,0x00 ,0x00 ,0x00 ,0x00 ,0x30   
    ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30   ,0x30 ,0x30 ,0x30 ,0x39 ,0x36 ,0x47 ,0x41 ,0x30   
    ,0x30 ,0x30 ,0x38 ,0x30 ,0x32 ,0x33 ,0x30 ,0x36   ,0x35 ,0x33 ,0x30 ,0x31 ,0x35 ,0x38 ,0x31 ,0x32   
    ,0x30 ,0x30 ,0x30 ,0x38 ,0x22 ,0x30 ,0x30 ,0x30   ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x20 ,0x20 ,0x20   
    ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30   ,0x20 ,0x20 ,0x20 ,0x31 ,0x35 ,0x36 ,0x00 ,0x14   
    ,0x22 ,0x00 ,0x00 ,0x24 ,0x00 ,0x05 ,0x00 ,0x00   ,0x03 ,0x43 ,0x50 ,0x55    
        };
        */
    DebugPrintChar("接收到平台返回值:",pStr,len);    

    datalen.i = 0;
    datalen.intbuf[0] = pStr[1];
    datalen.intbuf[1] = pStr[0];    
    if(datalen.i != (len-2))
    {
        printf("数据错误：长度不对！返回数据 : %d\n", RecvLen);
    }

    type[0]=pStr[13];
    type[1]=pStr[14];
    
    memcpy(pTempBuf,pStr+15,8);    //从数组中取位元表
    GetBitMap1(pTempBuf, pBitMap);
    DebugPrintChar("位元表内容:",pTempBuf,8);
    DebugPrintChar("转换后的内容:",pBitMap,64);

    // 2 受卡方系统跟踪号(POS终端交易流水) N19
    index = 23;
    if (pBitMap[1] == 1)
    {
        //  TempInt = *(pStr+23);
          TempInt=Convert_BcdTOInt(pStr+23,1);
          DebugPrintf("长度:%d\n",TempInt);
          index+=1;
         memcpy(ConsumeData.MainNo, pStr+index, TempInt/2);
         DebugPrintChar("主帐号:",ConsumeData.MainNo,TempInt/2);
         index+=(TempInt/2);
    }
    // 3交易处理码  N6
    if (pBitMap[2] == 1)
    {
         memcpy(ConsumeData.ProcCode, pStr+index, 3);
      DebugPrintChar("交易处理码:",ConsumeData.ProcCode,3);
      index+=3;
    }
    // 4交易金额    N12
    if (pBitMap[3] == 1)
    {
         memcpy(ConsumeData.MaxMoney, pStr+index, 6);
      DebugPrintChar("最大输入金额:",ConsumeData.MaxMoney,6);
      index+=6;
    }
    // 11受卡方系统跟踪号即终端交易流水号   N6
    if (pBitMap[10] == 1)
    {
         memcpy(ConsumeData.FlowNo, pStr+index, 3);
      DebugPrintChar("受卡方系统跟踪号:",ConsumeData.FlowNo,3);
      index+=3;
    }
    // 12受卡方所在地时间 N6
    if (pBitMap[11] == 1)
    {
         memcpy(ConsumeData.Time, pStr+index, 3);
      DebugPrintChar("受卡方时间:",ConsumeData.Time,3);
      index+=3;
    }    
    // 13受卡方所在地日期 N4
    if (pBitMap[12] == 1)
    {
         memcpy(ConsumeData.Date, pStr+index, 2);
      DebugPrintChar("受卡方日期:",ConsumeData.Date,2);
      index+=2;
    }    
    #if 0
    // 14卡有效期  N4
    if (pBitMap[13] == 1)
    {
         memcpy(ConsumeData.EffeTime, pStr+index, 2);
      DebugPrintChar("卡有效期:",ConsumeData.EffeTime,2);
      index+=2;
    }    
    #endif
    // 15平台清算日期 n2
    if (pBitMap[14] == 1)
    {
         memcpy(ConsumeData.BetTime, pStr+index, 2);
      DebugPrintChar("清算日期:",ConsumeData.BetTime,2);
      index+=2;
    }    
    #if 0
    // 23卡片序列号 n3
    if (pBitMap[22] == 1)
    {
         memcpy(ConsumeData.SeriCode, pStr+index, 2);
      DebugPrintChar("卡片序列号:",ConsumeData.SeriCode,2);
      index+=2;
    }    
    #endif
    // 25服务点条件码 N2
    if (pBitMap[24] == 1)
    {
      //   memcpy(ConsumeData.ConCode, pStr+57, 1);
            ConsumeData.ConCode = *(pStr+index);    
      printf("服务点条件码:%02x\n",*(pStr+index));
      index+=1;
    }    
    // 32受卡方标识 n..11
    if (pBitMap[31] == 1)
    {
         // TempInt = *(pStr+index);
          TempInt=Convert_BcdTOInt(pStr+index,1);
           DebugPrintf("长度:%d\n",TempInt);
          index+=1;
      memcpy(ConsumeData.strTCode, pStr+index, TempInt/2);      
      DebugPrintChar("受卡标识码:",ConsumeData.strTCode,TempInt/2);
      index+=(TempInt/2);
    }
    
    // 37检索参考码 An12
    if (pBitMap[36] == 1)
    {
          
      memcpy(ConsumeData.CheckNo, pStr+index, 12);
      DebugPrintChar("检索参考码:",ConsumeData.CheckNo,12);
      index+=12;
    }        
    // 39应答码   an2
    if (pBitMap[38] == 1)
    {
          
      memcpy(ConsumeData.ReplyCode, pStr+index, 2);
      DebugPrintChar("应答码:",ConsumeData.ReplyCode,2);
      //if((ConsumeData.ReplyCode[0]!=0x30)&&(ConsumeData.ReplyCode[1]!=0x30))
      //      return 1;
      //if((ConsumeData.ReplyCode[0]==0x41)&&(ConsumeData.ReplyCode[1]==0x30))
      //      return 1;
          if((ConsumeData.ReplyCode[0]==0x30)&&(ConsumeData.ReplyCode[1]==0x30))
                  return 0;
         if(type[0]==0x02 && type[1]==0x10) 
             if((ConsumeData.ReplyCode[0]==0x41)&&(ConsumeData.ReplyCode[1]==0x30))    
                  issign = 0;
      index+=2;
    }        

    // 40应答描述 n..11
    if (pBitMap[39] == 1)
    {
        //  TempInt = *(pStr+index);
          TempInt=Convert_BcdTOInt(pStr+index,1);
           DebugPrintf("长度:%d\n",TempInt);
          index+=1;
      memcpy(ConsumeData.Answerdes, pStr+index, TempInt);      
      DebugPrintChar("应答描述:",ConsumeData.Answerdes,TempInt);
      index+=(TempInt);
      
    }
    return 1;
    
    // 41终端代码 ans8
    if (pBitMap[40] == 1)
    {
          
      memcpy(ConsumeData.DeviceNo, pStr+index, 8);
      DebugPrintChar("终端代码:",ConsumeData.DeviceNo,8);
      index+=8;
    }    
    // 42商户代码 ans15
    if (pBitMap[41] == 1)
    {
          
      memcpy(ConsumeData.CostCode, pStr+index, 15);
      DebugPrintChar("商户代码:",ConsumeData.CostCode,15);
      index+=15;
    }    
    #if 0
    // 44收单机构 ans..25
    if (pBitMap[43] == 1)
    {
  
        temp = Convert_BcdTOInt(pStr+index, 1);
        index+=1;
      memcpy(ConsumeData.Reparea, pStr+index, temp);
      DebugPrintChar("收单机构:",ConsumeData.Reparea,temp);
      index+=temp;
    }
    #endif
    // 49交易货币代码 an3
    if (pBitMap[48] == 1)
    {

      memcpy(ConsumeData.Currencycode, pStr+index, 3);
      DebugPrintChar("交易货币代码:",ConsumeData.Currencycode,3);
      index+=3;
    }    
    #if 0
    // 55IC卡数据域 bcd...
    if(pBitMap[54]==1)
    {
        TempInt = Convert_BcdTOInt(pStr+index,2);
        index +=2;
         memcpy(ConsumeData.Carddata, pStr+index, TempInt);
         DebugPrintChar("IC卡数据域:",ConsumeData.Carddata,TempInt);
         index+=TempInt/2;
        }
    
    // 60自定义域  n...017
    if(pBitMap[59] == 1)
    {
        TempInt = Convert_BcdTOInt(pStr+index,2);
        index +=2;
         memcpy(ConsumeData.Userdefine60, pStr+index, TempInt/2);
         DebugPrintChar("自定义域60:",ConsumeData.Userdefine60,TempInt/2);
         index+=TempInt/2;
        }
    // 63自定义域  ans..063
    if(pBitMap[62] == 1)
    {
         TempInt = Convert_BcdTOInt(pStr+index,2);
         index +=2;
         memcpy(ConsumeData.Userdefine63, pStr+index, TempInt);
         DebugPrintChar("自定义域63:",ConsumeData.Userdefine63,TempInt);
         
       }
#endif

    
    return 1;
}
#endif

//处理黑名单下载的响应
int HandelBlacklist(char *RevBuf,int len)
{
    int RecvLen = 0, i=0, j=0;
    int index = 0, TempInt = 0;
    unsigned char pTempBuf[128], pBitMap[64];
    unsigned char RecvData[1024], TempChar;
    ShortUnon datalen;
    char temp;
    
    
    
    memset(pBitMap, 0, sizeof(pBitMap));
    memset(RecvData, 0, sizeof(RecvData));
    memset(pTempBuf, 0, sizeof(pTempBuf));


     
    char *pStr = RevBuf;

    DebugPrintChar("接收到平台返回值:",pStr,len);    

    datalen.i = 0;
    datalen.intbuf[0] = pStr[1];
    datalen.intbuf[1] = pStr[0];    
    if(datalen.i != (len-2))
    {
        printf("签到返回数据错误：长度不对！返回数据 : %d\n", RecvLen);
    }
    
    memcpy(pTempBuf,pStr+15,8);    //从数组中取位元表
    GetBitMap1(pTempBuf, pBitMap);
    DebugPrintChar("位元表内容:",pTempBuf,8);
    DebugPrintChar("转换后的内容:",pBitMap,64);
    
    index = 23;
   
    // 12受卡方所在地时间 N6
    if (pBitMap[11] == 1) 
    {
         memcpy(ConsumeData.Time, pStr+index, 3);
      DebugPrintChar("受卡方时间:",ConsumeData.Time,3);
      index+=3;
    }    
    // 13受卡方所在地日期 N4
    if (pBitMap[12] == 1)
    {
         memcpy(ConsumeData.Date, pStr+index, 2);
      DebugPrintChar("受卡方日期:",ConsumeData.Date,2);
      index+=2;
    }    
    #if 0
    // 14卡有效期  N4
    if (pBitMap[13] == 1)
    {
         memcpy(ConsumeData.EffeTime, pStr+index, 2);
      DebugPrintChar("卡有效期:",ConsumeData.EffeTime,2);
      index+=2;
    }    
    #endif
   
    
    // 37检索参考码 An12
    if (pBitMap[36] == 1)
    {
          
      memcpy(ConsumeData.CheckNo, pStr+index, 12);
      DebugPrintChar("检索参考码:",ConsumeData.CheckNo,12);
      index+=12;
    }        
    // 39应答码   an2
    if (pBitMap[38] == 1)
    {
          
      memcpy(ConsumeData.ReplyCode, pStr+index, 2);
      DebugPrintChar("应答码:",ConsumeData.ReplyCode,2);
      //if((ConsumeData.ReplyCode[0]!=0x30)&&(ConsumeData.ReplyCode[1]!=0x30))
      //      return 1;
      //if((ConsumeData.ReplyCode[0]==0x41)&&(ConsumeData.ReplyCode[1]==0x30))
      //      return 1;
          if((ConsumeData.ReplyCode[0]!=0x30)&&(ConsumeData.ReplyCode[1]!=0x30))        
              {
                
                return 1;
          }
      index+=2;
    }        

    // 40应答描述 n..11
    if (pBitMap[39] == 1)
    {
        //  TempInt = *(pStr+index);
          TempInt=Convert_BcdTOInt(pStr+index,1);
           DebugPrintf("长度:%d\n",TempInt);
          index+=1;
      memcpy(ConsumeData.Answerdes, pStr+index, TempInt);      
      DebugPrintChar("应答描述:",ConsumeData.Answerdes,TempInt);
      index+=(TempInt);
      
    }
    
    
    // 41终端代码 ans8
    if (pBitMap[40] == 1)
    {
          
      memcpy(ConsumeData.DeviceNo, pStr+index, 8);
      DebugPrintChar("终端代码:",ConsumeData.DeviceNo,8);
      index+=8;
    }    
    // 42商户代码 ans15
    if (pBitMap[41] == 1)
    {
          
      memcpy(ConsumeData.CostCode, pStr+index, 15);
      DebugPrintChar("商户代码:",ConsumeData.CostCode,15);
      index+=15;
    }    
    
    // 49交易货币代码 an3
    if (pBitMap[48] == 1)
    {

      memcpy(ConsumeData.Currencycode, pStr+index, 3);
      DebugPrintChar("交易货币代码:",ConsumeData.Currencycode,3);
      index+=3;
    }    
    
    // 60自定义域  n...017
    if(pBitMap[59] == 1)
    {
        TempInt = Convert_BcdTOInt(pStr+index,2);
        index +=2;
         if(TempInt%2)
            TempInt++;
         memcpy(ConsumeData.Userdefine60, pStr+index, TempInt/2);
         DebugPrintChar("自定义域60:",ConsumeData.Userdefine60,TempInt/2);
         index+=TempInt/2;
        }
    // 63自定义域  ans..063
    if(pBitMap[62] == 1)
    {
         TempInt = Convert_BcdTOInt(pStr+index,2);
         index +=2;
         memcpy(RecvData, pStr+index, TempInt);
         DebugPrintChar("黑名单信息:",pStr+index,TempInt);
        /*
        采用最新的方式保存黑名单
         */
         unsigned char  CsnAsc[10];
          //---------BlackItem item;
          int cardnum;
         if(RecvData[0]==2)        //表示有黑名单下载
         {
                cardnum = (TempInt-11)/33;
                for(i=0;i<cardnum;i++)
                {
                    memset(CsnAsc,0,sizeof(CsnAsc));
                    ASCToBCD(CsnAsc, RecvData[12+i*33+1], 20);
                    DebugPrintChar("转换后的卡号:", CsnAsc, 10);    
                    
                        //---------memcpy(item.dat, CsnAsc, sizeof(item));
                        //---------if((RecvData[12+i*33])==0x31)
                        //---------    update_sortfile(item, 0);
                        //---------else
                        //---------    update_sortfile(item, 1);
                }
                //---------SavetBlackListBuff();
            }
         else
         {
            printf("无黑名单下载或更新\n");
         }
    }
    return 0;
}





/* **********************************************
 * 函数名:int BuildUploadData_48(int *len,char *buf)
 * 函数功能: 组建48域数据包,上海银联
 * 参数: len : 组包的长度
 * 参数: buf : 组包的数据
 ************************************************/
int BuildUploadData_48(int *len,char *buf,char* stationid,int channel,int cmd ,void *pare,YLmain_Record *record)
{

    printf(" in func %s \n",__func__);
    unsigned char data[512]={0}, TempBuf[512]={0};
    int index = 0;
       char timebuf[7];
    int Tempint;
    
    timebuf[0]=0x20;
    Rd_time(timebuf+1);
        
    strcat(data,"PA");
    index+=2;
    strcat(data,"5709");
    index+=4;
    strcat(data,"0000");
    index+=4;
    strcat(data,"0001");
    index+=4;
    
    memcpy(data+index,"\x1F\x51",2);
    index+=2;
    data[index++]=0x02;
    memcpy(data+index,"24",2);
    index+=2;
    
    memcpy(data+index,"\x1F\x52",2);
    index+=2;
    data[index++]=0x02;
    //if(enableodaflag)
    if(channel==2)    
        memcpy(data+index,"01",2);
    else
        memcpy(data+index,"03",2);
    index+=2;
    
    
    
    //构造交易时间冲正的时候不要重组
    if(rechargeflag==0)
    memcpy(TagData.Tag_9A.buf,timebuf+1,3);

   //if(enableodaflag)
   if(channel==2)
    {
         memset(TempBuf, 0, sizeof(TempBuf));
     memcpy(TempBuf,"\xFF\x56",2);
    
    printf(" in LINE %d ,index %d \n",__LINE__,index);
    
    BuildUploadData_55(&Tempint, TempBuf+4);
    DebugPrintf("Tempint = %d\n", Tempint);
    
    *(TempBuf+2) = 0x81;
    *(TempBuf+3) = Tempint;    
    DebugPrintf("index = %d\n", index);
    memcpy(data+index, TempBuf, Tempint+4);
    
    memcpy(record->domain55,TempBuf+2,Tempint+2);
    index += (Tempint+4);
    }

    memcpy(data+index,"\xFF\x57",2);
    index+=2;
    data[index++]=strlen(stationid);
    memcpy(data+index,stationid,strlen(stationid));
    index+=strlen(stationid);

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\xFF\x58",2);    
    *(TempBuf+2)=14;
    
    Bcd_To_Asc(TempBuf+3,timebuf,14);
    memcpy(data+index,TempBuf,17);
    index+=17;

    


    /*所属公司*/
    if(strlen(mchantConf.CompanyInfo)>=1)
    {
        memcpy(data+index,"\xFF\x61",2);
        index+=2;
        data[index++]=strlen(mchantConf.CompanyInfo);
        memcpy(data+index,mchantConf.CompanyInfo,strlen(mchantConf.CompanyInfo));
        index+=strlen(mchantConf.CompanyInfo);
    }

    /*线路号*/
    if(strlen(mchantConf.LineNo)>=1)
    {
        memcpy(data+index,"\xFF\x62",2);
        index+=2;
        data[index++]=strlen(mchantConf.LineNo);
        memcpy(data+index,mchantConf.LineNo,strlen(mchantConf.LineNo));
        index+=strlen(mchantConf.LineNo);
    }

    /*车牌号*/
    if(strlen(mchantConf.LicensePlate)>=1)
    {
        memcpy(data+index,"\xFF\x63",2);
        index+=2;
        data[index++]=strlen(mchantConf.LicensePlate);
        memcpy(data+index,mchantConf.LicensePlate,strlen(mchantConf.LicensePlate));
        index+=strlen(mchantConf.LicensePlate);
    }
    /*司机号*/
    if(strlen(mchantConf.DriverId)>=1)
    {
        memcpy(data+index,"\xFF\x64",2);
        index+=2;
        data[index++]=strlen(mchantConf.DriverId);
        memcpy(data+index,mchantConf.DriverId,strlen(mchantConf.DriverId));
        index+=strlen(mchantConf.DriverId);
    }


    //if(enableodaflag)
    if(channel==2)
    {
        memset(TempBuf, 0, sizeof(TempBuf));
        memcpy(TempBuf,"\xFF\x43",2);    
        *(TempBuf+2)=8;
        
        //memcpy(TempBuf+3,"0000FSGJ",8);
        memcpy(TempBuf+3,mchantConf.institution,8);
        memcpy(data+index,TempBuf,11);
        index+=11;

        }

   if(cmd==0x11)
       {
        
        memset(TempBuf, 0, sizeof(TempBuf));
        memcpy(TempBuf,"\xFF\x65",2);    
        *(TempBuf+2)=1;
         *(TempBuf+3)=0x31;
         memcpy(data+index,TempBuf,4);
         index+=4;

        char asc[16];
        LongUnon * sid=(LongUnon *)pare;
        memset(TempBuf, 0, sizeof(TempBuf));
        memcpy(TempBuf,"\xFF\x66",2);    
        *(TempBuf+2)=6;
        sprintf(asc,"%06d",sid->i);
         memcpy(TempBuf+3,asc,6);
         memcpy(data+index,TempBuf,9);
         index+=9;            
       }



    data[index++]='#';

        
        
    *len = index;
    DebugPrintf("index = %d\n", index);
    memcpy(buf, data, *len);


#if 0
    /*将长度转换成bcd码*/
    sprintf(TempBuf,"%04d",index);
    asc_to_bcd(buf,TempBuf,4);
    printf(" 48 域长度 %s , %02x%02x \n",TempBuf,buf[0],buf[1]);
    memcpy(buf+2,data,index);
    *len=index+2;
#endif    
    return 0;
    
}





/* **********************************************
 * 函数名:int BuildUploadData_48(void)
 * 函数功能: 组建48域数据包
 * 参数: len : 组包的长度
 * 参数: buf : 组包的数据
 ************************************************/
int BuildUploadData_48_qr(int *len, char *buf,unsigned char mode)
{
    DebugPrintf("*len = %d\n", *len);

    
    unsigned char data[512], TempBuf[64];
    int index = 0;
    char timebuf[7];
    
    memset(data, 0, sizeof(data));   
    memcpy(data,"PA570900000001",14);
    index+=14;

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x1F\x51",2);    
    *(TempBuf+2)=2;
    memcpy(TempBuf + 3, "24", 2);

    memcpy(data+index,TempBuf,2+3);
    index += 5;

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x1F\x52",2);    
    *(TempBuf+2)=2;
    if(mode==2)
        memcpy(TempBuf + 3, "02", 2);              //01 ODA ,02    QR
    if(mode==1)
        memcpy(TempBuf + 3, "01", 2);              //01 ODA ,02    QR 03 传统POS
    if(mode==3)
        memcpy(TempBuf + 3, "03", 2);    
    memcpy(data+index,TempBuf,2+3);
    index += 5;

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\xFF\x57",2);    
    *(TempBuf+2)=1;
    memcpy(TempBuf + 3, "1", 2);

    memcpy(data+index,TempBuf,4);
    index += 4;

    
    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\xFF\x58",2);    
    *(TempBuf+2)=14;
    timebuf[0]=0x20;
    memcpy(timebuf+1,&Time.year,6);
    Bcd_To_Asc(TempBuf+3,timebuf,14);
    memcpy(data+index,TempBuf,17);
    index+=17;

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\xFF\x61",2);    
    *(TempBuf+2)=2;
    memcpy(TempBuf + 3, "11", 2);

    memcpy(data+index,TempBuf,5);
    index += 5;

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\xFF\x62",2);    
    *(TempBuf+2)=2;
    memcpy(TempBuf + 3, "22", 2);

    memcpy(data+index,TempBuf,5);
    index += 5;

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\xFF\x63",2);    
    *(TempBuf+2)=2;
    memcpy(TempBuf + 3, "33", 2);

    memcpy(data+index,TempBuf,5);
    index += 5;

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\xFF\x64",2);    
    *(TempBuf+2)=2;
    memcpy(TempBuf + 3, "44", 2);

    memcpy(data+index,TempBuf,5);
    index += 5;


    

    *(data+index)='#';
    index+=1;    
    
    *len = index;
    DebugPrintf("index = %d\n", index);
    memcpy(buf, data, *len);
    
    DebugPrintChar("data", data, *len);
    return 0;
}




/* **********************************************
 * 函数名:int BuildUploadData_55(void)
 * 函数功能: 组建55域数据包
 * 参数: len : 组包的长度
 * 参数: buf : 组包的数据
 ************************************************/
int BuildUploadData_55(int *len, char *buf)
{
    DebugPrintf("*len = %d\n", *len);

    
    unsigned char data[512], TempBuf[64];
    int index = 0;

    memset(data, 0, sizeof(data));
    
    memset(TempBuf, 0, sizeof(TempBuf));    

    memcpy(TempBuf,"\x9F\x26",2);
    *(TempBuf+2)=TagData.Tag_9F26.len;
    memcpy(TempBuf + 3, TagData.Tag_9F26.buf, TagData.Tag_9F26.len);

    memcpy(data,TempBuf,TagData.Tag_9F26.len + 3);
    index += TagData.Tag_9F26.len + 3;

    memset(TempBuf, 0, sizeof(TempBuf));

    memcpy(TempBuf,"\x9F\x27",2);
    *(TempBuf+2)=TagData.Tag_9F27.len;
    memcpy(TempBuf + 3, TagData.Tag_9F27.buf, TagData.Tag_9F27.len);

    memcpy(data+index,TempBuf,TagData.Tag_9F27.len+3);
    index += TagData.Tag_9F27.len + 3;


    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x9F\x10",2);
    *(TempBuf+2)=TagData.Tag_9F10.len;
    memcpy(TempBuf + 3, TagData.Tag_9F10.buf, TagData.Tag_9F10.len);
    memcpy(data+index,TempBuf,TagData.Tag_9F10.len+3);
    index += TagData.Tag_9F10.len + 3;

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x9F\x37",2);
    *(TempBuf+2)=TagData.Tag_9F37.len;
    memcpy(TempBuf + 3, TagData.Tag_9F37.buf, TagData.Tag_9F37.len);
    memcpy(data+index,TempBuf,TagData.Tag_9F37.len+3);
    index += TagData.Tag_9F37.len + 3;

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x9F\x36",2);
    *(TempBuf+2)=TagData.Tag_9F36.len;
    memcpy(TempBuf + 3, TagData.Tag_9F36.buf, TagData.Tag_9F36.len);
    memcpy(data+index,TempBuf,TagData.Tag_9F36.len+3);
    index += TagData.Tag_9F36.len + 3;

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x95",1);
    *(TempBuf+1)=TagData.Tag_95.len;
    memcpy(TempBuf + 2, TagData.Tag_95.buf, TagData.Tag_95.len);
    memcpy(data+index,TempBuf,TagData.Tag_95.len+2);
    index += TagData.Tag_95.len + 2;

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x9A",1);
    *(TempBuf+1)=TagData.Tag_9A.len;
    memcpy(TempBuf + 2, TagData.Tag_9A.buf, TagData.Tag_9A.len);
    memcpy(data+index,TempBuf,TagData.Tag_9A.len+2);
    index += TagData.Tag_9A.len + 2;

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x9C",1);
    *(TempBuf+1)=TagData.Tag_9C.len;
    memcpy(TempBuf + 2, TagData.Tag_9C.buf, TagData.Tag_9C.len);
    memcpy(data+index,TempBuf,TagData.Tag_9C.len+2);
    index += TagData.Tag_9C.len + 2;        

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x9F\x02",2);
    *(TempBuf+2)=TagData.Tag_9F02.len;
    memcpy(TempBuf + 3, TagData.Tag_9F02.buf, TagData.Tag_9F02.len);
    memcpy(data+index,TempBuf,TagData.Tag_9F02.len+3);
    index += TagData.Tag_9F02.len + 3;

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x5F\x2A",2);
    *(TempBuf+2)=TagData.Tag_5F2A.len;
    memcpy(TempBuf + 3, TagData.Tag_5F2A.buf, TagData.Tag_5F2A.len);
    memcpy(data+index,TempBuf,TagData.Tag_5F2A.len+3);
    index += TagData.Tag_5F2A.len + 3;


    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x82",1);
    *(TempBuf+1)=TagData.Tag_82.len;
    memcpy(TempBuf + 2, TagData.Tag_82.buf, TagData.Tag_82.len);
    memcpy(data+index,TempBuf,TagData.Tag_82.len+2);
    index += TagData.Tag_82.len + 2;    

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x9F\x1A",2);
    *(TempBuf+2)=TagData.Tag_9F1A.len;
    memcpy(TempBuf + 3, TagData.Tag_9F1A.buf, TagData.Tag_9F1A.len);
    memcpy(data+index,TempBuf,TagData.Tag_9F1A.len+3);
    index += TagData.Tag_9F1A.len + 3;

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x9F\x03",2);
    *(TempBuf+2)=TagData.Tag_9F03.len;
    memcpy(TempBuf + 3, TagData.Tag_9F03.buf, TagData.Tag_9F03.len);
    memcpy(data+index,TempBuf,TagData.Tag_9F03.len+3);
    index += TagData.Tag_9F03.len + 3;

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x9F\x33",2);
    *(TempBuf+2)=TagData.Tag_9F33.len;
    memcpy(TempBuf + 3, TagData.Tag_9F33.buf, TagData.Tag_9F33.len);
    memcpy(data+index,TempBuf,TagData.Tag_9F33.len+3);
    index += TagData.Tag_9F33.len + 3;

    
    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x9F\x34",2);
    *(TempBuf+2)=TagData.Tag_9F34.len;
    memcpy(TempBuf + 3, TagData.Tag_9F34.buf, TagData.Tag_9F34.len);
    memcpy(data+index,TempBuf,TagData.Tag_9F34.len+3);
    index += TagData.Tag_9F34.len + 3;        
    
    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x9F\x35",2);
    *(TempBuf+2)=TagData.Tag_9F35.len;
    memcpy(TempBuf + 3, TagData.Tag_9F35.buf, TagData.Tag_9F35.len);
    memcpy(data+index,TempBuf,TagData.Tag_9F35.len+3);
    index += TagData.Tag_9F35.len + 3;    

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x9F\x1E",2);
    *(TempBuf+2)=TagData.Tag_9F1E.len;
    memcpy(TempBuf + 3, TagData.Tag_9F1E.buf, TagData.Tag_9F1E.len);
    memcpy(data+index,TempBuf,TagData.Tag_9F1E.len+3);
    index += TagData.Tag_9F1E.len + 3;
    #if 0
    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x9F\x41",2);
    *(TempBuf+2)=TagData.Tag_9F41.len;
    memcpy(TempBuf + 3, TagData.Tag_9F41.buf, TagData.Tag_9F41.len);
    memcpy(data+index,TempBuf,TagData.Tag_9F41.len+3);
    index += TagData.Tag_9F41.len + 3;
    #endif
    *len = index;
    DebugPrintf("index = %d\n", index);
    memcpy(buf, data, *len);
    
    DebugPrintChar("data", data, *len);
    return 0;
}

/* **********************************************
 * 函数名:int BuildUploadData_571(void)
 * 函数功能: 组建55域数据包
 * 参数: len : 组包的长度
 * 参数: buf : 组包的数据
 ************************************************/
int BuildUploadData_571(int *len, char *buf)
{
    DebugPrintf("*len = %d\n", *len);

    
    unsigned char data[512], TempBuf[64];
    int index = 0;
    

    memset(data, 0, sizeof(data));

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\x1F\x51",2);   
    *(TempBuf+2)=3;
    memcpy(TempBuf+3,"ODA",3);    
    memcpy(data+index,TempBuf,6);
    index+=6;


    
    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\xFF\x55",2);    
    *(TempBuf+2)=G_QRuartInfo.length;
    memcpy(TempBuf + 3, G_QRuartInfo.id, G_QRuartInfo.length);

    memcpy(data+index,TempBuf,G_QRuartInfo.length+3);
    
    index += (G_QRuartInfo.length+3);

     memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\xFF\x57",2);    
    *(TempBuf+2)=16;
    TempBuf[3] = '0';
    memcpy(TempBuf + 4, basinfo.MerchantNum, 15);

    memcpy(data+index,TempBuf,19);
    
    index += 19;      

   memset(TempBuf, 0, sizeof(TempBuf));
   memcpy(TempBuf,"\xFF\x58",2);   
   *(TempBuf+2)=8;
  
   memcpy(TempBuf + 3, basinfo.TermNum, 8);
   
   memcpy(data+index,TempBuf,11);
   
   index += 11; 
    
   memset(TempBuf, 0, sizeof(TempBuf));
   memcpy(TempBuf,"\xFF\x61",2);   
   *(TempBuf+2)=1;
   TempBuf[3] = 0x03;   
   memcpy(data+index,TempBuf,4);
   index+=4;

   memset(TempBuf, 0, sizeof(TempBuf));
   memcpy(TempBuf,"\xFF\x42",2);   
   *(TempBuf+2)=4;
   memcpy(TempBuf+3,"FFFF",4);
   memcpy(data+index,TempBuf,7);
   index+=7;

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(TempBuf,"\xBF\x02",2);    
    *(TempBuf+2)=0x1A;
    memcpy(TempBuf+3,"11111111111111111111111111",26);
    memcpy(data+index,TempBuf,29);
    index+=29;


    

    *len = index;
    DebugPrintf("index = %d\n", index);
    memcpy(buf, data, *len);
    
    DebugPrintChar("data", data, *len);
    return 0;
}






/* **********************************************
 * 函数名:int BuildUploadData_57(void)
 * 函数功能: 组建55域数据包
 * 参数: len : 组包的长度
 * 参数: buf : 组包的数据
 ************************************************/
int BuildUploadData_57(int *len, char *buf)
{
    DebugPrintf("*len = %d\n", *len);

    
    unsigned char data[512], TempBuf[256],BcdBuf[16];
    int index = 0;
    int Tempint = 0;
    LongUnon lenth;
    memset(data, 0, sizeof(data));
    
    memcpy(data,"PB51A200000002",14);
    index +=14;

    DebugPrintChar("data",data,14);
    
    memset(TempBuf, 0x20, sizeof(TempBuf));
    memcpy(data+index,TempBuf,50);
    index+=50;

    DebugPrintChar("data",data,64);
    memcpy(data+index,"000000",6);
    index+=6;

    
    memset(TempBuf, 0, sizeof(TempBuf));
    BuildUploadData_571(&Tempint, TempBuf);
    DebugPrintf("Tempint = %d\n", Tempint);
    
    lenth.longbuf[1] = Tempint/100+'0';
    lenth.longbuf[2] = Tempint%100/10+'0';
    lenth.longbuf[3] = Tempint%100%10+'0';
    memcpy(data+index, &lenth.longbuf[1], 3);
    index += 3;
    DebugPrintf("index = %d\n", index);
    memcpy(data+index, TempBuf, Tempint);
    index += Tempint;

    
    *(data+index) = '#';
    index+=1;
    

    *len = index;
    DebugPrintf("index = %d\n", index);
    memcpy(buf, data, *len);
    
    DebugPrintChar("data", data, *len);
    return 0;
}

/* **********************************************
 * 函数名:int BuildUploadData_66(void)
 * 函数功能: 组建60域数据包
 * 参数: len : 组包的长度
 * 参数: buf : 组包的数据
 ************************************************/
int BuildUploadData_60(int *len, char *buf)
{
    unsigned char data[256], TempBuf[64];
    int index = 0;
    DebugPrintf("index = %d\n", index);
    //自定义包的长度
    data[index] = 0x00;
    data[index + 1] = 0x14;
    index += 2;

    DebugPrintf("index = %d\n", index);
    
    //60.1    交易类型码        2
    data[index] = 0x22;
    index += 1;

    //60.2    批次号
    if(mchantConf.worktype)
        memcpy(data+index, UploadData.BatchCode, 3);
    else
        memcpy(data+index, "\x30\x00\x01", 3);

    index += 3;

    //60.3    网络管理码
    data[index] = 0x00;
    data[index + 1] = 0x06;   //原来是0x05
    index += 2;

    //60.4    终端读取能力
    //60.5    基于PBOC借/贷记标准的IC卡条件代码
    data[index] = 0x01;
    index += 1;

    DebugPrintf("index = %d\n", index);

    *len = index;
    memcpy(buf, data, index);
    
    return 0;
}


/* **********************************************
 * 函数名:int BuildUploadData_66(void)
 * 函数功能: 组建60域数据包
 * 参数: len : 组包的长度
 * 参数: buf : 组包的数据
 ************************************************/
int BuildUploadData_60_qr(int *len, char *buf)
{
    unsigned char data[256], TempBuf[64];
    int index = 0;
    DebugPrintf("index = %d\n", index);
    //自定义包的长度
    data[index] = 0x00;
    data[index + 1] = 0x14;
    //data[index + 1] = 0x08;
    index += 2;

    DebugPrintf("index = %d\n", index);
    
    //60.1    交易类型码        2
    data[index] = 0x22;
    index += 1;

    //60.2    批次号
    if(mchantConf.worktype)
        memcpy(data+index, UploadData.BatchCode, 3);
    else
        memcpy(data+index, "\x30\x00\x01", 3);
    
    index += 3;
#if 1
    //60.3    网络管理码
    data[index] = 0x00;
    data[index + 1] = 0x06;   //原来是0x05
    index += 2;

    //60.4    终端读取能力
    //60.5    基于PBOC借/贷记标准的IC卡条件代码
    data[index] = 0x01;
    index += 1;
#endif
    DebugPrintf("index = %d\n", index);

    *len = index;
    memcpy(buf, data, index);
    
    return 0;
}


/* **********************************************
 * 函数名:int BuildUploadData_61(void)
 * 函数功能: 组建61域数据包
 * 参数: len : 组包的长度
 * 参数: buf : 组包的数据
 ************************************************/
int BuildUploadData_61(int *len, char *buf)
{
    unsigned char data[256], TempBuf[64];
    int index = 0;
    char BcdBuf[20];
    LongUnon tmp;
    DebugPrintf("index = %d\n", index);
    //自定义包的长度
//    data[index] = 0x00;
//    data[index + 1] = 0x08;
//    index += 2;
    data[index] = 0x08;
    index++;
    DebugPrintf("index = %d\n", index);
    


    //61.1    批次号
    //memcpy(data+index, UploadData.BatchCode, 3);
    memcpy(data+index, "\x30\x00\x01", 3);
    index += 3;

    //61.2    POS流水号    
    memset(BcdBuf,0,sizeof(BcdBuf));
    tmp.i = 0;
//    memcpy(tmp.longbuf,DevSerialID,4);
    memcpy(tmp.longbuf,DevSID.longbuf,4);
    Convert_IntTOBcd_2(tmp.i,BcdBuf);
    //ChangBigToEd(BcdBuf,BcdBuf);
    memcpy(data+index,BcdBuf+1,3);
    index += 3;

    //61.3 原始交易日期
    memcpy(data+index,ConsumeData.Date,2);
    index+=2;

    
    

    

    DebugPrintf("index = %d\n", index);

    *len = index;
    memcpy(buf, data, index);
    
    return 0;
}


int BuildUploadData_61_qr(int *len, char *buf)
{
    unsigned char data[256], TempBuf[64];
    int index = 0;
    char BcdBuf[20];
    LongUnon tmp;
    DebugPrintf("index = %d\n", index);
    //自定义包的长度
    data[index] = 0x00;
    data[index + 1] = 0x12;
    index += 2;
//    data[index] = 0x16;
//    index++;
    DebugPrintf("index = %d\n", index);
    


    //61.1    批次号
    if(mchantConf.worktype)
        memcpy(data+index, UploadData.BatchCode, 3);
    else
        memcpy(data+index, "\x30\x00\x01", 3);
    index += 3;

    //61.2    POS流水号    
    memset(BcdBuf,0,sizeof(BcdBuf));
    tmp.i = 0;
//    memcpy(tmp.longbuf,DevSerialID,4);
    memcpy(tmp.longbuf,DevSID.longbuf,4);
    Convert_IntTOBcd_2(tmp.i,BcdBuf);
    //ChangBigToEd(BcdBuf,BcdBuf);
    memcpy(data+index,BcdBuf+1,3);
    index += 3;

    //61.3 原始交易日期
//    memcpy(data+index,ConsumeData.Date,2);
//    index+=2;

    
    

    

    DebugPrintf("index = %d\n", index);

    *len = index;
    memcpy(buf, data, index);
    
    return 0;
}

/* **********************************************
 * 函数名:int BuildUploadData_591(void)        陕西信合
 * 函数功能: 组建59域数据包
 * 参数: len : 组包的长度
 * 参数: buf : 组包的数据
 ************************************************/
int BuildUploadData_591(int *len, char *buf)
{
    /* 9域数据结构【2B域长度+1B 字母"V"+2B机器号+14B程序版本+7B上笔交易时间 +15B机器号】 */
    unsigned char data[256], TempBuf[256],BcdBuf[16];
    int index = 0;
        int len1,len2,len3;
    DebugPrintf("index = %d\n", index);

    //自定义包的长度
    
        memcpy(data,"A2",2);
    index += 5;

       //tag 01        
    memcpy(data+index,"01",2);   
    index+=2;
    memcpy(data+index,"002",3);   
    index+=3;
    memcpy(data+index,"02",2);     
    index+=2;
    DebugPrintChar("5910 data:",data , index);

       //tag 02    
     memcpy(data+index,"02",2); 
    index+=2;
    memcpy(data+index,"023",3); 
    index+=3;
    memcpy(data+index,mchantConf.Terminaltype,6);    
    index+=6;    
    memcpy(data+index,"02",2);    
    index+=2;    
    memcpy(data+index,mchantConf.Terminalcsn,15);    
    index+=15;
        
    DebugPrintf("591 index=%d\n",index);     
    DebugPrintChar("5911 data:",data , index);

    //tag 03
     
     memcpy(data+index,"03",2); 
    index+=2;    
    memcpy(data+index,"006",3); 
    index+=3;
    memcpy(data+index, &G_QRuartInfo.id[G_QRuartInfo.length-6],6);
    index+=6;
    DebugPrintChar("5912 data:",data , index);

    //tag 04
     memcpy(data+index,"04",2); 
    index+=2;    
    memcpy(data+index,"008",3); 
    index+=3;    
    memcpy(data+index,"12345678",8);
    index+=8;
    DebugPrintChar("5913 data:",data , index);
    
    //tag 05
     memcpy(data+index,"05",2); 
    index+=2;    
    memcpy(data+index,"008",3); 
    index+=3;
    memcpy(data+index,"\x31\x32\x33\x34\x35\x36\x37\x38",8);
    index+=8;
    DebugPrintChar("5914 data:",data , index);
    
    
    data[2] = (index-5)/100+'0';
    data[3] = (index-5)%100/10+'0';
    data[4] = (index-5)%100%10+'0';
    
    memcpy(buf,data,index);
    *len = index;
    DebugPrintf("591 len=%d\n",*len);
    DebugPrintChar("591 data:",data , index);

    return 0;
}

/* **********************************************
 * 函数名:int BuildUploadData_59(void)        陕西信合
 * 函数功能: 组建59域数据包
 * 参数: len : 组包的长度
 * 参数: buf : 组包的数据
 ************************************************/
int BuildUploadData_59(int *len, char *buf)
{
    /* 9域数据结构【2B域长度+1B 字母"V"+2B机器号+14B程序版本+7B上笔交易时间 +15B机器号】 */
    unsigned char data[256], TempBuf[128],BcdBuf[16];
    int index = 0;
    int Tempint=0;
    LongUnon lenth;
    ShortUnon tmp;
    DebugPrintf("index = %d\n", index);
 
        //tag A2
         memset(TempBuf, 0, sizeof(TempBuf));
    BuildUploadData_591(&Tempint, TempBuf);
    DebugPrintf("Tempint = %d\n", Tempint);
    
        memcpy(data+index, TempBuf, Tempint);
    DebugPrintf("index = %d\n", index);
    memcpy(data+index, TempBuf, Tempint);
    index += Tempint;
    


    //tag A3
    memcpy(data+index,"A3",2);
    index+=2;
    BcdBuf[0]=0x30;
    lenth.i = G_QRuartInfo.length;
    
    BcdBuf[1] =G_QRuartInfo.length/10+'0';
    BcdBuf[2] = G_QRuartInfo.length%10+'0';
    memcpy(data+index, BcdBuf, 3);
    index+=3;
    memcpy(data+index, G_QRuartInfo.id, G_QRuartInfo.length);    
    index+=G_QRuartInfo.length;
    

  
        
    *len = index;
    memcpy(buf, data, index);
    DebugPrintf("59 len=%d\n",*len);
    DebugPrintChar("59 data:",data , index);
    return 0;
}

/* **********************************************
 * 函数名:int BuildUploadData_63(void)        
 * 函数功能: 组建59域数据包
 * 参数: len : 组包的长度
 * 参数: buf : 组包的数据
 ************************************************/
int BuildUploadData_63(int *len, char *buf)
{
    /* 
    63域数据结构
    【2B域长度+1B 字母"V"+2B机器号+14B程序版本+7B上笔交易时间 +15B机器号】 
    */
    unsigned char data[256], TempBuf[64];
    int index = 0;
    DebugPrintf("index = %d\n", index);

    //自定义包的长度
    data[index] =00;
    data[index + 1] = 0x26;
    index += 2;

    //机构号
    memcpy(&data[index],mchantConf.institution,8);
    index += 8;

    //上次更新日期时间 测试填默认值
    memcpy(&data[index],"00000000",8);
    index += 8;

    //上笔最后一个卡号
    memcpy(&data[index],"0000000000",10);
    index += 10;    
        
    *len = index;
    memcpy(buf, data, index);

    return 0;
}


/* **********************************************
 * 函数名:int GetUploadMac(int *len, char *buf)
 * 函数功能: 组建66域数据包
 * 参数: len : 组包的长度
 * 参数: buf : 组包的数据
 ************************************************/
 
int GetUploadMac(char *DataBuf, int DataLen)
{
    //char *pStrUploadData = "123456789123456789123456789jiaoyin123456789123456789123456789jiaoyin123456789123456789123456789jiaoyin";
//    char *pStrUploadData = "123456789123456789";
//    char *pStrKey = "cardlan";
//    int UploadDataLen = strlen(pStrUploadData);
    char *pStrUploadData = DataBuf;
    int UploadDataLen = DataLen;
    char TempBuf[128];
    int TempInt = 0, MabInt = 0, i, MabCount = 0;

    unsigned char pDataBuf[1024], TempMacBuf[9], ResultMabBuf[9], MacBuf_HEX[16];
    int pDataLen = 0;

    memset(TempBuf, 0, sizeof(TempBuf));
    memset(pDataBuf, 0, sizeof(pDataBuf));

    for(i=0; i<UploadDataLen; i++)
    {
        pDataBuf[i] = *(pStrUploadData+i);
        pDataLen++;
    }
    
//    DebugPrintf("pDataLen = %d\n",  pDataLen);
//    DebugPrintChar("pDataBuf", pDataBuf, pDataLen);
    /* 1.首先组建MAB，就是对8个字节取整 */
    //TempInt = UploadDataLen / 2;
    MabInt = 8 - (UploadDataLen % 8);
//    DebugPrintf("MabInt = %d pDataLen = %d\n", MabInt, pDataLen);
    if (MabInt < 8)
    {
        for (i = 0; i < MabInt; i++)
        {
            pDataBuf[pDataLen++] = 0x00;
        }
    }
    
//    DebugPrintf("pDataLen = %d\n",  pDataLen);
//    DebugPrintChar("pDataBuf", pDataBuf, pDataLen);
    /* 2.对MAB，按每8个字节做异或 */
    memset(ResultMabBuf, 0, sizeof(ResultMabBuf));
    memcpy(ResultMabBuf, pDataBuf, 8);
//    DebugPrintf("TempMacBuf = %s\n", ResultMabBuf);
//    DebugPrintChar("TempMacBuf", ResultMabBuf, 8);
    while(1)
    {
        MabCount++;
        memcpy(TempMacBuf, pDataBuf+8*MabCount, 8);
    //    DebugPrintf("TempMacBuf = %s\n", TempMacBuf);
    //    DebugPrintChar("TempMacBuf", TempMacBuf, 8);
        for(i=0; i<8; i++)
        {
            ResultMabBuf[i] = ResultMabBuf[i]^TempMacBuf[i];
        }
        
        if((8 * (MabCount+1)) >= pDataLen)
        {
            break;
        }
    //    DebugPrintChar("异或ResultMabBuf0000", ResultMabBuf, 8);
    }

//    DebugPrintChar("异或ResultMabBuf", ResultMabBuf, 8);

    
    /* 3.将异或运算后的最后8个字节（RESULT BLOCK）转换成16 个HEXDECIMAL */
    for (i = 0; i < 8; i++)
    {
        sprintf(MacBuf_HEX + 2*i, "%02X", (unsigned char)ResultMabBuf[i]);
    }

    /* 4.取前8个字节用MAK加密 */
    memcpy(TempMacBuf, MacBuf_HEX, 8);
    DebugPrintChar("PbocKey.DecryptKey", PbocKey.DecryptKey, 8);
    PbocDecrypt_DES(TempMacBuf, 8, ResultMabBuf, &MabInt, PbocKey.DecryptKey);
    DebugPrintChar("MAK加密ResultMabBuf = \n", ResultMabBuf,8);

    /* 5.将加密后的结果与后8个字节异或 */
    memcpy(TempMacBuf, MacBuf_HEX+8, 8);
    for(i=0; i<8; i++)
    {
        ResultMabBuf[i] = ResultMabBuf[i]^TempMacBuf[i];
    }

    /* 6.用异或的结果TEMP BLOCK再进行一次单倍长密钥算法运算 */
    PbocDecrypt_DES(ResultMabBuf, 8, TempMacBuf, &MabInt, PbocKey.DecryptKey);
    DebugPrintChar("单倍长加密TempMacBuf = \n", TempMacBuf,8);

    /* 7. 将运算后的结果（ENC BLOCK2）转换成16个HEXDECIMAL   */
    for (i = 0; i < 8; i++)
    {
        sprintf(MacBuf_HEX + 2*i, "%02X", (unsigned char)TempMacBuf[i]);
    }

    memcpy(UploadData.UploadMac, MacBuf_HEX, 8);
//    DebugPrintf("UploadData.UploadMac = %s\n", UploadData.UploadMac);
    DebugPrintChar("UploadData.UploadMac",UploadData.UploadMac, 8);
    return 0;
}


/*
    cbc算法
*/
int GetUploadMac_yinlian(char *DataBuf, int DataLen)
{
    //char *pStrUploadData = "123456789123456789123456789jiaoyin123456789123456789123456789jiaoyin123456789123456789123456789jiaoyin";
//    char *pStrUploadData = "123456789123456789";
//    char *pStrKey = "cardlan";
//    int UploadDataLen = strlen(pStrUploadData);
    char *pStrUploadData = DataBuf;
    int UploadDataLen = DataLen;
    char TempBuf[128];
    int TempInt = 0, MabInt = 0, i, MabCount = 0;

    unsigned char pDataBuf[1024], TempMacBuf[9], ResultMabBuf[9], MacBuf_HEX[16],startbuf[8];
    int pDataLen = 0;

    memset(TempBuf, 0, sizeof(TempBuf));
    memset(pDataBuf, 0, sizeof(pDataBuf));
    memset(startbuf,0,sizeof(startbuf));

    for(i=0; i<UploadDataLen; i++)
    {
        pDataBuf[i] = *(pStrUploadData+i);
        pDataLen++;
    }
    
    DebugPrintf("UploadDataLen = %d\n",  UploadDataLen);
    DebugPrintChar("pDataBuf", pDataBuf, pDataLen);
    /* 1.首先组建MAB，就是对8个字节取整 */
    //TempInt = UploadDataLen / 2;
    MabInt = 8 - (UploadDataLen % 8);
    //DebugPrintf("MabInt = %d pDataLen = %d\n", MabInt, pDataLen);
    if (MabInt < 8)
    {
        for (i = 0; i < MabInt; i++)
        {
            pDataBuf[pDataLen++] = 0x00;
        }
    }
    
//    DebugPrintf("pDataLen = %d\n",  pDataLen);
//    DebugPrintChar("pDataBuf", pDataBuf, pDataLen);
    /* 2.对MAB，按每8个字节做DES运算*/
    memset(ResultMabBuf, 0, sizeof(ResultMabBuf));
    memcpy(ResultMabBuf, pDataBuf, 8);
    

//    DebugPrintChar("PbocKey.DecryptKey", PbocKey.DecryptKey, 8);
//    DebugPrintChar("ResultMabBuf", ResultMabBuf, 8);
    PbocDecrypt_DES(ResultMabBuf, 8, startbuf, &MabInt, PbocKey.DecryptKey);

//    DebugPrintChar("MAK加密ResultMabBuf = \n", startbuf,8);
    
    
//    DebugPrintf("TempMacBuf = %s\n", ResultMabBuf);
//    DebugPrintChar("TempMacBuf", ResultMabBuf, 8);
    while(1)
    {
        MabCount++;
        memcpy(TempMacBuf, pDataBuf+8*MabCount, 8);
    //    DebugPrintf("TempMacBuf = %s\n", TempMacBuf);
    //    DebugPrintChar("异或前TempMacBuf", TempMacBuf, 8);
        for(i=0; i<8; i++)
        {
            ResultMabBuf[i] = startbuf[i]^TempMacBuf[i];
        }
    //    DebugPrintChar("异或后ResultMabBuf", ResultMabBuf, 8);
    //    DebugPrintChar("PbocKey.DecryptKey", PbocKey.DecryptKey, 8);
        PbocDecrypt_DES(ResultMabBuf, 8, startbuf, &MabInt, PbocKey.DecryptKey);

    //    DebugPrintChar("MAK加密ResultMabBuf = \n", startbuf,8);

        
        if((8 * (MabCount+1)) >= pDataLen)
        {
            break;
        }

    }


    

    memcpy(UploadData.UploadMac, startbuf, 8);
//    DebugPrintf("UploadData.UploadMac = %s\n", UploadData.UploadMac);
    DebugPrintChar("UploadData.UploadMac",UploadData.UploadMac, 8);
    return 0;
}



static void Hex2Decimal(const unsigned char *pInData, unsigned char cInLen, unsigned char *pOutData)
{
    unsigned char i;
    unsigned char Ptr = 0x00;

    for(i = 0; i < cInLen; i++)
    {
        unsigned char u8Temp;
        
        u8Temp = pInData[i] / 0x10;

        if(u8Temp <= 9)
        {
            pOutData[Ptr++] = u8Temp + '0';
        }
        else
        {
            pOutData[Ptr++] = u8Temp - 0x0a + 'A';
        }

        u8Temp = pInData[i] % 0x10;

        if(u8Temp <= 9)
        {
            pOutData[Ptr++] = u8Temp + '0';
        }
        else
        {
            pOutData[Ptr++] = u8Temp - 0x0a + 'A';
        }
    }
}


/*
    ecb算法算MAC
*/

void QPBOC_CalcMac(const unsigned char *pMacKey, 
                            const unsigned char *pInData, 
                            unsigned short cInLen, 
                            unsigned char *pOutMAC)
{
    unsigned char i;
    unsigned char j;
    unsigned char cXorBuf[8];
    unsigned char cDesBuf[8];
    unsigned char cHexBuf[16];
    unsigned char cGroup ;        //组数
    unsigned char cRes;         //余数

    //first: init
    memset(cXorBuf, 0x00, 8);
    
    cGroup = (cInLen / 8);
    
    cRes = (cInLen % 8);
    
    if(cRes == 0) //刚好8的倍数
    {
        for(i = 0; i < cGroup; i ++)        //RESULT BLOCK
        {
            for (j = 0; j < 8; j++)
            {
                cXorBuf[j] ^= pInData[(i<<3)+j];
            }
        }
    }
    else //不是8 的倍数
    {
        unsigned char BufEnd[8];            //报文最后补全的8BYTE 数据
        
        memcpy(BufEnd, &pInData[cGroup *8 ], cRes);
        
        memset(&BufEnd[cRes], 0, 8-cRes);
        
        for(i = 0; i < cGroup; i ++)        //RESULT BLOCK
        {
            for (j = 0; j < 8; j++)
            {
                cXorBuf[j] ^= pInData[(i<<3)+j];
            }
        }
        
        //最后一组计算
        for (j = 0; j < 8; j++)
        {
            cXorBuf[j] ^= BufEnd[j];
        }
    }

    
    des_context ctx;                        //des加密算法函数使用到的上下文
    //8byte的HEX转换成16byte字符串
    Hex2Decimal(cXorBuf, 0x08, cHexBuf);

    //用工作密钥加密
    //PbocEncrypt_DES(cHexBuf, 8,cDesBuf, &i, pMacKey);
    //dec_ecb_ec(cHexBuf, cDesBuf, 1, pMacKey);
    des_set_key(&ctx, pMacKey);
    des_encrypt(&ctx, cHexBuf, cDesBuf);
    
    memcpy(cXorBuf, cDesBuf, 8);
    
    //再次xor
    for(j = 0; j < 8; j++)
    {
        cXorBuf[j] ^= cHexBuf[8 + j] ;
    }    

    //用工作密钥加密
    //PbocEncrypt_DES(cXorBuf, 8, cDesBuf, &i, pMacKey);
    //dec_ecb_ec(cXorBuf, cDesBuf, 1, pMacKey);
    des_set_key(&ctx, pMacKey);
    des_encrypt(&ctx, cXorBuf, cDesBuf);
    //8byte的HEX转换成16byte字符串
    Hex2Decimal(cDesBuf, 0x08, cHexBuf);

    //取前面8个字节为mac
    memcpy(pOutMAC, cHexBuf, 0x08);  

    
}






int ParserRecordToGetTag(void)
{
    int index = 0, datalen = 0;
    char RecordBuf[256];

    memset(RecordBuf, 0, sizeof(RecordBuf));
    memcpy(RecordBuf,emvrecord,sizeof(emvrecord));
    DebugPrintChar("RecordBuf", RecordBuf, sizeof(RecordBuf));
    LOGI("func = %s::emvrecordlen=%d\n",__func__,emvrecordlen);

    index+=2;    //跳过最开始的2字节

    memset(&TagData,0,sizeof(TagData));
    
    while(index < emvrecordlen)
    {
        /* 取出来的记录的最后面是有0x00的，这部分要过滤掉 */
        //printf("index=%d ,  recordbuf[%d]=%02x   \n",index,RecordBuf[index]);
        
        if(RecordBuf[index] == 0x00)        
        {
            index++;
            continue;
        }
        
        if((RecordBuf[index] & 0x1F) == 0x1F)
        {
            datalen = RecordBuf[index+2];
            switch(RecordBuf[index])
            {
                case 0x9f:
                {
                    switch(RecordBuf[index+1])
                    {
                        case 0x26:
                            
                            memcpy(TagData.Tag_9F26.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F26.len = datalen;
                            index += datalen;
                            
                            DebugPrintChar("Tag_9F26", TagData.Tag_9F26.buf, TagData.Tag_9F26.len);
                            break;
                        case 0x27:
#if 0
                            memcpy(TagData.Tag_9F27.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F27.len = datalen;
                            index += datalen;
#else
                            memcpy(TagData.Tag_9F27.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F27.len = datalen;
                            index += datalen;
#endif                            
                            DebugPrintChar("Tag_9F27", TagData.Tag_9F27.buf, TagData.Tag_9F27.len);
                            break;
                        case 0x10:
                            
                            memcpy(TagData.Tag_9F10.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F10.len = datalen;
                            index += datalen;
                                
                            DebugPrintChar("Tag_9F10", TagData.Tag_9F10.buf, TagData.Tag_9F10.len);
                            break;
                        
                        case 0x36:
                            memcpy(TagData.Tag_9F36.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F36.len = datalen;
                            index += datalen;
                            DebugPrintChar("Tag_9F36", TagData.Tag_9F36.buf, TagData.Tag_9F36.len);
                            break;

                        case 0x37:            //add by wxy,后面会被Terminfo替换
                            memcpy(TagData.Tag_9F37.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F37.len = datalen;
                            index += datalen;
                            DebugPrintChar("Tag_9F37", TagData.Tag_9F37.buf, TagData.Tag_9F37.len);
                            break;
                            
                        case 0x33:    //add by wxy
                            memcpy(TagData.Tag_9F33.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F33.len = datalen;
                            index += datalen;
                            DebugPrintChar("Tag_9F33", TagData.Tag_9F33.buf, TagData.Tag_9F33.len);
                            break;
                        
                        case 0x34:
                            memcpy(TagData.Tag_9F34.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F34.len = datalen;
                            index += datalen;    
                            DebugPrintChar("Tag_9F34", TagData.Tag_9F34.buf, TagData.Tag_9F34.len);
                            break;
                        case 0x35:
                            memcpy(TagData.Tag_9F35.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F35.len = datalen;
                            index += datalen;
                            DebugPrintChar("Tag_9F35", TagData.Tag_9F35.buf, TagData.Tag_9F35.len);
                            break;
                        case 0x1E:
                            memcpy(TagData.Tag_9F1E.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F1E.len = datalen;
                            index += datalen;        
                            DebugPrintChar("Tag_9F1E", TagData.Tag_9F1E.buf, TagData.Tag_9F1E.len);
                            break;
                        case 0x09:
                            memcpy(TagData.Tag_9F09.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F09.len = datalen;
                            index += datalen;
                            DebugPrintChar("Tag_9F09", TagData.Tag_9F09.buf, TagData.Tag_9F09.len);
                            break;
                        case 0x41:
                            memcpy(TagData.Tag_9F41.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F41.len = datalen;
                            index += datalen;    
                            DebugPrintChar("Tag_9F41", TagData.Tag_9F41.buf, TagData.Tag_9F41.len);
                            break;
                            
                        case 0x74:
                            memcpy(TagData.Tag_9F74.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F74.len = datalen;
                            index += datalen;
                            DebugPrintChar("Tag_9F74", TagData.Tag_9F74.buf, TagData.Tag_9F74.len);
                            break;
                            
                        case 0x02:    //add by wxy
                            memcpy(TagData.Tag_9F02.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F02.len = datalen;
                            index += datalen;
                            DebugPrintChar("Tag_9F02", TagData.Tag_9F02.buf, TagData.Tag_9F02.len);
                            break;
                            
                        case 0x03:    //add by wxy
                            memcpy(TagData.Tag_9F03.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F03.len = datalen;
                            index += datalen;
                            DebugPrintChar("Tag_9F03", TagData.Tag_9F03.buf, TagData.Tag_9F03.len);
                            break;

                        case 0x1A: //add by wxy
                            memcpy(TagData.Tag_9F1A.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F1A.len = datalen;
                            index += datalen;
                            DebugPrintChar("Tag_9F1A", TagData.Tag_9F1A.buf, TagData.Tag_9F1A.len);
                            break;
                            
                        default:
                            printf("TERMAPP_EncodeTLV(): Unknown tag: %02X%02X\n", RecordBuf[index], RecordBuf[index + 1]);
                            index+=datalen;
                            break;
                    }
                }
                break;
                case 0x5f:
                {
                    switch(RecordBuf[index+1])
                    {
                        case 0x24:
                            memcpy(TagData.Tag_5F24.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_5F24.len = datalen;
                            index += datalen;
                            DebugPrintChar("Tag_5F24", TagData.Tag_5F24.buf, TagData.Tag_5F24.len);
                            break;

                        case 0x34:
                            memcpy(TagData.Tag_5F34.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_5F34.len = datalen;
                            index += datalen;
                            DebugPrintChar("Tag_5F34", TagData.Tag_5F34.buf, TagData.Tag_5F34.len);
                            break;
                        case 0x2a:    //add by wxy
                            memcpy(TagData.Tag_5F2A.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_5F2A.len = datalen;
                            index += datalen;
                            DebugPrintChar("Tag_5F2A", TagData.Tag_5F2A.buf, TagData.Tag_5F2A.len);
                            break;
                        default:
                            printf("TERMAPP_EncodeTLV(): Unknown tag: %02X%02X\n", RecordBuf[index], RecordBuf[index + 1]);
                            index+=datalen;
                            break;
                    }
                }
                break;
            }

            index += 3; /* 标签+长度标志 */
        }
        else
        {
            datalen = RecordBuf[index+1];            
            switch(RecordBuf[index])
            {
                case 0x57:
                    memcpy(TagData.Tag_57.buf, RecordBuf+index+2, datalen);
                    TagData.Tag_57.len = datalen;
                    index += datalen;
                    DebugPrintChar("Tag_57", TagData.Tag_57.buf, TagData.Tag_57.len);
                    break;

                case 0x82:
                    memcpy(TagData.Tag_82.buf, RecordBuf+index+2, datalen);
                    TagData.Tag_82.len = datalen;
                    index += datalen;
                    DebugPrintChar("Tag_82", TagData.Tag_82.buf, TagData.Tag_82.len);
                    break;
                case 0x84:
                    memcpy(TagData.Tag_84.buf, RecordBuf+index+2, datalen);
                    TagData.Tag_84.len = datalen;
                    index += datalen;
                    DebugPrintChar("Tag_84", TagData.Tag_84.buf, TagData.Tag_84.len);
                    break;
                case 0x8A:
                    memcpy(TagData.Tag_8A.buf, RecordBuf+index+2, datalen);
                    TagData.Tag_8A.len = datalen;
                    index += datalen;
                    DebugPrintChar("Tag_8A", TagData.Tag_8A.buf, TagData.Tag_8A.len);
                    break;
                case 0x5A:
                    memcpy(TagData.Tag_5A.buf, RecordBuf+index+2, datalen);
                    TagData.Tag_5A.len = datalen;
                    index += datalen;
                    DebugPrintChar("Tag_5A", TagData.Tag_5A.buf, TagData.Tag_5A.len);
                    break;
                    
                case 0x95:    //add by wxy,后面会被替换
                    memcpy(TagData.Tag_95.buf, RecordBuf+index+2, datalen);
                    TagData.Tag_95.len = datalen;
                    index += datalen;
                    DebugPrintChar("Tag_95", TagData.Tag_95.buf, TagData.Tag_95.len);
                    break;
                    
                case 0x9a:    //add by wxy
                    memcpy(TagData.Tag_9A.buf, RecordBuf+index+2, datalen);
                    TagData.Tag_9A.len = datalen;
                    index += datalen;
                    DebugPrintChar("Tag_9A", TagData.Tag_9A.buf, TagData.Tag_9A.len);
                    break;

                case 0x9c: //add by wxy
                    memcpy(TagData.Tag_9C.buf, RecordBuf+index+2, datalen);
                    TagData.Tag_9C.len = datalen;
                    index += datalen;
                    DebugPrintChar("Tag_9C", TagData.Tag_9C.buf, TagData.Tag_9C.len);
                    break;
                default:
                    printf("TERMAPP_EncodeTLV(): Unknown tag: %02X\n", RecordBuf[index]);
                    index+=datalen;
                    break;
            }
            index += 2; 
        }

    }

    if(TagData.Tag_9F37.len==0)
    {    
        memcpy(TagData.Tag_9F37.buf, TermInfo.UnpredictNum, 4);
        TagData.Tag_9F37.len = 4;
        DebugPrintChar("Tag_9F37", TagData.Tag_9F37.buf, TagData.Tag_9F37.len);
    }

    
    if(TagData.Tag_95.len==0)
    {
        memcpy(TagData.Tag_95.buf, TermInfo.TVR, 5);
        TagData.Tag_95.len = 5;        
        DebugPrintChar("Tag_95", TagData.Tag_95.buf, TagData.Tag_95.len);
    }

    if(TagData.Tag_9A.len==0)
    {
        memcpy(TagData.Tag_9A.buf, TermInfo.TransDate, 3);
        TagData.Tag_9A.len = 3;        
        DebugPrintChar("Tag_9A", TagData.Tag_9A.buf, TagData.Tag_9A.len);
    }
        /*
        memcpy(TagData.Tag_9C.buf, TermInfo.TransTime, 3);
        TagData.Tag_9C.len = 3;        
        DebugPrintChar("Tag_9C", TagData.Tag_9C.buf, TagData.Tag_9C.len);
*/
    if(TagData.Tag_9C.len==0)
    {
        memcpy(TagData.Tag_9C.buf, "\x00", 1);
        TagData.Tag_9C.len = 1;        
        DebugPrintChar("Tag_9C", TagData.Tag_9C.buf, TagData.Tag_9C.len);
    }

    if(TagData.Tag_9F02.len==0)
        {
        memcpy(TagData.Tag_9F02.buf, TermInfo.AmtAuthNum, 6);
        TagData.Tag_9F02.len = 6;                                        
        DebugPrintChar("Tag_9F02", TagData.Tag_9F02.buf, TagData.Tag_9F02.len);
        }

    if(TagData.Tag_5F2A.len==0)
        {
        memcpy(TagData.Tag_5F2A.buf, TermInfo.TransCurcyCode, 2);
        TagData.Tag_5F2A.len = 2;                                
        DebugPrintChar("Tag_5F2A", TagData.Tag_5F2A.buf, TagData.Tag_5F2A.len);
        }

    if(TagData.Tag_9F1A.len==0)
        {
        memcpy(TagData.Tag_9F1A.buf, TermInfo.CountryCode, 2);
        TagData.Tag_9F1A.len = 2;                                
        DebugPrintChar("Tag_9F1A", TagData.Tag_9F1A.buf, TagData.Tag_9F1A.len);
        }

    if(TagData.Tag_9F03.len==0)
        {
        memcpy(TagData.Tag_9F03.buf, "\x00\x00\x00\x00\x00\x00", 6);
        TagData.Tag_9F03.len = 6;                                                                
        DebugPrintChar("Tag_9F03", TagData.Tag_9F03.buf, TagData.Tag_9F03.len);
        }

    if(TagData.Tag_9F33.len==0)
        {
        memcpy(TagData.Tag_9F33.buf, TermInfo.TermCapab, 3);
        TagData.Tag_9F33.len = 3;                                                                                        
        DebugPrintChar("Tag_9F33", TagData.Tag_9F33.buf, TagData.Tag_9F33.len);
        }

    if(TagData.Tag_9F34.len==0)
        {
        memcpy(TagData.Tag_9F34.buf, "\x00\x00\x00", 3);
        TagData.Tag_9F34.len = 3;                                                                                        
        DebugPrintChar("Tag_9F34", TagData.Tag_9F34.buf, TagData.Tag_9F34.len);
        }


    return 0;
}


/*
    农商行的双免消费报文:
*/
int BuildComsumeData_Nongye(unsigned char *data)
{
    
    unsigned char bitmap[8], TempBuf[256], BcdBuf[16];
    int index = 0, istart = 0, bitMapPos = 0;
    int Tempint = 0;
    char tpdubuf[5];
    LongUnon tmp;
    char tmp1;
    memset(data, 0, sizeof(data));
    memset(BcdBuf, 0, sizeof(BcdBuf));
    memset(bitmap, 0, sizeof(bitmap));
    memset(tpdubuf,0,sizeof(tpdubuf));


    ParserRecordToGetTag();
    index+=2;                //预留2个字节用于数据长度
    
    memcpy(data+index,basinfo.TPDU,5);
    index += 5;

    memcpy(data+index,basinfo.HEAD,6);
    index+=6;

    istart = index;         //正真报文开始的地方
    //消息类型        12
    data[index] = 0x02;
    data[index + 1] = 0x00;
    index += 2;
    
    bitMapPos = index;
    index += 8;                 //这里是存放位元表的        20
    
    SetBitMap(bitmap, 2);
    data[index] = 0x19;         
    index += 1;
    //将57标签的前面一直到D停止的数据拷贝到这里去，这个就是银行卡号     21
    //57 13  62 30 65 00 00 60 00 01 72 1D 25 12 22 00 00 00  16 10 0F 
    //    memcpy(TagData.Tag_57.buf,"\x62\x30\x65\x00\x00\x60\x00\x01\x72\x1D",10);
    //    TagData.Tag_57.len=10;
    
    tmp1 = TagData.Tag_57.buf[9];
    TagData.Tag_57.buf[9] = TagData.Tag_57.buf[9]&0xf0;
    DebugPrintChar("Tag_57.buf",TagData.Tag_57.buf,10);
    memcpy(data+index, TagData.Tag_57.buf, 10);
    TagData.Tag_57.buf[9]=tmp1;
    index += 10;

    //03 交易处理码     31
    SetBitMap(bitmap, 3);
    data[index] = 0x00;
    data[index + 1] = 0x00;
    data[index + 2] = 0x00;
    index += 3;

    //04    交易金额    34    
    SetBitMap(bitmap, 4);
    memcpy(data+index,"\x00\x00",2);
    Convert_IntTOBcd_2(DecValue.i, BcdBuf);
    printf("在线交易金额:%d\n",DecValue.i);
    index+=2;
    memcpy(data+index,BcdBuf,4);
    index += 4;

    //11    受卡方系统跟踪号    40
    SetBitMap(bitmap, 11);
    memset(BcdBuf,0,sizeof(BcdBuf));
    tmp.i = 0;
    memcpy(tmp.longbuf,DevSID.longbuf,4);
    Convert_IntTOBcd_2(tmp.i,BcdBuf);
    memcpy(data+index,BcdBuf+1,3);
    index += 3;

    //14    卡有效期    43
    SetBitMap(bitmap, 14);
    memcpy(data+index,&CardLan.Effective[1],2);
    index += 2;

    //卡标志 03:IC卡；00：磁条卡；01：存折
    SetBitMap(bitmap,20);
    data[index]=0x30;
    data[index+1]=0x33; 
    index+=2;

    //22     服务点输入方式码    45
    SetBitMap( bitmap, 22);
    data[index] = 0x02 ;         //wsl20160702
    data[index + 1] = 0x20;     //wsl20160702
    index += 2;

    //23    卡片序列号        47
    SetBitMap(bitmap, 23);
    data[index] = 0x00;
    data[index + 1] = 0x01; 
    index += 2;

    //25    服务点条件码    49
    SetBitMap( bitmap, 25);
    data[index] = 0x00;
    index += 1;

    //35    二磁道数据
    SetBitMap( bitmap, 35);
    data[index] = 0x36;
    index += 1;
    memcpy(data+index,TagData.Tag_57.buf,TagData.Tag_57.len);
    index+=TagData.Tag_57.len;
    
#if 0

#else
    SetBitMap( bitmap, 41); 
    //memcpy(data+index, card_driver.TermNum, 8);
    memcpy(data+index, basinfo.TermNum, 8);
    index += 8;

    //42    受卡方标识码    58
    SetBitMap( bitmap, 42); 
    //memcpy(data+index, card_driver.MerchantNum, 15);
    memcpy(data+index, basinfo.MerchantNum, 15);
    index += 15;    

#endif

    //49 交易货币代码
    SetBitMap( bitmap, 49);
    //UploadData.MoneyType = {0x31,0x35,0x36};
    memcpy(UploadData.MoneyType,"\x31\x35\x36",3);
    memcpy(data+index, UploadData.MoneyType, 3);
    index += 3;
    

    //55    IC卡数据域,包含多个子域,自定义长度        
    /* Tempint 后面取出来之后需要转换成BCD码表示 */
    SetBitMap( bitmap, 55);
    memset(TempBuf, 0, sizeof(TempBuf));
    BuildUploadData_55(&Tempint, TempBuf);
    DebugPrintf("Tempint = %d\n", Tempint);
    Convert_IntTOBcd(Tempint, BcdBuf);
    DebugPrintf("index = %d\n", index);
    memcpy(data+index, BcdBuf, 2);
    index += 2;
    DebugPrintf("index = %d\n", index);
    memcpy(data+index, TempBuf, Tempint);
    index += Tempint;
    
//    DebugPrintChar("TagData", &TagData, 10);

    //59    这里是处理客户定制的功能，这里是客户定制的功能
/*
    SetBitMap( bitmap, 59);
    memset(TempBuf, 0, sizeof(TempBuf));
    BuildUploadData_59(&Tempint, TempBuf);
    memcpy(data+index, TempBuf, Tempint);
    index += Tempint;
*/
    //60    自定义域    
    SetBitMap( bitmap, 60);
    memset(TempBuf, 0, sizeof(TempBuf));
    BuildUploadData_60(&Tempint, TempBuf);
    memcpy(data+index, TempBuf, Tempint);
    index += Tempint;

#if 0
    //63    自定义域
    SetBitMap(bitmap, 63);
    data[index] = 0x00;
    data[index + 1] = 0x03;
    index += 2;
    memcpy(UploadData.CreditCompanyCode,"CPU",3);
    memcpy(data+index, UploadData.CreditCompanyCode, 3);
    index += 3;
#endif

    //64    通过秘钥计算得到的mac
    
    SetBitMap(bitmap, 64);
    //把位图表的信息赋值进来
    memcpy(data+bitMapPos, bitmap, 8);
    memcpy(PbocKey.BaseKey,basinfo.MasterKey,32);            //实际只有16字节
    memcpy(PbocKey.DecryptKey,basinfo.Workingkey,16);        //最多只用到16字节，单倍长或者双倍长
    GetUploadMac(data+13, index-istart);            //计算mac,从消息类型开始计算
    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(data+index, UploadData.UploadMac, 8);
    index += 8;
    
    data[0] = 0x00;
    data[1] = index-2;
    
    DebugPrintChar("data", data, index);
    
    return 0;
}




/*
    银联标准的双免消费报文:
    cmd:  0x11 表示再请款
    pare: 根据cmd的类型不同，pare为不同数据
*/
int BuildConsumeData(unsigned char *data,int channle,int cmd,void * pare,YLmain_Record * record)
{
     //unsigned char data[1024];
    unsigned char bitmap[8], TempBuf[512], BcdBuf[16];
    int index = 0, istart = 0, bitMapPos = 0;
    int Tempint = 0;
    char tpdubuf[5];
    LongUnon tmp;
    char tmp1;
    memset(data, 0, sizeof(data));
    memset(BcdBuf, 0, sizeof(BcdBuf));
    memset(bitmap, 0, sizeof(bitmap));
//    memset(&TagData, 0, sizeof(struct PbocTagContainer));
    memset(tpdubuf,0,sizeof(tpdubuf));


    ParserRecordToGetTag();
    index+=2;                //预留2个字节用于数据长度

    if(channle==2)
    {
    memcpy(data+index,basinfo.ODATPDU,5);
    index += 5;
    memcpy(data+index,basinfo.ODAHEAD,6);
    index+=6;
    }
    else
    {
    memcpy(data+index,basinfo.TPDU,5);
    index += 5;
    memcpy(data+index,basinfo.HEAD,6);
    index+=6;
    }

    istart = index;            //正真报文开始的地方
    //消息类型        12
    data[index] = 0x02;
    data[index + 1] = 0x00;
    index += 2;
    
    bitMapPos = index;
    index += 8;                    //这里是存放位元表的        20


    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap(bitmap, 2);
    //data[index] = 0x19;            
    //index += 1;
    //将57标签的前面一直到D停止的数据拷贝到这里去，这个就是银行卡号        21
    //57 13  62 30 65 00 00 60 00 01 72 1D 25 12 22 00 00 00  16 10 0F 
    //    memcpy(TagData.Tag_57.buf,"\x62\x30\x65\x00\x00\x60\x00\x01\x72\x1D",10);
    //    TagData.Tag_57.len=10;
    #if 1            //这里应该动态的获取卡号，卡号可能是16位 18位 也可能是19位的，最多是19位
    int i,find=9;    //默认为19位卡号    
    for(i=0;i<10;i++)
    {
        if((TagData.Tag_57.buf[i]&0xf0)==0xD0 || (TagData.Tag_57.buf[i]&0x0f)==0x0D)
        find=i;
    }
    tmp1=TagData.Tag_57.buf[find];
    
    if((tmp1&0x0f)==0x0D)       //19位卡号
    {
        char ascbuff[16]={0};
        char bcdbuff[16]={0};
        sprintf(ascbuff,"%02d",find*2+1);
        printf("读取卡号,长度%s \n",ascbuff);
        asc_to_bcd(bcdbuff,ascbuff,2);
        data[index]=bcdbuff[0];
        index++;
        TagData.Tag_57.buf[find] = TagData.Tag_57.buf[find]&0xf0;
        DebugPrintChar("Tag_57.buf",TagData.Tag_57.buf,find+1);    
        memcpy(data+index, TagData.Tag_57.buf, find+1);
        memset(record->AcountNum,0,sizeof(record->AcountNum));
        Bcd_To_Asc(record->AcountNum,TagData.Tag_57.buf,(find)*2);
        char ch=(TagData.Tag_57.buf[find]>>4)+'0';
        record->AcountNum[(find)*2]=ch;
        record->AcountNum[(find)*2+1]='\0';
        index+=(find+1);
        TagData.Tag_57.buf[find] = tmp1;
    }
    else
    {
        char ascbuff[16]={0};
        char bcdbuff[16]={0};
        sprintf(ascbuff,"%02d",find*2);
        asc_to_bcd(bcdbuff,ascbuff,2);
        data[index]=bcdbuff[0];
        index++;
        DebugPrintChar("Tag_57.buf",TagData.Tag_57.buf,find);    
        memcpy(data+index, TagData.Tag_57.buf, find);
        memset(record->AcountNum,0,sizeof(record->AcountNum));
        Bcd_To_Asc(record->AcountNum,TagData.Tag_57.buf,find*2);
        index+=(find);    
    }
    #else
    tmp1 = TagData.Tag_57.buf[9];
    TagData.Tag_57.buf[9] = TagData.Tag_57.buf[9]&0xf0;
    DebugPrintChar("Tag_57.buf",TagData.Tag_57.buf,10);
    memcpy(data+index, TagData.Tag_57.buf, 10);
    index += 10;
    #endif


    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    //03 交易处理码         ,这里不同的客户会有差异 31
    printf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap(bitmap, 3);
    if(mchantConf.worktype)
    {
        data[index] = 0x00;
        data[index + 1] = 0x00;
        data[index + 2] = 0x00;
       }
    else
        memcpy(data+index,mchantConf.TransProcessCode,3);

    index += 3;

    //04    交易金额    34    
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap(bitmap, 4);
    memcpy(data+index,"\x00\x00",2);
    Convert_IntTOBcd_2(HostValue.i, BcdBuf);
    printf("在线交易金额:%d\n",HostValue.i);
    index+=2;
    //ChangBigToEd(BcdBuf,BcdBuf);
    memcpy(data+index,BcdBuf,4);
    index += 4;

    //11    受卡方系统跟踪号    40
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap(bitmap, 11);
    memset(BcdBuf,0,sizeof(BcdBuf));
    tmp.i = 0;
//    memcpy(tmp.longbuf,DevSerialID,4);
    memcpy(tmp.longbuf,DevSID.longbuf,4);
    Convert_IntTOBcd_2(tmp.i,BcdBuf);
    //ChangBigToEd(BcdBuf,BcdBuf);
    memcpy(data+index,BcdBuf+1,3);
    index += 3;

    
#if 0
    //14     卡有效期    43
    printf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap(bitmap, 14);
    //UploadData.AvailableData={0x16,0x05};
//    memcpy(UploadData.AvailableData,"\x16\x05",2);
//    memcpy(data+index, UploadData.AvailableData, 2);
    memcpy(data+index,&CardLan.Effective[1],2);
    index += 2;
#endif


    //22      服务点输入方式码    各个客户也会有差异 ，写成配置文件 45
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap( bitmap, 22);
#if 1    
    memcpy(data+index,mchantConf.InputCode,2);
#else    
    data[index] = 0x02 ;         //wsl20160702
    data[index + 1] = 0x20;     //wsl20160702
#endif
    index += 2;

    //23    卡片序列号        47
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap(bitmap, 23);
    //data[index] = 0x00;
    data[index] = 00;
    data[index + 1] = TagData.Tag_5F34.buf[0];
    index += 2;

    //25     服务点条件码,客户不同，有配置文件决定    49  
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap( bitmap, 25);
    if(mchantConf.worktype)
        data[index] = 0x00;
    else
        data[index]=mchantConf.ServeConditionCode;

    index += 1;

    //35     二磁道数据
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap( bitmap, 35);
    //data[index] = 0x36;
    //bcd的位数,bcd码标识长度，最长位36位即18个字节
    {
        char ascbuff[8]={0};
        char bcdbuff[8]={0};
        
        for(i=0;i<TagData.Tag_57.len;i++)
        {
        //if((TagData.Tag_57.buf[i]&0xf0)==0xf0 || (TagData.Tag_57.buf[i]&0x0f)==0x0f)
        //        TagData.Tag_57.buf[i]=0;
        if((TagData.Tag_57.buf[i]&0xf0)==0xf0 )
            TagData.Tag_57.buf[i]=0;
        if((TagData.Tag_57.buf[i]&0x0f)==0x0f)
            TagData.Tag_57.buf[i]=TagData.Tag_57.buf[i]&0xf0;
        }
        
        sprintf(ascbuff,"%02d",TagData.Tag_57.len*2-1);
        
        
        asc_to_bcd(bcdbuff,ascbuff,2);
        data[index]=bcdbuff[0];
    }
    index += 1;
    memcpy(data+index,TagData.Tag_57.buf,TagData.Tag_57.len);
    index+=TagData.Tag_57.len;
#if 0
    //41    受卡机终端标识码    50
    SetBitMap( bitmap, 41);
//    memcpy(UploadData.AcceptDevID,"GA000802",strlen("GA000802"));
    memcpy(UploadData.AcceptDevID,"GA005901",strlen("GA005901"));
//    memcpy(data+index, UploadData.AcceptDevID, 8);
    memcpy(data+index,card_driver.TermNum,8);

    index += 8;

    //42    受卡方标识码    58
    SetBitMap( bitmap, 42);
//    memcpy(UploadData.AcceptOrganizationID,"306530158120008",strlen("306530158120008"));
    memcpy(UploadData.AcceptOrganizationID,"306530156910059",strlen("306530156910059"));

//    memcpy(data+index, UploadData.AcceptOrganizationID, 15);
    memcpy(data+index,card_driver.MerchantNum,15);

    index += 15;
#else
    SetBitMap( bitmap, 41);    
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    //memcpy(data+index, card_driver.TermNum, 8);
    memcpy(data+index, basinfo.TermNum, 8);
    index += 8;

    //42    受卡方标识码    58
    SetBitMap( bitmap, 42);    
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    //memcpy(data+index, card_driver.MerchantNum, 15);
    memcpy(data+index, basinfo.MerchantNum, 15);
    index += 15;    

#endif

    if(!mchantConf.worktype)
    {
        //48 域 ，这里很特殊，行业自定义数据
        SetBitMap(bitmap, 48);
        
        DebugPrintf(" in LINE %d ,index11 %d \n",__LINE__,index);
        memset(TempBuf, 0, sizeof(TempBuf));
        BuildUploadData_48(&Tempint,TempBuf,"1",channle,cmd,pare,record);
        //memcpy(data+index,TempBuf,Tempint);
        //index+=Tempint;
        DebugPrintf("Tempint = %d\n", Tempint);
        Convert_IntTOBcd(Tempint, BcdBuf);
        DebugPrintf("index22 = %d\n", index);
        memcpy(data+index, BcdBuf, 2);
        index += 2;
        DebugPrintf("index33 = %d\n", index);
        memcpy(data+index, TempBuf, Tempint);
        index += Tempint;
        }

    

    //49 交易货币代码
    SetBitMap( bitmap, 49);
    DebugPrintf(" in LINE %d ,index44 %d \n",__LINE__,index);
    memcpy(UploadData.MoneyType,"\x31\x35\x36",3);
    memcpy(data+index, UploadData.MoneyType, 3);
    index += 3;
    /*
      //53 安全控制信息
    SetBitMap( bitmap, 53);
    DebugPrintf(" in LINE %d ,index44 %d \n",__LINE__,index);
    //memcpy(UploadData.MoneyType,"\x31\x35\x36",3);
    memcpy(data+index, "\x06\x10\x0\x0\x0\x0\x0\x0", 8);
    index += 8;  
    */

    
    //55    IC卡数据域,包含多个子域,自定义长度        
    /* Tempint 后面取出来之后需要转换成BCD码表示 */
    //if(enableodaflag==0)
    if(channle!=2)
    {
    printf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap( bitmap, 55);
    memset(TempBuf, 0, sizeof(TempBuf));
    BuildUploadData_55(&Tempint, TempBuf);
    DebugPrintf("Tempint = %d\n", Tempint);
    Convert_IntTOBcd(Tempint, BcdBuf);
    DebugPrintf("index = %d\n", index);
    memcpy(data+index, BcdBuf, 2);
    index += 2;
    DebugPrintf("index = %d\n", index);
    memcpy(data+index, TempBuf, Tempint);
    memcpy(record->domain55,BcdBuf,2);
    memcpy(record->domain55+2,TempBuf,Tempint);
    index += Tempint;
    }
//    DebugPrintChar("TagData", &TagData, 10);

    //59    这里是处理客户定制的功能，这里是客户定制的功能
/*
    SetBitMap( bitmap, 59);
    memset(TempBuf, 0, sizeof(TempBuf));
    BuildUploadData_59(&Tempint, TempBuf);
    memcpy(data+index, TempBuf, Tempint);
    index += Tempint;
*/
    //if(enableodaflag==0)

/*
    if(channle!=2)
    {
    SetBitMap(bitmap, 59);    
    *(data+index) = 0x00;
    index++;
    *(data+index) = 0x05;
    index++;
    memcpy(data+index, "A30000", 5);
    index += 5;    
        }
*/
    //60     自定义域    
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap( bitmap, 60);
    memset(TempBuf, 0, sizeof(TempBuf));
    BuildUploadData_60(&Tempint, TempBuf);
    memcpy(data+index, TempBuf, Tempint);
    memcpy(record->BatchNum,TempBuf+3,3);
    index += Tempint;

    
    //63    自定义域
    
    if((channle==2)||(mchantConf.worktype))
    {
    SetBitMap(bitmap, 63);
    data[index] = 0x00;
    data[index + 1] = 0x03;
    index += 2;
//    memcpy(UploadData.CreditCompanyCode,"\x00\x00\x00",3);
    memcpy(UploadData.CreditCompanyCode,"CPU",3);
    memcpy(data+index, UploadData.CreditCompanyCode, 3);
    index += 3;
        }

//    DebugPrintf("index = %d  strlen(data) = %d\n", index, strlen(data));

    //64     通过秘钥计算得到的mac
    
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap(bitmap, 64);
    //把位图表的信息赋值进来
    memcpy(data+bitMapPos, bitmap, 8);
    memcpy(PbocKey.BaseKey,basinfo.MasterKey,32);            //实际只有16字节
    memcpy(PbocKey.DecryptKey,basinfo.Workingkey,16);        //最多只用到16字节，单倍长或者双倍长

    if(mchantConf.worktype)
        GetUploadMac(data+13, index-istart);    
    else
        GetUploadMac_yinlian(data+13, index-istart);
        //QPBOC_CalcMac(PbocKey.DecryptKey,data+13,index-istart,UploadData.UploadMac);
    
    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(data+index, UploadData.UploadMac, 8);
    index += 8;

    ShortUnon len;
    len.i = index;
    len.i -=2;
    data[0]  = len.intbuf[1];
    data[1]  = len.intbuf[0];
    DebugPrintf("当前的长度index=%d\n",index);
    
    DebugPrintChar("data", data, index);
    
    return 0;
}

//刷卡消费的冲正,channel 1 双免 ， 2 ODA
int BuildConsumeData_CZ(unsigned char *data,int channel,YLmain_Record *record)
{
     //unsigned char data[1024];
    unsigned char bitmap[8], TempBuf[512], BcdBuf[16];
    int index = 0, istart = 0, bitMapPos = 0;
    int Tempint = 0;
    char tpdubuf[5];
    LongUnon tmp;
    char tmp1;
    memset(data, 0, sizeof(data));
    memset(BcdBuf, 0, sizeof(BcdBuf));
    memset(bitmap, 0, sizeof(bitmap));
//    memset(&TagData, 0, sizeof(struct PbocTagContainer));
    memset(tpdubuf,0,sizeof(tpdubuf));
    int i;

//    ParserRecordToGetTag();
    index+=2;                //预留2个字节用于数据长度

    memcpy(data+index,basinfo.TPDU,5);


    index += 5;


    memcpy(data+index,basinfo.HEAD,6);
    index+=6;


    istart = index;            //正真报文开始的地方
    //消息类型        12
    data[index] = 0x04;
    data[index + 1] = 0x00;
    index += 2;
    
    bitMapPos = index;
    index += 8;                    //这里是存放位元表的        20


    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    

    if(mchantConf.worktype)
    {
       SetBitMap(bitmap, 2);
    //data[index] = 0x19;            
    //index += 1;
    //将57标签的前面一直到D停止的数据拷贝到这里去，这个就是银行卡号        21
    //57 13  62 30 65 00 00 60 00 01 72 1D 25 12 22 00 00 00  16 10 0F 
    //    memcpy(TagData.Tag_57.buf,"\x62\x30\x65\x00\x00\x60\x00\x01\x72\x1D",10);
    //    TagData.Tag_57.len=10;
        #if 1            //这里应该动态的获取卡号，卡号可能是16位 18位 也可能是19位的，最多是19位
        int i,find=9;    //默认为19位卡号    
        for(i=0;i<10;i++)
        {
            if((TagData.Tag_57.buf[i]&0xf0)==0xD0 || (TagData.Tag_57.buf[i]&0x0f)==0x0D)
            find=i;
        }
        tmp1=TagData.Tag_57.buf[find];
        
        if((tmp1&0x0f)==0x0D)       //19位卡号
        {
            char ascbuff[16]={0};
            char bcdbuff[16]={0};
            sprintf(ascbuff,"%02d",find*2+1);
            printf("读取卡号,长度%s \n",ascbuff);
            asc_to_bcd(bcdbuff,ascbuff,2);
            data[index]=bcdbuff[0];
            index++;
            TagData.Tag_57.buf[find] = TagData.Tag_57.buf[find]&0xf0;
            DebugPrintChar("Tag_57.buf",TagData.Tag_57.buf,find+1);    
            memcpy(data+index, TagData.Tag_57.buf, find+1);
            memset(record->AcountNum,0,sizeof(record->AcountNum));
            Bcd_To_Asc(record->AcountNum,TagData.Tag_57.buf,(find)*2);
            char ch=(TagData.Tag_57.buf[find]>>4)+'0';
            record->AcountNum[(find)*2]=ch;
            record->AcountNum[(find)*2+1]='\0';
            index+=(find+1);
            TagData.Tag_57.buf[find] = tmp1;
        }
        else
        {
            char ascbuff[16]={0};
            char bcdbuff[16]={0};
            sprintf(ascbuff,"%02d",find*2);
            asc_to_bcd(bcdbuff,ascbuff,2);
            data[index]=bcdbuff[0];
            index++;
            DebugPrintChar("Tag_57.buf",TagData.Tag_57.buf,find);    
            memcpy(data+index, TagData.Tag_57.buf, find);
            memset(record->AcountNum,0,sizeof(record->AcountNum));
            Bcd_To_Asc(record->AcountNum,TagData.Tag_57.buf,find*2);
            index+=(find);    
        }
        #else
        tmp1 = TagData.Tag_57.buf[9];
        TagData.Tag_57.buf[9] = TagData.Tag_57.buf[9]&0xf0;
        DebugPrintChar("Tag_57.buf",TagData.Tag_57.buf,10);
        memcpy(data+index, TagData.Tag_57.buf, 10);
        index += 10;
        #endif
        }

    
    //03 交易处理码         ,这里不同的客户会有差异 31
    printf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap(bitmap, 3);
    if(mchantConf.worktype)
      {  
        data[index] = 0x00;
        data[index + 1] = 0x00;
        data[index + 2] = 0x00;
        }
    else
        memcpy(data+index,mchantConf.TransProcessCode,3);

    index += 3;

    //04    交易金额    34    
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap(bitmap, 4);
    memcpy(data+index,"\x00\x00",2);
    Convert_IntTOBcd_2(DecValue.i, BcdBuf);
    printf("在线交易金额:%d\n",DecValue.i);
    index+=2;
    //ChangBigToEd(BcdBuf,BcdBuf);
    memcpy(data+index,BcdBuf,4);
    index += 4;

    //11    受卡方系统跟踪号    40
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap(bitmap, 11);
    memset(BcdBuf,0,sizeof(BcdBuf));
    tmp.i = 0;
//    memcpy(tmp.longbuf,DevSerialID,4);
    memcpy(tmp.longbuf,DevSID.longbuf,4);
    Convert_IntTOBcd_2(tmp.i,BcdBuf);
    //ChangBigToEd(BcdBuf,BcdBuf);
    memcpy(data+index,BcdBuf+1,3);
    index += 3;
#if 0
    //14     卡有效期    43
    printf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap(bitmap, 14);
    //UploadData.AvailableData={0x16,0x05};
//    memcpy(UploadData.AvailableData,"\x16\x05",2);
//    memcpy(data+index, UploadData.AvailableData, 2);
    memcpy(data+index,&CardLan.Effective[1],2);
    index += 2;
#endif


    //22      服务点输入方式码    各个客户也会有差异 ，写成配置文件 45
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap( bitmap, 22);
#if 1    
    memcpy(data+index,mchantConf.InputCode,2);
#else    
    data[index] = 0x02 ;         //wsl20160702
    data[index + 1] = 0x20;     //wsl20160702
#endif
    index += 2;

    //23    卡片序列号        47
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap(bitmap, 23);
    //data[index] = 0x00;
    data[index] = 00;
    data[index + 1] = TagData.Tag_5F34.buf[0];
    index += 2;
    

    //25     服务点条件码,客户不同，有配置文件决定    49  
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap( bitmap, 25);
    if(mchantConf.worktype)    
        data[index] = 0x00;
    else
        data[index]=mchantConf.ServeConditionCode;

    index += 1;

    //35     二磁道数据
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap( bitmap, 35);
    //data[index] = 0x36;
    //bcd的位数,bcd码标识长度，最长位36位即18个字节
    {
        char ascbuff[8]={0};
        char bcdbuff[8]={0};
        for(i=0;i<TagData.Tag_57.len;i++)
        {
        if((TagData.Tag_57.buf[i]&0xf0)==0xf0 || (TagData.Tag_57.buf[i]&0x0f)==0x0f)
                TagData.Tag_57.buf[i]=0;
        }
        
        sprintf(ascbuff,"%02d",TagData.Tag_57.len*2-1);
        asc_to_bcd(bcdbuff,ascbuff,2);
        data[index]=bcdbuff[0];
    }
    index += 1;
    memcpy(data+index,TagData.Tag_57.buf,TagData.Tag_57.len);
    index+=TagData.Tag_57.len;


    SetBitMap( bitmap, 39);    
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    //memcpy(data+index, card_driver.TermNum, 8);
    memcpy(data+index, "98", 2);
    index += 2;    



    
#if 0
    //41    受卡机终端标识码    50
    SetBitMap( bitmap, 41);
//    memcpy(UploadData.AcceptDevID,"GA000802",strlen("GA000802"));
    memcpy(UploadData.AcceptDevID,"GA005901",strlen("GA005901"));
//    memcpy(data+index, UploadData.AcceptDevID, 8);
    memcpy(data+index,card_driver.TermNum,8);

    index += 8;

    //42    受卡方标识码    58
    SetBitMap( bitmap, 42);
//    memcpy(UploadData.AcceptOrganizationID,"306530158120008",strlen("306530158120008"));
    memcpy(UploadData.AcceptOrganizationID,"306530156910059",strlen("306530156910059"));

//    memcpy(data+index, UploadData.AcceptOrganizationID, 15);
    memcpy(data+index,card_driver.MerchantNum,15);

    index += 15;
#else
    SetBitMap( bitmap, 41);    
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    //memcpy(data+index, card_driver.TermNum, 8);
    memcpy(data+index, basinfo.TermNum, 8);
    index += 8;

    //42    受卡方标识码    58
    SetBitMap( bitmap, 42);    
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    //memcpy(data+index, card_driver.MerchantNum, 15);
    memcpy(data+index, basinfo.MerchantNum, 15);
    index += 15;    

#endif

    if(!mchantConf.worktype)
    {
        //48 域 ，这里很特殊，行业自定义数据
        SetBitMap(bitmap, 48);
        
        DebugPrintf(" in LINE %d ,index11 %d \n",__LINE__,index);
        memset(TempBuf, 0, sizeof(TempBuf));
        BuildUploadData_48(&Tempint,TempBuf,"1",channel,0,NULL,record);
        //memcpy(data+index,TempBuf,Tempint);
        //index+=Tempint;
        DebugPrintf("Tempint = %d\n", Tempint);
        Convert_IntTOBcd(Tempint, BcdBuf);
        DebugPrintf("index22 = %d\n", index);
        memcpy(data+index, BcdBuf, 2);
        index += 2;
        DebugPrintf("index33 = %d\n", index);
        memcpy(data+index, TempBuf, Tempint);
        index += Tempint;
        }
    //49 交易货币代码
    SetBitMap( bitmap, 49);
    DebugPrintf(" in LINE %d ,index44 %d \n",__LINE__,index);
    memcpy(UploadData.MoneyType,"\x31\x35\x36",3);
    memcpy(data+index, UploadData.MoneyType, 3);
    index += 3;
    
    
    //55    IC卡数据域,包含多个子域,自定义长度        
    /* Tempint 后面取出来之后需要转换成BCD码表示 */
    
    //if(enableodaflag==0)
    if(channel!=2)
    {
    printf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap( bitmap, 55);
    memset(TempBuf, 0, sizeof(TempBuf));
    BuildUploadData_55(&Tempint, TempBuf);
    DebugPrintf("Tempint = %d\n", Tempint);
    Convert_IntTOBcd(Tempint, BcdBuf);
    DebugPrintf("index = %d\n", index);
    memcpy(data+index, BcdBuf, 2);
    index += 2;
    DebugPrintf("index = %d\n", index);
    memcpy(data+index, TempBuf, Tempint);
    index += Tempint;
    }
//    DebugPrintChar("TagData", &TagData, 10);

    //59    这里是处理客户定制的功能，这里是客户定制的功能
#if 0
    if(channel!=2)
    {
        SetBitMap(bitmap, 59);    
        *(data+index) = 0x00;
        index++;
        *(data+index) = 0x05;
        index++;
        memcpy(data+index, "A30000", 5);
        index += 5;    
    }
#endif

    //60     自定义域    
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap( bitmap, 60);
    memset(TempBuf, 0, sizeof(TempBuf));
    BuildUploadData_60(&Tempint, TempBuf);
    memcpy(data+index, TempBuf, Tempint);
    index += Tempint;

#if 0    
    //61    自定义域    
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap( bitmap, 61);
    memset(TempBuf, 0, sizeof(TempBuf));
    BuildUploadData_61(&Tempint, TempBuf);
    memcpy(data+index, TempBuf, Tempint);
    index += Tempint;
#endif

    
    //63    自定义域    
    if(channel==2)
    {
    SetBitMap(bitmap, 63);
    data[index] = 0x00;
    data[index + 1] = 0x03;
    index += 2;
//    memcpy(UploadData.CreditCompanyCode,"\x00\x00\x00",3);
    memcpy(UploadData.CreditCompanyCode,"CPU",3);
    memcpy(data+index, UploadData.CreditCompanyCode, 3);
    index += 3;
        }

//    DebugPrintf("index = %d  strlen(data) = %d\n", index, strlen(data));

    //64     通过秘钥计算得到的mac
    
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap(bitmap, 64);
    //把位图表的信息赋值进来
    memcpy(data+bitMapPos, bitmap, 8);
    memcpy(PbocKey.BaseKey,basinfo.MasterKey,32);            //实际只有16字节
    memcpy(PbocKey.DecryptKey,basinfo.Workingkey,16);        //最多只用到16字节，单倍长或者双倍长
    if(mchantConf.worktype)    
           GetUploadMac(data+13, index-istart);
    else
        GetUploadMac_yinlian(data+13, index-istart);

    memset(TempBuf, 0, sizeof(TempBuf));
    memcpy(data+index, UploadData.UploadMac, 8);
    index += 8;

    ShortUnon len;
    len.i = index;
    len.i -=2;
    data[0]  = len.intbuf[1];
    data[1]  = len.intbuf[0];
    DebugPrintf("当前的长度index=%d\n",index);
    
    DebugPrintChar("data", data, index);
    
    return 0;
}



int BuildConsumeData_QR(unsigned char *data,YLmain_Record * record)
{
     //unsigned char data[1024];
    unsigned char bitmap[8], TempBuf[256], BcdBuf[16];
    int index = 0, istart = 0, bitMapPos = 0;
    int Tempint = 0;
    char tpdubuf[5];
    LongUnon tmp;
    ShortUnon len;
    char tmp1;
    char Timebuf[7];
    memset(data, 0, sizeof(data));
    memset(BcdBuf, 0, sizeof(BcdBuf));
    memset(bitmap, 0, sizeof(bitmap));
//    memset(&TagData, 0, sizeof(struct PbocTagContainer));
    memset(tpdubuf,0,sizeof(tpdubuf));

    Rd_time(Timebuf+1);
    Time.year = Timebuf[1];
    Time.month = Timebuf[2];
    Time.day = Timebuf[3];
    Time.hour = Timebuf[4];
    Time.min = Timebuf[5];
    Time.sec = Timebuf[6];
    
    
    index+=2;                //预留2个字节用于数据长度

    memcpy(data+index,basinfo.TPDU,5);


    index += 5;


    memcpy(data+index,basinfo.HEAD,6);
    index+=6;


    istart = index;            //正真报文开始的地方
    //消息类型        12
    data[index] = 0x02;
    data[index + 1] = 0x00;
    index += 2;
    
    bitMapPos = index;
    index += 8;                    //这里是存放位元表的        20
/*    
    SetBitMap(bitmap, 2);
    data[index] = 0x19;            
    index += 1;
    //将57标签的前面一直到D停止的数据拷贝到这里去，这个就是银行卡号        21
    //57 13  62 30 65 00 00 60 00 01 72 1D 25 12 22 00 00 00  16 10 0F 
    //    memcpy(TagData.Tag_57.buf,"\x62\x30\x65\x00\x00\x60\x00\x01\x72\x1D",10);
    //    TagData.Tag_57.len=10;
    
    tmp1 = TagData.Tag_57.buf[9];
    TagData.Tag_57.buf[9] = TagData.Tag_57.buf[9]&0xf0;
    DebugPrintChar("Tag_57.buf",TagData.Tag_57.buf,10);
    memcpy(data+index, TagData.Tag_57.buf, 10);
    index += 10;
*/
    //03 交易处理码        31
    SetBitMap(bitmap, 3);
    if(mchantConf.worktype)    
    {
        data[index] = 0x00;
        data[index + 1] = 0x00;
        data[index + 2] = 0x00;
        }
    else
    {
        data[index] = 0x19;
        data[index + 1] = 0x00;
        data[index + 2] = 0x00;
        }
    index += 3;
    DebugPrintf("当前的长度index=%d\n",index);


    //04    交易金额    34    
    SetBitMap(bitmap, 4);
    memcpy(data+index,"\x00\x00",2);
    Convert_IntTOBcd_2(HostValue.i, BcdBuf);
    printf("在线交易金额:%d\n",HostValue.i);
    index+=2;
    //ChangBigToEd(BcdBuf,BcdBuf);
    memcpy(data+index,BcdBuf,4);
    index += 4;

    DebugPrintf("当前的长度index=%d\n",index);

    //11    受卡方系统跟踪号    40
    SetBitMap(bitmap, 11);
    memset(BcdBuf,0,sizeof(BcdBuf));
    tmp.i = 0;

    memcpy(tmp.longbuf,DevSID.longbuf,4);
    DebugPrintf("流水号tmp.i=%d\n",tmp.i);
    //tmp.i = 742650;    
    Convert_IntTOBcd_2(tmp.i,BcdBuf);
    //ChangBigToEd(BcdBuf,BcdBuf);
    memcpy(data+index,BcdBuf+1,3);
    index += 3;

    DebugPrintf("当前的长度index=%d\n",index);    
/*
    //14     卡有效期    43
    SetBitMap(bitmap, 14);
    //UploadData.AvailableData={0x16,0x05};
//    memcpy(UploadData.AvailableData,"\x16\x05",2);
//    memcpy(data+index, UploadData.AvailableData, 2);
    memcpy(data+index,&CardLan.Effective[1],2);
    index += 2;
*/
    //22      服务点输入方式码    45
    SetBitMap( bitmap, 22);
    if(mchantConf.worktype)    
    {
        data[index] = 0x03;
        data[index + 1] = 0x20;
        }
    else
    {
        data[index] = 0x07;
        data[index + 1] = 0x20;
        }
//    data[index] = 0x02 ;         //wsl20160702
//    data[index + 1] = 0x20;     //wsl20160702

    index += 2;
DebugPrintf("当前的长度index=%d\n",index);


/*
    //23    卡片序列号        47
    SetBitMap(bitmap, 23);
    data[index] = 0x00;
    data[index + 1] = 0x01;    
    index += 2;
*/
    //25     服务点条件码    49
    SetBitMap( bitmap, 25);
    if(mchantConf.worktype)    
    data[index] = 0x00;
    else
    data[index] = 0x91;
    index += 1;
DebugPrintf("当前的长度index=%d\n",index);    
/*
    //35     二磁道数据
    SetBitMap( bitmap, 35);
    data[index] = 0x36;
    index += 1;
    TagData.Tag_57.buf[9] = tmp1;
    memcpy(data+index,TagData.Tag_57.buf,TagData.Tag_57.len);
    index+=TagData.Tag_57.len;
    */
#if 0
    //41    受卡机终端标识码    50
    SetBitMap( bitmap, 41);
//    memcpy(UploadData.AcceptDevID,"GA000802",strlen("GA000802"));
    memcpy(UploadData.AcceptDevID,"GA005901",strlen("GA005901"));
//    memcpy(data+index, UploadData.AcceptDevID, 8);
    memcpy(data+index,card_driver.TermNum,8);

    index += 8;

    //42    受卡方标识码    58
    SetBitMap( bitmap, 42);
//    memcpy(UploadData.AcceptOrganizationID,"306530158120008",strlen("306530158120008"));
    memcpy(UploadData.AcceptOrganizationID,"306530156910059",strlen("306530156910059"));

//    memcpy(data+index, UploadData.AcceptOrganizationID, 15);
    memcpy(data+index,card_driver.MerchantNum,15);

    index += 15;
#else
    SetBitMap( bitmap, 41);    
    //memcpy(data+index, card_driver.TermNum, 8);
    memcpy(data+index, basinfo.TermNum, 8);
    index += 8;

    //42    受卡方标识码    58
    SetBitMap( bitmap, 42);    
    //memcpy(data+index, card_driver.MerchantNum, 15);
    memcpy(data+index, basinfo.MerchantNum, 15);
    index += 15;    

#endif
        
    DebugPrintf("当前的长度index=%d\n",index);

    if(!mchantConf.worktype)
    {
        //48  行业特定信息
        SetBitMap( bitmap, 48); 
        memset(TempBuf, 0, sizeof(TempBuf));
        BuildUploadData_48_qr(&Tempint, TempBuf,2);
        DebugPrintf("Tempint48 = %d\n", Tempint);
        Convert_IntTOBcd(Tempint, BcdBuf);
        DebugPrintf("index = %d\n", index);
        memcpy(data+index, BcdBuf, 2);
        index += 2;
        DebugPrintf("index = %d\n", index);
        memcpy(data+index, TempBuf, Tempint);
        index += Tempint;
        }
    DebugPrintf("当前的长度index=%d\n",index);

    

    //49 交易货币代码
    SetBitMap( bitmap, 49);
    //UploadData.MoneyType = {0x31,0x35,0x36};
    memcpy(UploadData.MoneyType,"\x31\x35\x36",3);
    memcpy(data+index, UploadData.MoneyType, 3);
    index += 3;
    DebugPrintf("当前的长度index=%d\n",index);

     if(!mchantConf.worktype)
    {
        //57    POS相关信息 
        /* Tempint 后面取出来之后需要转换成BCD码表示 */
        SetBitMap( bitmap, 57);
        memset(TempBuf, 0, sizeof(TempBuf));
        BuildUploadData_57(&Tempint, TempBuf);
        DebugPrintf("Tempint57 = %d\n", Tempint);
        Convert_IntTOBcd(Tempint, BcdBuf);
        DebugPrintf("index = %d\n", index);
        memcpy(data+index, BcdBuf, 2);
        index += 2;
        DebugPrintf("index = %d\n", index);
        memcpy(data+index, TempBuf, Tempint);
        index += Tempint;
        }
    DebugPrintf("当前的长度index=%d\n",index);


     if(mchantConf.worktype)    
     {
        //59   银联二维码支付需要的域
        SetBitMap( bitmap, 59);
        memset(TempBuf, 0, sizeof(TempBuf));
        BuildUploadData_59(&Tempint, TempBuf);
        Convert_IntTOBcd(Tempint, BcdBuf);
        memcpy(data+index, BcdBuf, 2);
        index += 2;
        memcpy(data+index, TempBuf, Tempint);
        index += Tempint;
    }
    //60     自定义域    
    SetBitMap( bitmap, 60);
    memset(TempBuf, 0, sizeof(TempBuf));
    BuildUploadData_60_qr(&Tempint, TempBuf);
    memcpy(data+index, TempBuf, Tempint);
    memcpy(record->BatchNum,TempBuf+3,3);
    index += Tempint;
    DebugPrintf("当前的长度index=%d\n",index);

#if 0
    //63    自定义域
    SetBitMap(bitmap, 63);
    data[index] = 0x00;
    data[index + 1] = 0x03;
    index += 2;
//    memcpy(UploadData.CreditCompanyCode,"\x00\x00\x00",3);
    memcpy(UploadData.CreditCompanyCode,"CPU",3);
    memcpy(data+index, UploadData.CreditCompanyCode, 3);
    index += 3;
#endif

//    DebugPrintf("index = %d  strlen(data) = %d\n", index, strlen(data));

    //64     通过秘钥计算得到的mac
    
    SetBitMap(bitmap, 64);
    //把位图表的信息赋值进来
    memcpy(data+bitMapPos, bitmap, 8);
    memcpy(PbocKey.BaseKey,basinfo.MasterKey,32);            //实际只有16字节
    memcpy(PbocKey.DecryptKey,basinfo.Workingkey,16);        //最多只用到16字节，单倍长或者双倍长
    if(mchantConf.worktype)    
        GetUploadMac(data+13, index-istart);
    else
        GetUploadMac_yinlian(data+13, index-istart);
    

    memset(TempBuf, 0, sizeof(TempBuf));
//      memcpy(data+index, TempBuf, 8);
    memcpy(data+index, UploadData.UploadMac, 8);
    index += 8;

    len.i = index;
    
    len.i -=2;
    data[0]  = len.intbuf[1];
    data[1]  = len.intbuf[0];
    DebugPrintf("当前的长度index=%d\n",index);

    
//    data[0] = 0x00;
//    data[1] = index-2;
    
    DebugPrintChar("data", data, index);
    
    return 0;
}

int BuildConsumeData_QR_CZ(unsigned char *data)
{
     //unsigned char data[1024];
    unsigned char bitmap[8], TempBuf[256], BcdBuf[16];
    int index = 0, istart = 0, bitMapPos = 0;
    int Tempint = 0;
    char tpdubuf[5];
    LongUnon tmp;
    ShortUnon len;
    char tmp1;
    memset(data, 0, sizeof(data));
    memset(BcdBuf, 0, sizeof(BcdBuf));
    memset(bitmap, 0, sizeof(bitmap));
//    memset(&TagData, 0, sizeof(struct PbocTagContainer));
    memset(tpdubuf,0,sizeof(tpdubuf));


    
    index+=2;                //预留2个字节用于数据长度


    memcpy(data+index,basinfo.TPDU,5);





    index += 5;


    memcpy(data+index,basinfo.HEAD,6);
    index+=6;


    istart = index;            //正真报文开始的地方
    //消息类型        12
    data[index] = 0x04;
    data[index + 1] = 0x00;
    index += 2;
    
    bitMapPos = index;
    index += 8;                    //这里是存放位元表的        20
/*    
    SetBitMap(bitmap, 2);
    data[index] = 0x19;            
    index += 1;
    //将57标签的前面一直到D停止的数据拷贝到这里去，这个就是银行卡号        21
    //57 13  62 30 65 00 00 60 00 01 72 1D 25 12 22 00 00 00  16 10 0F 
    //    memcpy(TagData.Tag_57.buf,"\x62\x30\x65\x00\x00\x60\x00\x01\x72\x1D",10);
    //    TagData.Tag_57.len=10;
    
    tmp1 = TagData.Tag_57.buf[9];
    TagData.Tag_57.buf[9] = TagData.Tag_57.buf[9]&0xf0;
    DebugPrintChar("Tag_57.buf",TagData.Tag_57.buf,10);
    memcpy(data+index, TagData.Tag_57.buf, 10);
    index += 10;
*/
    //03 交易处理码        31
    SetBitMap(bitmap, 3);
    if(mchantConf.worktype)    
    {
        data[index] = 0x00;
        data[index + 1] = 0x00;
        data[index + 2] = 0x00;
        }
    else
    {
        data[index] = 0x19;
        data[index + 1] = 0x00;
        data[index + 2] = 0x00;
        }

    index += 3;
    DebugPrintf("当前的长度index=%d\n",index);


    //04    交易金额    34    
    SetBitMap(bitmap, 4);
    memcpy(data+index,"\x00\x00",2);
    Convert_IntTOBcd_2(HostValue.i, BcdBuf);
    printf("在线交易金额:%d\n",HostValue.i);
    index+=2;
    //ChangBigToEd(BcdBuf,BcdBuf);
    memcpy(data+index,BcdBuf,4);
    index += 4;

    DebugPrintf("当前的长度index=%d\n",index);

    //11    受卡方系统跟踪号    40
    SetBitMap(bitmap, 11);
    memset(BcdBuf,0,sizeof(BcdBuf));
    tmp.i = 0;

    memcpy(tmp.longbuf,DevSID.longbuf,4);
    DebugPrintf("流水号tmp.i=%d\n",tmp.i);
    //tmp.i = 742650;    
    Convert_IntTOBcd_2(tmp.i,BcdBuf);
    //ChangBigToEd(BcdBuf,BcdBuf);
    memcpy(data+index,BcdBuf+1,3);
    index += 3;

    DebugPrintf("当前的长度index=%d\n",index);    
/*
    //14     卡有效期    43
    SetBitMap(bitmap, 14);
    //UploadData.AvailableData={0x16,0x05};
//    memcpy(UploadData.AvailableData,"\x16\x05",2);
//    memcpy(data+index, UploadData.AvailableData, 2);
    memcpy(data+index,&CardLan.Effective[1],2);
    index += 2;
*/
    //22      服务点输入方式码    45
    SetBitMap( bitmap, 22);
    if(mchantConf.worktype)    
    {
         data[index] = 0x03;
        data[index + 1] = 0x20;
        }
    else
    {
        data[index] = 0x07;
        data[index + 1] = 0x20;
        }
//    data[index] = 0x02 ;         //wsl20160702
//    data[index + 1] = 0x20;     //wsl20160702

    index += 2;
DebugPrintf("当前的长度index=%d\n",index);


/*
    //23    卡片序列号        47
    SetBitMap(bitmap, 23);
    data[index] = 0x00;
    data[index + 1] = 0x01;    
    index += 2;
*/
    //25     服务点条件码    49
    SetBitMap( bitmap, 25);
    if(mchantConf.worktype)    
    {
        data[index] = 0x00;
        }
    else
        data[index] = 0x91;
    index += 1;
DebugPrintf("当前的长度index=%d\n",index);    
/*
    //35     二磁道数据
    SetBitMap( bitmap, 35);
    data[index] = 0x36;
    index += 1;
    TagData.Tag_57.buf[9] = tmp1;
    memcpy(data+index,TagData.Tag_57.buf,TagData.Tag_57.len);
    index+=TagData.Tag_57.len;
    */

    SetBitMap( bitmap, 39); 
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    //memcpy(data+index, card_driver.TermNum, 8);
    memcpy(data+index, "98", 2);
    index += 2; 



#if 0
    //41    受卡机终端标识码    50
    SetBitMap( bitmap, 41);
//    memcpy(UploadData.AcceptDevID,"GA000802",strlen("GA000802"));
    memcpy(UploadData.AcceptDevID,"GA005901",strlen("GA005901"));
//    memcpy(data+index, UploadData.AcceptDevID, 8);
    memcpy(data+index,card_driver.TermNum,8);

    index += 8;

    //42    受卡方标识码    58
    SetBitMap( bitmap, 42);
//    memcpy(UploadData.AcceptOrganizationID,"306530158120008",strlen("306530158120008"));
    memcpy(UploadData.AcceptOrganizationID,"306530156910059",strlen("306530156910059"));

//    memcpy(data+index, UploadData.AcceptOrganizationID, 15);
    memcpy(data+index,card_driver.MerchantNum,15);

    index += 15;
#else
    SetBitMap( bitmap, 41);    
    //memcpy(data+index, card_driver.TermNum, 8);
    memcpy(data+index, basinfo.TermNum, 8);
    index += 8;

    //42    受卡方标识码    58
    SetBitMap( bitmap, 42);    
    //memcpy(data+index, card_driver.MerchantNum, 15);
    memcpy(data+index, basinfo.MerchantNum, 15);
    index += 15;    

#endif
        
    DebugPrintf("当前的长度index=%d\n",index);

    if(!mchantConf.worktype)
    {
        //48  行业特定信息
        SetBitMap( bitmap, 48); 
        memset(TempBuf, 0, sizeof(TempBuf));
        BuildUploadData_48_qr(&Tempint, TempBuf,2);
        DebugPrintf("Tempint48 = %d\n", Tempint);
        Convert_IntTOBcd(Tempint, BcdBuf);
        DebugPrintf("index = %d\n", index);
        memcpy(data+index, BcdBuf, 2);
        index += 2;
        DebugPrintf("index = %d\n", index);
        memcpy(data+index, TempBuf, Tempint);
        index += Tempint;
        }
    DebugPrintf("当前的长度index=%d\n",index);

    

    //49 交易货币代码
    SetBitMap( bitmap, 49);
    //UploadData.MoneyType = {0x31,0x35,0x36};
    memcpy(UploadData.MoneyType,"\x31\x35\x36",3);
    memcpy(data+index, UploadData.MoneyType, 3);
    index += 3;
    DebugPrintf("当前的长度index=%d\n",index);

    #if 0
    //57    POS相关信息    
    /* Tempint 后面取出来之后需要转换成BCD码表示 */
    SetBitMap( bitmap, 57);
    memset(TempBuf, 0, sizeof(TempBuf));
    BuildUploadData_57(&Tempint, TempBuf);
    DebugPrintf("Tempint57 = %d\n", Tempint);
    Convert_IntTOBcd(Tempint, BcdBuf);
    DebugPrintf("index = %d\n", index);
    memcpy(data+index, BcdBuf, 2);
    index += 2;
    DebugPrintf("index = %d\n", index);
    memcpy(data+index, TempBuf, Tempint);
    index += Tempint;
    
    DebugPrintf("当前的长度index=%d\n",index);
    #endif

    if(mchantConf.worktype)    
    {
        //59    这里是处理客户定制的功能，这里是客户定制的功能
        SetBitMap( bitmap, 59);
        memcpy(data+index,"\x00\x24",2);
        index+=2;
        memcpy(data+index,"A3",2);
        index+=2;
        BcdBuf[0]=0x30;    
        BcdBuf[1] =G_QRuartInfo.length/10+'0';
        BcdBuf[2] = G_QRuartInfo.length%10+'0';
        memcpy(data+index, BcdBuf, 3);
        index+=3;
        memcpy(data+index, G_QRuartInfo.id, G_QRuartInfo.length);    
        index+=G_QRuartInfo.length;
        }
    //60     自定义域    
    SetBitMap( bitmap, 60);
    memset(TempBuf, 0, sizeof(TempBuf));
    BuildUploadData_60_qr(&Tempint, TempBuf);
    memcpy(data+index, TempBuf, Tempint);
    index += Tempint;
    DebugPrintf("当前的长度index=%d\n",index);

#if 0
    //63    自定义域
    SetBitMap(bitmap, 63);
    data[index] = 0x00;
    data[index + 1] = 0x03;
    index += 2;
//    memcpy(UploadData.CreditCompanyCode,"\x00\x00\x00",3);
    memcpy(UploadData.CreditCompanyCode,"CPU",3);
    memcpy(data+index, UploadData.CreditCompanyCode, 3);
    index += 3;
#endif
    #if 0
    //61    自定义域    
    DebugPrintf(" in LINE %d ,index %d \n",__LINE__,index);
    SetBitMap( bitmap, 61);
    memset(TempBuf, 0, sizeof(TempBuf));
    BuildUploadData_61_qr(&Tempint, TempBuf);
    memcpy(data+index, TempBuf, Tempint);
    index += Tempint;
#endif




//    DebugPrintf("index = %d  strlen(data) = %d\n", index, strlen(data));

    //64     通过秘钥计算得到的mac
    
    SetBitMap(bitmap, 64);
    //把位图表的信息赋值进来
    memcpy(data+bitMapPos, bitmap, 8);
    memcpy(PbocKey.BaseKey,basinfo.MasterKey,32);            //实际只有16字节
    memcpy(PbocKey.DecryptKey,basinfo.Workingkey,16);        //最多只用到16字节，单倍长或者双倍长
    if(mchantConf.worktype)    
        GetUploadMac(data+13, index-istart);
    else    
        GetUploadMac_yinlian(data+13, index-istart);
    

    memset(TempBuf, 0, sizeof(TempBuf));
//      memcpy(data+index, TempBuf, 8);
    memcpy(data+index, UploadData.UploadMac, 8);
    index += 8;

    len.i = index;
    
    len.i -=2;
    data[0]  = len.intbuf[1];
    data[1]  = len.intbuf[0];
    DebugPrintf("当前的长度index=%d\n",index);

    
//    data[0] = 0x00;
//    data[1] = index-2;
    
    DebugPrintChar("data", data, index);
    
    return 0;
}

//创建请求黑名单下载
int BuildGetBlackList(unsigned char *data)
{
    char bitmap[8], tempBuf[32];
    unsigned char TempBuf[512], BcdBuf[16];    
    int Tempint = 0;    
    int index=0, BitMapPos = 0;
    char tpdubuf[5];
    ShortUnon len;
    
    memset(data, 0, sizeof(data));
    memset(bitmap, 0, sizeof(bitmap));
    memset(tempBuf, 0, sizeof(tempBuf));
    memset(tpdubuf,0,sizeof(tpdubuf));
    
    //data[index] = 0x00;
    //data[index+1] = 0x3C;
    index += 2;

    //TPDU共10位 5个字节，前两位为TPDU ID 一般为"60",中间4位为TPDU 目的地址,最后4位为TPDU 源地址为"0000",
    //默认值为"60 00 03 00 00"

    memcpy(data+index,basinfo.TPDU,5);    
    index += 5;

    memcpy(data+index,basinfo.HEAD,6);
    index+=6;


    //消息类型0 1
    data[index] = 0x08;
    data[index+1] = 0x00;    
    index += 2;
    DebugPrintf("index = %d\n", index);

    //这里是预留给位元表的
    BitMapPos = index;
    printf("位元起始下标:%d \n",BitMapPos);
    index += 8;





    //终端代码    41域   由银行提供
    SetBitMap(bitmap, 41);
    //测试终端号
    memset(tempBuf,0,sizeof(tempBuf));
//    memcpy(tempBuf,"GA000802",strlen("GA000802"));                    //测试终端号
//    memcpy(tempBuf,"GA005901",strlen("GA005901"));
//    memcpy(tempBuf,card_driver.TermNum,8);
    memcpy(tempBuf,basinfo.TermNum,8);
    memcpy(&data[index], tempBuf, 8);
    index += 8;
    DebugPrintf("index = %d\n", index);

    //商户代码    42域   由银行提供
    SetBitMap(bitmap, 42);
    //测试商户号
    memset(tempBuf, 0x0, sizeof(tempBuf));
//    memcpy(tempBuf,"306530158120008",strlen("306530158120008"));    //测试商户号
//    memcpy(tempBuf,"306530156910059",strlen("306530156910059"));
//    memcpy(tempBuf,card_driver.MerchantNum,15);
    memcpy(tempBuf,basinfo.MerchantNum,15);
    memcpy(&data[index], tempBuf, 15);
    index += 15;
    DebugPrintf("index = %d\n", index);

    //自定义域    60域
    SetBitMap(bitmap, 60);
    data[index]=0x00,
    data[index+1] = 0x11;        //60域的长度
    index += 2;

    //交易码类型     60.1域
    data[index] = 0x00;
    index += 1;
    
    //批次号 60.2域
    memcpy(data+index, "\x30\x00\x01", 3);
    index += 3;
    DebugPrintf("index = %d\n", index);
    
    //网络管理信息码 60.3域
    data[index] = 0x57;
    data[index + 1] = 0x00;
    index += 2;
    
    //63 自定义域    ans...003    LLLVAR    ASCII 2    M(44,2)    
    SetBitMap(bitmap, 63);

    memset(TempBuf, 0, sizeof(TempBuf));
    BuildUploadData_63(&Tempint, TempBuf);
    memcpy(data+index, TempBuf, Tempint);
    index += Tempint;    

    memcpy(data+BitMapPos, bitmap, 8);

    len.i = index;
    
    len.i -=2;
    data[0]  = len.intbuf[1];
    data[1]  = len.intbuf[0];
    DebugPrintf("当前的长度index=%d\n",index);
    
    
    printf("index = %d\n", index);
    
    DebugPrintChar("申请黑名单信息", data, index);

    return 0;
}


int BuildSettleInfo(unsigned char *data)
{
    char bitmap[8], tempBuf[32];
    //unsigned char data[1024];
    int index=0, TempInt = 0, pBitMapLocation = 0;
    
    //memset(data, 0, sizeof(data));
    memset(bitmap, 0, sizeof(bitmap));
    memset(tempBuf, 0, sizeof(tempBuf));

    data[index] = 0x60;
    data[index + 1] = 0x00;//正式
    data[index + 2] = 0x09;
    data[index + 3] = 0x00;
    data[index + 4] = 0x00;
    index += 5;

    //报文头：应用类别 1
    data[index] = 0x60;
    index += 1;
    
    //报文头：软件版本号 1
    data[index] = 0x21;// 0x31;
    index += 1;

    //报文头：终端状态和处理要求 00
    data[index] = 0x00;
    index += 1;

    //报文头：保留 3 
    data[index] = 0x00;
    data[index + 1] = 0x00;
    data[index + 2] = 0x00;
    index += 3;

    //消息类型
    data[index] = 0x05;
    data[index + 1] = 0x00;
    index += 2;

    //位元表
    pBitMapLocation = index;
    index += 8;

    //11 受卡方系统跟踪号
    SetBitMap(bitmap, 11);
    memcpy(data+index, SettleData.strSernialNo, 3);
    index += 3;
    
    //41 受卡机终端标识码(终端代码)
    SetBitMap(bitmap, 41);
    memcpy(data+index, SettleData.AcceptDevID, 8);
    index += 8;
        
    //42 受卡方标识码(商户代码) 
    SetBitMap(bitmap, 42);
    memcpy(data+index, SettleData.AcceptOrganizationID, 16);
    index += 15;

    //48 附加数据
    SetBitMap(bitmap, 48);    
    TempInt += 0;
    memcpy(data+index, SettleData.PrivateData, 16);
    index += TempInt;
    
    //49 交易货币代码
    SetBitMap(bitmap, 49);
    memcpy(data+index, SettleData.MoneyType, 3);
    index += 3;

    //60 自定义域
    SetBitMap(bitmap, 60);    
    memcpy(data+index, SettleData.CustomLen_60, 2);
    index += 2;

    //60.1 交易类型码
    data[index] = 0x00;
    index += 1;
    
    //60.2    批次号    
    memcpy(data+index, SettleData.BatchCode, 3);
    index += 3;

    //60.3 网络管理信息码
    memcpy(data+index, SettleData.NetManageCode, 2);
    index += 2;

    SetBitMap(bitmap, 63);
    data[index] = 0x00;
    data[index + 1] = 0x03;    
    index += 2;
    //DebugPrintChar("DATA", data, index);
    //63.1 操作员代码
    memcpy(data+index, SettleData.OperCode, 3);
    index += 3;
    
    DebugPrintChar("strSernialNo", SettleData.strSernialNo, 3);
    DebugPrintChar("AcceptDevID", SettleData.AcceptDevID, 3);
    DebugPrintChar("AcceptOrganizationID", SettleData.AcceptOrganizationID, 3);
    DebugPrintChar("PrivateData", SettleData.PrivateData, 3);
    DebugPrintChar("MoneyType", SettleData.MoneyType, 3);
    DebugPrintChar("CustomLen_60", SettleData.CustomLen_60, 3);
    DebugPrintChar("BatchCode", SettleData.BatchCode, 3);
    DebugPrintChar("NetManageCode", SettleData.NetManageCode, 3);
    DebugPrintChar("OperCode", SettleData.OperCode, 3);

    memcpy(data+pBitMapLocation, bitmap, 8);

    DebugPrintChar("DATA", data, index);

    
    return 0;
}

int HandelBatchSett(char *str)
{
    int RecvLen = 0, i=0, j=0;
    int index = 0, TempInt = 0;
    unsigned char pTempBuf[128], pBitMap[64];
    unsigned char RecvData[1024];

    struct SignContainerReply SignData;
    
    memset(&SignData, 0, sizeof(SignData));
    memset(pBitMap, 0, sizeof(pBitMap));
    memset(RecvData, 0, sizeof(RecvData));
    memset(pTempBuf, 0, sizeof(pTempBuf));
    
    char * pStr = "007860000000096021000000000510003A000108C18012000009122424031903190800096500313232343234363130373630323031303033343339353235313036343131313030303100620000000545603470000000000000002000000000000000000000000000000131353600110000231920100003303031";
    memcpy(pTempBuf, pStr, 4);
     RecvLen = Convert_HexTOInt(pTempBuf, 4);
    
    if(RecvLen != (strlen(pStr)/2 -2))
    {
        printf("签到返回数据错误：长度不对！返回数据 : %d\n", RecvLen);
    }

    for(i=0; i<(2*RecvLen+4); i++)
    {
        RecvData[i]= toupper(*pStr++);
    }

    index += 30;
     
    memset(pTempBuf, 0, sizeof(pTempBuf));
    memcpy(pTempBuf, RecvData+30 , 16);
    GetBitMap(pTempBuf, pBitMap);
    index += 16;


    //11 受卡方系统跟踪号
    index += 6;

    //12 受卡方所在地时间
    index += 6;

    //13 受卡方所在地日期
    index += 4;

    //15 清算日期
    index += 4;

    //32 受理方标识码
    memset(pTempBuf, 0, sizeof(pTempBuf));
    memcpy(pTempBuf, RecvData+index, 12);
    DebugPrintf("pTempBuf = %s\n", pTempBuf);
    index += 2;
    TempInt = Convert_StrToInt(pTempBuf, 2);
    DebugPrintf("受理方标识码 = %d\n", TempInt);
    index += TempInt;        //BCD码一个字节表示两位

    //37 检索参考号
    index += 24;

    //41 受卡机终端标识码
    index += 16;

    //42 受卡方标识码
    index += 30;
    
    //48 附加数据-私有
    memset(pTempBuf, 0, sizeof(pTempBuf));
    memcpy(pTempBuf, RecvData+index, 4);
    DebugPrintf("pTempBuf = %s\n", pTempBuf);
    index += 4;
    TempInt = Convert_StrToInt(pTempBuf, 4);
    DebugPrintf("受理方标识码 = %d\n", TempInt);
    index += TempInt;        //BCD码一个字节表示两位

    //49 交易货币代码
    index += 6;

    //60 自定义域    
    memset(pTempBuf, 0, sizeof(pTempBuf));
    memcpy(pTempBuf, RecvData+index, 14);
    
    DebugPrintf("pTempBuf = %s\n", pTempBuf);
    index += 4;
    TempInt = Convert_StrToInt(pTempBuf, 4);
    DebugPrintf("自定义域    = %d\n", TempInt);
    index += TempInt;
    
    return 0;
}

#if 0
int SendMsgToServer_PBOC(char * HttpSendContent, const char * serverIPAddr, int param)
{    
    int sockfd, ret, i, h, ReturnFlag = 0;
    socklen_t len;    
    fd_set HttpSet;       
    char RecvBuf[RECV_BUF_SIZE_HAICHEEN];
    char RecvCmpBuf[16];    
    struct timeval  tv;    
    struct sockaddr_in servaddr;        //创建socket连接    
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )     
    {            
        perror("socket error!\n");            
        return -1;    
    }    
    
    bzero(&servaddr, sizeof(servaddr));    
    servaddr.sin_family = AF_INET;    
    servaddr.sin_port = htons(PORT_HAICHEEN);
    
   if (inet_pton(AF_INET, serverIPAddr, &servaddr.sin_addr) <= 0 )    
    {        
        DebugPrintf("inet_pton error!\n");        
        return -1;    
    }    

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)    
    {        
        DebugPrintf("connect server error!\n");        
        return -1;    
    }
    
    printf("send : %s\n",HttpSendContent);        
    ret = write(sockfd, HttpSendContent, param);        
    if (ret < 0) 
    {        
        DebugPrintf("write error, code %d infomation : %s\n",errno, strerror(errno));        
        return -1;    
    }else{        
        DebugPrintf("write success %d\n", ret);    
    }   
    
    FD_ZERO(&HttpSet);    
    FD_SET(sockfd, &HttpSet);
    
    while(1)    
    {        
        sleep(2);        
        tv.tv_sec= 2;        
        tv.tv_usec= 0;        
        h= 0;        
        h= select(sockfd +1, &HttpSet, NULL, NULL, &tv);        
        if (h == 0)        
        {            
            DebugPrintf("continue time out\n");            
            close(sockfd);            
            return -1;        
        }        
        else if (h < 0)        
        {            
            close(sockfd);            
            DebugPrintf("select error\n");            
            return -1;        
        }        
        else if (h > 0)        
        {            
            memset(RecvBuf, 0, RECV_BUF_SIZE_HAICHEEN);
            i= read(sockfd, RecvBuf, RECV_BUF_SIZE_HAICHEEN);
            if (i==0)            
            {                
                close(sockfd);                
                DebugPrintf("remote close\n");                
                return -1;            
            }            
            printf("recive :%s\n", RecvBuf);            
            if(G_SignInfo_PBOC)
                ReturnFlag = HandleSignReplay(RecvBuf);
            else if(G_Consume_PBOC)
                ReturnFlag = HandelSendReplay(RecvBuf);
            else if(G_BatchSett_PBOC)
                ReturnFlag = HandelBatchSett(RecvBuf);
            break;
        }    
    }    
    close(sockfd);    
    
    return ReturnFlag;
}


int PbocDes_Demo(void)
{

    char dataIn[256];
    char dataOut[256];
    int datalen = 0, OutLen = 0, resault = 0;
    
    memset(dataIn, 0, sizeof(dataIn));
    memset(dataOut, 0, sizeof(dataOut));
    
    memcpy(dataIn, "123456789abcdefghijk", strlen("123465789abcdefghijk"));
    datalen = strlen("123465789abcdefghijk");

    DebugPrintf("dataIn = %s\n", dataIn);
    DebugPrintf("datalen  = %d\n", datalen );
    
    PbocEncrypt_DES(dataIn, datalen, dataOut, &OutLen, PbocKey.BaseKey);
    DebugPrintf("OutLen = %d\n", OutLen);

    memcpy(dataIn, dataOut, OutLen);
    datalen = OutLen;
    PbocDecrypt_DES(dataIn, datalen, dataOut, &OutLen, PbocKey.BaseKey);
    DebugPrintf("dataOut = %s\n", dataOut);

    return 0;
}


int ParserRecordToGetTag(void)
{
    int index = 0, datalen = 0;
    char RecordBuf[256];

    memset(RecordBuf, 0, sizeof(RecordBuf));

    while(index < datalen)
    {
        /* 取出来的记录的最后面是有0x00的，这部分要过滤掉 */
        if(RecordBuf[index] == 0x00)        
        {
            index++;
            continue;
        }
        
        if((RecordBuf[index] & 0x1F) == 0x1F)
        {
            datalen = RecordBuf[index+2];
            switch(RecordBuf[index])
            {
                case 0x9f:
                {
                    switch(RecordBuf[index+1])
                    {
                        case 0x26:
                            memcpy(TagData.Tag_9F26.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F26.len = datalen;
                            index += datalen;
                            break;
                        case 0x27:
                            memcpy(TagData.Tag_9F27.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F27.len = datalen;
                            index += datalen;                            
                            break;
                        case 0x10:
                            memcpy(TagData.Tag_9F10.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F10.len = datalen;
                            index += datalen;
                            break;
                        case 0x37:
                            memcpy(TagData.Tag_9F37.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F37.len = datalen;
                            index += datalen;                            
                            break;
                        case 0x36:
                            memcpy(TagData.Tag_9F36.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F36.len = datalen;
                            index += datalen;
                            break;
                        case 0x02:
                            memcpy(TagData.Tag_9F02.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F02.len = datalen;
                            index += datalen;                            
                            break;
                        case 0x1A:
                            memcpy(TagData.Tag_9F1A.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F1A.len = datalen;
                            index += datalen;
                            break;
                        case 0x03:
                            memcpy(TagData.Tag_9F03.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F03.len = datalen;
                            index += datalen;                            
                            break;
                        case 0x33:
                            memcpy(TagData.Tag_9F33.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F33.len = datalen;
                            index += datalen;
                            break;
                        case 0x34:
                            memcpy(TagData.Tag_9F34.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F34.len = datalen;
                            index += datalen;                            
                            break;
                        case 0x35:
                            memcpy(TagData.Tag_9F35.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F35.len = datalen;
                            index += datalen;
                            break;
                        case 0x1E:
                            memcpy(TagData.Tag_9F1E.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F1E.len = datalen;
                            index += datalen;                            
                            break;
                        case 0x09:
                            memcpy(TagData.Tag_9F09.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F09.len = datalen;
                            index += datalen;
                            break;
                        case 0x41:
                            memcpy(TagData.Tag_9F41.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F41.len = datalen;
                            index += datalen;                            
                            break;
                        case 0x74:
                            memcpy(TagData.Tag_9F74.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_9F74.len = datalen;
                            index += datalen;
                            break;
                        default:
                            printf("TERMAPP_EncodeTLV(): Unknown tag: %02X%02X\n", RecordBuf[index], RecordBuf[index + 1]);
                    }
                }
                break;
                case 0x5f:
                {
                    switch(RecordBuf[index+1])
                    {
                        case 0x24:
                            memcpy(TagData.Tag_5F24.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_5F24.len = datalen;
                            index += datalen;
                            break;
                        case 0x2A:
                            memcpy(TagData.Tag_5F2A.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_5F2A.len = datalen;
                            index += datalen;
                            break;
                        case 0x34:
                            memcpy(TagData.Tag_5F34.buf, RecordBuf+index+3, datalen);
                            TagData.Tag_5F34.len = datalen;
                            index += datalen;
                            break;
                        default:
                            printf("TERMAPP_EncodeTLV(): Unknown tag: %02X%02X\n", RecordBuf[index], RecordBuf[index + 1]);
                    }
                }
                break;
            }

            index += 3; /* 标签+长度标志 */
        }
        else
        {
            datalen = RecordBuf[index+1];            
            switch(RecordBuf[index])
            {
                case 0x57:
                    memcpy(TagData.Tag_57.buf, RecordBuf+index+2, datalen);
                    TagData.Tag_57.len = datalen;
                    index += datalen;
                    break;
                case 0x95:
                    memcpy(TagData.Tag_95.buf, RecordBuf+index+2, datalen);
                    TagData.Tag_95.len = datalen;
                    index += datalen;
                    break;
                case 0x9A:
                    memcpy(TagData.Tag_9A.buf, RecordBuf+index+2, datalen);
                    TagData.Tag_9A.len = datalen;
                    index += datalen;
                    break;
                case 0x9C:
                    memcpy(TagData.Tag_9C.buf, RecordBuf+index+2, datalen);
                    TagData.Tag_9C.len = datalen;
                    index += datalen;
                    break;
                case 0x82:
                    memcpy(TagData.Tag_82.buf, RecordBuf+index+2, datalen);
                    TagData.Tag_82.len = datalen;
                    index += datalen;
                    break;
                case 0x84:
                    memcpy(TagData.Tag_84.buf, RecordBuf+index+2, datalen);
                    TagData.Tag_84.len = datalen;
                    index += datalen;
                    break;
                case 0x8A:
                    memcpy(TagData.Tag_8A.buf, RecordBuf+index+2, datalen);
                    TagData.Tag_8A.len = datalen;
                    index += datalen;
                    break;
                case 0x5A:
                    memcpy(TagData.Tag_5A.buf, RecordBuf+index+2, datalen);
                    TagData.Tag_5A.len = datalen;
                    index += datalen;
                    break;
                default:
                    printf("TERMAPP_EncodeTLV(): Unknown tag: %02X\n", RecordBuf[index]);
            }
            index += 2; 
        }

    }

    return 0;
}

int main_demo(int argc, char **argv)
{
    char TempBuf[32];
    unsigned char data[512];
    
    memset(TempBuf, 0, sizeof(TempBuf));
    memset(data, 0, sizeof(data));
    
    GetUploadMac(NULL, 0);
    //PbocDes_Demo();        //单des加密

    //BuildConsumeData();
    
    //HandelSendReplay(NULL);
    //BuildSettleInfo();
    return 0;

    BuildSignInInfoByte(NULL);
    BuildConsumeData(NULL);
    BuildSettleInfo(NULL);
    
    HandleSignReplay(NULL);
    HandelSendReplay(NULL);
    HandelBatchSett(NULL);

    return 0;
}

#endif


