
#include "cardlan_JTB_operation.h"



#if 0

unsigned char Card_DebitChas_jiaotong(unsigned int Money,char *Key,unsigned char Keylen)
{
  #if 0
    int result;
    unsigned char Timebuf[8];
    unsigned char buff1[20];
    unsigned char Send[128],Recv[256 + 2];
    unsigned char flag,t,status,len,i;
    extern SysTime           Time;    

    Timebuf[0] = 0x20;
    Timebuf[1] = Time.year;
    Timebuf[2] = Time.month;
    Timebuf[3] = Time.day;
    Timebuf[4] = Time.hour;
    Timebuf[5] = Time.min;
    Timebuf[6] = Time.sec;

    flag = 1;
    t = 1;
    LOGI("in card_debitchas_jiaotong\n");
    while(flag)
    {
        switch(t)
        {
        case 1://脧没路脩鲁玫脢录禄炉
            {
                extern unsigned char PsamKeyIndex; 
                unsigned char PsamNum[6];
                memset(Send,0,sizeof(Send));
                memcpy(Send,"\x80\x50\x01\x02\x0B",5);         //脙眉脕卯脥路
                Send[5] = PsamKeyIndex;                     //脙脺脭驴脣梅脪媒
                Send[6] = (Money&0xFF000000)>>24;
                Send[7] = (Money&0xFF0000)>>16;
                Send[8] = (Money&0xFF00)>>8;
                Send[9] = Money&0xFF;                        //陆禄脪脳陆冒露卯
                memcpy(&Send[10],PsamNum,6);                 //脰脮露脣禄煤潞脜
                Send[16] = 0x0f;                             //Le
                len = 17;

                result = mifare_read_and_write(Send,len,Recv);
                if(result <= 2)
                {
                    LOGI("mifare_read_and_write fail result:%02X SW1:%02X SW2:%02X\n",result,receive_buf[0],receive_buf[1]);
                    flag = 0 ;
                    break;
                }
                t++;
                memcpy(JTB_CardInfo.offlineSN,&receive_buf[4],2);
                memcpy(JTB_CardInfo.overdraftAmount,&receive_buf[6],3);
                JTB_CardInfo.keyVersion = receive_buf[9];
                JTB_CardInfo.arithmeticLabel = receive_buf[10];
                memcpy(JTB_CardInfo.PRandom,&receive_buf[11],4);
            }
            break;
        case 2:
            {
                memset(Send,0,sizeof(Send));
                memcpy(Send,"\x80\x70\x00\x00\x24",5);            
                memcpy(&Send[5],JTB_CardInfo.PRandom,4);            
                memcpy(&Send[9],JTB_CardInfo.offlineSN,2);            
                Send[11] = (Money&0xFF000000)>>24;
                Send[12] = (Money&0xFF0000)>>16;
                Send[13] = (Money&0xFF00)>>8;
                Send[14] = Money&0xFF;                          
                Send[15] = 0x06;                                
                memcpy(&Send[16],Timebuf,7);                    
                Send[23] = JTB_CardInfo.keyVersion;                  
                Send[24] = JTB_CardInfo.arithmeticLabel;            
                memcpy(&Send[25],&JTB_CardInfo.appserialnumber[2],8);
                memcpy(&Send[33],JTB_CardInfo.issuerlabel,8);        
                len = 41;
                memset(Recv,0,sizeof(Recv));
                LOGI("录脝脣茫MAC路垄脣脥:");
                menu_print(Send, len);
                status = PsamCos(Send,Recv,&len);
                if((status == MI_OK)&&(Recv[len-2] == 0x90)&&(Recv[len-1] == 0x00))
                {
                    t++;
                    menu_print(Recv, len);
                    memcpy(JTB_CardInfo.PSAMOfflineSN,Recv,4);
                    memcpy(JTB_CardInfo.MAC1,&Recv[4],4);
                }
                else flag = 0;
           }
           break;
        
        case 3:
            {
                memset(Send,0,sizeof(Send));
                memcpy(Send,"\x80\x54\x01\x00\x0F",5);         
                memcpy(&Send[5],JTB_CardInfo.PSAMOfflineSN,4);    
                memcpy(&Send[9],Timebuf,7);                 
                memcpy(&Send[16],JTB_CardInfo.MAC1,4);        
                Send[20] = 0x08;                             
                len = 21;

                result = mifare_read_and_write(Send,len,receive_buf);
                if(result <= 2)
                {
                    LOGI("mifare_read_and_write fail result:%02X SW1:%02X SW2:%02X\n",result,receive_buf[0],receive_buf[1]);
                    flag = 0 ;
                    break;
                }
                t++;
                menu_print(receive_buf, receive_len[0]);
                memcpy(JTB_CardInfo.TAC,receive_buf,4);
                memcpy(JTB_CardInfo.MAC2,&receive_buf[4],4);

            }
            break;
        case 4://PSAM驴篓脩茅脰陇MAC2
            {
                memset(Send,0,sizeof(Send));
                memcpy(Send,"\x80\x72\x00\x00\x04",5);        //脙眉脕卯脥路
                memcpy(&Send[5],JTB_CardInfo.MAC2,4);        //MAC2
                len = 9;
                for(i=0;i<2;i++)
                {
                    status = PsamCos(Send,Recv,&len);
                    if((status == MI_OK)&&(Recv[len-2] == 0x90)&&(Recv[len-1] == 0x00))
                    {
                    //    LOGI("PSAM驴篓脩茅脰陇MAC2  SW1=%02X  SW2=%02X\n",Recv[len-2],Recv[len-1]);
                        break;
                    }
                }
                t++;
            }
            break;
        case 5:
            t = 0;
            flag =0 ;
            break;

        default :
            flag =0 ;
            break;
        }
    }
    return t;
 #endif   
}

