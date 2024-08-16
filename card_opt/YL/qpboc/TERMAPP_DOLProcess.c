#include "includes.h"
//---------#include "../gui/RC500.h"
#include "common/cardlan_StandardBus_tpye.h"
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

//---------
CardInform CardLan;
//---------      
uchar TERMAPP_PDOLProcess(uchar *pdolData,unsigned int *lenOut)
{
    uchar index,indexOut,len;
    uchar i,j,k,m,buf[255],bInTable;
    uchar amt[6];
    uchar temp=0;
    uchar ret;

    LOGI("TERMAPP_PDOLProcess() is called.\n");

    index=0;
    indexOut=0;
        
    while(index < SelectedApp.PDOLLen) //Process PDOL
    {
        temp++;

        if(SelectedApp.PDOL[index]==0xFF || SelectedApp.PDOL[index]==0x00) 
        {
            index++;
            continue;
        }
        
        memset(buf,0,255);        
        k = SelectedApp.PDOL[index];
        for(i = 0;i < TermDataNum; i++)
        {
            
            bInTable=0;
            if(k!=TermDataTable[i].Tag[0])
            {
                continue;
            }
            if((k & 0x1F)==0x1F)
            {
                if(SelectedApp.PDOL[index+1]!=TermDataTable[i].Tag[1])
                {
                    continue;
                }
            }

            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x66",2))
            {
                TermDataTable[MV_TermTransProp].bExist=1;
                bInTable=1;
                break;
            }
            
            if(k==0x81 || !memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x02",2))
            {
                bInTable=1;
                break;            
            }
            
            if((k==0x9F) && ((SelectedApp.PDOL[index+1]==0x03) ||(SelectedApp.PDOL[index+1]==0x04)))
            {
                    bInTable=1;
                    break;
            }
            
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x7A",2)) //VLP terminal support indicator
            {
                TermInfo.VLPIndicator=1;
                if(TermInfo.bTermSupportVLP==0)
                {
                    TermInfo.VLPIndicator=0;
                }
                if(TermInfo.TransType!=GOODS && TermInfo.TransType!=SERVICE)
                {
                    TermInfo.VLPIndicator=0;
                }
                if(AmtAuthBin>=bcd_to_bin(TermInfo.VLPTransLimit,6))
                {
                    TermInfo.VLPIndicator=0;
                }
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x1A",2))  //Terminal country code BCD
            {
                 if(!memcmp(TermInfo.CountryCode,"\x01\x56",2))
                 {
                    memcpy(TermInfo.CountryCode,"\x01\x56",2);
                 }
                 bInTable=1;
                 break;
            }
            if(SelectedApp.PDOL[index]==0x95)  //Terminal Verification Results
            {
                 bInTable=1;
                 break;
            }
            if(SelectedApp.PDOL[index]==0x9B)  //Transaction Status Information
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x33",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x1E",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x5F\x2A",2))   //TransCurcyCode
            {
                 bInTable=1;
                 break;
            }
            if(SelectedApp.PDOL[index]==0x98) 
            {
                 bInTable=1;
                 break;
            }
            if(SelectedApp.PDOL[index]==0x9A) 
            {
                 bInTable=1;
                 break;
            }
            if(SelectedApp.PDOL[index]==0x9C) //transtype value
            {
                 bInTable=1;
                 break;
            }
            if(SelectedApp.PDOL[index]==0x8A) 
            {
                 bInTable=1;
                 break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x37",2))
            {
                 bInTable=1;
                 break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x40",2))
            {
                 bInTable=1;
                 break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x06",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x15",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x16",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x4E",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x39",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x1B",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x35",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x3D",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x41",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x5F\x36",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x21",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x1C",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x01",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x4E",2))
            {
                bInTable=1;
                break;
            }

            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x3A",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x09",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x34",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\x9F\x3C",2))
            {
                bInTable=1;
                break;
            }
            if(!memcmp((uchar *)&SelectedApp.PDOL[index],"\xDF\x60",2))
            {
                if (CardLan.CardType== 3 || CardLan.CardType == 4)
                {
                    TermInfo.ExternApp=1;            //扩展应用
                }
                else 
                {
                    TermInfo.ExternApp=0;
                }
                bInTable=1;
                break;
            }    
        }
        
        if(!bInTable)                //unknow tag
        {                
            if((k&0x1F)==0x1F)
            {
                printf("TERMAPP_PDOLProcess(): Unknown tag: %02X%02X\n", SelectedApp.PDOL[index], SelectedApp.PDOL[index + 1]);
                index++;
            }
            else
            {
                printf("TERMAPP_PDOLProcess(): Unknown tag: %02X\n", SelectedApp.PDOL[index]);
            }
            index++;
            k = SelectedApp.PDOL[index];
            memcpy((uchar *)&pdolData[indexOut],buf,k);
        }
        else
        {
            if((k & 0x1F) == 0x1F)
            {
                index++;
            }
            index++;
            k = SelectedApp.PDOL[index];
            m = TermDataTable[i].Len2;
            if(TermDataTable[i].bAmt == 1) //numeric
            {
                if(k>=m)
                {
                    memcpy(buf+k-m,(uchar *)&TermInfo+TermDataTable[i].address,m);
                    memcpy((uchar *)&pdolData[indexOut],buf,k);
                }
                else
                {
                    memcpy(buf,(uchar *)&TermInfo+TermDataTable[i].address,m);
                    memcpy((uchar *)&pdolData[indexOut],(uchar *)&buf[m-k],k);
                }
            }
            else if(TermDataTable[i].bAmt==2) //compact numeric
            {
                memset(buf,0xFF,255);
                memcpy(buf,(uchar *)&TermInfo+TermDataTable[i].address,m);
                memcpy((uchar *)&pdolData[indexOut],buf,k);
            }
            else //other formats
            {
                memcpy(buf,(uchar *)&TermInfo+TermDataTable[i].address,m);
                memcpy((uchar *)&pdolData[indexOut],buf,k);
            }
        }
        
        index++;
        indexOut+=k;
    }
    
    *lenOut = (unsigned int)indexOut;

    if(TermDataTable[MV_TermTransProp].bExist!=1)
    {
        return NOK;
    }
    return OK;
}


       
uchar TERMAPP_DOLProcess(uchar type,uchar *DOL,uchar DOLLen,uchar *DOLData,uchar *DOLDataLen)
{
    uchar index,indexOut;
    unsigned long amt;
    uchar bInTable,i,k,m,buf[255],j;
    uchar TDOLData[512];
    uchar TDOLDataLen;
    uchar bHasUnpredictNum;

    LOGI("TERMAPP_DOLProcess() is called.\n");

    bHasUnpredictNum=0;

    memset(TDOLData,0,512);
    TDOLDataLen=0;

    index=0;
    indexOut=0;

    while(index<DOLLen)  //Process DOL
    {
        if(DOL[index]==0xFF || DOL[index]==0x00) {index++;continue;}
        memset(buf,0,255);
        bInTable=0;
        k=DOL[index];

        if (type == typePackData)
        {
            // add tag and length
            if((k&0x1F)==0x1F)
            {
                memcpy(&DOLData[indexOut], &DOL[index], 2);
                indexOut+=2;
                DOLData[indexOut] = DOL[index + 2];
                indexOut++;
            }
            else
            {
                DOLData[indexOut] = DOL[index];
                indexOut++;
                DOLData[indexOut] = DOL[index + 1];
                indexOut++;
            }
        }
        
        for(i=0;i<TermDataNum;i++)
        {
            if(k!=TermDataTable[i].Tag[0]) continue;
            if((k&0x1F)==0x1F)
            {
                if(DOL[index+1]!=TermDataTable[i].Tag[1]) continue;
            }
            if(!memcmp((uchar *)&DOL[index],"\x9F\x37",2))//Unpredict Num for DDOL
            {
                bHasUnpredictNum=1;
            }
            index++;
            if((k&0x1F)==0x1F)    index++;

            k=DOL[index];
            m=TermDataTable[i].Len2;
            if(TermDataTable[i].bAmt==1)//numeric
            {
                if(k>=m){
                    memcpy((uchar *)&buf[k-m],(uchar *)&TermInfo+TermDataTable[i].address,m);
                    memcpy((uchar *)&DOLData[indexOut],buf,k);
                }
                else{
                    memcpy(buf,(uchar *)&TermInfo+TermDataTable[i].address,m);
                    memcpy((uchar*)&DOLData[indexOut],(uchar*)&buf[m-k],k);
                }
            }
            else if(TermDataTable[i].bAmt==2)//compact numeric
            {
                memset(buf,0xFF,255);
                memcpy(buf,(uchar *)&TermInfo+TermDataTable[i].address,m);
                
                memcpy((uchar *)&DOLData[indexOut],buf,k);
            }
            else//other formats
            {
                memcpy(buf,(uchar *)&TermInfo+TermDataTable[i].address,m);
                memcpy((uchar *)&DOLData[indexOut],buf,k);
            }
            indexOut+=k;
            index++;
            bInTable=1;
            break;
        }
        if(!bInTable)//check if it is a data of card table.
        {
            for(i=0;i<ICCDataNum;i++)
            {
                if(k!=ICCDataTable[i].Tag[0]) continue;
                if((k&0x1F)==0x1F)
                {
                    if(DOL[index+1]!=ICCDataTable[i].Tag[1]) continue;
                }
                index++;
                if((k&0x1F)==0x1F)    index++;

                k=DOL[index];
                m=ICCDataTable[i].Len2;
                if(ICCDataTable[i].bAmt==1)//numeric
                {
                    if(k>=m)
                    {
                        memcpy((uchar *)&buf[k-m],(uchar *)&CardInfo+ICCDataTable[i].address,m);
                        memcpy((uchar *)&DOLData[indexOut],buf,k);
                    }
                    else
                    {
                        memcpy(buf,(uchar *)&CardInfo+ICCDataTable[i].address,m);
                        memcpy((uchar *)&DOLData[indexOut],(uchar*)&buf[m-k],k);
                    }
                }
                else if(ICCDataTable[i].bAmt==2)//compact numeric
                {
                    memset(buf,0xFF,255);
                    memcpy(buf,(uchar *)&CardInfo+ICCDataTable[i].address,m);
                    memcpy((uchar *)&DOLData[indexOut],buf,k);
                }
                else//other formats
                {
                    memcpy(buf,(uchar *)&CardInfo+ICCDataTable[i].address,m);
                    memcpy((uchar *)&DOLData[indexOut],buf,k);
                }
                indexOut+=k;
                index++;
                bInTable=1;
                break;
            }
        }
        if(!bInTable)
        {
            if((k&0x1F)==0x1F)
            {
                printf("TERMAPP_DOLProcess(): Unknown tag: %02X%02X\n", DOL[index], DOL[index + 1]);
                index++;
            }
            else
            {
                printf("TERMAPP_DOLProcess(): Unknown tag: %02X\n", DOL[index]);
            }

            index++;
            k=DOL[index];
            memcpy((uchar *)&DOLData[indexOut],buf,k);
            indexOut+=k;
            index++;
        }
    }
    *DOLDataLen=indexOut;

    if(bHasUnpredictNum==0)
    {
        if(type==typeDDOL)
        {
            bDDOLHasNoUnpredictNum=1;
        }
        else if(type==typeCDOL1)
        {
            bCDOL1HasNoUnpredictNum=1;
        }
        else if(type==typeCDOL2)
        {
            bCDOL2HasNoUnpredictNum=1;
        }
        else
        {}
        return NOK;
    }
        
    return OK;
}

