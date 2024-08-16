#ifndef _CARDLAN_JTB_READCARD_H_
#define _CARDLAN_JTB_READCARD_H_

#include "cpu_card_operation.h"

typedef enum{
    GET_JTB_BIN_0x05,
    GET_JTB_BIN_0x15,
    GET_JTB_BIN_0x16,
    GET_JTB_RECORD_0x17,
    GET_JTB_RECORD_0x18,
    GET_JTB_RECORD_0x1A,
    GET_JTB_RECORD_0x1E,
    GET_JTB_BALANCE,
}E_JTB_CMD;


typedef struct {
    unsigned char tradenumber[2];       //ED/EP联机或脱机交易序号
    unsigned char overdraftlimit[3];    //透支限额
    unsigned char trademoney[4];        //交易金额
    unsigned char tradetype;            //交易类型
    unsigned char deviceNO[6];          //终端机编号
    unsigned char tradedate[4];         //交易日期(终端)
    unsigned char tradetime[3];         //交易时间(终端)
}t_record_0018;

typedef struct {
    unsigned char tradetype1E;             //交易类型
    unsigned char deviceNO1E[8];           //终端编号
    unsigned char tradeserialnumber[8];    //交易流水号
    unsigned char trademoney1E[4];         //交易金额
    unsigned char tradeaftermoney1E[4];    //交易后余额
    unsigned char tradetime1E[7];          //交易日期时间
    unsigned char acceptorcitycode[2];     //受理方城市代码
    unsigned char acceptorissuerlabel[8];  //受理方机构标识
    unsigned char reserve_001E[6];         //本规范预留
}t_record_001E;

typedef struct{
    unsigned char record_id[2];         //记录ID标识
    unsigned char record_len;           //记录长度
    unsigned char app_flag;             //应用有效标识
    unsigned char Connectivity_falg;    //互联互通交易标识
    unsigned char app_lock_flag;        //应用锁定标识(0-应用没有锁定;1-应用锁定) BCD
    unsigned char tradeserial_number[8];//交易流水号
    unsigned char tradeserial_status;   //交易状态
    unsigned char in_city_code[2];         //进闸城市代码
    unsigned char out_city_code[2];        //出闸城市代码
    unsigned char in_issuer_label[8];      //进闸机构标识
    unsigned char out_issuer_label[8];     //出闸机构标识 
    unsigned char in_station[8];           //进闸站点
    unsigned char out_station[8];          //出闸站点
    unsigned char in_TermNo[8];            //进闸终端编号
    unsigned char on_TermNo[8];            //出闸终端编号
    unsigned char in_time[7];              //进闸时间      YYYYMMDDhhhmmss
    unsigned char on_time[7];              //出闸时间      YYYYMMDDhhhmmss
    unsigned char max_consumption_amount[4];    //最大消费金额
    unsigned char reserve[43];                  //预留
}t_record_001A_1;


typedef struct{
    unsigned char record_id[2];         //记录ID标识
    unsigned char record_len;           //记录长度
    unsigned char app_flag;             //应用有效标识
    unsigned char Connectivity_falg;    //互联互通交易标识
    unsigned char app_lock_flag;        //应用锁定标识(0-应用没有锁定;1-应用锁定) BCD
    unsigned char tradeserial_number[8];//交易流水号
    unsigned char tradeserial_status;   //交易状态
    unsigned char in_city_code[2];         //上车城市代码
    unsigned char out_city_code[2];        //下车城市代码
    unsigned char in_issuer_label[8];      //上车机构标识
    unsigned char out_issuer_label[8];     //下车机构标识 
    unsigned char in_station[8];           //上车站点
    unsigned char out_station[8];          //下车站点
    unsigned char in_TermNo[8];            //上车终端编号
    unsigned char on_TermNo[8];            //下车终端编号
    unsigned char in_time[7];              //上车时间      YYYYMMDDhhhmmss
    unsigned char on_time[7];              //下车时间      YYYYMMDDhhhmmss
    unsigned char max_consumption_amount[4];    //最大消费金额
    unsigned char directionflag;                //方向标识
    unsigned char line[2];                      //线路号
    unsigned char bus_number[6];                //车辆号
    unsigned char reserve[34];                  //预留
}t_record_001A_2;



