#include <stdio.h>
#include "libcardlan_CardInfo.h"
#include "libcardlan_StandardBus_util.h"
#include "mcu_opt/psam_opt.h"


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


//#define LOGI printf

void menu_print(unsigned char *buffer, int length)
{
    int i;

    for (i = 0; i < length; i++)
    {
        LOGI("%02X ", *(buffer + i));
        if ((i + 1) % 8 == 0) LOGI(" ");
        if ((i + 1) % 16 == 0) LOGI("\n");
    }
    LOGI("\n");
}

void print_buffer(char *string, char* p, int size)
{
    int i = 0;

    if(p == NULL || string == NULL)
        return;

    LOGI("%s\n", string);
    for(i=0; i<size; i++)
    {
        LOGI("%#X ", p[i]);
    }
    LOGI("\n");
}

void Dump_CardInfo(void)
{
    LOGI("\n--------------%s--------\n",__FUNCTION__);
    LOGI("PsamKeyIndex: %d\n", PsamKeyIndex);
    LOGI("是否本地卡 1-本地卡 2-外地卡: %d\n", JTB_CardInfo.IsLocalCard);
    print_buffer("物理卡号:", JTB_CardInfo.CSN, 4);
    print_buffer("交易前余额:", JTB_CardInfo.beforemoney, 4);
    print_buffer("本次交易金额:", JTB_CardInfo.thismoney, 4);
    print_buffer("交易后余额:", JTB_CardInfo.aftermoney, 4);
    print_buffer("脱机交易序号:", JTB_CardInfo.offlineSN, 2);
    print_buffer("透支限额:", JTB_CardInfo.overdraftAmount, 3);
    LOGI("密钥版本号: %d\n", JTB_CardInfo.keyVersion);
    LOGI("算法标识: %d\n", JTB_CardInfo.arithmeticLabel);
    print_buffer("伪随机数:", JTB_CardInfo.PRandom, 4);
    print_buffer("TAC:", JTB_CardInfo.TAC, 4);
    print_buffer("MAC2:", JTB_CardInfo.MAC2, 4);
    print_buffer("DESCRY:", JTB_CardInfo.DESCRY, 4);
    
    
    LOGI("\n分段控制信息\n");
    LOGI("上下车标志: %d\n", JTB_CardInfo.enterexitflag);
    
    LOGI("\nPSAM卡数据\n");
    print_buffer("终端脱机交易序号:", JTB_CardInfo.PSAMOfflineSN, 4);
    print_buffer("MAC1:", JTB_CardInfo.MAC1, 4);
    print_buffer("PSAM卡随机数", JTB_CardInfo.PSAMRandom, 8);

    LOGI("\n0015文件\n");
    print_buffer("发卡机构标识:", JTB_CardInfo.issuerlabel, 8);
    LOGI("应用类型标识(01-只有ED,02-只有EP,03-ED和EP都有): %d\n", JTB_CardInfo.apptypelabel);
    LOGI("发卡机构应用版本: %d\n", JTB_CardInfo.issuerappversion);
    print_buffer("应用序列号:", JTB_CardInfo.appserialnumber, 10);
    print_buffer("应用启用日期:", JTB_CardInfo.appstarttime, 4);
    print_buffer("应用截止日期:", JTB_CardInfo.appendtime, 4);

    LOGI("\n0016文件\n");
    print_buffer("司机卡证件号码:", JTB_CardInfo.CertiID,  18);
    print_buffer("司机卡姓名:", JTB_CardInfo.drivername,  20);
    
    LOGI("\n0017文件\n");
    print_buffer("国家代码:", JTB_CardInfo.countrycode,  4);
    print_buffer("省级代码:", JTB_CardInfo.provincecode, 2);
    print_buffer("城市代码:", JTB_CardInfo.citycode, 2);
    print_buffer("互通卡种 0000-非互通卡，0001-互通卡:", JTB_CardInfo.unioncardtype, 2);
    LOGI("卡类型: %d\n", JTB_CardInfo.cardtype);
    print_buffer("结算单元编号:", JTB_CardInfo.settlenumber, 4);

    LOGI("\n0018文件\n");
    print_buffer("ED/EP联机或脱机交易序号:", JTB_CardInfo.tradenumber,  2);
    print_buffer("透支限额:", JTB_CardInfo.overdraftlimit,  3);
    print_buffer("交易金额:", JTB_CardInfo.trademoney,  4);
    LOGI("交易类型: %d\n", JTB_CardInfo.tradetype);
    print_buffer("终端机编号:", JTB_CardInfo.deviceNO,  6);
    print_buffer("交易日期:", JTB_CardInfo.tradedate,  4);
    print_buffer("交易时间:", JTB_CardInfo.tradetime,  3);

    LOGI("\n001A文件\n");
    LOGI("应用锁定标志 0-应用没锁，1-应用已锁: %d\n", JTB_CardInfo.applockflag);
    print_buffer("交易流水号:", JTB_CardInfo.tradeserialnumber,  8);
    LOGI("交易状态     0-初始值，1-上车，2-下车: %d\n", JTB_CardInfo.tradestate);
    print_buffer("上车城市代码:", JTB_CardInfo.getoncitycode,  2);
    print_buffer("上车机构标识:", JTB_CardInfo.getonissuerlabel,  8);
    print_buffer("上车运营商代码:", JTB_CardInfo.getonoperatorcode,  2);
    print_buffer("上车线路号:", JTB_CardInfo.getonline,  2);
    LOGI("上车站点: %d\n", JTB_CardInfo.getonstation);
    print_buffer("上车车辆号ASCII:", JTB_CardInfo.getonbus,  8);
    print_buffer("上车终端编号BCD", JTB_CardInfo.getondevice,  8);
    print_buffer("上车时间", JTB_CardInfo.getontime,  7);
    print_buffer("标注金额,用于逃票追缴", JTB_CardInfo.markamount,  4);
    LOGI("方向标识 AB-上行，BA-下行: %d\n", JTB_CardInfo.directionflag);
    print_buffer("下车城市代码", JTB_CardInfo.getoffcitycode,  4);
    print_buffer("下车机构标识", JTB_CardInfo.getoffissuerlabel,  8);
    print_buffer("下车运营商代码", JTB_CardInfo.getoffoperatorcode,  2);
    print_buffer("下车线路号", JTB_CardInfo.getoffline,  2);
    LOGI("下车站点: %d\n", JTB_CardInfo.getoffstation);
    print_buffer("下车车辆号ASCII", JTB_CardInfo.getoffbus,  8);
    print_buffer("下车终端编号BCD", JTB_CardInfo.getoffdevice,  8);
    print_buffer("下车时间", JTB_CardInfo.getofftime,  7);
    print_buffer("交易金额", JTB_CardInfo.tradeamount,  4);
    print_buffer("乘车里程", JTB_CardInfo.ridedistance,  2);

    LOGI("\n001E文件\n");
    LOGI("交易类型: %d\n", JTB_CardInfo.tradetype1E);
    print_buffer("终端编号", JTB_CardInfo.deviceNO1E,  8);
    LOGI("行业代码 01-公交: %d\n", JTB_CardInfo.industrycode1E);
    print_buffer("线路", JTB_CardInfo.line1E,  2);
    print_buffer("站点", JTB_CardInfo.station1E,  2);
    print_buffer("运营代码", JTB_CardInfo.operatorcode,  2);
    print_buffer("交易金额", JTB_CardInfo.trademoney1E,  4);
    print_buffer("交易后余额", JTB_CardInfo.tradeaftermoney1E,  4);
    print_buffer("交易日期时间", JTB_CardInfo.tradetime1E,  7);
    print_buffer("受理方城市代码", JTB_CardInfo.acceptorcitycode,  2);
    print_buffer("受理方机构标识", JTB_CardInfo.acceptorissuerlabel,  8);

    LOGI("\n0005应用基本信息文件\n");
    LOGI("应用类型标识 00未启用 01启用: %d\n", JTB_CardInfo.appflag);
    LOGI("主卡类型 01普通 02学生 03老人 04测试 05军人...: %d\n", JTB_CardInfo.mastercardtype);

    LOGI("\n月票0015文件\n");
    LOGI("应用启用标志 00未启用 01启用: %d\n", JTB_CardInfo.yappflag);
    LOGI("城市代码: %d\n", JTB_CardInfo.ycitycode);
    print_buffer("月票启用日期  YYYYMMDD: ", JTB_CardInfo.ystarttime, 4);
    print_buffer("月票截止日期  YYYYMMDD: ", JTB_CardInfo.yendtime, 4);
    print_buffer("计次基数: ", JTB_CardInfo.basicnum, 2);
    LOGI("月票类型: %d\n", JTB_CardInfo.ycardtype);

    LOGI("\n月票0018记录文件\n");
    print_buffer("交易序号: ", JTB_CardInfo.ytranssn, 2);
    print_buffer("交易前次数: ", JTB_CardInfo.ytransq, 3);
    print_buffer("交易次数: ", JTB_CardInfo.ytrasnum, 4);
    print_buffer("交易终端: ", JTB_CardInfo.ytransdev, 6);
    print_buffer("交易日期: ", JTB_CardInfo.ytransdate, 4);
    print_buffer("交易时间: ", JTB_CardInfo.ytranstime, 3);
    print_buffer("月票余额: ", JTB_CardInfo.ycash, 4);
    LOGI("执行月票标志: %d\n", JTB_CardInfo.yflag);

    LOGI("\n线路卡0015文件\n");
    LOGI("线路卡卡类: %d\n", JTB_CardInfo.lcardtype);
    print_buffer("线路卡启用时间: ", JTB_CardInfo.lstarttime, 4);
    print_buffer("线路卡失效时间: ", JTB_CardInfo.lendtime, 4);
    print_buffer("线路号: ", JTB_CardInfo.lnum, 6);
    LOGI("收费方式: %d\n", JTB_CardInfo.lcmethod);
    LOGI("上行总站数: %d\n", JTB_CardInfo.lupnums);
    LOGI("下行总站数: %d\n", JTB_CardInfo.ldownnums);
    LOGI("包含的卡类数量: %d\n", JTB_CardInfo.lcardtypenums);

    LOGI("\n线路卡0018文件\n");
    print_buffer("年检提前天数: ", JTB_CardInfo.lyearcheck, 2);
    print_buffer("上行总段数: ", JTB_CardInfo.lupsegments, 2);
    print_buffer("下行总段数: ", JTB_CardInfo.ldownsegments, 2);
    print_buffer("卡内余额上限: ", JTB_CardInfo.lbalancelimit, 4);
    print_buffer("票价限额: ", JTB_CardInfo.lticketlimit, 4);
    print_buffer("上行区段起始站标: ", JTB_CardInfo.lupsegstation, 3);
    print_buffer("下行区段起始站标: ", JTB_CardInfo.ldownsegstation, 3);
    print_buffer("余额不多提示金额: ", JTB_CardInfo.linsbalance, 2);
    LOGI("是否空调  01 是00不是: %d\n", JTB_CardInfo.lkongtiaoflag);
    print_buffer("全程月票票价: ", JTB_CardInfo.lyupiaoprice, 2);
    print_buffer("全程普通票价: ", JTB_CardInfo.lputongprice, 2);
    LOGI("互通卡折率: %d\n", JTB_CardInfo.luniondisrate);
    LOGI("优惠时段带人折扣率: %d\n", JTB_CardInfo.ldistimeaddrate);
    LOGI("钱包首次换乘折扣率: %d\n", JTB_CardInfo.lwallettransfer1st);
    LOGI("带人折扣率: %d\n", JTB_CardInfo.laddrate);
    LOGI("空调折扣率: %d\n", JTB_CardInfo.lkongtiaorate);

    LOGI("\n线路卡0019文件\n");
    print_buffer("新折扣启用时间: ", JTB_CardInfo.lnewdiscountstarttime, 4);
    print_buffer("新折扣结束时间: ", JTB_CardInfo.lnewdiscountendtime, 4);
    LOGI("新折扣启用标志: %d\n", JTB_CardInfo.lnewdiscountflag);
    LOGI("判断有效期权限: %d\n", JTB_CardInfo.lchecktimeflag);
}

