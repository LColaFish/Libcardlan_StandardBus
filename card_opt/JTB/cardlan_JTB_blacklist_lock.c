#include "typea.h"
#include "libcardlan_CardInfo.h"
#include "mcu_opt/psam_opt.h"

#define            MI_OK           0x00

static int        receive_len[1] = {0};
static char		  receive_buf[128]= {0};

unsigned char CardLock(void)
{
	int result;
	unsigned char Timebuf[8];
	unsigned char Send[256],Recv[256];
	unsigned char flag,t,status,len,i;    
	
	flag = 1;
	t = 0;
	while(flag)
	{
	    switch(t)
		{

			case 0:
			{
				printf("=[%d]===%s:%d=====\n", t, __FUNCTION__, __LINE__);
				result = mifare_write("\x80\x5c\x03\x02\x04",5);
				if(result == MI_OK)
				{
					mifare_ioctl(FIFO_RCV_LEN, receive_len);
					result = mifare_read(receive_buf, receive_len[0]);
					if(receive_len[0] > 2)
					{
						t++;
						memcpy(JTB_CardInfo.beforemoney,receive_buf,4);
						printf("\n交易前余额:%d分\n",receive_buf[0]<<24|receive_buf[1]<<16|receive_buf[2]<<8|receive_buf[3]);
					}
					else
					{
						printf("查询余额 SW1=%02X  SW2=%02X\n",receive_buf[0],receive_buf[1]);
						flag = 0;
					}
				}
				else {
					flag = 0;
				}
			}
			break;
		
			case 1://进入PSAM卡片目录
			{
				printf("=[%d]===%s:%d=====\n", t, __FUNCTION__, __LINE__);
            	memset(Send,0,sizeof(Send));
				memcpy(Send,"\x00\x84\x00\x00\x08",5);		//命令头			
				len = 5;
				result = mifare_write(Send,len);
				if(result == MI_OK)
				{
					mifare_ioctl(FIFO_RCV_LEN, receive_len);
					result = mifare_read(receive_buf, receive_len[0]);
					if(receive_len[0] > 2)
					{
						t=3;
						printf("取卡随机数返回:");
						menu_print(receive_buf, receive_len[0]);
						memcpy(JTB_CardInfo.PSAMRandom,&receive_buf[0],8);
					}
					else
					{
						printf("取卡随机数返回SW1=%02X   SW2=%02X\n",receive_buf[0],receive_buf[1]);
						flag = 0;
					}
				}			
			}
			break;
            
			case 2://PSAM卡生随机数
			{
				printf("=[%d]===%s:%d=====\n", t, __FUNCTION__, __LINE__);
				memset(Send,0,sizeof(Send));
				memcpy(Send,"\x00\x84\x00\x00\x08",5);			//命令头			
				len = 5;
				memset(Recv,0,sizeof(Recv));

            	printf("生成随机数发送:");
		    	menu_print(Send, len);
            	status = Psam_Cmd_Send(0, Send,len,Recv,&len);
            	
				printf("生成随机数返回:");
		    	menu_print(Recv, len);
	
            	if((status == MI_OK)&&(Recv[len-2] == 0x90)&&(Recv[len-1] == 0x00))
				{
					t++;				
					memcpy(JTB_CardInfo.PSAMRandom,Recv,8);
					
				}
				else flag = 0;
			}
			break;          
			
			case 3://准备发送
			printf("=[%d]===%s:%d=====\n", t, __FUNCTION__, __LINE__);
            memset(Send,0,sizeof(Send));
			memcpy(Send,"\x80\x1A\x45\x02\x10",5);			//命令头	
            memcpy(Send+5,JTB_CardInfo.appserialnumber+2,8);
            memcpy(Send+13,JTB_CardInfo.issuerlabel,8);
            len = 21;
            
            printf("准备发送:");
		    menu_print(Send, len);
            status = Psam_Cmd_Send(0, Send,len,Recv,&len);
            
			printf("准备发送返回:");
		    menu_print(Recv, len);
	
            if((status == MI_OK)&&(Recv[len-2] == 0x90)&&(Recv[len-1] == 0x00))
			{
				t++;				
			}
			else flag = 0;
            
			break;

            case 4://初始化计算MAC
			printf("=[%d]===%s:%d=====\n", t, __FUNCTION__, __LINE__);
            memset(Send,0,sizeof(Send));
			memcpy(Send,"\x80\xFA\x05\x00\x10",5);
            memcpy(Send+5,JTB_CardInfo.PSAMRandom,8);
            memcpy(Send+13,"\x84\x1E\x00\x00\x04\x80\x00\x00",8);
            len = 21;
            printf("初始化计算MAC发送");
		    menu_print(Send, len);
            status = Psam_Cmd_Send(0, Send,len,Recv,&len);
            
			printf("初始化计算MAC返回");
		    menu_print(Recv, len);    
            if((status == MI_OK)&&(Recv[len-2] == 0x90)&&(Recv[len-1] == 0x00))
			{
				t++;
                memcpy(JTB_CardInfo.DESCRY,Recv,4);
			}
			else flag = 0;
            
            break;

			case 5://应用锁定			
			printf("=[%d]===%s:%d=====\n", t, __FUNCTION__, __LINE__);
			memset(Send,0,sizeof(Send));
			memcpy(Send,"\x84\x1E\x00\x00\x04",5);		//命令头
			memcpy(&Send[5],JTB_CardInfo.DESCRY,4);	//终端交易序号										
			len = 9;
            printf("应用锁定发送:");
		    menu_print(Send, len);
			result = mifare_write(Send,len);
			if(result == MI_OK)
			{
				mifare_ioctl(FIFO_RCV_LEN, receive_len);
				result = mifare_read(receive_buf, receive_len[0]);
				if(receive_len[0] > 2)
				{
					t++;
					printf("应用锁定发送:");
					menu_print(receive_buf, receive_len[0]);					
				}
				else if(receive_len[0] == 2)
				{
					printf("应用锁定返回:SW1=%02X   SW2=%02X\n",receive_buf[0],receive_buf[1]);
					flag = 0;
				}
				
			}
			else flag = 0;
			
			break;            
            
		           
		default:
			flag=0;
			t=0;
			break;
		}
	}
	return t;
}
