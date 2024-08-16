#ifndef __TERMAPP_DATAAUTH_H__
#define __TERMAPP_DATAAUTH_H__

typedef struct {
    uchar RID[5];			//注册的应用提供商标识
    uchar CAPKI;			//认证中心公钥索引
    unsigned int Exponent;	//认证中心公钥指数
    uchar ModulLen;			//认证中心公钥模长度
    uchar *pModul;			//认证中心公钥模
} CAPK_STRUCT2;

extern CAPK_STRUCT2 CAPK_list[];

#endif //#ifndef __TERMAPP_DATAAUTH_H__
