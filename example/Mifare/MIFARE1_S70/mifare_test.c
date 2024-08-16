//#include "card_opt/M1/M1_card_operation.h"
#include "card_opt/M1/M1_card_operation.h"
#include "cardlan_devctrl.h"
#include "cpu_card_operation.h"



unsigned char key[6]="\x59\xa8\x2f\xde\xc6\xb3";  

unsigned char key1[6] = "\x2c\x4e\xad\x2d\x63\xad";
unsigned char key2[6] = {0xc9,0x14,0xba,0x18,0x8b,0xca} ;  

unsigned char *key_map[16] = {&key,&key,&key,&key,\
                              &key,&key,&key2,&key2,\
                              &key2,&key2,&key2,&key2,\
                              &key2,&key2,&key2,&key2};
unsigned char read_map[16] = {1,0,1,0,\
                              1,0,1,0,\
                              1,0,1,0,\
                              1,0,1,0};
unsigned char read_map2[16] = {1,1,1,1,\
                              1,1,1,1,\
                              1,1,1,1,\
                              1,1,1,1};



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



static int m1_card_test(void)
{
    int ret;
    

    char write_buf[16];  
    unsigned char SectorNo=1;
    unsigned char BlockNo =0;
    
    /*m1 card read test*/  
    if(1)
    {
        char write_buf[16];  
        unsigned char SectorNo=1;
        unsigned char BlockNo =0;
		char receive_buf[20];  
        unsigned long receive_len = 0;  
  		if(1)
        {
        	/*read info from block*/
            int ret;            
            unsigned char mode = 0x0a;
			int i;
			/*
			for(i = 0;i<15;i++)
				{
		            ret = ReadOneSectorDataFromCard(receive_buf,&receive_len, SectorNo+i,BlockNo,VERIFY_KEY,key1,mode);
		            if( ret != 0)
		            {
		                printf("[%s %d] error ret : %d  sectorNo = %d !!\n",__FUNCTION__,__LINE__,ret,SectorNo+i);
		              //  return -1;
		            }
					else		            
		            {
		            printf("-------------ReadOneSectorDataFromCard:  --------------\n");  
		            menu_print(receive_buf, receive_len);  
						}
		            ret = ReadOneSectorDataFromCard(receive_buf,&receive_len, SectorNo+i,BlockNo+1,VERIFY_KEY,key1,mode);
		            if( ret != 0)
		            {
		                printf("[%s %d] error ret : %d  sectorNo = %d !!\n",__FUNCTION__,__LINE__,ret,SectorNo+i);
		              //  return -1;
		            }
		            else		            
		            {
		            printf("-------------ReadOneSectorDataFromCard:  --------------\n");  
		            menu_print(receive_buf, receive_len);  
						} 

					ret = ReadOneSectorDataFromCard(receive_buf,&receive_len, SectorNo+i,BlockNo+2,VERIFY_KEY,key1,mode);
		            if( ret != 0)
		            {
		                printf("[%s %d] error ret : %d  sectorNo = %d !!\n",__FUNCTION__,__LINE__,ret,SectorNo+i);
		              //  return -1;
		            }
		           else		            
		            {
		            printf("-------------ReadOneSectorDataFromCard:  --------------\n");  
		            menu_print(receive_buf, receive_len);  
						} 
				}
				*/
				 mode = 0x0b;
			for(i = 0;i<15;i++)
				{
		            ret = ReadOneSectorDataFromCard(receive_buf,&receive_len, SectorNo+i,BlockNo,VERIFY_KEY,key2,mode);
		            if( ret != 0)
		            {
		                printf("[%s %d] error ret : %d  sectorNo = %d !!\n",__FUNCTION__,__LINE__,ret,SectorNo+i);
		              //  return -1;
		            }
		            
		            else		            
		            {
		            printf("-------------ReadOneSectorDataFromCard:  --------------\n");  
		            menu_print(receive_buf, receive_len);  
						} 
		            
		            ret = ReadOneSectorDataFromCard(receive_buf,&receive_len, SectorNo+i,BlockNo+1,VERIFY_KEY,key2,mode);
		            if( ret != 0)
		            {
		                printf("[%s %d] error ret : %d  sectorNo = %d !!\n",__FUNCTION__,__LINE__,ret,SectorNo+i);
		             //  return -1;
		            }
		            
		           else		            
		            {
		            printf("-------------ReadOneSectorDataFromCard:  --------------\n");  
		            menu_print(receive_buf, receive_len);  
						} 

					ret = ReadOneSectorDataFromCard(receive_buf,&receive_len, SectorNo+i,BlockNo+2,VERIFY_KEY,key2,mode);
		            if( ret != 0)
		            {
		                printf("[%s %d] error ret : %d  sectorNo = %d !!\n",__FUNCTION__,__LINE__,ret,SectorNo+i);
		               // return -1;
		            }
		            else		            
		            {
		            printf("-------------ReadOneSectorDataFromCard:  --------------\n");  
		            menu_print(receive_buf, receive_len);  
						} 
				}

			
        }

		if(0)
		{
			/*write info to block*/
        	unsigned char mode;
       		 mode = 0x0b;

			ret = ReadOneSectorDataFromCard(receive_buf,&receive_len, SectorNo+2,BlockNo,VERIFY_KEY,key2,mode);
			if( ret != 0)
			{
			    printf("[%s %d] error ret : %d  sectorNo = %d !!\n",__FUNCTION__,__LINE__,ret,SectorNo+2);
			  //  return -1;
			}

			else		            
			{
			printf("-------------ReadOneSectorDataFromCard:  --------------\n");  
			menu_print(receive_buf, receive_len);  
				} 
			 
			memset(write_buf,0,sizeof(write_buf));
			memcpy(write_buf,"\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00\xfe\x01\xfe\x01",16);
            ret = WriteOneSertorDataToCard(write_buf,16, SectorNo+2,BlockNo,VERIFY_KEY,key2,mode);
            if(ret != 0)
            {
                printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
                return -2;
            }
            			
             mode = 0x0b;
			 ret = ReadOneSectorDataFromCard(receive_buf,&receive_len, SectorNo+2,BlockNo,VERIFY_KEY,key2,mode);
            if( ret != 0)
            {
                printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
                return -1;
            }
			 printf("-------------ReadOneSectorDataFromCard:  --------------\n");  
            menu_print(receive_buf, receive_len);
			
        }

		if(0) 
		{
			/*change block to money block*/
        	unsigned char mode;
       		 mode = 0x0b;

			ret = changOneBlockToMoneyBlock(SectorNo+1, BlockNo, VERIFY_KEY, key1, mode);            
            if(ret != 0)
            {
                printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
                return -2;
            }
            			
             mode = 0x0b;
			 ret = ReadOneSectorDataFromCard(receive_buf,&receive_len, SectorNo+1,BlockNo,VERIFY_KEY,key1,mode);
            if( ret != 0)
            {
                printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
                return -1;
            }
			 printf("-------------ReadOneSectorDataFromCard:  --------------\n");  
            menu_print(receive_buf, receive_len);
			
        }
		
		if(0)
		{
			/*inc money*/
        	unsigned char mode;
       		 mode = 0x0b;
			 int money = 1;
			  mode = 0x0b;
			ret = ReadOneSectorDataFromCard(receive_buf,&receive_len, SectorNo+2,BlockNo,VERIFY_KEY,key2,mode);
            if( ret != 0)
            {
                printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
                return -1;
            }
			 printf("-------------ReadOneSectorDataFromCard:  --------------\n");  
            menu_print(receive_buf, receive_len);
			
			ret = incMoney(money, SectorNo+1, BlockNo, VERIFY_KEY, key1, mode);			
            if(ret != 0)
            {
                printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
                return -2;
            }
            			
            
			 ret = ReadOneSectorDataFromCard(receive_buf,&receive_len, SectorNo+2,BlockNo,VERIFY_KEY,key2,mode);
            if( ret != 0)
            {
                printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
                return -1;
            }
			 printf("-------------ReadOneSectorDataFromCard:  --------------\n");  
            menu_print(receive_buf, receive_len);
	
			
        }

		if(0)
		{
			/*dec money*/
			unsigned char mode;
			mode = 0x0b;
			int money = 1;
			mode = 0x0b;
			ret = decMoney(money, SectorNo+1, BlockNo, VERIFY_KEY, key1, mode);			
            if(ret != 0)
            {
                printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
                return -2;
            }
            			
            
			ret = ReadOneSectorDataFromCard(receive_buf,&receive_len, SectorNo+1,BlockNo,VERIFY_KEY,key1,mode);
            if( ret != 0)
            {
                printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
                return -1;
            }
			 printf("-------------ReadOneSectorDataFromCard:  --------------\n");  
            menu_print(receive_buf, receive_len);		
		}
		
		if(0)
		{
			/*restor and transfer*/		
			unsigned char mode;
       		mode = 0x0b;
			unsigned char restoreBlockNo = 0;
		    unsigned char transferBlockNo = 1;
			ret = restoreAndTransfer(SectorNo+1, restoreBlockNo, transferBlockNo, key1, mode);
			if( ret != 0)
            {
                printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
                return -1;
            }

			ret = ReadOneSectorDataFromCard(receive_buf,&receive_len, SectorNo+1,transferBlockNo,VERIFY_KEY,key1,mode);
            if( ret != 0)
            {
                printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
                return -1;
            }
			 printf("-------------ReadOneSectorDataFromCard:  --------------\n");  
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
        return -1;
    }

    char             Buffer[10] = {0};
    unsigned char    len         = 0;
	while(1)
    {
    	ret = CardReset(Buffer, &len , 0);
		if(ret > 0)
	    {
	    	printf("cardreset:%02x\n",ret);
	    	menu_print(Buffer, len);   			
			}
		sleep(1);
		}    
    
    return 0;
}