int GetCardBinDataTongrui(unsigned char *buff,unsigned int position)
{
    CARDBIN CardBin;
    int datalen;
    datalen = 7;

    memcpy(buff , &CardBin.pCardBin[datalen*position], datalen);
    return 1;
}

//从buff中取出数据
int GetCardBinData(unsigned char *buff,unsigned int position)
{
    CARDBIN CardBin;
    memcpy(buff , &CardBin.pCardBin[4*position] , 4);
#if 1
    return 1;//比较前3字节
#else
    if(buff[3] == 0xff)
        return 1;//比较前3字节
    else
        return 2;//比较前4字节
#endif
}

/*
************************************************************************
- 函数名称 : unsigned char BinarySearch(unsigned char *buff, unsigned char *lastb)
- 函数说明 : 二分法查找
- 输入参数 : buff-待查找数据
- 输出参数 : 无
- 返回参数 :0-success   1-fail
************************************************************************
*/
unsigned char BinarySearch(unsigned char *buff, unsigned char *lastb)
{
    int i;
    int status = 1,ret;
    unsigned int source,key;
    unsigned char tmp[4];
    int low,high,mid,lowflag,highflag;
    CARDBIN CardBin;

    if(CardBin.num == 0 || CardBin.pCardBin == NULL)
        return 1;
    #if 0
    printf("\nCardBin.num=%d all:\n",CardBin.num);
    for(i=0;i<CardBin.num*4;i++)
        printf("%02x ",CardBin.pCardBin[i]);
    printf("\n");
    
    printf("dest:");
    for(i=0;i<4;i++)
        printf("%02x ",buff[i]);
    printf("\n");
    #endif
    
    lowflag = 0;
    highflag = 0;
    
    low = 0;
    high = CardBin.num - 1;
    while(low <= high)
    {
        memset(tmp , 0 , 4);
        ret = GetCardBinData(tmp , low);
        printf("\n1---ret=%d\n",ret);
        if(ret == 1)
        {
            key = buff[0]<<16|buff[1]<<8|buff[2];
            source = tmp[0]<<16|tmp[1]<<8|tmp[2];
        }
        printf("\n1---key=%x,source=%x\n",key,source);
        
        if(key == source)
        {
            status = 0;
            break;
        }
        else
        {
            memset(tmp , 0 , 4);
            ret = GetCardBinData(tmp , high);
            printf("\n2---ret=%d\n",ret);
            if(ret == 1)
            {
                key = buff[0]<<16|buff[1]<<8|buff[2];
                source = tmp[0]<<16|tmp[1]<<8|tmp[2];
            }
            printf("\n2---key=%x,source=%x\n",key,source);
            if(key == source)
            {
                status = 0;
                break;
            }
            else
            {
                mid = (low + high) / 2;
                memset(tmp , 0 , 4);
                ret = GetCardBinData(tmp , mid);
                printf("\n3---ret=%d\n",ret);
                if(ret == 1)
                {
                    key = buff[0]<<16|buff[1]<<8|buff[2];
                    source = tmp[0]<<16|tmp[1]<<8|tmp[2];
                }
                printf("\n3---key=%x,source=%x\n",key,source);
                if(key == source)
                {
                    status = 0;
                    break;
                }
                else if(key > source)
                {
                    if((mid == low)||(high ==mid))
                        lowflag++;
                    
                    if(lowflag == 2)    
                        break;
                    low = mid;
                }
                else
                {
                    if((mid == low)||(high ==mid))
                        highflag++;
                    
                    if(highflag == 2)    
                        break;
                    high = mid;
                }    
            }    
        }  
    }
    
    if ((!status) && (lastb != NULL)) *lastb = tmp[3];
    
    return(status);
}


