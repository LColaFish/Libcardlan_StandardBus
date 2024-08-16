#ifndef __TERAPP_VAR__H__
#define  __TERAPP_VAR__H__

//#define NO_DIAL
//#define TEST_PRT_PACKET  11
//#define TEST_WK
//#define DIAL_TEST_NAC
//#define CAPK_DEBUG
//#define QINGDAO_CUP
//#define NO_EMV
//#define PROPAY
//#define D_WIN32
//#define LOW 

#define NO_CUP

typedef unsigned char byte;
typedef unsigned char BYTE;
typedef unsigned short	WORD;
typedef unsigned long	ULONG;

#define DES_01    1
#define DES_03	  3

#define  TABLES_LEN     45
#define  ERR_INFO_NUM  	64  /*返回码个数*/

/**定义交易类型  */
#define  NO_TRANS_SELECT 	999
#define  TRANS_HAVE_MASKED  998
#define	 NO_TRANS_KEY		0xff

#define  POS_CON           1   /*消费    */
#define  POS_PRE           2   /*予授权  */
#define  POS_AUT_CM        3   /*予授权完成  刷卡*/
#define  POS_PRE_REV       4   /*预授权撤销	*/
#define  POS_PRECM_REV	   5	/*予授权完成撤销 */
#define  POS_CON_REV	   6   /*消费撤销	*/
#define  POS_PRE_ADD	   7
#define  POS_GR_TUI        8   /*隔日退货	*/
#define  POS_QUE           9   /*查余额		*/
#define  POS_STO           10    /*	*/
#define  POS_DUP           11

/*以下为结算类、管理类交易*/


#endif
#define	 POS_LOGON		   12	/*  POS签到		*/
#define  POS_LOGOFF		   13  /*  联机签退	*/
#define  POS_SETT		   14   /*  结算		*/
#define  POS_BATCH_UP	   15	/*  联机  披上送开始与传送*/
#define  POS_TRA           16   /*	参数传递	*/
#define  POS_MON           17   /*	POS监控		*/
#define  REJ_TES           18   /*	POS回响测试	*/
#define  OUT_JS            19   /*	离线结算	*/
#define  JS_TZ             20   /*	结算调整	*/
#define  JS_TZ_TIP         21   /*	结算调整小费	*/
#define  OUT_JS_BAT        22   /*	批上送 离线结算	*/
#define  JS_TZ_BAT         23   /*	批上送 结算调整	*/
#define  JS_TZ_TIP_BAT     24   /*	批上送 结算调整	*/
#define  POS_GR_TUI_BAT	   25	/*	批上送 隔日退货	*/

#define  IC_CON           26   /*消费    */
#define  IC_PRE           27  /*予授权  */
#define  IC_AUT_CM        28  /*予授权结算  */
#define  IC_PRE_REV       29  /*预授权撤销	*/
#define  IC_PRECM_REV	  30  /*予授权结算撤销 */
#define  IC_CON_REV		  31   /*消费撤销	*/
#define  IC_GR_TUI        32  /*隔日退货	*/
#define  IC_QUE           33  /*查余额		*/
#define  IC_BAT           34
#define  IC_OFF           35
#define  IC_DUP           36
#define  IC_KEY			  37
#define  IC_MON           38
#define  IC_SCR_SEND      39
#define  IC_KEY_END		  40
#define  IC_TRA           41
#define  IC_GR_TUI_BAT    42
#define  IC_AUT_BAT       43
#define  UPDATE_NOTICE    44   /*      新增交易用于参数和程序下载*/

