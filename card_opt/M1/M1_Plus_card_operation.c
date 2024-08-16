#include "M1_Plus_card_operation.h"
#include <stdio.h>
#include "libcardlan_StandardBus_util.h"
#include "common/cardlan_StandardBus_tpye.h"
#include "card_opt/apdu_cmd.h"
#include "openssl/aes.h"
#include "openssl/cmac.h"

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
/*最好用get set reset 控制这些变量*/
static unsigned char authkey_bak[16];
static unsigned char SessionKeyEnc[16];
static unsigned char SessionKeyMac[16];
static unsigned char trans_id[4];
static unsigned short write_count_;
static unsigned short read_count_;

static int is_trailing_block(unsigned char hex_addr1, unsigned char  hex_addr2);
static int validate_access_bytes(unsigned char data[16]);
static int probeLevel3(void);
static int isLevel0(void);
static int reset_Auth();
static int firstAuthenticate(unsigned short keyBNr,unsigned char *authkey);
static int aes_first_auth_step2(    unsigned char rnd_b[16],unsigned char *authkey);
static int aes_first_auth_final(unsigned char rnd_a[16],unsigned char rnd_b[16],unsigned char *authkey,unsigned char encrypted_data[32]);

static int deriveKEnc(unsigned char rnd_a[16],unsigned char rnd_b[16],unsigned char authkey[16],unsigned char trans_id[4],unsigned char SessionKeyEnc[16]);
static int deriveKMac(unsigned char rnd_a[16],unsigned char rnd_b[16],unsigned char authkey[16],unsigned char trans_id[4],unsigned char SessionKeyMac[16]);
static int computeWriteMac(unsigned char command_code,unsigned short block, unsigned char *data,unsigned char data_len,unsigned char write_mac[8]);
static int computeReadMac(unsigned char command_code,unsigned short block, unsigned char *data,unsigned char data_len,unsigned char write_mac[8]);
static int cipherData(const unsigned char *data,unsigned char data_len,unsigned char out_data[16]);


static int probeLevel3(void)
{
    int ret = 0;
    unsigned char recv[128];
    unsigned char cmd[]  = {MFP_CMD_FIRST_AUTHENTICATE, 0x00, 0x40,0x01,0x00};
    // Attempt to auth with the AES key 0
    ret = mifare_read_and_write(cmd,sizeof(cmd), recv);
    if(ret < 1 || recv[0] != 0x90)
    {
        LOGI("[%s %d] mifare_write fail ret %d \n",__FUNCTION__,__LINE__,ret);
        return HANDLE_ERR_CMD;
    }

    return HANDLE_OK;
}

static int isLevel0(void)
{
    int ret = 0;
    // Attempt to write full 0x00 in block 0x01, sector 0x00.
    unsigned char data[16] = {0};
	ret = MFPL0_write_perso(0x00, 0x01, data);
    if(ret != 0)
    {
        return HANDLE_ERR_CMD;
    }
    return HANDLE_OK;
}

int MFP_detect_Security_Level(void)
{
    if (probeLevel3() == HANDLE_OK)
    {
        return 3;
    }
    if (isLevel0() == HANDLE_OK)
    {
        return 0;
    }
    return -1;
}

/*
	return true : 1 false : 0
*/
static int validate_access_bytes(unsigned char data[16])
{
#if 0
	std::array<unsigned char, 3> access_bytes = { {data[6], data[7], data[8]} };
    MifareAccessInfo::SectorAccessBits sab;
    return sab.fromArray(&access_bytes[0], 3);
#endif 
	return 1;
}

/*
	return true : 1 false : 0
*/
int is_trailing_block(unsigned char  hex_addr1, unsigned char  hex_addr2)
{
    if (hex_addr1 == 0)
    {
        if (hex_addr2 <= 0x7F)
        {
            if (hex_addr2 % 4 == 3)
                return 1;
            return 0;
        }
        else
        {
            if (hex_addr2 % 16 == 15)
                return 1;
            return 0;
        }
    }
    return 0;
}


