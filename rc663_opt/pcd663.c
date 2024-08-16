
#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>
#include <unistd.h>
#include <fcntl.h>  

#include "libcardlan_StandardBus_util.h"

#if defined(ANDROID_CODE_DEBUG)
#define TARGET_ANDROID
#endif
#if defined(NDK_CODE_DEBUG)
#define TARGET_DEBUG
#endif

#if defined(TARGET_ANDROID)
#ifndef LOG_TAG
#define LOG_TAG __FILE__
#endif
#include <android/log.h>
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#elif defined(TARGET_DEBUG)
#include <stdio.h>
#define  LOGI    printf
#define  LOGD    printf
#define  LOGE    printf
#else 
#define  LOGI    
#define  LOGD    
#define  LOGE
#endif

#define GPIO_CONTROL_PATH           "/proc/gpio_set/rp_gpio_set"
#define GPIO_RST_VAL_H              "b_31_1_1"
#define GPIO_RST_VAL_L              "b_31_1_0"


int rc663_rst(void)
{
    int fd; 

    fd = open(GPIO_CONTROL_PATH, O_WRONLY);
    if(fd == -1)
    {
       printf("ERR: -1.\n");
       return -1;
    }

    {
       write(fd, GPIO_RST_VAL_L, strlen(GPIO_RST_VAL_L));
       usleep(100000);
       write(fd, GPIO_RST_VAL_H, strlen(GPIO_RST_VAL_H));
       usleep(100000);
       write(fd, GPIO_RST_VAL_L, strlen(GPIO_RST_VAL_L));
    }
    close(fd);
    return 0;
}

#include "pcd.h"
#include "status.h"

#ifdef _PCD663_

tpd_pcdpara g_sPcdPara;
tpd_pcdtrsv g_sPcdTrsv;


//******************************************************************************
// Function: 复位并初始化RC663
//		 IN: void
//		RET: unsigned char 		= MI_OK
//                     			= MI_RESETERR
//******************************************************************************
unsigned char PcdReset(void)
{
	unsigned char cStatus = MI_OK;
	unsigned char n;
	unsigned int i = 0xFFFF;
    int ret = 0;

    ret = rc663_rst();
    if(ret != 0x00)
    {
        printf("[%s %d]ret :%d \n",__FUNCTION__,__LINE__,ret);
        return MI_RESETERR;
    }

	do 
    {
		n = ReadIO(0x00);
		i--; 
    }while (i != 0 && n != 0x40);

	if (i == 0)
	{
		cStatus = MI_RESETERR;
	}
	else
	{
		n = ReadIO(PHHAL_HW_RC663_REG_VERSION);
	}

	return cStatus;
}

//******************************************************************************
// Function: 读寄存器,指定寄存器地址,返回寄存器数据,无返回值.
//		 IN: unsigned char Address
//		RET: unsigned char
//******************************************************************************
unsigned char ReadRawRC(unsigned char cAddress)
{
    unsigned char status;
    status=ReadIO(cAddress);
    return status;
}
//******************************************************************************
//	 	IN:	unsigned char cAddress
//			unsigned char cValue
//	   RET: void
// Comment: 写寄存器,指定寄存器地址,输出数据,
//******************************************************************************
void WriteRawRC(unsigned char cAddress, unsigned char cValue)
{
    WriteIO(cAddress,cValue);
}

//******************************************************************************
//       IN: unsigned char reg
//           unsigned char mask
//      OUT: void
//   RETURN: void
//******************************************************************************
void SetBitMask(unsigned char reg,unsigned char mask)
{
	unsigned char tmp = ReadRawRC(reg);
	WriteRawRC(reg, tmp | mask);		// set bit mask
	return;
}
//******************************************************************************
//       IN: unsigned char reg
//           unsigned char mask
//      OUT: void
//   RETURN: void
//  COMMENT: 清RC500寄存器位
//******************************************************************************
void ClearBitMask(unsigned char reg,unsigned char mask)
{
	unsigned char tmp = ReadRawRC(reg);
	WriteRawRC(reg, tmp & ~mask);		// clear bit mask
	return;
}


