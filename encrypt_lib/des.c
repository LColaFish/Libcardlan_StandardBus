/*********************************************************************************************
* File��	M_DES.c
* Author:	embest	
* Desc��	control board's two LEDs on or offf
* History:	
*********************************************************************************************/
#include <string.h>
#include "des.h"

static unsigned char IN_Data[50];
/*******DES----TAB----BEGIN**************************************************************/
const unsigned char  EPIP[128]   = {0x07,0x40,0x06,0x40,0x05,0x40,0x04,0x40,0x03,0x40,0x02,0x40,0x01,0x40,0x00,0x40,
	                    0x07,0x10,0x06,0x10,0x05,0x10,0x04,0x10,0x03,0x10,0x02,0x10,0x01,0x10,0x00,0x10,
	                    0x07,0x04,0x06,0x04,0x05,0x04,0x04,0x04,0x03,0x04,0x02,0x04,0x01,0x04,0x00,0x04,
	                    0x07,0x01,0x06,0x01,0x05,0x01,0x04,0x01,0x03,0x01,0x02,0x01,0x01,0x01,0x00,0x01,
                        0x07,0x80,0x06,0x80,0x05,0x80,0x04,0x80,0x03,0x80,0x02,0x80,0x01,0x80,0x00,0x80,
                        0x07,0x20,0x06,0x20,0x05,0x20,0x04,0x20,0x03,0x20,0x02,0x20,0x01,0x20,0x00,0x20,
                        0x07,0x08,0x06,0x08,0x05,0x08,0x04,0x08,0x03,0x08,0x02,0x08,0x01,0x08,0x00,0x08,
                        0x07,0x02,0x06,0x02,0x05,0x02,0x04,0x02,0x03,0x02,0x02,0x02,0x01,0x02,0x00,0x02};

const unsigned char  EPIP_1[128]    = {0x04,0x01,0x00,0x01,0x05,0x01,0x01,0x01,0x06,0x01,0x02,0x01,0x07,0x01,0x03,0x01,
	                    0x04,0x02,0x00,0x02,0x05,0x02,0x01,0x02,0x06,0x02,0x02,0x02,0x07,0x02,0x03,0x02,
	                    0x04,0x04,0x00,0x04,0x05,0x04,0x01,0x04,0x06,0x04,0x02,0x04,0x07,0x04,0x03,0x04,
	                    0x04,0x08,0x00,0x08,0x05,0x08,0x01,0x08,0x06,0x08,0x02,0x08,0x07,0x08,0x03,0x08,
	                    0x04,0x10,0x00,0x10,0x05,0x10,0x01,0x10,0x06,0x10,0x02,0x10,0x07,0x10,0x03,0x10,
	                    0x04,0x20,0x00,0x20,0x05,0x20,0x01,0x20,0x06,0x20,0x02,0x20,0x07,0x20,0x03,0x20,
	                    0x04,0x40,0x00,0x40,0x05,0x40,0x01,0x40,0x06,0x40,0x02,0x40,0x07,0x40,0x03,0x40,
                        0x04,0x80,0x00,0x80,0x05,0x80,0x01,0x80,0x06,0x80,0x02,0x80,0x07,0x80,0x03,0x80};

const unsigned char EP32_48[96]    = {0x03,0x01,0x00,0x80,0x00,0x40,0x00,0x20,0x00,0x10,0x00,0x08,
	                    0x00,0x10,0x00,0x08,0x00,0x04,0x00,0x02,0x00,0x01,0x01,0x80,
	                    0x00,0x01,0x01,0x80,0x01,0x40,0x01,0x20,0x01,0x10,0x01,0x08,
	                    0x01,0x10,0x01,0x08,0x01,0x04,0x01,0x02,0x01,0x01,0x02,0x80,
	                    0x01,0x01,0x02,0x80,0x02,0x40,0x02,0x20,0x02,0x10,0x02,0x08,
	                    0x02,0x10,0x02,0x08,0x02,0x04,0x02,0x02,0x02,0x01,0x03,0x80,
	                    0x02,0x01,0x03,0x80,0x03,0x40,0x03,0x20,0x03,0x10,0x03,0x08,
			    0x03,0x10,0x03,0x08,0x03,0x04,0x03,0x02,0x03,0x01,0x00,0x80};

