#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <termios.h>

#include "encrypt_lib/des.h"
#include "encrypt_lib/mac.h"
#include "encrypt_lib/stades.h"

#include "libcardlan_StandardBus_util.h"
#include "cardlan_devctrl.h"
#include "libcardlan_CardInfo.h"


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



#define  LOGD    printf


static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 1000000;


static uint16_t delay;
static int fd_spi;
static int fd_serial;


/*------------------------------------algorithm----------------------------------------*/
/* For M1 Card */
JNIEXPORT jint JNICALL JNI_API_DEF(DesCard)(JNIEnv *env, jobject obj, jbyteArray KeyArray, jbyteArray DataArray, jbyteArray OutDataArray)
{
    (void)obj;            
    
    unsigned char key[8] = {0};    
    unsigned char data[8] = {0};    
    unsigned char out_data[8] = {0};    
    int              i;
    
    LOGI("%s\n",__func__);

    
    // Key Array
    int len = (*env)->GetArrayLength(env,KeyArray);
    LOGI("len=%d",len);
    if(len > 8)    len = 8;

    jbyte* bytearr = (*env)->GetByteArrayElements(env,KeyArray,NULL);

    for(i=0;i<len;i++){
        LOGI("keyarr[%d]=%d",i,bytearr[i]);//int 数组原来每一个元素的初始值
        key[i] = bytearr[i];
    }
    
    (*env)->ReleaseByteArrayElements(env,KeyArray, bytearr, 0);

    //Data
    len = (*env)->GetArrayLength(env,DataArray);
    LOGI("len=%d",len);
    if(len > 8)    len = 8;

    bytearr = (*env)->GetByteArrayElements(env,DataArray,NULL);

    for(i=0;i<len;i++){
        LOGI("keyarr[%d]=%d",i,bytearr[i]);//int 数组原来每一个元素的初始值
        data[i] = bytearr[i];
    }
    
    (*env)->ReleaseByteArrayElements(env,DataArray, bytearr, 0);

    //Des
    DES_CARD(key,data,out_data);
    
    (*env)->SetByteArrayRegion(env, OutDataArray, 0, 6, (jbyte *)out_data);
    
    return 0;
}

/* For CPU Card */
/************************************************************************************
  函 数 名 称:    RunDes
  功 能 描 述：    执行DES算法对文本加解密
  参 数 说 明：    bType    :类型：加密ENCRYPT，解密DECRYPT
                bMode    :模式：ECB,CBC
                InBuff    :待加密串指针
                OutBuff    :待输出串指针
                datalen    :待加密串的长度，同时Out的缓冲区大小应大于或者等于datalen
                KeyIn    :密钥(可为8位,16位,24位)支持3密钥
                keylen    :密钥长度，多出24位部分将被自动裁减

  返回值 说明：    char    :是否加密成功
*************************************************************************************/
JNIEXPORT jchar JNICALL JNI_API_DEF(RunDes)(JNIEnv *env, jobject obj, jchar bType, jchar bMode, jbyteArray InBuff, jbyteArray OutBuff, jint datalen, jbyteArray KeyIn, jchar keylen)
{
    (void)obj;            
    int            i            = 0;
    char           In[8]        = {0};
    char           Out[8]       = {0};
    char           Key[24]      = {0};
    char           ret          = 0;

    // In Array
    int len = (*env)->GetArrayLength(env, InBuff);
    LOGI("len=%d",len);

    jbyte* bytearr = (*env)->GetByteArrayElements(env, InBuff,NULL);

    for(i=0;i<len;i++){
        LOGI("keyarr[%d]=%02X",i,bytearr[i]);//int 数组原来每一个元素的初始值
        In[i] = bytearr[i];
    }
    
    (*env)->ReleaseByteArrayElements(env, InBuff, bytearr, 0);

    // KeyIn Array
    len = (*env)->GetArrayLength(env, KeyIn);
    LOGI("len=%d",len);

    bytearr = (*env)->GetByteArrayElements(env, KeyIn,NULL);

    for(i=0;i<len;i++){
        LOGI("keyarr[%d]=%02X",i,bytearr[i]);//int 数组原来每一个元素的初始值
        Key[i] = bytearr[i];
    }
 
    (*env)->ReleaseByteArrayElements(env, KeyIn, bytearr, 0);

    ret = RunDes(bType, bMode, In, Out, datalen, Key, keylen);
    
    (*env)->SetByteArrayRegion(env, OutBuff, 0, 8, (jbyte *)Out);

    return ret;
}

