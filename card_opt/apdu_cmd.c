#include "apdu_cmd.h"
#include <stdio.h>
#include "libcardlan_StandardBus_util.h"

#if defined(ANDROID_CODE_DEBUG)
#define TARGET_ANDROID
#endif
#if defined(NDK_CODE_DEBUG)
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

/*
    分析自发行自定义数据
    来自从应用提供商、发卡行或 IC 卡供应商的 1
    个或多个附加（专用）数据元
*/
int pse_find_FCI_0xBF0C(RECORD_PPSE *p_recordPSE)
{
    int index = 0;

    if(p_recordPSE->tag_0xBF0C_Exist != 1)
    {
        LOGI("[ %s %d ] p_recordPSE->DirDiscretExist\n",__FUNCTION__,__LINE__);
        return -1;        
    }

    while(index < p_recordPSE->tag_0xBF0C_len)/*应用模板*/    
    { 
        if(p_recordPSE->tag_0xBF0C_data[index] != 0x61)
        {
            index++;
            continue;
        }
    
        int tag_0x61_index_start = index;
        int tag_0x61_index_end   = -1;
        int tag_0x61_len = 0;
        APPDATA_T appdata;
        memset(&appdata,0,sizeof(APPDATA_T));  

        {
            unsigned char t = 0 ;
            tag_0x61_len = p_recordPSE->tag_0xBF0C_data[tag_0x61_index_start + 1];
            tag_0x61_index_end = tag_0x61_index_start + t + 2 + tag_0x61_len;
            index += 2;    
            LOGI("[ %s %d ]: tag 0x61 len = %d ,index : %d \n",__FUNCTION__,__LINE__, tag_0x61_len,index);
        }

        while(index < tag_0x61_index_end && index < p_recordPSE->tag_0xBF0C_len) // recordPSE Dir discret
        {
            if(p_recordPSE->tag_0xBF0C_data[index] == 0xFF ||p_recordPSE->tag_0xBF0C_data[index] == 0x00) 
            {
               index++;
               continue;
            }
            else if(p_recordPSE->tag_0xBF0C_data[index] == 0x4f) /*应用标识符（AID）-卡片*/
            {
               index++;
               appdata.AIDLen = p_recordPSE->tag_0xBF0C_data[index];
               index++;
               memcpy(appdata.AID,p_recordPSE->tag_0xBF0C_data+index,appdata.AIDLen);
               index += appdata.AIDLen;
               appdata.AIDExist = 1;
               p_recordPSE->Type=2;
               LOGI("[ %s %d ]: tag 0x4f index : %d \n",__FUNCTION__,__LINE__,index);
            }
            else if(p_recordPSE->tag_0xBF0C_data[index] == 0x50) /*应用标签*/
            {                    
               index++;
               appdata.AppLabelLen = p_recordPSE->tag_0xBF0C_data[index];
               index++;
               memcpy(appdata.AppLabel,p_recordPSE->tag_0xBF0C_data + index,appdata.AppLabelLen);
               index += appdata.AppLabelLen;
               appdata.LabelExist = 1;
               LOGI("[ %s %d ]: tag 0x4f index : %d \n",__FUNCTION__,__LINE__,index);
            }
            else if(!memcmp(p_recordPSE->tag_0xBF0C_data + index,"\x9F\x12",2))  /*应用优先名称*/
            {                 
               index += 2;
               appdata.PreferNameLen=p_recordPSE->tag_0xBF0C_data[index];
               index++;
               memcpy(appdata.PreferName, p_recordPSE->tag_0xBF0C_data + index, appdata.PreferNameLen);
               index += appdata.PreferNameLen;
               appdata.PreferNameExist = 1;
               LOGI("[ %s %d ]: tag 0x9F12 index : %d \n",__FUNCTION__,__LINE__,index);
            }
            else if(p_recordPSE->tag_0xBF0C_data[index] == 0x87)         /*应用优先指示符*/
            {
               index += 2;
               appdata.Priority=p_recordPSE->tag_0xBF0C_data[index];
               index++;
               appdata.PriorityExist=1;
               LOGI("[ %s %d ]: tag 0x87 index : %d \n",__FUNCTION__,__LINE__,index);
            }
            else
            {
                /* skip next */
                unsigned char k;
                k = p_recordPSE->tag_0xBF0C_data[index];
                if((k & 0x1F)==0x1F)
                {
                    index++;
                }
                index++;
                k = p_recordPSE->tag_0xBF0C_data[index];
                index += k + 1;
            }
       }
        
       if(index != tag_0x61_index_end)
       {
           LOGI("[ %s %d ]  error index : %d tag_0x61_index_end : %d \n",__FUNCTION__,__LINE__,index,tag_0x61_index_end);
           return -4;
       }
       
       if(p_recordPSE->Type != 2 || appdata.AIDExist == 0)
       {
           return -4;
       } 

        LOGI("[ %s %d ]  found aid:",__FUNCTION__,__LINE__);
        menu_print(appdata.AID,appdata.AIDLen);
        memcpy(&p_recordPSE->AppList[p_recordPSE->Appnum],&appdata,sizeof(APPDATA_T));
        p_recordPSE->Appnum++; 
    }
    
    if(index != p_recordPSE->tag_0xBF0C_len)
    {
        return -5;
    }  
    return 0;
}

