
#include "card_opt/M1/M1_plus_card_operation.h"

/*for level 1 */
unsigned char keyA[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
unsigned char keyB[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

/*for level 2 */
unsigned char MASTER_KEY[16] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
unsigned char CONFIGURATION_KEY[16] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
unsigned char L3_SWITCH_KEY[16] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
unsigned char L2_SWITCH_KEY[16] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
unsigned char L1_CARD_AUTH_KEY[16] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};


unsigned char uid[4];

int level;
static int m1_plus_card_test_level0(void);
static int m1_plus_card_test_level1(void);
static int m1_plus_card_test_level2(void);
static int m1_plus_card_test_level3(void);


static int m1_plus_card_test(void)
{
    int ret = 0;
    printf("MFP_detect_Security_Level : %d\n",level );

    switch(level)
    {
    case 0:
        ret = m1_plus_card_test_level0();
        break;
    case 1:        
        ret = m1_plus_card_test_level1();
        break;
    case 2:
        ret = m1_plus_card_test_level2();
        break;
    case 3:        
        ret = m1_plus_card_test_level3();
        break;
    default:
        return -1;
    }
    if(ret != 0)
    {
        printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }
 
    return 0;
}
static int m1_plus_card_test_level0(void)
{
    int ret = 0;
    {
        unsigned char hex_addr_1 = (MF_CARD_MASTER_KEY >> 8) & 0xFF;
        unsigned char hex_addr_2 = (MF_CARD_MASTER_KEY >> 0) & 0xFF;
        memset(MASTER_KEY,0xFF,sizeof(MASTER_KEY));
        ret = MFPL0_write_perso(hex_addr_1,hex_addr_2,MASTER_KEY);
        if(ret != 0)
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
    }

    printf("[%s %d]  set MF_CARD_MASTER_KEY ok \n",__FUNCTION__,__LINE__);

    {
        unsigned char hex_addr_1 = (MF_CARD_CONFIGURATION_KEY >> 8) & 0xFF;
        unsigned char hex_addr_2 = (MF_CARD_CONFIGURATION_KEY >> 0) & 0xFF;
        ret = MFPL0_write_perso(hex_addr_1,hex_addr_2,CONFIGURATION_KEY);
        if(ret != 0)
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
    }
    printf("[%s %d]  set MF_CARD_CONFIGURATION_KEY ok \n",__FUNCTION__,__LINE__);
    //may be error
    {
 
        unsigned char hex_addr_1 = (MF_L2_SWITCH_KEY >> 8) & 0xFF;
        unsigned char hex_addr_2 = (MF_L2_SWITCH_KEY >> 0) & 0xFF;

        ret = MFPL0_write_perso(hex_addr_1,hex_addr_2,L2_SWITCH_KEY);
        if(ret != 0)
        {
            printf("[%s %d]  set MF_L2_SWITCH_KEY error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            //return ret;
        }
        else
        {
            printf("[%s %d]  set MF_L2_SWITCH_KEY ok \n",__FUNCTION__,__LINE__);
        }
    }

    {
        unsigned char hex_addr_1 = (MF_L3_SWITCH_KEY >> 8) & 0xFF;
        unsigned char hex_addr_2 = (MF_L3_SWITCH_KEY >> 0) & 0xFF;
        ret = MFPL0_write_perso(hex_addr_1,hex_addr_2,L3_SWITCH_KEY);
        if(ret != 0)
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
    }
    printf("[%s %d]  set MF_L3_SWITCH_KEY ok \n",__FUNCTION__,__LINE__);
    {
        unsigned char dataperso[16] ;
        unsigned char hex_addr_1 = (MF_L1_CARD_AUTH_KEY >> 8) & 0xFF;
        unsigned char hex_addr_2 = (MF_L1_CARD_AUTH_KEY >> 0) & 0xFF;
        memset(dataperso,0xFF,sizeof(dataperso));
        ret = MFPL0_write_perso(hex_addr_1,hex_addr_2,dataperso);
        if(ret != 0)
        {
            printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
    }
    printf("[%s %d]  set MF_L1_CARD_AUTH_KEY ok \n",__FUNCTION__,__LINE__);
    // OMG OMG
    // HOPE CARD WONT DIE  success will switch to level 1
    //ret = MFPL0_commit_perso(); 
    if(ret != 0)
    {
        printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }

    return 0;
}

static int m1_plus_card_test_level1(void)
{
    int ret;

    {
        unsigned char SectorBuf[16];
        
        ret = MFPL1_read(1,0,keyB,0x0B,uid,SectorBuf);
        if( ret != 0)
        {
            printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
            return -1;
        }
        printf("-------------Mifare_classic_read:  --------------\n");  
        menu_print(SectorBuf, 16);  
        
        SectorBuf[0]++;
        ret =MFPL1_write(1,0,keyA,0x0A,uid,SectorBuf);
        if( ret != 0)
        {
            printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
            return -1;
        }

        ret = MFPL1_read(1,0,keyA,0x0A,uid,SectorBuf);
        if( ret != 0)
        {
            printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
            return -1;
        }
        printf("-------------Mifare_classic_read:  --------------\n");  
        menu_print(SectorBuf, 16);  

        
    }
    /*m1 card read test*/   
    {
        int Sector;
        unsigned char type = 0x0A;
        for(Sector = 0; Sector < 64; Sector++ )
        {
            if(0)
            {
                continue;
            }
            printf("-------------Mifare_classic_read: Sector %d  --------------\n",Sector);  
            unsigned char Num = 4;
            unsigned char readdata[64];
            int readlen = 0;
            ret = MFPL1_authenticate(Sector,keyA,type,uid);
            if( ret != 0)
            {
                printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
                return -1;
            }
            ret = Mifare_classic_read_ex(Sector * 4, Num,readdata,&readlen);
            if( ret != 0)
            {
                printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
                return -1;
            }
            menu_print(readdata, readlen);      
        }  
    }

    /* switch level 3 不要触发多次*/
    {
        //ret = MFPL1_switch_to_level3(L3_SWITCH_KEY,sizeof(L3_SWITCH_KEY));
        if(ret != 0)
        {
            printf("[%s %d] error ret : %d !!\n",__FUNCTION__,__LINE__,ret);
            return -3;
        }
    }

    return 0;
}


static int m1_plus_card_test_level2(void)
{

    return 0;
}

static int m1_plus_card_test_level3(void)
{
    unsigned char authkey[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    int ret = 0;
    unsigned char block = 1;
    unsigned int  BNr = 0x0001;
    unsigned char write_data[256];
    unsigned char recv [256];
    int len;
    
    ret = MFPL3_authl3_key_ex(0x4000 + BNr * 2,authkey);
    if(ret != 0)
    {
        printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }
  
    ret = MFPL3_read_in_plain(BNr,block,recv,&len);
    if(ret != 0)
    {
        printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }
    printf("-------------MFPL3_read_in_plain:  --------------\n");  
    menu_print(recv,len);

#if 0
    memcpy(write_data,recv,len);
    write_data[0]++;
    write_data[len - 1]++;
    ret = MFPL3_write_encrypted(BNr,block,write_data,0);
    if(ret != 0)
    {
        printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }

    ret = MFPL3_read_encrypted(BNr,block,recv,0);
    if(ret != 0)
    {
        printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }
    printf("-------------MFPL3_read_in_plain:  --------------\n");  
    menu_print(recv,block*16);
#endif

    memcpy(write_data,recv,len);
    write_data[0]++;
    write_data[len - 1]++;
    ret = MFPL3_write_in_plain(BNr,block,write_data,len);
    if(ret != 0)
    {
        printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }

    ret = MFPL3_read_in_plain(BNr,block,recv,&len);
    if(ret != 0)
    {
        printf("[%s %d]  error ret:%d!!\n",__FUNCTION__,__LINE__,ret);
        return ret;
    }
    printf("-------------MFPL3_read_in_plain:  --------------\n");  
    menu_print(recv,len);

   
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

    char             Buffer[10] = {0};
    unsigned char    len         = 0;
    ret = CardReset(Buffer, &len , 0);
    switch(ret)
	{
	case 0x11:
        {
            level = 2;
        }
        break;
	case 0x18:
        {
            level = 1;
        }
        break;
    case 0x20:
        {
            level = MFP_detect_Security_Level();
            if(level != 0 && level != 3)   
            {
                printf("[%s %d]  error ret:0x%02X!!\n",__FUNCTION__,__LINE__,level);
                return -1;
            }
        }
        break;
	default:
	    printf("[%s %d]  error bSak[0]:0x%02X!!\n",__FUNCTION__,__LINE__,ret);
		return -1;
	}
    printf("cardreset:\n");
    menu_print(Buffer, len);
    int atuh_index = len == 8 ? 3 : 0;
    memcpy(uid,Buffer+ atuh_index,4);

    ret = m1_plus_card_test();
    if(ret != 0)   
    {
	    printf("[%s %d]  error ret:0x%02X!!\n",__FUNCTION__,__LINE__,ret);
        return -2;
    } 
    return 0;
}