//******************************************************************************
//       IN: unsigned char tmoLength
//      OUT: void
//   RETURN: void
//  COMMENT: 设置RC663定时器
//******************************************************************************
void PcdSetTmo(unsigned char cFWI)
{
	unsigned int cClock0 = 4096;
	unsigned int cClock1;
	unsigned char cTpcd = 2;

	// FWT+ΔFWT+ΔTpcd		
	// ΔFWT = 49152; ΔTpcd = 16.4ms = 222384;
	// cClock1 = (FWT+ΔFWT+ΔTpcd) / 4096;
	
	cClock1 = (1<<cFWI) + 12 + cTpcd;	// 计算方法
    //LOGI("cClock0 : %d,cClock1:%d\n",cClock0,cClock1);
	WriteRawRC(PHHAL_HW_RC663_REG_T0RELOADHI,(unsigned char)(cClock0>>8));
	WriteRawRC(PHHAL_HW_RC663_REG_T0RELOADLO,(unsigned char)cClock0);
	WriteRawRC(PHHAL_HW_RC663_REG_T1RELOADHI,(unsigned char)(cClock1>>8));
	WriteRawRC(PHHAL_HW_RC663_REG_T1RELOADLO,(unsigned char)cClock1);
//	WriteRawRC(PHHAL_HW_RC663_REG_T0CONTROL, 0x98);		// 时钟源为13.56
//	WriteRawRC(PHHAL_HW_RC663_REG_T1CONTROL, 0x9A);		// 时钟源为Timer0
	WriteRawRC(PHHAL_HW_RC663_REG_TCONTROL,0x33);
	
	return;
}
//******************************************************************************
//    PARAM: unsigned char RxPtl
//			 unsigned char TxPtl
//   RETURN: void
//  COMMENT: CLRC663 Datasheet 7.10.3.13 LoadProtocol command; Table 45;
//  		
//******************************************************************************
unsigned char phhalHw_Rc663_Cmd_LoadProtocol(unsigned char RxPtl, unsigned char TxPtl)
{
	unsigned char cStatus = MI_OK;
	
	PcdSetTmo(0);

	g_sPcdTrsv.cData[0] = RxPtl;
	g_sPcdTrsv.cData[1] = TxPtl;
    g_sPcdTrsv.iLength = 2;

	cStatus = PcdTransceive(PHHAL_HW_RC663_CMD_LOADPROTOCOL);
	
	return cStatus;
}
//******************************************************************************
//      IN: unsigned char cType
//  RETURN: void
// COMMENT: 配置基站操作卡类型

