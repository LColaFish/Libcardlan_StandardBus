
#include "libcardlan_mifareplus.h"

#if defined(JNI_API_DEF)
#undef JNI_API_DEF
#endif
#define JNI_API_DEF(f) Java_com_cardlan_twoshowinonescreen_CardLanMifarePlus_##f

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

JNIEXPORT jbyteArray JNICALL JNI_API_DEF(CardReset2)(JNIEnv *env, jobject obj)                                                                
{
    (void)obj;                                                                                                                               

    unsigned char    data[128]            =    {0};
    int              result               =    0;
    jsize            len                  =    0;
    int ret = 0;
    ret = CardReset(data, &len,0);
    if(ret < 0)
    {
        jbyteArray jarrRecv = (*env)->NewByteArray(env,0);//创建一个byte数组
        return jarrRecv;
    }
    jbyteArray jarrRecv = (*env)->NewByteArray(env,len);//创建一个byte数组
    (*env)->SetByteArrayRegion(env, jarrRecv, 0, len, (jbyte *)data);
	return jarrRecv;    
}

JNIEXPORT jint JNICALL JNI_API_DEF(MFPDetectSecurityLevel)(JNIEnv *env, jobject obj)
{
    LOGD("%s\n", __FUNCTION__);    
    return MFP_detect_Security_Level();
}

JNIEXPORT jint JNICALL JNI_API_DEF(MFPL0WritePerso)(JNIEnv *env, jobject obj,jbyte hex_addr_1,jbyte hex_addr_2,jbyteArray dataperso)
{
    char data_buff[256];
    int result;
    int i;

    LOGD("%s\n", __FUNCTION__);   

    /* Cmd Buff */
    int len = (*env)->GetArrayLength(env, dataperso);
    jbyte* bytearr = (*env)->GetByteArrayElements(env, dataperso,NULL);

    for(i = 0; i < len; i++)
    {
        data_buff[i] = bytearr[i];
    }

    (*env)->ReleaseByteArrayElements(env, dataperso, bytearr, 0);

    return MFPL0_write_perso(hex_addr_1,hex_addr_2,data_buff);
}

JNIEXPORT jint JNICALL JNI_API_DEF(MFPL0CommitPerso)(JNIEnv *env, jobject obj)
{
    LOGD("%s\n", __FUNCTION__);    
	return MFPL0_commit_perso(); 
}

JNIEXPORT jint JNICALL JNI_API_DEF(MFPL1Write)(JNIEnv *env, jobject obj,jbyte SectorNo,jbyte BlockNo,jbyteArray key,jbyte type,jbyteArray UID,jbyteArray SectorBuf)
{
    LOGD("%s\n", __FUNCTION__);  
    int uid_index = 0;
    jbyte* key_bytearr = (*env)->GetByteArrayElements(env,key,NULL);
    int key_len = (*env)->GetArrayLength(env,key);
    jbyte* uid_bytearr = (*env)->GetByteArrayElements(env,UID,NULL);
    int uid_len = (*env)->GetArrayLength(env,key);
    jbyte* data_buff = (*env)->GetByteArrayElements(env,SectorBuf,NULL);
    int data_len = (*env)->GetArrayLength(env,SectorBuf);

    if(key_len != 6 || data_len != 16)
    {
        return -1;
    }
    if(uid_len == 7)
    {
        uid_index = 3;
    }
    return MFPL1_write(SectorNo, BlockNo, key_bytearr, type, uid_bytearr + uid_index,data_buff);
}


JNIEXPORT jbyteArray JNICALL JNI_API_DEF(MFPL1Read)(JNIEnv *env, jobject obj,jbyte SectorNo,jbyte BlockNo,jbyteArray key,jbyte type,jbyteArray UID)
{
    char data_buff[256];
    int ret;
    int uid_index = 0;

    LOGD("%s\n", __FUNCTION__);   
    jbyte* key_bytearr = (*env)->GetByteArrayElements(env,key,NULL);
    int key_len = (*env)->GetArrayLength(env,key);
    jbyte* uid_bytearr = (*env)->GetByteArrayElements(env,UID,NULL);
    int uid_len = (*env)->GetArrayLength(env,key);
    
    if(key_len != 6)
    {
        jbyteArray jarrRecv = (*env)->NewByteArray(env,0);
        return jarrRecv;
    }

    if(uid_len == 7)
    {
        uid_index = 3;
    }

    ret = MFPL1_read(SectorNo,BlockNo,key_bytearr,type,uid_bytearr + uid_index, data_buff);
    if(ret != 0)
    {
        jbyteArray jarrRecv = (*env)->NewByteArray(env,0);
        return jarrRecv;
    } 
    jbyteArray jarrRecv = (*env)->NewByteArray(env,16);//创建一个byte数组
    (*env)->SetByteArrayRegion(env, jarrRecv, 0, 16, (jbyte *)data_buff);
  
    return jarrRecv;
}