int pse_find_FCI_data(unsigned char *p_buffer, unsigned char buffer_len, RECORD_PPSE *p_recordPSE)
{
    int ret = 0;

    if(p_recordPSE == NULL)
    {
        return -1;
    }
    memset(p_recordPSE,0,sizeof(RECORD_PPSE));
    
    /*FCI 模板*/
    {
        int index = 0;
        DATAELEMENT_T tag_0x6F;
        
        ret = find_tag_data(p_buffer,buffer_len,"\x6F", 1,&tag_0x6F);
        if(ret != 0 || tag_0x6F.bExist == 0)
        {
            return -2;
        }
        
        p_recordPSE->tag_0x6F_Exist = tag_0x6F.bExist;
        p_recordPSE->tag_0x6F_len   = tag_0x6F.data_len;
        memcpy(p_recordPSE->tag_0x6F_data,tag_0x6F.data,tag_0x6F.data_len); 

        while(index < p_recordPSE->tag_0x6F_len)
        {
            switch(p_recordPSE->tag_0x6F_data[index])
            {
            case 0x84:      /* 专 用 文 件（DF）名称         */
                { 
                    DATAELEMENT_T tag_0x84;
                    ret = find_tag_data(&p_recordPSE->tag_0x6F_data[index],0x0E + 1 + 1,"\x84", 1,&tag_0x84);
                    if(ret != 0 || tag_0x84.bExist == 0)
                    {
                            return -2;
                    }

                    p_recordPSE->tag_0x84_Exist = 1;
                    p_recordPSE->Type = 2;
                    p_recordPSE->tag_0x84_len = tag_0x84.data_len;
                    memcpy(p_recordPSE->tag_0x84_data,p_recordPSE->tag_0x6F_data + index,tag_0x84.data_len);      
                    index += tag_0x84.data_len + 2;
                }
                break;
            case 0xA5:      /* FCI 数据专用模板       */
                {
                    DATAELEMENT_T tag_0xA5;

                    ret = find_tag_data(&p_recordPSE->tag_0x6F_data[index],p_recordPSE->tag_0x6F_data[index + 1] + 1 + 1,"\xA5", 1,&tag_0xA5);
                    if(ret != 0 || tag_0xA5.bExist == 0)
                    {
                            return -2;
                    }
                    
                    p_recordPSE->tag_0xA5_Exist = tag_0xA5.bExist;
                    p_recordPSE->tag_0xA5_len = tag_0xA5.data_len;
                    memcpy(p_recordPSE->tag_0xA5_data,tag_0xA5.data,tag_0xA5.data_len);  

                    {
                        int  index = 0;
                        while(index < p_recordPSE->tag_0xA5_len)
                        {
                            if(!memcmp(p_recordPSE->tag_0xA5_data + index,"\xBF\x0C",2))    /*自发行自定义数据*/
                            {
                                unsigned char tag[2] = {0xBF,0x0C};
                                DATAELEMENT_T tag_0xBF0C;
                                
                                ret = find_tag_data(p_recordPSE->tag_0xA5_data + index,p_recordPSE->tag_0xA5_data[index + 2] + 1 + 2,"\xBF\x0C", 2,&tag_0xBF0C);
                                if(ret != 0 || tag_0xBF0C.bExist == 0)
                                {
                                        return -2;
                                }
                                
                                p_recordPSE->tag_0xBF0C_Exist = tag_0xBF0C.bExist;
                                p_recordPSE->tag_0xBF0C_len   = tag_0xBF0C.data_len;
                                memcpy(p_recordPSE->tag_0xBF0C_data,tag_0xBF0C.data,tag_0xBF0C.data_len);  
                                index += tag_0xBF0C.data_len + 3;
                            }
                            else if(!memcmp(p_recordPSE->tag_0xA5_data + index,"\x88",1)) /*目录基本文件的SFI*/
                            {
                                unsigned char tag[1] = {0x88};
                                DATAELEMENT_T tag_0x88;
                                
                                ret = find_tag_data(p_recordPSE->tag_0xA5_data + index,p_recordPSE->tag_0xA5_data[index + 1] + 1 + 1,"\x88",1,&tag_0x88);
                                if(ret != 0 || tag_0x88.bExist == 0)
                                {
                                        return -2;
                                }
                                
                                p_recordPSE->tag_0x88_Exist = tag_0x88.bExist;
                                p_recordPSE->tag_0x88_len   = tag_0x88.data_len;
                                memcpy(p_recordPSE->tag_0x88_data,tag_0x88.data,tag_0x88.data_len); 
                                index += tag_0x88.data_len + 2;
                            }
                            else if(!memcmp(p_recordPSE->tag_0xA5_data + index,"\x5F\x2D",2))/*语言选择*/
                            {
                                DATAELEMENT_T tag_0x5F2D;
                                
                                ret = find_tag_data(p_recordPSE->tag_0xA5_data + index,p_recordPSE->tag_0xA5_data[index + 1],"\x5F\x2D",2,&tag_0x5F2D);
                                if(ret != 0 || tag_0x5F2D.bExist == 0)
                                {
                                        return -2;
                                }
                                
                                p_recordPSE->tag_0x5F2D_Exist = tag_0x5F2D.bExist;
                                p_recordPSE->tag_0x5F2D_len   = tag_0x5F2D.data_len;
                                memcpy(p_recordPSE->tag_0x5F2D_data,tag_0x5F2D.data,tag_0x5F2D.data_len);  
                                index += tag_0x5F2D.data_len + 3;

                            }
                            else if(!memcmp(p_recordPSE->tag_0xA5_data  + index,"\x9F\x11",2))/*发卡行代码表索引*/
                            {
                                unsigned char tag[2] = {0x9F,0x11};
                                DATAELEMENT_T tag_0x9F11;
                                
                                ret = find_tag_data(p_recordPSE->tag_0xA5_data + index,p_recordPSE->tag_0xA5_data[index + 1],"\x9F\x11", 2,&tag_0x9F11);
                                if(ret != 0 || tag_0x9F11.bExist == 0)
                                {
                                        return -2;
                                }
                                
                                p_recordPSE->tag_0x9F11_Exist = tag_0x9F11.bExist;
                                p_recordPSE->tag_0x9F11_len   = tag_0x9F11.data_len;
                                memcpy(p_recordPSE->tag_0x9F11_data,tag_0x9F11.data,tag_0x9F11.data_len);  
                                index += tag_0x9F11.data_len + 3;
                            }
                            else 
                            {
                                index++;
                                continue;
                            }
                        }
                        
                        if(index != p_recordPSE->tag_0xA5_len)
                        {
                            LOGI("[ %s %d ]  error index : %d len : %d \n",__FUNCTION__,__LINE__,index,p_recordPSE->tag_0xA5_len);
                            return -1;
                        }
                    }
                    index += p_recordPSE->tag_0xA5_len + 2;
                }
                break;
            default:
                {
                    index++;
                    continue;
                }
                break;
            }
         }
        
         if(index != p_recordPSE->tag_0x6F_len)
         {
             LOGI("[ %s %d ]  error index : %d len : %d \n",__FUNCTION__,__LINE__,index,p_recordPSE->tag_0x6F_len);
             return -1;
         }
    }

    ret = pse_find_FCI_0xBF0C(p_recordPSE);
    if(ret != 0)
    {
        LOGI("[ %s %d ] error ret = %d  \n",__FUNCTION__,__LINE__,ret);  
        return -4;
    }

    
    return 0;
}


