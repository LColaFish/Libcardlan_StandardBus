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




static unsigned char CheakIsNeedWriteKey(unsigned char SectorNo, unsigned char VerifyFlag);


static unsigned char  MoneyYes(unsigned char *Date)
{
	unsigned char  status = 1;
	unsigned int ak0,ak1,ak2;
	LongUnon Buf;

	memcpy(Buf.longbuf,Date  ,4);
	ak0 = Buf.i;
	memcpy(Buf.longbuf,Date+4,4);
	ak1 = Buf.i;
	memcpy(Buf.longbuf,Date+8,4);
	ak2 = Buf.i;
	if((ak0 == ak2)&&(ak0 == ~ak1)&&(Date[12] == Date[14])&&(Date[13] == Date[15])&&((Date[12]^Date[13]) == 0xff)&&(ak0 < 0x7fffffff))status = 0;
	return status;
}



static unsigned char CheakIsNeedWriteKey(unsigned char SectorNo, unsigned char VerifyFlag)
{
    unsigned char TempBuf[8], ReturnFlag = 0;
    static unsigned char LastKeyType = 0; 
    
    LOGI("LastKeyType = 0x%02X  SectorNo = 0x%02X\n ",LastKeyType, SectorNo);
    
    if((SectorNo < 8 && LastKeyType == 2) || (SectorNo >= 8 && LastKeyType == 1) || (!LastKeyType) || (VerifyFlag == 1))
    {
        return 0x01;
    }    

	
	
    return 0x01;
}

int ReadOne2FiveSectorDataFromCard(unsigned char *SectorBuf, unsigned long *p_SectorBuf_len,unsigned char *key,unsigned char mode)
{
    char receive_buf[20];  
    unsigned long rec_len = 0;
    *p_SectorBuf_len = 0;
    if(ReadOneSectorDataFromCard(receive_buf, &rec_len, 1, 0, WRITE_KEY,key,mode) != MI_OK)
    {
        return -1;
    }
    memcpy(SectorBuf + *p_SectorBuf_len,receive_buf, rec_len);
    *p_SectorBuf_len += rec_len;
    
    if(ReadOneSectorDataFromCard(receive_buf, &rec_len,  1, 1, READ_ONLY,key,mode) != MI_OK)
    {
        return -2;
    }
    memcpy(SectorBuf + *p_SectorBuf_len,receive_buf, rec_len);
    *p_SectorBuf_len += rec_len;
    
    if(ReadOneSectorDataFromCard(receive_buf, &rec_len, 1, 2, READ_ONLY,key,mode) != MI_OK)
    {
        return -3;
    }
    memcpy(SectorBuf + *p_SectorBuf_len,receive_buf,  rec_len);
    *p_SectorBuf_len += rec_len;
    
    if(ReadOneSectorDataFromCard(receive_buf, &rec_len, 2, 0, VERIFY_KEY,key,mode) != MI_OK)
    {
        return -4;
    }
    memcpy(SectorBuf + *p_SectorBuf_len,receive_buf,  rec_len);
    *p_SectorBuf_len += rec_len;

    if(ReadOneSectorDataFromCard(receive_buf, &rec_len,  2, 1, READ_ONLY,key,mode) != MI_OK)
    {
        return -5;
    }
    memcpy(SectorBuf + *p_SectorBuf_len,receive_buf,  rec_len);
    *p_SectorBuf_len += rec_len;
    
    if(ReadOneSectorDataFromCard(receive_buf, &rec_len,  2, 2, READ_ONLY,key,mode) != MI_OK)
    {
        return -6;
    }
    memcpy(SectorBuf + *p_SectorBuf_len,receive_buf,  rec_len);
    *p_SectorBuf_len += rec_len;


    if(ReadOneSectorDataFromCard(receive_buf, &rec_len,  3, 0, VERIFY_KEY,key,mode) != MI_OK)
    {
        return -7;
    }
    
    memcpy(SectorBuf + *p_SectorBuf_len,receive_buf,  rec_len);
    *p_SectorBuf_len += rec_len;

    if(ReadOneSectorDataFromCard(receive_buf, &rec_len,  3, 1, READ_ONLY,key,mode) != MI_OK)
    {
        return -8;
    }

    memcpy(SectorBuf + *p_SectorBuf_len,receive_buf,  rec_len);
    *p_SectorBuf_len += rec_len;

    if(ReadOneSectorDataFromCard(receive_buf, &rec_len,  3, 2, READ_ONLY,key,mode) != MI_OK)
    {
        return -9;
    }

    memcpy(SectorBuf + *p_SectorBuf_len,receive_buf,  rec_len);
    *p_SectorBuf_len += rec_len;

    if(ReadOneSectorDataFromCard(receive_buf, &rec_len,  4, 0, VERIFY_KEY,key,mode) != MI_OK)
    {
        return -10;
    }
    memcpy(SectorBuf + *p_SectorBuf_len,receive_buf,  rec_len);
    *p_SectorBuf_len += rec_len;

    if(ReadOneSectorDataFromCard(receive_buf, &rec_len,  4, 1, READ_ONLY,key,mode) != MI_OK)
    {
        return -11;
    }
    memcpy(SectorBuf + *p_SectorBuf_len,receive_buf,  rec_len);
    *p_SectorBuf_len += rec_len;

    if(ReadOneSectorDataFromCard(receive_buf, &rec_len,  4, 2, READ_ONLY,key,mode) != MI_OK)
    {
        return -12;
    }
    memcpy(SectorBuf + *p_SectorBuf_len,receive_buf,  rec_len);
    *p_SectorBuf_len += rec_len;

    if(ReadOneSectorDataFromCard(receive_buf, &rec_len,  5, 0, VERIFY_KEY,key,mode) != MI_OK)
    {
        return -13;
    }
    memcpy(SectorBuf + *p_SectorBuf_len,receive_buf,  rec_len);
    *p_SectorBuf_len += rec_len;

    if(ReadOneSectorDataFromCard(receive_buf, &rec_len,  5, 1, READ_ONLY,key,mode) != MI_OK)
    {
        return -14;
    }
    memcpy(SectorBuf + *p_SectorBuf_len,receive_buf,  rec_len);
    *p_SectorBuf_len += rec_len;

    if(ReadOneSectorDataFromCard(receive_buf, &rec_len,  5, 2, READ_ONLY,key,mode) != MI_OK)
    {
        return -15;
    }
    memcpy(SectorBuf + *p_SectorBuf_len,receive_buf, rec_len);
    *p_SectorBuf_len += rec_len;
    return 0;
}


