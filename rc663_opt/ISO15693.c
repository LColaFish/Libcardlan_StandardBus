/*******************************************************************************
 * CopyRight(c)2010-2017, BJ-TOP Electronics Co.,Ltd. All rights reserved
 * FileName: ISO15693.c
 *   Author: Layne Wen
 *     Date: 2017.12.20
 *  COMMENT: 初始移植版本，有待整理
 ******************************************************************************/
#include "ISO15693.h"
#include "PCD.h"
#include "status.h"

extern tpd_pcdpara g_sPcdPara;
extern tpd_pcdtrsv g_sPcdTrsv;


unsigned char ISO15693_Inventory(unsigned char Flags, unsigned char AFI,
						unsigned char Mask_Length, unsigned char *Mask,
						unsigned char *cpReceiveLength, unsigned char *cpReceiveData)
{
	unsigned char cIndex, cByteCnt;
	unsigned char cStatus = 0xFF;
	unsigned char i, j, k;
	unsigned char cTempStatus;

	cStatus = PcdConfig(PCD_TYPE_15693);

	if (cStatus != ERR_SUCCESS)
	{
	    printf("[%s %d ]error %d \n",__FUNCTION__,__LINE__,cStatus);
		return cStatus;
	}
	cStatus = 0xFF;
	*cpReceiveLength = 0;

	g_sPcdTrsv.cData[0] = Flags; //Flag
	g_sPcdTrsv.cData[1] = ISO15693_INVENTORY;  //Command
	cIndex = 2;
	if((Flags&0x10) != 0x00)
	{
		g_sPcdTrsv.cData[cIndex++] = AFI;
	}
	g_sPcdTrsv.cData[cIndex++] = Mask_Length;
	
	if (Mask_Length%8)
	{
		cByteCnt = Mask_Length/8 + 1;
	}
	else
	{
		cByteCnt = Mask_Length/8;
	}
	if(cByteCnt)
	{
		memcpy (&(g_sPcdTrsv.cData[cIndex]), Mask, cByteCnt);	
		cIndex += cByteCnt;
	}
	
    g_sPcdTrsv.iLength = cIndex;
	if((Flags&0x20) != 0x00) //Only use 1 slot to inventory
	{
		SetTimeOut();
		cStatus = PcdTransceive(PCD_TRANSCEIVE);
		if (cStatus == 0)
		{
			j = g_sPcdTrsv.iLength - 1;
			if ((g_sPcdTrsv.cData[0] & 0x01) == 1)
			{
				cStatus = 0xFF;
			}
			else
			{
				memcpy(cpReceiveData, &g_sPcdTrsv.cData[1], j);
				*cpReceiveLength = j;									 	   
			}
		}
	}
	else
	{
		SetTimeOut();
		PcdTransceive(PCD_TRANSMIT);
		j = 0;
		for (i=0; i<16; i++)
		{
			ClearBitMask(PHHAL_HW_RC663_REG_TXCRCCON, 0x01);		// ClearBitMask(RegChannelRedundancy, 0x04);
			g_sPcdTrsv.iLength = 0x00;
			cTempStatus = PcdTransceive(PCD_TRANSCEIVE);	
			if ((cTempStatus == MI_OK) && (g_sPcdTrsv.iLength == 0x50))
			{
				cStatus = MI_OK;
				*cpReceiveLength += 9;
				for (k=0; k<9; k++)
				{
					cpReceiveData[k + 9 * j] = g_sPcdTrsv.cData[1 + k];
				}
//				memcpy(&cpReceiveData[k + 9 * j], &g_sPcdTrsv.cData[1], 9);		// can't use this line, the size is big
				j++;    
			}
		}
		SetBitMask(PHHAL_HW_RC663_REG_TXCRCCON, 0x01);		// SetBitMask(RegChannelRedundancy, 0x04);
	}

	return  cStatus;
}

unsigned char ISO15693_StayQuiet(unsigned char Flags, unsigned char *UID)
{
	unsigned char cStatus;
//	unsigned char i;
//	cStatus = Set15693Mode();
//	if (cStatus != 0)
//	{
//		return cStatus;
//	}

	g_sPcdTrsv.iLength = 10;
	g_sPcdTrsv.cData[0] = Flags; //Flag
	g_sPcdTrsv.cData[1] = ISO15693_STAY_QUIET;  //Command
//	for (i=0; i<8; i++)
//	{
//		g_sPcdTrsv.cData[2 + i] = UID[i];
//	}
	memcpy(&g_sPcdTrsv.cData[2], UID, 8);	
	SetTimeOut();
	cStatus = PcdTransceive(PCD_TRANSMIT);
//	cStatus = PcdTransceive(PCD_TRANSCEIVE);

	return  cStatus;
}

