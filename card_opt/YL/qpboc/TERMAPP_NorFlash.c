#include "includes.h"

/****************************************************************************
FUNCTION:  查找对应AID的其它参数
PARAMETER DESCRIPTION:
	 			 AID
				 length :AID长度
RETURN: 

*****************************************************************************/
uchar TERMAPP_GetICParameter(uchar *AID,uchar length)
{
	uchar temp[2048];
	uchar buf[74];
	uchar crctemp[2];
	uchar counter=0;
	uchar i;

	DBG_QPBOC_PRINTF("TERMAPP_GetICParameter() is called.\n");

	/*
	FlashRead(Flash_Address_ICPara,2048, temp);
	memcpy(buf,temp,2);
	Calc_crc(CRC16,2,buf,buf+2);
	if(temp[0] !=QPBOC_PARA_HEAD || memcmp(buf+2,temp+2,2) != 0)
	{
		FlashRead(Flash_Address_ICPara,2048, temp);
		memcpy(buf,temp,2);
		Calc_crc(CRC16,2,buf,buf+2);
		if(temp[0] !=QPBOC_PARA_HEAD || memcmp(buf+2,temp+2,2) != 0)
			return NOK;
	}
	*/
	memset(buf, 0, sizeof(buf));
	counter=buf[1];
	for(i=0;i<counter+1;i++)
	{
		if(memcmp(AID,temp+4+3+i*QPBOC_PARA_VALUELEN,length)==0)
		{
			memcpy(buf,temp+4+i*QPBOC_PARA_VALUELEN,QPBOC_PARA_VALUELEN);
			//Calc_crc(CRC16,2+sizeof(OTHER_ICPARAMETER_STRUCT),buf,crctemp);	
			if(memcmp(crctemp,buf+2+sizeof(OTHER_ICPARAMETER_STRUCT),2) != 0)
			{
				return NOK;
			}
			memcpy((uchar *)&OtherICPara,buf+2,sizeof(OTHER_ICPARAMETER_STRUCT));
			break;
		}
	}
	 //随机目标百分比
	TermInfo.TargetPercent=OtherICPara.TargetPercent;

  //偏置随机选择阈值
	memcpy(TermInfo.Threshold,OtherICPara.Threshold,4);
  //偏置随机选择的最大目标百分比
	TermInfo.MaxTargetPercent=OtherICPara.MaxTargetPercent;
		
	memcpy(TermInfo.CountryCode,"\x01\x56",2);
	
	//非接触读写器脱机最低限额
	memcpy(TermInfoEx.OfflineLowestLimit,OtherICPara.OfflineLowestLimit,6);	TermInfoEx.OfflineLowestLimitbExist=1;

	//非接触读写器交易限额
	memcpy(TermInfoEx.TransLimit,OtherICPara.TransLimit,6);TermInfoEx.TransLimitbExist=1;

	//终端最低限额
	memcpy(TermInfo.FloorLimit,OtherICPara.FloorLimit,4);
	//终端电子现金交易限额
	memcpy(TermInfo.VLPTransLimit,OtherICPara.VLPTransLimit,6);

	memcpy(TermInfo.TACDefault,OtherICPara.TACDefault,5);
	memcpy(TermInfo.TACOnline,OtherICPara.TACOnline,5);
	memcpy(TermInfo.TACDenial,OtherICPara.TACDenial,5);


	return OK;
}