int find_tag_data(unsigned char *buffer, int buffer_len,unsigned char tag[2],int tag_len,DATAELEMENT_T *pt_data)
{
    int index = 0;
    int tag_index_start = index;
    int tag_index_end   = -1;
    int tag_data_len = 0; 

    if( pt_data == NULL || buffer == NULL || tag == NULL || buffer_len < 0 || tag_len < 0 || tag_len <= 0 || tag_len > 2)
    {
        LOGI("[ %s %d ] error\n",__FUNCTION__,__LINE__);
        return -1;
    }
    
    if(memcmp(buffer,tag,tag_len))      
    {
        LOGI("[ %s %d ] tag  error\n",__FUNCTION__,__LINE__);
        return -2;
    }
    
    index += tag_len;
    /* len */
    {
        tag_data_len = buffer[tag_len];
        LOGI("[ %s %d ]: tag[%d] 0x%02X 0x%02X data_len = %d \n",__FUNCTION__,__LINE__,tag_len,tag[0],tag[1] ,tag_data_len);
        index += 1;
    }

    if( buffer_len != tag_data_len + tag_len + 1)
    {
        LOGI("[ %s %d ] error tag buffer_len : %d  \n",__FUNCTION__,__LINE__,buffer_len);
        return -3;
    }
    
    pt_data->bExist = 1;
    memcpy(pt_data->tag,tag + (2 - tag_len),tag_len);
    pt_data->tag_len = tag_len;
    memcpy(pt_data->data, buffer + index , tag_data_len);
    pt_data->data_len = tag_data_len;
    
    return 0;
}

int apdu_cmd_select_APP(const unsigned char *App_name,unsigned char App_len,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    size_t result;
    unsigned char receive_buf[256+2]= {0};
    int recv_len = 0,rec_data_len = 0;
    unsigned char SW[2];
    if(p_rev_buffer == NULL)
    {
        return -1;
    }
    
    {
        recv_len = apdu_cmd_select_file(0x04,0x00,App_len,App_name,receive_buf,SW);  
        
        if(p_SW != NULL)
        {
            p_SW[0] = SW[0];
            p_SW[1] = SW[1];
        }
        
        if(recv_len <= 2 || SW[0] != 0x90 || SW[1] != 0x00)
        {
            LOGI("[ %s %d ] error recv_len = %d SW[0]=0x%02X SW[1]0x%02X \n",__FUNCTION__,__LINE__,recv_len,SW[0],SW[1]);    
            return -2;
        }

        rec_data_len = recv_len - 2;
    }
    
    return rec_data_len;
}


int apdu_cmd_select_PPSE(const unsigned char *PPSE,unsigned char PPSE_len,RECORD_PPSE *p_recordPSE,unsigned char *p_SW)
{
    int ret = 0;
    unsigned char receive_buf[256+2]= {0};
    int recv_len = 0,rec_data_len = 0;
    unsigned char SW[2];
    if(p_recordPSE == NULL)
    {
        return -1;
    }
    
    {
        recv_len = apdu_cmd_select_file(0x04,0x00,PPSE_len,PPSE,receive_buf,SW);  
        
        if(p_SW != NULL)
        {
            p_SW[0] = SW[0];
            p_SW[1] = SW[1];
        }
        
        if(recv_len <= 2 || SW[0] != 0x90 || SW[1] != 0x00)
        {
            if(SW[0]==0x6A && SW[1]==0x81)   
            {
                LOGI("[ %s %d ] 卡片锁定或者选择（SELECT）命令不支持",__FUNCTION__,__LINE__);
            }
            else if(SW[0]==0x6A && SW[1]==0x82)   
            {
                LOGI("[ %s %d ] 卡片中没有 PSE，卡片响应选择（SELECT）命令指出文件不存在",__FUNCTION__,__LINE__);
            }
            else if(SW[0]==0x6A && SW[1]==0x83)   
            {
                LOGI("[ %s %d ] 卡片锁定或者选择（SELECT）命令不支持",__FUNCTION__,__LINE__);
            }
            else
            {
                LOGI("[ %s %d ] error ret = %d SW[0]=0x%02X SW[1]0x%02X \n",__FUNCTION__,__LINE__,recv_len,SW[0],SW[1]);    
            }
            return -2;
        }

        rec_data_len = recv_len - 2;
    }
    /*选择 PSE 的响应报文（FCI）*/

    ret = pse_find_FCI_data(receive_buf,rec_data_len,p_recordPSE);
    if(ret != 0)
    {
        LOGI("[ %s %d ] error ret = %d  \n",__FUNCTION__,__LINE__,ret);  
        return -3;
    }

    return 0;    
}


/*
    情况4
    命令首标 + Lc字段 + 数据字段 + Le字段
*/
int apdu_cmd4(unsigned char CLA, unsigned char INS,unsigned char P1,unsigned char P2,unsigned char Lc, unsigned char *data,unsigned char Le,unsigned char *p_rev_buffer)
{
    if(p_rev_buffer == NULL)
    {
        return -1;
    }
    unsigned char cmd[128];
    //其中CLA为指令类别；INS为指令码；P1、P2为参数；Lc为Data的长度；Le为希望响应时回答的数据字节数
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
    return apdu_cmd_read_and_write(cmd, 5 + Lc + 1 , p_rev_buffer);
}

