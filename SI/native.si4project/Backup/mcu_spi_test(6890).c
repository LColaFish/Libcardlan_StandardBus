
#include "mcu_opt/mcu_opt.h"
#include "cardlan_devctrl.h"

unsigned char PsamAtr[6][32];


static void menu_print(char *buffer, int length)
{
	int i;

	for (i = 0; i < length; i++)
	{
		printf("%02X ", *(buffer + i));
		if ((i + 1) % 8 == 0) printf(" ");
		if ((i + 1) % 16 == 0) printf("\n");
	}
	printf("\n");
}

static int mcu_spi_test(char psamnum)
{
    int ret;
    {
        unsigned char recv_data[256];
        unsigned char recv_len[4];
        unsigned char send_len[] = {0,0,0,0};
        ret = mcu_cmd_read_and_write(psamnum,0x01,NULL,send_len,recv_data,recv_len);
		printf("%s %d ret=%d\n",__func__,__LINE__,ret);
        if(ret != 0)
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
    }

    {
        unsigned char recv_data[256];
        unsigned char recv_len[4];
        unsigned char send_len[] = {2,0,0,0};
       // unsigned char send_data[2] = {0x96,0x00};
        unsigned char send_data[2] = {0x25,0x80};
        ret = mcu_cmd_read_and_write(psamnum,0x08,send_data,send_len,recv_data,recv_len);
		printf("%s %d ret=%d\n",__func__,__LINE__,ret);
        if(ret != 0)
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
    }
	
		printf("atr length:%d\n",PsamAtr[psamnum-0x60][1]);
		menu_print(&PsamAtr[psamnum-0x60][1], PsamAtr[psamnum-0x60][1]);
	
	/*冷复位*/
    {
        unsigned char recv_data[256];
        unsigned char recv_len[4];
        unsigned char send_len[] = {0,0,0,0};
        ret = mcu_cmd_read_and_write(psamnum,0x05,NULL,send_len,recv_data,recv_len);
        if(ret != 0)
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return ret;
        }


		 unsigned char buffer_len;
    {
        unsigned char recv_data[256];
        unsigned char recv_len[4];
        unsigned char send_len[] = {0,0,0,0};
        ret = mcu_cmd_read_and_write(psamnum,0x07,NULL,send_len,recv_data,recv_len);
        if(ret != 0)
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
        buffer_len = recv_data[8];
    }
    {
        unsigned char recv_data[256];
        unsigned char recv_len[4];
        unsigned char send_len[] = {1,0,0,0};
        ret = mcu_cmd_read_and_write(psamnum,0x04,&buffer_len,send_len,recv_data,recv_len);
        if(ret != 0)
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
		menu_print(recv_data,recv_len);
    }
	}
    /*发命令*/
    {
        unsigned char recv_data[256];
        unsigned char recv_len[4];
        unsigned char send_len[] = {19,0,0,0}; 
        //unsigned char send_data[5] = {0x00,0xB0,0x96,0x00,0x06};
        unsigned char send_data[32] = {0x00,0xa4,0x00,0x00,0x0f,0xa0,0x00,0x00,0x00,0x22,0x53,0x49,0x4e,0x4f,0x50,0x45,0x43,0x31,0x22};
        ret = mcu_cmd_read_and_write(psamnum,0x03,send_data,send_len,recv_data,recv_len);
        if(ret != 0)
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
    }
    unsigned char buffer_len;
    {
        unsigned char recv_data[256];
        unsigned char recv_len[4];
        unsigned char send_len[] = {0,0,0,0};
        ret = mcu_cmd_read_and_write(psamnum,0x07,NULL,send_len,recv_data,recv_len);
        if(ret != 0)
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
        buffer_len = recv_data[8];
    }
    {
        unsigned char recv_data[256];
        unsigned char recv_len[4];
        unsigned char send_len[] = {1,0,0,0};
        ret = mcu_cmd_read_and_write(psamnum,0x04,&buffer_len,send_len,recv_data,recv_len);
        if(ret != 0)
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
		menu_print(recv_data,recv_len);
    }

    return 0;
}

/* Add For Test */
int main(int argc, char **argv)
{
    int ret;
    ret = InitDev();
	printf("11111111111\n");
    if(ret != 0)
    {
        printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }

	/*				
	ret = InitPsam(3, 38400);
    if(ret != 0)
    {
        printf("[ %s %d ]InitPsam error ret : %d!!\n",__FUNCTION__,__LINE__,ret);
        return 2;  
    }
	*/

	if(*argv[1] == '0')
    {
    	mcu_spi_test(0x60);
	}
	else if(*argv[1] == '3')
    {
    	mcu_spi_test(0x62);
	}    
    return 0;
}
