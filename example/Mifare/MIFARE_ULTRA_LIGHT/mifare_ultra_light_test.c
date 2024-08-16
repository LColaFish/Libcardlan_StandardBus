
#include "card_opt/M1/M1_card_operation.h"
#include "cardlan_devctrl.h"

unsigned char key[6] ;  //keya keyb 算出来不一样
unsigned char KeyDes[8] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88}; //主控密钥
unsigned char SnBack[4] = {0x26,0x91,0x13,0x00};
unsigned char CSN[4];

static int m1_card_test(void)
{
    int ret;
    unsigned char TempBuf[8];
    memcpy(TempBuf, CSN, 4);
    memcpy(TempBuf+4, SnBack, 4);
    DES_CARD(KeyDes, TempBuf, key);

   
    /*m1 card read test*/   
    {
        char write_buf[16];  
        unsigned char SectorNo=3;
        unsigned char BlockNo =0;

        {
            int ret; 
            char receive_buf[20];  
            unsigned long receive_len = 0;  
            unsigned char mode = 0x0B;
            ret = ReadOneSectorDataFromCard(receive_buf,&receive_len, SectorNo, BlockNo, VERIFY_KEY,key,mode);
            if( ret != MI_OK)
            {
                printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
                return -1;
            }
            memcpy(write_buf,receive_buf,receive_len);
        }

        write_buf[0] += 1;   
        unsigned char mode = 0x0B;
        ret = WriteOneSertorDataToCard(write_buf,sizeof(write_buf), SectorNo, BlockNo,VERIFY_KEY,key,mode);
        if(ret != MI_OK)
        {
            printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
            return -2;
        }

        {  
            int ret;
            char receive_buf[1024];  
            unsigned long receive_len = 0; 
            unsigned char mode = 0x0B;
            ret = ReadOne2FiveSectorDataFromCard(receive_buf,&receive_len,key,mode);
            if(ret != MI_OK)
            {
                printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
                return -3;
            }
            printf("-------------ReadOne2FiveSectorDataFromCard:  --------------\n");  
            menu_print(receive_buf, receive_len);       
        }      
    }
    return 0;
}

/* Add For Test */
int main(void)
{
    int ret;
    ret = InitDev();
    if(ret != 0)
    {
        printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }

    char             Buffer[100] = {0};
    unsigned char    len         = 0;
    ret = CardReset(Buffer, &len , 0);
	if(ret != 0x08)
	{
		return -1;
	}
    printf("cardreset:\n");
    menu_print(Buffer, len);
    memcpy(CSN,Buffer,4);
    m1_card_test();
    
    return 0;
}