unsigned char BinarySearchTongrui(unsigned char *buff,unsigned char *cardtype)
{
    CARDBIN CardBin;

    typedef union
    {
        unsigned char longbuf[8];
        unsigned long long    i;
    } LongLongUnonBinarySearch;

    int i;
    int status = 1,ret;
    unsigned int source,key;
    unsigned char tmp[7];
    int low,high,mid,lowflag,highflag;
    int datalen;
    LongLongUnonBinarySearch source_l, key_l;
    int comparelen;
    datalen = 7;

    if(CardBin.num == 0 || CardBin.pCardBin == NULL)
        return 1;

    lowflag = 0;
    highflag = 0;
    
    low = 0;
    high = CardBin.num - 1;
    for (i=0; i < CardBin.num; i++)
    {
        memset(tmp , 0 , datalen);
        ret = GetCardBinDataTongrui(tmp , i);
        if(ret == 1)
        {
            comparelen = GetCompareLen(tmp);
            printf("comparelen:%d, local kabin:%02x %02x %02x %02x %02x %02x\n", comparelen,
                tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5]);
            if(mystrncmp(buff, tmp, comparelen) == 0)
            {
                //找到
                status = 0;                
                *cardtype = tmp[6];
                
                printf("1111111111111111111111 find a kabing:tmp[6]=%d\n",tmp[6]);
                break;
            }
            else
            {
                //没找到
                continue;
            }
        }

    }
    
    return(status);

}