/*0级函数*/
/*
	功能：设置个人化数据,可以设置卡的密钥，0级状态下不需安全机制，直接可以设置。

	参数：
		   [in]hex_addr_1 hex_addr_2:--------------要写入的个人化数据块号； 

		   [in]dataperso:--------要写入的数据，16个字节

	返回： 0表示执行成功，其他失败。
*/
int MFPL0_write_perso(unsigned char hex_addr_1,unsigned char hex_addr_2,unsigned char dataperso[16])
{
    int ret = 0;
    unsigned char recv[128];
    unsigned char cmd[19];
	cmd[0] = MFP_CMD_WRITE_PERSO;
	cmd[1] = hex_addr_2;
	cmd[2] = hex_addr_1;
	memcpy(&cmd[3],dataperso,16);
    int   receive_len[1] = {0};

	if (is_trailing_block(hex_addr_1, hex_addr_2) && !validate_access_bytes(dataperso))
    {
        LOGI("Access conditions bytes are invalid. Refusing to write.");
        return HANDLE_ERR_ARG_INVAL;
    }

	/*send command*/
    ret = mifare_read_and_write(cmd, sizeof(cmd),recv);
    if(recv[0] != 0x90 )
    {
        LOGI("[%s %d] mifare_write fail result %02X \n",__FUNCTION__,__LINE__,recv[0]);
        return HANDLE_ERR_CMD;
    }

	return HANDLE_OK;
}
/*
	功能：个人化卡片 ，个人化后卡片进入1级状态。

	参数: 


	返回：0表示成功、其他失败。
*/
int MFPL0_commit_perso(void)
{
    int receive_len[1] = {0};
    int ret = 0;
    unsigned char cmd[1] = {MFP_CMD_COMMIT_PERSO}; 
    unsigned char recv[128];
    
     /*send command*/
    ret = mifare_read_and_write(cmd, sizeof(cmd),recv);
    if(recv[0] != 0x90 )
    {
        LOGI("[%s %d] mifare_write fail result %02X \n",__FUNCTION__,__LINE__,recv[0]);
        return -1;
    }

	return 0;
}

int MFPL1_authenticate(unsigned char SectorNo, unsigned char *authkey,unsigned char type,unsigned char UID[4])
{
    unsigned char keyAB ;

    if((4*SectorNo + 3  >= 0x100) || (4*SectorNo + 3  < 0))
    {
        LOGI("Error SectorNo \n");
        return HANDLE_ERR_ARG_INVAL;
    }

    switch(type)
    {
        case KEYA:
            {
                keyAB = 0x60;
            }
            break;
        case KEYB:
            {
                 keyAB = 0x61;
            }
            break;
        default :
            return HANDLE_ERR_ARG_INVAL;
            break; 
    }
    return PiccAuthState_ex(4 * SectorNo + 3, keyAB, UID,authkey);
}

int MFPL1_write(unsigned char SectorNo, unsigned char BlockNo, unsigned char * key, unsigned char type,unsigned char UID[4], unsigned char SectorBuf [ 16 ])
{
    return Mifare_classic_write(SectorNo,BlockNo,key,type,UID,SectorBuf);
}


int MFPL1_read(unsigned char SectorNo, unsigned char BlockNo, unsigned char *key,unsigned char type,unsigned char UID[4],unsigned char SectorBuf[16])
{
    return Mifare_classic_read(SectorNo,BlockNo,key,type,UID,SectorBuf);
}

/*
	功能：状态切换函数，执行该操作后，1级状态的卡片转换到2级。

	参数：

		[in]authkey:-----------升级密码；
		[in]len:-----------authkey长度
	返回：

		0表示成功、其他失败。
*/
int MFPL1_switch_to_level2(unsigned char authkey[16])
{
	reset_Auth();
    return firstAuthenticate(MF_L2_SWITCH_KEY, authkey);
}
/*
	功能：状态切换函数，执行该操作后，1级状态的卡片转换到3级。

	参数：

		[in]authkey:-----------升级密码；
		[in]len:-----------authkey长度
	返回：

		0表示成功、其他失败。
*/
int MFPL1_switch_to_level3(unsigned char authkey[16])
{ 
    int ret = 0;
    ret = ResetM1FormCPU();
    if(ret != 0)
    {
        return -1;
    }
    return firstAuthenticate(MF_L3_SWITCH_KEY, authkey);
}

