#ifndef __CARDLAN_STANDARDBUS_TYPE_H__
#define __CARDLAN_STANDARDBUS_TYPE_H__

#include "typea.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>


#define HANDLE_OK_CONSUME        2     /*消费成功*/
#define HANDLE_OK_INTERCHANGE_TIME   1 /*换乘时间*/
#define HANDLE_OK                0
#define HANDLE_ERR              -1
#define HANDLE_ERR_ACTION       -2
#define HANDLE_ERR_CMD          -3
#define HANDLE_ERR_MAC          -4
#define HANDLE_ERR_FORMAT       -5            //卡片格式进程标志错误
#define HANDLE_ERR_NOT_CONTINUE -6
#define HANDLE_ERR_OVER_LIMIT   -7
#define HANDLE_ERR_OVERDUE      -8
#define HANDLE_ERR_CONSUME_TIME -9
#define HANDLE_ERR_BLACKLIST_CARD     -10
#define HANDLE_ERR_NOT_SUPPORT  -11
#define HANDLE_ERR_NOT_ENOUGH   -12            //金额不够
#define HANDLE_ERR_NOT_ALLOW    -13
#define HANDLE_ERR_NOT_FOUND    -13
#define HANDLE_ERR_NOT_IMPLEMENTS     -14
#define HANDLE_ERR_PARAM_LIST   -16
#define HANDLE_ERR_ARG_INVAL    -17  
#define HANDLE_ERR_PATH         -19
#define HANDLE_ERR_ADDRESS      -20
#define HANDLE_ERR_FULL         -21
#define HANDLE_ERR_AUTH         -23
#define HANDLE_ERR_SWIPE_CARD   -24
#define HANDLE_ERR_RECORD_PATH  -25
#define HANDLE_ERR_UPDATE       -28
#define HANDLE_ERR_UNKNOW_APP   -29
#define HANDLE_ERR_UNKNOW_STATUS     -40
#define HANDLE_ERR_READ_CARD    -41
#define HANDLE_ERR_OPEN         -44
#define HANDLE_ERR_NOT_FOUND_APP     -45
#define HANDLE_ERR_NOT_FOUND_PSAM    -46
#define HANDLE_ERR_LOCK_APP     -47
#define HANDLE_ERR_FORMAT_TIME  -48
#define HANDLE_ERR_START_TIME   -49
#define HANDLE_ERR_FLAG         -50
#define HANDLE_ERR_COSUME       -54
#define HANDLE_ERR_REFUSE       -60
#define HANDLE_ERR_NOT_CONNECT  -66   
#define HANDLE_ERR_NOT_RESPOND  -67 
#define HANDLE_ERR_TIMEOUT      -70     

#define        MI_OK                   0x00
#define        MI_FAIL                 0x01

typedef union
{
    unsigned char buff[8];
    unsigned long long  i;
}BigUnion;

typedef union
{
    unsigned char intbuf[2];
    unsigned short i;
} ShortUnon;

typedef union
{
    unsigned char longbuf[4];
    unsigned int  i;
} LongUnon;


typedef  struct
{
    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
    unsigned char weekday;
} SysTime;

typedef  struct
{
    unsigned  char SationNum[2];              //站台个数
    unsigned  char DeductMoney[4];              //预扣金额
    unsigned  char DeductTime[2];              //上限时间
    unsigned  char Enable;          //启用标志
    unsigned  char Enableup;        //启用标志
    unsigned  char StationOn;
    unsigned  char Updoor;          //前后门，上下定义
    unsigned  char Updown;          //上下行定义
    unsigned  char SationNow;
    unsigned  char Sationdis;        //站号
    unsigned  char Sationkey;
    unsigned  char Linenum[6];            //线路号,高字节在前，低字节在后
}SectionFarPar;