const unsigned char SBOX0[64]    = {14,0,4,15,13,7,1,4,2,14,15,2,11,13,8,
	                    1,3,10,10,6,6,12,12,11,5,9,9,5,0,3,7,8,
	                    4,15,1,12,14,8,8,2,13,4,6,9,2,1,11,7,15,
	                    5,12,11,9,3,7,14,3,10,10,0,5,6,0,13};

const unsigned char SBOX1[64]    = {15,3,1,13,8,4,14,7,6,15,11,2,3,8,4,14,
	                    9,12,7,0,2,1,13,10,12,6,0,9,5,11,10,5,
	                    0,13,14,8,7,10,11,1,10,3,4,15,13,4,1,
	                    2,5,11,8,6,12,7,6,12,9,0,3,5,2,14,15,9};

const unsigned char SBOX2[64]    = {10,13,0,7,9,0,14,9,6,3,3,4,15,6,5,10,1,
	                    2,13,8,12,5,7,14,11,12,4,11,2,15,8,1,
	                    13,1,6,10,4,13,9,0,8,6,15,9,3,8,0,7,
	                    11,4,1,15,2,14,12,3,5,11,10,5,14,2,7,12};

const unsigned char SBOX3[64]    = {7,13,13,8,14,11,3,5,0,6,6,15,9,0,10,3,1,
	                    4,2,7,8,2,5,12,11,1,12,10,4,14,15,9,
	                    10,3,6,15,9,0,0,6,12,10,11,1,7,13,13,
	                    8,15,9,1,4,3,5,14,11,5,12,2,7,8,2,4,14};

const unsigned char SBOX4[64]    = {2,14,12,11,4,2,1,12,7,4,10,7,11,13,6,1,
	                    8,5,5,0,3,15,15,10,13,3,0,9,14,8,9,6,
	                    4,11,2,8,1,12,11,7,10,1,13,14,7,2,8,13,
	                    15,6,9,15,12,0,5,9,6,10,3,4,0,5,14,3};

const unsigned char SBOX5[64]    = {12,10,1,15,10,4,15,2,9,7,2,12,6,9,8,5,
	                    0,6,13,1,3,13,4,14,14,0,7,11,5,3,11,8,
	                    9,4,14,3,15,2,5,12,2,9,8,5,12,15,3,10,
	                    7,11,0,14,4,1,10,7,1,6,13,0,11,8,6,13};

const unsigned char SBOX6[64]    = {4,13,11,0,2,11,14,7,15,4,0,9,8,1,13,10,
	                    3,14,12,3,9,5,7,12,5,2,10,15,6,8,1,6,
	                    1,6,4,11,11,13,13,8,12,1,3,4,7,10,14,
	                    7,10,9,15,5,6,0,8,15,0,14,5,2,9,3,2,12};

const unsigned char SBOX7[64]    ={13,1,2,15,8,13,4,8,6,10,15,3,11,7,1,4,10,
                    12,9,5,3,6,14,11,5,0,0,14,12,9,7,2,
                    7,2,11,1,4,14,1,7,9,4,12,10,14,8,2,13,
                    0,15,6,12,10,9,13,0,15,3,3,5,5,6,8,11};


const unsigned char EP32_32[64]    = {0x01,0x01,0x00,0x02,0x02,0x10,0x02,0x08,
	                    0x03,0x08,0x01,0x10,0x03,0x10,0x02,0x80,
	                    0x00,0x80,0x01,0x02,0x02,0x02,0x03,0x40,
	                    0x00,0x08,0x02,0x40,0x03,0x02,0x01,0x40,
	                    0x00,0x40,0x00,0x01,0x02,0x01,0x01,0x04,
	                    0x03,0x01,0x03,0x20,0x00,0x20,0x01,0x80,
	                    0x02,0x20,0x01,0x08,0x03,0x04,0x00,0x04,
	                    0x02,0x04,0x01,0x20,0x00,0x10,0x03,0x80};

const unsigned char EP64_56[112]    = {0x07,0x80,0x06,0x80,0x05,0x80,0x04,0x80,0x03,0x80,0x02,0x80,0x01,0x80,0x00,0x80,
	                    0x07,0x40,0x06,0x40,0x05,0x40,0x04,0x40,0x03,0x40,0x02,0x40,0x01,0x40,0x00,0x40,
                        0x07,0x20,0x06,0x20,0x05,0x20,0x04,0x20,0x03,0x20,0x02,0x20,0x01,0x20,0x00,0x20,
                        0x07,0x10,0x06,0x10,0x05,0x10,0x04,0x10,
				        0x07,0x02,0x06,0x02,0x05,0x02,0x04,0x02,0x03,0x02,0x02,0x02,0x01,0x02,0x00,0x02,
                        0x07,0x04,0x06,0x04,0x05,0x04,0x04,0x04,0x03,0x04,0x02,0x04,0x01,0x04,0x00,0x04,
				        0x07,0x08,0x06,0x08,0x05,0x08,0x04,0x08,0x03,0x08,0x02,0x08,0x01,0x08,0x00,0x08,
				        0x03,0x10,0x02,0x10,0x01,0x10,0x00,0x10};

