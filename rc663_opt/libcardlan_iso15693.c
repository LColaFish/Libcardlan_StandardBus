
#include "libcardlan_iso15693.h"

#if defined(JNI_API_DEF)
#undef JNI_API_DEF
#endif
#define JNI_API_DEF(f) Java_com_cardlan_twoshowinonescreen_CardlanISO15693_##f

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


JNIEXPORT jint JNICALL JNI_API_DEF(ISO15693OptCardReset)(JNIEnv *env, jobject obj, jbyteArray array)
{
    (void)obj;                                                                                                                               
    
   unsigned char    data[128]            =    {0};
   int              result               =    0;
   jsize            len                  =    0;

   LOGD("%s\n", __FUNCTION__);                                                                                                                                    

   result = ISO15693_Opt_CardReset(data, &len);

   if(result != 0)
   {
        return -1;
   }
   (*env)->SetByteArrayRegion(env, array, 0, len, (jbyte *)data);
   return result;
}
JNIEXPORT jint JNICALL JNI_API_DEF(ISO15693OptWriteAFI)(JNIEnv *env, jobject obj, jbyte data)
{
    LOGD("%s\n", __FUNCTION__);    
    return ISO15693_Opt_WriteAFI((unsigned char)data);
}

JNIEXPORT jint JNICALL JNI_API_DEF(ISO15693OptLockAFI)(JNIEnv *env, jobject obj)
{
    LOGD("%s\n", __FUNCTION__);    
    return ISO15693_Opt_LockAFI();
}
JNIEXPORT jint JNICALL JNI_API_DEF(ISO15693OptWriteDSFID)(JNIEnv *env, jobject obj,jbyte data)
{
    LOGD("%s\n", __FUNCTION__);    
    return ISO15693_Opt_WriteDSFID((unsigned char)data);
}
JNIEXPORT jint JNICALL JNI_API_DEF(ISO15693OptLockDSFID)(JNIEnv *env, jobject obj)
{
    LOGD("%s\n", __FUNCTION__);    
    return ISO15693_Opt_LockDSFID();
}
JNIEXPORT jint JNICALL JNI_API_DEF(ISO15693OptWriteData)(JNIEnv *env,jobject obj,jbyte block, jbyteArray DataArray)
{
    char data_buff[256];
    int result;
    int i;

    LOGD("%s\n",__func__);


    int ret = ISO15693_Opt_WriteBlockNum(block);
    if(ret != 0)
    {
        LOGI("ISO15693_Opt_WriteBlockNum fail len : %d \n",ret);
        return ret;
    }

    /* Cmd Buff */
    int len = (*env)->GetArrayLength(env, DataArray);
    jbyte* bytearr = (*env)->GetByteArrayElements(env, DataArray,NULL);

    for(i = 0; i < len; i++)
    {
        data_buff[i] = bytearr[i];
    }

    (*env)->ReleaseByteArrayElements(env, DataArray, bytearr, 0);
    
    return ISO15693_Opt_WriteData(data_buff,len);

   
}
JNIEXPORT jint JNICALL JNI_API_DEF(ISO15693OptReadSignalBlock)(JNIEnv *env, jobject obj, jbyte block,jbyteArray Recv)
{
    int result;
    unsigned char receive_buf[128];

    result = ISO15693_Opt_read_signal_block(block,receive_buf);
    if(result <= 0)
    {
        LOGI("ISO15693_Opt_read_signal_block fail result : %d\n",result);
        return -1;
    }
   (*env)->SetByteArrayRegion(env, Recv, 0, 4, (jbyte *)receive_buf);

   return result;
       
    
}

JNIEXPORT jint JNICALL JNI_API_DEF(ISO15693OptReadMultiple)(JNIEnv *env, jobject obj,jbyte num,jbyte block, jbyteArray Recv)
{
    unsigned char receive_buf[128];

    if(num <= 0)
    {
        LOGI("ISO15693_Opt_Read_Block_Nums arg fail \n");
        return -1;
    }
    int ret = ISO15693_Opt_Read_Block_Nums(num - 1);
    if(ret != 0)
    {
        LOGI("ISO15693_Opt_Read_Block_Nums fail ret : %d\n",ret);
        return ret;
    }


    int len = ISO15693_Opt_Read_Multiple(block,receive_buf);
    if(len <= 0)
    {
        printf("[%s %d]  ISO15693_Opt_read_signal_block len:%d!!\n",__FUNCTION__,__LINE__,len);
        return -1;
    }

    (*env)->SetByteArrayRegion(env, Recv, 0, len, (jbyte *)receive_buf);
    return len;
}


