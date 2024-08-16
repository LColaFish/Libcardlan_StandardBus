#include "M1_card_operation.h"
#include <stdio.h>
#include "libcardlan_StandardBus_util.h"
#include "common/cardlan_StandardBus_tpye.h"

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
/*
	功 能：防卡冲突，返回卡的序列号后4个字节

	参 数：

		   _Bcn： 设为0

		   _Snr：返回的卡序列号地址

	返 回：成功则返回 0
*/
int anticoll2(unsigned char _Bcnt,unsigned long *_Snr){return 0;}

/*
功 能：从多个卡中选取一个给定序列号(卡序列号的后4个字节)的卡

参 数：
       _Snr：卡序列号(dc_anticoll2返回的卡号)

       _Size：指向返回的卡容量的数据

返 回：成功则返回 0
*/
int select2(unsigned long _Snr,unsigned char *_Size){return 0;}
/*
调用：
      unsigned char *rlen ---- 返回复位信息的长度

      unsigned char * rbuff ---- 存放返回的复位信息

返回： <0 错误。其绝对值为错误号

      =0 成功。
*/
int pro_reset(unsigned char *rlen, unsigned char *rbuff){return 0;}
/*
	功 能：中止对该卡操作

	参 数：icdev：通讯设备标识符

	返 回：成功则返回0
*/
int pro_halt(void){return 0;}
/*
	说明：应用协议数据单元信息交换函数。该函数已封装T=CL操作

	调用：
		  unsigned char slen ---- 发送的信息长度

		  unsigned char * sbuff ---- 存放要发送的信息

		  unsigned char *rlen ---- 返回信息的长度

		  unsigned char * rbuff ---- 存放返回的信息

		  unsigned char tt---- 延迟时间，单位为：10ms

	返回： <0 错误。其绝对值为错误号

		  =0 成功。
*/
int pro_command(unsigned char slen,unsigned char * sbuff,unsigned char *rlen,unsigned char * rbuff,unsigned char tt){return 0;}

/*
	说明：应用协议数据单元信息交换函数。该函数不封装，用户需自行组织数据发送 

	调用：
		  unsigned char slen ---- 发送的信息长度

		  unsigned char * sbuff ---- 存放要发送的信息

		  unsigned char *rlen ---- 返回信息的长度

		  unsigned char * rbuff ---- 存放返回的信息

		  unsigned char timeout ---- 延迟时间，单位为：10ms

	返回： <0 错误。其绝对值为错误号 

		  =0 成功。

*/
int pro_commandsource(unsigned char slen,unsigned char * sbuff,unsigned char *rlen,unsigned char * rbuff,unsigned char timeout){return 0;}
/*
	说明：应用协议数据单元信息交换函数。该函数已封装T=CL操作

	调用：
		  unsigned char slen ---- 发送的信息长度

		  unsigned char * sbuff ---- 存放要发送的信息

		  unsigned char *rlen ---- 返回信息的长度

		  unsigned char * rbuff ---- 存放返回的信息

		  unsigned char tt---- 延迟时间，单位为：10ms

		  unsigned char FG---- 分割长度。建议此值小于64

	返回： <0 错误。其绝对值为错误号

		  =0 成功。

*/
int pro_commandlink(unsigned char slen,unsigned char * sbuff,unsigned char *rlen,unsigned char * rbuff,unsigned char tt,unsigned char FG){return 0;}

/*
	说明：Mifare Desfire卡密码认证函数

	调用：
		  unsigned char ucIndexOfKey, ----密钥标志

		  unsigned char ucKeyLenInByte, ----密钥长度（字节）

		  unsigned char *szKey, ----密钥

		  unsigned char RandA, ----随机数A

		  unsigned char RandB----随机数B



	返回： <0 错误。其绝对值为错误号

		  =0 成功。
*/
int mfdes_auth(unsigned char ucIndexOfKey, unsigned char ucKeyLenInByte, unsigned char *szKey,  unsigned char RandA, unsigned char RandB){return 0;}