/**************************************************************
  函 数 名 称:    MacAnyLength
  功 能 描 述：    MacAnyLength 算法,计算MAC验证码
  参 数 说 明：    InBuff        :输入串
                datalen        :输入串长度
                Out            :输出指针
                Key            :密钥(可为8位,16位,24位)支持3密钥
                keylen        :密钥长度，多出24位部分将被自动裁减

  返回值 说明：    char
**************************************************************/
JNIEXPORT jchar JNICALL JNI_API_DEF(MacAnyLength)(JNIEnv *env, jobject obj, jbyteArray InitIn, jbyteArray InBuff, jbyteArray OutBuff, jint datalen, jbyteArray KeyIn, jchar keylen)
{
    (void)obj;            
    char Init[8]    = {0};
    char In[256]    = {0};
    char Out[8]     = {0};
    char Key[24]    = {0};
    int    i        = 0;
    char ret        = 0;

    // Init Array
    int len = (*env)->GetArrayLength(env, InitIn);
    LOGI("len=%d",len);

    jbyte* bytearr = (*env)->GetByteArrayElements(env, InitIn,NULL);

    for(i=0;i<len;i++){
        LOGI("keyarr[%d]=%d",i,bytearr[i]);//int 数组原来每一个元素的初始值
        Init[i] = bytearr[i];
    }
    
    (*env)->ReleaseByteArrayElements(env, InitIn, bytearr, 0);

    
    // In Array
    len = (*env)->GetArrayLength(env, InBuff);
    LOGI("len=%d",len);

    bytearr = (*env)->GetByteArrayElements(env, InBuff,NULL);

    for(i=0;i<len;i++){
        LOGI("keyarr[%d]=%d",i,bytearr[i]);//int 数组原来每一个元素的初始值
        In[i] = bytearr[i];
    }
    
    (*env)->ReleaseByteArrayElements(env, InBuff, bytearr, 0);

    // KeyIn Array
    len = (*env)->GetArrayLength(env, KeyIn);
    LOGI("len=%d",len);

    bytearr = (*env)->GetByteArrayElements(env, KeyIn,NULL);

    for(i=0;i<len;i++){
        LOGI("keyarr[%d]=%d",i,bytearr[i]);//int 数组原来每一个元素的初始值
        Key[i] = bytearr[i];
    }

    (*env)->ReleaseByteArrayElements(env, KeyIn, bytearr, 0);

    ret = MacAnyLength(Init, In, Out, datalen, Key, keylen);

    (*env)->SetByteArrayRegion(env, OutBuff, 0, 8, (jbyte *)Out);

    return ret;
}


