#ifndef TYPEA_H
#define TYPEA_H

#include <sys/types.h>
#include "pcd.h"
#include "ISO14443.h"
#include "ISO15693.h"

#define		KEYA			0x0A
#define		KEYB			0x0B

#define 	DO_TYPEA_M1             0xB0
#define		DO_TYPEA_CPU            0xB1
#define		DO_TYPEB_CPU		    0xB4
#define     FIFO_RCV_LEN            0xB3
#define		WRITE_TYPE		        0xB2
#define		READ_SN					0xB5     //xy



//WRITE type
#define	    W_CARD                  0x01
#define 	W_CHAR			        0x02
#define 	W_CPU                   0x03


#define 	RC531_M1_CSN            0xAB
//TYPEA--------------M1 Command
#define		RC531_MFOUTSELECT       0xA0
#define 	RC531_REQIDL            0xA1
#define 	RC531_REQALL            0xA2
#define 	RC531_ANTICOLL          0xA3
#define 	RC531_SELECT            0xA4
#define 	RC531_AUTHENT           0xA5
#define 	RC531_READ              0xA6

#define 	RC531_WRITE             0xA7
#define 	RC531_INC               0xA8
#define 	RC531_DEC               0xA9

#define 	RC531_HALT              0xAA
#define		RC531_TRANSFER          0xC0
#define		RC531_RESTORE           0xC1


//TYPEA--------------FM1208------------------
#define		TYPEA_CPU_REST		    0xAC
#define		TYPEA_CPU_PPS		    0xAD
//TYPEB--------------SFEN------------------
#define		GET_SFEN_CSN	       0xAE

//RC531--------------POWER-------------------
#define		RF_POWER_OFF		0xD0
#define		RF_POWER_ON		    0xD1
#define     RC531_CHECKCARD     0xAF
#define     RC531_CARDREST      0xB6

//RC663------------15693-----------------
#define     TYPEA_15693_REST        0xB7        //清查卡片返回ATS
#define     TYPEA_15693_WRITE       0xB8        //写块数据
#define     TYPEA_15693_READSIN     0xB9        //读单一块数据
#define     TYPEA_15693_READMUL     0xBA        //读多块数据 
#define     TYPEA_15693_STAYQUIET   0xBB        //保持静默
#define     TYPEA_15693_WRITEAFI    0xBC        //写AFI
#define     TYPEA_15693_LOCKAFI     0xBD        //锁写AFI
#define     TYPEA_15693_WRITEDSFID  0xBE        //写DSFID
#define     TYPEA_15693_LOCKDSFID   0xBF        //锁定DSFID
#define     TYPEA_15693_READNUMS    0xC2        //读取连续的几个块
#define     TYPEA_15693_WRITENUMS   0xC3        //写数据的块号
#define     W_15693 				0xC4        //15693写标志

typedef struct
{
    unsigned char key[6];
    unsigned char mode;
    unsigned char rwbuf[16];
    unsigned char money[4];
} card_buf;

struct mifaredev
{
    int    rcv_len;
    char   rcv_data[1024];
};

tpd_Card g_sCard;						// 射频卡
struct mifaredev  mifare_dev;

#define FSDI 5
#define FSD 64

extern int mifare_open(void);
extern int mifare_release(void);
extern ssize_t mifare_write(const char *buff, size_t size);
extern ssize_t mifare_read(char *buff, size_t size);
extern int mifare_ioctl(unsigned int cmd, unsigned long arg);

#endif