/*****************************************
void Bcd_To_Asc(unsigned char *Asc, unsigned char *Bcd, unsigned char nlen)
功能 ： BCD －－＞ ASCII
入口参数： data: 转换数据的入口指针
buffer: 转换后数据入口指针
len : 需要转换的长度
返回参数：转换后数据长度
*******************************************/
void Bcd_To_Asc(unsigned char *Asc, unsigned char *Bcd, unsigned char nlen)
{
    unsigned char i;
    for(i = 0; i < nlen/2; i++)
    {
        Asc[2*i] = (Bcd[i]>>4) + '0';
        Asc[2*i+1] = (Bcd[i] & 0x0f) + '0';
    }
}
/*
*************************************************************************************************************
- 函数名称 : int hex_2_ascii(INT8U *INdata, char *buffer, INT16U len)
- 函数说明 : HEX 到 ASCII的转换函数
- 入口参数： INdata
- 输出参数 : buffer
*************************************************************************************************************
*/
int hex_2_ascii(unsigned char *INdata, char *buffer, unsigned int len)
{
    const char ascTable[17] = {"0123456789ABCDEF"};
    char *tmp_p = buffer;
    unsigned int i, pos;

    pos = 0;
    for(i = 0; i < len; i++)
    {
        tmp_p[pos++] = ascTable[INdata[i] >> 4];
        tmp_p[pos++] = ascTable[INdata[i] & 0x0f];
    }
    tmp_p[pos] = '\0';
    return pos;
}