//赂麓潞脧脧没路脩陆禄脪脳脕梅鲁脤
unsigned char Card_DebitChas_complex_jiaotong(unsigned int Money,char *Key,unsigned char Keylen)
{
#if 0
    int result;
    unsigned char Timebuf[8];
    unsigned char Send[256],Recv[256];
    unsigned char flag,t,status,len,i;
    unsigned int beforeMoney;
    Timebuf[0] = 0x20;
    Timebuf[1] = Time.year;
    Timebuf[2] = Time.month;
    Timebuf[3] = Time.day;
    Timebuf[4] = Time.hour;
    Timebuf[5] = Time.min;
    Timebuf[6] = Time.sec;
    
    flag = 1;
    t = 1;
    while(flag)
    {
        LOGI("%s:%d----%d----\n", __FUNCTION__, __LINE__, t);
        switch(t)
        {
            case 1://赂麓潞脧脫娄脫脙脧没路脩鲁玫脢录禄炉
                {
                    memset(Send,0,sizeof(Send));
                    memcpy(Send,"\x80\x50\x03\x02\x0B",5);         //脙眉脕卯脥路
                    Send[5] = PsamKeyIndex;                     //脙脺脭驴脣梅脪媒
                    Send[6] = (Money&0xFF000000)>>24;
                    Send[7] = (Money&0xFF0000)>>16;
                    Send[8] = (Money&0xFF00)>>8;
                    Send[9] = Money&0xFF;                        //陆禄脪脳陆冒露卯
                    memcpy(&Send[10],PsamNum,6);                 //脰脮露脣禄煤潞脜
                    Send[16] = 0x0f;                             //Le
                    len = 17;

                    result =  mifare_read_and_write(Send,len, receive_buf);
                    if(result <= 2)
                    {
                        LOGI("[%s %d] mifare_read_and_write fail result %02X SW1:%02X SW2:%02X\n",__FUNCTION__,__LINE__,result,receive_buf[0],receive_buf[1]);
                        flag = 0;
                        break;
                    }
                    t++;
                    memcpy(JTB_CardInfo.offlineSN,&receive_buf[4],2);
                    memcpy(JTB_CardInfo.overdraftAmount,&receive_buf[6],3);
                    JTB_CardInfo.keyVersion = receive_buf[9];
                    JTB_CardInfo.arithmeticLabel = receive_buf[10];
                    memcpy(JTB_CardInfo.PRandom,&receive_buf[11],4);
                }
                break;
            
            case 2://PSAM驴篓脡煤鲁脡MAC1
                {
                    LOGI("鲁玫脢录禄炉潞贸碌脛ATC1:%02x %02x\n",JTB_CardInfo.offlineSN[0],JTB_CardInfo.offlineSN[1]);
                    memset(Send,0,sizeof(Send));
                    memcpy(Send,"\x80\x70\x00\x00\x24",5);            //脙眉脕卯脥路
                    memcpy(&Send[5],JTB_CardInfo.PRandom,4);            //脫脙禄搂驴篓脝卢脣忙禄煤脢媒
                    memcpy(&Send[9],JTB_CardInfo.offlineSN,2);            //脫脙禄搂驴篓脥脩禄煤陆禄脪脳脨貌潞脜
                    Send[11] = (Money&0xFF000000)>>24;
                    Send[12] = (Money&0xFF0000)>>16;
                    Send[13] = (Money&0xFF00)>>8;
                    Send[14] = Money&0xFF;                            //陆禄脪脳陆冒露卯
                    Send[15] = 0x09;                                //陆禄脪脳脌脿脨脥
                    memcpy(&Send[16],Timebuf,7);                    //陆禄脪脳脠脮脝脷脢卤录盲
                    Send[23] = JTB_CardInfo.keyVersion;                    //脙脺脭驴掳忙卤戮潞脜
                    Send[24] = JTB_CardInfo.arithmeticLabel;                //脙脺脭驴脣茫路篓卤锚脢露
                    memcpy(&Send[25],&JTB_CardInfo.appserialnumber[2],8);//脫脙禄搂驴篓潞脜
                    memcpy(&Send[33],JTB_CardInfo.issuerlabel,8);        //路垄驴篓禄煤鹿鹿卤脿脗毛
                    len = 41;
                    memset(Recv,0,sizeof(Recv));

                    LOGI("脡煤鲁脡MAC1路垄脣脥:");
                    menu_print(Send, len);
                    status = PsamCos(Send,Recv,&len);
                    
                    LOGI("脡煤鲁脡MAC1路碌禄脴:");
                    menu_print(Recv, len);
            
                    if((status == MI_OK)&&(Recv[len-2] == 0x90)&&(Recv[len-1] == 0x00))
                    {
                        t++;
                        menu_print(Recv, len);
                        memcpy(JTB_CardInfo.PSAMOfflineSN,Recv,4);
                        memcpy(JTB_CardInfo.MAC1,&Recv[4],4);
                    }
                    else flag = 0;
                }
                break;          
            
            case 3://脫脙禄搂驴篓赂眉脨脗赂麓潞脧脫娄脫脙录脟脗录脦脛录镁
                {
                    if(Section.Enable != 0x55)
                    {
                        t++;
                    }
                    else
                    {
                        memset(Send,0,sizeof(Send));
                        memcpy(Send,"\x80\xDC\x02\xD0\x80",5);
                        memcpy(&Send[5],"\x27\x02\x7D\x01\x01",5);
                        
                        Send[10] = JTB_CardInfo.applockflag;
                        memcpy(&Send[11],JTB_CardInfo.tradeserialnumber,8);
                        
                        Send[19] = JTB_CardInfo.tradestate;
                        memcpy(&Send[20],JTB_CardInfo.getoncitycode,2);
                        memcpy(&Send[22],JTB_CardInfo.getonissuerlabel,8);
                        memcpy(&Send[30],JTB_CardInfo.getonoperatorcode,2);
                        memcpy(&Send[32],JTB_CardInfo.getonline,2);
                        
                        Send[34] = JTB_CardInfo.getonstation;
                        memcpy(&Send[35],JTB_CardInfo.getonbus,8);
                        memcpy(&Send[43],JTB_CardInfo.getondevice,8);
                        memcpy(&Send[51],JTB_CardInfo.getontime,7);
                        memcpy(&Send[58],JTB_CardInfo.markamount,4);
                        
                        Send[62] = JTB_CardInfo.directionflag;
                        memcpy(&Send[63],JTB_CardInfo.getoffcitycode,2);
                        memcpy(&Send[65],JTB_CardInfo.getoffissuerlabel,8);
                        memcpy(&Send[73],JTB_CardInfo.getoffoperatorcode,2);
                        memcpy(&Send[75],JTB_CardInfo.getoffline,2);
                        
                        Send[77] = JTB_CardInfo.getoffstation;
                        memcpy(&Send[78],JTB_CardInfo.getoffbus,8);
                        memcpy(&Send[86],JTB_CardInfo.getoffdevice,8);
                        memcpy(&Send[94],JTB_CardInfo.getofftime,7);
                        memcpy(&Send[101],JTB_CardInfo.tradeamount,4);
                        memcpy(&Send[105],JTB_CardInfo.ridedistance,2);
                        
                        len = 133;

                        result =  mifare_read_and_write(Send,len, receive_buf);
                        if(result <= 2)
                        {   
                            LOGI("[%s %d] mifare_read_and_write fail result %02X SW1:%02X SW2:%02X\n",__FUNCTION__,__LINE__,result,receive_buf[0],receive_buf[1]);
                            flag = 0;
                            break;
                        }

                        t++;
                        LOGI("赂眉脨脗赂麓潞脧脫娄脫脙脢媒戮脻禄潞麓忙路碌禄脴:");
                        menu_print(receive_buf, receive_len[0]);
                    }
                }
                break;

            case 4:
                {
                    memset(Send,0,sizeof(Send));
                    memcpy(Send,"\x80\xDC\x00\xF0\x30",5);
                    if(Section.Enable!=0x55)                     //陆禄脪脳脌脿脨脥
                    {
                        Send[5] = 0x06;
                    }
                    else
                    {
                        if(JTB_CardInfo.enterexitflag == 0x55)
                        {
                            Send[5] = 0x03;
                        }
                        else
                        {
                            Send[5] = 0x04;
                        }
                    }
                    
                    Send[6] = 0;
                    Send[7] = 0;
                    memcpy(&Send[8],PsamNum,6);                  //脰脮露脣卤脿潞脜
                    Send[14] = 0x02;                             //脨脨脪碌麓煤脗毛
                    //memcpy(&Send[15],Yanzhou_Card.LineNO,2);     //脧脽脗路潞脜
                    if(1)//Section.Enable!=0x55)                     //脮戮碌茫潞脜
                    {
                        Send[17] = 0x00;
                        Send[18] = 0x00;
                    }
                    else
                    {
                        if(JTB_CardInfo.enterexitflag == 0x55)
                        {
                            Send[17] = 0x00;
                            Send[18] = JTB_CardInfo.getonstation;
                        }
                        else
                        {
                            Send[17] = 0x00;
                            Send[18] = JTB_CardInfo.getoffstation;
                        }
                    }
                    //memcpy(&Send[19],Yanzhou_Card.OperatorCode,2);
                    Send[21] = 0;
                    Send[22] = (Money&0xFF000000)>>24;
                    Send[23] = (Money&0xFF0000)>>16;
                    Send[24] = (Money&0xFF00)>>8;
                    Send[25] = Money&0xFF;                            //陆禄脪脳陆冒露卯
                    beforeMoney=JTB_CardInfo.beforemoney[0]<<24|JTB_CardInfo.beforemoney[1]<<16|JTB_CardInfo.beforemoney[2]<<8|JTB_CardInfo.beforemoney[3];                                                   
                    //陆禄脪脳潞贸脫脿露卯    
                    Send[26] = ((beforeMoney-HostValue.i)&0xFF000000)>>24;
                    Send[27] = ((beforeMoney-HostValue.i)&0xFF0000)>>16;
                    Send[28] = ((beforeMoney-HostValue.i)&0xFF00)>>8;
                    Send[29] = ((beforeMoney-HostValue.i)&0xFF)>>0;
                    memcpy(&Send[30],Timebuf,7);                    //陆禄脪脳脢卤录盲
                    //memcpy(&Send[37],&Yanzhou_Card.CityCode[0],2);  //脢脺脌铆路陆鲁脟脢脨麓煤脗毛
                    //memcpy(&Send[39],&Yanzhou_Card.IssuerLabel,8);  //脢脺脌铆路陆禄煤鹿鹿卤锚脢露                
                    len = 53;

                    result = mifare_read_and_write(Send,len,receive_buf);
                    if(receive_buf[0]!=0x90 || receive_buf[1] != 0x00)
                    {
                        LOGI("[%s %d] mifare_read_and_write fail result %02X SW1:%02X SW2:%02X\n",__FUNCTION__,__LINE__,result,receive_buf[0],receive_buf[1]);
                        flag = 0 ;
                        break;
                    }
                    t++;
                    LOGI("赂眉脨脗陆禄脪脳脨脜脧垄录脟脗录路垄脣脥:");
                    menu_print(receive_buf, receive_len[0]);
                }
                break;

            case 5://脰麓脨脨驴脹驴卯
                {
                    LOGI("鲁玫脢录禄炉潞贸碌脛ATC2:%02x %02x\n",JTB_CardInfo.offlineSN[0],JTB_CardInfo.offlineSN[1]);
                    memset(Send,0,sizeof(Send));
                    memcpy(Send,"\x80\x54\x01\x00\x0F",5);         //脙眉脕卯脥路
                    memcpy(&Send[5],JTB_CardInfo.PSAMOfflineSN,4);    //脰脮露脣陆禄脪脳脨貌潞脜
                    memcpy(&Send[9],Timebuf,7);                 //脰脮露脣陆禄脪脳脠脮脝脷脢卤录盲
                    memcpy(&Send[16],JTB_CardInfo.MAC1,4);        //MAC1
                    
                    Send[20] = 0x08;                             //Le
                    len = 21;
                    LOGI("脰麓脨脨驴脹驴卯路垄脣脥:");                     
                    menu_print(Send, len);
                    
                    result = mifare_read_and_write(Send,len,receive_buf);
                    if(result <= 0)
                    {
                    /*
                        status = SecondDebitChas_jiaotong(Money,Key,Keylen);
                        LOGI("脰脴脨脗脣垄驴篓路碌禄脴:%02x\n",status);
                        if(status == 0)
                        {
                            t ++;
                        }
                        else if(status == 0xFF)
                        {
                            t = 1;
                            LOGI("SecondDebitChas_jiaotong fail\n");
                            flag = 0 ;
                        }
                        else
                    */
                        {
                            t = 0xAA;
                            LOGI("SecondDebitChas_jiaotong fail\n");
                            flag = 0 ;
                        } 
             
                        break;
                    }
                    t++;
                    LOGI("脰麓脨脨驴脹驴卯路碌禄脴:");
                    menu_print(receive_buf, receive_len[0]);
                    memcpy(JTB_CardInfo.TAC,receive_buf,4);
                    memcpy(JTB_CardInfo.MAC2,&receive_buf[4],4);
                }
                break;
            
            case 6://PSAM驴篓脩茅脰陇MAC2
            {
                memset(Send,0,sizeof(Send));
                memcpy(Send,"\x80\x72\x00\x00\x04",5);     //脙眉脕卯脥路
                memcpy(&Send[5],JTB_CardInfo.MAC2,4);        //MAC2
                len = 9;
                for(i=0;i<2;i++)
                {
                    status = PsamCos(Send,Recv,&len);
                    if((status == MI_OK)&&(Recv[len-2] == 0x90)&&(Recv[len-1] == 0x00))
                    {
                        printf("PSAM驴篓脩茅脰陇MAC2  SW1=%02X  SW2=%02X\n",Recv[len-2],Recv[len-1]);
                        break;
                    }
                }
                t++;
            }
            break;
                   
        default:
            {
                flag=0;
                t=0;
            }
            break;
        }
    }
    
    return t;
#endif
}
unsigned char TopUpCardInfor_CPU_jiaotong(unsigned int Value,int type)
{
#if 0
    unsigned char status = 1;
    unsigned char keybuff[20];
    Value = 100;
    if(type == 1) 
    {
        status  = Card_DebitChas_complex_jiaotong(Value,keybuff,16);
    }
    else if(type == 2)
    {
        status  = Card_DebitChas_jiaotong(Value,keybuff,16);
    }
    else 
    {
        LOGI("impossible at TopUpCardInfor_CPU\n");
    }
    LOGI("陆禄脪脳路碌禄脴脰碌:%02x\n",status);
    return status;
#endif
}