unsigned char ISO15693_ReadBlock(unsigned char Flags, unsigned char *UID, 
                       unsigned char cBlockNumber, unsigned char cNumberOfBlock,
					   unsigned char *cpReceiveLength, unsigned char *cpReceiveData)
{
	unsigned char cStatus;
	unsigned char cIndex;
	unsigned char j;
	g_sPcdTrsv.cData[0] = Flags; //Flag
	if (cNumberOfBlock > 0)
	{
		g_sPcdTrsv.cData[1] = ISO15693_READ_MULTIPLE_BLOCK;  //Command
	}
	else
	{
		g_sPcdTrsv.cData[1] = ISO15693_READ_SINGLE_BLOCK;  //Command
	}
	cIndex = 2;
	if (Flags & 0x20) // flags & 0x20 - Adresflag request flags & 
	{
		memcpy(&g_sPcdTrsv.cData[cIndex], UID, 8);
		cIndex += 8;
	}
	g_sPcdTrsv.cData[cIndex++] = cBlockNumber;
	if (cNumberOfBlock > 0)
	{
		g_sPcdTrsv.cData[cIndex++] = cNumberOfBlock;
	}
	g_sPcdTrsv.iLength = cIndex;
	SetTimeOut();
	cStatus = PcdTransceive(PCD_TRANSCEIVE);
	*cpReceiveLength = 0;
    //printf("ISO15693_ReadBlock = %02x\n",cStatus);
	if (cStatus == 0)
	{
		if ((g_sPcdTrsv.cData[0] & 0x01) == 1)
		{
			cStatus = 0xFF;
		}
		else
		{
			memcpy(cpReceiveData, &g_sPcdTrsv.cData[1], g_sPcdTrsv.iLength-1);
			*cpReceiveLength = g_sPcdTrsv.iLength-1;
		}
	}
    //printf("ISO15693_ReadBlock = %02x\n",cStatus);
	return  cStatus;
}

unsigned char TI_Process(unsigned char Flags)
{
	unsigned char cStatus = 0;
	if ((Flags & 0x40) == 0)
	{
		if ((g_sPcdTrsv.cData[0] & 0x01) == 1)
		{
			cStatus = 0xFF;
		}
	}
	else
	{
        usleep(10000);
		ClearBitMask(PHHAL_HW_RC663_REG_FRAMECON,0x03);		// SetBitMask(RegCoderControl, 0x80);
		ClearBitMask(PHHAL_HW_RC663_REG_TXDATANUM,0x08);
		g_sPcdTrsv.iLength = 0;
		cStatus = PcdTransceive(PCD_TRANSCEIVE);
		SetBitMask(PHHAL_HW_RC663_REG_FRAMECON,0x03);		// ClearBitMask(RegCoderControl, 0x80);
		SetBitMask(PHHAL_HW_RC663_REG_TXDATANUM,0x08);
		if (cStatus == 0)
		{
			if ((g_sPcdTrsv.cData[0] & 0x01) == 1)
			{
				cStatus = 0xFF;
			}
		}
	}
    
	return cStatus;
}

unsigned char ISO15693_Write(unsigned char Flags, unsigned char *UID, 
							unsigned char cBlockNumber,
							unsigned char *cpWriteData)
{
	unsigned char cStatus;
	unsigned char i;
	unsigned char cIndex;

	g_sPcdTrsv.cData[0] = Flags; //Flag
	g_sPcdTrsv.cData[1] = ISO15693_WRITE_SINGLE_BLOCK;  //Command
	cIndex = 2;
	if (Flags & 0x20)	// flags & 0x20 - Adresflag request flags & 
						// 0x10 - Selectflag request
	{
		memcpy(&g_sPcdTrsv.cData[cIndex], UID, 8);
		cIndex += 8;
	}
	g_sPcdTrsv.cData[cIndex++] = cBlockNumber;
	for (i=0; i<4; i++)
	{
		g_sPcdTrsv.cData[cIndex++] = cpWriteData[i];
	}
	g_sPcdTrsv.iLength = cIndex;
	SetTimeOut();
	if (Flags & 0x40)
	{
		cStatus = PcdTransceive(PCD_TRANSMIT);
	}
	else
	{
		cStatus = PcdTransceive(PCD_TRANSCEIVE);
	}

	if (cStatus == 0)
	{
		cStatus = TI_Process(Flags);
	}
 
//	*cpReceiveLength = 0;
	return  cStatus;
}

