#ifndef _LIBCARDLAN_JTB_UTIL_H_
#define _LIBCARDLAN_JTB_UTIL_H_
#include <jni.h>

#define JNI_API_DEF(f) Java_com_cardlan_colafish_psam_##f
//#define JNI_API_DEF(f) Java_com_cardlan_twoshowinonescreen_CardlanDesfireBus_##f



#define BUILD_VERSION "v1.0.0 "__DATE__" "__TIME__


//#define     ANDROID_CODE_DEBUG
//#define     NDK_CODE_DEBUG

#define     MI_OK                  0x00
#define     MI_FAIL                0x01
#define     MI_NOFOUND             0x02
#define     SWIPE_CARD_ERROR       0xFF

extern void Dump_CardInfo(void);

int hex_2_ascii(unsigned char *INdata, char *buffer, unsigned int len);
unsigned char HEX2BCD(unsigned char hex_data);

unsigned char BinarySearchTongrui(unsigned char *buff,unsigned char *cardtype);
unsigned int CardNumCBD_ascii(unsigned char *buf, unsigned int len, unsigned char *buf1,unsigned int size,unsigned char *ascii);


#endif