/*
    情况3
    命令首标 +     Lc字段 +   数据字段
*/
int apdu_cmd3(unsigned char CLA, unsigned char INS,unsigned char P1,unsigned char P2,unsigned char Lc, unsigned char *data,unsigned char *p_rev_buffer)
{
    if(p_rev_buffer == NULL)
    {
        return -1;
    }
    unsigned char cmd[128];
    //其中CLA为指令类别；INS为指令码；P1、P2为参数；Lc为Data的长度；Le为希望响应时回答的数据字节数
    cmd[0] = CLA;      
    cmd[1] = INS;    
    cmd[2] = P1;          
    cmd[3] = P2;      
    cmd[4] = Lc;
    if(Lc > 0 && data != NULL)
    {
        memcpy(&cmd[5], data, Lc); 
    }

    return apdu_cmd_read_and_write(cmd, 5 + Lc , p_rev_buffer);
}
/*
    情况2
    命令首标 +     Le字段
*/
int apdu_cmd2(unsigned char CLA, unsigned char INS,unsigned char P1,unsigned char P2,unsigned char Le ,unsigned char *p_rev_buffer)
{
    if(p_rev_buffer == NULL)
    {
        return -1;
    }
    unsigned char cmd[5];
    //其中CLA为指令类别；INS为指令码；P1、P2为参数；Lc为Data的长度；Le为希望响应时回答的数据字节数
    cmd[0] = CLA;      
    cmd[1] = INS;    
    cmd[2] = P1;          
    cmd[3] = P2;      
    cmd[4] = Le;
    return apdu_cmd_read_and_write(cmd, 5 , p_rev_buffer);
}
/*
    情况1
    命令首标
*/
int apdu_cmd1(unsigned char CLA, unsigned char INS,unsigned char P1,unsigned char P2,unsigned char *p_rev_buffer)
{
    if(p_rev_buffer == NULL)
    {
        return -1;
    }
    unsigned char cmd[5];
    //其中CLA为指令类别；INS为指令码；P1、P2为参数；Lc为Data的长度；Le为希望响应时回答的数据字节数
    cmd[0] = CLA;      
    cmd[1] = INS;    
    cmd[2] = P1;          
    cmd[3] = P2;      
    cmd[4] = 0x00;
    return apdu_cmd_read_and_write(cmd, sizeof(cmd) , p_rev_buffer);
}

/*
    CLA                 按5.4.1定义的
    INS                 A4
    P1                  选择控制 见表58
    P2                  选择选项 见表59
    
    L c 字段              空或后续数据字段的长度
    数据字段                如果存在下列内容 则按照P 1 -P 2
                            ——文件标识符
                            ——MF的路径
                            ——当前DF的路径
                            ——DF名称
    L e 字段              空或在响应时期望的数据最大长度
*/
int apdu_cmd_select_file(unsigned char p1,unsigned char p2,unsigned char len,const unsigned char *filename,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    size_t result;
    unsigned char receive_buf[256+2]= {0};
    unsigned char CLA = 0x00;   //指令类别      按照本规范定乿命令和响应的结构和编砿    
    unsigned char INS = 0xA4;   //指令代码      SELECT FILE 命令

    result = apdu_cmd3(CLA,INS,p1,p2,len,filename,receive_buf);
    if(result < 2)
    {
        LOGI("[%s %d ]apdu_cmd_read_and_write : result :%d \n",__FUNCTION__,__LINE__,result);
        return -1;
    }

    if(p_SW != NULL)
    {
        p_SW[0] = receive_buf[result - 2];
        p_SW[1] = receive_buf[result - 1];
    }
 
    memcpy(p_rev_buffer,receive_buf,(result - 2));
   
    if(result <= 2)
    {
        LOGI("[%s %d ]apdu_cmd_read_and_write : result :%d SW1=%02X  SW2=%02X\n",__FUNCTION__,__LINE__,result,p_SW[0], p_SW[1]);
        return -1;
    }

    return result;
}
int apdu_cmd_read_record(unsigned char p1,unsigned char p2,unsigned char *p_record_buffer,unsigned char *p_SW)
{
    unsigned char receive_buf[256+2]= {0};
    size_t result;
    unsigned char CLA = 0x00;   //指令类别      按照本规范定乿命令和响应的结构和编砿    
    unsigned char INS = 0xb2;   //指令代码      READ RECORD 命令

    result = apdu_cmd1(CLA,INS,p1,p2,receive_buf);
    if(result < 2)
    {
        LOGI("[%s %d ] result :%d \n",__FUNCTION__,__LINE__,result);
        return -1;
    }

    if(p_SW != NULL)
    {
        p_SW[0] = receive_buf[result - 2];
        p_SW[1] = receive_buf[result - 1];
    }
 
    memcpy(p_record_buffer,receive_buf,(result - 2));
   
    if(result <= 2)
    {
        LOGI("[%s %d ] result :%d SW1=%02X  SW2=%02X\n",__FUNCTION__,__LINE__,result,p_SW[0], p_SW[1]);
        return -1;
    }
    return result - 2;
}
/*

非接触CPU卡应该支持以下与应用无关的常用指令命令：

    指令                  指令类别        指令码       功能描述
 
APPLICATION UNBLOCK     84      18  应用解锁
APPLICATION BLOCK       84      1E  应用锁定 
CARD BLOCK              84      16  卡片锁定
EXTERNAL AUTHENTICATE   00      82  外部认证 
GET CHALLENGE           00      84  取随机数 
INTERNAL AUTHENTICATE   00      88  内部认证
PIN UNBLOCK             84      24  个人密码解锁
READ  BINARY            00      B0  读二进制文件内容
READ  RECORD            00      B2  读记录文件指定内容
SELECT                  00      A4  选择文件 
UPDATE  BINARY          00/04   D6  写二进制文件 
UPDATE  RECORD          00/04   DC  写记录文件
VERIFY                  00      20  验证口令

    指令                 指令类别       指令码            功能描述
 
CHANGE/PIN RELOAD       80          5E          重装/修改个人密码
CREDIT FOR LOAD         80          52          圈存
DEBIT FOR PURCHASE/CASE WITHDRAW/UNLOAD   80 54 消费/取现/圈提
GET BALANCE             80          5C          读余额
GET TRANSCATION PROVE   80          5A          取交易认证
INITIALIZE FOR XXX      80          50          初始化XXX交易 
UNBLOCK                 80          2C          解锁被锁住的口令
UPDATE OVERDRAW LIMIT   80          58          修改透支限额
*/