unsigned char ISO15693_LockBlock(unsigned char Flags, unsigned char *UID, 
                       unsigned char cBlockNumber)
{
	unsigned char cStatus;
//	unsigned char i;
	unsigned char cIndex;
//	cStatus = Set15693Mode();
//	if (cStatus != 0)
//	{
//		return cStatus;
//	}

	g_sPcdTrsv.cData[0] = Flags; //Flag
	g_sPcdTrsv.cData[1] = ISO15693_LOCK_BLOCK;  //Command
	cIndex = 2;
	if (Flags & 0x20) // flags & 0x20 - Adresflag request flags & 
			  // 0x10 - Selectflag request
	{
//		for (i=0; i<8; i++)
//		{
//			g_sPcdTrsv.cData[cIndex++] = UID[i];
//		}
		memcpy(&g_sPcdTrsv.cData[cIndex], UID, 8);
		cIndex += 8;
	}
	g_sPcdTrsv.cData[cIndex] = cBlockNumber;
	cIndex += 1;
	g_sPcdTrsv.iLength = cIndex;
	SetTimeOut();
	
	if (Flags & 0x40)
	{
		cStatus = PcdTransceive(PCD_TRANSMIT);
	}
	else
	{
		cStatus = PcdTransceive(PCD_TRANSCEIVE);
	}
	if (cStatus == 0)
	{
		cStatus = TI_Process(Flags);
	}
//	*cpReceiveLength = 0;

	return  cStatus;
}


unsigned char ISO15693_ResetToReady(unsigned char Flags, unsigned char *UID)
{
	unsigned char cStatus;
//	cStatus = Set15693Mode();
//	if (cStatus != 0)
//	{
//		return cStatus;
//	}
//	unsigned char i;
	g_sPcdTrsv.iLength  = 10;
	g_sPcdTrsv.cData[0] = Flags; //Flag
	g_sPcdTrsv.cData[1] = ISO15693_RESET_TO_READY;  //Command
//	for (i=0; i<8; i++)
//	{
//		g_sPcdTrsv.cData[2 + i] = UID[i];
//	}
	memcpy(&g_sPcdTrsv.cData[2], UID, 8);
	SetTimeOut();
	cStatus = PcdTransceive(PCD_TRANSCEIVE);
	if (cStatus == 0)
	{
		if ((g_sPcdTrsv.cData[0] & 0x01) == 1)
		{
			cStatus = 0xFF;
		}
	}
	return  cStatus;
}

unsigned char ISO15693_Select(unsigned char Flags, unsigned char *UID)
{
	unsigned char cStatus;
//	cStatus = Set15693Mode();
//	if (cStatus != 0)
//	{
//		return cStatus;
//	}
//	unsigned char i;
	g_sPcdTrsv.iLength  = 10;
	g_sPcdTrsv.cData[0] = Flags; //Flag
	g_sPcdTrsv.cData[1] = ISO15693_SELECT;  //Command
//	for (i=0; i<8; i++)
//	{
//		g_sPcdTrsv.cData[2 + i] = UID[i];
//	}
	memcpy(&g_sPcdTrsv.cData[2], UID, 8);
	SetTimeOut();
	cStatus = PcdTransceive(PCD_TRANSMIT);
	if (cStatus == 0)
	{
		if ((g_sPcdTrsv.cData[0] & 0x01) == 1)
		{
			cStatus = 0xFF;
		}
	}
 //    printk("ISO15693_Select = %02x\n",cStatus);
	return  cStatus;
}

