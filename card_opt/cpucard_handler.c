#include "cpucard_handler.h"
#include "../gui/InitSystem.h"
#include "../bzdes/stades.h"
#include "../psam/psam.h"
#include <stdio.h>
#include "Custom_record.h"
#include "custom_blacklist.h"
#include "../Display/fbtools.h"
#include "../gui/GeneralConfig.h"
#include "../parameter/ConsumpPara.h"

#define TARGET_DEBUG
extern LongUnon Fixvalue;        //�趨�̶�����ֵʹ�õı���

static int send_apdu_cmd(const unsigned char *cmd, int cmd_size,unsigned char *buff);

static int handle_consume_card(CPUCARD_T *self,CardInformCPU *pt_info);
static int handle_card(CPUCARD_T *self,CardInformCPU *pt_info);
static int update_sys_hostvalue(CPUCARD_T *self,CardInformCPU *pt_CardLanCPU);
static int display(CPUCARD_T *self,CardInformCPU *pt_CardLanCPU,int index,void *arg);
static int music(CPUCARD_T *self,CardInformCPU *pt_CardLanCPU,int index,void *arg);
static int check_info(CPUCARD_T *self,CardInformCPU *pt_CardLanCPU);
static int check_balance(CPUCARD_T *self ,CardInformCPU *pt_CardLanCPU,int consume_value);
static int check_Interchange_time(CPUCARD_T *self,CardInformCPU *pt_CardLanCPU);


extern unsigned char LCDKeepDisp;
extern unsigned char CardTwo;
extern struct timeval LCDKeepStartTime;

#define DBG_DATA_PRINTF printf;

static void menu_print(char *buffer, int length)
{
    int i;

    for (i = 0; i < length; i++)
    {
        DBG_DATA_PRINTF("%02X ", *(buffer + i));
        if ((i + 1) % 8 == 0) DBG_DATA_PRINTF(" ");
        if ((i + 1) % 16 == 0) DBG_DATA_PRINTF("\n");
    }
    DBG_DATA_PRINTF("\n");
}