int apdu_cmd_get_balance1(unsigned char *p_balance)
{
    unsigned char receive_buf[4]= {0};
    int result;

    if(p_balance == NULL)
    {
        return -1;
    }

    result = apdu_cmd_get_balance(0x03, 0x04, receive_buf);
    if(result <= 2 )
    {
        LOGI("[%s %d ]apdu_cmd_read_and_write : result :%d SW1=%02X  SW2=%02X\n",__FUNCTION__,__LINE__,result,receive_buf[0],receive_buf[1]);
        return -2;
    }

    LOGI("\nGET BALANCE : %d \n",receive_buf[0]<<24|receive_buf[1]<<16|receive_buf[2]<<8|receive_buf[3]);

    memcpy(p_balance, receive_buf, 4);

    return result - 2 ;
}

int apdu_cmd_application_block(unsigned char p1,unsigned char p2,unsigned char Lc, unsigned char * MAC,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd3(0x84, 0x1E, p1, p2, Lc,MAC,p_rev_buffer);
}

int apdu_cmd_application_unblock(unsigned char p1,unsigned char p2,unsigned char Lc, unsigned char * MAC,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd3(0x84, 0x18, p1, p2, Lc,MAC,p_rev_buffer);
}
int apdu_cmd_card_block(unsigned char p1,unsigned char p2,unsigned char Lc, unsigned char * data,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd3(0x84, 0x16, p1, p2, Lc,data,p_rev_buffer);
}
int apdu_cmd_external_authenticate(unsigned char Lc, unsigned char * data,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd3(0x00, 0x82,0x00,0x00, Lc,data,p_rev_buffer);
}
int apdu_cmd_generate_ac(unsigned char p1,unsigned char Lc, unsigned char * data,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd4(0x80, 0xAE, p1, 0x00, Lc, data, 0x00, p_rev_buffer);
}

int apdu_cmd_get_data(unsigned char p1,unsigned char p2,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    /*p1p2:放访问数据的标签*/
    return apdu_cmd1(0x80, 0xCA,p1, p2, p_rev_buffer);
}
/************************************************************************
FUNCTION:  GET GPO
PARAMETER DESCRIPTION:
    *gpolist - 标签数据         
    : B.8.2   emv 5.6
            编码               值
            CLA             0x80
            INS             0xA8
            P1              0x00
            P2              0x00
            Lc              var
            数据域             PDOL  相关数据（如果存在）或 8300
            Le              0x00
    
    len - gpolist len
RETURN:
    state - 返回的状态值
************************************************************************/
int apdu_cmd_get_processing_options(unsigned char  *gpolist,unsigned char  len,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    int result;
  
    unsigned char  data[256];
    unsigned char  data_len = 0;

    if(gpolist == NULL || p_rev_buffer == NULL)
    {
        return -1;
    }

    /* 设置 Lc 和 数据域*/
    {
        unsigned char  index = 0;
        memcpy(data,"\x83",1) ;
        index++;
        if(len > 127) 
        {
            memcpy(data + index,"\x81",1);
            index++;
        }
        memcpy(data+index,&len, 1);
        index++;
        memcpy(data + index, gpolist, len);
        index += len;
        data_len = index;
    }
    
    result = apdu_cmd4(0x80, 0xA8, 0x00, 0x00, data_len , data, 0x00, p_rev_buffer);
    if(p_SW != NULL)
    {
        p_SW[0] = p_rev_buffer[result - 2];
        p_SW[1] = p_rev_buffer[result - 1];
    }
    
    if(result <= 2 || p_rev_buffer[result - 2] != 0x90 || p_rev_buffer[result -1] != 0x00)
    {
        LOGI("[%s %d ] result :%d SW1=%02X  SW2=%02X\n",__FUNCTION__,__LINE__,result,p_rev_buffer[result - 2],p_rev_buffer[result -1]);
        return -1;
    }

    return result -2;
}

int apdu_cmd_internal_auhenticate(unsigned char Lc, unsigned char *data,unsigned char *p_rev_buffer,unsigned char *p_SW)
{  
    return apdu_cmd4(0x00, 0x88, 0x00, 0x00, Lc, data, 0x00, p_rev_buffer);
}
int apdu_cmd_pin_change_or_unblock(unsigned char p2,unsigned char Lc, unsigned char *data,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd3(0x84, 0x24, 0x00, p2, Lc, data, p_rev_buffer);
}
int apdu_cmd_put_data(unsigned char p1,unsigned char p2,unsigned char Lc, unsigned char *data,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd3(0x04, 0xDA, p1, p2, Lc, data, p_rev_buffer);
}
int apdu_cmd_verify(unsigned char p2,unsigned char Lc, unsigned char *data,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd3(0x00, 0x20, 0x00, p2, Lc, data, p_rev_buffer);

}
int apdu_cmd_read_capp_data(unsigned char p1,unsigned char p2,unsigned char Lc, unsigned char *data,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    switch(p1)
    {
        case 0x00:
            {
                /*采用DES算法*/
            }
            break;
        case 0x01:
            {
                /*采用SM4算法*/
            }
            break;
        default:
            {
                return -1;
            }
            break;
    }
    switch(Lc)
    {
        case 0x02:
        case 0x0A:
            {
            }
            break;
        default:
            {
                return -1;
            }
            break;
    }
    
    return  apdu_cmd4(0x80, 0xB4, p1, p2, Lc, data,0x00 ,p_rev_buffer);
}
int apdu_cmd_update_capp_data_cache1(unsigned char p2,unsigned char Lc, unsigned char *data,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return  apdu_cmd4(0x84, 0xDE,0x00, p2, Lc, data,0x00 ,p_rev_buffer);
}