unsigned char ISO15693_WriteAFI(unsigned char Flags, unsigned char *UID, 
                       unsigned char cAFI)
{
	unsigned char cStatus;
//	unsigned char i;
	unsigned char cIndex;
	unsigned char cCmd;
//	cStatus = Set15693Mode();
//	if (cStatus != 0)
//	{
//		return cStatus;
//	}
	if (Flags & 0x40)
	{
		cCmd = PCD_TRANSMIT;
	}
	else
	{
		cCmd = PCD_TRANSCEIVE;
	}
	g_sPcdTrsv.cData[0] = Flags; //Flag
	g_sPcdTrsv.cData[1] = ISO15693_WRITE_AFI;  //Command
	cIndex = 2;
	if (Flags & 0x20) // flags & 0x20 - Adresflag request flags & 
			  // 0x10 - Selectflag request
	{
//		for (i=0; i<8; i++)
//		{
//			g_sPcdTrsv.cData[cIndex++] = UID[i];
//		}
		memcpy(&g_sPcdTrsv.cData[cIndex], UID, 8);
		cIndex += 8;
	}
	g_sPcdTrsv.cData[cIndex] = cAFI;
	cIndex += 1;
	g_sPcdTrsv.iLength = cIndex;
	SetTimeOut();
	
	cStatus = PcdTransceive(cCmd);
	if (cStatus == 0)
	{
		cStatus = TI_Process(Flags);
	}
//	*cpReceiveLength = 0;

	return  cStatus;
}

unsigned char ISO15693_LockAFI(unsigned char Flags, unsigned char *UID)
{
	unsigned char cStatus;
//	unsigned char i;
	unsigned char cIndex;
	unsigned char cCmd;
//	cStatus = Set15693Mode();
//	if (cStatus != 0)
//	{
//		return cStatus;
//	}
	if (Flags & 0x40)
	{
		cCmd = PCD_TRANSMIT;
	}
	else
	{
		cCmd = PCD_TRANSCEIVE;
	}
	g_sPcdTrsv.cData[0] = Flags; //Flag
	g_sPcdTrsv.cData[1] = ISO15693_LOCK_AFI;  //Command
	cIndex = 2;
	if (Flags & 0x20) // flags & 0x20 - Adresflag request flags & 
			  // 0x10 - Selectflag request
	{
//		for (i=0; i<8; i++)
//		{
//			g_sPcdTrsv.cData[cIndex++] = UID[i];
//		}
		memcpy(&g_sPcdTrsv.cData[cIndex], UID, 8);
		cIndex += 8;
	}
	g_sPcdTrsv.iLength = cIndex;
	SetTimeOut();
	cStatus = PcdTransceive(cCmd);
	if (cStatus == 0)
	{
		cStatus = TI_Process(Flags);
	}
//	*cpReceiveLength = 0;

	return  cStatus;
}

unsigned char ISO15693_WriteDSFID(unsigned char Flags, unsigned char *UID, 
							unsigned char cDSFID)
{
	unsigned char cStatus;
//	unsigned char i;
	unsigned char cIndex;
	unsigned char cCmd;
//	cStatus = Set15693Mode();
//	if (cStatus != 0)
//	{
//		return cStatus;
//	}
	if (Flags & 0x40)
	{
		cCmd = PCD_TRANSMIT;
	}
	else
	{
		cCmd = PCD_TRANSCEIVE;
	}
	g_sPcdTrsv.cData[0] = Flags; //Flag
	g_sPcdTrsv.cData[1] = ISO15693_WRITE_DSFID;  //Command
	cIndex = 2;
	if (Flags & 0x20) // flags & 0x20 - Adresflag request flags & 
			  // 0x10 - Selectflag request
	{
//		for (i=0; i<8; i++)
//		{
//			g_sPcdTrsv.cData[cIndex++] = UID[i];
//		}
		memcpy(&g_sPcdTrsv.cData[cIndex], UID, 8);
		cIndex += 8;
	}
	g_sPcdTrsv.cData[cIndex] = cDSFID;
	cIndex += 1;
	g_sPcdTrsv.iLength = cIndex;
	SetTimeOut();
	cStatus = PcdTransceive(cCmd);
	if (cStatus == 0)
	{
		cStatus = TI_Process(Flags);
	}
//	*cpReceiveLength = 0;

	return  cStatus;
}

