#include "cardlan_devctrl.h"
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

int InitDev(void)
{
    int ret;
    ret = mifare_open();
    if(ret != 0)
    {
        LOGI("[ %s %d ]mifare_open error ret : %d!!\n",__FUNCTION__,__LINE__,ret);
        return -1;
    }
	
    /* Psam card operation */
    ret = init_mcu_spi();
    if(ret != 0)
    {
        LOGI("[ %s %d ]init_mcu_spierror ret : %d!!\n",__FUNCTION__,__LINE__,ret);
        return -2;  
    }
/*
    ret = InitPsam(0, 9600);
    if(ret != 0)
    {
        LOGI("[ %s %d ]InitPsam error ret : %d!!\n",__FUNCTION__,__LINE__,ret);
        return -3;  
    }
    
    ret = GetPsamID();
    if(ret != 0)
    {
        LOGI("[ %s %d ]GetPsamID error ret : %d!!\n",__FUNCTION__,__LINE__,ret);
        return -4;  
    }
*/
#if 0

    TERMAPP_QPBOCTermInit(0);

    {
        int ret = 0;
        TERMAPP_T aid_list[3];
        unsigned char JTB_AID[8] = {0xA0,0x00,0x00,0x06,0x32,0x01,0x01,0x05};
        unsigned char TermAID0[] = {0xA0, 0x00, 0x00, 0x03, 0x33, 0x01, 0x01, 0x01}; // 借记卡AID
        unsigned char TermAID1[] = {0xA0, 0x00, 0x00, 0x03, 0x33, 0x01, 0x01, 0x06}; // 非接电子现金AID

   
        aid_list[0].AIDLen = sizeof(JTB_AID);
        memcpy(aid_list[0].AID,JTB_AID,sizeof(JTB_AID));
        aid_list[0].ASI = 0;

        
        aid_list[1].AIDLen = sizeof(TermAID0);
        memcpy(aid_list[1].AID,TermAID0,sizeof(TermAID0));
        aid_list[1].ASI = 0;


        aid_list[2].AIDLen = sizeof(TermAID1);
        memcpy(aid_list[2].AID,TermAID1,sizeof(TermAID1));
        aid_list[2].ASI = 0;
        
        ret = set_terminal_support_AID(aid_list,sizeof(aid_list) / sizeof(TERMAPP_T));
        if(ret != 0)
        {
            /*not found*/
            return 1;
        }
    }
#endif	
    LOGI("[ %s %d ] success version : %s\n",__FUNCTION__,__LINE__,BUILD_VERSION);

    
    return 0;
}


