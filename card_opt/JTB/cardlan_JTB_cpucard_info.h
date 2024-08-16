#ifndef __CPU_CARD_INFORMATION_H__
#define __CPU_CARD_INFORMATION_H__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "typea.h"
#include "common/cardlan_StandardBus_tpye.h"


#define     GET_RECORD                 0x0001
#define     SAVE_RECORD                0x0002
#define     CARD_SPEC_CPU_PBCO20        1
#define     CARD_SPEC_CPU_PBCO30        2
#define     CONSUME_MODE_PRESET         0


#define        MAX_BLACK_CONNT         500000
#define        CITYUNION_BL_FILEWHI    "/mnt/record/JWhitelist.sys"            //交通部白名单

#define        MI_OK                   0x00
#define        MI_FAIL                 0x01
#define        DRIVER                  18
#define        SETSECTIONLINE          26    
#define        SETSECTION              19
#define        SETSECTIONUP            21
#define        KONGTIAO                29
#define        TRANSFER                30
#define        PSAMDIS                 1
#define        RECEIVE_LEN             0x12  //读取接收数据长度的命令





typedef struct {

    unsigned char ticketyear;       //月票年限
    unsigned char ticketdate;       //月票时间
    unsigned char tickettype;       //月票类型   0 表示月票   1 表示1季票   2 表示2季票  3 表示3季票  4 表示4季票   5 表示年票 
    ShortUnon blance;               //月票余额
    ShortUnon blanceuse;            //可用月票余额
    unsigned char rechargflag;      //使用基础次数标志
    
}MonthlyTicket;  