/*
	功能：在1级状态是和MIFARE系列卡一样操作的，此函数提供了一个专用的1级状态卡片认证功能，
		  可以在1级状态下实现严格的认证功能。（注意执行该命令卡片不要执行RATS操作）

	参数：

		[in]authkey:-----------16字节1级状态验证密钥返回：

		0表示成功、其他失败。
*/
int MFPL1_authl1key(unsigned char authkey[16])
{   
	reset_Auth();
    return firstAuthenticate(MF_L1_CARD_AUTH_KEY, authkey);
}
/*2级函数*/
/*
	功能：状态切换函数，执行该操作后，2级状态的卡片转换到3级。

	参数：

		[in]authkey:-----------升级密码；
		[in]len:-----------authkey长度
	返回：

		0表示成功、其他失败。


*/
int MFPL2_switch_to_level3(unsigned char *authkey,int len)
{
	
	return 0;
}

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
int MFPL3_read_encrypted(unsigned short BNr,unsigned char Numblock,unsigned char *readdata, unsigned char flag)
{
#if 1
    return HANDLE_ERR_NOT_SUPPORT;
#else

    int ret = 0;
    unsigned char cmd[12] ;
    unsigned char data[1] = {Numblock};

    cmd[0] = MFP_CMD_READ_MACED;
    cmd[1] = (BNr & 0xFF);
    cmd[2] = (BNr >> 8);

    {
        unsigned char encrypted_data[16]; 
        cipherData(data,1,encrypted_data);    
        memcpy(cmd + 3 ,encrypted_data,16);
    
        unsigned char WriteMac[8];
        ret = computeWriteMac(MFP_CMD_READ_MACED, BNr,encrypted_data,16, WriteMac);
        if(ret != HANDLE_OK)
        {
            return ret;
        }
        memcpy(cmd + 3 + 16,WriteMac,8);
    }
    unsigned char recv[256];
    /*send command*/
    ret = mifare_read_and_write(cmd,  3 + 16 + 8 ,recv);
    if(recv[0] != 0x90 )
    {
        LOGI("[%s %d] mifare_write fail result %02X \n",__FUNCTION__,__LINE__,recv[0]);
        return -1;
    }  
    /*校验 后面8个字节*/
    {

    }
	return 0;
#endif
}


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
int MFPL3_write_encrypted(unsigned short BNr,unsigned char Numblock, unsigned char *data, unsigned char flag)
{
#if 1
    return HANDLE_ERR_NOT_SUPPORT;
#else
    int ret = 0;
    unsigned char cmd[12] ;
    cmd[0] = MFP_CMD_WRITE_MACED;
    cmd[1] = (BNr & 0xFF);
    cmd[2] = (BNr >> 8);

    {
        unsigned char encrypted_data[16]; 
        cipherData(data,Numblock*16,encrypted_data);    
        memcpy(cmd + 3 ,encrypted_data,16);
    
        unsigned char WriteMac[8];
        ret = computeWriteMac(MFP_CMD_WRITE_MACED, BNr,encrypted_data,16, WriteMac);
        if(ret != HANDLE_OK)
        {
            return ret;
        }
        memcpy(cmd + 3 + 16,WriteMac,8);
    }
    unsigned char recv[256];
    /*send command*/
    ret = mifare_read_and_write(cmd,  3 + 16 + 8 ,recv);
    if(recv[0] != 0x90 )
    {
        LOGI("[%s %d] mifare_write fail result %02X \n",__FUNCTION__,__LINE__,recv[0]);
        return -1;
    }  
    /*校验 后面8个字节*/
    {

    }
    return 0;
#endif
}