const unsigned char EP56_48[96]    = {0x01,0x04,0x02,0x80,0x01,0x20,0x02,0x01,0x00,0x80,0x00,0x08,
	                    0x00,0x20,0x03,0x10,0x01,0x02,0x00,0x04,0x02,0x08,0x01,0x40,
	                    0x02,0x02,0x02,0x20,0x01,0x10,0x00,0x10,0x03,0x40,0x00,0x01,
	                    0x01,0x01,0x00,0x02,0x03,0x20,0x02,0x10,0x01,0x08,0x00,0x40,
	                    0x05,0x80,0x06,0x10,0x03,0x02,0x04,0x08,0x05,0x02,0x06,0x02,
	                    0x03,0x04,0x04,0x01,0x06,0x20,0x05,0x08,0x04,0x80,0x05,0x01,
	                    0x05,0x10,0x06,0x80,0x04,0x02,0x06,0x01,0x04,0x40,0x06,0x08,
	                    0x05,0x04,0x05,0x40,0x06,0x40,0x04,0x10,0x03,0x08,0x03,0x01};

const unsigned char SHIFTCNT[16]    ={1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};
/*******DES----TAB-----END**************************************************************/
static void EXCHANGE_EXC6BIT(unsigned char len,const unsigned char  *DES_Tab,unsigned char *DES_KEY,unsigned char *DES_OUT,unsigned char DES_cmd)
{  
unsigned char ch,i,j,x,y,z,m;
	j = 0;
	ch = DES_cmd;
	y = 0;
	for(i = 0;i < len;i++)
	{ 
      m = DES_KEY[DES_Tab[j]] ;
	  j++;
	  z	= DES_Tab[j]; 
      x = m & z;
    
	 if(x != 0) 
	 {
		 DES_OUT[y] =DES_OUT[y] | ch;
	 }
	  
	 if((ch&0x01) == 0x01)
	  {
	   	ch = DES_cmd;
	   y++;   
	  }
	  else ch = ch>>1;
	  j++;	  
	}
}


static void T2RL28(void)
{  
 unsigned char ch= 0,cl=0,flag1=0,flag2=0;

 ch = IN_Data[19] & 0xf0;
 cl = IN_Data[19] & 0x0f;
 
 flag1 = 0x00;
 if((ch & 0x80) == 0x80) flag1 = 0x01;
 IN_Data[19] = ch << 1;
 
 flag2 = 0x00;
 if((IN_Data[18] & 0x80) == 0x80) flag2 = 0x01;
 IN_Data[18] = (IN_Data[18]<<1) | flag1;

 flag1 = 0x00;
 if((IN_Data[17] & 0x80) == 0x80) flag1 = 0x01;
 IN_Data[17] = (IN_Data[17]<<1) | flag2;

 flag2 = 0x00;
 if((IN_Data[16] & 0x80) == 0x80) flag2 = 0x01;
 IN_Data[16] = (IN_Data[16]<<1) | flag1;

 if(flag2 == 0x01)
 {
   IN_Data[19] = IN_Data[19] | 0x10;
   IN_Data[19] = IN_Data[19] | cl;
 }
 else
 {
	 IN_Data[19] = IN_Data[19] & 0xef;
	 IN_Data[19] = IN_Data[19] | cl;
 }

 ch = IN_Data[19] & 0xf0;
 
 flag2 =0x00;
 flag1 = 0;
 if((IN_Data[22] & 0x80) == 0x80) flag1 = 0x01;
 IN_Data[22] = IN_Data[22] << 1 | flag2; 

 flag2 = 0;
 if((IN_Data[21] & 0x80) == 0x80) flag2 = 0x01;
 IN_Data[21] = (IN_Data[21]<<1) | flag1;

 flag1 = 0;
 if((IN_Data[20] & 0x80) == 0x80) flag1 = 0x01;
 IN_Data[20] = (IN_Data[20]<<1) | flag2;

 cl = (IN_Data[19] << 1)|flag1;
 IN_Data[19] = (cl & 0x0f) | ch;

 if((cl & 0x10) == 0x10)
 {
   IN_Data[19] = (cl & 0x0f) | ch;
   IN_Data[22] = IN_Data[22] | 0x01;
 }
 else
 {
  IN_Data[19] = (cl & 0x0f) | ch;
  IN_Data[22] = IN_Data[22] & 0xfe;
 }
}