#define  QUE_TOTAL	       50	/*当班交易总计	*/
#define  QUE_BATCH_TOTAL   51	/*当批交易总计	*/
#define  QUE_BATCH_MX      52   /*当批明细		*/
#define  QUE_MX		       53	/*当班明细		*/
#define	 QUE_BATCH_ANY	   54	/*当批某笔		*/
#define  PRT_LAST          55
#define  PRT_ANY           56
#define  PRT_MX            57
#define  PRT_TOTAL		   58	
#define  QUE_IC_MX         59
#define  QUE_IC_INFO       60
#define  QUE_IC_ZJ       61
/*操作员管理*/
#define	 TELLER_LOGON		70	/* 操作员签到 ***/
#define	 TELLER_LOGOFF		71	/* 操作员签退 **/
#define  TELLER_ADD			72	/* 增加操作员 **/
#define	 TELLER_DEL			73	/* 删除操作员***/
#define	 TELLER_DISP		74	/* 显示操作员****/
#define	 TELLER_CHGPWD		75	/* 操作员改密 ****/
#define	 SETUP_WKBANK		76	/*选择外卡结算银行 **/
#define  SETUP_EXTNUM		77 


#define  OK               0
#define  TRUE             1

#define	 SUCCESS		0x00
#define	 UNSUCCESS		0xff

#define  JY_ESC           1    /*交易被取消 */
#define  JY_FAIL          2    /*交易失败   */ 
#define  NO_RECORD        3   /* 无记录		*/      
#define  NO_JY            4   /* 无交易		*/
#define  ERR_MP           5   /* 打包错		*/
#define  ERR_CONNECT      6   /*联接失败	*/
#define  ERR_FP           7   /*发包错误  */  
#define  ERR_RP           8   /*收包错误  */
#define  ERR_JP           9   /*解包错误  */
#define  HF_FAIL          10  /* 冲正失败 */
#define  ERR_JY           11  /*不支持该交易 */
#define  NO_OLD_JY        12  /*无原始交易 */
#define  JY_ESCED         13  /*交易已被撤消 */
#define  NO_ESC_JY        14  /*不能取消取消交易*/
#define  NO_TUI_JY        15  /*原交易已退货*/
#define  ERR_SWIPE        16
#define  NO_EQU_AMT       17
#define  NO_VALUE_JY      18
#define  MEM_ERR          19
#define  E_MEM_ERR        19  /*兼容*/
#define  NO_KEY           20
#define  TIP_FIN          21
#define  ERR_POS_BATCH    22
#define  ERR_COMPUTE_TOTAL	  23
#define  ERR_POS_TOTAL		24
#define  DUP_SUC			25
#define  INVALID_TIP		26
#define  TIP_NO_OPEN		27
#define  TRANS_HAVE_ADJUESTED 28
#define  USER_CANCEL		29
#define  ERR_OPEN	30 /*打开文件错*/
#define  ERR_SEEK	31 /*定位文件错*/
#define  ERR_READ	32 /*读文件错*/ 
#define  ERR_WRITE	33 /*写文件错*/
#define  ERR_AMT	34 /*金额超限*/
#define  ERR_LOGGED 35 /*操作员已经签到*/


//EMV err_info
//#define err_IccDataFormat  34
#define  ERR_EMV	36	/*EMV处理时错误*/
#define  FALL_BACK  37
//#define err_ListNoMatchApp  47
//#define  err_CardBlock   51

/*NO_DISP定为出错显示的最大值，注意随时更改*/
#define  NO_DISP          255

#define  MAN_CARD         0    /* 手工输卡号   */
#define  MAN_SWIPE        1    /* 手输或刷卡   */
#define  ONLY_SWIPE       2    /* 刷卡        */
#define  CON_SWIPE        3    /* 起始刷卡  */
#define  INSERT_SWIPE     4    /* 刷卡或插卡 */
#define  INSERT_MAN_SWIPE 5    /* 刷 插 输  */

//输入方式
#define  IDLE_MODE        0	   /* 无输入   */
#define  MANUAL_MODE      1    /* 手输卡   */
#define  SWIPE_MODE       2    /* 刷卡     */
#define  INSERT_MODE	  3    /* IC卡插卡 */
#define  FALLBACK_MODE    4    /* IC卡回退 */

#define  DIAL_ON               0x01
#define  DIAL_OFF              0x00