typedef  struct
{
    unsigned char FlagValue;            //钱包指示:现金钱包
    unsigned char SOneZero[16];
    unsigned char SOneOne[16];
    unsigned char SOneTwo[16];
    unsigned char STwoZero[16];
    unsigned char STwoOne[16];
    unsigned char STwoTwo[16];
    unsigned char SThrZero[16];
    unsigned char SThrOne[16];
    unsigned char SThrTwo[16];
    unsigned char SForZero[16];
    unsigned char SForOne[16];
    unsigned char SForTwo[16];
    unsigned char SFivZero[16];
    unsigned char SFivOne[16];
    unsigned char SFivTwo[16];
    unsigned char SEigZero[16];
    unsigned char SEigOne[16];
    
    //unsigned char SNinZero[16];
    unsigned char SSevnBuf[16];
} JackRegal;

typedef  struct
{
    unsigned char RFIccsn[4];        //0芯片系列号码            0-3
    unsigned char RFDtac[4];         //数据区TAC             4-7
    unsigned char RFcsn[8];          //用户卡号               8-15
    unsigned char RFrove[4];         //硬件流水号              16-19    若是CPU卡，则为psam卡交易序号
    unsigned char RFtype;            //5卡类                20-20
    unsigned char RFXFtype;          //6消费类型              21-21    0x00 表示数据 01:预冲 02:冲值 03:转移   04:时间包月   05:次数包月    0x55表示错误代码
    unsigned char RFStationID;       //7站台编号              22-22
    unsigned char RFvalueq[3];       //8交易前卡片金额           23-25
    unsigned char RFvaluej[3];       //9实际交易金额            26-28
    unsigned char RFtime[4];         //10交易时间(UTC秒)       29-32
    unsigned char RFtran;            //11终端类型             33-33    01:公交 02:餐饮 03:会员 04:小额消费1 05：出租车消费   06:脱机充值机   07:固定消费机1 08:时间消费机
    unsigned char RFMoneyJF[4];      //12总积分              34-37    用作发卡机构标示
    unsigned char RFMoneyJFbuf[2];   //13积分数              38-39    用作城市代码
    unsigned char RFtac[3];          //14卡片操作次数           40-42
    unsigned char RFpurse;           //15交易钱包类型           43-43    0.次数 1.补贴 2.现金      8 时间包月
    unsigned char RFvalueh[3];       //16交易后余额            44-46
    unsigned char RFvaluey[3];       //17原交易金额            47-49
    unsigned char RFtimeno;          //18时间编号             50-50
    unsigned char RFderno[4];        //19终端机号             51-54
    unsigned char RFEnterOrExit;     //20进出标志             55-55   (0进  1表示出)
    unsigned char RFcarp;            //21上下行标志            56-56    0为上行 1为下行
    unsigned char RFoperator[4];     //22操作员              57-60    57-58为psam卡前2字节，59-60为操作员编号
    unsigned char RFflag;            //23卡标志              61-61    0.M1卡 1.PBOC2.0标准CPU卡 2:PBOC3.0标准CPU卡  61
    unsigned char RFspare;           //24交易类型             62-62   (定额:0 自由:1 分段:2 计时:3)
    unsigned char RFXor;             //25效验位 水平效验         63-63
} RecordFormat;

