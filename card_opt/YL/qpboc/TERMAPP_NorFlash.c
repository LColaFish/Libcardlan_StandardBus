#include "includes.h"

/****************************************************************************
FUNCTION:  ���Ҷ�ӦAID����������
PARAMETER DESCRIPTION:
	 			 AID
				 length :AID����
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
	 //���Ŀ��ٷֱ�
	TermInfo.TargetPercent=OtherICPara.TargetPercent;

  //ƫ�����ѡ����ֵ
	memcpy(TermInfo.Threshold,OtherICPara.Threshold,4);
  //ƫ�����ѡ������Ŀ��ٷֱ�
	TermInfo.MaxTargetPercent=OtherICPara.MaxTargetPercent;
		
	memcpy(TermInfo.CountryCode,"\x01\x56",2);
	
	//�ǽӴ���д���ѻ�����޶�
	memcpy(TermInfoEx.OfflineLowestLimit,OtherICPara.OfflineLowestLimit,6);	TermInfoEx.OfflineLowestLimitbExist=1;

	//�ǽӴ���д�������޶�
	memcpy(TermInfoEx.TransLimit,OtherICPara.TransLimit,6);TermInfoEx.TransLimitbExist=1;

	//�ն�����޶�
	memcpy(TermInfo.FloorLimit,OtherICPara.FloorLimit,4);
	//�ն˵����ֽ����޶�
	memcpy(TermInfo.VLPTransLimit,OtherICPara.VLPTransLimit,6);

	memcpy(TermInfo.TACDefault,OtherICPara.TACDefault,5);
	memcpy(TermInfo.TACOnline,OtherICPara.TACOnline,5);
	memcpy(TermInfo.TACDenial,OtherICPara.TACDenial,5);


	return OK;
}