unsigned char ISO15693_LockDSFID(unsigned char Flags, unsigned char *UID)
{
	unsigned char cStatus;
//	unsigned char i;
	unsigned char cIndex;
	unsigned char cCmd;
//	cStatus = Set15693Mode();
//	if (cStatus != 0)
//	{
//		return cStatus;
//	}
	if (Flags & 0x40)
	{
		cCmd = PCD_TRANSMIT;
	}
	else
	{
		cCmd = PCD_TRANSCEIVE;
	}
	g_sPcdTrsv.cData[0] = Flags; //Flag
	g_sPcdTrsv.cData[1] = ISO15693_LOCK_DSFID;  //Command
	cIndex = 2;
	if (Flags & 0x20) // flags & 0x20 - Adresflag request flags & 
			  // 0x10 - Selectflag request
	{
//		for (i=0; i<8; i++)
//		{
//			g_sPcdTrsv.cData[cIndex++] = UID[i];
//		}
		memcpy(&g_sPcdTrsv.cData[cIndex], UID, 8);
		cIndex += 8;
	}
	g_sPcdTrsv.iLength = cIndex;
	SetTimeOut();
	cStatus = PcdTransceive(cCmd);
	if (cStatus == 0)
	{
		cStatus = TI_Process(Flags);
	}
//	*cpReceiveLength = 0;
	return  cStatus;
}

unsigned char ISO15693_GetSystemInformation(unsigned char Flags, unsigned char *UID, 
                       unsigned char *cpReceiveLength, unsigned char *cpReceiveData)
{
	unsigned char cStatus;
//	unsigned char i;
	unsigned char cIndex;
//	cStatus = Set15693Mode();
//	if (cStatus != 0)
//	{
//		return cStatus;
//	}
	
	g_sPcdTrsv.cData[0] = Flags; //Flag
	g_sPcdTrsv.cData[1] = ISO15693_GET_SYSTEM_INFOMATION;  //Command
	g_sPcdTrsv.cData[2] = 0;
	cIndex = 2;
	if ((Flags & 0x20))	// flags & 0x20 - Adresflag request flags & 
						// 0x10 - Selectflag request
	{
//		for (i=0; i<8; i++)
//		{
//			g_sPcdTrsv.cData[cIndex++] = UID[i];
//		}
		memcpy(&g_sPcdTrsv.cData[cIndex], UID, 8);
		cIndex += 8;
	}
	g_sPcdTrsv.iLength = cIndex;// + 1;
	SetTimeOut();
	cStatus = PcdTransceive(PCD_TRANSCEIVE);
	*cpReceiveLength = 0;
	if (cStatus == 0)
	{
		unsigned char j = (g_sPcdTrsv.iLength / 8) - 1;
		if ((g_sPcdTrsv.cData[0] & 0x01) == 1)
		{
			cStatus = 0xFF;
		}
		else
		{
//			for (i=0; i<j; i++)
//			{
//				cpReceiveData[i] = g_sPcdTrsv.cData[i+1];
//			}
			memcpy(cpReceiveData, &g_sPcdTrsv.cData[1], j);
			*cpReceiveLength = j;
		}
	}
	return  cStatus;
}

unsigned char ISO15693_GetBlockSecurity(unsigned char Flags, unsigned char *UID, 
							unsigned char cBlockNumber, unsigned char cNumberOfBlock,
							unsigned char *cpReceiveLength, unsigned char *cpReceiveData)
{
	unsigned char cStatus;
//	unsigned char i;
	unsigned char cIndex;
//	cStatus = Set15693Mode();
//	if (cStatus != 0)
//	{
//		return cStatus;
//	}
	g_sPcdTrsv.cData[0] = Flags; //Flag
	g_sPcdTrsv.cData[1] = ISO15693_GET_MULTIPLE_BLOCK_SECURITY_STATUS;  //Command
	cIndex = 2;
	if (Flags & 0x20)	// flags & 0x20 - Adresflag request flags & 
						// 0x10 - Selectflag request
	{
//		for (i=0; i<8; i++)
//		{
//			g_sPcdTrsv.cData[cIndex++] = UID[i];
//		}
		memcpy(&g_sPcdTrsv.cData[cIndex], UID, 8);
		cIndex += 8;
	}
	g_sPcdTrsv.cData[cIndex++] = cBlockNumber;
	g_sPcdTrsv.cData[cIndex++] = cNumberOfBlock;
	g_sPcdTrsv.iLength = cIndex;
	SetTimeOut();
	cStatus = PcdTransceive(PCD_TRANSCEIVE);
	*cpReceiveLength = 0;
	if (cStatus == 0)
	{
		unsigned char j = (g_sPcdTrsv.iLength / 8) - 1;
		if ((g_sPcdTrsv.cData[0] & 0x01) == 1)
		{
			cStatus = 0xFF;
		}
		else
		{
//			for (i=0; i<j; i++)
//			{
//				cpReceiveData[i] = g_sPcdTrsv.cData[i+1];
//			}
			memcpy(cpReceiveData, &g_sPcdTrsv.cData[1], j);
			*cpReceiveLength = j;
		}
	}
	return  cStatus;
}