/*
    主记录结构体最大不超过512
*/
typedef struct YLmain_Record_{

    /*公共部分比传数据*/
    unsigned char CardType;             //卡片类型：1银行卡，2二维码 ，3M1卡 ,4普通cpu卡,5交通部cpu卡
    unsigned char LogiCardType;         //逻辑卡类
    unsigned char CardAttribution[2];    //卡片归属asc：'01' 银联二维码,'02'微信二维码,'03'支付宝二维码, '04'卡联二维码, '05'卡联自发卡，'06' 交通部
    unsigned char TransType;            //交易类型: hex 01 一票制消费,02冲正 ,03分段消费 04分段补回
    unsigned char Channel;              //通道 ，hex 01 双免，02oda，03电子现金 04 卡联在线消费   05 已上送的电子现金交易
    unsigned char BatchNum[3];          //批次号   ，bcd
    unsigned char SerialNum[4];         //终端交易流水号hex
    unsigned char TransDate[3];         //YYmmdd  bcd
    unsigned char TransTime[3];         //hhmmss  bcd

    unsigned char OriginalBalance[4];    //原余额
    unsigned char ShouldValue[4];       //应交金额
    unsigned char TransValue[4];        //实际交易金额hex
    unsigned char Balance[4];            //余额
    
    unsigned char CSN[4];               //芯片序列号
    unsigned char AcountNum[20+1];        //主账号 asc ,卡号，目前最长不超过20位数字
    unsigned char MchantUnm[15];        //商户号asc
    unsigned char TerminalNum[8];        //终端号asc
    unsigned char LineNum[8+1];         //线路号asc 实际是线路名称
    unsigned char VehicleNum[8+1];        //车辆号asc
    unsigned char DriverID[8+1];        //司机编号hex

    unsigned char PursType;             //钱包类型 hex , M1卡使用 ,0次数钱包，1补贴钱包，2现金钱包
    unsigned char ATC[4];               //卡片交易次数
    unsigned char roundTrip;            //去程返程标记 hex , 分段使用
    unsigned char InOutFlag;            //进出标记 hex , 分段使用
    unsigned char StationNo[2];         //站台编号 hex ,

    /*个性银行卡才有部分*/
    unsigned char CheckNo[12+1];          // asc 消费返回报文37域数据，联机交易，oda交易以及二维码交易都会有该字段
    unsigned char ODASendTime[6];         // oda记录上送时间，联机交易该字段为空 bcd, YYmmddhhmmss
    unsigned char domain55[256];          // 银行卡55域数据项,（该数据块可复用)

    unsigned char sop[123];              //无用数据

}YLmain_Record;



/****************商户配置信息相关定义*****************/

typedef struct merchantconfig_{
    unsigned char TPDU[10+1];            //ascii        
    unsigned char HEAD[12+1];            //ascii
    unsigned char TERMINALNO[8+1];    //终端机号    ascii
    unsigned char MERCHANTNO[15+1];    //           ascii
              int  keyindex;            //主密钥索引
    unsigned char netinteval;        //网络切换的时间间隔，先连接卡联的服务器进行签到，时间间隔后连接银行的

    unsigned char domain[40];        //主机域名地址
    unsigned char ipaddress[20];    //主服务器地址 ascii
             int port;            //端口
    unsigned char domainbak1[40];        //备份域名1
    unsigned char domainbak2[40];        // 备份域名2
    unsigned char ipaddressbak1[20];    //     备份ip
    
    
    /*专网通讯参数*/
    unsigned char privateIpaddr[20];        //专网ip
    unsigned char privateIpaddrbak[20];    //专网备份ip
            int     privatePort;                //专网端口
    unsigned char privateuser[40];            // 专网用户名
    unsigned char privatepssw[40];            // 专网密码

            int     comunicateTimout;        //通讯超时时间
            int     linkTimout;        //联网超时时间
    unsigned char PublicNet;        // 公网接入，1使用公网
    unsigned char DNS;            //DNS
    unsigned char APN[64];            //APN

    /*双免相关配置*/
    unsigned char CERTIPATH[128];        //证书路径    ascii
    unsigned char Terminaltype[20];    //终端机型    ascii
    unsigned char Terminalcsn[38];        //硬件序列号  ascii
    unsigned char FID[64];                //厂商标识

    unsigned char InputCode[2];            //输入点方式方式码
    unsigned char TransProcessCode[3];    //交易处理码
    unsigned char ServeConditionCode;        //服务点条件码

            int    ReSwipTimout;        //重刷超时时间     
    unsigned char DriverId[8+1];            //司机号
    unsigned char CompanyInfo[20];    //公司信息
    unsigned char LineNo[8+1];            //线路名称
    unsigned short LineID;                //线路编号    
    unsigned char LicensePlate[8+1];        //车牌号
    
    unsigned char masterkey[16];        //主控密钥，临时的，实际由母pos机灌入
    unsigned char mackey[16];            //运算MAC秘钥

    /*ODA相关配置*/
    unsigned char institution[8];        //机构号，asc ,定长
    unsigned char ODATPDU[10+1];        
    unsigned char ODAHEAD[12+1];        
    unsigned char ODAIP[128];
    int    ODAport;

    /*母pos选择*/
    unsigned char keypostype;            //0 升腾cv190,1使用的是联迪母pos,2使用的是新大陆


    /*Channel mode*/
    unsigned char ODAonly;
    unsigned char FreeSignOnly;
    unsigned char ODAandFree;
    unsigned char Changemode;   //在双免+ODA的自动模式下，在ODA上送成功的情况下(网络畅通)，具备自动切换回双免模式  0  支持自动切换  1 不支持自动切换
    unsigned char oda_type;        //oda模式下的业务类型，0支持姐贷记卡，1只支持借记卡，2只支持贷记卡
    unsigned char free_type;
    unsigned char dual_chanel_type;
    

    /*二维码开启开关*/
    unsigned char wefhatEnable;
    unsigned char alipayEnable;
    unsigned char unionpayEnable;
    unsigned char qrworkmode;           //微信和支付宝交易通道  0 卡联通道   1  银联商务通道

    /*银联和银商功能开关*/
    unsigned char worktype;             //0 银联商务直连,1 银联直连, 2双免用银联+oda


    /*方便测试*/
    unsigned char testmtk[5][16];
    unsigned char testmak[5][16];
    
}MerchantConfig;