int ReadOneSectorDataFromCard(unsigned char *SectorBuf, unsigned long *p_SectorBuf_len,unsigned char SectorNo, unsigned char BlockNo, 
                                                   unsigned char VerifyFlag, unsigned char *key,unsigned char mode)
{    
    int ret = 0;
    unsigned char Loop = 1, step = 1;
    struct 
    {
            unsigned char key[6];
            unsigned char mode;
            unsigned char rwbuf[16];
            unsigned char money[4];
    }KeyInfo;

    if(SectorBuf == NULL)
    {
        LOGI("SectorBuf is NULL\n");
        return -1;
    }

    /*get step and update keyinfo*/
    {
        switch(VerifyFlag)
        {
            case WRITE_KEY:
                {
                    step = 1;
                }
                break;
            case VERIFY_KEY:
                {
                    step = 2;
                }
                break;
            case 3: /*WRITE_ONLY and READ_ONLY*/
                {
                    step = 3;
                }
                break;
            default:
                {
                    LOGI("VerifyFlag default\n");
                }
                return -2;
        }
        
        memset(&KeyInfo, 0, sizeof(KeyInfo)); 
        if(CheakIsNeedWriteKey(SectorNo, VerifyFlag) != 0x00)
        {
            if(key == NULL)
            {
                return -3;
            }
            memcpy(KeyInfo.key,key,6);
            KeyInfo.mode = mode;
            memset(KeyInfo.rwbuf, 0xff, 16);
            memset(KeyInfo.money, 0x00, 4);
            LOGI("key=%02x %02x %02x %02x %02x %02x\n",KeyInfo.key[0],KeyInfo.key[1],KeyInfo.key[2],\
                                                       KeyInfo.key[3],KeyInfo.key[4],KeyInfo.key[5]);
            LOGI("keymode=%02x\n",KeyInfo.mode);
            step = 1;
        }
    }
    
    LOGI("SectorNo = 0x%02X BlockNo = 0x%02X\n",SectorNo, BlockNo);  
    
    while(Loop)
    {
        switch(step)
        {
        case 1:
            {
                ret = mifare_ioctl(WRITE_TYPE, W_CHAR);

                ret = mifare_write(&KeyInfo, sizeof(card_buf));
                if(ret != MI_OK)
                {
                    LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
                    Loop = 0;
                    break;
                }
                step++;
            }
            break;
        case 2:
            {
                ret = mifare_ioctl(RC531_AUTHENT, (4*SectorNo + 3));
                if(ret != MI_OK)
                {
                    LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
                    Loop = 0;
                    break;
                }
                step++;
            }
            break;

        case 3:
            {
  
                ret = mifare_ioctl(RC531_READ,(4*SectorNo + BlockNo));
                if(ret != MI_OK)
                {
                    LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
                    Loop = 0;
                    break;
                }
                
                {
                    unsigned long ReadLen = 0;  
                    ret =  mifare_ioctl(FIFO_RCV_LEN, &ReadLen);
                    if(ret != MI_OK)
                    {
                        LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
                        Loop = 0;
                        break;
                    }
                    
                    {
                        unsigned char ReadBuf[16];
         
                        memset(ReadBuf, 0, sizeof(ReadBuf));
                        ret = mifare_read( ReadBuf, ReadLen);
                        if(ret < 0)
                        {
                            LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
                            Loop = 0;
                            break;
                        }
                     
                        if(ReadBuf == NULL)
                        {
                            LOGI("read error\n");
                            Loop = 0;
                            break;
                        }
                        
                        memcpy(SectorBuf, ReadBuf, ReadLen);
                        *p_SectorBuf_len = ReadLen;
                    }
                }
                step++;   
            }
            break;
            
        default:
            {
                step = 0;
                Loop = 0;
            }
            break;
        }
    }


    if(step)
    {
        LOGI("ReadOneSectorDataFromCard [%02X %02X] step = 0x%02X\n", SectorNo, BlockNo, step);
        return step;
    }

    return 0;
}