typedef struct{

    //-------交通一卡通卡电子现金应用 start ----------
    //0001 ~ 0004  支付应用专用文件 

    //000B 消费交易明细文件

    //000C 圈存交易明细文件

    //0015 ~ 0019 发卡机构自定义文件

    //-------交通一卡通卡电子现金应用 end ----------


    //-------电子现金应用 和 电子钱包应用 公用文件
    //001A  联互通变长记录文件（0x1A）的交通信息记录
    t_record_001A_1 record_001A_1;          //城市轨道应用信息记录
    t_record_001A_2 record_001A_2;          //公共汽电车应用信息记录
    unsigned char record_001A_3[128];       //城市水上客运应用信息记录
    unsigned char record_001A_4[128];       //出租汽车应用信息记录
    unsigned char record_001A_5[128];       //租赁汽车应用信息记录
    unsigned char record_001A_6[128];       //公共自行车应用信息记录
    unsigned char record_001A_7[112];       //停车收费应用信息记录
    unsigned char record_001A_8[128];       //长途客运应用信息记录
    unsigned char record_001A_9[128];       //轮渡应用信息记录
    unsigned char record_001A_10[128];      //城际铁路应用信息记录
    unsigned char record_001A_11[128];      //民航应用信息记录
    unsigned char record_001A_12[128];      //高速公路收费应用信息记录
    unsigned char record_001A_13[30];       //优惠信息记录
    unsigned char record_001A_14[128];      //城铁应用信息记录
    unsigned char record_001A_15[128];      //本规范预留记录2
    unsigned char record_001A_16[128];      //本规范预留记录3
    unsigned char record_001A_17[128];      //本规范预留记录4
    unsigned char record_001A_18[128];      //本规范预留记录5
    //001E 互联互通循环记录文件
    t_record_001E record_001E[30];

  
    //共用余额文件
    unsigned char  balance[4];            //  余额
    
    //-------- 电子钱包应用 start ------------
    //0015 公共应用信息文件
    unsigned char issuerlabel_0015[8];              //发卡机构标识
    unsigned char apptypelabel_0015[1];             //应用类型标识(01-只有ED,02-只有EP,03-ED和EP都有)
    unsigned char issuerappversion_0015[1];         //发卡机构应用版本
    unsigned char appserialnumber_0015[10];         //应用序列号
    unsigned char appstarttime_0015[4];             //应用启用日期
    unsigned char appendtime_0015[4];               //应用截止日期
    unsigned char FCI_0015[2];                      //发卡机构自定义FCI数据

    //0016 持卡人基本信息文件
    unsigned char CardFlag_0016[1];                 //卡类型标识
    unsigned char Bank_Employees_0016[1];           //本行职工标识
    unsigned char Cardholder_Name_0016[20];         //持卡人姓名
    unsigned char Cardholder_ID_Number_0016[32];    //持卡人证件号码
    unsigned char Cardholder_ID_type_0016[1];       //持卡人证件类型

    //0017 管理信息文件
    unsigned char countrycode_0017[4];              //国家代码
    unsigned char provincecode_0017[2];             //省级代码
    unsigned char citycode_0017[2];                 //城市代码
    unsigned char unioncardtype_0017[2];            //互通卡种 0000-非互通卡，0001-互通卡
    unsigned char cardtype_0017[1];                 //卡类型
    unsigned char reserve_0017[49];                 //预留

    //0017预留
    unsigned char settlenumber_0017[4];             //结算单元编号

    //0018 交易明细文件
    t_record_0018 record_0018[10];

    //0005 ~ 0008, 0019 发卡机构自定义文件
    //0005 应用控制信息文件
    unsigned char RFU_1_0005[2];                //RFU
    unsigned char RFU_2_0005[2];                //RFU
    unsigned char industry_code_0005[2];        //行业代码 BCD
    unsigned char Card_Version_0005[2];         //卡片版本号
    unsigned char appflag_0005[1];              //应用类型标识 00未启用 01启用
    unsigned char Card_flag_0005[1];            //卡类型标识位
    unsigned char Connectivity_falg_0005[2];    //互联互通标识（参与互通城市的标识）
    unsigned char reserve_0005_1[8];            //预留
    unsigned char reserve_0005_2[4];            //预留
    unsigned char reserve_0005_3[4];            //预留
    unsigned char main_card_type_0005[1];          //卡类型标识位 01普通 02学生 03老人 04测试 05军人 11成人月票 12老人免费 13老人优惠 14成人季票 15成人年票 70司机 80线路 81脱机采集
    unsigned char sub_card_type_0005[1];           //卡子类型 预留
    unsigned char deposit_0005[1];                 //押金 HEX单位：元
    unsigned char Date_Of_Annual_Survey_0005[4];     //年检日期
    unsigned char Business_bitmap_0005[4];           //业务位图

    //0019 本地复合交易记录文件
    unsigned record_0019_1[48];              //互联互通复合交易
    unsigned record_0019_2[32];              //本地应用
    unsigned record_0019_3[32];              //公共自行车专用
    unsigned record_0019_4[64];              //预留
    unsigned record_0019_5[16];              //预留
    unsigned record_0019_6[16];              //预留
    unsigned record_0019_7[32];              //预留
    unsigned record_0019_8[32];              //公交专用
    unsigned record_0019_9[48];              //互联互通复合交易


    //0006 预留信息文件
    unsigned char reserve_0006[64]; 
    //-------- 电子钱包应用 end ------------

    //-------- 公交月票钱包应用 start-----------
    //0000 密钥文件
    //0002 电子钱包
    //0015 公共应用基本文件
    //0018 交易明细记录文件
    //0017 复合交易记录文件

    //-------- 公交月票钱包应用 end-----------


}JTB_CPUCARD_info_t;