/*****************************************************/
#define RECORD_HEAD_LEN     64

typedef struct RECORDFILE_
{
    //---------------------------------------------
    //  文件头64 字节
    unsigned int nSaveNum;
    unsigned int nSendNum;
    unsigned char reserve[RECORD_HEAD_LEN-8];

    //---------------------------------------------
    char filename[64];
    int singgelLen;
    //pthread_mutex_t m_fileLock;
    int dataLen;
    unsigned char *buf;

    /*function internal    */
    int (*InitReocrdfile)(struct RECORDFILE_ *);
    int (*GetSaveAndSendNum)(struct RECORDFILE_ *);
    int (*AddSendNum)(struct RECORDFILE_ *);
    int (*AddSaveNum)(struct RECORDFILE_ *);
    int (*FormatFile)(struct RECORDFILE_ *);
    int (*GetUnsendRecord)(struct RECORDFILE_ *,char *);
    int (*SaveRecord)(struct RECORDFILE_ *,char *);

    int (*GetFileHead)(struct RECORDFILE_ *,char *,int *);
    int (*SetFileHead)(struct RECORDFILE_ *,char *,int );
    int (*GetFileCertainRecord)(struct RECORDFILE_ *,int ,char *,int *);
    int (*SetFileCertainRecord)(struct RECORDFILE_ *,int ,char *);

    int (*FormatFileForSpace)(struct RECORDFILE_ * ,int);       //指定格式化n条记录
}RECORDFILE;


