#ifndef __TERMAPP_TERSTRUCT__
#define __TERMAPP_TERSTRUCT__


typedef struct pos_commstrc {
    
	long  tr_code;			/*  交易代码            N(6)    */
	long  amt;				/*  发生额              N(9)    */
	long  tipamt;
	long  qsamt;			/*  卡帐户余额          N(9)    */
	long  pos_ser;			/*  POS 流水号          N(6)    */
	long  batchNumber; 		/*批次号*/
	long  oldBatchNumber;   /*原批次号*/
	uchar  card_no[21];		/*  卡号                S(20)   */
	uchar  time[7];			/*  交易时间            S(6)    */
	uchar  date[9];			/*  交易日期            S(8)    */
	uchar  old_autdate[5];	/*  原交易日期                  */
	uchar  enter_mode[4];	/*  输入方式                    */
	uchar  host_ser[13];		/*  主机流水号          N(6)    */
	uchar  authno[7];		/*  授权号              S(6)    */
	uchar  rejcode[3];       /*  交易返回码              */
	int   trans_no;			/*  交易类型                    */
	int   oldtrans_no;

	uchar  pos_id[9];		/*  POS 终端号          N(8)    */
	uchar  unit_num[16];		/*  特约单位号          N(8)   */
	uchar  pin[9];			/*  个人密码                    */
	uchar  oldpos_ser[7];
	uchar  expiry[5];		/*  有效期     */
	uchar  tellerNumber[3];
	uchar  pinLen;
	uchar  wkUnit[4];
	uchar  authMode[3];
	uchar  authUnit[12];
	uchar  issuerResp[21];
	uchar  centerResp[21];
	uchar  recvBankResp[21];
	uchar  issuerBankId[9];
	uchar  recvBankId[9];
	uchar  centerId[9];
	uchar  rmbSettleFlag; /*内卡结算返回码*/
	uchar  frnSettleFlag;/*外卡卡结算返回码*/
	uchar  netInfoCode[4];
	uchar  prtOldPosSer[7]; /*原交易凭证号,用于打印*/
	uchar  prtOldAuthNo[7]; /*原交易授权号,用于打印*/
	uchar  prtOldHostSer[13]; /*原交易检索号,用于打印*/
	uchar	SwipeData2[TRACK2_LEN+1];
	uchar	SwipeData3[TRACK3_LEN+1];

	// EMV交易增加域
	uchar  iccdata[255];
	uchar  iccdatalen;
	uchar  dup_iccdata[255];
	uchar  dup_iccdatalen;
	uchar  card_seq;
	uchar  ic_online_id;
	uchar  fallback;
	uchar  tc[9];
	uchar  write_id;
	uchar  prt_id;
	uchar  AuthRespCode[3];
	uchar  TransResult;
	uchar  TVR[6];
	uchar  AID[32];
	uchar  applable[32];
	uchar  TSI[3];
	uchar  ATC[3];
}pos_commstr;
#define COM_SIZE  sizeof(struct pos_commstrc)

typedef struct _total_strc {	
	int	rmbCreditNumber;	/*人民币贷记笔数*/	  
	int	rmbDebitNumber;		/*人民币借纪录笔数*/
	int	frnCreditNumber;	/*外卡贷记笔数*/
	int frnDebitNumber;		/*外卡借记笔数*/

	int rmbSaleNumber;		/*消费笔数(未撤销)*/
	int rmbConfirmNumber;	/*授权完成笔数(未撤销)*/
	int rmbRefundNumber;	/*退货笔数(未撤销)*/
	int rmbOffNumber;		/*离线笔数(未撤销)*/
	
	int frnSaleNumber;		/*消费笔数(未撤销)*/
	int frnConfirmNumber;	/*授权完成笔数(未撤销)*/
	int frnRefundNumber;	/*退货笔数(未撤销)*/
	int frnOffNumber;		/*离线笔数(未撤销)*/

	long rmbDebitAmt;	/*人民币借记总额*/
	long rmbCreditAmt;	/*人民币贷记总额*/
	long frnCreditAmt;	/*外卡代记总额*/
	long frnDebitAmt;	/*外币借记总额*/

	long rmbSaleAmt;		/*消费金额(未撤销)*/
	long rmbConfirmAmt;	/*授权完成金额(未撤销)*/
	long rmbRefundAmt;	/*退货金额(未撤销)*/
	long rmbOffAmt;		/*离线金额(未撤销)*/
	
	long frnSaleAmt;		/*消费金额(未撤销)*/
	long frnConfirmAmt;	/*授权完成金额(未撤销)*/
	long frnRefundAmt;	/*退货金额(未撤销)*/
	long frnOffAmt;		/*离线金额(未撤销)*/

	long amtNetSettle; 
	uchar amtChar; /*+ D ; - C*/
	uchar settUnitCode[12]; /*清算机构代码*/

}total_str;

#define TOTAL_STRC_SIZE  sizeof(struct _total_strc)

typedef struct _pos_transtrc_ {
	int            trans_no;                 /*  N(2)  */
	long           tr_code;                 /*  交易代码  N(6)  */
	unsigned char  sev_code[3];
	unsigned char  news_code[3];
	unsigned char  netInfoCode[4];
	unsigned char  msgid_send[5];   /*  S(4)    */
	unsigned char  msgid_rev[5];    /*  S(4)    */
	unsigned char  bitmap_send[9];	/* 8583 Packet Bit Map S(8) */
} pos_transtrc;