int WriteOneSertorDataToCard(const unsigned char *SectorBuf, unsigned long SectorBuf_len,unsigned char SectorNo, unsigned char BlockNo, unsigned char VerifyFlag,unsigned char *key,unsigned char mode)
{
    unsigned char Loop = 1, step = 1, ReturnValue = 0;
    card_buf KeyInfo;

    /*get step and update keyinfo*/
    {
        switch(VerifyFlag)
        {
            case WRITE_KEY:
                {
                    step = 1;
                }
                break;
            case VERIFY_KEY:
                {
                    step = 2;
                }
                break;
            case 3: /*WRITE_ONLY and READ_ONLY*/
                {
                    step = 3;
                }
                break;
            default:
                {
                    LOGI("VerifyFlag default\n");
                }
                return -2;
        }

        memset(&KeyInfo, 0, sizeof(card_buf)); 
        if(CheakIsNeedWriteKey(SectorNo, VerifyFlag) != 0x00)
        {
            if(key == NULL)
            {
                return -3;
            }
            memcpy(KeyInfo.key,key,6);
            KeyInfo.mode = mode;
            memset(KeyInfo.rwbuf, 0xff, 16);
            memset(KeyInfo.money, 0x00, 4);
            LOGI("key=%02x %02x %02x %02x %02x %02x\n",KeyInfo.key[0],KeyInfo.key[1],KeyInfo.key[2],\
                                                       KeyInfo.key[3],KeyInfo.key[4],KeyInfo.key[5]);
            LOGI("keymode=%02x\n",KeyInfo.mode);
            step = 1;
        }
    }
    
    LOGI("SectorNo = 0x%02X BlockNo = 0x%02X\n",SectorNo, BlockNo);  
    
    while(Loop)
    {
        switch(step)
        {
            case 1:
                {
                    mifare_ioctl(WRITE_TYPE, W_CHAR);
                    
                    if(mifare_write(&KeyInfo, sizeof(card_buf)) == 0)
                    {
                        step++;
                    }
                    else 
                    {
                        Loop = 0;
                    }
                }
                break;
                
            case 2:
                {
                    if(mifare_ioctl(RC531_AUTHENT, (4 * SectorNo + 3)) == 0)
                    {
                        step++;
                    }
                    else 
                    {
                        Loop = 0;
                    }
                }
                break;
      
            case 3:
                {
                    if(mifare_ioctl(WRITE_TYPE,W_CARD) == 0)
                    {
                        step++;
                    }
                    else
                    {
                        Loop = 0;
                    }
                }
                break;
                
            case 4:
                {
                    if(mifare_ioctl(RC531_WRITE, (4*SectorNo + BlockNo)) == 0)
                    {
                        if((mifare_write(SectorBuf, SectorBuf_len)) == 0)
                        {
                            step++;
                        }
                        else 
                        {
                            Loop = 0;
                        }
                    }
                    else 
                    {
                        Loop = 0;
                    }
                }
                break;
            default:
                step = 0;
                Loop = 0;
        }
    }


    if(step)
    {
        LOGI("WriteOneSertorDataToCard [%02X %02X] step = 0x%02X\n", SectorNo, BlockNo, step);
        return step;
    }

    return 0;
}


