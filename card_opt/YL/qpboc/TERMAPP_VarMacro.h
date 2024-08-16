#ifndef __TERMAPP_VARMACRO__H__
#define __TERMAPP_VARMACRO__H__

#define TermDataBase   0
// Macro name variables which are to be sent to host in Message.
//card variable
#define MV_AppCrypt				0
#define	MV_AppCurcyCode			1
#define	MV_AppCurcyExp			2
#define	MV_AppDiscretData		3
#define	MV_AppEffectDate		4
#define	MV_AppExpireDate		5
#define	MV_AFL					6
#define	MV_ICC_AID				7
#define	MV_AIP					8
#define	MV_AppLabel				9
#define	MV_AppPreferName		10
#define	MV_PAN					11
#define	MV_PANSeqNum			12
#define	MV_AppPriority			13
#define	MV_AppReferCurcy		14
#define	MV_AppReferCurcyExp		15
#define	MV_ATC					16
#define	MV_AUC					17
#define	MV_ICC_AppVer			18
#define	MV_CDOL1				19
#define	MV_CDOL2				20
#define	MV_CardholderName		21
#define	MV_CardholderNameExt	22
#define	MV_CVMList				23
#define	MV_ICCAPKI				24
#define	MV_CryptInfoData		25
#define	MV_DataAuthenCode		26
#define	MV_DFName				27
#define	MV_DDFName				28
#define	MV_DDOL					29
#define	MV_ICCDynNum			30
#define	MV_ICCPIN_EPKCert		31
#define	MV_ICCPIN_EPKExp		32
#define	MV_ICCPIN_EPKRem		33
#define	MV_ICCPKCert			34
#define	MV_ICCPKExp				35
#define	MV_ICCPKRem				36
#define	MV_IACDenial			37
#define	MV_IACOnline			38
#define	MV_IACDefault			39
#define	MV_IssuAppData			40
#define	MV_ICTI					41
#define	MV_IssuCountryCode		42
#define	MV_IPKCert				43
#define	MV_IPKExp				44
#define	MV_IPKRem				45
#define	MV_LangPrefer			46
#define	MV_LOATCReg				47
#define	MV_LCOL					48
#define	MV_PINTryCount			49
#define	MV_PDOL					50
#define	MV_ServiceCode			51
#define	MV_SignDynAppData		52
#define	MV_SignStatAppData		53
#define	MV_SDATagList			54
#define	MV_Track1Discret		55
#define	MV_Track2Discret		56
#define	MV_Track2Equivalent		57
#define	MV_TDOL					58
#define	MV_UCOL					59
#define	MV_IssuerURL			60
#define	MV_VLPAvailableFund		61
#define	MV_VLPIssuAuthorCode	62
#define MV_PersonID             63
#define MV_PersonIDType         64
#define	MV_OfflineAmount        65
#define MV_CardTradePro         66

//other variable needed for host-terminal message
#define MV_MessageType			80
#define MV_EncPINData			81
#define MV_IssuAuthenData		82
#define MV_IssuScript			83
#define MV_IssuScriptResult		84
#define MV_CAPK                 85
#define MV_ExceptFile           86

//terminal variable
#define	MV_AcquireID			0					   
#define	MV_TermCapab			1
#define	MV_TermAddCapab			2
#define	MV_IFDSerial			3
#define	MV_TermID				4
#define	MV_MerchCateCode		5
#define	MV_MerchID				6
#define	MV_TermCountryCode		7
#define	MV_TRMData				8
#define	MV_TermType				9
#define	MV_TermAppVer			10
#define	MV_TransCurcyCode		11
#define	MV_TransCurcyExp		12
#define	MV_TransReferCurcyCode	13
#define	MV_TransReferCurcyExp	14
#define	MV_TACDenial			15
#define	MV_TACOnline			16
#define	MV_TACDefault			17
#define	MV_TransType			18
#define	MV_TransTypeValue		19
#define	MV_VLPTransLimit		20
#define	MV_VLPTACDenial			21
#define	MV_VLPTACOnline			22
#define	MV_VLPTACDefault		23
#define	MV_Language				24
#define	MV_bTermDDOL			25
#define	MV_bForceAccept			26
#define	MV_bForceOnline			27
#define	MV_bBatchCapture		28
#define	MV_bTermSupportVLP		29
#define	MV_MaxTargetPercent		30
#define	MV_TargetPercent		31
#define	MV_TermDDOL				32
#define	MV_TermTDOL				33
#define	MV_MerchNameLocate		34
#define	MV_TransLogMaxNum		35
#define	MV_Threshold			36
#define	MV_TermFloorLimit		37
#define	MV_AmtTrans				38
#define	MV_AmtNet				39
#define	MV_BatchTransNum		40
#define	MV_TransNum				41
#define	MV_TransIndex			42
#define	MV_TransSeqCount		43
#define	MV_AmtAuthorBin			44
#define	MV_AmtAuthorNum			45
#define	MV_AmtOtherBin			46
#define	MV_AmtOtherNum			47
#define	MV_AmtReferCurrency		48
#define	MV_TERM_AID				49
#define	MV_AuthorCode			50
#define	MV_AuthorRespCode		51
#define	MV_CVMResult			52
#define	MV_POSEntryMode			53
#define	MV_PIN					54
#define	MV_TVR					55
#define	MV_TSI					56
#define	MV_VLPIndicator			57
#define	MV_TransDate			58
#define	MV_TransTime			59
#define	MV_TCHashValue			60
#define	MV_UnpredictNum			61
#define	MV_IssuerAuthenData		62
#define MV_MCHIPTransCategoryCode  63
#define MV_TermTransProp        64
#define MV_MerchName            65
//define message type macro 
#define Msg_AuthRQ          0
#define Msg_FinaRQ			1
#define Msg_AuthRP			2
#define Msg_Confirm			3
#define Msg_BatchTrans		4
#define Msg_BatchAdvice		5
#define Msg_Reconci			6
#define Msg_OnlineAdvice	7
#define Msg_Reversal        8
#define Msg_DownloadKey     9

#define Msg_BatchOnlineApprove  10
#define Msg_BatchOnlineDecline  11
#define Msg_OnlineForceApprove  12
#define Msg_OfflineForceApprove 13

#define Msg_UpdatePara      14

//record types
#define REC_CAPK            1
#define REC_TERMAPP         2
#define REC_EXCEPTFILE      3
#define REC_IPKREVOKE       4
#define REC_TERMINFO        5

//option codes
#define OP_ADD				0
#define OP_DELETE			1
#define OP_MODIFY			2


#endif