static void Cler_buff(unsigned char *Buff,unsigned char begin,unsigned char ch)
{
	unsigned char i;
	for(i = begin;i < begin+ch;i++)
	{
		Buff[i] = 0x00;
	}
}


extern void DES_CARD(unsigned char *Key,unsigned char *M_Data,unsigned char *IC_Data)
{
unsigned char i,j,x,y;
unsigned char ch,cl,cx,cy;

memset(IN_Data,0,sizeof(IN_Data));
memcpy(IN_Data,M_Data,8);
memcpy(&IN_Data[24],Key,8);
 Cler_buff(IN_Data,16,8);
 EXCHANGE_EXC6BIT(56,EP64_56,&IN_Data[24],&IN_Data[16],EXCHANGE);
 Cler_buff(IN_Data,8,8);
 EXCHANGE_EXC6BIT(64,EPIP,&IN_Data[0],&IN_Data[8],EXCHANGE);
for(i =0 ;i< 16;i++)
 {
 Cler_buff(IN_Data,0,8);
 EXCHANGE_EXC6BIT(48,EP32_48,&IN_Data[12],&IN_Data[0],EXC6BIT);
  for(j =0;j<SHIFTCNT[i];j++)
  {
   T2RL28();
  }
  Cler_buff(IN_Data,24,8);
  EXCHANGE_EXC6BIT(48,EP56_48,&IN_Data[16],&IN_Data[24],EXC6BIT);//OK--------------OK
  for(y = 0;y < 8;y++)
  {
   IN_Data[y] = IN_Data[y]^IN_Data[y + 24];
  }
  x = 0;
  cx =  IN_Data[x];
  cx =   SBOX0[cx];
  ch = (cx<<4)&0xf0;
  cl = (cx>>4)&0x0f;
  cy = ch|cl;
  x++;
  cx =  IN_Data[x];
  cx =   SBOX1[cx];
  IN_Data[0] = cx|cy;
  x++;
  cx =  IN_Data[x];
  cx =   SBOX2[cx];
  ch = (cx<<4)&0xf0;
  cl = (cx>>4)&0x0f;
  cy = ch|cl;
  x++;
  cx =  IN_Data[x];
  cx =   SBOX3[cx];
  IN_Data[1] = cx|cy;
  x++;
  cx =  IN_Data[x];
  cx =   SBOX4[cx];
  ch = (cx<<4)&0xf0;
  cl = (cx>>4)&0x0f;
  cy = ch|cl;
  x++;
  cx =  IN_Data[x];
  cx =   SBOX5[cx];
  IN_Data[2] = cx|cy;
  x++;
  cx =  IN_Data[x];
  cx =   SBOX6[cx];
  ch = (cx<<4)&0xf0;
  cl = (cx>>4)&0x0f;
  cy = ch|cl;
  x++;
  cx =  IN_Data[x];
  cx =   SBOX7[cx];
  IN_Data[3] = cx|cy;//ok---------------------------OK 
  Cler_buff(IN_Data,4,4);
  EXCHANGE_EXC6BIT(32,EP32_32,&IN_Data[0],&IN_Data[4],EXCHANGE);//OK
  cy = IN_Data[8];
  IN_Data[8] = IN_Data[12];
  IN_Data[12] = cy ^ IN_Data[4];
  cy = IN_Data[9];
  IN_Data[9] = IN_Data[13];
  IN_Data[13] = cy ^ IN_Data[5];
  cy = IN_Data[10];
  IN_Data[10] = IN_Data[14];
  IN_Data[14] = cy ^ IN_Data[6];
  cy = IN_Data[11];
  IN_Data[11] = IN_Data[15];
  IN_Data[15] = cy ^ IN_Data[7];
}
Cler_buff(IN_Data,0,8);
EXCHANGE_EXC6BIT(64,EPIP_1,&IN_Data[8],&IN_Data[0],EXCHANGE);//OK
memcpy(IC_Data,IN_Data,6);
}