int hex_2_ascii_gps(unsigned char *INdata, char *buffer, unsigned int len)
{
    const char ascTable[17] = {"0123456789ABCDEF"};
    char *tmp_p = buffer;
    unsigned int i, pos;

    pos = 0;
    for(i = 0; i < len; i++)
    {
        tmp_p[pos++] = ascTable[INdata[i] >> 4];
        tmp_p[pos++] = ascTable[INdata[i] & 0x0f];
    }
    tmp_p[pos] = '\0';
    return pos;
}
/*
*************************************************************************************************************
- 函数名称 : unsigned char HEX2BCD(unsigned char hex_data)
- 函数说明 :
- 输入参数 : 无
- 输出参数 : 无
*************************************************************************************************************
*/
unsigned char HEX2BCD(unsigned char hex_data)
{
    unsigned int bcd_data;
    unsigned char temp;
    temp=hex_data%100;
    bcd_data=((unsigned int)hex_data)/100<<8;
    bcd_data=bcd_data|temp/10<<4;
    bcd_data=bcd_data|temp%10;
    temp = (unsigned char)bcd_data;
    return temp;
}

/*
*************************************************************************************************************
- 函数名称 : unsigned char HEX2BCD(unsigned char hex_data)
- 函数说明 : BCD －－ ＞HEX
- 输入参数 : BCD
- 输出参数 : HEX
*************************************************************************************************************
*/
unsigned char BCD2HEX(char bcd_data)
{
    unsigned char temp;
    temp=(((bcd_data>>4)*10)+(bcd_data&0x0f));
    return temp;
}


