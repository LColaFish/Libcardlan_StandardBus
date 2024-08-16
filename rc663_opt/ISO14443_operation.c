#include "ISO14443_operation.h"
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

int ISO14443_CardReset(void)
{
    return  mifare_ioctl(RC531_M1_CSN, 0);
}
int ISO14443_Halt(void)
{
    return mifare_ioctl(RC531_HALT, 0);
}

int ISO14443_read_and_write(const unsigned char *data, size_t data_len,unsigned char *result_buff)
{
    static int  receive_len[1] = {0};
    int result;

    if(data == NULL || result_buff == NULL)
    {
        LOGI("[%s %d] cmd:%08X buff:%08X \n",__FUNCTION__,__LINE__,data,result_buff);
        return -1;
    }
#if defined(TARGET_DEBUG)   
    LOGI("\n------------%s start-------------\n",__FUNCTION__);
    LOGI("  ---mifare_write  :\n");
    menu_print(data, data_len);
#endif 

    result = mifare_write(data,data_len);
    if(result != MI_OK)
    {
       LOGI("[%s %d] write fail result %02X \n",__FUNCTION__,__LINE__,result);
       return -2;
    }
    
    memset(receive_len,0,sizeof(receive_len));
    mifare_ioctl( FIFO_RCV_LEN, receive_len);
    result = mifare_read(result_buff, receive_len[0]);
    if(result <= 0)
    {
       LOGI("[%s %d]  read fail result %02X \n",__FUNCTION__,__LINE__,result);
       return -3;
    }
#if defined(TARGET_DEBUG)  
    LOGI("  ---mifare_read  :\n");
    menu_print(result_buff, result);    
    LOGI("--------------end----------------------\n\n");
#endif
    return result;
}