int changOneBlockToMoneyBlock(unsigned char SectorNo, unsigned char BlockNo, unsigned char VerifyFlag,unsigned char *key,unsigned char mode)
{
    unsigned char Loop = 1, step = 1, ReturnValue = 0;
    card_buf KeyInfo;
	int ret;
	char SectorBuf[16] = {0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0xFE,0x01,0xFE,0x01};

    /*get step and update keyinfo*/
    {
        switch(VerifyFlag)
        {
            case WRITE_KEY:
                {
                    step = 1;
                }
                break;
            case VERIFY_KEY:
                {
                    step = 2;
                }
                break;
            case 3: /*WRITE_ONLY and READ_ONLY*/
                {
                    step = 3;
                }
                break;
            default:
                {
                    LOGI("VerifyFlag default\n");
                }
                return -2;
        }

        memset(&KeyInfo, 0, sizeof(card_buf)); 
        if(CheakIsNeedWriteKey(SectorNo, VerifyFlag) != 0x00)
        {
            if(key == NULL)
            {
                return -3;
            }
            memcpy(KeyInfo.key,key,6);
            KeyInfo.mode = mode;
            memset(KeyInfo.rwbuf, 0xff, 16);
            memset(KeyInfo.money, 0x00, 4);
            LOGI("key=%02x %02x %02x %02x %02x %02x\n",KeyInfo.key[0],KeyInfo.key[1],KeyInfo.key[2],\
                                                       KeyInfo.key[3],KeyInfo.key[4],KeyInfo.key[5]);
            LOGI("keymode=%02x\n",KeyInfo.mode);
            step = 1;
        }
    }
    
    LOGI("SectorNo = 0x%02X BlockNo = 0x%02X\n",SectorNo, BlockNo);  
    
    while(Loop)
    {
        switch(step)
        {
            case 1:
                {
                    mifare_ioctl(WRITE_TYPE, W_CHAR);
                    
                    if(mifare_write(&KeyInfo, sizeof(card_buf)) == 0)
                    {
                        step++;
                    }
                    else 
                    {
                        Loop = 0;
                    }
                }
                break;
                
            case 2:
                {
                    if(mifare_ioctl(RC531_AUTHENT, (4 * SectorNo + 3)) == 0)
                    {
                        step++;
                    }
                    else 
                    {
                        Loop = 0;
                    }
                }
                break;

			case 3:
				{
  
                ret = mifare_ioctl(RC531_READ,(4*SectorNo + BlockNo));
                if(ret != MI_OK)
                {
                    LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
                    Loop = 0;
                    break;
                }
                
                {
                    unsigned long ReadLen = 0;  
                    ret =  mifare_ioctl(FIFO_RCV_LEN, &ReadLen);
                    if(ret != MI_OK)
                    {
                        LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
                        Loop = 0;
                        break;
                    }
                    
                    {
                        unsigned char ReadBuf[16];
         
                        memset(ReadBuf, 0, sizeof(ReadBuf));
                        ret = mifare_read( ReadBuf, ReadLen);
                        if(ret < 0)
                        {
                            LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
                            Loop = 0;
                            break;
                        }
                     
                        if(ReadBuf == NULL)
                        {
                            LOGI("read error\n");
                            Loop = 0;
                            break;
                        }
						LOGI("key=%02x %02x %02x %02x\n",ReadBuf[0],ReadBuf[1],ReadBuf[2],ReadBuf[3]);
						
						ret = MoneyYes(ReadBuf);
						if(ret == 0)
						{
							step = 0;
                			Loop = 0;
						}
                       
                    }
                }
                step++;   
            }

			break;	
				
            case 4:
                {
                    if(mifare_ioctl(WRITE_TYPE,W_CARD) == 0)
                    {
                        step++;
                    }
                    else
                    {
                        Loop = 0;
                    }
                }
                break;
                
            case 5:
                {
                    if(mifare_ioctl(RC531_WRITE, (4*SectorNo + BlockNo)) == 0)
                    {
                        if((mifare_write(SectorBuf, 16)) == 0)
                        {
                            step++;
                        }
                        else 
                        {
                            Loop = 0;
                        }
                    }
                    else 
                    {
                        Loop = 0;
                    }
                }
                break;
            default:
                step = 0;
                Loop = 0;
        }
    }


    if(step)
    {
        LOGI("ChangOneBlocktoMoneyBlock [%02X %02X] step = 0x%02X\n", SectorNo, BlockNo, step);
        return step;
    }

    return 0;
}