/*
	功能：3级状态读块函数，可以连续读多块，每块16字节

	参数：

		BNr：---------------块地址，从此地址开始读卡

		Numblock：----------要读取的块数目 （一般小于6）

		readdata：----------返回读取的数据。注意长度为 Numblock*16

	返回：成功则返回0
*/
int MFPL3_read_in_plain(unsigned short BNr,unsigned char Numblock, unsigned char *readdata,int *readlen)
{
    int ret = 0;
    unsigned char cmd[12] ;
    cmd[0] = MFP_CMD_READ_PLAIN_MACED;
    cmd[1] = (BNr & 0xFF);
    cmd[2] = (BNr >> 8);
    memcpy(cmd + 3 ,&Numblock,1);
    {
        unsigned char WriteMac[8];
        unsigned char data_Numblock[1] = {Numblock};
        ret = computeReadMac(MFP_CMD_READ_PLAIN_MACED, BNr,data_Numblock,1, WriteMac);
        if(ret != HANDLE_OK)
        {
            return ret;
        }
        memcpy(cmd + 4,WriteMac,8);
    }
    unsigned char recv[256];
    if(readdata == NULL)
    {
        return HANDLE_ERR_ARG_INVAL;
    }
    /*send command*/
    ret = mifare_read_and_write(cmd, 12 ,recv);
    if(recv[0] != 0x90 )
    {
        LOGI("[%s %d] mifare_write fail result %02X \n",__FUNCTION__,__LINE__,recv[0]);
        return -1;
    }  
    /*校验 后面8个字节*/
    {

    }
    unsigned char *data = recv + 1; 
    int datalen = ret - 1 - 8;  
    memcpy(readdata,data,datalen );
    *readlen =datalen;
	return 0;
}


/*
	功能：3级状态写块函数，可以连续写多块，每块16字节

	参数：
	
		BNr： -----------块地址，从此地址开始写卡

		Numblock：-------要写的块数目 （一般小于6）

		writedata：------存放要写入卡内的数据

	返回：成功则返回0
*/
int MFPL3_write_in_plain(unsigned short BNr,unsigned char Numblock,unsigned char *writedata,unsigned char write_len)
{
    int ret = 0;
    unsigned char cmd[256] ;
        
    if(writedata == NULL)
    {
        return HANDLE_ERR_ARG_INVAL;
    }
    
    cmd[0] = MFP_CMD_WRITE_PLAIN_MACED;
    cmd[1] = (BNr & 0xFF);
    cmd[2] = (BNr >> 8);
    memcpy(cmd + 3 ,writedata,write_len);
    {
        unsigned char WriteMac[8];
        ret = computeWriteMac(MFP_CMD_WRITE_PLAIN_MACED, BNr,writedata,write_len, WriteMac);
        if(ret != HANDLE_OK)
        {
            return ret;
        }
        memcpy(cmd + 3 + write_len,WriteMac,8);
    }
    unsigned char recv[256];
    /*send command*/
    ret = mifare_read_and_write(cmd, write_len + 3 + 8,recv);
    if(recv[0] != 0x90 )
    {
        LOGI("[%s %d] mifare_write fail result %02X \n",__FUNCTION__,__LINE__,recv[0]);
        return -1;
    }  
    /*校验 后面8个字节*/
    {

    }

	return HANDLE_OK;
}

/*
	功能： 3级状态卡片认证函数，根据密钥号的不同，验证不同的密钥。

	参数：

		keyBNr：-----------密钥块号

		authkey：----------16字节密钥

	返回：成功则返回0
*/
static unsigned short key_number_from_sector(int sector, MifareKeyType type)
{  
    int offset = sector * 2;
    if (type == KT_KEY_B)
    {
        offset++;
    }
    unsigned short  pos = 0;
    pos |= 0x40 << 8;
    pos |= offset & 0xFF;

    //if(pos >= 0x4000 || pos <= 0x403F)
    if(pos < 0x4000 || pos > 0x407F)
    {
        LOGI("error pos 0x%X\n",pos );
        return 0;
    }  
    return pos;
}
/*
	功能：3级状态卡片验证扇区密钥函数。

	参数：

		type: ----------密码验证模式. =0 验证A密码 =4 验证B密码

		keyBNr：---------扇区号

		authkey：--------16字节密钥

	返回：成功则返回0
*/