#define  PIN_KEY_ID		2
#define  MAC_KEY_ID		3
#define  TEST_KEY_ID	4

#define  DOUBLE_PIN_KEY_ID  6
#define  DOUBLE_MAC_KEY_ID  8

#define  MANUAL_ENTER     "012"  /*手工输入 无PIN*/
#define  SWIPE_ENTER      "022"  /*刷卡     无PIN*/
#define  INSERT_ENTER     "052"  /*插卡     无PIN*/
#define  FALLBACK_ENTER   "952"  /*IC刷卡   无PIN*/

/*ISO7812*/
#define  TRACK2_LEN        40
#define  TRACK3_LEN        107

#define  NORMAL            0
#define  REPRINT           1

#define	 MANUAL_OPENED		'1'
#define  MANUAL_CLOSED		'0'

#define  LEFT              0
#define  MIDDLE            1
#define  RIGHT             2

#define  MAX_8583PACKET_LEN 1024

#define  MAX_OPER_NUM 		11  /*POS 允许的最大操作员数量*/
#define  MAX_JY_TOTAL		500 /*POS 允许的交易数量*/

#define  SUP_TELLER_NO	"00"
#define  SUP_TELLER_PWD "123456"
#define  TELLER_NOBODY	"\x01\x01"	/* */
#define  TELLER_LOCK	"\x02\x01"	/*锁定 */

#define  LOGGEDOFF		0		/* teller or pos logged off */
#define  LOGGEDON		1		/* teller of pos logged on */	

#define	 RMBLOG				1	/*用于披上送时选择不同的交易纪录*/
#define  FRNLOG				2
#define  ALLLOG				3
#define  EMVLOG             4

#define  WORK_STATUS         '0'
#define  TEST_STATUS         '1'
#define  BATCH_STATUS		 '2'

#define  TIMER_SET_FOR_KEY	4
#define  TIMER_SET_FOR_COMM	3

#define  ICON_PHONE		1	/* phone 电话*/
#define  ICON_SIGNAL	2	/*wireless signal 信号*/
#define  ICON_PRINTER	3	/*printer 打印机*/
#define  ICON_ICCARD	4	/* smart card IC卡*/
#define  ICON_LOCK		5	/* lock 锁*/
#define  ICON_SPEAKER	6	/* speeker 扬声器*/
#define  ICON_UP		7	/*up 向上*/
#define  ICON_DOWN		8	/*down 向下*/
	/*action - 指定动作,	1:点亮，0:熄灭。*/
#define  CLOSEICON		0	/*关闭图标[针对所有图标]*/
#define  OPENICON        1   /* 显示图标[针对打印机、IC卡、锁、扬声器、向上、向下]*/
/*若需点亮摘机、挂机图标，则action的值设置如下：*/
#define  OFFHOOKICON	1	/*hook off 摘机*/
#define  ONHOOKICON	2	/*/hook on  挂机*/
/*若需点亮信号图标，则action的值设置为：INITSIGNAL+[0，5]*/
#define  INITSIGNAL   1   /*初始信号强度[最强信号为INITSIGNAL+5]*/


#define ALLOW_ZERO_IN		0x80
#define BIG_IN			0x40
#define NUM_IN			0x20
#define ALPHA_IN		0x10
#define PASS_IN			0x08
#define CARRY_IN		0x04
#define AMOUNT_IN		0x02
#define ECHO_IN			0x01

#define COM_TRANS_ACP		0

#define MODE_BOX		0x80
#define MODE_WAIT		0
#define MODE_30			60
#define MODE_60			60


// IC卡槽定义
#define ICC_USER		0

#define PROCESS_OK 0
#define PROCESS_FALSE 1

#define PARAM_OPEN    '1'
#define PARAM_CLOSE   '0'

/* 缺省的超时时间 */
#define TIMEOUT        40

//TIMS ADD
#define NUMBER_INPUT 0x61
#define PASSWORD_INPUT 0x79
#define MONEY_INPUT  0x63
#define ALPHA_NUMBER_INPUT 0x71