int incMoney(int Money,unsigned char SectorNo, unsigned char BlockNo, unsigned char VerifyFlag,unsigned char *key,unsigned char mode)
{
    unsigned char Loop = 1, step = 1, ReturnValue = 0;
    card_buf KeyInfo;
	int ret;
	LongUnon value;
	value.i = Money;
	
	
    /*get step and update keyinfo*/
    {
        switch(VerifyFlag)
        {
            case WRITE_KEY:
                {
                    step = 1;
                }
                break;
            case VERIFY_KEY:
                {
                    step = 2;
                }
                break;
            case 3: /*WRITE_ONLY and READ_ONLY*/
                {
                    step = 3;
                }
                break;
            default:
                {
                    LOGI("VerifyFlag default\n");
                }
                return -2;
        }

        memset(&KeyInfo, 0, sizeof(card_buf)); 
        if(CheakIsNeedWriteKey(SectorNo, VerifyFlag) != 0x00)
        {
            if(key == NULL)
            {
                return -3;
            }
			
            memcpy(KeyInfo.key,key,6);
            KeyInfo.mode = mode;
            memset(KeyInfo.rwbuf, 0xff, 16);

			memcpy(KeyInfo.money, value.longbuf, 4);
			
            LOGI("key=%02x %02x %02x %02x %02x %02x\n",KeyInfo.key[0],KeyInfo.key[1],KeyInfo.key[2],\
                                                       KeyInfo.key[3],KeyInfo.key[4],KeyInfo.key[5]);
            LOGI("keymode=%02x\n",KeyInfo.mode);
			LOGI("keymoney=%02x %02x %02x %02x\n",KeyInfo.money[0],KeyInfo.money[1],KeyInfo.money[2],\
                                                       KeyInfo.money[3]);
            step = 1;
        }
    }
    
    LOGI("SectorNo = 0x%02X BlockNo = 0x%02X\n",SectorNo, BlockNo);  
    
    while(Loop)
    {
        switch(step)
        {
            case 1:
                {
                    mifare_ioctl(WRITE_TYPE, W_CHAR);
                    
                    if(mifare_write(&KeyInfo, sizeof(card_buf)) == 0)
                    {
                        step++;
                    }
                    else 
                    {
                        Loop = 0;
                    }
                }
                break;
                
            case 2:
                {
                    if(mifare_ioctl(RC531_AUTHENT, (4 * SectorNo + 3)) == 0)
                    {
                        step++;
                    }
                    else 
                    {
                        Loop = 0;
                    }
                }
                break;
			#if 0
			case 3:
				{
  
                ret = mifare_ioctl(RC531_READ,(4*SectorNo + BlockNo));
                if(ret != MI_OK)
                {
                    LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
                    Loop = 0;
                    break;
                }
                
                {
                    unsigned long ReadLen = 0;  
                    ret =  mifare_ioctl(FIFO_RCV_LEN, &ReadLen);
                    if(ret != MI_OK)
                    {
                        LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
                        Loop = 0;
                        break;
                    }
                    
                    {
                        unsigned char ReadBuf[16];
         
                        memset(ReadBuf, 0, sizeof(ReadBuf));
                        ret = mifare_read( ReadBuf, ReadLen);
                        if(ret < 0)
                        {
                            LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
                            Loop = 0;
                            break;
                        }
                     
                        if(ReadBuf == NULL)
                        {
                            LOGI("read error\n");
                            Loop = 0;
                            break;
                        }
						LOGI("ReadBuf=%02x %02x %02x %02x\n",ReadBuf[0],ReadBuf[1],ReadBuf[2],ReadBuf[3]);
						
						ret = MoneyYes(ReadBuf);
						if(ret != 0)
						{							
                			Loop = 0;
						}
                       
                    }
                }
                step++;   
            }

			break;
			
			
				
            case 4:
            #else
			case 3:
            #endif
                {
                    if(mifare_ioctl(RC531_INC,(4*SectorNo)+BlockNo) == 0)
                    {
                        step++;
                    }
                    else
                    {
                        Loop = 0;
                    }
                }
                break;                

            default:
                step = 0;
                Loop = 0;
        }
    }


    if(step)
    {
        LOGI("incMoney [%02X %02X] step = 0x%02X\n", SectorNo, BlockNo, step);
        return step;
    }

    return 0;
}