/*
*************************************************************************************************************
- 函数名称 : void HEX8TOBCD(unsigned int In, unsigned char *Pdata)
- 函数说明 :
- 入口参数：
- 输出参数 :
*************************************************************************************************************
*/
void HEX8TOBCD(unsigned int In, unsigned char *Pdata)
{
    unsigned int iv,i;
    unsigned char BCD[4];//定长8位BCD码
    unsigned char sv[9];
    iv = In;
    sprintf(sv,"%08u",iv);
    for(i=0; i<8; i+=2)
    {
        BCD[i/2]=(sv[i]<<4)|(sv[i+1]&0x0F);
    }
    memcpy(Pdata,BCD,4);
}


/*
*************************************************************************************************************
- 函数名称 : unsigned int  BCDToDec(const unsigned char *bcd, unsigned  char length)
- 函数说明 :
- 入口参数：
- 输出参数 :
*************************************************************************************************************
*/
unsigned int  BCDToDec(const unsigned char *bcd, unsigned  char length)
{
    int tmp;
    unsigned int dec = 0;
    unsigned char i;

    for(i = 0; i < length; i++)
    {
        tmp = ((bcd[i] >> 4) & 0x0F) * 10 + (bcd[i] & 0x0F);
        dec += tmp * pow(100, length - 1 - i);
    }

    return dec;
}

unsigned int CardNumCBD_ascii(unsigned char *buf, unsigned int len, unsigned char *buf1,unsigned int size,unsigned char *ascii)
{
    unsigned int i,j;
    unsigned char *ptmp = ascii;
    if(!buf || !ascii) return 0;    
    for(i = 0; i < len; i++)
    {        
        if(buf[i]==0x5a)
        {
            break;        
        }
    }

    j = buf[++i];
    if((j)&&j<13)
    {
        for(i++;i<len;i++)
        {
            if((buf[i]&0xf0) != 0xf0)
            {
                *ptmp++ = (buf[i]>>4)+0x30;
            }
            else
            {
                break;
            }
            
            if((buf[i]&0x0f) != 0x0f)
            {
                *ptmp++ = (buf[i]&0x0F)+0x30;
                j--;
                if(j == 0)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        for(i = 0; i < size; i++)
        {        
            if(buf1[i]==0x57)
            {
                break;        
            }
        }

        j = buf1[++i];    
        for(i++;i<size;i++)
        {
            if((buf1[i]&0xf0) != 0xd0)
            {
                *ptmp++ = (buf1[i]>>4)+0x30;            
            }
            else
            {
                break;
            }

            if((buf1[i]&0x0f) != 0x0d)
            {
                *ptmp++ = (buf1[i]&0x0F)+0x30;
            }
            else
            {
                break;
            }
            j--;
            if(j == 0)
            {
                break;            
            }
        }        
    }

    
    return (ptmp - ascii);

}


