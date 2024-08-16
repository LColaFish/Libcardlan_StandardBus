/*******************************************************************************
 * CopyRight(c)2010-2017, BJ-TOP Electronics Co.,Ltd. All rights reserved
 * FileName: ISO15693.h
 *   Author: Layne Wen
 *     Date: 2017.12.20
 *  COMMENT: 版本日期更新
 ******************************************************************************/
#ifndef _ISO15693_H_
#define _ISO15693_H_

/*****************************************************************************
* 15693 卡命令字
*****************************************************************************/
#define ISO15693_INVENTORY                          0x01
#define ISO15693_STAY_QUIET                         0x02
#define ISO15693_READ_SINGLE_BLOCK                  0x20
#define ISO15693_WRITE_SINGLE_BLOCK                 0x21
#define ISO15693_LOCK_BLOCK                         0x22
#define ISO15693_READ_MULTIPLE_BLOCK                0x23
#define ISO15693_WRITE_MULTIPLE_BLOCK               0x24
#define ISO15693_SELECT                             0x25
#define ISO15693_RESET_TO_READY                     0x26
#define ISO15693_WRITE_AFI                          0x27
#define ISO15693_LOCK_AFI                           0x28
#define ISO15693_WRITE_DSFID                        0x29
#define ISO15693_LOCK_DSFID                         0x2A
#define ISO15693_GET_SYSTEM_INFOMATION              0x2B
#define ISO15693_GET_MULTIPLE_BLOCK_SECURITY_STATUS 0x2C

#define RC663_15693WRITE                            0x42
#define RC663_15693READ                             0x02
#define RC663_15693SELECT                           0x20
#define RC663_15693Quiet                          	0x23


unsigned char ISO15693_Inventory(unsigned char Flags, unsigned char AFI,
						unsigned char Mask_Length, unsigned char *Mask,
						unsigned char *RecLength, unsigned char *RecData);
unsigned char ISO15693_StayQuiet(unsigned char Flags, unsigned char *UID);
unsigned char ISO15693_Select(unsigned char Flags, unsigned char *UID);
unsigned char ISO15693_ResetToReady(unsigned char Flags, unsigned char *UID);
unsigned char ISO15693_ReadBlock(unsigned char Flags, unsigned char *UID, 
                       unsigned char cBlockNumber, unsigned char cNumberOfBlock,
					   unsigned char *cpReceiveLength, unsigned char *cpReceiveData);
unsigned char ISO15693_Write(unsigned char Flags, unsigned char *UID, 
							unsigned char cBlockNumber,
							unsigned char *cpWriteData);
unsigned char ISO15693_LockBlock(unsigned char Flags, unsigned char *UID, 
						unsigned char cBlockNumber);
unsigned char ISO15693_WriteAFI(unsigned char Flags, unsigned char *UID, 
						unsigned char cAFI);
unsigned char ISO15693_LockAFI(unsigned char Flags, unsigned char *UID);
unsigned char ISO15693_WriteDSFID(unsigned char Flags, unsigned char *UID, 
						unsigned char cDSFID);
unsigned char ISO15693_LockDSFID(unsigned char Flags, unsigned char *UID);
unsigned char ISO15693_GetBlockSecurity(unsigned char Flags, unsigned char *UID, 
							unsigned char cBlockNumber, unsigned char cNumberOfBlock,
							unsigned char *cpReceiveLength, unsigned char *cpReceiveData);
unsigned char ISO15693_GetSystemInformation(unsigned char Flags, unsigned char *UID, 
                       unsigned char *cpReceiveLength, unsigned char *cpReceiveData);
unsigned char ISO15693_MultiTagInventory(unsigned char cAFI_Enable, unsigned char cAFI,
						unsigned char * cpReceiveBytes, unsigned char * cpTagSNR);
unsigned char ISO15693_MultiTagAutoInventory(unsigned char * cpTagSNR, unsigned char cAFI, unsigned char cAFI_Enable);

void SetTimeOut(void);
						
#endif