int decMoney(int Money,unsigned char SectorNo, unsigned char BlockNo, unsigned char VerifyFlag,unsigned char *key,unsigned char mode)
{
    unsigned char Loop = 1, step = 1, ReturnValue = 0;
    card_buf KeyInfo;
	int ret;
	LongUnon value;
	value.i = Money;
	
	
    /*get step and update keyinfo*/
    {
        switch(VerifyFlag)
        {
            case WRITE_KEY:
                {
                    step = 1;
                }
                break;
            case VERIFY_KEY:
                {
                    step = 2;
                }
                break;
            case 3: /*WRITE_ONLY and READ_ONLY*/
                {
                    step = 3;
                }
                break;
            default:
                {
                    LOGI("VerifyFlag default\n");
                }
                return -2;
        }

        memset(&KeyInfo, 0, sizeof(card_buf)); 
        if(CheakIsNeedWriteKey(SectorNo, VerifyFlag) != 0x00)
        {
            if(key == NULL)
            {
                return -3;
            }
            memcpy(KeyInfo.key,key,6);
            KeyInfo.mode = mode;
            memset(KeyInfo.rwbuf, 0xff, 16);

			memcpy(KeyInfo.money, value.longbuf, 4);
			
            LOGI("key=%02x %02x %02x %02x %02x %02x\n",KeyInfo.key[0],KeyInfo.key[1],KeyInfo.key[2],\
                                                       KeyInfo.key[3],KeyInfo.key[4],KeyInfo.key[5]);
            LOGI("keymode=%02x\n",KeyInfo.mode);
            step = 1;
        }
    }
    
    LOGI("SectorNo = 0x%02X BlockNo = 0x%02X\n",SectorNo, BlockNo);  
    
    while(Loop)
    {
        switch(step)
        {
            case 1:
                {
                    mifare_ioctl(WRITE_TYPE, W_CHAR);
                    
                    if(mifare_write(&KeyInfo, sizeof(card_buf)) == 0)
                    {
                        step++;
                    }
                    else 
                    {
                        Loop = 0;
                    }
                }
                break;
                
            case 2:
                {
                    if(mifare_ioctl(RC531_AUTHENT, (4 * SectorNo + 3)) == 0)
                    {
                        step++;
                    }
                    else 
                    {
                        Loop = 0;
                    }
                }
                break;
			/*
			case 3:
				{
  
                ret = mifare_ioctl(RC531_READ,(4*SectorNo + BlockNo));
                if(ret != MI_OK)
                {
                    LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
                    Loop = 0;
                    break;
                }
                
                {
                    unsigned long ReadLen = 0;  
                    ret =  mifare_ioctl(FIFO_RCV_LEN, &ReadLen);
                    if(ret != MI_OK)
                    {
                        LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
                        Loop = 0;
                        break;
                    }
                    
                    {
                        unsigned char ReadBuf[16];
         
                        memset(ReadBuf, 0, sizeof(ReadBuf));
                        ret = mifare_read( ReadBuf, ReadLen);
                        if(ret < 0)
                        {
                            LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
                            Loop = 0;
                            break;
                        }
                     
                        if(ReadBuf == NULL)
                        {
                            LOGI("read error\n");
                            Loop = 0;
                            break;
                        }
						LOGI("key=%02x %02x %02x %02x\n",ReadBuf[0],ReadBuf[1],ReadBuf[2],ReadBuf[3]);
						
						ret = MoneyYes(ReadBuf);
						if(ret != 0)
						{							
                			Loop = 0;
						}
                       
                    }
                }
                step++;   
            }

			break;

			
				
            case 4:
            */
            case 3:
                {
                    if(mifare_ioctl(RC531_DEC,(4 * SectorNo)+BlockNo) == 0)
                    {
                        step++;
                    }
                    else
                    {
                        Loop = 0;
                    }
                }
                break;                

            default:
                step = 0;
                Loop = 0;
        }
    }


    if(step)
    {
        LOGI("ChangOneBlocktoMoneyBlock [%02X %02X] step = 0x%02X\n", SectorNo, BlockNo, step);
        return step;
    }

    return 0;
}


