#include "ISO15693_operation.h"
#include "cpu_card_operation.h"
#include "libcardlan_CardInfo.h"
#include "typea.h"
#include "mcu_opt/psam_opt.h"

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

int ISO15693_Opt_Read_Block_Nums(unsigned char block)
{
	int result=-1;
	LOGI("ISO15693_Opt_Read_Block_Nums() is called.\n");
	return mifare_ioctl(TYPEA_15693_READNUMS,block);

}
int ISO15693_Opt_WriteData(uint8_t  *Send,uint8_t Slen)
{
	LOGI("ISO15693_Opt_WriteData() is called.\n");
    mifare_ioctl( WRITE_TYPE, W_15693);
	return  mifare_write(Send, Slen);
}

void ISO15693_Opt_halt(void)
{
    LOGI("ISO15693_Opt_halt() is called.\n");
	int status = mifare_ioctl(TYPEA_15693_STAYQUIET,0);
	LOGI("Halt: status = %d\n", status);
}

int ISO15693_Opt_WriteAFI(unsigned char data)
{
	int result=-1;
	LOGI("ISO15693_Opt_WriteAFI() is called.\n");
	return mifare_ioctl(TYPEA_15693_WRITEAFI,data);
}

int ISO15693_Opt_LockAFI()
{
	int result=-1;
	LOGI("ISO15693_Opt_LockAFI() is called.\n");
	return  mifare_ioctl(TYPEA_15693_LOCKAFI,0);

}


int ISO15693_Opt_WriteDSFID(unsigned char data)
{
	int result=-1;
	LOGI("ISO15693_Opt_WriteDSFID() is called.\n");	
	return mifare_ioctl(TYPEA_15693_WRITEDSFID,data);;
}

int ISO15693_Opt_LockDSFID()
{
	int result=-1;
	LOGI("ISO15693_Opt_LockDSFID() is called.\n");
	return  mifare_ioctl(TYPEA_15693_LOCKDSFID,0);
}


int ISO15693_Opt_WriteBlockNum(unsigned char block)
{
	int result=-1;
	LOGI("ISO15693_Opt_WriteBlockNum() is called.\n");
	return mifare_ioctl(TYPEA_15693_WRITENUMS,block);

}

int ISO15693_Opt_Read_Multiple(unsigned char block,unsigned char *receive_buf)
{
	int result=-1;
    int receive_len[1] = {0};

	LOGI("ISO15693_Opt_Read_Multiple() is called.\n");
 
	result = mifare_ioctl( TYPEA_15693_READMUL,block);
	if(result != MI_OK)
    {
        return -1;
    }

	mifare_ioctl( FIFO_RCV_LEN, receive_len);
	mifare_read( receive_buf, receive_len[0]);			
    if(receive_len[0] <= 0)
    { 
        return -2;
	}

	return receive_len[0];

}


int ISO15693_Opt_read_signal_block(unsigned char block,unsigned char receive_buf[128])
{
	int result=-1;
    int receive_len[1] = {0};
	result =  mifare_ioctl(TYPEA_15693_READSIN,block);
	if(result != MI_OK)
    {
        return -1;
    }

    mifare_ioctl(FIFO_RCV_LEN, receive_len);
	mifare_read(receive_buf, receive_len[0]);			
    if(receive_len[0] <= 0)
    { 
        return -2;
	}
	return receive_len[0];
}

int ISO15693_Opt_CardReset(char *data,unsigned char *plen)
{
	int result=-1;
    unsigned char receive_buf[256+2]= {0};
    int           receive_len[1] = {0};
	LOGI("ISO15693_Opt_CardReset() is called.\n");
    int i ;
	for(i = 0; i < 5; i++)
	{
		result = mifare_ioctl(TYPEA_15693_REST,0);
        if(result == MI_OK)
		{
			break;
		}
	}
	if(i == 5)
    {
        return -1;
    }   

	mifare_ioctl(FIFO_RCV_LEN, receive_len);
	mifare_read(receive_buf, receive_len[0]);
    if(receive_len[0] <= 0)
    {
        return -2;
	}

    memcpy(data,receive_buf,receive_len[0]);
    *plen = receive_len[0];
 
	return 0;
}



int ISO15693_read_and_write(const unsigned char *data, size_t data_len,unsigned char *buff)
{
    return -1;//NOT_SUPPORT;
}


