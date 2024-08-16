#ifndef PBOC_RECORD_HEAD_H
#define PBOC_RECORD_HEAD_H


#define RECV_BUF_SIZE_HAICHEEN    2048
#define PORT_HAICHEEN             80


#define ATC_TAG         0x01


//#define DEBUG_PRINTF

#ifdef DEBUG_PRINTF
#define DebugPrintf(format, ...) fprintf(stdout,"[%s][%s][%d]"format,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__)
#else
#define DebugPrintf(format, ...)
#endif

/*
struct strPBOCKey{
   unsigned char BaseKey[32];         //母pos机灌下来的秘钥
   unsigned char DecryptKey[32];      //用签到返回的秘钥进行解密的明文秘钥
};




struct strDataTime{
   unsigned char year[3];
   unsigned char month[3];
   unsigned char day[3];
   unsigned char hour[3];
   unsigned char min[3];
   unsigned char sec[3];
};

struct SignContainerReply{
   char strSernialNo[16];      //POS终端交易流水
   struct strDataTime time;   //受理的时间
   char AcceptorFlag[16];      //受理方标识
   char RetrievalNo[32];      //检索参考号
   char ReplyCode[8];         //应答码
   char DeviceNo[16];         //终端机编号
   char strTCode[32];         //受卡方标识
   char TradeType[8];         //交易类型
   char strBatchNum[8];      //批次号
   int NetManageCode;         //网络管理信息码
   char DeviceKeyBuf[64];      //终端密钥
};
*/

struct strPBOCKey{
   unsigned char BaseKey[32];         //母pos机灌下来的秘钥
   unsigned char DecryptKey[60];      //用签到返回的秘钥进行解密的明文秘钥
};


struct strDataTime{
   
   unsigned char month;
   unsigned char day;
   unsigned char hour;
   unsigned char min;
   unsigned char sec;
};


struct SignContainerReply{
   char strSernialNo[3];      //POS终端交易流水
   struct strDataTime time;   //受理的时间
   char AcceptorFlag[4];      //受理方标识
   char RetrievalNo[12];      //检索参考号
   char ReplyCode[2];         //应答码
   char ReplayDescri[32];      //应答描述信息
   char DeviceNo[8];         //终端机编号
   char strTCode[15];         //受卡方标识
   char userdefine[6];         //   自定义内容
   //char netmange[2];         //  网络管理码   
   char DeviceKeyBuf[60];      //终端密钥
};

struct ConsumeContainerReply{
   char MainNo[10];         //主账号
   char ProcCode[3];         //交易处理码
   char MaxMoney[6];         //终端可输入最大金额
   char FlowNo[3];         //受理方系统跟踪号
   char Time[3];            //受理方时间
   char Date[2];            //受理方日期
   char EffeTime[2];         //卡有效期
   char BetTime[2];         //清算日期
   char SeriCode[2];      //服务点输入方式码
   char ConCode;         //服务点条件码
   char strTCode[6];         //受卡方标识
   char CheckNo[12];         //检索参考码   
   char ReplyCode[2];         //应答码
   char DeviceNo[8];         //终端机编号
   char CostCode[15];      //商户代码
   char Reparea[25];         //收单机构
   char Currencycode[3];   //货币代码   
   char Carddata[255];     //IC卡数据域
   char Userdefine60[9];         //   自定义内容
   char Userdefine63[123];      //终端密钥
   char Answerdes[95];         //应答描述
   char Mac[8];
    char QRvocher[20];               //二维码交易凭证
};


struct tag{
   char buf[32];
   unsigned char len;
};

struct PbocTagContainer{
   struct tag Tag_57;
   struct tag Tag_5F24;
   struct tag Tag_9F26;
   struct tag Tag_9F27;
   struct tag Tag_9F10;
   struct tag Tag_9F37;
   struct tag Tag_9F36;
   struct tag Tag_95;
   struct tag Tag_9A;
   struct tag Tag_9C;
   struct tag Tag_9F02;
   struct tag Tag_5F2A;
   struct tag Tag_82;
   struct tag Tag_9F1A;
   struct tag Tag_9F03;
   struct tag Tag_9F33;
   struct tag Tag_9F34;
   struct tag Tag_9F35;
   struct tag Tag_9F1E;
   struct tag Tag_84;
   struct tag Tag_9F09;
   struct tag Tag_9F41;
   struct tag Tag_9F74;
   struct tag Tag_8A;
   struct tag Tag_5F34;
   struct tag Tag_5A;
   struct tag Tag_9F63;
};

//上传记录需要用到的数据的集合，所有的数据均预留一个字节
struct UploadRecodContainer{
   unsigned char TradeSum[7];      //交易金额
   unsigned char AuditNum[4];       //受卡方系统跟踪号
   unsigned char AvailableData[3];   //有效期
   unsigned char AcceptDevID[9];            //受卡机终端标识码
   unsigned char AcceptOrganizationID[16];      //受卡方标识码
   unsigned char MoneyType[4];               //货币交易类型，固定值 156
   unsigned char BatchCode[4];               //批次号
   unsigned char CreditCompanyCode[4];         //批次号
   unsigned char UploadMac[16];            //计算获取的mac
};



struct UploadRecodContainerReply{
   unsigned char AcceptDevID[32];            //受卡机终端标识码
   unsigned char AcceptOrganizationID[32];      //受卡方标识码
   unsigned int ReplayCoade;               //返回的标识码
   unsigned int UploadMAC[32];               //返回的标识码
};

//批结算信息
struct SettleContainer{                  
   unsigned char strSernialNo[16];            //受卡方系统跟踪号
   unsigned char AcceptDevID[32];            //受卡机终端标识码
   unsigned char AcceptOrganizationID[32];      //受卡方标识码
   unsigned char PrivateData[64];            // 自定义私有数据
   unsigned char MoneyType[4];               //货币交易类型，固定值 156
   unsigned char CustomLen_60[4];            //60 域的长度
   unsigned char BatchCode[16];            //批次号
   unsigned char NetManageCode[4];            //网络管理信息码
   unsigned char OperCode[16];               //操作员编码
};



/* 刷卡错误的时候临时保存数据的结构体 */

struct SaveSwipeCardErrorData {      
   char Buf5A[150];
   char Buf57[40];
   unsigned int BankAmount;
   char ATC_buf[4];
   unsigned char AFL_last[4];
   unsigned char CardCsnB[4];
   unsigned char CardCsnB_blk[8];
};



#endif