unsigned char ISO15693_MultiTagAutoInventory(unsigned char * cpTagSNR, unsigned char cAFI, unsigned char cAFI_Enable)
{
	unsigned char j, k;
	unsigned char cCollByte;
	unsigned char cCollBit;
	unsigned char cStatus;
	unsigned char cMaskLength = 0;
	unsigned char cReceiveLength;
	unsigned char cFlag;

	cFlag = (cAFI_Enable == 0) ? 0x26:0x36;
	j = 0;
	do
	{
		j++;
		cStatus = ISO15693_Inventory(cFlag, cAFI, cMaskLength, cpTagSNR, &cReceiveLength, cpTagSNR);
	} while ((cStatus != MI_OK) && (cStatus != MI_COLLERR) && j < 4);
    
	if (cStatus == MI_COLLERR )
	{
		j = 0;
		do
		{
			j++;
			cMaskLength = ReadRawRC(0x0B) - 0x10;
			memcpy(cpTagSNR, &g_sPcdTrsv.cData[2], 8);
			cCollByte = (cMaskLength - 1) / 8;
			cCollBit  = (cMaskLength - 1) % 8;
			cpTagSNR[cCollByte] |= (0x01 << cCollBit);
			k = 0;
			do
			{
				k++;
				cStatus = ISO15693_Inventory(cFlag, cAFI, cMaskLength, cpTagSNR, &cReceiveLength, cpTagSNR);
			} while ((cStatus != MI_OK) && (cStatus != MI_COLLERR) && k < 4);
		} while (cStatus == MI_COLLERR && j < 64);
	}

    //printf("ISO15693_MultiTagAutoInventory = %02x\n",cStatus);
    
	return cStatus;
}


void SetTimeOut(void)
{
	unsigned char cReload;

	switch (g_sPcdTrsv.cData[1])
	{			   
		case ISO15693_STAY_QUIET:
				cReload = 0x04;
				break;
		case ISO15693_SELECT:
		case ISO15693_RESET_TO_READY:
				cReload = 0x0F;
				break;
		case ISO15693_LOCK_AFI:
		case ISO15693_LOCK_DSFID:
		case ISO15693_LOCK_BLOCK:
		case ISO15693_WRITE_SINGLE_BLOCK:
		//case ISO15693_WRITE_MULTIPLE_BLOCKS:
		case ISO15693_WRITE_AFI:
		case ISO15693_WRITE_DSFID:
				cReload = 0x29;
				break;
		case ISO15693_READ_SINGLE_BLOCK:
				cReload = 0x17;
				break;
		case ISO15693_INVENTORY:
				cReload = 0x1F;
				break;
		case ISO15693_GET_SYSTEM_INFOMATION:
				cReload = 0x25;
				break;
		case ISO15693_GET_MULTIPLE_BLOCK_SECURITY_STATUS:
				cReload = 0x40;
				break;
		case ISO15693_READ_MULTIPLE_BLOCK:
				cReload = 0x40;
				break;
		default:
				cReload = 0x86;
				break;
	}
	// T0理论值为0x0800，运行读块失败，改为0x0A00可以成功。
	// T0理论值为0x0800，运行StayQuiet不成功，所以修改为0x2A00。
	WriteRawRC(PHHAL_HW_RC663_REG_T0RELOADHI,(unsigned char)(0x2A00>>8));		// 有待进一步计算时间
	WriteRawRC(PHHAL_HW_RC663_REG_T0RELOADLO,(unsigned char)0x2A00);
	WriteRawRC(PHHAL_HW_RC663_REG_T1RELOADHI,(unsigned char)(cReload>>8));
	WriteRawRC(PHHAL_HW_RC663_REG_T1RELOADLO,(unsigned char)cReload);
	WriteRawRC(PHHAL_HW_RC663_REG_TCONTROL,0x33);	 //
}