typedef struct
{
    unsigned char IsLocalCard;                    //是否本地卡 1-本地卡 2-外地卡
    unsigned char CSN[4];                        //物理卡号
    unsigned char beforemoney[4];                //交易前余额
    unsigned char thismoney[4];                    //本次交易金额
    unsigned char aftermoney[4];                //交易后余额
    unsigned char offlineSN[2];                    //脱机交易序号
    unsigned char overdraftAmount[3];            //透支限额
    unsigned char keyVersion;                    //密钥版本号
    unsigned char arithmeticLabel;                //算法标识
    unsigned char PRandom[4];                    //伪随机数
    unsigned char TAC[4];                        //TAC
    unsigned char MAC2[4];                        //MAC2
    unsigned char DESCRY[4];

    //分段控制信息
    unsigned char enterexitflag;                //上下车标志

    //PSAM卡数据
    unsigned char PSAMOfflineSN[4];                //终端脱机交易序号
    unsigned char MAC1[4];                        //MAC1
    unsigned char PSAMRandom[8];                //PSAM卡随机数

    //0015文件
    unsigned char issuerlabel[8];                //发卡机构标识
    unsigned char apptypelabel;                    //应用类型标识(01-只有ED,02-只有EP,03-ED和EP都有)
    unsigned char issuerappversion;                //发卡机构应用版本
    unsigned char appserialnumber[10];            //应用序列号
    unsigned char appstarttime[4];                //应用启用日期
    unsigned char appendtime[4];                //应用截止日期

    //0016文件
    unsigned char CertiID[18];                    //司机卡证件号码
    unsigned char drivername[20];                //司机卡姓名
    
    //0017文件
    unsigned char countrycode[4];                //国家代码
    unsigned char provincecode[2];                //省级代码
    unsigned char citycode[2];                    //城市代码
    unsigned char unioncardtype[2];                //互通卡种 0000-非互通卡，0001-互通卡
    unsigned char cardtype;                        //卡类型
    unsigned char settlenumber[4];                //结算单元编号

    //0018文件
    unsigned char tradenumber[2];                //ED/EP联机或脱机交易序号
    unsigned char overdraftlimit[3];            //透支限额
    unsigned char trademoney[4];                //交易金额
    unsigned char tradetype;                    //交易类型
    unsigned char deviceNO[6];                    //终端机编号
    unsigned char tradedate[4];                    //交易日期
    unsigned char tradetime[3];                    //交易时间
    
    //001A记录文件
    unsigned char applockflag;                    //应用锁定标志 0-应用没锁，1-应用已锁
    unsigned char tradeserialnumber[8];            //交易流水号
    unsigned char tradestate;                    //交易状态     0-初始值，1-上车，2-下车
    unsigned char getoncitycode[2];                //上车城市代码
    unsigned char getonissuerlabel[8];            //上车机构标识
    unsigned char getonoperatorcode[2];            //上车运营商代码
    unsigned char getonline[2];                    //上车线路号
    unsigned char getonstation;                    //上车站点
    unsigned char getonbus[8];                    //上车车辆号ASCII
    unsigned char getondevice[8];                //上车终端编号BCD
    unsigned char getontime[7];                    //上车时间
    unsigned char markamount[4];                //标注金额,用于逃票追缴
    unsigned char directionflag;                //方向标识 AB-上行，BA-下行
    unsigned char getoffcitycode[2];            //下车城市代码
    unsigned char getoffissuerlabel[8];            //下车机构标识
    unsigned char getoffoperatorcode[2];        //下车运营商代码
    unsigned char getoffline[2];                //下车线路号
    unsigned char getoffstation;                //下车站点
    unsigned char getoffbus[8];                    //下车车辆号ASCII
    unsigned char getoffdevice[8];                //下车终端编号BCD
    unsigned char getofftime[7];                //下车时间
    unsigned char tradeamount[4];                //交易金额
    unsigned char ridedistance[2];                //乘车里程

    //001E记录文件
    unsigned char tradetype1E;                    //交易类型
    unsigned char deviceNO1E[8];                //终端编号
    unsigned char industrycode1E;                //行业代码 01-公交
    unsigned char line1E[2];                    //线路
    unsigned char station1E[2];                    //站点
    unsigned char operatorcode[2];                //运营代码
    unsigned char trademoney1E[4];                //交易金额
    unsigned char tradeaftermoney1E[4];         //交易后余额
    unsigned char tradetime1E[7];                //交易日期时间
    unsigned char acceptorcitycode[2];            //受理方城市代码
    unsigned char acceptorissuerlabel[8];        //受理方机构标识

    //0005应用基本信息文件
    unsigned char appflag;                        //应用类型标识 00未启用 01启用
    unsigned char mastercardtype;               //主卡类型 01普通 02学生 03老人 04测试 05军人 11成人月票 12老人免费 13老人优惠 14成人季票 15成人年票 70司机 80线路 81脱机采集

    //月票0015文件
    unsigned char yappflag;                        //应用启用标志 00未启用 01启用
    unsigned char ycitycode;                    //城市代码
    unsigned char ystarttime[4];                //月票启用日期  YYYYMMDD
    unsigned char yendtime[4];                  //月票截止日期  YYYYMMDD
    unsigned char basicnum[2];                  //计次基数
    unsigned char ycardtype;                    //月票类型

    //月票0018记录文件
    unsigned char ytranssn[2];                    //交易序号
    unsigned char ytransq[3];                   //交易前次数
    unsigned char ytrasnum[4];                  //交易次数
    unsigned char ytransdev[6];                 //交易终端
    unsigned char ytransdate[4];                //交易日期
    unsigned char ytranstime[3];                //交易时间
    unsigned char ycash[4];                     //月票余额
    unsigned char yflag;                        //执行月票标志

    //线路卡0015文件
    unsigned char lcardtype;                    //线路卡卡类
    unsigned char lstarttime[4];                //线路卡启用时间
    unsigned char lendtime[4];                    //线路卡失效时间
    unsigned char lnum[6];                        //线路号
    unsigned char lcmethod;                        //收费方式 
    unsigned char lupnums;                        //上行总站数
    unsigned char ldownnums;                    //下行总站数
    unsigned char lcardtypenums;                //包含的卡类数量

    //线路卡0018文件
    unsigned char lyearcheck[2];                //年检提前天数
    unsigned char lupsegments[2];                //上行总段数
    unsigned char ldownsegments[2];                //下行总段数
    unsigned char lbalancelimit[4];                //卡内余额上限
    unsigned char lticketlimit[4];                //票价限额
    unsigned char lupsegstation[3];                //上行区段起始站标
    unsigned char ldownsegstation[3];            //下行区段起始站标
    unsigned char linsbalance[2];                //余额不多提示金额
    unsigned char lkongtiaoflag;                //是否空调  01 是00不是 
    unsigned char lyupiaoprice[2];                //全程月票票价
    unsigned char lputongprice[2];                //全程普通票价
    unsigned char luniondisrate;                //互通卡折率
    unsigned char ldistimeaddrate;                //优惠时段带人折扣率
    unsigned char lwallettransfer1st;            //钱包首次换乘折扣率
    unsigned char laddrate;                        //带人折扣率
    unsigned char lkongtiaorate;                //空调折扣率 

    //线路卡0019文件
    unsigned char lnewdiscountstarttime[4];        //新折扣启用时间
    unsigned char lnewdiscountendtime[4];          //新折扣结束时间
    unsigned char lnewdiscountflag;                   //新折扣启用标志
    unsigned char lchecktimeflag;                   //判断有效期权限
}JTB_CPU_CardInfo;


unsigned char Card_JudgeDate_jiaotong(void);
int  Read_JTB_CPU_Card_info(unsigned char *ar_file_index,unsigned char file_index_len, JTB_CPUCARD_info_t *p_card_info);
unsigned char ReadCardInfor_CPU(JTB_CPU_CardInfo *JTB_CardInfo);


#endif
