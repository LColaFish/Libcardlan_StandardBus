
#ifndef _M1_Plus_CARD_OPEATION_H_
#define _M1_Plus_CARD_OPEATION_H_

#include "card_opt/M1/M1_card_operation.h"

typedef enum { KT_KEY_A = 0x60, KT_KEY_B = 0x61 } MifareKeyType;

/**d* MifPlusAPI/MFP_SAK_LEVEL_1
 *
 * NAME
 *   MFP_SAK_LEVEL_1
 *
 * DESCRIPTION
 *   SAK of a Mifare Plus at Level 1
 *
 **/
#define MFP_SAK_LEVEL_1                    0x18

/**d* MifPlusAPI/MFP_SAK_LEVEL_2
 *
 * NAME
 *   MFP_SAK_LEVEL_2
 *
 * DESCRIPTION
 *   SAK of a Mifare Plus at Level 2
 *
 **/
#define MFP_SAK_LEVEL_2                    0x11

/**d* MifPlusAPI/MFP_SAK_LEVEL_0_3
 *
 * NAME
 *   MFP_SAK_LEVEL_0_3
 *
 * DESCRIPTION
 *   SAK of a Mifare Plus at Level 0 or 3
 *
 **/
#define MFP_SAK_LEVEL_0_3                  0x20

enum  MifarePlusBlockKeyLocation 
{
    MF_CARD_MASTER_KEY = 0x9000,
    MF_CARD_CONFIGURATION_KEY = 0x9001,
    MF_L2_SWITCH_KEY = 0x9002,
    MF_L3_SWITCH_KEY = 0x9003,
    MF_L1_CARD_AUTH_KEY = 0x9004
};

/*
 * Mifare Plus commands (codes sent by the PCD)
 * --------------------------------------------
 */

#define MFP_CMD_READ                       0x30
#define MFP_CMD_READ_MACED                 0x31
#define MFP_CMD_READ_PLAIN                 0x32
#define MFP_CMD_READ_PLAIN_MACED           0x33
#define MFP_CMD_READ_UNMACED               0x34
#define MFP_CMD_READ_UNMACED_R_MACED       0x35
#define MFP_CMD_READ_PLAIN_UNMACED         0x36
#define MFP_CMD_READ_PLAIN_UNMACED_R_MACED 0x37

#define MFP_CMD_SELECT_VIRTUAL_CARD        0x40
#define MFP_CMD_VIRTUAL_CARD_SUPPORT       0x42
#define MFP_CMD_DESELECT_VIRTUAL_CARD      0x48
#define MFP_CMD_VIRTUAL_CARD_SUPPORT_LAST  0x4B

#define MFP_CMD_FIRST_AUTHENTICATE         0x70
#define MFP_CMD_AUTHENTICATE_PART_2        0x72
#define MFP_CMD_FOLLOWING_AUTHENTICATE     0x76
#define MFP_CMD_RESET_AUTHENTICATION       0x78

#define MFP_CMD_WRITE                      0xA0
#define MFP_CMD_WRITE_MACED                0xA1
#define MFP_CMD_WRITE_PLAIN                0xA2
#define MFP_CMD_WRITE_PLAIN_MACED          0xA3

#define MFP_CMD_WRITE_PERSO                0xA8
#define MFP_CMD_COMMIT_PERSO               0xAA


/*0级函数*/
/*
	功能：设置个人化数据,可以设置卡的密钥，0级状态下不需安全机制，直接可以设置。

	参数：
		   [in]BNr:--------------要写入的个人化数据块号；

		   [in]dataperso:--------要写入的数据，16个字节

	返回： 0表示执行成功，其他失败。
*/
int MFPL0_write_perso(unsigned char hex_addr_1,unsigned char hex_addr_2,unsigned char dataperso[16]);
/*
	功能：个人化卡片 ，个人化后卡片进入1级状态。

	参数: 


	返回：0表示成功、其他失败。
*/
int MFPL0_commit_perso(void);
/*1级函数*/
/*
	功能：状态切换函数，执行该操作后，1级状态的卡片转换到2级。

	参数：

		[in]authkey:-----------升级密码；
		[in]len:-----------authkey长度
	返回：

		0表示成功、其他失败。
*/
int MFPL1_switch_to_level2(unsigned char authkey[16]);