typedef struct
{
    unsigned char IsLocalCard;        //是否本地卡 1-本地卡 2-外地卡
    unsigned char CSN[4];            //物理卡号
    unsigned char beforemoney[4];    //交易前余额
    unsigned char thismoney[4];        //本次交易金额
    unsigned char aftermoney[4];        //交易后余额
    unsigned char offlineSN[2];        //脱机交易序号
    unsigned char overdraftAmount[3];    //透支限额
    unsigned char keyVersion;        //密钥版本号
    unsigned char arithmeticLabel;    //算法标识
    unsigned char PRandom[4];        //伪随机数
    unsigned char TAC[4];            //TAC
    unsigned char MAC2[4];            //MAC2
    unsigned char DESCRY[4];
    //分段控制信息
    unsigned char enterexitflag;        //上下车标志
    //PSAM卡数据
    unsigned char PSAMOfflineSN[4];    //终端脱机交易序号
    unsigned char MAC1[4];            //MAC1
    unsigned char PSAMRandom[8];       //PSAM卡随机数
    //0015文件
    unsigned char issuerlabel[8];        //发卡机构标识
    unsigned char apptypelabel;        //应用类型标识(01-只有ED,02-只有EP,03-ED和EP都有)
    unsigned char issuerappversion;    //发卡机构应用版本
    unsigned char appserialnumber[10];    //应用序列号
    unsigned char appstarttime[4];        //应用启用日期
    unsigned char appendtime[4];        //应用截止日期
    //0016文件
    unsigned char CertiID[18];            //司机卡证件号码
    unsigned char drivername[20];        //司机卡姓名
    
    //0017文件
    unsigned char countrycode[4];        //国家代码
    unsigned char provincecode[2];    //省级代码
    unsigned char citycode[2];        //城市代码
    unsigned char unioncardtype[2];    //互通卡种 0000-非互通卡，0001-互通卡
    unsigned char cardtype;            //卡类型
    unsigned char settlenumber[4];    //结算单元编号
    //0018文件
    unsigned char tradenumber[2];    //ED/EP联机或脱机交易序号
    unsigned char overdraftlimit[3];    //透支限额
    unsigned char trademoney[4];        //交易金额
    unsigned char tradetype;            //交易类型
    unsigned char deviceNO[6];        //终端机编号
    unsigned char tradedate[4];        //交易日期
    unsigned char tradetime[3];        //交易时间
    
    //001A记录文件
    unsigned char applockflag;            //应用锁定标志 0-应用没锁，1-应用已锁
    unsigned char tradeserialnumber[8];    //交易流水号
    unsigned char tradestate;                //交易状态     0-初始值，1-上车，2-下车
    unsigned char getoncitycode[2];        //上车城市代码
    unsigned char getonissuerlabel[8];        //上车机构标识
    unsigned char getonoperatorcode[2];    //上车运营商代码
    unsigned char getonline[2];            //上车线路号
    unsigned char getonstation;            //上车站点
    unsigned char getonbus[8];            //上车车辆号ASCII
    unsigned char getondevice[8];            //上车终端编号BCD
    unsigned char getontime[7];            //上车时间
    unsigned char markamount[4];        //标注金额,用于逃票追缴
    unsigned char directionflag;            //方向标识 AB-上行，BA-下行
    unsigned char getoffcitycode[2];        //下车城市代码
    unsigned char getoffissuerlabel[8];        //下车机构标识
    unsigned char getoffoperatorcode[2];    //下车运营商代码
    unsigned char getoffline[2];            //下车线路号
    unsigned char getoffstation;            //下车站点
    unsigned char getoffbus[8];            //下车车辆号ASCII
    unsigned char getoffdevice[8];            //下车终端编号BCD
    unsigned char getofftime[7];            //下车时间
    unsigned char tradeamount[4];        //交易金额
    unsigned char ridedistance[2];            //乘车里程
    //001E记录文件
    unsigned char tradetype1E;            //交易类型
    unsigned char deviceNO1E[8];            //终端编号
    unsigned char industrycode1E;            //行业代码 01-公交
    unsigned char line1E[2];                //线路
    unsigned char station1E[2];            //站点
    unsigned char operatorcode[2];        //运营代码
    unsigned char trademoney1E[4];        //交易金额
    unsigned char tradeaftermoney1E[4]; //交易后余额
    unsigned char tradetime1E[7];            //交易日期时间
    unsigned char acceptorcitycode[2];        //受理方城市代码
    unsigned char acceptorissuerlabel[8];    //受理方机构标识
        //0005应用基本信息文件
        unsigned char appflag;              //应用类型标识 00未启用 01启用
        unsigned char mastercardtype;       //主卡类型 01普通 02学生 03老人 04测试 05军人 11成人月票 12老人免费 13老人优惠 14成人季票 15成人年票 70司机 80线路 81脱机采集
        //月票0015文件
        unsigned char yappflag;             //应用启用标志 00未启用 01启用
        unsigned char ycitycode;            //城市代码
        unsigned char ystarttime[4];        //月票启用日期  YYYYMMDD
        unsigned char yendtime[4];          //月票截止日期  YYYYMMDD
        unsigned char basicnum[2];          //计次基数
        unsigned char ycardtype;            //月票类型
        //月票0018记录文件
        unsigned char ytranssn[2];          //交易序号
        unsigned char ytransq[3];           //交易前次数
        unsigned char ytrasnum[4];          //交易次数
        unsigned char ytransdev[6];         //交易终端
        unsigned char ytransdate[4];        //交易日期
        unsigned char ytranstime[3];        //交易时间
        unsigned char ycash[4];             //月票余额
        unsigned char yflag;                //执行月票标志

    //线路卡0015文件

    unsigned char lcardtype;            //线路卡卡类
    unsigned char lstarttime[4];        //线路卡启用时间
    unsigned char lendtime[4];            //线路卡失效时间
    unsigned char lnum[6];                //线路号
    unsigned char lcmethod;                //收费方式 
    unsigned char lupnums;                //上行总站数
    unsigned char ldownnums;            //下行总站数
    unsigned char lcardtypenums;        //包含的卡类数量

    //线路卡0018文件
    unsigned char lyearcheck[2];        //年检提前天数
    unsigned char lupsegments[2];        //上行总段数
    unsigned char ldownsegments[2];        //下行总段数
    unsigned char lbalancelimit[4];        //卡内余额上限
    unsigned char lticketlimit[4];        //票价限额
    unsigned char lupsegstation[3];        //上行区段起始站标
    unsigned char ldownsegstation[3];    //下行区段起始站标
    unsigned char linsbalance[2];        //余额不多提示金额
    unsigned char lkongtiaoflag;        //是否空调  01 是00不是 
    unsigned char lyupiaoprice[2];        //全程月票票价
    unsigned char lputongprice[2];        //全程普通票价
    unsigned char luniondisrate;        //互通卡折率
    unsigned char ldistimeaddrate;        //优惠时段带人折扣率
    unsigned char lwallettransfer1st;    //钱包首次换乘折扣率
    unsigned char laddrate;                //带人折扣率
    unsigned char lkongtiaorate;        //空调折扣率 

    //线路卡0019文件
    unsigned char lnewdiscountstarttime[4];//新折扣启用时间
    unsigned char lnewdiscountendtime[4];  //新折扣结束时间
    unsigned char lnewdiscountflag;           //新折扣启用标志
    unsigned char lchecktimeflag;           //判断有效期权限
}CardInformCPU;