unsigned char Card_JudgeDate_jiaotong(void)
{
#if 0
    unsigned char status = 1;
    unsigned char t;
    unsigned char buff[7];
    unsigned int start_time,end_time,this_time;


    for(t = 0; t<3; t++)
    {
        memset(buff,0,sizeof(buff));
        Rd_time (buff);
        Time.year = buff[0];
        Time.month = buff[1];
        Time.day = buff[2];
        Time.hour = buff[3];
        Time.min = buff[4];
        Time.sec = buff[5];

        this_time = 0x20<<24|buff[0]<<16|buff[1]<<8|buff[2];
        start_time = JTB_CardInfo.appstarttime[0]<<24|JTB_CardInfo.appstarttime[1]<<16|JTB_CardInfo.appstarttime[2]<<8|JTB_CardInfo.appstarttime[3];
        end_time = JTB_CardInfo.appendtime[0]<<24|JTB_CardInfo.appendtime[1]<<16|JTB_CardInfo.appendtime[2]<<8|JTB_CardInfo.appendtime[3];
        LOGI("this_time:%x \n", this_time);
        LOGI("start_time:%x \n", start_time);
        LOGI("end_time:%x \n", end_time);
        if(this_time>=start_time && this_time<=end_time)
        {
            status = 0;
            break;
        }
        if(start_time> this_time)
        {
            status = 1;
            break;
        }
        else if(end_time<this_time)
        {
            status = 2;
            break;
        }
        
    }
    return status;
#endif
}

#endif