int restoreAndTransfer(unsigned char SectorNo, unsigned char restoreBlockNo, unsigned char transferBlockNo,unsigned char *key,unsigned char mode)
{
    unsigned char Loop = 1, step = 1, ReturnValue = 0;
    card_buf KeyInfo;
	int ret;
	LongUnon value;
	value.i = 0;
				
    memcpy(KeyInfo.key,key,6);
    KeyInfo.mode = mode;
    memset(KeyInfo.rwbuf, 0xff, 16);

	memcpy(KeyInfo.money, value.longbuf, 4);
	
    LOGI("key=%02x %02x %02x %02x %02x %02x\n",KeyInfo.key[0],KeyInfo.key[1],KeyInfo.key[2],\
                                               KeyInfo.key[3],KeyInfo.key[4],KeyInfo.key[5]);
    LOGI("keymode=%02x\n",KeyInfo.mode);
	LOGI("keymoney=%02x %02x %02x %02x\n",KeyInfo.money[0],KeyInfo.money[1],KeyInfo.money[2],\
                                               KeyInfo.money[3]);
    step = 1;    
    
    LOGI("SectorNo = 0x%02X restoreBlockNo = 0x%02X ,transferBlockNo = 0x%02x\n",SectorNo, restoreBlockNo,transferBlockNo);  
    
    while(Loop)
    {
        switch(step)
        {
            case 1:
                {
                    mifare_ioctl(WRITE_TYPE, W_CHAR);
                    
                    if(mifare_write(&KeyInfo, sizeof(card_buf)) == 0)
                    {
                        step++;
                    }
                    else 
                    {
                        Loop = 0;
                    }
                }
                break;
                
            case 2:
                {
                    if(mifare_ioctl(RC531_AUTHENT, (4 * SectorNo + 3)) == 0)
                    {
                        step++;
                    }
                    else 
                    {
                        Loop = 0;
                    }
                }
                break;		
           
			case 3:
           
                {
                    if(mifare_ioctl(RC531_RESTORE,(4*SectorNo)+restoreBlockNo) == 0)
                    {
                        step++;
                    }
                    else
                    {
                        Loop = 0;
                    }
                }
                break;  

		   case 4:
				 {
                    if(mifare_ioctl(RC531_TRANSFER,(4*SectorNo)+transferBlockNo) == 0)
                    {
                        step++;
                    }
                    else
                    {
                        Loop = 0;
                    }
                }
				
		   break;
				

            default:
                step = 0;
                Loop = 0;
        }
    }


    if(step)
    {
        LOGI("incMoney [%02X %02X %02x] step = 0x%02X\n", SectorNo, restoreBlockNo, transferBlockNo,step);
        return step;
    }

    return 0;
}





