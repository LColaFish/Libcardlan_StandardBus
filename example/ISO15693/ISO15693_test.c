#include "ISO15693_operation.h"


int ISO15693_test(void)
{
    char             Buffer[100] = {0};
    int    len         = 0;
    unsigned char    status      = 0;
    int ret;
    unsigned char block = 0;

    status = ISO15693_Opt_CardReset(Buffer, &len);
    if(status != 0)
    {
        printf("[%s %d]  status:%d len:%d!!\n",__FUNCTION__,__LINE__,status,len);
        return -1;
    }
    printf("cardreset:\n");
    menu_print(Buffer, len);
    
    if(1)
    {
        printf("------------- ISO15693_Opt_WriteAFI test -------------\n");
        ret = ISO15693_Opt_WriteAFI(0x01);
        if(ret != 0)
        {
            printf("[%s %d] ret:%d!!\n",__FUNCTION__,__LINE__,ret); 
            return -1;
        }
    }

    if(0) 
    {
        ret = ISO15693_Opt_LockAFI();
        if(ret != 0)
        {
            printf("[%s %d] ret:%d!!\n",__FUNCTION__,__LINE__,ret); 
            return -1;
        }
    } 

    if(1)    
    {
        printf("------------- ISO15693_Opt_WriteDSFID test -------------\n");
        ret = ISO15693_Opt_WriteDSFID(0x01);
        if(ret != 0)
        {
            printf("[%s %d] ret:%d!!\n",__FUNCTION__,__LINE__,ret); 
            return -1;
        }
    }
     
    if(0)     
    {
        ret = ISO15693_Opt_LockDSFID();
        if(ret != 0)
        {
            printf("[%s %d] ret:%d!!\n",__FUNCTION__,__LINE__,ret); 
            return -1;
        }
    }
     

    
    if(1)
    {   
        printf("------------- ISO15693_Opt_WriteBlockNum test -------------\n");
        unsigned char  Send[] = {0x01,0x02,0x03,0x04};
        ret = ISO15693_Opt_WriteBlockNum(block);
        if(ret != 0)
        {
            printf("[%s %d] ret:%d!!\n",__FUNCTION__,__LINE__,ret); 
            return -1;
        }
        ret = ISO15693_Opt_WriteData(Send,4);
        if(ret != 0)
        {
            printf("[%s %d] ret:%d!!\n",__FUNCTION__,__LINE__,ret); 
            return -1;
        }
    }
    if(1)
    {
        printf("------------- ISO15693_Opt_read_signal_block test -------------\n");
        len = ISO15693_Opt_read_signal_block(block,Buffer);
        if(len <= 0)
        {
            printf("[%s %d]  ISO15693_Opt_read_signal_block len:%d!!\n",__FUNCTION__,__LINE__,len);
            return -1;
        }
        printf("[%s %d]  read len len:%d!!\n",__FUNCTION__,__LINE__,len);
        menu_print(Buffer, len);
    }
    if(1)
    {
        printf("------------- ISO15693_Opt_Read_Block_Nums test -------------\n");
        ret = ISO15693_Opt_Read_Block_Nums(4);
        if(ret != 0)
        {
            printf("[%s %d] ret:%d!!\n",__FUNCTION__,__LINE__,ret); 
            return -1;
        }

        len = ISO15693_Opt_Read_Multiple(block,Buffer);
        if(len <= 0)
        {
            printf("[%s %d]  ISO15693_Opt_read_signal_block len:%d!!\n",__FUNCTION__,__LINE__,len);
            return -1;
        }
        printf("[%s %d]  read len len:%d!!\n",__FUNCTION__,__LINE__,len);
        menu_print(Buffer, len);          
    }
    if(1)
    {
        printf("------------- ISO15693_Opt_halt test -------------\n");
        ISO15693_Opt_halt();
        status = ISO15693_Opt_CardReset(Buffer, &len);
        if(status != 0)
        {
            printf("[%s %d]  status:%d len:%d!!\n",__FUNCTION__,__LINE__,status,len);
            return -1;
        }
        menu_print(Buffer, len);
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

    ISO15693_test();
    
    return 0;
}