//******************************************************************************
unsigned char PcdConfig(unsigned char cType)
{
	unsigned char cStatus = ERR_FAILURE;

	if (cType == PCD_TYPE_14443A)			//Config Type A
	{
		cStatus = phhalHw_Rc663_Cmd_LoadProtocol(0x00, 0x00);	// 570us
		if (cStatus != 0x00)
		{
			// 当前次的寻卡失败后，协议配置可能失败，在此重新配置，此配置有效，但是会返回失败。B卡相同。
			cStatus = phhalHw_Rc663_Cmd_LoadProtocol(0x00, 0x00);
		}

		//----------Apply RegisterSet
		// 写一个寄存器时间3.3us
		WriteRawRC(PHHAL_HW_RC663_REG_T0CONTROL, 0X98); 		// Input clock is 13.56MHz
		WriteRawRC(PHHAL_HW_RC663_REG_T1CONTROL, 0X92); 		// Input clock is Timer0
//		WriteRawRC(PHHAL_HW_RC663_REG_T2CONTROL, 0X20);
//		WriteRawRC(PHHAL_HW_RC663_REG_T2RELOADHI, 0X03);
//		WriteRawRC(PHHAL_HW_RC663_REG_T2RELOADLO, 0XFF);
		
		WriteRawRC(PHHAL_HW_RC663_REG_FIFOCONTROL, 0x10);			// confirmed
		WriteRawRC(PHHAL_HW_RC663_REG_WATERLEVEL, _WATER_LEVEL_);	// confirmed
		WriteRawRC(PHHAL_HW_RC663_REG_RXBITCTRL, 0x88);				// confirmed
		WriteRawRC(PHHAL_HW_RC663_REG_DRVMODE, 0X8D);
		WriteRawRC(PHHAL_HW_RC663_REG_TXAMP, 0X80);			// 
		WriteRawRC(PHHAL_HW_RC663_REG_DRVCON, 0X01);
		WriteRawRC(PHHAL_HW_RC663_REG_TXI, 0x05);
		WriteRawRC(PHHAL_HW_RC663_REG_RXSOFD, 0X00);
		WriteRawRC(PHHAL_HW_RC663_REG_RCV, 0X12);
////////////////////////////////////////////////////////////////////////////////////////////////
		WriteRawRC(PHHAL_HW_RC663_REG_TXCRCCON, 0x18);		
		WriteRawRC(PHHAL_HW_RC663_REG_RXCRCCON, 0x18); 
		WriteRawRC(PHHAL_HW_RC663_REG_TXDATANUM, 0x08);
		WriteRawRC(PHHAL_HW_RC663_REG_TXMODWIDTH, 0x27);
		WriteRawRC(PHHAL_HW_RC663_REG_TXSYM10BURSTLEN, 0x00);
		WriteRawRC(PHHAL_HW_RC663_REG_TXWAITCTRL, 0xc0);
		WriteRawRC(PHHAL_HW_RC663_REG_TXWAITLO, 0x12);
		WriteRawRC(PHHAL_HW_RC663_REG_FRAMECON, 0xCF);
		WriteRawRC(PHHAL_HW_RC663_REG_RXCTRL, 0x04);
		WriteRawRC(PHHAL_HW_RC663_REG_RXTHRESHOLD, 0x7F);	// tested value: 0x7F; recommanded value: 0x32
		WriteRawRC(PHHAL_HW_RC663_REG_RXANA, 0x0F);			// tested value: 0x0F; recommanded value: 0x0A
		WriteRawRC(PHHAL_HW_RC663_REG_RXWAIT, 0x90);
		
		//WriteRawRC(PHHAL_HW_RC663_REG_SIGOUT, 0x19);
		

//		WriteRawRC(PHHAL_HW_RC663_REG_DRVMODE, 0x82);
//		WriteRawRC(PHHAL_HW_RC663_REG_STATUS, 0x00);
//		WriteRawRC(PHHAL_HW_RC663_REG_DRVMODE, 0x8A);	//FieldOn
       
	}
	else if (cType == PCD_TYPE_14443B)		// Config Type B
	{
		//----- LoadProtocol( bTxProtocol=04, bRxProtocol=04)
		cStatus = phhalHw_Rc663_Cmd_LoadProtocol(0x04, 0x04);
		if (cStatus != 0x00)
		{
			cStatus = phhalHw_Rc663_Cmd_LoadProtocol(0x04, 0x04);
		}

		//----------Apply RegisterSet
		WriteRawRC(PHHAL_HW_RC663_REG_T0CONTROL, 0X98); 	// Input clock is 13.56MHz
		WriteRawRC(PHHAL_HW_RC663_REG_T1CONTROL, 0X92); 	// Input clock is Timer0
//		WriteRawRC(PHHAL_HW_RC663_REG_T2CONTROL, 0X20);
//		WriteRawRC(PHHAL_HW_RC663_REG_T2RELOADHI, 0X03);
//		WriteRawRC(PHHAL_HW_RC663_REG_T2RELOADLO, 0XFF);
		
		WriteRawRC(PHHAL_HW_RC663_REG_FIFOCONTROL, 0X10);
		WriteRawRC(PHHAL_HW_RC663_REG_WATERLEVEL, _WATER_LEVEL_);
		WriteRawRC(PHHAL_HW_RC663_REG_RXBITCTRL, 0X80);
		WriteRawRC(PHHAL_HW_RC663_REG_DRVMODE, 0X8F);
		WriteRawRC(PHHAL_HW_RC663_REG_TXAMP, 0X8F);			//  0xCF:	0CM 13,8  4CM 11.2
		WriteRawRC(PHHAL_HW_RC663_REG_DRVCON, 0X01);
		WriteRawRC(PHHAL_HW_RC663_REG_TXI, 0X05);
		WriteRawRC(PHHAL_HW_RC663_REG_RXSOFD, 0X00);
		WriteRawRC(PHHAL_HW_RC663_REG_RCV, 0X12);

////////////////////////////////////////////////////////////////////////////////////////////////
		WriteRawRC(PHHAL_HW_RC663_REG_TXCRCCON, 0x7B);
		WriteRawRC(PHHAL_HW_RC663_REG_RXCRCCON, 0x7B); 
		WriteRawRC(PHHAL_HW_RC663_REG_TXDATANUM, 0x08);
		WriteRawRC(PHHAL_HW_RC663_REG_TXMODWIDTH, 0x00);
		WriteRawRC(PHHAL_HW_RC663_REG_TXSYM10BURSTLEN, 0x00);
		WriteRawRC(PHHAL_HW_RC663_REG_TXWAITCTRL, 0x01);
		//WriteRawRC(PHHAL_HW_RC663_REG_TXWAITLO, 0x0B);
		WriteRawRC(PHHAL_HW_RC663_REG_FRAMECON, 0x05);
		WriteRawRC(PHHAL_HW_RC663_REG_RXSOFD, 0x00);
		WriteRawRC(PHHAL_HW_RC663_REG_RXCTRL, 0x34);
		WriteRawRC(PHHAL_HW_RC663_REG_RXTHRESHOLD, 0x7F);		//tested value: 0x7F; recommanded value: 0x3F
		WriteRawRC(PHHAL_HW_RC663_REG_RXANA, 0x06);				//tested value: 0x06; recommanded value: 0x0A
		WriteRawRC(PHHAL_HW_RC663_REG_RXWAIT, 0x9F);

//		WriteRawRC(PHHAL_HW_RC663_REG_DRVMODE, 0x8F); 	//FieldOn

	}
    else if (cType == PCD_TYPE_15693)
	{
		cStatus = phhalHw_Rc663_Cmd_LoadProtocol(0x0A, 0x0A);
		//cStatus = phhalHw_Rc663_Cmd_LoadProtocol(0x0B, 0x0B);	
		//----------Apply RegisterSet
		WriteRawRC(PHHAL_HW_RC663_REG_T0CONTROL, 0X98);
		WriteRawRC(PHHAL_HW_RC663_REG_T1CONTROL, 0X9A);
		WriteRawRC(PHHAL_HW_RC663_REG_T2CONTROL, 0X20);
		WriteRawRC(PHHAL_HW_RC663_REG_T2RELOADHI, 0X03);
		WriteRawRC(PHHAL_HW_RC663_REG_T2RELOADLO, 0XFF);
		
		WriteRawRC(PHHAL_HW_RC663_REG_FIFOCONTROL,0XB0);
		WriteRawRC(PHHAL_HW_RC663_REG_WATERLEVEL,_WATER_LEVEL_);
		WriteRawRC(PHHAL_HW_RC663_REG_RXBITCTRL,0X80);
		WriteRawRC(PHHAL_HW_RC663_REG_DRVMODE,0X8F);
		WriteRawRC(PHHAL_HW_RC663_REG_TXAMP,0XCF);			// 0x80
		WriteRawRC(PHHAL_HW_RC663_REG_DRVCON,0X01);
		WriteRawRC(PHHAL_HW_RC663_REG_TXI,0X05);
		WriteRawRC(PHHAL_HW_RC663_REG_RXSOFD,0X00);
		WriteRawRC(PHHAL_HW_RC663_REG_RCV,0X12);

		//----------Apply RegisterSet
		WriteRawRC(PHHAL_HW_RC663_REG_TXCRCCON,0x7B);
		WriteRawRC(PHHAL_HW_RC663_REG_RXCRCCON,0x7B); 
		WriteRawRC(PHHAL_HW_RC663_REG_TXDATANUM,0x08);
		WriteRawRC(PHHAL_HW_RC663_REG_TXMODWIDTH,0x00);
		WriteRawRC(PHHAL_HW_RC663_REG_TXSYM10BURSTLEN,0x00);
		WriteRawRC(PHHAL_HW_RC663_REG_FRAMECON,0x0F);
		WriteRawRC(PHHAL_HW_RC663_REG_RXCTRL,0x02);
		WriteRawRC(PHHAL_HW_RC663_REG_RXTHRESHOLD,0x44);
		WriteRawRC(PHHAL_HW_RC663_REG_RXANA,0x06);
		//WriteRawRC(PHHAL_HW_RC663_REG_RXWAIT, 0x10);
		WriteRawRC(PHHAL_HW_RC663_REG_TXWAITCTRL,0x88);
		WriteRawRC(PHHAL_HW_RC663_REG_TXWAITLO,0xA9);
		WriteRawRC(PHHAL_HW_RC663_REG_RXSOFD,0x00);
		WriteRawRC(PHHAL_HW_RC663_REG_RCV,0x12);

	}
	else
	{
		return 0xFF;
	}
//	Delay100uS(30);
    usleep(3000);
	return cStatus;
}
//******************************************************************************
//       IN: unsigned char cCommand
//   RETURN: unsigned char
//  COMMENT: PCD and PICC transcive
//******************************************************************************
unsigned char PcdTransceive(unsigned char cCommand)
{
	unsigned char recebyte = 0;
	unsigned char cStatus = 0x01;
	unsigned char irq0En = 0x00;
	unsigned char irq1En = 0x00;
	unsigned char cIrq0waitFor;
	unsigned char cIrq1waitFor;
	unsigned char n1, n2, j;
	int i;
	unsigned char cError;

	n2 = 0x00;
	cIrq0waitFor = 0x00;
	cIrq1waitFor = 0x00;
	cError = ReadRawRC(PHHAL_HW_RC663_REG_ERROR);
	switch (cCommand)
	{
		case PHHAL_HW_RC663_CMD_WRITEE2:
			irq0En = 0x10;
			irq1En = 0x40;
			cIrq0waitFor = 0x00;
			cIrq1waitFor = 0x40;
			break;
		case PHHAL_HW_RC663_CMD_READE2:
			irq0En = 0x10;
			irq1En = 0x40;
			cIrq0waitFor = 0x00;
			cIrq1waitFor = 0x40;
			recebyte=1;
			break;
		case PHHAL_HW_RC663_CMD_LOADPROTOCOL:
			irq0En = 0x10;
			irq1En = 0x42;
			cIrq0waitFor = 0x10;
			cIrq1waitFor = 0x02;
			break;
		case PHHAL_HW_RC663_CMD_LOADKEY:
			irq0En = 0x18;
			irq1En = 0x42;
			cIrq0waitFor = 0x00;
			cIrq1waitFor = 0x40;
			break;
		case PHHAL_HW_RC663_CMD_MFAUTHENT:
			irq0En = 0x18;//0x14;
			irq1En = 0x42;
			cIrq0waitFor = 0x00;
			cIrq1waitFor = 0x40;
			break;
		case PHHAL_HW_RC663_CMD_TRANSMIT:
			irq0En = 0x18;//0x0A;
			irq1En = 0x42;
			cIrq0waitFor = 0x08;
			cIrq1waitFor = 0x42;
			break;
		case PHHAL_HW_RC663_CMD_TRANSCEIVE:
			irq0En = 0x14;
			irq1En = 0x42;
			cIrq0waitFor = 0x10;
			cIrq1waitFor = 0x02;
			recebyte=1;
			break;
		default:
			cCommand = MI_UNKNOWN_COMMAND;
			break;
	}
	if (cCommand != MI_UNKNOWN_COMMAND)
	{
		WriteRawRC(PHHAL_HW_RC663_REG_COMMAND, PHHAL_HW_RC663_CMD_IDLE);
		WriteRawRC(PHHAL_HW_RC663_REG_IRQ0, 0x7F);		//clear all IRQ0 flags
		WriteRawRC(PHHAL_HW_RC663_REG_IRQ1, 0x7F);		//clear all IRQ1 flags
		WriteRawRC(PHHAL_HW_RC663_REG_IRQ0EN, 0x00);	//clear all IRQ0 source
		WriteRawRC(PHHAL_HW_RC663_REG_IRQ1EN, 0x00);	//clear all IRQ1 source 
		WriteRawRC(PHHAL_HW_RC663_REG_IRQ0EN, irq0En);
		WriteRawRC(PHHAL_HW_RC663_REG_IRQ1EN, irq1En);
		SetBitMask(PHHAL_HW_RC663_REG_FIFOCONTROL, 0x10);	//FlushFIFO();
		
		for (i=0; i<g_sPcdTrsv.iLength; i++)
		{
			WriteRawRC(PHHAL_HW_RC663_REG_FIFODATA, g_sPcdTrsv.cData[i]);
		}
		WriteRawRC(PHHAL_HW_RC663_REG_COMMAND, cCommand);
        g_sPcdTrsv.iLength = 0;
        
		i = 2000;
		do
		{
			n1 = ReadRawRC(PHHAL_HW_RC663_REG_IRQ0);
			n2 = ReadRawRC(PHHAL_HW_RC663_REG_IRQ1);
			i--;
		} while ((i != 0) && !(n1 & cIrq0waitFor) && !(n2 & cIrq1waitFor));
        if(i < 1000)
		{
		    printf("[ %s %d ] i : %d n2: 0x%02X \n",__FUNCTION__,__LINE__,i,n2);
		}
        if ((i != 0) && !(n2 & 0x02))
		{
			cError = ReadRawRC(PHHAL_HW_RC663_REG_ERROR);
			if(0 != cError)
			{
			    printf("[ %s %d ]end cError : %2X\n",__FUNCTION__,__LINE__,cError);
			}
            
			if(0 == cError)
			{
				cStatus = MI_OK;
			}
			else if (cError & 0x02)
			{
				cStatus = MI_FRAMINGERR;
			}
			else if (cError & 0x04)
			{
				cStatus = MI_COLLERR;
                cError = ReadRawRC(PHHAL_HW_RC663_REG_RXCOLL);
			}
			else if (cError & 0x01)
			{
				cStatus = MI_CRCERR;
			}
			else
			{
				cStatus = MI_FATAL_ERR;
			}
			if (recebyte)
			{
				n1 = ReadRawRC(PHHAL_HW_RC663_REG_FIFOLENGTH);
				n2 = ReadRawRC(PHHAL_HW_RC663_REG_RXBITCTRL) & 0x07;
				if (n2 != 0)
				{
					g_sPcdTrsv.iLength = (n1 - 1) * 8 + n2;
				}
				else
				{
					g_sPcdTrsv.iLength = n1;
				}

				for (j = 0; j < n1; j++)
				{
					g_sPcdTrsv.cData[j] = ReadRawRC(PHHAL_HW_RC663_REG_FIFODATA);
				}
			}
		}
		else if (n2 & 0x02)
		{
			cStatus = MI_NOTAGERR;
		}
		else
		{
			cStatus = MI_COM_ERR;
		}
	}
	WriteRawRC(PHHAL_HW_RC663_REG_COMMAND, PHHAL_HW_RC663_CMD_IDLE);
	WriteRawRC(PHHAL_HW_RC663_REG_IRQ0, 0x7F);		//clear all IRQ0 flags
	WriteRawRC(PHHAL_HW_RC663_REG_IRQ1, 0x7F); 		//clear all IRQ1 flags
	WriteRawRC(PHHAL_HW_RC663_REG_IRQ0EN, 0x00);	//clear all IRQ0 source
	WriteRawRC(PHHAL_HW_RC663_REG_IRQ1EN, 0x00); 	//clear all IRQ1 source 
	WriteRawRC(PHHAL_HW_RC663_REG_TCONTROL, 0x33);		// stop timer now
	SetBitMask(PHHAL_HW_RC663_REG_T0CONTROL, 0x80);	// set timer auto stop
	SetBitMask(PHHAL_HW_RC663_REG_T1CONTROL, 0x80);	// set timer auto stop
	
//	Delay100uS(5);			// FDT PCD,MIN
    if(cStatus==MI_OK)
    {
        //    DBGPRINTK("PcdTransceive:");
    	//	print_char(g_sPcdTrsv.iLength, g_sPcdTrsv.cData);
    }
    usleep(50);
	return cStatus;
}

