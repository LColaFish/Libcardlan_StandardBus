#ifndef __INCLUDES_H__
#define __INCLUDES_H__

#include "TERMAPP_intger.h"
#include "TERMAPP_Apdu.h"
#include "TERMAPP_EMVdata.h"
#include "TERMAPP_posapi.h"
#include "TERMAPP_str.h"
#include "TERMAPP_VAR.h"
#include "TERMAPP_TERSTRUC.h"
#include "TERMAPP_VarMacro.h"
#include "TERMAPP_DataAuth.h"
#include "time.h"

#if 1
#ifndef NULL
#define NULL         ((void*)0)
#endif
#endif

#ifndef uint8_t
#define uint8_t      unsigned char
#define uint16_t     unsigned short
#define uint32_t     unsigned int
#endif

#ifndef int16_t
typedef signed short int16_t;
typedef signed int int32_t;
#endif

extern unsigned char TERMAPP_DOLProcess(uchar type,uchar *DOL,uchar DOLLen,uchar *DOLData,uchar *DOLDataLen);
extern uchar RSAPublicDecrypt(uchar *PublicKey,u32 exponent,int keyLength,uchar *encryptnum,uchar *decresult);
extern void sha1( const unsigned char *input, int ilen, unsigned char *output);

extern uchar TERMAPP_ChooseApp(void);
extern uchar TERMAPP_PDOLProcess(uchar *pdolData,unsigned int *lenOut);
extern uchar TERMAPP_DecodeTLV(uchar *TLVBuf,u32 Length);
extern uchar DecodeTLVLen(uchar *TLVBuf,u32 Length);
extern void  TERMAPP_AppCopy(uchar i,uchar j);
extern int TERMAPP_ReadRecord(uchar sfi,uchar num,unsigned char *p_rev_buffer,uchar *state);
extern int TERMAPP_GetGPO(uchar *gpolist,uchar len,unsigned char *p_rev_buffer,uchar *state);
extern uchar TERMAPP_GetICParameter(uchar *AID,uchar length);
extern uchar TERMAPP_ExpireDate(uchar* ExpireDate);    
void TERMAPP_EncodeTLV(uchar *DOL, uchar DOLLen, uchar *TLVBuf, uchar *TLVLen);
extern void HexToAsc(unsigned char *ascii_buf,unsigned char *hex_buf,int ascii_len);

extern long long bcd_to_bin(uchar *bcd, uchar len);

extern void asc_to_bcd(uchar *bcd, uchar *asc, unsigned int asc_len);
extern void ASCToBCD(uchar * bcd_buf, uchar * asc_buf, int n);
extern void BCDToASC(uchar * asc_buf, uchar * bcd_buf, int n);
extern uchar TERMAPP_AppSelect(void);
extern uchar TERMAPP_ReadAppData(void);
extern uchar TERMAPP_DataAuth(unsigned char channle);
extern uchar TERMAPP_StaticAuth(void);
extern uchar TERMAPP_DynamicAuth(void);
extern void  TERMAPP_QPBOCTransInit(unsigned int transSeqId, unsigned int amount,int channel);
extern void  TERMAPP_QPBOCTermInit(int channel);
extern uchar TERMAPP_ProcessRestrict(void);
extern uchar TERMAPP_TermRiskManage(void);
extern uchar TERMAPP_TermActAnaly(void);
extern void  TERMAPP_TransDetail(void);
extern uchar TERMAPP_SelectApp(const uchar P1,const uchar P2,uchar len,const uchar *filename, uchar * state);
extern void menu_print(char *buffer, int length);


#endif
