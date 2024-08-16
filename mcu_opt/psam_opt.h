#ifndef _PSAM_OPT_H_
#define _PSAM_OPT_H_

int GetPsamID_ex(unsigned char Psam_index, unsigned char PsamNum[6],unsigned char *p_PsamKeyIndex);
int GetPsamID(void);


int InitPsam(unsigned char Psam_index, int BaudRate);
int Psam_Cmd_Send(unsigned char Psam_index,unsigned char* send_cmd, unsigned char send_cmd_len, char* recv_cmd, unsigned char *recv_cmd_len);


extern unsigned char PsamKeyIndex;    
extern unsigned char PsamNum[6];
#endif