int apdu_cmd_append_record(unsigned char p1,unsigned char p2,unsigned char Lc, unsigned char *data,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    switch(p1)
    {
        case 0x00:
            {
                /*采用DES算法*/
            }
            break;
        case 0x01:
            {
                /*采用SM4算法*/
            }
            break;
        default:
            {
                return -1;
            }
            break;
    }

    return apdu_cmd3(0x04, 0xE2,p1, p2, Lc, data ,p_rev_buffer);

}

int apdu_cmd_get_trans_prove(unsigned char ATC[2],unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd4(0x80, 0x5A,0x00, 0x00, 0x02, ATC ,0x08,p_rev_buffer);
}

int apdu_cmd_security_update(unsigned char p1,unsigned char p2,unsigned char Lc, unsigned char *data,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd3(0x84, 0x12, p1, p2, Lc, data, p_rev_buffer);
}

int apdu_cmd_get_challenge(unsigned char Le,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    switch(Le)
    {
        case 0x04:
            {
            }
            break;
        case 0x08:
            {
            }
            break;
        default:
            {
            }
            return -1;
    }
    return apdu_cmd2(0x00, 0x84, 0x00, 0x00, Le,p_rev_buffer);
}
int apdu_cmd_read_binary1(unsigned char p1,unsigned char p2,unsigned char *p_rev_buffer,unsigned char *p_SW)
{   
    unsigned char receive_buf[256+2]= {0};
    size_t result;
    unsigned char CLA = 0x00;   //指令类别      按照本规范定乿命令和响应的结构和编砿    
    unsigned char INS = 0xB0;   //指令代码      read binary 命令    不需要安全报文

    result = apdu_cmd1(CLA,INS,p1,p2,receive_buf);
    if(result < 2)
    {
        LOGI("[%s %d ] result :%d \n",__FUNCTION__,__LINE__,result);
        return -1;
    }

    if(p_SW != NULL)
    {
        p_SW[0] = receive_buf[result - 2];
        p_SW[1] = receive_buf[result - 1];
    }
 
    memcpy(p_rev_buffer,receive_buf,(result - 2));
   
    if(result <= 2)
    {
        LOGI("[%s %d ] result :%d SW1=%02X  SW2=%02X\n",__FUNCTION__,__LINE__,result,p_SW[0], p_SW[1]);
        return -2;
    }
    return result - 2;

}
int apdu_cmd_read_binary2(unsigned char p1,unsigned char p2,unsigned char Lc, unsigned char *data,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    /*需要安全报文*/
    return apdu_cmd4(0x04, 0xB0, p1, p2,Lc,data,0x00,p_rev_buffer);
}
int apdu_cmd_update_binary1(unsigned char p1,unsigned char p2,unsigned char Lc, unsigned char *data,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd3(0x00, 0xD6, p1, p2,Lc,data,p_rev_buffer);
}
int apdu_cmd_update_binary2(unsigned char p1,unsigned char p2,unsigned char Lc, unsigned char *data,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd3(0x04, 0xD6, p1, p2,Lc,data,p_rev_buffer);
}

int apdu_cmd_credit_for_load(unsigned char data[0x0B],unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd4(0x80,0x52,0x00,0x00,0x0B,data,0x04,p_rev_buffer);
}
int apdu_cmd_debit_for_purchase(unsigned char data[0x0F],unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    unsigned char receive_buf[256+2]= {0};
    size_t result;
    unsigned char CLA = 0x80;   //指令类别      按照本规范定乿命令和响应的结构和编砿    
    unsigned char INS = 0x54;   //指令代码      read binary 命令    不需要安全报文
 
    result = apdu_cmd4(CLA,INS,0x01,0x00,0x0F,data,0x08,receive_buf);
    if(result < 2)
    {
        LOGI("[%s %d ] result :%d \n",__FUNCTION__,__LINE__,result);
        return -1;
    }
 
    if(p_SW != NULL)
    {
        p_SW[0] = receive_buf[result - 2];
        p_SW[1] = receive_buf[result - 1];
    }
 
    memcpy(p_rev_buffer,receive_buf,(result - 2));
   
    if(result <= 2)
    {
        LOGI("[%s %d ] result :%d SW1=%02X  SW2=%02X\n",__FUNCTION__,__LINE__,result,p_SW[0], p_SW[1]);
        return -2;
    }
    return result - 2;
}
int apdu_cmd_debit_for_upload(unsigned char data[0x0B],unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd4(0x80,0x54,0x03,0x00,0x0B,data,0x04,p_rev_buffer);
}
int apdu_cmd_get_balance(unsigned char p1,unsigned char Le,unsigned char *p_rev_buffer)
{
    switch(p1)
    {
        case 0x00:   /*可用余额*/
        case 0x01:   /*透支限额*/
        case 0x02:   /*已透支金额*/
        case 0x03:   /*实际余额*/
        case 0x04:   /*实际余额上限*/
            {
               if(Le != 0x04)        /*期望接收到的数据为0x04个字节*/
               {
                    return -1;
               }
            }
            break;
        case 0x05:   /*实际余额 + 实际余额上限 +   已透支金额 + 透支限额 */
            {
               if(Le != 0x10)        /*应该针对p1=0x05 期望接收到的数据为0x10个字节*/
               {
                    return -1;
               }
            }
            break;   
        default :
            {
                return -1;
            }
            break;
    }
    
    return apdu_cmd2(0x80,0x5C,p1,0x02,Le,p_rev_buffer);
}
int apdu_cmd_get_transaction_prove(unsigned char p2,unsigned char data[2],unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    unsigned char receive_buf[256+2]= {0};
    size_t result;
    unsigned char CLA = 0x80;   //指令类别      按照本规范定乿命令和响应的结构和编砿    
    unsigned char INS = 0x5A;   //指令代码      GET TRANSCATION PROVE  取交易认证
  
    result = apdu_cmd4(CLA,INS,0x00,p2,0x02,data,0x08,receive_buf);
    if(result < 2)
    {
        LOGI("[%s %d ] result :%d \n",__FUNCTION__,__LINE__,result);
        return -1;
    }
  
    if(p_SW != NULL)
    {
        p_SW[0] = receive_buf[result - 2];
        p_SW[1] = receive_buf[result - 1];
    }
  
    memcpy(p_rev_buffer,receive_buf,(result - 2));
   
    if(result <= 2)
    {
        LOGI("[%s %d ] result :%d SW1=%02X  SW2=%02X\n",__FUNCTION__,__LINE__,result,p_SW[0], p_SW[1]);
        return -2;
    }
    return result - 2;
}
int apdu_cmd_initialize_for_load(unsigned char data[0x0B],unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd4(0x80,0x50,0x00,0x02,0x0B,data,0x10,p_rev_buffer);
}
int apdu_cmd_initialize_for_purchase(unsigned char data[0x0B],unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd4(0x80,0x50,0x01,0x02,0x0B,data,0x0F,p_rev_buffer);
}
int apdu_cmd_initialize_for_upload(unsigned char data[0x0B],unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd4(0x80,0x50,0x05,0x02,0x0B,data,0x10,p_rev_buffer);
}
int apdu_cmd_initialize_for_update(unsigned char data[0x07],unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd4(0x80,0x50,0x04,0x01,0x07,data,0x13,p_rev_buffer);
}

