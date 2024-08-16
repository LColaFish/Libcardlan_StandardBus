#ifndef __TERMAPP_APDU_H__
#define __TERMAPP_APDU_H__

#include "TERMAPP_intger.h"

extern uchar SRCPUCardBuf[1024];
extern unsigned int SRCPUCardCount;

extern uchar TERMAPP_SelectApp(const uchar P1,const uchar P2,uchar len,const uchar *filename, uchar *state);
extern uchar TERMAPP_ReadBinary(uchar sfi,uchar lc,uchar *state) ;
int TERMAPP_ReadRecord(uchar sfi,uchar num,unsigned char *p_rev_buffer,uchar *state);
int TERMAPP_GetGPO(uchar *gpolist,uchar len,unsigned char *p_rev_buffer,uchar *state);
extern uchar TERMAPP_GetBalance(unsigned int *amount);

#endif