/*-------------------------------------Init Deivce--------------------------------------*/
JNIEXPORT jint JNICALL JNI_API_DEF(InitDev)(JNIEnv *env, jobject obj)                                                                
{
    (void)env;                                                                                                                               
    (void)obj;

    int ret = 0;
    ret = InitDev();
    if(ret != 0)
    {
        LOGI("[%s %d] error ret = %d\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }
      
    LOGD("%s\n", __FUNCTION__);                                                                                                                                  

    return 0;                                                                                                     
}

/*----------------------------------Device Control---------------------------------------*/
JNIEXPORT jint JNICALL JNI_API_DEF(mifare_open)(JNIEnv *env, jobject obj)                                                                
{
    (void)env;                                                                                                                               
    (void)obj;

    int ret = 0;
    ret = mifare_open();
    if(ret != 0)
    {
        LOGI("[%s %d] error ret = %d\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }
      
    LOGD("%s\n", __FUNCTION__);                                                                                                                                  

    return 0;                                                                                                     
}



JNIEXPORT jint JNICALL JNI_API_DEF(init_spi_psam)(JNIEnv *env, jobject obj)                                                                
{
    (void)env;                                                                                                                               
    (void)obj;

    int ret = 0;
    ret = init_spi();
    if(ret != 0)
    {
        LOGI("[%s %d] error ret = %d\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }
      
    LOGD("%s\n", __FUNCTION__);                                                                                                                                  

    return 0;                                                                                                     
}



JNIEXPORT jint JNICALL JNI_API_DEF(InitPsam)(JNIEnv *env, jobject obj,jchar PsamIndex,jint BaudRate)                                                                
{
    (void)env;                                                                                                                               
    (void)obj;

    int ret = 0;
    ret = InitPsam(PsamIndex,BaudRate);
    if(ret != 0)
    {
        LOGI("[%s %d] error ret = %d\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }
    
    LOGD("%s\n", __FUNCTION__);                                                                                                                             
    return 0;                                                                                                     
}


JNIEXPORT jint JNICALL JNI_API_DEF(PsamCmd)(JNIEnv *env, jobject obj,jchar PsamIndex,jbyteArray sendcmd,jint sendlen,jbyteArray rcvcmd,jint rcvlen)                                                                
{
    (void)env;                                                                                                                               
    (void)obj;

    int ret = 0;	
	char rcvbuf[255] = {0};

    ret = PsamCmd(PsamIndex,sendcmd,sendlen,rcvbuf,&rcvlen);
    if(ret != 0)
    {
        LOGI("[ %s %d ]GetPsamID error ret : %d!!\n",__FUNCTION__,__LINE__,ret);
		(*env)->SetByteArrayRegion(env, rcvcmd, 0, 2, (jbyte *)rcvbuf);
        return -1;  
    }

    {
        
        (*env)->SetByteArrayRegion(env, rcvcmd, 0, rcvlen, (jbyte *)rcvbuf);
    }
    
    LOGD("%s\n", __FUNCTION__);                                                                                                                                  

    return 0;                                                                                                     
}



#if 0

JNIEXPORT jint JNICALL JNI_API_DEF(GetPsamID)(JNIEnv *env, jobject obj,jchar PsamIndex,jbyteArray psamID)                                                                
{
    (void)env;                                                                                                                               
    (void)obj;

    int ret = 0;

    ret = GetPsamID();
    if(ret != 0)
    {
        LOGI("[ %s %d ]GetPsamID error ret : %d!!\n",__FUNCTION__,__LINE__,ret);
        return -4;  
    }

    {
        extern unsigned char PsamNum[6];
        (*env)->SetByteArrayRegion(env, psamID, 0, sizeof(PsamNum), (jbyte *)PsamNum);
    }
    
    LOGD("%s\n", __FUNCTION__);                                                                                                                                  

    return 0;                                                                                                     
}




JNIEXPORT jint JNICALL JNI_API_DEF(TERMAPP_QPBOCTermInit)(JNIEnv *env, jobject obj)                                                                
{
    (void)env;                                                                                                                               
    (void)obj;

    int ret = 0;
    ret = TERMAPP_QPBOCTermInit(0);
    if(ret != 0)
    {
        LOGI("[%s %d] error ret = %d\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }
      
    LOGD("%s\n", __FUNCTION__);                                                                                                                                  

    return 0;                                                                                                     
}  
#endif

/* SPI Operation */
JNIEXPORT jobject JNICALL JNI_API_DEF(SPIOpen)(JNIEnv *env, jclass thiz, jstring path, jint flags)
{
    int ret = 0;
    jobject mFileDescriptor;

    LOGD("%s\n",__func__);

    /* Opening device */
    {
        jboolean iscopy;
        const char *path_utf = (*env)->GetStringUTFChars(env, path, &iscopy);
        LOGD("Opening serial port %s with flags 0x%x", path_utf, O_RDWR | flags);
        fd_spi = open(path_utf, O_RDWR | flags);
        LOGD("open() fd_spi = %d", fd_spi);
        (*env)->ReleaseStringUTFChars(env, path, path_utf);
        if (fd_spi == -1)
        {
            /* Throw an exception */
            LOGE("Cannot open port");
            /* TODO: throw an exception */
            return NULL;
        }
    }

    /* Configure device */
    {
        /*
         * spi mode
         */
        ret = ioctl(fd_spi, SPI_IOC_WR_MODE, &mode);
        if (ret == -1)
        return NULL;

        ret = ioctl(fd_spi, SPI_IOC_RD_MODE, &mode);
        if (ret == -1)
            return NULL;

        /*
         * bits per word
         */
        ret = ioctl(fd_spi, SPI_IOC_WR_BITS_PER_WORD, &bits);
        if (ret == -1)
            return NULL;

        ret = ioctl(fd_spi, SPI_IOC_RD_BITS_PER_WORD, &bits);
        if (ret == -1)
            return NULL;
        /*
         * max speed hz
         */
        ret = ioctl(fd_spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        if (ret == -1)
        return NULL;

        ret = ioctl(fd_spi, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
        if (ret == -1)
            return NULL;
    }

    /* Create a corresponding file descriptor */
    {
        jclass cFileDescriptor = (*env)->FindClass(env, "java/io/FileDescriptor");
        jmethodID iFileDescriptor = (*env)->GetMethodID(env, cFileDescriptor, "<init>", "()V");
        jfieldID descriptorID = (*env)->GetFieldID(env, cFileDescriptor, "descriptor", "I");
        mFileDescriptor = (*env)->NewObject(env, cFileDescriptor, iFileDescriptor);
        (*env)->SetIntField(env, mFileDescriptor, descriptorID, (jint)fd_spi);
    }
    return mFileDescriptor;
}

JNIEXPORT void JNICALL JNI_API_DEF(SPIClose)(JNIEnv *env, jobject thiz)
{
    LOGD("%s\n",__func__);

    jclass SerialPortClass = (*env)->GetObjectClass(env, thiz);
    jclass FileDescriptorClass = (*env)->FindClass(env, "java/io/FileDescriptor");

    jfieldID mFdID = (*env)->GetFieldID(env, SerialPortClass, "mFd", "Ljava/io/FileDescriptor;");
    jfieldID descriptorID = (*env)->GetFieldID(env, FileDescriptorClass, "descriptor", "I");

    jobject mFd = (*env)->GetObjectField(env, thiz, mFdID);
    jint descriptor = (*env)->GetIntField(env, mFd, descriptorID);

    LOGD("close(fd_spi = %d)", descriptor);
    close(descriptor);
}

JNIEXPORT jbyteArray  JNICALL JNI_API_DEF(SPITransfer)(JNIEnv *env, jclass thiz, jbyteArray array)
{
    int ret;

    LOGD("%s\n",__func__);

    jsize len  = (*env)->GetArrayLength(env,array);

    LOGD("array length;%d",len);

    jbyte *tx = (jbyte *)malloc(len * sizeof(jbyte));
    (*env)->GetByteArrayRegion(env,array,0,len,tx);

    jbyte *rx = (jbyte *)malloc(len * sizeof(jbyte));

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = len,
        .delay_usecs = delay,
        .speed_hz = speed,
        .bits_per_word = bits,
    };

    ret = ioctl(fd_spi, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1){
        LOGI("can't send spi message");
        free(tx);
        free(rx);

        return NULL;
    }

    jbyteArray jarrRecv =(*env)->NewByteArray(env,len);//创建一个byte数组
    (*env)->SetByteArrayRegion(env,jarrRecv,0,len, (jbyte *)rx);
    (*env)->ReleaseByteArrayElements(env,array,tx,0);
    free(rx);

    return  jarrRecv;
}

static speed_t getBaudrate(jint baudrate)
{
    switch(baudrate) {
    case 0: return B0;
    case 50: return B50;
    case 75: return B75;
    case 110: return B110;
    case 134: return B134;
    case 150: return B150;
    case 200: return B200;
    case 300: return B300;
    case 600: return B600;
    case 1200: return B1200;
    case 1800: return B1800;
    case 2400: return B2400;
    case 4800: return B4800;
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
    case 230400: return B230400;
    case 460800: return B460800;
    case 500000: return B500000;
    case 576000: return B576000;
    case 921600: return B921600;
    case 1000000: return B1000000;
    case 1152000: return B1152000;
    case 1500000: return B1500000;
    case 2000000: return B2000000;
    case 2500000: return B2500000;
    case 3000000: return B3000000;
    case 3500000: return B3500000;
    case 4000000: return B4000000;
    default: return -1;
    }
}

/*
 * Class:     android_serialport_SerialPort
 * Method:    open
 * Signature: (Ljava/lang/String;II)Ljava/io/FileDescriptor;
 */
JNIEXPORT jobject JNICALL JNI_API_DEF(SerialOpen)(JNIEnv *env, jclass thiz, jstring path, jint baudrate, jint flags)
{
    speed_t speed;
    jobject mFileDescriptor;

    LOGD("%s\n",__func__);
    /* Check arguments */
    {
        speed = getBaudrate(baudrate);
        if (speed == -1) {
            /* TODO: throw an exception */
            LOGE("Invalid baudrate");
            return NULL;
        }
    }

    /* Opening device */
    {
        jboolean iscopy;
        const char *path_utf = (*env)->GetStringUTFChars(env, path, &iscopy);
        LOGD("Opening serial port %s with flags 0x%x", path_utf, O_RDWR | flags);
        fd_serial = open(path_utf, O_RDWR | flags);
        LOGD("open() fd_serial = %d", fd_serial);
        (*env)->ReleaseStringUTFChars(env, path, path_utf);
        if (fd_serial == -1)
        {
            /* Throw an exception */
            LOGE("Cannot open port");
            /* TODO: throw an exception */
            return NULL;
        }
    }

    /* Configure device */
    {
        struct termios cfg;
        LOGD("Configuring serial port");
        if (tcgetattr(fd_serial, &cfg))
        {
            LOGE("tcgetattr() failed");
            close(fd_serial);
            /* TODO: throw an exception */
            return NULL;
        }

        cfmakeraw(&cfg);
        cfsetispeed(&cfg, speed);
        cfsetospeed(&cfg, speed);

        if (tcsetattr(fd_serial, TCSANOW, &cfg))
        {
            LOGE("tcsetattr() failed");
            close(fd_serial);
            /* TODO: throw an exception */
            return NULL;
        }
    }

    /* Create a corresponding file descriptor */
    {
        jclass cFileDescriptor = (*env)->FindClass(env, "java/io/FileDescriptor");
        jmethodID iFileDescriptor = (*env)->GetMethodID(env, cFileDescriptor, "<init>", "()V");
        jfieldID descriptorID = (*env)->GetFieldID(env, cFileDescriptor, "descriptor", "I");
        mFileDescriptor = (*env)->NewObject(env, cFileDescriptor, iFileDescriptor);
        (*env)->SetIntField(env, mFileDescriptor, descriptorID, (jint)fd_serial);
    }

    return mFileDescriptor;
}

/*
 * Class:     cedric_serial_SerialPort
 * Method:    close
 * Signature: ()V
 * com.cardlan.twoshowinonescreen.serialport
 */
JNIEXPORT void JNICALL JNI_API_DEF(SerialClose)(JNIEnv *env, jobject thiz)
{
    LOGD("%s\n",__func__);
    jclass SerialPortClass = (*env)->GetObjectClass(env, thiz);
    jclass FileDescriptorClass = (*env)->FindClass(env, "java/io/FileDescriptor");

    jfieldID mFdID = (*env)->GetFieldID(env, SerialPortClass, "mFd", "Ljava/io/FileDescriptor;");
    jfieldID descriptorID = (*env)->GetFieldID(env, FileDescriptorClass, "descriptor", "I");

    jobject mFd = (*env)->GetObjectField(env, thiz, mFdID);
    jint descriptor = (*env)->GetIntField(env, mFd, descriptorID);

    LOGD("close(fd_serial = %d)", descriptor);
    close(descriptor);
}

/*----------------------------------Card Operation--------------------------------------*/

/* *************************************************************************************************************
- 函数说明 :复位卡片(寻卡，防碰撞，选卡)
- 输入参数 :data:卡片序列号(UID)    
- 返回值:卡类型 
**************************************************************************************************************/
JNIEXPORT jint JNICALL JNI_API_DEF(CardReset)(JNIEnv *env, jobject obj, jbyteArray array, jint type)                                                                
{
    (void)obj;                                                                                                                               

    unsigned char    data[128]            =    {0};
    int              result               =    0;
    jsize            len                  =    0;
    LOGD("%s\n", __FUNCTION__) ;                                                                                                                            
    result = CardReset(data, &len, type);
	LOGD("%s\n", __FUNCTION__) ;   
    (*env)->SetByteArrayRegion(env, array, 0, len, (jbyte *)data);
    return result;
}

JNIEXPORT void JNICALL JNI_API_DEF(CardHalt)(JNIEnv *env, jobject obj)
{
    (void)env;                                                                                                                               
    (void)obj;

    LOGD("%s\n", __FUNCTION__) ;   

    CardHalt();
   
}

/**************************************************************************************************************
- 函数说明 : 从卡中读取一个扇区的数据
- 输入参数 : BufLen: 缓冲区长度  SectorNo:读出那个扇区  BlockNo: 读出该扇区的第几块 VerifyFlag:密钥操作类型
                   key:扇区密钥        mode:密钥类型
- 输出参数 : 要读出的数据
- 返回值: 0 成功；1写密钥错误,2 密钥验证错误；3 读该块错误
**************************************************************************************************************/
JNIEXPORT jint JNICALL JNI_API_DEF(ReadOne2FiveSectorDataFromCard)(JNIEnv *env, jobject obj,
                                                                  jchar VerifyFlag, jbyteArray key_array, 
                                                                  jchar mode, jbyteArray array)
                                                                  
{                                                                                                                                            
    (void)obj;                                                                                                                                 
    jbyteArray jarrRecv; 

    unsigned char ReadBuf[512]; 
    unsigned char key[8] = {0};    
    unsigned int ReadLen;
    int ret;
    /* get key*/
    {
        int i;
        jbyte* bytearr = (*env)->GetByteArrayElements(env,key_array,NULL);
        int len = (*env)->GetArrayLength(env,key_array);

        if(len > 8)
        {
            len = 8;
        }
        
        for(i = 0; i < len; i++)
        {
            LOGI("bytearr[%d]=%d",i,bytearr[i]);//int 数组原来每一个元素的初始值
            key[i] = bytearr[i];
        }
    }
   
    ret = ReadOne2FiveSectorDataFromCard(ReadBuf,&ReadLen,key,mode);
    if(ret != 0)
    {
        LOGI("[%s %d] error ret = %d\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }
    LOGI("[%s %d] ReadLen = %d\n",__FUNCTION__,__LINE__,ReadLen);
            
    (*env)->SetByteArrayRegion(env,  array, 0, ReadLen, (jbyte *)ReadBuf);

    return 0;
}

/**************************************************************************************************************
- 函数说明 : 从卡中读取一个扇区的数据
- 输入参数 : BufLen: 缓冲区长度  SectorNo:读出那个扇区  BlockNo: 读出该扇区的第几块 VerifyFlag:密钥操作类型  key_array：密钥操作模式
                   key:扇区密钥        mode:密钥类型
- 输出参数 : 要读出的数据
- 返回值: 0 成功；1写密钥错误,2 密钥验证错误；3 读该块错误
**************************************************************************************************************/
JNIEXPORT jbyteArray JNICALL JNI_API_DEF(ReadOneSectorDataFromCard)(JNIEnv *env, jobject obj,
                                                            jchar SectorNo, jchar BlockNo,
                                                            jchar VerifyFlag, jbyteArray key_array, 
                                                            jchar mode)
{
    (void)obj;   
    jbyteArray jarrRecv; 

    unsigned char ReadBuf[16]; 
    unsigned char key[8] = {0};    
    unsigned long ReadLen;
    int ret;
    /* get key*/
    {
        int i;
        jbyte* bytearr = (*env)->GetByteArrayElements(env,key_array,NULL);
        int len = (*env)->GetArrayLength(env,key_array);

        if(len > 8)
        {
            len = 8;
        }
        
        for(i = 0; i < len; i++)
        {
            LOGI("bytearr[%d]=%d",i,bytearr[i]);//int 数组原来每一个元素的初始值
            key[i] = bytearr[i];
        }
    }
   
    ret = ReadOneSectorDataFromCard(ReadBuf,&ReadLen,SectorNo,BlockNo,VerifyFlag,key,mode);
    if(ret != 0)
    {
        LOGI("[%s %d] error ret = %d\n",__FUNCTION__,__LINE__,ret);
        ReadLen = 0;
    }
       
    jarrRecv = (*env)->NewByteArray(env,ReadLen);//创建一个byte数组
    (*env)->SetByteArrayRegion(env, jarrRecv, 0, ReadLen, (jbyte *)ReadBuf);
    
    return jarrRecv;
}


/**************************************************************************************************************
- 函数说明 : 将数据写到卡中
- 输入参数 : SectorArray: 要写入的数据    SectorNo:写入那个扇区  BlockNo: 写入该扇区的第几块  key_array：密钥操作模式
                  key:卡片密钥     mode:密钥类型
- 输出参数 :  0 成功；1写密钥错误,2 密钥验证错误；3 写该块错误
**************************************************************************************************************/
JNIEXPORT jint JNICALL JNI_API_DEF(WriteOneSertorDataToCard)(JNIEnv *env, jobject obj, jbyteArray SectorArray, 
                                                                                                jchar SectorNo, 
                                                                                                jchar BlockNo,
                                                                                                jchar VerifyFlag,
                                                                                                jbyteArray key_array,
                                                                                                jchar mode)
{
    (void)obj;       
    int ret ; 
    unsigned char key[8] = {0};    
    unsigned char WriteBuf[128] = {0};  
    int WriteBuf_len = 0;  

    /* get key*/
    { 
        int i;
        int len = (*env)->GetArrayLength(env,key_array);

        LOGI("len=%d",len);
        if(len > 8)    len = 8;

        jbyte* bytearr = (*env)->GetByteArrayElements(env,key_array,NULL);

        for(i = 0;i < len;i++)
        {
            LOGI("bytearr[%d]=%d",i,bytearr[i]);//int 数组原来每一个元素的初始值
            key[i] = bytearr[i];
        }
        (*env)->ReleaseByteArrayElements(env,key_array, bytearr, 0);
    }
    /* get send buffer and len*/
    { 
        int i;
        int len = (*env)->GetArrayLength(env,SectorArray);
        LOGI("len=%d",len);

        jbyte* bytearr = (*env)->GetByteArrayElements(env,SectorArray,NULL);

        for(i = 0; i < len; i++)
        {
            LOGI("bytearr[%d]=%d",i,bytearr[i]);//int 数组原来每一个元素的初始值
            WriteBuf[i] = bytearr[i];
        }
        WriteBuf_len = len;

        (*env)->ReleaseByteArrayElements(env,SectorArray, bytearr, 0);
    }

    ret = WriteOneSertorDataToCard(WriteBuf,WriteBuf_len,SectorNo,BlockNo,VerifyFlag,key,mode);
    if(ret != 0)
    {
        LOGI("[%s %d] error ret = %d\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }
       
    return 0;
}

/* CPU Card Operation */
JNIEXPORT jint JNICALL JNI_API_DEF(CpuSendCmd)(JNIEnv *env, jobject obj, jbyteArray Cmd, jbyteArray Recv)
{
    char recive_buff[256];
    char cmd_buff[256];
    int result;
    int i;

    LOGD("%s\n",__func__);

    /* Cmd Buff */
    int len = (*env)->GetArrayLength(env, Cmd);
    LOGI("len=%d",len);

    jbyte* bytearr = (*env)->GetByteArrayElements(env, Cmd,NULL);

    for(i = 0; i < len; i++)
    {
        cmd_buff[i] = bytearr[i];
    }

    (*env)->ReleaseByteArrayElements(env, Cmd, bytearr, 0);

    result = mifare_read_and_write(cmd_buff,len,recive_buff);
    if(result < 2)
    {
        LOGI("mifare_read_and_write fail result:%02X SW1:%02X SW2:%02X\n",result,recive_buff[0],recive_buff[1]);
        return -1;
    }
    (*env)->SetByteArrayRegion(env, Recv, 0, result, (jbyte *)recive_buff);

    return 0;
    
}


/**************************************************************************************************************
- 函数说明 : 将数据块转换成数值块
- 输入参数 :     SectorNo:扇区  号       BlockNo: 该扇区的第几块        key:卡片密钥     mode:密钥类型
- 输出参数 :  0 成功；1写密钥错误,2 密钥验证错误, 3 验证块错误 4 写该块错误
**************************************************************************************************************/
JNIEXPORT jint JNICALL JNI_API_DEF(ChangOneBlocktoMoneyBlock)(JNIEnv *env, jobject obj,jchar SectorNo, 
                                                                                       jchar BlockNo,
                                                                                       jchar VerifyFlag,
                                                                                       jbyteArray key_array,
                                                                                       jchar mode)
{
    (void)obj;       
    int ret ; 
    unsigned char key[8] = {0};     

    /* get key*/
    { 
        int i;
        int len = (*env)->GetArrayLength(env,key_array);

        LOGI("len=%d",len);
        if(len > 8)    len = 8;

        jbyte* bytearr = (*env)->GetByteArrayElements(env,key_array,NULL);

        for(i = 0;i < len;i++)
        {
            LOGI("bytearr[%d]=%d",i,bytearr[i]);//int 数组原来每一个元素的初始值
            key[i] = bytearr[i];
        }
        (*env)->ReleaseByteArrayElements(env,key_array, bytearr, 0);
    }  

    ret = changOneBlockToMoneyBlock(SectorNo,BlockNo,VerifyFlag,key,mode);
    if(ret != 0)
    {
        LOGI("[%s %d] error ret = %d\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }
       
    return 0;
}


/**************************************************************************************************************
- 函数说明 : 数值块增值
- 输入参数 : 	SectorNo:扇区号	   BlockNo: 该扇区的第几块		     key:卡片密钥	  mode:密钥类型 money:需要增值的金额
- 输出参数 :  0 成功；1写密钥错误,2 密钥验证错误, 3 验证块错误 4 写该块错误
**************************************************************************************************************/
JNIEXPORT jint JNICALL JNI_API_DEF(incMoney)(JNIEnv *env, jobject obj,jint Money,jchar SectorNo, 
																					  jchar BlockNo,
																					  jchar VerifyFlag,
																					  jbyteArray key_array,
																					  jchar mode)
{
   (void)obj;		
   int ret ; 
   unsigned char key[8] = {0};	   

   /* get key*/
   { 
	   int i;
	   int len = (*env)->GetArrayLength(env,key_array);

	   LOGI("len=%d",len);
	   if(len > 8)	  len = 8;

	   jbyte* bytearr = (*env)->GetByteArrayElements(env,key_array,NULL);

	   for(i = 0;i < len;i++)
	   {
		   LOGI("bytearr[%d]=%d",i,bytearr[i]);//int 数组原来每一个元素的初始值
		   key[i] = bytearr[i];
	   }
	   (*env)->ReleaseByteArrayElements(env,key_array, bytearr, 0);
   }  

   ret = incMoney(Money,SectorNo,BlockNo,VerifyFlag,key,mode);
   if(ret != 0)
   {
	   LOGI("[%s %d] error ret = %d\n",__FUNCTION__,__LINE__,ret);
	   return ret;
   }
	  
   return 0;
}

  /**************************************************************************************************************
  - 函数说明 : 数值块减值
  - 输入参数 :	  SectorNo:扇区号	 BlockNo: 该扇区的第几块 		   key:卡片密钥 	mode:密钥类型 money:需要增值的金额				  key_array：密钥操作模式
  - 输出参数 :	0 成功；1写密钥错误,2 密钥验证错误, 3 验证块错误 4 写该块错误
  **************************************************************************************************************/
  JNIEXPORT jint JNICALL JNI_API_DEF(decMoney)(JNIEnv *env, jobject obj,jint Money,jchar SectorNo, 
																						jchar BlockNo,
																						jchar VerifyFlag,
																						jbyteArray key_array,
																						jchar mode)
  {
	 (void)obj; 	  
	 int ret ; 
	 unsigned char key[8] = {0};	 
  
	 /* get key*/
	 { 
		 int i;
		 int len = (*env)->GetArrayLength(env,key_array);
  
		 LOGI("len=%d",len);
		 if(len > 8)	len = 8;
  
		 jbyte* bytearr = (*env)->GetByteArrayElements(env,key_array,NULL);
  
		 for(i = 0;i < len;i++)
		 {
			 LOGI("bytearr[%d]=%d",i,bytearr[i]);//int 数组原来每一个元素的初始值
			 key[i] = bytearr[i];
		 }
		 (*env)->ReleaseByteArrayElements(env,key_array, bytearr, 0);
	 }	
  
	 ret = decMoney(Money,SectorNo,BlockNo,VerifyFlag,key,mode);
	 if(ret != 0)
	 {
		 LOGI("[%s %d] error ret = %d\n",__FUNCTION__,__LINE__,ret);
		 return ret;
	 }
		
	 return 0;
  }


																						
/**************************************************************************************************************
- 函数说明 : 数据转存和转移
- 输入参数 :	SectorNo:扇区号	   restoreBlockNo: 待转存的块			transferBlockNo：目标块 key:卡片密钥	      mode:密钥类型 key_array：密钥操作模式
- 输出参数 :  0 成功；1写密钥错误,2 密钥验证错误, 3 验证块错误 4 写该块错误
**************************************************************************************************************/
JNIEXPORT jint JNICALL JNI_API_DEF(restoreAndTransfer)(JNIEnv *env, jobject obj,jchar SectorNo, 
																					  jchar restoreBlockNo,
																					  jchar transferBlockNo,																					  
																					  jbyteArray key_array,
																					  jchar mode)
{
   (void)obj;		
   int ret ; 
   unsigned char key[8] = {0};	   

   /* get key*/
   { 
	   int i;
	   int len = (*env)->GetArrayLength(env,key_array);

	   LOGI("len=%d",len);
	   if(len > 8)	  len = 8;

	   jbyte* bytearr = (*env)->GetByteArrayElements(env,key_array,NULL);

	   for(i = 0;i < len;i++)
	   {
		   LOGI("bytearr[%d]=%d",i,bytearr[i]);//int 数组原来每一个元素的初始值
		   key[i] = bytearr[i];
	   }
	   (*env)->ReleaseByteArrayElements(env,key_array, bytearr, 0);
   }  

   ret = restoreAndTransfer(SectorNo, restoreBlockNo, transferBlockNo,key, mode);
   if(ret != 0)
   {
	   LOGI("[%s %d] error ret = %d\n",__FUNCTION__,__LINE__,ret);
	   return ret;
   }
	  
   return 0;
}





#if 0
/*----------------------------------------JTB-------------------------------------------*/
JNIEXPORT jint JNICALL JNI_API_DEF(JTBCardread)(JNIEnv *env, jclass obj, jbyteArray data)
{   
    int ret = 0;

    struct
    {
        unsigned char CSN[4];                        //物理卡号
        unsigned char appserialnumber[10];           //应用序列号
        unsigned char mastercardtype;                //主卡类型 01普通 02学生 03老人 04测试 05军人 11成人月票 12老人免费 13老人优惠 14成人季票 15成人年票 70司机 80线路 81脱机采集
        unsigned char tradetime1E[7];                //交易日期时间
        unsigned char beforemoney[4];                //交易前余额
        unsigned char station1E[2];                  //站点
        unsigned char tradestate;                    //交易状态 0-初始值，1-上车，2-下车
        unsigned char directionflag;                 //方向标识 AB-上行，BA-下行
        unsigned char ytrasnum[4];                   //交易次数
        unsigned char CertiID[18];                   //司机卡证件号码
        unsigned char tradetype1E;                   //交易类型
    }TransferToApk;


    LOGI("%s\n",__func__);

    /* update CardLanCPU */
    ret = ReadCardInfor_CPU(&JTB_CardInfo);
    if(ret != MI_OK)
    {
        LOGI("[ %s %d ] error:%d!!\n",__FUNCTION__,__LINE__,ret);
        return -1;
    }

    /* File the struct */
    memcpy(TransferToApk.CSN, JTB_CardInfo.CSN, 4);
    memcpy(TransferToApk.appserialnumber, JTB_CardInfo.appserialnumber, 10);
    TransferToApk.mastercardtype = JTB_CardInfo.mastercardtype;
    memcpy(TransferToApk.tradetime1E, JTB_CardInfo.tradetime1E, 7);
    memcpy(TransferToApk.beforemoney, JTB_CardInfo.beforemoney, 4);
    memcpy(TransferToApk.station1E, JTB_CardInfo.station1E, 2);
    TransferToApk.tradestate = JTB_CardInfo.tradestate;
    TransferToApk.directionflag = JTB_CardInfo.directionflag;
    memcpy(TransferToApk.ytrasnum, JTB_CardInfo.ytrasnum, 4);
    memcpy(TransferToApk.CertiID, JTB_CardInfo.CertiID, 18);
    TransferToApk.tradetype1E = TransferToApk.tradetype1E;

    (*env)->SetByteArrayRegion(env, data, 0, sizeof(TransferToApk), (jbyte *)&TransferToApk);

    return sizeof(TransferToApk);
}

JNIEXPORT jint JNICALL JNI_API_DEF(JTBMonthlyCardConsume)(JNIEnv *env, jobject obj, jint Money)
{
    (void)env;                                                                                                                               
    (void)obj;
    LOGI("%s\n",__func__);
    return MonthlyCardConsume(&JTB_CardInfo,Money);
}

JNIEXPORT jint JNICALL JNI_API_DEF(JTBNormalCardConsume)(JNIEnv *env, jobject obj, jint Money)
{
    (void)env;                                                                                                                               
    (void)obj;
    LOGI("%s\n",__func__);

    return NormalCardConsume(&JTB_CardInfo,Money);
}


JNIEXPORT jint JNICALL JNI_API_DEF(JTBCardLock)(JNIEnv *env, jobject obj)
{
    (void)env;                                                                                                                               
    (void)obj;
    return CardLock();
}

JNIEXPORT void JNICALL JNI_API_DEF(DumpCardInfo)(JNIEnv *env, jobject obj)
{
    (void)env;                                                                                                                               
    (void)obj;
    Dump_CardInfo();
}

/*----------------------------------------YL-------------------------------------------*/
JNIEXPORT void JNICALL JNI_API_DEF(QPBOCCalcMac)(JNIEnv *env, jclass obj,jbyteArray jbyteArray_MacKey, jbyteArray jbyteArray_InData, jint InLen, jbyteArray jbyteArray_OutMAC)
{
    (void)env;                                                                                                                               
    (void)obj;

    unsigned char UploadMac[16];

    jbyte* MacKey = (*env)->GetByteArrayElements(env, jbyteArray_MacKey, NULL);
    jbyte* InData = (*env)->GetByteArrayElements(env, jbyteArray_InData, NULL);

    QPBOC_CalcMac(MacKey,InData,InLen,UploadMac);

    (*env)->SetByteArrayRegion(env, jbyteArray_OutMAC, 0, 8, (jbyte *)&UploadMac);
    
}

JNIEXPORT jint JNICALL JNI_API_DEF(YLCardread)(JNIEnv *env, jclass obj, jbyteArray data)
{   
    int ret = 0;
    struct
    {
        unsigned char tag_0x57[19+2];
        unsigned char tag_0x5a[10+2];

        unsigned char balance[4];
    }TransferToApk;
    
    YL_CPUCARD_Info_t   CardInfo;

    LOGI("%s\n",__func__);

    /* update CardLanCPU */
    ret = YL_ReadCardInfor_CPU(&CardInfo);
    if(ret != MI_OK)
    {
        LOGI("[ %s %d ] error:%d!!\n",__FUNCTION__,__LINE__,ret);
        return -1;
    }
    
    memset(&TransferToApk,0,sizeof(TransferToApk));
    memcpy(TransferToApk.tag_0x57, CardInfo.tag_0x57, sizeof(TransferToApk.tag_0x57));
    memcpy(TransferToApk.tag_0x5a, CardInfo.tag_0x5a, sizeof(TransferToApk.tag_0x5a));
    memcpy(TransferToApk.balance, CardInfo.balance,     sizeof(TransferToApk.balance));
    (*env)->SetByteArrayRegion(env, data, 0, sizeof(TransferToApk), (jbyte *)&TransferToApk);

    return sizeof(TransferToApk);
}


JNIEXPORT jint JNICALL JNI_API_DEF(YLCardConsume)(JNIEnv *env, jobject obj,jint type , jint Money)
{
    (void)env;                                                                                                                               
    (void)obj;

    LOGI("%s\n",__func__);
        
    if(type != 0)
    {
        LOGI("[ %s %d ] not support type :%d!!\n",__FUNCTION__,__LINE__,type);
        return -1;
    }
    int ret = TERMAPP_HandleCard(type,Money);
    return ret;
}

JNIEXPORT jint JNICALL JNI_API_DEF(YLCardHandleChannle1)(JNIEnv *env, jobject obj, jint Money,jbyteArray data)
{
    (void)env;                                                                                                                               
    (void)obj;

    LOGI("%s\n",__func__);
    int ret;
    struct
    {
        unsigned char emvrecord[256];
        int emvrecordlen;
    }TransferToApk;
/*
    {
        YL_CPU_CardInfo_t   CardInfo;
        ret = YL_ReadCardInfor_CPU(&CardInfo);
        if(ret != MI_OK)
        {
            LOGI("[ %s %d ] error:%d!!\n",__FUNCTION__,__LINE__,ret);
            return -1;
        }
        
        {
            memcpy(TransferToApk.balance, CardInfo.balance, sizeof(TransferToApk.balance));
        }
    }
  */  
    {
        ret = TERMAPP_HandleCard(1,Money);

        if(ret != MI_OK)
        {
            LOGI("[ %s %d ] error:%d!!\n",__FUNCTION__,__LINE__,ret);
            return -1;
        }
        {
            extern unsigned char emvrecord[256];
            extern int emvrecordlen;
            memset(&TransferToApk,0,sizeof(TransferToApk));
            memcpy(TransferToApk.emvrecord, emvrecord, sizeof(TransferToApk.emvrecord));
            memcpy(&TransferToApk.emvrecordlen, &emvrecordlen, sizeof(TransferToApk.emvrecordlen));
        }
    }
    (*env)->SetByteArrayRegion(env, data, 0, sizeof(TransferToApk), (jbyte *)&TransferToApk);

    return ret;
}
#endif

#if 0
JNIEXPORT jint JNICALL JNI_API_DEF(JTBCardLock)(JNIEnv *env, jobject obj)
{
    (void)env;                                                                                                                               
    (void)obj;
    return CardLock();
}

JNIEXPORT void JNICALL JNI_API_DEF(DumpCardInfo)(JNIEnv *env, jobject obj)
{
    (void)env;                                                                                                                               
    (void)obj;
    Dump_CardInfo();
}
#endif


