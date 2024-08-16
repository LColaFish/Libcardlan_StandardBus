#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 

#include "status.h"
#include "typea.h"
#include "libcardlan_StandardBus_util.h"


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


static char   csn[11];
static char   write_type;

static unsigned char Send[1024] = { 0 };
static unsigned char Recv[1024] = { 0 };
static unsigned int RLen = 0;
static unsigned char MFblook = 0;
static unsigned char CpucardCos = 0;
static unsigned char Block15693R = 0;
static unsigned char Block15693W = 0;

tpd_Card g_sCard;						// 射频�?
static unsigned char  g_cBuffer[256];			// 通信缓存
static unsigned short g_iLength;

static unsigned char  g_cATS[32];
static unsigned char  g_cATSLen;


card_buf keybuf;

int mifare_open(void)
{
    char Status;

    init_spi();

    memset(&mifare_dev, 0x00, sizeof(struct mifaredev));

    Status = PcdReset();

    LOGI("mifare_open status :0x%2X \n", Status);

    if (Status != MI_OK)
    {
        return -1;
    }
    else
    {
        PcdConfig(PCD_TYPE_14443A);
    }

    return 0;
}

int mifare_release(void)
{
    return 0;
}

//Init mifare M1

static int mifare_reset(void)
{
    unsigned char err, u, status/*,i*/;
    int result = 0x00;
    err = 1;
    u = 1;

    while (err)
    {
        switch (u)
        {
        case 1:
        {
            status = PiccRequest(PICC_REQIDL, g_sCard.cATQ);
            if (status == MI_OK)
            {
                u++;
            }
            else
            {
                err = 0;
            }
        }
        break;
        case 2:
        {
            status = PiccAnticollSelect(&g_sCard.cLen, g_sCard.cUID, &g_sCard.cSAK);
            if (status == MI_OK)
            {
                u++;
            }
            else
            {
                err = 0;
            }
        }
        break;

        default:
        {
            mifare_dev.rcv_len = g_sCard.cLen + 1;
            memcpy(mifare_dev.rcv_data, g_sCard.cUID, g_sCard.cLen);
            memcpy(mifare_dev.rcv_data + g_sCard.cLen, &g_sCard.cSAK, 1);
            u = 0;
            err = 0;
        }
        break;
        }
    }
    result = u;
    return result;
}

