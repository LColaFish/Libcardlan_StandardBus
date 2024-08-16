
#include "libcardlan_StandardBus_util.h"
#include "psam_opt.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#if defined(ANDROID_CODE_DEBUG)
//#define TARGET_ANDROID
#elif defined(NDK_CODE_DEBUG)
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

#define IRQ "/sys/class/gpio/gpio59/value"

unsigned char PsamNum[6] = {0};
unsigned char PsamKeyIndex = 0; 
unsigned char PsamAtr[6][32] = {0};


int InitPsam(unsigned char PsamNum, int BaudRate)
{
    char psam_num_temp   = 0;
    int  ret ;
    
    union
    {
        unsigned char longbuf[4];
        unsigned int  i;
    } baud;

    /* Psam Num 1-3(0x50-0x52) | Psam Num 4-6(0x60-0x62)*/
    switch(PsamNum)
    {
        case 0:
        case 1:
        case 2:
            {
                psam_num_temp = 0x50 + PsamNum;
            }
            break;

        case 3:
        case 4:
        case 5:
            {
                psam_num_temp = 0x60 + (PsamNum-3);
            }
            break;

        default:
        break;
    }
	printf("psam_num_temp=%d\n",psam_num_temp);
	
    /* Open */
    {
        unsigned char recv_data[256];
        unsigned char recv_len[4];
        unsigned char send_len[] = {0,0,0,0};
        ret = mcu_cmd_read_and_write(psam_num_temp,0x01,NULL,send_len,recv_data,recv_len);
        if(ret != 0)
        {
            return -1;
        }
    }
    
    /* Set BaudRate */
    {
        unsigned char recv_data[256];
        unsigned char recv_len[4];
        baud.i = BaudRate;
        unsigned char send_data[2];
     //   send_data [0] = baud.longbuf[1];
     //   send_data [1] = baud.longbuf[0];
	    send_data [1] = baud.longbuf[1];
        send_data [0] = baud.longbuf[0];
     
	 	unsigned char send_len[] = {2,0,0,0};
        ret = mcu_cmd_read_and_write(psam_num_temp,0x08,send_data,send_len,recv_data,recv_len);
        if(ret != 0)
        {
            return -1;
        }
    }
    
    /* Cold Reset */
    {
    	memset(&PsamAtr[PsamNum][0],0,32);
        unsigned char recv_data[256];
        unsigned char recv_len[4];
        unsigned char send_len[] = {0,0,0,0};
        ret = mcu_cmd_read_and_write(psam_num_temp,0x05,NULL,send_len,recv_data,recv_len);
        if(ret != 0)
        {
            return -1;
        }
		
		unsigned char buffer_len;	

		{
        unsigned char recv_data[256];
        unsigned char recv_len[4];
        unsigned char send_len[] = {0,0,0,0};
        ret = mcu_cmd_read_and_write(psam_num_temp,0x07,NULL,send_len,recv_data,recv_len);
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
        ret = mcu_cmd_read_and_write(psam_num_temp,0x04,&buffer_len,send_len,recv_data,recv_len);
        if(ret != 0)
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
		menu_print(recv_data,recv_len);
    }
		
		PsamAtr[PsamNum][0] = buffer_len;
		memcpy(&PsamAtr[PsamNum][1],&recv_data[8],buffer_len);
		menu_print(&PsamAtr[PsamNum][1],PsamAtr[PsamNum][0]);
    }
    return 0;
}

int GetPsamID(void)    
{
    return GetPsamID_ex(0,PsamNum,&PsamKeyIndex);
}

int PsamCmd(unsigned char PsamNum,unsigned char *sendcmd,int sendlen,unsigned char *rcvcmd,int *rcvlen)
{
	int ret = 0;
	unsigned char len[4];
	unsigned char buff[256]={0};
	unsigned char getResp[]={0x00,0xc0,0x00,0x00,0x00};
	
	ret = Psam_Cmd_Send(PsamNum,sendcmd,sendlen,buff,len);	
	#if 0
	if(ret != 0 || buff[len[0]-2] != 0x90 || buff[len[0]-1] != 0x00)
	{
	    LOGI("[ %s %d ] error step = %d  sw1 = 0x%2X sw2 = 0x%2X\n",__FUNCTION__,__LINE__,buff[len[0]-2],buff[len[0]-1]);
		memcpy(rcvcmd,&buff[len[0]-2],2);
	    *rcvlen = 2;
		return -1;
	    
	}
	#else
	if(ret == 0)
	{
		if((len[0] == 2))
		{
		//	printf("buff[len[0]-2]=%02x\n",buff[len[0]-2]);
			if((buff[len[0]-2] == 0x61)||(buff[len[0]-2] == 0x6c))
			{
				getResp[4] = buff[len[0]-1];
				memset(buff,0,sizeof(buff));
				ret = Psam_Cmd_Send(PsamNum,getResp,sizeof(getResp),buff,len);
			//	printf("receve len=%d\n",len[0]);	
				if(ret != 0 || buff[len[0]-2] != 0x90 || buff[len[0]-1] != 0x00)
				{
				    LOGI("[ %s %d ] error step = %d  sw1 = 0x%2X sw2 = 0x%2X\n",__FUNCTION__,__LINE__,buff[len[0]-2],buff[len[0]-1]);
					memcpy(rcvcmd,buff,2);
				    *rcvlen = 2;
					return -1;				    
				}
				else
				{
					memcpy(rcvcmd,buff,len[0]);
				    *rcvlen = len[0];
				}
		
			}
			else 
			{
					memcpy(rcvcmd,buff,2);
				    *rcvlen = 2;
			}
			
		}
		else
		{
			memcpy(rcvcmd,buff,len[0]);
			*rcvlen = len[0];
		}
	}
	else
	{
		return -1;
	}

	#endif

		
	return 0;

}