static int print_SW_error(unsigned char SW[2])
{
    printf("SW[0]:0x%2X SW[1]:0x%2X \n",SW[0], SW[1]);
    if(SW[0] == 0x62 && SW[1] == 0x00) { printf("���� ��Ϣδ�ṩ\n");    return 0;}
    if(SW[0] == 0x62 && SW[1] == 0x81) { printf("���� �������ݿ���\n");    return 0;}
    if(SW[0] == 0x62 && SW[1] == 0x82) { printf("���� �ļ�����С��Le\n");    return 0;}
    if(SW[0] == 0x62 && SW[1] == 0x83) { printf("���� ѡ�е��ļ���Ч\n");    return 0;}
    if(SW[0] == 0x62 && SW[1] == 0x84) { printf("���� FCI��ʽ��P2ָ���Ĳ���\n");    return 0;}
    if(SW[0] == 0x63 && SW[1] == 0x00) { printf("���� ����ʧ��\n");    return 0;}
    if(SW[0] == 0x63 && (SW[1]&0xF0) == 0xC0) { printf("���� У��ʧ��(�������Դ���:%d)",SW[1]&0xF);}
    if(SW[0] == 0x64 && SW[1] == 0x00) { printf("״̬��־λû�б�\n");    return 0;}
    if(SW[0] == 0x65 && SW[1] == 0x81) { printf("�ڴ�ʧ��\n");    return 0;}
    if(SW[0] == 0x67 && SW[1] == 0x00) { printf("���ȴ���\n");    return 0;}

    if(SW[0] == 0x68 && SW[1] == 0x82) { printf("��֧�ְ�ȫ����\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x81) { printf("�������ļ��ṹ�����ݣ���ǰ�ļ��������ļ�\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x82) { printf("��������(AC)�����㣬û��У��PIN\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x83) { printf("���Ŀ��ѱ�����\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x84) { printf("�������Ч�����õ�������Ч\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x85) { printf("ʹ������������\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x86) { printf("����������ִ������(����������INS�д�)\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x87) { printf("MAC��ʧ\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x88) { printf("MAC����ȷ\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x8D) { printf("����\n");    return 0;}

    if(SW[0] == 0x6A && SW[1] == 0x80) { printf("�������������ȷ\n");    return 0;}
    if(SW[0] == 0x6A && SW[1] == 0x81) { printf("���ܲ�֧��,����������Ŀ¼��Ч��Ӧ������\n");    return 0;}
    if(SW[0] == 0x6A && SW[1] == 0x82) { printf("���ļ�δ�ҵ�\n");    return 0;}
    if(SW[0] == 0x6A && SW[1] == 0x83) { printf("�ü�¼δ�ҵ�\n");    return 0;}
    if(SW[0] == 0x6A && SW[1] == 0x84) { printf("�ļ�Ԥ���ռ䲻��\n");    return 0;}
    if(SW[0] == 0x6A && SW[1] == 0x86) { printf("P1��P2����ȷ\n");    return 0;}
    if(SW[0] == 0x6A && SW[1] == 0x88) { printf("��������δ�ҵ�\n");    return 0;}

    if(SW[0] == 0x6B && SW[1] == 0x00) { printf("��������\n");    return 0;}
    if(SW[0] == 0x6C)                  { printf("Le���ȴ���ʵ�ʳ����� %d",SW[1]);}
    if(SW[0] == 0x6E && SW[1] == 0x00) { printf("��֧�ֵ��ࣺCLA�д�\n");    return 0;}
    if(SW[0] == 0x6F && SW[1] == 0x00) { printf("������Ч\n");    return 0;}
    if(SW[0] == 0x6F && SW[1] == 0x01) { printf("�����ж�\n");    return 0;}
    if(SW[0] == 0x6D && SW[1] == 0x00) { printf("��֧�ֵ�ָ�����\n");    return 0;}
    if(SW[0] == 0x93 && SW[1] == 0x01) { printf("���Ŀ�����\n");    return 0;}
    if(SW[0] == 0x93 && SW[1] == 0x02) { printf("MAC2����\n");    return 0;}
    if(SW[0] == 0x93 && SW[1] == 0x03) { printf("Ӧ�ñ���������\n");    return 0;}
    if(SW[0] == 0x94 && SW[1] == 0x01) { printf("���Ŀ�����\n");    return 0;}
    if(SW[0] == 0x94 && SW[1] == 0x02) { printf("���׼������ﵽ���ֵ\n");    return 0;}
    if(SW[0] == 0x94 && SW[1] == 0x03) { printf("��Կ������֧��\n");    return 0;}
    if(SW[0] == 0x94 && SW[1] == 0x06) { printf("����MAC������\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x00) { printf("���ܴ���\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x01) { printf("������ܣ���Ч״̬��\n");    return 0;}
    if(SW[0] == 0x61) { printf("���� �跢GET RESPONSE���� ��ȡָ��00C00000%2x",SW[1]);}
    if(SW[0] == 0x66 && SW[1] == 0x00) { printf("����ͨѶ��ʱ\n");    return 0;}
    if(SW[0] == 0x66 && SW[1] == 0x01) { printf("�����ַ���ż��\n");    return 0;}
    if(SW[0] == 0x66 && SW[1] == 0x02) { printf("У��Ͳ���\n");    return 0;}
    if(SW[0] == 0x66 && SW[1] == 0x03) { printf("���� ��ǰDF�ļ���FCI\n");    return 0;}
    if(SW[0] == 0x66 && SW[1] == 0x04) { printf("���� ��ǰDF����SF��KF\n");    return 0;}
    if(SW[0] == 0x6E && SW[1] == 0x81) { printf("Ƭ���뿪\n");    return 0;}

    printf("δ֪����\n"); 
    return -1;
}


/*
    ���4
    �����ױ� + Lc�ֶ� + �����ֶ� + Le�ֶ�
*/
int apdu_cmd4(unsigned char CLA, unsigned char INS,unsigned char P1,unsigned char P2,unsigned char Lc, unsigned char *data,unsigned char Le,unsigned char *p_rev_buffer)
{
    if(p_rev_buffer == NULL)
    {
        return -1;
    }
    unsigned char cmd[256];
    //����CLAΪָ�����INSΪָ���룻P1��P2Ϊ������LcΪData�ĳ��ȣ�LeΪϣ����Ӧʱ�ش�������ֽ���
    cmd[0] = CLA;      
    cmd[1] = INS;    
    cmd[2] = P1;          
    cmd[3] = P2;      
    cmd[4] = Lc;
    if(Lc > 0 && data != NULL)
    {
        memcpy(&cmd[5], data, Lc); 
    }
    cmd[5 + Lc] = Le;   
    return send_apdu_cmd(cmd, 5 + Lc + 1 , p_rev_buffer);
}
/*
    ���3
    �����ױ� +     Lc�ֶ� +   �����ֶ�
*/
int apdu_cmd3(unsigned char CLA, unsigned char INS,unsigned char P1,unsigned char P2,unsigned char Lc, unsigned char *data,unsigned char *p_rev_buffer)
{
    if(p_rev_buffer == NULL)
    {
        return -1;
    }
    unsigned char cmd[256];
    //����CLAΪָ�����INSΪָ���룻P1��P2Ϊ������LcΪData�ĳ��ȣ�LeΪϣ����Ӧʱ�ش�������ֽ���
    cmd[0] = CLA;      
    cmd[1] = INS;    
    cmd[2] = P1;          
    cmd[3] = P2;      
    cmd[4] = Lc;
    if(Lc > 0 && data != NULL)
    {
        memcpy(&cmd[5], data, Lc); 
    }

    return send_apdu_cmd(cmd, 5 + Lc , p_rev_buffer);
}
/*
    ���2
    �����ױ� +     Le�ֶ�
*/
int apdu_cmd2(unsigned char CLA, unsigned char INS,unsigned char P1,unsigned char P2,unsigned char Le ,unsigned char *p_rev_buffer)
{
    if(p_rev_buffer == NULL)
    {
        return -1;
    }
    unsigned char cmd[5];
    //����CLAΪָ�����INSΪָ���룻P1��P2Ϊ������LcΪData�ĳ��ȣ�LeΪϣ����Ӧʱ�ش�������ֽ���
    cmd[0] = CLA;      
    cmd[1] = INS;    
    cmd[2] = P1;          
    cmd[3] = P2;      
    cmd[4] = Le;
    return send_apdu_cmd(cmd, 5 , p_rev_buffer);
}
/*
    ���1
    �����ױ�
*/
int apdu_cmd1(unsigned char CLA, unsigned char INS,unsigned char P1,unsigned char P2,unsigned char *p_rev_buffer)
{
    if(p_rev_buffer == NULL)
    {
        return -1;
    }
    unsigned char cmd[5];
    //����CLAΪָ�����INSΪָ���룻P1��P2Ϊ������LcΪData�ĳ��ȣ�LeΪϣ����Ӧʱ�ش�������ֽ���
    cmd[0] = CLA;      
    cmd[1] = INS;    
    cmd[2] = P1;          
    cmd[3] = P2;      
    cmd[4] = 0x00;
    return send_apdu_cmd(cmd, sizeof(cmd) , p_rev_buffer);
}

static int send_apdu_cmd(const unsigned char *cmd, int cmd_size,unsigned char *buff)
{
    extern int mf_fd;
    static int  receive_len[1] = {0};
    int result;

    if(cmd == NULL || buff == NULL)
    {
        printf("[%s %d] cmd:%08X buff:%08X \n",__FUNCTION__,__LINE__,cmd,buff);
        return -1;
    }
#if defined(TARGET_DEBUG)   
    printf("\n------------%s start-------------\n",__FUNCTION__);
    printf("  ---mifare_write  :\n");
    menu_print(cmd, cmd_size);
#endif
    result = write(mf_fd,cmd,cmd_size);
    if(result != 0)
    {
       printf("[%s %d] mifare_write fail result %02X \n",__FUNCTION__,__LINE__,result);
       return 0;
    }
    memset(receive_len,0,sizeof(receive_len));

    ioctl(mf_fd, FIFO_RCV_LEN, receive_len);
    result = read(mf_fd, buff, receive_len[0]);
    if(receive_len[0] < 2)
    {
        printf("[%s %d] result:%02X\n",__FUNCTION__,__LINE__,receive_len[0]);
        return receive_len[0];
    }
    else if(buff[receive_len[0] - 2] != 0x90 || buff[receive_len[0] -1] != 0x00)
    {
        unsigned char SW[2];
        SW[0] = buff[receive_len[0] - 2] ;
        SW[1] = buff[receive_len[0] -1] ;
        print_SW_error(SW);
#if defined(TARGET_DEBUG)  
        printf("--------------end result : %d----------------------\n\n",receive_len[0] );
#endif
        return receive_len[0] ;
    }
#if defined(TARGET_DEBUG)  
    printf("  ---mifare_read  :\n");
    menu_print(buff, receive_len[0] );    
    printf("--------------end----------------------\n\n");
#endif
    return receive_len[0] ;
}

static int before_consume_card(CPUCARD_T *self,CardInformCPU *pt_CardLanCPU)
{
    int ret;

    /*��¼����ʱ��*/
    {
        /* ע�� �����ȡutcʱ�������� */
        time_t t;
        struct tm * tm;
        time (&t);
        tm = localtime (&t);        
        set_config(E_COSUME_TIME, tm, sizeof(struct tm));
    }
    /*��鿨Ƭ��Ϣ*/
    {
        if(self->f_check_info != NULL)
        {
             ret = self->f_check_info(self,pt_CardLanCPU);
             if(ret != HANDLE_OK)
             {
                 printf("[%s %d] error result %d\n",__FUNCTION__,__LINE__,ret);
                 return ret;
             }
        }
    }

    /*�������Ѳ���*/
    {
        int consume_value = 0;
        if(self->f_get_consume_value != NULL)
        {
            ret = self->f_get_consume_value(self,pt_CardLanCPU,&consume_value);
            if(ret != HANDLE_OK) 
            {    
                printf("[%s %d] error %d \n",__FUNCTION__,__LINE__,ret);
                return ret;
            }
        }

        /*��鵱ǰ����Ƿ��㹻�ۿ�*/
        if(self->f_check_balance != NULL)
        {
            ret = self->f_check_balance(self,pt_CardLanCPU,consume_value);
            if(ret != HANDLE_OK)
            {
                printf("[%s %d] error %d \n",__FUNCTION__,__LINE__,ret);
                return ret;
            }
        }  
    }

    return HANDLE_OK;
}

static int after_consume_card(CPUCARD_T * self,CardInformCPU *pt_CardLanCPU)
{
    int ret = 0;
    int HostValue = 0;
    /*�������Ѻ����*/
    {
        ret = get_config(E_HOSTVALUE,&HostValue, NULL);
        if(ret != 0)
        {
            printf("[%s %d] error result %d\n",__FUNCTION__,__LINE__,ret);
            return -1;      
        }
        IncSerId();
        LongUnon aftermoney;
        unsigned int beforeMoney = pt_CardLanCPU->beforemoney[0]<<24|pt_CardLanCPU->beforemoney[1]<<16|pt_CardLanCPU->beforemoney[2]<<8|pt_CardLanCPU->beforemoney[3];
        aftermoney.i = beforeMoney - HostValue;
        memcpy(pt_CardLanCPU->aftermoney,aftermoney.longbuf,4);
    }
    
    /*��������¼*/
    {
        if(self->f_save_record != NULL)
        {
            ret = self->f_save_record(self,pt_CardLanCPU);
            if(ret != HANDLE_OK)
            {
                return ret;
            }
        }
    }
    return HANDLE_OK;
}

static int check_permission(CPUCARD_T *self,CardInformCPU *pt_CardLanCPU)
{
    CONSUMPARA param;
    int ret = 0;

    ret = Get_ConsumParameter(CONSUME_PARA_PATH,&param);
    if( ret != 0 )
    {
        return HANDLE_ERR_NOT_SUPPORT;
    }
    {
        int i = 0;
        int found = 0;
        for(i = 0 ; i < param.cardTypeNum ; i++)
        {
            if( param.rateArr[i].logicCardType == pt_CardLanCPU->cardtype)
            {
                found = 1;
                break;
            }
        }
        if(found == 0)
        {
            return HANDLE_ERR_NOT_SUPPORT;
        }
    }
    return HANDLE_OK;
}

int check_swipecard_time(CPUCARD_T *self,CardInformCPU *pt_CardLanCPU)
{
    int ret = 0;
    unsigned char buf,i,status = 1;
    unsigned int min,hour,sec;
    LongUnon JackArm,now,tradetime;
    unsigned char *p,*q;
    unsigned int total_sec1, total_sec2;  
    SysTime Time;
    
    /*�ж��Ƿ�ͬһ̨��*/
    {
        extern unsigned char PsamNum[6];
        menu_print(PsamNum, 6);
        menu_print(pt_CardLanCPU->deviceNO, 6);
        if(memcmp(PsamNum,pt_CardLanCPU->deviceNO,6) != 0)
        {
            return HANDLE_OK;
        }
    }

    {
        struct tm tm;
        get_config(E_COSUME_TIME, &tm,NULL);
        Time.year = HEX2BCD((unsigned char)tm.tm_year-100);
        Time.month= HEX2BCD(tm.tm_mon+1);
        Time.day  = HEX2BCD(tm.tm_mday);
        Time.hour = HEX2BCD(tm.tm_hour);
        Time.min  = HEX2BCD(tm.tm_min);
        Time.sec  = HEX2BCD(tm.tm_sec);
        menu_print(&Time, sizeof(SysTime));    
    }

    if(pt_CardLanCPU->tradedate[1] != Time.year || pt_CardLanCPU->tradedate[2] != Time.month || pt_CardLanCPU->tradedate[3] != Time.day)
    {
        printf("[%s %d] error  \n",__FUNCTION__,__LINE__);
        return HANDLE_OK;
    }

    now.i = 0;
    now.longbuf[1]     = Time.sec;
    now.longbuf[2]     = Time.min;
    now.longbuf[3]     = Time.hour;
    tradetime.i = 0;
    tradetime.longbuf[1] = pt_CardLanCPU->tradetime[2];
    tradetime.longbuf[2] = pt_CardLanCPU->tradetime[1];
    tradetime.longbuf[3] = pt_CardLanCPU->tradetime[0];

    if(now.i < tradetime.i)
    {
        printf("[%s %d] error now.i %d tradetime.i :%d \n",__FUNCTION__,__LINE__,now.i , tradetime.i);
        return HANDLE_ERR_OVER_LIMIT;
    }

    for(i = 0 ; i < 4; i++)now.longbuf[i]  = BCD2HEX(now.longbuf[i]);
    for(i = 0 ; i < 4; i++)tradetime.longbuf[i] = BCD2HEX(tradetime.longbuf[i]);

    total_sec1 =  now.longbuf[1] +  now.longbuf[2]*60 +  now.longbuf[3]*3600;
    total_sec2 =  tradetime.longbuf[1] +  tradetime.longbuf[2]*60 +  tradetime.longbuf[3]*3600;

    if(total_sec1 < total_sec2)
    {
        printf("[%s %d] error  \n",__FUNCTION__,__LINE__);
        return HANDLE_ERR_OVER_LIMIT;
    }
   
    JackArm.i = total_sec1-total_sec2;

    if(JackArm.i > 86400)   
    {
        printf("[%s %d] error  \n",__FUNCTION__,__LINE__);
        return HANDLE_ERR_OVER_LIMIT;
    }
    
    {
        LongUnon Buf;
        unsigned char outpara [ 512 ];

        ret = Get_Parameter(CONSUME_PARA_PATH, 0, outpara);
        if(ret != 0 )
        {
            printf("[%s %d] error  \n",__FUNCTION__,__LINE__);
            return HANDLE_ERR;
        }
        for(i = 0; i < 32; i++)
        {
            if((outpara[i*5] == 0x55)&&(outpara[i*5+1] == 0xAA))
            {
                /*������û��ʱ����*/
                return HANDLE_OK;
            }

            if(outpara[i*5] != pt_CardLanCPU->cardtype )
            {
               continue;
            }
            memcpy(Buf.longbuf,outpara + i * 5 + 1 , 4);
            printf("[%s %d]  Buf.i:%d sec: %d \n",__FUNCTION__,__LINE__,Buf.i,JackArm.i);
            if(JackArm.i < Buf.i)
            {
                printf("[%s %d] error  Buf.i:%d sec: %d \n",__FUNCTION__,__LINE__,Buf.i,sec);
                return HANDLE_ERR_CONSUME_TIME;
            }
            break;
        }
    }

    return HANDLE_OK;    
}

static int check_Interchange_time(CPUCARD_T *self,CardInformCPU *pt_CardLanCPU)
{
    int ret = 0;
    unsigned char buf,i,status = 1;
    unsigned int min,hour,sec;
    LongUnon JackArm,now,Interchange_time;
    unsigned char *p,*q;
    unsigned int total_sec1, total_sec2;  
    SysTime Time;
  
    {
        struct tm tm;
        get_config(E_COSUME_TIME, &tm,NULL);
        Time.year = HEX2BCD((unsigned char)tm.tm_year-100);
        Time.month= HEX2BCD(tm.tm_mon+1);
        Time.day  = HEX2BCD(tm.tm_mday);
        Time.hour = HEX2BCD(tm.tm_hour);
        Time.min  = HEX2BCD(tm.tm_min);
        Time.sec  = HEX2BCD(tm.tm_sec);
    }

    if(pt_CardLanCPU->Interchange_time[1] != Time.year || pt_CardLanCPU->Interchange_time[2] != Time.month || pt_CardLanCPU->Interchange_time[3] != Time.day)
    {
        return HANDLE_OK;
    }


    now.i = 0;
    now.longbuf[1]     = Time.sec;
    now.longbuf[2]     = Time.min;
    now.longbuf[3]     = Time.hour;
    Interchange_time.i = 0;
    Interchange_time.longbuf[1] = pt_CardLanCPU->Interchange_time[6];
    Interchange_time.longbuf[2] = pt_CardLanCPU->Interchange_time[5];
    Interchange_time.longbuf[3] = pt_CardLanCPU->Interchange_time[4];

    if(now.i < Interchange_time.i)
    {
        return HANDLE_ERR_OVER_LIMIT;
    }

    for(i = 0 ; i < 4; i++)now.longbuf[i]  = BCD2HEX(now.longbuf[i]);
    for(i = 0 ; i < 4; i++)Interchange_time.longbuf[i] = BCD2HEX(Interchange_time.longbuf[i]);

    total_sec1 =  now.longbuf[1] +  now.longbuf[2]*60 +  now.longbuf[3]*3600;
    total_sec2 =  Interchange_time.longbuf[1] +  Interchange_time.longbuf[2]*60 +  Interchange_time.longbuf[3]*3600;

    if(total_sec1 < total_sec2)
    {
        return HANDLE_ERR_OVER_LIMIT;
    }
   
    JackArm.i = total_sec1-total_sec2;

    if(JackArm.i > 86400)   
    {
        return HANDLE_ERR_OVER_LIMIT;
    }

/*    
    for(i = 0; i < 64; i++)
    {
        memcpy(CsnBuf,NandBuf+(i * 8),8);
        if((CsnBuf[0] == 0x55)&&(CsnBuf[1] == 0xAA)) break;
        Tim.i  = 0;
        INbff.i  = 0;
        memcpy(Tim.intbuf,CsnBuf+1,2);
        memcpy(INbff.intbuf,CsnBuf+3,2);
    //printf(" ValueDiscounts time para=%d, %d, %d\n", Tim.i, INbff.i, g_FgDiscntRegTime);
        if((CsnBuf[0] == Type)&&(Tim.i <= sec)&&(sec < INbff.i))
        {
          
            status = CheckSwipeCardAfter(); //20170323 status = 1;
            
            if (status)  //��һ��ת�˴���
            {
                    INbff.i = 0;
                    memcpy(INbff.intbuf, CsnBuf+6, 2);
                    HostValue.i = ((HostValue.i*CsnBuf[5])/100);
                    if(HostValue.i >= INbff.i)HostValue.i  = HostValue.i - INbff.i;
                    status = 0xaa;
                    i = 200;
            }
            else
            {
                status = 0;  //���������ڲ����״�ˢ����������
            }
            break;
        }
    }
*/
    return HANDLE_OK;    
}


static int check_info(CPUCARD_T *self , CardInformCPU *pt_CardLanCPU)
{
    int ret = MI_OK;
    /*Ȩ��*/
    if(self->f_check_permission != NULL)
    {
        ret = self->f_check_permission(self,pt_CardLanCPU);
        if(ret < HANDLE_OK) 
        {
            printf("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
    }
    
    //��ǰ��û�кϷ�������ʱ���
    if(self->f_check_comsume_time != NULL)
    {
        ret = self->f_check_comsume_time(self,pt_CardLanCPU);
        if(ret < HANDLE_OK) 
        {
            printf("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
    }
    
    //�жϿ�Ƭ�Ƿ��Ǻ������� 
    if(self->f_check_blacklist != NULL)
    {
        ret = self->f_check_blacklist(self,pt_CardLanCPU);
        if(ret < HANDLE_OK) 
        {
            printf("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
    }

    /*�ж���������*/
    {
        if(0)//pt_CardLanCPU->tradetype != 0x09 && pt_CardLanCPU->tradetype != 0x06)
        {
            printf("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
            return HANDLE_ERR_COSUME;
        }
    } 

    //�жϿ�Ƭʱ����
    if(self->f_check_swipe_time != NULL)
    {
        ret = self->f_check_swipe_time(self,pt_CardLanCPU);
        if(ret < HANDLE_OK) 
        {
            printf("[%s %d] error  ret : %d \n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
    }       

    return HANDLE_OK;
}
static int get_consume_value(CPUCARD_T *self ,CardInformCPU *pt_CardLanCPU,int *p_consume_value)
{
    extern LongUnon HostValue;
    extern LongUnon DecValue;
    extern LongUnon Fixvalue;

    if(p_consume_value == NULL || p_consume_value == NULL)
    {
        return HANDLE_ERR_ARG_INVAL;
    }
    
    int ret = 0;
    unsigned int hostvalue = 0;
    DecValue.i = 0;
    if(Fixvalue.i > 0)
    {
        HostValue.i = DecValue.i = Fixvalue.i;
        return HANDLE_OK;
    }

    ret = Get_TicketPrice(pt_CardLanCPU->cardtype , &hostvalue);
    if(ret != 0)
    {
        printf("[%s %d] error  \n",__FUNCTION__,__LINE__);
        return HANDLE_ERR_NOT_SUPPORT;
    }
    
    /*ת���Ż�*/
    if(self->f_check_Interchange_time != NULL)
    {
        ret = self->f_check_Interchange_time(self,pt_CardLanCPU);
        if(ret ==  HANDLE_OK)
        {
            SysTime Time;
            {
                struct tm tm;
                get_config(E_COSUME_TIME, &tm,NULL);
                Time.year = HEX2BCD((unsigned char)tm.tm_year-100);
                Time.month= HEX2BCD(tm.tm_mon+1);
                Time.day  = HEX2BCD(tm.tm_mday);
                Time.hour = HEX2BCD(tm.tm_hour);
                Time.min  = HEX2BCD(tm.tm_min);
                Time.sec  = HEX2BCD(tm.tm_sec);
            }
            /*������λ����Żݿ�ʼʱ�� ������Ӧ�������Ѻ�Ÿ��¸������Ѽ�¼*/
            pt_CardLanCPU->Interchange_time[0] = 20;
            pt_CardLanCPU->Interchange_time[1] = Time.year;
            pt_CardLanCPU->Interchange_time[2] = Time.month;
            pt_CardLanCPU->Interchange_time[3] = Time.day;
            pt_CardLanCPU->Interchange_time[4] = Time.hour;
            pt_CardLanCPU->Interchange_time[5] = Time.min;
            pt_CardLanCPU->Interchange_time[6] = Time.sec; 
            pt_CardLanCPU->Interchange_count[0] = 0;
        }
        else if(ret == HANDLE_OK_INTERCHANGE_TIME )
        {
            /*���д����Ż�*/     
            pt_CardLanCPU->Interchange_count[0]++;
        }
        else
        {
            return ret;
        }
    }
    
    ret = set_config(E_HOSTVALUE,&hostvalue,sizeof(hostvalue));
    if(ret != 0)
    {
        printf("[%s %d] error  \n",__FUNCTION__,__LINE__);
        return HANDLE_ERR;
    }
    DecValue.i = Get_LocalParaFixValue();
    *p_consume_value = hostvalue;

    return HANDLE_OK;
}


static int check_balance(CPUCARD_T *self ,CardInformCPU *pt_CardLanCPU,int consume_value)
{
    int ret = 0;
    unsigned int beforeMoney;
    beforeMoney = pt_CardLanCPU->beforemoney[0]<<24|pt_CardLanCPU->beforemoney[1]<<16|pt_CardLanCPU->beforemoney[2]<<8|pt_CardLanCPU->beforemoney[3];
    if(beforeMoney > 100000)
    {
        return HANDLE_ERR_OVER_LIMIT;
    }

    if(beforeMoney < consume_value)
    {
        return HANDLE_ERR_NOT_ENOUGH;
    }
    return HANDLE_OK;
}

static int consume_music(unsigned char cardtype, unsigned char aftermoney[4],int HostValue)
{
    PlayMusic(32, 0);
    //������
    {
        LongUnon Dislong;
        memcpy(Dislong.longbuf,aftermoney,4);
        if(HostValue != 0 && FindCardValue(Dislong.i) == 0)
        {
            extern volatile unsigned char PlayMusicFinishFlag;
            // �ȴ����������������
            while(!PlayMusicFinishFlag)
                usleep(100*1000);
            // ���� ���࣬���ֵ        
            PlayMusic(188, 0);
        }
    }
    return 0;
}
static int consume_display(unsigned char CSN[4], unsigned char beforemoney[4],int HostValue)
{
    int ret = 0;
    {
        char DisBuf1[80];
        TextOut(100,25, "���ѳɹ�");
        sprintf(DisBuf1,"����:%02X%02X%02X%02X",CSN[0],CSN[1],CSN[2],CSN[3]);
        TextOut(30,70,DisBuf1);
    }
    
    {
        char DisBuf1[80];
        char dig[5];
        extern int mg_fd;
        memset(DisBuf1,0,sizeof(DisBuf1));
        strcpy(DisBuf1,"���ѽ��:");
        MoneyValue(DisBuf1+9,HostValue);
        MoneyValue1(dig,HostValue);              
        write(mg_fd,dig,5);
        TextOut(30,110,DisBuf1);                
    }

    {
        char DisBuf1[80];
        LongUnon Dislong;
        memset(DisBuf1,0,sizeof(DisBuf1));
        strcpy(DisBuf1,"�ֽ����:");
        Dislong.i = (beforemoney[0]<<24|beforemoney[1]<<16|beforemoney[2]<<8|beforemoney[3])-HostValue;
        MoneyValue(DisBuf1+9,Dislong.i);
        TextOut(30,150,DisBuf1);
    }

    TextOut( 100,200, "ллʹ��");

    return 0;
}

static int music(CPUCARD_T *self,CardInformCPU *pt_CardLanCPU,int index,void *arg)
{
    int ret = 0;
    switch(index)
    {
    case HANDLE_OK_INTERCHANGE_TIME:
        {
            ret = PlayMusic(44, 0);  
        }
        break;
    case HANDLE_OK:
    case HANDLE_OK_CONSUME:
        {   
            consume_music(pt_CardLanCPU->cardtype,pt_CardLanCPU->aftermoney,(int)arg);
        }
        break;
    case HANDLE_ERR_NOT_SUPPORT:
        {
            ret = PlayMusic(30, 0); 
        }
        break;
    case HANDLE_ERR_CONSUME_TIME:
        {
            //ret = PlayMusic(16,0);  
        }
        break;
    case HANDLE_ERR_NOT_ALLOW:
        {
           // ret = return_display1(pt_CardLanCPU->CSN,pt_CardLanCPU->beforemoney);    
        }
        break;
    case HANDLE_ERR_BLACKLIST_CARD:
        {
            ret = PlayMusic(45, 0); 
        }
        break;
    case HANDLE_ERR_MAC:
        {
            ret = PlayMusic(30, 0); 
        }
        break;
    default:
        {
            return HANDLE_ERR_NOT_IMPLEMENTS;
        }
        break;
    }
    if(ret != 0)
    {
        return HANDLE_ERR;
    }
   
    return HANDLE_OK;
}

static int display(CPUCARD_T *self,CardInformCPU *pt_CardLanCPU,int index,void *arg)
{
    LEDL(1);
    SetColor(Mcolor);
    unsigned short color = index < 0 ? Color_red : Color_white;
    SetTextColor(color);
    SetTextSize(32);

    switch(index)
    {
    case HANDLE_OK_CONSUME:
    case HANDLE_OK:
        {   
           int hostvalue = 0;
           get_config(E_HOSTVALUE,&hostvalue,NULL);
           consume_display(pt_CardLanCPU->CSN,pt_CardLanCPU->beforemoney,hostvalue);
        }
        break;
    case HANDLE_ERR_NOT_ENOUGH:
        {
           Err_display(12);
        }
        break;
    case HANDLE_ERR_NOT_SUPPORT:
        {   
            Err_display(11);
        }
        break;
    case HANDLE_ERR_CONSUME_TIME:
        {
            Err_display(22);
        }
        break;
    case HANDLE_ERR_NOT_ALLOW:
        {
            TextOut(0 , 35,"��������");
            if(arg != NULL)
            {
                TextOut(0 , 90, arg); 
            }
            TextOut(0 , 130,"ллʹ��");
        }
        break;
    case HANDLE_ERR_BLACKLIST_CARD:
        {
            Err_display(10);
        }
        break;
    case HANDLE_ERR_CMD:
    case HANDLE_ERR:
        {
            Err_display(24);
        }
        break;
    case HANDLE_ERR_UNKNOW_STATUS:
        {
            Err_display(40);
        }
        break;
    case HANDLE_ERR_START_TIME:
        {
            Err_display(49);
        }
        break;
    case HANDLE_ERR_OVERDUE:
        {
            Err_display(1);
        }
        break;
    case HANDLE_ERR_FORMAT_TIME:
        {
            Err_display(48);
        }
        break;
    case HANDLE_ERR_NOT_FOUND_APP:
        {
            Err_display(45);
        }
        break;
    case HANDLE_ERR_LOCK_APP:
        {
            Err_display(53);
        }
        break;
    case HANDLE_ERR_MAC:
        {
            TextOut(0 , 35, "��Ƭ����");
            TextOut(0 , 90, "MAC����");
            TextOut(0 , 130,"ллʹ��");
        }
        break;
    case HANDLE_ERR_NOT_FOUND_PSAM:
        {
            TextOut(0 , 35,"��������");
            TextOut(0 , 90,"��ȷ��PSAM���Ƿ����");
            TextOut(0 , 130,"ллʹ��");
        }
        break;
    default:
        {   
            /*unknow */
        }
        break;
    }
    LEDL(0);  

    LCDKeepDisp = 1;
	
	gettimeofday(&LCDKeepStartTime, 0);
	CardTwo = 0;
    
    return 0;
}


int Inherit_cpucard(CPUCARD_T *cpucard)
{
    if(cpucard == NULL) return HANDLE_ERR_ARG_INVAL;

    cpucard->f_read_info = NULL;
    cpucard->f_handle_card         = handle_card;
    cpucard->f_handle_consume_card = handle_consume_card;
    cpucard->f_before_consume_card = before_consume_card;
    cpucard->f_after_consume_card  = after_consume_card;
    cpucard->f_get_consume_value   = get_consume_value;

    {
        cpucard->f_check_info           = check_info;
        {
            cpucard->f_check_comsume_time = NULL;
            cpucard->f_check_permission = check_permission;
            cpucard->f_check_swipe_time = check_swipecard_time;
            cpucard->f_check_blacklist  = NULL;
        }   
    }
    cpucard->f_check_balance          = check_balance;
    cpucard->f_check_Interchange_time = check_Interchange_time;

    cpucard->f_consume_card = NULL;
    cpucard->f_save_record  = NULL;
    
    cpucard->f_display = display;
    cpucard->f_music   = music;

    return HANDLE_OK;
}
static int handle_card(CPUCARD_T *self,CardInformCPU *pt_info)
{
    int ret = 0;
    ret = self->f_handle_consume_card(self,pt_info);
    if(self->f_display != NULL)
    {
       self->f_display(self,pt_info,ret,NULL);
    }
    if(self->f_music != NULL)
    {
       self->f_music(self,pt_info,ret,NULL);
    }

    return ret;
}
static int handle_consume_card(CPUCARD_T *self,CardInformCPU *pt_info)
{
    int ret = 0;
    
    if(self->f_before_consume_card != NULL)
    {
        ret = self->f_before_consume_card(self,pt_info);
        if(ret < HANDLE_OK)
        {
            printf("[%s %d] error result %d\n",__FUNCTION__,__LINE__,ret);
            return ret;
        }
    }

    if(self->f_consume_card != NULL)
    {
        int HostValue;
        ret = get_config(E_HOSTVALUE,&HostValue, NULL);
        if(ret != 0)
        {
            printf("[%s %d] error result %d\n",__FUNCTION__,__LINE__,ret);
            return HANDLE_ERR;      
        } 
 
        ret = self->f_consume_card(self,pt_info,HostValue);
        if(ret < HANDLE_OK)
        {
            printf("[%s %d] error result %d\n",__FUNCTION__,__LINE__,ret); 
            return ret;
        }
    }

    if(self->f_after_consume_card != NULL)
    {
        ret = self->f_after_consume_card(self,pt_info);
        if(ret < HANDLE_OK)
        {
            printf("[%s %d] error result %d\n",__FUNCTION__,__LINE__,ret); 
            return ret;
        }
    }
    
    return ret;
}

int register_cpu_handler(int use_jiaotong_stander)
{
    return register_cpu_handler_ex(use_jiaotong_stander);
}

int register_cpu_handler_ex(int index)
{
    return 0;
}