int apdu_cmd_initialize_for_capp_purchase(unsigned char data[0x0B],unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    unsigned char receive_buf[256+2]= {0};
    size_t result;
    unsigned char CLA = 0x80;   //指令类别         
    unsigned char INS = 0x50;   //指令代码  

    result = apdu_cmd4(CLA,INS,0x03,0x02,0x0B,data,0x0F,receive_buf);
    if(result < 2)
    {
        LOGI("[%s %d ] result :%d \n",__FUNCTION__,__LINE__,result);
        return -1;
    }
  
    if(p_SW != NULL)
    {
        p_SW[0] = receive_buf[result - 2];
        p_SW[1] = receive_buf[result - 1];
    }
  
    memcpy(p_rev_buffer,receive_buf,(result - 2));
   
    if(result <= 2)
    {
        LOGI("[%s %d ] result :%d SW1=%02X  SW2=%02X\n",__FUNCTION__,__LINE__,result,p_SW[0], p_SW[1]);
        return -2;
    }

    return result - 2;
}
int apdu_cmd_update_capp_data_cache2(unsigned char p1,unsigned char p2,unsigned char Lc, unsigned char *data,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    unsigned char receive_buf[256+2]= {0};
    size_t result;
    unsigned char CLA = 0x80;   //指令类别      按照本规范定乿命令和响应的结构和编砿    
    unsigned char INS = 0xDC;   //指令代码 
    
    result = apdu_cmd3(CLA,INS,p1, p2, Lc,data,receive_buf);
    if(result < 2)
    {
        LOGI("[%s %d ] result :%d \n",__FUNCTION__,__LINE__,result);
        return -1;
    }
  
    if(p_SW != NULL)
    {
        p_SW[0] = receive_buf[result - 2];
        p_SW[1] = receive_buf[result - 1];
    }
  
    memcpy(p_rev_buffer,receive_buf,(result - 2));
   
    if(result <= 2)
    {
        LOGI("[%s %d ] result :%d SW1=%02X  SW2=%02X\n",__FUNCTION__,__LINE__,result,p_SW[0], p_SW[1]);
        return -2;
    }
 
    return result - 2;
}

int apdu_cmd_debit_for_capp_purchase(unsigned char data[0x0F],unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd4(0x80, 0xDC,0x01, 0x00, 0x0F, data ,0x08,p_rev_buffer);
}
int apdu_cmd_update_overdraw_limit(unsigned char data[0x0E],unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    return apdu_cmd4(0x80, 0x58,0x00, 0x00, 0x0E, data ,0x04,p_rev_buffer);
}
int apdu_cmd_cryptographic_operation(unsigned char CLA,unsigned char p1,unsigned char p2,unsigned char *data,unsigned char *p_rev_buffer,unsigned char *p_SW)
{
    int ret = 0;
    switch(CLA)
    {
        case 0x80:
        case 0x84:
            break;
        default:
            {
                return -1;
            }
            break;
    }

    switch(p1)
    {
        case 0x00: /*读取当前密钥组 */
            {
                if(p2 != 0x00 || data != NULL)
                {
                    return -1;
                }
                ret = apdu_cmd4(CLA, 0xCD, 0x00, 0x00, 0x00, NULL, 0x01, p_rev_buffer);
            }
            break;
        case 0x01:
        case 0x02:
            {
                if(data != NULL)
                {
                    return -1;
                }
                ret = apdu_cmd3(CLA, 0xCD, p1, p2, 0x00, NULL, p_rev_buffer);
            }
            break;
  
        case 0x03:
            {
                ret = apdu_cmd3(CLA, 0xCD, 0x03, p2, 0x04, data, p_rev_buffer);
            }
            break;
        default:
            {
                return -1;
            }
            break;
    }
    return ret;
}
int apdu_cmd_read_and_write(const unsigned char *cmd, int cmd_size,unsigned char *buff)
{
    static int  receive_len[1] = {0};
    int result;

    if(cmd == NULL || buff == NULL)
    {
        LOGI("[%s %d] cmd:%08X buff:%08X \n",__FUNCTION__,__LINE__,cmd,buff);
        return -1;
    }
#if defined(TARGET_DEBUG)   
    LOGI("\n------------%s start-------------\n",__FUNCTION__);
    LOGI("  ---mifare_write  :\n");
    menu_print(cmd, cmd_size);
#endif

    result = ISO14443_read_and_write(cmd,cmd_size,buff);
    if(result < 2)    
    {
        LOGI("[%s %d] result:%02X\n",__FUNCTION__,__LINE__,result);
        return result;
    }
    else if(buff[result - 2] != 0x90 || buff[result -1] != 0x00)
    {
        unsigned char SW[2];
        SW[0] = buff[result - 2] ;
        SW[1] = buff[result -1] ;
        print_SW_error(SW);
#if defined(TARGET_DEBUG)  
        LOGI("--------------end result : %d----------------------\n\n",result);
#endif
        return result;
    }
#if defined(TARGET_DEBUG)  
    LOGI("  ---mifare_read  :\n");
    menu_print(buff, result);    
    LOGI("--------------end----------------------\n\n");
#endif
    return result;
}


