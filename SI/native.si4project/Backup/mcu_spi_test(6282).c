
#include "mcu_opt/mcu_opt.h"
#include "cardlan_devctrl.h"

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

static int mcu_spi_test(void)
{
    int ret;
    {
        unsigned char recv_data[256];
        unsigned char recv_len[4];
        unsigned char send_len[] = {0,0,0,0};
        ret = mcu_cmd_read_and_write(0x50,0x01,NULL,send_len,recv_data,recv_len);
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
        unsigned char send_data[2] = {0x25,0x80};
        ret = mcu_cmd_read_and_write(0x50,0x08,send_data,send_len,recv_data,recv_len);
        if(ret != 0)
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
    }
    return 0;
    {
        unsigned char recv_data[256];
        unsigned char recv_len[4];
        unsigned char send_len[] = {0,0,0,0};
        ret = mcu_cmd_read_and_write(0x50,0x05,NULL,send_len,recv_data,recv_len);
        if(ret != 0)
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
    }
    
    {
        unsigned char recv_data[256];
        unsigned char recv_len[4];
        unsigned char send_len[] = {5,0,0,0}; 
        unsigned char send_data[5] = {0x00,0xB0,0x96,0x00,0x06};
        ret = mcu_cmd_read_and_write(0x50,0x03,send_data,send_len,recv_data,recv_len);
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
        ret = mcu_cmd_read_and_write(0x50,0x07,NULL,send_len,recv_data,recv_len);
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
        ret = mcu_cmd_read_and_write(0x50,0x04,&buffer_len,send_len,recv_data,recv_len);
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
int main(void)
{
    int ret;
    ret = InitDev();
    if(ret != 0)
    {
        printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }


    mcu_spi_test();
    
    return 0;
}