JNIEXPORT jint JNICALL JNI_API_DEF(MFPL1AuthLevel1key)(JNIEnv *env, jobject obj,jbyteArray authkey)
{
    char data_buff[16];
    int i = 0;
    LOGD("%s\n", __FUNCTION__);  
    /* Cmd Buff */
    jbyte* bytearr = (*env)->GetByteArrayElements(env, authkey,NULL);
    int key_len = (*env)->GetArrayLength(env,authkey);

    for(i = 0; i < 16; i++)
    {
        data_buff[i] = bytearr[i];
    }
    (*env)->ReleaseByteArrayElements(env, authkey, bytearr, 0);

	return MFPL1_authl1key(data_buff); 
}

JNIEXPORT jint JNICALL JNI_API_DEF(MFPL1SwitchTolevel3)(JNIEnv *env, jobject obj,jbyteArray authkey)
{
    char data_buff[16];
    int i = 0;
    LOGD("%s\n", __FUNCTION__);  
    /* Cmd Buff */
    jbyte* bytearr = (*env)->GetByteArrayElements(env, authkey,NULL);

    for(i = 0; i < 16; i++)
    {
        data_buff[i] = bytearr[i];
    }
    (*env)->ReleaseByteArrayElements(env, authkey, bytearr, 0);

	return MFPL1_switch_to_level3(data_buff); 
}

JNIEXPORT jbyteArray JNICALL JNI_API_DEF(MFPL3Read)(JNIEnv *env, jobject obj,jbyte hex_addr_1,jbyte hex_addr_2,jbyte count)
{
    jbyteArray jarrRecv; 
    LOGD("%s\n", __FUNCTION__);
    unsigned short BNr = hex_addr_1;
    unsigned char Numblock = count;
    int ret = 0;
    unsigned char readdata[256];
    int readlen;
    
    BNr = (BNr << 8) + (unsigned char)hex_addr_2;
    LOGI("BNr 0x%X 0x%X 0x%X\n",BNr,(unsigned char)hex_addr_1,(unsigned char)hex_addr_2);

    ret = MFPL3_read_in_plain(BNr,Numblock,readdata,&readlen);
    if(ret != 0)
    {
        jarrRecv = (*env)->NewByteArray(env,0);//创建一个byte数组
        return jarrRecv;
    }
    jarrRecv = (*env)->NewByteArray(env,readlen);//创建一个byte数组
    (*env)->SetByteArrayRegion(env, jarrRecv, 0, readlen, (jbyte *)readdata);
	return jarrRecv; 
}


JNIEXPORT jint JNICALL JNI_API_DEF(MFPL3Write)(JNIEnv *env, jobject obj,jbyte hex_addr_1,jbyte hex_addr_2,jbyteArray writedata)
{
    char data_buff[16];

    LOGD("%s\n", __FUNCTION__);    
    jbyte* bytearr = (*env)->GetByteArrayElements(env, writedata,NULL);
    int len = (*env)->GetArrayLength(env,writedata);
    int ret = 0;
    int i = 0;
    unsigned short BNr = hex_addr_1;
    if(len != 16)
    {
        LOGI("write data len error %d \n",len);
        return -1;
    }

    BNr = (BNr << 8) + (unsigned char)hex_addr_2;


    for(i = 0; i < 16; i++)
    {
        data_buff[i] = bytearr[i];
    }
    (*env)->ReleaseByteArrayElements(env, writedata, bytearr, 0);

	return MFPL3_write_in_plain(BNr,1,data_buff,16); 
}
JNIEXPORT jint JNICALL JNI_API_DEF(MFPL3Authl3Key)(JNIEnv *env, jobject obj,jbyte hex_addr_1,jbyte hex_addr_2,jbyteArray authkey)
{
    int ret = 0;
    int i = 0;
    char data_buff[16];
    unsigned short BNr = hex_addr_1;
    jbyte* bytearr = (*env)->GetByteArrayElements(env, authkey,NULL);
    int len = (*env)->GetArrayLength(env,authkey);
    if(len != 16)
    {
        return -1;
    }

    BNr = (BNr << 8) + (unsigned char)hex_addr_2;

    for(i = 0; i < 16; i++)
    {
        data_buff[i] = bytearr[i];
    }
    (*env)->ReleaseByteArrayElements(env, authkey, bytearr, 0);

    return MFPL3_authl3_key_ex(BNr, data_buff);
}