int Mifare_classic_loadKey(unsigned char key[6],unsigned char type)
{
    card_buf KeyInfo;
    memcpy(KeyInfo.key,key,6);
    KeyInfo.mode = type;
    memset(KeyInfo.rwbuf, 0xff, 16);
    memset(KeyInfo.money, 0x00, 4);
    LOGI("key=%02X %02X %02X %02X %02X %02X\n",KeyInfo.key[0],KeyInfo.key[1],KeyInfo.key[2],\
                                               KeyInfo.key[3],KeyInfo.key[4],KeyInfo.key[5]);
    LOGI("keymode=%02X\n",KeyInfo.mode);
    mifare_ioctl(WRITE_TYPE, W_CHAR);
    return mifare_write(&KeyInfo, sizeof(card_buf));
}
int Mifare_classic_authenticate(unsigned char SectorNo, unsigned char *authkey,unsigned char type,unsigned char UID[4])
{
    unsigned char keyAB ;

    if((4*SectorNo + 3  >= 64) || (4*SectorNo + 3  < 0))
    {
        LOGI("Error SectorNo \n");
        return HANDLE_ERR_ARG_INVAL;
    }

    switch(type)
    {
        case KEYA:
            {
                keyAB = 0x60;
            }
            break;
        case KEYB:
            {
                 keyAB = 0x61;
            }
            break;
        default :
            return HANDLE_ERR_ARG_INVAL;
            break; 
    }
    
    return PiccAuthState_ex(4 * SectorNo + 3, keyAB, UID,authkey);
}
int Mifare_classic_read_ex(unsigned short Block,unsigned char Numblock,unsigned char *readdata,int *readlen)
{
    int ret = 0;
    int i = 0;
    if(readdata == NULL)
    {
        return HANDLE_ERR_ARG_INVAL;
    }
    if(Numblock > 31)
    {
        return HANDLE_ERR_OVER_LIMIT;
    }
    if(readlen != NULL)
    {
        *readlen = 0;
    }
    for(i = 0 ; i < Numblock ; i++)
    {
        ret = mifare_ioctl(RC531_READ,Block + i);
        if(ret != HANDLE_OK)
        {
            LOGI("[%s %d] block %d error  ret : %d \n",__FUNCTION__,__LINE__,Block+ i,ret);
            memset(readdata + i * 16, 0, 16);
            ret |= 1 << i;
            continue;
        }
          
        {
            int ret2;
            unsigned char ReadBuf[16];
            unsigned long ReadLen = 0;  
            
            ret2 =  mifare_ioctl(FIFO_RCV_LEN, &ReadLen);
            if(ret2 != MI_OK)
            {
                LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret2);
                ret |= 1 << i;
                continue;
            }

            memset(ReadBuf, 0, sizeof(ReadBuf));
            ret2 = mifare_read( ReadBuf, ReadLen);
            if(ret2 < 0 || ReadBuf == NULL)
            {
                LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret2);
                ret |= 1 << i;
                continue;
            }
            
            memcpy(readdata + i * 16, ReadBuf, ReadLen);
            if(readlen != NULL)
            {
                *readlen += ReadLen;
            }
        }
    }
    return ret;    
}
int Mifare_classic_write_ex(unsigned short Block,unsigned char Numblock,unsigned char *writebuf,int len)
{
    int ret = 0;
    int i = 0;
    
    if(Numblock * 16 != len)
    {
        return HANDLE_ERR_ARG_INVAL;
    }
    
    ret = mifare_ioctl(WRITE_TYPE,W_CARD);
    if(ret != HANDLE_OK)
    {
        return HANDLE_ERR;
    }

    for(i < 0 ; i < Numblock ; i++)
    {
        /*这里应该判断如果是 控制块 不应该写*/
        {
            if((Block + i + 1) % 4  == 0 )
            {
                LOGI("[%s %d] write block  %d error \n",__FUNCTION__,__LINE__,Block);
                continue;
            }
        }
        ret = mifare_ioctl(RC531_WRITE,Block + i);
        if(ret != HANDLE_OK)
        {
            LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
            return HANDLE_ERR;
        }
        unsigned char writedata[16];
        memcpy(writedata,writebuf + 16 * i,16);
        ret = mifare_write(writedata, 16);
        if(ret != HANDLE_OK)
        {
            LOGI("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
            return HANDLE_ERR;
        }
    }
    return HANDLE_OK;  

}

int Mifare_classic_read(unsigned char SectorNo, unsigned char BlockNo, unsigned char *key,unsigned char type, unsigned char UID[4],unsigned char SectorBuf[16])
{    
    int ret = 0;
    int read_len = 0;
    if((4*SectorNo + BlockNo  >= 64) || (4*SectorNo + BlockNo  < 0))
    {
        LOGI("Error block number\n");
        return HANDLE_ERR_ARG_INVAL;
    }
    if((type != KEYA) && (type != KEYB))
    {
        LOGI("Error key type\n");
        return HANDLE_ERR_ARG_INVAL;
    }
    
    ret = Mifare_classic_loadKey(key,type);
    if(ret != HANDLE_OK)
    {
        LOGI("Mifare_classic_loadKey error\n");
        return ret;
    }
    ret = Mifare_classic_authenticate(SectorNo,key,type,UID);
    if(ret != HANDLE_OK)
    {
        LOGI("Mifare_classic_authenticate error\n");
        return ret;
    }  
    ret = Mifare_classic_read_ex(4*SectorNo + BlockNo,1,SectorBuf,&read_len);
    if(ret != HANDLE_OK || read_len == 0)
    {
        return HANDLE_ERR;
    }

    return HANDLE_OK;
}
int Mifare_classic_write(unsigned char SectorNo, unsigned char BlockNo, unsigned char *key,unsigned char type, unsigned char UID[4],unsigned char SectorBuf[16])
{
    int ret;
    if((4 * SectorNo + BlockNo  >= 64 ) || (4*SectorNo + BlockNo  < 0))
    {
        LOGI("Error block number\n");
        return -1;
    }

    if((type != KEYA) && (type != KEYB))
    {
        LOGI("Error key type\n");
        return HANDLE_ERR_ARG_INVAL;
    }
    ret = Mifare_classic_loadKey(key,type);
    if(ret != HANDLE_OK)
    {
        LOGI("Mifare_classic_loadKey error\n");
        return ret;
    }

    ret = Mifare_classic_authenticate(SectorNo,key,type,UID);
    if(ret != HANDLE_OK)
    {
        LOGI("Mifare_classic_authenticate error\n");
        return ret;
    }  

    return Mifare_classic_write_ex(4*SectorNo + BlockNo,1,SectorBuf,16);
}


