#ifndef __CPU_CARD_OPERATION_H__
#define __CPU_CARD_OPERATION_H__

#include "card_opt/M1/M1_card_operation.h"
#include "card_opt/JTB/cardlan_JTB_cpucard_info.h"

typedef struct {
    unsigned char AIDLen;
    unsigned char AID[16];//5-16
    unsigned char AppLabelLen;
    unsigned char AppLabel[16];
    unsigned char PreferNameLen;
    unsigned char PreferName[16];
    unsigned char Priority; //tag'87'
    unsigned char LangPreferLen;
    unsigned char LangPrefer[8];//2-8
    unsigned char ICTI; //Issuer Code Table Index.lang used for display app list according to ISO8859.but not include Chinese,Korea,etc.
    unsigned char PDOLLen;
    unsigned char PDOL[255];
    unsigned char DirDiscretLen;
    unsigned char DirDiscret[200];
    unsigned char IssuerDiscretLen;
    unsigned char IssuerDiscret[255];
    unsigned char bLocalName;//If display app list using local language considerless of info in card.0-use card info;1-use local language.
    unsigned char AppLocalNameLen;
    unsigned char AppLocalName[16];
}APPDATA;


typedef struct {
    unsigned char ASI;  //Application Selection Indicator.0-needn't match exactly(partial match up to the length);1-match exactly
    unsigned char AIDLen;
    unsigned char AID[16];//5-16
    unsigned char bLocalName;//If display app list using local language considerless of info in card.0-use card info;1-use local language.
    unsigned char AppLocalNameLen;
    unsigned char AppLocalName[16];
}TERMAPP_T;

int CardHalt(void);
int CheckCard(unsigned char *csn,unsigned char *plen);
int CardReset(char *data,unsigned char *plen,unsigned char type);



int half_search_white(WhiteItem dest, int *find);
unsigned char Card_White_Cpu_jiaotong(void);
unsigned char Check_CardDate();
char * Rd_time (char* buff);
unsigned char SelectAppDF(char *DFname,char *Recvdata,unsigned  char *namelen);
unsigned char SelectAID(unsigned char *input,unsigned char *inlen,unsigned char *AID,unsigned char *outlen);
size_t mifare_read_and_write(const unsigned char *cmd, size_t cmd_size,unsigned char *buff);
unsigned char TopUpCardInfor_CPU_jiaotong(unsigned int Value,int type);
unsigned char GetCardType();
unsigned char Card_DebitChas_complex_jiaotong(unsigned int Money,char *Key,unsigned char Keylen);


int MatchTermAID(unsigned char * aid,unsigned char aidLen,TERMAPP_T *pt_app_list,unsigned char app_list_num);
int get_terminal_support_AID(TERMAPP_T **pt_app_list,int *pt_app_num);
int set_terminal_support_AID(TERMAPP_T *pt_app_list,int app_num);

#endif  //__CPU_CARD_OPERATION_H__