int GetPsamID_ex(unsigned char Psam_index,unsigned char PsamNum[6],unsigned char *p_PsamKeyIndex)
{
    int ret = 0;
    unsigned char Loop = 1;
    unsigned char step = 1;
    unsigned char len[4];
    unsigned char buff[256];

    while(Loop)
    {
        LOGI("[ %s %d ] step = %d\n",__FUNCTION__,__LINE__,step);
        switch(step)
        {
            case 1:
                {
                    unsigned char getpsamnum[] = {0x00,0xb0,0x96,0x00,0x06};
                    ret = Psam_Cmd_Send(Psam_index,getpsamnum,sizeof(getpsamnum),buff,len);
                    if(ret != 0 || buff[len[0]-2] != 0x90 || buff[len[0]-1] != 0x00)
                    {
                        LOGI("[ %s %d ] error step = %d  sw1 = 0x%2X sw2 = 0x%2X\n",__FUNCTION__,__LINE__,step,buff[len[0]-2],buff[len[0]-1]);
                        Loop  = 0;
                        break;
                    }
                    memcpy(PsamNum,buff,6);
                    step ++;
                }
                break;

            case 2:
                {
                    unsigned  char selectapp[]  = {0x00,0xa4,0x00,0x00,0x02,0x80,0x11};
                    ret = Psam_Cmd_Send(Psam_index,selectapp,sizeof(selectapp),buff,len);
                    if(ret != 0 || buff[len[0]-2] != 0x90 || buff[len[0]-1] != 0x00)
                    {
                        LOGI("[ %s %d ] error step = %d  sw1 = 0x%2X sw2 = 0x%2X\n",__FUNCTION__,__LINE__,step,buff[len[0]-2],buff[len[0]-1]);
                        Loop  = 0;
                        break;
                    }
                    step ++;
                }
                break;

            case 3:                        
                {
                    unsigned char selectindex[] = {0x00,0xb0,0x97,0x00,0x01};
                    ret = Psam_Cmd_Send(Psam_index,selectindex,sizeof(selectindex),buff,len);
                    if(ret != 0 || buff[len[0]-2] != 0x90 || buff[len[0]-1] != 0x00)
                    {
                        LOGI("[ %s %d ] error step = %d  sw1 = 0x%2X sw2 = 0x%2X\n",__FUNCTION__,__LINE__,step,buff[len[0]-2],buff[len[0]-1]);
                        Loop  = 0;
                        break;
                    }
                    *p_PsamKeyIndex= buff[0];
                    step ++;
                }
                break; 

            case 4:
                step = 0;
                Loop = 0;
                break;

            default:
                Loop = 0;
                break;
        }
    }

    if(step != 0)
    {      
        int ret;
        ret = step;
        return ret;
    }

    return 0;
}

int  Psam_Cmd_Send(unsigned char PsamNum,unsigned        char* send_cmd, unsigned char send_cmd_len, char* recv_cmd, unsigned char *recv_cmd_len)
{
    unsigned char psam_num_temp   = 0;
    int  ret                      = 0;
    unsigned char  read_data_len  = 0;
    /* Psam Num 1-3(0x50-0x52) | Psam Num 4-6(0x60-0x62)*/
    switch(PsamNum)
    {
        case 0:
        case 1:
        case 2:
            psam_num_temp = 0x50 + PsamNum;
        break;

        case 3:
        case 4:
        case 5:
            psam_num_temp = 0x60 + (PsamNum-3);
        break;

        default:
        break;
    }
    
    /* Write Data */
    {
        unsigned char recv_buff[256]  = {0};
        unsigned char recv_len[4];
        unsigned char data_len[4] = {0,0,0,0};		
        data_len[0] = send_cmd_len;		
        ret = mcu_cmd_read_and_write(psam_num_temp,0x03,send_cmd,data_len,recv_buff,recv_len);
        if(ret != 0)
        {
            return -1;
        }
    }

    /* Get Data Length */
    {   
        unsigned char recv_buff[256]  = {0};
        unsigned char recv_len[4];
        unsigned char data_len[4] = {0,0,0,0};
        ret = mcu_cmd_read_and_write(psam_num_temp,0x07,NULL,data_len,recv_buff,recv_len);
        if(ret != 0)
        {
            return -2;
        }
        read_data_len = recv_buff[8];
    }

    /* Read */
    { 
        unsigned char recv_buff[256]  = {0};
        unsigned char recv_len[4];
        unsigned char data_len[4] = {1,0,0,0};
        ret = mcu_cmd_read_and_write(psam_num_temp,0x04,&read_data_len,data_len,recv_buff,recv_len);
        if(ret != 0)
        {
            return -3;
        }
        memcpy(recv_cmd, recv_buff+8, recv_len[0] + recv_len[1] + recv_len[2] + recv_len[3]);
		recv_cmd_len[0] = recv_buff[3];
    }
    return 0;
}