struct Permisson
{
    unsigned char walletflag;                //钱包权限
    unsigned char yueticketflag;            //月票权限
    unsigned char transferflag;             //换乘权限
    unsigned char sheetvoiceflag;            //让座语音权限
    unsigned char discounttimeflag;            //时段优惠权限
    unsigned char walletaddflag;            //钱包带人权限
    unsigned char yueticketaddflag;            //月票带人权限    
};

typedef struct {
    unsigned char ltransferstarttime[4];   //换乘启用时间
    unsigned char ltransferendtime[4];       //换乘结束时间
    unsigned char ltransferflag;           //换乘启用标识
    unsigned char ltransfertime[2];           //换乘有效时长
    unsigned char ltransferdevsec[2];       //换乘偏差秒数

}Transfer;

typedef  struct
{
    unsigned char dat[4];
}WhiteItem;

typedef  struct
{
    WhiteItem *buf;
    unsigned int count;
}st_WhiteFile;



typedef  struct
{
    unsigned char RFIccsn[4];       //0芯片系列号码          0-3
    unsigned char RFDtac[4];        //数据区TAC               4-7
    unsigned char RFcsn[8];         //用户卡号              8-15
    unsigned char RFrove[4];        //硬件流水号             16-19  若是CPU卡，则为psam卡交易序号
    unsigned char RFtype;           //5卡类                  20-20
    unsigned char CetiID[18];       //身份证号            21-38 ASCII码
    unsigned char RFtime[4];        //10交易时间(UTC秒)    39-42    
    unsigned char RFMoneyJF[4];     //12总积分            42-45    用作发卡机构标示
    unsigned char RFMoneyJFbuf[2];  //13积分数            46-47    用作城市代码
    unsigned char RFtac[3];         //14卡片操作次数          48-50    
    unsigned char RFderno[4];       //19终端机号              51-54
    unsigned char RFEnterOrExit;    //20进出标志              55-55(0进  1表示出)    
    unsigned char RFoperator[4];    //22操作员            57-60       57-58为psam卡前2字节，59-60为操作员编号
    unsigned char RFflag;           //23卡标志               61-61    0.M1卡 1.PBOC2.0标准CPU卡 2:PBOC3.0标准CPU卡  61
    unsigned char RFspare;          //24交易类型              62-62 (定额:0 自由:1 分段:2 计时:3)
    unsigned char RFXor;            //25效验位 水平效验    63-63
} DriverRecordFormat;

#endif  //__CPU_CARD_INFORMATION_H__