int MFPL3_authl3_key_ex(unsigned short keyBNr,unsigned char *authkey)
{
    reset_Auth();
    return firstAuthenticate(keyBNr, authkey);
}

int MFPL3_authl3_key(int sector,unsigned char *authkey,MifareKeyType type)
{
    unsigned short keyBNr = key_number_from_sector(sector, type);
    return MFPL3_authl3_key_ex(keyBNr, authkey);
}



static int reset_Auth(void)
{
    int ret = 0;
    unsigned char command[] = {MFP_CMD_RESET_AUTHENTICATION};
    unsigned char recv[128];
   
    ret = mifare_read_and_write(command, sizeof(command),recv);
}

static int  firstAuthenticate(unsigned short keyBNr,unsigned char *authkey)
{
    int ret = 0;
    unsigned char command[5];
    unsigned char recv[128];
    unsigned char rnd_bin[16];
    unsigned char rnd_bout[16];

    command[0] = MFP_CMD_FIRST_AUTHENTICATE; 
    command[1] = (keyBNr & 0xFF);
    command[2] = (keyBNr >> 8);
    command[3] = 0x01;
    command[4] = 0x00;
    
    ret = mifare_read_and_write(command, sizeof(command),recv);
    if(ret < 16 || recv[0] != 0x90)
    {
        LOGI("[%s %d] mifare_write fail result %02X ret %d \n",__FUNCTION__,__LINE__,recv[0],ret);
        return HANDLE_ERR_CMD;
    }
    memcpy(rnd_bin,recv + 1,16);

    AES_KEY aes_key;
    ret=AES_set_decrypt_key(authkey,128,&aes_key);
    if(ret < 0)
    {
        LOGI("设置密钥失败!!\n");
        return HANDLE_ERR;
    }
    AES_decrypt(rnd_bin,rnd_bout,&aes_key);

    return aes_first_auth_step2(rnd_bout,authkey);
}

static int aes_first_auth_step2(    unsigned char rnd_b[16],unsigned char *authkey)
{
    int ret = 0;
    unsigned char command[33];
    unsigned char recv[128];
    unsigned char iv[AES_BLOCK_SIZE];
    unsigned char rnd_b_[16];
    unsigned char rnd_a[16];
    unsigned char data[32];
    unsigned char result[32];

    ret = RAND_bytes(rnd_a,16);
  
    memcpy(data,rnd_a,16);
    {
        unsigned char tmp;
        int i = 0;    
        memcpy(rnd_b_,rnd_b,16);
        for(  tmp = rnd_b_[0]; i < 16 - 1 ; i++ )
        {
            rnd_b_[i] = rnd_b_[i + 1];
        }
        rnd_b_[15] = tmp;
    }
    memcpy(data + 16,rnd_b_,16);

    for(int i=0; i < AES_BLOCK_SIZE; ++i)//iv一般设置为全0,可以设置其他，但是加密解密要一样就行
    {
        iv[i]=0;
    }
    {
        AES_KEY aes_key;
        ret=AES_set_encrypt_key(authkey,128,&aes_key);
        if(ret < 0)
        {
            LOGI("设置密钥失败!!\n");
            return HANDLE_ERR;
        }
        AES_cbc_encrypt(data,result,32,&aes_key,iv,AES_ENCRYPT);
    }

    command[0] = MFP_CMD_AUTHENTICATE_PART_2;
    memcpy(command + 1 , result, sizeof(result));
    ret = mifare_read_and_write(command, sizeof(command),recv);
    if(ret < 32 || recv[0] != 0x90)
    {
        LOGI("[%s %d] mifare_write fail result %02X \n",__FUNCTION__,__LINE__,recv[0]);
        return HANDLE_ERR_CMD;
    }
    // make sure the result from the result is as expected.
    unsigned char encrypted_data[32];
    memcpy(encrypted_data,recv + 1,32);
    
    return aes_first_auth_final(rnd_a,rnd_b,authkey,encrypted_data);
}


