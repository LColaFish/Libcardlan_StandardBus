/*
These code were got from Internet,I have no any work on it.
*/
#ifndef _STADES_H
#define _STADES_H

#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef uint32
#define uint32 unsigned int
#endif

#define   true    ((char)0)
#define   false   ((char)-1)



typedef struct
{
    uint32 esk[32];     /* DES encryption subkeys */
    uint32 dsk[32];     /* DES decryption subkeys */
} des_context;

typedef struct
{
    uint32 esk[96];     /* Triple-DES encryption subkeys */
    uint32 dsk[96];     /* Triple-DES decryption subkeys */
} des3_context;

int  des_set_key( des_context *ctx, uint8 key[8] );
void des_encrypt( des_context *ctx, uint8 input[8], uint8 output[8] );
void des_decrypt( des_context *ctx, uint8 input[8], uint8 output[8] );

int  des3_set_2keys( des3_context *ctx, uint8 key1[8], uint8 key2[8] );
int  des3_set_3keys( des3_context *ctx, uint8 key1[8], uint8 key2[8], uint8 key3[8] );

void des3_encrypt( des3_context *ctx, uint8 input[8], uint8 output[8] );
void des3_decrypt( des3_context *ctx, uint8 input[8], uint8 output[8] );

#define  ENCRYPT 	 0	//加密
#define  DECRYPT 	 1	//解密
#define  ECB 	     0	//ECB模式
#define  CBC 		 1	//CBC模式
#define  PAD_ISO_1   0	//ISO_1填充：数据长度不足8比特的倍数，以0x00补足，如果为8比特的倍数，补8个0x00
#define  PAD_ISO_2   1	//ISO_2填充：数据长度不足8比特的倍数，以0x80,0x00..补足，如果为8比特的倍数，补0x80,0x00..0x00
#define  PAD_PKCS_7  2 	//PKCS7填充：数据长度除8余数为n,以(8-n)补足为8的倍数
#define  PAD_PBOC    3

/*******************************************************************/
/*
  函 数 名 称:	RunPad
  功 能 描 述：	根据协议对加密前的数据进行填充
  参 数 说 明：	bType	:类型：PAD类型
				In		:数据串指针
				Out		:填充输出串指针
				datalen	:数据的长度
				padlen	:(in,out)输出buffer的长度，填充后的长度

  返回值 说明：	bool	:是否填充成功
*/
/*******************************************************************/
char RunPad(char nType,const char* In,unsigned int datalen,char* Out,unsigned int* padlen);

/*******************************************************************/
/*
  函 数 名 称:	RunDes
  功 能 描 述：	执行DES算法对文本加解密
  参 数 说 明：	bType	:类型：加密ENCRYPT，解密DECRYPT
				bMode	:模式：ECB,CBC
				In		:待加密串指针
				Out		:待输出串指针
				datalen	:待加密串的长度，同时Out的缓冲区大小应大于或者等于datalen
				Key		:密钥(可为8位,16位,24位)支持3密钥
				keylen	:密钥长度，多出24位部分将被自动裁减

  返回值 说明：	bool	:是否加密成功
*/
/*******************************************************************/
char RunDes(char bType,char bMode,char* In,char* Out,unsigned int datalen,const char* Key,const unsigned char keylen);

//按位取反操作
void Not(char *Out, const char *In, int len);

//按位异或操作
void XOR(char *Out, const char *In, int len);


#endif /* des.h */