typedef  struct
{
    unsigned char UserIcNo[4];     //开始读
    unsigned char MonthOrCountflag;//包月/包次标志           HEX  OK
    unsigned char SMonth[3];       //启用包月  年月日                      HEX  OK
    unsigned char EMonth[3];       //结束包月  年月日                      HEX  OK
    unsigned char CityId[2];       //城市代码    HEX  OK
    unsigned char AppId[2];        //应用代码    HEX  OK
    unsigned char UserNum[2];      //用户编号    HEX  OK
    unsigned char CardCsnB[4];     //卡号        BCD  OK
    unsigned char CardCsnB_blk[8];
    unsigned char CardId[4];       //卡认证号    HEX  OK
    unsigned char CardType;        //卡类        HEX  OK
    unsigned char CardGroup;        //卡类        HEX  OK
    unsigned char Pwdflag;         //密码启用标志    HEX  OK
    unsigned char EnableH[4];      //启用日期    HEX  OK
    unsigned char Effective[4];    //有效日期    HEX  OK
    unsigned char UserWord[3];     //用户密码    BCD  OK
    unsigned char Views[4];        //次数钱包    HEX  OK
    unsigned char Subsidies[4];    //补贴钱包    HEX  OK
    unsigned char QCash[4];        //现金钱包    HEX  OK
    unsigned char MoneyJack[4];    //积分区域    HEX  OK
    unsigned char ViewsValue[3];   //当次消费金额    HEX  OK
    unsigned char DayValue[3];     //当天消费金额    HEX  OK
    unsigned char MonthValue[4];   //当月消费金额    HEX  OK
    unsigned char Period;          //上次交易的时段        HEX  OK
    unsigned char OldTime[6];      //上次交易时间        HEX  OK
    unsigned char OldTransType;    //上次交易类型        HEX  OK
    unsigned char OldTermNo[4];    //上次交易终端机号    HEX  OK
    unsigned char ViewMoney[4];    //钱包累计交易次数     HEX  OK
    unsigned char EnterExitFlag;   //进出标志                      HEX  OK
    unsigned char StationID;       // 站台编号                     HEX  OK
    unsigned char EnterCarCi;      // 上车人数                   HEX  OK
    unsigned char StationOn;       // 限次限额标准
    unsigned char StationDEC;       // 限次限额标准   
    unsigned char CardTac[4];       // CPU卡返回TAC
    unsigned char CiMoneyFlag;       // 限次限额标准
    unsigned char TimesTransfer[4];
    unsigned char TransCiFlag;
    unsigned char NoDiscountTime[6]; //非转乘优惠打卡所记录的时间
    unsigned char FirstTimeDiscount[6];//记录第一次在转乘优惠区间内打折的时间，临时设置2分钟一次循环，可修改
    //住建部CPU卡普通文件结构//
    unsigned char Appstate;
    unsigned char Appfalw[2];
    unsigned char Appcsn[8];    
    unsigned char CountType;
    unsigned char CAppstate;
    unsigned char CAppTermNo[6];
    unsigned char CAppCash[4];
    unsigned char CAppTime[6];
    unsigned char CardTypebak;

#ifdef PENGXIXIAN_BUS
    unsigned short nOlderConsumeCi;   //  老年卡计次消费的总次数
    unsigned char bOlderConsumeCi;    //  布尔变量，1 计次消费   0 电子现金消费
#endif
    unsigned char timebuf[7];
} CardInform;

/*通过配置文件或网络途径获取其信息*/
typedef struct Base_Info_{
    unsigned char  TPDU[5];                                //一般为 60 00 03 00 00
    unsigned char  HEAD[6];                                //        60 31 00 31 00 00

    unsigned char ODATPDU[5];
    unsigned char ODAHEAD[6];

    unsigned char TermNum[8];                    //终端号        ans 
    unsigned char MerchantNum[15];                //终端商户号    ans
    unsigned char batchnon[3];                    //批次号
    unsigned char netmange[2];                    //
    unsigned char TerCSN[16];                    //终端机序列号

    unsigned char operater[3];

    unsigned char MasterKey[60];            //主密钥 ,母pos机灌入的明文主密钥
    unsigned char MacKey[60];
    unsigned char keytype;                    //密钥类型    01单倍密钥(8byts)  03双倍密钥(16bytes)  04双倍密钥带磁道(16bytes)
    unsigned char keylen;                    //主密钥长度
    unsigned char keyindex;
    
    unsigned char Workingkey[16];            //使用主密钥解密后的工作密钥 ,最长是16字节
    unsigned char Pinkey[16];                //最长是16字节
    unsigned char tdkkey[16];        
}Base_Info;


/*
    串口二维码原始数据缓存,
*/

typedef struct Uart_QRCode_ {
    unsigned char id[256/*MAX_QRCODE_LEN*/];            
    unsigned char length;
    unsigned char status;
    unsigned char type;
    LongUnon  tranNo;
    unsigned char name_len;                //
    unsigned char name[128];            //服务器返回的相关信息

    unsigned char tag[64];                //过滤器添加的标签
}UART_QRCODE;

/*
     二维码控制管理，每次串口接收到一帧数据就需要更新该结构内容
*/
typedef struct QRManager{
    unsigned char g_QRCodeRcvDataFg;        //串口读取到二维码标记
    int QrcodFd;
    char *uartname;   
    unsigned char g_FgQRCodeRcvAck;            //请求返回状态 ,1 接受到后台返回  ，0无返回 ,2 错误，微信与支付宝使用该ack，其他的不一定
}QRMANAGER;

typedef struct
{
    unsigned int num;            //卡bin数目
    unsigned char* pCardBin;        //卡bin数据源地址
}CARDBIN;

#endif  //__CARDLAN_STANDARDBUS_TYPE_H__