//******************************************************************************
//      IN: unsigned short iTime
//  RETURN: void
// COMMENT: 
//  		
//******************************************************************************
void M500PcdRfReset(unsigned short iTime)
{
	if (iTime != 0)
	{
		ClearBitMask(PHHAL_HW_RC663_REG_DRVMODE, 0x08);
	//	Delay100uS(iTime * 10);            // Delay for 1 ms
	     usleep(iTime * 10*100);
		SetBitMask(PHHAL_HW_RC663_REG_DRVMODE, 0x08);
	}
	else
	{
		ClearBitMask(PHHAL_HW_RC663_REG_DRVMODE, 0x08);
	}
	return;
}
//******************************************************************************
//      IN: unsigned char cFSDI
//  RETURN: void
// COMMENT: FSDI转换为FSD
//  		
//******************************************************************************
void PcdSetFSD(unsigned char cFSDI)
{
	switch (cFSDI)
	{
		case 0:					// 14
		case 1:					// 22
		case 2:					// 30
		case 3:					// 38
		case 4:					// 46
			g_sPcdPara.cFSD = 14 + cFSDI * 8;
			break;
		case 5:
			g_sPcdPara.cFSD = 62;
			break;
		case 6:
			g_sPcdPara.cFSD = 94;
			break;
		case 7:
			g_sPcdPara.cFSD = 126;
			break;
		default:
			g_sPcdPara.cFSD = 254;
			break;
	}
	return;
}


// PCD支持的函数
//******************************************************************************
//      IN: unsigned char cSta			// 0 = Disable; else = Enable;
//     OUT: void
//  RETURN: void
// COMMENT: Enable or Disable RegTxControl 天线控制
//  		
//******************************************************************************
void PcdAntenna(unsigned char cSta)
{
	unsigned char cTemp = ReadRawRC(PHHAL_HW_RC663_REG_DRVMODE);
	if (cSta)
	{
		WriteRawRC(PHHAL_HW_RC663_REG_DRVMODE, cTemp | 0x08);
	}
	else
	{
		WriteRawRC(PHHAL_HW_RC663_REG_DRVMODE, cTemp & (~0x08));
	}
}

#endif