/*
	功能：状态切换函数，执行该操作后，1级状态的卡片转换到3级。

	参数：

		[in]authkey:-----------升级密码；
		[in]len:-----------authkey长度
	返回：

		0表示成功、其他失败。
*/
int MFPL1_switch_to_level3(unsigned char authkey[16]);
/*
	功能：在1级状态是和MIFARE系列卡一样操作的

	参数：


		0表示成功、其他失败。
*/

int MFPL1_authenticate(unsigned char SectorNo, unsigned char *authkey,unsigned char type,unsigned char UID[4]);
/*
	功能：在1级状态是和MIFARE系列卡一样操作的，此函数提供了一个专用的1级状态卡片认证功能，
		  可以在1级状态下实现严格的认证功能。（注意执行该命令卡片不要执行RATS操作）

	参数：

		[in]authkey:-----------16字节1级状态验证密钥返回：

		0表示成功、其他失败。
*/
int MFPL1_authl1key(unsigned char authkey[16]);
/*2级函数*/
/*
	功能：状态切换函数，执行该操作后，2级状态的卡片转换到3级。

	参数：

		[in]authkey:-----------升级密码；
		[in]len:-----------authkey长度
	返回：

		0表示成功、其他失败。


*/
int MFPL2_switch_to_level3(unsigned char *authkey,int len);

/*3级函数*/
/*
	功能：3级状态加密读块函数，可以连续读多块，每块16字节

	参数：

		[in]BNr:------------起始块地址。

		[in]Numblock:-------读取块数目。

		[out]readdata:------返回读取的数据。注意长度为 Numblock*16

		[in]flag:-----------1 表示读出的加密数据直接返回；0 表示解密后再返回

	返回：

		0表示成功、其他失败。
*/
int MFPL3_read_encrypted(unsigned short BNr,unsigned char Numblock,unsigned char *readdata, unsigned char flag);
/*
	功能： 3级状态写块函数，可以连续写多块，每块16字节；写密钥块必须要此函数，注意每次写1块。

	参数：

		[in]BNr:------------起始块地址。

		[in]Numblock:-------块数目。（0-6、写密码时只允许为1）

		[in]data:-----------数据。（长度必须是16的倍数关系）

		[in]flag:-----------1：表示要写入的数据已经是加密的；0：表示写入的数据需要加密后再写入

	返回：

		0表示成功、其他失败。
*/
int MFPL3_write_encrypted(unsigned short BNr,unsigned char Numblock, unsigned char *data, unsigned char flag);
/*
	功能：3级状态读块函数，可以连续读多块，每块16字节

	参数：

		BNr：---------------块地址，从此地址开始读卡

		Numblock：----------要读取的块数目 （一般小于6）

		readdata：----------返回读取的数据。注意长度为 Numblock*16

	返回：成功则返回0
*/
int MFPL3_read_in_plain(unsigned short BNr,unsigned char Numblock, unsigned char *readdata,int *readlen);


/*
	功能：3级状态写块函数，可以连续写多块，每块16字节

	参数：
	
		BNr： -----------块地址，从此地址开始写卡

		Numblock：-------要写的块数目 （一般小于6）

		writedata：------存放要写入卡内的数据

	返回：成功则返回0
*/
int MFPL3_write_in_plain(unsigned short BNr,unsigned char Numblock,unsigned char *writedata,unsigned char write_len);

/*
	功能： 3级状态卡片认证函数，根据密钥号的不同，验证不同的密钥。

	参数：

		keyBNr：-----------密钥块号

		authkey：----------16字节密钥

		type： ----------密码验证模式. =0 验证A密码 =4 验证B密码

	返回：成功则返回0
*/
int MFPL3_authl3_key_ex(unsigned short keyBNr,unsigned char *authkey);
int MFPL3_authl3_key(int sector,unsigned char *authkey,MifareKeyType type);
int MFP_detect_Security_Level(void);


#endif  //_M1_Plus_CARD_OPEATION_H_