/**/
static int aes_first_auth_final(unsigned char rnd_a[16],unsigned char rnd_b[16],unsigned char *authkey,unsigned char encrypted_data[32])
{
    // If we received garbage, the AES code may throw. This means auth failure.
    unsigned char data[32];
    AES_KEY aes_key;
    int ret = 0;
    unsigned char iv[AES_BLOCK_SIZE];
    ret=AES_set_decrypt_key(authkey,128,&aes_key);
    if(ret < 0)
    {
        LOGI("设置密钥失败!!\n");
        return HANDLE_ERR;
    }

    for(int i=0; i < AES_BLOCK_SIZE; ++i)//iv一般设置为全0,可以设置其他，但是加密解密要一样就行
    {
        iv[i]=0;
    }
    AES_cbc_encrypt(encrypted_data,data,32,&aes_key,iv,AES_DECRYPT);

    unsigned char rnd_areader[16];
    memcpy(rnd_areader,data + 4, 16);

    {
        unsigned char tmp;
        int i = 16;      
        for(  tmp = rnd_areader[16 - 1]; i > 0 ; i-- )
        {
            rnd_areader[i] = rnd_areader[i - 1];
        }
        rnd_areader[0] = tmp;
    }
    if(memcmp(rnd_areader,rnd_a,16) != 0)    /*自发行自定义数据*/
    {
        LOGI("RNDA doesn't match. AES authentication failed.\n");
        return HANDLE_ERR;
    }
    LOGI("AES Auth Success.\n");
    //Enc key 
    {
        deriveKEnc(rnd_a,rnd_b,authkey,trans_id,SessionKeyEnc);
        //menu_print(SessionKeyEnc,16);
        //set_SessionKeyEnc(SessionKeyEnc);
    }
    //Mac key 
    {
        //unsigned char SessionKeyMac[16];
        deriveKMac(rnd_a,rnd_b,authkey,trans_id,SessionKeyMac);
        //menu_print(SessionKeyMac,16);
        //set_SessionKeyMac(SessionKeyMac);
    }
    
    //4first byte is Transaction Identifier
    {
        //unsigned char trans_id[4];
        memcpy(trans_id,data,4);
    }
    {
        memcpy(authkey_bak,authkey,16);
        read_count_ = 0;
        write_count_ = 0;
    }
    return HANDLE_OK;
}


static int deriveKEnc(unsigned char rnd_a[16],unsigned char rnd_b[16],unsigned char authkey[16],unsigned char trans_id[4],unsigned char SessionKeyEnc[16])
{
    unsigned char tmp[16];

    unsigned char f[5];
    unsigned char g[5];
    unsigned char h[5];
    unsigned char i[5];
    int c = 0;
    //init data buffers
    while (c < 5)
    {
        h[c] = rnd_a[c + 4];
        i[c] = rnd_b[c + 4];
        f[c] = rnd_a[c + 11];
        g[c] = rnd_b[c + 11];
        c++;
    }
    //set the session key base for Kenc
    memcpy(tmp,f,5);
    memcpy(tmp+5,g,5);
    c = 0;
    while (c < 5)
    {
        tmp[c + 10] = (h[c] ^ i[c]);
        c++;
    }
    tmp[15] = 0x11;

    {
        AES_KEY aes_key;
        int ret = 0;
        unsigned char iv[AES_BLOCK_SIZE];
        memset(iv,0 , sizeof(iv));
        ret=AES_set_encrypt_key(authkey,128,&aes_key);
        if(ret < 0)
        {
            LOGI("设置密钥失败!!\n");
            return HANDLE_ERR;
        }
        AES_cbc_encrypt(tmp,SessionKeyEnc,16,&aes_key,iv,AES_ENCRYPT);
    }
    return HANDLE_OK;
}

