
#ifndef __APDU_CMD_H__
#define __APDU_CMD_H__

#define Lsh(x,y) (x << y)
#define Rsh(x,y) (x >> y)

typedef struct{
    unsigned char tag[3];//Second uchar '00' means only first one uchar tag.add third uchar to make int variable oven aligned.
    int tag_len;
    unsigned char data[256];
    int data_len;
    unsigned char  bExist;//0-not presented,1-have been existed.    
}DATAELEMENT_T;

typedef struct {
    unsigned char AIDExist;  //0-non exist;1-exist.
    unsigned char AIDLen;
    unsigned char AID[16];   //5-16

    unsigned char LabelExist;  //0-non exist;1-exist.
    unsigned char AppLabelLen;
    unsigned char AppLabel[16];

    unsigned char PreferNameExist;  //0-non exist;1-exist.
    unsigned char PreferNameLen;
    unsigned char PreferName[16];
    
    unsigned char PriorityExist;  //0-non exist;1-exist.
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
}APPDATA_T;


typedef struct {
    
    unsigned char Type;         //1:DDF,2:ADF
    
    unsigned char tag_0x6F_Exist;
    unsigned char tag_0x6F_len;
    unsigned char tag_0x6F_data[256];  //5-16,ADF or DDF name according to Type.


    
    unsigned char tag_0x84_Exist;
    unsigned char tag_0x84_len;
    unsigned char tag_0x84_data[16];  //5-16,ADF or DDF name according to Type.

   
    unsigned char tag_0xA5_Exist;
    unsigned char tag_0xA5_len;
    unsigned char tag_0xA5_data[256]; 


    unsigned char tag_0xBF0C_Exist;
    unsigned char tag_0xBF0C_len;
    unsigned char tag_0xBF0C_data[222];

    unsigned char tag_0x88_Exist;
    unsigned char tag_0x88_len;
    unsigned char tag_0x88_data[1];

    unsigned char tag_0x5F2D_Exist;
    unsigned char tag_0x5F2D_len;
    unsigned char tag_0x5F2D_data[8];
    
    unsigned char tag_0x9F11_Exist;
    unsigned char tag_0x9F11_len;
    unsigned char tag_0x9F11_data[1];

    unsigned char Appnum;
    APPDATA_T AppList[16];
}RECORD_PPSE;


int apdu_cmd1(unsigned char CLA, unsigned char INS,unsigned char P1,unsigned char P2,unsigned char *p_rev_buffer);
int apdu_cmd2(unsigned char CLA, unsigned char INS,unsigned char P1,unsigned char P2,unsigned char Le ,unsigned char *p_rev_buffer);
int apdu_cmd3(unsigned char CLA, unsigned char INS,unsigned char P1,unsigned char P2,unsigned char Lc, unsigned char *data,unsigned char *p_rev_buffer);
int apdu_cmd4(unsigned char CLA, unsigned char INS,unsigned char P1,unsigned char P2,unsigned char Lc, unsigned char *data,unsigned char Le,unsigned char *p_rev_buffer);

int apdu_cmd_select_file(unsigned char p1,unsigned char p2,unsigned char len,const unsigned char *filename,unsigned char *p_rev_buffer,unsigned char *p_SW);

int apdu_cmd_select_APP(const unsigned char *App_name,unsigned char App_len,unsigned char *p_rev_buffer,unsigned char *p_SW);


int apdu_cmd_read_record(unsigned char p1,unsigned char p2,unsigned char *p_record_buffer,unsigned char *p_SW);
int apdu_cmd_get_balance1(unsigned char *p_balance);
int apdu_cmd_get_balance(unsigned char p1,unsigned char Le,unsigned char *p_rev_buffer);

int apdu_cmd_select_PPSE(const unsigned char *PPSE,unsigned char PPSE_len,RECORD_PPSE *p_recordPSE,unsigned char *p_SW);


#endif //__APDU_CMD_H__