typedef struct   ElecType {
	uchar	batchSendFlag; /* '1' 已批上送 '0' 未批上送*/
	long    TradeCode;
	long    Amount;
	long    tipamt;
	long    qsamt;
	long    posflow;
	long	oldposflow; /*原POS流水*/
	long	batchNumber;
	long	oldBatchNumber;    
	int     BackNo;         /*  trans_no            */
	int     oldtrans_no;

	uchar    CardNo[21];
	uchar    B_Date[5];
	uchar    B_Time[7];
	uchar    freshid;        /*  撤销标志 '0' -未撤销  '1'-已撤销            */
	uchar    sys_js[13];     /*  系统检索号          */
	uchar    authno[7];      /*  授权号      S(6)    */
	uchar    expiry[5];
	uchar    enter_mode[4];
	uchar	tellerNumber[3];
	uchar	send_id;	/*上送标志*/
	uchar	send_fail;  /*上送失败标志，已经达到上送次数，但还是没有成功*/
	uchar	adjusted;	// 原交易是否已经调整过
	uchar	wkUnit[4];
	uchar	authMode[3];
	uchar	authUnit[12];
	uchar	issuerResp[21];
	uchar	centerResp[21];
	uchar	recvBankResp[21];
	uchar	issuerBankId[9];
	uchar	recvBankId[9];
	uchar	centerId[9];
	uchar	prtOldPosSer[7]; /*原交易凭证号,用于打印*/
	uchar	prtOldAuthNo[7]; /*原交易授权号,用于打印*/
	uchar	prtOldHostSer[13]; /*原交易检索号,用于打印*/
	uchar 	change_from_adjust; /*如果为'1'表明改笔交易是对未上送的离线结算交易作结算调整而产生的*/
	uchar    track2[TRACK2_LEN+1];/*二三磁道仅为退货保存，用于退货批上送*/
	uchar	track3[TRACK3_LEN+1];
	uchar	old_date[5];	/*仅用于退货，记录退货的原消费日期批上送时用到*/

	// EMV交易增加部分
	uchar	iccdata[255];
	uchar	iccdatalen;
	uchar	card_seq;
	uchar    ic_online_id;
	uchar	tc[9];
	uchar	AuthRespCode[3];
	uchar    TransResult;
	uchar	TVR[6];
	uchar	AID[32];
	uchar	applable[32];
	uchar    fallback;
	uchar  TSI[3];
	uchar  ATC[3];
}ElecTypeStr ;

#define ELEC_LEN sizeof(struct ElecType)

typedef struct _oper_structure {
	uchar tellerNo[3];
	uchar password[13];	/*  S(4)    */
}oper_str;
#define OPER_STRC_SIZE sizeof(struct _oper_structure)

typedef struct _glb_file_ {
	uchar PHONE1[20];
	uchar PHONE2[20];
	uchar PHONE3[20];
	uchar MPhone[20];
	uchar PREDIAL_SET;  	/*1*/
	uchar UnitNum[16];
	uchar PosIdNum[9];
	uchar sendOffFlag;	/*	离线交易是否当时上送　'0' 不　'1'是 */
	uchar Tpdu[6];
	uchar tipOpen;		/*	支持小费标志 '1'-支持 '0'-不支持		*/
	uchar jyOpen[3];		/*	支持的交易类型						*/
	uchar end_id;		/*	是否自动签退 1-是  0- 否			*/
	uchar manualInput;	/*可否手输卡号OPEN  CLOSE'1'-支持  '0'-不支持*/
	uchar key_index;
	uchar errorPrtOpen;	/*是否打印故障单据，1 打印， 0 不打印*/
	uchar unitChineseName[41];
	uchar testflag; 		/*当前为测试状态的标志 '1' 测试状态 '0' 非测试状态 */
	uchar recvBankId[9];  	/*收单行号*/
	uchar ticketNum;		/* 热敏打印打印几联 */
	uchar con_rev_swipe;		/*消费撤销刷卡标志*/
	uchar pre_cm_rev_swipe;	/*预授权完成撤销刷卡标志*/
	uchar rev_enter_pin;		/*各种撤销交易是否输入密码标志，‘0’不输 ’1‘输密*/
	int  commWaitTime; 	/*通讯等待时间*/
	int  tipper;		/*	小费百分比							*/
	int  dup_times;			/*重发次数*/
	int	 dialRetryTimes;	/* 拨号最大重试次数*/
	int  maxTransTotal;  	/*流水保存的最大交易笔数，可设*/
	long Pos_init_ser; 	/*交易流水号*/
	long nowBatchNumber;	/*当前批次号 */
	long refund_limit;	/*最高退货额－以分为单位*/
	uchar des_id;

	uchar emv_support;  //EMV

	/* 以下为TMS参数 */
    uchar dmk_index;		/*远程下载密钥序号，在工作密钥区*/
    uchar ucCheckVer;		/*下载程序是否校验版本*/
}glb_fileStr;

typedef struct _ctrl_file_ {
	uchar nowTellerNumber[3];	/*当前员号 */
	uchar nowTellerPassword[13];
	uchar oldTellerNumber[3];
	uchar posLogonFlag;
	uchar posStatus; /*'0' 正常 '1'测试状态 '2'批上送状态*/
	uchar batchSendType; 	/*当前批上送状态：RMBLOG 上送人民币卡 FRNLOG上送外卡 ALLLOG都上送状态 */
	char dup_reason[3];    /*冲正原因98-未收到应答；96－解包错；A0－MAC校验错 
								06－其它情况（返回96，98，A0）*/
	uchar extNum[5];	 /*所拨的外线号码 小于4位*/
	uchar delayTime;	 /* 延时时间 n秒 < 5 */
	uchar f_load_param;  /*下载参数标志 1 - 需要 0 - 不需要*/
	int  Jy_total;  	/*当前交易总笔数*/	
	uchar emv_load;
	uchar emv_key_load;
	uchar clear_id;
	uchar ic_pd;
    uchar ucDownAppFlag; /*有程序更新标志*/
}ctrl_file_Str;


#endif