static int deriveKMac(unsigned char rnd_a[16],unsigned char rnd_b[16],unsigned char authkey[16],unsigned char trans_id[4],unsigned char SessionKeyMac[16])
{
    unsigned char tmp[16];

    unsigned char f[5];
    unsigned char g[5];
    unsigned char h[5];
    unsigned char i[5];
    int c = 0;
    //init data buffers
    while (c < 5)
    {
        h[c] = rnd_a[c];
        i[c] = rnd_b[c];
        f[c] = rnd_a[c + 7];
        g[c] = rnd_b[c + 7];
        c++;
    }


    //set the session key base for Kmac
    memcpy(tmp,f,5);
    memcpy(tmp+5,g,5);

    c = 0;
    while (c < 5)
    {
        tmp[c + 10] = (h[c] ^ i[c]);
        c++;
    }

    tmp[15] = 0x22;
    {
        AES_KEY aes_key;
        int ret = 0;
        unsigned char iv[AES_BLOCK_SIZE] = {0};
        memset(iv,0 , sizeof(iv));
        ret=AES_set_encrypt_key(authkey,128,&aes_key);
        if(ret < 0)
        {
            LOGI("设置密钥失败!!\n");
            return HANDLE_ERR;
        }
        AES_cbc_encrypt(tmp,SessionKeyMac,16,&aes_key,iv,AES_ENCRYPT);
    }

    return HANDLE_OK;
}
static int computeMac(unsigned char command_code,unsigned short count, unsigned char trans_id[4],unsigned short block,unsigned char *data,unsigned char data_len,unsigned char KeyMac[16],unsigned char write_mac[8])
{
    unsigned char to_hash_in[256];
    unsigned char to_hash_out[16];
    int to_hash_out_len = 0;
    if(write_mac == NULL)
    {
        return HANDLE_ERR_ARG_INVAL;
    }
    
    to_hash_in[0] = command_code;
    to_hash_in[1] = (count & 0xFF);
    to_hash_in[2] = (count >> 8 & 0xFF);
    memcpy(to_hash_in + 3,trans_id,4);    
    to_hash_in[7] = block & 0xFF;
    to_hash_in[8] = block >> 8 & 0xFF;
    memcpy(to_hash_in + 9,data,data_len);

    {
        CMAC_CTX *ctx = CMAC_CTX_new();
        CMAC_Init(ctx, KeyMac, 16, EVP_aes_128_cbc(), NULL);
        CMAC_Update(ctx, to_hash_in,9 + data_len);
        CMAC_Final(ctx,to_hash_out, &to_hash_out_len);
        CMAC_CTX_free(ctx);
        if(to_hash_out_len != 16)
        {
            return HANDLE_ERR;
        }
    }

    {
        int i = 0;
        for (i = 0; i < 8; i++)
        {
            write_mac[i] = to_hash_out[i * 2 + 1];
        }
    }
    return HANDLE_OK;
}

static int computeReadMac(unsigned char command_code,unsigned short block, unsigned char *data,unsigned char data_len,unsigned char write_mac[8])
{
    return computeMac(command_code,read_count_++,trans_id,block,data,data_len,SessionKeyMac,write_mac);
}

static int computeWriteMac(unsigned char command_code,unsigned short block, unsigned char *data,unsigned char data_len,unsigned char write_mac[8])
{
    return computeMac(command_code,write_count_++,trans_id,block,data,data_len,SessionKeyMac,write_mac);
}

static int cipherData(const unsigned char *data,unsigned char data_len,unsigned char out_data[16])
{
    unsigned char iv[AES_BLOCK_SIZE];
    int i = 1;
    AES_KEY aes_key;
    int ret = 0;
    memcpy(iv,trans_id,4);
    for (i = 1; i < 4; i++)
    {
        iv[i * 4 + 0] = (read_count_ & 0xFF);
        iv[i * 4 + 1] = (read_count_ >> 8 & 0xFF);
        iv[i * 4 + 2] = (write_count_ & 0xFF);
        iv[i * 4 + 3] = (write_count_ >> 8 & 0xFF);
    }

    ret = AES_set_encrypt_key(authkey_bak,128,&aes_key);
    if(ret < 0)
    {
        LOGI("设置密钥失败!!\n");
        return HANDLE_ERR;
    }
    AES_cbc_encrypt(SessionKeyEnc,out_data,16,&aes_key,iv,AES_ENCRYPT);

    return HANDLE_OK;
}