int mifare_ioctl(unsigned int cmd, unsigned long arg)
{
    int  status = -1;
    unsigned char blockdata[16];
    switch (cmd)
    {
    case DO_TYPEA_M1:
    {
        PcdConfig(PCD_TYPE_14443A);
    }
    break;
    case RC531_M1_CSN:
    {
        status = mifare_reset();
        if (status != MI_OK)
        {
            CpucardCos = 0;
        }
    }
    break;
    case RC531_CHECKCARD:
        break;
    case RC531_CARDREST:
        break;
    case RC531_MFOUTSELECT:
        break;
    case RC531_REQIDL:
    {
        status = PiccRequest(PICC_REQIDL, g_sCard.cATQ);
    }
    break;
    case RC531_REQALL:
    {
        status = PiccRequest(PICC_REQALL, g_sCard.cATQ);
    }
    break;
    case RC531_ANTICOLL:
    {
        //  status = M500PiccAnticoll(0,csn); //csn 为返回的卡序列号SN
    }
    break;
    case RC531_SELECT:
    {
        //  status = M500PiccSelect(csn,catq);//选择这张�?
    }
    break;
    case RC531_AUTHENT:
    {
        if (keybuf.mode == KEYB)
        {
            status = PiccAuthState(arg, 0x61, 0x00, keybuf.key);
        }
        else
        {
            status = PiccAuthState(arg, 0x60, 0x00, keybuf.key);
        }
    }
    break;
    case RC531_READ:
    {
        status = PiccRead(arg, blockdata);
        mifare_dev.rcv_len = 16;
        memcpy(mifare_dev.rcv_data, blockdata, 16);
    }
    break;  //
    case READ_SN:  //xy
    {
        memcpy((char*)arg, blockdata, 16);
    }
    break;  //
    case RC531_WRITE:
    {
        MFblook = (unsigned char)arg;
        status = 0;
    }
    break;  //

    case RC531_INC:
    {
        status = PiccValue(PICC_INCREMENT, arg, keybuf.money);
    }
    break;  //

    case RC531_DEC:
    {
        status = PiccValue(PICC_DECREMENT, arg, keybuf.money);
    }
    break;  //

    case RC531_TRANSFER:
    {
        status = PiccTransfer(arg);
    }
    break;  //

    case RC531_RESTORE:
    {
        status = PiccRestore(arg);
    }
    break;  //

    case RC531_HALT:
    {
        status = PiccHaltA();           //停掉这张�?
    }
    break;


    case TYPEA_CPU_REST:
    {
        RLen = 0;
        status = ISO14443A_Rst((unsigned char *)&RLen, Recv);
        mifare_dev.rcv_len = RLen;
        memcpy(mifare_dev.rcv_data, Recv, RLen);
        if (status == MI_OK)
        {
            CpucardCos = 1;
        }
    }
    break;

    case RF_POWER_OFF:
    {
    }
    break;

    case RF_POWER_ON:
    {
        usleep(10000);
        PcdConfig(PCD_TYPE_14443A);
    }
    break;

    case FIFO_RCV_LEN:
    {
        *(int*)arg = mifare_dev.rcv_len;
        status = 0;
    }
    break;
    case WRITE_TYPE:
    {
        write_type = (char)arg;
        status = 0;
    }
    break;
    /****************************15693*******************/


    case TYPEA_15693_REST:
        {
            status = ISO15693_MultiTagAutoInventory(g_cATS, 0x00, 0x00);
            if (status != ERR_SUCCESS)
            {
                mifare_dev.rcv_len = 0;
                break;
            }
            #if 0
            status |= ISO15693_Select(RC663_15693SELECT, g_cATS + 1);
            if (status != ERR_SUCCESS)
            {
                mifare_dev.rcv_len = 0;
                break;
            }
            #endif
            mifare_dev.rcv_len = 8;
            memcpy(mifare_dev.rcv_data, g_cATS + 1, 8);
        }
        break;

    case  TYPEA_15693_READSIN:
    {
        status = ISO15693_ReadBlock(RC663_15693READ, g_cATS + 1, (unsigned char)arg, 0, &g_cATSLen, g_cBuffer);
        if (status != ERR_SUCCESS)
        {
            mifare_dev.rcv_len = 0;    
            break;
        }
        printf("TYPEA_15693_READSIN \n");
        menu_print(g_cBuffer,g_cATSLen);
        mifare_dev.rcv_len = g_cATSLen;
        memcpy(mifare_dev.rcv_data, g_cBuffer, g_cATSLen);
    }
    break;

    case TYPEA_15693_READNUMS:
        Block15693R = (unsigned char)arg;
        status = 0;
        break;

    case TYPEA_15693_READMUL:
    {
        status = ISO15693_ReadBlock(RC663_15693READ, g_cATS + 1, (unsigned char)arg, Block15693R, &g_cATSLen, g_cBuffer);
        if (status != ERR_SUCCESS)
        {
            mifare_dev.rcv_len = 0;
            break;
        }
        mifare_dev.rcv_len = g_cATSLen;
        memcpy(mifare_dev.rcv_data, g_cBuffer, g_cATSLen);
    }
    break;

    case TYPEA_15693_WRITENUMS:
    {
        Block15693W = (unsigned char)arg;
        status = 0;
    }
    break;

    case TYPEA_15693_WRITEAFI:
    {
        status = ISO15693_WriteAFI(RC663_15693WRITE, g_cATS + 1, (unsigned char)arg);
    }
    break;

    case TYPEA_15693_LOCKAFI:
        status = ISO15693_LockAFI(ISO15693_LOCK_AFI, g_cATS + 1);
        break;

    case TYPEA_15693_WRITEDSFID:
        status = ISO15693_WriteDSFID(RC663_15693WRITE, g_cATS + 1, (unsigned char)arg);

        break;

    case TYPEA_15693_LOCKDSFID:
        status = ISO15693_LockDSFID(ISO15693_LOCK_DSFID, g_cATS + 1);
        break;

    case TYPEA_15693_STAYQUIET:
        {
            status = ISO15693_StayQuiet(RC663_15693Quiet, g_cATS + 1);
            if (status != ERR_SUCCESS)
            {
                break;
            }
        }
        break;

    default:
    {
        status = -1;
    }
    break;
    }
    return status;
}


ssize_t mifare_read(char *buff, size_t size)
{
    if (mifare_dev.rcv_len < size)
    {
        size = mifare_dev.rcv_len;
    }
    memcpy(buff, mifare_dev.rcv_data, size);
    return  size;
}


ssize_t mifare_write(const char *buff, size_t size)
{
    char status;
    ssize_t  result = 1;
    unsigned char blockdata[16];

    switch (write_type)
    {
    case W_CARD:
    {
        memcpy(blockdata, buff, 16);
        status = PiccWrite(MFblook, blockdata);
        if (status == MI_OK)
        {
            result = 0;
        }

    }
    break;

    case W_CHAR:
    {
        memcpy(&keybuf, (card_buf*)buff, sizeof(card_buf));		
        result = 0;
    }
    break;

    case W_CPU:
    {
        memcpy(Send, buff, size);
        RLen = (unsigned int)size;
        status = ISO14443_COS((unsigned short *)&RLen, Send, Recv);
        if (status == MI_OK)
        {
            mifare_dev.rcv_len = RLen;
            memcpy(mifare_dev.rcv_data, Recv, RLen);
            result = 0;
        }
    }
    break;

    case W_15693:
    {
        memcpy(Send, buff, size);
        status = ISO15693_Write(RC663_15693WRITE, g_cATS + 1, Block15693W, Send);
        if (status == MI_OK)
        {
            result = 0;
        }
    }
    break;

    default:
    {
        result = -1;
    }
    break;
    }

    return result;
}


