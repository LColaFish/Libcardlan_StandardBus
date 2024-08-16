#ifndef  _CPUCARD_HANDLER_H_
#define  _CPUCARD_HANDLER_H_

#include "../gui/RC500.h"
#include "custom_config.h"


#define HANDLE_OK_CONSUME        2     /*消费成功*/
#define HANDLE_OK_INTERCHANGE_TIME   1 /*换乘时间*/
#define HANDLE_OK                0
#define HANDLE_ERR              -1
#define HANDLE_ERR_ACTION       -2
#define HANDLE_ERR_CMD          -3
#define HANDLE_ERR_MAC          -4
#define HANDLE_ERR_FORMAT       -5            //卡片格式进程标志错误
#define HANDLE_ERR_NOT_CONTINUE -6
#define HANDLE_ERR_OVER_LIMIT   -7
#define HANDLE_ERR_OVERDUE      -8
#define HANDLE_ERR_CONSUME_TIME -9
#define HANDLE_ERR_BLACKLIST_CARD     -10
#define HANDLE_ERR_NOT_SUPPORT  -11
#define HANDLE_ERR_NOT_ENOUGH   -12            //金额不够
#define HANDLE_ERR_NOT_ALLOW    -13
#define HANDLE_ERR_NOT_FOUND    -13
#define HANDLE_ERR_NOT_IMPLEMENTS     -14
#define HANDLE_ERR_PARAM_LIST   -16
#define HANDLE_ERR_ARG_INVAL    -17  
#define HANDLE_ERR_PATH         -19
#define HANDLE_ERR_ADDRESS      -20
#define HANDLE_ERR_FULL         -21
#define HANDLE_ERR_AUTH         -23
#define HANDLE_ERR_SWIPE_CARD   -24
#define HANDLE_ERR_RECORD_PATH  -25
#define HANDLE_ERR_UPDATE       -28
#define HANDLE_ERR_UNKNOW_APP   -29
#define HANDLE_ERR_UNKNOW_STATUS     -40
#define HANDLE_ERR_READ_CARD    -41
#define HANDLE_ERR_OPEN         -44
#define HANDLE_ERR_NOT_FOUND_APP     -45
#define HANDLE_ERR_NOT_FOUND_PSAM    -46
#define HANDLE_ERR_LOCK_APP     -47
#define HANDLE_ERR_FORMAT_TIME  -48
#define HANDLE_ERR_START_TIME   -49
#define HANDLE_ERR_FLAG         -50
#define HANDLE_ERR_COSUME       -54
#define HANDLE_ERR_REFUSE       -60
#define HANDLE_ERR_NOT_CONNECT  -66   
#define HANDLE_ERR_NOT_RESPOND  -67 
#define HANDLE_ERR_TIMEOUT      -70          

typedef struct _CPUCARD_T{
    int (*f_read_info)          (struct _CPUCARD_T *self,CardInformCPU *pt_info);
    int (*f_handle_card)        (struct _CPUCARD_T *self,CardInformCPU *pt_info);

    int (*f_handle_consume_card)(struct _CPUCARD_T *self,CardInformCPU *pt_info);   
    int (*f_before_consume_card)(struct _CPUCARD_T *self,CardInformCPU *pt_info);
    int (*f_consume_card)       (struct _CPUCARD_T *self,CardInformCPU *pt_info,unsigned int Money);
    int (*f_after_consume_card) (struct _CPUCARD_T *self,CardInformCPU *pt_info);    
    int (*f_save_record)        (struct _CPUCARD_T *self,CardInformCPU *pt_info);
    int (*f_get_consume_value)  (struct _CPUCARD_T *self,CardInformCPU *pt_info,int *p_value);
    int (*f_check_info)         (struct _CPUCARD_T *self,CardInformCPU *pt_info);
    int (*f_check_permission)   (struct _CPUCARD_T *self,CardInformCPU *pt_info);  
    int (*f_check_comsume_time) (struct _CPUCARD_T *self,CardInformCPU *pt_info);  
    int (*f_check_Interchange_time) (struct _CPUCARD_T *self,CardInformCPU *pt_info);
    int (*f_check_blacklist)    (struct _CPUCARD_T *self,CardInformCPU *pt_info);
    int (*f_check_balance)      (struct _CPUCARD_T *self,CardInformCPU *pt_info,int value);
    int (*f_check_swipe_time)   (struct _CPUCARD_T *self,CardInformCPU *pt_info);    
    int (*f_display)            (struct _CPUCARD_T *self,CardInformCPU *pt_info,int index,void *arg);
    int (*f_music)              (struct _CPUCARD_T *self,CardInformCPU *pt_info,int index,void *arg);

}CPUCARD_T;


int apdu_cmd4(unsigned char CLA, unsigned char INS,unsigned char P1,unsigned char P2,unsigned char Lc, unsigned char *data,unsigned char Le,unsigned char *p_rev_buffer);
int apdu_cmd3(unsigned char CLA, unsigned char INS,unsigned char P1,unsigned char P2,unsigned char Lc, unsigned char *data,unsigned char *p_rev_buffer);
int apdu_cmd2(unsigned char CLA, unsigned char INS,unsigned char P1,unsigned char P2,unsigned char Le ,unsigned char *p_rev_buffer);
int apdu_cmd1(unsigned char CLA, unsigned char INS,unsigned char P1,unsigned char P2,unsigned char *p_rev_buffer);

#endif // _CPUCARD_HANDLER_H_