int print_SW_error(unsigned char SW[2])
{
    LOGI("SW[0]:0x%2X SW[1]:0x%2X \n",SW[0], SW[1]);
    if(SW[0] == 0x62 && SW[1] == 0x00) { LOGI("警告 信息未提供\n");    return 0;}
    if(SW[0] == 0x62 && SW[1] == 0x81) { LOGI("警告 回送数据可能\n");    return 0;}
    if(SW[0] == 0x62 && SW[1] == 0x82) { LOGI("警告 文件长度小于Le\n");    return 0;}
    if(SW[0] == 0x62 && SW[1] == 0x83) { LOGI("警告 选中的文件无效\n");    return 0;}
    if(SW[0] == 0x62 && SW[1] == 0x84) { LOGI("警告 FCI格式与P2指定的不符\n");    return 0;}
    if(SW[0] == 0x63 && SW[1] == 0x00) { LOGI("警告 鉴别失败\n");    return 0;}
    if(SW[0] == 0x63 && (SW[1]&0xF0) == 0xC0) { LOGI("警告 校验失败(允许重试次数:%d)",SW[1]&0xF);}
    if(SW[0] == 0x64 && SW[1] == 0x00) { LOGI("状态标志位没有变\n");    return 0;}
    if(SW[0] == 0x65 && SW[1] == 0x81) { LOGI("内存失败\n");    return 0;}
    if(SW[0] == 0x67 && SW[1] == 0x00) { LOGI("长度错误\n");    return 0;}

    if(SW[0] == 0x68 && SW[1] == 0x82) { LOGI("不支持安全报文\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x81) { LOGI("命令与文件结构不相容，当前文件非所需文件\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x82) { LOGI("操作条件(AC)不满足，没有校验PIN\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x83) { LOGI("您的卡已被锁定\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x84) { LOGI("随机数无效，引用的数据无效\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x85) { LOGI("使用条件不满足\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x86) { LOGI("不满足命令执行条件(不允许的命令，INS有错)\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x87) { LOGI("MAC丢失\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x88) { LOGI("MAC不正确\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x8D) { LOGI("保留\n");    return 0;}

    if(SW[0] == 0x6A && SW[1] == 0x80) { LOGI("数据域参数不正确\n");    return 0;}
    if(SW[0] == 0x6A && SW[1] == 0x81) { LOGI("功能不支持,创建不允许；目录无效；应用锁定\n");    return 0;}
    if(SW[0] == 0x6A && SW[1] == 0x82) { LOGI("该文件未找到\n");    return 0;}
    if(SW[0] == 0x6A && SW[1] == 0x83) { LOGI("该记录未找到\n");    return 0;}
    if(SW[0] == 0x6A && SW[1] == 0x84) { LOGI("文件预留空间不足\n");    return 0;}
    if(SW[0] == 0x6A && SW[1] == 0x86) { LOGI("P1或P2不正确\n");    return 0;}
    if(SW[0] == 0x6A && SW[1] == 0x88) { LOGI("引用数据未找到\n");    return 0;}

    if(SW[0] == 0x6B && SW[1] == 0x00) { LOGI("参数错误\n");    return 0;}
    if(SW[0] == 0x6C)                  { LOGI("Le长度错误，实际长度是 %d",SW[1]);}
    if(SW[0] == 0x6E && SW[1] == 0x00) { LOGI("不支持的类：CLA有错\n");    return 0;}
    if(SW[0] == 0x6F && SW[1] == 0x00) { LOGI("数据无效\n");    return 0;}
    if(SW[0] == 0x6F && SW[1] == 0x01) { LOGI("连接中断\n");    return 0;}
    if(SW[0] == 0x6D && SW[1] == 0x00) { LOGI("不支持的指令代码\n");    return 0;}
    if(SW[0] == 0x93 && SW[1] == 0x01) { LOGI("您的卡余额不足\n");    return 0;}
    if(SW[0] == 0x93 && SW[1] == 0x02) { LOGI("MAC2错误\n");    return 0;}
    if(SW[0] == 0x93 && SW[1] == 0x03) { LOGI("应用被永久锁定\n");    return 0;}
    if(SW[0] == 0x94 && SW[1] == 0x01) { LOGI("您的卡余额不足\n");    return 0;}
    if(SW[0] == 0x94 && SW[1] == 0x02) { LOGI("交易计数器达到最大值\n");    return 0;}
    if(SW[0] == 0x94 && SW[1] == 0x03) { LOGI("密钥索引不支持\n");    return 0;}
    if(SW[0] == 0x94 && SW[1] == 0x06) { LOGI("所需MAC不可用\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x00) { LOGI("不能处理\n");    return 0;}
    if(SW[0] == 0x69 && SW[1] == 0x01) { LOGI("命令不接受（无效状态）\n");    return 0;}
    if(SW[0] == 0x61) { LOGI("正常 需发GET RESPONSE命令 读取指令00C00000%2x",SW[1]);}
    if(SW[0] == 0x66 && SW[1] == 0x00) { LOGI("接收通讯超时\n");    return 0;}
    if(SW[0] == 0x66 && SW[1] == 0x01) { LOGI("接收字符奇偶错\n");    return 0;}
    if(SW[0] == 0x66 && SW[1] == 0x02) { LOGI("校验和不对\n");    return 0;}
    if(SW[0] == 0x66 && SW[1] == 0x03) { LOGI("警告 当前DF文件无FCI\n");    return 0;}
    if(SW[0] == 0x66 && SW[1] == 0x04) { LOGI("警告 当前DF下无SF或KF\n");    return 0;}
    if(SW[0] == 0x6E && SW[1] == 0x81) { LOGI("片已离开\n");    return 0;}

    LOGI("未知错误\n"); 
    return -1;
}

